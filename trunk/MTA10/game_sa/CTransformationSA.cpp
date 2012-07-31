/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTransformationSA.cpp
*  PURPOSE:     Transformation matrix management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CTransformationSAInterface *pTransform = (CTransformationSAInterface*)CLASS_CTransformation;

void Transformation_Init()
{
    // Block the original transformation init
    *(unsigned char*)FUNC_InitTransformation = 0xC3;

    // Allocate enough matrices for everybody
    new (pTransform) CTransformationSAInterface( 65536 );
}

CTransformationSAInterface::CTransformationSAInterface( unsigned int max )
{
    CAllocatedTransformSAInterface *trans = (CAllocatedTransformSAInterface*)malloc( max * sizeof(CTransformSAInterface) + sizeof(CAllocatedTransformSAInterface) );

    // Store the count
    trans->m_count = max;

    m_stack = trans->Get( 0 );

    memset( m_stack + 1, 0, max * sizeof(CTransformSAInterface) );

    // Init the lists
    LIST_CLEAR( m_usedList );
    LIST_CLEAR( m_freeList );
    LIST_CLEAR( m_activeList );

    LIST_APPEND( m_usedList, m_usedItem );
    LIST_APPEND( m_freeList, m_freeItem );
    LIST_APPEND( m_activeList, m_activeItem );

    while ( max-- )
        LIST_APPEND( m_freeList, *( m_stack + max ) );
}

CTransformSAInterface* CTransformationSAInterface::Allocate()
{
    CTransformSAInterface *matrix = m_freeList.prev;

    if ( matrix == &m_freeItem )
        return NULL;

    LIST_REMOVE( *matrix );
    LIST_APPEND( m_activeList, *matrix );
    return matrix;
}

bool CTransformationSAInterface::IsFreeMatrixAvailable()
{
    return m_freeList.prev != &m_freeItem;
}

bool CTransformationSAInterface::FreeUnimportantMatrix()
{
    // Crashfix
    if ( m_usedList.prev == &m_usedItem )
        return false;

    // Steal unimportant matrices :3
    m_usedList.next->m_entity->FreeMatrix();
    return true;
}

void CTransformationSAInterface::NewMatrix()
{
    CTransformSAInterface *item = new CTransformSAInterface;

    LIST_APPEND( m_freeList, *item );
}