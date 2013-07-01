/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_texture.h
*  PURPOSE:     OpenGL driver texture management
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_OPENGL_TEXTURE_
#define _LUA_OPENGL_TEXTURE_

#define LUACLASS_GLTEXTURE  51

struct glTexture : public LuaInstance
{
    glTexture( lua_State *L, glDriver *gl, GLuint texIndex, unsigned int width, unsigned int height );
    ~glTexture( void );

    glDriver*       m_driver;
    GLuint          m_texIndex;
    unsigned int    m_width;
    unsigned int    m_height;

    RwListEntry <glTexture> textureNode;
};

extern const luaL_Reg tex_driver_interface[];

#endif //_LUA_OPENGL_TEXTURE_