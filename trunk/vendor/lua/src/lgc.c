/*
** $Id: lgc.c,v 2.38.1.1 2007/12/27 13:02:25 roberto Exp $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#include <string.h>

#define lgc_c
#define LUA_CORE

#include "lua.h"

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


#define GCSTEPSIZE	1024u
#define GCSWEEPMAX	40
#define GCSWEEPCOST	10
#define GCFINALIZECOST	100


#define maskmarks	cast_byte(~(bitmask(BLACKBIT)|WHITEBITS))

#define makewhite(g,x)	((x)->marked = cast_byte(((x)->marked & maskmarks) | luaC_white(g)))

#define white2gray(x)	reset2bits((x)->marked, WHITE0BIT, WHITE1BIT)
#define black2gray(x)	resetbit((x)->marked, BLACKBIT)

#define stringmark(s)	reset2bits((s)->marked, WHITE0BIT, WHITE1BIT)


#define isfinalized(u)		testbit((u)->marked, FINALIZEDBIT)
#define markfinalized(u)	l_setbit((u)->marked, FINALIZEDBIT)


#define KEYWEAK         bitmask(KEYWEAKBIT)
#define VALUEWEAK       bitmask(VALUEWEAKBIT)



#define markvalue(g,o) { checkconsistency(o); \
  if (iscollectable(o) && iswhite(gcvalue(o))) reallymarkobject(g,gcvalue(o)); }

#define markobject(g,t) { if (iswhite(t)) \
		reallymarkobject(g, t); }


#define setthreshold(g)  (g->GCthreshold = (g->estimate/100) * g->gcpause)


static void removeentry (Node *n) {
  lua_assert(ttisnil(gval(n)));
  if (iscollectable(gkey(n)))
    setttype(gkey(n), LUA_TDEADKEY);  /* dead key; remove it */
}

static void reallymarkobject( global_State *g, GCObject *o );

template <class type>
void StringTable <type>::TraverseGC( global_State *g )
{
    LIST_FOREACH_BEGIN( item_t, m_list.root, node )
        stringmark( item->key );
    LIST_FOREACH_END
}

int Class::TraverseGC( global_State *g )
{
    if ( destroyed )
        return 0;

    if ( destructor )
        markobject( g, destructor );

    markobject( g, env );
    markobject( g, outenv );
    markobject( g, storage );
    markobject( g, internStorage );

    if ( parent )
    {
        markobject( g, parent );
        markobject( g, childAPI );
    }

    LIST_FOREACH_BEGIN( Class, children.root, child_iter )
        markobject( g, item );
    LIST_FOREACH_END

    forceSuper->TraverseGC( g );

    for ( envList_t::const_iterator iter = envInherit.begin(); iter != envInherit.end(); iter++ )
        markobject( g, *iter );

    return 0;
}

void Udata::MarkGC( global_State *g )
{
    gray2black( this );  /* udata are never gray */

    if ( metatable )
        markobject( g, metatable );

    markobject( g, env );
}

// User-mode function
void Class::Propagate( lua_State *L )
{
    MarkGC( G(L) );
}

void Class::MarkGC( global_State *g )
{
    gray2black( this );
    TraverseGC( g );
}

void UpVal::MarkGC( global_State *g )
{
    markvalue( g, v );

    if ( v == &u.value )  /* closed? */
        gray2black( this );  /* open upvalues are never black */
}

void GrayObject::MarkGC( global_State *g )
{
    gclist = g->gray;
    g->gray = this;
}

inline static void reallymarkobject (global_State *g, GCObject *o)
{
    lua_assert( iswhite( o ) && !isdead( g, o ) );
    white2gray( o );

    o->MarkGC( g );
}

inline static void marktmu( global_State *g )
{
    GCObject *u = g->tmudata;

    if ( !u )
        return;

    do
    {
        u = u->next;
        makewhite(g, u);  /* may be marked, if left from previous GC */
        reallymarkobject(g, u);
    }
    while ( u != g->tmudata );
}

/* move `dead' udata that need finalization to list `tmudata' */
size_t luaC_separatefinalization( lua_State *L, bool all )
{
    global_State *g = G(L);
    size_t deadmem = 0;
    GCObject **p = &g->mainthread->next;
    GCObject *curr;

    while ( (curr = *p) != NULL )
    {
        Class *j;

        switch( curr->tt )
        {
        case LUA_TUSERDATA:
            if (!(iswhite(curr) || all) || isfinalized(gco2u(curr)))
                p = &curr->next;  /* don't bother with them */
            else if (fasttm(L, gco2u(curr)->metatable, TM_GC) == NULL)
            {
                markfinalized(gco2u(curr));  /* don't need finalization */
                p = &curr->next;
            }
            else
            {  /* must call its gc method */
                deadmem += sizeudata(gco2u(curr));
                markfinalized(gco2u(curr));
                *p = curr->next;

                /* link `curr' at the end of `tmudata' list */
                if (g->tmudata == NULL)  /* list is empty? */
                    g->tmudata = curr->next = curr;  /* creates a circular list */
                else
                {
                    curr->next = g->tmudata->next;
                    g->tmudata->next = curr;
                    g->tmudata = curr;
                }
            }
            break;
        case LUA_TCLASS:
            j = gco2j( curr );
            p = &curr->next;

            if ( !( iswhite(curr) || all ) ) // we cannot destroy our class yet, bound in thread
                break;

            // If we clear everything, .lua references do not count
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
    return deadmem;
}

int Table::TraverseGC( global_State *g )
{
    int i;
    bool weakkey;
    bool weakvalue;
    const TValue *mode;

    if ( metatable )
        markobject( g, metatable );

    mode = gfasttm( g, metatable, TM_MODE );

    if ( mode && ttisstring(mode) )
    {  /* is there a weak mode? */
        weakkey = (strchr(svalue(mode), 'k') != NULL);
        weakvalue = (strchr(svalue(mode), 'v') != NULL);

        if ( weakkey || weakvalue )
        {  /* is really weak? */
            marked &= ~(KEYWEAK | VALUEWEAK);  /* clear bits */
            marked |= cast_byte((weakkey << KEYWEAKBIT) | (weakvalue << VALUEWEAKBIT));
            gclist = g->weak;  /* must be cleared after GC, ... */
            g->weak = this;  /* ... so put in the appropriate list */
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
            markvalue( g, &array[i] );
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


/*
** All marks are conditional because a GC may happen while the
** prototype is still being created
*/
int Proto::TraverseGC( global_State *g )
{
    int i;

    if ( source )
        stringmark( source );

    for ( i=0; i<sizek; i++ )  /* mark literals */
        markvalue(g, &k[i]);

    for ( i=0; i<sizeupvalues; i++ )
    {  /* mark upvalue names */
        if ( upvalues[i] )
            stringmark( upvalues[i] );
    }

    for ( i=0; i<sizep; i++ )
    {  /* mark nested protos */
        if ( p[i] )
            markobject( g, p[i] );
    }

    for (i=0; i<sizelocvars; i++)
    {  /* mark local-variable names */
        if ( locvars[i].varname )
            stringmark( locvars[i].varname );
    }

    return 0;
}

int Closure::TraverseGC( global_State *g )
{
    markobject( g, env );
    return 0;
}

int LClosure::TraverseGC( global_State *g )
{
    unsigned int i;
    lua_assert( nupvalues == p->nups );

    markobject( g, p );

    for ( i=0; i<nupvalues; i++ )  /* mark its upvalues */
        markobject( g, upvals[i] );

    return Closure::TraverseGC( g );
}

int CClosure::TraverseGC( global_State *g )
{
    markobject( g, accessor );

    return Closure::TraverseGC( g );
}

size_t CClosureBasic::Propagate( global_State *g )
{
    unsigned int i;

    for ( i=0; i<nupvalues; i++ )  /* mark its upvalues */
        markvalue( g, &upvalues[i] );

    return CClosure::Propagate( g ) + sizeCclosure( nupvalues );
}

size_t CClosureMethodBase::Propagate( global_State *g )
{
    markobject( g, m_class );

    if ( super )
        markobject( g, super );

    return CClosure::Propagate( g );
}

size_t CClosureMethod::Propagate( global_State *g )
{
    unsigned int i;

    for ( i=0; i<nupvalues; i++ )  /* mark its upvalues */
        markvalue( g, &upvalues[i] );

    return CClosureMethodBase::Propagate( g ) + sizeCmethod( nupvalues );
}

size_t CClosureMethodTrans::Propagate( global_State *g )
{
    unsigned int i;

    for ( i=0; i<nupvalues; i++ )  /* mark its upvalues */
        markvalue( g, &upvalues[i] );

    return CClosureMethodBase::Propagate( g ) + sizeCmethodt( nupvalues );
}

static void checkstacksizes (lua_State *L, StkId max) {
  int ci_used = cast_int(L->ci - L->base_ci);  /* number of `ci' in use */
  int s_used = cast_int(max - L->stack);  /* part of stack in use */
  if (L->size_ci > LUAI_MAXCALLS)  /* handling overflow? */
    return;  /* do not touch the stacks */
  if (4*ci_used < L->size_ci && 2*BASIC_CI_SIZE < L->size_ci)
    luaD_reallocCI(L, L->size_ci/2);  /* still big enough... */
  condhardstacktests(luaD_reallocCI(L, ci_used + 1));
  if (4*s_used < L->stacksize &&
      2*(BASIC_STACK_SIZE+EXTRA_STACK) < L->stacksize)
    luaD_reallocstack(L, L->stacksize/2);  /* still big enough... */
  condhardstacktests(luaD_reallocstack(L, s_used));
}


static void traversestack (global_State *g, lua_State *l) {
  StkId o, lim;
  CallInfo *ci;
  markvalue(g, &l->storage);
  markvalue(g, gt(l));
  lim = l->top;
  for (ci = l->base_ci; ci <= l->ci; ci++) {
    lua_assert(ci->top <= l->stack_last);
    if (lim < ci->top) lim = ci->top;
  }
  for (o = l->stack; o < l->top; o++)
    markvalue(g, o);
  for (; o <= lim; o++)
    setnilvalue(o);
  checkstacksizes(l, lim);
}

size_t Table::Propagate( global_State *g )
{
    if ( TraverseGC( g ) )  /* table is weak? */
        black2gray( this );  /* keep it gray */

    return sizeof(Table) + sizeof(TValue) * sizearray +
        sizeof(Node) * sizenode( this );
}

size_t LClosure::Propagate( global_State *g )
{
    TraverseGC( g );
    return sizeLclosure( nupvalues );
}

size_t CClosure::Propagate( global_State *g )
{
    TraverseGC( g );
    return 0;
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
    gclist = g->grayagain;
    g->grayagain = this;

    black2gray( this );

    markmt( this );

    traversestack( g, this );
    return sizeof(lua_State) + sizeof(TValue) * stacksize + sizeof(CallInfo) * size_ci;
}

size_t Proto::Propagate( global_State *g )
{
    TraverseGC( g );

    return sizeof(Proto) + sizeof(Instruction) * sizecode +
        sizeof(Proto *) * sizep +
        sizeof(TValue) * sizek + 
        sizeof(int) * sizelineinfo +
        sizeof(LocVar) * sizelocvars +
        sizeof(TString *) * sizeupvalues;
}


/*
** traverse one gray object, turning it to black.
** Returns `quantity' traversed.
*/
static l_mem propagatemark (global_State *g)
{
    GrayObject *o = g->gray;
    lua_assert(isgray(o));
    gray2black(o);

    g->gray = o->gclist;
    return o->Propagate( g );
}


static size_t propagateall (global_State *g) {
  size_t m = 0;
  while (g->gray) m += propagatemark(g);
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
    stringmark(rawtsvalue(o));  /* strings are `values', so are never weak */
    return 0;
  }
  return iswhite(gcvalue(o)) ||
    (ttisuserdata(o) && (!iskey && isfinalized(uvalue(o))));
}


/*
** clear collected entries from weaktables
*/
static void cleartable (GCObject *l) {
  while (l) {
    Table *h = gco2h(l);
    int i = h->sizearray;
    lua_assert(testbit(h->marked, VALUEWEAKBIT) ||
               testbit(h->marked, KEYWEAKBIT));
    if (testbit(h->marked, VALUEWEAKBIT)) {
      while (i--) {
        TValue *o = &h->array[i];
        if (iscleared(o, 0))  /* value was collected? */
          setnilvalue(o);  /* remove value */
      }
    }
    i = sizenode(h);
    while (i--) {
      Node *n = gnode(h, i);
      if (!ttisnil(gval(n)) &&  /* non-empty entry? */
          (iscleared(key2tval(n), 1) || iscleared(gval(n), 0))) {
        setnilvalue(gval(n));  /* remove value ... */
        removeentry(n);  /* remove entry from table */
      }
    }
    l = h->gclist;
  }
}

#define sweepwholelist(L,p)	sweeplist(L,p,MAX_LUMEM)

static GCObject** sweeplist (lua_State *L, GCObject **p, lu_mem count) {
  GCObject *curr;
  global_State *g = G(L);
  int deadmask = otherwhite(g);
  while ((curr = *p) != NULL && count-- > 0) {
    if (curr->tt == LUA_TTHREAD)  /* sweep open upvalues of each thread */
      sweepwholelist(L, &gco2th(curr)->openupval);
    if ((curr->marked ^ WHITEBITS) & deadmask) {  /* not dead? */
      lua_assert(!isdead(g, curr) || testbit(curr->marked, FIXEDBIT));
      makewhite(g, curr);  /* make it white (for next cycle) */
      p = &curr->next;
    }
    else {  /* must erase `curr' */
      lua_assert(isdead(g, curr) || deadmask == bitmask(SFIXEDBIT));
      *p = curr->next;
      if (curr == g->rootgc)  /* is the first element of the list? */
        g->rootgc = curr->next;  /* adjust first */
      delete curr;
    }
  }
  return p;
}

static void checkSizes (lua_State *L) {
  global_State *g = G(L);
  /* check size of string hash */
  if (g->strt.nuse < cast(lu_int32, g->strt.size/4) &&
      g->strt.size > MINSTRTABSIZE*2)
    luaS_resize(L, g->strt.size/2);  /* table is too big */
  /* check size of buffer */
  if (luaZ_sizebuffer(&g->buff) > LUA_MINBUFFER*2) {  /* buffer too big? */
    size_t newsize = luaZ_sizebuffer(&g->buff) / 2;
    luaZ_resizebuffer(L, &g->buff, newsize);
  }
}


inline static void GCTM (lua_State *L)
{
    global_State *g = G(L);
    GCObject *o = g->tmudata->next;  /* get first element */
    const TValue *tm;
    Udata *udata = rawgco2u( o );

    makewhite( g, o );

    /* remove udata from `tmudata' */
    if (o == g->tmudata)  /* last element? */
        g->tmudata = NULL;
    else
        g->tmudata->next = udata->next;

    udata->next = g->mainthread->next;  /* return it to `root' list */
    g->mainthread->next = o;

    if ( !(tm = fasttm(L, udata->metatable, TM_GC)) )
        return;

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
    while( G(L)->tmudata )
        GCTM(L);
}


void luaC_freeall (lua_State *L) {
  global_State *g = G(L);
  int i;
  g->currentwhite = WHITEBITS | bitmask(SFIXEDBIT);  /* mask to collect all elements */
  sweepwholelist(L, &g->rootgc);
  for (i = 0; i < g->strt.size; i++)  /* free all string lists */
    sweepwholelist(L, &g->strt.hash[i]);
}

static void remarkupvals (global_State *g) {
  UpVal *uv;
  for (uv = g->uvhead.u.l.next; uv != &g->uvhead; uv = uv->u.l.next) {
    lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
    if (isgray(uv))
      markvalue(g, uv->v);
  }
}

void luaC_step (lua_State *L)
{
    global_State *g = G(L);

    // We cannot if GCthread is running
    if ( L == g->GCthread || g->GCthread->status != THREAD_SUSPENDED )
        return;

    lu_mem lim = (GCSTEPSIZE/100) * g->gcstepmul;

    if ( lim == 0 )
        lim = (MAX_LUMEM-1)/2;  /* no limit */

    g->GCcollect = lim;
    g->gcdept += g->totalbytes - g->GCthreshold;

    // Execute GCthread stealthy
    g->GCthread->resume();

    switch( g->gcstate )
    {
    case GCSpause:
        lua_assert( g->totalbytes >= g->estimate );
        setthreshold( g );
        return;
    }

    if ( g->gcdept < GCSTEPSIZE )
        g->GCthreshold = g->totalbytes + GCSTEPSIZE;  /* - lim/g->gcstepmul;*/
    else
    {
        g->gcdept -= GCSTEPSIZE;
        g->GCthreshold = g->totalbytes;
    }
}


void luaC_finish( lua_State *L )
{
    global_State *g = G(L);

    while (g->gcstate != GCSpause)
        g->GCthread->resume();
}


void luaC_fullgc (lua_State *L)
{
    global_State *g = G(L);

    // Collect infinite memory
    g->GCcollect = 0xFFFFFFFF;

    /* finish any pending sweep phase */
    luaC_finish( L );

    g->GCthread->resume();
    
    while ( g->gcstate != GCSpause )
        g->GCthread->resume();

    g->GCcollect = 0;

    setthreshold(g);
}


void luaC_barrierf (lua_State *L, GCObject *o, GCObject *v)
{
    global_State *g = G(L);
    lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
    //lua_assert(g->gcstate != GCSfinalize && g->gcstate != GCSpause);
    lua_assert(ttype(o) != LUA_TTABLE);

    /* must keep invariant? */
    if (g->gcstate == GCSpropagate)
        return reallymarkobject(g, v);  /* restore invariant */

    /* don't mind */
    makewhite(g, o);  /* mark as white just to avoid other barriers */
}


void luaC_forceupdatef( lua_State *L, GCObject *o )
{
    global_State *g = G(L);

    /* must keep invariant? */
    if (g->gcstate == GCSpropagate)
        return reallymarkobject(g, o);  /* restore invariant */

    /* don't mind */
    makewhite(g, o);  /* mark as white just to avoid other barriers */
}


void luaC_barrierback (lua_State *L, Table *t)
{
    global_State *g = G(L);
    lua_assert(isblack(t) && !isdead(g, t));
    lua_assert(g->gcstate != GCSfinalize && g->gcstate != GCSpause);
    black2gray(t);  /* make table gray (again) */
    t->gclist = g->grayagain;
    g->grayagain = t;
}


void luaC_link (lua_State *L, GCObject *o, lu_byte tt)
{
    global_State *g = G(L);
    o->next = g->rootgc;
    g->rootgc = o;
    o->marked = luaC_white(g);
    o->tt = tt;
}


void luaC_linkupval (lua_State *L, UpVal *uv)
{
    global_State *g = G(L);

    uv->next = g->rootgc;  /* link upvalue into `rootgc' list */
    g->rootgc = uv;

    if (isgray(uv))
    { 
        if (g->gcstate == GCSpropagate)
        {
            gray2black(uv);  /* closed upvalues need barrier */
            luaC_barrier(L, uv, uv->v);
        }
        else
        {  /* sweep phase: sweep it (turning it into white) */
            makewhite(g, uv);
            lua_assert(g->gcstate != GCSfinalize && g->gcstate != GCSpause);
        }
    }
}

inline void luaC_paycost( global_State *g, lua_Thread *L, lu_mem collected )
{
    // Check whether we collected enough memory
    if ( g->GCcollect <= collected )
    {
        g->GCcollect = 0;
        L->yield();
        return;
    }
    
    // We rerun
    g->GCcollect -= collected;
}

static inline void luaC_processestimate( global_State *g, size_t mem )
{
    if ( g->estimate > mem )
        g->estimate -= mem;
    else
        g->estimate = 0;
}

static int luaC_runtime( lua_State *L )
{
    global_State *g = G(L);

    for (;;)
    {
        // Set up the runtime for another collection
        g->gray = NULL;
        g->grayagain = NULL;
        g->weak = NULL;

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
                    markobject( g, evtCall );
            }
        }

        // Check for active objects
        g->gcstate = GCSpropagate;

        // Notify the user; he may yield the runtime at his own will (or use lua_gcpaycost for the official API)
        if ( Closure *evtCall = G(L)->events[LUA_EVENT_GC_PROPAGATE] )
        {
            setclvalue( L, L->top++, evtCall );
            lua_call( L, 0, 0 );
        }

        while ( g->gray )
            luaC_paycost( g, (lua_Thread*)L, propagatemark(g) );

        /* no more `gray' objects */
        size_t udsize;  /* total size of userdata to be finalized */
        /* mark specific globals */
        stringmark( g->superCached );   // 'super' used by classes
        /* remark occasional upvalues of (maybe) dead threads */
        remarkupvals(g);
        /* traverse objects cautch by write barrier and by 'remarkupvals' */
        propagateall(g);
        /* remark weak tables */
        g->gray = g->weak;
        g->weak = NULL;
        lua_assert(!iswhite(g->mainthread));
        markmt( L );  /* mark basic metatables (again) */
        propagateall(g);
        /* remark gray again */
        g->gray = g->grayagain;
        g->grayagain = NULL;
        propagateall(g);
        udsize = luaC_separatefinalization(L, false);  /* separate userdata to be finalized */
        marktmu(g);  /* mark `preserved' userdata */
        udsize += propagateall(g);  /* remark, to propagate `preserveness' */
        cleartable(g->weak);  /* remove collected objects from weak tables */
        /* flip current white */
        g->currentwhite = cast_byte(otherwhite(g));
        g->sweepstrgc = 0;
        g->sweepgc = &g->rootgc;
        g->gcstate = GCSsweepstring;
        g->estimate = g->totalbytes;
        luaC_processestimate( g, udsize );  /* first estimate */

        do
        {
            lu_mem old = g->totalbytes;
            sweepwholelist( L, &g->strt.hash[g->sweepstrgc++] );

            lua_assert(old >= g->totalbytes);
            luaC_processestimate( g, old - g->totalbytes );

            luaC_paycost( g, (lua_Thread*)L, GCSWEEPCOST );
        }
        while ( g->sweepstrgc < g->strt.size ); // end phase once we swept all strings
        
        // Sweep (free) unused objects
        g->gcstate = GCSsweep;

        for (;;)
        {
            lu_mem old = g->totalbytes;
            g->sweepgc = sweeplist(L, g->sweepgc, GCSWEEPMAX);

            lua_assert( old >= g->totalbytes );
            luaC_processestimate( g, old - g->totalbytes );

            if ( *g->sweepgc == NULL )
            {  /* nothing more to sweep? */
                checkSizes(L);
                break;
            }

            luaC_paycost( g, (lua_Thread*)L, GCSWEEPMAX*GCSWEEPCOST );
        }

        // Finalize all executive objects
        g->gcstate = GCSfinalize;
        
        while ( g->tmudata )
        {
            GCTM(L);

            luaC_processestimate( g, GCFINALIZECOST );

            luaC_paycost( g, (lua_Thread*)L, GCFINALIZECOST );
        }

        g->gcstate = GCSpause;  /* end collection */
        g->gcdept = 0;

        // Yield for every run
        ((lua_Thread*)L)->yield();
    }
}

void luaC_init( global_State *g )
{
    // Init global variables
    g->GCthreshold = 0;  /* mark it as unfinished state */
    g->gcstate = GCSpause;
    g->rootgc = g->mainthread;
    g->sweepstrgc = 0;
    g->sweepgc = &g->rootgc;
    g->gray = NULL;
    g->grayagain = NULL;
    g->gcpause = LUAI_GCPAUSE;
    g->gcstepmul = LUAI_GCMUL;
    g->gcdept = 0;
    g->GCcollect = 0;
}

void luaC_initthread( global_State *g )
{
    // Initialize thread environment
    LIST_CLEAR( g->threads.root );

    // Allocate the main garbage collector runtime and set it up
    lua_Thread *L = g->GCthread = luaE_newthread( g->mainthread );
    
    if ( !L->AllocateRuntime() )
        throw lua_exception( g->mainthread, LUA_ERRRUN, "fatal: could not allocate GCthread" );

    // We will stealth yield and resume; GC has to be a main thread!
    L->SetMainThread( true );

    // Let it enter the runtime without arguments
    lua_pushcclosure( L, luaC_runtime, 0 );
}

static void callallgcTM (lua_State *L, void *ud)
{
    UNUSED(ud);
    luaC_callGCTM(L);  /* call GC metamethods for all udata */
}

void luaC_shutdown( global_State *g )
{
    lua_Thread *L = g->GCthread;

    // Terminate all threads (GC should be last)
    LIST_FOREACH_BEGIN( lua_Thread, g->threads.root, threadNode )
        luaE_terminate( item );
    LIST_FOREACH_END

    luaC_separatefinalization( L, true );  /* separate udata that have GC metamethods */
    L->errfunc = 0;  /* no error function during GC metamethods */

    std::string errMsg;

    do
    {  /* repeat until no more errors */
        L->ci = L->base_ci;
        L->base = L->top = L->ci->base;
        L->nCcalls = 0;
    } while (luaD_rawrunprotected(L, callallgcTM, NULL, errMsg, NULL) != 0);
}

