/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.rwapi.cpp
*  PURPOSE:     RenderWare native function implementations
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

#include "RenderWare/RwRenderTools.hxx"

/*****************************************************************************
*
*   RenderWare Functions
*
*   WARNING: Do not modify these classes if you do not know what you are doing!
*   They are C++ representations of internal GTA:SA logic. I suggest you analyze
*   the internals first before you touch the RenderWare interfaces. Any change
*   might break the compatibility with the engine, so be careful.
*
*   The definitions for each RenderWare struct are in their own files now.
*   You can locate them inside the "RenderWare/" folder.
*
*****************************************************************************/

RwScene *const *p_gtaScene = (RwScene**)0x00C17038;

/*=========================================================
    RwObjectFrame::AddToFrame

    Arguments:
        frame - new parent frame
    Purpose:
        Set the parent for a frame extension object. The
        previous parent is unlinked.
    Binary offsets:
        (1.0 US): 0x0074BF20
        (1.0 EU): 0x0074BF70
=========================================================*/
void RwObjectFrame::AddToFrame( RwFrame *frame )
{
    RemoveFromFrame();

    parent = frame;

    if ( frame )
    {
        LIST_INSERT( frame->objects.root, lFrame );

        frame->Update();
    }
}

RpAtomic* __cdecl RpAtomicSetFrame( RpAtomic *atomic, RwFrame *frame )
{
    atomic->AddToFrame( frame );

    atomic->privateFlags |= 1;
    return atomic;
}
/*=========================================================
    RwObjectFrame::RemoveFromFrame

    Purpose:
        Unlink the current parent frame if there is one.
=========================================================*/
void RwObjectFrame::RemoveFromFrame( void )
{
    if ( !parent )
        return;

    LIST_REMOVE( lFrame );

    parent = NULL;
}

RwStaticGeometry::RwStaticGeometry( void )
{
    count = 0;
    link = NULL;

    unknown2 = NULL;
    unknown = 0;
    unknown3 = 0;
}

void* RwStaticGeometry::operator new ( size_t memSize )
{
    return _mallocNew( memSize );
}
/*=========================================================
    RwStaticGeometry::AllocateLink (GTA:SA extension)

    Arguments:
        count - number of links to allocate
    Purpose:
        Allocates a number of links to this static geometry.
        Previous links are discarded.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004CF140
=========================================================*/
RwRenderLink* RwStaticGeometry::AllocateLink( unsigned int count )
{
    if ( link )
        RwFreeAligned( link );

    this->count = count;

    return link = (RwRenderLink*)RwAllocAligned( (((count * sizeof(RwRenderLink) - 1) >> 6 ) + 1) << 6, 0x40 );
}

/*=========================================================
    RwFindTexture

    Arguments:
        name - primary name of the texture you want to find
        secName - name of a fallback texture if primary one is not found
    Purpose:
        Scans the global and the current TXD containers for a
        matching texture. If the TXD is found in the current TXD
        (m_findInstanceRef), it is referenced and returned to the
        caller. If not, the global texture container is researched.
        Textures in the global environment are not referenced upon
        return but are put into the current texture container if it
        is set. The global texture environment is either like a
        temporary storage for textures or a routine to dynamically
        create them and give to models.
    Note:
        GTA:SA does not use the global texture routine (m_findInstance).
    Binary offsets:
        (1.0 US): 0x007F3AC0
        (1.0 EU): 0x007F3B00
=========================================================*/
RwTexture* RwFindTexture( const char *name, const char *secName )
{
    RwInterface *rwInterface = RenderWare::GetInterface();

    RwTexture *tex = rwInterface->m_textureManager.findInstanceRef( name );

    // The global store will reference textures
    if ( tex )
    {
        tex->refs++;
        return tex;
    }

    if ( !( tex = rwInterface->m_textureManager.findInstance( name, secName ) ) )
    {
        // If we have not found anything, we tell the system about an error
        RwError err;
        err.err1 = 0x01;
        err.err2 = 0x16;

        // Actually, there is a missing texture handler; it is void though
        RwSetError( &err );
        return NULL;
    }

    if ( RwTexDictionary *txd = RwTexDictionaryGetCurrent() )
    {
        tex->RemoveFromDictionary();
        tex->AddToDictionary( txd );
    }

    return tex;
}

/*=========================================================
    RwSetError

    Arguments:
        info - error information
    Purpose:
        Notifies the RenderWare system about a runtime error.
    Binary offsets:
        (1.0 US): 0x00808820
        (1.0 EU): 0x00808860
=========================================================*/
RwError* RwSetError( RwError *info )
{
    RwInterface *rwInterface = RenderWare::GetInterface();

    if ( rwInterface->m_errorInfo.err1 )
        return info;

    if ( rwInterface->m_errorInfo.err2 != 0x80000000 )
        return info;

    if ( info->err1 & 0x80000000 )
        rwInterface->m_errorInfo.err1 = 0;
    else
        rwInterface->m_errorInfo.err1 = info->err1;

    rwInterface->m_errorInfo.err2 = info->err2;
    return info;
}

/*=========================================================
    RpD3D9SetSurfaceProperties

    Arguments:
        matLight - lighting intensity container of the surface
        matColor - surface color
        renderFlags - rendering flags of the current context
    Purpose:
        Sets up the surface of the to-be-rendered geometry
        chunk. This function is generally used for all kinds
        of meshes.
    Binary offsets:
        (1.0 US): 0x007FC4D0
        (1.0 EU): 0x007FC510
=========================================================*/
int __cdecl RpD3D9SetSurfaceProperties( RpMaterialLighting& matLight, RwColor& matColor, unsigned int renderFlags )
{
    // Check whether any material parameter has changed.
    if ( *(float*)0x00C9A5FC == matLight.diffuse && *(float*)0x00C9A600 == matLight.ambient &&
         *(RwColor*)0x00C9A5F8 == matColor && *(unsigned int*)0x00C9A5F4 == ( renderFlags & 0x48 ) &&
         *(float*)0x00C9A5D8 == GetAmbientColor().r &&
         *(float*)0x00C9A5DC == GetAmbientColor().g &&
         *(float*)0x00C9A5E0 == GetAmbientColor().b )
    {
        return true;
    }

    // Update current material paramters.
    *(float*)0x00C9A5FC = matLight.diffuse;
    *(float*)0x00C9A600 = matLight.ambient;
    *(RwColor*)0x00C9A5F8 = matColor;
    *(unsigned int*)0x00C9A5F4 = ( renderFlags & 0x48 );
    *(float*)0x00C9A5D8 = GetAmbientColor().r;
    *(float*)0x00C9A5DC = GetAmbientColor().g;
    *(float*)0x00C9A5E0 = GetAmbientColor().b;

    // Push the new data onto the GPU.
    D3DMATERIAL9& surfMat = GetInlineSurfaceMaterial();
    
    // Color parameters that decide how data is pushed to the pipeline.
    bool useMaterialColor = false;
    bool putAmbientColor = false;
    bool putEmissiveColor = false;
    bool requireEmissiveColor = true;
    
    bool fillAmbientColorWithMaterialColor = false;
    bool useVertexColor = IS_ANY_FLAG( renderFlags, 0x08 );

    D3DMATERIALCOLORSOURCE ambientMaterialSource = D3DMCS_MATERIAL;
    D3DMATERIALCOLORSOURCE emissiveMaterialSource = D3DMCS_MATERIAL;

    // Calculate the color parameter values.
    if ( IS_ANY_FLAG( renderFlags, 0x40 ) && matColor != 0xFFFFFFFF )
    {
        // Calculate the real vehicle lighting color.
        useMaterialColor = true;

        if ( useVertexColor )
        {
            // Apply ambient color from matColor.
            fillAmbientColorWithMaterialColor = true;
            
            ambientMaterialSource = D3DMCS_COLOR1;

            putEmissiveColor = true;
        }
        else
        {
            // Default to standard pipeline.
            putAmbientColor = true;
        }
    }
    else
    {
        // We do not use the color, only brightness/intensity.
        if ( useVertexColor )
        {
            emissiveMaterialSource = D3DMCS_COLOR1;
        }
        else
        {
            requireEmissiveColor = false;
        }

        putAmbientColor = true;
    }

    // Generate diffuse color (RGBA).
    long double diffuseRed = 1.0f, diffuseGreen = 1.0f, diffuseBlue = 1.0f, diffuseAlpha = 1.0f;

    if ( useMaterialColor )
    {
        diffuseRed      *= (long double)matColor.r / 255.0f;
        diffuseGreen    *= (long double)matColor.g / 255.0f;
        diffuseBlue     *= (long double)matColor.b / 255.0f;
        diffuseAlpha    *= (long double)matColor.a / 255.0f;
    }

    // Modulate the diffuse color with its intensity.
    diffuseRed *= matLight.diffuse;
    diffuseGreen *= matLight.diffuse;
    diffuseBlue *= matLight.diffuse;

    // Now put it into the material.
    surfMat.Diffuse.r = (float)diffuseRed;  // convert to GPU precision.
    surfMat.Diffuse.g = (float)diffuseGreen;
    surfMat.Diffuse.b = (float)diffuseBlue;
    surfMat.Diffuse.a = (float)diffuseAlpha;

    // We ignore the specular color entirely.

    // Execute post-diffuse parameters.
    {
        DWORD gpuAmbientColor = 0xFFFFFFFF;

        if ( fillAmbientColorWithMaterialColor )
        {
            gpuAmbientColor = matColor.ToD3DColor();
        }

        HOOK_RwD3D9SetRenderState( D3DRS_AMBIENT, gpuAmbientColor );
        HOOK_RwD3D9SetRenderState( D3DRS_COLORVERTEX, useVertexColor );

        HOOK_RwD3D9SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, ambientMaterialSource );
        HOOK_RwD3D9SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, emissiveMaterialSource );
    }

    // Generate ambient color (only RGB).
    long double ambientRed = GetAmbientColor().r, ambientGreen = GetAmbientColor().g, ambientBlue = GetAmbientColor().b;

    if ( useMaterialColor )
    {
        ambientRed      *= (long double)matColor.r / 255.0f;
        ambientGreen    *= (long double)matColor.g / 255.0f;
        ambientBlue     *= (long double)matColor.b / 255.0f;
    }

    // Modulate the ambient color with its intensity.
    ambientRed *= matLight.ambient;
    ambientGreen *= matLight.ambient;
    ambientBlue *= matLight.ambient;

    // Decide where to put the ambient color.
    float gpuAmbientRed = (float)ambientRed;    // convert to GPU precision.
    float gpuAmbientGreen = (float)ambientGreen;
    float gpuAmbientBlue = (float)ambientBlue;

    if ( putAmbientColor )
    {
        surfMat.Ambient.r = gpuAmbientRed;
        surfMat.Ambient.g = gpuAmbientGreen;
        surfMat.Ambient.b = gpuAmbientBlue;
    }
    else
    {
        // Reset ambient color.
        surfMat.Ambient.r = 0.0f;
        surfMat.Ambient.g = 0.0f;
        surfMat.Ambient.b = 0.0f;
    }

    // Only update emissive color if it is truly required.
    if ( requireEmissiveColor )
    {
        if ( putEmissiveColor )
        {
            surfMat.Emissive.r = gpuAmbientRed;
            surfMat.Emissive.g = gpuAmbientGreen;
            surfMat.Emissive.b = gpuAmbientBlue;
        }
        else
        {
            // Reset color as we are not using it.
            surfMat.Emissive.r = 0.0f;
            surfMat.Emissive.g = 0.0f;
            surfMat.Emissive.b = 0.0f;
        }
    }

    return RwD3D9SetMaterial( surfMat );
}

void RenderWareAPI_Init( void )
{
    // Initialization based on version.
    switch( pGame->GetGameVersion() )
    {
    case VERSION_EU_10:
        _mallocNew      = (_mallocNew_t)0x008211DA;
        _freeMemGame    = (_freeMemGame_t)0x008214FD;
        break;
    case VERSION_US_10:
        _mallocNew      = (_mallocNew_t)0x0082119A;
        _freeMemGame    = (_freeMemGame_t)0x008214BD;
        break;
    }
}

void RenderWareAPI_Shutdown( void )
{
}