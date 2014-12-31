#include <StdInc.h>

namespace rw
{

Interface::Interface( void )
{
    // Default to San Andreas version.
    this->version = rw::SA;

    // Setup standard members.
    this->customFileInterface = NULL;

    this->warningManager = NULL;

    // Only use the native toolchain.
    this->palRuntimeType = PALRUNTIME_NATIVE;
}

Interface::~Interface( void )
{
    
}

void Interface::SetVersion( uint32 version )
{
    this->version = version;
}

void Interface::BeginDebug( void )
{
    rw::NativeTexture::StartDebug();
}

void Interface::ProcessDebug( void )
{
    rw::NativeTexture::DebugParameters();
}

void Interface::SetWarningManager( WarningManagerInterface *warningMan )
{
    this->warningManager = warningMan;
}

void Interface::PushWarning( const std::string& message )
{
    if ( WarningManagerInterface *warningMan = this->warningManager )
    {
        warningMan->OnWarning( message );
    }
}

void Interface::SetPaletteRuntime( ePaletteRuntimeType palRunType )
{
    this->palRuntimeType = palRunType;
}

ePaletteRuntimeType Interface::GetPaletteRuntime( void ) const
{
    return this->palRuntimeType;
}

// Main interface of the engine.
Interface rwInterface;

};