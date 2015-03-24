#include <cstring>
#include <cstdlib>
#include <fstream>
#include <assert.h>
#include <bitset>
#define _USE_MATH_DEFINES
#include <math.h>
#include <map>
#include <algorithm>
#include <cmath>

#include "StdInc.h"

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"

#include "pluginutil.hxx"

#include "txdread.common.hxx"

#include "txdread.d3d.dxt.hxx"

namespace rw
{

/*
 * Texture Dictionary
 */

TexDictionary* texDictionaryStreamPlugin::CreateTexDictionary( Interface *engineInterface ) const
{
    GenericRTTI *rttiObj = engineInterface->typeSystem.Construct( engineInterface, this->txdTypeInfo, NULL );

    if ( rttiObj == NULL )
    {
        return NULL;
    }
    
    TexDictionary *txdObj = (TexDictionary*)RwTypeSystem::GetObjectFromTypeStruct( rttiObj );

    return txdObj;
}

inline bool isRwObjectInheritingFrom( Interface *engineInterface, RwObject *rwObj, RwTypeSystem::typeInfoBase *baseType )
{
    GenericRTTI *rtObj = engineInterface->typeSystem.GetTypeStructFromAbstractObject( rwObj );

    if ( rtObj )
    {
        // Check whether the type of the dynamic object matches that of TXD.
        RwTypeSystem::typeInfoBase *objTypeInfo = RwTypeSystem::GetTypeInfoFromTypeStruct( rtObj );

        if ( engineInterface->typeSystem.IsTypeInheritingFrom( baseType, objTypeInfo ) )
        {
            return true;
        }
    }

    return false;
}

TexDictionary* texDictionaryStreamPlugin::ToTexDictionary( Interface *engineInterface, RwObject *rwObj )
{
    if ( isRwObjectInheritingFrom( engineInterface, rwObj, this->txdTypeInfo ) )
    {
        return (TexDictionary*)rwObj;
    }

    return NULL;
}

void texDictionaryStreamPlugin::Deserialize( Interface *engineInterface, BlockProvider& inputProvider, RwObject *objectToDeserialize ) const
{
    // Cast our object.
    TexDictionary *txdObj = (TexDictionary*)objectToDeserialize;

    // Read the textures.
    {
        BlockProvider texDictMetaStructBlock( &inputProvider );

        uint32 textureBlockCount = 0;
        bool requiresRecommendedPlatform = true;

        texDictMetaStructBlock.EnterContext();

        if ( texDictMetaStructBlock.getBlockID() == CHUNK_STRUCT )
        {
            // Read the header block depending on version.
            LibraryVersion libVer = texDictMetaStructBlock.getBlockVersion();

            if (libVer.rwLibMinor <= 5)
            {
                textureBlockCount = texDictMetaStructBlock.readUInt32();
            }
            else
            {
                textureBlockCount = texDictMetaStructBlock.readUInt16();
                uint16 recommendedPlatform = texDictMetaStructBlock.readUInt16();

                // So if there is a recommended platform set, we will also give it one if we will write it.
                requiresRecommendedPlatform = ( recommendedPlatform != 0 );
            }
        }
        else
        {
            engineInterface->PushWarning( "could not find texture dictionary meta information" );
        }

        txdObj->hasRecommendedPlatform = requiresRecommendedPlatform;

        // We finished reading meta data.
        texDictMetaStructBlock.LeaveContext();

        // Now follow multiple TEXTURENATIVE blocks.
        // Deserialize all of them.

        for ( uint32 n = 0; n < textureBlockCount; n++ )
        {
            BlockProvider textureNativeBlock( &inputProvider );

            // Deserialize this block.
            RwObject *rwObj = NULL;

            std::string errDebugMsg;

            try
            {
                rwObj = engineInterface->DeserializeBlock( textureNativeBlock );
            }
            catch( RwException& except )
            {
                // Catch the exception and try to continue.
                rwObj = NULL;

                if ( textureNativeBlock.doesIgnoreBlockRegions() )
                {
                    // If we failed any texture parsing in the "ignoreBlockRegions" parse mode,
                    // there is no point in continuing, since the environment does not recover.
                    throw;
                }

                errDebugMsg = except.message;
            }

            if ( rwObj )
            {
                // If it is a texture, add it to our TXD.
                bool hasBeenAddedToTXD = false;

                GenericRTTI *rttiObj = RwTypeSystem::GetTypeStructFromObject( rwObj );

                RwTypeSystem::typeInfoBase *typeInfo = RwTypeSystem::GetTypeInfoFromTypeStruct( rttiObj );

                if ( engineInterface->typeSystem.IsTypeInheritingFrom( engineInterface->textureTypeInfo, typeInfo ) )
                {
                    TextureBase *texture = (TextureBase*)rwObj;

                    texture->AddToDictionary( txdObj );

                    hasBeenAddedToTXD = true;
                }

                // If it has not been added, delete it.
                if ( hasBeenAddedToTXD == false )
                {
                    engineInterface->DeleteRwObject( rwObj );
                }
            }
            else
            {
                std::string pushWarning;

                if ( errDebugMsg.empty() == false )
                {
                    pushWarning = "texture native reading failure: ";
                    pushWarning += errDebugMsg;
                }
                else
                {
                    pushWarning = "failed to deserialize texture native block in texture dictionary";
                }

                engineInterface->PushWarning( pushWarning.c_str() );
            }
        }
    }

    // Read extensions.
    engineInterface->DeserializeExtensions( txdObj, inputProvider );
}

TexDictionary::TexDictionary( const TexDictionary& right ) : RwObject( right )
{
    // Create a new dictionary with all the textures.
    this->hasRecommendedPlatform = right.hasRecommendedPlatform;
    
    this->numTextures = 0;

    LIST_CLEAR( textures.root );

    Interface *engineInterface = right.engineInterface;

    LIST_FOREACH_BEGIN( TextureBase, right.textures.root, texDictNode )

        TextureBase *texture = item;

        // Clone the texture and insert it into us.
        TextureBase *newTex = (TextureBase*)engineInterface->CloneRwObject( texture );

        if ( newTex )
        {
            newTex->AddToDictionary( this );
        }

    LIST_FOREACH_END
}

TexDictionary::~TexDictionary( void )
{
    // Delete all textures that are part of this dictionary.
    while ( LIST_EMPTY( this->textures.root ) == false )
    {
        TextureBase *theTexture = LIST_GETITEM( TextureBase, this->textures.root.next, texDictNode );

        // Delete us.
        // This should automatically remove us from this TXD.
        this->engineInterface->DeleteRwObject( theTexture );
    }
}

static PluginDependantStructRegister <texDictionaryStreamPlugin, RwInterfaceFactory_t> texDictionaryStreamStore( engineFactory );

void initializeTXDEnvironment( Interface *theInterface )
{
    texDictionaryStreamPlugin *txdStream = texDictionaryStreamStore.GetPluginStruct( (EngineInterface*)theInterface );

    if ( txdStream )
    {
        theInterface->RegisterSerialization( CHUNK_TEXDICTIONARY, txdStream->txdTypeInfo, txdStream, RWSERIALIZE_ISOF );
    }
}

void shutdownTXDEnvironment( Interface *theInterface )
{
    texDictionaryStreamPlugin *txdStream = texDictionaryStreamStore.GetPluginStruct( (EngineInterface*)theInterface );

    if ( txdStream )
    {
        theInterface->UnregisterSerialization( CHUNK_TEXDICTIONARY, txdStream->txdTypeInfo, txdStream );
    }
}

void TexDictionary::clear(void)
{
	// We remove the links of all textures inside of us.
    while ( LIST_EMPTY( this->textures.root ) == false )
    {
        TextureBase *texture = LIST_GETITEM( TextureBase, this->textures.root.next, texDictNode );

        // Call the texture's own removal.
        texture->RemoveFromDictionary();
    }
}

TexDictionary* CreateTexDictionary( Interface *engineInterface )
{
    TexDictionary *texDictOut = NULL;

    texDictionaryStreamPlugin *txdStream = texDictionaryStreamStore.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( txdStream )
    {
        texDictOut = txdStream->CreateTexDictionary( engineInterface );
    }

    return texDictOut;
}

TexDictionary* ToTexDictionary( Interface *engineInterface, RwObject *rwObj )
{
    TexDictionary *texDictOut = NULL;

    texDictionaryStreamPlugin *txdStream = texDictionaryStreamStore.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( txdStream )
    {
        texDictOut = txdStream->ToTexDictionary( engineInterface, rwObj );
    }

    return texDictOut;
}

/*
 * Texture Base
 */

TextureBase::TextureBase( const TextureBase& right ) : RwObject( right )
{
    // General cloning business.
    this->texRaster = AcquireRaster( right.texRaster );
    this->name = right.name;
    this->maskName = right.maskName;
    this->filterMode = right.filterMode;
    this->uAddressing = right.uAddressing;
    this->vAddressing = right.vAddressing;
    
    // We do not want to belong to a TXD by default.
    // Even if the original texture belonged to one.
    this->texDict = NULL;
}

TextureBase::~TextureBase( void )
{
    // Clear our raster.
    if ( Raster *texRaster = this->texRaster )
    {
        DeleteRaster( texRaster );

        this->texRaster = NULL;
    }

    // Make sure we are not in a texture dictionary.
    this->RemoveFromDictionary();
}

void TextureBase::AddToDictionary( TexDictionary *dict )
{
    this->RemoveFromDictionary();

    // Note: original RenderWare performs an insert, not an append.
    // I switched this around, so that textures stay in the correct order.
    LIST_APPEND( dict->textures.root, texDictNode );

    dict->numTextures++;

    this->texDict = dict;
}

void TextureBase::RemoveFromDictionary( void )
{
    TexDictionary *belongingTXD = this->texDict;

    if ( belongingTXD != NULL )
    {
        LIST_REMOVE( this->texDictNode );

        belongingTXD->numTextures--;

        this->texDict = NULL;
    }
}

TexDictionary* TextureBase::GetTexDictionary( void ) const
{
    return this->texDict;
}

TextureBase* CreateTexture( Interface *engineInterface, Raster *texRaster )
{
    TextureBase *textureOut = NULL;

    if ( RwTypeSystem::typeInfoBase *textureTypeInfo = engineInterface->textureTypeInfo )
    {
        GenericRTTI *rtObj = engineInterface->typeSystem.Construct( engineInterface, textureTypeInfo, NULL );

        if ( rtObj )
        {
            textureOut = (TextureBase*)RwTypeSystem::GetObjectFromTypeStruct( rtObj );

            // Set the raster into the texture.
            textureOut->texRaster = texRaster;
        }
    }

    return textureOut;
}

TextureBase* ToTexture( Interface *engineInterface, RwObject *rwObj )
{
    if ( isRwObjectInheritingFrom( engineInterface, rwObj, engineInterface->textureTypeInfo ) )
    {
        return (TextureBase*)rwObj;
    }

    return NULL;
}

/*
 * Native Texture
 */

bool RegisterNativeTextureType( Interface *engineInterface, const char *nativeName, texNativeTypeProvider *typeProvider )
{
    return false;
}

static PlatformTexture* CreateNativeTexture( Interface *engineInterface, RwTypeSystem::typeInfoBase *nativeTexType )
{
    PlatformTexture *texOut = NULL;
    {
        GenericRTTI *rtObj = engineInterface->typeSystem.Construct( engineInterface, nativeTexType, NULL );

        if ( rtObj )
        {
            texOut = (PlatformTexture*)RwTypeSystem::GetObjectFromTypeStruct( rtObj );
        }
    }
    return texOut;
}

static PlatformTexture* CloneNativeTexture( Interface *engineInterface, const PlatformTexture *srcNativeTex )
{
    PlatformTexture *texOut = NULL;
    {
        const GenericRTTI *srcRtObj = engineInterface->typeSystem.GetTypeStructFromConstAbstractObject( srcNativeTex );

        if ( srcRtObj )
        {
            GenericRTTI *dstRtObj = engineInterface->typeSystem.Clone( srcRtObj );

            if ( dstRtObj )
            {
                texOut = (PlatformTexture*)RwTypeSystem::GetObjectFromTypeStruct( dstRtObj );
            }
        }
    }
    return texOut;
}

static void DeleteNativeTexture( Interface *engineInterface, PlatformTexture *nativeTexture )
{
    GenericRTTI *rtObj = engineInterface->typeSystem.GetTypeStructFromAbstractObject( nativeTexture );

    if ( rtObj )
    {
        engineInterface->typeSystem.Destroy( rtObj );
    }
}

struct nativeTextureStreamPlugin : public serializationProvider
{
    inline void Initialize( Interface *engineInterface )
    {
        this->platformTexType = engineInterface->typeSystem.RegisterAbstractType <PlatformTexture> ( "native_texture" );

        // Initialize the list that will keep all native texture types.
        LIST_CLEAR( this->texNativeTypes.root );
    }

    inline void Shutdown( Interface *engineInterface )
    {
        // Unregister all type providers.
        LIST_FOREACH_BEGIN( texNativeTypeProvider, this->texNativeTypes.root, managerData.managerNode )
            // We just set them to false.
            item->managerData.isRegistered = false;
        LIST_FOREACH_END

        LIST_CLEAR( this->texNativeTypes.root );

        if ( RwTypeSystem::typeInfoBase *platformTexType = this->platformTexType )
        {
            engineInterface->typeSystem.DeleteType( platformTexType );

            this->platformTexType = NULL;
        }
    }

    void Serialize( Interface *engineInterface, BlockProvider& outputProvider, RwObject *objectToStore ) const
    {
        // Make sure we are a valid texture.
        if ( !isRwObjectInheritingFrom( engineInterface, objectToStore, engineInterface->textureTypeInfo ) )
        {
            throw RwException( "invalid RwObject at texture serialization (not a texture base)" );
        }

        TextureBase *theTexture = (TextureBase*)objectToStore;

        // Fetch the raster, which is the virtual interface to the platform texel data.
        if ( Raster *texRaster = theTexture->texRaster )
        {
            // The raster also requires GPU native data, the heart of the texture.
            if ( PlatformTexture *nativeTex = texRaster->platformData )
            {
                // Get the type information of this texture and find its type provider.
                GenericRTTI *rtObj = RwTypeSystem::GetTypeStructFromObject( nativeTex );

                RwTypeSystem::typeInfoBase *nativeTypeInfo = RwTypeSystem::GetTypeInfoFromTypeStruct( rtObj );

                // Attempt to cast the type interface to our native texture type provider.
                // If successful, it indeeed is a native texture.
                {
                    nativeTextureCustomTypeInterface *nativeTexTypeInterface = dynamic_cast <nativeTextureCustomTypeInterface*> ( nativeTypeInfo->tInterface );

                    if ( nativeTexTypeInterface )
                    {
                        // Serialize the texture.
                        nativeTexTypeInterface->texTypeProvider->SerializeTexture( theTexture, nativeTex, outputProvider );
                    }
                    else
                    {
                        throw RwException( "could not serialize texture: native data is not valid" );
                    }
                }
            }
            else
            {
                throw RwException( "could not serialize texture: no native data" );
            }
        }
        else
        {
            throw RwException( "could not serialize texture: no raster attached" );
        }
    }

    struct interestedNativeType
    {
        eTexNativeCompatibility typeOfInterest;
        texNativeTypeProvider *interestedParty;
    };

    struct QueuedWarningHandler : public WarningHandler
    {
        void OnWarningMessage( const std::string& theMessage )
        {
            this->message_list.push_back( theMessage );
        }

        typedef std::vector <std::string> messages_t;

        messages_t message_list;
    };

    void Deserialize( Interface *engineInterface, BlockProvider& inputProvider, RwObject *objectToDeserialize ) const
    {
        // This is a pretty complicated algorithm that will need revision later on, when networked streams are allowed.
        // It is required because tex native rules have been violated by War Drum Studios.
        // First, we need to analyze the given block; this is done by getting candidates from texNativeTypes that
        // have an interest in this block.
        typedef std::vector <interestedNativeType> interestList_t;

        interestList_t interestedTypeProviders;

        LIST_FOREACH_BEGIN( texNativeTypeProvider, this->texNativeTypes.root, managerData.managerNode )

            // Reset the input provider.
            inputProvider.seek( 0, RWSEEK_BEG );

            // Check this block's compatibility and if it is something, register it.
            eTexNativeCompatibility thisCompat = RWTEXCOMPAT_NONE;

            try
            {
                thisCompat = item->IsCompatibleTextureBlock( inputProvider );
            }
            catch( RwException& )
            {
                // If there was any exception, there is no point in selecting this provider
                // as a valid candidate.
                thisCompat = RWTEXCOMPAT_NONE;
            }

            if ( thisCompat != RWTEXCOMPAT_NONE )
            {
                interestedNativeType nativeInfo;
                nativeInfo.typeOfInterest = thisCompat;
                nativeInfo.interestedParty = item;

                interestedTypeProviders.push_back( nativeInfo );
            }

        LIST_FOREACH_END

        // Check whether the interest is valid.
        // There may only be one full-on interested party, but there can be multiple "maybe".
        struct providerInfo_t
        {
            inline providerInfo_t( void )
            {
                this->theProvider = NULL;
            }

            texNativeTypeProvider *theProvider;
            
            QueuedWarningHandler _warningQueue;
        };

        typedef std::vector <providerInfo_t> providers_t;

        providers_t maybeProviders;

        texNativeTypeProvider *definiteProvider = NULL;

        for ( interestList_t::const_iterator iter = interestedTypeProviders.begin(); iter != interestedTypeProviders.end(); iter++ )
        {
            const interestedNativeType& theInterest = *iter;

            eTexNativeCompatibility compatType = theInterest.typeOfInterest;

            if ( compatType == RWTEXCOMPAT_MAYBE )
            {
                providerInfo_t providerInfo;

                providerInfo.theProvider = theInterest.interestedParty;

                maybeProviders.push_back( providerInfo );
            }
            else if ( compatType == RWTEXCOMPAT_ABSOLUTE )
            {
                if ( definiteProvider == NULL )
                {
                    definiteProvider = theInterest.interestedParty;
                }
                else
                {
                    throw RwException( "texture native block compatibility conflict" );
                }
            }
        }

        // If we have no providers that recognized that texture block, we tell the runtime.
        if ( definiteProvider == NULL && maybeProviders.empty() )
        {
            throw RwException( "unknown texture native block" );
        }

        // If we have only one maybe provider, we set it as definite provider.
        if ( definiteProvider == NULL && maybeProviders.size() == 1 )
        {
            definiteProvider = maybeProviders.front().theProvider;

            maybeProviders.clear();
        }

        // If we have a definite provider, it is elected to parse the block.
        // Otherwise we try going through all "maybe" providers and select the first successful one.
        TextureBase *texOut = (TextureBase*)objectToDeserialize;

        // We will need a raster which is an interface to the native GPU data.
        // It serves as a container, so serialization will not access it directly.
        Raster *texRaster = CreateRaster( engineInterface );

        if ( texRaster )
        {
            // We require to allocate a platform texture, so lets keep a pointer.
            PlatformTexture *platformData = NULL;

            try
            {
                if ( definiteProvider != NULL )
                {
                    // If we have a definite provider, we do not need special warning dispatching.
                    // Good for us.

                    // Create a native texture for this provider.
                    platformData = CreateNativeTexture( engineInterface, definiteProvider->managerData.rwTexType );

                    if ( platformData )
                    {
                        // Attempt to deserialize the native texture.
                        inputProvider.seek( 0, RWSEEK_BEG );

                        definiteProvider->DeserializeTexture( texOut, platformData, inputProvider );
                    }
                }
                else
                {
                    // Loop through all maybe providers.
                    bool hasSuccessfullyDeserialized = false;

                    providerInfo_t *successfulProvider = NULL;

                    for ( providers_t::iterator iter = maybeProviders.begin(); iter != maybeProviders.end(); iter++ )
                    {
                        providerInfo_t& thisInfo = *iter;

                        texNativeTypeProvider *theProvider = thisInfo.theProvider;

                        // Just attempt deserialization.
                        bool success = false;

                        // For any warning that has been performed during this process, we need to queue it.
                        // In case we succeeded serializing an object, we just output the warning of its runtime.
                        // On failure we warn the runtime that the deserialization was ambiguous.
                        // Then we output warnings in sections, after the native type names.
                        WarningHandler *currentWarningHandler = &thisInfo._warningQueue;

                        GlobalPushWarningHandler( engineInterface, currentWarningHandler );

                        PlatformTexture *nativeData = NULL;

                        try
                        {
                            // We create a native texture for this provider.
                            nativeData = CreateNativeTexture( engineInterface, theProvider->managerData.rwTexType );

                            if ( nativeData )
                            {
                                try
                                {
                                    inputProvider.seek( 0, RWSEEK_BEG );

                                    try
                                    {
                                        theProvider->DeserializeTexture( texOut, nativeData, inputProvider );

                                        success = true;
                                    }
                                    catch( RwException& theError )
                                    {
                                        // We failed, try another deserialization.
                                        success = false;

                                        DeleteNativeTexture( engineInterface, nativeData );

                                        // We push this error as warning.
                                        if ( theError.message.size() != 0 )
                                        {
                                            engineInterface->PushWarning( theError.message );
                                        }
                                    }
                                }
                                catch( ... )
                                {
                                    // This catch is required if we encounter any other exception that wrecks the runtime
                                    // We do not want any memory leaks.
                                    DeleteNativeTexture( engineInterface, nativeData );

                                    nativeData = NULL;

                                    throw;
                                }
                            }
                            else
                            {
                                engineInterface->PushWarning( "failed to allocate native texture data for texture deserialization" );
                            }
                        }
                        catch( ... )
                        {
                            // We kinda failed somewhere, so lets unregister our warning handler.
                            GlobalPopWarningHandler( engineInterface );

                            throw;
                        }

                        GlobalPopWarningHandler( engineInterface );

                        if ( success )
                        {
                            successfulProvider = &thisInfo;
                            
                            hasSuccessfullyDeserialized = true;

                            // Give the native data to the runtime.
                            platformData = nativeData;
                            break;
                        }
                    }

                    if ( !hasSuccessfullyDeserialized )
                    {
                        // We need to inform the user of the warnings that he might have missed.
                        if ( maybeProviders.size() > 1 )
                        {
                            engineInterface->PushWarning( "ambiguous texture native block!" );
                        }

                        // Output all warnings in sections.
                        for ( providers_t::const_iterator iter = maybeProviders.begin(); iter != maybeProviders.end(); iter++ )
                        {
                            const providerInfo_t& thisInfo = *iter;

                            const QueuedWarningHandler& warningQueue = thisInfo._warningQueue;

                            texNativeTypeProvider *typeProvider = thisInfo.theProvider;

                            // Create a buffered warning output.
                            std::string typeWarnBuf;

                            typeWarnBuf += "[";
                            typeWarnBuf += typeProvider->managerData.rwTexType->name;
                            typeWarnBuf += "]:";

                            if ( warningQueue.message_list.empty() )
                            {
                                typeWarnBuf += "no warnings.\n";
                            }
                            else
                            {
                                typeWarnBuf += "\n";

                                for ( QueuedWarningHandler::messages_t::const_iterator iter = warningQueue.message_list.begin(); iter != warningQueue.message_list.end(); iter++ )
                                {
                                    typeWarnBuf += *iter;
                                    typeWarnBuf += "\n";
                                }
                            }

                            engineInterface->PushWarning( typeWarnBuf );
                        }

                        // On failure, delete the texture and bail.
                        engineInterface->DeleteRwObject( texOut );

                        texOut = NULL;
                    }
                    else
                    {
                        // Just output the warnings of the successful provider.
                        for ( QueuedWarningHandler::messages_t::const_iterator iter = successfulProvider->_warningQueue.message_list.begin(); iter != successfulProvider->_warningQueue.message_list.end(); iter++ )
                        {
                            engineInterface->PushWarning( *iter );
                        }
                    }
                }
            }
            catch( ... )
            {
                // If there was any exception, just pass it on.
                // We clear the texture beforehand.
                DeleteRaster( texRaster );

                texRaster = NULL;

                if ( platformData )
                {
                    DeleteNativeTexture( engineInterface, platformData );

                    platformData = NULL;
                }

                throw;
            }

            // Attempt to link all the data together.
            bool texLinkSuccess = false;

            if ( platformData )   
            {
                // We link the raster with the texture and put the platform data into the raster.
                texRaster->platformData = platformData;

                texOut->texRaster = texRaster;

                // We succeeded!
                texLinkSuccess = true;
            }

            if ( texLinkSuccess == false )
            {
                // Delete platform data if we had one.
                if ( platformData )
                {
                    DeleteNativeTexture( engineInterface, platformData );

                    platformData = NULL;
                }

                // Delete a texture if we had one.
                if ( texOut )
                {
                    engineInterface->DeleteRwObject( texOut );

                    texOut = NULL;
                }

                // Since we have no more texture object to store the raster into, we delete the raster.
                DeleteRaster( texRaster );

                throw RwException( "failed to link the raster object" );
            }
        }
        else
        {
            throw RwException( "failed to allocate raster object for deserialization" );
        }
    }

    struct nativeTextureCustomTypeInterface : public RwTypeSystem::typeInterface
    {
        void Construct( void *mem, Interface *engineInterface, void *construct_params ) const
        {
            this->texTypeProvider->ConstructTexture( this->engineInterface, mem, this->actualObjSize );
        }

        void CopyConstruct( void *mem, const void *srcMem ) const
        {
            this->texTypeProvider->CopyConstructTexture( this->engineInterface, mem, srcMem, this->actualObjSize );
        }

        void Destruct( void *mem ) const
        {
            this->texTypeProvider->DestroyTexture( this->engineInterface, mem, this->actualObjSize );
        }

        size_t GetTypeSize( void *construct_params ) const
        {
            return this->actualObjSize;
        }

        size_t GetTypeSizeByObject( const void *objMem ) const
        {
            return this->actualObjSize;
        }

        Interface *engineInterface;
        texNativeTypeProvider *texTypeProvider;
        size_t actualObjSize;
    };

    bool RegisterNativeTextureType( Interface *engineInterface, const char *nativeName, texNativeTypeProvider *typeProvider, size_t memSize )
    {
        bool registerSuccess = false;

        if ( typeProvider->managerData.isRegistered == false )
        {
            RwTypeSystem::typeInfoBase *platformTexType = this->platformTexType;

            if ( platformTexType != NULL )
            {
                // Register this type.
                nativeTextureCustomTypeInterface *newNativeTypeInterface = _newstruct <nativeTextureCustomTypeInterface> ( *engineInterface->typeSystem._memAlloc );

                if ( newNativeTypeInterface )
                {
                    // Set up our type.
                    newNativeTypeInterface->engineInterface = engineInterface;
                    newNativeTypeInterface->texTypeProvider = typeProvider;
                    newNativeTypeInterface->actualObjSize = memSize;

                    RwTypeSystem::typeInfoBase *newType = NULL;

                    try
                    {
                        newType = engineInterface->typeSystem.RegisterCommonTypeInterface( nativeName, newNativeTypeInterface, platformTexType );
                    }
                    catch( ... )
                    {
                        _delstruct <nativeTextureCustomTypeInterface> ( newNativeTypeInterface, *engineInterface->typeSystem._memAlloc );

                        registerSuccess = false;
                    }

                    // Alright, register us.
                    if ( newType )
                    {
                        typeProvider->managerData.rwTexType = newType;

                        LIST_APPEND( this->texNativeTypes.root, typeProvider->managerData.managerNode );

                        typeProvider->managerData.isRegistered = true;

                        registerSuccess = true;
                    }
                }
            }
            else
            {
                engineInterface->PushWarning( "tried to register native texture type with no platform type around" );
            }
        }

        return registerSuccess;
    }

    bool UnregisterNativeTextureType( Interface *engineInterface, const char *nativeName )
    {
        bool unregisterSuccess = false;

        // Try removing the type with said name.
        {
            RwTypeSystem::typeInfoBase *nativeTypeInfo = engineInterface->typeSystem.FindTypeInfo( nativeName, this->platformTexType );

            if ( nativeTypeInfo && nativeTypeInfo->IsImmutable() == false )
            {
                // We can cast the type interface to get the type provider.
                nativeTextureCustomTypeInterface *nativeTypeInterface = dynamic_cast <nativeTextureCustomTypeInterface*> ( nativeTypeInfo->tInterface );

                if ( nativeTypeInterface )
                {
                    texNativeTypeProvider *texProvider = nativeTypeInterface->texTypeProvider;

                    // Remove it.
                    LIST_REMOVE( texProvider->managerData.managerNode );

                    texProvider->managerData.isRegistered = false;

                    // Delete the type.
                    engineInterface->typeSystem.DeleteType( nativeTypeInfo );
                }
            }
        }

        return unregisterSuccess;
    }
    
    RwTypeSystem::typeInfoBase *platformTexType;

    RwList <texNativeTypeProvider> texNativeTypes;
};

static PluginDependantStructRegister <nativeTextureStreamPlugin, RwInterfaceFactory_t> nativeTextureStreamStore( engineFactory );

// Texture native type registrations.
void registerD3DNativeTexture( Interface *engineInterface );
void registerXBOXNativeTexture( Interface *engineInterface );
void registerPS2NativeTexture( Interface *engineInterface );
void registerMobileDXTNativeTexture( Interface *engineInterface );
void registerATCNativeTexture( Interface *engineInterface );
void registerPVRNativeTexture( Interface *engineInterface );
void registerMobileUNCNativeTexture( Interface *engineInterface );

void initializeNativeTextureEnvironment( Interface *engineInterface )
{
    nativeTextureStreamPlugin *nativeTexEnv = nativeTextureStreamStore.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( nativeTexEnv )
    {
        // Register the native texture stream plugin.
        engineInterface->RegisterSerialization( CHUNK_TEXTURENATIVE, engineInterface->textureTypeInfo, nativeTexEnv, RWSERIALIZE_INHERIT );
    }

    // Register sub environments.
    registerD3DNativeTexture( engineInterface );
    registerXBOXNativeTexture( engineInterface );
    registerPS2NativeTexture( engineInterface );
    registerMobileDXTNativeTexture( engineInterface );
    registerATCNativeTexture( engineInterface );
    registerPVRNativeTexture( engineInterface );
    registerMobileUNCNativeTexture( engineInterface );
}

void shutdownNativeTextureEnvironment( Interface *engineInterface )
{
    nativeTextureStreamPlugin *nativeTexEnv = nativeTextureStreamStore.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( nativeTexEnv )
    {
        // Unregister us again.
        engineInterface->UnregisterSerialization( CHUNK_TEXTURENATIVE, engineInterface->textureTypeInfo, nativeTexEnv );
    }
}

bool RegisterNativeTextureType( Interface *engineInterface, const char *nativeName, texNativeTypeProvider *typeProvider, size_t memSize )
{
    bool success = false;

    nativeTextureStreamPlugin *nativeTexEnv = nativeTextureStreamStore.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( nativeTexEnv )
    {
        success = nativeTexEnv->RegisterNativeTextureType( engineInterface, nativeName, typeProvider, memSize );
    }

    return success;
}

bool UnregisterNativeTextureType( Interface *engineInterface, const char *nativeName )
{
    bool success = false;

    nativeTextureStreamPlugin *nativeTexEnv = nativeTextureStreamStore.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( nativeTexEnv )
    {
        success = nativeTexEnv->UnregisterNativeTextureType( engineInterface, nativeName );
    }

    return success;
}

texNativeTypeProvider* GetNativeTextureTypeProvider( Interface *engineInterface, void *objMem )
{
    texNativeTypeProvider *platformData = NULL;

    // We first need the native texture type environment.
    nativeTextureStreamPlugin *nativeTexEnv = nativeTextureStreamStore.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( nativeTexEnv )
    {
        // Attempt to get the type info of the native data.
        GenericRTTI *rtObj = engineInterface->typeSystem.GetTypeStructFromAbstractObject( objMem );

        if ( rtObj )
        {
            RwTypeSystem::typeInfoBase *typeInfo = RwTypeSystem::GetTypeInfoFromTypeStruct( rtObj );

            // Check that we indeed are a native texture type.
            // This is guarranteed if we inherit from the native texture type info.
            if ( engineInterface->typeSystem.IsTypeInheritingFrom( nativeTexEnv->platformTexType, typeInfo ) )
            {
                // We assume that the type provider is of our native type.
                // For safety, do a dynamic cast.
                nativeTextureStreamPlugin::nativeTextureCustomTypeInterface *nativeTypeInterface =
                    dynamic_cast <nativeTextureStreamPlugin::nativeTextureCustomTypeInterface*> ( typeInfo->tInterface );

                if ( nativeTypeInterface )
                {
                    // Return the type provider.
                    platformData = nativeTypeInterface->texTypeProvider;
                }
            }
        }
    }

    return platformData;
}

void pixelDataTraversal::FreePixels( Interface *engineInterface )
{
    if ( this->isNewlyAllocated )
    {
        uint32 mipmapCount = this->mipmaps.size();

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            mipmapResource& thisLayer = this->mipmaps[ n ];

            if ( void *texels = thisLayer.texels )
            {
                engineInterface->PixelFree( texels );

                thisLayer.texels = NULL;
            }
        }

        this->mipmaps.clear();

        if ( void *paletteData = this->paletteData )
        {
            engineInterface->PixelFree( paletteData );

            this->paletteData = NULL;
        }

        this->isNewlyAllocated = false;
    }
}

void pixelDataTraversal::CloneFrom( Interface *engineInterface, const pixelDataTraversal& right )
{
    // Free any previous data.
    this->FreePixels( engineInterface );

    // Clone parameters.
    eRasterFormat rasterFormat = right.rasterFormat;

    this->isNewlyAllocated = true;  // since we clone
    this->rasterFormat = rasterFormat;
    this->depth = right.depth;
    this->colorOrder = right.colorOrder;

    // Clone palette.
    this->paletteType = right.paletteType;
    
    void *srcPaletteData = right.paletteData;
    void *dstPaletteData = NULL;

    uint32 dstPaletteSize = 0;

    if ( srcPaletteData )
    {
        uint32 srcPaletteSize = right.paletteSize;  
        
        // Copy the palette texels.
        uint32 palRasterDepth = Bitmap::getRasterFormatDepth( rasterFormat );

        uint32 palDataSize = getRasterDataSize( srcPaletteSize, palRasterDepth );

        dstPaletteData = engineInterface->PixelAllocate( palDataSize );

        memcpy( dstPaletteData, srcPaletteData, palDataSize );

        dstPaletteSize = srcPaletteSize;
    }

    this->paletteData = dstPaletteData;
    this->paletteSize = dstPaletteSize;

    // Clone mipmaps.
    uint32 mipmapCount = right.mipmaps.size();

    this->mipmaps.resize( mipmapCount );

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        const mipmapResource& srcLayer = right.mipmaps[ n ];

        mipmapResource newLayer;

        newLayer.width = srcLayer.width;
        newLayer.height = srcLayer.height;

        newLayer.mipWidth = srcLayer.mipWidth;
        newLayer.mipHeight = srcLayer.mipHeight;

        // Copy the mipmap layer texels.
        uint32 mipDataSize = srcLayer.dataSize;

        const void *srcTexels = srcLayer.texels;

        void *newtexels = engineInterface->PixelAllocate( mipDataSize );

        memcpy( newtexels, srcTexels, mipDataSize );

        newLayer.texels = newtexels;
        newLayer.dataSize = mipDataSize;

        // Store this layer.
        this->mipmaps[ n ] = newLayer;
    }

    // Clone non-trivial parameters.
    this->compressionType = right.compressionType;
    this->hasAlpha = right.hasAlpha;
    this->autoMipmaps = right.autoMipmaps;
    this->cubeTexture = right.cubeTexture;
    this->rasterType = right.rasterType;
}

void pixelDataTraversal::mipmapResource::Free( Interface *engineInterface )
{
    // Free the data here, since we have the Interface struct defined.
    if ( void *ourTexels = this->texels )
    {
        engineInterface->PixelFree( ourTexels );

        // We have no more texels.
        this->texels = NULL;
    }
}

inline void CompatibilityTransformPixelData( Interface *engineInterface, pixelDataTraversal& pixelData, const pixelCapabilities& pixelCaps )
{
    // Make sure the pixelData does not violate the capabilities struct.
    // This is done by "downcasting". It preserves maximum image quality, but increases memory requirements.

    // First get the original parameters onto stack.
    eRasterFormat srcRasterFormat = pixelData.rasterFormat;
    uint32 srcDepth = pixelData.depth;
    eColorOrdering srcColorOrder = pixelData.colorOrder;
    ePaletteType srcPaletteType = pixelData.paletteType;
    void *srcPaletteData = pixelData.paletteData;
    uint32 srcPaletteSize = pixelData.paletteSize;
    eCompressionType srcCompressionType = pixelData.compressionType;

    uint32 srcMipmapCount = pixelData.mipmaps.size();

    // Now decide the target format depending on the capabilities.
    eRasterFormat dstRasterFormat = srcRasterFormat;
    uint32 dstDepth = srcDepth;
    eColorOrdering dstColorOrder = srcColorOrder;
    ePaletteType dstPaletteType = srcPaletteType;
    void *dstPaletteData = srcPaletteData;
    uint32 dstPaletteSize = srcPaletteSize;
    eCompressionType dstCompressionType = srcCompressionType;

    bool hasBeenModded = false;

    if ( dstCompressionType == RWCOMPRESS_DXT1 && pixelCaps.supportsDXT1 == false ||
         dstCompressionType == RWCOMPRESS_DXT2 && pixelCaps.supportsDXT2 == false ||
         dstCompressionType == RWCOMPRESS_DXT3 && pixelCaps.supportsDXT3 == false ||
         dstCompressionType == RWCOMPRESS_DXT4 && pixelCaps.supportsDXT4 == false ||
         dstCompressionType == RWCOMPRESS_DXT5 && pixelCaps.supportsDXT5 == false )
    {
        // Set proper decompression parameters.
        uint32 dxtType;

        bool isDXT = IsDXTCompressionType( dstCompressionType, dxtType );

        assert( isDXT == true );

        eRasterFormat targetRasterFormat = getDXTDecompressionRasterFormat( engineInterface, dxtType, pixelData.hasAlpha );

        dstRasterFormat = targetRasterFormat;
        dstDepth = Bitmap::getRasterFormatDepth( targetRasterFormat );
        dstColorOrder = COLOR_BGRA;
        dstPaletteType = PALETTE_NONE;
        dstPaletteData = NULL;
        dstPaletteSize = 0;

        // We decompress stuff.
        dstCompressionType = RWCOMPRESS_NONE;

        hasBeenModded = true;
    }

    if ( hasBeenModded == false )
    {
        if ( dstPaletteType != PALETTE_NONE && pixelCaps.supportsPalette == false )
        {
            // We want to do things without a palette.
            dstPaletteType = PALETTE_NONE;
            dstPaletteSize = 0;
            dstPaletteData = NULL;

            dstDepth = Bitmap::getRasterFormatDepth(dstRasterFormat);

            hasBeenModded = true;
        }
    }

    // Check whether we even want an update.
    bool wantsUpdate = false;

    if ( srcRasterFormat != dstRasterFormat || dstDepth != srcDepth || dstColorOrder != srcColorOrder ||
         dstPaletteType != srcPaletteType || dstPaletteData != srcPaletteData || dstPaletteSize != srcPaletteSize ||
         dstCompressionType != srcCompressionType )
    {
        wantsUpdate = true;
    }

    if ( wantsUpdate )
    {
        // Convert the pixels now.
        bool hasUpdated;
        {
            // Create a destination format struct.
            pixelFormat dstPixelFormat;

            dstPixelFormat.rasterFormat = dstRasterFormat;
            dstPixelFormat.depth = dstDepth;
            dstPixelFormat.colorOrder = dstColorOrder;
            dstPixelFormat.paletteType = dstPaletteType;
            dstPixelFormat.compressionType = dstCompressionType;

            hasUpdated = ConvertPixelData( engineInterface, pixelData, dstPixelFormat );
        }

        // If we want an update, we should get an update.
        // Otherwise, ConvertPixelData is to blame.
        assert( hasUpdated == true );

        // If we have updated at all, apply changes.
        if ( hasUpdated )
        {
            // We must have the correct parameters.
            // Here we verify problematic parameters only.
            // Params like rasterFormat are expected to be handled properly no matter what.
            assert( pixelData.compressionType == dstCompressionType );
        }
    }
}

bool ConvertRasterTo( Raster *theRaster, const char *nativeName )
{
    bool conversionSuccess = false;

    Interface *engineInterface = theRaster->engineInterface;

    // First get the native texture environment.
    // This is used to browse for convertible types.
    nativeTextureStreamPlugin *nativeTexEnv = nativeTextureStreamStore.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( nativeTexEnv )
    {
        // Only convert if the raster has image data.
        if ( PlatformTexture *nativeTex = theRaster->platformData )
        {
            // Get the type information of the original platform data.
            GenericRTTI *origNativeRtObj = RwTypeSystem::GetTypeStructFromObject( nativeTex );

            RwTypeSystem::typeInfoBase *origTypeInfo = RwTypeSystem::GetTypeInfoFromTypeStruct( origNativeRtObj );

            // Get the type information of the destination format.
            RwTypeSystem::typeInfoBase *dstTypeInfo = engineInterface->typeSystem.FindTypeInfo( nativeName, nativeTexEnv->platformTexType );

            if ( origTypeInfo != NULL && dstTypeInfo != NULL )
            {
                // If the destination type and the source type match, we are finished.
                if ( engineInterface->typeSystem.IsSameType( origTypeInfo, dstTypeInfo ) )
                {
                    conversionSuccess = true;
                }
                else
                {
                    // Attempt to get the native texture type interface for both types.
                    nativeTextureStreamPlugin::nativeTextureCustomTypeInterface *origTypeInterface = dynamic_cast <nativeTextureStreamPlugin::nativeTextureCustomTypeInterface*> ( origTypeInfo->tInterface );
                    nativeTextureStreamPlugin::nativeTextureCustomTypeInterface *dstTypeInterface = dynamic_cast <nativeTextureStreamPlugin::nativeTextureCustomTypeInterface*> ( dstTypeInfo->tInterface );

                    // Only proceed if both could resolve.
                    if ( origTypeInterface != NULL && dstTypeInterface != NULL )
                    {
                        // Use the original type provider to grab pixel data from the texture.
                        // Then get the pixel capabilities of both formats and convert the pixel data into a compatible format for the destination format.
                        // Finally, apply the pixels to the destination format texture.
                        // * PERFORMANCE: at best, it can fetch pixel data (without allocation), free the original texture, allocate the new texture and put the pixels to it.
                        // * this would be a simple move operation. the actual operation depends on the complexity of both formats.
                        texNativeTypeProvider *origTypeProvider = origTypeInterface->texTypeProvider;
                        texNativeTypeProvider *dstTypeProvider = dstTypeInterface->texTypeProvider;

                        // In case of an exception, we have to deal with the pixel information, so we do not leak memory.
                        pixelDataTraversal pixelStore;

                        // 1. Fetch the pixel data.
                        origTypeProvider->GetPixelDataFromTexture( engineInterface, nativeTex, pixelStore );

                        try
                        {
                            // 2. detach the pixel data from the texture and free it.
                            //    free the pixels if we got a private copy.
                            origTypeProvider->UnsetPixelDataFromTexture( engineInterface, nativeTex, ( pixelStore.isNewlyAllocated == true ) );

                            // Since we are the only owners of pixelData now, inform it.
                            pixelStore.SetStandalone();

                            // 3. Allocate a new texture.
                            PlatformTexture *newNativeTex = CreateNativeTexture( engineInterface, dstTypeInfo );

                            if ( newNativeTex )
                            {
                                // 4. Get the pixel capabilities of the target resource and put the texels we fetched in
                                //    the highest quality format possible.
                                pixelCapabilities dstSurfaceCaps;

                                dstTypeProvider->GetPixelCapabilities( dstSurfaceCaps );

                                // TODO: make pixels compatible for the target format.
                                // * First decide what pixel format we have to deduce from the capabilities
                                //   and then call the "ConvertPixelData" function to do the job.
                                CompatibilityTransformPixelData( engineInterface, pixelStore, dstSurfaceCaps );

                                // 5. Put the texels into our texture.
                                //    Throwing an exception here means that the texture did not apply any of the pixel
                                //    information. We can safely free pixelStore.
                                texNativeTypeProvider::acquireFeedback_t acquireFeedback;

                                dstTypeProvider->SetPixelDataToTexture( engineInterface, newNativeTex, pixelStore, acquireFeedback );

                                if ( acquireFeedback.hasDirectlyAcquired == false )
                                {
                                    // We need to release the pixels from the storage.
                                    pixelStore.FreePixels( engineInterface );
                                }
                                else
                                {
                                    // Since the texture now owns the pixels, we just detach.
                                    pixelStore.DetachPixels();
                                }

                                // 6. Link the new native texture!
                                //    Also delete the old one.
                                DeleteNativeTexture( engineInterface, nativeTex );

                                theRaster->platformData = newNativeTex;

                                // We are successful!
                                conversionSuccess = true;
                            }
                            else
                            {
                                conversionSuccess = false;
                            }
                        }
                        catch( ... )
                        {
                            conversionSuccess = false;
                        }

                        if ( conversionSuccess == false )
                        {
                            // We failed at doing our task.
                            // Terminate any resource that allocated.
                            pixelStore.FreePixels( engineInterface );
                        }
                    }
                }
            }
        }
    }

    return conversionSuccess;
}

/*
 * Raster
 */

Raster* CreateRaster( Interface *engineInterface )
{
    RwTypeSystem::typeInfoBase *rasterTypeInfo = engineInterface->rasterTypeInfo;

    if ( rasterTypeInfo )
    {
        GenericRTTI *rtObj = engineInterface->typeSystem.Construct( engineInterface, rasterTypeInfo, NULL );

        if ( rtObj )
        {
            Raster *theRaster = (Raster*)RwTypeSystem::GetObjectFromTypeStruct( rtObj );

            return theRaster;
        }
    }
    else
    {
        engineInterface->PushWarning( "no raster type info present in CreateRaster" );
    }

    return NULL;
}

Raster* AcquireRaster( Raster *theRaster )
{
    // Attempts to get a handle to this raster by referencing it.
    // This function could fail if the resource has reached its maximum refcount.

    Raster *returnObj = NULL;

    if ( theRaster )
    {
        // TODO: implement ref count overflow security check.

        theRaster->refCount++;

        returnObj = theRaster;
    }

    return returnObj;
}

void DeleteRaster( Raster *theRaster )
{
    Interface *engineInterface = theRaster->engineInterface;

    // We use reference counting on rasters.
    theRaster->refCount--;

    if ( theRaster->refCount == 0 )
    {
        // Just delete it.
        GenericRTTI *rtObj = engineInterface->typeSystem.GetTypeStructFromAbstractObject( theRaster );

        if ( rtObj )
        {
            engineInterface->typeSystem.Destroy( rtObj );
        }
        else
        {
            engineInterface->PushWarning( "invalid raster object pushed to DeleteRaster" );
        }
    }
}

Raster::Raster( const Raster& right )
{
    // Copy raster specifics.
    this->engineInterface = right.engineInterface;

    // Copy native platform data.
    PlatformTexture *platformTex = NULL;

    if ( right.platformData )
    {
        platformTex = CloneNativeTexture( this->engineInterface, right.platformData );
    }

    this->platformData = platformTex;

    this->refCount = 1;
}

Raster::~Raster( void )
{
    // Delete the platform data, if available.
    if ( PlatformTexture *platformData = this->platformData )
    {
        DeleteNativeTexture( this->engineInterface, platformData );

        this->platformData = NULL;
    }

    assert( this->refCount == 0 );
}

void Raster::SetEngineVersion( LibraryVersion version )
{
    // TODO.
}

LibraryVersion Raster::GetEngineVersion( void ) const
{
    // TODO.
    return LibraryVersion();
}

void Raster::convertToFormat(eRasterFormat newFormat)
{
    // TODO: use ConvertPixelData function here.

    //if (this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9)
    //    return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    // If the texture is DXT compressed, uncompress it.
#if 0
    if ( platformTex->dxtCompression )
    {
        platformTex->decompressDxt();
    }
#endif

    eRasterFormat rasterFormat = platformTex->rasterFormat;
    ePaletteType paletteType = platformTex->paletteType;
    eColorOrdering colorOrder = platformTex->colorOrdering;
    uint32 mipmapCount = platformTex->mipmaps.size();
    uint32 srcDepth = platformTex->depth;

    bool isPaletteRaster = ( paletteType != PALETTE_NONE );

    if ( isPaletteRaster || newFormat != rasterFormat )
    {
        void *paletteData = platformTex->palette;
        uint32 maxpalette = platformTex->paletteSize;

        uint32 newDepth = Bitmap::getRasterFormatDepth(newFormat);

	    for (uint32 j = 0; j < mipmapCount; j++)
        {
            // We will put new texels into the mipLayer.
            NativeTextureD3D::mipmapLayer& mipLayer = platformTex->mipmaps[ j ];
            
            uint32 dataSize = 0;

            // Decide about output data.
            uint32 layerWidth = mipLayer.layerWidth;
            uint32 layerHeight = mipLayer.layerHeight;

            uint32 colorIndiceCount = ( layerWidth * layerHeight );

            dataSize = getRasterDataSize(colorIndiceCount, newDepth);

            {
	            void *newtexels = new uint8[ dataSize ];

                const void *texelSource = mipLayer.texels;

                for (uint32 y = 0; y < layerHeight; y++)
                {
                    for (uint32 x = 0; x < layerWidth; x++)
                    {
                        // Calculate the combined color texture index.
                        uint32 colorIndex = PixelFormat::coord2index(x, y, layerWidth);

                        // Grab the color values.
                        uint8 red, green, blue, alpha;
                        bool hasColor = browsetexelcolor(texelSource, paletteType, paletteData, maxpalette, colorIndex, rasterFormat, colorOrder, srcDepth, red, green, blue, alpha);

                        if ( !hasColor )
                        {
                            red = 0;
                            green = 0;
                            blue = 0;
                            alpha = 0;
                        }

                        // Write the color data.
                        puttexelcolor(newtexels, colorIndex, newFormat, colorOrder, newDepth, red, green, blue, alpha);
                    }
                }

                // Write the texture data.
	            delete[] texelSource;

	            mipLayer.texels = newtexels;
	            mipLayer.dataSize = dataSize;

                // Also update the dimensions.
                mipLayer.width = layerWidth;
                mipLayer.height = layerHeight;
            }
	    }

        // Update the depth.
        platformTex->depth = newDepth;

        // Update the format.
        platformTex->rasterFormat = newFormat;

        // Delete unnecessary palette data.
	    if (isPaletteRaster)
        {
		    delete[] paletteData;
		    platformTex->palette = NULL;

            platformTex->paletteSize = 0;
		    
            platformTex->paletteType = PALETTE_NONE;
	    }

        // Make sure we update the d3dFormat.
        platformTex->updateD3DFormat();

        // Since we most likely changed colors, recalculate the alpha flag.
        //platformTex->hasAlpha = platformTex->doesHaveAlpha();
    }
}

Bitmap Raster::getBitmap(void) const
{
    Bitmap resultBitmap;

    // If the texture is Direct3D, we can easily get the texels.
    //if ( this->platform == PLATFORM_D3D8 || this->platform == PLATFORM_D3D9 )
    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // If it has any mipmap at all.
        if ( platformTex->mipmaps.empty() == false )
        {
            uint32 width;
            uint32 height;
            uint32 depth;
            eRasterFormat theFormat;
            eColorOrdering theOrder;
            
            // Get the color data.
            bool hasAllocatedNewPixelData = false;
            void *pixelData = NULL;

            uint32 texDataSize;

            {
                rawBitmapFetchResult rawBitmap;

                bool gotPixelData = platformTex->getRawBitmap(0, false, rawBitmap);

                if ( gotPixelData )
                {
                    width = rawBitmap.width;
                    height = rawBitmap.height;
                    texDataSize = rawBitmap.dataSize;
                    depth = rawBitmap.depth;
                    theFormat = rawBitmap.rasterFormat;
                    theOrder = rawBitmap.colorOrder;
                    hasAllocatedNewPixelData = rawBitmap.isNewlyAllocated;
                    pixelData = rawBitmap.texelData;
                }
            }

            if ( pixelData != NULL )
            {
                // Set the data into the bitmap.
                resultBitmap.setImageData(
                    pixelData, theFormat, theOrder, depth, width, height, texDataSize
                );

                if ( hasAllocatedNewPixelData )
                {
                    delete [] pixelData;
                }
            }
        }
    }

    return resultBitmap;
}

void Raster::setImageData(const Bitmap& srcImage)
{
    // If the texture is Direct3D, we can easily set the texels.
    //if ( this->platform == PLATFORM_D3D8 || this->platform == PLATFORM_D3D9 )
    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // Delete old image data.
        {
            uint32 mipmapCount = platformTex->mipmaps.size();

            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                void *texels = platformTex->mipmaps[ n ].texels;

                delete [] texels;
            }

            // If we have palette data, delete it.
            if ( void *paletteTexels = platformTex->palette )
            {
                delete [] paletteTexels;

                platformTex->palette = NULL;
            }
            platformTex->paletteType = PALETTE_NONE;
            platformTex->paletteSize = 0;
        }

        // Resize mipmap containers.
        platformTex->mipmaps.resize( 1 );

        NativeTextureD3D::mipmapLayer& mipLayer = platformTex->mipmaps[ 0 ];

        // Set the new texel data.
        uint32 newWidth, newHeight;
        uint32 newDepth = srcImage.getDepth();
        eRasterFormat newFormat = srcImage.getFormat();
        eColorOrdering newColorOrdering = srcImage.getColorOrder();
        uint32 newDataSize = srcImage.getDataSize();

        srcImage.getSize( newWidth, newHeight );

        mipLayer.width = newWidth;
        mipLayer.height = newHeight;
        
        // Since it is raw image data, layer dimm is same as regular dimm.
        mipLayer.layerWidth = newWidth;
        mipLayer.layerHeight = newHeight;

        platformTex->depth = newDepth;

        mipLayer.texels = srcImage.copyPixelData();
        mipLayer.dataSize = newDataSize;

        platformTex->colorOrdering = newColorOrdering;

        platformTex->rasterFormat = newFormat;

        // Update generics.
        platformTex->updateD3DFormat();

        // We need to update the alpha flag.
        //platformTex->hasAlpha = platformTex->doesHaveAlpha();
    }
}

void Raster::resize(uint32 width, uint32 height)
{
    Bitmap mainBitmap = this->getBitmap();

    Bitmap targetBitmap( mainBitmap.getDepth(), mainBitmap.getFormat(), mainBitmap.getColorOrder() );

    targetBitmap.setSize(width, height);

    targetBitmap.drawBitmap(
        mainBitmap, 0, 0, width, height,
        Bitmap::SHADE_ZERO, Bitmap::SHADE_ONE, Bitmap::BLEND_ADDITIVE
    );

    this->setImageData( targetBitmap );
}

void Raster::getSize(uint32& width, uint32& height) const
{
    PlatformTexture *platformTex = this->platformData;

    width = platformTex->getWidth();
    height = platformTex->getHeight();
}

void TextureBase::improveFiltering(void)
{
    // Handle stuff depending on our current settings.
    eRasterStageFilterMode currentFilterMode = this->filterMode;

    eRasterStageFilterMode newFilterMode = currentFilterMode;

    if ( currentFilterMode == RWFILTER_POINT )
    {
        newFilterMode = RWFILTER_LINEAR;
    }
    else if ( currentFilterMode == RWFILTER_POINT_POINT )
    {
        newFilterMode = RWFILTER_POINT_LINEAR;
    }

    // Update our texture fields.
    if ( currentFilterMode != newFilterMode )
    {
        this->filterMode = newFilterMode;
    }
}

void Raster::newDirect3D(void)
{
    //if ( this->platform != 0 )
    //    return;

#if 0

    assert( this->platformData == NULL );

    // Create new platform data.
    NativeTextureD3D *d3dtex = new NativeTextureD3D;

    // Create the backlink.
    d3dtex->parent = this;

    d3dtex->platformType = NativeTextureD3D::PLATFORM_D3D9;

    this->platformData = d3dtex;
#endif
}

void Raster::optimizeForLowEnd(float quality)
{
    //if (this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9)
    //    return;

    NativeTextureD3D *d3dtex = (NativeTextureD3D*)this->platformData;

    // Textures that should run on low end hardware should not be too HD.
    // This routine takes the PlayStation 2 as reference hardware.

    const uint32 maxTexWidth = 256;
    const uint32 maxTexHeight = 256;

    while ( true )
    {
        // Get properties of this texture first.
        uint32 texWidth, texHeight;

        texWidth = d3dtex->getWidth();
        texHeight = d3dtex->getHeight();

        // Optimize this texture.
        bool hasTakenStep = false;

        if ( !hasTakenStep && quality < 1.0f )
        {
            // We first must make sure that the texture is not unnecessaringly huge.
            if ( texWidth > maxTexWidth || texHeight > maxTexHeight )
            {
                // Half the texture size until we are at a suitable size.
                uint32 targetWidth = texWidth;
                uint32 targetHeight = texHeight;

                do
                {
                    targetWidth /= 2;
                    targetHeight /= 2;
                }
                while ( targetWidth > maxTexWidth || targetHeight > maxTexHeight );

                // The texture dimensions are too wide.
                // We half the texture in size.
                this->resize( targetWidth, targetHeight );

                hasTakenStep = true;
            }
        }
        
        if ( !hasTakenStep )
        {
            // We should decrease the texture size by palettization.
            if (d3dtex->paletteType == PALETTE_NONE)
            {
                ePaletteType targetPalette = ( quality > 0.0f ) ? ( PALETTE_8BIT ) : ( PALETTE_4BIT );

                if (d3dtex->hasAlpha == false)
                {
                    // 4bit palette is only feasible for non-alpha textures (at higher quality settings).
                    // Otherwise counting the colors is too complicated.
                    
                    // TODO: still allow 8bit textures.
                    targetPalette = PALETTE_4BIT;
                }

                // The texture should be palettized for good measure.
                //this->convertToPalette( targetPalette );

                // TODO: decide whether 4bit or 8bit palette.

                hasTakenStep = true;
            }
        }

        if ( !hasTakenStep )
        {
            // The texture dimension is important for renderer performance. That is why we need to scale the texture according
            // to quality settings aswell.

        }
    
        if ( !hasTakenStep )
        {
            break;
        }
    }
}

bool Raster::isDirect3DWritable(void) const
{
    bool isWritable = false;

    //uint32 platform = this->platform;

    //if (platform == PLATFORM_D3D8 || platform == PLATFORM_D3D9)
    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        if ( platformTex->platformType == NativeTextureD3D::PLATFORM_D3D9 )
        {
            // We can write the texture if it has a Direct3D format.
            if (platformTex->hasD3DFormat == true)
            {
                isWritable = true;
            }
        }
        else if ( platformTex->platformType == NativeTextureD3D::PLATFORM_D3D8 )
        {
            bool isCompressed = ( platformTex->dxtCompression != 0 );

            if ( isCompressed )
            {
                // If we are compressed, then we definately are writable.
                isWritable = true;
            }
            else
            {
                // I can be a bit lazy here.
                // Returning false here means that "makeDirect3DCompatible" has to be called.
                // That routine will sort things out.
                isWritable = false;
            }
        }
    }

    return isWritable;
}

void Raster::makeDirect3DCompatible(void)
{
    // Only works if the texture is in Direct3D platform already.
    // This routine is pretty complicated.

    //uint32 platform = this->platform;

    //if (platform == PLATFORM_D3D8 || platform == PLATFORM_D3D9)
    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        bool requiresCheck = false;

        // On the Direct3D 8 platform, we should check for compatibility, aswell.
        // But it requires different checks than the Direct3D 9 platform.
        if ( platformTex->platformType == NativeTextureD3D::PLATFORM_D3D8 )
        {
            bool isCompressed = ( platformTex->dxtCompression != 0 );

            // Being DXT compressed means that we are writable.
            if ( !isCompressed )
            {
                requiresCheck = true;
            }
        }
        else if ( platformTex->platformType == NativeTextureD3D::PLATFORM_D3D9 )
        {
            // Otherwise, we only update if we have no Direct3D format.
            requiresCheck = ( platformTex->hasD3DFormat == false );
        }

        bool hasUpdated = false;

        // Make sure this texture is writable.
        if ( requiresCheck )
        {
            // Well, if we are compressed we usually have a D3DFORMAT.
            // But things can go really wrong, so lets decompress anyway.
#if 0
            if ( platformTex->dxtCompression )
            {
                platformTex->decompressDxt();
            }
#endif

            // Get the color parameters of the source texels.
            uint32 srcDepth = platformTex->depth;

            eRasterFormat srcRasterFormat = platformTex->rasterFormat;
            ePaletteType srcPaletteType = platformTex->paletteType;

            void *srcPaletteData = platformTex->palette;
            uint32 srcPaletteSize = platformTex->paletteSize;

            bool srcHasAlpha = platformTex->hasAlpha;

            eColorOrdering srcColorOrder = platformTex->colorOrdering;

            // If we are on D3D, we have to avoid typical configurations that may come from
            // other hardware.
            uint32 dstDepth = srcDepth;

            eRasterFormat dstRasterFormat = srcRasterFormat;
            ePaletteType dstPaletteType = srcPaletteType;

            eColorOrdering dstColorOrder = srcColorOrder;

            if ( dstPaletteType != PALETTE_NONE )
            {
                if ( dstPaletteType == PALETTE_4BIT || dstPaletteType == PALETTE_8BIT )
                {
                    dstDepth = 8;
                }
                else
                {
                    assert( 0 );
                }

                dstColorOrder = COLOR_RGBA;
            }
            else
            {
                if ( dstRasterFormat == RASTER_1555 )
                {
                    dstDepth = 16;
                }
                else if ( dstRasterFormat == RASTER_565 )
                {
                    dstDepth = 16;
                }
                else if ( dstRasterFormat == RASTER_4444 )
                {
                    dstDepth = 16;
                }
                else if ( dstRasterFormat == RASTER_8888 )
                {
                    dstDepth = 32;
                }
                else if ( dstRasterFormat == RASTER_888 )
                {
                    dstDepth = 32;
                }
                else if ( dstRasterFormat == RASTER_555 )
                {
                    dstDepth = 16;
                }

                dstColorOrder = COLOR_BGRA;
            }

            // Call the generic pixel data conversion algorithm.
            {
                // Fill the source texel struct.
                pixelDataTraversal pixelData;

                pixelData.rasterFormat = srcRasterFormat;
                pixelData.depth = srcDepth;
                pixelData.colorOrder = srcColorOrder;
                pixelData.paletteType = srcPaletteType;
                pixelData.paletteData = srcPaletteData;
                pixelData.paletteSize = srcPaletteSize;
                pixelData.compressionType = RWCOMPRESS_NONE;
                pixelData.hasAlpha = srcHasAlpha;

                uint32 srcMipmapCount = platformTex->mipmaps.size();

                pixelData.mipmaps.resize( srcMipmapCount );

                for ( uint32 n = 0; n < srcMipmapCount; n++ )
                {
                    const NativeTextureD3D::mipmapLayer& curLayer = platformTex->mipmaps[ n ];

                    pixelDataTraversal::mipmapResource newLayer;

                    newLayer.width = curLayer.width;
                    newLayer.height = curLayer.height;

                    newLayer.mipWidth = curLayer.layerWidth;
                    newLayer.mipHeight = curLayer.layerHeight;

                    newLayer.texels = curLayer.texels;
                    newLayer.dataSize = curLayer.dataSize;

                    // Store this virtual layer.
                    pixelData.mipmaps[ n ] = newLayer;
                }

                // Fill the pixel format that we want to convert to.
                pixelFormat dstPixelFormat;

                dstPixelFormat.rasterFormat = dstRasterFormat;
                dstPixelFormat.depth = dstDepth;
                dstPixelFormat.colorOrder = dstColorOrder;
                dstPixelFormat.paletteType = dstPaletteType;
                dstPixelFormat.compressionType = RWCOMPRESS_NONE;

                // Call the converter.
                bool hasUpdated = ConvertPixelData( engineInterface, pixelData, dstPixelFormat );

                // If it has updated anything, we make sure we are up to date.
                if ( hasUpdated )
                {
                    eRasterFormat newRasterFormat = pixelData.rasterFormat;
                    uint32 newDepth = pixelData.depth;
                    eColorOrdering newColorOrder = pixelData.colorOrder;
                    ePaletteType newPaletteType = pixelData.paletteType;
                    void *newPaletteData = pixelData.paletteData;
                    uint32 newPaletteSize = pixelData.paletteSize;
                    bool newHasAlpha = pixelData.hasAlpha;
                    
                    // Make sure we are still not compressed.
                    assert( pixelData.compressionType == RWCOMPRESS_NONE );

                    // Check for changes (general properties).
                    if ( newRasterFormat != srcRasterFormat )
                    {
                        platformTex->rasterFormat = newRasterFormat;
                    }

                    if ( newDepth != srcDepth )
                    {
                        platformTex->depth = newDepth;
                    }

                    if ( newColorOrder != srcColorOrder )
                    {
                        platformTex->colorOrdering = newColorOrder;
                    }

                    if ( newPaletteType != srcPaletteType )
                    {
                        platformTex->paletteType = newPaletteType;
                    }

                    if ( newPaletteData != srcPaletteData )
                    {
                        platformTex->palette = newPaletteData;
                    }

                    if ( newPaletteSize != srcPaletteSize )
                    {
                        platformTex->paletteSize = newPaletteSize;
                    }

                    if ( newHasAlpha != srcHasAlpha )
                    {
                        platformTex->hasAlpha = newHasAlpha;
                    }

                    // Check for changes (texel mipmap data).

                    // Make sure we have NOT reduced the amount of mipmaps.
                    assert( srcMipmapCount == pixelData.mipmaps.size() );

                    for ( uint32 n = 0; n < srcMipmapCount; n++ )
                    {
                        const pixelDataTraversal::mipmapResource& curLayer = pixelData.mipmaps[ n ];

                        NativeTextureD3D::mipmapLayer& texMipLayer = platformTex->mipmaps[ n ];

                        // Get the new values onto the stack.
                        uint32 width = curLayer.width;
                        uint32 height = curLayer.height;
                        uint32 layerWidth = curLayer.mipWidth;
                        uint32 layerHeight = curLayer.mipHeight;
                        
                        void *newTexels = curLayer.texels;
                        uint32 newDataSize = curLayer.dataSize;

                        if ( texMipLayer.width != width )
                        {
                            texMipLayer.width = width;
                        }

                        if ( texMipLayer.height != height )
                        {
                            texMipLayer.height = height;
                        }

                        // Actually, the layer dimensions must never change.
                        assert( texMipLayer.layerWidth == layerWidth );
                        assert( texMipLayer.layerHeight == layerHeight );

                        if ( texMipLayer.texels != newTexels )
                        {
                            texMipLayer.texels = newTexels;
                            texMipLayer.dataSize = newDataSize;
                        }
                    }
                }
            }
        }

        if ( hasUpdated )
        {
            // Alright, lets do it.
            // If this produces a valid D3DFORMAT, we succeeded.
            // Keep in mind that this process is allowed to fail.
            platformTex->updateD3DFormat();
        }
    }
}

#pragma pack(1)
struct TgaHeader
{
    BYTE IDLength;        /* 00h  Size of Image ID field */
    BYTE ColorMapType;    /* 01h  Color map type */
    BYTE ImageType;       /* 02h  Image type code */
    WORD CMapStart;       /* 03h  Color map origin */
    WORD CMapLength;      /* 05h  Color map length */
    BYTE CMapDepth;       /* 07h  Depth of color map entries */
    WORD XOffset;         /* 08h  X origin of image */
    WORD YOffset;         /* 0Ah  Y origin of image */
    WORD Width;           /* 0Ch  Width of image */
    WORD Height;          /* 0Eh  Height of image */
    BYTE PixelDepth;      /* 10h  Image pixel size */

    struct
    {
        unsigned char numAttrBits : 4;
        unsigned char imageOrdering : 2;
        unsigned char reserved : 2;
    } ImageDescriptor; /* 11h  Image descriptor byte */
};
#pragma pack()

static void writeTGAPixels(
    const void *texelSource, uint32 colorUnitCount,
    eRasterFormat srcRasterFormat, uint32 srcItemDepth, ePaletteType srcPaletteType, const void *srcPaletteData, uint32 srcMaxPalette,
    eRasterFormat dstRasterFormat, uint32 dstItemDepth,
    eColorOrdering srcColorOrder,
    std::ostream& tga
)
{
    // Check whether we need to create a TGA compatible color array.
    if ( srcRasterFormat != dstRasterFormat || srcItemDepth != dstItemDepth || srcPaletteType != PALETTE_NONE || srcColorOrder != COLOR_BGRA )
    {
        // Get the data size.
        uint32 texelDataSize = getRasterDataSize(colorUnitCount, dstItemDepth);

        // Allocate a buffer for the fixed pixel data.
        void *tgaColors = new uint8[ texelDataSize ];

        for ( uint32 n = 0; n < colorUnitCount; n++ )
        {
            // Grab the color.
            uint8 red, green, blue, alpha;

            browsetexelcolor(texelSource, srcPaletteType, srcPaletteData, srcMaxPalette, n, srcRasterFormat, srcColorOrder, srcItemDepth, red, green, blue, alpha);

            // Write it with correct ordering.
            puttexelcolor(tgaColors, n, dstRasterFormat, COLOR_BGRA, dstItemDepth, red, green, blue, alpha);
        }

        tga.write((const char*)tgaColors, texelDataSize);

        // Free memory.
        delete [] tgaColors;
    }
    else
    {
        // Simply write the color source.
        uint32 texelDataSize = getRasterDataSize(colorUnitCount, srcItemDepth);

        tga.write((const char*)texelSource, texelDataSize);
    }
}

void Raster::writeTGA(const char *path, bool optimized)
{
    // If optimized == true, then this routine will output files in a commonly unsupported format
    // that is much smaller than in regular mode.
    // Problem with TGA is that it is poorly supported by most of the applications out there.

    //if ( this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9 )
    //    return;

	std::ofstream tga(path, std::ios::binary);
	if (tga.fail())
    {
        engineInterface->PushWarning( "not writing file: " + std::string( path ) );
		return;
	}

    writeTGAStream(tga, optimized);

	tga.close();
}

void Raster::writeTGAStream(std::ostream& tga, bool optimized)
{
    //if ( this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9 )
    //    return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    // Get the raw bitmap from layer 0.
    rawBitmapFetchResult rawBmp;

    bool gotRawBmp = platformTex->getRawBitmap(0, true, rawBmp);

    if ( !gotRawBmp )
        return;

    // Decide how to write the raster.
    eRasterFormat srcRasterFormat = rawBmp.rasterFormat;
    ePaletteType srcPaletteType = rawBmp.paletteType;
    uint32 srcItemDepth = rawBmp.depth;

    eRasterFormat dstRasterFormat;
    uint32 dstColorDepth;
    uint32 dstAlphaBits;
    bool hasDstRasterFormat = false;

    ePaletteType dstPaletteType;

    if ( !optimized )
    {
        // We output in a format that every parser should understand.
        dstRasterFormat = RASTER_8888;
        dstColorDepth = 32;
        dstAlphaBits = 8;

        dstPaletteType = PALETTE_NONE;

        hasDstRasterFormat = true;
    }
    else
    {
        if ( srcRasterFormat == RASTER_1555 )
        {
            dstRasterFormat = RASTER_1555;
            dstColorDepth = 16;
            dstAlphaBits = 1;

            hasDstRasterFormat = true;
        }
        else if ( srcRasterFormat == RASTER_565 )
        {
            dstRasterFormat = RASTER_565;
            dstColorDepth = 16;
            dstAlphaBits = 0;

            hasDstRasterFormat = true;
        }
        else if ( srcRasterFormat == RASTER_4444 )
        {
            dstRasterFormat = RASTER_4444;
            dstColorDepth = 16;
            dstAlphaBits = 4;

            hasDstRasterFormat = true;
        }
        else if ( srcRasterFormat == RASTER_8888 ||
                  srcRasterFormat == RASTER_888 )
        {
            dstRasterFormat = RASTER_8888;
            dstColorDepth = 32;
            dstAlphaBits = 8;

            hasDstRasterFormat = true;
        }
        else if ( srcRasterFormat == RASTER_555 )
        {
            dstRasterFormat = RASTER_555;
            dstColorDepth = 15;
            dstAlphaBits = 0;

            hasDstRasterFormat = true;
        }

        // We palettize if present.
        dstPaletteType = srcPaletteType;
    }

    if ( !hasDstRasterFormat )
    {
        engineInterface->PushWarning( "could not find a raster format to write TGA image with" );
    }
    else
    {
        // Prepare the TGA header.
        TgaHeader header;

        uint32 maxpalette = rawBmp.paletteSize;

        bool isPalette = (dstPaletteType != PALETTE_NONE);

        header.IDLength = 0;
        header.ColorMapType = ( isPalette ? 1 : 0 );
        header.ImageType = ( isPalette ? 1 : 2 );

        // The pixel depth is the number of bits a color entry is going to take (real RGBA color).
        uint32 pixelDepth = 0;

        if (isPalette)
        {
            pixelDepth = Bitmap::getRasterFormatDepth(dstRasterFormat);
        }
        else
        {
            pixelDepth = dstColorDepth;
        }
        
        header.CMapStart = 0;
        header.CMapLength = ( isPalette ? maxpalette : 0 );
        header.CMapDepth = ( isPalette ? pixelDepth : 0 );

        header.XOffset = 0;
        header.YOffset = 0;

        uint32 width = rawBmp.width;
        uint32 height = rawBmp.height;

        header.Width = width;
        header.Height = height;
        header.PixelDepth = ( isPalette ? srcItemDepth : dstColorDepth );

        header.ImageDescriptor.numAttrBits = dstAlphaBits;
        header.ImageDescriptor.imageOrdering = 2;
        header.ImageDescriptor.reserved = 0;

        // Write the header.
        tga.write((const char*)&header, sizeof(header));

        const void *texelSource = rawBmp.texelData;
        const void *paletteData = rawBmp.paletteData;
        eColorOrdering colorOrder = rawBmp.colorOrder;

        // Write the palette if we require.
        if (isPalette)
        {
            writeTGAPixels(
                paletteData, maxpalette,
                srcRasterFormat, pixelDepth, PALETTE_NONE, NULL, 0,
                dstRasterFormat, pixelDepth,
                colorOrder, tga
            );
        }

        // Now write image information.
        // If we are a palette, we simply write the color indice.
        uint32 colorUnitCount = ( width * height );

        if (isPalette)
        {
            assert( srcPaletteType != PALETTE_NONE );

            // Write a fixed version of the palette indice.
            uint32 texelDataSize = rawBmp.dataSize; // for palette items its the same size.

            void *fixedPalItems = new uint8[ texelDataSize ];

            for (uint32 n = 0; n < colorUnitCount; n++)
            {
                uint8 palIndex;

                bool couldGetPalIndex = getpaletteindex(texelSource, srcPaletteType, maxpalette, srcItemDepth, n, palIndex);

                if ( !couldGetPalIndex )
                {
                    palIndex = 0;
                }

                // Put the palette index.
                if (srcItemDepth == 4)
                {
                    ( (PixelFormat::palette4bit*)fixedPalItems )->setvalue(n, palIndex);
                }
                else if (srcItemDepth == 8)
                {
                    ( (PixelFormat::palette8bit*)fixedPalItems )->setvalue(n, palIndex);
                }
                else
                {
                    assert( 0 );
                }
            }

            tga.write((const char*)fixedPalItems, texelDataSize);

            // Clean up memory.
            delete [] fixedPalItems;
        }
        else
        {
            writeTGAPixels(
                texelSource, colorUnitCount,
                srcRasterFormat, srcItemDepth, srcPaletteType, paletteData, maxpalette,
                dstRasterFormat, dstColorDepth,
                colorOrder, tga
            );
        }
    }

    // Free raw bitmap resources.
    if ( rawBmp.isNewlyAllocated )
    {
        delete [] rawBmp.texelData;
    }
}

inline bool isValidFilterMode( uint32 binaryFilterMode )
{
    if ( binaryFilterMode == RWFILTER_POINT ||
         binaryFilterMode == RWFILTER_LINEAR ||
         binaryFilterMode == RWFILTER_POINT_POINT ||
         binaryFilterMode == RWFILTER_LINEAR_POINT ||
         binaryFilterMode == RWFILTER_POINT_LINEAR ||
         binaryFilterMode == RWFILTER_LINEAR_LINEAR ||
         binaryFilterMode == RWFILTER_ANISOTROPY )
    {
        return true;
    }

    return false;
}

inline bool isValidTexAddressingMode( uint32 binary_addressing )
{
    if ( binary_addressing == RWTEXADDRESS_WRAP ||
         binary_addressing == RWTEXADDRESS_MIRROR ||
         binary_addressing == RWTEXADDRESS_CLAMP ||
         binary_addressing == RWTEXADDRESS_BORDER )
    {
        return true;
    }

    return false;
}

void texFormatInfo::parse(TextureBase& outTex) const
{
    eRasterStageFilterMode rwFilterMode = RWFILTER_LINEAR;

    eRasterStageAddressMode rw_uAddressing = RWTEXADDRESS_WRAP;
    eRasterStageAddressMode rw_vAddressing = RWTEXADDRESS_WRAP;

    // Read our fields, which are from a binary stream.
    uint32 binaryFilterMode = this->filterMode;

    uint32 binary_uAddressing = this->uAddressing;
    uint32 binary_vAddressing = this->vAddressing;

    // Make sure they are valid.
    if ( isValidFilterMode( binaryFilterMode ) )
    {
        rwFilterMode = (eRasterStageFilterMode)binaryFilterMode;
    }
    else
    {
        outTex.engineInterface->PushWarning( "texture " + outTex.name + " has an invalid filter mode; defaulting to linear" );
    }

    if ( isValidTexAddressingMode( binary_uAddressing ) )
    {
        rw_uAddressing = (eRasterStageAddressMode)binary_uAddressing;
    }
    else
    {
        outTex.engineInterface->PushWarning( "texture " + outTex.name + " has an invalid uAddressing mode; defaulting to wrap" );
    }

    if ( isValidTexAddressingMode( binary_vAddressing ) )
    {
        rw_vAddressing = (eRasterStageAddressMode)binary_vAddressing;
    }
    else
    {
        outTex.engineInterface->PushWarning( "texture " + outTex.name + " has an invalid vAddressing mode; defaulting to wrap" );
    }

    // Put the fields.
    outTex.filterMode = rwFilterMode;
    outTex.uAddressing = rw_uAddressing;
    outTex.vAddressing = rw_vAddressing;
}

void texFormatInfo::set(const TextureBase& inTex)
{
    this->filterMode = (uint32)inTex.filterMode;
    this->uAddressing = (uint32)inTex.uAddressing;
    this->vAddressing = (uint32)inTex.vAddressing;
    this->pad1 = 0;
}

}
