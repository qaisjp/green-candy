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

#include <list>
#include <rwlist.hpp>
#include <core/CFileSystem.common.h>

template <typename numberType>
class InfiniteCollisionlessBlockAllocator
{
public:
    typedef sliceOfData <numberType> memSlice_t;

    struct block_t
    {
        RwListEntry <block_t> node;

        memSlice_t slice;
    };

    RwList <block_t> blockList;

    typedef RwListEntry <block_t>* blockIter_t;

    struct allocInfo
    {
        memSlice_t slice;
        blockIter_t blockToAppendAt;
    };

    inline InfiniteCollisionlessBlockAllocator( void )
    {
        LIST_CLEAR( blockList.root );
    }

    inline bool FindSpace( numberType sizeOfBlock, allocInfo& infoOut )
    {
        // Try to allocate memory at the first position we find.
        memSlice_t newAllocSlice( 0, sizeOfBlock );

        blockIter_t appendNode = &blockList.root;

        LIST_FOREACH_BEGIN( block_t, blockList.root, node )
            // Intersect the current memory slice with the ones on our list.
            const memSlice_t& blockSlice = item->slice;

            memSlice_t::eIntersectionResult intResult = newAllocSlice.intersectWith( blockSlice );

            if ( !memSlice_t::isFloatingIntersect( intResult ) )
            {
                // Advance the try memory offset.
                {
                    numberType tryMemPosition = blockSlice.GetSliceEndPoint() + 1;
                    
                    // Make sure we align to the system integer.
                    // todo: maybe this is not correct all the time?
                    const numberType sysIntSize = sizeof( unsigned long );

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

        infoOut.slice = newAllocSlice;
        infoOut.blockToAppendAt = appendNode;
        return true;
    }

    inline void PutBlock( block_t *allocatedStruct, allocInfo& info )
    {
        allocatedStruct->slice = info.slice;

        LIST_APPEND( *info.blockToAppendAt, allocatedStruct->node );
    }

    inline void RemoveBlock( block_t *theBlock )
    {
        LIST_REMOVE( theBlock->node );
    }
};

template <size_t memorySize>
class StaticMemoryAllocator
{
    typedef InfiniteCollisionlessBlockAllocator <size_t> blockAlloc_t;

    blockAlloc_t blockRegions;
public:

    AINLINE StaticMemoryAllocator( void ) : validAllocationRegion( 0, memorySize )
    {
#ifdef _DEBUG
        memset( memData, 0, memorySize );
#endif
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

        blockAlloc_t::allocInfo allocInfo;

        bool hasSpace = blockRegions.FindSpace( actualMemSize, allocInfo );

        // The space allocation could fail if there is not enough space given by the size_t type.
        // This is very unlikely to happen, but must be taken care of.
        if ( hasSpace == false )
            return NULL;

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

        // Insert into the block manager.
        blockRegions.PutBlock( entry, allocInfo );

        return entry->GetData();
    }

    AINLINE void Free( void *ptr )
    {
        // Make sure this structure is a valid pointer from our heap.
        assert( IsValidPointer( ptr ) );

        // Remove the structure from existance.
        memoryEntry *entry = (memoryEntry*)ptr - 1;

        blockRegions.RemoveBlock( entry );
    }

private:
    blockAlloc_t::memSlice_t validAllocationRegion;

    struct memoryEntry
    {
        inline void* GetData( void )
        {
            return this + 1;
        }

        size_t blockSize;
    };

    char memData[ memorySize ];
};

// Class used to register anonymous structs that can be placed on top of a C++ type.
template <typename abstractionType, typename pluginDescriptorType>
struct AnonymousPluginStructRegistry
{
    // A plugin struct registry is based on an infinite range of memory that can be allocated on, like a heap.
    // The structure of this memory heap is then applied onto the underlying type.
    typedef InfiniteCollisionlessBlockAllocator <size_t> blockAlloc_t;

    blockAlloc_t pluginRegions;

    typedef typename pluginDescriptorType::pluginOffset_t pluginOffset_t;

    pluginOffset_t preferredAlignment;

    inline AnonymousPluginStructRegistry( void )
    {
        // Reset to zero as we have no plugins allocated.
        this->pluginAllocSize = 0;
        this->preferredAlignment = (pluginOffset_t)4;
    }

    inline ~AnonymousPluginStructRegistry( void )
    {
        // TODO: allow custom memory allocators.
        return;
    }

    // Virtual interface used to describe plugin classes.
    // The construction process must be immutable across the runtime.
    struct pluginInterface
    {
        virtual ~pluginInterface( void )            {}

        virtual bool OnPluginConstruct( abstractionType *object, pluginOffset_t pluginOffset, pluginDescriptorType pluginId )
        {
            // By default, construction of plugins should succeed.
            return true;
        }

        virtual void OnPluginDestruct( abstractionType *object, pluginOffset_t pluginOffset, pluginDescriptorType pluginId )
        {
            return;
        }

        virtual bool OnPluginAssign( abstractionType *dstObject, const abstractionType *srcObject, pluginOffset_t pluginOffset, pluginDescriptorType pluginId )
        {
            // Assignment of data to another plugin struct is optional.
            return false;
        }

        virtual void DeleteOnUnregister( void )
        {
            // Overwrite this method if unregistering should delete this class.
            return;
        }
    };

    // Struct that holds information about registered plugins.
    struct registered_plugin : public blockAlloc_t::block_t
    {
        size_t pluginSize;
        pluginDescriptorType pluginId;
        pluginOffset_t pluginOffset;
        pluginInterface *descriptor;
    };

    // Container that holds plugin information.
    // TODO: Using STL types is crap (?). Use a custom type instead!
    typedef std::list <registered_plugin> registeredPlugins_t;

    registeredPlugins_t regPlugins;

    // Size of the plugins combined, currently.
    size_t pluginAllocSize;

    // Function used to register a new plugin struct into the class.
    inline pluginOffset_t RegisterPlugin( size_t pluginSize, const pluginDescriptorType& pluginId, pluginInterface *plugInterface )
    {
        // Make sure we have got valid parameters passed.
        if ( pluginSize == 0 || plugInterface == NULL )
            return 0;   // TODO: fix this crap, 0 is ambivalent since its a valid index!

        // Determine the plugin offset that should be used for allocation.
        blockAlloc_t::allocInfo blockAllocInfo;

        bool hasSpace = pluginRegions.FindSpace( pluginSize, blockAllocInfo );

        // Handle obscure errors.
        if ( hasSpace == false )
            return 0;

        // The beginning of the free space is where our plugin should be placed at.
        pluginOffset_t useOffset = blockAllocInfo.slice.GetSliceStartPoint();

        // Register ourselves.
        registered_plugin info;
        info.pluginSize = pluginSize;
        info.pluginId = pluginId;
        info.pluginOffset = useOffset;
        info.descriptor = plugInterface;
        
        regPlugins.push_back( info );

        // Register the pointer to the registered plugin.
        pluginRegions.PutBlock( &regPlugins.back(), blockAllocInfo );

        // Update the overall class size.
        // It is determined by the end of this plugin struct.
        this->pluginAllocSize = useOffset + pluginSize;

        return useOffset;
    }

    inline bool UnregisterPlugin( pluginOffset_t pluginOffset )
    {
        bool hasDeleted = false;

        // Get the plugin information that represents this plugin offset.
        for ( registeredPlugins_t::iterator iter = regPlugins.begin(); iter != regPlugins.end(); iter++ )
        {
            registered_plugin& thePlugin = *iter;

            if ( thePlugin.pluginOffset == pluginOffset )
            {
                // We found it!
                // Now remove it and (probably) delete it.
                thePlugin.descriptor->DeleteOnUnregister();

                pluginRegions.RemoveBlock( &thePlugin );

                regPlugins.erase( iter );

                hasDeleted = true;
                break;  // there can be only one.
            }
        }

        return hasDeleted;
    }

    inline bool ConstructPluginBlock( abstractionType *pluginObj )
    {
        // Construct all plugins.
        bool pluginConstructionSuccessful = true;

        registeredPlugins_t::iterator iter = regPlugins.begin();

        try
        {
            for ( ; iter != regPlugins.end(); iter++ )
            {
                registered_plugin& regPluginInfo = *iter;

                bool success =
                    regPluginInfo.descriptor->OnPluginConstruct(
                        pluginObj,
                        regPluginInfo.pluginOffset,
                        regPluginInfo.pluginId
                    );

                if ( !success )
                {
                    pluginConstructionSuccessful = false;
                    break;
                }
            }
        }
        catch( ... )
        {
            // There was an exception while trying to construct a plugin.
            // We do not let it pass and terminate here.
            pluginConstructionSuccessful = false;
        }

        if ( !pluginConstructionSuccessful )
        {
            // The plugin failed to construct, so destroy all plugins that
            // constructed up until that point.
            while ( iter != regPlugins.begin() )
            {
                iter--;

                registered_plugin& constructed_plugin = *iter;

                // Destroy that plugin again.
                constructed_plugin.descriptor->OnPluginDestruct(
                    pluginObj,
                    constructed_plugin.pluginOffset,
                    constructed_plugin.pluginId
                );
            }
        }

        return pluginConstructionSuccessful;
    }

    inline bool AssignPluginBlock( abstractionType *dstPluginObj, const abstractionType *srcPluginObj )
    {
        // Call all assignment operators.
        bool cloneSuccess = true;

        try
        {
            for ( unsigned int n = 0; n < this->regPlugins.size(); n++ )
            {
                registered_plugin& regPluginInfo = this->regPlugins.at( n );

                bool assignSuccess = regPluginInfo.descriptor->OnPluginAssign(
                    dstPluginObj, srcPluginObj,
                    regPluginInfo.pluginOffset,
                    regPluginInfo.pluginId
                );

                if ( !assignSuccess )
                {
                    cloneSuccess = false;
                    break;
                }
            }
        }
        catch( ... )
        {
            // There was an exception while cloning plugin data.
            // We do not let it pass and terminate here.
            cloneSuccess = false;
        }

        return cloneSuccess;
    }

    inline void DestroyPluginBlock( abstractionType *pluginObj )
    {
        // Call destructors of all registered plugins.
        try
        {
            for ( registeredPlugins_t::reverse_iterator iter = regPlugins.rbegin(); iter != regPlugins.rend(); iter++ )
            {
                registered_plugin& regPlugInfo = *iter;

                regPlugInfo.descriptor->OnPluginDestruct(
                    pluginObj,
                    regPlugInfo.pluginOffset,
                    regPlugInfo.pluginId
                );
            }
        }
        catch( ... )
        {
            // There was an exception while destroying a plugin.
            // This should never happen, so throw a breakpoint.
            assert( 0 );
        }
    }
};

// Helper struct for common plugin system functions.
template <typename classType, typename systemType, typename pluginDescriptorType>
struct CommonPluginSystemDispatch
{
    systemType& sysType;

    typedef typename systemType::pluginOffset_t pluginOffset_t;

    inline CommonPluginSystemDispatch( systemType& sysType ) : sysType( sysType )
    {
        return;
    }

    template <typename interfaceType>
    inline pluginOffset_t RegisterCommonPluginInterface( interfaceType *plugInterface, size_t structSize, const pluginDescriptorType& pluginId )
    {
        pluginOffset_t pluginOffset = 0;

        if ( plugInterface )
        {
            // Register our plugin!
            pluginOffset = sysType.RegisterPlugin(
                structSize, pluginId,
                plugInterface
            );

            // Delete our interface again if the plugin offset is invalid.
            if ( !systemType::IsOffsetValid( pluginOffset ) )
            {
                delete plugInterface;
            }
        }
        return pluginOffset;
    }

    // Helper functions used to create common plugin templates.
    template <typename structType>
    inline pluginOffset_t RegisterStructPlugin( const pluginDescriptorType& pluginId )
    {
        struct structPluginInterface : systemType::pluginInterface
        {
            bool OnPluginConstruct( classType *obj, pluginOffset_t pluginOffset, pluginDescriptorType pluginId )
            {
                void *structMem = pluginId.RESOLVE_STRUCT <structType> ( obj, pluginOffset );

                if ( structMem == NULL )
                    return false;

                // Construct the struct!
                structType *theStruct = new (structMem) structType;

                return ( theStruct != NULL );
            }

            void OnPluginDestruct( classType *obj, pluginOffset_t pluginOffset, pluginDescriptorType pluginId )
            {
                structType *theStruct = pluginId.RESOLVE_STRUCT <structType> ( obj, pluginOffset );

                // Destruct the struct!
                theStruct->~structType();
            }

            bool OnPluginAssign( classType *dstObject, const classType *srcObject, pluginOffset_t pluginOffset, pluginDescriptorType pluginId )
            {
                // To an assignment operation.
                structType *dstStruct = pluginId.RESOLVE_STRUCT <structType> ( dstObject, pluginOffset );
                const structType *srcStruct = pluginId.RESOLVE_STRUCT <structType> ( srcObject, pluginOffset );

                *dstStruct = *srcStruct;
                return true;
            }

            void DeleteOnUnregister( void )
            {
                delete this;
            }
        };

        // Create the interface that should handle our plugin.
        structPluginInterface *plugInterface = new structPluginInterface();

        return RegisterCommonPluginInterface( plugInterface, sizeof( structType ), pluginId );
    }

    template <typename structType>
    inline pluginOffset_t RegisterDependantStructPlugin( const pluginDescriptorType& pluginId, size_t structSize = sizeof(structType) )
    {
        struct structPluginInterface : systemType::pluginInterface
        {
            bool OnPluginConstruct( classType *obj, pluginOffset_t pluginOffset, pluginDescriptorType pluginId )
            {
                void *structMem = pluginId.RESOLVE_STRUCT <structType> ( obj, pluginOffset );

                if ( structMem == NULL )
                    return false;

                // Construct the struct!
                structType *theStruct = new (structMem) structType;

                if ( theStruct )
                {
                    // Initialize the manager.
                    theStruct->Initialize( obj );
                }

                return ( theStruct != NULL );
            }

            void OnPluginDestruct( classType *obj, pluginOffset_t pluginOffset, pluginDescriptorType pluginId )
            {
                structType *theStruct = pluginId.RESOLVE_STRUCT <structType> ( obj, pluginOffset );

                // Deinitialize the manager.
                theStruct->Shutdown( obj );

                // Destruct the struct!
                theStruct->~structType();
            }

            bool OnPluginAssign( classType *dstObject, const classType *srcObject, pluginOffset_t pluginOffset, pluginDescriptorType pluginId )
            {
                // To an assignment operation.
                structType *dstStruct = pluginId.RESOLVE_STRUCT <structType> ( dstObject, pluginOffset );
                const structType *srcStruct = pluginId.RESOLVE_STRUCT <structType> ( srcObject, pluginOffset );

                *dstStruct = *srcStruct;
                return true;
            }

            void DeleteOnUnregister( void )
            {
                delete this;
            }
        };

        // Create the interface that should handle our plugin.
        structPluginInterface *plugInterface = new structPluginInterface();

        return RegisterCommonPluginInterface( plugInterface, structSize, pluginId );
    }
};

// Static plugin system that constructs classes that can be extended at runtime.
// This one is inspired by the RenderWare plugin system.
// This container is NOT MULTI-THREAD SAFE.
// All operations are expected to be ATOMIC.
template <typename classType>
struct StaticPluginClassFactory
{
    typedef classType hostType_t;

    static const unsigned int ANONYMOUS_PLUGIN_ID = 0xFFFFFFFF;

    unsigned int aliveClasses;

    inline StaticPluginClassFactory( void )
    {
        aliveClasses = 0;
    }

    inline ~StaticPluginClassFactory( void )
    {
        assert( aliveClasses == 0 );
    }

    // Number type used to store the plugin offset.
    typedef ptrdiff_t pluginOffset_t;

    // Helper functoid.
    struct pluginDescriptor
    {
        typedef pluginOffset_t pluginOffset_t;

        inline pluginDescriptor( void )
        {
            this->pluginId = StaticPluginClassFactory::ANONYMOUS_PLUGIN_ID;
        }

        inline pluginDescriptor( unsigned int pluginId )
        {
            this->pluginId = pluginId;
        }

        operator const unsigned int& ( void ) const
        {
            return this->pluginId;
        }

        template <typename pluginStructType>
        AINLINE static pluginStructType* RESOLVE_STRUCT( classType *object, pluginOffset_t offset )
        {
            return StaticPluginClassFactory::RESOLVE_STRUCT <pluginStructType> ( object, offset );
        }

        template <typename pluginStructType>
        AINLINE static const pluginStructType* RESOLVE_STRUCT( const classType *object, pluginOffset_t offset )
        {
            return StaticPluginClassFactory::RESOLVE_STRUCT <const pluginStructType> ( object, offset );
        }

        unsigned int pluginId;
    };

    typedef AnonymousPluginStructRegistry <classType, pluginDescriptor> structRegistry_t;

    structRegistry_t structRegistry;

    // Localize certain plugin registry types.
    typedef typename structRegistry_t::pluginInterface pluginInterface;

    static const pluginOffset_t INVALID_PLUGIN_OFFSET = (pluginOffset_t)-1;

    AINLINE static bool IsOffsetValid( pluginOffset_t offset )
    {
        return ( offset != INVALID_PLUGIN_OFFSET );
    }

    template <typename pluginStructType>
    AINLINE static pluginStructType* RESOLVE_STRUCT( classType *object, pluginOffset_t offset )
    {
        if ( IsOffsetValid( offset ) == false )
            return NULL;

        return (pluginStructType*)( (char*)object + sizeof( classType ) + offset );
    }

    template <typename pluginStructType>
    AINLINE static const pluginStructType* RESOLVE_STRUCT( const classType *object, pluginOffset_t offset )
    {
        if ( IsOffsetValid( offset ) == false )
            return NULL;

        return (const pluginStructType*)( (char*)object + sizeof( classType ) + offset );
    }

    // Function used to register a new plugin struct into the class.
    inline pluginOffset_t RegisterPlugin( size_t pluginSize, unsigned int pluginId, pluginInterface *plugInterface )
    {
        assert( this->aliveClasses == 0 );

        return structRegistry.RegisterPlugin( pluginSize, pluginId, plugInterface );
    }

    inline void UnregisterPlugin( pluginOffset_t pluginOffset )
    {
        assert( this->aliveClasses == 0 );

        structRegistry.UnregisterPlugin( pluginOffset );
    }

    typedef CommonPluginSystemDispatch <classType, StaticPluginClassFactory, pluginDescriptor> functoidHelper_t;

    // Helper functions used to create common plugin templates.
    template <typename structType>
    inline pluginOffset_t RegisterStructPlugin( unsigned int pluginId )
    {
        return functoidHelper_t( *this ).RegisterStructPlugin <structType> ( pluginId );
    }

    template <typename structType>
    inline pluginOffset_t RegisterDependantStructPlugin( unsigned int pluginId, size_t structSize = sizeof( structType ) )
    {
        return functoidHelper_t( *this ).RegisterDependantStructPlugin <structType> ( pluginId, structSize );
    }

    inline size_t GetClassSize( void ) const
    {
        return ( sizeof( classType ) + this->structRegistry.pluginAllocSize );
    }

    template <typename allocatorType, typename constructorType>
    inline classType* ConstructTemplate( allocatorType& memAllocator, constructorType constructor )
    {
        // Attempt to allocate the necessary memory.
        const size_t baseClassSize = sizeof( classType );
        const size_t wholeClassSize = this->GetClassSize();

        void *classMem = memAllocator.Allocate( wholeClassSize );

        if ( !classMem )
            return NULL;

        classType *resultObject = NULL;
        {
            classType *intermediateClassObject = NULL;

            try
            {
                intermediateClassObject = constructor.Construct( classMem );
            }
            catch( ... )
            {
                // The base object failed to construct, so terminate here.
                intermediateClassObject = NULL;
            }

            if ( intermediateClassObject )
            {
                bool pluginConstructionSuccessful = structRegistry.ConstructPluginBlock( intermediateClassObject );

                if ( pluginConstructionSuccessful )
                {
                    // We succeeded, so return our pointer.
                    // We promote it to a real class object.
                    resultObject = intermediateClassObject;
                }
                else
                {
                    // Else we cannot keep the intermediate class object anymore.
                    intermediateClassObject->~classType();
                }
            }
        }

        if ( resultObject )
        {
            this->aliveClasses++;
        }
        else
        {
            // Clean up.
            memAllocator.Free( classMem, wholeClassSize );
        }

        return resultObject;
    }

    struct basicClassConstructor
    {
        inline classType* Construct( void *mem )
        {
            return new (mem) classType;
        }
    };

    template <typename allocatorType>
    inline classType* Construct( allocatorType& memAllocator )
    {
        basicClassConstructor constructor;

        return ConstructTemplate( memAllocator, constructor );
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
                bool cloneSuccess = structRegistry.AssignPluginBlock( dstObject, srcObject );

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
            // Destroy plugin data first.
            structRegistry.DestroyPluginBlock( classObject );

            try
            {
                // Destroy the base class object.
                classObject->~classType();
            }
            catch( ... )
            {
                // There was an exception while destroying the base class.
                // This must not happen either; we have to notify the guys!
                assert( 0 );
            }

            // Decrease the number of alive classes.
            this->aliveClasses--;
        }

        // Free our memory.
        void *classMem = classObject;

        memAllocator.Free( classMem, this->GetClassSize() );
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

        template <typename constructorType>
        inline classType* ConstructTemplate( constructorType& constructor )
        {
            return pluginRegistry->ConstructTemplate( memAllocator, constructor );
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
        // We might introduce a hyper-paranoid assertion that even checks this...
        data[index] = dataField;
    }

    AINLINE dataType& GetFast( countType index ) const
    {
        // and that.
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

    AINLINE bool Front( dataType& outVal ) const
    {
        bool success = ( GetCount() != 0 );

        if ( success )
        {
            outVal = data[ 0 ];
        }

        return success;
    }

    AINLINE bool Tail( dataType& outVal ) const
    {
        countType count = GetCount();

        bool success = ( count != 0 );

        if ( success )
        {
            outVal = data[ count - 1 ];
        }

        return success;
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