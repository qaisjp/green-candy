/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        dxElements/lua_dxElements.cpp
*  PURPOSE:     Lua dxElements GUI development environment
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

namespace _dxElements
{
    // Define the API for usage with dxElements.
    // What should be supported: D3D and OpenGL
    static LUA_DECLARE( setRenderLayer )
    {
        dxElements *dx = (dxElements*)lua_getmethodtrans( L );

        return 0;
    }

    static LUA_DECLARE( createElement )
    {
        dxElements *dx = (dxElements*)lua_getmethodtrans( L );

        dxElement *element = new dxElement( L, dx );
        element->PushStack( L );
        element->DisableKeepAlive();
        return 1;
    }

    static LUA_DECLARE( getRoot )
    {
        dxElements *dx = (dxElements*)lua_getmethodtrans( L );
        dxElement *dxRoot = dx->dxRoot;

        if ( !dxRoot )
            return 0;

        dx->dxRoot->PushStack( L );
        return 1;
    }

    static LUA_DECLARE( getElements )
    {
        dxElements *dx = (dxElements*)lua_getmethodtrans( L );

        int n = 1;

        lua_createtable( L, dx->numElements, 0 );

        LIST_FOREACH_BEGIN( dxElement, dx->elements.root, envNode )
            item->PushStack( L );
            lua_rawseti( L, -2, n++ );
        LIST_FOREACH_END

        return 1;
    }

    const luaL_Reg _interface[] =
    {
        LUA_METHOD( setRenderLayer ),
        LUA_METHOD( createElement ),
        LUA_METHOD( getRoot ),
        LUA_METHOD( getElements ),
        { NULL, NULL }
    };

    static LUA_DECLARE( destroy )
    {
        delete (dxElements*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        return 0;
    }

    const luaL_Reg _interface_sys[] =
    {
        LUA_METHOD( destroy ),
        { NULL, NULL }
    };
}

void dxElement::MarkGC( lua_State *L )
{
    if ( markEnv && env )   env->Propagate( L );

    MarkGCCommon( L );
}

static LUA_DECLARE( getInterface )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElements *dx = element->env;
    
    if ( !dx )
        return 0;

    dx->PushStack( L );
    return 1;
}

static LUA_DECLARE( isDragging )
{
    lua_pushboolean( L, false );
    return 1;
}

static LUA_DECLARE( supportAlpha )
{
    bool support;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( support );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    if ( element->supportAlpha != support )
    {
        element->supportAlpha = support;
        element->Update( L );
    }

    LUA_SUCCESS;
}

static LUA_DECLARE( isSupportingAlpha )
{
    lua_pushboolean( L, ((dxElement*)lua_getmethodtrans( L ))->supportAlpha );
    return 1;
}

static LUA_DECLARE( setCaptiveMode )
{
    bool captive;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( captive );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    if ( element->captiveMode != captive )
    {
        element->captiveMode = captive;
        element->Update( L );
    }

    LUA_SUCCESS;
}

static LUA_DECLARE( isCaptiveMode )
{
    lua_pushboolean( L, ((dxElement*)lua_getmethodtrans( L ))->captiveMode );
    return 1;
}

static LUA_DECLARE( setOutbreakMode )
{
    bool outbreak;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( outbreak );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    if ( element->outbreakMode != outbreak )
    {
        element->outbreakMode = outbreak;
        element->Update( L );
    }
    
    LUA_SUCCESS;
}

static LUA_DECLARE( isOutbreakMode )
{
    lua_pushboolean( L, ((dxElement*)lua_getmethodtrans( L ))->outbreakMode );
    return 1;
}

static LUA_DECLARE( getRenderCapture )
{
    return 0;
}

static LUA_DECLARE( getScreenSize )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElement *parent = element->parent;
    dxElements *dx = element->env;

    if ( !parent || !dx )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    if ( element->outbreakMode )
        dx->dxRoot->PushMethod( L, "getScreenSize" );
    else if ( !element->captiveMode )
        parent->PushMethod( L, "getScreenSize" );
    else
        parent->PushMethod( L, "getSize" );

    lua_call( L, 0, 2 );
    return 2;
}

static LUA_DECLARE( setPosition )
{
    unsigned int x, y;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( x );
    argStream.ReadNumber( y );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    element->PushMethod( L, "triggerEvent" );
    lua_pushlstring( L, "onPosition", 10 );
    lua_pushnumber( L, x );
    lua_pushnumber( L, y );
    lua_call( L, 3, 1 );
    bool success = lua_toboolean( L, -1 );

    if ( !success )
        return 1;

    element->UpdateParent( L );
    element->pos = CVector2D( (float)x, (float)y );
    LUA_SUCCESS;
}

static LUA_DECLARE( getScreenPosition )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElement *parent = element->parent;

    if ( !parent )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    parent->PushMethod( L, "getScreenPosition" );
    lua_call( L, 0, 2 );

    lua_pushnumber( L, lua_tonumber( L, -2 ) + element->pos[0] );
    lua_pushnumber( L, lua_tonumber( L, -2 ) + element->pos[1] );
    return 2;
}

static LUA_DECLARE( getAbsolutePosition )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElement *parent = element->parent;

    if ( !parent )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    if ( element->outbreakMode )
    {
        element->PushMethod( L, "getScreenPosition" );
        lua_call( L, 0, 2 );
    }
    else if ( !element->captiveMode )
    {
        parent->PushMethod( L, "getAbsolutePosition" );
        lua_call( L, 0, 2 );

        lua_pushnumber( L, lua_tonumber( L, -2 ) + element->pos[0] );
        lua_pushnumber( L, lua_tonumber( L, -2 ) + element->pos[1] );
    }
    else
    {
        lua_pushnumber( L, element->pos[0] );
        lua_pushnumber( L, element->pos[1] );
    }
    
    return 2;
}

static LUA_DECLARE( getAbsoluteMousePosition )
{
    //todo
    return 0;
}

static LUA_DECLARE( getMousePosition )
{
    return 0;
}

static LUA_DECLARE( getPosition )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    lua_pushnumber( L, element->pos[0] );
    lua_pushnumber( L, element->pos[1] );
    return 2;
}

static LUA_DECLARE( setSize )
{
    unsigned int w, h;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( w );
    argStream.ReadNumber( h );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    LUA_CHECK( w != element->rect[0] || h != element->rect[1] );

    element->PushMethod( L, "triggerEvent" );
    lua_pushcstring( L, "onSize" );
    lua_pushnumber( L, w );
    lua_pushnumber( L, h );
    lua_call( L, 3, 1 );
    bool success = lua_toboolean( L, -1 );

    if ( !success )
        return 1;

    element->rect[0] = (float)w;
    element->rect[1] = (float)h;
    element->Update( L );
    LUA_SUCCESS;
}

static LUA_DECLARE( getSize )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    lua_pushnumber( L, element->rect[0] );
    lua_pushnumber( L, element->rect[1] );
    return 2;
}

static LUA_DECLARE( setWidth )
{
    unsigned int w;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( w );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    element->PushMethod( L, "setSize" );
    lua_pushvalue( L, 1 );
    lua_pushnumber( L, element->rect[1] );
    lua_call( L, 2, 1 );
    return 1;
}

static LUA_DECLARE( setHeight )
{
    unsigned int h;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( h );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    element->PushMethod( L, "setSize" );
    lua_pushnumber( L, element->rect[0] );
    lua_pushvalue( L, 1 );
    lua_call( L, 2, 1 );
    return 1;
}

static LUA_DECLARE( setBlendColor )
{
    RwColor color;

    LUA_ARGS_BEGIN;
    argStream.ReadColor( color.r );
    argStream.ReadColor( color.g );
    argStream.ReadColor( color.b );
    argStream.ReadColor( color.a );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    if ( element->blendColor != color )
    {
        element->blendColor = color;
        element->UpdateParent( L );
    }

    LUA_SUCCESS;
}

static LUA_DECLARE( getBlendColor )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    lua_pushnumber( L, element->blendColor.r );
    lua_pushnumber( L, element->blendColor.g );
    lua_pushnumber( L, element->blendColor.b );
    lua_pushnumber( L, element->blendColor.a );
    return 4;
}

static LUA_DECLARE( isInLocalArea )
{
    unsigned int x, y;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( x );
    argStream.ReadNumber( y );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    lua_pushboolean( L, x >= 0 && y >= 0 && x < element->rect[0] && y < element->rect[1] );
    return 1;
}

static LUA_DECLARE( isHit )
{
    lua_pushboolean( L, true );
    return 1;
}

static LUA_DECLARE( test )
{
    unsigned int x, y;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( x );
    argStream.ReadNumber( y );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    element->PushMethod( L, "isInLocalArea" );
    lua_pushnumber( L, x );
    lua_pushnumber( L, y );
    lua_call( L, 2, 1 );

    bool inLocalArea = lua_toboolean( L, -1 );
    lua_pop( L, 1 );

    LIST_FOREACH_BEGIN( dxElement, element->children.root, childNode )
        if ( item->visible )
        {
            if ( !item->captiveMode || inLocalArea )
            {
                item->PushMethod( L, "getPosition" );
                lua_call( L, 0, 2 );

                unsigned int dx, dy;
                dx = lua_tointeger( L, -2 );
                dy = lua_tointeger( L, -1 );

                lua_pop( L, 2 );

                item->PushMethod( L, "test" );
                lua_pushnumber( L, dx );
                lua_pushnumber( L, dy );
                lua_call( L, 2, 1 );
                
                bool successful = lua_toboolean( L, -1 );
                lua_pop( L, 1 );

                if ( successful )
                    return 1;
            }
        }
    LIST_FOREACH_END

    LUA_CHECK( inLocalArea );

    element->PushMethod( L, "isHit" );
    lua_pushnumber( L, x );
    lua_pushnumber( L, y );
    lua_call( L, 2, 1 );
    return 1;
}

static LUA_DECLARE( setVisible )
{
    bool visible;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( visible );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    LUA_CHECK( element->visible != visible );

    element->PushMethod( L, "triggerEvent" );
    lua_pushstring( L, visible ? "onShow" : "onHide" );
    lua_call( L, 0, 1 );
    LUA_CHECK( lua_toboolean( L, -1 ) );

    element->UpdateParent( L );

    if ( visible )
        element->PushMethod( L, "show" );
    else
        element->PushMethod( L, "hide" );

    lua_call( L, 0, 0 );

    LUA_SUCCESS;
}

static LUA_DECLARE( defereControls )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElements *dx = element->env;

    if ( !dx )
        return 0;

    if ( element == dx->mouseElement )
    {
        dx->mouseElement = NULL;

        element->PushMethod( L, "triggerEvent" );
        lua_pushlstring( L, "onMouseLeave", 12 );
        lua_call( L, 1, 0 );

        element->PushMethod( L, "mouseleave" );
        lua_call( L, 0, 0 );

        if ( !dx->mouseElement )
        {
            //todo: update mouse
        }
    }

    dx->DeleteActiveFocusFrom( L, element );

    return 0;
}

static LUA_DECLARE( show )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    LIST_FOREACH_BEGIN( dxElement, element->children.root, childNode )
        if ( !item->visible )
        {
            item->PushMethod( L, "show" );
            lua_call( L, 0, 0 );
        }
    LIST_FOREACH_END

    LUA_SUCCESS;
}

static LUA_DECLARE( hide )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    LIST_FOREACH_BEGIN( dxElement, element->children.root, childNode )
        if ( item->visible )
        {
            item->PushMethod( L, "hide" );
            lua_call( L, 0, 0 );
        }
    LIST_FOREACH_END

    element->PushMethod( L, "defereControls" );
    lua_call( L, 0, 0 );
    LUA_SUCCESS;
}

static LUA_DECLARE( isVisible )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    LUA_CHECK( element->visible || element->parent );

    element->parent->PushMethod( L, "isVisible" );
    lua_call( L, 0, 1 );

    lua_pushboolean( L, lua_toboolean( L, -1 ) );
    return 1;
}

static LUA_DECLARE( getVisible )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    lua_pushboolean( L, element->visible );
    return 1;
}

static LUA_DECLARE( setDisabled )
{
    bool mode;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( mode );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    if ( element->disabled != mode )
    {
        element->disabled = mode;

        element->PushMethod( L, mode ? "disable" : "enable" );
        lua_call( L, 0, 0 );
    }

    LUA_SUCCESS;
}

static LUA_DECLARE( enable )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    LIST_FOREACH_BEGIN( dxElement, element->children.root, childNode )
        if ( !item->disabled )
        {
            item->PushMethod( L, "enable" );
            lua_call( L, 0, 0 );
        }
    LIST_FOREACH_END

    LUA_SUCCESS;
}

static LUA_DECLARE( disable )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    LIST_FOREACH_BEGIN( dxElement, element->children.root, childNode )
        if ( !item->disabled )
        {
            item->PushMethod( L, "disable" );
            lua_call( L, 0, 0 );
        }
    LIST_FOREACH_END

    LUA_SUCCESS;
}

static LUA_DECLARE( isDisabled )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    if ( element->parent )
    {
        element->parent->PushMethod( L, "isDisabled" );
        lua_call( L, 0, 1 );

        if ( lua_toboolean( L, -1 ) )
            return 1;
    }

    lua_pushboolean( L, element->disabled );
    return 1;
}

static LUA_DECLARE( getDisabled )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    lua_pushboolean( L, element->disabled );
    return 1;
}

static LUA_DECLARE( createContext )
{
    return 0;
}

static LUA_DECLARE( executeEventHandlers )
{
    return 0;
}

static LUA_DECLARE( triggerEvent )
{
    return 0;
}

static LUA_DECLARE( setChild )
{
    return 0;
}

static LUA_DECLARE( moveToBack )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElements *dx = element->env;

    if ( !dx )
        return 0;

    dx->DeleteActiveFocusFrom( L, element );

    return 0;
}

static LUA_DECLARE( giveFocus )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElements *dx = element->env;

    if ( !dx )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    if ( dx->activeElement == element )
    {
        lua_pushboolean( L, true );
        return 1;
    }

    element->PushMethod( L, "triggerEvent" );
    lua_pushcstring( L, "onFocus" );
    lua_call( L, 0, 1 );
    
    if ( !lua_toboolean( L, -1 ) )
        return 1;

    if ( dx->activeElement )
        dx->UnsetActiveFocus( L );

    dx->activeElement = element;

    element->PushMethod( L, "focus" );
    lua_call( L, 0, 0 );

    lua_pushboolean( L, true );
    return 1;
}

static LUA_DECLARE( moveToFront )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    if ( !element->parent )
        return 0;

    element->parent->PushMethod( L, "moveToFront" );
    lua_call( L, 0, 0 );

    element->PushMethod( L, "giveFocus" );
    lua_call( L, 0, 0 );

    //todo

    return 0;
}

static LUA_DECLARE( setAlwaysOnTop )
{
    bool enabled;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( enabled );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    if ( element->alwaysOnTop == enabled )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    //todo

    element->alwaysOnTop = enabled;

    lua_pushboolean( L, true );
    return 1;
}

static LUA_DECLARE( isAlwaysOnTop )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    lua_pushboolean( L, element->alwaysOnTop );
    return 1;
}

static LUA_DECLARE( focus )
{
    //todo: input mode management

    return 0;
}

static LUA_DECLARE( blur )
{
    //todo: input mode management

    return 0;
}

static LUA_DECLARE( isActive )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElements *dx = element->env;

    lua_pushboolean( L, dx && element == dx->activeElement );
    return 1;
}

static LUA_DECLARE( isMouseActive )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElements *dx = element->env;

    lua_pushboolean( L, dx && element == dx->mouseElement );
    return 1;
}

static inline dxElement* element_testDrawOrder( lua_State *L, RwList <dxElement>& order, unsigned int offX, unsigned int offY )
{
    LIST_FOREACH_BEGIN( dxElement, order.root, drawOrderNode )
        item->PushMethod( L, "isVisible" );
        lua_call( L, 0, 1 );

        bool visible = lua_toboolean( L, -1 );
        lua_pop( L, 1 );

        if ( visible )
        {
            unsigned int ex, ey;

            item->PushMethod( L, "getPosition" );
            lua_call( L, 0, 2 );

            ex = lua_tointeger( L, -2 );
            ey = lua_tointeger( L, -1 );
            lua_pop( L, 2 );

            item->PushMethod( L, "test" );
            lua_pushnumber( L, offX - ex );
            lua_pushnumber( L, offY - ey );
            lua_call( L, 2, 1 );

            bool success = lua_toboolean( L, -1 );
            lua_pop( L, 1 );

            if ( success )
                return item;
        }
    LIST_FOREACH_END

    return NULL;
}

static LUA_DECLARE( getTopElementAtOffset )
{
    unsigned int offX, offY;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( offX );
    argStream.ReadNumber( offY );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    
    if ( dxElement *target = element_testDrawOrder( L, element->alwaysOnTopElements, offX, offY ) )
    {
        target->PushStack( L );
        return 1;
    }

    if ( dxElement *target = element_testDrawOrder( L, element->noncaptive, offX, offY ) )
    {
        target->PushStack( L );
        return 1;
    }

    lua_pushboolean( L, false );
    return 1;
}

static LUA_DECLARE( getElementAtOffset )
{
    unsigned int offX, offY;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( offX );
    argStream.ReadNumber( offY );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    element->PushMethod( L, "getTopElementAtOffset" );
    lua_pushnumber( L, offX );
    lua_pushnumber( L, offY );
    lua_call( L, 2, 1 );

    if ( lua_type( L, -1 ) == LUA_TCLASS )
        return 1;

    if ( dxElement *target = element_testDrawOrder( L, element->captAlwaysOnTop, offX, offY ) )
    {
        target->PushStack( L );
        return 1;
    }

    if ( dxElement *target = element_testDrawOrder( L, element->drawingOrder, offX, offY ) )
    {
        target->PushStack( L );
        return 1;
    }

    lua_pushboolean( L, false );
    return 1;
}

static LUA_DECLARE( handleMouseClick )
{
    lua_String button;
    unsigned int offX, offY;

    LUA_ARGS_BEGIN;
    argStream.ReadString( button );
    argStream.ReadNumber( offX );
    argStream.ReadNumber( offY );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElements *dx = element->env;

    LUA_CHECK( dx );

    element->PushMethod( L, "getElementAtOffset" );
    lua_pushnumber( L, offX );
    lua_pushnumber( L, offY );
    lua_call( L, 2, 1 );

    dxElement *mouse = lua_readclass <dxElement> ( L, -1, LUACLASS_DXELEMENT );

    if ( mouse )
    {
        mouse->PushMethod( L, "getPosition" );
        lua_call( L, 0, 2 );

        unsigned int x = (unsigned int)lua_tonumber( L, -2 );
        unsigned int y = (unsigned int)lua_tonumber( L, -1 );

        mouse->PushMethod( L, "handleMouseClick" );
        lua_pushnumber( L, offX - x );
        lua_pushnumber( L, offY - y );
        lua_call( L, 2, 1 );

        bool success = lua_toboolean( L, -1 );

        if ( success )
            return 1;
    }

    element->PushMethod( L, "triggerEvent" );
    int triggerFunc = lua_gettop( L );

    unsigned int now = GetTickCount();

    if ( element != dx->activeElement )
    {
        element->PushMethod( L, "moveToFront" );
        lua_call( L, 0, 0 );
    }
    else if ( dx->lastMouseButton == button && now - dx->mouseClickTime < 200 )
    {
        lua_pushcstring( L, "onDoubleClick" );
        lua_pushstring( L, button );
        lua_pushnumber( L, offX );
        lua_pushnumber( L, offY );

        lua_pushvalue( L, triggerFunc );
        lua_pushvalue( L, -5 );
        lua_pushvalue( L, -5 );
        lua_pushvalue( L, -5 );
        lua_pushvalue( L, -5 );
        lua_call( L, 4, 1 );

        bool success = lua_toboolean( L, -1 );
        
        if ( success )
        {
            lua_pushvalue( L, triggerFunc );
            lua_pushvalue( L, -6 );
            lua_pushvalue( L, -6 );
            lua_pushvalue( L, -6 );
            lua_call( L, 3, 0 );
        }

        lua_settop( L, triggerFunc );
    }

    dx->lastMouseButton = button;
    dx->mouseClickTime = now;

    lua_pushvalue( L, triggerFunc );
    lua_pushcstring( L, "onClick" );
    lua_pushstring( L, button );
    lua_pushboolean( L, true );
    lua_pushnumber( L, offX );
    lua_pushnumber( L, offY );
    lua_call( L, 5, 1 );

    bool success = lua_toboolean( L, -1 );

    if ( success )
    {
        element->PushMethod( L, "mouseclick" );
        lua_pushstring( L, button );
        lua_pushboolean( L, true );
        lua_pushnumber( L, offX );
        lua_pushnumber( L, offY );
        lua_call( L, 4, 0 );
        return 1;
    }

    lua_pushboolean( L, true );
    return 1;
}

static LUA_DECLARE( handleMouseMove )
{
    unsigned int offX, offY;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( offX );
    argStream.ReadNumber( offY );
    LUA_ARGS_END;

    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElements *dx = element->env;

    LUA_CHECK( dx );

    element->PushMethod( L, "getElementAtOffset" );
    lua_pushnumber( L, offX );
    lua_pushnumber( L, offY );
    lua_call( L, 2, 1 );

    dxElement *mouse = lua_readclass <dxElement> ( L, -1, LUACLASS_DXELEMENT );

    if ( !mouse )
    {
        element->PushMethod( L, "triggerEvent" );
        int triggerFunc = lua_gettop( L );

        if ( mouse != element )
        {
            if ( !dx->AttemptRemoveMouseFocus( L ) )
            {
                lua_pushboolean( L, false );
                return 1;
            }

            dx->mouseElement = element;

            lua_pushvalue( L, triggerFunc );
            lua_pushcstring( L, "onMouseEnter" );
            lua_call( L, 0, 1 );

            bool success = lua_toboolean( L, -1 );
            
            if ( !success )
                return 1;

            element->PushMethod( L, "mouseenter" );
            lua_call( L, 0, 0 );
        }

        lua_pushvalue( L, triggerFunc );
        lua_pushcstring( L, "onMouseMove" );
        lua_pushnumber( L, offX );
        lua_pushnumber( L, offY );
        lua_call( L, 3, 1 );

        bool success = lua_toboolean( L, -1 );
        
        if ( success )
        {
            element->PushMethod( L, "mousemove" );
            lua_call( L, 0, 0 );
        }
        return 1;
    }

    mouse->PushMethod( L, "getPosition" );
    lua_call( L, 0, 2 );

    unsigned int x = (unsigned int)lua_tonumber( L, -2 );
    unsigned int y = (unsigned int)lua_tonumber( L, -1 );
    
    mouse->PushMethod( L, "handleMouseMove" );
    lua_pushnumber( L, offX - x );
    lua_pushnumber( L, offY - y );
    lua_call( L, 2, 1 );
    return 1;
}

// Finalizer stubs
static LUA_DECLARE( mouseclick )
{
    return 0;
}

static LUA_DECLARE( mousedoubleclick )
{
    return 0;
}

static LUA_DECLARE( mouseenter )
{
    return 0;
}

static LUA_DECLARE( mousemove )
{
    return 0;
}

static LUA_DECLARE( mouseleave )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElements *dx = element->env;

    if ( !dx )
        return 0;

    dx->lastMouseButton = NULL;
    return 0;
}

static LUA_DECLARE( acceptInput )
{
    lua_pushboolean( L, false );
    return 1;
}

static LUA_DECLARE( keyInput )
{
    return 0;
}

static LUA_DECLARE( input )
{
    return 0;
}

static LUA_DECLARE( update )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );
    dxElement *parent = element->parent;
    
    if ( !parent )
        return 0;

    parent->Update( L );

    element->changed = true;
    LUA_SUCCESS;
}

static LUA_DECLARE( getRenderTarget )
{
    //todo
    return 0;
}

static LUA_DECLARE( destroyRenderTarget )
{
    //todo
    return 0;
}

static LUA_DECLARE( ready )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    element->PushMethod( L, "isVisible" );
    lua_call( L, 0, 1 );

    bool visible = lua_toboolean( L, -1 );

    if ( !visible )
        return 1;

    //todo
    return 0;
}

static LUA_DECLARE( preRender )
{
    //todo
    return 0;
}

static LUA_DECLARE( render )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    element->changed = false;

    //todo

    element->PushMethod( L, "triggerEvent" );
    lua_pushcstring( L, "onRender" );
    lua_call( L, 1, 0 );
    return 0;
}

static LUA_DECLARE( renderTop )
{
    //todo
    return 0;
}

static LUA_DECLARE( present )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    element->PushMethod( L, "getAbsolutePosition" );
    lua_call( L, 0, 2 );

    unsigned int x = (unsigned int)lua_tonumber( L, -2 );
    unsigned int y = (unsigned int)lua_tonumber( L, -1 );

    //todo

    element->PushMethod( L, "triggerEvent" );
    lua_pushcstring( L, "onPresent" );
    lua_pushboolean( L, false );
    lua_pushnumber( L, x );
    lua_pushnumber( L, y );
    lua_call( L, 4, 0 );
    return 0;
}

static LUA_DECLARE( resetRenderTarget )
{
    //todo
    return 0;
}

static LUA_DECLARE( invalidate )
{
    return 0;
}

static LUA_DECLARE( destroy )
{
    dxElement *element = (dxElement*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    //todo

    element->PushMethod( L, "triggerEvent" );
    lua_pushcstring( L, "onDestruction" );
    lua_call( L, 1, 0 );

    element->PushMethod( L, "destroyRenderTarget" );
    lua_call( L, 0, 0 );

    element->PushMethod( L, "defereControls" );
    lua_call( L, 0, 0 );

    delete element;

    return 0;
}

static const luaL_Reg dxElement_interface[] =
{
    LUA_METHOD( getInterface ),
    LUA_METHOD( isDragging ),
    LUA_METHOD( supportAlpha ),
    LUA_METHOD( isSupportingAlpha ),
    LUA_METHOD( setCaptiveMode ),
    LUA_METHOD( isCaptiveMode ),
    LUA_METHOD( setOutbreakMode ),
    LUA_METHOD( isOutbreakMode ),
    LUA_METHOD( getRenderCapture ),
    LUA_METHOD( getScreenSize ),
    LUA_METHOD( setPosition ),
    LUA_METHOD( getScreenPosition ),
    LUA_METHOD( getAbsolutePosition ),
    LUA_METHOD( getAbsoluteMousePosition ),
    LUA_METHOD( getMousePosition ),
    LUA_METHOD( setSize ),
    LUA_METHOD( getSize ),
    LUA_METHOD( setWidth ),
    LUA_METHOD( setHeight ),
    LUA_METHOD( setBlendColor ),
    LUA_METHOD( getBlendColor ),
    LUA_METHOD( isInLocalArea ),
    LUA_METHOD( isHit ),
    LUA_METHOD( test ),
    LUA_METHOD( setVisible ),
    LUA_METHOD( defereControls ),
    LUA_METHOD( show ),
    LUA_METHOD( hide ),
    LUA_METHOD( isVisible ),
    LUA_METHOD( getVisible ),
    LUA_METHOD( setDisabled ),
    LUA_METHOD( enable ),
    LUA_METHOD( disable ),
    LUA_METHOD( isDisabled ),
    LUA_METHOD( getDisabled ),
    LUA_METHOD( createContext ),
    LUA_METHOD( executeEventHandlers ),
    LUA_METHOD( triggerEvent ),
    LUA_METHOD( setChild ),
    LUA_METHOD( moveToBack ),
    LUA_METHOD( giveFocus ),
    LUA_METHOD( moveToFront ),
    LUA_METHOD( setAlwaysOnTop ),
    LUA_METHOD( isAlwaysOnTop ),
    LUA_METHOD( focus ),
    LUA_METHOD( blur ),
    LUA_METHOD( isActive ),
    LUA_METHOD( isMouseActive ),
    LUA_METHOD( getTopElementAtOffset ),
    LUA_METHOD( getElementAtOffset ),
    LUA_METHOD( handleMouseClick ),
    LUA_METHOD( handleMouseMove ),
    LUA_METHOD( mouseclick ),
    LUA_METHOD( mousedoubleclick ),
    LUA_METHOD( mouseenter ),
    LUA_METHOD( mousemove ),
    LUA_METHOD( mouseleave ),
    LUA_METHOD( acceptInput ),
    LUA_METHOD( keyInput ),
    LUA_METHOD( input ),
    LUA_METHOD( update ),
    LUA_METHOD( getRenderTarget ),
    LUA_METHOD( destroyRenderTarget ),
    LUA_METHOD( ready ),
    LUA_METHOD( preRender ),
    LUA_METHOD( render ),
    LUA_METHOD( renderTop ),
    LUA_METHOD( present ),
    LUA_METHOD( resetRenderTarget ),
    LUA_METHOD( invalidate ),
    { NULL, NULL }
};

static const luaL_Reg dxElement_interface_sys[] =
{
    LUA_METHOD( destroy ),
    { NULL, NULL }
};

static LUA_DECLARE( dxElement_constructor )
{
    dxElement *element = (dxElement*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, LUA_ENVIRONINDEX );
    j.SetTransmit( LUACLASS_DXELEMENT, element );

    j.RegisterInterfaceTrans( L, dxElement_interface, 0, LUACLASS_DXELEMENT );

    lua_pushlightuserdata( L, element );
    j.RegisterInterface( L, dxElement_interface_sys, 1 );

    // Include the events interface.
    luaevent_extend( L );

    lua_pushlstring( L, "dxElement", 9 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

static ILuaClass* _trefget( lua_State *L, dxElement *element )
{
    lua_pushlightuserdata( L, element );
    lua_pushcclosure( L, dxElement_constructor, 1 );
    lua_newclass( L );
    
    ILuaClass *j = lua_refclass( L, -1 );
    lua_pop( L, 1 );
    return j;
}

dxElement::dxElement( lua_State *L, dxElements *dx ) : LuaClass( L, _trefget( L, this ) )
{
    env = dx;
    keepAlive = true;
    markEnv = true;

    LIST_INSERT( dx->elements.root, envNode );
    dx->numElements++;

    updateTarget = false;
    alwaysOnTop = false;
    changed = false;
    previousGUIState = NULL;
    captiveMode = false;
    outbreakMode = false;
    supportAlpha = false;
    disabled = false;
    blendColor = 0xFFFFFFFF;
    childAPI = NULL;
    parent = NULL;

    LIST_CLEAR( drawingOrder.root );
    LIST_CLEAR( captAlwaysOnTop.root );
    LIST_CLEAR( noncaptive.root );
    LIST_CLEAR( alwaysOnTopElements.root );

    LIST_CLEAR( children.root );
}

dxElement::~dxElement( void )
{
    Unlink();
}

void dxElement::Unlink( void )
{
    if ( dxElements *dx = env )
    {
        dx->numElements--;

        env = NULL;
        LIST_REMOVE( envNode );
    }
}

static LUA_DECLARE( createInterface )
{
    dxElements *dx = new dxElements( L );

    dx->PushStack( L );
    dx->DisableKeepAlive();
    return 1;
}

const static luaL_Reg dxElements_library[] =
{
    LUA_METHOD( createInterface ),
    { NULL, NULL }
};

void luaopen_dxElements( lua_State *L )
{
    luaL_openlib( L, "dx", dxElements_library, 0 );
    lua_pop( L, 1 );
}