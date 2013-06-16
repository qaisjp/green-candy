/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        dxElements/lua_dxElements.h
*  PURPOSE:     Lua dxElements GUI development environment
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_DXELEMENTS_
#define _LUA_DXELEMENTS_

#define LUACLASS_DXELEMENT          160
#define LUACLASS_DXELEMENTS_SYS     161

struct dxElement : public LuaClass
{
    dxElement( lua_State *L, struct dxElements *dx );
    ~dxElement( void );

    bool keepAlive;

    inline void MarkGCCommon( lua_State *L )
    {
        if ( childAPI )     childAPI->Propagate( L );

        if ( keepAlive )
            LuaClass::MarkGC( L );
    }

    void MarkGC( lua_State *L );

    inline void DisableKeepAlive( void )
    {
        keepAlive = false;
    }

    inline void Update( lua_State *L )
    {
        PushMethod( L, "update" );
        lua_call( L, 0, 0 );
    }

    inline void UpdateParent( lua_State *L )
    {
        if ( dxElement *elem = parent )
            elem->Update( L );
    }

    void Unlink( void );

    dxElements *env;
    RwListEntry <dxElement> envNode;
    bool markEnv;

    CVector2D pos;
    CVector2D rect;
    bool visible;
    //events
    //render target
    bool updateTarget;
    bool alwaysOnTop;
    bool changed;
    const char *previousGUIState;
    bool captiveMode;
    bool outbreakMode;
    bool supportAlpha;
    bool disabled;
    RwColor blendColor;
    RwListEntry <dxElement> drawOrderNode;
    RwList <dxElement> drawingOrder;
    RwList <dxElement> captAlwaysOnTop;
    RwList <dxElement> noncaptive;
    RwList <dxElement> alwaysOnTopElements;
    ILuaClass *childAPI;
    dxElement *parent;
    //rootContext
    RwListEntry <dxElement> childNode;
    RwList <dxElement> children;
};

void luaopen_dxElements( lua_State *L );

#endif //_LUA_DXELEMENTS_