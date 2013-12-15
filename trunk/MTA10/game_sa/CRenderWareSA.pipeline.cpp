/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.pipeline.cpp
*  PURPOSE:     RenderWare pipeline (OS implementation) management
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

// Nasty pools which limit rendering.
CEnvMapMaterialPool **ppEnvMapMaterialPool = (CEnvMapMaterialPool**)0x00C02D28;
CEnvMapAtomicPool **ppEnvMapAtomicPool = (CEnvMapAtomicPool**)0x00C02D2C;
CSpecMapMaterialPool **ppSpecMapMaterialPool = (CSpecMapMaterialPool**)0x00C02D30;

RwRenderStateLock::_rsLockDesc RwRenderStateLock::_rsLockInfo[210];

inline IDirect3DDevice9* GetRenderDevice( void )
{
    return core->GetGraphics()->GetDevice();
}

/*=========================================================
    RwD3D9SetRenderState

    Arguments:
        type - D3D9 RenderState id
        value - integer with the new RenderState value
    Purpose:
        Sets a RenderState to the RenderWare D3D9 pipeline
        system and applies it during next atomic render.
    Binary offsets:
        (1.0 US): 0x007FC2D0
        (1.0 EU): 0x007FC310
=========================================================*/
struct _renderStateDesc
{
    DWORD value;
    int addedToQuery;
};

void __cdecl RwD3D9SetRenderState( D3DRENDERSTATETYPE type, DWORD value )
{
    _renderStateDesc& desc = ((_renderStateDesc*)0x00C991D0)[type];

    if ( desc.value == value )
        return;

    desc.value = value;

    if ( !desc.addedToQuery )
    {
        desc.addedToQuery = 1;

        ((D3DRENDERSTATETYPE*)0x00C98B40)[ (*(unsigned int*)0x00C9A5EC)++ ] = type;
    }
}

/*=========================================================
    RwD3D9ForceRenderState (MTA extension)

    Arguments:
        type - D3D9 RenderState id
        value - integer with the new RenderState value
    Purpose:
        Applies a RenderState and ignores further RenderState
        assignments until that RenderState is not required
        by MTA anymore.
=========================================================*/
struct _internalRenderStateDesc
{
    _internalRenderStateDesc( void )
    {
        value = 0;
        isForced = false;
    }

    DWORD value;
    bool isForced;
};
static _internalRenderStateDesc _intRenderStates[210];

void RwD3D9ForceRenderState( D3DRENDERSTATETYPE type, DWORD value )
{
    _internalRenderStateDesc& desc = _intRenderStates[type];

    // Notify the system that we force a RenderState
    if ( !desc.isForced )
    {
        desc.isForced = true;
        RwD3D9GetRenderState( type, desc.value );
    }

    // Set the MTA RenderState
    RwD3D9SetRenderState( type, value );
}

/*=========================================================
    HOOK_RwD3D9SetRenderState

    Arguments:
        type - D3D9 RenderState id
        value - integer with the new RenderState value
    Purpose:
        Called by the engine to set a RenderWare D3D9 pipeline
        RenderState. MTA RenderStates can take priority over
        GTA:SA ones, so they should be delayed if there is a
        conflict.
    Binary offsets:
        (1.0 US): 0x007FC2D0
        (1.0 EU): 0x007FC310
=========================================================*/
static void __cdecl HOOK_RwD3D9SetRenderState( D3DRENDERSTATETYPE type, DWORD value )
{
    // Actual bugfix.
    __asm push edx

    _internalRenderStateDesc& desc = _intRenderStates[type];

    // Check whether MTA wants to force a RenderState
    if ( desc.isForced )
    {
        // Store the internal value
        desc.value = value;
    }
    else    // Otherwise we just set it
        RwD3D9SetRenderState( type, value );

    __asm pop edx
}

/*=========================================================
    RwD3D9GetRenderState

    Arguments:
        type - D3D9 RenderState id
    Purpose:
        Retrieves the current RenderWare D3D9 pipeline
        RenderState.
    Binary offsets:
        (1.0 US): 0x007FC320
        (1.0 EU): 0x007FC360
    Note:
        When dealing with API that tries to adapt to proprietory
        APIs like Direct3D, we better leave the usage as close to
        the original as possible. I had the idea to let this
        function return a DWORD, but decided against it since I
        realized that it did not match the GTA:SA representation.
=========================================================*/
void __cdecl RwD3D9GetRenderState( D3DRENDERSTATETYPE type, DWORD& value )
{
    value = ((_renderStateDesc*)0x00C991D0)[type].value;
}

/*=========================================================
    RwD3D9SetTextureStageState

    Arguments:
        stageId - index of the Direct3D 9 TSS
        stateType - type index of the TSS
        value - the new value to the TSS
    Purpose:
        Updates a texture stage state of the main Direct3D 9
        device. The entries are cached, so that unnecessary
        accesses are not made.
    Binary offsets:
        (1.0 US): 0x007FC340
        (1.0 EU): 0x007FC380
    Note:
        When dealing with API that tries to adapt to proprietory
        APIs like Direct3D, we better leave the usage as close to
        the original as possible. I had the idea to let this
        function return a DWORD, but decided against it since I
        realized that it did not match the GTA:SA representation.
=========================================================*/
struct _texStageStateQuery
{
    DWORD id;
    D3DTEXTURESTAGESTATETYPE type;
};

struct _texStageStateDesc
{
    DWORD value;
    int isQueried;
};

static _texStageStateDesc _texStateStages[8][33];

void __cdecl RwD3D9SetTextureStageState( DWORD stageId, D3DTEXTURESTAGESTATETYPE stateType, DWORD value )
{
    _texStageStateDesc& stateInfo = _texStateStages[stageId][stateType];

    if ( stateInfo.value == value )
        return;

    stateInfo.value = value;

    // Make sure the update it notified to the device.
    if ( !stateInfo.isQueried )
    {
        stateInfo.isQueried = true;

        // Queue the request to update our TSS to the device.
        _texStageStateQuery& queryItem = ((_texStageStateQuery*)0x00C99C80)[ (*(unsigned int*)0x00C9A5F0)++ ];
        queryItem.id = stageId;
        queryItem.type = stateType;
    }
}

/*=========================================================
    RwD3D9GetTextureStageState

    Arguments:
        stageId - index of the Direct3D 9 TSS
        stateType - type index of the TSS
        value - pointer to the value where the TSS value
                should be written to
    Purpose:
        Retrieves the current Direct3D 9 device texture stage
        state. The value may not be written to the device yet
        but is pending for a push on RwD3D9ApplyDeviceStates.
    Binary offsets:
        (1.0 US): 0x007FC3A0
        (1.0 EU): 0x007FC3E0
    Note:
        When dealing with API that tries to adapt to proprietory
        APIs like Direct3D, we better leave the usage as close to
        the original as possible. I had the idea to let this
        function return a DWORD, but decided against it since I
        realized that it did not match the GTA:SA representation.
=========================================================*/
void __cdecl RwD3D9GetTextureStageState( DWORD stageId, D3DTEXTURESTAGESTATETYPE stateType, DWORD& value )
{
    value = _texStateStages[stageId][stateType].value;
}

/*=========================================================
    HOOK_RwD3D9GetRenderState

    Arguments:
        type - D3D9 RenderState id
    Purpose:
        Retrieves the current RenderWare D3D9 pipeline
        RenderState (for GTA:SA).
    Binary offsets:
        (1.0 US): 0x007FC320
        (1.0 EU): 0x007FC360
=========================================================*/
static void __cdecl HOOK_RwD3D9GetRenderState( D3DRENDERSTATETYPE type, DWORD& value )
{
    // Actual bugfix.
    __asm push edx

    _internalRenderStateDesc& desc = _intRenderStates[type];

    if ( desc.isForced )
        value = desc.value;
    else
        RwD3D9GetRenderState( type, value );

    __asm pop edx
}

/*=========================================================
    RwD3D9FreeRenderState (MTA extension)

    Arguments:
        type - D3D9 RenderState id
    Purpose:
        Removes the forced assignment of a RenderWare D3D9
        pipeline RenderState by MTA and sets the GTA:SA
        RenderState value.
=========================================================*/
void RwD3D9FreeRenderState( D3DRENDERSTATETYPE type )
{
    _internalRenderStateDesc& desc = _intRenderStates[type];

    if ( !desc.isForced )
        return;

    // Notify we are not forcing a value anymore
    desc.isForced = false;

    // Revert to the GTA:SA version
    RwD3D9SetRenderState( type, desc.value );
}

/*=========================================================
    RwD3D9FreeRenderStates (MTA extension)

    Purpose:
        Works through the internal queue of GTA:SA assigned
        RenderStates and reverts them to their internal
        values.
=========================================================*/
void RwD3D9FreeRenderStates( void )
{
    for ( unsigned int n = 0; n < 210; n++ )
    {
        D3DRENDERSTATETYPE type = (D3DRENDERSTATETYPE)n;
        _internalRenderStateDesc& desc = _intRenderStates[type];

        // Revert the value (if forced)
        RwD3D9FreeRenderState( type );
    }

    RwD3D9ApplyDeviceStates();
}

/*=========================================================
    RwD3D9ApplyDeviceStates

    Purpose:
        Applies the queued RenderWare D3D9 RenderStates to
        the device.
    Binary offsets:
        (1.0 US): 0x007FC200
        (1.0 EU): 0x007FC240
=========================================================*/
struct texStageStateValue
{
    texStageStateValue( void )
    {
        value = 0xFFFFFFFF;
    }

    void operator = ( DWORD value )
    {
        this->value = value;
    }

    operator DWORD ( void ) const
    {
        return value;
    }

    DWORD value;
};

static texStageStateValue currentTextureStageStateValues[8][33];

void __cdecl RwD3D9ApplyDeviceStates( void )
{
    IDirect3DDevice9 *renderDevice = GetRenderDevice();
    
    // Apply RenderStates
    unsigned int renderStates = *(unsigned int*)0x00C9A5EC;

    for ( unsigned int n = 0; n < renderStates; n++ )
    {
        D3DRENDERSTATETYPE type = ((D3DRENDERSTATETYPE*)0x00C98B40)[n];
        _renderStateDesc& desc = ((_renderStateDesc*)0x00C991D0)[type];
        
        DWORD newState = desc.value;
        DWORD& currentState = ((DWORD*)0x00C98E88)[type];

        // We processed this entry
        desc.addedToQuery = false;

        if ( currentState != newState )
        {
            renderDevice->SetRenderState( type, newState );

            currentState = newState;
        }
    }

    // Processed, so zero out
    *(unsigned int*)0x00C9A5EC = 0;

    unsigned int textureStates = *(unsigned int*)0x00C9A5F0;

    // Apply texture stage states
    for ( unsigned int n = 0; n < textureStates; n++ )
    {
        _texStageStateQuery& item = ((_texStageStateQuery*)0x00C99C80)[n];
        _texStageStateDesc& desc = _texStateStages[item.id][item.type];

        desc.isQueried = false;

        DWORD newState = desc.value;
        texStageStateValue& currentState = currentTextureStageStateValues[item.id][item.type];

        if ( newState != currentState )
        {
            // Update the texture stage
            renderDevice->SetTextureStageState( item.id, item.type, newState );

            currentState = newState;
        }
    }

    // No more texture states to process.
    *(unsigned int*)0x00C9A5F0 = 0;
}

/*=========================================================
    RwD3D9SetStreams

    Arguments:
        streamInfo - descriptor of stream to set as current

    Purpose:
        Sets up the atomic pipeline, prepares the device information
        and initializes the material, atomic and specular material
        custom pools.
    Binary offsets:
        (1.0 US): 0x007FA090
        (1.0 EU): 0x007FA0D0
=========================================================*/
struct RwD3D9StreamInfo //size: 16 bytes
{
    int unk;            // 0
    void *unk2;         // 4
    void *unk3;         // 8
    int unk4;           // 12
};

inline RwD3D9StreamInfo& GetStreamInfo( unsigned int index )
{
    assert( index < 4 );

    return ((RwD3D9StreamInfo*)0x00C97BD8)[index];
}

void __cdecl RwD3D9SetStreams( RwD3D9StreamInfo *sinfo, int someBool )
{
    for ( unsigned int n = 0; n < 2; n++ )
    {
        //RwD3D9StreamInfo& 
    }
}

/*=========================================================
    HOOK_DefaultAtomicRenderingCallback

    Purpose:
        Sets up the atomic pipeline, prepares the device information
        and initializes the material, atomic and specular material
        custom pools.
    Binary offsets:
        (1.0 US): 0x00756DF0
        (1.0 EU): 0x00756E40
=========================================================*/
struct RwRenderCallbackTraverse
{
    BYTE                        m_pad[32];              // 0
    IDirect3DIndexBuffer9*      m_indexBuffer;          // 32
};

inline IDirect3DIndexBuffer9*& GetCurrentIndexBuffer( void )
{
    return *(IDirect3DIndexBuffer9**)0x008E2450;
}

inline void RwD3D9SetCurrentIndexBuffer( IDirect3DIndexBuffer9 *indexBuf )
{
    if ( indexBuf && GetCurrentIndexBuffer() != indexBuf )
    {
        GetCurrentIndexBuffer() = indexBuf;

        GetRenderDevice()->SetIndices( indexBuf );
    }
}

void __cdecl HOOK_DefaultAtomicRenderingCallback( RwRenderCallbackTraverse *rtinfo, RwObject *renderObject, eRwType renderType, void *unk3 )
{
    IDirect3DDevice9 *renderDevice = GetRenderDevice();

    IDirect3DPixelShader9 *currentPixelShader = *(IDirect3DPixelShader9**)0x008E244C;

    if ( currentPixelShader )
    {
        *(IDirect3DPixelShader9**)0x008E244C = NULL;

        renderDevice->SetPixelShader( currentPixelShader );
    }

    bool isObjectVisible = false;
    RwCamera *currentCam = pRwInterface->m_renderCam;

    if ( renderType == RW_ATOMIC )
    {
        isObjectVisible = RwD3D9CameraIsSphereFullyInsideFrustum( currentCam, ((RpAtomic*)renderObject)->GetWorldBoundingSphere() );
    }
    else
    {
        // Unknown type rendering!
        isObjectVisible = RwD3D9CameraIsBBoxFullyInsideFrustum( currentCam, ((CVector*) ((char*)renderObject + 96) ) );
    }

    HOOK_RwD3D9SetRenderState( D3DRS_CLIPPING, false );

    RwD3D9SetCurrentIndexBuffer( rtinfo->m_indexBuffer );

    
}

/*=========================================================
    HOOK_InitDeviceSystem

    Purpose:
        Sets up the atomic pipeline, prepares the device information
        and initializes the material, atomic and specular material
        custom pools.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x005DA020
=========================================================*/
static int __cdecl HOOK_InitDeviceSystem( void )
{
    RwPipeline *defPipeline;

    // Pah, might analyze those chunks way later :P
    // I am here because of the pools!
    __asm
    {
        // Initialize pipelines
        mov eax,0x005D9F80
        call eax
        mov defPipeline,eax
    }

    if ( !defPipeline )
        return 0;

    *(RwPipeline**)0x00C02D24 = defPipeline;

    int iReturn;    // did you know that "iRet" is a reserved value in VC++ ASM?

    __asm
    {
        // Initialize device information
        mov eax,0x005D8980
        call eax
        mov iReturn,eax
    }

    if ( !iReturn )
        return 0;

    // Set some NULL pointers
    memset( (void*)0x00C02CB0, 0, sizeof(void*) * 26 );

    // Finally initialize the pools!
    *ppEnvMapMaterialPool = new CEnvMapMaterialPool;
    *ppEnvMapAtomicPool = new CEnvMapAtomicPool;
    *ppSpecMapMaterialPool = new CSpecMapMaterialPool;
    return 1;
}

void RenderWarePipeline_Init( void )
{
    // Depending on game version
    switch( pGame->GetGameVersion() )
    {
    case VERSION_EU_10:
        HookInstall( 0x007FC310, (DWORD)HOOK_RwD3D9SetRenderState, 5 );
        HookInstall( 0x007FC360, (DWORD)HOOK_RwD3D9GetRenderState, 5 );
        HookInstall( 0x007FC380, (DWORD)RwD3D9SetTextureStageState, 5 );
        HookInstall( 0x007FC3E0, (DWORD)RwD3D9GetTextureStageState, 5 );
        HookInstall( 0x007FC240, (DWORD)RwD3D9ApplyDeviceStates, 5 );
        break;
    case VERSION_US_10:
        HookInstall( 0x007FC2D0, (DWORD)HOOK_RwD3D9SetRenderState, 5 );
        HookInstall( 0x007FC320, (DWORD)HOOK_RwD3D9GetRenderState, 5 );
        HookInstall( 0x007FC340, (DWORD)RwD3D9SetTextureStageState, 5 );
        HookInstall( 0x007FC3A0, (DWORD)RwD3D9GetTextureStageState, 5 );
        HookInstall( 0x007FC200, (DWORD)RwD3D9ApplyDeviceStates, 5 );
        break;
    }

    HookInstall( 0x005DA020, (DWORD)HOOK_InitDeviceSystem, 5 );
}

void RenderWarePipeline_Shutdown( void )
{
}