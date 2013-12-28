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
};

// Localized variables.
namespace D3D9Lighting
{
    static unsigned int countOfAvailableLightSlots = 0;             // Binary offsets: (1.0 US and 1.0 EU): 0x00C926BC
    static unsigned int numberOfAvailableLightSlots = 0;            // Binary offsets: (1.0 US and 1.0 EU): 0x00C926B8

    static int* availableLightSlots = NULL;                         // Binary offsets: (1.0 US and 1.0 EU): 0x00C926C4

    static int incrementalLightIndex = 0;                           // Binary offsets: (1.0 US and 1.0 EU): 0x00C926C0

    static nativeLightInfo* nativeLightArray = NULL;                // Binary offsets: (1.0 US and 1.0 EU): 0x00C98088

    static int countOfNativeLightArray = 0;                         // Binary offsets: (1.0 US and 1.0 EU): 0x00C98084

    static D3DLIGHT9 dirLightStruct;                                // Binary offsets: (1.0 US and 1.0 EU): 0x00C92648
    static D3DLIGHT9 localLightStruct;                              // Binary offsets: (1.0 US and 1.0 EU): 0x00C925E0

    static unsigned int maxNumberOfActiveLights = 0;                // Binary offsets: (1.0 US and 1.0 EU): 0x00C926B4

    static int* activeGlobalLightsArray = NULL;                     // Binary offsets: (1.0 US and 1.0 EU): 0x00C926C8
    static unsigned int numberOfActiveGlobalLightsArray = 0;        // Binary offsets: (1.0 US and 1.0 EU): 0x00C926CC
    static unsigned int countOfActiveGlobalLightsArray = 0;         // Binary offsets: (1.0 US and 1.0 EU): 0x00C926D0

    static int* swap_activeGlobalLightsArray = NULL;                // Binary offsets: (1.0 US and 1.0 EU): 0x00C926D4
    static unsigned int swap_numberOfActiveGlobalLightsArray = 0;   // Binary offsets: (1.0 US and 1.0 EU): 0x00C926D8
    static unsigned int swap_countOfActiveGlobalLightsArray = 0;    // Binary offsets: (1.0 US and 1.0 EU): 0x00C926DC
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
    int countOfAvailableLightSlots = D3D9Lighting::countOfAvailableLightSlots;
    int numberOfAvailableLightSlots = D3D9Lighting::numberOfAvailableLightSlots;

    int*& availableLightSlots = D3D9Lighting::availableLightSlots;

    if ( numberOfAvailableLightSlots >= countOfAvailableLightSlots )
    {
        countOfAvailableLightSlots += 0x100;

        size_t newArraySize = sizeof(int) * countOfAvailableLightSlots;

        if ( !availableLightSlots )
            availableLightSlots = (int*)pRwInterface->m_memory.m_malloc( newArraySize, 0x103050D );
        else
            availableLightSlots = (int*)pRwInterface->m_memory.m_realloc( availableLightSlots, newArraySize, 0x103050D );

        D3D9Lighting::countOfAvailableLightSlots = countOfAvailableLightSlots;
    }

    availableLightSlots[numberOfAvailableLightSlots++] = lightIndex;

    D3D9Lighting::numberOfAvailableLightSlots = numberOfAvailableLightSlots;
}

RpLight* __cdecl RpD3D9LightDestructor( RpLight *light )
{
    if ( !light )
        return NULL;

    if ( light->m_lightIndex >= 0 )
    {
        int incrementalIndex = D3D9Lighting::incrementalLightIndex - 1;
        int numberOfAvailableLightSlots = D3D9Lighting::numberOfAvailableLightSlots;

        if ( numberOfAvailableLightSlots >= incrementalIndex )
        {
            D3D9Lighting::numberOfAvailableLightSlots = 0;
            D3D9Lighting::countOfAvailableLightSlots = 0;
            D3D9Lighting::incrementalLightIndex = 0;

            int*& availableLightSlots = D3D9Lighting::availableLightSlots;

            if ( availableLightSlots )
            {
                pRwInterface->m_memory.m_free( availableLightSlots );

                availableLightSlots = NULL;
            }
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

static nativeLightInfo*& GetNativeLightArray( void )
{
    return D3D9Lighting::nativeLightArray;
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
    int countOfAllocatedArray = D3D9Lighting::countOfNativeLightArray;
    nativeLightInfo*& lightArray = GetNativeLightArray();
    
    if ( countOfAllocatedArray <= lightIndex )
    {
        int oldAllocatedCount = countOfAllocatedArray;

        countOfAllocatedArray = lightIndex + 1;

        size_t newArraySize = sizeof( nativeLightInfo ) * countOfAllocatedArray;

        if ( !lightArray )
            lightArray = (nativeLightInfo*)pRwInterface->m_memory.m_malloc( newArraySize, 0x1030411 );
        else
            lightArray = (nativeLightInfo*)pRwInterface->m_memory.m_realloc( lightArray, newArraySize, 0x1030411 );

        // Fill the array gap.
        for ( int n = oldAllocatedCount; n < countOfAllocatedArray; n++ )
        {
            nativeLightInfo& info = lightArray[n];

            // By default, lights get added to phase 0.
            // Phase 0 sets them to "not rendered" for their first cycle occurance.
            // This phase index describes on which list (primary or swap) the light
            // resides on. Sort of like a scan-code algorithm (as seen in the
            // entity world streaming, CEntitySAInterface::m_scanCode).
            info.active = false;
        }

        D3D9Lighting::countOfNativeLightArray = countOfAllocatedArray;
    }
    else
    {
        nativeLightInfo& info = lightArray[lightIndex];

        if ( RpD3D9LightsEqual( lightStruct, info.native ) )
            return true;
    }

    nativeLightInfo& info = lightArray[lightIndex];
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
        unsigned int numAvailableLightSlots = D3D9Lighting::numberOfAvailableLightSlots;

        int slot = -1;

        if ( numAvailableLightSlots != 0 )
        {
            slot = *( D3D9Lighting::availableLightSlots + --numAvailableLightSlots );

            D3D9Lighting::numberOfAvailableLightSlots = numAvailableLightSlots;
        }
        else
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

static int*& GetActiveGlobalLightsArray( void )
{
    return D3D9Lighting::activeGlobalLightsArray;
}

static int*& GetSwapActiveGlobalLightsArray( void )
{
    return D3D9Lighting::swap_activeGlobalLightsArray;
}

inline void AddLightToGlobalList( RpLight *light )
{
    // todo: create a template for this container management.
    // should be done once this RenderWare plugin is localized.

    unsigned int countOfAllocatedArray = D3D9Lighting::countOfActiveGlobalLightsArray;
    unsigned int numberOfActiveGlobalLights = D3D9Lighting::numberOfActiveGlobalLightsArray;

    int*& activeLightArray = GetActiveGlobalLightsArray();

    if ( numberOfActiveGlobalLights >= countOfAllocatedArray )
    {
        unsigned int oldArrayCount = countOfAllocatedArray;

        countOfAllocatedArray += 8;

        size_t newArraySize = sizeof( int ) * countOfAllocatedArray;

        if ( !activeLightArray )
            activeLightArray = (int*)pRwInterface->m_memory.m_malloc( newArraySize, 0x103050D );
        else
            activeLightArray = (int*)pRwInterface->m_memory.m_realloc( activeLightArray, newArraySize, 0x103050D );

        // Fill the array gap.
        for ( unsigned int n = oldArrayCount; n < countOfAllocatedArray; n++ )
        {
            activeLightArray[n] = 0;
        }

        D3D9Lighting::countOfActiveGlobalLightsArray = countOfAllocatedArray;
    }

    activeLightArray[numberOfActiveGlobalLights++] = light->m_lightIndex;

    D3D9Lighting::numberOfActiveGlobalLightsArray = numberOfActiveGlobalLights;
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
    if ( lightIndex >= D3D9Lighting::countOfNativeLightArray )
        return true;

    nativeLightInfo& lightInfo = GetNativeLightArray()[lightIndex];

    if ( lightInfo.active != enable )
    {
        lightInfo.active = enable;
        return GetRenderDevice()->LightEnable( lightIndex, enable ) >= 0;
    }
    return true;
}

inline bool GetPrimaryLightArrayIndex( int lightIndex, unsigned int countTo, unsigned int& indexOut )
{
    for ( unsigned int n = 0; n < countTo; n++ )
    {
        if ( GetActiveGlobalLightsArray()[n] == lightIndex )
        {
            indexOut = n;
            return true;
        }
    }

    return false;
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
        unsigned int numberOfActiveGlobalLights = D3D9Lighting::numberOfActiveGlobalLightsArray;
        unsigned int maxNumberOfActiveLights = D3D9Lighting::maxNumberOfActiveLights;

        if ( numberOfActiveGlobalLights > maxNumberOfActiveLights )
            numberOfActiveGlobalLights = maxNumberOfActiveLights;

        // Cache pointers to light arrays.
        int*& primaryLights = GetActiveGlobalLightsArray();
        int*& swapLights = GetSwapActiveGlobalLightsArray();

        unsigned int lastActiveGlobalLightsCount = D3D9Lighting::swap_numberOfActiveGlobalLightsArray;

        for ( unsigned int n = 0; n < lastActiveGlobalLightsCount; n++ )
        {
            int lightIndex = swapLights[n];

            // Check whether we should disable this light.
            // It is either disabled if it does not exist in the active lights queue
            // or has an index that is outside of the GPUs maximum light index
            // count.
            unsigned int primaryIndex;

            if ( !GetPrimaryLightArrayIndex( lightIndex, numberOfActiveGlobalLights, primaryIndex ) || primaryIndex >= numberOfActiveGlobalLights )
            {
                // Disable the light.
                RpD3D9EnableLight( lightIndex, false );
            }
        }

        // If there is anything to enable, do it.
        if ( numberOfActiveGlobalLights )
        {
            for ( unsigned int n = 0; n < numberOfActiveGlobalLights; n++ )
            {
                RpD3D9EnableLight( primaryLights[n], true );
            }

            int *swapLightsPtr = swapLights;
            int *primaryLightsPtr = primaryLights;

            swapLights = primaryLightsPtr;
            primaryLights = swapLightsPtr;

            unsigned int swapCount = D3D9Lighting::swap_countOfActiveGlobalLightsArray;
            unsigned int primaryCount = D3D9Lighting::countOfActiveGlobalLightsArray;

            D3D9Lighting::swap_countOfActiveGlobalLightsArray = primaryCount;
            D3D9Lighting::countOfActiveGlobalLightsArray = swapCount;

            lastActiveGlobalLightsCount = numberOfActiveGlobalLights;
        }
        else
            lastActiveGlobalLightsCount = 0;

        D3D9Lighting::swap_numberOfActiveGlobalLightsArray = lastActiveGlobalLightsCount;
        D3D9Lighting::numberOfActiveGlobalLightsArray = 0;
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
    int*& availableLightSlots = D3D9Lighting::availableLightSlots;

    if ( availableLightSlots )
    {
        pRwInterface->m_memory.m_free( availableLightSlots );

        availableLightSlots = NULL;
    }
    
    D3D9Lighting::numberOfAvailableLightSlots = 0;
    D3D9Lighting::countOfAvailableLightSlots = 0;
    D3D9Lighting::incrementalLightIndex = 0;

    int*& activeGlobalLightsArray = GetActiveGlobalLightsArray();

    if ( activeGlobalLightsArray )
    {
        pRwInterface->m_memory.m_free( activeGlobalLightsArray );

        activeGlobalLightsArray = NULL;
    }

    D3D9Lighting::numberOfActiveGlobalLightsArray = 0;
    D3D9Lighting::countOfActiveGlobalLightsArray = 0;

    int*& swap_activeGlobalLightsArray = GetSwapActiveGlobalLightsArray();

    if ( swap_activeGlobalLightsArray )
    {
        pRwInterface->m_memory.m_free( swap_activeGlobalLightsArray );

        swap_activeGlobalLightsArray = NULL;
    }

    D3D9Lighting::swap_numberOfActiveGlobalLightsArray = 0;
    D3D9Lighting::swap_countOfActiveGlobalLightsArray = 0;
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

    // Set initial ambient light.
    GetAmbientColor() = RwColorFloat( 0, 0, 0, 1.0f );

    // Initialize light structs (so only minimal initialization will be required).
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