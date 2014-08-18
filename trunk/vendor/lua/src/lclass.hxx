// Lua class shared include file
#ifndef _LUA_CLASS_SHARED_INCLUDE_
#define _LUA_CLASS_SHARED_INCLUDE_

#include "lapi.h"
#include "lgc.h"
#include "lmem.h"
#include "lstate.h"
#include "ltable.h"
#include "lclass.h"
#include "lauxlib.h"
#include "lstring.h"
#include "lfunc.h"
#include "lvm.h"
#include "ldebug.h"

extern globalStatePluginOffset_t _classGlobalStatePlugin;

typedef StaticPluginClassFactory <Class> classObjFactory_t;

struct globalStateClassEnvPlugin
{
    inline globalStateClassEnvPlugin( void )
    {
        this->superCached = NULL;
    }

    classObjFactory_t factory;
    TString *superCached;   // 'super' string
};

inline globalStateClassEnvPlugin* GetGlobalClassEnv( global_State *g )
{
    return globalStateFactory_t::RESOLVE_STRUCT <globalStateClassEnvPlugin> ( g, _classGlobalStatePlugin );
}

inline TString* GetSuperString( lua_State *L )
{
    global_State *g = G(L);

    TString *theString = NULL;
    
    globalStateClassEnvPlugin *classEnv = GetGlobalClassEnv( g );

    if ( classEnv )
    {
        theString = classEnv->superCached;
    }

    if ( !theString )
    {
        const char *_cStr = "super";
        size_t _cStrLen = strlen( _cStr );

        theString = luaS_newlstr( L, _cStr, _cStrLen );

        if ( classEnv )
        {
            classEnv->superCached = theString;
        }
    }

    return theString;
}

#endif //_LUA_CLASS_SHARED_INCLUDE_