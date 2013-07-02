/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/Bitmap.h
*  PURPOSE:     MTA:Lua bitmap instance for image data traversal
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_BITMAP_
#define _LUA_BITMAP_

#define LUACLASS_BITMAP     48

struct s3tc_dxt1_block
{
    s3tc_dxt1_block( void )
    {
        *( (unsigned int*)this ) = 0;
        *( (unsigned int*)this + 1 ) = 0;
    }

    struct packed_pixel
    {
        unsigned short red : 5;
        unsigned short green : 6;
        unsigned short blue : 5;
    };

    packed_pixel color0;
    packed_pixel color1;
    unsigned int bits;
};

struct s3tc_dxt3_block : public s3tc_dxt1_block
{
    s3tc_dxt3_block( void )
    {
        *( (unsigned int*)this + 2 ) = 0;
        *( (unsigned int*)this + 3 ) = 0;
    }

    unsigned char alphaData[8];
};

typedef s3tc_dxt3_block s3tc_dxt5_block;

struct BitmapRGB
{
    BitmapRGB( void ) : r( 0 ), g( 0 ), b( 0 )
    { }

    char r, g, b;
};

struct Bitmap : public LuaInstance
{
    enum eDataType : unsigned int
    {
        BITMAP_RGB,
        BITMAP_RGBA,
        BITMAP_DXT1_RGB,
        BITMAP_DXT1_RGBA,
        BITMAP_DXT3_RGBA,
        BITMAP_DXT5_RGBA
    };

    static const luaL_Reg _interface[];

    static LUA_DECLARE( constructor )
    {
        Bitmap *map = (Bitmap*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_BITMAP, map );

        j->RegisterInterfaceTrans( L, _interface, 0, LUACLASS_BITMAP );

        lua_pushcstring( L, "bitmap" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }

    Bitmap( lua_State *L, eDataType dataType, void *data, size_t dataSize, unsigned int width, unsigned int height ) : LuaInstance( L )
    {
        Extend( L, constructor );

        m_dataType = dataType;
        m_data = data;
        m_dataSize = dataSize;
        m_width = width;
        m_height = height;
    }

    ~Bitmap( void )
    {
    }

    const void* GetData( void ) const
    {
        return m_data;
    }

    eDataType GetDataType( void ) const
    {
        return m_dataType;
    }

    unsigned int GetWidth( void ) const
    {
        return m_width;
    }

    unsigned int GetHeight( void ) const
    {
        return m_height;
    }

    eDataType       m_dataType;
    void*           m_data;
    size_t          m_dataSize;
    unsigned int    m_width;
    unsigned int    m_height;
};

struct BitmapCopy : public Bitmap
{
    BitmapCopy( lua_State *L, eDataType dataType, void *data, size_t dataSize, unsigned int width, unsigned int height ) : Bitmap( L, dataType, data, dataSize, width, height )
    {
    }

    ~BitmapCopy( void )
    {
        delete m_data;
    }
};

void luabitmap_open( lua_State *L );

#endif //_LUA_BITMAP_