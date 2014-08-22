/*
** $Id: lgc.c,v 2.38.1.1 2007/12/27 13:02:25 roberto Exp $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#include "luacore.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lclass.h"
#include "ltm.h"

#include "lgc.internal.hxx"
#include "lgc.interface.hxx"


// Lua Garbage collector configurations.
// Change these to affect GC performance.
// Note that changing these may trigger hidden bugs!
#define GCSTEPSIZE	1024u
#define GCSWEEPMAX	40
#define GCSWEEPCOST	10
#define GCFINALIZECOST	100

// GC Environment global variables.
globalStatePluginOffset_t _gcEnvPluginOffset = globalStateFactory_t::INVALID_PLUGIN_OFFSET;


FASTAPI void setthreshold( globalStateGCEnv *g )
{
    g->GCthreshold = ( (g->estimate/100) * g->gcpause );
}

#define KEYWEAK         bitmask(KEYWEAKBIT)
#define VALUEWEAK       bitmask(VALUEWEAKBIT)


static void removeentry (Node *n)
{
    lua_assert(ttisnil(gval(n)));

    if (iscollectable(gkey(n)))
    {
        setttype(gkey(n), LUA_TDEADKEY);  /* dead key; remove it */
    }
}

void Udata::MarkGC( global_State *g )
{
    gray2black( this );  /* udata are never gray */

    if ( metatable )
        markobject( g, metatable );

    markobject( g, env );
}

void Class::MarkGC( global_State *g )
{
    gray2black( this );
    TraverseGC( g );
}

void GrayObject::MarkGC( global_State *g )
{
    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        gcEnv->gray.Insert( &this->gclist );
    }
}

void reallymarkobject (global_State *g, GCObject *o)
{
    lua_assert( iswhite( o ) && !isdead( g, o ) );
    white2gray( o );

    o->MarkGC( g );
}

inline static void _marktmuobj( global_State *g, globalStateGCEnv *gcEnv, GCObject *u )
{
    makewhite(g, u);  /* may be marked, if left from previous GC */
    reallymarkobject(g, u);
}

inline static void marktmu( global_State *g, globalStateGCEnv *gcEnv )
{
    GCObject *tmuRoot = (GCObject*)gcEnv->tmudata.GetFirst();

    if ( tmuRoot )
    {
        for ( gcObjList_t::iterator iter( tmuRoot->next ); !iter.IsEnd(); iter.Increment() )
        {
            GCObject *u = (GCObject*)iter.Resolve();

            _marktmuobj( g, gcEnv, u );
        }

        _marktmuobj( g, gcEnv, tmuRoot );
    }
}

void luaC_checkGC( lua_State *L )
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );
    
    if ( gcEnv )
    {
        condhardstacktests( luaD_reallocstack(L, L->stacksize - EXTRA_STACK - 1) );

        if ( g->totalbytes >= gcEnv->GCthreshold )
        {
            luaC_step(L);
        }
    }
}

int luaC_getstate( lua_State *L )
{
    global_State *g = G(L);

    int gcstate = GCSpause;

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        gcstate = gcEnv->gcstate;
    }

    return gcstate;
}

void luaC_setthreshold( lua_State *L, lu_mem threshold )
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        gcEnv->GCthreshold = threshold;
    }
}

lu_byte luaC_white( global_State *g )
{
    lu_byte whiteBits = WHITEBITS;
    {
        globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

        if ( gcEnv )
        {
            whiteBits = ( (g)->currentwhite & WHITEBITS );
        }
    }
    return whiteBits;
}

void luaC_markobject( global_State *g, GCObject *o )
{
    markobject( g, o );
}

/* move `dead' udata that need finalization to list `tmudata' */
size_t luaC_separatefinalization( lua_State *L, bool all )
{
    global_State *g = G(L);

    // Get the GC environment.
    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( !gcEnv )
        return 0;

    size_t deadmem = 0;

    do
    {
        gcObjList_t::removable_iterator iter = gcObjList_t::removable_iterator( &g->mainthread->next );

        bool collected = false;

        while ( !iter.IsEnd() )
        {
            GCObject *curr = (GCObject*)iter.Resolve();

            Class *j;

            switch( curr->tt )
            {
            case LUA_TUSERDATA:
                if (!(iswhite(curr) || all) || isfinalized(gco2u(curr)))
                {
                    iter.Increment();  /* don't bother with them */
                }
                else
                {
                    if (gfasttm(g, gco2u(curr)->metatable, TM_GC) == NULL)
                    {
                        markfinalized(gco2u(curr));  /* don't need finalization */
                        iter.Increment();
                    }
                    else
                    {  /* must call its gc method */
                        deadmem += sizeudata(gco2u(curr));
                        markfinalized(gco2u(curr));

                        // Remove from the garbage collector list.
                        iter.Remove();

                        /* link `curr' at the end of `tmudata' list */
                        gcEnv->tmudata.Insert( curr );
                    }

                    collected = true;
                }
                break;
            case LUA_TCLASS:
                j = gco2j( curr );
                iter.Increment();

                if ( !( iswhite(curr) || all ) ) // we cannot destroy our class yet, bound in thread
                    break;

                // If we clear everything, .lua references scripting shall be removed.
                // System references still count (IncrementMethodStack).
                if ( all )
                    j->ClearReferences( L );

                if ( j->inMethod )
                {
                    // Poll it for next collection cycle
                    j->TraverseGC( g );
                    break;
                }

                markfinalized( j );

                if ( j->destroyed )
                    break;

                if ( !j->PreDestructor( L ) )
                {
                    // Not ready to be destroyed yet
                    j->TraverseGC( g );
                    break;
                }

                deadmem += sizeof( Class );
                collected = true;
                {
                    debughook_shield shield( *L );

                    // Call it's destructor
                    setclvalue( L, L->top, j->destructor );
                    luaD_call( L, L->top++, 0 );
                }
                break;
            default:
                lua_assert( 0 );
            }
        }

        if ( !collected )
            break;
    }
    while ( all );

    return deadmem;
}

int Table::TraverseGC( global_State *g )
{
    int i;
    bool weakkey;
    bool weakvalue;

    if ( metatable )
    {
        markobject( g, metatable );
    }

    const TValue *mode = gfasttm( g, metatable, TM_MODE );

    if ( mode && ttisstring(mode) )
    {  /* is there a weak mode? */
        weakkey = (strchr(svalue(mode), 'k') != NULL);
        weakvalue = (strchr(svalue(mode), 'v') != NULL);

        if ( weakkey || weakvalue )
        {  /* is really weak? */
            this->marked &= ~(KEYWEAK | VALUEWEAK);  /* clear bits */
            this->marked |= cast_byte((weakkey << KEYWEAKBIT) | (weakvalue << VALUEWEAKBIT));

            globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

            if ( gcEnv )
            {
                /* must be cleared after GC, ... */
                /* ... so put in the appropriate list */
                gcEnv->weak.Insert( &this->gclist );
            }
        }

        if ( weakkey && weakvalue )
            return 1;
    }
    else
    {
        weakkey = false;
        weakvalue = false;
    }

    if ( !weakvalue )
    {
        i = sizearray;

        while ( i-- )
        {
            markvalue( g, &array[i] );
        }
    }
    i = sizenode( this );

    while ( i-- )
    {
        Node *n = gnode( this, i );
        lua_assert( ttype(gkey(n)) != LUA_TDEADKEY || ttisnil(gval(n)) );

        if (ttisnil( gval(n)) )
            removeentry( n );  /* remove empty entries */
        else
        {
            lua_assert( !ttisnil(gkey(n)) );

            if (!weakkey)
                markvalue( g, gkey(n) );
            if (!weakvalue)
                markvalue( g, gval(n) );
        }
    }
    return weakkey || weakvalue;
}

static void checkstacksizes (lua_State *L, StkId max)
{
    int ci_used = cast_int( L->ci - L->base_ci );  /* number of `ci' in use */
    int s_used = cast_int( max - L->stack );  /* part of stack in use */

    if ( L->size_ci > LUAI_MAXCALLS )  /* handling overflow? */
    {
        return;  /* do not touch the stacks */
    }

    if ( 4*ci_used < L->size_ci && 2*BASIC_CI_SIZE < L->size_ci )
    {
        luaD_reallocCI(L, L->size_ci/2);  /* still big enough... */
    }
    condhardstacktests(luaD_reallocCI(L, ci_used + 1));

    if ( 4*s_used < L->stacksize && 2*(BASIC_STACK_SIZE+EXTRA_STACK) < L->stacksize )
    {
        luaD_reallocstack(L, L->stacksize/2);  /* still big enough... */
    }
    condhardstacktests(luaD_reallocstack(L, s_used));
}


static void traversestack (global_State *g, lua_State *l)
{
    markvalue( g, &l->storage );
    markvalue( g, gt(l) );

    StkId lim = l->top;

    for ( CallInfo *ci = l->base_ci; ci <= l->ci; ci++ )
    {
        lua_assert(ci->top <= l->stack_last);

        if ( lim < ci->top )
        {
            lim = ci->top;
        }
    }

    {
        StkId o;

        for ( o = l->stack; o < l->top; o++ )
        {
            markvalue(g, o);
        }

        for ( ; o <= lim; o++ )
        {
            setnilvalue(o);
        }
    }

    checkstacksizes(l, lim);
}

size_t Table::Propagate( global_State *g )
{
    if ( TraverseGC( g ) )  /* table is weak? */
    {
        black2gray( this );  /* keep it gray */
    }

    return sizeof(Table) + sizeof(TValue) * sizearray +
        sizeof(Node) * sizenode( this );
}

static void markmt( lua_State *L )
{
    int i;

    for ( i=0; i<NUM_TAGS; i++ )
    {
        if ( L->mt[i] )
            markobject( G(L), L->mt[i] );
    }
}

size_t Dispatch::Propagate( global_State *g )
{
    return sizeof(Dispatch);
}

size_t ClassDispatch::Propagate( global_State *g )
{
    markobject( g, m_class );
    return sizeof(ClassDispatch);
}

size_t ClassMethodDispatch::Propagate( global_State *g )
{
    ClassDispatch::Propagate( g );

    markobject( g, m_prevEnv );

    return sizeof(ClassMethodDispatch);
}

size_t lua_State::Propagate( global_State *g )
{
    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        // Make it gray, again.
        gcEnv->grayagain.Insert( &this->gclist );
    }

    black2gray( this );

    markmt( this );

    traversestack( g, this );
    return sizeof(lua_State) + sizeof(TValue) * stacksize + sizeof(CallInfo) * size_ci;
}


/*
** traverse one gray object, turning it to black.
** Returns `quantity' traversed.
*/
static l_mem propagatemark (global_State *g)
{
    l_mem propagateCount = 0;
    {
        globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

        if ( gcEnv )
        {
            GrayObject *o = LIST_GETITEM( GrayObject, gcEnv->gray.GetFirst(), gclist );
            lua_assert(isgray(o));

            gray2black(o);

            gcEnv->gray.RemoveFirst();

            propagateCount = o->Propagate( g );
        }
    }
    return propagateCount;
}


static size_t propagateall (global_State *g)
{
    size_t m = 0;

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );
   
    if ( gcEnv )
    {
        while ( !gcEnv->gray.IsEmpty() )
        {
            m += propagatemark(g);
        }
    }

    return m;
}


/*
** The next function tells whether a key or value can be cleared from
** a weak table. Non-collectable objects are never removed from weak
** tables. Strings behave as `values', so are never removed too. for
** other objects: if really collected, cannot keep them; for userdata
** being finalized, keep them in keys, but not in values
*/
inline static int iscleared (const TValue *o, int iskey) {
  if (!iscollectable(o)) return 0;
  if (ttisstring(o)) {
    stringmark(tsvalue(o));  /* strings are `values', so are never weak */
    return 0;
  }
  return iswhite(gcvalue(o)) ||
    (ttisuserdata(o) && (!iskey && isfinalized(uvalue(o))));
}


/*
** clear collected entries from weaktables
*/
static void cleartable ( grayObjList_t& theList )
{
    grayObjList_t::iterator iter = theList.GetIterator();

    while ( !iter.IsEnd() )
    {
        GrayObject *grayObj = LIST_GETITEM( GrayObject, iter.Resolve(), gclist );

        Table *h = gco2h(grayObj);
        int i = h->sizearray;

        lua_assert(testbit(h->marked, VALUEWEAKBIT) || testbit(h->marked, KEYWEAKBIT));

        if ( testbit(h->marked, VALUEWEAKBIT) )
        {
            while ( i-- )
            {
                TValue *o = &h->array[i];

                if ( iscleared(o, 0) )  /* value was collected? */
                {
                    setnilvalue(o);  /* remove value */
                }
            }
        }

        i = sizenode(h);

        while ( i-- )
        {
            Node *n = gnode(h, i);

            if ( !ttisnil(gval(n)) && /* non-empty entry? */ (iscleared(key2tval(n), 1) || iscleared(gval(n), 0)) )
            {
                setnilvalue(gval(n));  /* remove value ... */

                removeentry(n);  /* remove entry from table */
            }
        }
        
        iter.Increment();
    }
}

// Dispatch destruction of any GCObject.
static void delete_gcobject( lua_State *L, GCObject *obj )
{
    int type = obj->tt;

    if ( type == LUA_TSTRING )
    {
        TString *theString = (TString*)obj;

        luaS_free( L, theString );
    }
    else if ( type == LUA_TTABLE )
    {
        Table *theTable = (Table*)obj;

        luaH_free( L, theTable );
    }
    else if ( type == LUA_TFUNCTION )
    {
        Closure *theClosure = (Closure*)obj;

        luaF_freeclosure( L, theClosure );
    }
    else if ( type == LUA_TUSERDATA )
    {
        Udata *data = (Udata*)obj;

        luaS_freeudata( L, data );
    }
    else if ( type == LUA_TCLASS )
    {
        Class *theClass = (Class*)obj;

        luaJ_free( L, theClass );
    }
    else if ( type == LUA_TDISPATCH )
    {
        Dispatch *theDispatch = (Dispatch*)obj;

        luaQ_free( L, theDispatch );
    }
    else if ( type == LUA_TTHREAD )
    {
        lua_Thread *theThread = (lua_Thread*)obj;

        luaE_freethread( L, theThread );
    }
    else if ( type == LUA_TPROTO )
    {
        Proto *theProto = (Proto*)obj;

        luaF_freeproto( L, theProto );
    }
    else if ( type == LUA_TUPVAL )
    {
        UpVal *theUpValue = (UpVal*)obj;

        luaF_freeupval( L, theUpValue );
    }
    else
    {
        // Unknown type detected; cannot delete.
        assert( 0 );
    }
}

static void sweeplist( lua_State *L, gcObjList_t::removable_iterator& p, lu_mem count )
{
    global_State *g = G(L);
    int deadmask = otherwhite(g);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        while ( !p.IsEnd() && count-- > 0 )
        {
            GCObject *curr = (GCObject*)p.Resolve();

            if ( curr->tt == LUA_TTHREAD )  /* sweep open upvalues of each thread */
            {
                gcObjList_t::removable_iterator iter = gco2th(curr)->openupval.GetRemovableIterator();

                sweeplist( L, iter, MAX_LUMEM );
            }

            if ( (curr->marked ^ WHITEBITS) & deadmask )
            {  /* not dead? */
                lua_assert( !isdead(g, curr) || testbit(curr->marked, FIXEDBIT) );

                makewhite( g, curr );  /* make it white (for next cycle) */

                p.Increment();
            }
            else
            {  /* must erase `curr' */
                lua_assert( isdead(g, curr) || deadmask == bitmask(SFIXEDBIT) );

                p.Remove();

                if ( curr == gcEnv->rootgc.root )  /* is the first element of the list? */
                {
                    gcEnv->rootgc.root = curr->next;  /* adjust first */
                }

                delete_gcobject( L, curr );
            }
        }
    }
}

static inline void sweepwholelist( lua_State *L, gcObjList_t& theList )
{
    gcObjList_t::removable_iterator iter = theList.GetRemovableIterator();

    sweeplist( L, iter, MAX_LUMEM );
}

static void checkSizes (lua_State *L)
{
    global_State *g = G(L);

    /* check size of string hash */
    if (g->strt.nuse < cast(lu_int32, g->strt.size/4) && g->strt.size > MINSTRTABSIZE*2)
    {
        luaS_resize(L, g->strt.size/2);  /* table is too big */
    }

    luaS_globalgc( L );
}


inline static void GCTM (lua_State *L, global_State *g, globalStateGCEnv *gcEnv)
{
    GCObject *o = (GCObject*)gcEnv->tmudata.GetFirst();
    Udata *udata = rawgco2u( o );

    makewhite( g, o );

    /* remove udata from `tmudata' */
    gcEnv->tmudata.RemoveFirst();

    // Put the tmudata back, after the mainthread.
    gcObjList_t::InsertAfter( g->mainthread, o );

    if ( const TValue *tm = gfasttm(g, udata->metatable, TM_GC) )
    {
        debughook_shield shield( *L );

        setobj2s( L, L->top, tm );
        setuvalue( L, L->top+1, udata );
        L->top += 2;

        luaD_call(L, L->top - 2, 0);
    }
}


/*
** Call all GC tag methods
*/
void luaC_callGCTM (lua_State *L)
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        while ( !gcEnv->tmudata.IsEmpty() )
        {
            GCTM(L, g, gcEnv);
        }
    }
}


void luaC_freeall (lua_State *L)
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        int i;

        g->currentwhite = WHITEBITS | bitmask(SFIXEDBIT);  /* mask to collect all elements */

        sweepwholelist(L, gcEnv->rootgc);

        for (i = 0; i < g->strt.size; i++)  /* free all string lists */
        {
            sweepwholelist(L, g->strt.hash[i]);
        }

        lua_assert(gcEnv->rootgc.GetFirst() == L);
    }
}

void luaC_step (lua_State *L)
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        // We cannot if GCthread is running
        if ( L == gcEnv->GCthread || gcEnv->GCthread->status != THREAD_SUSPENDED )
            return;

        lu_mem lim = (GCSTEPSIZE/100) * gcEnv->gcstepmul;

        if ( lim == 0 )
            lim = (MAX_LUMEM-1)/2;  /* no limit */

        gcEnv->GCcollect = lim;
        gcEnv->gcdept += g->totalbytes - gcEnv->GCthreshold;

        // Execute GCthread stealthy
        gcEnv->GCthread->resume();

        if ( gcEnv->gcstate == GCSpause )
        {
            if ( g->totalbytes < gcEnv->estimate )
                gcEnv->estimate = g->totalbytes;

            setthreshold( gcEnv );
        }
        else
        {
            if ( gcEnv->gcdept < GCSTEPSIZE )
                gcEnv->GCthreshold = g->totalbytes + GCSTEPSIZE;  /* - lim/g->gcstepmul;*/
            else
            {
                gcEnv->gcdept -= GCSTEPSIZE;
                gcEnv->GCthreshold = g->totalbytes;
            }
        }
    }
}


void luaC_finish( lua_State *L )
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        while (gcEnv->gcstate != GCSpause)
        {
            gcEnv->GCthread->resume();
        }
    }
}


void luaC_fullgc (lua_State *L)
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        // Collect infinite memory
        gcEnv->GCcollect = 0xFFFFFFFF;

        /* finish any pending sweep phase */
        luaC_finish( L );

        gcEnv->GCthread->resume();
        
        while ( gcEnv->gcstate != GCSpause )
        {
            gcEnv->GCthread->resume();
        }

        gcEnv->GCcollect = 0;

        setthreshold(gcEnv);
    }
}


void luaC_barrierf (lua_State *L, GCObject *o, GCObject *v)
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
        //lua_assert(g->gcstate != GCSfinalize && g->gcstate != GCSpause);
        lua_assert(o->tt != LUA_TTABLE);

        // The so-called "invariant" is the liveliness of the object. Keeping the invariant
        // means that the runtime knows that the object is required. Under certain GC
        // conditions the invariant can disappear, so special "barriers" are required to
        // bring objects back into visibility.

        // luaC_barrierf is a "front barrier". It is employed for objects that can easily be marked
        // without disturbing the GC runtime. Those must not need propagation.

        // luaC_barrierback is a "back barrier". It shall be used for objects that should be sheduled
        // for marking on the GC thread. This is done solely to increase the propagation awareness
        // of the GC algorithm.

        /* must keep invariant? */
        if (gcEnv->gcstate == GCSpropagate)
        {
            reallymarkobject(g, v);  /* restore invariant */
        }
        else
        {
            /* don't mind */
            makewhite(g, o);  /* mark as white just to avoid other barriers */
        }
    }
}

void luaC_barrier( lua_State *L, GCObject *p, const TValue *v )
{
    if ( valiswhite(v) && isblack(p) )
    {
        luaC_barrierf( L, p, gcvalue(v) );
    }
}

void luaC_objbarrier( lua_State *L, GCObject *p, GCObject *o )
{
    if ( iswhite(o) && isblack(p) )
    {
		luaC_barrierf( L, p, o );
    }
}


void luaC_stringmark( lua_State *L, TString *string )
{
    // Keep crucial strings alive.
    stringmark( string );
}


void luaC_forceupdatef( lua_State *L, GCObject *o )
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        /* must keep invariant? */
        if (gcEnv->gcstate == GCSpropagate)
        {
            reallymarkobject(g, o);  /* restore invariant */
        }
        else
        {
            /* don't mind */
            makewhite(g, o);  /* mark as white just to avoid other barriers */
        }
    }
}


void luaC_barrierback( lua_State *L, GrayObject *o )
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        lua_assert(isblack(o) && !isdead(g, o));
        lua_assert(gcEnv->gcstate != GCSfinalize && gcEnv->gcstate != GCSpause);

        black2gray(o);  /* make GrayObject gray (again) */

        gcEnv->grayagain.Insert( &o->gclist );
    }
}

void luaC_barriert( lua_State *L, Table *t, const TValue *v )
{
    if ( valiswhite(v) && isblack(t) )
    {
	    luaC_barrierback(L,t);
    }
}

void luaC_objbarriert( lua_State *L, Table *t, GCObject *o )
{
    if ( iswhite(o) && isblack(t) )
    {
        luaC_barrierback(L,t);
    }
}


void luaC_register( lua_State *L, GCObject *o, lu_byte tt )
{
    global_State *g = G(L);

    // Put general stuff.
    o->marked = luaC_white(g);
    o->tt = tt;
}


void luaC_link (lua_State *L, GCObject *o, lu_byte tt)
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        gcEnv->rootgc.Insert( o );
    }

    luaC_register( L, o, tt );
}


void luaC_linktmu( lua_State *L, GCObject *o, lu_byte tt )
{
    global_State *g = G(L);

    // Insert tmu items after the mainthread.
    gcObjList_t::InsertAfter( g->mainthread, o );

    luaC_register( L, o, tt );
}


void luaC_linkupval (lua_State *L, UpVal *uv)
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        gcEnv->rootgc.Insert( uv ); /* link upvalue into `rootgc' list */

        if (isgray(uv))
        { 
            if (gcEnv->gcstate == GCSpropagate)
            {
                gray2black(uv);  /* closed upvalues need barrier */
                luaC_barrier(L, uv, uv->v);
            }
            else
            {  /* sweep phase: sweep it (turning it into white) */
                makewhite(g, uv);

                lua_assert(gcEnv->gcstate != GCSfinalize && gcEnv->gcstate != GCSpause);
            }
        }
    }
}

inline void _luaC_paycost( globalStateGCEnv *gcEnv, lua_Thread *L, lu_mem collected )
{
    // Check whether we collected enough memory
    if ( gcEnv->GCcollect <= collected )
    {
        gcEnv->GCcollect = 0;
        L->yield();
    }
    else
    {
        // We continue running
        gcEnv->GCcollect -= collected;
    }
}

void luaC_paycost( global_State *g, lua_Thread *L, lu_mem collected )
{
    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        _luaC_paycost( gcEnv, L, collected );
    }
}

static inline void luaC_processestimate( globalStateGCEnv *gcEnv, size_t mem )
{
    if ( gcEnv->estimate > mem )
        gcEnv->estimate -= mem;
    else
        gcEnv->estimate = 0;
}

static int luaC_runtime( lua_State *L )
{
    global_State *g = G(L);

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        for (;;)
        {
            // Set up the runtime for another collection
            // First we clear lists.
            gcEnv->gray.Clear();
            gcEnv->grayagain.Clear();
            gcEnv->weak.Clear();

            {
                // Do the root marking
                lua_State *main = g->mainthread;
                markobject( g, main );
                markobject( g, L ); // ourselves!
                /* make global table be traversed before main stack */
                markvalue(g, gt(main));
                markvalue(g, registry(main));
                markmt( main );

                // Mark all event functions
                for ( unsigned int n = 0; n < LUA_NUM_EVENTS; n++ )
                {
                    if ( Closure *evtCall = g->events[n] )
                    {
                        markobject( g, evtCall );
                    }
                }
            }

            // Check for active objects
            gcEnv->gcstate = GCSpropagate;

            // Notify the user; he may yield the runtime at his own will (or use lua_gcpaycost for the official API)
            if ( Closure *evtCall = G(L)->events[LUA_EVENT_GC_PROPAGATE] )
            {
                setclvalue( L, L->top++, evtCall );
                lua_call( L, 0, 0 );
            }

            while ( !gcEnv->gray.IsEmpty() )
            {
                _luaC_paycost( gcEnv, (lua_Thread*)L, propagatemark(g) );
            }

            /* no more `gray' objects */
            size_t udsize;  /* total size of userdata to be finalized */
            /* give control to sub-modules to mark their global values */
            {
                luaJ_gcruntime( L );
                luaF_gcruntime( L );
            }
            /* traverse objects caught by write barrier and by 'remarkupvals' */
            propagateall(g);
            /* remark weak tables */
            gcEnv->weak.SwapWith( gcEnv->gray );
            lua_assert(!iswhite(g->mainthread));
            markmt( L );  /* mark basic metatables (again) */
            propagateall(g);
            /* remark gray again */
            gcEnv->grayagain.SwapWith( gcEnv->gray );
            propagateall(g);
            udsize = luaC_separatefinalization(L, false);  /* separate userdata to be finalized */
            marktmu(g, gcEnv);  /* mark `preserved' userdata */
            udsize += propagateall(g);  /* remark, to propagate `preserveness' */
            cleartable(gcEnv->weak);  /* remove collected objects from weak tables */
            /* flip current white */
            g->currentwhite = cast_byte(otherwhite(g));
            gcEnv->sweepstrgc = 0;
            gcEnv->sweepgc = gcEnv->rootgc.GetRemovableIterator();
            gcEnv->gcstate = GCSsweepstring;
            gcEnv->estimate = g->totalbytes;
            luaC_processestimate( gcEnv, udsize );  /* first estimate */

            do
            {
                lu_mem old = g->totalbytes;
                sweepwholelist( L, g->strt.hash[gcEnv->sweepstrgc++] );

                lua_assert(old >= g->totalbytes);
                luaC_processestimate( gcEnv, old - g->totalbytes );

                _luaC_paycost( gcEnv, (lua_Thread*)L, GCSWEEPCOST );
            }
            while ( gcEnv->sweepstrgc < g->strt.size ); // end phase once we swept all strings
            
            // Sweep (free) unused objects
            gcEnv->gcstate = GCSsweep;

            for (;;)
            {
                lu_mem old = g->totalbytes;
                sweeplist(L, gcEnv->sweepgc, GCSWEEPMAX);

                lua_assert( old >= g->totalbytes );
                luaC_processestimate( gcEnv, old - g->totalbytes );

                if ( gcEnv->sweepgc.IsEnd() )
                {  /* nothing more to sweep? */
                    lu_mem old2 = g->totalbytes;

                    checkSizes(L);

                    lua_assert( old2 >= g->totalbytes );
                    luaC_processestimate( gcEnv, old2 - g->totalbytes );
                    break;
                }

                _luaC_paycost( gcEnv, (lua_Thread*)L, GCSWEEPMAX*GCSWEEPCOST );
            }

            // Finalize all executive objects
            gcEnv->gcstate = GCSfinalize;
            
            while ( !gcEnv->tmudata.IsEmpty() )
            {
                GCTM(L, g, gcEnv);

                luaC_processestimate( gcEnv, GCFINALIZECOST );

                _luaC_paycost( gcEnv, (lua_Thread*)L, GCFINALIZECOST );
            }

            gcEnv->gcstate = GCSpause;  /* end collection */
            gcEnv->gcdept = 0;

            // Yield for every run
            ((lua_Thread*)L)->yield();
        }
    }
    return 0;
}

void luaC_init( global_State *g )
{
    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        // Init global variables
        gcEnv->weak.Clear();
        gcEnv->tmudata.Clear();
        gcEnv->GCthreshold = 0;  /* mark it as unfinished state */
        gcEnv->gcstate = GCSpause;
        gcEnv->rootgc.Insert( g->mainthread );
        gcEnv->sweepstrgc = 0;
        gcEnv->sweepgc = gcEnv->rootgc.GetRemovableIterator();
        gcEnv->gray.Clear();
        gcEnv->grayagain.Clear();
        gcEnv->gcpause = LUAI_GCPAUSE;
        gcEnv->gcstepmul = LUAI_GCMUL;
        gcEnv->gcdept = 0;
        gcEnv->GCcollect = 0;
    }
}

void luaC_initthread( global_State *g )
{
    // Initialize thread environment
    LIST_CLEAR( g->threads.root );

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        // Allocate the main garbage collector runtime and set it up
        lua_Thread *L = gcEnv->GCthread = luaE_newthread( g->mainthread );
        
        if ( !L->AllocateRuntime() )
            throw lua_exception( g->mainthread, LUA_ERRRUN, "fatal: could not allocate GCthread" );

        // We will stealth yield and resume; GC has to be a main thread!
        L->SetMainThread( true );

        // Let it enter the runtime without arguments
        lua_pushcclosure( L, luaC_runtime, 0 );
    }
}

static void callallgcTM (lua_State *L, void *ud)
{
    UNUSED(ud);
    luaC_callGCTM(L);  /* call GC metamethods for all udata */
}

void luaC_shutdown( global_State *g )
{
    lua_State *L = NULL;

    globalStateGCEnv *gcEnv = GetGlobalGCEnvironment( g );

    if ( gcEnv )
    {
        L = gcEnv->GCthread;

        // Finish a pending garbage collection cycle.
        // We must do that to not break the integrity of the GC thread.
        luaC_finish( L );
    }
    else
    {
        L = g->mainthread;
    }

    // Terminate all threads (GC should be last)
    // This disables their runtime environments.
    LIST_FOREACH_BEGIN( lua_Thread, g->threads.root, threadNode )
        luaE_terminate( item );
    LIST_FOREACH_END

    if ( gcEnv )
    {
        luaC_separatefinalization( L, true );

        // Make sure we eliminated all classes.
        // If we have not, the runtime is faulty.
        Closure *dfail = g->events[LUA_EVENT_GC_DEALLOC_FAIL];
        
        // Grab all objects and attempt another deallocation.
        if ( dfail )
        {
            bool nonFinalizedObject = false;

            for ( gcObjList_t::iterator iter( g->mainthread->next ); !iter.IsEnd(); iter.Increment() )
            {
                GCObject *o = (GCObject*)iter.Resolve();

                if ( !isfinalized( o ) )
                {
                    nonFinalizedObject = true;

                    try
                    {
                        // Since we are running on the GC thread, collection is disabled by design.
                        // Hence we are safe to loop the GC list.
                        StkId funcPtr = L->top++;

                        setclvalue( L, funcPtr, dfail );
                        setgcvalue( L, L->top++, o );
                        luaD_call( L, funcPtr, 0 );
                    }
                    catch( ... )
                    {
                        // Notify the system that we encountered an exception.
                        lua_assert( 0 );
                    }
                }
            }

            if ( nonFinalizedObject )
            {
                // Attempt to finalize more objects.
                luaC_separatefinalization( L, true );
            }
        }

        // If there still are objects left, the runtime is experiencing undefined behavior.
        // Let us assert the application.
        for ( gcObjList_t::iterator iter( g->mainthread->next ); !iter.IsEnd(); iter.Increment() )
        {
            GCObject *o = (GCObject*)iter.Resolve();

            lua_assert( isfinalized( o ) );
        }
    }

    L->errfunc = 0;  /* no error function during GC metamethods */

    std::string errMsg;

    do
    {  /* repeat until no more errors */
        L->ci = L->base_ci;
        L->base = L->top = L->ci->base;
        L->nCcalls = 0;
    } while (luaD_rawrunprotected(L, callallgcTM, NULL, errMsg, NULL) != 0);

    if ( gcEnv )
    {
        lua_assert(gcEnv->tmudata.IsEmpty() == true);
    }
}

// Module initialization.
void luaC_moduleinit( void )
{
    // NOTE: Lua5.1ex can actually run without GC support and not crash.
#ifndef LUA_EXCLUDE_GARBAGE_COLLECTOR
    // Register the Lua garbage collector runtime.
    _gcEnvPluginOffset =
        globalStateFactory.RegisterStructPlugin <globalStateGCEnv> ( globalStateFactory_t::ANONYMOUS_PLUGIN_ID );
#endif
}

void luaC_moduleshutdown( void )
{
    return;
}