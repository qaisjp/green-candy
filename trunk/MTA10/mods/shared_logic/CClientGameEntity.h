/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientGameEntity.h
*  PURPOSE:     Game rendered entity instance
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CLIENT_GAME_ENTITY_
#define _CLIENT_GAME_ENTITY_

#include "CClientStreamElement.h"

class CClientColModel;

#define LUACLASS_GAMEENTITY 104

class CClientGameEntity : public CClientStreamElement
{
public:
                                CClientGameEntity( CClientStreamer *pStreamer, ElementID ID, lua_State *L, bool system );
                                ~CClientGameEntity();

    void                        SetColModel( CClientColModel *col );
    CClientColModel*            GetColModel()                           { return m_colModel; }

    void                        NotifyCreate();

protected:
    CClientColModel*            m_colModel;
};

#endif //_CLIENT_GAME_ENTITY_