/*
** $Id: lfunc.c,v 2.12.1.2 2007/12/28 14:58:43 roberto Exp $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/


#include <stddef.h>

#define lfunc_c
#define LUA_CORE

#include "lua.h"

#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"


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
    CClosureMethodRedirect *c = new (L) CClosureMethodRedirect;
    luaC_link(L, c, LUA_TFUNCTION);

    c->genFlags = 0;
    c->isC = true;
    c->isEnvLocked = true;
    c->env = e;
    c->nupvalues = 0;
    c->accessor = gcvalue( luaF_getcurraccessor( L ) );
    c->redirect = redirect;
    c->m_class = j;

    return c;
}

TValue* CClosureMethodRedirect::ReadUpValue( unsigned char index )
{
    return (TValue*)luaO_nilobject;
}

CClosureMethodRedirectSuper* luaF_newCmethodredirectsuper( lua_State *L, GCObject *e, Closure *redirect, Class *j, Closure *super )
{
    CClosureMethodRedirectSuper *c = new (L) CClosureMethodRedirectSuper;
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

    return c;
}

TValue* CClosureMethodRedirectSuper::ReadUpValue( unsigned char index )
{
    return (TValue*)luaO_nilobject;
}

CClosureBasic* luaF_newCclosure (lua_State *L, int nelems, GCObject *e)
{
    CClosureBasic *c = new (L, nelems) CClosureBasic;
    luaC_link(L, c, LUA_TFUNCTION);

    c->genFlags = 0;
    c->isC = true;
    c->isEnvLocked = false;
    c->env = e;
    c->nupvalues = cast_byte(nelems);
    c->accessor = gcvalue( luaF_getcurraccessor( L ) );

    return c;
}

TValue* CClosureBasic::ReadUpValue( unsigned char index )
{
    if ( index >= nupvalues )
        return (TValue*)luaO_nilobject; // we trust the programmer that he will not write into the nil object (possibly through upvalues)

    return &upvalues[index];
}

CClosureMethod* luaF_newCmethod( lua_State *L, int nelems, GCObject *e, Class *j )
{
    CClosureMethod *c = new (L, nelems) CClosureMethod;
    luaC_link( L, c, LUA_TFUNCTION );

    c->genFlags = 0;
    c->isC = true;
    c->isEnvLocked = true;
    c->env = e;
    c->nupvalues = cast_byte(nelems);
    c->accessor = gcvalue( luaF_getcurraccessor( L ) );
    c->m_class = j;

    return c;
}

TValue* CClosureMethod::ReadUpValue( unsigned char index )
{
    if ( index >= nupvalues )
        return (TValue*)luaO_nilobject; // we trust the programmer that he will not write into the nil object (possibly through upvalues)

    return &upvalues[index];
}

CClosureMethodTrans* luaF_newCmethodtrans( lua_State *L, int nelems, GCObject *e, Class *j, int trans )
{
    CClosureMethodTrans *c = new (L, nelems) CClosureMethodTrans;
    luaC_link( L, c, LUA_TFUNCTION );

    c->genFlags = 0;
    c->isC = true;
    c->isEnvLocked = true;
    c->env = e;
    c->nupvalues = cast_byte(nelems);
    c->accessor = gcvalue( luaF_getcurraccessor( L ) );
    c->m_class = j;
    c->trans = trans;

    return c;
}

TValue* CClosureMethodTrans::ReadUpValue( unsigned char index )
{
    if ( index >= nupvalues )
        return (TValue*)luaO_nilobject; // we trust the programmer that he will not write into the nil object (possibly through upvalues)

    return &upvalues[index];
}

LClosure *luaF_newLclosure (lua_State *L, int nelems, GCObject *e)
{
    LClosure *c = new (L, nelems) LClosure;
    luaC_link(L, c, LUA_TFUNCTION);

    c->genFlags = 0;
    c->isC = false;
    c->isEnvLocked = false;
    c->env = e;
    c->nupvalues = cast_byte(nelems);

    while (nelems--)
        c->upvals[nelems] = NULL;

    return c;
}

TValue* LClosure::ReadUpValue( unsigned char index )
{
    if ( index > nupvalues )
        return (TValue*)luaO_nilobject;

    return upvals[index]->v;
}

UpVal *luaF_newupval (lua_State *L)
{
    UpVal *uv = new (L) UpVal;
    luaC_link(L, uv, LUA_TUPVAL);
    uv->v = &uv->u.value;
    setnilvalue(uv->v);
    return uv;
}


UpVal *luaF_findupval (lua_State *L, StkId level) {
  global_State *g = G(L);
  GCObject **pp = &L->openupval;
  UpVal *p;
  UpVal *uv;
  while (*pp != NULL && (p = ngcotouv(*pp))->v >= level) {
    lua_assert(p->v != &p->u.value);
    if (p->v == level) {  /* found a corresponding upvalue? */
      if (isdead(g, p))  /* is it dead? */
        changewhite(p);  /* ressurect it */
      return p;
    }
    pp = &p->next;
  }
  uv = new (L) UpVal;  /* not found: create a new one */
  uv->tt = LUA_TUPVAL;
  uv->marked = luaC_white(g);
  uv->v = level;  /* current value lives in the stack */
  uv->next = *pp;  /* chain it in the proper position */
  *pp = uv;
  uv->u.l.prev = &g->uvhead;  /* double link it in `uvhead' list */
  uv->u.l.next = g->uvhead.u.l.next;
  uv->u.l.next->u.l.prev = uv;
  g->uvhead.u.l.next = uv;
  lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
  return uv;
}


static void unlinkupval (UpVal *uv) {
  lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
  uv->u.l.next->u.l.prev = uv->u.l.prev;  /* remove from `uvhead' list */
  uv->u.l.prev->u.l.next = uv->u.l.next;
}

UpVal::~UpVal()
{
    if ( v != &u.value )  /* is it open? */
        unlinkupval( this );  /* remove from open list */
}

void luaF_close (lua_State *L, StkId level) {
  UpVal *uv;
  global_State *g = G(L);
  while (L->openupval != NULL && (uv = ngcotouv(L->openupval))->v >= level) {
    GCObject *o = uv;
    lua_assert(!isblack(o) && uv->v != &uv->u.value);
    L->openupval = uv->next;  /* remove from `open' list */
    if (isdead(g, o))
    {
      delete o;  /* free upvalue */
    }
    else {
      unlinkupval(uv);
      setobj(L, &uv->u.value, uv->v);
      uv->v = &uv->u.value;  /* now current value lives here */
      luaC_linkupval(L, uv);  /* link upvalue into `gcroot' list */
    }
  }
}

Proto *luaF_newproto (lua_State *L) {
  Proto *f = new (L) Proto;
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
  return f;
}

Proto::~Proto()
{
    luaM_freearray(_lua, code, sizecode, Instruction);
    luaM_freearray(_lua, p, sizep, Proto *);
    luaM_freearray(_lua, k, sizek, TValue);
    luaM_freearray(_lua, lineinfo, sizelineinfo, int);
    luaM_freearray(_lua, locvars, sizelocvars, LocVar);
    luaM_freearray(_lua, upvalues, sizeupvalues, TString *);
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

