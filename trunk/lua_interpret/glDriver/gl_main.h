/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_main.h
*  PURPOSE:     OpenGL driver for MTA:Lua
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _OPENGL_MAIN_
#define _OPENGL_MAIN_

#include "gl_lua.h"

// We avoid exporting OpenGL headers as global headers.
// OpenGL internal information should stay inside of driver files.

#define LUACLASS_GLDRIVER   41

struct glContextDescriptor;
struct glFrameBufferBase;
struct glProgramARB;

struct glDriver : public LuaClass
{
    glDriver( lua_State *L, Win32Dialog *wnd );
    ~glDriver( void );

    Win32Dialog *m_window;
    HDC deviceContext;
    HGLRC glContext;

    RwListEntry <glDriver> rootNode;

    bool isBatching;

    unsigned int vMajor, vMinor;

    // Main properties
    bool allowTexturesOfArbitrarySize;
    bool supportsDXT1;
    bool supportsDXT3;
    bool supportsDXT5;
    bool supportsFXT1;
    bool supports_ARB_compression;
    bool supports_ARB_BPTC;
    bool supportsLATC;
    bool supportsRGTC;
    bool supportsS3TC;
    bool supportsASTC;
    bool supports_srgb_compression;
    int maxTextureSize;
    bool supports1_3;
    bool supports_FBO_EXT;
    bool supportsFBO;
    bool supports2_0;
    bool supports3_0;
    bool supports_ARB_imaging;
    bool supports_ARB_vertex_shader;
    bool supports_ARB_shader_objects;
    bool supports_ARB_vertex_program;
    bool supports_ARB_fragment_shader;

    int maxFBOColorAttachments;
    int maxFBOColorAttachmentsEXT;

    // Context functions
    // According to MSDN, extension functions should be used in a context only.
    struct glContextDescriptor *contextInfo;

    // Object context stacks
    glFrameBufferBase *fboDrawStack;
    glFrameBufferBase *fboReadStack;

    // Single context acquisitions
    // The object context stack overrides a single context acquisition.
    // If the stack is active, an acquisition cannot be made.
    glProgramARB *currentProgramARB;
    //TODO: add core version
    bool isProgramStackActive;  // if not NULL, do not allow calling glDriver.useProgram()
};

void luagl_initDrivers( void );
void luagl_shutdownDrivers( void );
void luagl_pulseDrivers( lua_State *L );

// Global OpenGL exports
extern HMODULE glMainLibrary;
extern HMODULE gluLibrary;

#endif //_OPENGL_MAIN_