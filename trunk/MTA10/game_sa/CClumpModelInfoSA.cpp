/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CClumpModelInfoSA.cpp
*  PURPOSE:     Clump model instance
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

void CClumpModelInfoSAInterface::Init()
{
    m_rwClump = NULL;

    m_animBlock = -1;
}

void CClumpModelInfoSAInterface::DeleteRwObject()
{
    RpAtomic *atomic;

    if ( !m_rwClump )
        return;

    atomic = m_rwClump->GetFirstAtomic();

    if ( atomic )
    {
        // No idea what this is, please invest time
        // 004C4E83
    }

    RpClumpDestroy( m_rwClump );

    DeleteTextures();

    if ( m_collFlags & COLL_SWAYINWIND )
        DeleteCollision();
}

eRwType CClumpModelInfoSAInterface::GetRwModelType()
{
    return RW_CLUMP;
}

RpClump* CClumpModelInfoSAInterface::CreateRwObjectEx( int rwTag )
{
    return CreateRwObject();
}

bool RpSetAtomicAnimHierarchy( RpAtomic *child, RpAnimHierarchy *anim )
{
    child->m_anim = anim;
    return false;
}

RpClump* CClumpModelInfoSAInterface::CreateRwObject()
{
    RpClump *clump;
    RpAtomic *atomic;

    if ( !m_rwClump )
        return NULL;

    Reference();

    clump = RpClumpClone( m_rwClump );

    atomic = RpClumpGetLastAtomic( clump );

    if ( atomic && !( m_collFlags & COLL_NOSKELETON ) )
    {
        if ( atomic->m_geometry->m_skeleton )
        {
            RpAnimHierarchy *anim = clump->GetAnimHierarchy();

            clump->ForAllAtomics( RpSetAtomicAnimHierarchy, anim );

            // Set up the animation
            RwAnimationInit( anim->m_anim, pGame->GetAnimManager()->CreateAnimation( anim ) );

            anim->m_flags = 0x0300;
        }
    }

    if ( m_collFlags & COLL_STATIC )
    {
        CAnimBlendHierarchySAInterface *anim;

        // Cache the animation and skeleton
        clump->InitStaticSkeleton();

        // Set idle animation
        if ( anim = pGame->GetAnimManager()->GetAnimBlock( m_animBlock )->GetAnimation( m_hash ) )
            pGame->GetAnimManager()->BlendAnimation( m_rwClump, anim, 2, 1.0 );
    }

    Dereference();
    return clump;
}

void CClumpModelInfoSAInterface::SetAnimFile( const char *name )
{
    char *anim;

    if ( strcmp(name, "null") == 0 )
        return;

    anim = malloc( strlen( name ) + 1 );

    strcpy(anim, name);

    // this is one nasty hack
    m_animBlock = (int)anim;
}

void CClumpModelInfoSAInterface::ConvertAnimFileIndex()
{
    int animBlock;

    if ( m_animBlock == -1 )
        return;

    animBlock = pGame->GetAnimManager()->GetAnimBlockIndex( (const char*)m_animBlock );

    free( (void*)m_animBlock );

    // Yeah, weird
    m_animBlock = animBlock;
}

int CClumpModelInfoSAInterface::GetAnimFileIndex()
{
    return m_animBlock;
}

CColModelSAInterface* CClumpModelInfoSAInterface::GetCollision()
{
    return m_pColModel;
}

bool RwAtomicSetupAnimHierarchy( RpAtomic *child, void *data )
{
    child->m_anim = child->m_parent->GetAnimHierarchy();
    return true;
}

void CClumpModelInfoSAInterface::SetClump( RpClump *clump )
{
    RpAtomic *effAtomic = m_rwClump->Find2dfx();
    RpAtomic *atomic = clump->GetFirstAtomic();
    RpAnimHierarchy *hier;
    RpSkeleton *skel;
    unsigned short anim;
    unsigned int n;

    // Decrease effect count
    if ( effAtomic )
        m_num2dfx -= effAtomic->m_2dfx->m_count;

    m_rwClump = clump;

    if ( clump )
    {
        effAtomic = clump->Find2dfx();

        if ( effAtomic )
            m_num2dfx += effAtomic->m_2dfx->m_count;
    }

    // Set some callbacks
    RpClumpSetupFrameCallback( clump, (int)this );

    CTxdStore_AddRef( m_textureDictionary );

    anim = GetAnimFileIndex();

    if ( anim )
        pGame->GetAnimManager()->AddAnimBlockRef( anim );

    clump->SetupAtomicRender();

    if ( !atomic || !atomic->m_geometry )
        return;

    if ( m_collFlags & COLL_NOSKELETON )
    {
        clump->ForAllAtomics( RwAtomicSetupAnimHierarchy, 0 );
        return;
    }

    atomic->m_geometry->m_dimension->m_scale *= 1.2;

    // Get the animation
    hier = clump->GetAnimHierarchy();

    clump->ForAllAtomics( RpSetAtomicAnimHierarchy, hier );

    skel = atomic->m_geometry->m_skeleton;

    for (n=0; n<atomic->m_geometry->m_verticeSize; n++)
    {
        RwV4d *info = skel->m_vertexInfo[n];
        float sum = (*info)[0] + (*info)[1] + (*info)[2] + (*info)[3];

        //sum /= 1.0;

        (*info)[0] *= sum;
        (*info)[1] *= sum;
        (*info)[2] *= sum;
        (*info)[3] *= sum;
    }

    // Set flag
    hier->m_flags = 0x0300;
}

void CClumpModelInfoSAInterface::AssignAtomics( CAtomicHierarchySAInterface *atomics )
{
    for (atomics; atomics->m_name; atomics++)
    {
        RwFrame *component;

        if ( !(atomics->m_flags & ATOMIC_HIER_ACTIVE) )
            continue;

        component = m_rwClump->m_parent->FindFreeChildByName( atomics->m_name );

        if ( !component )
            continue;

        component->m_hierarchyId = atomics->m_frameHierarchy;
    }
}