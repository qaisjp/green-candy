#include "luacore.h"

#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lstate.h"
#include "lundump.h"
#include "lvm.h"

#include "lapi.hxx"


/*
** `load' and `call' functions (run Lua code)
*/


#define adjustresults(L,nres) \
    { if (nres == LUA_MULTRET && L->top >= L->ci->top) L->ci->top = L->top; }


#define checkresults(L,na,nr) \
     api_check(L, (nr) == LUA_MULTRET || (L->ci->top - L->top >= (nr) - (na)))
	

LUA_API void lua_call (lua_State *L, int nargs, int nresults)
{
    StkId func;
    lua_lock(L);
    api_checknelems(L, nargs+1);
    checkresults(L, nargs, nresults);
    func = L->top - (nargs+1);
    luaD_call(L, func, nresults);
    adjustresults(L, nresults);
    lua_unlock(L);
}



/*
** Execute a protected call.
*/
struct CallS {  /* data to `f_call' */
  StkId func;
  int nresults;
};


static void f_call (lua_State *L, void *ud) {
  struct CallS *c = cast(struct CallS *, ud);
  luaD_call(L, c->func, c->nresults);
}



LUA_API int lua_pcallex (lua_State *L, int nargs, int nresults, int errfunc, lua_Debug *debug)
{
  struct CallS c;
  int status;
  ptrdiff_t func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  if (errfunc == 0)
    func = 0;
  else
  {
    StkId o = index2adr(L, errfunc);
    api_checkvalidindex(L, o);
    func = savestack(L, o);
  }
  c.func = L->top - (nargs+1);  /* function to be called */
  c.nresults = nresults;
  status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func, debug);
  adjustresults(L, nresults);
  lua_unlock(L);
  return status;
}


LUA_API int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc)
{
    return lua_pcallex( L, nargs, nresults, errfunc, NULL );
}


/*
** Execute a protected C call.
*/
struct CCallS {  /* data to `f_Ccall' */
  lua_CFunction func;
  void *ud;
};


static void f_Ccall (lua_State *L, void *ud) {
  struct CCallS *c = cast(struct CCallS *, ud);
  CClosure *cl;
  cl = luaF_newCclosure(L, 0, getcurrenv(L));
  cl->f = c->func;
  setclvalue(L, L->top, cl);  /* push function */
  api_incr_top(L);
  setpvalue(L->top, c->ud);  /* push only argument */
  api_incr_top(L);
  luaD_call(L, L->top - 2, 0);
}


LUA_API int lua_cpcall (lua_State *L, lua_CFunction func, void *ud) {
  struct CCallS c;
  int status;
  lua_lock(L);
  c.func = func;
  c.ud = ud;
  status = luaD_pcall(L, f_Ccall, &c, savestack(L, L->top), 0, NULL);
  lua_unlock(L);
  return status;
}


LUA_API int lua_load (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname) {
  ZIO z;
  int status;
  lua_lock(L);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname);
  lua_unlock(L);
  return status;
}


LUA_API int lua_dump (lua_State *L, lua_Writer writer, void *data)
{
    int status;
    TValue *o;
    lua_lock(L);

    api_checknelems(L, 1);
    o = L->top - 1;

    if (isLfunction(o))
        status = luaU_dump(L, clvalue(o)->GetLClosure()->p, writer, data, 0);
    else
        status = 1;

    lua_unlock(L);
    return status;
}


LUA_API int  lua_status (lua_State *L)
{
	if ( !L->IsThread() )
		return 0;

	return ((lua_Thread*)L)->status;
}