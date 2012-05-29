/*
** $Id: lobject.h,v 2.20.1.2 2008/08/06 13:29:48 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"
#include "lmem.h"


/* tags for values visible from Lua */
#define LAST_TAG	LUA_TTHREAD

#define NUM_TAGS	(LAST_TAG+1)


/*
** Extra tags for non-values
*/
#define LUA_TPROTO	(LAST_TAG+1)
#define LUA_TUPVAL	(LAST_TAG+2)
#define LUA_TDEADKEY	(LAST_TAG+3)


/*
** Union of all collectable objects
*/
class GCObject;

/*
** Union of all Lua values
*/
typedef union {
  GCObject *gc;
  void *p;
  lua_Number n;
  bool b;
} Value;


/*
** Tagged Values
*/

#define TValuefields	Value value; int tt

typedef struct lua_TValue
{
    TValuefields;
} TValue;

class TString;
class Table;
class Proto;
class Closure;
class Udata;
class Class;
class UpVal;
class LocVar;
class lua_State;


/* Macros to test type */
#define ttisnil(o)	(ttype(o) == LUA_TNIL)
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER)
#define ttisstring(o)	(ttype(o) == LUA_TSTRING)
#define ttistable(o)	(ttype(o) == LUA_TTABLE)
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION)
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN)
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA)
#define ttisclass(o)    (ttype(o) == LUA_TCLASS)
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD)
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)

/* Macros to access values */
#define ttype(o)	((o)->tt)
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)
#define rawtsvalue(o)	check_exp(ttisstring(o), (TString*)((o)->value.gc))
#define tsvalue(o)	(rawtsvalue(o))
#define rawuvalue(o)	check_exp(ttisuserdata(o), (Udata*)((o)->value.gc))
#define uvalue(o)	(rawuvalue(o))
#define clvalue(o)	check_exp(ttisfunction(o), (Closure*)((o)->value.gc))
#define hvalue(o)	check_exp(ttistable(o), (Table*)((o)->value.gc))
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
#define thvalue(o)	check_exp(ttisthread(o), (lua_State*)((o)->value.gc))
#define jvalue(o)   check_exp(ttisclass(o), (Class*)((o)->value.gc))

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))

/*
** for internal debug only
*/
#define checkconsistency(obj) \
  lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->tt))

#define checkliveness(g,obj) \
  lua_assert(!iscollectable(obj) || \
  ((ttype(obj) == (obj)->value.gc->tt) && !isdead(g, (obj)->value.gc)))


/* Macros to set values */
#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)

#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }

#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x) & 1; i_o->tt=LUA_TBOOLEAN; }

#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=(x); i_o->tt=LUA_TSTRING; \
    checkliveness(G(L),i_o); }

#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=(x); i_o->tt=LUA_TUSERDATA; \
    checkliveness(G(L),i_o); }

#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=(x); i_o->tt=LUA_TTHREAD; \
    checkliveness(G(L),i_o); }

#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=(x); i_o->tt=LUA_TFUNCTION; \
    checkliveness(G(L),i_o); }

#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=(x); i_o->tt=LUA_TTABLE; \
    checkliveness(G(L),i_o); }

#define setptvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=(x); i_o->tt=LUA_TPROTO; \
    checkliveness(G(L),i_o); }

#define setjvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=(x); i_o->tt=LUA_TCLASS; \
    checkliveness(G(L),i_o); }



#define setobj(L,obj1,obj2) \
  { const TValue *o2=(obj2); TValue *o1=(obj1); \
    o1->value = o2->value; o1->tt=o2->tt; \
    checkliveness(G(L),o1); }


/*
** different types of sets, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to table */
#define setobj2t	setobj
/* to new object */
#define setobj2n	setobj
#define setsvalue2n	setsvalue

#define setttype(obj, tt) (ttype(obj) = (tt))


#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)



typedef TValue *StkId;  /* index to stack elements */

struct global_State;


class GCObject
{
public:
    virtual             ~GCObject()         { }

    virtual TString*    GetTString()        { return NULL; }
    virtual Udata*      GetUserData()       { return NULL; }
    virtual Closure*    GetClosure()        { return NULL; }
    virtual Table*      GetTable()          { return NULL; }
    virtual Class*      GetClass()          { return NULL; }
    virtual Proto*      GetProto()          { return NULL; }
    virtual UpVal*      GetUpValue()        { return NULL; }
    virtual LocVar*     GetLocVar()         { return NULL; }
    virtual lua_State*  GetThread()         { return NULL; }

    virtual void        MarkGC( global_State *g )       { }
    virtual int         TraverseGC( global_State *g )   { return 0; }

    void* operator new( size_t size, lua_State *main ) throw()
    {
        GCObject *obj = (GCObject*)luaM_realloc_( main, NULL, 0, size );
        obj->_lua = main;
        return obj;
    }

    void operator delete( void *ptr ) throw()
    {
        luaM_freemem( ((GCObject*)ptr)->_lua, ptr, 0 );
    }

    void operator delete( void *ptr, size_t size ) throw()
    {
        luaM_freemem( ((GCObject*)ptr)->_lua, ptr, size );
    }

    GCObject *next;
    lu_byte tt;
    lu_byte marked;
    lua_State *_lua;
};

class GrayObject : public GCObject
{
public:
    ~GrayObject() {}

    void                MarkGC( global_State *g );
    virtual size_t      Propagate( global_State *g ) = 0;

    GrayObject *gclist;
};

/*
** String headers for string table
*/
#define sizestring(s)	(sizeof(TString)+((s)->len+1)*sizeof(char))
#define sizeudata(u)	(sizeof(Udata)+(u)->len)

LUA_MAXALIGN class TString : public GCObject
{
public:
    ~TString();

    void* operator new( size_t size, lua_State *main, size_t len ) throw()
    {
        return GCObject::operator new( (len+1)*sizeof(char) + size, main );
    }

    void operator delete( void *ptr ) throw()
    {
        GCObject::operator delete( ptr, sizestring( ((TString*)ptr) ) );
    }

    lu_byte reserved;
    unsigned int hash;
    size_t len;
};

#define getstr(ts)	cast(const char *, (ts) + 1)
#define svalue(o)       getstr(rawtsvalue(o))


LUA_MAXALIGN class Udata : public GCObject
{
public:
    ~Udata();

    void MarkGC( global_State *g );

    void* operator new( size_t size, lua_State *main, size_t sData ) throw()
    {
        return GCObject::operator new( sData + size, main );
    }

    void operator delete( void *ptr ) throw()
    {
        GCObject::operator delete( ptr, sizeudata( (Udata*)ptr ) );
    }

    Table *metatable;
    Table *env;
    size_t len;
};

/*
** Function Prototypes
*/
class Proto : public GrayObject
{
public:
    ~Proto();

    int TraverseGC( global_State *g );
    size_t Propagate( global_State *g );

    void operator delete( void *ptr ) throw()
    {
        GCObject::operator delete( ptr, sizeof(Proto) );
    }

    TValue *k;  /* constants used by the function */
    Instruction *code;
    Proto **p;  /* functions defined inside the function */
    int *lineinfo;  /* map from opcodes to source lines */
    LocVar *locvars;  /* information about local variables */
    TString **upvalues;  /* upvalue names */
    TString  *source;
    int sizeupvalues;
    int sizek;  /* size of `k' */
    int sizecode;
    int sizelineinfo;
    int sizep;  /* size of `p' */
    int sizelocvars;
    int linedefined;
    int lastlinedefined;
    lu_byte nups;  /* number of upvalues */
    lu_byte numparams;
    lu_byte is_vararg;
    lu_byte maxstacksize;
};


/* masks for new-style vararg */
#define VARARG_HASARG		1
#define VARARG_ISVARARG		2
#define VARARG_NEEDSARG		4


class LocVar
{
public:
    TString *varname;
    int startpc;  /* first point where variable is active */
    int endpc;    /* first point where variable is dead */
};


/*
** Upvalues
*/

class UpVal : public GCObject
{
public:
    ~UpVal();

    void MarkGC( global_State *g );

    void operator delete( void *ptr ) throw()
    {
        GCObject::operator delete( ptr, sizeof(UpVal) );
    }

    TValue *v;  /* points to stack or to its own value */
    union
    {
        TValue value;  /* the value (when closed) */

        struct
        {  /* double linked list (when open) */
          UpVal *prev;
          UpVal *next;
        } l;
    } u;
};


/*
** Closures
*/

#define sizeCclosure(n)	(cast(int, sizeof(CClosure)) + \
                         cast(int, sizeof(TValue)*((n)-1)))

#define sizeLclosure(n)	(cast(int, sizeof(LClosure)) + \
                         cast(int, sizeof(TValue *)*((n)-1)))

class Closure : public GrayObject
{
public:
                            ~Closure();

    int                     TraverseGC( global_State *g );

    Closure*                GetClosure()    { return this; }
    virtual class CClosure* GetCClosure()   { return NULL; }
    virtual class LClosure* GetLClosure()   { return NULL; }

    lu_byte isC;
    lu_byte nupvalues;
    Table *env;
};

class CClosure : public Closure
{
public:
    ~CClosure();

    int TraverseGC( global_State *g );
    size_t Propagate( global_State *g );

    CClosure* GetCClosure()     { return this; }

    void* operator new( size_t size, lua_State *main, unsigned int nup )
    {
        return GCObject::operator new( sizeCclosure( nup ), main );
    }

    void operator delete( void *ptr ) throw()
    {
        GCObject::operator delete( ptr, sizeCclosure( ((Closure*)ptr)->nupvalues ) );
    }

    lua_CFunction f;
    TValue upvalue[1];
};

class LClosure : public Closure
{
public:
    ~LClosure();

    int TraverseGC( global_State *g );
    size_t Propagate( global_State *g );

    LClosure* GetLClosure()     { return this; }

    void* operator new( size_t size, lua_State *main, unsigned int nup )
    {
        return GCObject::operator new( sizeLclosure( nup ), main );
    }

    void operator delete( void *ptr ) throw()
    {
        GCObject::operator delete( ptr, sizeLclosure( ((Closure*)ptr)->nupvalues ) );
    }

    Proto *p;
    UpVal *upvals[1];
};

#define iscfunction(o)	(ttype(o) == LUA_TFUNCTION && clvalue(o)->isC)
#define isLfunction(o)	(ttype(o) == LUA_TFUNCTION && !clvalue(o)->isC)


/*
** Tables
*/

typedef union TKey {
  struct {
    TValuefields;
    struct Node *next;  /* for chaining */
  } nk;
  TValue tvk;
} TKey;


typedef struct Node {
  TValue i_val;
  TKey i_key;
} Node;

class Table : public GrayObject
{
public:
    ~Table();

    int TraverseGC( global_State *g );
    size_t Propagate( global_State *g );

    Table* GetTable()   { return this; }

    void operator delete( void *ptr ) throw()
    {
        GCObject::operator delete( ptr, sizeof(Table) );
    }

    lu_byte flags;  /* 1<<p means tagmethod(p) is not present */ 
    lu_byte lsizenode;  /* log2 of size of `node' array */
    Table *metatable;
    TValue *array;  /* array part */
    Node *node;
    Node *lastfree;  /* any free position is before this position */
    int sizearray;  /* size of `array' array */
};

class Class : public GCObject, public virtual ILuaClass
{
public:
    ~Class();

    void    MarkGC( global_State *g );
    int     TraverseGC( global_State *g );

    Class*  GetClass()   { return this; }

    void    IncrementMethodStack( lua_State *lua );
    void    DecrementMethodStack( lua_State *lua );

    void    SetTransmit( int type );
    int     GetTransmit();
    bool    IsTransmit( int type );

    void    PushEnvironment( lua_State *L );
    void    PushOuterEnvironment( lua_State *L );

    void    RequestDestruction();

    TValue* GetSuperMethod( lua_State *lua );

    void operator delete( void *ptr ) throw()
    {
        GCObject::operator delete( ptr, sizeof(Class) );
    }

    Table *env;
    Table *outenv;
    Table *methods;
    Table *storage;
    Table *forceSuper;
    Table *internStorage;
    unsigned int inMethod;
    unsigned int refCount;
    bool reqDestruction;
    bool destroyed;

    int transType;

    // Cached values
    TValue destructor;
    TString *superCached;
};


/*
** `module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


#define luaO_nilobject		(&luaO_nilobject_)

LUAI_DATA const TValue luaO_nilobject_;

#define ceillog2(x)	(luaO_log2((x)-1) + 1)

LUAI_FUNC int luaO_log2 (unsigned int x);
LUAI_FUNC int luaO_int2fb (unsigned int x);
LUAI_FUNC int luaO_fb2int (int x);
LUAI_FUNC int luaO_rawequalObj (const TValue *t1, const TValue *t2);
LUAI_FUNC int luaO_str2d (const char *s, lua_Number *result);
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t len);


#endif

