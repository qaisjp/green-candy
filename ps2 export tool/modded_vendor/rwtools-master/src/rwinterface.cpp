#include <StdInc.h>

#include "pluginutil.hxx"

namespace rw
{

// Factory for interfaces.
RwMemoryAllocator _engineMemAlloc;

RwInterfaceFactory_t engineFactory;

Interface::Interface( void )
{
    // We set the version in a specialized constructor.

    // Set up the type system.
    this->typeSystem._memAlloc = &memAlloc;

    // Register the main RenderWare types.
    {
        this->streamTypeInfo = this->typeSystem.RegisterAbstractType <Stream> ( "stream" );
        this->rasterTypeInfo = this->typeSystem.RegisterStructType <Raster> ( "raster" );
        this->rwobjTypeInfo = this->typeSystem.RegisterAbstractType <RwObject> ( "rwobj" );
        this->textureTypeInfo = this->typeSystem.RegisterStructType <TextureBase> ( "texture", this->rwobjTypeInfo );
    }

    // Setup standard members.
    this->customFileInterface = NULL;

    this->warningManager = NULL;
    this->warningLevel = 3;
    this->ignoreSecureWarnings = true;

    // Only use the native toolchain.
    this->palRuntimeType = PALRUNTIME_NATIVE;

    // Prefer the native toolchain.
    this->dxtRuntimeType = DXTRUNTIME_NATIVE;

    this->fixIncompatibleRasters = true;
    this->dxtPackedDecompression = false;

    this->ignoreSerializationBlockRegions = false;
}

RwObject::RwObject( Interface *engineInterface, void *construction_params )
{
    // Constructor that is called for creation.
    this->engineInterface = engineInterface;
    this->objVersion = engineInterface->GetVersion();   // when creating an object, we assign it the current version.
}

inline void SafeDeleteType( Interface *engineInterface, RwTypeSystem::typeInfoBase*& theType )
{
    RwTypeSystem::typeInfoBase *theTypeVal = theType;

    if ( theTypeVal )
    {
        engineInterface->typeSystem.DeleteType( theTypeVal );

        theType = NULL;
    }
}

Interface::~Interface( void )
{
    // Unregister all types again.
    {
        SafeDeleteType( this, this->textureTypeInfo );
        SafeDeleteType( this, this->rwobjTypeInfo );
        SafeDeleteType( this, this->rasterTypeInfo );
        SafeDeleteType( this, this->streamTypeInfo );
    }
}

void Interface::SetVersion( LibraryVersion version )
{
    this->version = version;
}

RwObject* Interface::ConstructRwObject( const char *typeName )
{
    RwObject *newObj = NULL;

    if ( RwTypeSystem::typeInfoBase *rwobjTypeInfo = this->rwobjTypeInfo )
    {
        // Try to find a type that inherits from RwObject with this name.
        RwTypeSystem::typeInfoBase *rwTypeInfo = this->typeSystem.FindTypeInfo( typeName, rwobjTypeInfo );

        if ( rwTypeInfo )
        {
            // Try to construct us.
            GenericRTTI *rtObj = this->typeSystem.Construct( this, rwTypeInfo, NULL );

            if ( rtObj )
            {
                // We are successful! Return the new object.
                newObj = (RwObject*)RwTypeSystem::GetObjectFromTypeStruct( rtObj );
            }
        }
    }

    return newObj;
}

RwObject* Interface::CloneRwObject( const RwObject *srcObj )
{
    RwObject *newObj = NULL;

    // We simply use our type system to do the job.
    const GenericRTTI *rttiObj = this->typeSystem.GetTypeStructFromConstAbstractObject( srcObj );

    if ( rttiObj )
    {
        GenericRTTI *newRtObj = this->typeSystem.Clone( rttiObj );

        if ( newRtObj )
        {
            newObj = (RwObject*)RwTypeSystem::GetObjectFromTypeStruct( newRtObj );
        }
    }

    return newObj;
}

void Interface::DeleteRwObject( RwObject *obj )
{
    // Delete it using the type system.
    GenericRTTI *rttiObj = this->typeSystem.GetTypeStructFromAbstractObject( obj );

    if ( rttiObj )
    {
        this->typeSystem.Destroy( rttiObj );
    }
}

void Interface::GetObjectTypeNames( rwobjTypeNameList_t& listOut ) const
{
    if ( RwTypeSystem::typeInfoBase *rwobjTypeInfo = this->rwobjTypeInfo )
    {
        LIST_FOREACH_BEGIN( RwTypeSystem::typeInfoBase, this->typeSystem.registeredTypes.root, node )

            if ( item != rwobjTypeInfo )
            {
                if ( this->typeSystem.IsTypeInheritingFrom( rwobjTypeInfo, item ) )
                {
                    listOut.push_back( item->name );
                }
            }

        LIST_FOREACH_END
    }
}

bool Interface::IsObjectRegistered( const char *typeName ) const
{
    if ( RwTypeSystem::typeInfoBase *rwobjTypeInfo = this->rwobjTypeInfo )
    {
        // Try to find a type that inherits from RwObject with this name.
        RwTypeSystem::typeInfoBase *rwTypeInfo = this->typeSystem.FindTypeInfo( typeName, rwobjTypeInfo );

        if ( rwTypeInfo )
        {
            return true;
        }
    }

    return false;
}

const char* Interface::GetObjectTypeName( const RwObject *rwObj ) const
{
    const char *typeName = "unknown";

    const GenericRTTI *rtObj = this->typeSystem.GetTypeStructFromConstAbstractObject( rwObj );

    if ( rtObj )
    {
        RwTypeSystem::typeInfoBase *typeInfo = RwTypeSystem::GetTypeInfoFromTypeStruct( rtObj );

        // Return its type name.
        typeName = typeInfo->name;
    }

    return typeName;
}

void Interface::SetWarningManager( WarningManagerInterface *warningMan )
{
    this->warningManager = warningMan;
}

void Interface::SetWarningLevel( int level )
{
    this->warningLevel = level;
}

int Interface::GetWarningLevel( void ) const
{
    return this->warningLevel;
}

struct warningHandlerPlugin
{
    // The purpose of the warning handler stack is to fetch warning output requests and to reroute them
    // so that they make more sense.
    std::vector <WarningHandler*> warningHandlerStack;

    inline void Initialize( Interface *engineInterface )
    {

    }

    inline void Shutdown( Interface *engineInterface )
    {
        // We unregister all warning handlers.
        // The deallocation has to happen through the registree.
    }
};

static PluginDependantStructRegister <warningHandlerPlugin, RwInterfaceFactory_t> warningHandlerPluginRegister( engineFactory );

void Interface::PushWarning( const std::string& message )
{
    if ( this->warningLevel > 0 )
    {
        // If we have a warning handler, we redirect the message to it instead.
        // The warning handler is supposed to be an internal class that only the library has access to.
        WarningHandler *currentWarningHandler = NULL;

        warningHandlerPlugin *whandlerEnv = warningHandlerPluginRegister.GetPluginStruct( (EngineInterface*)this );

        if ( whandlerEnv )
        {
            if ( !whandlerEnv->warningHandlerStack.empty() )
            {
                currentWarningHandler = whandlerEnv->warningHandlerStack.back();
            }
        }

        if ( currentWarningHandler )
        {
            // Give it the warning.
            currentWarningHandler->OnWarningMessage( message );
        }
        else
        {
            // Else we just post the warning to the runtime.
            if ( WarningManagerInterface *warningMan = this->warningManager )
            {
                warningMan->OnWarning( message );
            }
        }
    }
}

void GlobalPushWarningHandler( Interface *engineInterface, WarningHandler *theHandler )
{
    warningHandlerPlugin *whandlerEnv = warningHandlerPluginRegister.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( whandlerEnv )
    {
        whandlerEnv->warningHandlerStack.push_back( theHandler );
    }
}

void GlobalPopWarningHandler( Interface *engineInterface )
{
    warningHandlerPlugin *whandlerEnv = warningHandlerPluginRegister.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( whandlerEnv )
    {
        assert( whandlerEnv->warningHandlerStack.empty() == false );

        whandlerEnv->warningHandlerStack.pop_back();
    }
}

void Interface::SetIgnoreSecureWarnings( bool doIgnore )
{
    this->ignoreSecureWarnings = doIgnore;
}

bool Interface::GetIgnoreSecureWarnings( void ) const
{
    return this->ignoreSecureWarnings;
}

void Interface::SetPaletteRuntime( ePaletteRuntimeType palRunType )
{
    this->palRuntimeType = palRunType;
}

ePaletteRuntimeType Interface::GetPaletteRuntime( void ) const
{
    return this->palRuntimeType;
}

void Interface::SetDXTRuntime( eDXTCompressionMethod dxtRunType )
{
    this->dxtRuntimeType = dxtRunType;
}

eDXTCompressionMethod Interface::GetDXTRuntime( void ) const
{
    return this->dxtRuntimeType;
}

void Interface::SetFixIncompatibleRasters( bool doFix )
{
    this->fixIncompatibleRasters = doFix;
}

bool Interface::GetFixIncompatibleRasters( void ) const
{
    return this->fixIncompatibleRasters;
}

void Interface::SetDXTPackedDecompression( bool packedDecompress )
{
    this->dxtPackedDecompression = packedDecompress;
}

bool Interface::GetDXTPackedDecompression( void ) const
{
    return this->dxtPackedDecompression;
}

void Interface::SetIgnoreSerializationBlockRegions( bool doIgnore )
{
    this->ignoreSerializationBlockRegions = doIgnore;
}

bool Interface::GetIgnoreSerializationBlockRegions( void ) const
{
    return this->ignoreSerializationBlockRegions;
}

// Interface creation for the RenderWare engine.
Interface* CreateEngine( LibraryVersion theVersion )
{
    // Create a specialized engine depending on the version.
    Interface *engineOut = engineFactory.Construct( _engineMemAlloc );

    if ( engineOut )
    {
        engineOut->SetVersion( theVersion );

        try
        {
            // Initialize all environments.
            initializeTXDEnvironment( engineOut );
            initializeNativeTextureEnvironment( engineOut );
        }
        catch( ... )
        {
            engineFactory.Destroy( _engineMemAlloc, (EngineInterface*)engineOut );
            return NULL;
        }
    }

    return engineOut;
}

void DeleteEngine( Interface *theEngine )
{
    shutdownNativeTextureEnvironment( theEngine );
    shutdownTXDEnvironment( theEngine );

    // Destroy the engine again.
    engineFactory.Destroy( _engineMemAlloc, (EngineInterface*)theEngine );
}

};