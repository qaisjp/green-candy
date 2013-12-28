/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.lighting.h
*  PURPOSE:     GTA:SA lighting management
*               Extended to properly support dynamic lighting
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RENDERWARE_LIGHTING_MANAGEMENT_
#define _RENDERWARE_LIGHTING_MANAGEMENT_

// Lighting system exports.
int __cdecl     RpD3D9LightsEqual           ( const D3DLIGHT9& left, const D3DLIGHT9& right );
int __cdecl     RpD3D9SetLight              ( int lightIndex, const D3DLIGHT9& lightStruct );
int __cdecl     RpD3D9DirLightEnable        ( RpLight *light );
int __cdecl     RpD3D9LocalLightEnable      ( RpLight *light );
int __cdecl     RpD3D9EnableLight           ( int lightIndex, int phase );
void __cdecl    RpD3D9EnableLights          ( bool enable, int unused );

// Module initialization.
void RenderWareLighting_Init( void );
void RenderWareLighting_Shutdown( void );

#endif //_RENDERWARE_LIGHTING_MANAGEMENT_