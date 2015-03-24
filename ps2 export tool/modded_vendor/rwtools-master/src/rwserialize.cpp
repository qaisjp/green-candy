#include "StdInc.h"

#include "pluginutil.hxx"

namespace rw
{

// Since we do not want to pollute the Interface class, we do things privately.
struct serializationStorePlugin
{
    RwList <serializationProvider> serializers;

    inline void Initialize( Interface *engineInterface )
    {
        LIST_CLEAR( serializers.root );
    }

    inline void Shutdown( Interface *engineInterface )
    {
        // Unregister all serializers.
        LIST_FOREACH_BEGIN( serializationProvider, serializers.root, managerData.managerNode )

            item->managerData.isRegistered = false;

        LIST_FOREACH_END

        LIST_CLEAR( serializers.root );
    }

    inline serializationProvider* FindSerializer( uint32 chunkID, RwTypeSystem::typeInfoBase *rwType )
    {
        LIST_FOREACH_BEGIN( serializationProvider, serializers.root, managerData.managerNode )
        
            if ( item->managerData.chunkID == chunkID && item->managerData.rwType == rwType )
            {
                return item;
            }

        LIST_FOREACH_END

        return NULL;
    }

    inline serializationProvider* FindSerializerByChunkID( uint32 chunkID )
    {
        LIST_FOREACH_BEGIN( serializationProvider, serializers.root, managerData.managerNode )

            if ( item->managerData.chunkID == chunkID )
            {
                return item;
            }

        LIST_FOREACH_END

        return NULL;
    }
};

static PluginDependantStructRegister <serializationStorePlugin, RwInterfaceFactory_t> serializationStoreRegister( engineFactory );

bool Interface::RegisterSerialization( uint32 chunkID, RwTypeSystem::typeInfoBase *rwType, serializationProvider *serializer, eSerializationTypeMode mode )
{
    bool registerSuccess = false;

    serializationStorePlugin *serializeStore = serializationStoreRegister.GetPluginStruct( (EngineInterface*)this );

    if ( serializeStore )
    {
        // Make sure we do not have a serializer that handles this already.
        serializationProvider *alreadyExisting = serializeStore->FindSerializer( chunkID, rwType );

        if ( alreadyExisting == NULL )
        {
            if ( serializer->managerData.isRegistered == false )
            {
                serializer->managerData.chunkID = chunkID;
                serializer->managerData.rwType = rwType;
                serializer->managerData.mode = mode;

                LIST_APPEND( serializeStore->serializers.root, serializer->managerData.managerNode );

                serializer->managerData.isRegistered = true;

                registerSuccess = true;
            }
        }
    }

    return registerSuccess;
}

bool Interface::UnregisterSerialization( uint32 chunkID, RwTypeSystem::typeInfoBase *rwType, serializationProvider *serializer )
{
    bool unregisterSuccess = false;

    serializationStorePlugin *serializeStore = serializationStoreRegister.GetPluginStruct( (EngineInterface*)this );

    if ( serializeStore )
    {
        if ( serializer->managerData.isRegistered == true )
        {
            LIST_REMOVE( serializer->managerData.managerNode );

            serializer->managerData.isRegistered = false;

            unregisterSuccess = true;
        }
    }

    return unregisterSuccess;
}

inline serializationProvider* BrowseForSerializer( Interface *engineInterface, RwObject *objectToStore )
{
    serializationProvider *theSerializer = NULL;

    serializationStorePlugin *serializeStore = serializationStoreRegister.GetPluginStruct( (EngineInterface*)engineInterface );

    GenericRTTI *rttiObj = RwTypeSystem::GetTypeStructFromObject( objectToStore );

    if ( rttiObj )
    {
        RwTypeSystem::typeInfoBase *typeInfo = RwTypeSystem::GetTypeInfoFromTypeStruct( rttiObj );

        if ( serializeStore )
        {
            LIST_FOREACH_BEGIN( serializationProvider, serializeStore->serializers.root, managerData.managerNode )
                
                eSerializationTypeMode typeMode = item->managerData.mode;

                bool isOkay = false;

                if ( typeInfo )
                {
                    if ( typeMode == RWSERIALIZE_INHERIT )
                    {
                        isOkay = ( engineInterface->typeSystem.IsTypeInheritingFrom( item->managerData.rwType, typeInfo ) );
                    }
                    else if ( typeMode == RWSERIALIZE_ISOF )
                    {
                        isOkay = ( engineInterface->typeSystem.IsSameType( item->managerData.rwType, typeInfo ) );
                    }
                }

                if ( isOkay )
                {
                    theSerializer = item;
                    break;
                }

            LIST_FOREACH_END
        }
    }

    return theSerializer;
}

void Interface::SerializeBlock( RwObject *objectToStore, BlockProvider& outputProvider )
{
    // Find a serializer that can handle this object.
    serializationProvider *theSerializer = BrowseForSerializer( this, objectToStore );

    if ( theSerializer )
    {
        // Serialize it!
        bool requiresBlockContext = ( outputProvider.inContext() == false );

        if ( requiresBlockContext )
        {
            outputProvider.EnterContext();

            // We only set chunk meta data if we set the context ourselves.
            outputProvider.setBlockID( theSerializer->managerData.chunkID );
            outputProvider.setBlockVersion( objectToStore->GetEngineVersion() );

            // TODO: maybe handle version in a special way?
        }

        try
        {
            // Call into the serializer.
            theSerializer->Serialize( this, outputProvider, objectToStore );
        }
        catch( ... )
        {
            // If any exception was triggered during serialization, we want to cleanly leave the context
            // and rethrow it.
            if ( requiresBlockContext )
            {
                outputProvider.LeaveContext();
            }

            throw;
        }

        if ( requiresBlockContext )
        {
            outputProvider.LeaveContext();
        }
    }
    else
    {
        throw RwException( "no serializer found for object" );
    } 
}

void Interface::Serialize( RwObject *objectToStore, Stream *outputStream )
{
    BlockProvider mainBlock( outputStream, RWBLOCKMODE_WRITE );

    this->SerializeBlock( objectToStore, mainBlock );
}

RwObject* Interface::DeserializeBlock( BlockProvider& inputProvider )
{
    RwObject *returnObj = NULL;

    // Try reading the block and finding a serializer that can handle it.
    serializationStorePlugin *serializeStore = serializationStoreRegister.GetPluginStruct( (EngineInterface*)this );

    if ( serializeStore )
    {
        // Try entering the block.
        bool requiresBlockContext = ( inputProvider.inContext() == false );

        if ( requiresBlockContext )
        {
            inputProvider.EnterContext();
        }

        try
        {
            // Get its chunk ID and search for a serializer.
            uint32 chunkID = inputProvider.getBlockID();

            serializationProvider *theSerializer = serializeStore->FindSerializerByChunkID( chunkID );

            if ( theSerializer )
            {
                // Create an object for deserialization.
                RwTypeSystem::typeInfoBase *rwTypeInfo = theSerializer->managerData.rwType;

                GenericRTTI *rtObj = this->typeSystem.Construct( this, rwTypeInfo, NULL );

                if ( rtObj )
                {
                    // Cast to the language part, the RwObject.
                    RwObject *rwObj = (RwObject*)RwTypeSystem::GetObjectFromTypeStruct( rtObj );

                    // Make sure we deserialize into the object version of the block.
                    rwObj->SetEngineVersion( inputProvider.getBlockVersion() );

                    try
                    {
                        // Call into the (de-)serializer.
                        theSerializer->Deserialize( this, inputProvider, rwObj );
                    }
                    catch( ... )
                    {
                        // We failed for some reason, so destroy the object again.
                        this->typeSystem.Destroy( rtObj );

                        throw;
                    }

                    // Return our object.
                    returnObj = rwObj;
                }
                else
                {
                    throw RwException( std::string( "failed to allocate '" ) + rwTypeInfo->name + "' object for deserialization" );
                }
            }
            else
            {
                this->PushWarning( "unknown RenderWare stream block" );
            }
        }
        catch( ... )
        {
            // We encountered an exception.
            // For that it is important to cleanly leave the context.
            if ( requiresBlockContext )
            {
                inputProvider.LeaveContext();
            }

            throw;
        }

        // Leave the context.
        if ( requiresBlockContext )
        {
            inputProvider.LeaveContext();
        }
    }
    else
    {
        throw RwException( "no serialization environment" );
    }

    return returnObj;
}

RwObject* Interface::Deserialize( Stream *inputStream )
{
    BlockProvider mainBlock( inputStream, RWBLOCKMODE_READ );

    return this->DeserializeBlock( mainBlock );
}

};