#ifndef _LUA_TABLE_NATIVE_IMPLEMENTATION_
#define _LUA_TABLE_NATIVE_IMPLEMENTATION_

typedef union TKey
{
    struct : TValue
    {
        struct Node *next;  /* for chaining */
    } nk;
    TValue tvk;
} TKey;

struct Node
{
    TValue i_val;
    TKey i_key;
};

// Utility macros.
#define gnode(t,i)	(&(t)->node[i])
#define gkey(n)		(&(n)->i_key.nk)
#define gval(n)		(&(n)->i_val)
#define gnext(n)	((n)->i_key.nk.next)

#define key2tval(n)	(&(n)->i_key.tvk)

#define sizenode(t)	(twoto((t)->lsizenode))


#include "lpluginutil.hxx"

// Table access contexts for TValue reading and writing.
template <typename valueType, bool updateBarrier>
struct TableValueAccessContextTemplate : public DataContext <valueType>
{
    typedef LuaCachedConstructedClassAllocator <TableValueAccessContextTemplate> allocator_t;

    allocator_t *_usedAlloc;

    unsigned long refCount;

    // These values have to be set on allocation.
    Table *linkedTable;

    TValue key;
    valueType *value;

    bool hasFetchedValuePointer;

    inline TableValueAccessContextTemplate( void )
    {
        this->refCount = 0;
        this->_usedAlloc = NULL;
        this->linkedTable = NULL;
        setnilvalue( &key );
        this->value = NULL;
        this->hasFetchedValuePointer = false;
    }

    void Reference( lua_State *L )
    {
        this->refCount++;
    }

    void Dereference( lua_State *L )
    {
        if ( updateBarrier && this->hasFetchedValuePointer )
        {
            // Update the table barrier.
            luaC_barriert( L, this->linkedTable, this->value );
        }

        if ( this->refCount-- == 1 )
        {
            this->_usedAlloc->Free( this );
        }
    }

    valueType* const* GetValuePointer( void )
    {
        this->hasFetchedValuePointer = true;

        return &value;
    }
};

typedef TableValueAccessContextTemplate <TValue, true> TableValueAccessContext;
typedef TableValueAccessContextTemplate <const TValue, false> TableValueConstAccessContext;

// Table environment plugin.
struct globalStateTableEnvPlugin
{
    inline globalStateTableEnvPlugin( void )
    {
        // Set up the dummy node.
        setnilvalue( &dummynode.i_key.nk );
        dummynode.i_key.nk.next = NULL;
        setnilvalue( &dummynode.i_val );
    }

    inline void Initialize( global_State *g )
    {
        return;
    }

    inline void ClearMemory( global_State *g )
    {
        _tvalueAccessAllocator.Shutdown( g );
        _tvalueConstAccessAllocator.Shutdown( g );
    }

    inline void Shutdown( global_State *g )
    {
        ClearMemory( g );
    }
    
    // Dummy node that is used for optimization.
    Node dummynode;

    // Allocator for TValue access contexts.
    TableValueAccessContext::allocator_t _tvalueAccessAllocator;
    TableValueConstAccessContext::allocator_t _tvalueConstAccessAllocator;
};

typedef PluginConnectingBridge
    <globalStateTableEnvPlugin,
        globalStateDependantStructFactoryMeta <globalStateTableEnvPlugin, globalStateFactory_t, lua_config>,
    namespaceFactory_t>
        tableEnvConnectingBridge_t;

extern tableEnvConnectingBridge_t tableEnvConnectingBridge;

inline globalStateTableEnvPlugin* GetGlobalTableEnv( global_State *g )
{
    return tableEnvConnectingBridge.GetPluginStruct( g->config, g );
}

inline Node* GetDummyNode( global_State *g )
{
    globalStateTableEnvPlugin *globalEnv = GetGlobalTableEnv( g );

    assert( globalEnv != NULL );

    return &globalEnv->dummynode;
}

inline TableValueAccessContext* NewTableValueAccessContext( lua_State *L, Table *t )
{
    globalStateTableEnvPlugin *tableEnv = GetGlobalTableEnv( G(L) );

    if ( tableEnv )
    {
        TableValueAccessContext *ctx = tableEnv->_tvalueAccessAllocator.Allocate( L );

        ctx->_usedAlloc = &tableEnv->_tvalueAccessAllocator;
        ctx->linkedTable = t;
        ctx->hasFetchedValuePointer = false;

        return ctx;
    }
    return NULL;
}

inline TableValueConstAccessContext* NewTableValueConstAccessContext( lua_State *L, Table *t )
{
    globalStateTableEnvPlugin *tableEnv = GetGlobalTableEnv( G(L) );

    if ( tableEnv )
    {
        TableValueConstAccessContext *ctx = tableEnv->_tvalueConstAccessAllocator.Allocate( L );

        ctx->_usedAlloc = &tableEnv->_tvalueConstAccessAllocator;
        ctx->linkedTable = t;
        ctx->hasFetchedValuePointer = false;

        return ctx;
    }
    return NULL;
}

struct tableNativeImplementation
{
    inline tableNativeImplementation( void )
    {
        this->flags = 0;
        this->lsizenode = 0;
        this->array = NULL;
        this->node = NULL;
        this->lastfree = NULL;
        this->sizearray = 0;
    }

    inline tableNativeImplementation( const tableNativeImplementation& right, global_State *g )
    {
        lua_State *L = g->mainthread;

        this->flags = right.flags;

        Node *rightNode = right.node;

        Node *dummyNode = GetDummyNode( g );

        Node *newNodes = rightNode;

        if ( newNodes != dummyNode )
        {
            int nodesize = sizenode(&right);

            newNodes = luaM_newvector <Node> ( L, nodesize );

            for ( int i = 0; i < nodesize; i++ )
            {
                const Node *srcNode = &rightNode[i];

                Node *newItem = &newNodes[i];

                // Decide about node linkage.
                const Node *srcNextNode = gnext(srcNode);

                if ( srcNextNode == NULL )
                {
                    gnext(newItem) = NULL;
                }
                else
                {
                    // Link into our array.
                    int nodeoff = ( srcNextNode - rightNode );

                    gnext(newItem) = &newNodes[ nodeoff ];
                }

                // Copy values.
                setobj( L, gkey(newItem), gkey(srcNode) );
                setobj( L, gval(newItem), gval(srcNode) );
            }
        }

        this->node = newNodes;
        this->lsizenode = right.lsizenode;

        // Now copy the last free identifier.
        Node *rightLastfree = right.lastfree;

        Node *newLastfree = NULL;

        if ( rightLastfree != NULL )
        {
            // We need to work with offsets.
            int lastfreeoff = ( rightLastfree - rightNode );

            newLastfree = newNodes + lastfreeoff;
        }

        this->lastfree = newLastfree;

        // Now copy the array of TValues.
        int sizearray = right.sizearray;

        TValue *newArray = NULL;

        if ( sizearray > 0 )
        {
            newArray = luaM_clonevector( g->mainthread, right.array, sizearray );
        }

        this->sizearray = sizearray;
        this->array = newArray;
    }

    inline ~tableNativeImplementation( void )
    {
        lua_assert( this->array == NULL );
    }

    inline void Initialize( global_State *g )
    {
        this->flags = cast_byte(~0);
        /* main table implementation parameters */
        this->array = NULL;
        this->sizearray = 0;
        this->lsizenode = 0;
        this->node = GetDummyNode( g );
        this->lastfree = NULL;
    }

    inline void Shutdown( global_State *g )
    {
        // Clean up runtime data of the table.
        lua_State *L = g->mainthread;

        if ( this->node != NULL )
        {
            if ( this->node != GetDummyNode( g ) )
            {
                luaM_freearray( L, this->node, sizenode( this ) );
            }

            this->node = NULL;
        }

        if ( this->array != NULL )
        {
            luaM_freearray( L, this->array, this->sizearray );

            this->array = NULL;
        }
    }

    lu_byte flags;  /* 1<<p means tagmethod(p) is not present */ 
    lu_byte lsizenode;  /* log2 of size of `node' array */
    TValue *array;  /* array part */
    Node *node;
    Node *lastfree;  /* any free position is before this position */
    int sizearray;  /* size of `array' array */
};

// Table type information plugin.
struct tableTypeInfoPlugin : public LuaTypeSystem::typeInterface
{
    // Type methods.
    void Construct( void *objMem, global_State *g, void *construct_params ) const
    {
        // First construct the table item.
        Table *newTable = new (objMem) Table( g, construct_params );

        // Now that it is constructed, we must remember to destroy it when something goes wrong.
        try
        {
            void *advObjMem = ( newTable + 1 );

            // Construct the advanced object.
            tableNativeImplementation *nativeImpl = new (advObjMem) tableNativeImplementation();

            try
            {
                // Initialize the object.
                nativeImpl->Initialize( g );
            }
            catch( ... )
            {
                nativeImpl->~tableNativeImplementation();

                throw;
            }
        }
        catch( ... )
        {
            newTable->~Table();

            throw;
        }
    }

    void CopyConstruct( void *objMem, const void *srcMem ) const
    {
        const Table *srcTable = (const Table*)srcMem;

        Table *newTable = new (objMem) Table( *srcTable );

        try
        {
            // Also clone the advanced struct.
            void *advObjMem = ( newTable + 1 );

            const void *srcAdvObjMem = ( srcTable + 1 );

            // Copy construct it.
            tableNativeImplementation *nativeImpl =
                new (advObjMem) tableNativeImplementation( *(const tableNativeImplementation*)srcAdvObjMem, srcTable->gstate );

            // Fin.
        }
        catch( ... )
        {
            newTable->~Table();

            throw;
        }
    }

    void Destruct( void *objMem ) const
    {
        Table *theTable = (Table*)objMem;

        // Destroy the advanced object first.
        {
            void *advObjMem = ( theTable + 1 );

            tableNativeImplementation *nativeImpl = (tableNativeImplementation*)advObjMem;

            nativeImpl->Shutdown( theTable->gstate );

            nativeImpl->~tableNativeImplementation();
        }

        // Finally kill our table object.
        ((Table*)objMem)->~Table();
    }

    size_t GetTypeSize( void *construct_params ) const
    {
        return sizeof( Table ) + sizeof( tableNativeImplementation );
    }

    size_t GetTypeSizeByObject( const void *mem ) const
    {
        return sizeof( Table ) + sizeof( tableNativeImplementation );
    }

    inline void Initialize( lua_config *cfg )
    {
        LuaTypeSystem& typeSys = cfg->typeSys;

        // Register type information.
        tableTypeInfo = typeSys.RegisterType( "table", this, cfg->grayobjTypeInfo );
    }

    inline void Shutdown( lua_config *cfg )
    {
        LuaTypeSystem& typeSys = cfg->typeSys;

        // Delete the types again.
        typeSys.DeleteType( tableTypeInfo );
    }
    LuaTypeSystem::typeInfoBase *tableTypeInfo;
};

extern PluginDependantStructRegister <tableTypeInfoPlugin, namespaceFactory_t> tableTypeInfo;

inline tableNativeImplementation* GetTableNativeImplementation( Table *tab )
{
    return (tableNativeImplementation*)( tab + 1 );
}

inline const tableNativeImplementation* GetTableConstNativeImplementation( const Table *tab )
{
    return (const tableNativeImplementation*)( tab + 1 );
}

#endif //_LUA_TABLE_NATIVE_IMPLEMENTATION_