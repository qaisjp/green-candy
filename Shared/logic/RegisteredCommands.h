/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/RegisteredCommands.h
*  PURPOSE:     Console command registry
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_REGISTERED_COMMANDS_
#define _BASE_REGISTERED_COMMANDS_

class RegisteredCommands
{
    struct SCommand
    {
        class CLuaMain* pLuaMain;
        std::string key;
        LuaFunctionRef ref;
        bool restrict;
        bool caseSensitive;
    };

public:
                                        RegisteredCommands();
                                        ~RegisteredCommands();

    bool                                Add( class CLuaMain* pLuaMain, const char* szKey, const LuaFunctionRef& iLuaFunction, bool bRestricted, bool bCaseSensitive );
    bool                                Remove( class CLuaMain* pLuaMain, const char* szKey, const LuaFunctionRef& iLuaFunction = LuaFunctionRef() );
    void                                Clear();
    void                                CleanUpForVM( class CLuaMain* pLuaMain );

    bool                                Exists( const char* szKey, class CLuaMain* pLuaMain = NULL );

    bool                                Process( const char* szKey, const char* szArguments, class CClient* pClient );

protected:
    SCommand*                           Get( const char* szKey, class CLuaMain* pLuaMain = NULL );
    void                                Call( class CLuaMain* pLuaMain, const LuaFunctionRef& iLuaFunction, const char* szKey, const char* szArguments, class CClient* pClient );

    list < SCommand* >                  m_Commands;
    list < SCommand* >                  m_TrashCan;
    bool                                m_bIteratingList;
};

#endif //_BASE_REGISTERED_COMMANDS_