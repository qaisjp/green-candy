/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.cpp
*  PURPOSE:     Base entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Kent Simon <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CGameSA *pGame;
extern CBaseModelInfoSAInterface **ppModelInfo;

CPlaceableSAInterface::CPlaceableSAInterface()
{
    m_heading = 0;
    m_matrix = NULL;
}

CPlaceableSAInterface::~CPlaceableSAInterface()
{
    FreeMatrix();

    (*(unsigned int*)0x00B74238)--;

    //m_matrix = &transformIdentity;
}

void CPlaceableSAInterface::AllocateMatrix()
{
    if ( m_matrix )
    {
        // We already have a matrix, make sure its in the active list
        LIST_REMOVE( *m_matrix );
        LIST_APPEND( pTransform->m_activeList, *m_matrix );
        return;
    }

    // Extend the matrix list
    if ( !pTransform->IsFreeMatrixAvailable() && !pTransform->FreeUnimportantMatrix() )
        pTransform->NewMatrix();

    // Allocate it
    m_matrix = pTransform->Allocate();
    m_matrix->m_entity = this;
}

void CPlaceableSAInterface::AcquaintMatrix()
{
    if ( m_matrix )
        return;

    // Extend the matrix list
    if ( !pTransform->IsFreeMatrixAvailable() && !pTransform->FreeUnimportantMatrix() )
        pTransform->NewMatrix();

    // Allocate it
    m_matrix = pTransform->Allocate();
    m_matrix->m_entity = this;
}

void CPlaceableSAInterface::RestoreMatrix()
{
    // Optimized
    m_matrix->pos = m_position;
    m_matrix->FromHeading( m_heading );
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

    LIST_REMOVE( *trans );
    LIST_APPEND( pTransform->m_freeList, *trans );
}

void Placeable_Init()
{
    HookInstall( 0x0054F560, h_memFunc( &CPlaceableSAInterface::AcquaintMatrix ), 5 );
    HookInstall( 0x0054F4C0, h_memFunc( &CPlaceableSAInterface::AllocateMatrix ), 5 );
    HookInstall( 0x0054F3B0, h_memFunc( &CPlaceableSAInterface::FreeMatrix ), 5 );
}

void Placeable_Shutdown()
{

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

    m_randomSeed = rand();

    m_references = NULL;
    m_lastRenderedLink = NULL;
    
    m_numLOD = 0;
    m_numRenderedLOD = 0;
    m_lod = NULL;
}

CEntitySAInterface::~CEntitySAInterface()
{
}

void CEntitySAInterface::SetModelIndex( unsigned short id )
{
    // TODO
}

void CEntitySAInterface::GetPosition( CVector& pos ) const
{
    if ( !m_matrix )
        pos = m_position;
    else
        pos = m_matrix->pos;
}

static bool RpMaterialSetAlpha( RpMaterial *mat, unsigned char alpha )
{
    mat->m_color.a = alpha;
    return true;
}

static bool RpAtomicMaterialSetAlpha( RpAtomic *atom, unsigned char alpha )
{
    atom->m_geometry->ForAllMateria( RpMaterialSetAlpha, alpha );
    return true;
}

void CEntitySAInterface::SetAlpha( unsigned char alpha )
{
    if ( !m_rwObject )
        return;

    if ( m_rwObject->m_type == RW_ATOMIC )
    {
        RpAtomicMaterialSetAlpha( (RpAtomic*)m_rwObject, alpha );
    }
    else if ( m_rwObject->m_type == RW_CLUMP )
    {
        ((RpClump*)m_rwObject)->ForAllAtomics( RpAtomicMaterialSetAlpha, alpha );
    }
}

CColModelSAInterface* CEntitySAInterface::GetColModel() const
{
    CEntitySA *entity = (CEntitySA*)pGame->GetPools()->GetEntity( const_cast <CEntitySAInterface*> ( this ) );

    if ( entity )
    {
        CColModelSA *col = entity->GetColModel();

        if ( col )
            return col->GetInterface();
    }

    if ( m_type == ENTITY_TYPE_VEHICLE )
    {
        CVehicleSAInterface *veh = (CVehicleSAInterface*)this;
        unsigned char n = veh->m_specialColModel;

        if ( n != 0xFF )
            return (CColModelSAInterface*)VAR_CVehicle_SpecialColModels + n;
    }

    return ppModelInfo[m_model]->m_pColModel;
}

bool CEntitySAInterface::IsOnScreen() const
{
    CColModelSAInterface *col = GetColModel();
    CVector pos;

    DWORD dwFunc = 0x005334F0;

    __asm
    {
        mov eax,col
        add eax,24
        push eax
        lea eax,pos
        push eax
        mov ecx,this
        call dwFunc
    }

    if ( pGame->GetCamera()->GetInterface()->IsSphereVisible( pos, col->m_bounds.fRadius, (void*)0x00B6FA74 ) )
        return true;

    if ( *(unsigned char*)0x00B6F998 )
        return pGame->GetCamera()->GetInterface()->IsSphereVisible( pos, col->m_bounds.fRadius, (void*)0x00B6FABC );

    return false;
}

void Entity_Init()
{
    HookInstall( 0x00535300, h_memFunc( &CEntitySAInterface::GetColModel ), 5 );
    HookInstall( 0x00534540, h_memFunc( &CEntitySAInterface::IsOnScreen ), 5 );
}

void Entity_Shutdown()
{

}

CEntitySA::CEntitySA()
{
    // Set these variables to a constant state
    m_pInterface = NULL;
    m_doNotRemoveFromGame = false;
    m_pStoredPointer = NULL;
    m_colModel = NULL;
}

CEntitySA::~CEntitySA()
{
    // Unlink disabled-collisions
    while ( !m_disabledColl.empty() )
        SetCollidableWith( pGame->GetPools()->GetEntity( m_disabledColl.begin()->first ), true );

    if ( m_doNotRemoveFromGame )
        return;

    CWorldSA *world = pGame->GetWorld();
    world->Remove( m_pInterface );
    world->RemoveReferencesToDeletedObject( m_pInterface );

    // Clever little trick, eh
    delete m_pInterface;

    OnInterfaceDestruction();
}

void CEntitySA::SetPosition( float x, float y, float z )
{
    DEBUG_TRACE("void CEntitySA::SetPosition( float x, float y, float z )");

    CVector *vecPos;

    if ( m_pInterface->m_matrix )
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

void CEntitySA::SetPosition( const CVector& pos )
{
    SetPosition( pos.fX, pos.fY, pos.fZ );
}

void CEntitySA::GetPosition( CVector& pos ) const
{
    DEBUG_TRACE("void CEntitySA::GetPosition( CVector& pos ) const");

    if ( !m_pInterface->m_matrix )
        m_pInterface->AllocateMatrix();

    pos = m_pInterface->m_matrix->pos;
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

        push    z
        push    y
        push    x
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
    m_pInterface->m_status = bStatus;
}

eEntityStatus CEntitySA::GetEntityStatus() const
{
    DEBUG_TRACE("eEntityStatus CEntitySA::GetEntityStatus() const");
    return (eEntityStatus) m_pInterface->m_status;
}

void CEntitySA::SetAlpha( unsigned char alpha )
{
    DEBUG_TRACE("void CEntitySA::SetAlpha( unsigned char alpha )");

    DWORD dwFunc = FUNC_SetRwObjectAlpha;
    DWORD dwThis = (DWORD) m_pInterface;
    _asm
    {
        mov     ecx, dwThis
        push    alpha
        call    dwFunc
    }
}

bool CEntitySA::IsOnScreen() const
{
    return GetInterface()->IsOnScreen();
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

bool CEntitySA::IsCollidableWith( CEntity *entity ) const
{
    return !MapContains( m_disabledColl, dynamic_cast <CEntitySA*> ( entity )->GetInterface() );
}

void CEntitySA::SetCollidableWith( CEntity *entity, bool enabled )
{
    CEntitySA *intEnt = dynamic_cast <CEntitySA*> ( entity );

    if ( intEnt == this )
        return;

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