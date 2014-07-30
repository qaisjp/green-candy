/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CMemoryUtilsSA.h
*  PURPOSE:     Classes for memory abstraction
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _MEMORY_ABSTRACTION_UTILITIES_
#define _MEMORY_ABSTRACTION_UTILITIES_

// Array implementation that extends on concepts found inside GTA:SA
template <typename dataType, unsigned int pulseCount, unsigned int allocFlags, typename arrayMan, typename countType>
struct growableArray
{
    typedef dataType dataType_t;

    static AINLINE void* _memAlloc( size_t memSize, unsigned int flags )
    {
#ifndef _USE_ROCKSTAR_MEMORY_FUNCTIONS
        return malloc( memSize );
#else
        return rwInterface->m_memory.m_malloc( memSize, flags );
#endif //_USE_ROCKSTAR_MEMORY_FUNCTIONS
    }

    static AINLINE void* _memRealloc( void *memPtr, size_t memSize, unsigned int flags )
    {
#ifndef _USE_ROCKSTAR_MEMORY_FUNCTIONS
        return realloc( memPtr, memSize );
#else
        return rwInterface->m_memory.m_realloc( memPtr, memSize, flags );
#endif //_USE_ROCKSTAR_MEMORY_FUNCTIONS
    }

    static AINLINE void _memFree( void *memPtr )
    {
#ifndef _USE_ROCKSTAR_MEMORY_FUNCTIONS
        free( memPtr );
#else
        rwInterface->m_memory.m_free( memPtr );
#endif //_USE_ROCKSTAR_MEMORY_FUNCTIONS
    }

    AINLINE growableArray( void )
    {
        data = NULL;
        numActiveEntries = 0;
        sizeCount = 0;
    }

    AINLINE growableArray( const growableArray& right )
    {
        operator = ( right );
    }

    AINLINE void operator = ( const growableArray& right )
    {
        SetSizeCount( right.GetSizeCount() );

        // Copy all data over.
        if ( sizeCount != 0 )
        {
            std::copy( right.data, right.data + sizeCount, data );
        }

        // Set the number of active entries.
        numActiveEntries = right.numActiveEntries;
    }

    AINLINE void SetArrayCachedTo( growableArray& target )
    {
        countType targetSizeCount = GetSizeCount();
        countType oldTargetSizeCount = target.GetSizeCount();

        target.AllocateToIndex( targetSizeCount );

        if ( targetSizeCount != 0 )
        {
            std::copy( data, data + targetSizeCount, target.data );

            // Anything that is above the new target size count must be reset.
            for ( countType n = targetSizeCount; n < oldTargetSizeCount; n++ )
            {
                dataType *theField = &target.data[ n ];

                // Reset it.
                theField->~dataType();

                new (theField) dataType;

                // Tell it to the manager.
                manager.InitField( *theField );
            }
        }

        // Set the number of active entries.
        target.numActiveEntries = numActiveEntries;
    }

    AINLINE ~growableArray( void )
    {
        Shutdown();
    }

    AINLINE void Init( void )
    { }

    AINLINE void Shutdown( void )
    {
        if ( data )
            SetSizeCount( 0 );

        numActiveEntries = 0;
        sizeCount = 0;
    }

    AINLINE void SetSizeCount( countType index )
    {
        if ( index != sizeCount )
        {
            countType oldCount = sizeCount;

            sizeCount = index;

            if ( data )
            {
                // Destroy any structures that got removed.
                for ( countType n = index; n < oldCount; n++ )
                {
                    data[n].~dataType();
                }
            }

            RwInterface *rwInterface = RenderWare::GetInterface();

            if ( index == 0 )
            {
                // Handle clearance requests.
                if ( data )
                {
                    _memFree( data );

                    data = NULL;
                }
            }
            else
            {
                size_t newArraySize = sizeCount * sizeof( dataType );

                if ( !data )
                    data = (dataType*)_memAlloc( newArraySize, allocFlags );
                else
                    data = (dataType*)_memRealloc( data, newArraySize, allocFlags );
            }

            if ( data )
            {
                // Fill the gap.
                for ( countType n = oldCount; n < index; n++ )
                {
                    new (&data[n]) dataType;

                    manager.InitField( data[n] );
                }
            }
            else
                sizeCount = 0;
        }
    }

    AINLINE void AllocateToIndex( countType index )
    {
        if ( index >= sizeCount )
        {
            SetSizeCount( index + ( pulseCount + 1 ) );
        }
    }

    AINLINE void SetItem( const dataType& dataField, countType index )
    {
        AllocateToIndex( index );

        data[index] = dataField;
    }

    AINLINE void SetFast( const dataType& dataField, countType index )
    {
        // God mercy the coder knows why and how he is using this.
        data[index] = dataField;
    }

    AINLINE dataType& GetFast( countType index ) const
    {
        return data[index];
    }

    AINLINE void AddItem( const dataType& data )
    {
        SetItem( data, numActiveEntries );

        numActiveEntries++;
    }

    AINLINE dataType& ObtainItem( countType obtainIndex )
    {
        AllocateToIndex( obtainIndex );

        return data[obtainIndex];
    }

    AINLINE dataType& ObtainItem( void )
    {
        return ObtainItem( numActiveEntries++ );
    }

    AINLINE countType GetCount( void ) const
    {
        return numActiveEntries;
    }

    AINLINE countType GetSizeCount( void ) const
    {
        return sizeCount;
    }

    AINLINE dataType& Get( countType index )
    {
        assert( index < sizeCount );

        return data[index];
    }

    AINLINE const dataType& Get( countType index ) const
    {
        assert( index < sizeCount );

        return data[index];
    }

    AINLINE bool Pop( dataType& item )
    {
        if ( numActiveEntries != 0 )
        {
            item = data[--numActiveEntries];
            return true;
        }

        return false;
    }

    AINLINE bool Pop( void )
    {
        if ( numActiveEntries != 0 )
        {
            --numActiveEntries;
            return true;
        }

        return false;
    }

    AINLINE void RemoveItem( countType foundSlot )
    {
        assert( foundSlot >= 0 && foundSlot < numActiveEntries );
        assert( numActiveEntries != 0 );

        countType moveCount = numActiveEntries - ( foundSlot + 1 );

        if ( moveCount != 0 )
            std::copy( data + foundSlot + 1, data + numActiveEntries, data + foundSlot );

        numActiveEntries--;
    }

    AINLINE bool RemoveItem( const dataType& item )
    {
        countType foundSlot = -1;
        
        if ( !Find( item, foundSlot ) )
            return false;

        RemoveItem( foundSlot );
        return true;
    }

    AINLINE bool Find( const dataType& inst, countType& indexOut ) const
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

    AINLINE unsigned int Count( const dataType& inst ) const
    {
        unsigned int count = 0;

        for ( countType n = 0; n < numActiveEntries; n++ )
        {
            if ( data[n] == inst )
                count++;
        }

        return count;
    }

    AINLINE void Clear( void )
    {
        numActiveEntries = 0;
    }

    AINLINE void TrimTo( countType indexTo )
    {
        if ( numActiveEntries > indexTo )
            numActiveEntries = indexTo;
    }

    AINLINE void SwapContents( growableArray& right )
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
    
    AINLINE void SetContents( growableArray& right )
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

template <size_t memorySize>
class StaticMemoryAllocator
{
public:
    AINLINE StaticMemoryAllocator( void ) : validAllocationRegion( 0, memorySize )
    {
#ifdef _DEBUG
        memset( memData, 0, memorySize );
#endif

        LIST_CLEAR( memEntryList.root );
    }

    AINLINE ~StaticMemoryAllocator( void )
    {
        return;
    }

    AINLINE bool IsValidPointer( void *ptr )
    {
        return ( ptr >= memData && ptr <= ( memData + sizeof( memData ) ) );
    }

    AINLINE void* Allocate( size_t memSize )
    {
        // We cannot allocate zero size.
        if ( memSize == 0 )
            return NULL;

        // Get the actual mem block size.
        size_t actualMemSize = ( memSize + sizeof( memoryEntry ) );

        // Try to allocate memory at the first position we find.
        memSlice_t newAllocSlice( 0, actualMemSize );

        RwListEntry <memoryEntry> *appendNode = &memEntryList.root;

        LIST_FOREACH_BEGIN( memoryEntry, memEntryList.root, node )
            // Intersect the current memory slice with the ones on our list.
            const memSlice_t& blockSlice = item->slice;

            memSlice_t::eIntersectionResult intResult = newAllocSlice.intersectWith( blockSlice );

            if ( !memSlice_t::isFloatingIntersect( intResult ) )
            {
                // Advance the try memory offset.
                {
                    size_t tryMemPosition = blockSlice.GetSliceEndPoint() + 1;
                    
                    // Make sure we align to the system integer.
                    // todo: maybe this is not correct all the time?
                    const size_t sysIntSize = sizeof( unsigned long );

                    tryMemPosition = ALIGN( tryMemPosition, sysIntSize, sysIntSize );

                    newAllocSlice.SetSliceStartPoint( tryMemPosition );
                }

                // Set the append node further.
                appendNode = &item->node;
            }
            else
            {
                // Make sure we do not get behind a memory block.
                assert( intResult != memSlice_t::INTERSECT_FLOATING_END );
                break;
            }
        LIST_FOREACH_END

        // Make sure we allocate in the valid region.
        {
            memSlice_t::eIntersectionResult intResult = newAllocSlice.intersectWith( validAllocationRegion );

            if ( intResult != memSlice_t::INTERSECT_EQUAL &&
                 intResult != memSlice_t::INTERSECT_INSIDE )
            {
                return NULL;
            }
        }

        // Create the allocation information structure and return it.
        memoryEntry *entry = (memoryEntry*)( (char*)memData + newAllocSlice.GetSliceStartPoint() );

        entry->blockSize = memSize;
        entry->slice = newAllocSlice;

        // Append us at the correct position.
        LIST_APPEND( *appendNode, entry->node );

        return entry->GetData();
    }

    AINLINE void Free( void *ptr )
    {
        // Make sure this structure is a valid pointer from our heap.
        assert( IsValidPointer( ptr ) );

        // Remove the structure from existance.
        memoryEntry *entry = (memoryEntry*)ptr - 1;

        LIST_REMOVE( entry->node );
    }

private:
    typedef sliceOfData <size_t> memSlice_t;

    memSlice_t validAllocationRegion;

    struct memoryEntry
    {
        inline void* GetData( void )
        {
            return this + 1;
        }

        size_t blockSize;
        memSlice_t slice;

        RwListEntry <memoryEntry> node;
    };

    RwList <memoryEntry> memEntryList;

    char memData[ memorySize ];
};

template <typename dataType>
struct iterativeGrowableArrayManager
{
    AINLINE void InitField( dataType& theField )
    {
        return;
    }
};

template <typename dataType, unsigned int pulseCount, unsigned int allocFlags, typename countType>
struct iterativeGrowableArray : growableArray <dataType, pulseCount, allocFlags, iterativeGrowableArrayManager <dataType>, countType>
{
};

// Plugin container for dynamic structure extension.
template <typename pluginType>
class CPluginContainerSA
{
public:
    // pluginType requirements:
    /*
        pluginSentry_t pluginSentry;  // data member
    */

    struct pluginInterface
    {
        virtual bool OnPluginConstruct( pluginType *thePlugin, void *mem, unsigned int pluginId )
        {
            return false;
        }

        virtual void OnPluginDestroy( pluginType *thePlugin, void *mem, unsigned int pluginId )
        {
            return;
        }
    };

    struct pluginInfo
    {
        pluginInterface *plugInterface;
        size_t pluginSize;
        unsigned int pluginId;
    };

    typedef iterativeGrowableArray <pluginInfo, 4, 0, unsigned int> pluginInfoList_t;

    pluginInfoList_t plugins;

    // Data member for plugin types.
    struct pluginSentry_t
    {
        struct regPlugInfo
        {
            pluginInfo *descriptor;
            void *allocMem;
        };

        typedef iterativeGrowableArray <regPlugInfo, 4, 0, unsigned int> registeredPlugins_t;

        registeredPlugins_t plugins;

        inline void* GetPluginByID( unsigned int id ) const
        {
            for ( unsigned int n = 0; n < plugins.GetCount(); n++ )
            {
                const regPlugInfo& regInfo = plugins.Get( n );

                if ( regInfo.descriptor->pluginId == id )
                {
                    return regInfo.allocMem;
                }
            }
            return NULL;
        }
    };

    inline bool RegisterPlugin( size_t pluginSize, unsigned int pluginId, pluginInterface *plugInterface )
    {
        if ( pluginSize == 0 || plugInterface == NULL )
            return false;

        pluginInfo& info = plugins.ObtainItem();

        info.plugInterface = plugInterface;
        info.pluginId = pluginId;
        info.pluginSize = pluginSize;
        return true;
    }

    inline void OnPluginObjectCreate( pluginType *obj )
    {
        pluginSentry_t& sentry = obj->pluginSentry;
        pluginType::pluginMemory_t& alloc = obj->pluginMemory;

        for ( unsigned int n = 0; n < plugins.GetCount(); n++ )
        {
            pluginInfo& theInfo = plugins.Get( n );

            // Allocate memory for the plugin (if possible).
            void *mem = alloc.Allocate( theInfo.pluginSize );

            if ( mem )
            {
                bool constructSuccess = theInfo.plugInterface->OnPluginConstruct( obj, mem, theInfo.pluginId );

                if ( constructSuccess )
                {
                    pluginSentry_t::regPlugInfo& regInfo = sentry.plugins.ObtainItem();
                    
                    regInfo.descriptor = &theInfo;
                    regInfo.allocMem = mem;
                }
                else
                {
                    alloc.Free( mem );
                }
            }
        }
    }

    inline void OnPluginObjectDestroy( pluginType *obj )
    {
        pluginSentry_t& sentry = obj->pluginSentry;

        for ( unsigned int n = 0; n < sentry.plugins.GetCount(); n++ )
        {
            pluginSentry_t::regPlugInfo& regInfo = sentry.plugins.Get( n );

            regInfo.descriptor->plugInterface->OnPluginDestroy( obj, regInfo.allocMem, regInfo.descriptor->pluginId );
        }
    }
};

#endif //_MEMORY_ABSTRACTION_UTILITIES_