#include <StdInc.h>

namespace rw
{

Interface::Interface( void )
{
    // Default to San Andreas version.
    this->version = KnownVersions::getGameVersion( KnownVersions::SA );

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
}

Interface::~Interface( void )
{
    return;
}

void Interface::SetVersion( LibraryVersion version )
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

// Main interface of the engine.
Interface rwInterface;

};