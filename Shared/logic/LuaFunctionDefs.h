/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaFunctionDefs.h
*  PURPOSE:     Shared Lua function definitions
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_BASE_DEFS_
#define _LUA_BASE_DEFS_

#define LUA_DECLARE(x) int x( lua_State *L )

namespace LuaFunctionDefs
{
    void SetResourceManager( ResourceManager& man );
    void SetRegisteredCommands( RegisteredCommands& cmds );
    void SetDebugging( ScriptDebugging& debug );

    // Reserved functions
    LUA_DECLARE( DisabledFunction );

    // File functions
    LUA_DECLARE( fileCreate );
    LUA_DECLARE( fileExists );
    LUA_DECLARE( fileOpen );
    LUA_DECLARE( fileIsEOF );
    LUA_DECLARE( fileGetPos );
    LUA_DECLARE( fileSetPos );
    LUA_DECLARE( fileGetSize );
    LUA_DECLARE( fileRead );
    LUA_DECLARE( fileWrite );
    LUA_DECLARE( fileFlush );
    LUA_DECLARE( fileClose );
    LUA_DECLARE( fileDelete );
    LUA_DECLARE( fileRename );
    LUA_DECLARE( fileCopy );

    // UTF functions
    LUA_DECLARE( utfLen );
    LUA_DECLARE( utfSeek );
    LUA_DECLARE( utfSub );
    LUA_DECLARE( utfChar );
    LUA_DECLARE( utfCode );

#ifndef _KILLFRENZY
    // XML Functions
    LUA_DECLARE( XMLNodeFindChild );
    LUA_DECLARE( XMLNodeGetChildren );
    LUA_DECLARE( XMLNodeGetValue );
    LUA_DECLARE( XMLNodeSetValue );
    LUA_DECLARE( XMLNodeGetAttributes );
    LUA_DECLARE( XMLNodeGetAttribute );
    LUA_DECLARE( XMLNodeSetAttribute );
    LUA_DECLARE( XMLNodeGetParent );
    LUA_DECLARE( XMLLoadFile );
    LUA_DECLARE( XMLCreateFile );
    LUA_DECLARE( XMLUnloadFile );
    LUA_DECLARE( XMLSaveFile );
    LUA_DECLARE( XMLCreateChild );
    LUA_DECLARE( XMLDestroyNode );
    LUA_DECLARE( XMLCopyFile );
    LUA_DECLARE( XMLNodeGetName );
    LUA_DECLARE( XMLNodeSetName );
#endif //_KILLFRENZY

    // JSON functions
    LUA_DECLARE( toJSON );
    LUA_DECLARE( fromJSON );

    // Utility
    LUA_DECLARE( md5 );
    LUA_DECLARE( gettok );
    LUA_DECLARE( split );
    LUA_DECLARE( setTimer );
    LUA_DECLARE( killTimer );
    LUA_DECLARE( resetTimer );
    LUA_DECLARE( getTimers );
    LUA_DECLARE( isTimer );
    LUA_DECLARE( getTimerDetails );
    LUA_DECLARE( getTickCount );
    LUA_DECLARE( getCTime );
    LUA_DECLARE( getColorFromString );
    LUA_DECLARE( getValidPedModels );
    LUA_DECLARE( outputDebugString );
    LUA_DECLARE( print );

    LUA_DECLARE( getDistance2D );
    LUA_DECLARE( getDistance3D );
};

#endif //_LUA_BASE_DEFS_