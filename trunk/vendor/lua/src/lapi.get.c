#include "luacore.h"

#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lstring.h"
#include "ltable.h"
#include "lclass.h"
#include "lvm.h"

#include "lfunc.class.hxx"

#include "lapi.hxx"


/*
** get functions (Lua -> stack)
*/


LUA_API void lua_gettable (lua_State *L, int idx)
{
    lua_lock(L);
    ConstValueAddress t = index2constadr(L, idx);
    api_checkvalidindex(L, t);
    ValueAddress keyAndVal = index2adr( L, -1 );
    luaV_gettable(L, t, keyAndVal.ConstCast(), keyAndVal);
    lua_unlock(L);
}


LUA_API void lua_getfield (lua_State *L, int idx, const char *k)
{
    lua_lock(L);

    ConstValueAddress t = index2constadr(L, idx);
    api_checkvalidindex(L, t);

    LocalValueAddress key;

    setsvalue(L, key, luaS_new(L, k));

    {
        RtStackAddr rtStack = L->rtStack.LockedAcquisition( L );

        ValueAddress stackItem = newstackslot( L );

        luaV_gettable( L, t, key.ConstCast(), stackItem );
    }

    lua_unlock(L);
}


LUA_API void lua_rawget (lua_State *L, int idx)
{
    lua_lock(L);
    ConstValueAddress t = index2constadr(L, idx);
    api_check(L, ttistable(t));
    {
        RtStackAddr rtStack = L->rtStack.LockedAcquisition( L );

        StkId stackTop = L->GetCurrentStackFrame().TopMutable( L, *rtStack );
        
        ConstValueAddress tableValueAddr = luaH_get(L, hvalue(t), stackTop);

        setobj2s(L, stackTop, tableValueAddr);
    }
    lua_unlock(L);
}


LUA_API void lua_rawgeti (lua_State *L, int idx, int n)
{
    lua_lock(L);

    ConstValueAddress o = index2constadr(L, idx);
    api_check(L, ttistable(o));

    ConstValueAddress tableValueAddr = luaH_getnum(L, hvalue(o), n);

    pushtvalue( L, tableValueAddr );

    lua_unlock(L);
}


LUA_API void lua_createtable (lua_State *L, int narray, int nrec)
{
    lua_lock(L);
    luaC_checkGC(L);
    pushhvalue(L, luaH_new(L, narray, nrec));
    lua_unlock(L);
}


LUA_API int lua_getmetatable (lua_State *L, int objindex)
{
    Table *mt = NULL;
    int res;
    lua_lock(L);

    {
        ConstValueAddress t = index2constadr( L, objindex );

        mt = luaT_getmetabyobj( L, t );
    }

    if ( mt == NULL )
        res = 0;
    else
    {
        pushhvalue(L, mt);
        res = 1;
    }
    lua_unlock(L);
    return res;
}


LUA_API void lua_getfenv (lua_State *L, int idx)
{
    lua_lock(L);

    ConstValueAddress o = index2constadr(L, idx);
    api_checkvalidindex(L, o);

    switch( ttype(o) )
    {
    case LUA_TFUNCTION:
        {
            Closure *cl = clvalue( o );

            // Lua may not retrieve the environment of locked closures
            if ( cl->IsEnvLocked() )
                pushnilvalue( L );
            else
                pushgcvalue(L, cl->env);
        }
        break;
    case LUA_TUSERDATA:
        pushgcvalue(L, uvalue(o)->env);
        break;
    case LUA_TTHREAD:
        pushgcvalue(L, gcvalue(gt(thvalue(o))));
        break;
    case LUA_TCLASS:
        pushgcvalue(L, jvalue(o)->env);
        break;
    default:
        pushnilvalue(L);
        break;
    }
    lua_unlock(L);
}


LUA_API ILuaClass* lua_getmethodclass( lua_State *L )
{
    return ((CClosureMethodBase*)curr_func( L ))->m_class;
}

LUA_API void* lua_getmethodtrans( lua_State *L )
{
    return ((CClosureMethodTrans*)curr_func( L ))->data;
}

LUA_API void lua_pushmethodsuper( lua_State *L )
{
    lua_lock( L );

    CClosureMethodBase *method = (CClosureMethodBase*)curr_func( L );

    if ( method->super )
    {
        pushclvalue( L, method->super );
    }
    else
    {
        pushnilvalue( L );
    }

    lua_unlock( L );
}

LUA_API void lua_getclass( lua_State *L )
{
    lua_lock( L );

    RtCtxItem classIdCtx = index2stackadr( L, -1 );

    StkId classId = classIdCtx.Pointer();

    lua_assert( iscollectable( classId ) );

    Class *j = gcvalue( classId )->GetClass();

    if ( j )
    {
        setjvalue( L, classId, j );
    }
    else
    {
        setnilvalue( classId );
    }

    lua_unlock( L );
}

