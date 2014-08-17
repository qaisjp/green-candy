/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        dxElements/lua_dxElements_master.h
*  PURPOSE:     Lua dxElements GUI development environment master include
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_DXELEMENTS_MASTER_INCLUDE_
#define _LUA_DXELEMENTS_MASTER_INCLUDE_

namespace _dxElements
{
    const extern luaL_Reg _interface[];
    const extern luaL_Reg _interface_sys[];
}

struct dxElements : public LuaClass
{
    static LUA_DECLARE( constructor )
    {
        dxElements *dx = (dxElements*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_DXELEMENTS_SYS, dx );

        j->RegisterInterfaceTrans( L, _dxElements::_interface, 0, LUACLASS_DXELEMENTS_SYS );

        lua_pushvalue( L, lua_upvalueindex( 1 ) );
        j->RegisterInterface( L, _dxElements::_interface_sys, 1 );

        lua_pushcstring( L, "dxElements" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }

    static ILuaClass* _trefget( lua_State *L, dxElements *dx )
    {
        lua_pushlightuserdata( L, dx );
        lua_pushcclosure( L, constructor, 1 );
        lua_newclassex( L, LCLASS_API_LIGHT );

        ILuaClass *j = lua_refclass( L, -1 );
        lua_pop( L, 1 );
        return j;
    }

    static LUA_DECLARE( dxRoot_destroy )
    {
        dxElement *element = (dxElement*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        element->env->dxRoot = NULL;
        return 0;
    }

    dxElements( lua_State *L ) : LuaClass( L, _trefget( L, this ) )
    {
        LIST_CLEAR( elements.root );
        numElements = 0;

        dxRoot = new dxElement( L, this );
        lua_dxElement_extendRoot( L, dxRoot );

        lua_pushlightuserdata( L, dxRoot );
        dxRoot->GetClass()->RegisterMethod( L, "destroy", dxRoot_destroy, 1, true );

        mouseElement = NULL;
        activeElement = NULL;

        lastMouseButton = NULL;
        mouseClickTime = 0;

        keepAlive = true;
    }

    ~dxElements( void )
    {
        // Destroy all elements which belong to this manager
        while ( !LIST_EMPTY( elements.root ) )
        {
            dxElement *item = LIST_GETITEM( dxElement, elements.root.next, envNode );
            ILuaClass *j = item->GetClass();

            //todo: reliable way (without side-effects) to prevent garbage collection!

            item->Destroy();

            if ( !j->IsDestroyed() )
            {
                // We are not destroyed here. Unlink us to prevent crashes.
                item->Unlink();
            }
        }
    }

    void UnsetActiveFocus( lua_State *L )
    {
        dxElement *element = activeElement;

        activeElement = NULL;

        element->PushMethod( L, "triggerEvent" );
        lua_pushlstring( L, "onBlur", 6 );
        lua_call( L, 0, 0 );

        element->PushMethod( L, "blur" );
        lua_call( L, 0, 0 );
    }

    void DeleteActiveFocusFrom( lua_State *L, dxElement *element )
    {
        if ( element == activeElement )
            UnsetActiveFocus( L );
    }

    void UnsetMouseFocus( lua_State *L )
    {
        dxElement *element = mouseElement;

        mouseElement = NULL;

        element->PushMethod( L, "mouseleave" );
        lua_call( L, 0, 0 );
    }

    bool AttemptRemoveMouseFocus( lua_State *L )
    {
        if ( mouseElement )
        {
            dxElement *element = mouseElement;
            
            element->PushMethod( L, "triggerEvent" );
            lua_pushcstring( L, "onMouseLeave" );
            lua_call( L, 0, 1 );

            bool success = ( lua_toboolean( L, -1 ) != 0 );
            lua_pop( L, 1 );

            if ( success )
            {
                UnsetMouseFocus( L );
                return true;
            }
        }

        return false;
    }

    dxElement *dxRoot;
    dxElement *mouseElement;
    dxElement *activeElement;

    unsigned int numElements;
    RwList <dxElement> elements;

    lua_String lastMouseButton;
    unsigned int mouseClickTime;

    inline void DisableKeepAlive( void )
    {
        keepAlive = false;
    }

    void MarkGC( lua_State *L )
    {
        dxRoot->MarkGC( L );

        if ( mouseElement )     mouseElement->MarkGC( L );
        if ( activeElement )    mouseElement->MarkGC( L );

        if ( keepAlive )
        {
            m_class->Propagate( L );
            lua_gcpaycost( L, sizeof(*this) );
        }
    }

    bool keepAlive;
};

#endif //_LUA_DXELEMENTS_MASTER_INCLUDE_