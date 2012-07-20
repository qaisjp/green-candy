/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.cpp
*  PURPOSE:     Base entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Kent Simon <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CGameSA *pGame;

CPlaceableSAInterface::CPlaceableSAInterface()
{
    m_heading = 0;
    m_matrix = NULL;
}

void CPlaceableSAInterface::AllocateMatrix()
{
    CTransformSAInterface *trans;

    if ( m_matrix )
    {
        // We already have a matrix, make sure its in the active list
        LIST_REMOVE( m_matrix );
        LIST_APPEND( pTransformation->m_activeList, m_matrix );
        return;
    }

    // Extend the matrix list
    if ( !pTransformation->IsFreeMatrixAvailable() && !pTransformation->FreeUnimportantMatrix() )
        pTransformation->NewMatrix();

    // Allocate it
    trans = pTransformation->Allocate();

    trans->m_entity = this;
    m_matrix = trans;
}

void CPlaceableSAInterface::FreeMatrix()
{
    CTransformSAInterface *trans = m_matrix;

    // Transform the entity
    m_position = CVector( m_matrix->pos.fX, m_matrix->pos.fY, m_matrix->pos.fZ );
    m_heading = atan( m_matrix->up.fY ) / -m_matrix->up.fX;

    // Free the matrix
    m_matrix = NULL;
    trans->m_entity = NULL;

    LIST_REMOVE( m_matrix );
    LIST_APPEND( pTransformation->m_freeList, trans );
}

CEntitySAInterface::CEntitySAInterface()
{
    m_status = 4;
    m_entityFlags = ENTITY_VISIBLE | ENTITY_BACKFACECULL;

    m_scanCode = 0;

    m_model = 0xFFFF;
    m_rwObject = NULL;

    m_iplIndex = 0;
    m_areaCode = 0;

    m_randomSeek = rand();

    m_references = NULL;
    m_lastRenderedLink = NULL;
    
    m_numLOD = 0;
    m_numRenderedLOD = 0;
    m_lod = NULL;
}

CEntitySA::CEntitySA()
{
    // Set these variables to a constant state
    m_pInterface = NULL;
    internalID = 0;
    BeingDeleted = false;
    DoNotRemoveFromGame = false;
    m_pStoredPointer = NULL;
    m_ulArrayID = 0;
}

CEntitySA::~CEntitySA()
{
    // Unlink disabled-collisions
    while ( !m_disabledColl.empty() )
        SetCollidableWith( m_disabledColl.begin()->first, true );
}

void CEntitySA::SetPosition( float x, float y, float z )
{
    DEBUG_TRACE("void CEntitySA::SetPosition( float x, float y, float z )");

    CVector *vecPos;

    if ( m_pInterface->Placeable.matrix )
        vecPos = &m_pInterface->m_matrix->pos;
    else
        vecPos = &m_pInterface->m_position;

    if ( vecPos )
    {
        vecPos->fX = x;
        vecPos->fY = y;
        vecPos->fZ = z;
    }

    unsigned short usModelID = GetModelIndex();

    switch( usModelID )
    {
    case 537:
    case 538:
    case 569:
    case 570:
    case 590:
    case 449:
        {
            // If it's a train, recalculate its rail position parameter (does not affect derailed state)
            DWORD dwThis = (DWORD)m_pInterface;
            DWORD dwFunc = FUNC_CVehicle_RecalcOnRailDistance;
            _asm
            {
                mov     ecx, dwThis
                call    dwFunc
            }
        }
    }
}

void CEntitySA::Teleport( float x, float y, float z )
{
    DEBUG_TRACE("void CEntitySA::Teleport( float x, float y, float z )");

    if ( !m_pInterface->m_matrix )
        m_pInterface->AllocateMatrix();

    m_pInterface->Teleport( x, y, z, true );
}

void CEntitySA::ProcessControl()
{
    DEBUG_TRACE("void CEntitySA::ProcessControl()");
    
    m_pInterface->ProcessControl();
}

void CEntitySA::SetupLighting()
{
    DEBUG_TRACE("void CEntitySA::SetupLighting()");
    
    m_pInterface->SetupLighting();
}

void CEntitySA::Render()
{
    DEBUG_TRACE("void CEntitySA::Render()");

    m_pInterface->Render();

/*  DWORD dwFunc = 0x553260;
    DWORD dwThis = (DWORD) m_pInterface;

    _asm
    {
        push    dwThis
        call    dwFunc
        add     esp, 4
    }*/
}

void CEntitySA::SetOrientation( float x, float y, float z )
{
    DEBUG_TRACE("void CEntitySA::SetOrientation( float x, float y, float z )");

    pGame->GetWorld()->Remove( this );

    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_SetOrientation;
    _asm
    {
        // ChrML: I've switched the X and Z at this level because that's how the real rotation
        //        is. GTA has kinda swapped them in this function.

        push    fZ
        push    fY
        push    fX
        mov     ecx, dwThis
        call    dwFunc
    }

    dwFunc = 0x446F90;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    dwFunc = 0x532B00;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    pGame->GetWorld()->Add( this );
}

void CEntitySA::FixBoatOrientation()
{
    DEBUG_TRACE("void CEntitySA::FixBoatOrientation()");
    pGame->GetWorld()->Remove ( this );
    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = 0x446F90;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    dwFunc = 0x532B00;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    pGame->GetWorld()->Add ( this );
}

void CEntitySA::SetPosition( const CVector& pos )
{
    DEBUG_TRACE("void CEntitySA::SetPosition( const CVector& pos )");
/*  FLOAT fX = vecPosition->fX;
    FLOAT fY = vecPosition->fY;
    FLOAT fZ = vecPosition->fZ;
    DWORD dwFunc = 0x5A17B0;
    DWORD dwThis = (DWORD) m_pInterface;
    _asm
    {
        mov     ecx, dwThis
        push 0
        push fZ
        push fY
        push fX
        call    dwFunc
    }*/

    SetPosition( pos.fX, pos.fY, pos.fZ );
}

void CEntitySA::GetPosition( CVector& pos ) const
{
    DEBUG_TRACE("void CEntitySA::GetPosition( CVector& pos ) const");

    if ( !m_pInterface->m_matrix )
        m_pInterface->AllocateMatrix();

    pos = m_pInterface->m_matrix->pos;
}

void CEntitySA::GetMatrix( RwMatrix& mat ) const
{
    DEBUG_TRACE("void CEntitySA::GetMatrix( RwMatrix& mat ) const");

    if ( !m_pInterface->m_matrix )
        m_pInterface->AllocateMatrix(); // We allocate a matrix for us
    
    mat = *m_pInterface->m_matrix;
}

void CEntitySA::SetMatrix( const RwMatrix& mat )
{
    DEBUG_TRACE("void CEntitySA::SetMatrix( const RwMatrix& mat )");

    if ( !m_pInterface->m_matrix )
        m_pInterface->AllocateMatrix(); // We need a matrix nao

    *m_pInterface->m_matrix = mat;
    m_pInterface->m_position = mat.pos;

    /*
    WORD wModelID = GetModelIndex();
    if ( wModelID == 537 || wModelID == 538 || wModelID == 569 || wModelID == 570 || wModelID == 590 || wModelID == 449 )
    {
        DWORD dwThis = (DWORD) m_pInterface;
        DWORD dwFunc = 0x6F6CC0;
        _asm
        {
            mov     ecx, dwThis
            call    dwFunc
        }
        
        //OutputDebugString ( "Set train position on tracks (matrix)!\n" );
    }
    */

    pGame->GetWorld()->Remove( this );

    DWORD dwThis = (DWORD)m_pInterface;
    DWORD dwFunc = 0x446F90;    // CEntity::UpdateRwMatrix
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    dwFunc = 0x532B00;          // CEntity::UpdateRwFrame
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    pGame->GetWorld()->Add( this );
}

unsigned short CEntitySA::GetModelIndex() const
{
    DEBUG_TRACE("unsigned short CEntitySA::GetModelIndex() const");
    return m_pInterface->m_model;
}

eEntityType CEntitySA::GetEntityType() const
{
    DEBUG_TRACE("eEntityType CEntitySA::GetEntityType() const");
    return (eEntityType)m_pInterface->m_type;
}

float CEntitySA::GetBasingDistance() const
{
    DEBUG_TRACE("float CEntitySA::GetBasingDistance() const");
    DWORD dwFunc = FUNC_GetDistanceFromCentreOfMassToBaseOfModel;
    DWORD dwThis = (DWORD) m_pInterface;
    FLOAT fReturn;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        fstp    fReturn
    }
    return fReturn;
}   

void CEntitySA::SetEntityStatus( eEntityStatus bStatus )
{
    DEBUG_TRACE("void CEntitySA::SetEntityStatus( eEntityStatus bStatus )");
    m_pInterface->nStatus = bStatus;
}

eEntityStatus CEntitySA::GetEntityStatus() const
{
    DEBUG_TRACE("eEntityStatus CEntitySA::GetEntityStatus() const");
    return (eEntityStatus) m_pInterface->nStatus;
}

RwFrame* CEntitySA::GetFrameFromId( int id ) const
{
    DWORD dwClump = (DWORD)m_pInterface->m_pRwObject;
    DWORD dwReturn;
    _asm
    {
        push    id
        push    dwClump
        call    FUNC_CClumpModelInfo__GetFrameFromId
        add     esp, 8
        mov     dwReturn, eax
    }
    return (RwFrame*)dwReturn;
}

RwMatrix* CEntitySA::GetLTMFromId( int id ) const
{
    return GetFrameFromId( id )->m_ltm;
}

void CEntitySA::SetAlpha( unsigned char alpha )
{
    DEBUG_TRACE("void CEntitySA::SetAlpha( unsigned char alpha )");

    DWORD dwFunc = FUNC_SetRwObjectAlpha;
    DWORD dwThis = (DWORD) m_pInterface;
    _asm
    {
        mov     ecx, dwThis
        push    dwAlpha
        call    dwFunc
    }
}

bool CEntitySA::IsOnScreen() const
{
    /**(BYTE *)0x534540 = 0x83;
    MemPut < BYTE > ( 0x534541, 0xEC );
    MemPut < BYTE > ( 0x534542, 0x10 );
*/
    DWORD dwFunc = FUNC_IsVisible; //FUNC_IsOnScreen;
    DWORD dwThis = (DWORD) m_pInterface;
    bool bReturn = false;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }
/*
    MemPut < BYTE > ( 0x534540, 0xB0 );
    MemPut < BYTE > ( 0x534541, 0x01 );
    MemPut < BYTE > ( 0x534542, 0xC3 );
*/
    return bReturn;
}

void CEntitySA::MatrixConvertFromEulerAngles( float x, float y, float z, int unk )
{
    if ( !m_pInterface->m_matrix )
        m_pInterface->AllocateMatrix();

    DWORD matint = (DWORD)m_pInterface->m_matrix;

    DWORD dwFunc = FUNC_CMatrix__ConvertFromEulerAngles;
    _asm
    {
        push    unk
        push    z
        push    y
        push    x
        mov     ecx,matint
        call    dwFunc
    }
}

void CEntitySA::MatrixConvertToEulerAngles( float& x, float& y, float& z, int unk ) const
{
    if ( !m_pInterface->m_matrix )
    {
        x = 0;
        y = 0;
        z = 0;
        return;
    }

    DWORD matint = (DWORD)m_pInterface->m_matrix;

    DWORD dwFunc = FUNC_CMatrix__ConvertToEulerAngles;
    _asm
    {
        push    unk
        push    z
        push    y
        push    x
        mov     ecx,matint
        call    dwFunc
    }
}

bool CEntitySA::IsPlayingAnimation( const char *name ) const
{
    DWORD dwReturn = 0;
    DWORD dwFunc = FUNC_RpAnimBlendClumpGetAssociation;
    DWORD dwThis = (DWORD)m_pInterface->m_pRwObject;

    _asm
    {
        push    name
        push    dwThis
        call    dwFunc
        add     esp, 8
        mov     dwReturn, eax
    }
    return dwReturn != 0;
}

bool CEntitySA::IsCollidableWith( CEntity *entity ) const
{
    return !MapContains( m_disabledColl, dynamic_cast <CEntitySA*> ( entity )->GetInterface() );
}

void CEntitySA::SetCollidableWith( CEntity *entity, bool enabled )
{
    CEntitySA *intEnt = dynamic_cast <CEntitySA*> ( entity );

    // quit if no change
    if ( MapContains( m_disabledColl, intEnt->GetInterface() ) != enabled )
        return;

    if ( enabled )
    {
        MapRemove( m_disabledColl, intEnt->GetInterface() );

        if ( m_disabledColl.empty() )
            MapRemove( pGame->m_disabledColl, this );
    }
    else
    {
        MapSet( m_disabledColl, intEnt->GetInterface(), true );
        MapSet( pGame->m_disabledColl, this, true );
    }

    // Set in the other entity as well
    intEnt->SetCollidableWith( this, enabled );
}

unsigned char CEntitySA::GetAreaCode() const
{
    return m_pInterface->m_areaCode;
}

void CEntitySA::SetAreaCode( unsigned char area )
{
    m_pInterface->m_areaCode = area;
}

void CEntitySA::SetUnderwater( bool bUnderwater )
{
    m_pInterface->bUnderwater = bUnderwater;
}

bool CEntitySA::GetUnderwater() const
{
    return m_pInterface->bUnderwater;
}