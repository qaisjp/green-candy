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
    LUA_DECLARE( UtfLen );
    LUA_DECLARE( UtfSeek );
    LUA_DECLARE( UtfSub );
    LUA_DECLARE( UtfChar );
    LUA_DECLARE( UtfCode );

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
    LUA_DECLARE( Md5 );
    LUA_DECLARE( GetTok );
    LUA_DECLARE( Split );
    LUA_DECLARE( SetTimer );
    LUA_DECLARE( KillTimer );
    LUA_DECLARE( ResetTimer );
    LUA_DECLARE( GetTimers );
    LUA_DECLARE( IsTimer );
    LUA_DECLARE( GetTimerDetails );
    LUA_DECLARE( GetTickCount_ );
    LUA_DECLARE( GetCTime );
    LUA_DECLARE( Reference );
    LUA_DECLARE( Dereference );
    LUA_DECLARE( GetColorFromString );
    LUA_DECLARE( GetValidPedModels );
    LUA_DECLARE( OutputDebugString );

    LUA_DECLARE( GetDistanceBetweenPoints2D );
    LUA_DECLARE( GetDistanceBetweenPoints3D );
    LUA_DECLARE( GetEasingValue );
    LUA_DECLARE( InterpolateBetween );
};

#endif //_LUA_BASE_DEFS_