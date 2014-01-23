/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RwInternals.h
*  PURPOSE:     RenderWare internal definitions
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RENDERWARE_INTERNALS_
#define _RENDERWARE_INTERNALS_

// padlevel: 2
struct d3d9RasterStage  //size: 24 bytes
{
    RwRaster *raster;                   // 0
    unsigned int u_addressMode;         // 4
    unsigned int v_addressMode;         // 8
    unsigned int filterType;            // 12
    RwColor borderColor;                // 16
    unsigned int maxAnisotropy;         // 20
};

inline d3d9RasterStage& GetRasterStageInfo( unsigned int index )
{
    return ((d3d9RasterStage*)0x00C9A508)[index];
}

#endif //_RENDERWARE_INTERNALS_