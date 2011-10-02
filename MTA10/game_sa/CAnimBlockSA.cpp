/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CAnimBlockSA.cpp
*  PURPOSE:     Animation block
*  DEVELOPERS:  Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CAnimBlendHierarchySAInterface*     CAnimBlockSAInterface::GetAnimation( unsigned int hash )
{
    unsigned int n;

    for (n=0; n<m_count; n++)
    {
        CAnimBlendHierarchySAInterface *anim = (CAnimBlendHierarchySAInterface*)ARRAY_CAnimManager_Animations + m_animationIndex + n;

        if (anim->m_hash == hash)
            return anim;
    }

    return NULL;
}

int CAnimBlockSAInterface::GetIndex ( void )
{
    return (((long)this - ARRAY_CAnimManager_AnimBlocks) / sizeof(this));
}
