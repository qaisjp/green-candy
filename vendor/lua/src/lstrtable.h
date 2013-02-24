/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        vendor/lua/src/lstrtable.h
*  PURPOSE:     Lua fast string table
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_STRINGTABLE_
#define _LUA_STRINGTABLE_

template <class type>
struct STableItem
{
    RwListEntry <STableItem <type>> node;
    TString *key;
    type val;
};

template <class type>
class StringTable
{
public:
    typedef STableItem <type> item_t;

    StringTable()
    {
        LIST_CLEAR( m_list.root );
    }

    ~StringTable()
    {
        while ( !LIST_EMPTY( m_list.root ) )
        {
            item_t *item = LIST_GETITEM( item_t, m_list.root.next, node );
            LIST_REMOVE( item->node );

            luaM_free( L, item );
        }
    }

    void* operator new( size_t size, lua_State *main )
    {
        StringTable *tab = (StringTable <type>*)luaM_malloc( main, sizeof(StringTable <type>) );
        tab->L = lua_getmainstate( main );
        return tab;
    }

    void operator delete( void *ptr )
    {
        StringTable *tab = (StringTable*)ptr;
        luaM_free( tab->L, tab );
    }

    void operator delete( void *ptr, lua_State *main )
    {
        __asm int 3
    }

    void    SetItem( lua_State *L, TString *key, type val )
    {
        item_t *item = FindNode( key );

        if ( !item )
        {
            item = luaM_new( L, item_t );
            item->key = key;
            LIST_INSERT( m_list.root, item->node );
        }

        item->val = val;
    }

    type    GetItem( const TString *key )
    {
        item_t *item = FindNode( key );

        if ( !item )
            return NULL;

        return item->val;
    }

    void    TraverseGC( global_State *g );

private:
    item_t* FindNode( const TString *key )
    {
        LIST_FOREACH_BEGIN( item_t, m_list.root, node )
            if ( item->key == key )
                return item;
        LIST_FOREACH_END

        return NULL;
    }

    lua_State *L; // main Lua state
    RwList <item_t> m_list;
};

#endif //_LUA_STRINGTABLE_