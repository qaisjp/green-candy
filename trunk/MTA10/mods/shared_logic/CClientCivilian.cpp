/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientCivilian.cpp
*  PURPOSE:     Civilian ped entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include <StdInc.h>

static const luaL_Reg civilian_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_civilian( lua_State *L )
{
    CClientCivilian *cam = (CClientCivilian*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_CIVILIAN, cam );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, civilian_interface, 1 );

    lua_pushlstring( L, "civilian", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientCivilian::CClientCivilian ( CClientManager* pManager, ElementID ID, int iModel ) : CClientEntity ( ID, false, *resMan )
{
    lua_State *L = resMan->GetVM();

    // Lua boot
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_civilian, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    // Initialize members
    m_pManager = pManager;
    m_pCivilianManager = pManager->GetCivilianManager ();
    m_iModel = iModel;

    m_pCivilianPed = NULL;

    // Add this civilian to the civilian list
    m_pCivilianManager->AddToList ( this );

    // Show us by default
    Create ();
}

CClientCivilian::CClientCivilian ( CClientManager* pManager, ElementID ID, CCivilianPed * pCivilianPed ) : CClientEntity ( ID, false, *resMan )
{
    // Initialize members
    m_pManager = pManager;
    m_pCivilianManager = pManager->GetCivilianManager ();
    m_iModel = 0;

    // Store the given civilian. Also make sure we set our pointer in its stored pointer data.
    m_pCivilianPed = pCivilianPed;
    if ( m_pCivilianPed )
    {
        m_pCivilianPed->SetStoredPointer ( this );
    }

    // Add our pointer to the manager list
    m_pCivilianManager->AddToList ( this );
}


CClientCivilian::~CClientCivilian( void )
{
    // Reset the stored pointer in the civilian ped if neccessary
    if ( m_pCivilianPed )
    {
        m_pCivilianPed->SetStoredPointer ( NULL );
    }

    // Destroy the civilian
    Destroy ();

    // Remove us from the civilian list
    Unlink ();
}

void CClientCivilian::Unlink()
{
    m_pCivilianManager->RemoveFromList( this );
}

int CClientCivilian::GetRotation()
{
    int iRotation = 0;
    double dRotation;

    RwMatrix pMat;
    m_pCivilianPed->GetMatrix( pMat );

    float fX = pMat.at.fX;
    float fY = pMat.at.fY;
    dRotation = acos ( fY );

    if (fX <= 0)
    {
        iRotation = (int)(dRotation * 57.29577951);
    }
    else
    {
        iRotation = 360 - (int)(dRotation * 57.29577951);
    }

    return iRotation;
}


void CClientCivilian::GetRotation ( CVector& vecRotation ) const
{
//    vecRotation = m_vecRotation;
}


void CClientCivilian::SetRotation ( const CVector& vecRotation )
{
    if ( m_pCivilianPed )
    {
        m_pCivilianPed->SetOrientation ( vecRotation.fX, vecRotation.fY, vecRotation.fZ );
    }

//    m_vecRotation = vecRotation;
}


void CClientCivilian::SetRotation ( int iRotation )
{
    // Convert
    float fX = static_cast < float > ( iRotation ) / 57.29577951f;

    // Eventually apply it to the vehicle
    if ( m_pCivilianPed )
    {
        m_pCivilianPed->SetOrientation ( fX, 0, 0 );
    }

    // Store the rotation vector
//    m_vecRotation = CVector ( fX, 0, 0 );

    RwMatrix pMat;
    m_pCivilianPed->GetMatrix ( pMat );
    m_pCivilianPed->SetMatrix ( pMat );
}


float CClientCivilian::GetDistanceFromCentreOfMassToBaseOfModel ( void )
{
    if ( m_pCivilianPed )
    {
        return m_pCivilianPed->GetBasingDistance();
    }
   return 0.0f;;
}


bool CClientCivilian::GetMatrix( RwMatrix& Matrix ) const
{
    m_pCivilianPed->GetMatrix( Matrix );
    return true;
}


bool CClientCivilian::SetMatrix ( const RwMatrix& Matrix )
{
    m_pCivilianPed->SetMatrix ( Matrix );
    return true;
}


void CClientCivilian::GetMoveSpeed ( CVector& vecMoveSpeed ) const
{
    m_pCivilianPed->GetMoveSpeed ( vecMoveSpeed );
}


void CClientCivilian::SetMoveSpeed ( const CVector& vecMoveSpeed )
{
    m_pCivilianPed->SetMoveSpeed ( vecMoveSpeed );
}


void CClientCivilian::GetTurnSpeed ( CVector& vecTurnSpeed ) const
{
    m_pCivilianPed->GetTurnSpeed ( vecTurnSpeed );
}


void CClientCivilian::SetTurnSpeed ( const CVector& vecTurnSpeed )
{
    m_pCivilianPed->SetTurnSpeed ( vecTurnSpeed );
}


bool CClientCivilian::IsVisible ( void )
{
    return m_pCivilianPed->IsVisible ();
}


void CClientCivilian::SetVisible ( bool bVisible )
{
    m_pCivilianPed->SetVisible ( bVisible );
    m_pCivilianPed->SetUsesCollision ( bVisible );
}


float CClientCivilian::GetHealth ( void ) const
{
    return m_pCivilianPed->GetHealth ();
}


void CClientCivilian::SetHealth ( float fHealth )
{
    // Set the health
    m_pCivilianPed->SetHealth ( fHealth );
}

int CClientCivilian::GetModelID ( void )
{
    //return m_iVehicleModel;
    return 0;
}


void CClientCivilian::SetModelID ( int iModelID )
{

}


void CClientCivilian::Dump ( FILE* pFile, bool bDumpDetails, unsigned int uiIndex )
{

}

void CClientCivilian::Create ( void )
{

}

void CClientCivilian::Destroy ( void )
{
 
}


void CClientCivilian::ReCreate ( void )
{
    // Re-create the vehicle
    Destroy ();
    Create ();
}
