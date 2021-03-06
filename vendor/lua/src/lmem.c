/*
** $Id: lmem.c,v 1.70.1.1 2007/12/27 13:02:25 roberto Exp $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/


#include "luacore.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lmem.h"
#include "lobject.h"

// Include garbage collector internals.
#include "lgc.internal.hxx"



/*
** About the realloc function:
** void * frealloc (void *ud, void *ptr, size_t osize, size_t nsize);
** (`osize' is the old size, `nsize' is the new size)
**
** Lua ensures that (ptr == NULL) iff (osize == 0).
**
** * frealloc(ud, NULL, 0, x) creates a new block of size `x'
**
** * frealloc(ud, p, x, 0) frees the block `p'
** (in this specific case, frealloc must return NULL).
** particularly, frealloc(ud, NULL, 0, 0) does nothing
** (which is equivalent to free(NULL) in ANSI C)
**
** frealloc returns NULL if it cannot create or reallocate the area
** (any reallocation to an equal or smaller size cannot fail!)
*/



#define MINSIZEARRAY	4


void *luaM_growaux_ (lua_State *L, void *block, int *size, size_t size_elems,
                     int limit, const char *errormsg) {
  void *newblock;
  int newsize;
  if (*size >= limit/2) {  /* cannot double it? */
    if (*size >= limit)  /* cannot grow even a little? */
      luaG_runerror(L, errormsg);
    newsize = limit;  /* still have at least one free place */
  }
  else {
    newsize = (*size)*2;
    if (newsize < MINSIZEARRAY)
      newsize = MINSIZEARRAY;  /* minimum size */
  }
  newblock = luaM_reallocv(L, block, *size, newsize, size_elems);
  *size = newsize;  /* update only when everything else is OK */
  return newblock;
}


void *luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
  return NULL;  /* to avoid warnings */
}



/*
** generic allocation routine.
*/
#include "lstate.lowlevel.hxx"

void *luaM_realloc__(global_State *g, void *block, size_t osize, size_t nsize)
{
    lua_assert((osize == 0) == (block == NULL));

    // Get the native allocation data.
    GlobalStateAllocPluginData *allocData = (GlobalStateAllocPluginData*)g->config->allocData;

    if ( !allocData )
        return NULL;

    block = allocData->_memAlloc.ReAlloc( block, osize, nsize );

    if (block == NULL && nsize > 0)
        return NULL;

    lua_assert((nsize == 0) == (block == NULL));

    g->totalbytes = (g->totalbytes - osize) + nsize;
    return block;
}

// For backwards compatibility.
void *luaM_realloc_ (lua_State *L, void *block, size_t osize, size_t nsize)
{
    void *memBlock = luaM_realloc__( G(L), block, osize, nsize );

    if ( memBlock == NULL && nsize > 0 )
        throw lua_exception( L, LUA_ERRMEM, "memory allocation failure" );

    return memBlock;
}

// Module initialization.
void luaM_init( lua_config *cfg )
{
    return;
}

void luaM_shutdown( lua_config *cfg )
{
    return;
}