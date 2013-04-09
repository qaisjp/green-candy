/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.pipeline.h
*  PURPOSE:     RenderWare pipeline (OS implementation) management
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RENDERWARE_PIPELINES_
#define _RENDERWARE_PIPELINES_

// D3D9 pipeline functions
void __cdecl RwD3D9SetRenderState       ( D3DRENDERSTATETYPE type, DWORD value );
void __cdecl RwD3D9GetRenderState       ( D3DRENDERSTATETYPE type, DWORD& value );
void __cdecl RwD3D9ApplyDeviceStates    ( void );

// MTA extensions
void RwD3D9ForceRenderState             ( D3DRENDERSTATETYPE type, DWORD value );
void RwD3D9FreeRenderState              ( D3DRENDERSTATETYPE type );
void RwD3D9FreeRenderStates             ( void );


void RenderWarePipeline_Init( void );
void RenderWarePipeline_Shutdown( void );

#endif //_RENDERWARE_PIPELINES_