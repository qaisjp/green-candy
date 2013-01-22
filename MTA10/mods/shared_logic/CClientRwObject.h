/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientRwObject.h
*  PURPOSE:     RenderWare object virtualization
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CLIENT_RWOBJECT_
#define _CLIENT_RWOBJECT_

#define LUACLASS_RWOBJECT   97

class CClientRwFrame;

class CClientRwObject abstract
{
    friend class CClientRwFrame;
    friend class CResourceManager;
    friend class CResource;
public:
                                    CClientRwObject( lua_State *L, CRwObject& object );
    virtual                         ~CClientRwObject();

    CRwObject&                      GetObject()                     { return m_object; }
    const CRwObject&                GetObject() const               { return m_object; }

    inline void                     PushStack( lua_State *L )       { m_class->Push( L ); }
    inline void                     PushMethod( lua_State *L, const char *name )    { m_class->PushMethod( L, name ); }

    inline void                     Reference()                     { m_class->IncrementMethodStack( g_L ); }
    inline void                     Dereference()                   { m_class->DecrementMethodStack( g_L ); }

    inline void Reference( lua_class_reference& ref )
    {
        PushStack( g_L );
        new (&ref) lua_class_reference( g_L, -1 );    // Do not construct the object in this scope
        lua_pop( g_L, 1 );
    }

    inline void Reference( luaRefs& refs )
    {
        lua_class_reference *ref = new lua_class_reference;
        Reference( *ref );

        refs.push_back( ref );
    }

    void                            SetOwner( CResource *res );
    CResource*                      GetOwner()                      { return m_owner; }

    void                            SetRoot( CClientRwFrame *root ) { m_root = root; }
    CClientRwFrame*                 GetRoot()                       { return m_root; }

    // Garbage Collector event
    virtual void                    MarkGC( lua_State *L );

    inline bool IsDestroying() const
    {
        return m_class->IsDestroying();
    }

    inline void Destroy()
    {
        // Prevent Lua referencing
        m_class->ClearReferences( g_L );

        m_class->PushMethod( g_L, "destroy" );
        lua_call( g_L, 0, 0 );

        // At this point the class may be destroyed; do not use it anymore!
    }

    inline void Delete()
    {
        Destroy();
    }

protected:
    void                            UnlinkOwner();

    ILuaClass*          m_class;

    CRwObject&          m_object;

    CResource*          m_owner;
    RwListEntry <CClientRwObject>   m_ownerObjects;

public:
    CClientRwFrame*     m_parent;
    CClientRwFrame*     m_root;
};

#endif //_CLIENT_RWOBJECT_