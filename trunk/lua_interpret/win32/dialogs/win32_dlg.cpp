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

static LUA_DECLARE( getRect )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );
    RECT rect;

    GetWindowRect( dlg->handle, &rect );

    lua_pushnumber( L, rect.left );
    lua_pushnumber( L, rect.top );
    lua_pushnumber( L, rect.right );
    lua_pushnumber( L, rect.bottom );
    return 4;
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

static LUA_DECLARE( getClientRect )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );
    RECT rect;

    GetClientRect( dlg->handle, &rect );

    lua_pushnumber( L, rect.left );
    lua_pushnumber( L, rect.top );
    lua_pushnumber( L, rect.right );
    lua_pushnumber( L, rect.bottom );
    return 4;
}

static LUA_DECLARE( getClientOffset )
{
    POINT pt;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( pt.x, 0 );
    argStream.ReadNumber( pt.y, 0 );
    LUA_ARGS_END;

    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    if ( ScreenToClient( dlg->handle, &pt ) != TRUE )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    lua_pushnumber( L, pt.x );
    lua_pushnumber( L, pt.y );
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

static LUA_DECLARE( isMouseOver )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    lua_pushboolean( L, dlg->isMouseOver );
    return 1;
}

static LUA_DECLARE( isActive )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    lua_pushboolean( L, GetActiveWindow() == dlg->handle );
    return 1;
}

static LUA_DECLARE( isFocused )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    lua_pushboolean( L, GetFocus() == dlg->handle );
    return 1;
}

static LUA_DECLARE( isForeground )
{
    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );

    lua_pushboolean( L, GetForegroundWindow() == dlg->handle );
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

static bool GetKeyNameFromCode( unsigned int code, std::string& keyName )
{
    switch( code )
    {
    case VK_LBUTTON:    keyName = "mouse1"; break;
    case VK_RBUTTON:    keyName = "mouse2"; break;
    case VK_CANCEL:     keyName = "cbreak"; break;
    case VK_MBUTTON:    keyName = "mouse3"; break;
    case VK_XBUTTON1:   keyName = "mouse4"; break;
    case VK_XBUTTON2:   keyName = "mouse5"; break;
    case VK_BACK:       keyName = "backspace"; break;
    case VK_TAB:        keyName = "tab"; break;
    case VK_CLEAR:      keyName = "clear"; break;
    case VK_RETURN:     keyName = "enter"; break;
    case VK_SHIFT:      keyName = "shift"; break;
    case VK_CONTROL:    keyName = "ctrl"; break;
    case VK_MENU:       keyName = "menu"; break;
    case VK_PAUSE:      keyName = "pause"; break;
    case VK_CAPITAL:    keyName = "capslock"; break;
    case VK_KANA:       keyName = "kana"; break;
    case VK_JUNJA:      keyName = "junja"; break;
    case VK_FINAL:      keyName = "final"; break;
    case VK_HANJA:      keyName = "hanja"; break;
    //case VK_KANJI:      keyName = "kanji"; break;
    case VK_ESCAPE:     keyName = "escape"; break;
    case VK_CONVERT:    keyName = "convert"; break;
    case VK_NONCONVERT: keyName = "nonconvert"; break;
    case VK_ACCEPT:     keyName = "accept"; break;
    case VK_MODECHANGE: keyName = "modechange"; break;
    case VK_SPACE:      keyName = "space"; break;
    case VK_PRIOR:      keyName = "pgup"; break;
    case VK_NEXT:       keyName = "pgdn"; break;
    case VK_END:        keyName = "end"; break;
    case VK_HOME:       keyName = "home"; break;
    case VK_LEFT:       keyName = "arrow_l"; break;
    case VK_UP:         keyName = "arrow_u"; break;
    case VK_RIGHT:      keyName = "arrow_r"; break;
    case VK_DOWN:       keyName = "arrow_d"; break;
    case VK_SELECT:     keyName = "select"; break;
    case VK_PRINT:      keyName = "print"; break;
    case VK_EXECUTE:    keyName = "execute"; break;
    case VK_SNAPSHOT:   keyName = "printscr"; break;
    case VK_INSERT:     keyName = "insert"; break;
    case VK_DELETE:     keyName = "delete"; break;
    case VK_HELP:       keyName = "help"; break;
    case '0':           keyName = "0"; break;
    case '1':           keyName = "1"; break;
    case '2':           keyName = "2"; break;
    case '3':           keyName = "3"; break;
    case '4':           keyName = "4"; break;
    case '5':           keyName = "5"; break;
    case '6':           keyName = "6"; break;
    case '7':           keyName = "7"; break;
    case '8':           keyName = "8"; break;
    case '9':           keyName = "9"; break;
    case 'A':           keyName = "a"; break;
    case 'B':           keyName = "b"; break;
    case 'C':           keyName = "c"; break;
    case 'D':           keyName = "d"; break;
    case 'E':           keyName = "e"; break;
    case 'F':           keyName = "f"; break;
    case 'G':           keyName = "g"; break;
    case 'H':           keyName = "h"; break;
    case 'I':           keyName = "i"; break;
    case 'J':           keyName = "j"; break;
    case 'K':           keyName = "k"; break;
    case 'L':           keyName = "l"; break;
    case 'M':           keyName = "m"; break;
    case 'N':           keyName = "n"; break;
    case 'O':           keyName = "o"; break;
    case 'P':           keyName = "p"; break;
    case 'Q':           keyName = "q"; break;
    case 'R':           keyName = "r"; break;
    case 'S':           keyName = "s"; break;
    case 'T':           keyName = "t"; break;
    case 'U':           keyName = "u"; break;
    case 'V':           keyName = "v"; break;
    case 'W':           keyName = "w"; break;
    case 'X':           keyName = "x"; break;
    case 'Y':           keyName = "y"; break;
    case 'Z':           keyName = "z"; break;
    case VK_LWIN:       keyName = "lwin"; break;
    case VK_RWIN:       keyName = "rwin"; break;
    case VK_APPS:       keyName = "apps"; break;
    case VK_SLEEP:      keyName = "sleep"; break;
    case VK_NUMPAD0:    keyName = "num_0"; break;
    case VK_NUMPAD1:    keyName = "num_1"; break;
    case VK_NUMPAD2:    keyName = "num_2"; break;
    case VK_NUMPAD3:    keyName = "num_3"; break;
    case VK_NUMPAD4:    keyName = "num_4"; break;
    case VK_NUMPAD5:    keyName = "num_5"; break;
    case VK_NUMPAD6:    keyName = "num_6"; break;
    case VK_NUMPAD7:    keyName = "num_7"; break;
    case VK_NUMPAD8:    keyName = "num_8"; break;
    case VK_NUMPAD9:    keyName = "num_9"; break;
    case VK_MULTIPLY:   keyName = "num_mul"; break;
    case VK_ADD:        keyName = "num_add"; break;
    case VK_SEPARATOR:  keyName = "num_sep"; break;
    case VK_SUBTRACT:   keyName = "num_sub"; break;
    case VK_DECIMAL:    keyName = "num_dec"; break;
    case VK_DIVIDE:     keyName = "num_div"; break;
    case VK_F1:         keyName = "f1"; break;
    case VK_F2:         keyName = "f2"; break;
    case VK_F3:         keyName = "f3"; break;
    case VK_F4:         keyName = "f4"; break;
    case VK_F5:         keyName = "f5"; break;
    case VK_F6:         keyName = "f6"; break;
    case VK_F7:         keyName = "f7"; break;
    case VK_F8:         keyName = "f8"; break;
    case VK_F9:         keyName = "f9"; break;
    case VK_F10:        keyName = "f10"; break;
    case VK_F11:        keyName = "f11"; break;
    case VK_F12:        keyName = "f12"; break;
    case VK_F13:        keyName = "f13"; break;
    case VK_F14:        keyName = "f14"; break;
    case VK_F15:        keyName = "f15"; break;
    case VK_F16:        keyName = "f16"; break;
    case VK_F17:        keyName = "f17"; break;
    case VK_F18:        keyName = "f18"; break;
    case VK_F19:        keyName = "f19"; break;
    case VK_F20:        keyName = "f20"; break;
    case VK_F21:        keyName = "f21"; break;
    case VK_F22:        keyName = "f22"; break;
    case VK_F23:        keyName = "f23"; break;
    case VK_F24:        keyName = "f24"; break;
    case VK_NUMLOCK:    keyName = "num_lock"; break;
    case VK_SCROLL:     keyName = "scroll"; break;
    case VK_LSHIFT:     keyName = "lshift"; break;
    case VK_RSHIFT:     keyName = "rshift"; break;
    case VK_LCONTROL:   keyName = "lctrl"; break;
    case VK_RCONTROL:   keyName = "rctrl"; break;
    case VK_LMENU:      keyName = "lmenu"; break;
    case VK_RMENU:      keyName = "rmenu"; break;
    case VK_BROWSER_BACK:   keyName = "browser_back"; break;
    case VK_BROWSER_FORWARD:    keyName = "browser_forward"; break;
    case VK_BROWSER_REFRESH:    keyName = "browser_refresh"; break;
    case VK_BROWSER_STOP:   keyName = "browser_stop"; break;
    case VK_BROWSER_SEARCH: keyName = "browser_search"; break;
    case VK_BROWSER_FAVORITES:  keyName = "browser_fav"; break;
    case VK_BROWSER_HOME:   keyName = "browser_home"; break;
    case VK_VOLUME_MUTE:    keyName = "vol_mute"; break;
    case VK_VOLUME_DOWN:    keyName = "vol_down"; break;
    case VK_VOLUME_UP:  keyName = "vol_up"; break;
    case VK_MEDIA_NEXT_TRACK:   keyName = "next_track"; break;
    case VK_MEDIA_PREV_TRACK:   keyName = "prev_track"; break;
    case VK_MEDIA_STOP: keyName = "media_stop"; break;
    case VK_MEDIA_PLAY_PAUSE:   keyName = "media_playpause"; break;
    case VK_LAUNCH_MAIL:    keyName = "launch_mail"; break;
    case VK_LAUNCH_MEDIA_SELECT:    keyName = "launch_media_select"; break;
    case VK_LAUNCH_APP1:    keyName = "launch_app1"; break;
    case VK_LAUNCH_APP2:    keyName = "launch_app2"; break;
    case VK_OEM_1:      keyName = "oem_1"; break;
    case VK_OEM_PLUS:   keyName = "oem_plus"; break;
    case VK_OEM_COMMA:  keyName = "oem_comma"; break;
    case VK_OEM_MINUS:  keyName = "oem_minus"; break;
    case VK_OEM_PERIOD: keyName = "oem_period"; break;
    case VK_OEM_2:      keyName = "oem_2"; break;
    case VK_OEM_3:      keyName = "oem_3"; break;
    case VK_OEM_4:      keyName = "oem_4"; break;
    case VK_OEM_5:      keyName = "oem_5"; break;
    case VK_OEM_6:      keyName = "oem_6"; break;
    case VK_OEM_7:      keyName = "oem_7"; break;
    case VK_OEM_8:      keyName = "oem_8"; break;
    case VK_OEM_102:    keyName = "oem_102"; break;
    case VK_PROCESSKEY: keyName = "process"; break;
    case VK_ATTN:       keyName = "attn"; break;
    case VK_CRSEL:      keyName = "crsel"; break;
    case VK_EXSEL:      keyName = "exsel"; break;
    case VK_EREOF:      keyName = "ereof"; break;
    case VK_PLAY:       keyName = "play"; break;
    case VK_ZOOM:       keyName = "zoom"; break;
    case VK_NONAME:     keyName = "noname"; break;
    case VK_PA1:        keyName = "pa1"; break;
    case VK_OEM_CLEAR:  keyName = "clear"; break;
    default:            return false;
    }

    return true;
}

bool GetCodeFromKeyName( const std::string& keyName, unsigned int& code )
{
    for ( unsigned int n = 0; n < 256; n++ )
    {
        std::string _keyName;

        if ( GetKeyNameFromCode( n, _keyName ) && _keyName == keyName )
        {
            code = n;
            return true;
        }
    }

    return false;
}

static LUA_DECLARE( getKeyState )
{
    std::string keyName;

    LUA_ARGS_BEGIN;
    argStream.ReadString( keyName );
    LUA_ARGS_END;

    unsigned int keyCode;

    LUA_CHECK( GetCodeFromKeyName( keyName, keyCode ) );

    Win32Dialog *dlg = (Win32Dialog*)lua_getmethodtrans( L );
    Win32Dialog::keyState_t& state = dlg->keyStates[keyCode];

    lua_pushboolean( L, state.isDown );
    return 1;
}

static const luaL_Reg dialog_interface[] =
{
    LUA_METHOD( setPosition ),
    LUA_METHOD( setSize ),
    LUA_METHOD( getSize ),
    LUA_METHOD( getRect ),
    LUA_METHOD( getClientSize ),
    LUA_METHOD( getClientRect ),
    LUA_METHOD( getClientOffset ),
    LUA_METHOD( setVisible ),
    LUA_METHOD( getText ),
    LUA_METHOD( setText ),
    LUA_METHOD( isMouseOver ),
    LUA_METHOD( isActive ),
    LUA_METHOD( isFocused ),
    LUA_METHOD( isForeground ),
    LUA_METHOD( update ),
    LUA_METHOD( getKeyState ),
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

static AINLINE void PostMouseClickEvent( lua_State *L, Win32Dialog *dlg, short x, short y, const char *buttonName, bool isDown )
{
    dlg->PushMethod( L, "triggerEvent" );
    lua_pushcstring( L, "onMouseClick" );
    lua_pushstring( L, buttonName );
    lua_pushstring( L, isDown ? "down" : "up" );
    lua_pushinteger( L, x );
    lua_pushinteger( L, y );
    lua_call( L, 5, 0 );
}

static AINLINE bool GetExtendedKeyUpdate( WPARAM wparam, LPARAM lparam, bool isDown, unsigned char& keyCode, std::string& keyName )
{
    // Special handle extended keys.
    // See http://wunderwerk.blogspot.de/2012/05/detect-left-and-right-shift-ctrl-and.html
    if ( wparam == VK_MENU )
    {
        if ( lparam & ( 1 << 24 ) )
        {
            keyName = "ralt";
            keyCode = VK_RMENU;
        }
        else
        {
            keyName = "lalt";
            keyCode = VK_LMENU;
        }

        return true;
    }
    else if ( wparam == VK_CONTROL )
    {
        if ( lparam & ( 1 << 24 ) )
        {
            keyName = "lctrl";
            keyCode = VK_LCONTROL;
        }
        else
        {
            keyName = "rctrl";
            keyCode = VK_RCONTROL;
        }

        return true;
    }
    else if ( wparam == VK_SHIFT )
    {
        DWORD lshiftStatus = GetKeyState( VK_LSHIFT );
        DWORD rshiftStatus = GetKeyState( VK_RSHIFT );

        if ( ( ( lshiftStatus >> 15 ) & 0x01 ) & ( isDown ? 0x01 : 0x00 ) )
        {
            keyName = "lshift";
            keyCode = VK_LSHIFT;
            return true;
        }
        else if ( ( ( rshiftStatus >> 15 ) & 0x01 ) & ( isDown ? 0x01 : 0x00 ) )
        {
            keyName = "rshift";
            keyCode = VK_RSHIFT;
            return true;
        }
    }

    return false;
}

LRESULT CALLBACK DialogProcedure( HWND myWindow, UINT msg, WPARAM wparam, LPARAM lparam )
{
    lua_State *L = userLuaState;
    Win32Dialog *dlg = (Win32Dialog*)GetWindowLong( myWindow, GWL_USERDATA );

    if ( !dlg )
        return DefWindowProc( myWindow, msg, wparam, lparam );

    // Reference the window so that it will not be destroyed during this routine.
    lua_class_reference winRef( L, dlg->GetClass() );

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
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        {
            std::string keyName;
            unsigned char keyCode;

            // Attempt to get an extended key.
            bool keySuccess = GetExtendedKeyUpdate( wparam, lparam, true, keyCode, keyName );

            // If we have not found a special key, check the key database.
            if ( !keySuccess )
            {
                keySuccess = GetKeyNameFromCode( wparam, keyName );

                if ( keySuccess )
                {
                    keyCode = (unsigned char)wparam;
                }
            }

            // If we succeeded to find a key, trigger the event handler.
            if ( keySuccess )
            {
                Win32Dialog::keyState_t& state = dlg->keyStates[wparam];
                state.isDown = true;

                dlg->PushMethod( L, "triggerEvent" );
                lua_pushcstring( L, "onKey" );
                lua_pushlstring( L, keyName.c_str(), keyName.size() );
                lua_pushboolean( L, true );
                lua_call( L, 3, 0 );
            }
        }
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        {
            std::string keyName;
            unsigned char keyCode;

            // Attempt to get an extended key.
            bool keySuccess = GetExtendedKeyUpdate( wparam, lparam, false, keyCode, keyName );

            // If we have not found a special key, check the key database.
            if ( !keySuccess )
            {
                keySuccess = GetKeyNameFromCode( wparam, keyName );

                if ( keySuccess )
                {
                    keyCode = (unsigned char)wparam;
                }
            }

            // If we succeeded to find a key, trigger the event handler.
            if ( keySuccess )
            {
                Win32Dialog::keyState_t& state = dlg->keyStates[wparam];
                state.isDown = false;

                dlg->PushMethod( L, "triggerEvent" );
                lua_pushcstring( L, "onKey" );
                lua_pushlstring( L, keyName.c_str(), keyName.size() );
                lua_pushboolean( L, false );
                lua_call( L, 3, 0 );
            }
        }
        break;
    case WM_CHAR:
        {
            // Only pass-through printable characters.
            // Control codes are assumed to be non-printable.
            if ( wparam >= 0x20 )
            {
                // Put the charCode into a ASCII character buffer.
                char charCode[1] = { wparam };

                dlg->PushMethod( L, "triggerEvent" );
                lua_pushcstring( L, "onInput" );
                lua_pushlstring( L, charCode, 1 );
                lua_call( L, 2, 0 );
            }
        }
        break;
    case WM_MOUSEMOVE:
        {
            short x = (short)LOWORD( lparam );
            short y = (short)HIWORD( lparam );

            // We first must identify mouse captures.
            if ( !dlg->isMouseOver )
            {
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = myWindow;

                // Request mouse tracking.
                if ( TrackMouseEvent( &tme ) )
                {
                    // We can track the event.
                    // Now we must notify the event system.
                    dlg->PushMethod( L, "triggerEvent" );
                    lua_pushcstring( L, "onMouseEnter" );
                    lua_pushinteger( L, x );
                    lua_pushinteger( L, y );
                    lua_call( L, 3, 0 );

                    // TODO: should we allow denial?
                    dlg->isMouseOver = true;
                }
            }

            // We can only process movement if the entry succeeded.
            if ( dlg->isMouseOver )
            {
                // Post mouse movement event.
                dlg->PushMethod( L, "triggerEvent" );
                lua_pushcstring( L, "onMouseMove" );
                lua_pushinteger( L, x );
                lua_pushinteger( L, y );
                lua_call( L, 3, 0 );
            }
        }
        break;
    case WM_MOUSELEAVE:
        if ( dlg->isMouseOver )
        {
            // We must notify the system that the mouse is outside.
            short x = (short)LOWORD( lparam );
            short y = (short)HIWORD( lparam );

            // Call the Lua event.
            dlg->PushMethod( L, "triggerEvent" );
            lua_pushcstring( L, "onMouseLeave" );
            lua_pushinteger( L, x );
            lua_pushinteger( L, y );
            lua_call( L, 3, 0 );
            
            // Update the status.
            dlg->isMouseOver = false;
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        {
            short x = (short)LOWORD( lparam );
            short y = (short)HIWORD( lparam );

            // Notify the environment about the mouse click.
            PostMouseClickEvent( L, dlg, x, y, "left", msg == WM_LBUTTONDOWN );
        }
        break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        {
            short x = (short)LOWORD( lparam );
            short y = (short)HIWORD( lparam );

            // Notify the environment about the mouse click.
            PostMouseClickEvent( L, dlg, x, y, "right", msg == WM_RBUTTONDOWN );
        }
        break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        {
            short x = (short)LOWORD( lparam );
            short y = (short)HIWORD( lparam );

            // Notify the environment about the mouse click.
            PostMouseClickEvent( L, dlg, x, y, "middle", msg == WM_MBUTTONDOWN );
        }
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
        return dlg->defaultWndProc( myWindow, msg, wparam, lparam );
	}
	return 0;
}

Win32Dialog::Win32Dialog( lua_State *L, unsigned int w, unsigned int h ) : LuaClass( L, _trefget( L, this ) )
{
    // Set this for the beginning.
    // This is a security measurement.
    defaultWndProc = DefWindowProc;

    handle = CreateWindow( "MTA:Lua_window", NULL, WS_SYSMENU, 0, 0, w, h, NULL, NULL, NULL, NULL );
    SetWindowLong( handle, GWL_USERDATA, (LONG)this );
    defaultWndProc = (wndProc_t)SetWindowLong( handle, GWL_WNDPROC, (LONG)DialogProcedure );

    if ( !defaultWndProc || defaultWndProc == DialogProcedure )
        defaultWndProc = DefWindowProc;

    // Initially we assume mouse is not on our dialog.
    isMouseOver = false;

    // Initialize common events
    const char *windowEvents[] =
    {
        "onMouseEnter",
        "onMouseMove",
        "onMouseClick",
        "onMouseLeave",
        "onFocus",
        "onBlur",
        "onPaint",
        "onKey",
        "onInput",
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