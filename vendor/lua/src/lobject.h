/*
** $Id: lobject.h,v 2.20.1.2 2008/08/06 13:29:48 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>
#include <vector>
#include <map>
#include <rwlist.hpp>

#include "llimits.h"
#include "lua.h"
#include "lmem.h"
#include "lutils.h"


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

struct TValue
{
    TValuefields;
};

class TString;
class Table;
class Proto;
class Closure;
class Udata;
class Class;
class UpVal;
class LocVar;
class Dispatch;
class lua_State;


typedef TValue* StkId;  /* index to stack elements */
typedef const TValue* StkId_const;
struct global_State;

// List of garbage collectible items.
typedef SingleLinkedList <class GCObject> gcObjList_t;

// Type that holds garbage collectible Lua types.
class GCObject abstract : public gcObjList_t::node
{
public:
    // Every type must give a runtime sizeof operator.
    virtual lu_mem      GetTypeSize( global_State *g ) const = 0;

    virtual TString*    GetTString()        { return NULL; }
    virtual Udata*      GetUserData()       { return NULL; }
    virtual Closure*    GetClosure()        { return NULL; }
    virtual Table*      GetTable()          { return NULL; }
    virtual Class*      GetClass()          { return NULL; }
    virtual Proto*      GetProto()          { return NULL; }
    virtual UpVal*      GetUpValue()        { return NULL; }
    virtual LocVar*     GetLocVar()         { return NULL; }
    virtual Dispatch*   GetDispatch()       { return NULL; }
    virtual lua_State*  GetThread()         { return NULL; }

    virtual void        MarkGC( global_State *g )       { }

    // Global indexing and new-indexing routines (since every object can potencially be accessed)
    virtual void        Index( lua_State *L, const TValue *key, StkId val );
    virtual void        NewIndex( lua_State *L, const TValue *key, StkId val );

    lu_byte tt;
    lu_byte marked;
};

class GrayObject abstract : public GCObject
{
public:
    ~GrayObject() {}

    void                MarkGC( global_State *g );
    virtual size_t      Propagate( global_State *g ) = 0;

    virtual bool        GCRequiresBackBarrier( void ) const = 0;

    GrayObject *gclist;
};

/*
** String headers for string table
*/
#ifdef LUA_USE_C_MACROS

#define _sizestring( baseSize, n )          (baseSize + (n+1)*sizeof(char))
#define _sizeudata( baseSize, n )           (baseSize + (n)*sizeof(char))

#else

FASTAPI size_t _sizestring( size_t baseSize, size_t n )     { return (baseSize + (n+1)*sizeof(char)); }
FASTAPI size_t _sizeudata( size_t baseSize, size_t n )      { return (baseSize + (n)*sizeof(char)); }

#endif //LUA_USE_C_MACROS

LUA_MAXALIGN class TString : public GCObject
{
public:
    ~TString();

    lu_mem GetTypeSize( global_State *g ) const             { return _sizestring( sizeof(*this), this->len ); }

    lu_byte reserved;
    unsigned int hash;
    size_t len;
};

LUA_MAXALIGN class Udata : public GCObject
{
public:
    ~Udata();

    lu_mem GetTypeSize( global_State *g ) const             { return _sizeudata( sizeof(*this), this->len ); }

    void MarkGC( global_State *g );

    Table *metatable;
    GCObject *env;
    size_t len;
};

/*
** Function Prototypes
*/
class Proto : public GrayObject
{
public:
    ~Proto();

    lu_mem GetTypeSize( global_State *g ) const             { return sizeof(*this); }

    int TraverseGC( global_State *g );
    size_t Propagate( global_State *g );

    bool GCRequiresBackBarrier( void ) const                { return true; }

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

    lu_mem GetTypeSize( global_State *g ) const             { return sizeof(*this); }

    void MarkGC( global_State *g );

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

class Closure abstract : public GrayObject
{
public:
    virtual                 ~Closure        ( void );

    int                     TraverseGC      ( global_State *g );

    bool                    GCRequiresBackBarrier( void ) const { return true; }

    virtual TValue*         ReadUpValue     ( unsigned char index ) = 0;

    inline void             SetEnvLocked    ( bool locked )     { isEnvLocked = locked; }
    inline bool             IsEnvLocked     ( void ) const      { return isEnvLocked; }

    Closure*                GetClosure      ( void )            { return this; }
    virtual class CClosure* GetCClosure     ( void )            { return NULL; }
    virtual class LClosure* GetLClosure     ( void )            { return NULL; }

    union
    {
        unsigned char genFlags;
        struct
        {
            bool isC : 1;
            bool isEnvLocked : 1;
        };
    };
    lu_byte nupvalues;
    GCObject *env;
};

class CClosure abstract : public Closure
{
public:
    ~CClosure();

    int TraverseGC( global_State *g );
    size_t Propagate( global_State *g );

    CClosure* GetCClosure()     { return this; }

    lua_CFunction f;
    GCObject *accessor; // Usually the storage of the thread
};

class CClosureMethodRedirect : public CClosure
{
public:
    ~CClosureMethodRedirect();

    lu_mem GetTypeSize( global_State *g ) const         { return sizeof(*this); }

    size_t Propagate( global_State *g );

    TValue* ReadUpValue( unsigned char index );

    Closure* redirect;
    Class* m_class;
};

class CClosureMethodRedirectSuper : public CClosureMethodRedirect
{
public:
    ~CClosureMethodRedirectSuper();

    lu_mem GetTypeSize( global_State *g ) const         { return sizeof(*this); }

    size_t Propagate( global_State *g );

    TValue* ReadUpValue( unsigned char index );

    Closure *super;
};

class CClosureBasic : public CClosure
{
public:
    ~CClosureBasic();

    lu_mem GetTypeSize( global_State *g ) const;

    size_t Propagate( global_State *g );

    TValue* ReadUpValue( unsigned char index );

    TValue upvalues[1];
};

class CClosureMethodBase abstract : public CClosure
{
public:
    ~CClosureMethodBase();

    size_t Propagate( global_State *g );

    Class*          m_class;
    lua_CFunction   method;
    Closure*        super;
};

class CClosureMethod : public CClosureMethodBase
{
public:
    ~CClosureMethod();

    lu_mem GetTypeSize( global_State *g ) const;

    size_t Propagate( global_State *g );

    TValue* ReadUpValue( unsigned char index );

    TValue  upvalues[1];
};

class CClosureMethodTrans : public CClosureMethodBase
{
public:
    ~CClosureMethodTrans();

    lu_mem GetTypeSize( global_State *g ) const;

    size_t Propagate( global_State *g );

    TValue* ReadUpValue( unsigned char index );

    unsigned char trans;
    void *data;
    TValue  upvalues[1];
};

class LClosure : public Closure
{
public:
    ~LClosure();

    lu_mem GetTypeSize( global_State *g ) const;

    int TraverseGC( global_State *g );
    size_t Propagate( global_State *g );

    TValue* ReadUpValue( unsigned char idx );

    LClosure* GetLClosure()     { return this; }

    Proto *p;
    UpVal *upvals[1];
};


/*
** Tables
*/

typedef union TKey {
    struct : TValue
    {
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

    lu_mem  GetTypeSize( global_State *g ) const;

    bool GCRequiresBackBarrier( void ) const    { return true; }

    int TraverseGC( global_State *g );
    size_t Propagate( global_State *g );

    Table* GetTable()   { return this; }

    void    Index( lua_State *L, const TValue *key, StkId val );
    void    NewIndex( lua_State *L, const TValue *key, StkId val );

    lu_byte flags;  /* 1<<p means tagmethod(p) is not present */ 
    lu_byte lsizenode;  /* log2 of size of `node' array */
    Table *metatable;
    TValue *array;  /* array part */
    Node *node;
    Node *lastfree;  /* any free position is before this position */
    int sizearray;  /* size of `array' array */
};

#include "lstrtable.h"

// Native C++ index wrapper
class Dispatch abstract : public GrayObject
{
public:
    virtual ~Dispatch()                     {}

    size_t  Propagate( global_State *g );

    Dispatch*   GetDispatch()               { return this; }
};

class ClassDispatch abstract : public Dispatch
{
public:
    size_t                  Propagate( global_State *g );

    bool                    GCRequiresBackBarrier( void ) const     { return true; }

    Class*                  GetClass()      { return m_class; }

    Class*                  m_class;
};

class ClassEnvDispatch : public ClassDispatch
{
public:
    lu_mem                  GetTypeSize( global_State *g ) const    { return sizeof( ClassEnvDispatch ); }

    void                    Index( lua_State *L, const TValue *key, StkId val );
    void                    NewIndex( lua_State *L, const TValue *key, StkId val );
};

class ClassOutEnvDispatch : public ClassDispatch
{
public:
    lu_mem                  GetTypeSize( global_State *g ) const    { return sizeof( ClassOutEnvDispatch ); }

    void                    Index( lua_State *L, const TValue *key, StkId val );
    void                    NewIndex( lua_State *L, const TValue *key, StkId val );
};

class ClassMethodDispatch : public ClassDispatch
{
public:
    lu_mem                  GetTypeSize( global_State *g ) const    { return sizeof( ClassMethodDispatch ); }

    size_t                  Propagate( global_State *g );

    void                    Index( lua_State *L, const TValue *key, StkId val );
    void                    NewIndex( lua_State *L, const TValue *key, StkId val );

    GCObject*               m_prevEnv;
};

#include "ldispatch.h"

struct _methodRegisterInfo
{
    _methodRegisterInfo()
    {
        numUpValues = 0;
        isTrans = false;
    }

    unsigned char numUpValues;
    bool isTrans;
    unsigned char transID;
};

struct _methodCacheEntry : _methodRegisterInfo
{
    lua_CFunction   method;
};

class Class : public GCObject, public virtual ILuaClass
{
public:
    ~Class();

    lu_mem  GetTypeSize( global_State *g ) const;

    void    Propagate( lua_State *L );

    void    MarkGC( global_State *g );
    int     TraverseGC( global_State *g );

    void    Index( lua_State *L, const TValue *key, StkId val );
    void    NewIndex( lua_State *L, const TValue *key, StkId val );

    Class*  GetClass()   { return this; }

    unsigned int    GetRefCount() const     { return inMethod; }

    void    IncrementMethodStack( lua_State *lua );
    void    DecrementMethodStack( lua_State *lua );
    void    ClearReferences( lua_State *lua );
    void    CheckDestruction( lua_State *lua );
    bool    PreDestructor( lua_State *L );

    void    Push( lua_State *L );
    void    PushMethod( lua_State *L, const char *key );
    void    PushSuperMethod( lua_State *L );

    void    CallMethod( lua_State *L, const char *key );

    void    SetTransmit( int type, void *entity );
    bool    GetTransmit( int type, void*& entity );
    int     GetTransmit() const;
    bool    IsTransmit( int type ) const;

    Dispatch*   AcquireEnvDispatcher( lua_State *L );
    Dispatch*   AcquireEnvDispatcherEx( lua_State *L, GCObject *env );

    Closure*    GetMethodFrom( lua_State *L, const TString *name, Table *methodTable );
    Closure*    GetMethod( lua_State *L, const TString *name, Table*& table );
    void    SetMethod( lua_State *L, TString *name, Closure *method, Table *table );

    FASTAPI void    RegisterMethod( lua_State *L, TString *name, bool handlers = false );
    FASTAPI void    RegisterMethod( lua_State *L, TString *methName, lua_CFunction proto, _methodRegisterInfo& info, bool handlers = false );
    FASTAPI void    RegisterMethodAt( lua_State *L, TString *methName, lua_CFunction proto, Table *methodTable, _methodRegisterInfo& info, bool handlers = false );

    void    RegisterMethod( lua_State *L, const char *name, bool handlers = false );
    void    RegisterMethod( lua_State *L, const char *name, lua_CFunction proto, int nupval, bool handlers = false );
    void    RegisterInterface( lua_State *L, const luaL_Reg *intf, int nupval, bool handlers = true );
    void    RegisterInterfaceAt( lua_State *L, const luaL_Reg *intf, int nupval, Table *methodTable, bool handlers = true );
    void    RegisterMethodTrans( lua_State *L, const char *name, lua_CFunction proto, int nupval, int trans, bool handlers = false );
    void    RegisterInterfaceTrans( lua_State *L, const luaL_Reg *intf, int nupval, int trans, bool handlers = true );
    void    RegisterInterfaceTransAt( lua_State *L, const luaL_Reg *intf, int nupval, int trans, Table *methodTable, bool handlers = true );
    void    RegisterLightMethod( lua_State *L, const char *name );
    void    RegisterLightMethodTrans( lua_State *L, const char *name, int trans );
    void    RegisterLightInterface( lua_State *L, const luaL_Reg *intf, void *udata );
    void    RegisterLightInterfaceTrans( lua_State *L, const luaL_Reg *intf, void *udata, int trans );

    void    EnvPutFront( lua_State *L );
    void    EnvPutBack( lua_State *L );

    bool    IsDestroying() const;
    bool    IsDestroyed() const;

    bool    IsRootedIn( Class *root ) const;
    bool    IsRootedIn( lua_State *L, int idx ) const;

    void    PushEnvironment( lua_State *L );
    void    PushOuterEnvironment( lua_State *L );
    void    PushInternStorage( lua_State *L );
    void    PushChildAPI( lua_State *L );
    void    PushParent( lua_State *L );
    const TValue*   GetEnvValue( lua_State *L, const TValue *key );
    const TValue*   GetEnvValueString( lua_State *L, const char *key );

    void    RequestDestruction();

    TValue* SetSuperMethod( lua_State *L );
    const TValue*   GetSuperMethod( lua_State *L );

    lua_State *hostState;

    Dispatch *env;
    Dispatch *outenv;
    Table *storage;
    Table *internStorage;
    Class *parent;
    Class *childAPI;
    unsigned int inMethod;
    unsigned int refCount;
	bool reqDestruction : 1;
	bool destroyed : 1;
	bool destroying : 1;
	unsigned char transCount : 5;

    typedef Closure* (*forceSuperCallback)( lua_State *L, Closure *newMethod, Class *j, Closure *prevMethod );
    typedef Closure* (*forceSuperCallbackNative)( lua_State *L, lua_CFunction proto, Class *j, Closure *prevMethod, _methodRegisterInfo& info );
    
    struct forceSuperItem
    {
        forceSuperCallback  cb;
        forceSuperCallbackNative    cbNative;
    };

    typedef StringTable <forceSuperItem> ForceSuperTable;

    ForceSuperTable *forceSuper;
    ClassStringTable *methodCache;

#pragma pack(1)
	struct trans_t
	{
		unsigned char id;
		void *ptr;
	};
#pragma pack()

	trans_t *trans;

    RwList <Class> children;
    size_t childCount;
    RwListEntry <Class> child_iter;

    typedef std::vector <GCObject*> envList_t;
    envList_t envInherit;

    // Cached values
    Closure *destructor;
};

/*
** `per thread' state
*/

/* extra stack space to handle TM calls and some other extras */
#define EXTRA_STACK   5


#define BASIC_CI_SIZE           8

#define BASIC_STACK_SIZE        (2*LUA_MINSTACK)



/*
** informations about a call
*/
typedef struct CallInfo {
  StkId base;  /* base for this function */
  StkId func;  /* function index in the stack */
  StkId	top;  /* top for this function */
  const Instruction *savedpc;
  int nresults;  /* expected number of results from this function */
  int tailcalls;  /* number of tail calls lost under this entry */
} CallInfo;

class lua_State : public GrayObject, virtual public ILuaState
{
public:
    lua_State( void )
    {
        // Set the runtime state.
        defaultAlloc.SetThread( this );
    }
    virtual ~lua_State();

    lu_mem GetTypeSize( global_State *g ) const;

    // lua_State is always the main thread
    virtual void    SetMainThread( bool enabled )       {}
    virtual bool    IsThread()                          { return false; }

    virtual void    SetYieldDisabled( bool disable )    {}
    virtual bool    IsYieldDisabled()                   { return true; }

    bool GCRequiresBackBarrier( void ) const    { return true; }

    size_t Propagate( global_State *g );

    StkId top;  /* first free slot in the stack */
    StkId base;  /* base of current function */
    global_State *l_G;
    CallInfo *ci;  /* call info for current function */
    const Instruction *savedpc;  /* `savedpc' of current function */
    StkId stack_last;  /* last free slot in the stack */
    StkId stack;  /* stack base */
    CallInfo *end_ci;  /* points after end of ci array*/
    CallInfo *base_ci;  /* array of CallInfo's */
    int stacksize;
    int size_ci;  /* size of array `base_ci' */
    unsigned short nCcalls;  /* number of nested C calls */
    lu_byte hookmask;
    bool allowhook;
    int basehookcount;
    int hookcount;
    lua_Hook hook;
    TValue l_gt;  /* table of globals */
    TValue env;  /* temporary place for environments */
    gcObjList_t openupval;  /* list of open upvalues in this stack */
    ptrdiff_t errfunc;  /* current error handling function (stack index) */
    TValue storage;
    Table *mt[NUM_TAGS];  /* metatables for basic types */

    // Memory allocator for class memory.
    // It allocates memory with thread focus.
    struct LuaThreadRuntimeAllocator
    {
        lua_State *runtimeThread;

        inline LuaThreadRuntimeAllocator( void )
        {
            this->runtimeThread = NULL;
        }

        inline ~LuaThreadRuntimeAllocator( void )
        {
            return;
        }

        inline void SetThread( lua_State *theThread )
        {
            this->runtimeThread = theThread;
        }

        inline void* Allocate( size_t memSize )
        {
            return luaM_realloc_( this->runtimeThread, NULL, 0, memSize );
        }

        inline void Free( void *ptr, size_t memSize )
        {
            luaM_realloc_( this->runtimeThread, ptr, memSize, 0 );
        }
    };
    LuaThreadRuntimeAllocator defaultAlloc;
};

// General stuff.
struct stringtable
{
    gcObjList_t *hash;
    lu_int32 nuse;  /* number of elements */
    int size;
};

class lua_Thread;
class GrayObject;
class TString;
class Closure;

/*
** 'global state', shared by all threads of this state
*/
struct global_State
{
    stringtable strt;  /* hash table for strings */
    lua_Alloc frealloc;  /* function to (re-)allocate memory */
    void *ud;         /* auxiliary data to 'frealloc' */
    lu_byte currentwhite;

    lu_mem totalbytes;  /* number of bytes currently allocated */

    TValue l_registry;
    lua_State *mainthread;
    TString *tmname[TM_N];  /* array with tag-method names */
    Closure *events[LUA_NUM_EVENTS];

    RwList <lua_Thread> threads; /* all existing threads in this machine */

    void *allocData; /* private allocation data used for memory requests */
};

// Macros to create and destroy Lua types accordingly.
template <typename classType>
FASTAPI classType* lua_new( lua_State *L, lu_mem memSize = sizeof( classType ) )
{
    // Allocate the memory.
    void *objMem = L->defaultAlloc.Allocate( memSize );

    if ( objMem )
    {
        // Try to construct the object and return it.
        return new (objMem) classType;
    }

    return NULL;
}

template <typename classType>
FASTAPI void lua_delete( lua_State *L, classType *obj )
{
    // Obtain the type size.
    lu_mem memSize = obj->GetTypeSize( G(L) );

    // Deconstruct the type.
    obj->~classType();

    // Release the memory.
    void *objMem = obj;

    L->defaultAlloc.Free( objMem, memSize );
}

// Helper macros for using lua_State and global_State.
#ifdef LUA_USE_C_MACROS

#define G(L)	(L->l_G)

#else

FASTAPI global_State* G( lua_State *L )         { return L->l_G; }

#endif

/* Macros to test type */
#ifdef LUA_USE_C_MACROS

#define ttype(o)	((o)->tt)
#define setttype(obj, tt) (ttype(obj) = (tt))
#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)

#define ttisnil(o)	(ttype(o) == LUA_TNIL)
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER)
#define ttisstring(o)	(ttype(o) == LUA_TSTRING)
#define ttistable(o)	(ttype(o) == LUA_TTABLE)
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION)
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN)
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA)
#define ttisclass(o)    (ttype(o) == LUA_TCLASS)
#define ttisdispatch(o) (ttype(o) == LUA_TDISPATCH)
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD)
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)

#else

FASTAPI int& ttype( TValue *o )                 { return o->tt; }
FASTAPI int ttype( const TValue *o )            { return o->tt; }
FASTAPI void setttype(TValue *obj, int tt)      { ttype(obj) = tt; }
FASTAPI bool iscollectable(const TValue *o)     { return ttype(o) >= LUA_TSTRING; }

FASTAPI bool ttisnil( const TValue *o )             { return ttype( o ) == LUA_TNIL; }
FASTAPI bool ttisnumber( const TValue *o )          { return ttype( o ) == LUA_TNUMBER; }
FASTAPI bool ttisstring( const TValue *o )          { return ttype( o ) == LUA_TSTRING; }
FASTAPI bool ttistable( const TValue *o )           { return ttype( o ) == LUA_TTABLE; }
FASTAPI bool ttisfunction( const TValue *o )        { return ttype( o ) == LUA_TFUNCTION; }
FASTAPI bool ttisboolean( const TValue *o )         { return ttype( o ) == LUA_TBOOLEAN; }
FASTAPI bool ttisuserdata( const TValue *o )        { return ttype( o ) == LUA_TUSERDATA; }
FASTAPI bool ttisclass( const TValue *o )           { return ttype( o ) == LUA_TCLASS; }
FASTAPI bool ttisdispatch( const TValue *o )        { return ttype( o ) == LUA_TDISPATCH; }
FASTAPI bool ttisthread( const TValue *o )          { return ttype( o ) == LUA_TTHREAD; }
FASTAPI bool ttislightuserdata( const TValue *o )   { return ttype( o ) == LUA_TLIGHTUSERDATA; }

#endif //LUA_USE_C_MACROS

// A special cast operator that supports safe casting and C-style casting depending on
// compilation settings.
template <typename resultType>
FASTAPI resultType* gcobj_cast( GCObject *obj )
{
    return
#if defined(_DEBUG) && defined(LUA_OBJECT_VERIFICATION)
        dynamic_cast <resultType*>
#else
        (resultType*)
#endif
            ( obj );
}

template <typename resultType>
FASTAPI const resultType* gcobj_cast( const GCObject *obj )
{
    return
#if defined(_DEBUG) && defined(LUA_OBJECT_VERIFICATION)
        dynamic_cast <const resultType*>
#else
        (const resultType*)
#endif
            ( obj );
}

#ifdef LUA_USE_C_MACROS
#define gcobj(o)    (gcobj_cast <GCObject> (o))
#define sobj(o)     (gcobj_cast <TString> (o))
#define uobj(o)     (gcobj_cast <Udata> (o))
#define clobj(o)    (gcobj_cast <Closure> (o))
#define hobj(o)     (gcobj_cast <Table> (o))
#define thobj(o)    (gcobj_cast <lua_State> (o))
#define qobj(o)     (gcobj_cast <Dispatch> (o))
#define jobj(o)     (gcobj_cast <Class> (o))
#define ptobj(o)    (gcobj_cast <Proto> (o))
#else
FASTAPI GCObject*   gcobj(GCObject *o)      { return gcobj_cast <GCObject> ( o ); }
FASTAPI TString*    sobj(GCObject *o)       { return gcobj_cast <TString> ( o ); }
FASTAPI Udata*      uobj(GCObject *o)       { return gcobj_cast <Udata> ( o ); }
FASTAPI Closure*    clobj(GCObject *o)      { return gcobj_cast <Closure> ( o ); }
FASTAPI Table*      hobj(GCObject *o)       { return gcobj_cast <Table> ( o ); }
FASTAPI lua_State*  thobj(GCObject *o)      { return gcobj_cast <lua_State> ( o ); }
FASTAPI Dispatch*   qobj(GCObject *o)       { return gcobj_cast <Dispatch> ( o ); }
FASTAPI Class*      jobj(GCObject *o)       { return gcobj_cast <Class> ( o ); }
FASTAPI Proto*      ptobj(GCObject *o)      { return gcobj_cast <Proto> ( o ); }

FASTAPI const GCObject*     gcobj(const GCObject *o)    { return gcobj_cast <const GCObject> ( o ); }
FASTAPI const TString*      sobj(const GCObject *o)     { return gcobj_cast <const TString> ( o ); }
FASTAPI const Udata*        uobj(const GCObject *o)     { return gcobj_cast <const Udata> ( o ); }
FASTAPI const Closure*      clobj(const GCObject *o)    { return gcobj_cast <const Closure> ( o ); }
FASTAPI const Table*        hobj(const GCObject *o)     { return gcobj_cast <const Table> ( o ); }
FASTAPI const lua_State*    thobj(const GCObject *o)    { return gcobj_cast <const lua_State> ( o ); }
FASTAPI const Dispatch*     qobj(const GCObject *o)     { return gcobj_cast <const Dispatch> ( o ); }
FASTAPI const Class*        jobj(const GCObject *o)     { return gcobj_cast <const Class> ( o ); }
FASTAPI const Proto*        ptobj(const GCObject *o)    { return gcobj_cast <const Proto> ( o ); }
#endif //LUA_USE_C_MACROS

/* Macros to access values */
#ifdef LUA_USE_C_MACROS
#define gcvalue(o)	check_exp(iscollectable(o), gcobj((o)->value.gc))
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)
#define rawtsvalue(o)	check_exp(ttisstring(o), sobj((o)->value.gc))
#define tsvalue(o)	(rawtsvalue(o))
#define rawuvalue(o)	check_exp(ttisuserdata(o), uobj((o)->value.gc))
#define uvalue(o)	(rawuvalue(o))
#define clvalue(o)	check_exp(ttisfunction(o), clobj((o)->value.gc))
#define hvalue(o)	check_exp(ttistable(o), hobj((o)->value.gc))
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
#define thvalue(o)	check_exp(ttisthread(o), thobj((o)->value.gc))
#define qvalue(o)   check_exp(ttisdispatch(o), qobj((o)->value.gc))
#define jvalue(o)   check_exp(ttisclass(o), jobj((o)->value.gc))

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))
#else
FASTAPI lua_Number  nvalue(const TValue *o)         { return check_exp(ttisnumber(o), (o)->value.n); }
FASTAPI TString*    rawtsvalue(const TValue *o)     { return check_exp(ttisstring(o), sobj((o)->value.gc)); }
FASTAPI bool        bvalue(const TValue *o)         { return check_exp(ttisboolean(o), (o)->value.b); }

FASTAPI GCObject*     gcvalue(const TValue *o)        { return check_exp(iscollectable(o), gcobj((o)->value.gc)); }
FASTAPI void*         pvalue(const TValue *o)         { return check_exp(ttislightuserdata(o), (o)->value.p); }
FASTAPI TString*      tsvalue(const TValue *o)        { return check_exp(ttisstring(o), sobj((o)->value.gc)); }
FASTAPI char*         rawuvalue(const TValue *o)      { return (char*)check_exp(ttisuserdata(o), uobj((o)->value.gc)); }
FASTAPI Udata*        uvalue(const TValue *o)         { return check_exp(ttisuserdata(o), uobj((o)->value.gc)); }
FASTAPI Closure*      clvalue(const TValue *o)        { return check_exp(ttisfunction(o), clobj((o)->value.gc)); }
FASTAPI Table*        hvalue(const TValue *o)         { return check_exp(ttistable(o), hobj((o)->value.gc)); }
FASTAPI lua_State*    thvalue(const TValue *o)        { return check_exp(ttisthread(o), thobj((o)->value.gc)); }
FASTAPI Dispatch*     qvalue(const TValue *o)         { return check_exp(ttisdispatch(o), qobj((o)->value.gc)); }
FASTAPI Class*        jvalue(const TValue *o)         { return check_exp(ttisclass(o), jobj((o)->value.gc)); }

FASTAPI bool        l_isfalse(const TValue *o)    { return (ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0)); }
#endif //LUA_USE_C_MACROS

/*
** some userful bit tricks
*/
#ifdef LUA_USE_C_MACROS

#define resetbits(x,m)	((x) &= cast(lu_byte, ~(m)))
#define setbits(x,m)	((x) |= (m))
#define testbits(x,m)	((x) & (m))
#define bitmask(b)	(1<<(b))
#define bit2mask(b1,b2)	(bitmask(b1) | bitmask(b2))
#define l_setbit(x,b)	setbits(x, bitmask(b))
#define resetbit(x,b)	resetbits(x, bitmask(b))
#define testbit(x,b)	testbits(x, bitmask(b))
#define set2bits(x,b1,b2)	setbits(x, (bit2mask(b1, b2)))
#define reset2bits(x,b1,b2)	resetbits(x, (bit2mask(b1, b2)))
#define test2bits(x,b1,b2)	testbits(x, (bit2mask(b1, b2)))

#else

template <typename bitMask> FASTAPI void resetbits(bitMask& x, bitMask m)                   { ((x) &= cast(lu_byte, ~(m))); }
template <typename bitMask> FASTAPI void setbits(bitMask& x, bitMask m)                     { ((x) |= (m)); }
template <typename bitMask> FASTAPI bool testbits(bitMask x, bitMask m)                     { return ( ((x) & (m)) != 0 ); }

template <typename bitMaskType> FASTAPI bitMaskType bitmask(bitMaskType b)                  { return ((bitMaskType)1<<(b)); }

template <typename bitMask> FASTAPI bitMask bit2mask(bitMask b1, bitMask b2)                { return (bitmask(b1) | bitmask(b2)); }

template <typename bitMask> FASTAPI void l_setbit(bitMask& x, bitMask b)                    { setbits(x, bitmask(b)); }
template <typename bitMask> FASTAPI void resetbit(bitMask& x, bitMask b)                    { resetbits(x, bitmask(b)); }
template <typename bitMask> FASTAPI bool testbit(bitMask x, bitMask b)                      { return testbits(x, bitmask(b)); }
template <typename bitMask> FASTAPI void set2bits(bitMask& x, bitMask b1, bitMask b2)       { setbits(x, (bit2mask(b1, b2))); }
template <typename bitMask> FASTAPI void reset2bits(bitMask& x, bitMask b1, bitMask b2)     { resetbits(x, (bit2mask(b1, b2))); }
template <typename bitMask> FASTAPI bool test2bits(bitMask x, bitMask b1, bitMask b2)       { return testbits(x, (bit2mask(b1, b2))); }

#endif //LUA_USE_C_MACROS


/*
** Layout for bit use in `marked' field:
** bit 0 - object is white (type 0)
** bit 1 - object is white (type 1)
** bit 2 - object is black
** bit 3 - for userdata: has been finalized
** bit 3 - for tables: has weak keys
** bit 4 - for tables: has weak values
** bit 5 - object is fixed (should not be collected)
** bit 6 - object is "super" fixed (only the main thread)
*/

/*
** garbage collection internals.
*/
#define WHITE0BIT	    ((lu_byte)0)
#define WHITE1BIT	    ((lu_byte)1)
#define BLACKBIT	    ((lu_byte)2)
#define FINALIZEDBIT	((lu_byte)3)
#define KEYWEAKBIT	    ((lu_byte)3)
#define VALUEWEAKBIT	((lu_byte)4)
#define FIXEDBIT	    ((lu_byte)5)
#define SFIXEDBIT	    ((lu_byte)6)
#define WHITEBITS	    ((lu_byte)bit2mask(WHITE0BIT, WHITE1BIT))

#ifdef LUA_USE_C_MACROS

#define iswhite(x)      test2bits((x)->marked, WHITE0BIT, WHITE1BIT)
#define isblack(x)      testbit((x)->marked, BLACKBIT)
#define isgray(x)	(!isblack(x) && !iswhite(x))

#define otherwhite(g)	(g->currentwhite ^ WHITEBITS)
#define isdead(g,v)	((v)->marked & otherwhite(g) & WHITEBITS)

#define changewhite(x)	((x)->marked ^= WHITEBITS)
#define gray2black(x)	l_setbit((x)->marked, BLACKBIT)

#define valiswhite(x)	(iscollectable(x) && iswhite(gcvalue(x)))

#define checkconsistency(obj) \
  lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->tt))

#define checkliveness(g,obj) \
  lua_assert(!iscollectable(obj) || \
  ((ttype(obj) == (obj)->value.gc->tt) && !isdead(g, (obj)->value.gc)))

#else

FASTAPI bool iswhite(const GCObject *x)         { return test2bits((lu_byte)(x)->marked, WHITE0BIT, WHITE1BIT); }
FASTAPI bool isblack(const GCObject *x)         { return testbit((lu_byte)(x)->marked, BLACKBIT); }
FASTAPI bool isgray(const GCObject *x)          { return (!isblack(x) && !iswhite(x)); }

FASTAPI lu_byte otherwhite(global_State *g)     { return g->currentwhite ^ WHITEBITS; }

FASTAPI bool isdead(global_State *g, const GCObject *v)     { return ( ((v)->marked & otherwhite(g) & WHITEBITS) != 0 ); }

FASTAPI void changewhite(GCObject *x)   { ((x)->marked ^= WHITEBITS); }
FASTAPI void gray2black(GCObject *x)    { l_setbit((x)->marked, BLACKBIT); }

FASTAPI bool valiswhite(const TValue *x)    { return (iscollectable(x) && iswhite(gcvalue(x))); }

FASTAPI void checkconsistency(TValue *obj)
{ lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->tt)); }

FASTAPI void checkliveness(global_State *g, TValue *obj)
{ lua_assert(!iscollectable(obj) || ((ttype(obj) == (obj)->value.gc->tt) && !isdead(g, (obj)->value.gc))); }

#endif //LUA_USE_C_MACROS


/* Macros to set values */
#ifdef LUA_USE_C_MACROS

#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)

#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }

#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x) & 1; i_o->tt=LUA_TBOOLEAN; }

#define setgcvalue(L,obj,x) \
  { GCObject *inst = gcobj(x); \
    TValue *i_o=(obj); \
    i_o->value.gc=inst; i_o->tt=inst->tt; \
    checkliveness(G(L),i_o); }

#define setsvalue(L,obj,x) \
  { TString *inst = sobj(x); \
    TValue *i_o=(obj); \
    i_o->value.gc=inst; i_o->tt=LUA_TSTRING; \
    checkliveness(G(L),i_o); }

#define setuvalue(L,obj,x) \
  { Udata *inst = uobj(x); \
    TValue *i_o=(obj); \
    i_o->value.gc=inst; i_o->tt=LUA_TUSERDATA; \
    checkliveness(G(L),i_o); }

#define setthvalue(L,obj,x) \
  { lua_State *inst = thobj(x); \
    TValue *i_o=(obj); \
    i_o->value.gc=inst; i_o->tt=LUA_TTHREAD; \
    checkliveness(G(L),i_o); }

#define setclvalue(L,obj,x) \
  { Closure *inst = clobj(x); \
    TValue *i_o=(obj); \
    i_o->value.gc=inst; i_o->tt=LUA_TFUNCTION; \
    checkliveness(G(L),i_o); }

#define sethvalue(L,obj,x) \
  { Table *inst = hobj(x); \
    TValue *i_o=(obj); \
    i_o->value.gc=inst; i_o->tt=LUA_TTABLE; \
    checkliveness(G(L),i_o); }

#define setptvalue(L,obj,x) \
  { Proto *inst = ptobj(x); \
    TValue *i_o=(obj); \
    i_o->value.gc=inst; i_o->tt=LUA_TPROTO; \
    checkliveness(G(L),i_o); }

#define setqvalue(L,obj,x) \
  { Dispatch *inst = qobj(x); \
    TValue *i_o=(obj); \
    i_o->value.gc=inst; i_o->tt=LUA_TDISPATCH; \
    checkliveness(G(L),i_o); }

#define setjvalue(L,obj,x) \
  { Class *inst = jobj(x); \
    TValue *i_o=(obj); \
    i_o->value.gc=inst; i_o->tt=LUA_TCLASS; \
    checkliveness(G(L),i_o); }



#define setobj(L,obj1,obj2) \
  { const TValue *o2=(obj2); TValue *o1=(obj1); \
    o1->value = o2->value; o1->tt=o2->tt; \
    checkliveness(G(L),o1); }

#else

FASTAPI void setnilvalue(TValue *obj)       { obj->tt = LUA_TNIL; }
FASTAPI void setnvalue(TValue *obj, lua_Number num )
{
    obj->value.n = num;
    obj->tt = LUA_TNUMBER;
}
FASTAPI void setpvalue(TValue *obj, void *ptr )
{
    obj->value.p = ptr;
    obj->tt = LUA_TLIGHTUSERDATA;
}
FASTAPI void setbvalue(TValue *obj, bool bval )
{
    obj->value.b = bval;
    obj->tt = LUA_TBOOLEAN;
}
FASTAPI void setgcvalue(lua_State *L, TValue *val, GCObject *obj)
{
    val->value.gc = obj;
    val->tt = obj->tt;
    checkliveness(G(L),val);
}
template <int typeDesc, typename objType>
FASTAPI void setvalue(lua_State *L, TValue *val, objType *obj)
{
    val->value.gc = obj;
    val->tt = typeDesc;
    checkliveness(G(L),val);
}
FASTAPI void setsvalue(lua_State *L, TValue *val, TString *obj)         { setvalue <LUA_TSTRING> ( L, val, obj ); }
FASTAPI void setuvalue(lua_State *L, TValue *val, Udata *obj)           { setvalue <LUA_TUSERDATA> ( L, val, obj ); }
FASTAPI void setthvalue(lua_State *L, TValue *val, lua_State *obj)      { setvalue <LUA_TTHREAD> ( L, val, obj ); }
FASTAPI void setclvalue(lua_State *L, TValue *val, Closure *obj)        { setvalue <LUA_TFUNCTION> ( L, val, obj ); }
FASTAPI void sethvalue(lua_State *L, TValue *val, Table *obj)           { setvalue <LUA_TTABLE> ( L, val, obj ); }
FASTAPI void setptvalue(lua_State *L, TValue *val, Proto *obj)          { setvalue <LUA_TPROTO> ( L, val, obj ); }
FASTAPI void setqvalue(lua_State *L, TValue *val, Dispatch *obj)        { setvalue <LUA_TDISPATCH> ( L, val, obj ); }
FASTAPI void setjvalue(lua_State *L, TValue *val, Class *obj)           { setvalue <LUA_TCLASS> ( L, val, obj ); }
FASTAPI void setobj(lua_State *L, TValue *dstVal, const TValue *srcVal)
{
    dstVal->value = srcVal->value;
    dstVal->tt = srcVal->tt;
    checkliveness(G(L),dstVal);
}

#endif //LUA_USE_C_MACROS


/*
** different types of sets, according to destination
*/
#ifdef LUA_USE_C_MACROS

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

#else

/* from stack to (same) stack */
FASTAPI void setobjs2s(lua_State *L, StkId dstVal, const StkId srcVal)          { setobj(L, dstVal, srcVal); }
/* to stack (not from same stack) */
FASTAPI void setobj2s(lua_State *L, StkId dstVal, const TValue *srcVal)         { setobj(L, dstVal, srcVal); }
FASTAPI void setsvalue2s(lua_State *L, StkId val, TString *obj)                 { setsvalue(L, val, obj); }
FASTAPI void sethvalue2s(lua_State *L, StkId val, Table *obj)                   { sethvalue(L, val, obj); }
FASTAPI void setptvalue2s(lua_State *L, StkId val, Proto *obj)                  { setptvalue(L, val, obj); }
/* from table to same table */
FASTAPI void setobjt2t(lua_State *L, TValue *dstVal, const TValue *srcVal)      { setobj(L, dstVal, srcVal); }
/* to table */
FASTAPI void setobj2t(lua_State *L, TValue *dstVal, const TValue *srcVal)       { setobj(L, dstVal, srcVal); }
/* to new object */
FASTAPI void setobj2n(lua_State *L, TValue *dstVal, const TValue *srcVal)       { setobj(L, dstVal, srcVal); }
FASTAPI void setsvalue2n(lua_State *L, TValue *val, TString *obj)               { setsvalue(L, val, obj); }

#endif //LUA_USE_C_MACROS

// Macros to access lua_State stuff.
#ifdef LUA_USE_C_MACROS

#define curr_func(L)	(clvalue(L->ci->func))
#define ci_func(ci)	(clvalue((ci)->func))
#define f_isLua(ci)	(!ci_func(ci)->isC)
#define isLua(ci)	(ttisfunction((ci)->func) && f_isLua(ci))

#else

FASTAPI Closure*    curr_func(lua_State *L)     { return (clvalue(L->ci->func)); }
FASTAPI Closure*    ci_func(CallInfo *ci)       { return (clvalue((ci)->func)); }
FASTAPI bool        f_isLua(CallInfo *ci)       { return (!ci_func(ci)->isC); }
FASTAPI bool        isLua(CallInfo *ci)         { return (ttisfunction((ci)->func) && f_isLua(ci)); }

#endif

// Macros for closures.
#ifdef LUA_USE_C_MACROS

#define sizeCclosure(n)	(cast(int, sizeof(CClosureBasic)) + \
                         cast(int, sizeof(TValue)*((n)-1)))

#define sizeCmethod(n)  (cast(int, sizeof(CClosureMethod)) + \
                         cast(int, sizeof(TValue)*((n)-1)))

#define sizeCmethodt(n) (cast(int, sizeof(CClosureMethodTrans)) + \
                         cast(int, sizeof(TValue)*((n)-1)))

#define sizeLclosure(n)	(cast(int, sizeof(LClosure)) + \
                         cast(int, sizeof(TValue*)*((n)-1)))

#define iscfunction(o)	(ttype(o) == LUA_TFUNCTION && clvalue(o)->isC)
#define isLfunction(o)	(ttype(o) == LUA_TFUNCTION && !clvalue(o)->isC)

#else

FASTAPI size_t sizeCclosure( int n )            { return sizeof(CClosureBasic) + ( sizeof(TValue)*((n)-1) ); }
FASTAPI size_t sizeCmethod( int n )             { return sizeof(CClosureMethod) + ( sizeof(TValue)*((n)-1) ); }
FASTAPI size_t sizeCmethodt( int n )            { return sizeof(CClosureMethodTrans) + ( sizeof(TValue)*((n)-1) ); }
FASTAPI size_t sizeLclosure( int n )            { return sizeof(LClosure) + ( sizeof(TValue*)*((n)-1) ); }

FASTAPI bool iscfunction(const TValue *o)       { return (ttype(o) == LUA_TFUNCTION && clvalue(o)->isC); }
FASTAPI bool isLfunction(const TValue *o)       { return (ttype(o) == LUA_TFUNCTION && !clvalue(o)->isC); }

#endif //LUA_USE_C_MACROS

// TString and Udata helper macros.
#ifdef LUA_USE_C_MACROS

#define sizestring(s)	(_sizestring(sizeof(TString),s->len))
#define sizeudata(u)	(_sizeudata(sizeof(Udata),u->len))

#define getstr(ts)	cast(const char *, (ts) + 1)
#define svalue(o)       getstr(rawtsvalue(o))

#else

FASTAPI size_t sizestring( TString *s )         { return (_sizestring(sizeof(TString),s->len)); }
FASTAPI size_t sizeudata( Udata *u )            { return (_sizeudata(sizeof(Udata),u->len)); }

FASTAPI const char* getstr( const TString *ts )             { return cast(const char *, (ts) + 1); }
FASTAPI const char* svalue( const TValue *o )               { return getstr( rawtsvalue( o ) ); }

#endif //LUA_USE_C_MACROS

// Lua state helper macros.
#ifdef LUA_USE_C_MACROS

/* table of globals */
#define gt(L)	(&L->l_gt)

/* registry */
#define registry(L)	(&G(L)->l_registry)

#else

/* table of globals */
FASTAPI TValue* gt( lua_State *L )          { return &L->l_gt; }

/* registry */
FASTAPI TValue* registry( lua_State *L )    { return &G(L)->l_registry; }

#endif //LUA_USE_C_MACROS

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

// Module initialization.
LUAI_FUNC void luaO_init( void );
LUAI_FUNC void luaO_shutdown( void );


#endif

