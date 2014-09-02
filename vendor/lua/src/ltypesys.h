// Lua type abstraction system.
#ifndef _LUA_TYPE_ABSTRACTION_SYSTEM_
#define _LUA_TYPE_ABSTRACTION_SYSTEM_

#include "lmem.h"

// Type sentry struct of the Lua type system.
// It notes the programmer that the struct has RTTI.
// Classes must inherit from this if they want RTTI support.
struct LuaRTTI
{
#ifdef _DEBUG
    void *typesys_ptr;      // pointer to the LuaTypeSystem manager (for debugging)
#endif //_DEBUG
    void *type_meta;        // pointer to the struct that denotes the Lua type
};

// This class manages type information of the Lua runtime.
// It allows for dynamic C++ class extension depending on runtime conditions.
// Main purpose for its creation are the tight memory requirements of Lua VMs.
struct LuaTypeSystem
{
    typedef GeneralMemoryAllocator memAllocType;

    static const unsigned int ANONYMOUS_PLUGIN_ID = 0xFFFFFFFF;

    // Exceptions thrown by this system.
    class abstraction_construction_exception    {};

    inline LuaTypeSystem( void )
    {
        LIST_CLEAR( registeredTypes.root );
        
        this->_memAlloc = NULL;
    }

    inline ~LuaTypeSystem( void )
    {
        // Clean up our memory by deleting all types.
        while ( !LIST_EMPTY( registeredTypes.root ) )
        {
            typeInfoBase *info = LIST_GETITEM( typeInfoBase, registeredTypes.root.next, node );

            DeleteType( info );
        }
    }

    // This field has to be set by the Lua runtime.
    memAllocType *_memAlloc;

    // Interface for type lifetime management.
    struct typeInterface abstract
    {
        virtual void Construct( void *mem, void *construct_params ) const = 0;
        virtual void Destruct( void *mem ) const = 0;

        virtual size_t GetTypeSize( void *construct_params ) const = 0;

        virtual size_t GetTypeSizeByObject( void *mem ) const = 0;
    };

    struct typeInfoBase;

    struct pluginDescriptor
    {
        typedef ptrdiff_t pluginOffset_t;

        inline pluginDescriptor( void )
        {
            this->pluginId = LuaTypeSystem::ANONYMOUS_PLUGIN_ID;
            this->typeInfo = NULL;
        }

        inline pluginDescriptor( unsigned int id, typeInfoBase *typeInfo )
        {
            this->pluginId = id;
            this->typeInfo = typeInfo;
        }

        unsigned int pluginId;
        typeInfoBase *typeInfo;

        template <typename pluginStructType>
        AINLINE pluginStructType* RESOLVE_STRUCT( LuaRTTI *object, pluginOffset_t offset )
        {
            return LuaTypeSystem::RESOLVE_STRUCT <pluginStructType> ( object, this->typeInfo, offset );
        }

        template <typename pluginStructType>
        AINLINE const pluginStructType* RESOLVE_STRUCT( const LuaRTTI *object, pluginOffset_t offset )
        {
            return LuaTypeSystem::RESOLVE_STRUCT <const pluginStructType> ( object, this->typeInfo, offset );
        }
    };

    typedef AnonymousPluginStructRegistry <LuaRTTI, pluginDescriptor> structRegistry_t;

    // Localize important struct details.
    typedef structRegistry_t::pluginOffset_t pluginOffset_t;
    typedef structRegistry_t::pluginInterface pluginInterface;

    static const pluginOffset_t INVALID_PLUGIN_OFFSET = (pluginOffset_t)-1;

    struct typeInfoBase
    {
        virtual void Cleanup( memAllocType& memAlloc ) = 0;

        const char *name;   // name of this type
        typeInterface *tInterface;  // type construction information

        unsigned long refCount; // number of entities that use the type

        // WARNING: as long as a type is referenced, it MUST not change!!!
        inline bool IsImmutable( void ) const
        {
            return ( this->refCount != 0 );
        }

        unsigned long inheritanceCount; // number of types inheriting from this type

        inline bool IsEndType( void ) const
        {
            return ( this->inheritanceCount == 0 );
        }

        // Inheritance information.
        typeInfoBase *inheritsFrom; // type that this type inherits from

        // Plugin information.
        structRegistry_t structRegistry;

        RwListEntry <typeInfoBase> node;
    };

    AINLINE static size_t GetTypePluginOffset( typeInfoBase *subclassTypeInfo, typeInfoBase *offsetInfo )
    {
        size_t offset = 0;

        if ( typeInfoBase *inheritsFrom = offsetInfo->inheritsFrom )
        {
            offset += GetTypePluginOffset( subclassTypeInfo, inheritsFrom );

            offset += inheritsFrom->structRegistry.pluginAllocSize;
        }

        return offset;
    }

    AINLINE static typeInfoBase* GetTypeInfoFromTypeStruct( LuaRTTI *rtObj )
    {
        return (typeInfoBase*)rtObj->type_meta;
    }

    // Function to get the offset of a type information on an object.
    AINLINE static pluginOffset_t GetTypeInfoStructOffset( LuaRTTI *rtObj, typeInfoBase *offsetInfo )
    {
        pluginOffset_t baseOffset = 0;

        // Get generic type information.
        typeInfoBase *subclassTypeInfo = GetTypeInfoFromTypeStruct( rtObj );

        // Get the pointer to the language object.
        void *langObj = GetObjectFromTypeStruct( rtObj );

        baseOffset += sizeof( LuaRTTI );

        // Add the dynamic size of the language object.
        baseOffset += subclassTypeInfo->tInterface->GetTypeSizeByObject( langObj );

        // Add the plugin offset.
        baseOffset += GetTypePluginOffset( subclassTypeInfo, offsetInfo );

        return baseOffset;
    }

    AINLINE static bool IsOffsetValid( pluginOffset_t offset )
    {
        return ( offset != INVALID_PLUGIN_OFFSET );
    }

    template <typename pluginStructType>
    AINLINE static pluginStructType* RESOLVE_STRUCT( LuaRTTI *rtObj, typeInfoBase *typeInfo, pluginOffset_t offset )
    {
        if ( !IsOffsetValid( offset ) )
            return NULL;

        size_t baseOffset = GetTypeInfoStructOffset( rtObj, typeInfo );

        return (pluginStructType*)( (char*)rtObj + baseOffset + offset );
    }

    template <typename pluginStructType>
    AINLINE static const pluginStructType* RESOLVE_STRUCT( const LuaRTTI *rtObj, typeInfoBase *typeInfo, pluginOffset_t offset )
    {
        if ( !IsOffsetValid( offset ) )
            return NULL;

        size_t baseOffset = GetTypeInfoStructOffset( (LuaRTTI*)rtObj, typeInfo );

        return (const pluginStructType*)( (char*)rtObj + baseOffset + offset );
    }

    // Function used to register a new plugin struct into the class.
    inline pluginOffset_t RegisterPlugin( size_t pluginSize, const pluginDescriptor& descriptor, pluginInterface *plugInterface )
    {
        lua_assert( descriptor.typeInfo->IsImmutable() == false );

        return descriptor.typeInfo->structRegistry.RegisterPlugin( pluginSize, descriptor, plugInterface );
    }

    inline void UnregisterPlugin( typeInfoBase *typeInfo, pluginOffset_t pluginOffset )
    {
        lua_assert( typeInfo->IsImmutable() == false );

        typeInfo->structRegistry.UnregisterPlugin( pluginOffset );
    }

    // Plugin registration functions.
    typedef CommonPluginSystemDispatch <LuaRTTI, LuaTypeSystem, pluginDescriptor> functoidHelper_t;

    template <typename structType>
    inline pluginOffset_t RegisterStructPlugin( typeInfoBase *typeInfo, unsigned int pluginId )
    {
        pluginDescriptor descriptor( pluginId, typeInfo );

        return functoidHelper_t( *this ).RegisterStructPlugin <structType> ( descriptor );
    }

    typedef CommonPluginSystemDispatch <LuaRTTI, LuaTypeSystem, pluginDescriptor> functoidHelper_t;

    template <typename structType>
    inline pluginOffset_t RegisterDependantStructPlugin( typeInfoBase *typeInfo, unsigned int pluginId, size_t structSize = sizeof(structType) )
    {
        pluginDescriptor descriptor( pluginId, typeInfo );

        return functoidHelper_t( *this ).RegisterDependantStructPlugin <structType> ( descriptor, structSize );
    }

    RwList <typeInfoBase> registeredTypes;

    inline void SetupTypeInfoBase( typeInfoBase *tInfo, const char *typeName, typeInterface *tInterface )
    {
        tInfo->name = typeName;
        tInfo->refCount = 0;
        tInfo->inheritanceCount = 0;
        tInfo->tInterface = tInterface;
        tInfo->inheritsFrom = NULL;
        LIST_INSERT( registeredTypes.root, tInfo->node );
    }

    inline typeInfoBase* RegisterType( const char *typeName, typeInterface *typeInterface )
    {
        struct typeInfoGeneral : public typeInfoBase
        {
            void Cleanup( memAllocType& memAlloc )
            {
                // Terminate ourselves.
                _delstruct( this, memAlloc );
            }
        };

        typeInfoBase *info = _newstruct <typeInfoGeneral> ( *_memAlloc );

        if ( info )
        {
            SetupTypeInfoBase( info, typeName, typeInterface );
        }

        return info;
    }

    template <typename structTypeTypeInterface>
    inline typeInfoBase* RegisterCommonTypeInterface( const char *typeName, structTypeTypeInterface *tInterface )
    {
        struct typeInfoStruct : public typeInfoBase
        {
            void Cleanup( memAllocType& memAlloc )
            {
                _delstruct( tInterface, memAlloc );
                _delstruct( this, memAlloc );
            }

            structTypeTypeInterface *tInterface;
        };

        typeInfoStruct *tInfo = _newstruct <typeInfoStruct> ( *_memAlloc );

        if ( tInfo )
        {
            SetupTypeInfoBase( tInfo, typeName, tInterface );

            tInfo->tInterface = tInterface;
            return tInfo;
        }
        
        _delstruct( tInterface, *_memAlloc );
        return NULL;
    }

    template <typename structType>
    inline typeInfoBase* RegisterAbstractType( const char *typeName )
    {
        struct structTypeInterface : public typeInterface
        {
            void Construct( void *mem, void *construct_params ) const
            {
                throw abstraction_construction_exception();
            }

            void Destruct( void *mem ) const
            {
                return;
            }

            size_t GetTypeSize( void *construct_params ) const
            {
                return sizeof( structType );
            }

            size_t GetTypeSizeByObject( void *langObj ) const
            {
                return (size_t)0;
            }
        };

        structTypeInterface *tInterface = _newstruct <structTypeInterface> ( *_memAlloc );

        return RegisterCommonTypeInterface( typeName, tInterface );
    }

    template <typename structType>
    inline typeInfoBase* RegisterStructType( const char *typeName )
    {
        struct structTypeInterface : public typeInterface
        {
            void Construct( void *mem, void *construct_params ) const
            {
                new (mem) structType( construct_params );
            }

            void Destruct( void *mem ) const
            {
                ((structType*)mem)->~structType();
            }

            size_t GetTypeSize( void *construct_params ) const
            {
                return sizeof( structType );
            }

            size_t GetTypeSizeByObject( void *langObj ) const
            {
                return sizeof( structType );
            }
        };

        structTypeInterface *tInterface = _newstruct <structTypeInterface> ( *_memAlloc );

        return RegisterCommonTypeInterface( typeName, tInterface );
    }

    template <typename classType, typename staticRegistry>
    inline pluginOffset_t StaticPluginRegistryRegisterTypeConstruction( staticRegistry& registry, typeInfoBase *typeInfo, void *construction_params = NULL )
    {
        struct structPluginInterface : staticRegistry::pluginInterface
        {
            bool OnPluginConstruct( staticRegistry::hostType_t *obj, staticRegistry::pluginOffset_t pluginOffset, staticRegistry::pluginDescriptor pluginId )
            {
                void *structMem = pluginId.RESOLVE_STRUCT <void> ( obj, pluginOffset );

                if ( structMem == NULL )
                    return false;

                // Construct the type.
                LuaRTTI *rtObj = typeSys->ConstructPlacement( structMem, typeInfo, construction_params );

                // Hack: tell it about the struct.
                if ( rtObj != NULL )
                {
                    void *langObj = LuaTypeSystem::GetObjectFromTypeStruct( rtObj );

                    ((classType*)langObj)->Initialize( obj );
                }
                
                return ( rtObj != NULL );
            }

            void OnPluginDestruct( staticRegistry::hostType_t *obj, staticRegistry::pluginOffset_t pluginOffset, staticRegistry::pluginDescriptor pluginId )
            {
                LuaRTTI *rtObj = pluginId.RESOLVE_STRUCT <LuaRTTI> ( obj, pluginOffset );

                // Hack: deinitialize the class.
                {
                    void *langObj = LuaTypeSystem::GetObjectFromTypeStruct( rtObj );

                    ((classType*)langObj)->Shutdown( obj );
                }

                // Destruct the type.
                typeSys->DestroyPlacement( rtObj );
            }

            bool OnPluginAssign( staticRegistry::hostType_t *dstObject, const staticRegistry::hostType_t *srcObject, staticRegistry::pluginOffset_t pluginOffset, staticRegistry::pluginDescriptor pluginId )
            {
                return false;
            }

            LuaTypeSystem *typeSys;
            typeInfoBase *typeInfo;
            void *construction_params;
        };

        staticRegistry::pluginOffset_t offset = 0;

        structPluginInterface *tInterface = _newstruct <structPluginInterface> ( *_memAlloc );

        if ( tInterface )
        {
            tInterface->typeSys = this;
            tInterface->typeInfo = typeInfo;
            tInterface->construction_params = construction_params;

            offset = registry.RegisterPlugin(
                this->GetTypeStructSize( typeInfo, construction_params ),
                staticRegistry::pluginDescriptor( staticRegistry::ANONYMOUS_PLUGIN_ID ),
                tInterface
            );

            if ( !staticRegistry::IsOffsetValid( offset ) )
            {
                _delstruct( tInterface, *_memAlloc );
            }
        }

        return offset;
    }

    struct structTypeMetaInfo abstract
    {
        virtual ~structTypeMetaInfo( void )     {}

        virtual size_t GetTypeSize( void *construct_params ) const = 0;

        virtual size_t GetTypeSizeByObject( void *mem ) const = 0;
    };

    template <typename structType>
    inline typeInfoBase* RegisterDynamicStructType( const char *typeName, structTypeMetaInfo *metaInfo )
    {
        struct structTypeInterface : public typeInterface
        {
            void Construct( void *mem, void *construct_params ) const
            {
                new (mem) structType( construct_params );
            }

            void Destruct( void *mem ) const
            {
                ((structType*)mem)->~structType();
            }

            size_t GetTypeSize( void *construct_params ) const
            {
                return meta_info->GetTypeSize( construct_params );
            }

            size_t GetTypeSizeByObject( void *obj ) const
            {
                return meta_info->GetTypeSizeByObject( obj );
            }
           
            structTypeMetaInfo *meta_info;
        };

        structTypeInterface *tInterface = _newstruct <structTypeInterface> ( *_memAlloc );
        tInterface->meta_info = metaInfo;

        return RegisterCommonTypeInterface( typeName, tInterface );
    }

    static inline size_t GetTypePluginSize( typeInfoBase *typeInfo )
    {
        size_t sizeOut = (size_t)typeInfo->structRegistry.pluginAllocSize;
        
        // Add the plugin sizes of all inherited classes.
        if ( typeInfoBase *inheritedClass = typeInfo->inheritsFrom )
        {
            sizeOut += GetTypePluginSize( inheritedClass );
        }

        return sizeOut;
    }

    inline bool ConstructPlugins( typeInfoBase *typeInfo, LuaRTTI *rtObj )
    {
        bool pluginConstructSuccess = true;

        if ( typeInfoBase *inheritedClass = typeInfo->inheritsFrom )
        {
            pluginConstructSuccess = ConstructPlugins( inheritedClass, rtObj );
        }

        if ( pluginConstructSuccess )
        {
            pluginConstructSuccess = typeInfo->structRegistry.ConstructPluginBlock( rtObj );
        }

        if ( !pluginConstructSuccess )
        {
            typeInfo->structRegistry.DestroyPluginBlock( rtObj );
        }

        return pluginConstructSuccess;
    }

    inline void DestructPlugins( typeInfoBase *typeInfo, LuaRTTI *rtObj )
    {
        try
        {
            typeInfo->structRegistry.DestroyPluginBlock( rtObj );
        }
        catch( ... )
        {
            lua_assert( 0 );
        }

        if ( typeInfoBase *inheritedClass = typeInfo->inheritsFrom )
        {
            DestructPlugins( inheritedClass, rtObj );
        }
    }

    inline size_t GetTypeStructSize( typeInfoBase *typeInfo, void *construct_params )
    {
        typeInterface *tInterface = typeInfo->tInterface;

        // Attempt to get the memory the language object will take.
        size_t objMemSize = tInterface->GetTypeSize( construct_params );

        if ( objMemSize != 0 )
        {
            // Adjust that objMemSize so we can store meta information + plugins.
            objMemSize += sizeof( LuaRTTI );

            // Calculate the memory that is required by all plugin structs.
            objMemSize += GetTypePluginSize( typeInfo );
        }

        return objMemSize;
    }

    inline size_t GetTypeStructSize( LuaRTTI *rtObj )
    {
        typeInfoBase *typeInfo = GetTypeInfoFromTypeStruct( rtObj );
        typeInterface *tInterface = typeInfo->tInterface;

        // Get the pointer to the object.
        void *langObj = GetObjectFromTypeStruct( rtObj );

        // Get the memory that is taken by the language object.
        size_t objMemSize = tInterface->GetTypeSizeByObject( langObj );

        if ( objMemSize != 0 )
        {
            // Take the meta data into account.
            objMemSize += sizeof( LuaRTTI );

            // Add the memory taken by the plugins.
            objMemSize += GetTypePluginSize( typeInfo );
        }
        
        return objMemSize;
    }

    inline void ReferenceTypeInfo( typeInfoBase *typeInfo )
    {
        typeInfo->refCount++;

        // For every type we inherit, we reference it as well.
        if ( typeInfoBase *inheritedClass = typeInfo->inheritsFrom )
        {
            ReferenceTypeInfo( inheritedClass );
        }
    }

    inline void DereferenceTypeInfo( typeInfoBase *typeInfo )
    {
        // For every type we inherit, we dereference it as well.
        if ( typeInfoBase *inheritedClass = typeInfo->inheritsFrom )
        {
            DereferenceTypeInfo( inheritedClass );
        }

        typeInfo->refCount--;
    }

    inline LuaRTTI* ConstructPlacement( void *objMem, typeInfoBase *typeInfo, void *construct_params )
    {
        LuaRTTI *objOut = NULL;
        {
            // Reference the type info.
            ReferenceTypeInfo( typeInfo );

            // Get the specialization interface.
            typeInterface *tInterface = typeInfo->tInterface;

            // Get a pointer to LuaRTTI and the object memory.
            LuaRTTI *objTypeMeta = (LuaRTTI*)objMem;

            // Initialize the RTTI struct.
            objTypeMeta->type_meta = typeInfo;
#ifdef _DEBUG
            objTypeMeta->typesys_ptr = this;
#endif //_DEBUG

            // Initialize the language object.
            void *objStruct = objTypeMeta + 1;

            try
            {
                // Attempt to construct the language part.
                tInterface->Construct( objStruct, construct_params );
            }
            catch( ... )
            {
                // We failed to construct the object struct, so it is invalid.
                objStruct = NULL;
            }

            if ( objStruct )
            {
                // Only proceed if we have successfully constructed the object struct.
                // Now construct the plugins.
                bool pluginConstructSuccess = ConstructPlugins( typeInfo, objTypeMeta );

                if ( pluginConstructSuccess )
                {
                    // We are finished! Return the meta info.
                    objOut = objTypeMeta;
                }
                else
                {
                    // We failed, so destruct the class again.
                    tInterface->Destruct( objStruct );
                }
            }

            if ( objOut == NULL )
            {
                // Since we did not return a proper object, dereference again.
                DereferenceTypeInfo( typeInfo );
            }
        }
        return objOut;
    }

    inline LuaRTTI* Construct( typeInfoBase *typeInfo, void *construct_params )
    {
        LuaRTTI *objOut = NULL;
        {
            size_t objMemSize = GetTypeStructSize( typeInfo, construct_params );

            if ( objMemSize != 0 )
            {
                void *objMem = _memAlloc->Allocate( objMemSize );

                if ( objMem )
                {
                    // Attempt to construct the object on the memory.
                    objOut = ConstructPlacement( objMem, typeInfo, construct_params );
                }

                if ( !objOut )
                {
                    // Deallocate the memory again, as we seem to have failed.
                    _memAlloc->Free( objMem, objMemSize );
                }
            }
        }
        return objOut;
    }

    inline void SetTypeInfoInheritingClass( typeInfoBase *subClass, typeInfoBase *inheritedClass )
    {
        bool subClassImmutability = subClass->IsImmutable();

        lua_assert( subClassImmutability == false );

        if ( subClassImmutability == false )
        {
            if ( typeInfoBase *prevInherit = subClass->inheritsFrom )
            {
                prevInherit->inheritanceCount--;
            }

            subClass->inheritsFrom = inheritedClass;

            if ( inheritedClass )
            {
                inheritedClass->inheritanceCount++;
            }
        }
    }

    static inline void* GetObjectFromTypeStruct( LuaRTTI *rtObj )
    {
        return (void*)( rtObj + 1 );
    }

    static inline LuaRTTI* GetTypeStructFromObject( void *langObj )
    {
        return ( (LuaRTTI*)langObj - 1 );
    }

    inline LuaRTTI* GetTypeStructFromAbstractObject( void *langObj )
    {
        LuaRTTI *typeInfo = ( (LuaRTTI*)langObj - 1 );

#ifdef _DEBUG
        // If this assertion fails, Lua types from different global_States may have
        // been mixed up together. Then there is an error in the usage of the Lua API!
        lua_assert( typeInfo->typesys_ptr == this );
#endif //_DEBUG

        return typeInfo;
    }

    inline void DestroyPlacement( LuaRTTI *typeStruct )
    {
        typeInfoBase *typeInfo = GetTypeInfoFromTypeStruct( typeStruct );
        typeInterface *tInterface = typeInfo->tInterface;

        // Destroy all object plugins.
        DestructPlugins( typeInfo, typeStruct );

        // Pointer to the language object.
        void *langObj = (void*)( typeStruct + 1 );

        // Destroy the actual object.
        try
        {
            tInterface->Destruct( langObj );
        }
        catch( ... )
        {
            // We cannot handle this, since it must not happen in the first place.
            lua_assert( 0 );
        }

        // Dereference the type info since we do not require it anymore.
        DereferenceTypeInfo( typeInfo );
    }

    inline void Destroy( LuaRTTI *typeStruct )
    {
        // Get the actual type struct size.
        size_t objMemSize = GetTypeStructSize( typeStruct );

        lua_assert( objMemSize != 0 );  // it cannot be zero.

        // Delete the object from the memory.
        DestroyPlacement( typeStruct );

        // Free the memory.
        void *rttiMem = (void*)typeStruct;

        _memAlloc->Free( rttiMem, objMemSize );
    }

    inline void DeleteType( typeInfoBase *typeInfo )
    {
        LIST_REMOVE( typeInfo->node );

        // Make sure we do not inherit from anything anymore.
        SetTypeInfoInheritingClass( typeInfo, NULL );

        typeInfo->Cleanup( *_memAlloc );
    }

    struct type_iterator
    {
        RwList <typeInfoBase>& listRoot;
        RwListEntry <typeInfoBase> *curNode;

        inline type_iterator( LuaTypeSystem& typeSys ) : listRoot( typeSys.registeredTypes )
        {
            this->curNode = listRoot.root.next;
        }

        inline bool IsEnd( void ) const
        {
            return ( &listRoot.root == curNode );
        }

        inline typeInfoBase* Resolve( void ) const
        {
            return LIST_GETITEM( typeInfoBase, curNode, node );
        }

        inline void Increment( void )
        {
            this->curNode = this->curNode->next;
        }
    };

    inline type_iterator GetTypeIterator( void )
    {
        return type_iterator( *this );
    }
};

#endif //_LUA_TYPE_ABSTRACTION_SYSTEM_