/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CScriptDebugging.h
*  PURPOSE:     Lua script debugging system
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_SCRIPT_DEBUGGING_DERIVED_
#define _LUA_SCRIPT_DEBUGGING_DERIVED_

class CScriptDebugging : public ScriptDebugging
{
public:
                            CScriptDebugging( class CLuaManager& manager );

    bool                    SetLogfile( const char *path, unsigned int level );

protected:
    void                    NotifySystem( unsigned int level, const filePath& filename, int line, std::string& msg, unsigned char r, unsigned char g, unsigned char b );
    void                    PathRelative( const char *in, filePath& out );
};

#endif //_LUA_SCRIPT_DEBUGGING_DERIVED_