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

#include "lpluginutil.hxx"


struct stringConstructionParams
{
    int length;
};

// String type information.
struct stringTypeInfo_t
{
    struct stringTypeMetaInfo : public LuaTypeSystem::structTypeMetaInfo
    {
        size_t GetTypeSize( void *construction_params ) const
        {
            stringConstructionParams *params = (stringConstructionParams*)construction_params;

            return _sizestring( sizeof( TString ), params->length );
        }

        size_t GetTypeSizeByObject( void *langObj ) const
        {
            TString *strObj = (TString*)langObj;

            return _sizestring( sizeof( TString ), strObj->len );
        }
    };

    struct udataTypeMetaInfo : public LuaTypeSystem::structTypeMetaInfo
    {
        size_t GetTypeSize( void *construction_params ) const
        {
            stringConstructionParams *params = (stringConstructionParams*)construction_params;

            return _sizeudata( sizeof( Udata ), params->length );
        }

        size_t GetTypeSizeByObject( void *langObj ) const
        {
            Udata *u = (Udata*)langObj;

            return _sizeudata( sizeof( Udata ), u->len );
        }
    };

    stringTypeMetaInfo _stringMetaInfo;
    udataTypeMetaInfo _udataMetaInfo;

    inline void Initialize( lua_config *cfg )
    {
        LuaTypeSystem& typeSys = cfg->typeSys;

        // Register the string type.
        stringTypeInfo = typeSys.RegisterDynamicStructType <TString> ( "string", &_stringMetaInfo );
        udataTypeInfo = typeSys.RegisterDynamicStructType <Udata> ( "udata", &_stringMetaInfo );

        // Set up the inheritance.
        typeSys.SetTypeInfoInheritingClass(
            stringTypeInfo,
            cfg->gcobjTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            udataTypeInfo,
            cfg->gcobjTypeInfo
        );
    }

    inline void Shutdown( lua_config *cfg )
    {
        LuaTypeSystem& typeSys = cfg->typeSys;

        // Delete all types.
        typeSys.DeleteType( stringTypeInfo );
    }

    LuaTypeSystem::typeInfoBase *stringTypeInfo;
    LuaTypeSystem::typeInfoBase *udataTypeInfo;
};

PluginDependantStructRegister <stringTypeInfo_t, namespaceFactory_t> stringTypeInfo( namespaceFactory );

// Global state string environment.
struct globalStateStringEnv
{
    Mbuffer buff;  /* temporary buffer for string concatentation */
};

typedef PluginConnectingBridge
    <globalStateStringEnv,
        globalStateStructFactoryMeta <globalStateStringEnv, globalStateFactory_t, lua_config>,
    namespaceFactory_t>
        stringEnvConnectingBridge_t;

stringEnvConnectingBridge_t stringEnvConnectingBridge( namespaceFactory );

globalStateStringEnv* GetGlobalStringEnv( global_State *g )
{
    return stringEnvConnectingBridge.GetPluginStruct( g->config, g );
}

void luaS_resize (lua_State *L, int newsize)
{
    gcObjList_t *newhash;
    stringtable *tb;

    if (luaC_getstate( L ) == GCSsweepstring)
    {
        /* cannot resize during GC traverse, so lock until we have left it */
        luaC_finish( L );
    }

    newhash = luaM_newvector <gcObjList_t> (L, newsize);
    tb = &G(L)->strt;

    /* rehash */
    for (int i = 0; i < tb->size; i++)
    {
        gcObjList_t::iterator p = tb->hash[i].GetIterator();

        while ( !p.IsEnd() )
        {  /* for each node in the list */
            GCObject *curr = (GCObject*)p.Resolve();

            p.Increment();

            unsigned int h = gco2ts(curr)->hash;
            int h1 = lmod(h, newsize);  /* new position */

            lua_assert( cast_int( h % newsize ) == lmod( h, newsize ) );

            newhash[h1].Insert( curr );     /* chain it */
        }

        // Make sure we clear the list before deleting it.
        tb->hash[i].Clear();
    }

    luaM_freearray(L, tb->hash, tb->size);
    tb->size = newsize;
    tb->hash = newhash;
}

TString::~TString()
{
}

static TString* newlstr (lua_State *L, const char *str, size_t l, unsigned int h)
{
    // Attempt to get the string type information.
    stringTypeInfo_t *typeInfo = stringTypeInfo.GetPluginStruct( G(L)->config );

    // If we have no information about the string type, fail.
    if ( !typeInfo )
        return NULL;

    if (l+1 > (MAX_SIZET - sizeof(TString))/sizeof(char))
    {
        luaM_toobig(L);
    }

    stringConstructionParams params;
    params.length = l;

    TString *ts = lua_new <TString> ( L, typeInfo->stringTypeInfo, &params );

    if ( ts )
    {
        ts->len = l;
        ts->hash = h;
        
        luaC_register( L, ts, LUA_TSTRING );

        ts->reserved = 0;

        memcpy(ts+1, str, l*sizeof(char));
        ((char *)(ts+1))[l] = '\0';  /* ending 0 */

        stringtable *tb = &G(L)->strt;
        h = lmod(h, tb->size);
        tb->hash[h].Insert( ts );  /* chain new entry */
        tb->nuse++;

        if (tb->nuse > cast(lu_int32, tb->size) && tb->size <= MAX_INT/2)
        {
            luaS_resize(L, tb->size*2);  /* too crowded */
        }
    }
    return ts;
}


TString *luaS_newlstr (lua_State *L, const char *str, size_t l)
{
    unsigned int h = cast(unsigned int, l);  /* seed */
    size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
    size_t l1;

    for (l1=l; l1>=step; l1-=step)  /* compute hash */
    {
        h = h ^ ((h<<5)+(h>>2)+cast(unsigned char, str[l1-1]));
    }

    gcObjList_t& strHashList = G(L)->strt.hash[lmod(h, G(L)->strt.size)];

    for ( gcObjList_t::iterator iter = strHashList.GetIterator(); !iter.IsEnd(); iter.Increment() )
    {
        GCObject *o = (GCObject*)iter.Resolve();

        TString *ts = rawgco2ts(o);

        if (ts->len == l && (memcmp(str, getstr(ts), l) == 0))
        {
            /* string may be dead */
            if (isdead(G(L), o))
            {
                // TODO: this does not belong here.
                changewhite(o);
            }
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
    // Attempt to get the string type information.
    stringTypeInfo_t *typeInfo = stringTypeInfo.GetPluginStruct( G(L)->config );

    // If we have no information about the string type, fail.
    if ( !typeInfo )
        return NULL;

    // Check that we do not encounter number overflow!
    if (s > MAX_SIZET - sizeof(Udata))
    {
        luaM_toobig(L);
    }

    stringConstructionParams params;
    params.length = s;

    Udata *u = lua_new <Udata> ( L, typeInfo->udataTypeInfo, &params );

    if ( u )
    {
        luaC_linktmu( L, u, LUA_TUSERDATA );
        u->len = s;
        u->metatable = NULL;
        u->env = e;
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
void luaS_init( lua_config *cfg )
{
    return;
}

void luaS_shutdown( lua_config *cfg )
{
    return;
}