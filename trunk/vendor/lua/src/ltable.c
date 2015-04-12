/*
** $Id: ltable.c,v 2.32.1.2 2007/12/28 15:32:23 roberto Exp $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/


/*
** Implementation of tables (aka arrays, objects, or hash tables).
** Tables keep its elements in two parts: an array part and a hash part.
** Non-negative integer keys are all candidates to be kept in the array
** part. The actual size of the array is the largest `n' such that at
** least half the slots between 0 and n are in use.
** Hash uses a mix of chained scatter table with Brent's variation.
** A main invariant of these tables is that, if an element is not
** in its main position (i.e. the `original' position that its hash gives
** to it), then the colliding element is in its own main position.
** Hence even when the load factor reaches 100%, performance remains good.
*/

#include "luacore.h"

#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "ltable.h"
#include "lvm.h"

#include "lpluginutil.hxx"


/*
** max size of array part is 2^MAXBITS
*/
#if LUAI_BITSINT > 26
#define MAXBITS		26
#else
#define MAXBITS		(LUAI_BITSINT-2)
#endif

#define MAXASIZE	(1 << MAXBITS)


#define hashpow2(t,n)      (gnode(t, lmod((n), sizenode(t))))
  
#define hashstr(t,str)  hashpow2(t, (str)->hash)
#define hashboolean(t,p)        hashpow2(t, p)


/*
** for some types, it is better to avoid modulus by power of 2, as
** they tend to have many 2 factors.
*/
#define hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1)|1))))


#define hashpointer(t,p)	hashmod(t, IntPoint(p))


// Table type information plugin.
struct tableTypeInfoPlugin
{
    inline void Initialize( lua_config *cfg )
    {
        LuaTypeSystem& typeSys = cfg->typeSys;

        // Register type information.
        tableTypeInfo = typeSys.RegisterStructType <Table> ( "table" );

        // Set inheritance information.
        typeSys.SetTypeInfoInheritingClass(
            tableTypeInfo,
            cfg->grayobjTypeInfo
        );
    }

    inline void Shutdown( lua_config *cfg )
    {
        LuaTypeSystem& typeSys = cfg->typeSys;

        // Delete the types again.
        typeSys.DeleteType( tableTypeInfo );
    }

    LuaTypeSystem::typeInfoBase *tableTypeInfo;
};

PluginDependantStructRegister <tableTypeInfoPlugin, namespaceFactory_t> tableTypeInfo( namespaceFactory );

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

tableEnvConnectingBridge_t tableEnvConnectingBridge( namespaceFactory );

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

// Lua table environment initializator.
void luaH_init( lua_config *cfg )
{
    return;
}

void luaH_shutdown( lua_config *cfg )
{
    return;
}

void luaH_clearRuntimeMemory( global_State *g )
{
    globalStateTableEnvPlugin *tableEnv = GetGlobalTableEnv( g );

    if ( tableEnv )
    {
        tableEnv->ClearMemory( g );
    }
}


/*
** hash for lua_Numbers
*/
static Node *hashnum (const Table *t, lua_Number n)
{
    if (luai_numeq(n, 0))  /* avoid problems with -0 */
    {
        return gnode(t, 0);
    }

    const size_t numInts = ( sizeof( lua_Number ) / sizeof( unsigned int ) );

    union
    {
        lua_Number theNum;
        unsigned int a[ numInts ];
    };

    theNum = n;

    for ( size_t i = 1; i < numInts; i++ )
    {
        a[0] += a[i];
    }

    return hashmod(t, a[0]);
}



/*
** returns the `main' position of an element in a table (that is, the index
** of its hash value)
*/
static Node *mainposition (const Table *t, const TValue *key)
{
    switch (ttype(key))
    {
    case LUA_TNUMBER:
        return hashnum(t, nvalue(key));
    case LUA_TSTRING:
        return hashstr(t, tsvalue(key));
    case LUA_TBOOLEAN:
    {
        int b = bvalue(key);
        return hashboolean(t, b);
    }
    case LUA_TLIGHTUSERDATA:
        return hashpointer(t, pvalue(key));
    default:
        return hashpointer(t, gcvalue(key));
    }
}


/*
** returns the index for `key' if `key' is an appropriate key to live in
** the array part of the table, -1 otherwise.
*/
static inline long arrayindex( const TValue *key )
{
    if (ttisnumber(key))
    {
        lua_Number n = nvalue(key);
        int k;

        lua_number2int(k, n);

        if (luai_numeq(cast_num(k), n))
        {
            return k;
        }
    }
    return -1;  /* `key' did not match some condition */
}

FASTAPI bool isarrayindex( int index, int arraySize )
{
    return ( 0 < index && index <= arraySize );
}

FASTAPI bool isarrayindex_unsigned( int index, int arraySize )
{
    return ( cast(unsigned int, index-1) < cast(unsigned int, arraySize) );
}

/*
** returns the index of a `key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signalled by -1.
*/
static inline int findindex (lua_State *L, Table *t, ConstValueAddress& key )
{
    if ( ttisnil(key) )
    {
        return -1;  /* first iteration */
    }

    int i = arrayindex( key );

    if ( isarrayindex( i, t->sizearray ) )  /* is `key' inside array part? */
    {
        return i-1;  /* yes; that's the index (corrected to C) */
    }
    else
    {
        Node *n = mainposition(t, key);

        do
        {  /* check whether `key' is somewhere in the chain */
            /* key may be dead already, but it is ok to use it in `next' */
            if (luaO_rawequalObj(key2tval(n), key) || (ttype(gkey(n)) == LUA_TDEADKEY && iscollectable(key) && gcvalue(gkey(n)) == gcvalue(key)))
            {
                i = cast_int(n - gnode(t, 0));  /* key index in hash table */
                /* hash elements are numbered after array ones */
                return i + t->sizearray;
            }
            else
            {
                n = gnext(n);
            }
        } while (n);

        luaG_runerror(L, "invalid key to " LUA_QL("next"));  /* key not found */
    }

    return 0;  /* to avoid warnings */
}

bool luaH_next( lua_State *L, Table *t )
{
    int i;
    {
        ConstValueAddress keyval = index2constadr( L, -1 );

        i = findindex( L, t, keyval );  /* find original element */

        popstack( L, 1 );
    }

    for ( i++; i < t->sizearray; i++ )
    {  /* try first array part */
        if ( !ttisnil( &t->array[i] ) )
        {  /* a non-nil value? */
            pushnvalue( L, cast_num(i+1) );
            pushtvalue( L, &t->array[i] );
            return true;
        }
    }
    for ( i -= t->sizearray; i < sizenode(t); i++ )
    {  /* then hash part */
        if ( !ttisnil( gval(gnode(t, i)) ) )
        {  /* a non-nil value? */
            pushtvalue( L, key2tval(gnode(t, i)) );
            pushtvalue( L, gval(gnode(t, i)) );
            return true;
        }
    }

    return false;  /* no more elements */
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

static int computesizes (int nums[], int *narray)
{
    int i;
    int twotoi;  /* 2^i */
    int a = 0;  /* number of elements smaller than 2^i */
    int na = 0;  /* number of elements to go to array part */
    int n = 0;  /* optimal size for array part */

    for (i = 0, twotoi = 1; twotoi/2 < *narray; i++, twotoi *= 2)
    {
        if (nums[i] > 0)
        {
            a += nums[i];

            if (a > twotoi/2)
            {  /* more than half elements present? */
                n = twotoi;  /* optimal size (till now) */
                na = a;  /* all elements smaller than n will go to array part */
            }
        }
        if (a == *narray)
        {
            break;  /* all elements already counted */
        }
    }

    *narray = n;

    lua_assert(*narray/2 <= na && na <= *narray);

    return na;
}

static int countint (const TValue *key, int *nums)
{
    int k = arrayindex(key);

    if ( isarrayindex( k, MAXASIZE ) )
    {  /* is `key' an appropriate array index? */
        nums[ ceillog2(k) ]++;  /* count as such */
        return 1;
    }

    return 0;
}

static int numusearray (const Table *t, int *nums)
{
    int lg;
    int ttlg;  /* 2^lg */
    int ause = 0;  /* summation of `nums' */
    int i = 1;  /* count to traverse all array keys */

    for ( lg = 0, ttlg = 1; lg <= MAXBITS; lg++, ttlg*=2 )
    {  /* for each slice */
        int lc = 0;  /* counter */
        int lim = ttlg;

        if ( lim > t->sizearray )
        {
            lim = t->sizearray;  /* adjust upper limit */

            if (i > lim)
            {
                break;  /* no more elements to count */
            }
        }
        /* count elements in range (2^(lg-1), 2^lg] */
        for ( ; i <= lim; i++ )
        {
            if ( !ttisnil(&t->array[i-1]) )
            {
                lc++;
            }
        }

        nums[lg] += lc;
        ause += lc;
    }

    return ause;
}

static int numusehash (const Table *t, int *nums, int *pnasize)
{
    int totaluse = 0;  /* total number of elements */
    int ause = 0;  /* summation of `nums' */
    int i = sizenode(t);

    while ( i-- )
    {
        Node *n = &t->node[i];

        if ( !ttisnil(gval(n)) )
        {
            ause += countint(key2tval(n), nums);
            totaluse++;
        }
    }
    *pnasize += ause;

    return totaluse;
}

static void setarrayvector (lua_State *L, Table *t, int size)
{
    luaM_reallocvector( L, t->array, t->sizearray, size );

    for ( int i = t->sizearray; i < size; i++ )
    {
        setnilvalue(&t->array[i]);
    }
    t->sizearray = size;
}

static void setnodevector (lua_State *L, Table *t, int size)
{
    int lsize;

    if ( size == 0 )
    {  /* no elements to hash part? */
        t->node = GetDummyNode( G(L) );  /* use common `dummynode' */
        lsize = 0;
    }
    else
    {
        lsize = ceillog2(size);

        if ( lsize > MAXBITS )
        {
            luaG_runerror(L, "table overflow");
        }

        size = twoto(lsize);
        t->node = luaM_newvector <Node> (L, size);

        for ( int i = 0; i < size; i++ )
        {
            Node *n = gnode(t, i);

            gnext(n) = NULL;

            setnilvalue(gkey(n));
            setnilvalue(gval(n));
        }
    }

    t->lsizenode = cast_byte(lsize);
    t->lastfree = gnode(t, size);  /* all positions are free */
}

static void resize (lua_State *L, Table *t, int nasize, int nhsize)
{
    int oldasize = t->sizearray;
    int oldhsize = t->lsizenode;
    Node *nold = t->node;  /* save old hash ... */

    if ( nasize > oldasize )  /* array part must grow? */
    {
        setarrayvector(L, t, nasize);
    }

    /* create new hash part with appropriate size */
    setnodevector(L, t, nhsize);  

    if ( nasize < oldasize )
    {  /* array part must shrink? */
        t->sizearray = nasize;

        /* re-insert elements from vanishing slice */
        for ( int i = nasize; i < oldasize; i++ )
        {
            if ( !ttisnil(&t->array[i]) )
            {
                ValueAddress numObj = luaH_setnum(L, t, i+1);

                setobjt2t(L, numObj, &t->array[i]);
            }
        }

        /* shrink array */
        luaM_reallocvector(L, t->array, oldasize, nasize);
    }

    /* re-insert elements from hash part */
    for ( int i = twoto(oldhsize) - 1; i >= 0; i-- )
    {
        Node *old = nold+i;

        if ( !ttisnil(gval(old)) )
        {
            ValueAddress tableItemPtr = luaH_set(L, t, key2tval(old));

            setobjt2t( L, tableItemPtr, gval(old) );
        }
    }

    if ( nold != GetDummyNode( G(L) ) )
    {
        luaM_freearray( L, nold, twoto(oldhsize) );  /* free old array */
    }
}

void luaH_resizearray (lua_State *L, Table *t, int nasize)
{
    int nsize = (t->node == GetDummyNode( G(L) )) ? 0 : sizenode(t);
    resize(L, t, nasize, nsize);
}

static void rehash (lua_State *L, Table *t, const TValue *ek)
{
    int nums[MAXBITS+1];  /* nums[i] = number of keys between 2^(i-1) and 2^i */

    for ( int i = 0; i <= MAXBITS; i++ )
    {
        nums[i] = 0;  /* reset counts */
    }

    int nasize = numusearray(t, nums);  /* count keys in array part */
    int totaluse = nasize;  /* all those keys are integer keys */
    totaluse += numusehash(t, nums, &nasize);  /* count keys in hash part */
    /* count extra key */
    nasize += countint(ek, nums);
    totaluse++;

    /* compute new size for array part */
    int na = computesizes(nums, &nasize);
    /* resize the table to new computed sizes */
    resize(L, t, nasize, totaluse - na);
}


/*
** }=============================================================
*/

Table::Table( global_State *g, void *construction_params ) : GrayObject( g )
{
    // Initialize the table.
    luaC_link(g, this, LUA_TTABLE);
    this->metatable = NULL;
    this->flags = cast_byte(~0);
    /* temporary values (kept only if some malloc fails) */
    this->array = NULL;
    this->sizearray = 0;
    this->lsizenode = 0;
    this->node = GetDummyNode( g );
    this->lastfree = NULL;
}

Table::~Table( void )
{
    // Clean up runtime data of the table.
    lua_State *L = gstate->mainthread;

    if ( this->node != GetDummyNode( gstate ) )
    {
        luaM_freearray( L, this->node, sizenode( this ) );
    }

    luaM_freearray( L, this->array, this->sizearray );
}

Table *luaH_new (lua_State *L, int narray, int nhash)
{
    global_State *g = G(L);

    // Get the table type info plugin.
    tableTypeInfoPlugin *typeInfo = tableTypeInfo.GetPluginStruct( g->config );

    // No point in creating table if we cannot get our type information.
    if ( !typeInfo )
        return NULL;

    // Construct the table using its factory.
    Table *t = lua_new <Table> ( L, typeInfo->tableTypeInfo );

    if ( t )
    {
        setarrayvector(L, t, narray);
        setnodevector(L, t, nhash);
    }
    return t;
}

void luaH_free (lua_State *L, Table *t)
{
    // Destroy the table.
    lua_delete <Table> ( L, t );
}

static inline Node* getfreepos (Table *t)
{
    if ( t->lastfree == NULL )
    {
        return NULL;
    }

    while ( t->lastfree-- > t->node )
    {
        if ( ttisnil( gkey(t->lastfree) ) )
        {
            return t->lastfree;
        }
    }
    return NULL;  /* could not find a free place */
}



/*
** inserts a new key into a hash table; first, check whether key's main 
** position is free. If not, check whether colliding node is in its main 
** position or not: if it is not, move colliding node to an empty place and 
** put new key in its main position; otherwise (colliding node is in its main 
** position), new key goes to an empty position. 
*/
template <typename valueType>
bool _findkey( Table *t, const TValue *key, valueType*& valueOut )
{
    bool foundValue = false;

    TValue *retValue = NULL;

    switch (ttype(key))
    {
    case LUA_TNIL:
        return false;
    case LUA_TSTRING:
        foundValue = _findkeybystr( t, tsvalue( key ), retValue );
        break;
    case LUA_TNUMBER:
    {
        int k;
        lua_Number n = nvalue(key);
        lua_number2int(k, n);

        if ( luai_numeq(cast_num(k), nvalue(key)) ) /* index is int? */
        {
            foundValue = _findkeybynum( t, k, retValue );  /* use specialized version */
            break;
        }
        /* else go through */
    }
    default:
    {
        Node *n = mainposition(t, key);

        do
        {  /* check whether `key' is somewhere in the chain */
            if (luaO_rawequalObj(key2tval(n), key))
            {
                retValue = gval(n);  /* that's it */

                foundValue = true;
                break;
            }
            else
            {
                n = gnext(n);
            }
        }
        while (n);

        break;
    }
    }

    if ( foundValue )
    {
        valueOut = retValue;
    }

    return foundValue;
}

static TValue* newkey (lua_State *L, Table *t, const TValue *key)
{
    // Get the dummy node.
    const Node *dummynode = GetDummyNode( G(L) );

    Node *mp = mainposition(t, key);

    if ( !ttisnil(gval(mp)) || mp == dummynode )
    {
        Node *othern;
        Node *n = getfreepos(t);  /* get a free place */

        if (n == NULL)
        {  /* cannot find a free place? */
            rehash(L, t, key);  /* grow table */

            TValue *newVal = NULL;

            bool hasFoundKey = _findkey( t, key, newVal );

            if ( !hasFoundKey )
            {
                newVal = newkey( L, t, key ); /* re-insert key into grown table */
            }

            lua_assert( newVal != NULL );
            return newVal;
        }

        lua_assert(n != dummynode);

        othern = mainposition(t, key2tval(mp));

        if (othern != mp)
        {  /* is colliding node out of its main position? */
            /* yes; move colliding node into free position */
            while (gnext(othern) != mp)
            {
                othern = gnext(othern);  /* find previous */
            }

            gnext(othern) = n;  /* redo the chain with `n' in place of `mp' */
            *n = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
            gnext(mp) = NULL;  /* now `mp' is free */

            setnilvalue(gval(mp));
        }
        else
        {  /* colliding node is in its own main position */
            /* new node will go into free position */
            gnext(n) = gnext(mp);  /* chain new position */
            gnext(mp) = n;
            mp = n;
        }
    }
    gkey(mp)->value = key->value; gkey(mp)->tt = key->tt;
    luaC_barriert(L, t, key);

    lua_assert(ttisnil(gval(mp)));
    return gval(mp);
}

/*
** search function for integers
*/
template <typename valueType>
bool _findkeybynum( Table *t, int key, valueType*& valueOut )
{
    bool valueFound = false;

    TValue *retValue = NULL;

    /* (1 <= key && key <= t->sizearray) */
    if ( isarrayindex_unsigned( key, t->sizearray ) )
    {
        retValue = &t->array[key-1];

        valueFound = true;
    }
    else
    {
        lua_Number nk = cast_num(key);
        Node *n = hashnum(t, nk);

        do
        {  /* check whether `key' is somewhere in the chain */
            if ( ttisnumber(gkey(n)) && luai_numeq(nvalue(gkey(n)), nk) )
            {
                retValue = gval(n);  /* that's it */

                valueFound = true;
                break;
            }
            else
            {
                n = gnext(n);
            }
        }
        while (n);
    }

    if ( valueFound )
    {
        valueOut = retValue;
    }

    return valueFound;
}

ConstValueAddress luaH_getnum (lua_State *L, Table *t, int key)
{
    ConstValueAddress retAddr;

    const TValue *retValue = luaO_nilobject;
    
    // Give it access to the value.
    _findkeybynum( t, key, retValue );
    {
        TableValueConstAccessContext *ctx = NewTableValueConstAccessContext( L, t );

        setnvalue( &ctx->key, cast_num(key) );
        ctx->value = retValue;

        retAddr.Setup( L, ctx );
    }

    return retAddr;
}

/*
** search function for strings
*/
template <typename valueType>
bool _findkeybystr( Table *t, const TString *key, valueType*& valueOut )
{
    bool valueFound = false;

    TValue *retValue = NULL;

    Node *n = hashstr(t, key);

    do
    {  /* check whether `key' is somewhere in the chain */
        if ( ttisstring(gkey(n)) && tsvalue(gkey(n)) == key )
        {
            retValue = gval(n);  /* that's it */

            valueFound = true;
            break;
        }
        else
        {
            n = gnext(n);
        }
    }
    while (n);

    if ( valueFound )
    {
        valueOut = retValue;
    }

    return valueFound;
}

ConstValueAddress luaH_getstr (lua_State *L, Table *t, TString *key)
{
    ConstValueAddress retAddr;

    const TValue *retValue = luaO_nilobject;

    // Give it access to the value.
    _findkeybystr( t, key, retValue );
    {
        TableValueConstAccessContext *ctx = NewTableValueConstAccessContext( L, t );

        setsvalue( L, &ctx->key, key );
        ctx->value = retValue;

        retAddr.Setup( L, ctx );
    }

    return retAddr;
}

/*
** main search function
*/
ConstValueAddress luaH_get (lua_State *L, Table *t, const TValue *key)
{
    ConstValueAddress retAddr;

    const TValue *retValue = luaO_nilobject;

    // Give access to the value.
    _findkey( t, key, retValue );
    {
        TableValueConstAccessContext *ctx = NewTableValueConstAccessContext( L, t );

        setobj( L, &ctx->key, key );
        ctx->value = retValue;

        retAddr.Setup( L, ctx );
    }

    return retAddr;
}

ValueAddress luaH_set (lua_State *L, Table *t, const TValue *key)
{
    ValueAddress retAddr;

    TValue *returnValue = NULL;
    {
        bool valueExists = _findkey( t, key, returnValue );

        // Reset optimization flags of the table.
        t->flags = 0;

        if ( !valueExists )
        {
            if (ttisnil(key))
            {
                luaG_runerror(L, "table index is nil");
            }
            else if (ttisnumber(key) && luai_numisnan(nvalue(key)))
            {
                luaG_runerror(L, "table index is NaN");
            }

            returnValue = newkey(L, t, key);
        }
    }

    // Give access to the value.
    {
        TableValueAccessContext *ctx = NewTableValueAccessContext( L, t );

        setobj( L, &ctx->key, key );
        ctx->value = returnValue;

        retAddr.Setup( L, ctx );
    }

    lua_assert( retAddr != &GetDummyNode( G(L) )->i_val );
    
    return retAddr;
}

ValueAddress luaH_setnum (lua_State *L, Table *t, int key)
{
    ValueAddress retAddr;

    TValue *returnValue = NULL;
    {
        bool valueExists = _findkeybynum( t, key, returnValue );

        if ( !valueExists )
        {
            TValue k;
            setnvalue(&k, cast_num(key));
            returnValue = newkey(L, t, &k);
        }
    }

    // Give access to the value.
    {
        TableValueAccessContext *ctx = NewTableValueAccessContext( L, t );

        setnvalue( &ctx->key, cast_num(key) );
        ctx->value = returnValue;

        retAddr.Setup( L, ctx );
    }

    lua_assert( retAddr != &GetDummyNode( G(L) )->i_val );

    return retAddr;
}

ValueAddress luaH_setstr (lua_State *L, Table *t, TString *key)
{
    ValueAddress retAddr;

    TValue *returnValue = NULL;
    {
        bool hasValue = _findkeybystr( t, key, returnValue );

        if ( !hasValue )
        {
            TValue k;
            setsvalue(L, &k, key);
            returnValue = newkey(L, t, &k);
        }
    }

    // Give access to the value.
    {
        TableValueAccessContext *ctx = NewTableValueAccessContext( L, t );

        setsvalue( L, &ctx->key, key );
        ctx->value = returnValue;

        retAddr.Setup( L, ctx );
    }

    lua_assert( retAddr != &GetDummyNode( G(L) )->i_val );

    return retAddr;
}

static inline int unbound_search (lua_State *L, Table *t, unsigned int j)
{
    unsigned int i = j;  /* i is zero or a present index */

    j++;

    /* find `i' and `j' such that i is present and j is not */
    while ( true )
    {
        ConstValueAddress jval = luaH_getnum( L, t, j );

        if ( ttisnil(jval) )
            break;

        i = j;
        j *= 2;

        if ( j > cast(unsigned int, MAX_INT) )
        {  /* overflow? */
            /* table was built with bad purposes: resort to linear search */
            i = 1;

            while ( true )
            {
                ConstValueAddress ival = luaH_getnum( L, t, i );

                if ( ttisnil(ival) )
                    break;

                i++;
            }

            return i - 1;
        }
    }
    /* now do a binary search between them */
    while ( j - i > 1 )
    {
        unsigned int m = (i+j)/2;

        ConstValueAddress mval = luaH_getnum( L, t, m );

        if ( ttisnil(mval) )
        {
            j = m;
        }
        else
        {
            i = m;
        }
    }
    return i;
}


/*
** Try to find a boundary in table `t'. A `boundary' is an integer index
** such that t[i] is non-nil and t[i+1] is nil (and 0 if t[1] is nil).
*/
int luaH_getn (lua_State *L, Table *t)
{
    unsigned int j = t->sizearray;

    if ( j > 0 && ttisnil(&t->array[j - 1]) )
    {
        /* there is a boundary in the array part: (binary) search for it */
        unsigned int i = 0;

        while ( j - i > 1 )
        {
            unsigned int m = (i+j)/2;

            if ( ttisnil(&t->array[m - 1]) )
            {
                j = m;
            }
            else
            {
                i = m;
            }
        }

        return i;
    }
    /* else must find a boundary in hash part */
    else if ( t->node == GetDummyNode( G(L) ) )  /* hash part is empty? */
    {
        return j;  /* that is easy... */
    }

    // Fall back.
    return unbound_search(L, t, j);
}

Table& luaH_copy( lua_State *L, const Table& t )
{
    Table& h = *luaH_new( L, t.sizearray, (int)pow( 2.0f, t.lsizenode ) );

    for ( int n = 0; n < t.sizearray; n++ )
    {
        h.array[n].tt = t.array[n].tt;
        h.array[n].value = t.array[n].value;
    }

    return h;
}

#if defined(LUA_DEBUG)

Node *luaH_mainposition (const Table *t, const TValue *key)
{
    return mainposition(t, key);
}

#endif

void Table::Index( lua_State *L, ConstValueAddress& key, ValueAddress& val )
{
    ConstValueAddress res = luaH_get( L, this, key ); /* do a primitive get */
    ConstValueAddress tm;

    if ( !ttisnil(res) || (tm = fasttm( L, metatable, TM_INDEX )) == NULL )
    {
        setobj2s(L, val, res);
    }
    else if ( ttisfunction( tm ) )
    {
        rtStack_t& rtStack = L->rtStack;

        rtStack.Lock( L );

        luaD_checkstack( L, 3 );    // we allocate the stack here!

        // Working with the new stack
        pushclvalue( L, clvalue( tm ) );
        pushhvalue( L, this );
        pushtvalue( L, key );
        lua_call( L, 2, 1 );

        // Store at appropriate position
        popstkvalue( L, val );

        rtStack.Unlock( L );
    }
    else
        luaV_gettable( L, tm, key, val );
}

void Table::NewIndex( lua_State *L, ConstValueAddress& key, ConstValueAddress& val )
{
    ValueAddress oldval = luaH_set( L, this, key ); /* do a primitive set */
    ConstValueAddress tm;

    if ( !ttisnil(oldval) || ( tm = fasttm( L, metatable, TM_NEWINDEX )) == NULL )
    {
        setobj2t(L, oldval, val);
    }
    else if ( ttisfunction( tm ) )
    {
        rtStack_t& rtStack = L->rtStack;

        rtStack.Lock( L );

        luaD_checkstack( L, 4 );    // allocate a new stack

        // Work with the new stack
        pushclvalue( L, clvalue( tm ) );
        pushhvalue( L, this );
        pushtvalue( L, key );
        pushtvalue( L, val );
        lua_call( L, 3, 0 );

        rtStack.Unlock( L );
    }
    else
    {
        // Repeat the process with the index object
        // For security reasons, we should increment the callstack depth
        callstack_ref indexRef( *L );

        luaV_settable( L, tm, key, val );
    }
}