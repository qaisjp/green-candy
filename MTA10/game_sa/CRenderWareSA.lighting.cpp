/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.lighting.cpp
*  PURPOSE:     GTA:SA lighting management
*               Extended to properly support dynamic lighting
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "RenderWare/RwRenderTools.hxx"

//todo: implement a light state management so that GTA:SA light settings are seperate from MTA light settings.
// then we can preserve the original functionality of GTA:SA -> things will still look the same by default.
// interesting idea: engineLightingChooseMethod( "shader" ) -> will use AngerMAN's pointlighting instead of Direct3D 9
// model lighting for GTA:SA too.

// Localized variables.
namespace D3D9Lighting
{
    static unsigned int maxNumberOfActiveLights = 0;                // Binary offsets: (1.0 US and 1.0 EU): 0x00C926B4

    static lightState curLightState;
    static lightIndexArray swap_activeGlobalLights;

    static nativeLightInfoArray deviceLightInfo;

    lightState::lightState( void )
    {
        // Initialize members.
        incrementalLightIndex = 0;

        // Initialize light structs (so only minimal initialization will be required during runtime).
        // Directional lights.
        D3DLIGHT9& dirLight = dirLightStruct;

        dirLight.Type = D3DLIGHT_DIRECTIONAL;
        dirLight.Diffuse.r = 0.0f;
        dirLight.Diffuse.g = 0.0f;
        dirLight.Diffuse.b = 0.0f;
        dirLight.Diffuse.a = 1.0f;

        dirLight.Direction.x = 0.0f;
        dirLight.Direction.y = 0.0f;
        dirLight.Direction.z = 0.0f;

        dirLight.Specular.r = 0.0f;
        dirLight.Specular.g = 0.0f;
        dirLight.Specular.b = 0.0f;
        dirLight.Specular.a = 1.0f;

        dirLight.Ambient.r = 0.0f;
        dirLight.Ambient.g = 0.0f;
        dirLight.Ambient.b = 0.0f;
        dirLight.Ambient.a = 1.0f;

        dirLight.Position.x = 0.0f;
        dirLight.Position.y = 0.0f;
        dirLight.Position.z = 0.0f;

        dirLight.Range = 0.0f;
        dirLight.Falloff = 0.0f;
        dirLight.Attenuation0 = 0.0f;
        dirLight.Attenuation1 = 0.0f;
        dirLight.Attenuation2 = 0.0f;
        dirLight.Theta = 0.0f;
        dirLight.Phi = 0.0f;

        // Local lights.
        D3DLIGHT9& localLight = localLightStruct;

        localLight.Diffuse.r = 0.0f;
        localLight.Diffuse.g = 0.0f;
        localLight.Diffuse.b = 0.0f;
        localLight.Diffuse.a = 1.0f;

        localLight.Specular.r = 0.0f;
        localLight.Specular.g = 0.0f;
        localLight.Specular.b = 0.0f;
        localLight.Specular.a = 1.0f;

        localLight.Ambient.r = 0.0f;
        localLight.Ambient.g = 0.0f;
        localLight.Ambient.b = 0.0f;
        localLight.Ambient.a = 1.0f;

        localLight.Attenuation1 = 1.0f;
    }

    lightState::~lightState( void )
    {
        Shutdown();
    }
    
    void lightState::Shutdown( void )
    {
        activeGlobalLights.Shutdown();
        nativeLights.Shutdown();

        incrementalLightIndex = 0;
    }

    bool lightState::SetLight( int lightIndex, const D3DLIGHT9& lightStruct )
    {
        nativeLights.SetItem( lightStruct, lightIndex );
        return true;
    }

    bool lightState::EnableLight( int lightIndex, bool enabled )
    {
        if ( enabled )
        {
            unsigned int foundAt = -1;

            if ( !activeGlobalLights.Find( lightIndex, foundAt ) )
            {
                activeGlobalLights.AddItem( lightIndex );
                return true;
            }

            return false;
        }
        
        return activeGlobalLights.RemoveItem( lightIndex );
    }

    void lightState::ResetActiveLights( void )
    {
        activeGlobalLights.Clear();

        incrementalLightIndex = 0;
    }

    bool lightState::ActivateDirLight( RpLight *light )
    {
        RwFrame *lightParent = light->m_parent;

        if ( !lightParent )
            return false;

        int lightIndex = GetLightIndex( light );

        // Fill the light struct.
        D3DLIGHT9& dirLight = dirLightStruct;
        dirLight.Diffuse.r = light->m_color.r;
        dirLight.Diffuse.g = light->m_color.g;
        dirLight.Diffuse.b = light->m_color.b;

        const RwMatrix& lightPos = lightParent->GetLTM();

        dirLight.Direction.x = lightPos.vUp.fX;
        dirLight.Direction.y = lightPos.vUp.fY;
        dirLight.Direction.z = lightPos.vUp.fZ;

        SetLight( lightIndex, dirLight );

        activeGlobalLights.AddItem( lightIndex );
        return true;
    }

    bool lightState::ActivateLocalLight( RpLight *light )
    {
        RwFrame *lightParent = light->m_parent;

        if ( !lightParent )
            return false;

        int lightIndex = GetLightIndex( light );

        D3DLIGHT9& localLight = localLightStruct;
        localLight.Diffuse.r = light->m_color.r;
        localLight.Diffuse.g = light->m_color.g;
        localLight.Diffuse.b = light->m_color.b;

        const RwMatrix& lightPos = lightParent->GetLTM();

        localLight.Position.x = lightPos.vPos.fX;
        localLight.Position.y = lightPos.vPos.fY;
        localLight.Position.z = lightPos.vPos.fZ;

        float lightRadius = light->m_radius;

        localLight.Range = lightRadius;

        if ( lightRadius <= 0.0f )
            return false;

        localLight.Attenuation0 = light->m_attenuation[0];
        localLight.Attenuation1 = light->m_attenuation[1] / lightRadius;
        localLight.Attenuation2 = light->m_attenuation[2] / ( lightRadius * lightRadius );

        switch( light->m_subtype )
        {
        case LIGHT_TYPE_POINT:
            localLight.Type = D3DLIGHT_POINT;
            localLight.Direction.x = 0;
            localLight.Direction.y = 0;
            localLight.Direction.z = 0;
            localLight.Theta = 0;
            localLight.Phi = 0;
            break;
        case LIGHT_TYPE_SPOT_1:
            localLight.Type = D3DLIGHT_SPOT;
            localLight.Direction.x = lightPos.vUp.fX;
            localLight.Direction.y = lightPos.vUp.fY;
            localLight.Direction.z = lightPos.vUp.fZ;

            {
                float coneAngle = RpLightGetConeAngle( light );
                coneAngle += coneAngle;

                localLight.Theta = ( coneAngle >= M_PI ) ? (float)( coneAngle - 0.001 ) : coneAngle;
                localLight.Phi = coneAngle;
            }
            break;
        case LIGHT_TYPE_SPOT_2:
            localLight.Type = D3DLIGHT_SPOT;
            localLight.Direction.x = lightPos.vUp.fX;
            localLight.Direction.y = lightPos.vUp.fY;
            localLight.Direction.z = lightPos.vUp.fZ;

            localLight.Theta = 0;
            localLight.Phi = RpLightGetConeAngle( light ) * 2;
            break;
        default:
            return false;
        }

        SetLight( lightIndex, localLight );

        activeGlobalLights.AddItem( lightIndex );
        return true;
    }
};

/*=========================================================
    RpD3D9InitializeLightingPlugin

    Purpose:
        Initializes the Direct3D 9 interfacing of the light
        plugin object.
    Binary offsets:
        (1.0 US): 0x00755B90
        (1.0 EU): 0x00755BE0
=========================================================*/
static size_t d3d9lightplg = 0xFFFFFFFF;

static RpLight* __cdecl RpD3D9LightConstructor( RpLight *light, size_t offset )
{
    if ( !light )
        return NULL;

    light->m_lightIndex = -1;
    light->m_attenuation[0] = 1.0;
    light->m_attenuation[1] = 0.0;
    light->m_attenuation[2] = 5.0;
    return light;
}

static RpLight* __cdecl RpD3D9LightDestructor( RpLight *light )
{
    if ( !light )
        return NULL;

    if ( light->m_lightIndex >= 0 )
    {
        light->m_lightIndex = -1;
    }

    return light;
}

static void __cdecl RpD3D9LightCopyConstructor( RpLight *light, const RpLight *srcObj, size_t offset, unsigned int pluginId )
{
    light->m_lightIndex = srcObj->m_lightIndex;
    light->m_attenuation = srcObj->m_attenuation;
}

static int __cdecl RpD3D9InitializeLightingPlugin( void )
{
    d3d9lightplg = RpLightRegisterPlugin( 0x10, 0x505,
        (RpLightPluginConstructor)RpD3D9LightConstructor,
        (RpLightPluginDestructor)RpD3D9LightDestructor,
        (RpLightPluginCopyConstructor)RpD3D9LightCopyConstructor
    );
    return (int)d3d9lightplg >= 0;
}

/*=========================================================
    RpD3D9LightsEqual

    Arguments:
        left - first light for comparison
        right - second light for comparison
    Purpose:
        Compares two lights for logical equality. This function
        is optimized to compare only the necessary fields.
    Binary offsets:
        (1.0 US): 0x007FA8B0
        (1.0 EU): 0x007FA8F0
=========================================================*/
int __cdecl RpD3D9LightsEqual( const D3DLIGHT9& left, const D3DLIGHT9& right )
{
    // A clever compiler optimizes this into lots of branches.
    bool equals = 
        left.Type == right.Type &&
        left.Diffuse.r == right.Diffuse.r &&
        left.Diffuse.g == right.Diffuse.g &&
        left.Diffuse.b == right.Diffuse.b;

    if ( !equals )
        return false;

    switch( left.Type )
    {
    case D3DLIGHT_POINT:
        equals = 
            left.Position.x == right.Position.x &&
            left.Position.y == right.Position.y &&
            left.Position.z == right.Position.z &&
            left.Range == right.Range;

        break;
    case D3DLIGHT_SPOT:
        equals = 
            left.Position.x == right.Position.x &&
            left.Position.y == right.Position.y &&
            left.Position.z == right.Position.z &&
            left.Direction.x == right.Direction.x &&
            left.Direction.y == right.Direction.y &&
            left.Direction.z == right.Direction.z &&
            left.Range == right.Range &&
            left.Theta == right.Theta &&
            left.Phi == right.Phi;

        break;
    case D3DLIGHT_DIRECTIONAL:
        equals = 
            left.Direction.x == right.Direction.x &&
            left.Direction.y == right.Direction.y &&
            left.Direction.z == right.Direction.z;

        break;
    default:
        equals = true;
    }

    return equals;
}

bool _SetLightGlobal( int lightIndex, const D3DLIGHT9& lightStruct )
{
    if ( D3D9Lighting::deviceLightInfo.GetSizeCount() <= lightIndex )
    {
        D3D9Lighting::deviceLightInfo.SetSizeCount( lightIndex + 1 );
    }
    else
    {
        nativeLightInfo& info = D3D9Lighting::deviceLightInfo.Get( lightIndex );

        if ( RpD3D9LightsEqual( lightStruct, info.native ) )
            return true;
    }

    nativeLightInfo& info = D3D9Lighting::deviceLightInfo.Get( lightIndex );
    info.native = lightStruct;

    return GetRenderDevice()->SetLight( lightIndex, &lightStruct ) >= 0;
}

bool _LightEnableGlobal( int lightIndex, bool enable )
{
    // Ignore invalid light indice.
    if ( lightIndex >= D3D9Lighting::deviceLightInfo.GetSizeCount() )
        return true;

    nativeLightInfo& lightInfo = D3D9Lighting::deviceLightInfo.Get( lightIndex );

    DWORD status = ( enable ? 1 : 0 );

    if ( lightInfo.active != status )
    {
        lightInfo.active = enable;
        return GetRenderDevice()->LightEnable( lightIndex,status ) >= 0;
    }
    return true;
}

// Manager for rendering light states.
struct lightPassManager
{
    lightPassManager( D3D9Lighting::lightState& state, CShaderItem *useShader = NULL ) : light_state( state )
    {
        inPhase = false;
        lightShader = useShader;
        inPass = false;
    }

    bool SetLight( int lightIndex, const D3DLIGHT9& lightStruct )
    {
        if ( !inPhase )
            return light_state.SetLight( lightIndex, lightStruct );

        return _SetLightGlobal( lightIndex, lightStruct );
    }

    bool EnableLight( int lightIndex, bool enable )
    {
        if ( !inPhase )
        {
            return light_state.EnableLight( lightIndex, enable );
        }
        else
        {
            return _LightEnableGlobal( lightIndex, enable );
        }
    }

    void Begin( void )
    {
        assert( global_inPhase == false );
        assert( inPhase == false );

        inPhase = true;
        global_inPhase = true;

        // Make sure we only activate the number of maximally supported lights on this GPU.
        unsigned int maxGPULights = D3D9Lighting::maxNumberOfActiveLights;

        D3D9Lighting::swap_activeGlobalLights.TrimTo( maxGPULights );
        
        unsigned int lastActiveGlobalLightsCount = D3D9Lighting::swap_activeGlobalLights.GetCount();

        for ( unsigned int n = 0; n < lastActiveGlobalLightsCount; n++ )
        {
            int lightIndex = D3D9Lighting::swap_activeGlobalLights.Get( n );

            // Check whether we should disable this light.
            // It is either disabled if it does not exist in the active lights queue
            // or has an index that is outside of the GPUs maximum light index
            // count.
            unsigned int primaryIndex;

            if ( !light_state.activeGlobalLights.Find( lightIndex, primaryIndex ) || primaryIndex >= maxGPULights )
            {
                // Disable the light.
                EnableLight( lightIndex, false );
            }
        }

        index = 0;

        // Activate the lighting shader (if present)
        if ( lightShader )
        {
            lightShader->Begin( numLightShaderPasses );
            
            core->GetGraphics()->GetRenderItemManager()->SetForceShader( lightShader );
        }
    }

    void ClearPass( void )
    {
        if ( inPass )
        {
            if ( lightShader )
                lightShader->EndPass();

            inPass = false;
        }
    }

    bool DoLightPass( void )
    {
        assert( inPhase == true );

        ClearPass();

        if ( lightShader )
            lightShader->BeginPass( 0 );

        bool wantPass = false;
        unsigned int maxGPULights = D3D9Lighting::maxNumberOfActiveLights;

        int n = 0;
        
        for ( ; n < (int)maxGPULights; n++ )
        {
            unsigned int currentIndex = index + (unsigned int)n;

            if ( light_state.activeGlobalLights.GetCount() <= currentIndex )
                break;

            int lightIndex = light_state.activeGlobalLights.Get( currentIndex );
            D3DLIGHT9& lightStruct = light_state.nativeLights.Get( lightIndex );

            if ( lightShader )
            {
                const RwMatrix& camTransform = pRwInterface->m_renderCam->m_parent->GetLTM();

                // Update light shader info.
                CVector lightPos = camTransform.vPos - (CVector&)lightStruct.Position;

                lightShader->SetValue( "gLightPosition", (float*)&lightPos, 3 );
                lightShader->SetValue( "gLightDiffuse", (float*)&lightStruct.Diffuse, 4 );
                lightShader->SetValue( "gLightAttenuation0", &lightStruct.Attenuation0, 1 );
                lightShader->SetValue( "gLightAttenuation1", &lightStruct.Attenuation1, 1 );
                lightShader->SetValue( "gLightAttenuation2", &lightStruct.Attenuation2, 1 );
                lightShader->SetValue( "gLightRange", &lightStruct.Range, 1 );

                wantPass = true;

                n++;
                break;
            }
            else
            {
                if ( SetLight( n, lightStruct ) == 0 || EnableLight( n, true ) == 0 )
                    break;

                wantPass = true;
            }
        }

        index += n;

        if ( wantPass )
        {
            // Disable all stray lights.
            for ( ; n < (int)maxGPULights; n++ )
                EnableLight( n, false );
        }

        inPass = true;

        return wantPass;
    }

    bool IsInPass( void )
    {
        return inPass;
    }

    void End( void )
    {
        ClearPass();

        // Terminate light shader.
        if ( lightShader )
        {
            lightShader->End();

            core->GetGraphics()->GetRenderItemManager()->SetForceShader( NULL );
        }

        light_state.activeGlobalLights.SetContents( D3D9Lighting::swap_activeGlobalLights );

        inPhase = false;
        global_inPhase = false;
    }

    void SetLightingShader( CShaderItem *item )
    {
        assert( inPhase == false );

        lightShader = item;
    }

    D3D9Lighting::lightState& light_state;
    unsigned int index;
    bool inPhase;
    bool inPass;

    CShaderItem *lightShader;
    unsigned int numLightShaderPasses;

    static bool global_inPhase;
};

static lightPassManager globalLightPassMan( D3D9Lighting::curLightState );
bool lightPassManager::global_inPhase = false;

/*=========================================================
    RpD3D9SetLight

    Arguments:
        lightIndex - the index to set the light to
        lightStruct - Direct3D light information
    Purpose:
        Updates lighting information that is pointed to by
        lightIndex. The lighting information is natively pushed
        onto the GPU. The light has to be enabled to be active.
    Binary offsets:
        (1.0 US): 0x007FA660
        (1.0 EU): 0x007FA6A0
=========================================================*/
int __cdecl RpD3D9SetLight( int lightIndex, const D3DLIGHT9& lightStruct )
{
    return globalLightPassMan.SetLight( lightIndex, lightStruct ) ? 1 : 0;
}

/*=========================================================
    RpD3D9DirLightEnable

    Arguments:
        light - RenderWare light plugin object
    Purpose:
        Activates the light that is described by the given
        RenderWare light plugin object. The light object has
        to be a directional light.
    Binary offsets:
        (1.0 US): 0x00756260
        (1.0 EU): 0x007562B0
    Note:
        Deprecated function.
=========================================================*/
int __cdecl RpD3D9DirLightEnable( RpLight *light )
{
    return D3D9Lighting::curLightState.ActivateDirLight( light ) ? 1 : 0;
}

/*=========================================================
    RpD3D9LocalLightEnable

    Arguments:
        light - RenderWare light plugin object
    Purpose:
        Activates the light that is described by the given
        RenderWare light plugin object. The light object has
        to be a spotlight or pointlight.
    Binary offsets:
        (1.0 US): 0x00756390
        (1.0 EU): 0x007563E0
    Note:
        Deprecated function.
=========================================================*/
int __cdecl RpD3D9LocalLightEnable( RpLight *light )
{
    return D3D9Lighting::curLightState.ActivateLocalLight( light ) ? 1 : 0;
}

/*=========================================================
    RpD3D9EnableLight

    Arguments:
        lightIndex - light index to enable
        enable - boolean whether to enable the light or not
    Purpose:
        Turns on or off the light pointed at by lightIndex
        for the next rendering operations. For lights to work,
        the lighting renderstate has to be activated. The
        light at lightIndex has to be previously initialized
        by RpD3D9SetLight.
    Binary offsets:
        (1.0 US): 0x007FA860
        (1.0 EU): 0x007FA8A0
=========================================================*/
int __cdecl RpD3D9EnableLight( int lightIndex, int enable )
{
    return globalLightPassMan.EnableLight( lightIndex, enable != 0 ) ? 1 : 0;
}

/*=========================================================
    RpD3D9EnableLights

    Arguments:
        enable - boolean whether to enable or disable the lights
        unused - unknown parameter
    Purpose:
        Enables or disables all local and global lights for
        the next rendering operations. Makes sure that only
        lights that have been activated this frame are
        showing.
    Binary offsets:
        (1.0 US): 0x00756600
        (1.0 EU): 0x00756650
=========================================================*/
void __cdecl RpD3D9EnableLights( bool enable, int unused )
{
#if 0
    // This function is part of the problem, too.
    // It trims the lights to the maximum light count as supported by the GPU.
    // This is wrong, because there is an infinite amount of lights supported,
    // using additive blending. Shame that Rockstar did not think of that.

    if ( enable )
    {
        // Make sure we only activate the number of maximally supported lights on this GPU.
        D3D9Lighting::activeGlobalLights.TrimTo( D3D9Lighting::maxNumberOfActiveLights );

        unsigned int lastActiveGlobalLightsCount = D3D9Lighting::swap_activeGlobalLights.GetCount();

        for ( unsigned int n = 0; n < lastActiveGlobalLightsCount; n++ )
        {
            int lightIndex = D3D9Lighting::swap_activeGlobalLights.Get( n );

            // Check whether we should disable this light.
            // It is either disabled if it does not exist in the active lights queue
            // or has an index that is outside of the GPUs maximum light index
            // count.
            unsigned int primaryIndex;

            if ( !D3D9Lighting::activeGlobalLights.Find( lightIndex, primaryIndex ) || primaryIndex >= D3D9Lighting::activeGlobalLights.GetCount() )
            {
                // Disable the light.
                RpD3D9EnableLight( lightIndex, false );
            }
        }

        // If there is anything to enable, do it.
        if ( unsigned int activeCount = D3D9Lighting::activeGlobalLights.GetCount() )
        {
            for ( unsigned int n = 0; n < activeCount; n++ )
            {
                RpD3D9EnableLight( D3D9Lighting::activeGlobalLights.Get( n ), true );
            }

            D3D9Lighting::activeGlobalLights.SwapContents( D3D9Lighting::swap_activeGlobalLights );
        }
        else
            D3D9Lighting::swap_activeGlobalLights.Clear();

        D3D9Lighting::activeGlobalLights.Clear();
    }

    HOOK_RwD3D9SetRenderState( D3DRS_LIGHTING, enable );
#endif
}

/*=========================================================
    RwD3D9GlobalLightsEnable

    Arguments:
        flags - flags to check at the global lights
    Purpose:
        Prepares all global lights for the next rendering pass.
        When ambient lights are detected, their color values
        are added to the ambient light vector. Returns a boolean
        whether lighting has been enabled.
    Binary offsets:
        (1.0 US): 0x00757400
        (1.0 EU): 0x00757450
=========================================================*/
static RwColorFloat& GetAmbientColor( void )
{
    return *(RwColorFloat*)0x008E2418;
}

template <typename lightMan>
int _GlobalLightsEnable( D3D9Lighting::lightState& state, lightMan& cb )
{
    RwColorFloat& ambientColor = state.ambientLightColor;
    RwScene *curScene = pRwInterface->m_currentScene;

    ambientColor = RwColorFloat( 0, 0, 0, 1.0f );

    bool isLighting = false;

    LIST_FOREACH_BEGIN( RpLight, curScene->m_globalLights.root, m_sceneLights )
        if ( cb.CanProcessLight( item ) )
        {
            switch( item->m_subtype )
            {
            case LIGHT_TYPE_DIRECTIONAL:
                if ( cb.CanProcessDirectionalLight() )
                {
                    if ( state.ActivateDirLight( item ) != FALSE )
                        isLighting = true;
                }
                break;
            default:
                ambientColor.r += item->m_color.r;
                ambientColor.g += item->m_color.g;
                ambientColor.b += item->m_color.b;

                isLighting = true;
                break;
            }
        }
    LIST_FOREACH_END

    //HOOK_RwD3D9SetRenderState( D3DRS_AMBIENT, ( isLighting ) ? 0xFFFFFFFF : 0 );
    return isLighting;
}

struct AtomicGlobalLightManager
{
    inline AtomicGlobalLightManager( RpGeometry*& _geom ) : m_geom( _geom )
    { }

    inline bool CanProcessLight( RpLight *globLight )
    {
        return globLight->IsLightActive();
    }

    inline bool CanProcessDirectionalLight( void )
    {
        return IS_ANY_FLAG( m_geom->flags, 0x10 );
    }

    RpGeometry*& m_geom;
};

struct WorldGlobalLightManager
{
    inline WorldGlobalLightManager( unsigned int flags ) : lightFlags( flags )
    { }

    inline bool CanProcessLight( RpLight *globLight )
    {
        return IS_ANY_FLAG( globLight->m_flags, lightFlags );
    }

    inline bool CanProcessDirectionalLight( void )
    {
        return true;
    }

    unsigned int lightFlags;
};

int __cdecl RpD3D9GlobalLightsEnable( unsigned char flags )
{
    return _GlobalLightsEnable( D3D9Lighting::curLightState, WorldGlobalLightManager( flags ) );
}

/*=========================================================
    HOOK_DefaultAtomicLightingCallback

    Arguments:
        atomic - RenderWare video data that is about to be rendered
    Purpose:
        Prepares the lighting situation for the given atomic
        object. All global lights are enabled for it. All
        local lights in its proximity are enabled too.
    Binary offsets:
        (1.0 US): 0x00757400
        (1.0 EU): 0x00757450
=========================================================*/
static D3D9Lighting::lightState localLight_lightState;  // MTA extension.
static bool hasGlobalLighting = false;
static bool hasLocalLighting = false;

void __cdecl HOOK_DefaultAtomicLightingCallback( RpAtomic *atomic )
{
    RpGeometry *geom = atomic->m_geometry;
    RwScene *curScene = pRwInterface->m_currentScene;

    RpD3D9ResetLightStatus();

    // Reset light states.
    D3D9Lighting::curLightState.ResetActiveLights();
    localLight_lightState.ResetActiveLights();

    if ( IS_ANY_FLAG( geom->flags, 0x20 ) && curScene )
    {
        // Process main light-state.
        D3D9Lighting::curLightState.ResetActiveLights();

        hasGlobalLighting = _GlobalLightsEnable( D3D9Lighting::curLightState, AtomicGlobalLightManager( geom ) ) != 0;

        // Set the global ambient light color to the one specified by the native light-state.
        GetAmbientColor() = D3D9Lighting::curLightState.ambientLightColor;

        // Activate all local lights on a seperate light-state.
        if ( IS_ANY_FLAG( geom->flags, 0x10 ) )
        {
            D3D9Lighting::lightState& localLightState = localLight_lightState;
            localLightState.ResetActiveLights();

            // Scancode algorithm, so lights do not get processed twice.
            pRwInterface->m_frame++;

            LIST_FOREACH_BEGIN( RpAtomic::sectorNode, atomic->sectors.root, node )
                RwSector *sector = item->data;

                LIST_FOREACH_BEGIN( RwSector::lightNode, sector->m_localLights.root, node )
                    RpLight *light = item->data;

                    if ( light && light->m_frame != pRwInterface->m_frame && light->IsLightActive() )
                    {
                        light->m_frame = pRwInterface->m_frame;

                        const RwMatrix& lightPos = light->m_parent->GetLTM();
                        const RwSphere& atomicSphere = atomic->GetWorldBoundingSphere();

                        float lightSphereActivityRange = light->m_radius + atomicSphere.radius;

                        lightSphereActivityRange *= lightSphereActivityRange;

                        if ( ( atomicSphere.pos - lightPos.vPos ).LengthSquared() <= lightSphereActivityRange )
                        {
                            if ( localLightState.ActivateLocalLight( light ) )
                                hasLocalLighting = true;
                        }
                    }
                LIST_FOREACH_END
            LIST_FOREACH_END
        }
    }

    bool doLighting = RpD3D9GlobalLightingPrePass();

    HOOK_RwD3D9SetRenderState( D3DRS_AMBIENT, ( doLighting ) ? 0xFFFFFFFF : 0x00000000 );
}

bool RpD3D9GlobalLightingPrePass( void )
{
    bool doLighting = false;

    if ( hasGlobalLighting )
    {
        if ( globalLightPassMan.inPhase )
            doLighting = true;
        else
        {
            // Do the first pass of the global light pass manager here.
            globalLightPassMan.Begin();

            doLighting = globalLightPassMan.DoLightPass();

            if ( !doLighting )
                globalLightPassMan.End();
        }
    }

    HOOK_RwD3D9SetRenderState( D3DRS_LIGHTING, doLighting );
    return doLighting;
}

/*=========================================================
    RpD3D9RenderLightMeshForPass (MTA extension)

    Arguments:
        rtPass - the render pass to render a light mesh off
    Purpose:
        Renders the light mesh associated to the given render
        pass using additive blending. Should be executed after
        a mesh has been rendered.
=========================================================*/
static CShaderItem *lightingShader = NULL;  // todo: one shader for every light type.

void RpD3D9RenderLightMeshForPass( RwRenderCallbackTraverseImpl *rtinfo, RwRenderPass *rtPass )
{
    bool enableLighting = ( hasLocalLighting || hasGlobalLighting );

    if ( enableLighting )
    {
        RwRenderStateLock lightState( D3DRS_LIGHTING, true );
        RwRenderStateLock srcBlend( D3DRS_SRCBLEND, D3DBLEND_ONE );
        RwRenderStateLock dstBlend( D3DRS_DESTBLEND, D3DBLEND_ONE );
        RwRenderStateLock alphaRender( D3DRS_ALPHABLENDENABLE, true );
        RwRenderStateLock fog( D3DRS_FOGENABLE, false );
        RwRenderStateLock zwrite( D3DRS_ZWRITEENABLE, false );
        RwRenderStateLock zfunc( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
        RwRenderStateLock diffusematsrc( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
        RwRenderStateLock specular( D3DRS_SPECULARENABLE, false );
        RwRenderStateLock ambient( D3DRS_AMBIENT, 0x00000000 );
        {
            // Finish the global lights render.
            if ( globalLightPassMan.inPhase || hasGlobalLighting )
            {
                if ( !globalLightPassMan.inPhase )
                    globalLightPassMan.Begin();

                while ( globalLightPassMan.DoLightPass() )
                {
#if 0
                    // Todo: fix white texture issues appearing because of this.
                    RwD3D9SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
                    RwD3D9SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
                    RwD3D9SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );

                    RwD3D9SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
                    RwD3D9SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

                    RwD3D9SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
                    RwD3D9SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
#endif

                    // Render as often as we require light meshes.
                    RwD3D9DrawRenderPassPrimitive( rtinfo, rtPass );
                }

                globalLightPassMan.End();
            }

            if ( hasLocalLighting )
            {
                D3D9Lighting::lightState& state = localLight_lightState;

                lightPassManager lightMan( state, lightingShader );

                lightMan.Begin();

                while ( lightMan.DoLightPass() )
                {
                    // Render as often as we require light meshes.
                    RwD3D9DrawRenderPassPrimitive( rtinfo, rtPass );
                }

                lightMan.End();
            }
        }
    }
}

/*=========================================================
    RpD3D9ResetLightStatus (MTA extension)

    Purpose:
        Resets the global light status.
=========================================================*/
void __cdecl RpD3D9ResetLightStatus( void )
{
    // Make sure we terminate any global pass that did not have a light mesh render.
    if ( globalLightPassMan.inPhase )
        globalLightPassMan.End();

    hasGlobalLighting = false;
    hasLocalLighting = false;

    HOOK_RwD3D9SetRenderState( D3DRS_LIGHTING, false );
}

/*=========================================================
    RpD3D9ShutdownLighting

    Purpose:
        Deallocates all data that has been mantained by the
        RenderWare Direct3D 9 light plugin object interfacing.
    Binary offsets:
        (1.0 US): 0x00755FE0
        (1.0 EU): 0x00756030
=========================================================*/
void __cdecl RpD3D9ShutdownLighting( void )
{
    D3D9Lighting::curLightState.Shutdown();

    D3D9Lighting::deviceLightInfo.Shutdown();

    D3D9Lighting::swap_activeGlobalLights.Shutdown();
}

/*=========================================================
    RpD3D9InitializeLighting

    Purpose:
        Initializes the Direct3D 9 light plugin object interfacing
        for this RenderWare session. This clears any data that
        might have been left over from a previous RenderWare
        session.
    Binary offsets:
        (1.0 US): 0x00755D80
        (1.0 EU): 0x00755DD0
=========================================================*/
int __cdecl RpD3D9InitializeLighting( void )
{
    // Clear activity status.
    RpD3D9ShutdownLighting();

    unsigned int maxGPULights = pRwDeviceInfo->maxLights;

    D3D9Lighting::maxNumberOfActiveLights = ( maxGPULights != 0 ) ? maxGPULights : 0xFFFFFFFF;

    // Set initial ambient light color.
    GetAmbientColor() = RwColorFloat( 0, 0, 0, 1.0f );
    return true;
}

void RenderWareLighting_Init( void )
{
    // Hook the Direct3D 9 lighting API.
    switch( pGame->GetGameVersion() )
    {
    case VERSION_EU_10:
        HookInstall( 0x00756030, (DWORD)RpD3D9ShutdownLighting, 5 );
        HookInstall( 0x00755DD0, (DWORD)RpD3D9InitializeLighting, 5 );
        HookInstall( 0x00755BE0, (DWORD)RpD3D9InitializeLightingPlugin, 5 );
        HookInstall( 0x007FA8F0, (DWORD)RpD3D9LightsEqual, 5 );
        HookInstall( 0x007FA6A0, (DWORD)RpD3D9SetLight, 5 );
        HookInstall( 0x007562B0, (DWORD)RpD3D9DirLightEnable, 5 );
        HookInstall( 0x007563E0, (DWORD)RpD3D9LocalLightEnable, 5 );
        HookInstall( 0x007FA8A0, (DWORD)RpD3D9EnableLight, 5 );
        HookInstall( 0x00757450, (DWORD)RpD3D9GlobalLightsEnable, 5 );
        HookInstall( 0x00756650, (DWORD)RpD3D9EnableLights, 5 );
        HookInstall( 0x00757450, (DWORD)HOOK_DefaultAtomicLightingCallback, 5 );
        break;
    case VERSION_US_10:
        HookInstall( 0x00755FE0, (DWORD)RpD3D9ShutdownLighting, 5 );
        HookInstall( 0x00755D80, (DWORD)RpD3D9InitializeLighting, 5 );
        HookInstall( 0x00755B90, (DWORD)RpD3D9InitializeLightingPlugin, 5 );
        HookInstall( 0x007FA8B0, (DWORD)RpD3D9LightsEqual, 5 );
        HookInstall( 0x007FA660, (DWORD)RpD3D9SetLight, 5 );
        HookInstall( 0x00756260, (DWORD)RpD3D9DirLightEnable, 5 );
        HookInstall( 0x00756390, (DWORD)RpD3D9LocalLightEnable, 5 );
        HookInstall( 0x007FA860, (DWORD)RpD3D9EnableLight, 5 );
        HookInstall( 0x00757400, (DWORD)RpD3D9GlobalLightsEnable, 5 );
        HookInstall( 0x00756600, (DWORD)RpD3D9EnableLights, 5 );
        HookInstall( 0x00757400, (DWORD)HOOK_DefaultAtomicLightingCallback, 5 );
        break;
    }
}

void RenderWareLighting_Shutdown( void )
{
    
}

void RenderWareLighting_InitShaders( void )
{
    // Initialize lighting shader.
    CFileTranslator *mtaFileRoot = core->GetDataRoot();

    filePath shaderPath;
    mtaFileRoot->GetFullPath( "/shaders/lighting.fx", true, shaderPath );

    SString outStatus;

    lightingShader = core->GetGraphics()->GetRenderItemManager()->CreateShader( shaderPath, shaderPath, outStatus, 1.0, 0.0, true );

    if ( !lightingShader )
        core->GetConsole()->Print( outStatus );
}

void RenderWareLighting_ResetShaders( void )
{
    RenderWareLighting_ShutdownShaders();
    RenderWareLighting_InitShaders();
}

void RenderWareLighting_ShutdownShaders( void )
{
    // todo.
    if ( lightingShader )
        lightingShader->Release();

    lightingShader = NULL;
}