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

        // Inheritance information.
        typeInfoBase *inheritsFrom; // type that this type inherits from

        // Plugin information.
        typedef AnonymousPluginStructRegistry <LuaRTTI> structRegistry_t;
        structRegistry_t structRegistry;

        RwListEntry <typeInfoBase> node;
    };

    RwList <typeInfoBase> registeredTypes;

    inline void SetupTypeInfoBase( typeInfoBase *tInfo, const char *typeName, typeInterface *tInterface )
    {
        tInfo->name = typeName;
        tInfo->refCount = 0;
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
        bool pluginConstructSuccess = typeInfo->structRegistry.ConstructPluginBlock( rtObj );

        if ( pluginConstructSuccess )
        {
            if ( typeInfoBase *inheritedClass = typeInfo->inheritsFrom )
            {
                bool parentSuccess = ConstructPlugins( inheritedClass, rtObj );

                if ( !parentSuccess )
                {
                    pluginConstructSuccess = false;
                }
            }

            if ( !pluginConstructSuccess )
            {
                typeInfo->structRegistry.DestroyPluginBlock( rtObj );
            }
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
        typeInfoBase *typeInfo = (typeInfoBase*)rtObj->type_meta;
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
            subClass->inheritsFrom = inheritedClass;
        }
    }

    inline void* GetObjectFromTypeStruct( LuaRTTI *rtObj )
    {
        return (void*)( rtObj + 1 );
    }

    inline LuaRTTI* GetTypeStructFromObject( void *langObj )
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
        typeInfoBase *typeInfo = (typeInfoBase*)typeStruct->type_meta;
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

        typeInfo->Cleanup( *_memAlloc );
    }
};

#endif //_LUA_TYPE_ABSTRACTION_SYSTEM_