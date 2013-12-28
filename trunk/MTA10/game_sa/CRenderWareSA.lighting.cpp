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

struct nativeLightInfo  //size: 108 bytes
{
    D3DLIGHT9   native;     // 0
    int         active;     // 104

    inline bool operator == ( const nativeLightInfo& right ) const
    {
        return RpD3D9LightsEqual( native, right.native ) != 0;
    }
};

template <typename dataType, unsigned int pulseCount, unsigned int allocFlags, typename arrayMan, typename countType>
struct growableArray
{
    inline growableArray( void )
    {
        data = NULL;
        numActiveEntries = 0;
        sizeCount = 0;
    }

    inline ~growableArray( void )
    {
        Shutdown();
    }

    inline void Init( void )
    { }

    inline void Shutdown( void )
    {
        if ( data )
        {
            pRwInterface->m_memory.m_free( data );
            
            data = NULL;
        }

        numActiveEntries = 0;
        sizeCount = 0;
    }

    inline void SetSizeCount( countType index )
    {
        countType oldCount = sizeCount;

        sizeCount = index;

        size_t newArraySize = sizeCount * sizeof( dataType );

        if ( !data )
            data = (dataType*)pRwInterface->m_memory.m_malloc( newArraySize, allocFlags );
        else
            data = (dataType*)pRwInterface->m_memory.m_realloc( data, newArraySize, allocFlags );

        // Fill the gap.
        for ( countType n = oldCount; n < sizeCount; n++ )
        {
            manager.InitField( data[n] );
        }
    }

    inline void SetItem( dataType dataField, countType index )
    {
        if ( index >= sizeCount )
        {
            SetSizeCount( sizeCount + pulseCount );
        }

        data[index] = dataField;
    }

    inline void AddItem( dataType data )
    {
        SetItem( data, numActiveEntries );

        numActiveEntries++;
    }

    inline countType GetCount( void ) const
    {
        return numActiveEntries;
    }

    inline countType GetSizeCount( void ) const
    {
        return sizeCount;
    }

    inline dataType& Get( countType index )
    {
        return data[index];
    }

    inline bool Pop( dataType& item )
    {
        if ( numActiveEntries != 0 )
        {
            item = data[--numActiveEntries];
            return true;
        }

        return false;
    }

    inline bool Find( const dataType& inst, countType& indexOut ) const
    {
        for ( countType n = 0; n < numActiveEntries; n++ )
        {
            if ( data[n] == inst )
            {
                indexOut = n;
                return true;
            }
        }

        return false;
    }

    inline void Clear( void )
    {
        numActiveEntries = 0;
    }

    inline void TrimTo( countType indexTo )
    {
        if ( numActiveEntries > indexTo )
            numActiveEntries = indexTo;
    }

    inline void SwapContents( growableArray& right )
    {
        dataType *myData = this->data;
        dataType *swapData = right.data;

        this->data = swapData;
        right.data = myData;

        countType myActiveCount = this->numActiveEntries;
        countType swapActiveCount = right.numActiveEntries;

        this->numActiveEntries = swapActiveCount;
        right.numActiveEntries = myActiveCount;

        countType mySizeCount = this->sizeCount;
        countType swapSizeCount = right.sizeCount;

        this->sizeCount = swapSizeCount;
        right.sizeCount = mySizeCount;
    }

    dataType* data;
    countType numActiveEntries;
    countType sizeCount;
    arrayMan manager;
};

// Localized variables.
namespace D3D9Lighting
{
    struct lightIndexArrayManager
    {
        inline void InitField( int& lightIndex )
        {
            lightIndex = 0;
        }
    };

    struct availableLightIndexArrayManager
    {
        inline void InitField( int& lightIndex )
        {
            return;
        }
    };

    struct nativeLightInfoArrayManager
    {
        inline void InitField( nativeLightInfo& info )
        {
            // By default, lights get added to phase 0.
            // Phase 0 sets them to "not rendered" for their first cycle occurance.
            // This phase index describes on which list (primary or swap) the light
            // resides on. Sort of like a scan-code algorithm (as seen in the
            // entity world streaming, CEntitySAInterface::m_scanCode).
            info.active = false;
        }
    };

    typedef growableArray <int, 8, 0x103050D, lightIndexArrayManager, unsigned int> lightIndexArray;
    typedef growableArray <int, 0x100, 0x103050D, availableLightIndexArrayManager, int> availableLightIndexArray;
    typedef growableArray <nativeLightInfo, 8, 0x1030411, nativeLightInfoArrayManager, int> nativeLightInfoArray;

    static int incrementalLightIndex = 0;                           // Binary offsets: (1.0 US and 1.0 EU): 0x00C926C0

    static D3DLIGHT9 dirLightStruct;                                // Binary offsets: (1.0 US and 1.0 EU): 0x00C92648
    static D3DLIGHT9 localLightStruct;                              // Binary offsets: (1.0 US and 1.0 EU): 0x00C925E0

    static unsigned int maxNumberOfActiveLights = 0;                // Binary offsets: (1.0 US and 1.0 EU): 0x00C926B4

    static lightIndexArray activeGlobalLights;
    static lightIndexArray swap_activeGlobalLights;

    static availableLightIndexArray availableLightIndice;

    static nativeLightInfoArray nativeLights;
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

RpLight* __cdecl RpD3D9LightConstructor( RpLight *light, size_t offset )
{
    if ( !light )
        return NULL;

    light->m_lightIndex = -1;
    light->m_attenuation[0] = 1.0;
    light->m_attenuation[1] = 0.0;
    light->m_attenuation[2] = 5.0;
    return light;
}

inline void AddLightIndexToAvailableLightSlots( int lightIndex )
{
    D3D9Lighting::availableLightIndice.AddItem( lightIndex );
}

RpLight* __cdecl RpD3D9LightDestructor( RpLight *light )
{
    if ( !light )
        return NULL;

    if ( light->m_lightIndex >= 0 )
    {
        int incrementalIndex = D3D9Lighting::incrementalLightIndex - 1;

        if ( D3D9Lighting::availableLightIndice.GetCount() >= incrementalIndex )
        {
            D3D9Lighting::incrementalLightIndex = 0;

            D3D9Lighting::availableLightIndice.Shutdown();
        }
        else
        {
            AddLightIndexToAvailableLightSlots( light->m_lightIndex );
        }
    }

    return light;
}

void __cdecl RpD3D9LightCopyConstructor( RpLight *light, const RpLight *srcObj, size_t offset, unsigned int pluginId )
{
    light->m_lightIndex = srcObj->m_lightIndex;
    light->m_attenuation = srcObj->m_attenuation;
}

int __cdecl RpD3D9InitializeLightingPlugin( void )
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
    if ( D3D9Lighting::nativeLights.GetSizeCount() <= lightIndex )
    {
        D3D9Lighting::nativeLights.SetSizeCount( lightIndex + 1 );
    }
    else
    {
        nativeLightInfo& info = D3D9Lighting::nativeLights.Get( lightIndex );

        if ( RpD3D9LightsEqual( lightStruct, info.native ) )
            return true;
    }

    nativeLightInfo& info = D3D9Lighting::nativeLights.Get( lightIndex );
    info.native = lightStruct;

    // This should not be done. Will fail if lightIndex is too big anyway.
    // This is where the lighting bugs come from.
    // Should be replaced by a callback-orriented lighting management.
    return GetRenderDevice()->SetLight( lightIndex, &lightStruct ) >= 0;
}

inline void AssignLightIndex( RpLight *light )
{
    if ( light->m_lightIndex < 0 )
    {
        int slot = -1;

        if ( !D3D9Lighting::availableLightIndice.Pop( slot ) )
        {
            slot = D3D9Lighting::incrementalLightIndex++;
        }

        light->m_lightIndex = slot;
    }
}

static D3DLIGHT9& GetDirLightStruct( void )
{
    return D3D9Lighting::dirLightStruct;
}

inline void AddLightToGlobalList( RpLight *light )
{
    D3D9Lighting::activeGlobalLights.AddItem( light->m_lightIndex );
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
=========================================================*/
int __cdecl RpD3D9DirLightEnable( RpLight *light )
{
    AssignLightIndex( light );

    // Fill the light struct.
    D3DLIGHT9& dirLight = GetDirLightStruct();
    dirLight.Diffuse.r = light->m_color.r;
    dirLight.Diffuse.g = light->m_color.g;
    dirLight.Diffuse.b = light->m_color.b;

    const RwMatrix& lightPos = light->m_parent->GetLTM();

    dirLight.Direction.x = lightPos.vUp.fX;
    dirLight.Direction.y = lightPos.vUp.fY;
    dirLight.Direction.z = lightPos.vUp.fZ;

    RpD3D9SetLight( light->m_lightIndex, dirLight );

    AddLightToGlobalList( light );
    return true;
}

static D3DLIGHT9& GetLocalLightStruct( void )
{
    return D3D9Lighting::localLightStruct;
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
=========================================================*/
int __cdecl RpD3D9LocalLightEnable( RpLight *light )
{
    AssignLightIndex( light );

    D3DLIGHT9& localLight = GetLocalLightStruct();
    localLight.Diffuse.r = light->m_color.r;
    localLight.Diffuse.g = light->m_color.g;
    localLight.Diffuse.b = light->m_color.b;

    const RwMatrix& lightPos = light->m_parent->GetLTM();

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

    RpD3D9SetLight( light->m_lightIndex, localLight );

    AddLightToGlobalList( light );
    return true;
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
    // Ignore invalid light indices.
    if ( lightIndex >= D3D9Lighting::nativeLights.GetSizeCount() )
        return true;

    nativeLightInfo& lightInfo = D3D9Lighting::nativeLights.Get( lightIndex );

    if ( lightInfo.active != enable )
    {
        lightInfo.active = enable;
        return GetRenderDevice()->LightEnable( lightIndex, enable ) >= 0;
    }
    return true;
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
int _GlobalLightsEnable( lightMan& cb )
{
    RwColorFloat& ambientColor = GetAmbientColor();
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
                    if ( RpD3D9DirLightEnable( item ) != FALSE )
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

    HOOK_RwD3D9SetRenderState( D3DRS_AMBIENT, ( isLighting ) ? 0xFFFFFFFF : 0 );
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
    return _GlobalLightsEnable( WorldGlobalLightManager( flags ) );
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
void __cdecl HOOK_DefaultAtomicLightingCallback( RpAtomic *atomic )
{
    RpGeometry *geom = atomic->m_geometry;
    RwScene *curScene = pRwInterface->m_currentScene;

    bool isLighting = false;

    if ( IS_ANY_FLAG( geom->flags, 0x20 ) && curScene )
    {
        isLighting = _GlobalLightsEnable( AtomicGlobalLightManager( geom ) ) != 0;

        if ( IS_ANY_FLAG( geom->flags, 0x10 ) )
        {
            // Scancode algorithm, so lights do not get processed twice.
            pRwInterface->m_frame++;

            LIST_FOREACH_BEGIN( RpAtomic::sectorNode, atomic->sectors.root, node )
                RwSector *sector = item->data;

                LIST_FOREACH_BEGIN( RwSector::lightNode, sector->m_localLights.root, node )
                    RpLight *light = item->data;

                    if ( light && light->m_frame != pRwInterface->m_frame && light->IsLightActive() )
                    {
                        const RwMatrix& lightPos = light->m_parent->GetLTM();
                        const RwSphere& atomicSphere = atomic->GetWorldBoundingSphere();

                        float lightSphereActivityRange = light->m_radius + atomicSphere.radius;

                        lightSphereActivityRange *= lightSphereActivityRange;

                        if ( ( atomicSphere.pos - lightPos.vPos ).LengthSquared() <= lightSphereActivityRange )
                        {
                            if ( RpD3D9LocalLightEnable( light ) )
                                isLighting = true;
                        }
                    }
                LIST_FOREACH_END
            LIST_FOREACH_END
        }
    }

    // Push the lights onto the GPU.
    RpD3D9EnableLights( isLighting, 1 );
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
    D3D9Lighting::availableLightIndice.Shutdown();
    D3D9Lighting::incrementalLightIndex = 0;

    D3D9Lighting::activeGlobalLights.Shutdown();

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

    // Initialize light structs (so only minimal initialization will be required during runtime).
    D3DLIGHT9& dirLight = GetDirLightStruct();

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

    D3DLIGHT9& localLight = GetLocalLightStruct();

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