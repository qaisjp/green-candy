/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaMain.h
*  PURPOSE:     Lua hyperstructure extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_MAIN_
#define _LUA_MAIN_

class CLuaMain : public LuaMain
{
    friend class CLuaManager;

                            CLuaMain( class CLuaManager& man, CFileTranslator& root );
                            ~CLuaMain();

public:
    bool                    ParseRelative( const char *in, filePath& out ) const;

    CFileTranslator&        m_fileRoot;
};

#endif //_LUA_MAIN_