// Lua Closure interface local shared include
#ifndef _LUA_CLOSURE_SHARED_INCLUDE_
#define _LUA_CLOSURE_SHARED_INCLUDE_

#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"

#include "lpluginutil.hxx"

struct cclosureSharedConstructionParams
{
    int nelems;
};

// Closure type info plugin.
struct closureTypeInfoPlugin_t
{
    struct cclosureBasicMetaTypeInfo : public LuaTypeSystem::structTypeMetaInfo
    {
        size_t GetTypeSize( void *construction_params ) const
        {
            cclosureSharedConstructionParams *params = (cclosureSharedConstructionParams*)construction_params;

            return sizeCclosure( params->nelems );
        }

        size_t GetTypeSizeByObject( void *mem ) const
        {
            CClosureBasic *cl = (CClosureBasic*)mem;
       
            return sizeCclosure( cl->nupvalues );
        }
    };

    struct cclosureMethodMetaTypeInfo : public LuaTypeSystem::structTypeMetaInfo
    {
        size_t GetTypeSize( void *construction_params ) const
        {
            cclosureSharedConstructionParams *params = (cclosureSharedConstructionParams*)construction_params;

            return sizeCmethod( params->nelems );
        }

        size_t GetTypeSizeByObject( void *mem ) const
        {
            CClosureMethod *method = (CClosureMethod*)mem;

            return sizeCmethod( method->nupvalues );
        }
    };

    struct cclosureMethodTransMetaTypeInfo : public LuaTypeSystem::structTypeMetaInfo
    {
        size_t GetTypeSize( void *construction_params ) const
        {
            cclosureSharedConstructionParams *params = (cclosureSharedConstructionParams*)construction_params;

            return sizeCmethodt( params->nelems );
        }

        size_t GetTypeSizeByObject( void *mem ) const
        {
            CClosureMethodTrans *method = (CClosureMethodTrans*)mem;

            return sizeCmethodt( method->nupvalues );
        }
    };

    struct lclosureMetaTypeInfo : public LuaTypeSystem::structTypeMetaInfo
    {
        size_t GetTypeSize( void *construction_params ) const
        {
            cclosureSharedConstructionParams *params = (cclosureSharedConstructionParams*)construction_params;

            return sizeLclosure( params->nelems );
        }

        size_t GetTypeSizeByObject( void *mem ) const
        {
            LClosure *cl = (LClosure*)mem;
            
            return sizeLclosure( cl->nupvalues );
        }
    };

    // Closure meta stuff.
    cclosureBasicMetaTypeInfo _cclosureBasicMeta;
    cclosureMethodMetaTypeInfo _cclosureMethodMeta;
    cclosureMethodTransMetaTypeInfo _cclosureMethodTransMeta;
    lclosureMetaTypeInfo _lclosureMeta;

    inline void Initialize( lua_config *cfg )
    {
        LuaTypeSystem& typeSys = cfg->typeSys;

        // Set up types.
        closureTypeInfo =
            typeSys.RegisterAbstractType <Closure> ( "closure" );
        cclosureTypeInfo =
            typeSys.RegisterAbstractType <CClosure> ( "cclosure" );
        cclosureMethodRedirectTypeInfo = 
            typeSys.RegisterStructType <CClosureMethodRedirect> ( "cclosure_MethodRedirect" );
        cclosureMethodRedirectSuperTypeInfo =
            typeSys.RegisterStructType <CClosureMethodRedirectSuper> ( "cclosure_MethodRedirectSuper" );
        cclosureBasicTypeInfo =
            typeSys.RegisterDynamicStructType <CClosureBasic> ( "cclosure_Basic", &_cclosureBasicMeta );
        cclosureMethodBaseTypeInfo =
            typeSys.RegisterAbstractType <CClosureMethodBase> ( "cclosure_MethodBase" );
        cclosureMethodTypeInfo =
            typeSys.RegisterDynamicStructType <CClosureMethod> ( "cclosure_Method", &_cclosureMethodMeta );
        cclosureMethodTransTypeInfo =
            typeSys.RegisterDynamicStructType <CClosureMethodTrans> ( "cclosure_MethodTrans", &_cclosureMethodTransMeta );
        lclosureTypeInfo =
            typeSys.RegisterDynamicStructType <LClosure> ( "lclosure", &_lclosureMeta );
        upvalueTypeInfo =
            typeSys.RegisterStructType <UpVal> ( "upvalue" );
        protoTypeInfo =
            typeSys.RegisterStructType <Proto> ( "proto" );

        // Set up inheritance information.
        typeSys.SetTypeInfoInheritingClass(
            closureTypeInfo,
            cfg->grayobjTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            cclosureTypeInfo,
            closureTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            cclosureMethodRedirectTypeInfo,
            cclosureTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            cclosureMethodRedirectSuperTypeInfo,
            cclosureTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            cclosureBasicTypeInfo,
            cclosureTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            cclosureMethodBaseTypeInfo,
            cclosureTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            cclosureMethodTypeInfo,
            cclosureMethodBaseTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            cclosureMethodTransTypeInfo,
            cclosureMethodBaseTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            lclosureTypeInfo,
            closureTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            upvalueTypeInfo,
            cfg->gcobjTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            protoTypeInfo,
            cfg->grayobjTypeInfo
        );
    }

    inline void Shutdown( lua_config *cfg )
    {
        // Destroy types.
        LuaTypeSystem& typeSys = cfg->typeSys;

        typeSys.DeleteType( lclosureTypeInfo );
        typeSys.DeleteType( cclosureMethodTransTypeInfo );
        typeSys.DeleteType( cclosureMethodTypeInfo );
        typeSys.DeleteType( cclosureMethodBaseTypeInfo );
        typeSys.DeleteType( cclosureBasicTypeInfo );
        typeSys.DeleteType( cclosureMethodRedirectSuperTypeInfo );
        typeSys.DeleteType( cclosureMethodRedirectTypeInfo );
        typeSys.DeleteType( cclosureTypeInfo );
        typeSys.DeleteType( closureTypeInfo );
    }

    // Type information.
    LuaTypeSystem::typeInfoBase *closureTypeInfo;
    LuaTypeSystem::typeInfoBase *cclosureTypeInfo;
    LuaTypeSystem::typeInfoBase *lclosureTypeInfo;
    LuaTypeSystem::typeInfoBase *cclosureMethodRedirectTypeInfo;
    LuaTypeSystem::typeInfoBase *cclosureMethodRedirectSuperTypeInfo;
    LuaTypeSystem::typeInfoBase *cclosureBasicTypeInfo;
    LuaTypeSystem::typeInfoBase *cclosureMethodBaseTypeInfo;
    LuaTypeSystem::typeInfoBase *cclosureMethodTypeInfo;
    LuaTypeSystem::typeInfoBase *cclosureMethodTransTypeInfo;
    LuaTypeSystem::typeInfoBase *upvalueTypeInfo;
    LuaTypeSystem::typeInfoBase *protoTypeInfo;
};

typedef PluginDependantStructRegister <closureTypeInfoPlugin_t, namespaceFactory_t> closureTypeInfo_t;

extern closureTypeInfo_t closureTypeInfo;

// Closure global state plugin.
struct globalStateClosureEnvPlugin
{
    inline globalStateClosureEnvPlugin( void )
    {
        uvhead.u.l.prev = &uvhead;
        uvhead.u.l.next = &uvhead;
    }

    UpVal uvhead;  /* head of double-linked list of all open upvalues (NOT AN ACTUAL LUA OBJECT) */
};

typedef PluginConnectingBridge
    <globalStateClosureEnvPlugin,
        globalStateStructFactoryMeta <globalStateClosureEnvPlugin, globalStateFactory_t, lua_config>,
    namespaceFactory_t>
        closureEnvConnectingBridge_t;

extern closureEnvConnectingBridge_t closureEnvConnectingBridge;

#endif //_LUA_CLOSURE_SHARED_INCLUDE_