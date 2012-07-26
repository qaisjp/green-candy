/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CObjectSA.cpp
*  PURPOSE:     Object entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

//#define MTA_USE_BUILDINGS_AS_OBJECTS

// GTA uses this to pass to CFileLoader::LoadObjectInstance the info it wants to load
struct CFileObjectInstance
{
    float x;
    float y;
    float z;
    float rx;
    float ry;
    float rz;
    float rr; // = 1
    DWORD modelId;
    DWORD areaNumber;
    long flags; // = -1
};

CObjectSA::CObjectSA( CObjectSAInterface *obj )
{
    DEBUG_TRACE("CObjectSA::CObjectSA( CObjectSAInterface *obj )");

    m_pInterface = obj;

    // Setup some flags
    m_doNotRemoveFromGame = false;
    obj->m_unk = 6;
    BOOL_FLAG( m_entityFlags, ENTITY_DISABLESTREAMING, true );
    
    // TODO: Some old building code; figure out what to do with it
#ifdef MTA_USE_BUILDINGS_AS_OBJECTS
    DWORD dwFunc = 0x538090; // CFileLoader__LoadObjectInstance
    CFileObjectInstance fileLoader;
    MemSetFast (&fileLoader, 0, sizeof(CFileObjectInstance));
    fileLoader.modelId = dwModel;
    fileLoader.rr = 1;
    fileLoader.areaNumber = 0;
    fileLoader.flags = -1;

    _asm
    {
        push    0
        lea     ecx, fileLoader
        push    ecx
        call    dwFunc
        add     esp, 8
        mov     dwThis, eax
    }

    this->SetInterface((CEntitySAInterface*)dwThis);
    
    MemPutFast < DWORD > ( 0xBCC0E0, dwThis );
    MemPutFast < DWORD > ( 0xBCC0D8, 1 );

    dwFunc = 0x404DE0; // CIplStore__SetupRelatedIpls
    DWORD dwTemp = 0;
    char szTemp[255];
    strcpy(szTemp, "moo");

    _asm
    {
        push    0xBCC0E0
        push    -1
        lea     eax, szTemp
        push    eax
        call    dwFunc
        add     esp, 0xC
        mov     dwTemp, eax
    }

//    _asm int 3
    dwFunc = 0x5B51E0; // AddBuildingInstancesToWorld
    _asm
    {
        push    dwTemp
        call    dwFunc
        add     esp, 4
    }

    dwFunc = 0x405110; // CIplStore__RemoveRelatedIpls
    _asm
    {
        push    -1
        call    dwFunc
        add     esp, 4
    }

    // VITAL to get colmodels to appear
    // this gets the level for a colmodel (colmodel+40)
    dwFunc = 0x4107A0;
    _asm
    {
        mov     eax, dwModel
        mov     eax, 0xA9B0C8[eax*4]
        mov     eax, [eax+20]
        movzx   eax, byte ptr [eax+40]
        push    eax
        call    dwFunc
        add     esp, 4
    }
#endif

    m_poolIndex = (*ppObjectPool)->GetIndex( obj );
    mtaObjects[m_poolIndex] = this;

    m_ucAlpha = 255;

    CheckForGangTag ();
}

CObjectSA::~CObjectSA()
{
    DEBUG_TRACE("CObjectSA::~CObjectSA()");

    mtaObjects[m_poolIndex] = NULL;

    if ( m_doNotRemoveFromGame )
        return;

    CWorldSA *world = pGame->GetWorld();
    world->Remove( GetInterface() );
    world->RemoveReferencesToDeletedObject( GetInterface() );

    delete m_pInterface;

#ifdef MTA_USE_BUILDINGS_AS_OBJECTS
    DWORD dwModelID = this->internalInterface->m_nModelIndex;
    // REMOVE ref to colstore thingy
    dwFunc = 0x4107D0;
    _asm
    {
        mov     eax, dwModelID
        mov     eax, 0xA9B0C8[eax*4]
        mov     eax, [eax+20]
        movzx   eax, byte ptr [eax+40]
        push    eax
        call    dwFunc
        add     esp, 4
    }
#endif
}

void CObjectSA::Explode()
{
    DWORD dwFunc = FUNC_CObject_Explode;
    DWORD dwThis = (DWORD)GetInterface();

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CObjectSA::Break()
{
    // Works only if health is 0
    DWORD dwFunc = 0x5A0D90;
    DWORD dwThis = (DWORD)GetInterface ();

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CObjectSA::SetModelIndex( unsigned short ulModel )
{
    m_pInterface->SetModelIndex( ulModel );

    CheckForGangTag();
}

void CObjectSA::CheckForGangTag()
{
    switch( GetModelIndex() )
    {
    case 1524: case 1525: case 1526: case 1527:
    case 1528: case 1529: case 1530: case 1531:
        m_bIsAGangTag = true;
        break;
    default:
        m_bIsAGangTag = false; 
        break;
    }
}

void* CObjectSAInterface::operator new( size_t )
{
    return (*ppObjectPool)->Allocate();
}

void CObjectSAInterface::operator delete( void *ptr )
{
    (*ppObjectPool)->Free( (CObjectSAInterface*)ptr );
}