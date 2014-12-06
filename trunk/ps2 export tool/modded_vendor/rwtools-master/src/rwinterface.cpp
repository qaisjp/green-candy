#include <StdInc.h>

namespace rw
{

Interface::Interface( void )
{
    // Default to San Andreas version.
    this->version = rw::SA;

    // Setup standard members.
    this->customFileInterface = NULL;
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

// Main interface of the engine.
Interface rwInterface;

};