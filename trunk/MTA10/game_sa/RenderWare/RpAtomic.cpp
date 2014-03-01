/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RpAtomic.cpp
*  PURPOSE:     RenderWare native function implementations
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

const RwSphere& RpAtomic::GetWorldBoundingSphere()
{
    return RpAtomicGetWorldBoundingSphere( this );
}

bool RpAtomic::IsNight()
{
    if ( pipeline == RW_ATOMIC_RENDER_NIGHT )
        return true;

    if ( pipeline == RW_ATOMIC_RENDER_REFLECTIVE )
        return false;

    return geometry->nightColor && geometry->colors != NULL;
}

void RpAtomic::AddToClump( RpClump *clump )
{
    RemoveFromClump();

    this->clump = clump;

    LIST_INSERT( clump->atomics.root, atomics );
}

void RpAtomic::RemoveFromClump()
{
    if ( !clump )
        return;

    LIST_REMOVE( atomics );

    clump = NULL;
}

void RpAtomic::SetRenderCallback( RpAtomicCallback callback )
{
    if ( !callback )
    {
        renderCallback = (RpAtomicCallback)RpAtomicRender;
    }
    else
    {
        renderCallback = callback;
    }
}

void RpAtomic::FetchMateria( RpMaterials& mats )
{
    if ( componentFlags & 0x2000 )
        return;

    for ( unsigned int n = 0; n < geometry->linkedMateria->count; n++ )
    {
        mats.Add( geometry->linkedMateria->Get(n)->material );
    }
}