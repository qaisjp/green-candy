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
    this->warningLevel = 3;

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

void Interface::SetWarningLevel( int level )
{
    this->warningLevel = level;
}

int Interface::GetWarningLevel( void ) const
{
    return this->warningLevel;
}

void Interface::PushWarning( const std::string& message )
{
    if ( this->warningLevel > 0 )
    {
        if ( WarningManagerInterface *warningMan = this->warningManager )
        {
            warningMan->OnWarning( message );
        }
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

void Interface::SetFixIncompatibleRasters( bool doFix )
{
    this->fixIncompatibleRasters = doFix;
}

bool Interface::GetFixIncompatibleRasters( void ) const
{
    return this->fixIncompatibleRasters;
}

// Main interface of the engine.
Interface rwInterface;

};