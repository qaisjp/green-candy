/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        vendor/lua/src/ldispatch.c
*  PURPOSE:     Lua dispatch extension
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "luacore.h"

#include "lgc.h"
#include "lobject.h"
#include "lstate.h"

#include "ldispatch.class.hxx"

#include "lpluginutil.hxx"

// Lua global_State dispatch type plugin environment
struct dispatchTypeInfoPlugin_t
{
    inline void Initialize( lua_config *cfg )
    {
        // Create type information.
        dispatchTypeInfo = cfg->typeSys.RegisterAbstractType <Dispatch> ( "dispatch" );
        dispatchClassEnvTypeInfo = cfg->typeSys.RegisterStructType <ClassEnvDispatch> ( "dispatch_ClassEnv" );
        dispatchClassOutEnvTypeInfo = cfg->typeSys.RegisterStructType <ClassOutEnvDispatch> ( "dispatch_ClassOutEnv" );
        dispatchClassMethodEnvTypeInfo = cfg->typeSys.RegisterStructType <ClassMethodDispatch> ( "dispatch_ClassMethodEnv" );

        // Set inheritance rules.
        cfg->typeSys.SetTypeInfoInheritingClass(
            dispatchTypeInfo,
            cfg->grayobjTypeInfo
        );
        cfg->typeSys.SetTypeInfoInheritingClass(
            dispatchClassEnvTypeInfo,
            dispatchTypeInfo
        );
        cfg->typeSys.SetTypeInfoInheritingClass(
            dispatchClassOutEnvTypeInfo,
            dispatchTypeInfo
        );
        cfg->typeSys.SetTypeInfoInheritingClass(
            dispatchClassMethodEnvTypeInfo,
            dispatchTypeInfo
        );
    }

    inline void Shutdown( lua_config *cfg )
    {
        // Delete type information.
        cfg->typeSys.DeleteType( dispatchClassMethodEnvTypeInfo );
        cfg->typeSys.DeleteType( dispatchClassOutEnvTypeInfo );
        cfg->typeSys.DeleteType( dispatchClassEnvTypeInfo );
        cfg->typeSys.DeleteType( dispatchTypeInfo );
    }
    
    LuaTypeSystem::typeInfoBase *dispatchTypeInfo;
    LuaTypeSystem::typeInfoBase *dispatchClassEnvTypeInfo;
    LuaTypeSystem::typeInfoBase *dispatchClassOutEnvTypeInfo;
    LuaTypeSystem::typeInfoBase *dispatchClassMethodEnvTypeInfo;
};

PluginDependantStructRegister <dispatchTypeInfoPlugin_t, namespaceFactory_t> dispatchTypeInfoPlugin( namespaceFactory, namespaceFactory_t::ANONYMOUS_PLUGIN_ID );

ClassEnvDispatch* luaQ_newclassenv( lua_State *L, Class *j )
{
    ClassEnvDispatch *outObj = NULL;
    {
        global_State *g = G(L);

        dispatchTypeInfoPlugin_t *dispatchEnv = dispatchTypeInfoPlugin.GetPluginStruct( g->config );

        if ( dispatchEnv )
        {
            ClassEnvDispatch *q = lua_new <ClassEnvDispatch> ( L, dispatchEnv->dispatchClassEnvTypeInfo );

            if ( q )
            {
                luaC_link( g, q, LUA_TDISPATCH );
                q->m_class = j;

                outObj = q;
            }
        }
    }
    return outObj;
}

ClassOutEnvDispatch* luaQ_newclassoutenv( lua_State *L, Class *j )
{
    ClassOutEnvDispatch *outObj = NULL;
    {
        global_State *g = G(L);

        dispatchTypeInfoPlugin_t *dispatchEnv = dispatchTypeInfoPlugin.GetPluginStruct( g->config );

        if ( dispatchEnv )
        {
            ClassOutEnvDispatch *q = lua_new <ClassOutEnvDispatch> ( L, dispatchEnv->dispatchClassOutEnvTypeInfo );

            if ( q )
            {
                luaC_link( g, q, LUA_TDISPATCH );
                q->m_class = j;

                outObj = q;
            }
        }
    }
    return outObj;
}

ClassMethodDispatch* luaQ_newclassmethodenv( lua_State *L, Class *j, GCObject *prevEnv )
{
    ClassMethodDispatch *outObj = NULL;
    {
        global_State *g = G(L);

        dispatchTypeInfoPlugin_t *dispatchEnv = dispatchTypeInfoPlugin.GetPluginStruct( g->config );

        if ( dispatchEnv )
        {
            ClassMethodDispatch *q = lua_new <ClassMethodDispatch> ( L, dispatchEnv->dispatchClassMethodEnvTypeInfo );

            if ( q )
            {
                luaC_link( g, q, LUA_TDISPATCH );
                q->m_class = j;
                luaC_objbarrier( L, q, j );
                q->m_prevEnv = prevEnv;
                luaC_objbarrier( L, q, prevEnv );

                outObj = q;
            }
        }
    }
    return outObj;
}

void luaQ_free( lua_State *L, Dispatch *q )
{
    // Free using default operator.
    lua_delete <Dispatch> ( L, q );
}

// Module initialization.
void luaQ_init( lua_config *cfg )
{
    return;
}

void luaQ_shutdown( lua_config *cfg )
{
    return;
}

// Runtime initialization.
void luaQ_runtimeinit( global_State *g )
{
    return;
}

void luaQ_runtimeshutdown( global_State *g )
{
    return;
}