/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelInfoSA.cpp
*  PURPOSE:     Entity model information handler
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CGameSA * pGame;

CBaseModelInfoSAInterface** ppModelInfo = (CBaseModelInfoSAInterface**) ARRAY_ModelInfo;

std::map < unsigned short, int > CModelInfoSA::ms_RestreamTxdIDMap;

struct timeInfo
{
    BYTE                m_pad[2];
    unsigned short      m_model;
};

void CBaseModelInfoSAInterface::SetColModel( CColModelSAInterface *col, bool putTimed )
{
    SetCollision( col, putTimed );
}

void CBaseModelInfoSAInterface::UnsetColModel()
{
    DeleteCollision();
}

static void __cdecl ConvertIDECompositeToInfoFlags( CBaseModelInfoSAInterface *info, unsigned int flags )
{
    BOOL_FLAG( info->m_renderFlags, RENDER_LAST, flags & ( OBJECT_ALPHA1 | OBJECT_ALPHA2 ) );
    BOOL_FLAG( info->m_renderFlags, RENDER_ADDITIVE, flags & OBJECT_ALPHA2 );
    BOOL_FLAG( info->m_renderFlags, RENDER_NOZBUFFER, flags & OBJECT_NOSHADOW );
    BOOL_FLAG( info->m_renderFlags, RENDER_NOSHADOW, flags & OBJECT_NOCULL );
    BOOL_FLAG( info->m_renderFlags, RENDER_BACKFACECULL, !( flags & OBJECT_NOBACKFACECULL ) );
}

static void __cdecl ConvertIDEModelToInfoFlags( CBaseModelInfoSAInterface *info, unsigned int flags )
{
    // Composite flags apply to clumps and atomic models
    ConvertIDECompositeToInfoFlags( info, flags );

    BOOL_FLAG( info->m_renderFlags, RENDER_STATIC, flags & OBJECT_WETEFFECT );
 
    if ( flags & OBJECT_BREAKGLASS )
    {
        info->m_collFlags &= ~( COLL_SWAYINWIND | COLL_STREAMEDWITHMODEL | COLL_COMPLEX );
        info->m_collFlags |= COLL_NOCOLLFLYER;
    }

    if ( flags & OBJECT_BREAKGLASS_CRACK )
    {
        info->m_collFlags &= ~( COLL_STREAMEDWITHMODEL | COLL_COMPLEX );
        info->m_collFlags |= COLL_NOCOLLFLYER | COLL_SWAYINWIND;
    }

    if ( flags & OBJECT_GARAGE )
    {
        info->m_collFlags &= ~COLL_COMPLEX;
        info->m_collFlags |= COLL_NOCOLLFLYER | COLL_SWAYINWIND | COLL_STREAMEDWITHMODEL;
    }

    if ( flags & OBJECT_VEGETATION )
    {
        info->m_collFlags &= ~( COLL_STREAMEDWITHMODEL | COLL_NOCOLLFLYER | COLL_COMPLEX );
        info->m_collFlags |= COLL_SWAYINWIND;
    }

    if ( flags & OBJECT_BIG_VEGETATION )
    {
        info->m_collFlags &= ~( COLL_SWAYINWIND | COLL_NOCOLLFLYER | COLL_COMPLEX );
        info->m_collFlags |= COLL_STREAMEDWITHMODEL;
    }

    BOOL_FLAG( info->m_collFlags, COLL_AUDIO, flags & OBJECT_USE_POLYSHADOW );

    if ( flags & OBJECT_GRAFFITI )
    {
        info->m_collFlags &= ~( COLL_SWAYINWIND | COLL_COMPLEX );
        info->m_collFlags |= COLL_STREAMEDWITHMODEL | COLL_NOCOLLFLYER;
    }

    if ( flags & OBJECT_STATUE )
    {
        info->m_collFlags &= ~COLL_NOCOLLFLYER;
        info->m_collFlags |= COLL_COMPLEX | COLL_STREAMEDWITHMODEL | COLL_SWAYINWIND;
    }

    if ( flags & OBJECT_UNKNOWN_2 )
    {
        info->m_collFlags &= ~( COLL_NOCOLLFLYER | COLL_SWAYINWIND );
        info->m_collFlags |= COLL_COMPLEX | COLL_STREAMEDWITHMODEL;
    }
}

void ModelInfo_Init()
{
    // Install some fixes
    HookInstall( 0x004C4BC0, h_memFunc( &CBaseModelInfoSAInterface::SetColModel ), 5 );
    HookInstall( 0x004C4C40, h_memFunc( &CBaseModelInfoSAInterface::UnsetColModel ), 5 );

    // Investigation of model flags
    HookInstall( 0x005B3AD0, (DWORD)ConvertIDECompositeToInfoFlags, 5 );
    HookInstall( 0x005B3B20, (DWORD)ConvertIDEModelToInfoFlags, 5 );
}

void ModelInfo_Shutdown()
{

}


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

void CBaseModelInfoSAInterface::Init()
{
    m_numberOfRefs = 0;
    m_textureDictionary = -1;
    m_pColModel = NULL;
    m_effectID = -1;
    m_num2dfx = 0;
    m_dynamicIndex = -1;
    m_lodDistance = 2000;

    m_renderFlags = RENDER_COLMODEL | RENDER_BACKFACECULL;
}

void CBaseModelInfoSAInterface::Shutdown()
{
    DeleteRwObject();

    DeleteCollision();

    m_renderFlags |= RENDER_PRERENDERED;

    m_effectID = -1;
    m_num2dfx = 0;

    m_dynamicIndex = -1;
    m_textureDictionary = -1;
}

unsigned int CBaseModelInfoSAInterface::GetTimeInfo()
{
    return 0;
}

void CBaseModelInfoSAInterface::SetCollision( CColModelSAInterface *col, bool putTimed )
{
    m_pColModel = col;

    if ( putTimed )
    {
        m_renderFlags |= RENDER_COLMODEL;

        timeInfo *timed = (timeInfo*)GetTimeInfo();

        if ( timed && timed->m_model != 0xFFFF )
            ppModelInfo[timed->m_model]->SetCollision( col, false );
    }
    else
        m_renderFlags &= ~RENDER_COLMODEL;
}

void CBaseModelInfoSAInterface::DeleteCollision()
{
    if ( m_pColModel && ( m_renderFlags & RENDER_COLMODEL ) )
        delete m_pColModel;

    m_pColModel = NULL;
}

void CBaseModelInfoSAInterface::DeleteTextures()
{
    if ( m_textureDictionary == -1 )
        return;

    (*ppTxdPool)->Get( m_textureDictionary )->Dereference();

    m_textureDictionary = -1;

    if ( GetAnimFileIndex() != -1 )
        pGame->GetAnimManager()->RemoveAnimBlockRef( GetAnimFileIndex() );

    // What is this about?
    //pGame->GetAnimManager()->RemoveAnimBlockRef( GetAnimFileIndex() );
}

void CBaseModelInfoSAInterface::Reference()
{
    m_numberOfRefs++;

    (*ppTxdPool)->Get( m_textureDictionary )->Reference();
}

void CBaseModelInfoSAInterface::Dereference()
{
    m_numberOfRefs--;

    (*ppTxdPool)->Get( m_textureDictionary )->Dereference();
}

unsigned short CBaseModelInfoSAInterface::GetFlags()
{
    return m_renderFlags | ( m_collFlags >> 10 );
}

CModelInfoSA::CModelInfoSA()
{
    m_pInterface = NULL;
    m_modelID = 0xFFFF;
    m_dwReferences = 0;
    m_pOriginalColModelInterface = NULL;
    m_pCustomColModel = NULL;
}

CModelInfoSA::CModelInfoSA( unsigned short id )
{
    m_modelID = id;
    m_pInterface = ppModelInfo [ id ];
    m_dwReferences = 0;
    m_pOriginalColModelInterface = NULL;
    m_pCustomColModel = NULL;
}

eVehicleType CModelInfoSA::GetVehicleType() const
{
    return ((CVehicleModelInfoSAInterface*)ppModelInfo[m_modelID])->m_vehicleType;
}

bool CModelInfoSA::IsBoat() const
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_BOAT;
}

bool CModelInfoSA::IsCar() const
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_CAR;
}

bool CModelInfoSA::IsTrain() const
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_TRAIN;
}   

bool CModelInfoSA::IsHeli() const
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_HELI;
}   

bool CModelInfoSA::IsPlane() const
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_PLANE;
}   

bool CModelInfoSA::IsBike() const
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_BIKE;
}

bool CModelInfoSA::IsBmx() const
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_BICYCLE;
}   

bool CModelInfoSA::IsTrailer() const
{
    if ( !IsVehicle() )
        return false;

    return ((CVehicleModelInfoSAInterface*)m_pInterface)->m_vehicleType == VEHICLE_AUTOMOBILETRAILER;
}   

bool CModelInfoSA::IsVehicle() const
{
    return m_pInterface->GetModelType() == MODEL_VEHICLE;
}   

bool CModelInfoSA::IsUpgrade() const
{
    return m_modelID >= 1000 && m_modelID <= 1193;
}

bool CModelInfoSA::IsPed() const
{
    CBaseModelInfoSAInterface *info = ppModelInfo[m_modelID];

    if ( !info )
        return false;

    return info->GetModelType() == MODEL_PED;
}

bool CModelInfoSA::IsObject() const
{
    CBaseModelInfoSAInterface *info = ppModelInfo[m_modelID];

    if ( !info )
        return false;

    return info->GetRwModelType() == RW_ATOMIC;
}

const char* CModelInfoSA::GetNameIfVehicle() const
{
    DEBUG_TRACE("const char* CModelInfoSA::GetNameIfVehicle() const");

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
    return (const char*)dwReturn;
}

void CModelInfoSA::Request( bool bAndLoad, bool bWaitForLoad, bool bHighPriority )
{
    DEBUG_TRACE("void CModelInfoSA::Request( bool bAndLoad, bool bWaitForLoad, bool bHighPriority )");

    // don't bother loading it if it already is
    if ( IsLoaded() )
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
        // Remove the model
        pGame->GetStreaming()->FreeModel( m_modelID );
    }
}

unsigned char CModelInfoSA::GetLevelFromPosition( const CVector& vecPosition ) const
{
    DEBUG_TRACE("unsigned char CModelInfoSA::GetLevelFromPosition( const CVector& vecPosition ) const");
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

bool CModelInfoSA::IsLoaded() const
{
    DEBUG_TRACE("bool CModelInfoSA::IsLoaded() const");

    if ( IsUpgrade() )
        return pGame->GetStreaming()->HasVehicleUpgradeLoaded( m_modelID );

    //return (BOOL)*(BYTE *)(ARRAY_ModelLoaded + 20*dwModelID);
    return pGame->GetStreaming()->HasModelLoaded(m_modelID);
}

unsigned char CModelInfoSA::GetFlags() const
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

const CBoundingBox* CModelInfoSA::GetBoundingBox() const
{
    CBaseModelInfoSAInterface *info = ppModelInfo[m_modelID];

    if ( !info )
        return NULL;

    if ( info->m_pColModel )
        return NULL;

    return &ppModelInfo[m_modelID]->m_pColModel->m_bounds;
}

bool CModelInfoSA::IsValid()
{
    return( m_pInterface = ppModelInfo[m_modelID] ) != 0;
}

float CModelInfoSA::GetDistanceFromCentreOfMassToBaseOfModel() const
{
    CBaseModelInfoSAInterface *info = ppModelInfo[m_modelID];

    if ( !info )
        return 0;

    return -info->m_pColModel->m_bounds.vecBoundMin.fZ;
}

unsigned short CModelInfoSA::GetTextureDictionaryID() const
{
    CBaseModelInfoSAInterface *info = ppModelInfo[m_modelID];

    if ( !info )
        return 0;

    return info->m_textureDictionary;
}

void CModelInfoSA::SetTextureDictionaryID( unsigned short usID )
{
    if ( !m_pInterface )
        return;

    m_pInterface->m_textureDictionary = usID;
}

float CModelInfoSA::GetLODDistance() const
{
    if ( !m_pInterface )
        return 0;

    return m_pInterface->m_lodDistance;
}

void CModelInfoSA::SetLODDistance( float fDistance )
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

void CModelInfoSA::RestreamIPL()
{
    MapSet( ms_RestreamTxdIDMap, GetTextureDictionaryID(), 0 );
}

void CModelInfoSA::StaticFlushPendingRestreamIPL()
{
    if ( ms_RestreamTxdIDMap.empty() )
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
            CEntitySAInterface *pEntity = (CEntitySAInterface*)pSectorEntry[0];

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

            if ( MapContains( ms_RestreamTxdIDMap, pGame->GetModelInfo( pEntity->m_model )->GetTextureDictionaryID () ) )
            {
                if ( !IS_FLAG( pEntity->m_entityFlags, ENTITY_DISABLESTREAMING ) && !IS_FLAG( pEntity->m_entityFlags, ENTITY_RENDERING ) )
                {
                    // Delete the renderware object
                    pEntity->DeleteRwObject();

                    removedModels.insert ( pEntity->m_model );
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
            if ( MapContains ( ms_RestreamTxdIDMap, pGame->GetModelInfo ( pEntity->m_model )->GetTextureDictionaryID () ) )
            {
                if ( !IS_FLAG( pEntity->m_entityFlags, ENTITY_DISABLESTREAMING ) && !IS_FLAG( pEntity->m_entityFlags, ENTITY_RENDERING ) )
                {
                    pEntity->DeleteRwObject();

                    removedModels.insert( pEntity->m_model );
                }
            }
            pSectorEntry = (DWORD *)pSectorEntry [ 1 ];
        }
    }

    ms_RestreamTxdIDMap.clear();

    std::set < unsigned short >::iterator it;
    for ( it = removedModels.begin(); it != removedModels.end(); it++ )
    {
        pGame->GetStreaming()->FreeModel(*it);
    }
}

static modelRequestCallback_t modelRequestCallback = NULL;
static modelFreeCallback_t modelFreeCallback = NULL;

void CModelManagerSA::SetRequestCallback( modelRequestCallback_t callback )
{
    modelRequestCallback = callback;
}

void CModelManagerSA::SetFreeCallback( modelFreeCallback_t callback )
{
    modelFreeCallback = callback;
}

void CModelInfoSA::AddRef( bool bWaitForLoad, bool bHighPriority )
{
    if ( m_dwReferences == 0 )
    {
        // Notify the client
        if ( modelRequestCallback )
            modelRequestCallback( m_modelID );
    }

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

        m_pInterface->Reference();
    }

    m_dwReferences++;
}

void CModelInfoSA::RemoveRef( bool bRemoveExtraGTARef )
{
    // Decrement the references
    if ( m_dwReferences > 0 )
        m_dwReferences--;

    // Handle extra ref if requested
    if ( bRemoveExtraGTARef )
        MaybeRemoveExtraGTARef ();

    // Unload it if 0 references left and we're not CJ model.
    // And if we're loaded.
    if ( m_dwReferences == 0 )
    {
        if ( m_modelID != 0 && IsLoaded() )
            Remove();

        // Notify the client
        if ( modelFreeCallback )
            modelFreeCallback( m_modelID );
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

void CModelInfoSA::MaybeRemoveExtraGTARef()
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
void CModelInfoSA::DoRemoveExtraGTARef()
{
    if (m_modelID == 0)
        return;

    m_pInterface->m_numberOfRefs++; // Hack because Remove() decrements this
    Remove ();
}

short CModelInfoSA::GetAvailableVehicleMod( unsigned short usUpgrade ) const
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

bool CModelInfoSA::IsUpgradeAvailable( eVehicleUpgradePosn posn ) const
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

void CModelInfoSA::SetCustomCarPlateText( const char *szText )
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

unsigned int CModelInfoSA::GetNumRemaps() const
{
    return ((CVehicleModelInfoSAInterface*)m_pInterface)->GetNumberOfValidPaintjobs();
}

void* CModelInfoSA::GetVehicleSuspensionData() const
{
    return ppModelInfo[m_modelID]->m_pColModel->pColData->pSuspensionLines;
}

void* CModelInfoSA::SetVehicleSuspensionData( void* pSuspensionLines )
{
    CColDataSA* pColData = m_pInterface->m_pColModel->pColData;
    void* pOrigSuspensionLines = pColData->pSuspensionLines;

    pColData->pSuspensionLines = pSuspensionLines;
    return pOrigSuspensionLines;
}

void CModelInfoSA::RequestVehicleUpgrade()
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

#if 0
void CModelInfoSA::SetCustomModel( RpClump *pClump )
{
    // Replace the vehicle model if we're loaded.
    if ( IsLoaded() )
    {
        // Are we a vehicle?
        if ( !IsVehicle() )
        {
            // We are an object.
            pGame->GetRenderWare()->ReplaceAllAtomicsInModel( pClump, m_modelID );
        }
    }
}
#endif

void CModelInfoSA::GetVoice( short* psVoiceType, short* psVoiceID ) const
{
    if ( psVoiceType )
        *psVoiceType = GetPedModelInfoInterface ()->m_sVoiceType;

    if ( psVoiceID )
        *psVoiceID = GetPedModelInfoInterface ()->m_sFirstVoice;
}

void CModelInfoSA::GetVoice( const char** pszVoiceType, const char** pszVoice ) const
{
    short sVoiceType, sVoiceID;
    GetVoice ( &sVoiceType, &sVoiceID );

    if ( pszVoiceType )
        *pszVoiceType = CPedSoundSA::GetVoiceTypeNameFromID ( sVoiceType );

    if ( pszVoice )
        *pszVoice = CPedSoundSA::GetVoiceNameFromID ( sVoiceType, sVoiceID );
}

void CModelInfoSA::SetVoice( short sVoiceType, short sVoiceID )
{
    GetPedModelInfoInterface ()->m_sVoiceType = sVoiceType;
    GetPedModelInfoInterface ()->m_sFirstVoice = sVoiceID;
    GetPedModelInfoInterface ()->m_sLastVoice = sVoiceID;
    GetPedModelInfoInterface ()->m_sNextVoice = sVoiceID;
}

void CModelInfoSA::SetVoice( const char* szVoiceType, const char* szVoice )
{
    short sVoiceType = CPedSoundSA::GetVoiceTypeIDFromName ( szVoiceType );
    if ( sVoiceType < 0 )
        return;

    short sVoiceID = CPedSoundSA::GetVoiceIDFromName ( sVoiceType, szVoice );
    if ( sVoiceID < 0 )
        return;

    SetVoice ( sVoiceType, sVoiceID );
}

void CModelInfoSA::MakePedModel( const char *szTexture )
{
    // Create a new CPedModelInfo
    CPedModelInfoSA pedModelInfo;
    ppModelInfo[m_modelID] = pedModelInfo.GetPedModelInfoInterface();

    // Load our texture
    pGame->GetStreaming ()->RequestSpecialModel ( m_modelID, szTexture, 0 );
}
