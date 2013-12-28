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

struct d3d9RasterStage  //size: 24 bytes
{
    RwRaster *raster;       // 0
    BYTE pad[20];           // 4
};

inline d3d9RasterStage& GetRasterStageInfo( unsigned int index )
{
    return ((d3d9RasterStage*)0x00C9A508)[index];
}

inline RwRaster*& GetCurrentRaster( void )
{
    return *((RwRaster**)0x00C9A4EC);
}

int __cdecl RwD3D9SetTexture( RwTexture *texture, unsigned int stageIndex )
{
#if 0
    if ( !texture )
    {
        if ( stageIndex == 0 )
        {
            
        }

        return 1;
    }
#else
    return ((int (__cdecl*)( RwTexture *tex, unsigned int stageIndex ))0x007FDE70)( texture, stageIndex );
#endif
}

inline DWORD& GetIsVertexAlphaEnabled( void )
{
    return *(DWORD*)0x00C9A4E8;
}

inline DWORD& GetIsAlphaTestEnabled( void )
{
    return *(DWORD*)0x00C9A5D4;
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
void __cdecl RwD3D9RenderStateSetVertexAlphaEnabled( DWORD enabled )
{
    if ( enabled == GetIsVertexAlphaEnabled() )
        return;

    GetIsVertexAlphaEnabled() = enabled;

    if ( GetCurrentRaster() )
        return;

    // Actually an inlined-always-change call, but we do it thisway rather.
    // If the implementation follows its own rules, everything is fine.
    // This asserts, that MTA will not set rogue render states anymore!
    HOOK_RwD3D9SetRenderState( D3DRS_ALPHABLENDENABLE, enabled );
    HOOK_RwD3D9SetRenderState( D3DRS_ALPHATESTENABLE, ( enabled ) ? GetIsAlphaTestEnabled() : FALSE );
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
DWORD __cdecl RwD3D9RenderStateIsVertexAlphaEnabled( void )
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

    return raster ? raster->isAlpha : false;
}

void RwTextureD3D9_Init( void )
{
    switch( pGame->GetGameVersion() )
    {
    case VERSION_EU_10:
        HookInstall( 0x007FE0E0, (DWORD)RwD3D9RenderStateSetVertexAlphaEnabled, 5 );
        HookInstall( 0x007FE1D0, (DWORD)RwD3D9RenderStateIsVertexAlphaEnabled, 5 );
        break;
    case VERSION_US_10:
        HookInstall( 0x007FE0A0, (DWORD)RwD3D9RenderStateSetVertexAlphaEnabled, 5 );
        HookInstall( 0x007FE190, (DWORD)RwD3D9RenderStateIsVertexAlphaEnabled, 5 );
        break;
    }
}

void RwTextureD3D9_Shutdown( void )
{
    
}