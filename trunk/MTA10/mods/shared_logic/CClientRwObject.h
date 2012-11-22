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

class CClientRwObject abstract : public LuaElement
{
    friend class CClientRwFrame;
public:
                                    CClientRwObject( lua_State *L, CRwObject& object );
                                    ~CClientRwObject();

    CRwObject&                      GetObject()                     { return m_object; }
    const CRwObject&                GetObject() const               { return m_object; }

    void                            SetOwner( CResource *res );
    CResource*                      GetOwner() const                { return m_owner; }

protected:
    void                            UnlinkOwner();

    CRwObject&          m_object;

    CResource*          m_owner;
    RwListEntry <CClientRwObject>   m_ownerObjects;

public:
    CClientRwFrame*     m_parent;
};

#endif //_CLIENT_RWOBJECT_