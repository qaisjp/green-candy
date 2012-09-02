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

class CClientRwObject : public LuaElement
{
public:
                                    CClientRwObject( LuaClass& root, CRwObject& object );
                                    ~CClientRwObject();

    CRwObject&                      GetObject()             { return m_object; }
    const CRwObject&                GetObject() const       { return m_object; }

    const char*                     GetName() const         { return m_object.GetName(); }
    unsigned int                    GetHash() const         { return m_object.GetHash(); }

protected:
    CRwObject&  m_object;
};

#endif //_CLIENT_RWOBJECT_