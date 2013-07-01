/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        win32/dialogs/win32_dlg.cpp
*  PURPOSE:     Win32 MTA:Lua dialog logic
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( setPosition )
{
    unsigned int x, y;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( x );
    argStream.ReadNumber( y );
    LUA_ARGS_END;

    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    SetWindowPos( dlg->handle, 0, x, y, 0, 0,
        SWP_NOZORDER | SWP_NOSIZE
    );

    LUA_SUCCESS;
}

static LUA_DECLARE( setSize )
{
    unsigned int w, h;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( w );
    argStream.ReadNumber( h );
    LUA_ARGS_END;

    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    SetWindowPos( dlg->handle, 0, 0, 0, w, h,
        SWP_NOZORDER | SWP_NOMOVE
    );

    LUA_SUCCESS;
}

static LUA_DECLARE( getSize )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );
    RECT rect;

    GetWindowRect( dlg->handle, &rect );

    lua_pushnumber( L, rect.right - rect.left );
    lua_pushnumber( L, rect.bottom - rect.top );
    return 2;
}

static LUA_DECLARE( getClientSize )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );
    RECT rect;

    GetClientRect( dlg->handle, &rect );

    lua_pushnumber( L, rect.right - rect.left );
    lua_pushnumber( L, rect.bottom - rect.top );
    return 2;
}

static LUA_DECLARE( setVisible )
{
    bool visible;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( visible );
    LUA_ARGS_END;

    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    SetWindowPos( dlg->handle, 0, 0, 0, 0, 0,
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE |
        ( visible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW )
    );

    LUA_SUCCESS;
}

static LUA_DECLARE( setText )
{
    lua_String text;

    LUA_ARGS_BEGIN;
    argStream.ReadString( text );
    LUA_ARGS_END;

    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    SetWindowText( dlg->handle, text );
    LUA_SUCCESS;
}

static LUA_DECLARE( getText )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );
    int len = GetWindowTextLength( dlg->handle );

    if ( len < 1 )
    {
        lua_pushcstring( L, "" );
        return 1;
    }

    char *buf = new char[len + 1];
    GetWindowText( dlg->handle, buf, len + 1 );
    
    lua_pushlstring( L, buf, len );

    delete buf;
    return 1;
}

static LUA_DECLARE( update )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    HWND wnd = dlg->handle;
    RECT rect;
    GetClientRect( wnd, &rect );

    InvalidateRect( dlg->handle, &rect, false );
    UpdateWindow( dlg->handle );
    return 0;
}

static const luaL_Reg dialog_interface[] =
{
    LUA_METHOD( setPosition ),
    LUA_METHOD( setSize ),
    LUA_METHOD( getSize ),
    LUA_METHOD( getClientSize ),
    LUA_METHOD( setVisible ),
    LUA_METHOD( getText ),
    LUA_METHOD( setText ),
    LUA_METHOD( update ),
    { NULL, NULL }
};

static LUA_DECLARE( destroy )
{
    delete (Win32Dialog*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    return 0;
}

static const luaL_Reg dialog_interface_sys[] =
{
    LUA_METHOD( destroy ),
    { NULL, NULL }
};

static LUA_DECLARE( dlg_constructor )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
    j->SetTransmit( LUACLASS_WIN32DIALOG, dlg );

    j->RegisterInterfaceTrans( L, dialog_interface, 0, LUACLASS_WIN32DIALOG );

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    j->RegisterInterface( L, dialog_interface_sys, 1 );

    // Include the event API
    luaevent_extend( L );

    lua_pushcstring( L, "dialog" );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );

    // Allow extensions by constructors
    lua_basicextend( L );
    return 0;
}

static ILuaClass* _trefget( lua_State *L, Win32Dialog *dlg )
{
    lua_pushlightuserdata( L, dlg );
    lua_pushcclosure( L, dlg_constructor, 1 );
    lua_newclassex( L, LCLASS_API_LIGHT );
    
    ILuaClass *j = lua_refclass( L, -1 );
    lua_pop( L, 1 );
    return j;
}

LRESULT CALLBACK DialogProcedure( HWND myWindow, UINT msg, WPARAM wparam, LPARAM lparam )
{
    lua_State *L = userLuaState;
    Win32Dialog *dlg = (Win32Dialog*)GetWindowLong( myWindow, GWL_USERDATA );

	switch( msg )
	{
	case WM_ACTIVATE:
		break;
    case WM_PAINT:
        dlg->isPainting = true;
        {
            dlg->deviceContext = BeginPaint( myWindow, &dlg->ps );

            dlg->PushMethod( L, "triggerEvent" );
            lua_pushcstring( L, "onPaint" );
            lua_call( L, 1, 0 );

            EndPaint( myWindow, &dlg->ps );
        }
        dlg->isPainting = false;
        return 0;
	case WM_APPCOMMAND:
		break;
	case WM_CLOSE:
        {
            dlg->PushMethod( L, "triggerEvent" );
            lua_pushcstring( L, "onClose" );
            lua_call( L, 1, 1 );

            bool success = lua_toboolean( L, -1 );
            lua_pop( L, 1 );

            if ( success )
		        dlg->Destroy();
        }
		break;
	case WM_DESTROY:
	default:
        return ( dlg ? dlg->defaultWndProc : DefWindowProc )( myWindow, msg, wparam, lparam );
	}
	return 0;
}

Win32Dialog::Win32Dialog( lua_State *L, unsigned int w, unsigned int h ) : LuaClass( L, _trefget( L, this ) )
{
    // Set this for the beginning.
    defaultWndProc = DefWindowProc;

    handle = CreateWindow( "MTA:Lua_window", NULL, WS_SYSMENU, 0, 0, w, h, NULL, NULL, NULL, NULL );
    SetWindowLong( handle, GWL_USERDATA, (LONG)this );
    defaultWndProc = (wndProc_t)SetWindowLong( handle, GWL_WNDPROC, (LONG)DialogProcedure );

    if ( !defaultWndProc || defaultWndProc == DialogProcedure )
        defaultWndProc = DefWindowProc;

    // Initialize common events
    const char *windowEvents[] =
    {
        "onFocus",
        "onBlur",
        "onPaint",
        "onClose",
        "onDestruction"
    };

    m_class->PushMethod( L, "addEvent" );

    for ( unsigned int n = 0; n < NUMELMS(windowEvents); n++ )
    {
        lua_pushvalue( L, -1 );
        lua_pushstring( L, windowEvents[n] );
        lua_call( L, 1, 0 );
    }
}

Win32Dialog::~Win32Dialog()
{
    DestroyWindow( handle );
}

static LUA_DECLARE( createDialog )
{
    unsigned int w, h;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( w );
    argStream.ReadNumber( h );
    LUA_ARGS_END;

    LUA_CHECK( w > 0 && h > 0 );

    Win32Dialog *dlg = new Win32Dialog( L, w, h );
    dlg->PushStack( L );
    return 1;
}

const static luaL_Reg dialogs_interface[] =
{
    LUA_METHOD( createDialog ),
    { NULL, NULL }
};

void luawin32_extendDialogs( lua_State *L )
{
	WNDCLASSEX wndClass;
	memset( &wndClass, 0, sizeof( wndClass ) );
	wndClass.cbSize = sizeof( wndClass );
	wndClass.lpfnWndProc = DialogProcedure;
    wndClass.lpszClassName = "MTA:Lua_window";
	//wndClass.hIcon = LoadIcon( hCur, MAKEINTRESOURCE( IDI_ICON1 ) );
	wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	ATOM classdesc = RegisterClassEx( &wndClass );

    luaL_openlib( L, NULL, dialogs_interface, 0 );
}

void luawin32_updateWindows()
{
}