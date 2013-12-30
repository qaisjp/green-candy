/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RwRenderTools.hxx
*  PURPOSE:     RenderWare device rendering tools
*               Include this file when you want to effectively create
*               RenderWare rendering callbacks.
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RENDERWARE_RENDER_TOOLS_
#define _RENDERWARE_RENDER_TOOLS_

#include "gamesa_renderware.h"

inline IDirect3DDevice9* GetRenderDevice( void )
{
    return core->GetGraphics()->GetDevice();
}

inline IDirect3DDevice9* GetRenderDevice_Native( void )
{
    // Required for callback operations using dummy interfaces.
    // This has been introduced by Core module.
    return *(IDirect3DDevice9**)0x00C97C28;
}

struct RwD3D9StreamInfo //size: 16 bytes
{
    IDirect3DVertexBuffer9* m_vertexBuf;    // 0
    size_t m_offset;                        // 4
    size_t m_stride;                        // 8
    int unk4;                               // 12
};

struct RwD3D9Streams    //size: 32 bytes
{
    RwD3D9StreamInfo    streams[2];     // 0
};

void __cdecl RwD3D9SetStreams                   ( RwD3D9Streams& streams, int useOffset );

//padlevel: 2
struct RwRenderPass //size: 36 bytes
{
    RpMaterial*                     m_useMaterial;          // 0
    int                             m_vertexAlpha;          // 4, boolean whether render pass requires alpha blending
    IDirect3DVertexShader9*         m_vertexShader;         // 8
    unsigned int                    m_startVertex;          // 12
    unsigned int                    m_indexedNumVertice;    // 16, used for indexed rendering
    unsigned int                    m_indexedStartIndex;    // 20
    unsigned int                    m_numPrimitives;        // 24
    BYTE                            m_pad2[8];              // 28
};

//padlevel: 2
struct RwRenderCallbackTraverseImpl
{
    BYTE                            m_pad[4];               // 24
    unsigned int                    m_numPasses;            // 28
    IDirect3DIndexBuffer9*          m_indexBuffer;          // 32
    D3DPRIMITIVETYPE                m_primitiveType;        // 36
    RwD3D9Streams                   m_vertexStreams;        // 40
    int                             m_useOffsets;           // 72
    IDirect3DVertexDeclaration9*    m_vertexDecl;           // 76
    BYTE                            m_pad2[16];             // 80

    // Returns a dynamically allocated render pass from this internal interface.
    // These are appended at the end of the RwRenderCallbackTraverse structure by
    // m_numPasses count.
    inline RwRenderPass& GetRenderPass( unsigned int pass )
    {
        assert( pass < m_numPasses );

        return *( (RwRenderPass*)( this + 1 ) + pass );
    }
};

// padlevel: 1
struct RwRenderCallbackTraverse //size: 24 bytes
{
    BYTE                            m_pad[24];              // 0
    RwRenderCallbackTraverseImpl    m_impl;                 // 24
};

void __cdecl RwD3D9SetCurrentPixelShader        ( IDirect3DPixelShader9 *pixelShader );
void __cdecl RwD3D9UnsetPixelShader             ( void );

void __cdecl RwD3D9SetCurrentVertexShader       ( IDirect3DVertexShader9 *vertexShader );
void __cdecl RwD3D9SetCurrentIndexBuffer        ( IDirect3DIndexBuffer9 *indexBuf );
void __cdecl RwD3D9SetCurrentVertexDeclaration  ( IDirect3DVertexDeclaration9 *vertexDecl );

void __cdecl RpD3D9DrawIndexedPrimitive         ( D3DPRIMITIVETYPE primitiveType, INT baseVertexIndex, UINT minVertexIndex, UINT numVertice, UINT startIndex, UINT primCount );
void __cdecl RpD3D9DrawPrimitive                ( D3DPRIMITIVETYPE primitiveType, UINT startVertex, UINT numPrimitives );

void __cdecl RwD3D9EnableClippingIfNeeded       ( RwObject *obj, eRwType renderType );

// Declaration every render callback must have.
typedef void (__cdecl*rxRenderCallback_t)( RwRenderCallbackTraverse *rtinfo, RwObject *renderObject, eRwType renderType, unsigned int renderFlags );

// Helper function to decide whether alpha rendering is required.
inline bool RwD3D9IsAlphaRenderingRequired( unsigned int renderFlags, DWORD& lightValue )
{
    HOOK_RwD3D9GetRenderState( D3DRS_LIGHTING, lightValue );

    return !IS_ANY_FLAG( renderFlags, 0x08 ) && !lightValue;
}

// Helper function to decide whether vertex alpha rendering is required.
inline bool RwD3D9IsVertexAlphaRenderingRequired( RwRenderPass *rtPass, RpMaterial *curMat )
{
    return ( curMat->m_color.a != 0xFF || rtPass->m_vertexAlpha );
}

// Helper function to update surface properties.
inline bool RwD3D9UpdateRenderPassSurfaceProperties( RwRenderPass *rtPass, DWORD lightValue, RpMaterial *curMat, unsigned int renderFlags )
{
    if ( lightValue )
    {
        RpD3D9SetSurfaceProperties( curMat->m_lighting, curMat->m_color, renderFlags );
        return true;
    }

    return false;
}

// Helper function to draw meshes of current render pass.
inline void RwD3D9DrawRenderPassPrimitive( RwRenderCallbackTraverseImpl *rtinfo, RwRenderPass *rtPass )
{
    // Assumes that we set index buffer according to rt-rules.
    if ( rtinfo->m_indexBuffer )
        RpD3D9DrawIndexedPrimitive( rtinfo->m_primitiveType, rtPass->m_startVertex, 0, rtPass->m_indexedNumVertice, rtPass->m_indexedStartIndex, rtPass->m_numPrimitives );
    else
        RpD3D9DrawPrimitive( rtinfo->m_primitiveType, rtPass->m_startVertex, rtPass->m_numPrimitives );
}

void RpD3D9RenderLightMeshForPass( RwRenderCallbackTraverseImpl *rtinfo, RwRenderPass *rtPass );

// Generic render pass loop.
template <typename callbackType>
__forceinline void _RenderVideoDataGeneric( RwRenderCallbackTraverseImpl *rtinfo, callbackType& cb )
{
    // Draw rendering passes.
    for ( unsigned int n = 0; n < rtinfo->m_numPasses; n++ )
    {
        RwRenderPass *rtPass = &rtinfo->GetRenderPass( n );

        // Enable global lighting beforehand.
        // This should optimize things, as we avoid two render passes.
        RpD3D9GlobalLightingPrePass();

        // Notify the callback template.
        cb.OnRenderPass( rtPass );

        // Update vertex shader status.
        RwD3D9SetCurrentVertexShader( rtPass->m_vertexShader );

        // Draw the primitive.
        RwD3D9DrawRenderPassPrimitive( rtinfo, rtPass );

        RpD3D9RenderLightMeshForPass( rtinfo, rtPass );
    }

    RpD3D9ResetLightStatus();
}

// Helper function to set textured render states.
inline void RwD3D9SetTexturedRenderStates( bool hasTexture )
{
    if ( hasTexture )
    {
        RwD3D9SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
        RwD3D9SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        RwD3D9SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

        RwD3D9SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
        RwD3D9SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        RwD3D9SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    }
    else
    {
        RwD3D9SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
        RwD3D9SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

        RwD3D9SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
        RwD3D9SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    }
}

#endif //_RENDERWARE_RENDER_TOOLS_