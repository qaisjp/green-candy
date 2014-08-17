/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/MemoryUtils.h
*  PURPOSE:     Memory management templates
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _GLOBAL_MEMORY_UTILITIES_
#define _GLOBAL_MEMORY_UTILITIES_

#include <SharedUtil.h>
#include <rwlist.hpp>
#include <core/CFileSystem.common.h>

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

                    newAllocSlice.SetSlicePosition( tryMemPosition );
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

// Static plugin system that constructs classes that can be extended at runtime.
// This one is inspired by the RenderWare plugin system.
// This container is NOT MULTI-THREAD SAFE.
// All operations are expected to be ATOMIC.
template <typename classType>
struct StaticPluginClassFactory
{
    static const unsigned int ANONYMOUS_PLUGIN_ID = 0xFFFFFFFF;

    unsigned int aliveClasses;

    inline StaticPluginClassFactory( void )
    {
        aliveClasses = 0;
        preferredAlignment = (pluginOffset_t)4;
        classSize = sizeof( classType );
    }

    inline ~StaticPluginClassFactory( void )
    {
        return;
    }

    // Number type used to store the plugin offset.
    typedef ptrdiff_t pluginOffset_t;

    AINLINE static bool IsOffsetValid( pluginOffset_t offset )
    {
        return ( offset >= sizeof( classType ) );
    }

    pluginOffset_t preferredAlignment;

    template <typename pluginStructType>
    AINLINE static pluginStructType* RESOLVE_STRUCT( classType *object, pluginOffset_t offset )
    {
        if ( IsOffsetValid( offset ) == false )
            return NULL;

        return (pluginStructType*)( (char*)object + offset );
    }

    template <typename pluginStructType>
    AINLINE static const pluginStructType* RESOLVE_STRUCT( const classType *object, pluginOffset_t offset )
    {
        if ( IsOffsetValid( offset ) == false )
            return NULL;

        return (const pluginStructType*)( (char*)object + offset );
    }

    // Virtual interface used to describe plugin classes.
    // The construction process must be immutable across the runtime.
    struct pluginInterface
    {
        virtual bool OnPluginConstruct( classType *object, pluginOffset_t pluginOffset, unsigned int pluginId )
        {
            // By default, construction of plugins should succeed.
            return true;
        }

        virtual void OnPluginDestruct( classType *object, pluginOffset_t pluginOffset, unsigned int pluginId )
        {
            return;
        }

        virtual bool OnPluginAssign( classType *dstObject, const classType *srcObject, pluginOffset_t pluginOffset, unsigned int pluginId )
        {
            // Assignment of data to another plugin struct is optional.
            return false;
        }
    };

    // Struct that holds information about registered plugins.
    struct registered_plugin
    {
        size_t pluginSize;
        unsigned int pluginId;
        pluginOffset_t pluginOffset;
        pluginInterface *descriptor;
    };

    // Container that holds plugin information.
    // TODO: Using STL types is crap. Use a custom type instead!
    typedef std::vector <registered_plugin> registeredPlugins_t;

    registeredPlugins_t regPlugins;

    // Size of the plugin currently.
    size_t classSize;

    // Function used to register a new plugin struct into the class.
    inline pluginOffset_t RegisterPlugin( size_t pluginSize, unsigned int pluginId, pluginInterface *plugInterface )
    {
        // Make sure we have got valid parameters passed.
        if ( pluginSize == 0 || plugInterface == NULL )
            return 0;

        // We can only register plugins if we have no constructed objects yet.
        assert( aliveClasses == 0 );

        // Determine the plugin offset that should be used for allocation.
        pluginOffset_t useOffset = 0;
        {
            useOffset = classSize;

            // Align us to the preferred size.
            useOffset = ALIGN( useOffset, preferredAlignment, preferredAlignment );
        }

        // Register ourselves.
        registered_plugin info;
        info.pluginSize = pluginSize;
        info.pluginId = pluginId;
        info.pluginOffset = useOffset;
        info.descriptor = plugInterface;
        
        regPlugins.push_back( info );

        // Update the overall class size.
        // It is determined by the end of this plugin struct.
        classSize = useOffset + pluginSize;
        return useOffset;
    }

    inline void UnregisterPlugin( pluginOffset_t pluginOffset )
    {
        // TODO.
    }

    // Helper functions used to create common plugin templates.
    template <typename structType>
    inline pluginOffset_t RegisterStructPlugin( unsigned int pluginId )
    {
        struct structPluginInterface : pluginInterface
        {
            bool OnPluginConstruct( classType *obj, pluginOffset_t pluginOffset, unsigned int pluginId )
            {
                void *structMem = RESOLVE_STRUCT <structType> ( obj, pluginOffset );

                if ( structMem == NULL )
                    return false;

                // Construct the struct!
                structType *theStruct = new (structMem) structType;

                return ( theStruct != NULL );
            }

            void OnPluginDestruct( classType *obj, pluginOffset_t pluginOffset, unsigned int pluginId )
            {
                structType *theStruct = RESOLVE_STRUCT <structType> ( obj, pluginOffset );

                // Destruct the struct!
                theStruct->~structType();
            }

            bool OnPluginAssign( classType *dstObject, const classType *srcObject, pluginOffset_t pluginOffset, unsigned int pluginId )
            {
                // To an assignment operation.
                structType *dstStruct = RESOLVE_STRUCT <structType> ( dstObject, pluginOffset );
                const structType *srcStruct = RESOLVE_STRUCT <structType> ( srcObject, pluginOffset );

                *dstStruct = *srcStruct;
                return true;
            }
        };

        pluginOffset_t pluginOffset = 0;

        // Create the interface that should handle our plugin.
        structPluginInterface *plugInterface = new structPluginInterface();

        if ( plugInterface )
        {
            // Register our plugin!
            pluginOffset = RegisterPlugin(
                sizeof( structType ), pluginId,
                plugInterface
            );

            // Delete our interface again if the plugin offset is invalid.
            if ( !IsOffsetValid( pluginOffset ) )
            {
                delete plugInterface;
            }
        }
        return pluginOffset;
    }

    inline size_t GetClassSize( void ) const
    {
        return this->classSize;
    }

    template <typename allocatorType>
    inline classType* Construct( allocatorType& memAllocator )
    {
        // Attempt to allocate the necessary memory.
        const size_t baseClassSize = sizeof( classType );
        const size_t wholeClassSize = this->classSize;

        void *classMem = memAllocator.Allocate( wholeClassSize );

        if ( !classMem )
            return NULL;

        classType *resultObject = NULL;
        {
            classType *intermediateClassObject = new (classMem) classType;

            if ( intermediateClassObject )
            {
                // Construct all plugins.
                bool pluginConstructionSuccessful = true;

                unsigned int constrPluginIndex = 0;

                while ( constrPluginIndex < this->regPlugins.size() )
                {
                    registered_plugin& regPluginInfo = this->regPlugins.at( constrPluginIndex );

                    bool success =
                        regPluginInfo.descriptor->OnPluginConstruct(
                            intermediateClassObject,
                            regPluginInfo.pluginOffset,
                            regPluginInfo.pluginId
                        );

                    if ( !success )
                    {
                        pluginConstructionSuccessful = false;
                        break;
                    }
                    else
                    {
                        // We succeeded, so continue.
                        constrPluginIndex++;
                    }
                }

                if ( pluginConstructionSuccessful )
                {
                    // We succeeded, so return our pointer.
                    // We promote it to a real class object.
                    resultObject = intermediateClassObject;
                }
                else
                {
                    // The plugin failed to construct, so destroy all plugins that
                    // constructed up until that point.
                    unsigned int n = constrPluginIndex;

                    while ( n != 0 )
                    {
                        n--;

                        registered_plugin& constructed_plugin = this->regPlugins.at( n );

                        // Destroy that plugin again.
                        constructed_plugin.descriptor->OnPluginDestruct(
                            intermediateClassObject,
                            constructed_plugin.pluginOffset,
                            constructed_plugin.pluginId
                        );
                    }
                }
            }
        }

        if ( resultObject )
        {
            this->aliveClasses++;
        }

        return resultObject;
    }

    template <typename allocatorType>
    inline classType* Clone( allocatorType& memAllocator, const classType *srcObject )
    {
        classType *clonedObject = NULL;
        {
            // Construct a basic class where we assign stuff to.
            classType *dstObject = Construct( memAllocator );

            if ( dstObject )
            {
                // Call all assignment operators.
                bool cloneSuccess = true;

                for ( unsigned int n = 0; n < this->regPlugins.size(); n++ )
                {
                    registered_plugin& regPluginInfo = this->regPlugins.at( n );

                    bool assignSuccess = regPluginInfo.descriptor->OnPluginAssign(
                        dstObject, srcObject,
                        regPluginInfo.pluginOffset,
                        regPluginInfo.pluginId
                    );

                    if ( !assignSuccess )
                    {
                        cloneSuccess = false;
                        break;
                    }
                }

                if ( cloneSuccess )
                {
                    clonedObject = dstObject;
                }
                else
                {
                    // We can simply destroy and NULL the destination object.
                    Destroy( memAllocator, dstObject );

                    dstObject = NULL;
                }
            }
        }

        return clonedObject;
    }

    template <typename allocatorType>
    inline void Destroy( allocatorType& memAllocator, classType *classObject )
    {
        if ( classObject == NULL )
            return;

        {
            // Call destructors of all registered plugins.
            {
                unsigned int n = this->regPlugins.size();

                while ( n != 0 )
                {
                    n--;

                    registered_plugin& regPlugInfo = this->regPlugins.at( n );

                    regPlugInfo.descriptor->OnPluginDestruct(
                        classObject,
                        regPlugInfo.pluginOffset,
                        regPlugInfo.pluginId
                    );
                }
            }

            // Destroy the base class object.
            classObject->~classType();

            // Decrease the number of alive classes.
            this->aliveClasses--;
        }

        // Free our memory.
        void *classMem = classObject;

        memAllocator.Free( classMem, this->classSize );
    }

    template <typename allocatorType>
    struct DeferredConstructor
    {
        StaticPluginClassFactory *pluginRegistry;
        allocatorType& memAllocator;

        inline DeferredConstructor( StaticPluginClassFactory *pluginRegistry, allocatorType& memAllocator ) : memAllocator( memAllocator )
        {
            this->pluginRegistry = pluginRegistry;
        }

        inline allocatorType& GetAllocator( void )
        {
            return memAllocator;
        }

        inline classType* Construct( void )
        {
            return pluginRegistry->Construct( memAllocator );
        }

        inline classType* Clone( const classType *srcObject )
        {
            return pluginRegistry->Clone( memAllocator, srcObject );
        }

        inline void Destroy( classType *object )
        {
            return pluginRegistry->Destroy( memAllocator, object );
        }
    };

    template <typename allocatorType>
    inline DeferredConstructor <allocatorType>* CreateConstructor( allocatorType& memAllocator )
    {
        typedef DeferredConstructor <allocatorType> Constructor;

        Constructor *result = NULL;

        {
            void *constructorMem = memAllocator.Allocate( sizeof( Constructor ) );

            if ( constructorMem )
            {
                result = new (constructorMem) Constructor( this, memAllocator );
            }
        }
        
        return result;
    }

    template <typename allocatorType>
    inline void DeleteConstructor( DeferredConstructor <allocatorType> *handle )
    {
        typedef DeferredConstructor <allocatorType> Constructor;

        allocatorType& memAlloc = handle->GetAllocator();
        {
            handle->~Constructor();
        }

        void *constructorMem = handle;

        memAlloc.Free( constructorMem, sizeof( Constructor ) );
    }
};

// Array implementation that extends on concepts found inside GTA:SA
// NOTE: This array type is a 'trusted type'.
// -> Use it whenever necessary.
template <typename dataType, unsigned int pulseCount, unsigned int allocFlags, typename arrayMan, typename countType, typename allocatorType>
struct growableArrayEx
{
    typedef dataType dataType_t;

    allocatorType _memAllocator;

    AINLINE void* _memAlloc( size_t memSize, unsigned int flags )
    {
        return _memAllocator.Allocate( memSize, flags );
    }

    AINLINE void* _memRealloc( void *memPtr, size_t memSize, unsigned int flags )
    {
        return _memAllocator.Realloc( memPtr, memSize, flags );
    }

    AINLINE void _memFree( void *memPtr )
    {
        _memAllocator.Free( memPtr );
    }

    AINLINE growableArrayEx( void )
    {
        data = NULL;
        numActiveEntries = 0;
        sizeCount = 0;
    }

    AINLINE growableArrayEx( const growableArrayEx& right )
    {
        operator = ( right );
    }

    AINLINE void operator = ( const growableArrayEx& right )
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

    AINLINE void SetArrayCachedTo( growableArrayEx& target )
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

    AINLINE ~growableArrayEx( void )
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

    AINLINE bool Find( const dataType& inst ) const
    {
        countType trashIndex;

        return Find( inst, trashIndex );
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

    AINLINE void SwapContents( growableArrayEx& right )
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
    
    AINLINE void SetContents( growableArrayEx& right )
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

template <typename dataType>
struct iterativeGrowableArrayExManager
{
    AINLINE void InitField( dataType& theField )
    {
        return;
    }
};

template <typename dataType, unsigned int pulseCount, unsigned int allocFlags, typename countType, typename allocatorType>
struct iterativeGrowableArrayEx : growableArrayEx <dataType, pulseCount, allocFlags, iterativeGrowableArrayExManager <dataType>, countType, allocatorType>
{
};

#endif //_GLOBAL_MEMORY_UTILITIES_