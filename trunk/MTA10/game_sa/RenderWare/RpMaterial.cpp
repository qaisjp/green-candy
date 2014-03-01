/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RpMaterial.cpp
*  PURPOSE:     RenderWare native function implementations
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

void RpMaterial::SetTexture( RwTexture *tex )
{
    // Reference our texture for usage
    if ( tex )
        tex->refs++;

    // Dereference the previous texture
    if ( this->texture )
        RwTextureDestroy( this->texture );

    // Assign the new texture
    this->texture = tex;
}

RpMaterials::RpMaterials( unsigned int max )
{
    data = (RpMaterial**)pRwInterface->m_memory.m_malloc( sizeof(long) * max, 0 );

    max = max;
    entries = 0;
}

RpMaterials::~RpMaterials()
{
    if ( data )
    {
        for ( unsigned int n = 0; n < entries; n++ )
            RpMaterialDestroy( data[n] );

        pRwInterface->m_memory.m_free( data );

        data = NULL;
    }

    entries = 0;
    max = 0;
}

bool RpMaterials::Add( RpMaterial *mat )
{
    if ( entries == max )
        return false;   // We do not need RW error handlers anymore?

    // Reference it
    mat->refs++;

    data[ entries++ ] = mat;
    return true;
}

RwLinkedMaterial* RwLinkedMateria::Get( unsigned int index )
{
    if ( index >= count )
        return NULL;

    return (RwLinkedMaterial*)( this + 1 ) + index;
}