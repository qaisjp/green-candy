/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_func.h
*  PURPOSE:     OpenGL driver function typedefs
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _OPENGL_FUNCTION_TYPEDEFS_
#define _OPENGL_FUNCTION_TYPEDEFS_

#ifdef _WIN32
// Windows management specific
typedef BOOL    (WINAPI*wglCopyContext_t)           ( HGLRC src, HGLRC dst, UINT mask );
typedef HGLRC   (WINAPI*wglCreateContext_t)         ( HDC dc );
typedef HGLRC   (WINAPI*wglCreateLayerContext_t)    ( HDC dc, int iLayerPlane );
typedef BOOL    (WINAPI*wglDeleteContext_t)         ( HGLRC glrc );
typedef HGLRC   (WINAPI*wglGetCurrentContext_t)     ( void );
typedef HDC     (WINAPI*wglGetCurrentDC_t)          ( void );
typedef PROC    (WINAPI*wglGetProcAddress_t)        ( LPCSTR name );
typedef BOOL    (WINAPI*wglMakeCurrent_t)           ( HDC dc, HGLRC rc );
typedef BOOL    (WINAPI*wglShareLists_t)            ( HGLRC dst, HGLRC src );
typedef BOOL    (WINAPI*wglUseFontBitmapsA_t)       ( HGLRC rc, DWORD first, DWORD count, DWORD listBase );
typedef BOOL    (WINAPI*wglUseFontBitmapsW_t)       ( HGLRC rc, DWORD first, DWORD count, DWORD listBase );
#endif //_WIN32

// OpenGL standard functions
typedef void (APIENTRY*glAccum_t) (GLenum op, GLfloat value);
typedef void (APIENTRY*glAlphaFunc_t) (GLenum func, GLclampf ref);
typedef GLboolean (APIENTRY*glAreTexturesResident_t) (GLsizei n, const GLuint *textures, GLboolean *residences);
typedef void (APIENTRY*glArrayElement_t) (GLint i);
typedef void (APIENTRY*glBegin_t) (GLenum mode);
typedef void (APIENTRY*glBindTexture_t) (GLenum target, GLuint texture);
typedef void (APIENTRY*glBitmap_t) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
typedef void (APIENTRY*glBlendFunc_t) (GLenum sfactor, GLenum dfactor);
typedef void (APIENTRY*glCallList_t) (GLuint list);
typedef void (APIENTRY*glCallLists_t) (GLsizei n, GLenum type, const GLvoid *lists);
typedef void (APIENTRY*glClear_t) (GLbitfield mask);
typedef void (APIENTRY*glClearAccum_t) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRY*glClearColor_t) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (APIENTRY*glClearDepth_t) (GLclampd depth);
typedef void (APIENTRY*glClearIndex_t) (GLfloat c);
typedef void (APIENTRY*glClearStencil_t) (GLint s);
typedef void (APIENTRY*glClipPlane_t) (GLenum plane, const GLdouble *equation);
typedef void (APIENTRY*glColor3b_t) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (APIENTRY*glColor3bv_t) (const GLbyte *v);
typedef void (APIENTRY*glColor3d_t) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (APIENTRY*glColor3dv_t) (const GLdouble *v);
typedef void (APIENTRY*glColor3f_t) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (APIENTRY*glColor3fv_t) (const GLfloat *v);
typedef void (APIENTRY*glColor3i_t) (GLint red, GLint green, GLint blue);
typedef void (APIENTRY*glColor3iv_t) (const GLint *v);
typedef void (APIENTRY*glColor3s_t) (GLshort red, GLshort green, GLshort blue);
typedef void (APIENTRY*glColor3sv_t) (const GLshort *v);
typedef void (APIENTRY*glColor3ub_t) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (APIENTRY*glColor3ubv_t) (const GLubyte *v);
typedef void (APIENTRY*glColor3ui_t) (GLuint red, GLuint green, GLuint blue);
typedef void (APIENTRY*glColor3uiv_t) (const GLuint *v);
typedef void (APIENTRY*glColor3us_t) (GLushort red, GLushort green, GLushort blue);
typedef void (APIENTRY*glColor3usv_t) (const GLushort *v);
typedef void (APIENTRY*glColor4b_t) (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
typedef void (APIENTRY*glColor4bv_t) (const GLbyte *v);
typedef void (APIENTRY*glColor4d_t) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
typedef void (APIENTRY*glColor4dv_t) (const GLdouble *v);
typedef void (APIENTRY*glColor4f_t) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRY*glColor4fv_t) (const GLfloat *v);
typedef void (APIENTRY*glColor4i_t) (GLint red, GLint green, GLint blue, GLint alpha);
typedef void (APIENTRY*glColor4iv_t) (const GLint *v);
typedef void (APIENTRY*glColor4s_t) (GLshort red, GLshort green, GLshort blue, GLshort alpha);
typedef void (APIENTRY*glColor4sv_t) (const GLshort *v);
typedef void (APIENTRY*glColor4ub_t) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void (APIENTRY*glColor4ubv_t) (const GLubyte *v);
typedef void (APIENTRY*glColor4ui_t) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
typedef void (APIENTRY*glColor4uiv_t) (const GLuint *v);
typedef void (APIENTRY*glColor4us_t) (GLushort red, GLushort green, GLushort blue, GLushort alpha);
typedef void (APIENTRY*glColor4usv_t) (const GLushort *v);
typedef void (APIENTRY*glColorMask_t) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (APIENTRY*glColorMaterial_t) (GLenum face, GLenum mode);
typedef void (APIENTRY*glColorPointer_t) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY*glCopyPixels_t) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
typedef void (APIENTRY*glCopyTexImage1D_t) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (APIENTRY*glCopyTexImage2D_t) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (APIENTRY*glCopyTexSubImage1D_t) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (APIENTRY*glCopyTexSubImage2D_t) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY*glCullFace_t) (GLenum mode);
typedef void (APIENTRY*glDeleteLists_t) (GLuint list, GLsizei range);
typedef void (APIENTRY*glDeleteTextures_t) (GLsizei n, const GLuint *textures);
typedef void (APIENTRY*glDepthFunc_t) (GLenum func);
typedef void (APIENTRY*glDepthMask_t) (GLboolean flag);
typedef void (APIENTRY*glDepthRange_t) (GLclampd zNear, GLclampd zFar);
typedef void (APIENTRY*glDisable_t) (GLenum cap);
typedef void (APIENTRY*glDisableClientState_t) (GLenum array);
typedef void (APIENTRY*glDrawArrays_t) (GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRY*glDrawBuffer_t) (GLenum mode);
typedef void (APIENTRY*glDrawElements_t) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (APIENTRY*glDrawPixels_t) (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY*glEdgeFlag_t) (GLboolean flag);
typedef void (APIENTRY*glEdgeFlagPointer_t) (GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY*glEdgeFlagv_t) (const GLboolean *flag);
typedef void (APIENTRY*glEnable_t) (GLenum cap);
typedef void (APIENTRY*glEnableClientState_t) (GLenum array);
typedef void (APIENTRY*glEnd_t) (void);
typedef void (APIENTRY*glEndList_t) (void);
typedef void (APIENTRY*glEvalCoord1d_t) (GLdouble u);
typedef void (APIENTRY*glEvalCoord1dv_t) (const GLdouble *u);
typedef void (APIENTRY*glEvalCoord1f_t) (GLfloat u);
typedef void (APIENTRY*glEvalCoord1fv_t) (const GLfloat *u);
typedef void (APIENTRY*glEvalCoord2d_t) (GLdouble u, GLdouble v);
typedef void (APIENTRY*glEvalCoord2dv_t) (const GLdouble *u);
typedef void (APIENTRY*glEvalCoord2f_t) (GLfloat u, GLfloat v);
typedef void (APIENTRY*glEvalCoord2fv_t) (const GLfloat *u);
typedef void (APIENTRY*glEvalMesh1_t) (GLenum mode, GLint i1, GLint i2);
typedef void (APIENTRY*glEvalMesh2_t) (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
typedef void (APIENTRY*glEvalPoint1_t) (GLint i);
typedef void (APIENTRY*glEvalPoint2_t) (GLint i, GLint j);
typedef void (APIENTRY*glFeedbackBuffer_t) (GLsizei size, GLenum type, GLfloat *buffer);
typedef void (APIENTRY*glFinish_t) (void);
typedef void (APIENTRY*glFlush_t) (void);
typedef void (APIENTRY*glFogf_t) (GLenum pname, GLfloat param);
typedef void (APIENTRY*glFogfv_t) (GLenum pname, const GLfloat *params);
typedef void (APIENTRY*glFogi_t) (GLenum pname, GLint param);
typedef void (APIENTRY*glFogiv_t) (GLenum pname, const GLint *params);
typedef void (APIENTRY*glFrontFace_t) (GLenum mode);
typedef void (APIENTRY*glFrustum_t) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef GLuint (APIENTRY*glGenLists_t) (GLsizei range);
typedef void (APIENTRY*glGenTextures_t) (GLsizei n, GLuint *textures);
typedef void (APIENTRY*glGetBooleanv_t) (GLenum pname, GLboolean *params);
typedef void (APIENTRY*glGetClipPlane_t) (GLenum plane, GLdouble *equation);
typedef void (APIENTRY*glGetDoublev_t) (GLenum pname, GLdouble *params);
typedef GLenum (APIENTRY*glGetError_t) (void);
typedef void (APIENTRY*glGetFloatv_t) (GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetIntegerv_t) (GLenum pname, GLint *params);
typedef void (APIENTRY*glGetLightfv_t) (GLenum light, GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetLightiv_t) (GLenum light, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetMapdv_t) (GLenum target, GLenum query, GLdouble *v);
typedef void (APIENTRY*glGetMapfv_t) (GLenum target, GLenum query, GLfloat *v);
typedef void (APIENTRY*glGetMapiv_t) (GLenum target, GLenum query, GLint *v);
typedef void (APIENTRY*glGetMaterialfv_t) (GLenum face, GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetMaterialiv_t) (GLenum face, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetPixelMapfv_t) (GLenum map, GLfloat *values);
typedef void (APIENTRY*glGetPixelMapuiv_t) (GLenum map, GLuint *values);
typedef void (APIENTRY*glGetPixelMapusv_t) (GLenum map, GLushort *values);
typedef void (APIENTRY*glGetPointerv_t) (GLenum pname, GLvoid* *params);
typedef void (APIENTRY*glGetPolygonStipple_t) (GLubyte *mask);
typedef const GLubyte* (APIENTRY*glGetString_t) (GLenum name);
typedef void (APIENTRY*glGetTexEnvfv_t) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetTexEnviv_t) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetTexGendv_t) (GLenum coord, GLenum pname, GLdouble *params);
typedef void (APIENTRY*glGetTexGenfv_t) (GLenum coord, GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetTexGeniv_t) (GLenum coord, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetTexImage_t) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
typedef void (APIENTRY*glGetTexLevelParameterfv_t) (GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetTexLevelParameteriv_t) (GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetTexParameterfv_t) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetTexParameteriv_t) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY*glHint_t) (GLenum target, GLenum mode);
typedef void (APIENTRY*glIndexMask_t) (GLuint mask);
typedef void (APIENTRY*glIndexPointer_t) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY*glIndexd_t) (GLdouble c);
typedef void (APIENTRY*glIndexdv_t) (const GLdouble *c);
typedef void (APIENTRY*glIndexf_t) (GLfloat c);
typedef void (APIENTRY*glIndexfv_t) (const GLfloat *c);
typedef void (APIENTRY*glIndexi_t) (GLint c);
typedef void (APIENTRY*glIndexiv_t) (const GLint *c);
typedef void (APIENTRY*glIndexs_t) (GLshort c);
typedef void (APIENTRY*glIndexsv_t) (const GLshort *c);
typedef void (APIENTRY*glIndexub_t) (GLubyte c);
typedef void (APIENTRY*glIndexubv_t) (const GLubyte *c);
typedef void (APIENTRY*glInitNames_t) (void);
typedef void (APIENTRY*glInterleavedArrays_t) (GLenum format, GLsizei stride, const GLvoid *pointer);
typedef GLboolean (APIENTRY*glIsEnabled_t) (GLenum cap);
typedef GLboolean (APIENTRY*glIsList_t) (GLuint list);
typedef GLboolean (APIENTRY*glIsTexture_t) (GLuint texture);
typedef void (APIENTRY*glLightModelf_t) (GLenum pname, GLfloat param);
typedef void (APIENTRY*glLightModelfv_t) (GLenum pname, const GLfloat *params);
typedef void (APIENTRY*glLightModeli_t) (GLenum pname, GLint param);
typedef void (APIENTRY*glLightModeliv_t) (GLenum pname, const GLint *params);
typedef void (APIENTRY*glLightf_t) (GLenum light, GLenum pname, GLfloat param);
typedef void (APIENTRY*glLightfv_t) (GLenum light, GLenum pname, const GLfloat *params);
typedef void (APIENTRY*glLighti_t) (GLenum light, GLenum pname, GLint param);
typedef void (APIENTRY*glLightiv_t) (GLenum light, GLenum pname, const GLint *params);
typedef void (APIENTRY*glLineStipple_t) (GLint factor, GLushort pattern);
typedef void (APIENTRY*glLineWidth_t) (GLfloat width);
typedef void (APIENTRY*glListBase_t) (GLuint base);
typedef void (APIENTRY*glLoadIdentity_t) (void);
typedef void (APIENTRY*glLoadMatrixd_t) (const GLdouble *m);
typedef void (APIENTRY*glLoadMatrixf_t) (const GLfloat *m);
typedef void (APIENTRY*glLoadName_t) (GLuint name);
typedef void (APIENTRY*glLogicOp_t) (GLenum opcode);
typedef void (APIENTRY*glMap1d_t) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
typedef void (APIENTRY*glMap1f_t) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
typedef void (APIENTRY*glMap2d_t) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
typedef void (APIENTRY*glMap2f_t) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
typedef void (APIENTRY*glMapGrid1d_t) (GLint un, GLdouble u1, GLdouble u2);
typedef void (APIENTRY*glMapGrid1f_t) (GLint un, GLfloat u1, GLfloat u2);
typedef void (APIENTRY*glMapGrid2d_t) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
typedef void (APIENTRY*glMapGrid2f_t) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
typedef void (APIENTRY*glMaterialf_t) (GLenum face, GLenum pname, GLfloat param);
typedef void (APIENTRY*glMaterialfv_t) (GLenum face, GLenum pname, const GLfloat *params);
typedef void (APIENTRY*glMateriali_t) (GLenum face, GLenum pname, GLint param);
typedef void (APIENTRY*glMaterialiv_t) (GLenum face, GLenum pname, const GLint *params);
typedef void (APIENTRY*glMatrixMode_t) (GLenum mode);
typedef void (APIENTRY*glMultMatrixd_t) (const GLdouble *m);
typedef void (APIENTRY*glMultMatrixf_t) (const GLfloat *m);
typedef void (APIENTRY*glNewList_t) (GLuint list, GLenum mode);
typedef void (APIENTRY*glNormal3b_t) (GLbyte nx, GLbyte ny, GLbyte nz);
typedef void (APIENTRY*glNormal3bv_t) (const GLbyte *v);
typedef void (APIENTRY*glNormal3d_t) (GLdouble nx, GLdouble ny, GLdouble nz);
typedef void (APIENTRY*glNormal3dv_t) (const GLdouble *v);
typedef void (APIENTRY*glNormal3f_t) (GLfloat nx, GLfloat ny, GLfloat nz);
typedef void (APIENTRY*glNormal3fv_t) (const GLfloat *v);
typedef void (APIENTRY*glNormal3i_t) (GLint nx, GLint ny, GLint nz);
typedef void (APIENTRY*glNormal3iv_t) (const GLint *v);
typedef void (APIENTRY*glNormal3s_t) (GLshort nx, GLshort ny, GLshort nz);
typedef void (APIENTRY*glNormal3sv_t) (const GLshort *v);
typedef void (APIENTRY*glNormalPointer_t) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY*glOrtho_t) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef void (APIENTRY*glPassThrough_t) (GLfloat token);
typedef void (APIENTRY*glPixelMapfv_t) (GLenum map, GLsizei mapsize, const GLfloat *values);
typedef void (APIENTRY*glPixelMapuiv_t) (GLenum map, GLsizei mapsize, const GLuint *values);
typedef void (APIENTRY*glPixelMapusv_t) (GLenum map, GLsizei mapsize, const GLushort *values);
typedef void (APIENTRY*glPixelStoref_t) (GLenum pname, GLfloat param);
typedef void (APIENTRY*glPixelStorei_t) (GLenum pname, GLint param);
typedef void (APIENTRY*glPixelTransferf_t) (GLenum pname, GLfloat param);
typedef void (APIENTRY*glPixelTransferi_t) (GLenum pname, GLint param);
typedef void (APIENTRY*glPixelZoom_t) (GLfloat xfactor, GLfloat yfactor);
typedef void (APIENTRY*glPointSize_t) (GLfloat size);
typedef void (APIENTRY*glPolygonMode_t) (GLenum face, GLenum mode);
typedef void (APIENTRY*glPolygonOffset_t) (GLfloat factor, GLfloat units);
typedef void (APIENTRY*glPolygonStipple_t) (const GLubyte *mask);
typedef void (APIENTRY*glPopAttrib_t) (void);
typedef void (APIENTRY*glPopClientAttrib_t) (void);
typedef void (APIENTRY*glPopMatrix_t) (void);
typedef void (APIENTRY*glPopName_t) (void);
typedef void (APIENTRY*glPrioritizeTextures_t) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef void (APIENTRY*glPushAttrib_t) (GLbitfield mask);
typedef void (APIENTRY*glPushClientAttrib_t) (GLbitfield mask);
typedef void (APIENTRY*glPushMatrix_t) (void);
typedef void (APIENTRY*glPushName_t) (GLuint name);
typedef void (APIENTRY*glRasterPos2d_t) (GLdouble x, GLdouble y);
typedef void (APIENTRY*glRasterPos2dv_t) (const GLdouble *v);
typedef void (APIENTRY*glRasterPos2f_t) (GLfloat x, GLfloat y);
typedef void (APIENTRY*glRasterPos2fv_t) (const GLfloat *v);
typedef void (APIENTRY*glRasterPos2i_t) (GLint x, GLint y);
typedef void (APIENTRY*glRasterPos2iv_t) (const GLint *v);
typedef void (APIENTRY*glRasterPos2s_t) (GLshort x, GLshort y);
typedef void (APIENTRY*glRasterPos2sv_t) (const GLshort *v);
typedef void (APIENTRY*glRasterPos3d_t) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY*glRasterPos3dv_t) (const GLdouble *v);
typedef void (APIENTRY*glRasterPos3f_t) (GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY*glRasterPos3fv_t) (const GLfloat *v);
typedef void (APIENTRY*glRasterPos3i_t) (GLint x, GLint y, GLint z);
typedef void (APIENTRY*glRasterPos3iv_t) (const GLint *v);
typedef void (APIENTRY*glRasterPos3s_t) (GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY*glRasterPos3sv_t) (const GLshort *v);
typedef void (APIENTRY*glRasterPos4d_t) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY*glRasterPos4dv_t) (const GLdouble *v);
typedef void (APIENTRY*glRasterPos4f_t) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY*glRasterPos4fv_t) (const GLfloat *v);
typedef void (APIENTRY*glRasterPos4i_t) (GLint x, GLint y, GLint z, GLint w);
typedef void (APIENTRY*glRasterPos4iv_t) (const GLint *v);
typedef void (APIENTRY*glRasterPos4s_t) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY*glRasterPos4sv_t) (const GLshort *v);
typedef void (APIENTRY*glReadBuffer_t) (GLenum mode);
typedef void (APIENTRY*glReadPixels_t) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
typedef void (APIENTRY*glRectd_t) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
typedef void (APIENTRY*glRectdv_t) (const GLdouble *v1, const GLdouble *v2);
typedef void (APIENTRY*glRectf_t) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
typedef void (APIENTRY*glRectfv_t) (const GLfloat *v1, const GLfloat *v2);
typedef void (APIENTRY*glRecti_t) (GLint x1, GLint y1, GLint x2, GLint y2);
typedef void (APIENTRY*glRectiv_t) (const GLint *v1, const GLint *v2);
typedef void (APIENTRY*glRects_t) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
typedef void (APIENTRY*glRectsv_t) (const GLshort *v1, const GLshort *v2);
typedef GLint (APIENTRY*glRenderMode_t) (GLenum mode);
typedef void (APIENTRY*glRotated_t) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY*glRotatef_t) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY*glScaled_t) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY*glScalef_t) (GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY*glScissor_t) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY*glSelectBuffer_t) (GLsizei size, GLuint *buffer);
typedef void (APIENTRY*glShadeModel_t) (GLenum mode);
typedef void (APIENTRY*glStencilFunc_t) (GLenum func, GLint ref, GLuint mask);
typedef void (APIENTRY*glStencilMask_t) (GLuint mask);
typedef void (APIENTRY*glStencilOp_t) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void (APIENTRY*glTexCoord1d_t) (GLdouble s);
typedef void (APIENTRY*glTexCoord1dv_t) (const GLdouble *v);
typedef void (APIENTRY*glTexCoord1f_t) (GLfloat s);
typedef void (APIENTRY*glTexCoord1fv_t) (const GLfloat *v);
typedef void (APIENTRY*glTexCoord1i_t) (GLint s);
typedef void (APIENTRY*glTexCoord1iv_t) (const GLint *v);
typedef void (APIENTRY*glTexCoord1s_t) (GLshort s);
typedef void (APIENTRY*glTexCoord1sv_t) (const GLshort *v);
typedef void (APIENTRY*glTexCoord2d_t) (GLdouble s, GLdouble t);
typedef void (APIENTRY*glTexCoord2dv_t) (const GLdouble *v);
typedef void (APIENTRY*glTexCoord2f_t) (GLfloat s, GLfloat t);
typedef void (APIENTRY*glTexCoord2fv_t) (const GLfloat *v);
typedef void (APIENTRY*glTexCoord2i_t) (GLint s, GLint t);
typedef void (APIENTRY*glTexCoord2iv_t) (const GLint *v);
typedef void (APIENTRY*glTexCoord2s_t) (GLshort s, GLshort t);
typedef void (APIENTRY*glTexCoord2sv_t) (const GLshort *v);
typedef void (APIENTRY*glTexCoord3d_t) (GLdouble s, GLdouble t, GLdouble r);
typedef void (APIENTRY*glTexCoord3dv_t) (const GLdouble *v);
typedef void (APIENTRY*glTexCoord3f_t) (GLfloat s, GLfloat t, GLfloat r);
typedef void (APIENTRY*glTexCoord3fv_t) (const GLfloat *v);
typedef void (APIENTRY*glTexCoord3i_t) (GLint s, GLint t, GLint r);
typedef void (APIENTRY*glTexCoord3iv_t) (const GLint *v);
typedef void (APIENTRY*glTexCoord3s_t) (GLshort s, GLshort t, GLshort r);
typedef void (APIENTRY*glTexCoord3sv_t) (const GLshort *v);
typedef void (APIENTRY*glTexCoord4d_t) (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (APIENTRY*glTexCoord4dv_t) (const GLdouble *v);
typedef void (APIENTRY*glTexCoord4f_t) (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (APIENTRY*glTexCoord4fv_t) (const GLfloat *v);
typedef void (APIENTRY*glTexCoord4i_t) (GLint s, GLint t, GLint r, GLint q);
typedef void (APIENTRY*glTexCoord4iv_t) (const GLint *v);
typedef void (APIENTRY*glTexCoord4s_t) (GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (APIENTRY*glTexCoord4sv_t) (const GLshort *v);
typedef void (APIENTRY*glTexCoordPointer_t) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY*glTexEnvf_t) (GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRY*glTexEnvfv_t) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRY*glTexEnvi_t) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRY*glTexEnviv_t) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY*glTexGend_t) (GLenum coord, GLenum pname, GLdouble param);
typedef void (APIENTRY*glTexGendv_t) (GLenum coord, GLenum pname, const GLdouble *params);
typedef void (APIENTRY*glTexGenf_t) (GLenum coord, GLenum pname, GLfloat param);
typedef void (APIENTRY*glTexGenfv_t) (GLenum coord, GLenum pname, const GLfloat *params);
typedef void (APIENTRY*glTexGeni_t) (GLenum coord, GLenum pname, GLint param);
typedef void (APIENTRY*glTexGeniv_t) (GLenum coord, GLenum pname, const GLint *params);
typedef void (APIENTRY*glTexImage1D_t) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY*glTexImage2D_t) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY*glTexParameterf_t) (GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRY*glTexParameterfv_t) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRY*glTexParameteri_t) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRY*glTexParameteriv_t) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY*glTexSubImage1D_t) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY*glTexSubImage2D_t) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY*glTranslated_t) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY*glTranslatef_t) (GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY*glVertex2d_t) (GLdouble x, GLdouble y);
typedef void (APIENTRY*glVertex2dv_t) (const GLdouble *v);
typedef void (APIENTRY*glVertex2f_t) (GLfloat x, GLfloat y);
typedef void (APIENTRY*glVertex2fv_t) (const GLfloat *v);
typedef void (APIENTRY*glVertex2i_t) (GLint x, GLint y);
typedef void (APIENTRY*glVertex2iv_t) (const GLint *v);
typedef void (APIENTRY*glVertex2s_t) (GLshort x, GLshort y);
typedef void (APIENTRY*glVertex2sv_t) (const GLshort *v);
typedef void (APIENTRY*glVertex3d_t) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY*glVertex3dv_t) (const GLdouble *v);
typedef void (APIENTRY*glVertex3f_t) (GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY*glVertex3fv_t) (const GLfloat *v);
typedef void (APIENTRY*glVertex3i_t) (GLint x, GLint y, GLint z);
typedef void (APIENTRY*glVertex3iv_t) (const GLint *v);
typedef void (APIENTRY*glVertex3s_t) (GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY*glVertex3sv_t) (const GLshort *v);
typedef void (APIENTRY*glVertex4d_t) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY*glVertex4dv_t) (const GLdouble *v);
typedef void (APIENTRY*glVertex4f_t) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY*glVertex4fv_t) (const GLfloat *v);
typedef void (APIENTRY*glVertex4i_t) (GLint x, GLint y, GLint z, GLint w);
typedef void (APIENTRY*glVertex4iv_t) (const GLint *v);
typedef void (APIENTRY*glVertex4s_t) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY*glVertex4sv_t) (const GLshort *v);
typedef void (APIENTRY*glVertexPointer_t) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY*glViewport_t) (GLint x, GLint y, GLsizei width, GLsizei height);

// Prototype links to main library
extern wglCopyContext_t            _wglCopyContext;
extern wglCreateContext_t          _wglCreateContext;
extern wglCreateLayerContext_t     _wglCreateLayerContext;
extern wglDeleteContext_t          _wglDeleteContext;
extern wglGetCurrentContext_t      _wglGetCurrentContext;
extern wglGetCurrentDC_t           _wglGetCurrentDC;
extern wglGetProcAddress_t         _wglGetProcAddress;
extern wglMakeCurrent_t            _wglMakeCurrent;
extern wglShareLists_t             _wglShareLists;
extern wglUseFontBitmapsA_t        _wglUseFontBitmapsA;
extern wglUseFontBitmapsW_t        _wglUseFontBitmapsW;

// MACHINE GENERATED; USE gltools.lua !!!
extern glAccum_t                            _glAccum;
extern glAlphaFunc_t                        _glAlphaFunc;
extern glAreTexturesResident_t              _glAreTexturesResident;
extern glArrayElement_t                     _glArrayElement;
extern glBegin_t                            _glBegin;
extern glBindTexture_t                      _glBindTexture;
extern glBitmap_t                           _glBitmap;
extern glBlendFunc_t                        _glBlendFunc;
extern glCallList_t                         _glCallList;
extern glCallLists_t                        _glCallLists;
extern glClear_t                            _glClear;
extern glClearAccum_t                       _glClearAccum;
extern glClearColor_t                       _glClearColor;
extern glClearDepth_t                       _glClearDepth;
extern glClearIndex_t                       _glClearIndex;
extern glClearStencil_t                     _glClearStencil;
extern glClipPlane_t                        _glClipPlane;
extern glColor3b_t                          _glColor3b;
extern glColor3bv_t                         _glColor3bv;
extern glColor3d_t                          _glColor3d;
extern glColor3dv_t                         _glColor3dv;
extern glColor3f_t                          _glColor3f;
extern glColor3fv_t                         _glColor3fv;
extern glColor3i_t                          _glColor3i;
extern glColor3iv_t                         _glColor3iv;
extern glColor3s_t                          _glColor3s;
extern glColor3sv_t                         _glColor3sv;
extern glColor3ub_t                         _glColor3ub;
extern glColor3ubv_t                        _glColor3ubv;
extern glColor3ui_t                         _glColor3ui;
extern glColor3uiv_t                        _glColor3uiv;
extern glColor3us_t                         _glColor3us;
extern glColor3usv_t                        _glColor3usv;
extern glColor4b_t                          _glColor4b;
extern glColor4bv_t                         _glColor4bv;
extern glColor4d_t                          _glColor4d;
extern glColor4dv_t                         _glColor4dv;
extern glColor4f_t                          _glColor4f;
extern glColor4fv_t                         _glColor4fv;
extern glColor4i_t                          _glColor4i;
extern glColor4iv_t                         _glColor4iv;
extern glColor4s_t                          _glColor4s;
extern glColor4sv_t                         _glColor4sv;
extern glColor4ub_t                         _glColor4ub;
extern glColor4ubv_t                        _glColor4ubv;
extern glColor4ui_t                         _glColor4ui;
extern glColor4uiv_t                        _glColor4uiv;
extern glColor4us_t                         _glColor4us;
extern glColor4usv_t                        _glColor4usv;
extern glColorMask_t                        _glColorMask;
extern glColorMaterial_t                    _glColorMaterial;
extern glColorPointer_t                     _glColorPointer;
extern glCopyPixels_t                       _glCopyPixels;
extern glCopyTexImage1D_t                   _glCopyTexImage1D;
extern glCopyTexImage2D_t                   _glCopyTexImage2D;
extern glCopyTexSubImage1D_t                _glCopyTexSubImage1D;
extern glCopyTexSubImage2D_t                _glCopyTexSubImage2D;
extern glCullFace_t                         _glCullFace;
extern glDeleteLists_t                      _glDeleteLists;
extern glDeleteTextures_t                   _glDeleteTextures;
extern glDepthFunc_t                        _glDepthFunc;
extern glDepthMask_t                        _glDepthMask;
extern glDepthRange_t                       _glDepthRange;
extern glDisable_t                          _glDisable;
extern glDisableClientState_t               _glDisableClientState;
extern glDrawArrays_t                       _glDrawArrays;
extern glDrawBuffer_t                       _glDrawBuffer;
extern glDrawElements_t                     _glDrawElements;
extern glDrawPixels_t                       _glDrawPixels;
extern glEdgeFlag_t                         _glEdgeFlag;
extern glEdgeFlagPointer_t                  _glEdgeFlagPointer;
extern glEdgeFlagv_t                        _glEdgeFlagv;
extern glEnable_t                           _glEnable;
extern glEnableClientState_t                _glEnableClientState;
extern glEnd_t                              _glEnd;
extern glEndList_t                          _glEndList;
extern glEvalCoord1d_t                      _glEvalCoord1d;
extern glEvalCoord1dv_t                     _glEvalCoord1dv;
extern glEvalCoord1f_t                      _glEvalCoord1f;
extern glEvalCoord1fv_t                     _glEvalCoord1fv;
extern glEvalCoord2d_t                      _glEvalCoord2d;
extern glEvalCoord2dv_t                     _glEvalCoord2dv;
extern glEvalCoord2f_t                      _glEvalCoord2f;
extern glEvalCoord2fv_t                     _glEvalCoord2fv;
extern glEvalMesh1_t                        _glEvalMesh1;
extern glEvalMesh2_t                        _glEvalMesh2;
extern glEvalPoint1_t                       _glEvalPoint1;
extern glEvalPoint2_t                       _glEvalPoint2;
extern glFeedbackBuffer_t                   _glFeedbackBuffer;
extern glFinish_t                           _glFinish;
extern glFlush_t                            _glFlush;
extern glFogf_t                             _glFogf;
extern glFogfv_t                            _glFogfv;
extern glFogi_t                             _glFogi;
extern glFogiv_t                            _glFogiv;
extern glFrontFace_t                        _glFrontFace;
extern glFrustum_t                          _glFrustum;
extern glGenLists_t                         _glGenLists;
extern glGenTextures_t                      _glGenTextures;
extern glGetBooleanv_t                      _glGetBooleanv;
extern glGetClipPlane_t                     _glGetClipPlane;
extern glGetDoublev_t                       _glGetDoublev;
extern glGetError_t                         _glGetError;
extern glGetFloatv_t                        _glGetFloatv;
extern glGetIntegerv_t                      _glGetIntegerv;
extern glGetLightfv_t                       _glGetLightfv;
extern glGetLightiv_t                       _glGetLightiv;
extern glGetMapdv_t                         _glGetMapdv;
extern glGetMapfv_t                         _glGetMapfv;
extern glGetMapiv_t                         _glGetMapiv;
extern glGetMaterialfv_t                    _glGetMaterialfv;
extern glGetMaterialiv_t                    _glGetMaterialiv;
extern glGetPixelMapfv_t                    _glGetPixelMapfv;
extern glGetPixelMapuiv_t                   _glGetPixelMapuiv;
extern glGetPixelMapusv_t                   _glGetPixelMapusv;
extern glGetPointerv_t                      _glGetPointerv;
extern glGetPolygonStipple_t                _glGetPolygonStipple;
extern glGetString_t                        _glGetString;
extern glGetTexEnvfv_t                      _glGetTexEnvfv;
extern glGetTexEnviv_t                      _glGetTexEnviv;
extern glGetTexGendv_t                      _glGetTexGendv;
extern glGetTexGenfv_t                      _glGetTexGenfv;
extern glGetTexGeniv_t                      _glGetTexGeniv;
extern glGetTexImage_t                      _glGetTexImage;
extern glGetTexLevelParameterfv_t           _glGetTexLevelParameterfv;
extern glGetTexLevelParameteriv_t           _glGetTexLevelParameteriv;
extern glGetTexParameterfv_t                _glGetTexParameterfv;
extern glGetTexParameteriv_t                _glGetTexParameteriv;
extern glHint_t                             _glHint;
extern glIndexMask_t                        _glIndexMask;
extern glIndexPointer_t                     _glIndexPointer;
extern glIndexd_t                           _glIndexd;
extern glIndexdv_t                          _glIndexdv;
extern glIndexf_t                           _glIndexf;
extern glIndexfv_t                          _glIndexfv;
extern glIndexi_t                           _glIndexi;
extern glIndexiv_t                          _glIndexiv;
extern glIndexs_t                           _glIndexs;
extern glIndexsv_t                          _glIndexsv;
extern glIndexub_t                          _glIndexub;
extern glIndexubv_t                         _glIndexubv;
extern glInitNames_t                        _glInitNames;
extern glInterleavedArrays_t                _glInterleavedArrays;
extern glIsEnabled_t                        _glIsEnabled;
extern glIsList_t                           _glIsList;
extern glIsTexture_t                        _glIsTexture;
extern glLightModelf_t                      _glLightModelf;
extern glLightModelfv_t                     _glLightModelfv;
extern glLightModeli_t                      _glLightModeli;
extern glLightModeliv_t                     _glLightModeliv;
extern glLightf_t                           _glLightf;
extern glLightfv_t                          _glLightfv;
extern glLighti_t                           _glLighti;
extern glLightiv_t                          _glLightiv;
extern glLineStipple_t                      _glLineStipple;
extern glLineWidth_t                        _glLineWidth;
extern glListBase_t                         _glListBase;
extern glLoadIdentity_t                     _glLoadIdentity;
extern glLoadMatrixd_t                      _glLoadMatrixd;
extern glLoadMatrixf_t                      _glLoadMatrixf;
extern glLoadName_t                         _glLoadName;
extern glLogicOp_t                          _glLogicOp;
extern glMap1d_t                            _glMap1d;
extern glMap1f_t                            _glMap1f;
extern glMap2d_t                            _glMap2d;
extern glMap2f_t                            _glMap2f;
extern glMapGrid1d_t                        _glMapGrid1d;
extern glMapGrid1f_t                        _glMapGrid1f;
extern glMapGrid2d_t                        _glMapGrid2d;
extern glMapGrid2f_t                        _glMapGrid2f;
extern glMaterialf_t                        _glMaterialf;
extern glMaterialfv_t                       _glMaterialfv;
extern glMateriali_t                        _glMateriali;
extern glMaterialiv_t                       _glMaterialiv;
extern glMatrixMode_t                       _glMatrixMode;
extern glMultMatrixd_t                      _glMultMatrixd;
extern glMultMatrixf_t                      _glMultMatrixf;
extern glNewList_t                          _glNewList;
extern glNormal3b_t                         _glNormal3b;
extern glNormal3bv_t                        _glNormal3bv;
extern glNormal3d_t                         _glNormal3d;
extern glNormal3dv_t                        _glNormal3dv;
extern glNormal3f_t                         _glNormal3f;
extern glNormal3fv_t                        _glNormal3fv;
extern glNormal3i_t                         _glNormal3i;
extern glNormal3iv_t                        _glNormal3iv;
extern glNormal3s_t                         _glNormal3s;
extern glNormal3sv_t                        _glNormal3sv;
extern glNormalPointer_t                    _glNormalPointer;
extern glOrtho_t                            _glOrtho;
extern glPassThrough_t                      _glPassThrough;
extern glPixelMapfv_t                       _glPixelMapfv;
extern glPixelMapuiv_t                      _glPixelMapuiv;
extern glPixelMapusv_t                      _glPixelMapusv;
extern glPixelStoref_t                      _glPixelStoref;
extern glPixelStorei_t                      _glPixelStorei;
extern glPixelTransferf_t                   _glPixelTransferf;
extern glPixelTransferi_t                   _glPixelTransferi;
extern glPixelZoom_t                        _glPixelZoom;
extern glPointSize_t                        _glPointSize;
extern glPolygonMode_t                      _glPolygonMode;
extern glPolygonOffset_t                    _glPolygonOffset;
extern glPolygonStipple_t                   _glPolygonStipple;
extern glPopAttrib_t                        _glPopAttrib;
extern glPopClientAttrib_t                  _glPopClientAttrib;
extern glPopMatrix_t                        _glPopMatrix;
extern glPopName_t                          _glPopName;
extern glPrioritizeTextures_t               _glPrioritizeTextures;
extern glPushAttrib_t                       _glPushAttrib;
extern glPushClientAttrib_t                 _glPushClientAttrib;
extern glPushMatrix_t                       _glPushMatrix;
extern glPushName_t                         _glPushName;
extern glRasterPos2d_t                      _glRasterPos2d;
extern glRasterPos2dv_t                     _glRasterPos2dv;
extern glRasterPos2f_t                      _glRasterPos2f;
extern glRasterPos2fv_t                     _glRasterPos2fv;
extern glRasterPos2i_t                      _glRasterPos2i;
extern glRasterPos2iv_t                     _glRasterPos2iv;
extern glRasterPos2s_t                      _glRasterPos2s;
extern glRasterPos2sv_t                     _glRasterPos2sv;
extern glRasterPos3d_t                      _glRasterPos3d;
extern glRasterPos3dv_t                     _glRasterPos3dv;
extern glRasterPos3f_t                      _glRasterPos3f;
extern glRasterPos3fv_t                     _glRasterPos3fv;
extern glRasterPos3i_t                      _glRasterPos3i;
extern glRasterPos3iv_t                     _glRasterPos3iv;
extern glRasterPos3s_t                      _glRasterPos3s;
extern glRasterPos3sv_t                     _glRasterPos3sv;
extern glRasterPos4d_t                      _glRasterPos4d;
extern glRasterPos4dv_t                     _glRasterPos4dv;
extern glRasterPos4f_t                      _glRasterPos4f;
extern glRasterPos4fv_t                     _glRasterPos4fv;
extern glRasterPos4i_t                      _glRasterPos4i;
extern glRasterPos4iv_t                     _glRasterPos4iv;
extern glRasterPos4s_t                      _glRasterPos4s;
extern glRasterPos4sv_t                     _glRasterPos4sv;
extern glReadBuffer_t                       _glReadBuffer;
extern glReadPixels_t                       _glReadPixels;
extern glRectd_t                            _glRectd;
extern glRectdv_t                           _glRectdv;
extern glRectf_t                            _glRectf;
extern glRectfv_t                           _glRectfv;
extern glRecti_t                            _glRecti;
extern glRectiv_t                           _glRectiv;
extern glRects_t                            _glRects;
extern glRectsv_t                           _glRectsv;
extern glRenderMode_t                       _glRenderMode;
extern glRotated_t                          _glRotated;
extern glRotatef_t                          _glRotatef;
extern glScaled_t                           _glScaled;
extern glScalef_t                           _glScalef;
extern glScissor_t                          _glScissor;
extern glSelectBuffer_t                     _glSelectBuffer;
extern glShadeModel_t                       _glShadeModel;
extern glStencilFunc_t                      _glStencilFunc;
extern glStencilMask_t                      _glStencilMask;
extern glStencilOp_t                        _glStencilOp;
extern glTexCoord1d_t                       _glTexCoord1d;
extern glTexCoord1dv_t                      _glTexCoord1dv;
extern glTexCoord1f_t                       _glTexCoord1f;
extern glTexCoord1fv_t                      _glTexCoord1fv;
extern glTexCoord1i_t                       _glTexCoord1i;
extern glTexCoord1iv_t                      _glTexCoord1iv;
extern glTexCoord1s_t                       _glTexCoord1s;
extern glTexCoord1sv_t                      _glTexCoord1sv;
extern glTexCoord2d_t                       _glTexCoord2d;
extern glTexCoord2dv_t                      _glTexCoord2dv;
extern glTexCoord2f_t                       _glTexCoord2f;
extern glTexCoord2fv_t                      _glTexCoord2fv;
extern glTexCoord2i_t                       _glTexCoord2i;
extern glTexCoord2iv_t                      _glTexCoord2iv;
extern glTexCoord2s_t                       _glTexCoord2s;
extern glTexCoord2sv_t                      _glTexCoord2sv;
extern glTexCoord3d_t                       _glTexCoord3d;
extern glTexCoord3dv_t                      _glTexCoord3dv;
extern glTexCoord3f_t                       _glTexCoord3f;
extern glTexCoord3fv_t                      _glTexCoord3fv;
extern glTexCoord3i_t                       _glTexCoord3i;
extern glTexCoord3iv_t                      _glTexCoord3iv;
extern glTexCoord3s_t                       _glTexCoord3s;
extern glTexCoord3sv_t                      _glTexCoord3sv;
extern glTexCoord4d_t                       _glTexCoord4d;
extern glTexCoord4dv_t                      _glTexCoord4dv;
extern glTexCoord4f_t                       _glTexCoord4f;
extern glTexCoord4fv_t                      _glTexCoord4fv;
extern glTexCoord4i_t                       _glTexCoord4i;
extern glTexCoord4iv_t                      _glTexCoord4iv;
extern glTexCoord4s_t                       _glTexCoord4s;
extern glTexCoord4sv_t                      _glTexCoord4sv;
extern glTexCoordPointer_t                  _glTexCoordPointer;
extern glTexEnvf_t                          _glTexEnvf;
extern glTexEnvfv_t                         _glTexEnvfv;
extern glTexEnvi_t                          _glTexEnvi;
extern glTexEnviv_t                         _glTexEnviv;
extern glTexGend_t                          _glTexGend;
extern glTexGendv_t                         _glTexGendv;
extern glTexGenf_t                          _glTexGenf;
extern glTexGenfv_t                         _glTexGenfv;
extern glTexGeni_t                          _glTexGeni;
extern glTexGeniv_t                         _glTexGeniv;
extern glTexImage1D_t                       _glTexImage1D;
extern glTexImage2D_t                       _glTexImage2D;
extern glTexParameterf_t                    _glTexParameterf;
extern glTexParameterfv_t                   _glTexParameterfv;
extern glTexParameteri_t                    _glTexParameteri;
extern glTexParameteriv_t                   _glTexParameteriv;
extern glTexSubImage1D_t                    _glTexSubImage1D;
extern glTexSubImage2D_t                    _glTexSubImage2D;
extern glTranslated_t                       _glTranslated;
extern glTranslatef_t                       _glTranslatef;
extern glVertex2d_t                         _glVertex2d;
extern glVertex2dv_t                        _glVertex2dv;
extern glVertex2f_t                         _glVertex2f;
extern glVertex2fv_t                        _glVertex2fv;
extern glVertex2i_t                         _glVertex2i;
extern glVertex2iv_t                        _glVertex2iv;
extern glVertex2s_t                         _glVertex2s;
extern glVertex2sv_t                        _glVertex2sv;
extern glVertex3d_t                         _glVertex3d;
extern glVertex3dv_t                        _glVertex3dv;
extern glVertex3f_t                         _glVertex3f;
extern glVertex3fv_t                        _glVertex3fv;
extern glVertex3i_t                         _glVertex3i;
extern glVertex3iv_t                        _glVertex3iv;
extern glVertex3s_t                         _glVertex3s;
extern glVertex3sv_t                        _glVertex3sv;
extern glVertex4d_t                         _glVertex4d;
extern glVertex4dv_t                        _glVertex4dv;
extern glVertex4f_t                         _glVertex4f;
extern glVertex4fv_t                        _glVertex4fv;
extern glVertex4i_t                         _glVertex4i;
extern glVertex4iv_t                        _glVertex4iv;
extern glVertex4s_t                         _glVertex4s;
extern glVertex4sv_t                        _glVertex4sv;
extern glVertexPointer_t                    _glVertexPointer;
extern glViewport_t                         _glViewport;

// glut library
typedef const GLubyte* (APIENTRY*gluErrorString_t) (
    GLenum   errCode);

typedef const wchar_t* (APIENTRY*gluErrorUnicodeStringEXT_t) (
    GLenum   errCode);

typedef const GLubyte* (APIENTRY*gluGetString_t) (
    GLenum   name);

typedef void (APIENTRY*gluOrtho2D_t) (
    GLdouble left, 
    GLdouble right, 
    GLdouble bottom, 
    GLdouble top);

typedef void (APIENTRY*gluPerspective_t) (
    GLdouble fovy, 
    GLdouble aspect, 
    GLdouble zNear, 
    GLdouble zFar);

typedef void (APIENTRY*gluPickMatrix_t) (
    GLdouble x, 
    GLdouble y, 
    GLdouble width, 
    GLdouble height, 
    GLint    viewport[4]);

typedef void (APIENTRY*gluLookAt_t) (
    GLdouble eyex, 
    GLdouble eyey, 
    GLdouble eyez, 
    GLdouble centerx, 
    GLdouble centery, 
    GLdouble centerz, 
    GLdouble upx, 
    GLdouble upy, 
    GLdouble upz);

typedef int (APIENTRY*gluProject_t) (
    GLdouble        objx, 
    GLdouble        objy, 
    GLdouble        objz,  
    const GLdouble  modelMatrix[16], 
    const GLdouble  projMatrix[16], 
    const GLint     viewport[4], 
    GLdouble        *winx, 
    GLdouble        *winy, 
    GLdouble        *winz);

typedef int (APIENTRY*gluUnProject_t) (
    GLdouble       winx, 
    GLdouble       winy, 
    GLdouble       winz, 
    const GLdouble modelMatrix[16], 
    const GLdouble projMatrix[16], 
    const GLint    viewport[4], 
    GLdouble       *objx, 
    GLdouble       *objy, 
    GLdouble       *objz);


typedef int (APIENTRY*gluScaleImage_t) (
    GLenum      format, 
    GLint       widthin, 
    GLint       heightin, 
    GLenum      typein, 
    const void  *datain, 
    GLint       widthout, 
    GLint       heightout, 
    GLenum      typeout, 
    void        *dataout);


typedef int (APIENTRY*gluBuild1DMipmaps_t) (
    GLenum      target, 
    GLint       components, 
    GLint       width, 
    GLenum      format, 
    GLenum      type, 
    const void  *data);

typedef int (APIENTRY*gluBuild2DMipmaps_t) (
    GLenum      target, 
    GLint       components, 
    GLint       width, 
    GLint       height, 
    GLenum      format, 
    GLenum      type, 
    const void  *data);

typedef GLUquadric* (APIENTRY*gluNewQuadric_t) (void);
typedef void (APIENTRY*gluDeleteQuadric_t) (
    GLUquadric          *state);

typedef void (APIENTRY*gluQuadricNormals_t) (
    GLUquadric          *quadObject, 
    GLenum              normals);

typedef void (APIENTRY*gluQuadricTexture_t) (
    GLUquadric          *quadObject, 
    GLboolean           textureCoords);

typedef void (APIENTRY*gluQuadricOrientation_t) (
    GLUquadric          *quadObject, 
    GLenum              orientation);

typedef void (APIENTRY*gluQuadricDrawStyle_t) (
    GLUquadric          *quadObject, 
    GLenum              drawStyle);

typedef void (APIENTRY*gluCylinder_t) (
    GLUquadric          *qobj, 
    GLdouble            baseRadius, 
    GLdouble            topRadius, 
    GLdouble            height, 
    GLint               slices, 
    GLint               stacks);

typedef void (APIENTRY*gluDisk_t) (
    GLUquadric          *qobj, 
    GLdouble            innerRadius, 
    GLdouble            outerRadius, 
    GLint               slices, 
    GLint               loops);

typedef void (APIENTRY*gluPartialDisk_t) (
    GLUquadric          *qobj, 
    GLdouble            innerRadius, 
    GLdouble            outerRadius, 
    GLint               slices, 
    GLint               loops, 
    GLdouble            startAngle, 
    GLdouble            sweepAngle);

typedef void (APIENTRY*gluSphere_t) (
    GLUquadric          *qobj, 
    GLdouble            radius, 
    GLint               slices, 
    GLint               stacks);

typedef void (APIENTRY*gluQuadricCallback_t) (
    GLUquadric          *qobj, 
    GLenum              which, 
    void                (CALLBACK* fn)());

typedef GLUtesselator* (APIENTRY*gluNewTess_t)(          
    void );

typedef void (APIENTRY*gluDeleteTess_t) (       
    GLUtesselator       *tess );

typedef void (APIENTRY*gluTessBeginPolygon_t) ( 
    GLUtesselator       *tess,
    void                *polygon_data );

typedef void (APIENTRY*gluTessBeginContour_t) ( 
    GLUtesselator       *tess );

typedef void (APIENTRY*gluTessVertex_t) (       
    GLUtesselator       *tess,
    GLdouble            coords[3], 
    void                *data );

typedef void (APIENTRY*gluTessEndContour_t) (   
    GLUtesselator       *tess );

typedef void (APIENTRY*gluTessEndPolygon_t) (   
    GLUtesselator       *tess );

typedef void (APIENTRY*gluTessProperty_t) (     
    GLUtesselator       *tess,
    GLenum              which, 
    GLdouble            value );
 
typedef void (APIENTRY*gluTessNormal_t) (       
    GLUtesselator       *tess, 
    GLdouble            x,
    GLdouble            y, 
    GLdouble            z );

typedef void (APIENTRY*gluTessCallback_t) (     
    GLUtesselator       *tess,
    GLenum              which, 
    void                (CALLBACK *fn)());

typedef void (APIENTRY*gluGetTessProperty_t) (  
    GLUtesselator       *tess,
    GLenum              which, 
    GLdouble            *value );
 
typedef GLUnurbs* (APIENTRY*gluNewNurbsRenderer_t) (void);

typedef void (APIENTRY*gluDeleteNurbsRenderer_t) (
    GLUnurbs            *nobj);

typedef void (APIENTRY*gluBeginSurface_t) (
    GLUnurbs            *nobj);

typedef void (APIENTRY*gluBeginCurve_t) (
    GLUnurbs            *nobj);

typedef void (APIENTRY*gluEndCurve_t) (
    GLUnurbs            *nobj);

typedef void (APIENTRY*gluEndSurface_t) (
    GLUnurbs            *nobj);

typedef void (APIENTRY*gluBeginTrim_t) (
    GLUnurbs            *nobj);

typedef void (APIENTRY*gluEndTrim_t) (
    GLUnurbs            *nobj);

typedef void (APIENTRY*gluPwlCurve_t) (
    GLUnurbs            *nobj, 
    GLint               count, 
    GLfloat             *array, 
    GLint               stride, 
    GLenum              type);

typedef void (APIENTRY*gluNurbsCurve_t) (
    GLUnurbs            *nobj, 
    GLint               nknots, 
    GLfloat             *knot, 
    GLint               stride, 
    GLfloat             *ctlarray, 
    GLint               order, 
    GLenum              type);

typedef void (APIENTRY*gluNurbsSurface_t) (     
    GLUnurbs            *nobj, 
    GLint               sknot_count, 
    float               *sknot, 
    GLint               tknot_count, 
    GLfloat             *tknot, 
    GLint               s_stride, 
    GLint               t_stride, 
    GLfloat             *ctlarray, 
    GLint               sorder, 
    GLint               torder, 
    GLenum              type);

typedef void (APIENTRY*gluLoadSamplingMatrices_t) (
    GLUnurbs            *nobj, 
    const GLfloat       modelMatrix[16], 
    const GLfloat       projMatrix[16], 
    const GLint         viewport[4] );

typedef void (APIENTRY*gluNurbsProperty_t) (
    GLUnurbs            *nobj, 
    GLenum              property, 
    GLfloat             value );

typedef void (APIENTRY*gluGetNurbsProperty_t) (
    GLUnurbs            *nobj, 
    GLenum              property, 
    GLfloat             *value );

typedef void (APIENTRY*gluNurbsCallback_t) (
    GLUnurbs            *nobj, 
    GLenum              which, 
    void                (CALLBACK* fn)() );

typedef void (APIENTRY*gluBeginPolygon_t) ( GLUtesselator *tess );

typedef void (APIENTRY*gluNextContour_t) ( GLUtesselator *tess, GLenum type );

typedef void (APIENTRY*gluEndPolygon_t) ( GLUtesselator *tess );


// MACHINE GENERATED !!! USE gltools.lua !!!
extern gluErrorString_t                     _gluErrorString;
extern gluErrorUnicodeStringEXT_t           _gluErrorUnicodeStringEXT;
extern gluGetString_t                       _gluGetString;
extern gluOrtho2D_t                         _gluOrtho2D;
extern gluPerspective_t                     _gluPerspective;
extern gluPickMatrix_t                      _gluPickMatrix;
extern gluLookAt_t                          _gluLookAt;
extern gluProject_t                         _gluProject;
extern gluUnProject_t                       _gluUnProject;
extern gluScaleImage_t                      _gluScaleImage;
extern gluBuild1DMipmaps_t                  _gluBuild1DMipmaps;
extern gluBuild2DMipmaps_t                  _gluBuild2DMipmaps;
extern gluNewQuadric_t                      _gluNewQuadric;
extern gluDeleteQuadric_t                   _gluDeleteQuadric;
extern gluQuadricNormals_t                  _gluQuadricNormals;
extern gluQuadricTexture_t                  _gluQuadricTexture;
extern gluQuadricOrientation_t              _gluQuadricOrientation;
extern gluQuadricDrawStyle_t                _gluQuadricDrawStyle;
extern gluCylinder_t                        _gluCylinder;
extern gluDisk_t                            _gluDisk;
extern gluPartialDisk_t                     _gluPartialDisk;
extern gluSphere_t                          _gluSphere;
extern gluQuadricCallback_t                 _gluQuadricCallback;
extern gluNewTess_t                         _gluNewTess;
extern gluDeleteTess_t                      _gluDeleteTess;
extern gluTessBeginPolygon_t                _gluTessBeginPolygon;
extern gluTessBeginContour_t                _gluTessBeginContour;
extern gluTessVertex_t                      _gluTessVertex;
extern gluTessEndContour_t                  _gluTessEndContour;
extern gluTessEndPolygon_t                  _gluTessEndPolygon;
extern gluTessProperty_t                    _gluTessProperty;
extern gluTessNormal_t                      _gluTessNormal;
extern gluTessCallback_t                    _gluTessCallback;
extern gluGetTessProperty_t                 _gluGetTessProperty;
extern gluNewNurbsRenderer_t                _gluNewNurbsRenderer;
extern gluDeleteNurbsRenderer_t             _gluDeleteNurbsRenderer;
extern gluBeginSurface_t                    _gluBeginSurface;
extern gluBeginCurve_t                      _gluBeginCurve;
extern gluEndCurve_t                        _gluEndCurve;
extern gluEndSurface_t                      _gluEndSurface;
extern gluBeginTrim_t                       _gluBeginTrim;
extern gluEndTrim_t                         _gluEndTrim;
extern gluPwlCurve_t                        _gluPwlCurve;
extern gluNurbsCurve_t                      _gluNurbsCurve;
extern gluNurbsSurface_t                    _gluNurbsSurface;
extern gluLoadSamplingMatrices_t            _gluLoadSamplingMatrices;
extern gluNurbsProperty_t                   _gluNurbsProperty;
extern gluGetNurbsProperty_t                _gluGetNurbsProperty;
extern gluNurbsCallback_t                   _gluNurbsCallback;
extern gluBeginPolygon_t                    _gluBeginPolygon;
extern gluNextContour_t                     _gluNextContour;
extern gluEndPolygon_t                      _gluEndPolygon;

// OpenGL 1.3 typedefs
typedef void (APIENTRY*glActiveTexture_t) (GLenum);
typedef void (APIENTRY*glClientActiveTexture_t) (GLenum);
typedef void (APIENTRY*glMultiTexCoord1d_t) (GLenum, GLdouble);
typedef void (APIENTRY*glMultiTexCoord1dv_t) (GLenum, const GLdouble *);
typedef void (APIENTRY*glMultiTexCoord1f_t) (GLenum, GLfloat);
typedef void (APIENTRY*glMultiTexCoord1fv_t) (GLenum, const GLfloat *);
typedef void (APIENTRY*glMultiTexCoord1i_t) (GLenum, GLint);
typedef void (APIENTRY*glMultiTexCoord1iv_t) (GLenum, const GLint *);
typedef void (APIENTRY*glMultiTexCoord1s_t) (GLenum, GLshort);
typedef void (APIENTRY*glMultiTexCoord1sv_t) (GLenum, const GLshort *);
typedef void (APIENTRY*glMultiTexCoord2d_t) (GLenum, GLdouble, GLdouble);
typedef void (APIENTRY*glMultiTexCoord2dv_t) (GLenum, const GLdouble *);
typedef void (APIENTRY*glMultiTexCoord2f_t) (GLenum, GLfloat, GLfloat);
typedef void (APIENTRY*glMultiTexCoord2fv_t) (GLenum, const GLfloat *);
typedef void (APIENTRY*glMultiTexCoord2i_t) (GLenum, GLint, GLint);
typedef void (APIENTRY*glMultiTexCoord2iv_t) (GLenum, const GLint *);
typedef void (APIENTRY*glMultiTexCoord2s_t) (GLenum, GLshort, GLshort);
typedef void (APIENTRY*glMultiTexCoord2sv_t) (GLenum, const GLshort *);
typedef void (APIENTRY*glMultiTexCoord3d_t) (GLenum, GLdouble, GLdouble, GLdouble);
typedef void (APIENTRY*glMultiTexCoord3dv_t) (GLenum, const GLdouble *);
typedef void (APIENTRY*glMultiTexCoord3f_t) (GLenum, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY*glMultiTexCoord3fv_t) (GLenum, const GLfloat *);
typedef void (APIENTRY*glMultiTexCoord3i_t) (GLenum, GLint, GLint, GLint);
typedef void (APIENTRY*glMultiTexCoord3iv_t) (GLenum, const GLint *);
typedef void (APIENTRY*glMultiTexCoord3s_t) (GLenum, GLshort, GLshort, GLshort);
typedef void (APIENTRY*glMultiTexCoord3sv_t) (GLenum, const GLshort *);
typedef void (APIENTRY*glMultiTexCoord4d_t) (GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
typedef void (APIENTRY*glMultiTexCoord4dv_t) (GLenum, const GLdouble *);
typedef void (APIENTRY*glMultiTexCoord4f_t) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY*glMultiTexCoord4fv_t) (GLenum, const GLfloat *);
typedef void (APIENTRY*glMultiTexCoord4i_t) (GLenum, GLint, GLint, GLint, GLint);
typedef void (APIENTRY*glMultiTexCoord4iv_t) (GLenum, const GLint *);
typedef void (APIENTRY*glMultiTexCoord4s_t) (GLenum, GLshort, GLshort, GLshort, GLshort);
typedef void (APIENTRY*glMultiTexCoord4sv_t) (GLenum, const GLshort *);
typedef void (APIENTRY*glLoadTransposeMatrixf_t) (const GLfloat *);
typedef void (APIENTRY*glLoadTransposeMatrixd_t) (const GLdouble *);
typedef void (APIENTRY*glMultTransposeMatrixf_t) (const GLfloat *);
typedef void (APIENTRY*glMultTransposeMatrixd_t) (const GLdouble *);
typedef void (APIENTRY*glSampleCoverage_t) (GLclampf, GLboolean);
typedef void (APIENTRY*glCompressedTexImage3D_t) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
typedef void (APIENTRY*glCompressedTexImage2D_t) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
typedef void (APIENTRY*glCompressedTexImage1D_t) (GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
typedef void (APIENTRY*glCompressedTexSubImage3D_t) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
typedef void (APIENTRY*glCompressedTexSubImage2D_t) (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
typedef void (APIENTRY*glCompressedTexSubImage1D_t) (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
typedef void (APIENTRY*glGetCompressedTexImage_t) (GLenum, GLint, GLvoid *);

// OpenGL RenderBufferEXT typedefs
typedef GLboolean (APIENTRY*glIsRenderbufferEXT_t) (GLuint);
typedef void (APIENTRY*glBindRenderbufferEXT_t) (GLenum, GLuint);
typedef void (APIENTRY*glDeleteRenderbuffersEXT_t) (GLsizei, const GLuint *);
typedef void (APIENTRY*glGenRenderbuffersEXT_t) (GLsizei, GLuint *);
typedef void (APIENTRY*glRenderbufferStorageEXT_t) (GLenum, GLenum, GLsizei, GLsizei);
typedef void (APIENTRY*glGetRenderbufferParameterivEXT_t) (GLenum, GLenum, GLint *);
typedef GLboolean (APIENTRY*glIsFramebufferEXT_t) (GLuint);
typedef void (APIENTRY*glBindFramebufferEXT_t) (GLenum, GLuint);
typedef void (APIENTRY*glDeleteFramebuffersEXT_t) (GLsizei, const GLuint *);
typedef void (APIENTRY*glGenFramebuffersEXT_t) (GLsizei, GLuint *);
typedef GLenum (APIENTRY*glCheckFramebufferStatusEXT_t) (GLenum);
typedef void (APIENTRY*glFramebufferTexture1DEXT_t) (GLenum, GLenum, GLenum, GLuint, GLint);
typedef void (APIENTRY*glFramebufferTexture2DEXT_t) (GLenum, GLenum, GLenum, GLuint, GLint);
typedef void (APIENTRY*glFramebufferTexture3DEXT_t) (GLenum, GLenum, GLenum, GLuint, GLint, GLint);
typedef void (APIENTRY*glFramebufferRenderbufferEXT_t) (GLenum, GLenum, GLenum, GLuint);
typedef void (APIENTRY*glGetFramebufferAttachmentParameterivEXT_t) (GLenum, GLenum, GLenum, GLint *);
typedef void (APIENTRY*glGenerateMipmapEXT_t) (GLenum);

// OpenGL ARB_shader_objects typedefs
typedef void (APIENTRY*glDeleteObjectARB_t) (GLhandleARB obj);
typedef GLhandleARB (APIENTRY*glGetHandleARB_t) (GLenum pname);
typedef void (APIENTRY*glDetachObjectARB_t) (GLhandleARB containerObj, GLhandleARB attachedObj);
typedef GLhandleARB (APIENTRY*glCreateShaderObjectARB_t) (GLenum shaderType);
typedef void (APIENTRY*glShaderSourceARB_t) (GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length);
typedef void (APIENTRY*glCompileShaderARB_t) (GLhandleARB shaderObj);
typedef GLhandleARB (APIENTRY*glCreateProgramObjectARB_t) (void);
typedef void (APIENTRY*glAttachObjectARB_t) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (APIENTRY*glLinkProgramARB_t) (GLhandleARB programObj);
typedef void (APIENTRY*glUseProgramObjectARB_t) (GLhandleARB programObj);
typedef void (APIENTRY*glValidateProgramARB_t) (GLhandleARB programObj);
typedef void (APIENTRY*glUniform1fARB_t) (GLint location, GLfloat v0);
typedef void (APIENTRY*glUniform2fARB_t) (GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRY*glUniform3fARB_t) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY*glUniform4fARB_t) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY*glUniform1iARB_t) (GLint location, GLint v0);
typedef void (APIENTRY*glUniform2iARB_t) (GLint location, GLint v0, GLint v1);
typedef void (APIENTRY*glUniform3iARB_t) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (APIENTRY*glUniform4iARB_t) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (APIENTRY*glUniform1fvARB_t) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY*glUniform2fvARB_t) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY*glUniform3fvARB_t) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY*glUniform4fvARB_t) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY*glUniform1ivARB_t) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY*glUniform2ivARB_t) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY*glUniform3ivARB_t) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY*glUniform4ivARB_t) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY*glUniformMatrix2fvARB_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY*glUniformMatrix3fvARB_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY*glUniformMatrix4fvARB_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY*glGetObjectParameterfvARB_t) (GLhandleARB obj, GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetObjectParameterivARB_t) (GLhandleARB obj, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetInfoLogARB_t) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef void (APIENTRY*glGetAttachedObjectsARB_t) (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
typedef GLint (APIENTRY*glGetUniformLocationARB_t) (GLhandleARB programObj, const GLcharARB *name);
typedef void (APIENTRY*glGetActiveUniformARB_t) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef void (APIENTRY*glGetUniformfvARB_t) (GLhandleARB programObj, GLint location, GLfloat *params);
typedef void (APIENTRY*glGetUniformivARB_t) (GLhandleARB programObj, GLint location, GLint *params);
typedef void (APIENTRY*glGetShaderSourceARB_t) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);

// OpenGL ARB_vertex_program typedefs
typedef void (APIENTRY*glVertexAttrib1dARB_t) (GLuint index, GLdouble x);
typedef void (APIENTRY*glVertexAttrib1dvARB_t) (GLuint index, const GLdouble *v);
typedef void (APIENTRY*glVertexAttrib1fARB_t) (GLuint index, GLfloat x);
typedef void (APIENTRY*glVertexAttrib1fvARB_t) (GLuint index, const GLfloat *v);
typedef void (APIENTRY*glVertexAttrib1sARB_t) (GLuint index, GLshort x);
typedef void (APIENTRY*glVertexAttrib1svARB_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib2dARB_t) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRY*glVertexAttrib2dvARB_t) (GLuint index, const GLdouble *v);
typedef void (APIENTRY*glVertexAttrib2fARB_t) (GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRY*glVertexAttrib2fvARB_t) (GLuint index, const GLfloat *v);
typedef void (APIENTRY*glVertexAttrib2sARB_t) (GLuint index, GLshort x, GLshort y);
typedef void (APIENTRY*glVertexAttrib2svARB_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib3dARB_t) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY*glVertexAttrib3dvARB_t) (GLuint index, const GLdouble *v);
typedef void (APIENTRY*glVertexAttrib3fARB_t) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY*glVertexAttrib3fvARB_t) (GLuint index, const GLfloat *v);
typedef void (APIENTRY*glVertexAttrib3sARB_t) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY*glVertexAttrib3svARB_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib4NbvARB_t) (GLuint index, const GLbyte *v);
typedef void (APIENTRY*glVertexAttrib4NivARB_t) (GLuint index, const GLint *v);
typedef void (APIENTRY*glVertexAttrib4NsvARB_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib4NubARB_t) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (APIENTRY*glVertexAttrib4NubvARB_t) (GLuint index, const GLubyte *v);
typedef void (APIENTRY*glVertexAttrib4NuivARB_t) (GLuint index, const GLuint *v);
typedef void (APIENTRY*glVertexAttrib4NusvARB_t) (GLuint index, const GLushort *v);
typedef void (APIENTRY*glVertexAttrib4bvARB_t) (GLuint index, const GLbyte *v);
typedef void (APIENTRY*glVertexAttrib4dARB_t) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY*glVertexAttrib4dvARB_t) (GLuint index, const GLdouble *v);
typedef void (APIENTRY*glVertexAttrib4fARB_t) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY*glVertexAttrib4fvARB_t) (GLuint index, const GLfloat *v);
typedef void (APIENTRY*glVertexAttrib4ivARB_t) (GLuint index, const GLint *v);
typedef void (APIENTRY*glVertexAttrib4sARB_t) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY*glVertexAttrib4svARB_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib4ubvARB_t) (GLuint index, const GLubyte *v);
typedef void (APIENTRY*glVertexAttrib4uivARB_t) (GLuint index, const GLuint *v);
typedef void (APIENTRY*glVertexAttrib4usvARB_t) (GLuint index, const GLushort *v);
typedef void (APIENTRY*glVertexAttribPointerARB_t) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY*glEnableVertexAttribArrayARB_t) (GLuint index);
typedef void (APIENTRY*glDisableVertexAttribArrayARB_t) (GLuint index);
typedef void (APIENTRY*glGetVertexAttribdvARB_t) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY*glGetVertexAttribfvARB_t) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetVertexAttribivARB_t) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetVertexAttribPointervARB_t) (GLuint index, GLenum pname, GLvoid **pointer);

// OpenGL ARB_vertex_shader typedefs
typedef void (APIENTRY*glBindAttribLocationARB_t) (GLhandleARB programObj, GLuint index, const GLcharARB *name);
typedef void (APIENTRY*glGetActiveAttribARB_t) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
typedef GLint (APIENTRY*glGetAttribLocationARB_t) (GLhandleARB programObj, const GLcharARB *name);

// OpenGL 2.0 typedefs
typedef void (APIENTRY*glBlendEquationSeparate_t) (GLenum modeRGB, GLenum modeAlpha);
typedef void (APIENTRY*glDrawBuffers_t) (GLsizei n, const GLenum *bufs);
typedef void (APIENTRY*glStencilOpSeparate_t) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (APIENTRY*glStencilFuncSeparate_t) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void (APIENTRY*glStencilMaskSeparate_t) (GLenum face, GLuint mask);
typedef void (APIENTRY*glAttachShader_t) (GLuint program, GLuint shader);
typedef void (APIENTRY*glBindAttribLocation_t) (GLuint program, GLuint index, const GLchar *name);
typedef void (APIENTRY*glCompileShader_t) (GLuint shader);
typedef GLuint (APIENTRY*glCreateProgram_t) (void);
typedef GLuint (APIENTRY*glCreateShader_t) (GLenum type);
typedef void (APIENTRY*glDeleteProgram_t) (GLuint program);
typedef void (APIENTRY*glDeleteShader_t) (GLuint shader);
typedef void (APIENTRY*glDetachShader_t) (GLuint program, GLuint shader);
typedef void (APIENTRY*glDisableVertexAttribArray_t) (GLuint index);
typedef void (APIENTRY*glEnableVertexAttribArray_t) (GLuint index);
typedef void (APIENTRY*glGetActiveAttrib_t) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (APIENTRY*glGetActiveUniform_t) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (APIENTRY*glGetAttachedShaders_t) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
typedef GLint (APIENTRY*glGetAttribLocation_t) (GLuint program, const GLchar *name);
typedef void (APIENTRY*glGetProgramiv_t) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetProgramInfoLog_t) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRY*glGetShaderiv_t) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetShaderInfoLog_t) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRY*glGetShaderSource_t) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef GLint (APIENTRY*glGetUniformLocation_t) (GLuint program, const GLchar *name);
typedef void (APIENTRY*glGetUniformfv_t) (GLuint program, GLint location, GLfloat *params);
typedef void (APIENTRY*glGetUniformiv_t) (GLuint program, GLint location, GLint *params);
typedef void (APIENTRY*glGetVertexAttribdv_t) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY*glGetVertexAttribfv_t) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY*glGetVertexAttribiv_t) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetVertexAttribPointerv_t) (GLuint index, GLenum pname, GLvoid **pointer);
typedef GLboolean (APIENTRY*glIsProgram_t) (GLuint program);
typedef GLboolean (APIENTRY*glIsShader_t) (GLuint shader);
typedef void (APIENTRY*glLinkProgram_t) (GLuint program);
typedef void (APIENTRY*glShaderSource_t) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRY*glUseProgram_t) (GLuint program);
typedef void (APIENTRY*glUniform1f_t) (GLint location, GLfloat v0);
typedef void (APIENTRY*glUniform2f_t) (GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRY*glUniform3f_t) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY*glUniform4f_t) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY*glUniform1i_t) (GLint location, GLint v0);
typedef void (APIENTRY*glUniform2i_t) (GLint location, GLint v0, GLint v1);
typedef void (APIENTRY*glUniform3i_t) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (APIENTRY*glUniform4i_t) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (APIENTRY*glUniform1fv_t) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY*glUniform2fv_t) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY*glUniform3fv_t) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY*glUniform4fv_t) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY*glUniform1iv_t) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY*glUniform2iv_t) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY*glUniform3iv_t) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY*glUniform4iv_t) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRY*glUniformMatrix2fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY*glUniformMatrix3fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY*glUniformMatrix4fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY*glValidateProgram_t) (GLuint program);
typedef void (APIENTRY*glVertexAttrib1d_t) (GLuint index, GLdouble x);
typedef void (APIENTRY*glVertexAttrib1dv_t) (GLuint index, const GLdouble *v);
typedef void (APIENTRY*glVertexAttrib1f_t) (GLuint index, GLfloat x);
typedef void (APIENTRY*glVertexAttrib1fv_t) (GLuint index, const GLfloat *v);
typedef void (APIENTRY*glVertexAttrib1s_t) (GLuint index, GLshort x);
typedef void (APIENTRY*glVertexAttrib1sv_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib2d_t) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRY*glVertexAttrib2dv_t) (GLuint index, const GLdouble *v);
typedef void (APIENTRY*glVertexAttrib2f_t) (GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRY*glVertexAttrib2fv_t) (GLuint index, const GLfloat *v);
typedef void (APIENTRY*glVertexAttrib2s_t) (GLuint index, GLshort x, GLshort y);
typedef void (APIENTRY*glVertexAttrib2sv_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib3d_t) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY*glVertexAttrib3dv_t) (GLuint index, const GLdouble *v);
typedef void (APIENTRY*glVertexAttrib3f_t) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY*glVertexAttrib3fv_t) (GLuint index, const GLfloat *v);
typedef void (APIENTRY*glVertexAttrib3s_t) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY*glVertexAttrib3sv_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib4Nbv_t) (GLuint index, const GLbyte *v);
typedef void (APIENTRY*glVertexAttrib4Niv_t) (GLuint index, const GLint *v);
typedef void (APIENTRY*glVertexAttrib4Nsv_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib4Nub_t) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (APIENTRY*glVertexAttrib4Nubv_t) (GLuint index, const GLubyte *v);
typedef void (APIENTRY*glVertexAttrib4Nuiv_t) (GLuint index, const GLuint *v);
typedef void (APIENTRY*glVertexAttrib4Nusv_t) (GLuint index, const GLushort *v);
typedef void (APIENTRY*glVertexAttrib4bv_t) (GLuint index, const GLbyte *v);
typedef void (APIENTRY*glVertexAttrib4d_t) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY*glVertexAttrib4dv_t) (GLuint index, const GLdouble *v);
typedef void (APIENTRY*glVertexAttrib4f_t) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY*glVertexAttrib4fv_t) (GLuint index, const GLfloat *v);
typedef void (APIENTRY*glVertexAttrib4iv_t) (GLuint index, const GLint *v);
typedef void (APIENTRY*glVertexAttrib4s_t) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY*glVertexAttrib4sv_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttrib4ubv_t) (GLuint index, const GLubyte *v);
typedef void (APIENTRY*glVertexAttrib4uiv_t) (GLuint index, const GLuint *v);
typedef void (APIENTRY*glVertexAttrib4usv_t) (GLuint index, const GLushort *v);
typedef void (APIENTRY*glVertexAttribPointer_t) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);

// OpenGL 3.0 typedefs
typedef void (APIENTRY*glColorMaski_t) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
typedef void (APIENTRY*glGetBooleani_v_t) (GLenum target, GLuint index, GLboolean *data);
typedef void (APIENTRY*glGetIntegeri_v_t) (GLenum target, GLuint index, GLint *data);
typedef void (APIENTRY*glEnablei_t) (GLenum target, GLuint index);
typedef void (APIENTRY*glDisablei_t) (GLenum target, GLuint index);
typedef GLboolean (APIENTRY*glIsEnabledi_t) (GLenum target, GLuint index);
typedef void (APIENTRY*glBeginTransformFeedback_t) (GLenum primitiveMode);
typedef void (APIENTRY*glEndTransformFeedback_t) (void);
typedef void (APIENTRY*glBindBufferRange_t) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRY*glBindBufferBase_t) (GLenum target, GLuint index, GLuint buffer);
typedef void (APIENTRY*glTransformFeedbackVaryings_t) (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
typedef void (APIENTRY*glGetTransformFeedbackVarying_t) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
typedef void (APIENTRY*glClampColor_t) (GLenum target, GLenum clamp);
typedef void (APIENTRY*glBeginConditionalRender_t) (GLuint id, GLenum mode);
typedef void (APIENTRY*glEndConditionalRender_t) (void);
typedef void (APIENTRY*glVertexAttribIPointer_t) (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY*glGetVertexAttribIiv_t) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetVertexAttribIuiv_t) (GLuint index, GLenum pname, GLuint *params);
typedef void (APIENTRY*glVertexAttribI1i_t) (GLuint index, GLint x);
typedef void (APIENTRY*glVertexAttribI2i_t) (GLuint index, GLint x, GLint y);
typedef void (APIENTRY*glVertexAttribI3i_t) (GLuint index, GLint x, GLint y, GLint z);
typedef void (APIENTRY*glVertexAttribI4i_t) (GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void (APIENTRY*glVertexAttribI1ui_t) (GLuint index, GLuint x);
typedef void (APIENTRY*glVertexAttribI2ui_t) (GLuint index, GLuint x, GLuint y);
typedef void (APIENTRY*glVertexAttribI3ui_t) (GLuint index, GLuint x, GLuint y, GLuint z);
typedef void (APIENTRY*glVertexAttribI4ui_t) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void (APIENTRY*glVertexAttribI1iv_t) (GLuint index, const GLint *v);
typedef void (APIENTRY*glVertexAttribI2iv_t) (GLuint index, const GLint *v);
typedef void (APIENTRY*glVertexAttribI3iv_t) (GLuint index, const GLint *v);
typedef void (APIENTRY*glVertexAttribI4iv_t) (GLuint index, const GLint *v);
typedef void (APIENTRY*glVertexAttribI1uiv_t) (GLuint index, const GLuint *v);
typedef void (APIENTRY*glVertexAttribI2uiv_t) (GLuint index, const GLuint *v);
typedef void (APIENTRY*glVertexAttribI3uiv_t) (GLuint index, const GLuint *v);
typedef void (APIENTRY*glVertexAttribI4uiv_t) (GLuint index, const GLuint *v);
typedef void (APIENTRY*glVertexAttribI4bv_t) (GLuint index, const GLbyte *v);
typedef void (APIENTRY*glVertexAttribI4sv_t) (GLuint index, const GLshort *v);
typedef void (APIENTRY*glVertexAttribI4ubv_t) (GLuint index, const GLubyte *v);
typedef void (APIENTRY*glVertexAttribI4usv_t) (GLuint index, const GLushort *v);
typedef void (APIENTRY*glGetUniformuiv_t) (GLuint program, GLint location, GLuint *params);
typedef void (APIENTRY*glBindFragDataLocation_t) (GLuint program, GLuint color, const GLchar *name);
typedef GLint (APIENTRY*glGetFragDataLocation_t) (GLuint program, const GLchar *name);
typedef void (APIENTRY*glUniform1ui_t) (GLint location, GLuint v0);
typedef void (APIENTRY*glUniform2ui_t) (GLint location, GLuint v0, GLuint v1);
typedef void (APIENTRY*glUniform3ui_t) (GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (APIENTRY*glUniform4ui_t) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (APIENTRY*glUniform1uiv_t) (GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRY*glUniform2uiv_t) (GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRY*glUniform3uiv_t) (GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRY*glUniform4uiv_t) (GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRY*glTexParameterIiv_t) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRY*glTexParameterIuiv_t) (GLenum target, GLenum pname, const GLuint *params);
typedef void (APIENTRY*glGetTexParameterIiv_t) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY*glGetTexParameterIuiv_t) (GLenum target, GLenum pname, GLuint *params);
typedef void (APIENTRY*glClearBufferiv_t) (GLenum buffer, GLint drawbuffer, const GLint *value);
typedef void (APIENTRY*glClearBufferuiv_t) (GLenum buffer, GLint drawbuffer, const GLuint *value);
typedef void (APIENTRY*glClearBufferfv_t) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
typedef void (APIENTRY*glClearBufferfi_t) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef const GLubyte *(APIENTRY*glGetStringi_t) (GLenum name, GLuint index);
typedef GLboolean (APIENTRY*glIsRenderbuffer_t) (GLuint renderbuffer);
typedef void (APIENTRY*glBindRenderbuffer_t) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRY*glDeleteRenderbuffers_t) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRY*glGenRenderbuffers_t) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRY*glRenderbufferStorage_t) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY*glGetRenderbufferParameteriv_t) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (APIENTRY*glIsFramebuffer_t) (GLuint framebuffer);
typedef void (APIENTRY*glBindFramebuffer_t) (GLenum target, GLuint framebuffer);
typedef void (APIENTRY*glDeleteFramebuffers_t) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRY*glGenFramebuffers_t) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRY*glCheckFramebufferStatus_t) (GLenum target);
typedef void (APIENTRY*glFramebufferTexture1D_t) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRY*glFramebufferTexture2D_t) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRY*glFramebufferTexture3D_t) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (APIENTRY*glFramebufferRenderbuffer_t) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRY*glGetFramebufferAttachmentParameteriv_t) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (APIENTRY*glGenerateMipmap_t) (GLenum target);
typedef void (APIENTRY*glBlitFramebuffer_t) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void (APIENTRY*glRenderbufferStorageMultisample_t) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY*glFramebufferTextureLayer_t) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void *(APIENTRY*glMapBufferRange_t) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (APIENTRY*glFlushMappedBufferRange_t) (GLenum target, GLintptr offset, GLsizeiptr length);
typedef void (APIENTRY*glBindVertexArray_t) (GLuint array);
typedef void (APIENTRY*glDeleteVertexArrays_t) (GLsizei n, const GLuint *arrays);
typedef void (APIENTRY*glGenVertexArrays_t) (GLsizei n, GLuint *arrays);
typedef GLboolean (APIENTRY* glIsVertexArray_t) (GLuint array);

#endif //_OPENGL_FUNCTION_TYPEDEFS_