/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.lighting.h
*  PURPOSE:     GTA:SA lighting management
*               Extended to properly support dynamic lighting
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RENDERWARE_LIGHTING_MANAGEMENT_
#define _RENDERWARE_LIGHTING_MANAGEMENT_

// Lighting system exports.
int __cdecl     RpD3D9LightsEqual           ( const D3DLIGHT9& left, const D3DLIGHT9& right );
int __cdecl     RpD3D9SetLight              ( int lightIndex, const D3DLIGHT9& lightStruct );
int __cdecl     RpD3D9DirLightEnable        ( RpLight *light );
int __cdecl     RpD3D9LocalLightEnable      ( RpLight *light );
int __cdecl     RpD3D9EnableLight           ( int lightIndex, int phase );
int __cdecl     RpD3D9GlobalLightsEnable    ( unsigned char flags );
bool            RpD3D9GlobalLightingPrePass ( void );
void __cdecl    RpD3D9ResetLightStatus      ( void );
void __cdecl    RpD3D9EnableLights          ( bool enable, int unused );

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

    inline void SetItem( const dataType& dataField, countType index )
    {
        if ( index >= sizeCount )
        {
            SetSizeCount( sizeCount + pulseCount );
        }

        data[index] = dataField;
    }

    inline void AddItem( const dataType& data )
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

    inline bool RemoveItem( const dataType& item )
    {
        countType foundSlot = -1;
        
        if ( !Find( item, foundSlot ) )
            return false;

        countType moveCount = numActiveEntries - ( foundSlot + 1 );

        if ( moveCount != 0 )
            std::copy( data + foundSlot + 1, data + numActiveEntries, data + foundSlot );

        numActiveEntries--;
        return true;
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
    
    inline void SetContents( growableArray& right )
    {
        right.SetSizeCount( numActiveEntries );

        for ( countType n = 0; n < numActiveEntries; n++ )
            right.data[n] = data[n];

        right.numActiveEntries = numActiveEntries;
    }

    dataType* data;
    countType numActiveEntries;
    countType sizeCount;
    arrayMan manager;
};

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

    struct deviceLightInfoArrayManager
    {
        inline void InitField( D3DLIGHT9& lightStruct )
        {
            return;
        }
    };

    typedef growableArray <int, 8, 0x103050D, lightIndexArrayManager, unsigned int> lightIndexArray;
    typedef growableArray <int, 0x100, 0x103050D, availableLightIndexArrayManager, int> availableLightIndexArray;
    typedef growableArray <nativeLightInfo, 8, 0x1030411, nativeLightInfoArrayManager, int> nativeLightInfoArray;

    struct lightState
    {
        lightState( void );
        ~lightState( void );

        void Shutdown( void );

        bool SetLight( int lightIndex, const D3DLIGHT9& lightInfo );
        bool EnableLight( int lightIndex, bool enable );

        inline int GetFreeLightIndex( void )
        {
            return incrementalLightIndex++;
        }

        inline int GetLightIndex( RpLight *light )
        {
            int preferedLightIndex = light->m_lightIndex;
            unsigned int foundAt = 0;

            return ( preferedLightIndex < 0 || !activeGlobalLights.Find( preferedLightIndex, foundAt ) ) ? GetFreeLightIndex() : preferedLightIndex;
        }

        void ResetActiveLights( void );
        bool ActivateDirLight( RpLight *light );
        bool ActivateLocalLight( RpLight *light );

        void SwapWith( lightState& right );
        void Set( lightState& right );

        D3DLIGHT9 dirLightStruct;                           // Binary offsets: (1.0 US and 1.0 EU): 0x00C92648
        D3DLIGHT9 localLightStruct;                         // Binary offsets: (1.0 US and 1.0 EU): 0x00C925E0

        typedef growableArray <D3DLIGHT9, 8, 0x1030411, deviceLightInfoArrayManager, unsigned int> deviceLightInfoArray;

        lightIndexArray             activeGlobalLights;     // Array of light indice that are active
        deviceLightInfoArray        nativeLights;           // Array of light data that is set

        int                         incrementalLightIndex;

        RwColorFloat                ambientLightColor;
    };
};

// Module initialization.
void RenderWareLighting_Init( void );
void RenderWareLighting_Shutdown( void );

void RenderWareLighting_InitShaders( void );
void RenderWareLighting_ResetShaders( void );
void RenderWareLighting_ShutdownShaders( void );

#endif //_RENDERWARE_LIGHTING_MANAGEMENT_