// Plugin utilities to easy the developer's frustration.
// Reading this code is discouraged, since it may wreck them brainz!!!
#ifndef _LUA_PLUGIN_UTILITIES_
#define _LUA_PLUGIN_UTILITIES_

// Helper structure to handle plugin dependant struct registration.
template <typename structType, typename factoryType>
class PluginDependantStructRegister
{
    typedef typename factoryType::pluginOffset_t pluginOffset_t;

    factoryType *hostFactory;

    pluginOffset_t structPluginOffset;

    unsigned int pluginId;

public:
    inline PluginDependantStructRegister( factoryType& theFactory, unsigned int pluginId = factoryType::ANONYMOUS_PLUGIN_ID )
    {
        this->hostFactory = NULL;
        this->pluginId = pluginId;
        this->structPluginOffset = factoryType::INVALID_PLUGIN_OFFSET;

        RegisterPlugin( theFactory );
    }

    inline PluginDependantStructRegister( unsigned int pluginId = factoryType::ANONYMOUS_PLUGIN_ID )
    {
        this->hostFactory = NULL;
        this->pluginId = pluginId;
        this->structPluginOffset = factoryType::INVALID_PLUGIN_OFFSET;
    }

    inline ~PluginDependantStructRegister( void )
    {
        if ( this->hostFactory && factoryType::IsOffsetValid( this->structPluginOffset ) )
        {
            this->hostFactory->UnregisterPlugin( this->structPluginOffset );
        }
    }

    inline void RegisterPlugin( factoryType& theFactory )
    {
        this->structPluginOffset =
            theFactory.RegisterDependantStructPlugin <structType> ( this->pluginId );

        this->hostFactory = &theFactory;
    }

    inline structType* GetPluginStruct( typename factoryType::hostType_t *hostObj )
    {
        return factoryType::RESOLVE_STRUCT <structType> ( hostObj, this->structPluginOffset );
    }
};

template <typename hostType, typename factoryType, typename structType, typename metaInfoType>
struct factoryMetaDefault
{
    typedef factoryType factoryType;

    typedef factoryType endingPointFactory_t;
    typedef typename endingPointFactory_t::pluginOffset_t endingPointPluginOffset_t;

    endingPointPluginOffset_t endingPointPluginOffset;

    metaInfoType metaInfo;

    inline void Initialize( hostType *namespaceObj )
    {
        // Register our plugin.
        this->endingPointPluginOffset =
            metaInfo.RegisterPlugin <structType> ( namespaceObj, endingPointFactory_t::ANONYMOUS_PLUGIN_ID );
    }

    inline void Shutdown( hostType *namespaceObj )
    {
        // Unregister our plugin again.
        if ( this->endingPointPluginOffset != endingPointFactory_t::INVALID_PLUGIN_OFFSET )
        {
            endingPointFactory_t& endingPointFactory =
                metaInfo.ResolveFactoryLink( *namespaceObj );

            endingPointFactory.UnregisterPlugin( this->endingPointPluginOffset );
        }
    }
};

struct _globalStateStructFactoryMeta
{
    typedef globalStateFactory_t factoryType;

    inline factoryType& ResolveFactoryLink( lua_config& cfgStruct )
    {
        return cfgStruct.globalStateFactory;
    }

    template <typename structType>
    inline globalStatePluginOffset_t RegisterPlugin( lua_config *cfgStruct, unsigned int pluginId )
    {
        return cfgStruct->globalStateFactory.RegisterStructPlugin <structType> ( pluginId );
    }
};

template <typename structType, typename factoryType, typename hostType>
struct globalStateStructFactoryMeta : public factoryMetaDefault <hostType, factoryType, structType, _globalStateStructFactoryMeta>
{
};

struct _globalStateDependantStructFactoryMeta
{
    typedef globalStateFactory_t factoryType;

    inline factoryType& ResolveFactoryLink( lua_config& cfgStruct )
    {
        return cfgStruct.globalStateFactory;
    }

    template <typename structType>
    inline globalStatePluginOffset_t RegisterPlugin( lua_config *cfgStruct, unsigned int pluginId )
    {
        return cfgStruct->globalStateFactory.RegisterDependantStructPlugin <structType> ( pluginId );
    }
};

template <typename structType, typename factoryType, typename hostType>
struct globalStateDependantStructFactoryMeta : public factoryMetaDefault <hostType, factoryType, structType, _globalStateDependantStructFactoryMeta>
{
};

// Helper structure to register a plugin on the interface of another plugin.
template <typename structType, typename factoryMetaType, typename hostFactoryType>
class PluginConnectingBridge
{
public:
    typedef typename factoryMetaType::factoryType endingPointFactory_t;
    typedef typename endingPointFactory_t::pluginOffset_t endingPointPluginOffset_t;

    typedef typename endingPointFactory_t::hostType_t endingPointType_t;

    typedef hostFactoryType hostFactory_t;
    typedef typename hostFactory_t::pluginOffset_t hostPluginOffset_t;

    typedef typename hostFactory_t::hostType_t hostType_t;

private:
    PluginDependantStructRegister <factoryMetaType, hostFactoryType> connectingBridgePlugin;

    typedef factoryMetaType hostFactoryDependantStruct;
    
public:

    inline PluginConnectingBridge( void )
    {
        return;
    }

    inline PluginConnectingBridge( hostFactory_t& hostFactory ) : connectingBridgePlugin( hostFactory )
    {
        return;
    }

    inline void RegisterPluginStruct( hostFactory_t& theFactory )
    {
        connectingBridgePlugin.RegisterPlugin( theFactory );
    }

    inline factoryMetaType* GetMetaStruct( hostType_t *host )
    {
        return connectingBridgePlugin.GetPluginStruct( host );
    }

    inline structType* GetPluginStructFromMetaStruct( factoryMetaType *metaStruct, endingPointType_t *endingPoint )
    {
        return endingPointFactory_t::RESOLVE_STRUCT <structType> ( endingPoint, metaStruct->endingPointPluginOffset );
    }

    inline structType* GetPluginStruct( hostType_t *host, endingPointType_t *endingPoint )
    {
        structType *resultStruct = NULL;
        {
            hostFactoryDependantStruct *metaStruct = GetMetaStruct( host );

            if ( metaStruct )
            {
                resultStruct = GetPluginStructFromMetaStruct( metaStruct, endingPoint );
            }
        }
        return resultStruct;
    }
};

#endif //_LUA_PLUGIN_UTILITIES_