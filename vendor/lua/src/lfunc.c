/*
** $Id: lfunc.c,v 2.12.1.2 2007/12/28 14:58:43 roberto Exp $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/


#include "luacore.h"

#include "lfunc.hxx"

// Global state plugin definitions.
closureEnvConnectingBridge_t closureEnvConnectingBridge( namespaceFactory );

// Closure type information plugin.
closureTypeInfo_t closureTypeInfo( namespaceFactory );

TValue* luaF_getcurraccessor( lua_State *L )
{
    if ( L->ci == L->base_ci )
        return &L->storage;

    if ( CClosure *cl = curr_func( L )->GetCClosure() )
    {
        setgcvalue( L, &L->env, cl->accessor );
        return &L->env;
    }

    return &L->storage;
}

CClosureMethodRedirect* luaF_newCmethodredirect( lua_State *L, GCObject *e, Closure *redirect, Class *j )
{
    global_State *g = G(L);

    closureTypeInfoPlugin_t *typeInfo = closureTypeInfo.GetPluginStruct( g->config );

    if ( typeInfo )
    {
        CClosureMethodRedirect *c = lua_new <CClosureMethodRedirect> ( L, typeInfo->cclosureMethodRedirectTypeInfo );

        if ( c )
        {
            luaC_link(L, c, LUA_TFUNCTION);

            c->genFlags = 0;
            c->isC = true;
            c->isEnvLocked = true;
            c->env = e;
            c->nupvalues = 0;
            c->accessor = gcvalue( luaF_getcurraccessor( L ) );
            c->redirect = redirect;
            c->m_class = j;
        }

        return c;
    }
    return NULL;
}

TValue* CClosureMethodRedirect::ReadUpValue( unsigned char index )
{
    return (TValue*)luaO_nilobject;
}

CClosureMethodRedirectSuper* luaF_newCmethodredirectsuper( lua_State *L, GCObject *e, Closure *redirect, Class *j, Closure *super )
{
    global_State *g = G(L);

    closureTypeInfoPlugin_t *typeInfo = closureTypeInfo.GetPluginStruct( g->config );

    if ( typeInfo )
    {
        CClosureMethodRedirectSuper *c = lua_new <CClosureMethodRedirectSuper> ( L, typeInfo->cclosureMethodRedirectSuperTypeInfo );

        if ( c )
        {
            luaC_link(L, c, LUA_TFUNCTION);

            c->genFlags = 0;
            c->isC = true;
            c->isEnvLocked = true;
            c->env = e;
            c->nupvalues = 0;
            c->accessor = gcvalue( luaF_getcurraccessor( L ) );
            c->redirect = redirect;
            c->m_class = j;
            c->super = super;
        }
        return c;
    }
    return NULL;
}

TValue* CClosureMethodRedirectSuper::ReadUpValue( unsigned char index )
{
    return (TValue*)luaO_nilobject;
}

CClosureBasic::CClosureBasic( void *construction_params )
{
    cclosureSharedConstructionParams *params = (cclosureSharedConstructionParams*)construction_params;

    this->nupvalues = params->nelems;
}

CClosureBasic* luaF_newCclosure (lua_State *L, int nelems, GCObject *e)
{
    global_State *g = G(L);

    closureTypeInfoPlugin_t *typeInfo = closureTypeInfo.GetPluginStruct( g->config );

    if ( typeInfo )
    {
        cclosureSharedConstructionParams params;
        params.nelems = nelems;

        CClosureBasic *c = lua_new <CClosureBasic> ( L, typeInfo->cclosureBasicTypeInfo, &params );

        if ( c )
        {
            luaC_link(L, c, LUA_TFUNCTION);

            c->genFlags = 0;
            c->isC = true;
            c->isEnvLocked = false;
            c->env = e;
            c->accessor = gcvalue( luaF_getcurraccessor( L ) );
        }
        return c;
    }
    return NULL;
}

TValue* CClosureBasic::ReadUpValue( unsigned char index )
{
    if ( index >= nupvalues )
        return (TValue*)luaO_nilobject; // we trust the programmer that he will not write into the nil object (possibly through upvalues)

    return &upvalues[index];
}

CClosureMethod::CClosureMethod( void *construction_params )
{
    cclosureSharedConstructionParams *params = (cclosureSharedConstructionParams*)construction_params;

    this->nupvalues = params->nelems;
}

CClosureMethod* luaF_newCmethod( lua_State *L, int nelems, GCObject *e, Class *j )
{
    global_State *g = G(L);

    closureTypeInfoPlugin_t *typeInfo = closureTypeInfo.GetPluginStruct( g->config );

    if ( typeInfo )
    {
        cclosureSharedConstructionParams params;
        params.nelems = nelems;

        CClosureMethod *c = lua_new <CClosureMethod> ( L, typeInfo->cclosureMethodTypeInfo, &params );
        
        if ( c )
        {
            luaC_link( L, c, LUA_TFUNCTION );

            c->genFlags = 0;
            c->isC = true;
            c->isEnvLocked = true;
            c->env = e;
            c->accessor = gcvalue( luaF_getcurraccessor( L ) );
            c->m_class = j;
        }
        return c;
    }
    return NULL;
}

TValue* CClosureMethod::ReadUpValue( unsigned char index )
{
    if ( index >= nupvalues )
        return (TValue*)luaO_nilobject; // we trust the programmer that he will not write into the nil object (possibly through upvalues)

    return &upvalues[index];
}

CClosureMethodTrans::CClosureMethodTrans( void *construction_params )
{
    cclosureSharedConstructionParams *params = (cclosureSharedConstructionParams*)construction_params;

    this->nupvalues = params->nelems;
}

CClosureMethodTrans* luaF_newCmethodtrans( lua_State *L, int nelems, GCObject *e, Class *j, int trans )
{
    global_State *g = G(L);

    closureTypeInfoPlugin_t *typeInfo = closureTypeInfo.GetPluginStruct( g->config );

    if ( typeInfo )
    {
        cclosureSharedConstructionParams params;
        params.nelems = nelems;

        CClosureMethodTrans *c = lua_new <CClosureMethodTrans> ( L, typeInfo->cclosureMethodTransTypeInfo, &params );

        if ( c )
        {
            luaC_link( L, c, LUA_TFUNCTION );

            c->genFlags = 0;
            c->isC = true;
            c->isEnvLocked = true;
            c->env = e;
            c->nupvalues = cast_byte(nelems);
            c->accessor = gcvalue( luaF_getcurraccessor( L ) );
            c->m_class = j;
            c->trans = trans;
        }
        return c;
    }
    return NULL;
}

TValue* CClosureMethodTrans::ReadUpValue( unsigned char index )
{
    if ( index >= nupvalues )
        return (TValue*)luaO_nilobject; // we trust the programmer that he will not write into the nil object (possibly through upvalues)

    return &upvalues[index];
}

LClosure::LClosure( void *construction_params )
{
    cclosureSharedConstructionParams *params = (cclosureSharedConstructionParams*)construction_params;

    this->nupvalues = params->nelems;
}

LClosure *luaF_newLclosure (lua_State *L, int nelems, GCObject *e)
{
    global_State *g = G(L);

    closureTypeInfoPlugin_t *typeInfo = closureTypeInfo.GetPluginStruct( g->config );

    if ( typeInfo )
    {
        cclosureSharedConstructionParams params;
        params.nelems = nelems;

        LClosure *c = lua_new <LClosure> ( L, typeInfo->lclosureTypeInfo, &params );

        if ( c )
        {
            luaC_link(L, c, LUA_TFUNCTION);

            c->genFlags = 0;
            c->isC = false;
            c->isEnvLocked = false;
            c->env = e;
            c->nupvalues = cast_byte(nelems);

            while ( nelems-- )
            {
                c->upvals[nelems] = NULL;
            }
        }
        return c;
    }
    return NULL;
}

void luaF_freeclosure (lua_State *L, Closure *c)
{
    lua_delete <Closure> ( L, c );
}

TValue* LClosure::ReadUpValue( unsigned char index )
{
    if ( index > nupvalues )
        return (TValue*)luaO_nilobject;

    return upvals[index]->v;
}

UpVal *luaF_newupval (lua_State *L)
{
    global_State *g = G(L);

    closureTypeInfoPlugin_t *typeInfo = closureTypeInfo.GetPluginStruct( g->config );

    if ( typeInfo )
    {
        UpVal *uv = lua_new <UpVal> ( L, typeInfo->upvalueTypeInfo );

        if ( uv )
        {
            luaC_link(L, uv, LUA_TUPVAL);
            uv->v = &uv->u.value;
            setnilvalue(uv->v);
        }
        return uv;
    }
    return NULL;
}

void luaF_freeupval (lua_State *L, UpVal *u)
{
    lua_delete <UpVal> ( L, u );
}

UpVal *luaF_findupval (lua_State *L, StkId level)
{
    global_State *g = G(L);
    gcObjList_t::removable_iterator iter = L->openupval.GetRemovableIterator();
    UpVal *uv = NULL;

    {
        UpVal *p;

        while (!iter.IsEnd() && (p = ngcotouv((GCObject*)iter.Resolve()))->v >= level)
        {
            lua_assert(p->v != &p->u.value);

            if (p->v == level)
            {  /* found a corresponding upvalue? */
                if (isdead(g, p))  /* is it dead? */
                {
                    // TODO: this does not belong here.
                    changewhite(p);  /* ressurect it */
                }

                return p;
            }
            iter.Increment();
        }
    }

    closureTypeInfoPlugin_t *typeInfo = closureTypeInfo.GetPluginStruct( g->config );

    if ( typeInfo )
    {
        uv = lua_new <UpVal> ( L, typeInfo->upvalueTypeInfo );    /* not found: create a new one */
        
        if ( uv )
        {
            luaC_register( L, uv, LUA_TUPVAL );
            uv->v = level;  /* current value lives in the stack */

            iter.Insert( uv );  /* chain it in the proper position */

            {
                globalStateClosureEnvPlugin *closureEnv = closureEnvConnectingBridge.GetPluginStruct( g->config, g );

                if ( closureEnv )
                {
                    uv->u.l.prev = &closureEnv->uvhead;  /* double link it in `uvhead' list */
                    uv->u.l.next = closureEnv->uvhead.u.l.next;
                    uv->u.l.next->u.l.prev = uv;
                    closureEnv->uvhead.u.l.next = uv;

                    lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
                }
                else
                {
                    uv->u.l.next = uv;
                    uv->u.l.prev = uv;
                }
            }
        }
    }
    return uv;
}

static void unlinkupval (UpVal *uv)
{
    lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
    uv->u.l.next->u.l.prev = uv->u.l.prev;  /* remove from `uvhead' list */
    uv->u.l.prev->u.l.next = uv->u.l.next;
}

UpVal::~UpVal()
{
    if ( v != &u.value )  /* is it open? */
    {
        unlinkupval( this );  /* remove from open list */
    }
}

void luaF_close (lua_State *L, StkId level)
{
    UpVal *uv;
    global_State *g = G(L);

    while (L->openupval.GetFirst() != NULL && (uv = ngcotouv(L->openupval.GetFirst()))->v >= level)
    {
        GCObject *o = uv;

        lua_assert(!isblack(o) && uv->v != &uv->u.value);

        L->openupval.RemoveFirst();  /* remove from `open' list */

        if (isdead(g, o))
        {
            luaF_freeupval( L, uv ); /* free upvalue */
        }
        else
        {
            unlinkupval(uv);

            setobj(L, &uv->u.value, uv->v);

            uv->v = &uv->u.value;  /* now current value lives here */
            luaC_linkupval(L, uv);  /* link upvalue into `gcroot' list */
        }
    }
}

Proto *luaF_newproto (lua_State *L)
{
    global_State *g = G(L);

    closureTypeInfoPlugin_t *typeInfo = closureTypeInfo.GetPluginStruct( g->config );

    if ( typeInfo )
    {
        Proto *f = lua_new <Proto> ( L, typeInfo->protoTypeInfo );

        if ( f )
        {
            luaC_link(L, f, LUA_TPROTO);
            f->k = NULL;
            f->sizek = 0;
            f->p = NULL;
            f->sizep = 0;
            f->code = NULL;
            f->sizecode = 0;
            f->sizelineinfo = 0;
            f->sizeupvalues = 0;
            f->nups = 0;
            f->upvalues = NULL;
            f->numparams = 0;
            f->is_vararg = 0;
            f->maxstacksize = 0;
            f->lineinfo = NULL;
            f->sizelocvars = 0;
            f->locvars = NULL;
            f->linedefined = 0;
            f->lastlinedefined = 0;
            f->source = NULL;
        }
        return f;
    }
    return NULL;
}

void luaF_freeproto (lua_State *L, Proto *p)
{
    luaM_freearray(L, p->code, p->sizecode);
    luaM_freearray(L, p->p, p->sizep);
    luaM_freearray(L, p->k, p->sizek);
    luaM_freearray(L, p->lineinfo, p->sizelineinfo);
    luaM_freearray(L, p->locvars, p->sizelocvars);
    luaM_freearray(L, p->upvalues, p->sizeupvalues);

    lua_delete <Proto> ( L, p );
}

Proto::~Proto()
{
}

Closure::~Closure()
{
}

CClosure::~CClosure()
{
}

CClosureMethodRedirect::~CClosureMethodRedirect()
{
}

CClosureMethodRedirectSuper::~CClosureMethodRedirectSuper()
{
}

CClosureBasic::~CClosureBasic()
{
}

CClosureMethodBase::~CClosureMethodBase()
{
}

CClosureMethod::~CClosureMethod()
{
}

CClosureMethodTrans::~CClosureMethodTrans()
{
}

LClosure::~LClosure()
{
}

/*
** Look for n-th local variable at line `line' in function `func'.
** Returns NULL if not found.
*/
const char *luaF_getlocalname (const Proto *f, int local_number, int pc) {
  int i;
  for (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    if (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_number--;
      if (local_number == 0)
        return getstr(f->locvars[i].varname);
    }
  }
  return NULL;  /* not found */
}

// Module initialization.
void luaF_init( lua_config *cfg )
{
    return;
}

void luaF_shutdown( lua_config *cfg )
{
    return;
}

// Runtime initialization.
void luaF_runtimeinit( global_State *g )
{
    return;
}

void luaF_runtimeshutdown( global_State *g )
{
    return;
}