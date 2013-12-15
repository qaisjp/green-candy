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
    if ( !texture )
    {
        if ( stageIndex == 0 )
        {
            
        }

        return 1;
    }
}