/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CScriptDebugging.h
*  PURPOSE:     Game Script Debugging class
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _SCRIPT_DEBUGGING_
#define _SCRIPT_DEBUGGING_

class CScriptDebugging : public ScriptDebugging
{
public:
                            CScriptDebugging( class CLuaManager& manager );

    bool                    SetLogfile( const char *path, unsigned int level );

protected:
    void                    NotifySystem( unsigned int level, const filePath& filename, int line, std::string& msg, unsigned char r, unsigned char g, unsigned char b );
    void                    PathRelative( const char *in, filePath& out );
};

#endif //_SCRIPT_DEBUGGING_