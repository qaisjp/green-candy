/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/ScriptDebugging.h
*  PURPOSE:     Script debugging base
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_SCRIPT_DEBUGGING_
#define _BASE_SCRIPT_DEBUGGING_

using namespace std;

class ScriptDebugging
{
public:
                                    ScriptDebugging                 ();
    inline                          ~ScriptDebugging                ();

    void                            LogCustom                       ( lua_State *lua, unsigned char red, unsigned char green, unsigned char blue, const char *fmt, ... );
    void                            LogInformation                  ( lua_State *lua, const char *fmt, ... );
    void                            LogWarning                      ( lua_State *lua, const char *fmt, ... );
    void                            LogError                        ( lua_State *lua, const char *fmt, ... );
    void                            LogBadPointer                   ( lua_State *lua, const char *func, const char *argType, unsigned int argID );
    void                            LogBadType                      ( lua_State *lua, const char *func );
    void                            LogBadLevel                     ( lua_State *lua, const char *func, unsigned int level );

protected:
    // Inheritance requirements
    virtual void                    NotifySystem                    ( unsigned int level, lua_State *lua, const filePath& filename, int line, std::string& msg, unsigned char r, unsigned char g, unsigned char b ) = 0;
    virtual void                    PathRelative                    ( const char *in, filePath& out ) = 0;

    void                            LogString                       ( const char *pre, lua_State *lua, const char *msg, unsigned int minLevel, unsigned char red = 255, unsigned char green = 255, unsigned char blue = 255 );
    void                            PrintLog                        ( const char *msg );

    CFile*                          m_file;
    unsigned int                    m_fileLevel;
};

#endif //_BASE_SCRIPT_DEBUGGING_