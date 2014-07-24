/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RwRenderStates.h
*  PURPOSE:     RenderWare renderstate management
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RENDERWARE_RENDERSTATES_
#define _RENDERWARE_RENDERSTATES_

inline DWORD F2DW( float val )  { return *(DWORD*)&val; }

// RenderState functions.
int RwD3D9SetFogColor                       ( unsigned int color );
int RwD3D9ResetFogColor                     ( void );
int RwD3D9SetFogTableMode                   ( unsigned int mode );
int RwD3D9ResetFogTableMode                 ( void );
int RwD3D9SetShadeMode                      ( unsigned int mode );
int RwD3D9ResetShadeMode                    ( void );
int RwD3D9SetZWriteEnable                   ( unsigned int enable );
int RwD3D9ResetZWriteEnable                 ( void );
int RwD3D9SetDepthFunctionEnable            ( unsigned int enable );
int RwD3D9ResetDepthFunctionEnable          ( void );
int RwD3D9SetStencilEnable                  ( unsigned int enable );
int RwD3D9ResetStencilEnable                ( void );
int RwD3D9SetStencilFail                    ( unsigned int opMode );
int RwD3D9ResetStencilFail                  ( void );
int RwD3D9SetStencilZFail                   ( unsigned int opMode );
int RwD3D9ResetStencilZFail                 ( void );
int RwD3D9SetStencilPass                    ( unsigned int opMode );
int RwD3D9ResetStencilPass                  ( void );
int RwD3D9SetStencilFunc                    ( unsigned int cmpOp );
int RwD3D9ResetStencilFunc                  ( void );
int RwD3D9SetStencilRef                     ( unsigned int ref );
int RwD3D9ResetStencilRef                   ( void );
int RwD3D9SetStencilMask                    ( unsigned int mask );
int RwD3D9ResetStencilMask                  ( void );
int RwD3D9SetStencilWriteMask               ( unsigned int mask );
int RwD3D9ResetStencilWriteMask             ( void );

// Renderstate get functions.
unsigned int RwD3D9GetFogColor              ( void );
unsigned int RwD3D9GetFogTableMode          ( void );
unsigned int RwD3D9GetShadeMode             ( void );
unsigned int RwD3D9GetZWriteEnable          ( void );
unsigned int RwD3D9GetDepthFunctionEnable   ( void );
unsigned int RwD3D9GetStencilEnable         ( void );
unsigned int RwD3D9GetStencilFail           ( void );
unsigned int RwD3D9GetStencilZFail          ( void );
unsigned int RwD3D9GetStencilPass           ( void );
unsigned int RwD3D9GetStencilFunc           ( void );
unsigned int RwD3D9GetStencilRef            ( void );
unsigned int RwD3D9GetStencilMask           ( void );
unsigned int RwD3D9GetStencilWriteMask      ( void );

// Raster stage functions.
int RwD3D9RasterStageSetFilterMode          ( unsigned int stageIdx, unsigned int filterMode );
int RwD3D9RasterStageResetFilterMode        ( unsigned int stageIdx );
int RwD3D9RasterStageSetAddressModeU        ( unsigned int stageIdx, unsigned int modeIdx );
int RwD3D9RasterStageResetAddressModeU      ( unsigned int stageIdx );
int RwD3D9RasterStageSetAddressModeV        ( unsigned int stageIdx, unsigned int modeIdx );
int RwD3D9RasterStageResetAddressModeV      ( unsigned int stageIdx );
int RwD3D9RasterStageSetBorderColor         ( unsigned int stageIdx, unsigned int color );
int RwD3D9RasterStageResetBorderColor       ( unsigned int stageIdx );
int RwD3D9RasterStageSetMaxAnisotropy       ( unsigned int stageIdx, int maxAnisotropy );
int RwD3D9RasterStageResetMaxAnisotropy     ( unsigned int stageIdx );

// Raster stage get functions.
unsigned int RwD3D9RasterStageGetFilterMode     ( unsigned int stageIdx );
unsigned int RwD3D9RasterStageGetAddressModeU   ( unsigned int stageIdx );
unsigned int RwD3D9RasterStageGetAddressModeV   ( unsigned int stageIdx );
unsigned int RwD3D9RasterStageGetBorderColor    ( unsigned int stageIdx );
int RwD3D9RasterStageGetMaxAnisotropy           ( unsigned int stageIdx );

// More RenderState functions.
int RwD3D9SetSrcBlend                       ( unsigned int blendMode );
int RwD3D9ResetSrcBlend                     ( void );
int RwD3D9SetDstBlend                       ( unsigned int blendMode );
int RwD3D9ResetDstBlend                     ( void );
int RwD3D9SetAlphaFunc                      ( unsigned int cmpOp );
int RwD3D9ResetAlphaFunc                    ( void );
int RwD3D9SetCullMode                       ( unsigned int cullMode );
int RwD3D9ResetCullMode                     ( void );
int RwD3D9SetFogEnable                      ( unsigned int enable );
int RwD3D9ResetFogEnable                    ( void );

// More RenderState get functions.
unsigned int RwD3D9GetSrcBlend              ( void );
unsigned int RwD3D9GetDstBlend              ( void );
unsigned int RwD3D9GetAlphaFunc             ( void );
unsigned int RwD3D9GetCullMode              ( void );
unsigned int RwD3D9GetFogEnable             ( void );

// Initializators.
void __cdecl RwD3D9InitializeDeviceStates   ( void );
void __cdecl RwD3D9ResetDeviceStates        ( void );

// Module initialization.
void RwRenderStatesD3D9_Init( void );
void RwRenderStatesD3D9_Shutdown( void );

#endif //_RENDERWARE_RENDERSTATES_