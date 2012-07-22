/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luaentity.h
*  PURPOSE:     Game Lua entity
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _GAME_ENTITY_
#define _GAME_ENTITY_

#define LUACLASS_ENTITY     10
#define LUACLASS_SYSENTITY  14

class CGameEntity abstract : public LuaElement
{
public:
                            CGameEntity( LuaClass& root, bool system, CEntity& entity );
    virtual                 ~CGameEntity();

    inline CEntity&         GetEntity()     { return m_entity; }
    inline bool             IsSystem()      { return m_system; }

protected:
    CEntity&                m_entity;
    bool                    m_system;   // System entities may not be destroyed
};

#endif //_GAME_ENTITY_