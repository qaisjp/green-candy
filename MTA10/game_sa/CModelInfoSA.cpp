/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelInfoSA.cpp
*  PURPOSE:     Entity model information handler
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CGameSA * pGame;

CBaseModelInfoSAInterface** ppModelInfo = (CBaseModelInfoSAInterface**) ARRAY_ModelInfo;

std::map < unsigned short, int > CModelInfoSA::ms_RestreamTxdIDMap;


CBaseModelInfoSAInterface::CBaseModelInfoSAInterface()
{

}

CBaseModelInfoSAInterface::~CBaseModelInfoSAInterface()
{

}

CAtomicModelInfoSA* CBaseModelInfoSAInterface::GetAtomicModelInfo()
{
    return NULL;
}

CDamageAtomicModelInfoSA* CBaseModelInfoSAInterface::GetDamageAtomicModelInfo()
{
    return NULL;
}

CLODAtomicModelInfoSA* CBaseModelInfoSAInterface::GetLODAtomicModelInfo()
{
    return NULL;
}

unsigned int CBaseModelInfoSAInterface::GetTimeInfo()
{
    return 0;
}

void CClumpModelInfoSAInterface::Init()
{
    m_numberOfRefs = 0;
    m_textureDictionary = -1;
    m_pColModel = NULL;
    m_effectID = -1;
    m_numberOf2DEffects = 0;
    m_dynamicIndex = -1;
    m_lodDistance = 2000;
    m_rwClump = NULL;

    m_renderFlags = RENDER_COLMODEL | RENDER_BACKFACECULL;
}

void CClumpModelInfoSAInterface::Shutdown()
{
    DeleteRwObject();

    DeleteCollision();

    m_renderFlags |= RENDER_COLMODEL;

    m_effectID = -1;
    m_numberOf2DEffects = 0;

    m_dynamicIndex = -1;
    m_textureDictionary = -1;
}

void CClumpModelInfoSAInterface::DeleteCollision()
{
    if (m_pColModel && ( m_renderFlags & RENDER_COLMODEL ))
    {
        delete m_pColModel;

        m_pColModel = NULL;
    }
}

void CBaseModelInfoSAInterface::DeleteTextures()
{
    if ( m_textureDictionary == -1)
        return;

    CTxdStore_RemoveRef( m_textureDictionary );

    m_textureDictionary = -1;

    if ( GetAnimFileIndex() != -1 )
        pGame->GetAnimManager()->RemoveAnimBlockRef( GetAnimFileIndex() );

    pGame->GetAnimManager()->RemoveAnimBlockRef( GetAnimFileIndex() );
}

void CBaseModelInfoSAInterface::Reference()
{
    m_numberOfRefs++;

    CTxdStore_AddRef( m_textureDictionary );
}

void CBaseModelInfoSAInterface::Dereference()
{
    m_numberOfRefs--;

    CTxdStore_RemoveRef( m_textureDictionary );
}

void CClumpModelInfoSAInterface::DeleteRwObject()
{
    RpAtomic *atomic;

    if ( !m_rwClump )
        return;

    atomic = RpClumpGetLastAtomic( m_rwClump );

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

RpClump* CClumpModelInfoSAInterface::CreateRwObject()
{
    RpClump *clump;
    RpAtomic *atomic;

    if ( !m_rwClump )
        return NULL;

    Reference();

    clump = RpClumpClone( m_rwClump );

    atomic = RpClumpGetLastAtomic( clump );

    if ( atomic )
    {

    }
}

CModelInfoSA::CModelInfoSA ( void )
{
    m_pInterface = NULL;
    m_modelID = 0xFFFF;
    m_dwReferences = 0;
    m_pOriginalColModelInterface = NULL;
    m_pCustomClump = NULL;
    m_pCustomColModel = NULL;
}


CModelInfoSA::CModelInfoSA ( unsigned short id )
{    
    m_modelID = id;
    m_pInterface = ppModelInfo [ id ];
    m_dwReferences = 0;
    m_pOriginalColModelInterface = NULL;
    m_pCustomClump = NULL;
    m_pCustomColModel = NULL;
}


CBaseModelInfoSAInterface * CModelInfoSA::GetInterface ( void )
{
    return m_pInterface = ppModelInfo [ m_modelID ];
}


bool CModelInfoSA::IsBoat ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_BOAT;
}

bool CModelInfoSA::IsCar ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_CAR;
}

bool CModelInfoSA::IsTrain ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_TRAIN;
}   

bool CModelInfoSA::IsHeli ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_HELI;
}   

bool CModelInfoSA::IsPlane ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_PLANE;
}   

bool CModelInfoSA::IsBike ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_BIKE;
}

bool CModelInfoSA::IsFakePlane ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_FAKEPLANE;
}   

bool CModelInfoSA::IsMonsterTruck ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_MONSTERTRUCK;
}

bool CModelInfoSA::IsQuadBike ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_QUADBIKE;
}   

bool CModelInfoSA::IsBmx ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_BMX;
}   

bool CModelInfoSA::IsTrailer ( )
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_TRAILER;
}   

bool CModelInfoSA::IsVehicle ( )
{
    return m_pInterface->GetModelType() == MODEL_VEHICLE;
}   

bool CModelInfoSA::IsUpgrade ( void )
{
    return m_modelID >= 1000 && m_modelID <= 1193;
}


char * CModelInfoSA::GetNameIfVehicle ( )
{
    DEBUG_TRACE("char * CModelInfoSA::GetNameIfVehicle ( )");
//  if(this->IsVehicle())
//  {
        DWORD dwModelInfo = ARRAY_ModelInfo;
        DWORD dwFunc = FUNC_CText_Get;
        DWORD ModelID = m_modelID;
        DWORD dwReturn = 0;

        _asm
        {       
            push    eax
            push    ebx
            push    ecx

            mov     ebx, ModelID
            lea     ebx, [ebx*4]
            add     ebx, ARRAY_ModelInfo
            mov     eax, [ebx]
            add     eax, 50

            push    eax
            mov     ecx, CLASS_CText
            call    dwFunc

            mov     dwReturn, eax

            pop     ecx
            pop     ebx
            pop     eax
        }
        return (char *)dwReturn;
//  }
//  return NULL;
}

VOID CModelInfoSA::Request( bool bAndLoad, bool bWaitForLoad, bool bHighPriority )
{
    DEBUG_TRACE("VOID CModelInfoSA::Request( BOOL bAndLoad, BOOL bWaitForLoad )");
    // don't bother loading it if it already is
    if ( IsLoaded () )
        return;

    if ( m_modelID <= 288 && m_modelID != 7 && !pGame->GetModelInfo ( 7 )->IsLoaded () )
    {
        // Skin 7 must be loaded in order for other skins to work. No, really. (#4010)
        pGame->GetModelInfo ( 7 )->Request ( bAndLoad, false );
    }

    DWORD dwFlags;
    if ( bHighPriority )
        dwFlags = 0x16;
    else
        dwFlags = 6;

    pGame->GetStreaming()->RequestModel( m_modelID, dwFlags );

    int iTimeToWait = 50;

    if(bAndLoad)
    {
        pGame->GetStreaming()->LoadAllRequestedModels();
        
        if(bWaitForLoad)
        {
            while(!this->IsLoaded() && iTimeToWait != 0)
            {
                iTimeToWait--;
                Sleep(10);
            }
        }
    }
}

VOID CModelInfoSA::Remove ( )
{
    DEBUG_TRACE("VOID CModelInfoSA::Remove ( )");

    // Don't remove if GTA refers to it somehow.
    // Or we'll screw up SA's map for example.

    m_pInterface = ppModelInfo [ m_modelID ];

    // Remove our reference
    m_pInterface->Dereference();

    // No references left?
    if ( m_pInterface->m_numberOfRefs == 0 )
    {  
        // Make our collision model original again before we unload.
        RestoreColModel ();

        // Remove the model
        pGame->GetStreaming()->FreeModel( m_modelID );
    }
}

BYTE CModelInfoSA::GetLevelFromPosition ( CVector * vecPosition )
{
    DEBUG_TRACE("BYTE CModelInfoSA::GetLevelFromPosition ( CVector * vecPosition )");
    DWORD dwFunction = FUNC_GetLevelFromPosition;
    BYTE bReturn = 0;
    _asm
    {
        push    vecPosition
        call    dwFunction
        add     esp, 4
        mov     bReturn, al
    }
    return bReturn;
}

BOOL CModelInfoSA::IsLoaded ( )
{
    DEBUG_TRACE("BOOL CModelInfoSA::IsLoaded ( )");
    if ( IsUpgrade () )
        return pGame->GetStreaming ()->HasVehicleUpgradeLoaded ( m_modelID );

    //return (BOOL)*(BYTE *)(ARRAY_ModelLoaded + 20*dwModelID);
    BOOL bLoaded = pGame->GetStreaming()->HasModelLoaded(m_modelID);
    m_pInterface = ppModelInfo [ m_modelID ];
    return bLoaded;
}

BYTE CModelInfoSA::GetFlags ( )
{
    DWORD dwFunc = FUNC_GetModelFlags;
    DWORD ModelID = m_modelID;
    BYTE bFlags = 0;
    _asm
    {
        push    ModelID
        call    dwFunc
        add     esp, 4
        mov     bFlags, al
    }
    return bFlags;
}

CBoundingBox * CModelInfoSA::GetBoundingBox ( )
{
    DWORD dwFunc = FUNC_GetBoundingBox;
    DWORD ModelID = m_modelID;
    CBoundingBox * dwReturn = 0;
    _asm
    {
        push    ModelID
        call    dwFunc
        add     esp, 4
        mov     dwReturn, eax
    }
    return dwReturn;
}

bool CModelInfoSA::IsValid ( )
{
    return ppModelInfo [ m_modelID ] != 0;
}

float CModelInfoSA::GetDistanceFromCentreOfMassToBaseOfModel ( )
{

    DWORD dwModelInfo = 0;
    DWORD ModelID = m_modelID;
    FLOAT fReturn = 0;
    _asm
    {
        mov     eax, ModelID
        mov     eax, ARRAY_ModelInfo[eax*4]
        mov     eax, [eax+20]
        cmp     eax, 0
        jz      skip
        fld     [eax + 8]
        fchs
        fstp    fReturn
skip:
    }
    return fReturn;
}

unsigned short CModelInfoSA::GetTextureDictionaryID ()
{
    if ( !m_pInterface )
        return 0;

    return m_pInterface->m_textureDictionary;
}

void CModelInfoSA::SetTextureDictionaryID ( unsigned short usID )
{
    if ( !m_pInterface )
        return;

    m_pInterface->m_textureDictionary = usID;
}

float CModelInfoSA::GetLODDistance ()
{
    if ( !m_pInterface )
        return 0;

    return m_pInterface->m_lodDistance;
}

void CModelInfoSA::SetLODDistance ( float fDistance )
{
#if 0
    // fLodDistanceUnscaled values:
    //
    // With the draw distance setting in GTA SP options menu set to maximum:
    //      0 - 170     roughly correlates to a LOD distance of 0 - 300 game units
    //      170 - 480   sets the LOD distance to 300 game units and has a negative effect on the alpha fade-in
    //      490 - 1e7   sets the LOD distance to 300 game units and removes the alpha fade-in completely
    //
    // With the draw distance setting in GTA SP options menu set to minimum:
    //      0 - 325     roughly correlates to a LOD distance of 0 - 300 game units
    //      340 - 960   sets the LOD distance to 300 game units and has a negative effect on the alpha fade-in
    //      1000 - 1e7  sets the LOD distance to 300 game units and removes the alpha fade-in completely
    //
    // So, to ensure the maximum draw distance with a working alpha fade-in, fLodDistanceUnscaled has to be
    // no more than: 325 - (325-170) * draw_distance_setting
    //

    // Change GTA draw distance value from 0.925 to 1.8 into 0 to 1
    float fDrawDistanceSetting = UnlerpClamped ( 0.925f, CSettingsSA ().GetDrawDistance (), 1.8f );

    // Calc max setting allowed for fLodDistanceUnscaled to preserve alpha fade-in
    float fMaximumValue = Lerp ( 325.f, fDrawDistanceSetting, 170.f );

    // Ensure fDistance is in range
    fDistance = Min ( fDistance, fMaximumValue );
#endif
    if ( !m_pInterface )
        return;

    m_pInterface->m_lodDistance = fDistance;
}

void CModelInfoSA::RestreamIPL ()
{
    MapSet ( ms_RestreamTxdIDMap, GetTextureDictionaryID (), 0 );
}

void CModelInfoSA::StaticFlushPendingRestreamIPL ( void )
{
    if ( ms_RestreamTxdIDMap.empty () )
        return;
    // This function restreams all instances of the model *that are from the default SA world (ipl)*.
    // In other words, it does not affect elements created by MTA.
    // It's mostly a reimplementation of SA's DeleteAllRwObjects, except that it filters by model ID.

    ( (void (*)())FUNC_FlushRequestList )();

    std::set < unsigned short > removedModels;
    
    for ( int i = 0; i < 2*NUM_StreamSectorRows*NUM_StreamSectorCols; i++ )
    {
        DWORD* pSectorEntry = ((DWORD **)ARRAY_StreamSectors) [ i ];
        while ( pSectorEntry )
        {
            CEntitySAInterface* pEntity = (CEntitySAInterface *)pSectorEntry [ 0 ];

#ifdef _todo
            // Possible bug - pEntity seems to be invalid here occasionally
            if ( pEntity->vtbl->DeleteRwObject != 0x00534030 )
            {
                // Log info
                OutputDebugString ( SString ( "Entity 0x%08x (with model %d) at ARRAY_StreamSectors[%d,%d] is invalid\n", pEntity, pEntity->m_nModelIndex, i / 2 % NUM_StreamSectorRows, i / 2 / NUM_StreamSectorCols ) );
                // Assert in debug
                #if _DEBUG
                    assert ( pEntity->vtbl->DeleteRwObject == 0x00534030 );
                #endif
                pSectorEntry = (DWORD *)pSectorEntry [ 1 ];
                continue;
            }
#endif

            if ( MapContains ( ms_RestreamTxdIDMap, pGame->GetModelInfo ( pEntity->m_nModelIndex )->GetTextureDictionaryID () ) )
            {
                if ( !pEntity->bStreamingDontDelete && !pEntity->bImBeingRendered )
                {
                    // Delete the renderware object
                    pEntity->DeleteRwObject();

                    removedModels.insert ( pEntity->m_nModelIndex );
                }
            }
            
            pSectorEntry = (DWORD *)pSectorEntry [ 1 ];
        }
    }

    for ( int i = 0; i < NUM_StreamRepeatSectorRows*NUM_StreamRepeatSectorCols; i++ )
    {
        DWORD* pSectorEntry = ((DWORD **)ARRAY_StreamRepeatSectors) [ 3*i + 2 ];
        while ( pSectorEntry )
        {
            CEntitySAInterface* pEntity = (CEntitySAInterface *)pSectorEntry [ 0 ];
            if ( MapContains ( ms_RestreamTxdIDMap, pGame->GetModelInfo ( pEntity->m_nModelIndex )->GetTextureDictionaryID () ) )
            {
                if ( !pEntity->bStreamingDontDelete && !pEntity->bImBeingRendered )
                {
                    pEntity->DeleteRwObject();

                    removedModels.insert ( pEntity->m_nModelIndex );
                }
            }
            pSectorEntry = (DWORD *)pSectorEntry [ 1 ];
        }
    }

    ms_RestreamTxdIDMap.clear ();

    std::set < unsigned short >::iterator it;
    for ( it = removedModels.begin (); it != removedModels.end (); it++ )
    {
        pGame->GetStreaming()->FreeModel(*it);
    }
}

void CModelInfoSA::AddRef ( bool bWaitForLoad, bool bHighPriority )
{
    // Are we not loaded?
    if ( !IsLoaded () )
    {
        // Request it. Wait for it to load if we're asked to.
        if ( pGame && pGame->IsASyncLoadingEnabled () )
            Request ( bWaitForLoad, bWaitForLoad, bHighPriority );
        else
            Request ( true, bWaitForLoad, bHighPriority );
    }

    // Increment the references.
    if ( m_dwReferences == 0 )
    {
        m_pInterface = ppModelInfo [ m_modelID ];

        m_pInterface->m_numberOfRefs++;
    }

    m_dwReferences++;
}

int CModelInfoSA::GetRefCount ()
{
    return static_cast < int > ( m_dwReferences );
}

void CModelInfoSA::RemoveRef ( bool bRemoveExtraGTARef )
{
    // Decrement the references
    if ( m_dwReferences > 0 )
        m_dwReferences--;

    // Handle extra ref if requested
    if ( bRemoveExtraGTARef )
        MaybeRemoveExtraGTARef ();

    // Unload it if 0 references left and we're not CJ model.
    // And if we're loaded.
    if ( m_dwReferences == 0 &&
         m_modelID != 0 &&
         IsLoaded () )
    {
        Remove ();
    }
}

//
// Notes on changing model for the local player only
// -------------------------------------------------
// When changing model for the local player, the model ref count is not decremented correctly. (Due to the ped not being re-created.)
// Eventually, after many changes of local player skin, we run out of entries in COL_MODEL_POOL
//
// So, when changing model for the local player, an extra reference must be removed to allow the old model to be unloaded.
//
// However, manually removing the extra reference creates a memory leak when the model is unloaded.
//
// Therefore, to keep the memory loss to a minimum, the removal of the extra reference is delayed.
//
// The delay scheme implemented to do this consists of two queues:
//      The first queue is filled up with the first 5 models that are used for the local player. Once full the queue is not changed until MTA is restarted.
//      The second queue is FILO, with the last out having it's extra reference removed.
//

std::vector < CModelInfoSA* >   ms_FirstDelayQueue;     // Don't remove ref for first 5 unique models
std::list < CModelInfoSA* >     ms_SecondDelayQueue;    // Then delay remove for 5 changes

void CModelInfoSA::MaybeRemoveExtraGTARef ( void )
{
    // Safety check
    if ( m_pInterface->m_numberOfRefs == 0 )
        return;

    //
    // Handle first queue
    //
    if ( ListContains ( ms_FirstDelayQueue, this ) )
    {
        // If already delaying a previous extra ref, we can remove this extra ref now
        m_pInterface->Dereference();
        return;
    }

    if ( ms_FirstDelayQueue.size () < 5 )
    {
        // Fill initial queue
        ms_FirstDelayQueue.push_back ( this );
        return;
    }

    //
    // Handle second queue
    //
    if ( ListContains ( ms_SecondDelayQueue, this ) )
    {
        // If already delaying a previous extra ref, we can remove this extra ref now
        m_pInterface->Dereference();

        // Bring item to front
        ListRemove ( ms_SecondDelayQueue, this );
        ms_SecondDelayQueue.push_back ( this );
    }
    else
    {
        // Top up secondary queue
        ms_SecondDelayQueue.push_back ( this );
        
        // Remove extra ref from the oldest queue item
        if ( ms_SecondDelayQueue.size () > 5 )
        {
            CModelInfoSA* pOldModelInfo = ms_SecondDelayQueue.front ();
            ms_SecondDelayQueue.pop_front ();
            pOldModelInfo->DoRemoveExtraGTARef ();
        }
    }
}

// Remove extra GTA ref and handle actual model unload if then required
void CModelInfoSA::DoRemoveExtraGTARef ( void )
{
    // Seriously, if we are not loaded, why the fuck is there a modelInfo?
    if (m_modelID == 0)
        return;

    m_pInterface->usNumberOfRefs++; // Hack because Remove() decrements this
    Remove ();
}

short CModelInfoSA::GetAvailableVehicleMod ( unsigned short usUpgrade )
{
    short sreturn = -1;
    if ( usUpgrade >= 1000 && usUpgrade <= 1193 )
    {
        DWORD ModelID = m_modelID;
        _asm
        {
            mov     eax, ModelID
            movsx   edx, usUpgrade
            mov     eax, ARRAY_ModelInfo[eax*4]
            mov     ax, [eax+edx*2+0x2D6]
            mov     sreturn, ax
        }
    }
    return sreturn;
}

bool CModelInfoSA::IsUpgradeAvailable ( eVehicleUpgradePosn posn )
{
    bool bRet = false;
    DWORD ModelID = m_modelID;
    _asm
    {
        mov     eax, ModelID
        lea     ecx, ARRAY_ModelInfo[eax*4]

        mov     eax, posn
        mov     ecx, [ecx+0x5C]
        shl     eax, 5
        push    esi
        mov     esi, [ecx+eax+0D0h]
        xor     edx, edx
        test    esi, esi
        setnl   dl
        mov     al, dl
        pop     esi

        mov     bRet, al
    }
    return bRet;
}

void CModelInfoSA::SetCustomCarPlateText ( const char * szText )
{
    char * szStoredText;
    DWORD ModelID = m_modelID;
    _asm
    {
        push    ecx
        mov     ecx, ModelID
        mov     ecx, ARRAY_ModelInfo[ecx*4]
        add     ecx, 40
        mov     szStoredText, ecx
        pop     ecx
    }

    if ( szText ) 
        strncpy ( szStoredText, szText, 8 );
    else
        szStoredText[0] = 0;
}

unsigned int CModelInfoSA::GetNumRemaps ( void )
{
    DWORD dwFunc = FUNC_CVehicleModelInfo__GetNumRemaps;
    DWORD ModelID = m_modelID;
    unsigned int uiReturn = 0;
    _asm
    {
        mov     ecx, ModelID
        mov     ecx, ARRAY_ModelInfo[ecx*4]
        call    dwFunc
        mov     uiReturn, eax
    }
    return uiReturn;
}

void* CModelInfoSA::GetVehicleSuspensionData ( void )
{
    return GetInterface ()->m_pColModel->pColData->pSuspensionLines;
}

void* CModelInfoSA::SetVehicleSuspensionData ( void* pSuspensionLines )
{
    CColDataSA* pColData = m_pInterface->m_pColModel->pColData;
    void* pOrigSuspensionLines = pColData->pSuspensionLines;

    pColData->pSuspensionLines = pSuspensionLines;
    return pOrigSuspensionLines;
}

void CModelInfoSA::RequestVehicleUpgrade ( void )
{
    DWORD dwFunc = FUNC_RequestVehicleUpgrade;
    DWORD ModelID = m_modelID;
    _asm
    {
        push    10
        push    ModelID
        call    dwFunc
        add     esp, 8
    }
}

void CModelInfoSA::SetCustomModel ( RpClump* pClump )
{
    // Error
    if ( pClump == NULL )
        return;

    // Store the custom clump
    m_pCustomClump = pClump;

    // Replace the vehicle model if we're loaded.
    if ( IsLoaded () )
    {
        // Are we a vehicle?
        if ( IsVehicle () )
        {
            pGame->GetRenderWare ()->ReplaceVehicleModel ( pClump, m_modelID );
        }
        else
        {
            // We are an object.
            pGame->GetRenderWare ()->ReplaceAllAtomicsInModel ( pClump, m_modelID );
        }
    }
}

void CModelInfoSA::RestoreOriginalModel ( void )
{
    // Are we loaded?
    if ( IsLoaded () )
        pGame->GetStreaming()->FreeModel( m_dwModelID );

    // Reset the stored custom vehicle clump
    m_pCustomClump = NULL;
}

void CModelInfoSA::SetColModel ( CColModel* pColModel )
{
    // Grab the interfaces
    CColModelSAInterface* pInterface = pColModel->GetInterface ();

    // Store the col model we set
    m_pCustomColModel = pColModel;

    // Do the following only if we're loaded
    m_pInterface = ppModelInfo [ m_dwModelID ];
    if ( m_pInterface )
    {
        // If no collision model has been set before, store the original in case we want to restore it
        if ( !m_pOriginalColModelInterface )
            m_pOriginalColModelInterface = m_pInterface->pColModel;

        // Apply some low-level hacks
        MemPutFast < BYTE > ( (BYTE*) pInterface + 40, 0xA9 );

        // Extra flags (3064) -- needs to be tested
        m_pInterface->bDoWeOwnTheColModel = false;
        m_pInterface->bCollisionWasStreamedWithModel = false;

        // Call SetColModel
        DWORD dwFunc = FUNC_SetColModel;
        DWORD ModelID = m_dwModelID;
        _asm
        {
            mov     ecx, ModelID
            mov     ecx, ARRAY_ModelInfo[ecx*4]
            push    1
            push    pInterface
            call    dwFunc
        }

        // public: static void __cdecl CColAccel::addCacheCol(int, class CColModel const &)
        DWORD func = 0x5B2C20;
        __asm {
            push    pInterface
            push    ModelID
            call    func
            add     esp, 8
        }
        #pragma message(__LOC__ "(IJs) Document this function some time.")
    }
}

void CModelInfoSA::RestoreColModel ( void )
{
    // Are we loaded?
    m_pInterface = ppModelInfo [ m_dwModelID ];
    if ( m_pInterface )
    {
        // We only have to store if the collision model was set
        // Also only if we have a col model set
        if ( m_pOriginalColModelInterface && m_pCustomColModel )
        {
            DWORD dwFunc = FUNC_SetColModel;
            DWORD dwOriginalColModelInterface = (DWORD)m_pOriginalColModelInterface;
            DWORD ModelID = m_dwModelID;
            _asm
            {
                mov     ecx, ModelID
                mov     ecx, ARRAY_ModelInfo[ecx*4]
                push    1
                push    dwOriginalColModelInterface
                call    dwFunc
            }

            // public: static void __cdecl CColAccel::addCacheCol(int, class CColModel const &)
            DWORD func = 0x5B2C20;
            __asm {
                push    dwOriginalColModelInterface
                push    ModelID
                call    func
                add     esp, 8
            }
            #pragma message(__LOC__ "(IJs) Document this function some time.")
        }
    }

    // We currently have no custom model loaded
    m_pCustomColModel = NULL;
}


void CModelInfoSA::MakeCustomModel ( void )
{
    // We have a custom model?
    if ( m_pCustomClump )
    {
        SetCustomModel ( m_pCustomClump );
    }

    // Custom collision model is not NULL and it's different from the original?
    if ( m_pCustomColModel )
    {
        SetColModel ( m_pCustomColModel );
    }
}


void CModelInfoSA::GetVoice ( short* psVoiceType, short* psVoiceID )
{
    if ( psVoiceType )
        *psVoiceType = GetPedModelInfoInterface ()->sVoiceType;
    if ( psVoiceID )
        *psVoiceID = GetPedModelInfoInterface ()->sFirstVoice;
}

void CModelInfoSA::GetVoice ( const char** pszVoiceType, const char** pszVoice )
{
    short sVoiceType, sVoiceID;
    GetVoice ( &sVoiceType, &sVoiceID );
    if ( pszVoiceType )
        *pszVoiceType = CPedSoundSA::GetVoiceTypeNameFromID ( sVoiceType );
    if ( pszVoice )
        *pszVoice = CPedSoundSA::GetVoiceNameFromID ( sVoiceType, sVoiceID );
}

void CModelInfoSA::SetVoice ( short sVoiceType, short sVoiceID )
{
    GetPedModelInfoInterface ()->sVoiceType = sVoiceType;
    GetPedModelInfoInterface ()->sFirstVoice = sVoiceID;
    GetPedModelInfoInterface ()->sLastVoice = sVoiceID;
    GetPedModelInfoInterface ()->sNextVoice = sVoiceID;
}

void CModelInfoSA::SetVoice ( const char* szVoiceType, const char* szVoice )
{
    short sVoiceType = CPedSoundSA::GetVoiceTypeIDFromName ( szVoiceType );
    if ( sVoiceType < 0 )
        return;
    short sVoiceID = CPedSoundSA::GetVoiceIDFromName ( sVoiceType, szVoice );
    if ( sVoiceID < 0 )
        return;
    SetVoice ( sVoiceType, sVoiceID );
}

void CModelInfoSA::MakePedModel ( char * szTexture )
{
    // Create a new CPedModelInfo
    CPedModelInfoSA pedModelInfo;
    ppModelInfo [ m_dwModelID ] = ( CBaseModelInfoSAInterface * ) pedModelInfo.GetPedModelInfoInterface ();

    // Load our texture
    pGame->GetStreaming ()->RequestSpecialModel ( m_dwModelID, szTexture, 0 );
}
