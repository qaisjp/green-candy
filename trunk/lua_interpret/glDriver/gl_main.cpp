/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_main.cpp
*  PURPOSE:     OpenGL driver for MTA:Lua
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <gl.h>
#include "gl_func.h"

// Prototype links to main library
wglCopyContext_t            _wglCopyContext = NULL;
wglCreateContext_t          _wglCreateContext = NULL;
wglCreateLayerContext_t     _wglCreateLayerContext = NULL;
wglDeleteContext_t          _wglDeleteContext = NULL;
wglGetCurrentContext_t      _wglGetCurrentContext = NULL;
wglGetCurrentDC_t           _wglGetCurrentDC = NULL;
wglGetProcAddress_t         _wglGetProcAddress = NULL;
wglMakeCurrent_t            _wglMakeCurrent = NULL;
wglShareLists_t             _wglShareLists = NULL;
wglUseFontBitmapsA_t        _wglUseFontBitmapsA = NULL;
wglUseFontBitmapsW_t        _wglUseFontBitmapsW = NULL;

// OpenGL standard functions (machine generated, do not touch; use gltools.lua)
glAccum_t                           _glAccum = NULL;
glAlphaFunc_t                       _glAlphaFunc = NULL;
glAreTexturesResident_t             _glAreTexturesResident = NULL;
glArrayElement_t                    _glArrayElement = NULL;
glBegin_t                           _glBegin = NULL;
glBindTexture_t                     _glBindTexture = NULL;
glBitmap_t                          _glBitmap = NULL;
glBlendFunc_t                       _glBlendFunc = NULL;
glCallList_t                        _glCallList = NULL;
glCallLists_t                       _glCallLists = NULL;
glClear_t                           _glClear = NULL;
glClearAccum_t                      _glClearAccum = NULL;
glClearColor_t                      _glClearColor = NULL;
glClearDepth_t                      _glClearDepth = NULL;
glClearIndex_t                      _glClearIndex = NULL;
glClearStencil_t                    _glClearStencil = NULL;
glClipPlane_t                       _glClipPlane = NULL;
glColor3b_t                         _glColor3b = NULL;
glColor3bv_t                        _glColor3bv = NULL;
glColor3d_t                         _glColor3d = NULL;
glColor3dv_t                        _glColor3dv = NULL;
glColor3f_t                         _glColor3f = NULL;
glColor3fv_t                        _glColor3fv = NULL;
glColor3i_t                         _glColor3i = NULL;
glColor3iv_t                        _glColor3iv = NULL;
glColor3s_t                         _glColor3s = NULL;
glColor3sv_t                        _glColor3sv = NULL;
glColor3ub_t                        _glColor3ub = NULL;
glColor3ubv_t                       _glColor3ubv = NULL;
glColor3ui_t                        _glColor3ui = NULL;
glColor3uiv_t                       _glColor3uiv = NULL;
glColor3us_t                        _glColor3us = NULL;
glColor3usv_t                       _glColor3usv = NULL;
glColor4b_t                         _glColor4b = NULL;
glColor4bv_t                        _glColor4bv = NULL;
glColor4d_t                         _glColor4d = NULL;
glColor4dv_t                        _glColor4dv = NULL;
glColor4f_t                         _glColor4f = NULL;
glColor4fv_t                        _glColor4fv = NULL;
glColor4i_t                         _glColor4i = NULL;
glColor4iv_t                        _glColor4iv = NULL;
glColor4s_t                         _glColor4s = NULL;
glColor4sv_t                        _glColor4sv = NULL;
glColor4ub_t                        _glColor4ub = NULL;
glColor4ubv_t                       _glColor4ubv = NULL;
glColor4ui_t                        _glColor4ui = NULL;
glColor4uiv_t                       _glColor4uiv = NULL;
glColor4us_t                        _glColor4us = NULL;
glColor4usv_t                       _glColor4usv = NULL;
glColorMask_t                       _glColorMask = NULL;
glColorMaterial_t                   _glColorMaterial = NULL;
glColorPointer_t                    _glColorPointer = NULL;
glCopyPixels_t                      _glCopyPixels = NULL;
glCopyTexImage1D_t                  _glCopyTexImage1D = NULL;
glCopyTexImage2D_t                  _glCopyTexImage2D = NULL;
glCopyTexSubImage1D_t               _glCopyTexSubImage1D = NULL;
glCopyTexSubImage2D_t               _glCopyTexSubImage2D = NULL;
glCullFace_t                        _glCullFace = NULL;
glDeleteLists_t                     _glDeleteLists = NULL;
glDeleteTextures_t                  _glDeleteTextures = NULL;
glDepthFunc_t                       _glDepthFunc = NULL;
glDepthMask_t                       _glDepthMask = NULL;
glDepthRange_t                      _glDepthRange = NULL;
glDisable_t                         _glDisable = NULL;
glDisableClientState_t              _glDisableClientState = NULL;
glDrawArrays_t                      _glDrawArrays = NULL;
glDrawBuffer_t                      _glDrawBuffer = NULL;
glDrawElements_t                    _glDrawElements = NULL;
glDrawPixels_t                      _glDrawPixels = NULL;
glEdgeFlag_t                        _glEdgeFlag = NULL;
glEdgeFlagPointer_t                 _glEdgeFlagPointer = NULL;
glEdgeFlagv_t                       _glEdgeFlagv = NULL;
glEnable_t                          _glEnable = NULL;
glEnableClientState_t               _glEnableClientState = NULL;
glEnd_t                             _glEnd = NULL;
glEndList_t                         _glEndList = NULL;
glEvalCoord1d_t                     _glEvalCoord1d = NULL;
glEvalCoord1dv_t                    _glEvalCoord1dv = NULL;
glEvalCoord1f_t                     _glEvalCoord1f = NULL;
glEvalCoord1fv_t                    _glEvalCoord1fv = NULL;
glEvalCoord2d_t                     _glEvalCoord2d = NULL;
glEvalCoord2dv_t                    _glEvalCoord2dv = NULL;
glEvalCoord2f_t                     _glEvalCoord2f = NULL;
glEvalCoord2fv_t                    _glEvalCoord2fv = NULL;
glEvalMesh1_t                       _glEvalMesh1 = NULL;
glEvalMesh2_t                       _glEvalMesh2 = NULL;
glEvalPoint1_t                      _glEvalPoint1 = NULL;
glEvalPoint2_t                      _glEvalPoint2 = NULL;
glFeedbackBuffer_t                  _glFeedbackBuffer = NULL;
glFinish_t                          _glFinish = NULL;
glFlush_t                           _glFlush = NULL;
glFogf_t                            _glFogf = NULL;
glFogfv_t                           _glFogfv = NULL;
glFogi_t                            _glFogi = NULL;
glFogiv_t                           _glFogiv = NULL;
glFrontFace_t                       _glFrontFace = NULL;
glFrustum_t                         _glFrustum = NULL;
glGenLists_t                        _glGenLists = NULL;
glGenTextures_t                     _glGenTextures = NULL;
glGetBooleanv_t                     _glGetBooleanv = NULL;
glGetClipPlane_t                    _glGetClipPlane = NULL;
glGetDoublev_t                      _glGetDoublev = NULL;
glGetError_t                        _glGetError = NULL;
glGetFloatv_t                       _glGetFloatv = NULL;
glGetIntegerv_t                     _glGetIntegerv = NULL;
glGetLightfv_t                      _glGetLightfv = NULL;
glGetLightiv_t                      _glGetLightiv = NULL;
glGetMapdv_t                        _glGetMapdv = NULL;
glGetMapfv_t                        _glGetMapfv = NULL;
glGetMapiv_t                        _glGetMapiv = NULL;
glGetMaterialfv_t                   _glGetMaterialfv = NULL;
glGetMaterialiv_t                   _glGetMaterialiv = NULL;
glGetPixelMapfv_t                   _glGetPixelMapfv = NULL;
glGetPixelMapuiv_t                  _glGetPixelMapuiv = NULL;
glGetPixelMapusv_t                  _glGetPixelMapusv = NULL;
glGetPointerv_t                     _glGetPointerv = NULL;
glGetPolygonStipple_t               _glGetPolygonStipple = NULL;
glGetString_t                       _glGetString = NULL;
glGetTexEnvfv_t                     _glGetTexEnvfv = NULL;
glGetTexEnviv_t                     _glGetTexEnviv = NULL;
glGetTexGendv_t                     _glGetTexGendv = NULL;
glGetTexGenfv_t                     _glGetTexGenfv = NULL;
glGetTexGeniv_t                     _glGetTexGeniv = NULL;
glGetTexImage_t                     _glGetTexImage = NULL;
glGetTexLevelParameterfv_t          _glGetTexLevelParameterfv = NULL;
glGetTexLevelParameteriv_t          _glGetTexLevelParameteriv = NULL;
glGetTexParameterfv_t               _glGetTexParameterfv = NULL;
glGetTexParameteriv_t               _glGetTexParameteriv = NULL;
glHint_t                            _glHint = NULL;
glIndexMask_t                       _glIndexMask = NULL;
glIndexPointer_t                    _glIndexPointer = NULL;
glIndexd_t                          _glIndexd = NULL;
glIndexdv_t                         _glIndexdv = NULL;
glIndexf_t                          _glIndexf = NULL;
glIndexfv_t                         _glIndexfv = NULL;
glIndexi_t                          _glIndexi = NULL;
glIndexiv_t                         _glIndexiv = NULL;
glIndexs_t                          _glIndexs = NULL;
glIndexsv_t                         _glIndexsv = NULL;
glIndexub_t                         _glIndexub = NULL;
glIndexubv_t                        _glIndexubv = NULL;
glInitNames_t                       _glInitNames = NULL;
glInterleavedArrays_t               _glInterleavedArrays = NULL;
glIsEnabled_t                       _glIsEnabled = NULL;
glIsList_t                          _glIsList = NULL;
glIsTexture_t                       _glIsTexture = NULL;
glLightModelf_t                     _glLightModelf = NULL;
glLightModelfv_t                    _glLightModelfv = NULL;
glLightModeli_t                     _glLightModeli = NULL;
glLightModeliv_t                    _glLightModeliv = NULL;
glLightf_t                          _glLightf = NULL;
glLightfv_t                         _glLightfv = NULL;
glLighti_t                          _glLighti = NULL;
glLightiv_t                         _glLightiv = NULL;
glLineStipple_t                     _glLineStipple = NULL;
glLineWidth_t                       _glLineWidth = NULL;
glListBase_t                        _glListBase = NULL;
glLoadIdentity_t                    _glLoadIdentity = NULL;
glLoadMatrixd_t                     _glLoadMatrixd = NULL;
glLoadMatrixf_t                     _glLoadMatrixf = NULL;
glLoadName_t                        _glLoadName = NULL;
glLogicOp_t                         _glLogicOp = NULL;
glMap1d_t                           _glMap1d = NULL;
glMap1f_t                           _glMap1f = NULL;
glMap2d_t                           _glMap2d = NULL;
glMap2f_t                           _glMap2f = NULL;
glMapGrid1d_t                       _glMapGrid1d = NULL;
glMapGrid1f_t                       _glMapGrid1f = NULL;
glMapGrid2d_t                       _glMapGrid2d = NULL;
glMapGrid2f_t                       _glMapGrid2f = NULL;
glMaterialf_t                       _glMaterialf = NULL;
glMaterialfv_t                      _glMaterialfv = NULL;
glMateriali_t                       _glMateriali = NULL;
glMaterialiv_t                      _glMaterialiv = NULL;
glMatrixMode_t                      _glMatrixMode = NULL;
glMultMatrixd_t                     _glMultMatrixd = NULL;
glMultMatrixf_t                     _glMultMatrixf = NULL;
glNewList_t                         _glNewList = NULL;
glNormal3b_t                        _glNormal3b = NULL;
glNormal3bv_t                       _glNormal3bv = NULL;
glNormal3d_t                        _glNormal3d = NULL;
glNormal3dv_t                       _glNormal3dv = NULL;
glNormal3f_t                        _glNormal3f = NULL;
glNormal3fv_t                       _glNormal3fv = NULL;
glNormal3i_t                        _glNormal3i = NULL;
glNormal3iv_t                       _glNormal3iv = NULL;
glNormal3s_t                        _glNormal3s = NULL;
glNormal3sv_t                       _glNormal3sv = NULL;
glNormalPointer_t                   _glNormalPointer = NULL;
glOrtho_t                           _glOrtho = NULL;
glPassThrough_t                     _glPassThrough = NULL;
glPixelMapfv_t                      _glPixelMapfv = NULL;
glPixelMapuiv_t                     _glPixelMapuiv = NULL;
glPixelMapusv_t                     _glPixelMapusv = NULL;
glPixelStoref_t                     _glPixelStoref = NULL;
glPixelStorei_t                     _glPixelStorei = NULL;
glPixelTransferf_t                  _glPixelTransferf = NULL;
glPixelTransferi_t                  _glPixelTransferi = NULL;
glPixelZoom_t                       _glPixelZoom = NULL;
glPointSize_t                       _glPointSize = NULL;
glPolygonMode_t                     _glPolygonMode = NULL;
glPolygonOffset_t                   _glPolygonOffset = NULL;
glPolygonStipple_t                  _glPolygonStipple = NULL;
glPopAttrib_t                       _glPopAttrib = NULL;
glPopClientAttrib_t                 _glPopClientAttrib = NULL;
glPopMatrix_t                       _glPopMatrix = NULL;
glPopName_t                         _glPopName = NULL;
glPrioritizeTextures_t              _glPrioritizeTextures = NULL;
glPushAttrib_t                      _glPushAttrib = NULL;
glPushClientAttrib_t                _glPushClientAttrib = NULL;
glPushMatrix_t                      _glPushMatrix = NULL;
glPushName_t                        _glPushName = NULL;
glRasterPos2d_t                     _glRasterPos2d = NULL;
glRasterPos2dv_t                    _glRasterPos2dv = NULL;
glRasterPos2f_t                     _glRasterPos2f = NULL;
glRasterPos2fv_t                    _glRasterPos2fv = NULL;
glRasterPos2i_t                     _glRasterPos2i = NULL;
glRasterPos2iv_t                    _glRasterPos2iv = NULL;
glRasterPos2s_t                     _glRasterPos2s = NULL;
glRasterPos2sv_t                    _glRasterPos2sv = NULL;
glRasterPos3d_t                     _glRasterPos3d = NULL;
glRasterPos3dv_t                    _glRasterPos3dv = NULL;
glRasterPos3f_t                     _glRasterPos3f = NULL;
glRasterPos3fv_t                    _glRasterPos3fv = NULL;
glRasterPos3i_t                     _glRasterPos3i = NULL;
glRasterPos3iv_t                    _glRasterPos3iv = NULL;
glRasterPos3s_t                     _glRasterPos3s = NULL;
glRasterPos3sv_t                    _glRasterPos3sv = NULL;
glRasterPos4d_t                     _glRasterPos4d = NULL;
glRasterPos4dv_t                    _glRasterPos4dv = NULL;
glRasterPos4f_t                     _glRasterPos4f = NULL;
glRasterPos4fv_t                    _glRasterPos4fv = NULL;
glRasterPos4i_t                     _glRasterPos4i = NULL;
glRasterPos4iv_t                    _glRasterPos4iv = NULL;
glRasterPos4s_t                     _glRasterPos4s = NULL;
glRasterPos4sv_t                    _glRasterPos4sv = NULL;
glReadBuffer_t                      _glReadBuffer = NULL;
glReadPixels_t                      _glReadPixels = NULL;
glRectd_t                           _glRectd = NULL;
glRectdv_t                          _glRectdv = NULL;
glRectf_t                           _glRectf = NULL;
glRectfv_t                          _glRectfv = NULL;
glRecti_t                           _glRecti = NULL;
glRectiv_t                          _glRectiv = NULL;
glRects_t                           _glRects = NULL;
glRectsv_t                          _glRectsv = NULL;
glRenderMode_t                      _glRenderMode = NULL;
glRotated_t                         _glRotated = NULL;
glRotatef_t                         _glRotatef = NULL;
glScaled_t                          _glScaled = NULL;
glScalef_t                          _glScalef = NULL;
glScissor_t                         _glScissor = NULL;
glSelectBuffer_t                    _glSelectBuffer = NULL;
glShadeModel_t                      _glShadeModel = NULL;
glStencilFunc_t                     _glStencilFunc = NULL;
glStencilMask_t                     _glStencilMask = NULL;
glStencilOp_t                       _glStencilOp = NULL;
glTexCoord1d_t                      _glTexCoord1d = NULL;
glTexCoord1dv_t                     _glTexCoord1dv = NULL;
glTexCoord1f_t                      _glTexCoord1f = NULL;
glTexCoord1fv_t                     _glTexCoord1fv = NULL;
glTexCoord1i_t                      _glTexCoord1i = NULL;
glTexCoord1iv_t                     _glTexCoord1iv = NULL;
glTexCoord1s_t                      _glTexCoord1s = NULL;
glTexCoord1sv_t                     _glTexCoord1sv = NULL;
glTexCoord2d_t                      _glTexCoord2d = NULL;
glTexCoord2dv_t                     _glTexCoord2dv = NULL;
glTexCoord2f_t                      _glTexCoord2f = NULL;
glTexCoord2fv_t                     _glTexCoord2fv = NULL;
glTexCoord2i_t                      _glTexCoord2i = NULL;
glTexCoord2iv_t                     _glTexCoord2iv = NULL;
glTexCoord2s_t                      _glTexCoord2s = NULL;
glTexCoord2sv_t                     _glTexCoord2sv = NULL;
glTexCoord3d_t                      _glTexCoord3d = NULL;
glTexCoord3dv_t                     _glTexCoord3dv = NULL;
glTexCoord3f_t                      _glTexCoord3f = NULL;
glTexCoord3fv_t                     _glTexCoord3fv = NULL;
glTexCoord3i_t                      _glTexCoord3i = NULL;
glTexCoord3iv_t                     _glTexCoord3iv = NULL;
glTexCoord3s_t                      _glTexCoord3s = NULL;
glTexCoord3sv_t                     _glTexCoord3sv = NULL;
glTexCoord4d_t                      _glTexCoord4d = NULL;
glTexCoord4dv_t                     _glTexCoord4dv = NULL;
glTexCoord4f_t                      _glTexCoord4f = NULL;
glTexCoord4fv_t                     _glTexCoord4fv = NULL;
glTexCoord4i_t                      _glTexCoord4i = NULL;
glTexCoord4iv_t                     _glTexCoord4iv = NULL;
glTexCoord4s_t                      _glTexCoord4s = NULL;
glTexCoord4sv_t                     _glTexCoord4sv = NULL;
glTexCoordPointer_t                 _glTexCoordPointer = NULL;
glTexEnvf_t                         _glTexEnvf = NULL;
glTexEnvfv_t                        _glTexEnvfv = NULL;
glTexEnvi_t                         _glTexEnvi = NULL;
glTexEnviv_t                        _glTexEnviv = NULL;
glTexGend_t                         _glTexGend = NULL;
glTexGendv_t                        _glTexGendv = NULL;
glTexGenf_t                         _glTexGenf = NULL;
glTexGenfv_t                        _glTexGenfv = NULL;
glTexGeni_t                         _glTexGeni = NULL;
glTexGeniv_t                        _glTexGeniv = NULL;
glTexImage1D_t                      _glTexImage1D = NULL;
glTexImage2D_t                      _glTexImage2D = NULL;
glTexParameterf_t                   _glTexParameterf = NULL;
glTexParameterfv_t                  _glTexParameterfv = NULL;
glTexParameteri_t                   _glTexParameteri = NULL;
glTexParameteriv_t                  _glTexParameteriv = NULL;
glTexSubImage1D_t                   _glTexSubImage1D = NULL;
glTexSubImage2D_t                   _glTexSubImage2D = NULL;
glTranslated_t                      _glTranslated = NULL;
glTranslatef_t                      _glTranslatef = NULL;
glVertex2d_t                        _glVertex2d = NULL;
glVertex2dv_t                       _glVertex2dv = NULL;
glVertex2f_t                        _glVertex2f = NULL;
glVertex2fv_t                       _glVertex2fv = NULL;
glVertex2i_t                        _glVertex2i = NULL;
glVertex2iv_t                       _glVertex2iv = NULL;
glVertex2s_t                        _glVertex2s = NULL;
glVertex2sv_t                       _glVertex2sv = NULL;
glVertex3d_t                        _glVertex3d = NULL;
glVertex3dv_t                       _glVertex3dv = NULL;
glVertex3f_t                        _glVertex3f = NULL;
glVertex3fv_t                       _glVertex3fv = NULL;
glVertex3i_t                        _glVertex3i = NULL;
glVertex3iv_t                       _glVertex3iv = NULL;
glVertex3s_t                        _glVertex3s = NULL;
glVertex3sv_t                       _glVertex3sv = NULL;
glVertex4d_t                        _glVertex4d = NULL;
glVertex4dv_t                       _glVertex4dv = NULL;
glVertex4f_t                        _glVertex4f = NULL;
glVertex4fv_t                       _glVertex4fv = NULL;
glVertex4i_t                        _glVertex4i = NULL;
glVertex4iv_t                       _glVertex4iv = NULL;
glVertex4s_t                        _glVertex4s = NULL;
glVertex4sv_t                       _glVertex4sv = NULL;
glVertexPointer_t                   _glVertexPointer = NULL;
glViewport_t                        _glViewport = NULL;

HMODULE glMainLibrary = NULL;
static RwList <glDriver> allDevices;

// Last driver used for context stack
static glDriver *lastStackDriver = NULL;

// Quick context switching object (to stack context acquisitions safely)
struct glContextStack
{
    __forceinline glContextStack( glDriver *driver )
    {
        if ( lastStackDriver != driver )
        {
            // Save previous context
            dc = _wglGetCurrentDC();
            glrc = _wglGetCurrentContext();

            // Change current context
            _wglMakeCurrent( driver->deviceContext, driver->glContext );

            prevStackDriver = lastStackDriver;
            lastStackDriver = driver;

            contextSwapped = true;
        }
        else
            contextSwapped = false;
    }

    __forceinline ~glContextStack( void )
    {
        if ( contextSwapped )
        {
            lastStackDriver = prevStackDriver;

            _wglMakeCurrent( dc, glrc );
        }
    }

    bool contextSwapped;
    HDC dc;
    HGLRC glrc;
    glDriver *prevStackDriver;
};

static LUA_DECLARE( runContext )
{
    if ( lua_type( L, 1 ) != LUA_TFUNCTION )
        throw lua_exception( L, LUA_ERRRUN, "expected function at OpenGL context function" );

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    lua_call( L, 0, 0 );
    return 0;
};

static const luaL_Reg driver_interface[] =
{
    LUA_METHOD( runContext ),
    { NULL, NULL }
};

static LUA_DECLARE( destroy )
{
    glDriver *driver = (glDriver*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    // Call the destruction event.
    driver->PushMethod( L, "triggerEvent" );
    lua_pushcstring( L, "onDestruction" );
    lua_call( L, 1, 0 );

    delete driver;

    return 0;
}

static const luaL_Reg driver_interface_sys[] =
{
    LUA_METHOD( destroy ),
    { NULL, NULL }
};

static LUA_DECLARE( constructor )
{
    glDriver *driver = (glDriver*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
    j->SetTransmit( LUACLASS_GLDRIVER, driver );

    j->RegisterInterfaceTrans( L, driver_interface, 0, LUACLASS_GLDRIVER );

    // Include the event API
    luaevent_extend( L );

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    j->RegisterInterface( L, driver_interface_sys, 1 );

    lua_pushcstring( L, "glDriver" );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

static ILuaClass* _trefget( lua_State *L, glDriver *driver )
{
    lua_pushlightuserdata( L, driver );
    lua_pushcclosure( L, constructor, 1 );
    lua_newclassex( L, LCLASS_API_LIGHT );

    ILuaClass *j = lua_refclass( L, -1 );
    lua_pop( L, 1 );
    return j;
}

glDriver::glDriver( lua_State *L, Win32Dialog *wnd ) : LuaClass( L, _trefget( L, this ) )
{
    // Reference the window this driver uses as we use resources from it
    // The OpenGL driver has to be destroyed before the window.
    wnd->IncrementMethodStack();

    // Initialize the OpenGL environment
    m_window = wnd;
    deviceContext = GetDC( wnd->handle );

    // Setup the device context
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int iFormat = ChoosePixelFormat( deviceContext, &pfd );
    SetPixelFormat( deviceContext, iFormat, &pfd );

    glContext = _wglCreateContext( deviceContext );

    LIST_INSERT( allDevices.root, rootNode );

    // Register events
    const char* glEvents[] =
    {
        "onFrame",
        "onDestruction"
    };
    
    for ( unsigned int n = 0; n < NUMELMS(glEvents); n++ )
    {
        m_class->PushMethod( L, "addEvent" );
        lua_pushlstring( L, glEvents[n], strlen( glEvents[n] ) );
        lua_call( L, 1, 0 );
    }
}

glDriver::~glDriver( void )
{
    LIST_REMOVE( rootNode );

    // Destroy OpenGL environment
    _wglDeleteContext( glContext );
    ReleaseDC( m_window->handle, deviceContext );

    m_window->DecrementMethodStack();
}

#define API_LOAD( name ) \
    _##name = (##name##_t)GetProcAddress( glMainLibrary, #name )
#define API_CHECK( module, x ) \
    if ( !( x ) ) \
    { \
        apiFatal = true; \
        printf( "critical function " #x " of library " module " is missing\n" ); \
    }

void luagl_initDrivers( void )
{
    // Load the main Win32 OpenGL library
    glMainLibrary = LoadLibrary( "opengl32.dll" );

    if ( glMainLibrary )
    {
        _wglCopyContext =           (wglCopyContext_t)GetProcAddress( glMainLibrary, "wglCopyContext" );
        _wglCreateContext =         (wglCreateContext_t)GetProcAddress( glMainLibrary, "wglCreateContext" );
        _wglCreateLayerContext =    (wglCreateLayerContext_t)GetProcAddress( glMainLibrary, "wglCreateLayerContext" );
        _wglDeleteContext =         (wglDeleteContext_t)GetProcAddress( glMainLibrary, "wglDeleteContext" );
        _wglGetCurrentContext =     (wglGetCurrentContext_t)GetProcAddress( glMainLibrary, "wglGetCurrentContext" );
        _wglGetCurrentDC =          (wglGetCurrentDC_t)GetProcAddress( glMainLibrary, "wglGetCurrentDC" );
        _wglGetProcAddress =        (wglGetProcAddress_t)GetProcAddress( glMainLibrary, "wglGetProcAddress" );
        _wglMakeCurrent =           (wglMakeCurrent_t)GetProcAddress( glMainLibrary, "wglMakeCurrent" );
        _wglShareLists =            (wglShareLists_t)GetProcAddress( glMainLibrary, "wglShareLists" );
        _wglUseFontBitmapsA =       (wglUseFontBitmapsA_t)GetProcAddress( glMainLibrary, "wglUseFontBitmapsA" );
        _wglUseFontBitmapsW =       (wglUseFontBitmapsW_t)GetProcAddress( glMainLibrary, "wglUseFontBitmapsW" );

        // MACHINE GENERATED; USE gltools.lua !!!
        API_LOAD( glAccum );
        API_LOAD( glAlphaFunc );
        API_LOAD( glAreTexturesResident );
        API_LOAD( glArrayElement );
        API_LOAD( glBegin );
        API_LOAD( glBindTexture );
        API_LOAD( glBitmap );
        API_LOAD( glBlendFunc );
        API_LOAD( glCallList );
        API_LOAD( glCallLists );
        API_LOAD( glClear );
        API_LOAD( glClearAccum );
        API_LOAD( glClearColor );
        API_LOAD( glClearDepth );
        API_LOAD( glClearIndex );
        API_LOAD( glClearStencil );
        API_LOAD( glClipPlane );
        API_LOAD( glColor3b );
        API_LOAD( glColor3bv );
        API_LOAD( glColor3d );
        API_LOAD( glColor3dv );
        API_LOAD( glColor3f );
        API_LOAD( glColor3fv );
        API_LOAD( glColor3i );
        API_LOAD( glColor3iv );
        API_LOAD( glColor3s );
        API_LOAD( glColor3sv );
        API_LOAD( glColor3ub );
        API_LOAD( glColor3ubv );
        API_LOAD( glColor3ui );
        API_LOAD( glColor3uiv );
        API_LOAD( glColor3us );
        API_LOAD( glColor3usv );
        API_LOAD( glColor4b );
        API_LOAD( glColor4bv );
        API_LOAD( glColor4d );
        API_LOAD( glColor4dv );
        API_LOAD( glColor4f );
        API_LOAD( glColor4fv );
        API_LOAD( glColor4i );
        API_LOAD( glColor4iv );
        API_LOAD( glColor4s );
        API_LOAD( glColor4sv );
        API_LOAD( glColor4ub );
        API_LOAD( glColor4ubv );
        API_LOAD( glColor4ui );
        API_LOAD( glColor4uiv );
        API_LOAD( glColor4us );
        API_LOAD( glColor4usv );
        API_LOAD( glColorMask );
        API_LOAD( glColorMaterial );
        API_LOAD( glColorPointer );
        API_LOAD( glCopyPixels );
        API_LOAD( glCopyTexImage1D );
        API_LOAD( glCopyTexImage2D );
        API_LOAD( glCopyTexSubImage1D );
        API_LOAD( glCopyTexSubImage2D );
        API_LOAD( glCullFace );
        API_LOAD( glDeleteLists );
        API_LOAD( glDeleteTextures );
        API_LOAD( glDepthFunc );
        API_LOAD( glDepthMask );
        API_LOAD( glDepthRange );
        API_LOAD( glDisable );
        API_LOAD( glDisableClientState );
        API_LOAD( glDrawArrays );
        API_LOAD( glDrawBuffer );
        API_LOAD( glDrawElements );
        API_LOAD( glDrawPixels );
        API_LOAD( glEdgeFlag );
        API_LOAD( glEdgeFlagPointer );
        API_LOAD( glEdgeFlagv );
        API_LOAD( glEnable );
        API_LOAD( glEnableClientState );
        API_LOAD( glEnd );
        API_LOAD( glEndList );
        API_LOAD( glEvalCoord1d );
        API_LOAD( glEvalCoord1dv );
        API_LOAD( glEvalCoord1f );
        API_LOAD( glEvalCoord1fv );
        API_LOAD( glEvalCoord2d );
        API_LOAD( glEvalCoord2dv );
        API_LOAD( glEvalCoord2f );
        API_LOAD( glEvalCoord2fv );
        API_LOAD( glEvalMesh1 );
        API_LOAD( glEvalMesh2 );
        API_LOAD( glEvalPoint1 );
        API_LOAD( glEvalPoint2 );
        API_LOAD( glFeedbackBuffer );
        API_LOAD( glFinish );
        API_LOAD( glFlush );
        API_LOAD( glFogf );
        API_LOAD( glFogfv );
        API_LOAD( glFogi );
        API_LOAD( glFogiv );
        API_LOAD( glFrontFace );
        API_LOAD( glFrustum );
        API_LOAD( glGenLists );
        API_LOAD( glGenTextures );
        API_LOAD( glGetBooleanv );
        API_LOAD( glGetClipPlane );
        API_LOAD( glGetDoublev );
        API_LOAD( glGetError );
        API_LOAD( glGetFloatv );
        API_LOAD( glGetIntegerv );
        API_LOAD( glGetLightfv );
        API_LOAD( glGetLightiv );
        API_LOAD( glGetMapdv );
        API_LOAD( glGetMapfv );
        API_LOAD( glGetMapiv );
        API_LOAD( glGetMaterialfv );
        API_LOAD( glGetMaterialiv );
        API_LOAD( glGetPixelMapfv );
        API_LOAD( glGetPixelMapuiv );
        API_LOAD( glGetPixelMapusv );
        API_LOAD( glGetPointerv );
        API_LOAD( glGetPolygonStipple );
        API_LOAD( glGetString );
        API_LOAD( glGetTexEnvfv );
        API_LOAD( glGetTexEnviv );
        API_LOAD( glGetTexGendv );
        API_LOAD( glGetTexGenfv );
        API_LOAD( glGetTexGeniv );
        API_LOAD( glGetTexImage );
        API_LOAD( glGetTexLevelParameterfv );
        API_LOAD( glGetTexLevelParameteriv );
        API_LOAD( glGetTexParameterfv );
        API_LOAD( glGetTexParameteriv );
        API_LOAD( glHint );
        API_LOAD( glIndexMask );
        API_LOAD( glIndexPointer );
        API_LOAD( glIndexd );
        API_LOAD( glIndexdv );
        API_LOAD( glIndexf );
        API_LOAD( glIndexfv );
        API_LOAD( glIndexi );
        API_LOAD( glIndexiv );
        API_LOAD( glIndexs );
        API_LOAD( glIndexsv );
        API_LOAD( glIndexub );
        API_LOAD( glIndexubv );
        API_LOAD( glInitNames );
        API_LOAD( glInterleavedArrays );
        API_LOAD( glIsEnabled );
        API_LOAD( glIsList );
        API_LOAD( glIsTexture );
        API_LOAD( glLightModelf );
        API_LOAD( glLightModelfv );
        API_LOAD( glLightModeli );
        API_LOAD( glLightModeliv );
        API_LOAD( glLightf );
        API_LOAD( glLightfv );
        API_LOAD( glLighti );
        API_LOAD( glLightiv );
        API_LOAD( glLineStipple );
        API_LOAD( glLineWidth );
        API_LOAD( glListBase );
        API_LOAD( glLoadIdentity );
        API_LOAD( glLoadMatrixd );
        API_LOAD( glLoadMatrixf );
        API_LOAD( glLoadName );
        API_LOAD( glLogicOp );
        API_LOAD( glMap1d );
        API_LOAD( glMap1f );
        API_LOAD( glMap2d );
        API_LOAD( glMap2f );
        API_LOAD( glMapGrid1d );
        API_LOAD( glMapGrid1f );
        API_LOAD( glMapGrid2d );
        API_LOAD( glMapGrid2f );
        API_LOAD( glMaterialf );
        API_LOAD( glMaterialfv );
        API_LOAD( glMateriali );
        API_LOAD( glMaterialiv );
        API_LOAD( glMatrixMode );
        API_LOAD( glMultMatrixd );
        API_LOAD( glMultMatrixf );
        API_LOAD( glNewList );
        API_LOAD( glNormal3b );
        API_LOAD( glNormal3bv );
        API_LOAD( glNormal3d );
        API_LOAD( glNormal3dv );
        API_LOAD( glNormal3f );
        API_LOAD( glNormal3fv );
        API_LOAD( glNormal3i );
        API_LOAD( glNormal3iv );
        API_LOAD( glNormal3s );
        API_LOAD( glNormal3sv );
        API_LOAD( glNormalPointer );
        API_LOAD( glOrtho );
        API_LOAD( glPassThrough );
        API_LOAD( glPixelMapfv );
        API_LOAD( glPixelMapuiv );
        API_LOAD( glPixelMapusv );
        API_LOAD( glPixelStoref );
        API_LOAD( glPixelStorei );
        API_LOAD( glPixelTransferf );
        API_LOAD( glPixelTransferi );
        API_LOAD( glPixelZoom );
        API_LOAD( glPointSize );
        API_LOAD( glPolygonMode );
        API_LOAD( glPolygonOffset );
        API_LOAD( glPolygonStipple );
        API_LOAD( glPopAttrib );
        API_LOAD( glPopClientAttrib );
        API_LOAD( glPopMatrix );
        API_LOAD( glPopName );
        API_LOAD( glPrioritizeTextures );
        API_LOAD( glPushAttrib );
        API_LOAD( glPushClientAttrib );
        API_LOAD( glPushMatrix );
        API_LOAD( glPushName );
        API_LOAD( glRasterPos2d );
        API_LOAD( glRasterPos2dv );
        API_LOAD( glRasterPos2f );
        API_LOAD( glRasterPos2fv );
        API_LOAD( glRasterPos2i );
        API_LOAD( glRasterPos2iv );
        API_LOAD( glRasterPos2s );
        API_LOAD( glRasterPos2sv );
        API_LOAD( glRasterPos3d );
        API_LOAD( glRasterPos3dv );
        API_LOAD( glRasterPos3f );
        API_LOAD( glRasterPos3fv );
        API_LOAD( glRasterPos3i );
        API_LOAD( glRasterPos3iv );
        API_LOAD( glRasterPos3s );
        API_LOAD( glRasterPos3sv );
        API_LOAD( glRasterPos4d );
        API_LOAD( glRasterPos4dv );
        API_LOAD( glRasterPos4f );
        API_LOAD( glRasterPos4fv );
        API_LOAD( glRasterPos4i );
        API_LOAD( glRasterPos4iv );
        API_LOAD( glRasterPos4s );
        API_LOAD( glRasterPos4sv );
        API_LOAD( glReadBuffer );
        API_LOAD( glReadPixels );
        API_LOAD( glRectd );
        API_LOAD( glRectdv );
        API_LOAD( glRectf );
        API_LOAD( glRectfv );
        API_LOAD( glRecti );
        API_LOAD( glRectiv );
        API_LOAD( glRects );
        API_LOAD( glRectsv );
        API_LOAD( glRenderMode );
        API_LOAD( glRotated );
        API_LOAD( glRotatef );
        API_LOAD( glScaled );
        API_LOAD( glScalef );
        API_LOAD( glScissor );
        API_LOAD( glSelectBuffer );
        API_LOAD( glShadeModel );
        API_LOAD( glStencilFunc );
        API_LOAD( glStencilMask );
        API_LOAD( glStencilOp );
        API_LOAD( glTexCoord1d );
        API_LOAD( glTexCoord1dv );
        API_LOAD( glTexCoord1f );
        API_LOAD( glTexCoord1fv );
        API_LOAD( glTexCoord1i );
        API_LOAD( glTexCoord1iv );
        API_LOAD( glTexCoord1s );
        API_LOAD( glTexCoord1sv );
        API_LOAD( glTexCoord2d );
        API_LOAD( glTexCoord2dv );
        API_LOAD( glTexCoord2f );
        API_LOAD( glTexCoord2fv );
        API_LOAD( glTexCoord2i );
        API_LOAD( glTexCoord2iv );
        API_LOAD( glTexCoord2s );
        API_LOAD( glTexCoord2sv );
        API_LOAD( glTexCoord3d );
        API_LOAD( glTexCoord3dv );
        API_LOAD( glTexCoord3f );
        API_LOAD( glTexCoord3fv );
        API_LOAD( glTexCoord3i );
        API_LOAD( glTexCoord3iv );
        API_LOAD( glTexCoord3s );
        API_LOAD( glTexCoord3sv );
        API_LOAD( glTexCoord4d );
        API_LOAD( glTexCoord4dv );
        API_LOAD( glTexCoord4f );
        API_LOAD( glTexCoord4fv );
        API_LOAD( glTexCoord4i );
        API_LOAD( glTexCoord4iv );
        API_LOAD( glTexCoord4s );
        API_LOAD( glTexCoord4sv );
        API_LOAD( glTexCoordPointer );
        API_LOAD( glTexEnvf );
        API_LOAD( glTexEnvfv );
        API_LOAD( glTexEnvi );
        API_LOAD( glTexEnviv );
        API_LOAD( glTexGend );
        API_LOAD( glTexGendv );
        API_LOAD( glTexGenf );
        API_LOAD( glTexGenfv );
        API_LOAD( glTexGeni );
        API_LOAD( glTexGeniv );
        API_LOAD( glTexImage1D );
        API_LOAD( glTexImage2D );
        API_LOAD( glTexParameterf );
        API_LOAD( glTexParameterfv );
        API_LOAD( glTexParameteri );
        API_LOAD( glTexParameteriv );
        API_LOAD( glTexSubImage1D );
        API_LOAD( glTexSubImage2D );
        API_LOAD( glTranslated );
        API_LOAD( glTranslatef );
        API_LOAD( glVertex2d );
        API_LOAD( glVertex2dv );
        API_LOAD( glVertex2f );
        API_LOAD( glVertex2fv );
        API_LOAD( glVertex2i );
        API_LOAD( glVertex2iv );
        API_LOAD( glVertex2s );
        API_LOAD( glVertex2sv );
        API_LOAD( glVertex3d );
        API_LOAD( glVertex3dv );
        API_LOAD( glVertex3f );
        API_LOAD( glVertex3fv );
        API_LOAD( glVertex3i );
        API_LOAD( glVertex3iv );
        API_LOAD( glVertex3s );
        API_LOAD( glVertex3sv );
        API_LOAD( glVertex4d );
        API_LOAD( glVertex4dv );
        API_LOAD( glVertex4f );
        API_LOAD( glVertex4fv );
        API_LOAD( glVertex4i );
        API_LOAD( glVertex4iv );
        API_LOAD( glVertex4s );
        API_LOAD( glVertex4sv );
        API_LOAD( glVertexPointer );
        API_LOAD( glViewport );

        // Check that all API is linked
        bool apiFatal = false;

        API_CHECK( "opengl32.dll", _wglCopyContext );
        API_CHECK( "opengl32.dll", _wglCreateContext );
        API_CHECK( "opengl32.dll", _wglCreateLayerContext );
        API_CHECK( "opengl32.dll", _wglDeleteContext );
        API_CHECK( "opengl32.dll", _wglGetCurrentContext );
        API_CHECK( "opengl32.dll", _wglGetCurrentDC );
        API_CHECK( "opengl32.dll", _wglGetProcAddress );
        API_CHECK( "opengl32.dll", _wglMakeCurrent );
        API_CHECK( "opengl32.dll", _wglShareLists );
        API_CHECK( "opengl32.dll", _wglUseFontBitmapsA );
        API_CHECK( "opengl32.dll", _wglUseFontBitmapsW );

        // MACHINE GENERATED; USE gltools.lua !!!
        API_CHECK( "opengl32.dll", _glAccum );
        API_CHECK( "opengl32.dll", _glAlphaFunc );
        API_CHECK( "opengl32.dll", _glAreTexturesResident );
        API_CHECK( "opengl32.dll", _glArrayElement );
        API_CHECK( "opengl32.dll", _glBegin );
        API_CHECK( "opengl32.dll", _glBindTexture );
        API_CHECK( "opengl32.dll", _glBitmap );
        API_CHECK( "opengl32.dll", _glBlendFunc );
        API_CHECK( "opengl32.dll", _glCallList );
        API_CHECK( "opengl32.dll", _glCallLists );
        API_CHECK( "opengl32.dll", _glClear );
        API_CHECK( "opengl32.dll", _glClearAccum );
        API_CHECK( "opengl32.dll", _glClearColor );
        API_CHECK( "opengl32.dll", _glClearDepth );
        API_CHECK( "opengl32.dll", _glClearIndex );
        API_CHECK( "opengl32.dll", _glClearStencil );
        API_CHECK( "opengl32.dll", _glClipPlane );
        API_CHECK( "opengl32.dll", _glColor3b );
        API_CHECK( "opengl32.dll", _glColor3bv );
        API_CHECK( "opengl32.dll", _glColor3d );
        API_CHECK( "opengl32.dll", _glColor3dv );
        API_CHECK( "opengl32.dll", _glColor3f );
        API_CHECK( "opengl32.dll", _glColor3fv );
        API_CHECK( "opengl32.dll", _glColor3i );
        API_CHECK( "opengl32.dll", _glColor3iv );
        API_CHECK( "opengl32.dll", _glColor3s );
        API_CHECK( "opengl32.dll", _glColor3sv );
        API_CHECK( "opengl32.dll", _glColor3ub );
        API_CHECK( "opengl32.dll", _glColor3ubv );
        API_CHECK( "opengl32.dll", _glColor3ui );
        API_CHECK( "opengl32.dll", _glColor3uiv );
        API_CHECK( "opengl32.dll", _glColor3us );
        API_CHECK( "opengl32.dll", _glColor3usv );
        API_CHECK( "opengl32.dll", _glColor4b );
        API_CHECK( "opengl32.dll", _glColor4bv );
        API_CHECK( "opengl32.dll", _glColor4d );
        API_CHECK( "opengl32.dll", _glColor4dv );
        API_CHECK( "opengl32.dll", _glColor4f );
        API_CHECK( "opengl32.dll", _glColor4fv );
        API_CHECK( "opengl32.dll", _glColor4i );
        API_CHECK( "opengl32.dll", _glColor4iv );
        API_CHECK( "opengl32.dll", _glColor4s );
        API_CHECK( "opengl32.dll", _glColor4sv );
        API_CHECK( "opengl32.dll", _glColor4ub );
        API_CHECK( "opengl32.dll", _glColor4ubv );
        API_CHECK( "opengl32.dll", _glColor4ui );
        API_CHECK( "opengl32.dll", _glColor4uiv );
        API_CHECK( "opengl32.dll", _glColor4us );
        API_CHECK( "opengl32.dll", _glColor4usv );
        API_CHECK( "opengl32.dll", _glColorMask );
        API_CHECK( "opengl32.dll", _glColorMaterial );
        API_CHECK( "opengl32.dll", _glColorPointer );
        API_CHECK( "opengl32.dll", _glCopyPixels );
        API_CHECK( "opengl32.dll", _glCopyTexImage1D );
        API_CHECK( "opengl32.dll", _glCopyTexImage2D );
        API_CHECK( "opengl32.dll", _glCopyTexSubImage1D );
        API_CHECK( "opengl32.dll", _glCopyTexSubImage2D );
        API_CHECK( "opengl32.dll", _glCullFace );
        API_CHECK( "opengl32.dll", _glDeleteLists );
        API_CHECK( "opengl32.dll", _glDeleteTextures );
        API_CHECK( "opengl32.dll", _glDepthFunc );
        API_CHECK( "opengl32.dll", _glDepthMask );
        API_CHECK( "opengl32.dll", _glDepthRange );
        API_CHECK( "opengl32.dll", _glDisable );
        API_CHECK( "opengl32.dll", _glDisableClientState );
        API_CHECK( "opengl32.dll", _glDrawArrays );
        API_CHECK( "opengl32.dll", _glDrawBuffer );
        API_CHECK( "opengl32.dll", _glDrawElements );
        API_CHECK( "opengl32.dll", _glDrawPixels );
        API_CHECK( "opengl32.dll", _glEdgeFlag );
        API_CHECK( "opengl32.dll", _glEdgeFlagPointer );
        API_CHECK( "opengl32.dll", _glEdgeFlagv );
        API_CHECK( "opengl32.dll", _glEnable );
        API_CHECK( "opengl32.dll", _glEnableClientState );
        API_CHECK( "opengl32.dll", _glEnd );
        API_CHECK( "opengl32.dll", _glEndList );
        API_CHECK( "opengl32.dll", _glEvalCoord1d );
        API_CHECK( "opengl32.dll", _glEvalCoord1dv );
        API_CHECK( "opengl32.dll", _glEvalCoord1f );
        API_CHECK( "opengl32.dll", _glEvalCoord1fv );
        API_CHECK( "opengl32.dll", _glEvalCoord2d );
        API_CHECK( "opengl32.dll", _glEvalCoord2dv );
        API_CHECK( "opengl32.dll", _glEvalCoord2f );
        API_CHECK( "opengl32.dll", _glEvalCoord2fv );
        API_CHECK( "opengl32.dll", _glEvalMesh1 );
        API_CHECK( "opengl32.dll", _glEvalMesh2 );
        API_CHECK( "opengl32.dll", _glEvalPoint1 );
        API_CHECK( "opengl32.dll", _glEvalPoint2 );
        API_CHECK( "opengl32.dll", _glFeedbackBuffer );
        API_CHECK( "opengl32.dll", _glFinish );
        API_CHECK( "opengl32.dll", _glFlush );
        API_CHECK( "opengl32.dll", _glFogf );
        API_CHECK( "opengl32.dll", _glFogfv );
        API_CHECK( "opengl32.dll", _glFogi );
        API_CHECK( "opengl32.dll", _glFogiv );
        API_CHECK( "opengl32.dll", _glFrontFace );
        API_CHECK( "opengl32.dll", _glFrustum );
        API_CHECK( "opengl32.dll", _glGenLists );
        API_CHECK( "opengl32.dll", _glGenTextures );
        API_CHECK( "opengl32.dll", _glGetBooleanv );
        API_CHECK( "opengl32.dll", _glGetClipPlane );
        API_CHECK( "opengl32.dll", _glGetDoublev );
        API_CHECK( "opengl32.dll", _glGetError );
        API_CHECK( "opengl32.dll", _glGetFloatv );
        API_CHECK( "opengl32.dll", _glGetIntegerv );
        API_CHECK( "opengl32.dll", _glGetLightfv );
        API_CHECK( "opengl32.dll", _glGetLightiv );
        API_CHECK( "opengl32.dll", _glGetMapdv );
        API_CHECK( "opengl32.dll", _glGetMapfv );
        API_CHECK( "opengl32.dll", _glGetMapiv );
        API_CHECK( "opengl32.dll", _glGetMaterialfv );
        API_CHECK( "opengl32.dll", _glGetMaterialiv );
        API_CHECK( "opengl32.dll", _glGetPixelMapfv );
        API_CHECK( "opengl32.dll", _glGetPixelMapuiv );
        API_CHECK( "opengl32.dll", _glGetPixelMapusv );
        API_CHECK( "opengl32.dll", _glGetPointerv );
        API_CHECK( "opengl32.dll", _glGetPolygonStipple );
        API_CHECK( "opengl32.dll", _glGetString );
        API_CHECK( "opengl32.dll", _glGetTexEnvfv );
        API_CHECK( "opengl32.dll", _glGetTexEnviv );
        API_CHECK( "opengl32.dll", _glGetTexGendv );
        API_CHECK( "opengl32.dll", _glGetTexGenfv );
        API_CHECK( "opengl32.dll", _glGetTexGeniv );
        API_CHECK( "opengl32.dll", _glGetTexImage );
        API_CHECK( "opengl32.dll", _glGetTexLevelParameterfv );
        API_CHECK( "opengl32.dll", _glGetTexLevelParameteriv );
        API_CHECK( "opengl32.dll", _glGetTexParameterfv );
        API_CHECK( "opengl32.dll", _glGetTexParameteriv );
        API_CHECK( "opengl32.dll", _glHint );
        API_CHECK( "opengl32.dll", _glIndexMask );
        API_CHECK( "opengl32.dll", _glIndexPointer );
        API_CHECK( "opengl32.dll", _glIndexd );
        API_CHECK( "opengl32.dll", _glIndexdv );
        API_CHECK( "opengl32.dll", _glIndexf );
        API_CHECK( "opengl32.dll", _glIndexfv );
        API_CHECK( "opengl32.dll", _glIndexi );
        API_CHECK( "opengl32.dll", _glIndexiv );
        API_CHECK( "opengl32.dll", _glIndexs );
        API_CHECK( "opengl32.dll", _glIndexsv );
        API_CHECK( "opengl32.dll", _glIndexub );
        API_CHECK( "opengl32.dll", _glIndexubv );
        API_CHECK( "opengl32.dll", _glInitNames );
        API_CHECK( "opengl32.dll", _glInterleavedArrays );
        API_CHECK( "opengl32.dll", _glIsEnabled );
        API_CHECK( "opengl32.dll", _glIsList );
        API_CHECK( "opengl32.dll", _glIsTexture );
        API_CHECK( "opengl32.dll", _glLightModelf );
        API_CHECK( "opengl32.dll", _glLightModelfv );
        API_CHECK( "opengl32.dll", _glLightModeli );
        API_CHECK( "opengl32.dll", _glLightModeliv );
        API_CHECK( "opengl32.dll", _glLightf );
        API_CHECK( "opengl32.dll", _glLightfv );
        API_CHECK( "opengl32.dll", _glLighti );
        API_CHECK( "opengl32.dll", _glLightiv );
        API_CHECK( "opengl32.dll", _glLineStipple );
        API_CHECK( "opengl32.dll", _glLineWidth );
        API_CHECK( "opengl32.dll", _glListBase );
        API_CHECK( "opengl32.dll", _glLoadIdentity );
        API_CHECK( "opengl32.dll", _glLoadMatrixd );
        API_CHECK( "opengl32.dll", _glLoadMatrixf );
        API_CHECK( "opengl32.dll", _glLoadName );
        API_CHECK( "opengl32.dll", _glLogicOp );
        API_CHECK( "opengl32.dll", _glMap1d );
        API_CHECK( "opengl32.dll", _glMap1f );
        API_CHECK( "opengl32.dll", _glMap2d );
        API_CHECK( "opengl32.dll", _glMap2f );
        API_CHECK( "opengl32.dll", _glMapGrid1d );
        API_CHECK( "opengl32.dll", _glMapGrid1f );
        API_CHECK( "opengl32.dll", _glMapGrid2d );
        API_CHECK( "opengl32.dll", _glMapGrid2f );
        API_CHECK( "opengl32.dll", _glMaterialf );
        API_CHECK( "opengl32.dll", _glMaterialfv );
        API_CHECK( "opengl32.dll", _glMateriali );
        API_CHECK( "opengl32.dll", _glMaterialiv );
        API_CHECK( "opengl32.dll", _glMatrixMode );
        API_CHECK( "opengl32.dll", _glMultMatrixd );
        API_CHECK( "opengl32.dll", _glMultMatrixf );
        API_CHECK( "opengl32.dll", _glNewList );
        API_CHECK( "opengl32.dll", _glNormal3b );
        API_CHECK( "opengl32.dll", _glNormal3bv );
        API_CHECK( "opengl32.dll", _glNormal3d );
        API_CHECK( "opengl32.dll", _glNormal3dv );
        API_CHECK( "opengl32.dll", _glNormal3f );
        API_CHECK( "opengl32.dll", _glNormal3fv );
        API_CHECK( "opengl32.dll", _glNormal3i );
        API_CHECK( "opengl32.dll", _glNormal3iv );
        API_CHECK( "opengl32.dll", _glNormal3s );
        API_CHECK( "opengl32.dll", _glNormal3sv );
        API_CHECK( "opengl32.dll", _glNormalPointer );
        API_CHECK( "opengl32.dll", _glOrtho );
        API_CHECK( "opengl32.dll", _glPassThrough );
        API_CHECK( "opengl32.dll", _glPixelMapfv );
        API_CHECK( "opengl32.dll", _glPixelMapuiv );
        API_CHECK( "opengl32.dll", _glPixelMapusv );
        API_CHECK( "opengl32.dll", _glPixelStoref );
        API_CHECK( "opengl32.dll", _glPixelStorei );
        API_CHECK( "opengl32.dll", _glPixelTransferf );
        API_CHECK( "opengl32.dll", _glPixelTransferi );
        API_CHECK( "opengl32.dll", _glPixelZoom );
        API_CHECK( "opengl32.dll", _glPointSize );
        API_CHECK( "opengl32.dll", _glPolygonMode );
        API_CHECK( "opengl32.dll", _glPolygonOffset );
        API_CHECK( "opengl32.dll", _glPolygonStipple );
        API_CHECK( "opengl32.dll", _glPopAttrib );
        API_CHECK( "opengl32.dll", _glPopClientAttrib );
        API_CHECK( "opengl32.dll", _glPopMatrix );
        API_CHECK( "opengl32.dll", _glPopName );
        API_CHECK( "opengl32.dll", _glPrioritizeTextures );
        API_CHECK( "opengl32.dll", _glPushAttrib );
        API_CHECK( "opengl32.dll", _glPushClientAttrib );
        API_CHECK( "opengl32.dll", _glPushMatrix );
        API_CHECK( "opengl32.dll", _glPushName );
        API_CHECK( "opengl32.dll", _glRasterPos2d );
        API_CHECK( "opengl32.dll", _glRasterPos2dv );
        API_CHECK( "opengl32.dll", _glRasterPos2f );
        API_CHECK( "opengl32.dll", _glRasterPos2fv );
        API_CHECK( "opengl32.dll", _glRasterPos2i );
        API_CHECK( "opengl32.dll", _glRasterPos2iv );
        API_CHECK( "opengl32.dll", _glRasterPos2s );
        API_CHECK( "opengl32.dll", _glRasterPos2sv );
        API_CHECK( "opengl32.dll", _glRasterPos3d );
        API_CHECK( "opengl32.dll", _glRasterPos3dv );
        API_CHECK( "opengl32.dll", _glRasterPos3f );
        API_CHECK( "opengl32.dll", _glRasterPos3fv );
        API_CHECK( "opengl32.dll", _glRasterPos3i );
        API_CHECK( "opengl32.dll", _glRasterPos3iv );
        API_CHECK( "opengl32.dll", _glRasterPos3s );
        API_CHECK( "opengl32.dll", _glRasterPos3sv );
        API_CHECK( "opengl32.dll", _glRasterPos4d );
        API_CHECK( "opengl32.dll", _glRasterPos4dv );
        API_CHECK( "opengl32.dll", _glRasterPos4f );
        API_CHECK( "opengl32.dll", _glRasterPos4fv );
        API_CHECK( "opengl32.dll", _glRasterPos4i );
        API_CHECK( "opengl32.dll", _glRasterPos4iv );
        API_CHECK( "opengl32.dll", _glRasterPos4s );
        API_CHECK( "opengl32.dll", _glRasterPos4sv );
        API_CHECK( "opengl32.dll", _glReadBuffer );
        API_CHECK( "opengl32.dll", _glReadPixels );
        API_CHECK( "opengl32.dll", _glRectd );
        API_CHECK( "opengl32.dll", _glRectdv );
        API_CHECK( "opengl32.dll", _glRectf );
        API_CHECK( "opengl32.dll", _glRectfv );
        API_CHECK( "opengl32.dll", _glRecti );
        API_CHECK( "opengl32.dll", _glRectiv );
        API_CHECK( "opengl32.dll", _glRects );
        API_CHECK( "opengl32.dll", _glRectsv );
        API_CHECK( "opengl32.dll", _glRenderMode );
        API_CHECK( "opengl32.dll", _glRotated );
        API_CHECK( "opengl32.dll", _glRotatef );
        API_CHECK( "opengl32.dll", _glScaled );
        API_CHECK( "opengl32.dll", _glScalef );
        API_CHECK( "opengl32.dll", _glScissor );
        API_CHECK( "opengl32.dll", _glSelectBuffer );
        API_CHECK( "opengl32.dll", _glShadeModel );
        API_CHECK( "opengl32.dll", _glStencilFunc );
        API_CHECK( "opengl32.dll", _glStencilMask );
        API_CHECK( "opengl32.dll", _glStencilOp );
        API_CHECK( "opengl32.dll", _glTexCoord1d );
        API_CHECK( "opengl32.dll", _glTexCoord1dv );
        API_CHECK( "opengl32.dll", _glTexCoord1f );
        API_CHECK( "opengl32.dll", _glTexCoord1fv );
        API_CHECK( "opengl32.dll", _glTexCoord1i );
        API_CHECK( "opengl32.dll", _glTexCoord1iv );
        API_CHECK( "opengl32.dll", _glTexCoord1s );
        API_CHECK( "opengl32.dll", _glTexCoord1sv );
        API_CHECK( "opengl32.dll", _glTexCoord2d );
        API_CHECK( "opengl32.dll", _glTexCoord2dv );
        API_CHECK( "opengl32.dll", _glTexCoord2f );
        API_CHECK( "opengl32.dll", _glTexCoord2fv );
        API_CHECK( "opengl32.dll", _glTexCoord2i );
        API_CHECK( "opengl32.dll", _glTexCoord2iv );
        API_CHECK( "opengl32.dll", _glTexCoord2s );
        API_CHECK( "opengl32.dll", _glTexCoord2sv );
        API_CHECK( "opengl32.dll", _glTexCoord3d );
        API_CHECK( "opengl32.dll", _glTexCoord3dv );
        API_CHECK( "opengl32.dll", _glTexCoord3f );
        API_CHECK( "opengl32.dll", _glTexCoord3fv );
        API_CHECK( "opengl32.dll", _glTexCoord3i );
        API_CHECK( "opengl32.dll", _glTexCoord3iv );
        API_CHECK( "opengl32.dll", _glTexCoord3s );
        API_CHECK( "opengl32.dll", _glTexCoord3sv );
        API_CHECK( "opengl32.dll", _glTexCoord4d );
        API_CHECK( "opengl32.dll", _glTexCoord4dv );
        API_CHECK( "opengl32.dll", _glTexCoord4f );
        API_CHECK( "opengl32.dll", _glTexCoord4fv );
        API_CHECK( "opengl32.dll", _glTexCoord4i );
        API_CHECK( "opengl32.dll", _glTexCoord4iv );
        API_CHECK( "opengl32.dll", _glTexCoord4s );
        API_CHECK( "opengl32.dll", _glTexCoord4sv );
        API_CHECK( "opengl32.dll", _glTexCoordPointer );
        API_CHECK( "opengl32.dll", _glTexEnvf );
        API_CHECK( "opengl32.dll", _glTexEnvfv );
        API_CHECK( "opengl32.dll", _glTexEnvi );
        API_CHECK( "opengl32.dll", _glTexEnviv );
        API_CHECK( "opengl32.dll", _glTexGend );
        API_CHECK( "opengl32.dll", _glTexGendv );
        API_CHECK( "opengl32.dll", _glTexGenf );
        API_CHECK( "opengl32.dll", _glTexGenfv );
        API_CHECK( "opengl32.dll", _glTexGeni );
        API_CHECK( "opengl32.dll", _glTexGeniv );
        API_CHECK( "opengl32.dll", _glTexImage1D );
        API_CHECK( "opengl32.dll", _glTexImage2D );
        API_CHECK( "opengl32.dll", _glTexParameterf );
        API_CHECK( "opengl32.dll", _glTexParameterfv );
        API_CHECK( "opengl32.dll", _glTexParameteri );
        API_CHECK( "opengl32.dll", _glTexParameteriv );
        API_CHECK( "opengl32.dll", _glTexSubImage1D );
        API_CHECK( "opengl32.dll", _glTexSubImage2D );
        API_CHECK( "opengl32.dll", _glTranslated );
        API_CHECK( "opengl32.dll", _glTranslatef );
        API_CHECK( "opengl32.dll", _glVertex2d );
        API_CHECK( "opengl32.dll", _glVertex2dv );
        API_CHECK( "opengl32.dll", _glVertex2f );
        API_CHECK( "opengl32.dll", _glVertex2fv );
        API_CHECK( "opengl32.dll", _glVertex2i );
        API_CHECK( "opengl32.dll", _glVertex2iv );
        API_CHECK( "opengl32.dll", _glVertex2s );
        API_CHECK( "opengl32.dll", _glVertex2sv );
        API_CHECK( "opengl32.dll", _glVertex3d );
        API_CHECK( "opengl32.dll", _glVertex3dv );
        API_CHECK( "opengl32.dll", _glVertex3f );
        API_CHECK( "opengl32.dll", _glVertex3fv );
        API_CHECK( "opengl32.dll", _glVertex3i );
        API_CHECK( "opengl32.dll", _glVertex3iv );
        API_CHECK( "opengl32.dll", _glVertex3s );
        API_CHECK( "opengl32.dll", _glVertex3sv );
        API_CHECK( "opengl32.dll", _glVertex4d );
        API_CHECK( "opengl32.dll", _glVertex4dv );
        API_CHECK( "opengl32.dll", _glVertex4f );
        API_CHECK( "opengl32.dll", _glVertex4fv );
        API_CHECK( "opengl32.dll", _glVertex4i );
        API_CHECK( "opengl32.dll", _glVertex4iv );
        API_CHECK( "opengl32.dll", _glVertex4s );
        API_CHECK( "opengl32.dll", _glVertex4sv );
        API_CHECK( "opengl32.dll", _glVertexPointer );
        API_CHECK( "opengl32.dll", _glViewport );

        if ( apiFatal )
        {
            printf( "failed to load 'opengl32.dll'\n\n" );

            FreeLibrary( glMainLibrary );
            glMainLibrary = NULL;
        }
    }

    // initialize the list of all active OpenGL devices
    LIST_CLEAR( allDevices.root );
}

void luagl_shutdownDrivers( void )
{
    if ( glMainLibrary )
    {
        FreeLibrary( glMainLibrary );
        glMainLibrary = NULL;
    }
}

void luagl_pulseDrivers( lua_State *L )
{
    // Frame the drivers
    LIST_FOREACH_BEGIN( glDriver, allDevices.root, rootNode )
        item->PushMethod( L, "triggerEvent" );
        lua_pushcstring( L, "onFrame" );
        lua_call( L, 1, 0 );
    LIST_FOREACH_END
}