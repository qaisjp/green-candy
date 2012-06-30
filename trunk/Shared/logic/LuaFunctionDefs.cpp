/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaFunctionDefs.cpp
*  PURPOSE:     Shared Lua function definitions
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static ScriptDebugging *debug;
static RegisteredCommands *cmds;
static ResourceManager *resManager;

static inline const char* lua_getclassdesc( int ctype )
{
    switch( ctype )
    {
    case LUACLASS_FILE:
        return "file";
    case LUACLASS_FILETRANSLATOR:
        return "fileTranslator";
    }

    return "class";
}

static inline LuaMain* lua_readcontext( lua_State *L )
{
    return lua_readuserdata <LuaMain, LUA_STORAGEINDEX, 2> ( L );
}

static inline Resource* lua_readresource( lua_State *L )
{
    return lua_readuserdata <Resource, LUA_STORAGEINDEX, 3> ( L );
}

static inline ILuaClass& lua_classobtain( lua_State *L, int idx, int ctype )
{
    luaL_checktype( L, idx, LUA_TCLASS );

    ILuaClass *j = lua_refclass( L, idx );

    if ( !j->IsTransmit( ctype ) )
        throw lua_exception( L, LUA_ERRRUN, SString( "expected class type '%s'", lua_getclassdesc( ctype ) ) );

    return *j;
}

static inline void luaL_checktyperange( lua_State *L, int idx, int t, int r )
{
    while ( r-- )
        luaL_checktype( L, idx++, t );
}

namespace LuaFunctionDefs
{
    void SetResourceManager( ResourceManager& man )
    {
        resManager = &man;
    }

    void SetRegisteredCommands( RegisteredCommands& _cmds )
    {
        cmds = &_cmds;
    }
    
    void SetDebugging( ScriptDebugging& _debug )
    {
        debug = &_debug;
    }

    LUA_DECLARE( DisabledFunction )
    {
        debug->LogError( "Unsafe function was called." );

        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( fileCreate )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        CFile *file = resManager->OpenStream( lua_readresource( L ), lua_tostring( L, 1 ), "wb" );

        if ( !file )
        {
            lua_pushboolean( L, false );
            return 1;
        }

        lua_pushlightuserdata( L, file );
        lua_pushcclosure( L, luaconstructor_file, 1 );
        lua_newclass( L );
        return 1;
    }

    LUA_DECLARE( fileExists )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        lua_pushboolean( L, resManager->FileExists( lua_readresource( L ), lua_tostring( L, 1 ) ) );
        return 1;
    }

    LUA_DECLARE( fileOpen )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        CFile *file = resManager->OpenStream( lua_readresource( L ), lua_tostring( L, 1 ), "rb+" );

        if ( !file )
        {
            lua_pushboolean( L, false );
            return 1;
        }

        lua_pushlightuserdata( L, file );
        lua_pushcclosure( L, luaconstructor_file, 1 );
        lua_newclass( L );
        return 1;
    }

    LUA_DECLARE( fileIsEOF )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );

        lua_getfield( L, 1, "eof" );
        lua_call( L, 0, 1 );
        return 1;
    }

    LUA_DECLARE( fileGetPos )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );

        lua_getfield( L, 1, "tell" );
        lua_call( L, 0, 1 );
        return 1;
    }

    LUA_DECLARE( fileSetPos )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        luaL_checktype( L, 2, LUA_TNUMBER );

        lua_getfield( L, 1, "seek" );
        lua_pushvalue( L, 2 );
        lua_pushlstring( L, "set", 3 );
        lua_call( L, 2, 1 );
        return 1;
    }

    LUA_DECLARE( fileGetSize )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        
        lua_getfield( L, 1, "size" );
        lua_call( L, 0, 1 );
        return 1;
    }

    LUA_DECLARE( fileRead )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        luaL_checktype( L, 2, LUA_TNUMBER );

        lua_getfield( L, 1, "read" );
        lua_pushvalue( L, 2 );
        lua_call( L, 1, 1 );
        return 1;
    }

    LUA_DECLARE( fileWrite )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        luaL_checktype( L, 2, LUA_TSTRING );

        lua_getfield( L, 1, "write" );
        lua_pushvalue( L, 2 );
        lua_call( L, 1, 1 );
        return 1;
    }

    LUA_DECLARE( fileFlush )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        
        lua_getfield( L, 1, "flush" );
        lua_call( L, 0, 1 );
        return 1;
    }

    LUA_DECLARE( fileClose )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );

        lua_getfield( L, 1, "destroy" );
        lua_call( L, 0, 0 );

        lua_pushboolean( L, true );
        return 1;
    }

    LUA_DECLARE( fileDelete )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        lua_pushboolean( L, resManager->FileDelete( lua_readresource( L ), lua_tostring( L, 1 ) ) );
        return 1;
    }

    LUA_DECLARE( fileRename )
    {
        luaL_checktype( L, 1, LUA_TSTRING );
        luaL_checktype( L, 2, LUA_TSTRING );

        lua_pushboolean( L, resManager->FileRename( lua_readresource( L ), lua_tostring( L, 1 ), lua_tostring( L, 2 ) ) );
        return 1;
    }

    LUA_DECLARE( fileCopy )
    {
        luaL_checktype( L, 1, LUA_TSTRING );
        luaL_checktype( L, 2, LUA_TSTRING );

        lua_pushboolean( L, resManager->FileCopy( lua_readresource( L ), lua_tostring( L, 1 ), lua_tostring( L, 2 ) ) );
        return 1;
    }

    LUA_DECLARE( utfLen )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        lua_pushnumber( L, MbUTF8ToUTF16( lua_getstring( L, 1 ) ).size() );
        return 1;
    }

    LUA_DECLARE( utfSeek )
    {
        luaL_checktype( L, 1, LUA_TSTRING );
        luaL_checktype( L, 2, LUA_TNUMBER );

        int iPos = (int)lua_tonumber( L, 2 );
        std::wstring utf = MbUTF8ToUTF16( lua_getstring( L, 1 ) );

        if ( iPos > (int)utf.size() || iPos < 0 )
        {
            lua_pushboolean( L, false );
            return 1;
        }

        utf = utf.substr( 0, iPos );
        lua_pushnumber( L, UTF16ToMbUTF8( utf ).size() );
        return 1;
    }

    LUA_DECLARE( utfSub )
    {
        luaL_checktype( L, 1, LUA_TSTRING );
        luaL_checktype( L, 2, LUA_TNUMBER );

        //Ripped and modded Lua source.  It's pretty disgusting, i know.

        const char *s = lua_tostring(L, 1);
        std::wstring strUTF = MbUTF8ToUTF16(s);
        size_t l = static_cast < int > ( strUTF.size() );

        ptrdiff_t start = luaL_checkinteger(L, 2);
        ptrdiff_t end = luaL_optinteger(L, 3, -1);

        //posrelat them both
        if (start < 0) start += (ptrdiff_t)l + 1;
            start = (start >= 0) ? start : 0;

        if (end < 0) end += (ptrdiff_t)l + 1;
            end = (end >= 0) ? end : 0;

        if (start < 1) start = 1;
        if (end > (ptrdiff_t)l) end = (ptrdiff_t)l;
        if (start <= end)
        {
            strUTF = strUTF.substr(start-1, end-start+1);
            lua_pushstring(L, UTF16ToMbUTF8(strUTF).c_str());
        }
        else lua_pushliteral(L, "");
        return 1;
    }

    LUA_DECLARE( utfChar )
    {
        luaL_checktype( L, 1, LUA_TNUMBER );

        int iChar = (int)lua_tonumber( L, 1 );

        if ( iChar > 65534 || iChar < 32 )
            throw lua_exception( L, LUA_ERRRUN, "invalid character to utfChar" );

        // Generate a null-terminating string for our character
        wchar_t wUNICODE[2] = { iChar, '\0' };

        // Return the ANSI representation
        lua_pushlstring( L, UTF16ToMbUTF8( wUNICODE ).c_str(), 1 );
        return 1;
    }

    LUA_DECLARE( utfCode )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        std::wstring utf = MbUTF8ToUTF16( lua_getstring( L, 1 ) );

        lua_pushnumber( L, utf[0] );
        return 1;
    }

    LUA_DECLARE( getDistance2D )
    {
        luaL_checktyperange( L, 1, LUA_TNUMBER, 4 );

        CVector2D a( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ) );
        CVector2D b( (float)lua_tonumber( L, 3 ), (float)lua_tonumber( L, 4 ) );
        a -= b;

        lua_pushnumber( L, a.Length() );
        return 1;
    }

    LUA_DECLARE( getDistance3D )
    {
        luaL_checktyperange( L, 1, LUA_TNUMBER, 6 );

        CVector a( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ) );
        CVector b( (float)lua_tonumber( L, 4 ), (float)lua_tonumber( L, 5 ), (float)lua_tonumber( L, 6 ) );
        a -= b;

        lua_pushnumber( L, a.Length() );
        return 1;
    }

    LUA_DECLARE( getTickCount )
    {
        lua_pushnumber( L, GetSecondCount() * 1000.0 );
        return 1;
    }


    LUA_DECLARE( getCTime )
    {
        time_t timer;
        time( &timer );

        switch( lua_type( L, 1 ) )
        {
        case LUA_TNUMBER:
        case LUA_TSTRING:
            timer = (time_t)lua_tonumber( L, 1 );

            if ( timer < 0 )
            {
                lua_pushboolean( L, false );
                return 1;
            }
        }

        tm *time = localtime( &timer );

        lua_settop( L, 1 );

        lua_newtable( L );
        lua_pushlstring( L, "second", 6 );
        lua_pushnumber( L, time->tm_sec );
        lua_pushlstring( L, "minute", 6 );
        lua_pushnumber( L, time->tm_min );
        lua_pushlstring( L, "hour", 4 );
        lua_pushnumber( L, time->tm_hour );
        lua_pushlstring( L, "monthday", 8 );
        lua_pushnumber( L, time->tm_mday );
        lua_pushlstring( L, "month", 5 );
        lua_pushnumber( L, time->tm_mon );
        lua_pushlstring( L, "year", 4 );
        lua_pushnumber( L, time->tm_year );
        lua_pushlstring( L, "weekday", 7 );
        lua_pushnumber( L, time->tm_wday );
        lua_pushlstring( L, "yearday", 7 );
        lua_pushnumber( L, time->tm_yday );
        lua_pushlstring( L, "isdst", 5 );
        lua_pushboolean( L, time->tm_isdst == 1 );
        lua_pushlstring( L, "timestamp", 9 );
        lua_pushnumber( L, (double)timer );

        lua_rawset( L, 2 );
        lua_rawset( L, 2 );
        lua_rawset( L, 2 );
        lua_rawset( L, 2 );
        lua_rawset( L, 2 );
        lua_rawset( L, 2 );
        lua_rawset( L, 2 );
        lua_rawset( L, 2 );
        lua_rawset( L, 2 );
        lua_rawset( L, 2 );
        return 1;
    }

#if 0
    LUA_DECLARE( Split )
    {
        if ( ( lua_type ( luaVM, 1 ) != LUA_TSTRING ) || ( lua_type ( luaVM, 2 ) != LUA_TNUMBER && ( lua_type ( luaVM, 2 ) != LUA_TSTRING ) ) )
        {
            m_pScriptDebugging->LogBadType ( luaVM, "split" );

            lua_pushboolean ( luaVM, false );
            return 1;
        }

        SString strDelimiter;
        if ( lua_type ( luaVM, 2 ) == LUA_TNUMBER )
        {
            unsigned int uiCharacter = static_cast < unsigned int > ( lua_tonumber ( luaVM, 2 ) );
            wchar_t wUNICODE[2] = { uiCharacter, '\0' };
            strDelimiter = UTF16ToMbUTF8(wUNICODE);
        }
        else  // It's already a string
            strDelimiter = lua_tostring ( luaVM, 2 );

        const char* szText = lua_tostring ( luaVM, 1 );

        // Copy the string
        char* strText = new char [ strlen ( szText ) + 1 ];
        strcpy ( strText, szText );

        unsigned int uiCount = 0;
        char* szToken = strtok ( strText, strDelimiter );

        // Create a new table
        lua_newtable ( luaVM );

        // Add our first token
        lua_pushnumber ( luaVM, ++uiCount );
        lua_pushstring ( luaVM, szToken );
        lua_settable ( luaVM, -3 );

        // strtok until we're out of tokens
        while ( ( szToken = strtok ( NULL, strDelimiter ) ) )
        {
            // Add the token to the table
            lua_pushnumber ( luaVM, ++uiCount );
            lua_pushstring ( luaVM, szToken );
            lua_settable ( luaVM, -3 );
        }

        // Delete the text
        delete [] strText;

        return 1;
    }

    int CLuaFunctionDefinitions::GetTok ( lua_State* luaVM )
    {
        if ( ( lua_type ( luaVM, 1 ) != LUA_TSTRING ) || ( lua_type ( luaVM, 2 ) != LUA_TNUMBER ) || ( ( lua_type ( luaVM, 3 ) != LUA_TNUMBER ) && ( lua_type ( luaVM, 3 ) != LUA_TSTRING ) ) )
        {
            m_pScriptDebugging->LogBadType ( luaVM, "gettok" );

            lua_pushboolean ( luaVM, false );
            return 1;
        }

        SString strDelimiter;
        if ( lua_type ( luaVM, 3 ) == LUA_TNUMBER )
        {
            unsigned int uiCharacter = static_cast < unsigned int > ( lua_tonumber ( luaVM, 3 ) );
            wchar_t wUNICODE[2] = { uiCharacter, '\0' };
            strDelimiter = UTF16ToMbUTF8(wUNICODE);
        }
        else  // It's already a string
            strDelimiter = lua_tostring ( luaVM, 3 );

        const char* szText = lua_tostring ( luaVM, 1 );
        int iToken = static_cast < int > ( lua_tonumber ( luaVM, 2 ) );

        if ( iToken > 0 && iToken < 1024 )
        {
            // Copy the string
            char* strText = new char [ strlen ( szText ) + 1 ];
            strcpy ( strText, szText );

            int iCount = 1;
            char* szToken = strtok ( strText, strDelimiter );

            // We're looking for the first part?
            if ( iToken != 1 )
            {
                // strtok count number of times
                do
                {
                    iCount++;
                    szToken = strtok ( NULL, strDelimiter );
                }
                while ( iCount != iToken );
            }

            // Found it?
            if ( szToken )
            {
                // Return it
                lua_pushstring ( luaVM, szToken );
                delete [] strText;
                return 1;
            }

            // Delete the text
            delete [] strText;
        }
        else
            m_pScriptDebugging->LogWarning ( luaVM, "Token parameter sent to split must be greater than 0 and smaller than 1024" );

        lua_pushboolean ( luaVM, false );
        return 1;
    }
#endif

    LUA_DECLARE( setTimer )
    {
        luaL_checktype( L, 1, LUA_TFUNCTION );
        luaL_checktype( L, 2, LUA_TNUMBER );
        luaL_checktype( L, 3, LUA_TNUMBER );

        LuaMain& main = *lua_readcontext( L );
        unsigned int count = (unsigned int)lua_tonumber( L, 2 );
        double interval = lua_tonumber( L, 3 );

        // Check for the minimum interval
        if ( interval < LUA_TIMER_MIN_INTERVAL )
            throw lua_exception( L, LUA_ERRRUN, "interval is below 50" );

        LuaFunctionRef ref = main.CreateReference( 1 );

        LuaTimer& timer = *main.GetTimerManager().AddTimer( L, ref, CTickCount( interval ), count );
        timer.ObtainArguments( L, 4 );
        timer.PushStack( L );
        return 1;
    }

    LUA_DECLARE( killTimer )
    {
        lua_classobtain( L, 1, LUACLASS_TIMER );

        lua_getfield( L, 1, "destroy" );
        lua_call( L, 0, 0 );
        return 0;
    }

#if 0
    int CLuaFunctionDefinitions::ResetTimer ( lua_State* luaVM )
    {
    //  bool resetTimer ( timer theTimer )
        CLuaTimer* pLuaTimer;

        CScriptArgReader argStream ( luaVM );
        argStream.ReadUserData ( pLuaTimer );

        if ( !argStream.HasErrors () )
        {
            CLuaMain * luaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
            if ( luaMain )
            {
                luaMain->GetTimerManager ()->ResetTimer ( pLuaTimer );

                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", "resetTimer", *argStream.GetErrorMessage () ) );

        lua_pushboolean ( luaVM, false );
        return 1;
    }
#endif

    LUA_DECLARE( isTimer )
    {
        return lua_type( L, 1 ) == LUA_TCLASS && lua_refclass( L, 1 )->IsTransmit( LUACLASS_TIMER );
    }

#if 0
    int CLuaFunctionDefinitions::GetTimers ( lua_State* luaVM )
    {
    //  table getTimers ( [ time ] )
        double dTime;

        CScriptArgReader argStream ( luaVM );
        argStream.ReadNumber ( dTime, 0 );

        if ( !argStream.HasErrors () )
        {
            // Find our VM
            CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
            if ( pLuaMain )
            {
                // Create a new table
                lua_newtable ( luaVM );

                // Add all the timers with less than ulTime left
                pLuaMain->GetTimerManager ()->GetTimers ( CTickCount ( dTime ), luaVM );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", "getTimers", *argStream.GetErrorMessage () ) );

        lua_pushboolean ( luaVM, false );
        return 1;
    }


    int CLuaFunctionDefinitions::GetTimerDetails ( lua_State* luaVM )
    {
    //  int, int, int getTimerDetails ( timer theTimer )
        CLuaTimer* pLuaTimer;

        CScriptArgReader argStream ( luaVM );
        argStream.ReadUserData ( pLuaTimer );

        if ( !argStream.HasErrors () )
        {
            lua_pushnumber( luaVM, pLuaTimer->GetTimeLeft ().ToDouble () );
            lua_pushnumber( luaVM, pLuaTimer->GetRepeats () );
            lua_pushnumber( luaVM, pLuaTimer->GetDelay ().ToDouble () );
            return 3;
        }
        else
            m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", "getTimerDetails", *argStream.GetErrorMessage () ) );

        lua_pushboolean ( luaVM, false );
        return 1;
    }


    int CLuaFunctionDefinitions::GetColorFromString ( lua_State* luaVM )
    {
        unsigned char ucColorRed, ucColorGreen, ucColorBlue, ucColorAlpha;
        int iArgument1 = lua_type ( luaVM, 1 );
        if ( iArgument1 == LUA_TSTRING )
        {
            const char* szColor = lua_tostring ( luaVM, 1 );

            if ( XMLColorToInt ( szColor, ucColorRed, ucColorGreen, ucColorBlue, ucColorAlpha ) )
            {
                lua_pushnumber ( luaVM, ucColorRed );
                lua_pushnumber ( luaVM, ucColorGreen );
                lua_pushnumber ( luaVM, ucColorBlue );
                lua_pushnumber ( luaVM, ucColorAlpha );
                return 4;
            }
        }
        lua_pushboolean ( luaVM, false );
        return 1;
    }

    int CLuaFunctionDefinitions::GetValidPedModels ( lua_State* luaVM )
    {
        int iIndex = 0;
        lua_newtable ( luaVM );
        for( int i = 0; i < 289; i++)
        {
            if ( CPlayerManager::IsValidPlayerModel(i) )
            {
                lua_pushnumber ( luaVM , ++iIndex);
                lua_pushnumber ( luaVM , i);
                lua_settable ( luaVM , -3);
            }
        }

        return 1;
    }
#endif

    LUA_DECLARE( print )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        debug->LogInformation( "%s", lua_tostring( L, 1 ) );
        return 0;
    }
};