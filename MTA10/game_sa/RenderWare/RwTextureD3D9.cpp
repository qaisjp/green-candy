/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RwTextureD3D9.cpp
*  PURPOSE:     RenderWare texture implementation for D3D9
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "../gamesa_renderware.h"
#include "RwInternals.h"

#include "RwRenderTools.hxx"

// Render state modification functions that are mainly used by this module.
static rwDeviceValue_t _currentAlphaBlendEnable = false;        // Binary offsets: (1.0 US and 1.0 EU): 0x00C9A4E8
static rwDeviceValue_t _currentAlphaTestEnable = false;         // Binary offsets: (1.0 US and 1.0 EU): 0x00C9A5D4
static rwDeviceValue_t _currentIsVertexAlphaLocked = false;     // Binary offsets: (1.0 US and 1.0 EU): 0x00C9A4EC

inline unsigned int& GetCurrentAlphaBlendEnable( void )     { return _currentAlphaBlendEnable; }
inline unsigned int& GetCurrentAlphaTestEnable( void )      { return _currentAlphaTestEnable; }
inline unsigned int& GetIsVertexAlphaLocked( void )         { return _currentIsVertexAlphaLocked; }

inline void _RwD3D9UpdateAlphaEnable( rwDeviceValue_t blendEnable, rwDeviceValue_t testEnable )
{
    HOOK_RwD3D9SetRenderState( D3DRS_ALPHABLENDENABLE, blendEnable );
    HOOK_RwD3D9SetRenderState( D3DRS_ALPHATESTENABLE, ( blendEnable && testEnable ) );
}

// Used by RwD3D9RenderStateSetVertexAlphaEnabled.
int _RwD3D9SetAlphaEnable( rwDeviceValue_t blendEnable, rwDeviceValue_t testEnable )
{
    _RwD3D9UpdateAlphaEnable( blendEnable, testEnable );

    GetCurrentAlphaBlendEnable() = blendEnable;
    GetCurrentAlphaTestEnable() = testEnable;
    return 1;
}

int RwD3D9SetAlphaEnable( rwDeviceValue_t blendEnable, rwDeviceValue_t testEnable )
{
    if ( GetCurrentAlphaBlendEnable() == blendEnable &&
         GetCurrentAlphaTestEnable() == testEnable )
    {
        return 1;
    }

    return _RwD3D9SetAlphaEnable( blendEnable, testEnable );
}

int RwD3D9ResetAlphaEnable( void )
{
    _RwD3D9UpdateAlphaEnable( GetCurrentAlphaBlendEnable(), GetCurrentAlphaTestEnable() );
    return 1;
}

int RwD3D9SetVirtualAlphaTestState( bool enable )
{
    DWORD currentAlphaBlendState;

    HOOK_RwD3D9GetRenderState( D3DRS_ALPHABLENDENABLE, currentAlphaBlendState );

    GetCurrentAlphaTestEnable() = enable;

    if ( currentAlphaBlendState == TRUE )
    {
        HOOK_RwD3D9SetRenderState( D3DRS_ALPHATESTENABLE, enable );
    }

    return 1;
}

// Direct3D9_texture plugin code.
struct d3d9RasterData
{
    IDirect3DBaseTexture9*  renderResource;     // Direct3D texture resource
    unsigned int            paletteNumber;
    bool                    isAlpha;
};

AINLINE int GetPluginOffset( void )
{
    return *(int*)0x00B4E9E0;
}

AINLINE d3d9RasterData* GetRasterInfo( RwRaster *raster )
{
    if ( raster == NULL )
        return NULL;

    int offset = GetPluginOffset();

    if ( offset == -1 )
        return NULL;

    return RW_PLUGINSTRUCT <d3d9RasterData> ( raster, offset );
}

AINLINE const d3d9RasterData* GetRasterInfoConst( const RwRaster *raster )
{
    if ( raster == NULL )
        return NULL;

    int offset = GetPluginOffset();

    if ( offset == -1 )
        return NULL;

    return RW_PLUGINSTRUCT <const d3d9RasterData> ( raster, offset );
}

/*=========================================================
    RwD3D9SetRasterForStage

    Arguments:
        raster - pointer to GPU texture data
        stageIdx - the stage number to apply the texture data to
    Purpose:
        Sets the raster for the specified Direct3D stage index.
        Renderstates are adjusted in this function, if stageIdx
        is zero.
    Binary offsets:
        (1.0 US): 0x007FDCD0
        (1.0 EU): 0x007FDD10
=========================================================*/
// raster stage descriptors.
d3d9RasterStage _currentRasterStages[ MAX_SAMPLERS ];           // Binary offsets: (1.0 US and 1.0 EU): 0x00C9A508

// virtual renderstates.
static rwDeviceValue_t _currentIsVertexAlphaEnabled = false;    // Binary offsets: (1.0 US and 1.0 EU): 0x00C9A4E8
static rwDeviceValue_t _currentIsAlphaTestEnabled = false;      // Binary offsets: (1.0 US and 1.0 EU): 0x00C9A5D4

inline rwDeviceValue_t& GetIsVertexAlphaEnabled( void )         { return _currentIsVertexAlphaEnabled; }
inline rwDeviceValue_t& GetIsAlphaTestEnabled( void )           { return _currentIsAlphaTestEnabled; }

AINLINE void _RwD3D9LockVertexAlpha( unsigned int shouldLock )
{
    unsigned int& vertexAlphaLocked = GetIsVertexAlphaLocked();

    if ( vertexAlphaLocked != shouldLock )
    {
        vertexAlphaLocked = shouldLock;

        if ( GetIsVertexAlphaEnabled() == false )
        {
            bool enableAlphaBlend = ( shouldLock == TRUE );

            _RwD3D9UpdateAlphaEnable( enableAlphaBlend, GetIsAlphaTestEnabled() );
        }
    }
}

int __cdecl RwD3D9SetRasterForStage( RwRaster *raster, unsigned int stageIdx )
{
    d3d9RasterData *info = GetRasterInfo( raster );

    if ( stageIdx == 0 )
    {
        bool lockVertexAlpha = ( raster && info->isAlpha );
        
        _RwD3D9LockVertexAlpha( lockVertexAlpha );
    }

    // Update the Direct3D 9 texture data.
    d3d9RasterStage& rasterStage = GetRasterStageInfo( stageIdx );

    if ( rasterStage.raster != raster )
    {
        rasterStage.raster = raster;

        IDirect3DBaseTexture9 *gpuTextureData = NULL;
        bool hasTexturePalette = false;
        unsigned int paletteNumber = 0;

        if ( raster != NULL )
        {
            gpuTextureData = info->renderResource;

            if ( info->paletteNumber != 0 )
            {
                hasTexturePalette = true;
                paletteNumber = info->paletteNumber;
            }
        }

        {
            IDirect3DDevice9 *renderDevice = GetRenderDevice_Native();

            renderDevice->SetTexture( stageIdx, gpuTextureData );

            if ( hasTexturePalette )
            {
                renderDevice->SetCurrentTexturePalette( paletteNumber );
            }
        }
    }

    return 1;
}

// Utilities that do not necessaringly belong here.
template <typename numberType>
AINLINE numberType easyPow( numberType base, numberType exp )
{
    const numberType zero_val = (numberType)0;

    numberType current = (numberType)1;

    while ( exp != 0 )
    {
        current *= base;
    }

    return current;
}

unsigned int GetBitRegion( unsigned int value, unsigned int offset, unsigned int span )
{
    return ( value << offset ) & ( easyPow( 2u, span ) - 1 );
}

int& GetSharedTextureAnisotOffset( void )
{
    return *(int*)0x00C9A5E8;
}

struct d3d9TextureAnisotropyInfo
{
    unsigned char anisotropy;

    BYTE pad[3];
};

d3d9TextureAnisotropyInfo* GetTextureAnisotropyInfo( RwTexture *tex )
{
    if ( tex == NULL )
        return NULL;

    int pluginOffset = GetSharedTextureAnisotOffset();

    return ( pluginOffset >= 0 ) ? ( RW_PLUGINSTRUCT <d3d9TextureAnisotropyInfo> ( tex, pluginOffset ) ) : ( NULL );
}

const d3d9TextureAnisotropyInfo* GetTextureAnisotropyInfoConst( const RwTexture *tex )
{
    if ( tex == NULL )
        return NULL;

    int pluginOffset = GetSharedTextureAnisotOffset();

    return ( pluginOffset >= 0 ) ? ( RW_PLUGINSTRUCT <const d3d9TextureAnisotropyInfo> ( tex, pluginOffset ) ) : ( NULL );
}

/*=========================================================
    RwD3D9SetTexture

    Arguments:
        texture - texture information with raster
        stageIdx - the stage number to apply the texture data to
    Purpose:
        Updates the given raster stage (stageIndex) by texture
        information of the texture object. If texture is NULL,
        then the raster stage is disabled.
    Binary offsets:
        (1.0 US): 0x007FDE70
        (1.0 EU): 0x007FDEB0
=========================================================*/
int __cdecl RwD3D9SetTexture( RwTexture *texture, unsigned int stageIndex )
{
    RwRaster *texRaster = NULL;

    if ( texture )
    {
        // Set Texture address modes.
        RwD3D9RasterStageSetAddressModeU( stageIndex, texture->u_addressMode );
        RwD3D9RasterStageSetAddressModeV( stageIndex, texture->v_addressMode );

        unsigned int filterMode = texture->filterMode;

        // Attempt to get the texture plugin struct.
        // It does not have to be loaded.
        if ( d3d9TextureAnisotropyInfo *anisot = GetTextureAnisotropyInfo( texture ) )
        {
            RwD3D9RasterStageSetMaxAnisotropy( stageIndex, anisot->anisotropy );

            if ( anisot->anisotropy > 1 )
            {
                filterMode = 7;
            }
        }

        // Set Texture filter mode.
        RwD3D9RasterStageSetFilterMode( stageIndex, filterMode );

        texRaster = texture->raster;
    }

    RwD3D9SetRasterForStage( texRaster, stageIndex );
    return 1;
}

/*=========================================================
    RwD3D9RenderStateSetVertexAlphaEnabled

    Arguments:
        enabled - switch to turn vertex alpha on or off
    Purpose:
        Central RenderWare function to enable or disable
        raster alpha. This function ensure that the proper
        states are called depending on what the current engine
        status is. Should be prefered instead of manually
        setting the alpha status.
    Binary offsets:
        (1.0 US): 0x007FE0A0
        (1.0 EU): 0x007FE0E0
=========================================================*/
// note: RenderWare must have a way to determine alpha by raster.
// having access to this feature could greatly improve rendering.
int __cdecl RwD3D9RenderStateSetVertexAlphaEnabled( rwDeviceValue_t enabled )
{
    if ( enabled != GetIsVertexAlphaEnabled() )
    {
        GetIsVertexAlphaEnabled() = enabled;

        if ( GetIsVertexAlphaLocked() == false )
        {
            // Actually an inlined-always-change call, but we do it thisway rather.
            // If the implementation follows its own rules, everything is fine.
            // This asserts, that MTA will not set rogue render states anymore!
            _RwD3D9SetAlphaEnable( enabled, GetIsAlphaTestEnabled() );
        }
    }

    return 1;
}

/*=========================================================
    RwD3D9RenderStateIsVertexAlphaEnabled

    Purpose:
        Returns whether the device renders rasters using the
        alpha channel or not.
    Binary offsets:
        (1.0 US): 0x007FE190
        (1.0 EU): 0x007FE1D0
=========================================================*/
rwDeviceValue_t __cdecl RwD3D9RenderStateIsVertexAlphaEnabled( void )
{
    return GetIsVertexAlphaEnabled();
}

/*=========================================================
    RwTextureHasAlpha

    Arguments:
        tex - the texture to check the alpha status of
    Purpose:
        Returns whether the given texture is not fully opaque.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C9EA0
=========================================================*/
int __cdecl RwTextureHasAlpha( RwTexture *tex )
{
    RwRaster *raster = tex->raster;

    if ( raster )
    {
        const d3d9RasterData *rasterInfo = GetRasterInfoConst( raster );

        return rasterInfo ? rasterInfo->isAlpha : false;
    }

    return false;
}

/*=========================================================
    RwTextureGetVideoData (MTA extension)

    Arguments:
        tex - the texture to get the video data pointer of
    Purpose:
        Returns the Direct3D 9 video data pointer from the
        given texture.
=========================================================*/
IDirect3DBaseTexture9* RwTextureGetVideoData( RwTexture *tex )
{
    RwRaster *raster = tex->raster;

    if ( !raster )
        return NULL;

    const d3d9RasterData *rasterInfo = GetRasterInfoConst( raster );

    return rasterInfo ? rasterInfo->renderResource : NULL;
}

void RwTextureD3D9_Init( void )
{
    switch( pGame->GetGameVersion() )
    {
    case VERSION_EU_10:
        HookInstall( 0x007FE0E0, (DWORD)RwD3D9RenderStateSetVertexAlphaEnabled, 5 );
        HookInstall( 0x007FE1D0, (DWORD)RwD3D9RenderStateIsVertexAlphaEnabled, 5 );
        HookInstall( 0x007FDEB0, (DWORD)RwD3D9SetTexture, 5 );
        HookInstall( 0x007FDD10, (DWORD)RwD3D9SetRasterForStage, 5 );
        break;
    case VERSION_US_10:
        HookInstall( 0x007FE0A0, (DWORD)RwD3D9RenderStateSetVertexAlphaEnabled, 5 );
        HookInstall( 0x007FE190, (DWORD)RwD3D9RenderStateIsVertexAlphaEnabled, 5 );
        HookInstall( 0x007FDE70, (DWORD)RwD3D9SetTexture, 5 );
        HookInstall( 0x007FDCD0, (DWORD)RwD3D9SetRasterForStage, 5 );
        break;
    }
}

void RwTextureD3D9_Shutdown( void )
{
    
}