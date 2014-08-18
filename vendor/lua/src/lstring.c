/*
** $Id: lstring.c,v 2.8.1.1 2007/12/27 13:02:25 roberto Exp $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/


#include "luacore.h"

#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ldebug.h"
#include "lvm.h"


// Global string plugin.
static globalStatePluginOffset_t _stringEnvPluginOffset = globalStateFactory_t::INVALID_PLUGIN_OFFSET;

struct globalStateStringEnv
{
    Mbuffer buff;  /* temporary buffer for string concatentation */
};

globalStateStringEnv* GetGlobalStringEnv( global_State *g )
{
    return globalStateFactory_t::RESOLVE_STRUCT <globalStateStringEnv> ( g, _stringEnvPluginOffset );
}

void luaS_resize (lua_State *L, int newsize) {
  GCObject **newhash;
  stringtable *tb;
  int i;
  if (luaC_getstate( L ) == GCSsweepstring)
    return;  /* cannot resize during GC traverse */
  newhash = luaM_newvector(L, newsize, GCObject *);
  tb = &G(L)->strt;
  for (i=0; i<newsize; i++) newhash[i] = NULL;
  /* rehash */
  for (i=0; i<tb->size; i++) {
    GCObject *p = tb->hash[i];
    while (p) {  /* for each node in the list */
      GCObject *next = p->next;  /* save next */
      unsigned int h = gco2ts(p)->hash;
      int h1 = lmod(h, newsize);  /* new position */
      lua_assert(cast_int(h%newsize) == lmod(h, newsize));
      p->next = newhash[h1];  /* chain it */
      newhash[h1] = p;
      p = next;
    }
  }
  luaM_freearray(L, tb->hash, tb->size, TString *);
  tb->size = newsize;
  tb->hash = newhash;
}

TString::~TString()
{
}

static TString *newlstr (lua_State *L, const char *str, size_t l, unsigned int h)
{
    TString *ts;
    stringtable *tb;

    if (l+1 > (MAX_SIZET - sizeof(TString))/sizeof(char))
    {
        luaM_toobig(L);
    }

    ts = lua_new <TString> ( L, sizeof(TString) + ( l + 1 ) );

    if ( ts )
    {
        ts->len = l;
        ts->hash = h;
        ts->marked = luaC_white(G(L));
        ts->tt = LUA_TSTRING;
        ts->reserved = 0;

        memcpy(ts+1, str, l*sizeof(char));
        ((char *)(ts+1))[l] = '\0';  /* ending 0 */

        tb = &G(L)->strt;
        h = lmod(h, tb->size);
        ts->next = tb->hash[h];  /* chain new entry */
        tb->hash[h] = ts;
        tb->nuse++;

        if (tb->nuse > cast(lu_int32, tb->size) && tb->size <= MAX_INT/2)
        {
            luaS_resize(L, tb->size*2);  /* too crowded */
        }
    }
    return ts;
}


TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  GCObject *o;
  unsigned int h = cast(unsigned int, l);  /* seed */
  size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
  size_t l1;
  for (l1=l; l1>=step; l1-=step)  /* compute hash */
    h = h ^ ((h<<5)+(h>>2)+cast(unsigned char, str[l1-1]));
  for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
       o != NULL;
       o = o->next) {
    TString *ts = rawgco2ts(o);
    if (ts->len == l && (memcmp(str, getstr(ts), l) == 0)) {
      /* string may be dead */
      if (isdead(G(L), o)) changewhite(o);
      return ts;
    }
  }
  return newlstr(L, str, l, h);  /* not found */
}

int luaS_concat (lua_State *L, StkId top, int total)
{
    // Only concat strings if we can get the global string environment.
    globalStateStringEnv *stringEnv = GetGlobalStringEnv( G(L) );

    if ( !stringEnv )
    {
        luaG_runerror(L, "no global string environment");
    }

    /* at least two string values; get as many as possible */
    size_t tl = tsvalue(top-1)->len;
    char *buffer;
    int i, n;

    /* collect total length */
    for (n = 1; n < total && tostring(L, top-n-1); n++)
    {
        size_t l = tsvalue(top-n-1)->len;

        if (l >= MAX_SIZET - tl)
        {
            luaG_runerror(L, "string length overflow");
        }

        tl += l;
    }

    buffer = luaZ_openspace(L, &stringEnv->buff, tl);
    tl = 0;

    for (i=n; i>0; i--)
    {  /* concat all strings */
        size_t l = tsvalue(top-i)->len;

        memcpy(buffer+tl, svalue(top-i), l);
        tl += l;
    }
    setsvalue2s(L, top-n, luaS_newlstr(L, buffer, tl));

    return n;
}

void luaS_globalgc (lua_State *L)
{
    globalStateStringEnv *stringEnv = GetGlobalStringEnv( G(L) );

    // Only proceed if we have the string environment.
    if ( stringEnv )
    {
        /* check size of buffer */
        if (luaZ_sizebuffer(&stringEnv->buff) > LUA_MINBUFFER*2)
        {  /* buffer too big? */
            size_t newsize = luaZ_sizebuffer(&stringEnv->buff) / 2;
            luaZ_resizebuffer(L, &stringEnv->buff, newsize);
        }
    }
}

void luaS_free (lua_State *L, TString *s)
{
    G(L)->strt.nuse--;

    lua_delete <TString> ( L, s );
}

Udata::~Udata()
{
}

Udata *luaS_newudata (lua_State *L, size_t s, GCObject *e)
{
    Udata *u;

    // Check that we do not encounter number overflow!
    if (s > MAX_SIZET - sizeof(Udata))
    {
        luaM_toobig(L);
    }

    u = lua_new <Udata> ( L, sizeof(Udata) + s );

    if ( u )
    {
        u->marked = luaC_white(G(L));  /* is not finalized */
        u->tt = LUA_TUSERDATA;
        u->len = s;
        u->metatable = NULL;
        u->env = e;
        /* chain it on udata list (after main thread) */
        u->next = G(L)->mainthread->next;
        G(L)->mainthread->next = u;
    }
    return u;
}

void luaS_freeudata (lua_State *L, Udata *u)
{
    lua_delete <Udata> ( L, u );
}

// Global state initialization.
void luaS_stateinit( lua_State *L )
{
    globalStateStringEnv *stringEnv = GetGlobalStringEnv( G(L) );

    if ( stringEnv )
    {
        luaZ_initbuffer( L, &stringEnv->buff );
    }
}

void luaS_stateshutdown( lua_State *L )
{
    globalStateStringEnv *stringEnv = GetGlobalStringEnv( G(L) );

    if ( stringEnv )
    {
        luaZ_freebuffer( L, &stringEnv->buff );
    }
}

// Module initialization.
void luaS_init( void )
{
    _stringEnvPluginOffset =
        globalStateFactory.RegisterStructPlugin <globalStateStringEnv> ( GLOBAL_STATE_PLUGIN_STRING_META );
}

void luaS_shutdown( void )
{
    return;
}