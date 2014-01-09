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
#include <sstream>
#include "RenderWare/RwRenderTools.hxx"

#define NUM_LIGHTING_SHADER_CACHE_COUNT			8

//todo: implement a light state management so that GTA:SA light settings are seperate from MTA light settings.
// then we can preserve the original functionality of GTA:SA -> things will still look the same by default.
// interesting idea: engineLightingChooseMethod( "shader" ) -> will use AngerMAN's pointlighting instead of Direct3D 9
// model lighting for GTA:SA too.

// Localized variables.
namespace D3D9Lighting
{
    static unsigned int maxNumberOfActiveLights = 0;                // Binary offsets: (1.0 US and 1.0 EU): 0x00C926B4

    static lightState curLightState;

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
struct StructComparator
{
    template <typename dataType>
    inline bool CompareData( const dataType& left, const dataType& right )
    {
        return ( left == right );
    }
};

template <typename comparatorType>
__forceinline bool _LightsCompare( const D3DLIGHT9& left, const D3DLIGHT9& right, comparatorType& cmp )
{
    bool equals = 
        cmp.CompareData( left.Type, right.Type ) &&
        cmp.CompareData( left.Diffuse.r, right.Diffuse.r ) &&
        cmp.CompareData( left.Diffuse.g, right.Diffuse.g ) &&
        cmp.CompareData( left.Diffuse.b, right.Diffuse.b );

    if ( !equals )
        return false;

    switch( left.Type )
    {
    case D3DLIGHT_POINT:
        equals = 
            cmp.CompareData( left.Position.x, right.Position.x ) &&
            cmp.CompareData( left.Position.y, right.Position.y ) &&
            cmp.CompareData( left.Position.z, right.Position.z ) &&
            cmp.CompareData( left.Range, right.Range );

        break;
    case D3DLIGHT_SPOT:
        equals = 
            cmp.CompareData( left.Position.x, right.Position.x ) &&
            cmp.CompareData( left.Position.y, right.Position.y ) &&
            cmp.CompareData( left.Position.z, right.Position.z ) &&
            cmp.CompareData( left.Direction.x, right.Direction.x ) &&
            cmp.CompareData( left.Direction.y, right.Direction.y ) &&
            cmp.CompareData( left.Direction.z, right.Direction.z ) &&
            cmp.CompareData( left.Range, right.Range ) &&
            cmp.CompareData( left.Theta, right.Theta ) &&
            cmp.CompareData( left.Phi, right.Phi );

        break;
    case D3DLIGHT_DIRECTIONAL:
        equals = 
            cmp.CompareData( left.Direction.x, right.Direction.x ) &&
            cmp.CompareData( left.Direction.y, right.Direction.y ) &&
            cmp.CompareData( left.Direction.z, right.Direction.z );

        break;
    default:
        equals = true;
    }

    return equals;
}

int __cdecl RpD3D9LightsEqual( const D3DLIGHT9& left, const D3DLIGHT9& right )
{
    // A clever compiler optimizes this into lots of branches.
    return _LightsCompare( left, right, StructComparator() );
}

/*=========================================================
    RpD3D9LightsGetApplicance (MTA extension)

    Arguments:
        left - first light for comparison
        right - second light for comparison
        applicance - ptr to write applicance to if successful
    Purpose:
        Returns the applicance factor of a light source. It
        tells you how different two light structs are. The more
        different they are, the more values have to be updated
        to the GPU.
=========================================================*/
struct StructApplicanceCalculator
{
    inline StructApplicanceCalculator( void )
    {
        applicance = 0;
    }

    template <typename dataType>
    inline bool CompareData( const dataType& left, const dataType& right )
    {
        bool same = ( left == right );

        if ( !same )
            applicance++;

        return true;
    }

    template <>
    inline bool CompareData( const D3DLIGHTTYPE& left, const D3DLIGHTTYPE& right )
    {
        //return ( left == right );
        return true;
    }

    int applicance;
};

bool __cdecl RpD3D9LightsGetApplicance( const D3DLIGHT9& left, const D3DLIGHT9& right, int& applicance )
{
    StructApplicanceCalculator applCalc;

    if ( !_LightsCompare( left, right, applCalc ) )
        return false;

    applicance = applCalc.applicance;
    return true;
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

template <typename processorType>
bool _FindLightSlotGlobal( const D3DLIGHT9& lightStruct, int& lightIndex, processorType& cb )
{
    for ( int n = 0; n < D3D9Lighting::deviceLightInfo.GetSizeCount(); n++ )
    {
        nativeLightInfo& info = D3D9Lighting::deviceLightInfo.Get( n );

        if ( !cb.IsSlotTaken( n ) && RpD3D9LightsEqual( info.native, lightStruct ) )
        {
            lightIndex = n;

            cb.TakeSlot( n );
            return true;
        }
    }

    return false;
}

template <typename processorType>
bool _FindFreeLightSlotGlobal( int& lightIndex, processorType& cb )
{
    int i = 0;

    for ( ; i < D3D9Lighting::deviceLightInfo.GetSizeCount(); i++ )
    {
        nativeLightInfo& info = D3D9Lighting::deviceLightInfo.Get( i );

        if ( !cb.IsSlotTaken( i ) )
        {
            lightIndex = i;

            cb.TakeSlot( i );
            return true;
        }
    }

    if ( i < (int)D3D9Lighting::maxNumberOfActiveLights )
    {
        lightIndex = i;

        cb.TakeSlot( i );
        return true;
    }

    return false;
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

inline CShaderItem* _NewLightShaderInstance( const char *name )
{
    CFileTranslator *mtaFileRoot = core->GetModRoot();

    filePath shaderPath;
    mtaFileRoot->GetFullPath( ( std::string( "/shaders/" ) + name ).c_str(), true, shaderPath );

    filePath rootPath;
    mtaFileRoot->GetFullPath( shaderPath.c_str(), false, rootPath );

    SString outStatus;

    CShaderItem *shader = core->GetGraphics()->GetRenderItemManager()->CreateShader( shaderPath, rootPath, outStatus, 1.0, 0.0, true );

    if ( !shader )
        core->GetConsole()->Print( outStatus );

    return shader;
}

// Manager for rendering light states.
using namespace D3D9Lighting;

struct lightPassManager
{
    lightPassManager( D3D9Lighting::lightState& state, CShaderItem *useShader = NULL ) : light_state( state )
    {
        inPhase = false;
        inPass = false;

        activeShader = NULL;

        SetLightingShader( useShader );
    }

    ~lightPassManager( void )
    {
        SetLightingShader( NULL );
    }

    inline bool IsFixedFunction( void )
    {
        return ( lightShader == NULL );
    }

    bool SetLight( int lightIndex, const D3DLIGHT9& lightStruct )
    {
        if ( !inPhase )
            return light_state.SetLight( lightIndex, lightStruct );
        else
        {
            if ( IsFixedFunction() )
            {
                return _SetLightGlobal( lightIndex, lightStruct );
            }
        }

        return false;
    }

    bool EnableLight( int lightIndex, bool enable )
    {
        if ( !inPhase )
        {
            return light_state.EnableLight( lightIndex, enable );
        }
        else
        {
            if ( IsFixedFunction() )
            {
                return _LightEnableGlobal( lightIndex, enable );
            }
        }

        return false;
    }

    void Begin( void )
    {
        assert( global_inPhase == false );
        assert( inPhase == false );

        inPhase = true;
        global_inPhase = true;

        index = 0;
    }

    void ClearPass( void )
    {
        if ( inPass )
        {
            if ( activeShader )
            {
                activeShader->EndPass();
                activeShader->End();

                core->GetGraphics()->GetRenderItemManager()->SetForceShader( NULL );

                activeShader = NULL;
            }

            inPass = false;
        }
    }

    inline int GetLightPhaseCount( void )
    {
        if ( IsFixedFunction() )
            return (int)D3D9Lighting::maxNumberOfActiveLights;
        else if ( lightShader )
        {
            // todo: dispatch by type and possibility.
            return 9;
        }

        return 0;
    }

    struct activeLightInfo
    {
        activeLightInfo( const D3DLIGHT9 *info ) : lightStruct( info )
        { }

        activeLightInfo( void )
        { }

        const D3DLIGHT9 *lightStruct;
    };

    struct activeLightInfoArrayManager
    {
        inline void InitField( activeLightInfo& info )
        {
            info.lightStruct = NULL;
        }
    };

    typedef growableArray <activeLightInfo, 8, 0x138340, activeLightInfoArrayManager, unsigned int> lightPass;

    struct shaderCacheItem
    {
        nativeLightInfoArray activeLights;
        CShaderItem *cachedShader;
    };

    template <typename findMan>
    inline bool _ProcessShaderCacheItem( shaderCacheItem& item, const D3DLIGHT9& lightStruct, findMan& cb )
    {
        // Find the non-active light slot that is closest to our needs.
        // Closer to our needs == less values to set using SetShaderValue.

        int closestLightIndex = -1;
        int lightNumApplicance = -1;

        for ( int n = 0; n < item.activeLights.GetSizeCount(); n++ )
        {
            if ( !cb.IsIndexTaken( n ) )
            {
                nativeLightInfo& info = item.activeLights.Get( n );

                int lightApplicance = 0;

                if ( RpD3D9LightsGetApplicance( lightStruct, info.native, lightApplicance ) )
                {
                    if ( closestLightIndex == -1 || lightApplicance < lightNumApplicance )
                    {
                        closestLightIndex = n;
                        lightNumApplicance = lightApplicance;
                    }
                }
            }
        }

        if ( closestLightIndex != -1 )
        {
            cb.OnClosestLightIndex( item, closestLightIndex, lightNumApplicance );
            return true;
        }

        return false;
    }

    template <typename findMan>
    struct BatchApplicanceCollector
    {
        AINLINE BatchApplicanceCollector( findMan& man ) : m_findMan( man )
        { }

        AINLINE bool IsIndexTaken( int lightIndex )
        {
            unsigned int slotFound = 0;

            return usedIndice.Find( lightIndex, slotFound );
        }

        AINLINE void OnClosestLightIndex( shaderCacheItem& item, int lightIndex, int applicance )
        {
            m_applicance = applicance;
            usedIndice.AddItem( lightIndex );

            m_findMan.OnClosestLightIndex( item, lightIndex, applicance );
        }

        findMan& m_findMan;
        int m_applicance;
        lightIndexArray usedIndice;
    };

    template <typename findMan>
    inline bool _FindClosestShaderBatch( lightPass& pass, findMan& cb )
    {
        int closestShaderIndex = -1;
        int shaderApplicance = -1;

        int passCount = pass.GetCount();

        for ( int n = 0; n < lightShaderCache.GetSizeCount(); n++ )
        {
            shaderCacheItem& item = lightShaderCache.Get( n );

            if ( item.cachedShader )
            {
                int thisApplicance = 0;

                BatchApplicanceCollector <findMan> collector( cb );

                for ( int i = 0; i < passCount; i++ )
                {
                    const D3DLIGHT9& lightStruct = *pass.Get( i ).lightStruct;

                    if ( _ProcessShaderCacheItem( item, lightStruct, collector ) )
                    {
                        thisApplicance += collector.m_applicance;
                    }
                }

                cb.OnCacheItemApplicance( item, thisApplicance );

                if ( closestShaderIndex == -1 || shaderApplicance > thisApplicance )
                {
                    closestShaderIndex = n;
                    shaderApplicance = thisApplicance;
                }
            }
        }

        if ( closestShaderIndex != -1 )
        {
            cb.OnClosestApplicance( pass, closestShaderIndex, shaderApplicance );
            return true;
        }
        
        return false;
    }

    struct lightApplicanceCacheItem
    {
        lightIndexArray applicatorsByPass;
    };

    struct lightApplicanceCacheItemArrayManager
    {
        inline void InitField( lightApplicanceCacheItem& item )
        {
            return;
        }
    };

    typedef growableArray <lightApplicanceCacheItem, 8, 0x102411, lightApplicanceCacheItemArrayManager, int> lightApplicanceCache;

    lightApplicanceCache m_lightPassCache;

    struct ProcessLightPassApplicance
    {
        AINLINE ProcessLightPassApplicance( lightApplicanceCache& _cache ) : cache( _cache )
        {
            cache.Clear();

            curItem = &cache.ObtainItem();
        }

        AINLINE ~ProcessLightPassApplicance( void )
        {
            for ( int n = 0; n < cache.GetCount(); n++ )
            {
                cache.Get( n ).applicatorsByPass.Clear();
            }
        }

        AINLINE void OnClosestLightIndex( shaderCacheItem& item, int lightIndex, int applicance )
        {
            curItem->applicatorsByPass.AddItem( lightIndex );
        }

        AINLINE void OnCacheItemApplicance( shaderCacheItem& item, int applicance )
        {
            curItem = &cache.ObtainItem();
        }

        AINLINE void OnClosestApplicance( lightPass& pass, int closestIndex, int applicance )
        {
            m_closestIndex = closestIndex;
        }

        lightApplicanceCache& cache;
        lightApplicanceCacheItem *curItem;
        int m_closestIndex;
    };

    AINLINE bool PrepareShaderCacheItem( shaderCacheItem& shader, bool& hadLightShader )
    {
        // Make sure we have a shader instance.
        CShaderItem *cachedLightShader = shader.cachedShader;

        hadLightShader = ( cachedLightShader != NULL );

        if ( !cachedLightShader )
        {
            // todo: implement efficient light shader cloning.
            cachedLightShader = _NewLightShaderInstance( "shader_point_9sm3.fx" );

            shader.cachedShader = cachedLightShader;
        }

        return ( cachedLightShader != NULL );
    }

    AINLINE std::string to_string( int num )
    {
        std::stringstream stream;

        stream << num;

        return stream.str();
    }

    AINLINE void EnableShaderLightInstance( shaderCacheItem& item, int instanceIndex, bool enable )
    {
        // Make sure we have enough slots allocated.
        if ( item.activeLights.GetSizeCount() <= instanceIndex )
            item.activeLights.SetSizeCount( instanceIndex + 1 );

        // Update activity state.
        CShaderItem *cachedLightShader = item.cachedShader;
        nativeLightInfo& dstStruct = item.activeLights.Get( instanceIndex );

        int activeNum = ( enable ? 1 : 0 );

        if ( dstStruct.active != activeNum )
        {
            cachedLightShader->SetValue( ( ( std::string( "gLight" ) + to_string( instanceIndex ) ) + "Enable" ).c_str(), enable );

            dstStruct.active = activeNum;
        }
    }

    AINLINE void UpdateShaderLightInstance( shaderCacheItem& item, bool hadLightShader, int instanceIndex, const D3DLIGHT9& lightStruct )
    {
        // Make sure we have enough slots allocated.
        if ( item.activeLights.GetSizeCount() <= instanceIndex )
            item.activeLights.SetSizeCount( instanceIndex + 1 );

        // Get the lightStruct that we want to update.
        nativeLightInfo& dstStruct = item.activeLights.Get( instanceIndex );
        CShaderItem *cachedLightShader = item.cachedShader;

        // Update general stuff.
        dstStruct.native.Type = lightStruct.Type;

        // Get the ~pointer to the shader light instance.
        std::string instancePrefix = "gLight" + to_string( instanceIndex );

        // Update position.
        {
            const RwMatrix& camTransform = pRwInterface->m_renderCam->m_parent->GetLTM();

            CVector lightPos = camTransform.vPos - (CVector&)lightStruct.Position;

            cachedLightShader->SetValue( ( instancePrefix + "Position" ).c_str(), (float*)&lightPos, 3 );

            dstStruct.native.Position = lightStruct.Position;
        }

        // Update color.
        if ( !hadLightShader ||
             lightStruct.Diffuse.r != dstStruct.native.Diffuse.r ||
             lightStruct.Diffuse.g != dstStruct.native.Diffuse.g ||
             lightStruct.Diffuse.b != dstStruct.native.Diffuse.b )
        {
            cachedLightShader->SetValue( ( instancePrefix + "Diffuse" ).c_str(), (float*)&lightStruct.Diffuse, 4 );

            dstStruct.native.Diffuse = lightStruct.Diffuse;
        }

        // Update attenuation values.
        if ( !hadLightShader ||
             lightStruct.Attenuation0 != dstStruct.native.Attenuation0 )
        {
            cachedLightShader->SetValue( ( instancePrefix + "Attenuation0" ).c_str(), &lightStruct.Attenuation0, 1 );

            dstStruct.native.Attenuation0 = lightStruct.Attenuation0;
        }

        if ( !hadLightShader || 
             lightStruct.Attenuation1 != dstStruct.native.Attenuation1 )
        {
            cachedLightShader->SetValue( ( instancePrefix + "Attenuation1" ).c_str(), &lightStruct.Attenuation1, 1 );

            dstStruct.native.Attenuation1 = lightStruct.Attenuation1;
        }

        if ( !hadLightShader ||
             lightStruct.Attenuation2 != dstStruct.native.Attenuation2 )
        {
            cachedLightShader->SetValue( ( instancePrefix + "Attenuation2" ).c_str(), &lightStruct.Attenuation2, 1 );

            dstStruct.native.Attenuation2 = lightStruct.Attenuation2;
        }

        // Update light range.
        if ( !hadLightShader ||
             lightStruct.Range != dstStruct.native.Range )
        {
            cachedLightShader->SetValue( ( instancePrefix + "Range" ).c_str(), &lightStruct.Range, 1 );

            dstStruct.native.Range = lightStruct.Range;
        }
    }

    inline bool ApplyLightPassConfiguration( lightPass& pass, lightIndexArray& passLightIndice, int shaderIndex )
    {
        unsigned int passCount = pass.GetCount();

        shaderCacheItem& item = lightShaderCache.Get( shaderIndex );

        bool hadLightShader = false;

        if ( !PrepareShaderCacheItem( item, hadLightShader ) )
            return false;

        assert( passCount == passLightIndice.GetCount() );

        for ( unsigned int n = 0; n < passCount; n++ )
        {
            int lightIndex = passLightIndice.Get( n );

            // Update the light sauce.
            UpdateShaderLightInstance( item, hadLightShader, lightIndex, *pass.Get( n ).lightStruct );

            EnableShaderLightInstance( item, lightIndex, true );
        }

        // Disable the remaining lights.
        int phaseCount = GetLightPhaseCount();

        if ( (int)passCount < phaseCount )
        {
            for ( int n = 0; n < phaseCount; n++ )
            {
                unsigned int foundIndex = 0;

                if ( !passLightIndice.Find( n, foundIndex ) )
                {
                    EnableShaderLightInstance( item, n, false );
                }
            }
        }

        // Update the shader parameters.
        item.cachedShader->UpdateParams();

        return true;
    }

    lightIndexArray _cachedActiveLightIndice;

    AINLINE void GenerateLightPass( lightPass& thisPass )
    {
        int phaseCount = GetLightPhaseCount();

        for ( int n = 0; n < phaseCount; n++ )
        {
            unsigned int currentIndex = index + (unsigned int)n;

            if ( light_state.activeGlobalLights.GetCount() <= currentIndex )
                break;

            int lightIndex = light_state.activeGlobalLights.Get( currentIndex );
            const D3DLIGHT9& lightStruct = light_state.nativeLights.Get( lightIndex );

            thisPass.AddItem( activeLightInfo( &lightStruct ) );
        }
    }

    AINLINE bool ActivateShaderLighting( int shaderIndex )
    {
        CShaderItem *shader = lightShaderCache.Get( shaderIndex ).cachedShader;

        bool successful = false;

        if ( shader )
        {
            assert( activeShader == NULL );

            // Set current light shader.
            shader->Begin( numLightShaderPasses );
            
            if ( numLightShaderPasses > 0 )
            {
                core->GetGraphics()->GetRenderItemManager()->SetForceShader( shader );

                activeShader = shader;

                shader->UpdatePipelineParams();

                shader->BeginPass( 0 );

                successful = true;
            }
            else
                shader->End();
        }

        return successful;
    }

    struct lightingDirectApplicator
    {
        AINLINE lightingDirectApplicator( void ) : m_globLightProc( *this )
        { }

        AINLINE bool OnShaderLightEnable( lightPassManager& lightPassMan, int shaderIndex )
        {
            return lightPassMan.ActivateShaderLighting( shaderIndex );
        }

        AINLINE bool OnFixedFunctionLightEnable( lightPassManager& lightPassMan, int lightIndex )
        {
            return lightPassMan.EnableLight( lightIndex, true );
        }

        struct globLightProcessor
        {
            AINLINE globLightProcessor( lightingDirectApplicator& applicator ) : m_applicator( applicator )
            { }

            AINLINE bool IsSlotTaken( int slot )
            {
                return D3D9Lighting::deviceLightInfo.Get( slot ).active != 0;
            }

            AINLINE void TakeSlot( int slot )
            {
                m_applicator.m_takenSlots.AddItem( slot );
            }

            lightingDirectApplicator& m_applicator;
        };

        AINLINE globLightProcessor& GetGlobalLightProcessor( void )
        {
            return m_globLightProc;
        }

        globLightProcessor m_globLightProc;
        lightIndexArray m_takenSlots;
    };
    
    template <typename applicatorType>
    AINLINE bool ProcessLightingPass( lightPass& thisPass, applicatorType& cb )
    {
        bool wantPass = false;

        // Apply this light pass.
        int passCount = thisPass.GetCount();

        if ( passCount > 0 )
        {
            int actualPassCount = 0;

            if ( lightShader )
            {
                ProcessLightPassApplicance applProc( m_lightPassCache );

                lightIndexArray *lightIndice = NULL;

                int shaderIndex = -1;

                // Try to select an inactive shader.
                for ( int n = 0; n < lightShaderCache.GetSizeCount(); n++ )
                {
                    shaderCacheItem& item = lightShaderCache.Get( n );

                    if ( !item.cachedShader )
                    {
                        shaderIndex = n;
                        break;
                    }
                }

                if ( shaderIndex != -1 )
                {
                    lightIndice = &_cachedActiveLightIndice;

                    lightIndice->numActiveEntries = passCount;

                    // Make sure the cached index array has enough members.
                    int oldCount = lightIndice->GetSizeCount();

                    for ( int n = oldCount; n < passCount; n++ )
                        lightIndice->SetItem( n, (unsigned int)n );
                }
                else if ( _FindClosestShaderBatch( thisPass, applProc ) )
                {
                    lightIndice = &applProc.cache.Get( applProc.m_closestIndex ).applicatorsByPass;

                    shaderIndex = applProc.m_closestIndex;
                }

                if ( shaderIndex != -1 )
                {
                    // Push light data onto GPU.
                    wantPass = ApplyLightPassConfiguration( thisPass, *lightIndice, shaderIndex );

                    if ( wantPass )
                    {
                        wantPass = cb.OnShaderLightEnable( *this, shaderIndex );
                    }
                }

                actualPassCount = passCount;
            }
            else if ( IsFixedFunction() )
            {
                int n = 0;

                for ( ; n < passCount; n++ )
                {
                    int nativeIndex = -1;

                    const D3DLIGHT9& lightStruct = *thisPass.Get( n ).lightStruct;

                    if ( !_FindLightSlotGlobal( lightStruct, nativeIndex, cb.GetGlobalLightProcessor() ) )
                    {
                        // Try to get any free slot.
                        if ( !_FindFreeLightSlotGlobal( nativeIndex, cb.GetGlobalLightProcessor() ) )
                            break;

                        if ( !SetLight( nativeIndex, lightStruct ) )
                            break;
                    }

                    if ( !cb.OnFixedFunctionLightEnable( *this, nativeIndex ) )
                        break;

                    wantPass = true;
                }

                actualPassCount = n;
            }

            index += actualPassCount;
        }

        return wantPass;
    }

    struct cachedLightingData
    {
        cachedLightingData( void )
        {
            isAvailable = false;
            cachedTraversalIndex = 0;
        }

        D3D9Lighting::lightIndexArray cachedIndice;
        unsigned int cachedTraversalIndex;
        bool isAvailable;
    };

    cachedLightingData _cachedLightingData;

    struct lightingDataCacheCollector
    {
        AINLINE lightingDataCacheCollector( cachedLightingData& cache ) : m_cache( cache ), m_globLightProc( m_cache )
        { }

        AINLINE bool OnShaderLightEnable( lightPassManager& lightPassMan, int shaderIndex )
        {
            m_cache.cachedIndice.AddItem( shaderIndex );
            return true;
        }

        AINLINE bool OnFixedFunctionLightEnable( lightPassManager& lightPassMan, int lightIndex )
        {
            m_cache.cachedIndice.AddItem( lightIndex );
            return true;
        }

        struct globLightProcessor
        {
            AINLINE globLightProcessor( cachedLightingData& cache ) : m_cache( cache )
            { }

            AINLINE bool IsSlotTaken( int slot )
            {
                unsigned int foundIndex = -1;

                return m_cache.cachedIndice.Find( slot, foundIndex );
            }

            AINLINE void TakeSlot( int slot )
            {
                return;
            }

            cachedLightingData& m_cache;
        };

        AINLINE globLightProcessor& GetGlobalLightProcessor( void )
        {
            return m_globLightProc;
        }

        cachedLightingData& m_cache;
        globLightProcessor m_globLightProc;
    };

    void CacheLightingData( void )
    {
        if ( _cachedLightingData.isAvailable )
            return;

        // Generate a light pass.
        lightPass thisPass;

        GenerateLightPass( thisPass );

        // Collect cache information.
        ProcessLightingPass( thisPass, lightingDataCacheCollector( _cachedLightingData ) );

        _cachedLightingData.isAvailable = true;
    }

    void ResetCachePass( void )
    {
        _cachedLightingData.cachedTraversalIndex = 0;
    }

    void ClearCache( void )
    {
        ResetCachePass();

        _cachedLightingData.cachedIndice.Clear();
        _cachedLightingData.isAvailable = false;
    }

    bool DoLightPass( void )
    {
        assert( inPhase == true );

        ClearPass();

        lightingDirectApplicator applicator;

        if ( _cachedLightingData.isAvailable )
        {
            unsigned int numLoopCount = 0;

            if ( lightShader )
            {
                numLoopCount = 1;
            }
            else if ( IsFixedFunction() )
            {
                numLoopCount = D3D9Lighting::maxNumberOfActiveLights;
            }

            for ( unsigned int n = 0; n < numLoopCount; n++ )
            {
                unsigned int realIndex = _cachedLightingData.cachedTraversalIndex + n;

                if ( realIndex >= _cachedLightingData.cachedIndice.GetCount() )
                    break;

                int lightIndex = _cachedLightingData.cachedIndice.Get( realIndex );

                bool success = false;

                if ( lightShader )
                {
                    success = applicator.OnShaderLightEnable( *this, lightIndex );
                }
                else
                {
                    success = applicator.OnFixedFunctionLightEnable( *this, lightIndex );
                }

                if ( success )
                    inPass = true;
            }

            _cachedLightingData.cachedTraversalIndex += numLoopCount;
        }
        else
        {
            // Generate a light pass.
            lightPass thisPass;

            GenerateLightPass( thisPass );

            // Apply lighting data directly.
            inPass = ProcessLightingPass( thisPass, applicator );
        }

        // Clear up any active light states.
        if ( IsFixedFunction() )
        {
            // Make sure we only activate the number of maximally supported lights on this GPU.
            unsigned int maxGPULights = D3D9Lighting::maxNumberOfActiveLights;

            for ( int n = 0; n < (int)maxGPULights; n++ )
            {
                unsigned int foundIndex = -1;

                if ( !applicator.m_takenSlots.Find( n, foundIndex ) )
                {
                    EnableLight( n, false );
                }
            }
        }

        return inPass;
    }

    bool IsInPass( void ) const
    {
        return inPass;
    }

    bool IsInPhase( void ) const
    {
        return inPhase;
    }

    void End( void )
    {
        ClearPass();
        ResetCachePass();

        inPhase = false;
        global_inPhase = false;
    }

    void SetLightingShader( CShaderItem *item )
    {
        assert( inPhase == false );

        if ( item == lightShader )
            return;

        if ( lightShader )
        {
            // Make sure all shader instances are terminated.
            for ( int n = 0; n < lightShaderCache.GetSizeCount(); n++ )
            {
                shaderCacheItem& item = lightShaderCache.Get( n );

                if ( CShaderItem *shader = item.cachedShader )
                {
                    shader->Release();

                    item.cachedShader = NULL;
                }
            }
        }

        lightShader = item;

        ClearCache();

        if ( lightShader )
        {
            // Initialize the shader light cache that is used to accelerate lights that are applied to multiple meshes.
            lightShaderCache.SetSizeCount( NUM_LIGHTING_SHADER_CACHE_COUNT );
        }
    }

    D3D9Lighting::lightState& light_state;
    unsigned int index;
    bool inPhase;
    bool inPass;

    CShaderItem *lightShader;
    CShaderItem *activeShader;
    unsigned int numLightShaderPasses;

    struct shaderCacheItemArrayManager
    {
        inline void InitField( shaderCacheItem& item )
        {
            item.cachedShader = NULL;
        }
    };

    typedef growableArray <shaderCacheItem, 8, 0x104742, shaderCacheItemArrayManager, int> shaderCacheArray;

    shaderCacheArray lightShaderCache;

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
    // legacy code.
    return;
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
static bool forceGlobalLighting = false;
static bool forceLocalLighting = false;

static RwColorFloat& GetAmbientColor( void )
{
    return *(RwColorFloat*)0x008E2418;
}

template <typename lightMan>
int _GlobalLightsEnable( D3D9Lighting::lightState& state, lightMan& cb )
{
    RwColorFloat& ambientColor = state.ambientLightColor;
    RwScene *curScene = pRwInterface->m_currentScene;

    ambientColor = RwColorFloat( 0.0f, 0, 0, 1.0f );

    bool isLighting = false;
    unsigned int numLights = 0;

    LIST_FOREACH_BEGIN( RpLight, curScene->m_globalLights.root, m_sceneLights )
        if ( cb.CanProcessLight( item ) )
        {
            numLights++;

            switch( item->m_subtype )
            {
            case LIGHT_TYPE_DIRECTIONAL:
                if ( cb.CanProcessDirectionalLight() )
                {
                    if ( state.ActivateDirLight( item ) )
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
        return IS_ANY_FLAG( m_geom->flags, 0x10 ) || forceGlobalLighting;
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

static CShaderItem *lightingShader = NULL;  // todo: one shader for every light type.
static lightPassManager localLightPassMan( localLight_lightState );

void __cdecl HOOK_DefaultAtomicLightingCallback( RpAtomic *atomic )
{
    RpGeometry *geom = atomic->m_geometry;
    RwScene *curScene = pRwInterface->m_currentScene;

    RpD3D9ResetLightStatus();

    // Reset light states.
    D3D9Lighting::curLightState.ResetActiveLights();
    localLight_lightState.ResetActiveLights();

    bool requiresAmbientLighting = false;

    bool enableLighting = IS_ANY_FLAG( geom->flags, 0x20 );

    if ( curScene )
    {
        if ( enableLighting || forceGlobalLighting )
        {
            // Process main light-state.
            D3D9Lighting::curLightState.ResetActiveLights();

            hasGlobalLighting = _GlobalLightsEnable( D3D9Lighting::curLightState, AtomicGlobalLightManager( geom ) ) != 0;

            // Set the global ambient light color to the one specified by the native light-state.
            GetAmbientColor() = D3D9Lighting::curLightState.ambientLightColor;

            requiresAmbientLighting = true;

            // todo: optimize light sectoring.
            // we should make sure it is quad-tree oriented (not proven for now, tho its quite fast this way)
        }

        if ( enableLighting && IS_ANY_FLAG( geom->flags, 0x10 ) || forceLocalLighting )
        {
            // Activate all local lights on a seperate light-state.
            D3D9Lighting::lightState& localLightState = localLight_lightState;
            localLightState.ResetActiveLights();

            // Scancode algorithm, so lights do not get processed twice.
            pRwInterface->m_frame++;

            LIST_FOREACH_BEGIN( RpAtomic::sectorNode, atomic->sectors.root, node )
                RwSector *sector = item->data;

                LIST_FOREACH_BEGIN( RwSector::lightNode, sector->m_localLights.root, node )
                    RpLight *light = item->data;

                    if ( light && light->m_parent && light->m_frame != pRwInterface->m_frame && light->IsLightActive() )
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

    // Optimize lighting by enabling it here.
    // This way we behave like original GTA:SA.
    RpD3D9GlobalLightingPrePass();

    HOOK_RwD3D9SetRenderState( D3DRS_AMBIENT, ( requiresAmbientLighting ) ? 0xFFFFFFFF : 0x00000000 );

    if ( hasLocalLighting )
    {
        // Update light shader.
        localLightPassMan.SetLightingShader( lightingShader );
    }

    HOOK_RwD3D9SetRenderState( D3DRS_LIGHTING, ( requiresAmbientLighting || hasGlobalLighting && globalLightPassMan.IsFixedFunction() ) );
}

/*=========================================================
    D3D9Lighting::SetGlobalLightingAlwaysEnabled

    Arguments:
        enabled - switch to force global lights rendering
    Purpose:
        Switches the status that decides whether global lighting
        is applied to all atomics. If enabled is true, then
        all meshes are rendered while having directional and
        ambient lights applied to them.
=========================================================*/
void D3D9Lighting::SetGlobalLightingAlwaysEnabled( bool enabled )
{
    forceGlobalLighting = enabled;
}

bool D3D9Lighting::IsGlobalLightingAlwaysEnabled( void )
{
    return forceGlobalLighting;
}

void CRenderWareSA::SetGlobalLightingAlwaysEnabled( bool enabled )  { D3D9Lighting::SetGlobalLightingAlwaysEnabled( enabled ); }
bool CRenderWareSA::IsGlobalLightingAlwaysEnabled( void ) const     { return D3D9Lighting::IsGlobalLightingAlwaysEnabled(); }
/*=========================================================
    D3D9Lighting::SetLocalLightingAlwaysEnabled

    Arguments:
        enabled - switch to force local lights rendering
    Purpose:
        Switches the status that decides whether local lighting
        is applied to all atomics. If enabled is true, then
        point- and spotlights are applied to all rendering meshes.
=========================================================*/
void D3D9Lighting::SetLocalLightingAlwaysEnabled( bool enabled )
{
    forceLocalLighting = enabled;
}

bool D3D9Lighting::IsLocalLightingAlwaysEnabled( void )
{
    return forceLocalLighting;
}

void CRenderWareSA::SetLocalLightingAlwaysEnabled( bool enabled )   { D3D9Lighting::SetLocalLightingAlwaysEnabled( enabled ); }
bool CRenderWareSA::IsLocalLightingAlwaysEnabled( void ) const      { return D3D9Lighting::IsLocalLightingAlwaysEnabled(); }
/*=========================================================
    RpD3D9GlobalLightingPrePass (MTA extension)

    Purpose:
        Activates global lighting states that are applied
        when the game mesh is first rendered. By default, this
        function expects that the fixed function pipeline
        is issued. The game will set texture stage states
        depending on how content will be rendered.
=========================================================*/
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

    return doLighting;
}

/*=========================================================
    RpD3D9CacheLighting (MTA extension)

    Purpose:
        Cached lighting data so that it does not have to be
        recalculated every pass.
=========================================================*/
void RpD3D9CacheLighting( void )
{
    // Cache lighting data.
    if ( hasLocalLighting )
    {
        localLightPassMan.CacheLightingData();
        //globalLightPassMan.CacheLightingData();
    }
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
static float lightingDepthBias = -0.0000016f;
static float lightingSlopedDepthBias = -1.0f;

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
        RwRenderStateLock alpharef( D3DRS_ALPHAREF, 0x01 );

        float _lightingDepthBias = lightingDepthBias;
        float _lightingSlopedDepthBias = lightingSlopedDepthBias;

        unsigned int depthBias = *(unsigned int*)&_lightingDepthBias;
        unsigned int slopedDepthBias = *(unsigned int*)&_lightingSlopedDepthBias;

        RwRenderStateLock depthbias( D3DRS_DEPTHBIAS, depthBias );
        RwRenderStateLock slopeddepth( D3DRS_SLOPESCALEDEPTHBIAS, slopedDepthBias );
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
                localLightPassMan.Begin();

                while ( localLightPassMan.DoLightPass() )
                {
                    // Render as often as we require light meshes.
                    RwD3D9DrawRenderPassPrimitive( rtinfo, rtPass );
                }

                localLightPassMan.End();
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

    // Make sure we clear the cache.
    globalLightPassMan.ClearCache();
    localLightPassMan.ClearCache();

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

void RenderWareLighting_Reset( void )
{
    D3D9Lighting::SetGlobalLightingAlwaysEnabled( false );
    D3D9Lighting::SetLocalLightingAlwaysEnabled( false );
}

void RenderWareLighting_InitShaders( void )
{
    // Initialize lighting shader.
    lightingShader = _NewLightShaderInstance( "shader_point_9sm3.fx" );

    // Prepare device capabilities.
    D3DCAPS9 deviceCaps;

    GetRenderDevice()->GetDeviceCaps( &deviceCaps );

    // Fix some render state usage.
    RwD3D9SetRenderState( D3DRS_DEPTHBIAS, 0 );
    RwD3D9SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, 0 );
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

    localLightPassMan.SetLightingShader( NULL );
}