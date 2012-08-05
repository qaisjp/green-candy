/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CCustomData.h
*  PURPOSE:     Custom data storage class header
*  DEVELOPERS:  Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*
*****************************************************************************/

#ifndef __CCUSTOMDATA_H
#define __CCUSTOMDATA_H

#include "lua/CLuaArgument.h"

#define MAX_CUSTOMDATA_NAME_LENGTH 128

struct SCustomData
{
    CLuaArgument        Variable;
    class CLuaMain*     pLuaMain;
};

class CCustomData
{
public:

    void                    Copy( CCustomData* pCustomData );

    SCustomData*            Get( const char* szName );
    void                    Set( const char* szName, const CLuaArgument& Variable, class CLuaMain* pLuaMain );

    bool                    Delete( const char* szName );
    void                    DeleteAll( class CLuaMain* pLuaMain );
    void                    DeleteAll();

    inline unsigned int     Count()             { return m_Data.size(); }

    typedef std::map <std::string, SCustomData> itemList_t;

    itemList_t::const_iterator  IterBegin()     { return m_Data.begin(); }
    itemList_t::const_iterator  IterEnd()       { return m_Data.end(); }

private:
    std::map < std::string, SCustomData >       m_Data;
};

#endif
