/*
** $Id: lmem.h,v 1.31.1.1 2007/12/27 13:02:25 roberto Exp $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#ifndef lmem_h
#define lmem_h


#include <stddef.h>

#include "llimits.h"
#include "lua.h"

struct global_State;

#define MEMERRMSG	"not enough memory"

#define FASTAPI inline


LUAI_FUNC void *luaM_realloc__(global_State *g, void *block, size_t oldSize, size_t size);
LUAI_FUNC void *luaM_realloc_ (lua_State *L, void *block, size_t oldsize, size_t size);
LUAI_FUNC void *luaM_toobig   (lua_State *L);
LUAI_FUNC void *luaM_growaux_ (lua_State *L, void *block, int *size,
                               size_t size_elem, int limit,
                               const char *errormsg);

FASTAPI void* luaM_reallocv( lua_State *L, void *block, size_t oldN, size_t newN, size_t e )
{
    if ( cast(size_t, (newN)+1) > MAX_SIZET/(e) )  /* +1 to avoid warnings */ \
    {
        luaM_toobig( L );
        return NULL;
    }

    return luaM_realloc_( L, block, oldN*e, newN*e );
}
FASTAPI void luaM_freemem( lua_State *L, void *block, size_t memSize )
{
    luaM_realloc_( L, block, memSize, 0 );
}
template <typename structType>
FASTAPI void luaM_free( lua_State *L, structType *theData )
{
    luaM_realloc_( L, theData, sizeof(*theData), 0 );
}
template <typename structType>
FASTAPI void luaM_freearray( lua_State *L, structType *block, size_t n )
{
    for ( size_t i = 0; i < n; i++ )
    {
        (block+i)->~structType();
    }

    luaM_reallocv( L, block, n, 0, sizeof( structType ) );
}

FASTAPI void* luaM_malloc( lua_State *L, size_t memSize )
{
    return luaM_realloc_( L, NULL, 0, memSize );
}
template <typename structType>
FASTAPI structType* luaM_newvector( lua_State *L, size_t n )
{
    structType *vecOut = NULL;
    {
        void *mem = luaM_reallocv( L, NULL, 0, n, sizeof( structType ) );

        if ( mem )
        {
            for ( size_t i = 0; i < n; i++ )
            {
                new ((structType*)mem + i) structType;
            }

            vecOut = (structType*)mem;
        }
    }

    return vecOut;
}
template <typename structType>
FASTAPI void luaM_growvector( lua_State *L, structType*& vInOut, int nelems, int& size, size_t limit, const char *errormsg )
{
    if ( nelems + 1 > size )
    {
        void *vMem = vInOut;

        int oldSize = size;

        void *vPartial = luaM_growaux_( L, vMem, &size, sizeof(structType), limit, errormsg );

        // Construct the new objects.
        int newSize = size;

        for ( int n = oldSize; n < newSize; n++ )
        {
            new ((structType*)vPartial + n) structType;
        }

        vInOut = (structType*)vPartial;
    }
}
template <typename structType>
FASTAPI void luaM_reallocvector( lua_State *L, structType*& vInOut, size_t oldN, size_t newN )
{
    if ( oldN != newN )
    {
        // If the array should decrease itself, delete the entries that are removed.
        for ( size_t n = newN; n < oldN; n++ )
        {
            ( vInOut + n )->~structType();
        }

        // Reallocate the memory.
        void *vPartial = luaM_reallocv( L, vInOut, oldN, newN, sizeof( structType ) );

        // Create entries if the array increased in size.
        for ( size_t n = oldN; n < newN; n++ )
        {
            new ((structType*)vPartial + n) structType;
        }

        // The vector is complete now, return it.
        vInOut = (structType*)vPartial;
    }
}

// Module initialization.
LUAI_FUNC void luaM_init( void );
LUAI_FUNC void luaM_shutdown( void );

// Default class factory memory allocator.
struct LuaDefaultAllocator
{
    global_State *globalState;

    inline LuaDefaultAllocator( void )
    {
        this->globalState = NULL;
    }

    inline ~LuaDefaultAllocator( void )
    {
        return;
    }

    inline void SetState( global_State *theState )
    {
        this->globalState = theState;
    }

    inline void* Allocate( size_t memSize )
    {
        void *mem = luaM_realloc__( this->globalState, NULL, 0, memSize );
       
        lua_assert( mem != NULL );

        return mem;
    }

    inline void Free( void *ptr, size_t memSize )
    {
        luaM_realloc__( this->globalState, ptr, memSize, 0 );
    }
};

#endif

