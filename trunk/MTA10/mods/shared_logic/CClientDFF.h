/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientDFF.h
*  PURPOSE:     .dff model handling class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CClientDFF;

#ifndef __CCLIENTDFF_H
#define __CCLIENTDFF_H

#define LUACLASS_DFF    23

class CClientDFF : public LuaElement
{
public:
                                    CClientDFF( LuaClass& root, CModel& model );
                                    ~CClientDFF();

    const char*                     GetName() const         { return m_model.GetName(); }
    unsigned int                    GetHash() const         { return m_model.GetHash(); }

    bool                            ReplaceModel( unsigned short id );
    bool                            HasReplaced( unsigned short id );
    void                            RestoreModel( unsigned short id );
    void                            RestoreModels();

protected:
    void                            RestreamAll() const;

    CModel&                         m_model;
};

#endif
