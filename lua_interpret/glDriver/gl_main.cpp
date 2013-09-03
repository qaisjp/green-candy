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
#include "gl_internal.h"

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

gluErrorString_t                    _gluErrorString = NULL;
gluErrorUnicodeStringEXT_t          _gluErrorUnicodeStringEXT = NULL;
gluGetString_t                      _gluGetString = NULL;
gluOrtho2D_t                        _gluOrtho2D = NULL;
gluPerspective_t                    _gluPerspective = NULL;
gluPickMatrix_t                     _gluPickMatrix = NULL;
gluLookAt_t                         _gluLookAt = NULL;
gluProject_t                        _gluProject = NULL;
gluUnProject_t                      _gluUnProject = NULL;
gluScaleImage_t                     _gluScaleImage = NULL;
gluBuild1DMipmaps_t                 _gluBuild1DMipmaps = NULL;
gluBuild2DMipmaps_t                 _gluBuild2DMipmaps = NULL;
gluNewQuadric_t                     _gluNewQuadric = NULL;
gluDeleteQuadric_t                  _gluDeleteQuadric = NULL;
gluQuadricNormals_t                 _gluQuadricNormals = NULL;
gluQuadricTexture_t                 _gluQuadricTexture = NULL;
gluQuadricOrientation_t             _gluQuadricOrientation = NULL;
gluQuadricDrawStyle_t               _gluQuadricDrawStyle = NULL;
gluCylinder_t                       _gluCylinder = NULL;
gluDisk_t                           _gluDisk = NULL;
gluPartialDisk_t                    _gluPartialDisk = NULL;
gluSphere_t                         _gluSphere = NULL;
gluQuadricCallback_t                _gluQuadricCallback = NULL;
gluNewTess_t                        _gluNewTess = NULL;
gluDeleteTess_t                     _gluDeleteTess = NULL;
gluTessBeginPolygon_t               _gluTessBeginPolygon = NULL;
gluTessBeginContour_t               _gluTessBeginContour = NULL;
gluTessVertex_t                     _gluTessVertex = NULL;
gluTessEndContour_t                 _gluTessEndContour = NULL;
gluTessEndPolygon_t                 _gluTessEndPolygon = NULL;
gluTessProperty_t                   _gluTessProperty = NULL;
gluTessNormal_t                     _gluTessNormal = NULL;
gluTessCallback_t                   _gluTessCallback = NULL;
gluGetTessProperty_t                _gluGetTessProperty = NULL;
gluNewNurbsRenderer_t               _gluNewNurbsRenderer = NULL;
gluDeleteNurbsRenderer_t            _gluDeleteNurbsRenderer = NULL;
gluBeginSurface_t                   _gluBeginSurface = NULL;
gluBeginCurve_t                     _gluBeginCurve = NULL;
gluEndCurve_t                       _gluEndCurve = NULL;
gluEndSurface_t                     _gluEndSurface = NULL;
gluBeginTrim_t                      _gluBeginTrim = NULL;
gluEndTrim_t                        _gluEndTrim = NULL;
gluPwlCurve_t                       _gluPwlCurve = NULL;
gluNurbsCurve_t                     _gluNurbsCurve = NULL;
gluNurbsSurface_t                   _gluNurbsSurface = NULL;
gluLoadSamplingMatrices_t           _gluLoadSamplingMatrices = NULL;
gluNurbsProperty_t                  _gluNurbsProperty = NULL;
gluGetNurbsProperty_t               _gluGetNurbsProperty = NULL;
gluNurbsCallback_t                  _gluNurbsCallback = NULL;
gluBeginPolygon_t                   _gluBeginPolygon = NULL;
gluNextContour_t                    _gluNextContour = NULL;
gluEndPolygon_t                     _gluEndPolygon = NULL;

glActiveTexture_t                   _glActiveTexture = NULL;
glClientActiveTexture_t             _glClientActiveTexture = NULL;
glMultiTexCoord1d_t                 _glMultiTexCoord1d = NULL;
glMultiTexCoord1dv_t                _glMultiTexCoord1dv = NULL;
glMultiTexCoord1f_t                 _glMultiTexCoord1f = NULL;
glMultiTexCoord1fv_t                _glMultiTexCoord1fv = NULL;
glMultiTexCoord1i_t                 _glMultiTexCoord1i = NULL;
glMultiTexCoord1iv_t                _glMultiTexCoord1iv = NULL;
glMultiTexCoord1s_t                 _glMultiTexCoord1s = NULL;
glMultiTexCoord1sv_t                _glMultiTexCoord1sv = NULL;
glMultiTexCoord2d_t                 _glMultiTexCoord2d = NULL;
glMultiTexCoord2dv_t                _glMultiTexCoord2dv = NULL;
glMultiTexCoord2f_t                 _glMultiTexCoord2f = NULL;
glMultiTexCoord2fv_t                _glMultiTexCoord2fv = NULL;
glMultiTexCoord2i_t                 _glMultiTexCoord2i = NULL;
glMultiTexCoord2iv_t                _glMultiTexCoord2iv = NULL;
glMultiTexCoord2s_t                 _glMultiTexCoord2s = NULL;
glMultiTexCoord2sv_t                _glMultiTexCoord2sv = NULL;
glMultiTexCoord3d_t                 _glMultiTexCoord3d = NULL;
glMultiTexCoord3dv_t                _glMultiTexCoord3dv = NULL;
glMultiTexCoord3f_t                 _glMultiTexCoord3f = NULL;
glMultiTexCoord3fv_t                _glMultiTexCoord3fv = NULL;
glMultiTexCoord3i_t                 _glMultiTexCoord3i = NULL;
glMultiTexCoord3iv_t                _glMultiTexCoord3iv = NULL;
glMultiTexCoord3s_t                 _glMultiTexCoord3s = NULL;
glMultiTexCoord3sv_t                _glMultiTexCoord3sv = NULL;
glMultiTexCoord4d_t                 _glMultiTexCoord4d = NULL;
glMultiTexCoord4dv_t                _glMultiTexCoord4dv = NULL;
glMultiTexCoord4f_t                 _glMultiTexCoord4f = NULL;
glMultiTexCoord4fv_t                _glMultiTexCoord4fv = NULL;
glMultiTexCoord4i_t                 _glMultiTexCoord4i = NULL;
glMultiTexCoord4iv_t                _glMultiTexCoord4iv = NULL;
glMultiTexCoord4s_t                 _glMultiTexCoord4s = NULL;
glMultiTexCoord4sv_t                _glMultiTexCoord4sv = NULL;
glLoadTransposeMatrixf_t            _glLoadTransposeMatrixf = NULL;
glLoadTransposeMatrixd_t            _glLoadTransposeMatrixd = NULL;
glMultTransposeMatrixf_t            _glMultTransposeMatrixf = NULL;
glMultTransposeMatrixd_t            _glMultTransposeMatrixd = NULL;
glSampleCoverage_t                  _glSampleCoverage = NULL;
glCompressedTexImage3D_t            _glCompressedTexImage3D = NULL;
glCompressedTexImage2D_t            _glCompressedTexImage2D = NULL;
glCompressedTexImage1D_t            _glCompressedTexImage1D = NULL;
glCompressedTexSubImage3D_t         _glCompressedTexSubImage3D = NULL;
glCompressedTexSubImage2D_t         _glCompressedTexSubImage2D = NULL;
glCompressedTexSubImage1D_t         _glCompressedTexSubImage1D = NULL;
glGetCompressedTexImage_t           _glGetCompressedTexImage = NULL;


HMODULE glMainLibrary = NULL;
HMODULE gluLibrary = NULL;
static RwList <glDriver> allDevices;

// Static def for last used driver on context stack
glDriver* glContextStack::lastStackDriver = NULL;

static LUA_DECLARE( runContext )
{
    if ( lua_type( L, 1 ) != LUA_TFUNCTION )
        throw lua_exception( L, LUA_ERRRUN, "expected function at OpenGL context function" );

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );
    lua_yield_shield shield( L );   // do not allow yields out of the context

    lua_call( L, 0, 0 );
    return 0;
}

namespace glBatch
{
    static LUA_DECLARE( vertex2f )
    {
        float x, y;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glVertex2f( x, y );
        return 0;
    }

    static LUA_DECLARE( vertex3f )
    {
        float x, y, z;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glVertex3f( x, y, z );
        return 0;
    }

    static LUA_DECLARE( vertex4f )
    {
        float x, y, z, w;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        argStream.ReadNumber( w );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glVertex4d( x, y, z, w );
        return 0;
    }

    static LUA_DECLARE( vertex2d )
    {
        double x, y;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glVertex2d( x, y );
        return 0;
    }

    static LUA_DECLARE( vertex3d )
    {
        double x, y, z;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );


        _glVertex3d( x, y, z );
        return 0;
    }

    static LUA_DECLARE( vertex4d )
    {
        double x, y, z, w;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        argStream.ReadNumber( w );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glVertex4d( x, y, z, w );
        return 0;
    }

    static LUA_DECLARE( texCoord2f )
    {
        float x, y;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glTexCoord2f( x, y );
        return 0;
    }

    static LUA_DECLARE( texCoord3f )
    {
        float x, y, z;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glTexCoord3f( x, y, z );
        return 0;
    }

    static LUA_DECLARE( texCoord4f )
    {
        float x, y, z, w;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        argStream.ReadNumber( w );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glTexCoord4d( x, y, z, w );
        return 0;
    }

    static LUA_DECLARE( texCoord2d )
    {
        double x, y;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glTexCoord2d( x, y );
        return 0;
    }

    static LUA_DECLARE( texCoord3d )
    {
        double x, y, z;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glTexCoord3d( x, y, z );
        return 0;
    }

    static LUA_DECLARE( texCoord4d )
    {
        double x, y, z, w;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        argStream.ReadNumber( w );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glTexCoord4d( x, y, z, w );
        return 0;
    }

    static LUA_DECLARE( color3f )
    {
        float x, y, z;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glColor3f( x, y, z );
        return 0;
    }

    static LUA_DECLARE( color4f )
    {
        float x, y, z, w;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        argStream.ReadNumber( w );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glColor4d( x, y, z, w );
        return 0;
    }

    static LUA_DECLARE( color3d )
    {
        double x, y, z;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glColor3d( x, y, z );
        return 0;
    }

    static LUA_DECLARE( color4d )
    {
        double x, y, z, w;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( x );
        argStream.ReadNumber( y );
        argStream.ReadNumber( z );
        argStream.ReadNumber( w );
        LUA_ARGS_END;

        glDriver *driver = lua_readclass <glDriver> ( L, lua_upvalueindex( 1 ), LUACLASS_GLDRIVER );

        if ( !driver->isBatching )
            throw lua_exception( L, LUA_ERRRUN, "attempt to call batch function outside of driver runBatch" );

        _glColor4d( x, y, z, w );
        return 0;
    }

    static const luaL_Reg batch_interface[] =
    {
        LUA_METHOD( vertex2f ),
        LUA_METHOD( vertex3f ),
        LUA_METHOD( vertex4f ),
        LUA_METHOD( vertex2d ),
        LUA_METHOD( vertex3d ),
        LUA_METHOD( vertex4d ),

        LUA_METHOD( texCoord2f ),
        LUA_METHOD( texCoord3f ),
        LUA_METHOD( texCoord4f ),
        LUA_METHOD( texCoord2d ),
        LUA_METHOD( texCoord3d ),
        LUA_METHOD( texCoord4d ),

        LUA_METHOD( color3f ),
        LUA_METHOD( color4f ),
        LUA_METHOD( color3d ),
        LUA_METHOD( color4d ),
        { NULL, NULL }
    };
}

static LUA_DECLARE( makeBatch )
{
    if ( lua_type( L, 1 ) != LUA_TFUNCTION )
        throw lua_exception( L, LUA_ERRRUN, "expected function at OpenGL context function" );

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    // Create a special environment for it
    lua_createEnvProxy( L );

    driver->PushStack( L );
    luaL_openlib( L, NULL, glBatch::batch_interface, 1 );
    lua_setfenv( L, 1 );
    return 1;
}

enum GLeBatchMode : GLenum
{
};

DECLARE_ENUM( GLeBatchMode );

IMPLEMENT_ENUM_BEGIN( GLeBatchMode )
    ADD_ENUM( GL_POINTS, "points" )
    ADD_ENUM( GL_LINES, "lines" )
    ADD_ENUM( GL_LINE_STRIP, "line_strip" )
    ADD_ENUM( GL_LINE_LOOP, "line_loop" )
    ADD_ENUM( GL_TRIANGLES, "triangles" )
    ADD_ENUM( GL_TRIANGLE_STRIP, "triangle_strip" )
    ADD_ENUM( GL_TRIANGLE_FAN, "triangle_fan" )
    ADD_ENUM( GL_QUADS, "quads" )
    ADD_ENUM( GL_QUAD_STRIP, "quad_strip" )
    ADD_ENUM( GL_POLYGON, "polygon" )
IMPLEMENT_ENUM_END( "GLeBatchMode" )

static LUA_DECLARE( runBatch )
{
    if ( lua_type( L, 1 ) != LUA_TFUNCTION )
        throw lua_exception( L, LUA_ERRRUN, "expected function at OpenGL context function" );

    lua_String mode = lua_tostring( L, 2 );

    if ( !mode )
        throw lua_exception( L, LUA_ERRRUN, "expected batch mode string at OpenGL context function" );

    GLeBatchMode eMode;
    
    if ( !StringToEnum( mode, eMode ) )
        throw lua_exception( L, LUA_ERRRUN, "invalid batch mode" );

    int top = lua_gettop( L );

    if ( top >= 3 )
    {
        lua_pushvalue( L, 1 );
        lua_pushvalue( L, 2 );

        for ( int n = 3; n <= top; n++ )
            lua_pushvalue( L, n );
    }

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );
    lua_yield_shield shield( L );

    _glBegin( eMode );
    driver->isBatching = true;

    if ( top >= 3 )
        lua_pcall( L, 1 + top - 2, 0, 0 );
    else
        lua_pcall( L, 1, 0, 0 );

    driver->isBatching = false;
    _glEnd();
    return 0;
}

static LUA_DECLARE( present )
{
    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack stack( driver );

    lua_pushboolean( L, SwapBuffers( driver->deviceContext ) == TRUE );
    return 1;
}

enum GLeMatrixMode
{
};

DECLARE_ENUM( GLeMatrixMode );

IMPLEMENT_ENUM_BEGIN( GLeMatrixMode )
    ADD_ENUM1( GL_MODELVIEW )
    ADD_ENUM1( GL_PROJECTION )
    ADD_ENUM1( GL_TEXTURE )
IMPLEMENT_ENUM_END( "GLeMatrixMode" )

static LUA_DECLARE( matrixMode )
{
    GLeMatrixMode mode;

    LUA_ARGS_BEGIN;
    argStream.ReadEnumString( mode );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glMatrixMode( mode );
    LUA_SUCCESS;
}

static LUA_DECLARE( pushMatrix )
{
    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glPushMatrix();
    return 0;
}

static LUA_DECLARE( popMatrix )
{
    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glPopMatrix();
    return 0;
}

static LUA_DECLARE( loadMatrix )
{
    RwMatrix *mat;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( (void*&)mat, LUACLASS_MATRIX );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glLoadMatrixf( (float*)mat );
    LUA_SUCCESS;
}

enum eGLMatrixType : GLenum
{};

DECLARE_ENUM( eGLMatrixType );

IMPLEMENT_ENUM_BEGIN( eGLMatrixType )
    ADD_ENUM( GL_MODELVIEW_MATRIX, "modelview" )
    ADD_ENUM( GL_PROJECTION_MATRIX, "projection" )
    ADD_ENUM( GL_TEXTURE_MATRIX, "texture" )
IMPLEMENT_ENUM_END( "eGLMatrixType" )

static LUA_DECLARE( getMatrix )
{
    eGLMatrixType matrixType;

    LUA_ARGS_BEGIN;
    argStream.ReadEnumString( matrixType );
    LUA_ARGS_END;

    RwMatrix mat;
    _glGetFloatv( matrixType, (float*)&mat );

    lua_creatematrix( L, mat );
    return 1;
}

static LUA_DECLARE( loadIdentity )
{
    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glLoadIdentity();
    LUA_SUCCESS;
}

static LUA_DECLARE( multiplyMatrix )
{
    RwMatrix *mat;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( (void*&)mat, LUACLASS_MATRIX );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glMultMatrixf( (float*)mat );
    LUA_SUCCESS;
}

static LUA_DECLARE( rotate )
{
    GLdouble angle, vx, vy, vz;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( angle );
    argStream.ReadNumber( vx );
    argStream.ReadNumber( vy );
    argStream.ReadNumber( vz );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glRotated( angle, vx, vy, vz );
    LUA_SUCCESS;
}

static LUA_DECLARE( translate )
{
    GLdouble x, y, z;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( x );
    argStream.ReadNumber( y );
    argStream.ReadNumber( z );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glTranslated( x, y, z );
    LUA_SUCCESS;
}

static LUA_DECLARE( get )
{
    const char *name;
    size_t len;

    name = lua_tolstring( L, 1, &len );

    LUA_CHECK( name );

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    const GLparaminfo *info = gl_parameters::GetParamByName( driver, name, len );

    LUA_CHECK( info );

    union
    {
        float _float[16];
        int _int[16];
        GLboolean _bool[16];
        double _double[16];
    };

    glContextStack context( driver );

    switch( info->type )
    {
    case GLPARAM_BOOLEAN:
        _glGetBooleanv( info->builtin, _bool );

        for ( unsigned int n = 0; n < info->numArgs; n++ )
            lua_pushboolean( L, _bool[n] );

        break;
    case GLPARAM_FLOAT:
        _glGetFloatv( info->builtin, _float );

        for ( unsigned int n = 0; n < info->numArgs; n++ )
            lua_pushnumber( L, _float[n] );

        break;
    case GLPARAM_DOUBLE:
        _glGetDoublev( info->builtin, _double );

        for ( unsigned int n = 0; n < info->numArgs; n++ )
            lua_pushnumber( L, _double[n] );

        break;
    default:
        _glGetIntegerv( info->builtin, _int );

        for ( unsigned int n = 0; n < info->numArgs; n++ )
            lua_pushinteger( L, _int[n] );

        break;
    }

    return info->numArgs;
}

static LUA_DECLARE( enable )
{
    size_t len;
    lua_String name = lua_tolstring( L, 1, &len );

    if ( !name )
        throw lua_exception( L, LUA_ERRRUN, "invalid bool property" );

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    const GLparamdesc *info = gl_capabilities::GetParamByName( driver, name, len );

    LUA_CHECK( info );

    glContextStack context( driver );

    _glEnable( info->builtin );

    lua_pushboolean( L, true );
    return 1;
}

static LUA_DECLARE( disable )
{
    size_t len;
    lua_String name = lua_tolstring( L, 1, &len );

    if ( !name )
        throw lua_exception( L, LUA_ERRRUN, "invalid bool property" );

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    const GLparamdesc *info = gl_capabilities::GetParamByName( driver, name, len );

    LUA_CHECK( info );

    glContextStack context( driver );

    _glDisable( info->builtin );

    lua_pushboolean( L, true );
    return 1;
}

static LUA_DECLARE( isEnabled )
{
    size_t len;
    lua_String name = lua_tolstring( L, 1, &len );

    if ( !name )
        throw lua_exception( L, LUA_ERRRUN, "invalid bool property" );

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    const GLparamdesc *info = gl_capabilities::GetParamByName( driver, name, len );

    LUA_CHECK( info );

    glContextStack context( driver );

    lua_pushboolean( L, _glIsEnabled( info->builtin ) );
    return 1;
}

static LUA_DECLARE( ortho )
{
    double _left, _right, _bottom, _top, _near, _far;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( _left );
    argStream.ReadNumber( _right );
    argStream.ReadNumber( _bottom );
    argStream.ReadNumber( _top );
    argStream.ReadNumber( _near );
    argStream.ReadNumber( _far );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glOrtho( _left, _right, _bottom, _top, _near, _far );
    LUA_SUCCESS;
}

static LUA_DECLARE( viewport )
{
    GLint x, y;
    GLsizei width, height;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( x );
    argStream.ReadNumber( y );
    argStream.ReadNumber( width );
    argStream.ReadNumber( height );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glViewport( x, y, width, height );
    LUA_SUCCESS;
}

enum GLeClearType : GLenum
{ };

DECLARE_ENUM( GLeClearType );

IMPLEMENT_ENUM_BEGIN( GLeClearType )
    ADD_ENUM( GL_COLOR_BUFFER_BIT, "color" )
    ADD_ENUM( GL_DEPTH_BUFFER_BIT, "depth" )
IMPLEMENT_ENUM_END( "GLeClearType" )

static LUA_DECLARE( clear )
{
    GLeClearType type;

    LUA_ARGS_BEGIN;
    argStream.ReadEnumString( type );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack stack( driver );

    _glClear( type );
    LUA_SUCCESS;
}

static LUA_DECLARE( scissor )
{
    unsigned int x, y, width, height;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( x );
    argStream.ReadNumber( y );
    argStream.ReadNumber( width );
    argStream.ReadNumber( height );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack stack( driver );

    _glScissor( x, y, width, height );
    LUA_SUCCESS;
}

static LUA_DECLARE( getInternalFormats )
{
    gl_internalFormat::PushAvailableParams( L, (glDriver*)lua_getmethodtrans( L ) );
    return 1;
}

static const luaL_Reg driver_interface[] =
{
    LUA_METHOD( runContext ),
    LUA_METHOD( makeBatch ),
    LUA_METHOD( runBatch ),
    LUA_METHOD( present ),
    LUA_METHOD( matrixMode ),
    LUA_METHOD( pushMatrix ),
    LUA_METHOD( popMatrix ),
    LUA_METHOD( loadMatrix ),
    LUA_METHOD( getMatrix ),
    LUA_METHOD( loadIdentity ),
    LUA_METHOD( multiplyMatrix ),
    LUA_METHOD( rotate ),
    LUA_METHOD( translate ),
    LUA_METHOD( get ),
    LUA_METHOD( enable ),
    LUA_METHOD( disable ),
    LUA_METHOD( isEnabled ),
    LUA_METHOD( ortho ),
    LUA_METHOD( viewport ),
    LUA_METHOD( clear ),
    LUA_METHOD( scissor ),
    LUA_METHOD( getInternalFormats ),
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

    // Register all interfaces
    j->RegisterInterfaceTrans( L, driver_interface, 0, LUACLASS_GLDRIVER );
    j->RegisterInterfaceTrans( L, tex_driver_interface, 0, LUACLASS_GLDRIVER );
    j->RegisterInterfaceTrans( L, fbo_driver_interface, 0, LUACLASS_GLDRIVER );
    j->RegisterInterfaceTrans( L, driver_util_interface, 0, LUACLASS_GLDRIVER );
    j->RegisterInterfaceTrans( L, shader_driver_interface, 0, LUACLASS_GLDRIVER );

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

inline void TriggerExtension( glDriver *driver, const char *extension, size_t extNameLen )
{
    const std::string name( extension, extNameLen );

    if ( name == "GL_ARB_texture_non_power_of_two" )                    driver->allowTexturesOfArbitrarySize = true;
    else if ( name == "GL_EXT_texture_compression_dxt1" )               driver->supportsDXT1 = true;
    else if ( name == "GL_EXT_texture_compression_s3tc" )
    {
        driver->supportsDXT1 = true;
        driver->supportsDXT3 = true;
        driver->supportsDXT5 = true;
        driver->supportsS3TC = true;
    }
    else if ( name == "GL_ARB_framebuffer_object" )                     driver->supportsFBO = true;
    else if ( name == "GL_EXT_framebuffer_object" )                     driver->supports_FBO_EXT = true;
    else if ( name == "GL_3DFX_texture_compression_FXT1" )              driver->supportsFXT1 = true;
    else if ( name == "GL_ARB_texture_compression" )                    driver->supports_ARB_compression = true;
    else if ( name == "GL_ARB_texture_compression_bptc" )               driver->supports_ARB_BPTC = true;
    else if ( name == "GL_EXT_texture_compression_latc" )               driver->supportsLATC = true;
    else if ( name == "GL_EXT_texture_compression_rgtc" )               driver->supportsRGTC = true;
    else if ( name == "GL_KHR_texture_compression_astc_ldr" )           driver->supportsASTC = true;
    else if ( name == "GL_EXT_texture_sRGB" )                           driver->supports_srgb_compression = true;
    else if ( name == "GL_ARB_imaging" )                                driver->supports_ARB_imaging = true;
    else if ( name == "GL_ARB_vertex_shader" )                          driver->supports_ARB_vertex_shader = true;
    else if ( name == "GL_ARB_shader_objects" )                         driver->supports_ARB_shader_objects = true;
    else if ( name == "GL_ARB_vertex_program" )                         driver->supports_ARB_vertex_program = true;
}

inline void ParseExtensionString( glDriver *driver, const char *extString )
{
    const char *startName = extString;

    while ( const int chr = *extString )
    {
        if ( chr == ' ' )
        {
            if ( extString == startName )
            {
                startName++;
            }
            else
            {
                TriggerExtension( driver, startName, extString - startName );

                startName = extString + 1;
            }
        }

        extString++;
    }

    if ( extString > startName )
        TriggerExtension( driver, startName, extString - startName );
}

glDriver::glDriver( lua_State *L, Win32Dialog *wnd ) : LuaClass( L, _trefget( L, this ) )
{
    // Reference the window this driver uses as we use resources from it
    // The OpenGL driver has to be destroyed before the window.
    wnd->IncrementMethodStack();

    // Initialize the OpenGL environment
    m_window = wnd;
    deviceContext = GetDC( wnd->handle );

    isBatching = false;

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

    // Get advanced properties
    {
        glContextStack stack( this );

        vMajor = 1;
        vMinor = 0;

        // Get the OpenGL version
        sscanf( (const char*)_glGetString( GL_VERSION ), "%u.%u", &vMajor, &vMinor );

        // Parse all extensions
        allowTexturesOfArbitrarySize = false;
        supportsDXT1 = false;
        supportsDXT3 = false;
        supportsDXT5 = false;
        supportsS3TC = false;
        supportsFXT1 = false;
        supports_ARB_compression = false;
        supports_ARB_BPTC = false;
        supportsLATC = false;
        supportsRGTC = false;
        supportsASTC = false;
        supports_srgb_compression = false;
        supports_FBO_EXT = false;
        supportsFBO = false;
        supports_ARB_imaging = false;
        supports_ARB_vertex_shader = false;
        supports_ARB_shader_objects = false;
        supports_ARB_vertex_program = false;

        maxFBOColorAttachments = 1;
        maxFBOColorAttachmentsEXT = 1;

        // Get OpenGL parameters
        _glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTextureSize );

        ParseExtensionString( this, (const char*)_glGetString( GL_EXTENSIONS ) );
        
        // Set up driver context stacks
        fboDrawStack = NULL;
        fboReadStack = NULL;

        // Initialize the GLSL program context
        currentProgramARB = NULL;

#define API_LOAD_EXT( name ) \
    contextInfo->##name = (##name##_t)_wglGetProcAddress( #name )
#define API_CHECK_EXT( x ) \
    if ( !( contextInfo->##x ) ) \
    { \
        apiFatal = true; \
    }
        
        // Load all OpenGL advanced functions we'd like to use.
        glContextDescriptor *contextInfo = new glContextDescriptor;
        this->contextInfo = contextInfo;

        // Reset the context
        memset( contextInfo, 0, sizeof(*contextInfo) );

        if ( vMajor == 1 && vMinor >= 3 || vMajor > 1 )
        {
            // OpenGL 1.3 prototypes
            API_LOAD_EXT( glActiveTexture );
            API_LOAD_EXT( glClientActiveTexture );
            API_LOAD_EXT( glMultiTexCoord1d );
            API_LOAD_EXT( glMultiTexCoord1dv );
            API_LOAD_EXT( glMultiTexCoord1f );
            API_LOAD_EXT( glMultiTexCoord1fv );
            API_LOAD_EXT( glMultiTexCoord1i );
            API_LOAD_EXT( glMultiTexCoord1iv );
            API_LOAD_EXT( glMultiTexCoord1s );
            API_LOAD_EXT( glMultiTexCoord1sv );
            API_LOAD_EXT( glMultiTexCoord2d );
            API_LOAD_EXT( glMultiTexCoord2dv );
            API_LOAD_EXT( glMultiTexCoord2f );
            API_LOAD_EXT( glMultiTexCoord2fv );
            API_LOAD_EXT( glMultiTexCoord2i );
            API_LOAD_EXT( glMultiTexCoord2iv );
            API_LOAD_EXT( glMultiTexCoord2s );
            API_LOAD_EXT( glMultiTexCoord2sv );
            API_LOAD_EXT( glMultiTexCoord3d );
            API_LOAD_EXT( glMultiTexCoord3dv );
            API_LOAD_EXT( glMultiTexCoord3f );
            API_LOAD_EXT( glMultiTexCoord3fv );
            API_LOAD_EXT( glMultiTexCoord3i );
            API_LOAD_EXT( glMultiTexCoord3iv );
            API_LOAD_EXT( glMultiTexCoord3s );
            API_LOAD_EXT( glMultiTexCoord3sv );
            API_LOAD_EXT( glMultiTexCoord4d );
            API_LOAD_EXT( glMultiTexCoord4dv );
            API_LOAD_EXT( glMultiTexCoord4f );
            API_LOAD_EXT( glMultiTexCoord4fv );
            API_LOAD_EXT( glMultiTexCoord4i );
            API_LOAD_EXT( glMultiTexCoord4iv );
            API_LOAD_EXT( glMultiTexCoord4s );
            API_LOAD_EXT( glMultiTexCoord4sv );
            API_LOAD_EXT( glLoadTransposeMatrixf );
            API_LOAD_EXT( glLoadTransposeMatrixd );
            API_LOAD_EXT( glMultTransposeMatrixf );
            API_LOAD_EXT( glMultTransposeMatrixd );
            API_LOAD_EXT( glSampleCoverage );
            API_LOAD_EXT( glCompressedTexImage3D );
            API_LOAD_EXT( glCompressedTexImage2D );
            API_LOAD_EXT( glCompressedTexImage1D );
            API_LOAD_EXT( glCompressedTexSubImage3D );
            API_LOAD_EXT( glCompressedTexSubImage2D );
            API_LOAD_EXT( glCompressedTexSubImage1D );
            API_LOAD_EXT( glGetCompressedTexImage );

            // Check that all prototypes are present.
            bool apiFatal = false;

            API_CHECK_EXT( glActiveTexture );
            API_CHECK_EXT( glClientActiveTexture );
            API_CHECK_EXT( glMultiTexCoord1d );
            API_CHECK_EXT( glMultiTexCoord1dv );
            API_CHECK_EXT( glMultiTexCoord1f );
            API_CHECK_EXT( glMultiTexCoord1fv );
            API_CHECK_EXT( glMultiTexCoord1i );
            API_CHECK_EXT( glMultiTexCoord1iv );
            API_CHECK_EXT( glMultiTexCoord1s );
            API_CHECK_EXT( glMultiTexCoord1sv );
            API_CHECK_EXT( glMultiTexCoord2d );
            API_CHECK_EXT( glMultiTexCoord2dv );
            API_CHECK_EXT( glMultiTexCoord2f );
            API_CHECK_EXT( glMultiTexCoord2fv );
            API_CHECK_EXT( glMultiTexCoord2i );
            API_CHECK_EXT( glMultiTexCoord2iv );
            API_CHECK_EXT( glMultiTexCoord2s );
            API_CHECK_EXT( glMultiTexCoord2sv );
            API_CHECK_EXT( glMultiTexCoord3d );
            API_CHECK_EXT( glMultiTexCoord3dv );
            API_CHECK_EXT( glMultiTexCoord3f );
            API_CHECK_EXT( glMultiTexCoord3fv );
            API_CHECK_EXT( glMultiTexCoord3i );
            API_CHECK_EXT( glMultiTexCoord3iv );
            API_CHECK_EXT( glMultiTexCoord3s );
            API_CHECK_EXT( glMultiTexCoord3sv );
            API_CHECK_EXT( glMultiTexCoord4d );
            API_CHECK_EXT( glMultiTexCoord4dv );
            API_CHECK_EXT( glMultiTexCoord4f );
            API_CHECK_EXT( glMultiTexCoord4fv );
            API_CHECK_EXT( glMultiTexCoord4i );
            API_CHECK_EXT( glMultiTexCoord4iv );
            API_CHECK_EXT( glMultiTexCoord4s );
            API_CHECK_EXT( glMultiTexCoord4sv );
            API_CHECK_EXT( glLoadTransposeMatrixf );
            API_CHECK_EXT( glLoadTransposeMatrixd );
            API_CHECK_EXT( glMultTransposeMatrixf );
            API_CHECK_EXT( glMultTransposeMatrixd );
            API_CHECK_EXT( glSampleCoverage );
            API_CHECK_EXT( glCompressedTexImage3D );
            API_CHECK_EXT( glCompressedTexImage2D );
            API_CHECK_EXT( glCompressedTexImage1D );
            API_CHECK_EXT( glCompressedTexSubImage3D );
            API_CHECK_EXT( glCompressedTexSubImage2D );
            API_CHECK_EXT( glCompressedTexSubImage1D );
            API_CHECK_EXT( glGetCompressedTexImage );

            supports1_3 = !apiFatal;

            if ( apiFatal )
                printf( "WARNING: OpenGL 1.3 implementation found, but not all functions supported\n" );
        }
        else
            supports1_3 = false;

        if ( supports_FBO_EXT )
        {
            // Obtain FBO properties
            _glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &maxFBOColorAttachmentsEXT );

            // OpenGL EXT frame buffer prototypes
            API_LOAD_EXT( glIsRenderbufferEXT );
            API_LOAD_EXT( glBindRenderbufferEXT );
            API_LOAD_EXT( glDeleteRenderbuffersEXT );
            API_LOAD_EXT( glGenRenderbuffersEXT );
            API_LOAD_EXT( glRenderbufferStorageEXT );
            API_LOAD_EXT( glGetRenderbufferParameterivEXT );
            API_LOAD_EXT( glIsFramebufferEXT );
            API_LOAD_EXT( glBindFramebufferEXT );
            API_LOAD_EXT( glDeleteFramebuffersEXT );
            API_LOAD_EXT( glGenFramebuffersEXT );
            API_LOAD_EXT( glCheckFramebufferStatusEXT );
            API_LOAD_EXT( glFramebufferTexture1DEXT );
            API_LOAD_EXT( glFramebufferTexture2DEXT );
            API_LOAD_EXT( glFramebufferTexture3DEXT );
            API_LOAD_EXT( glFramebufferRenderbufferEXT );
            API_LOAD_EXT( glGetFramebufferAttachmentParameterivEXT );
            API_LOAD_EXT( glGenerateMipmapEXT );

            // Check that all FBO EXT API is present
            bool apiFatal = false;

            API_CHECK_EXT( glIsRenderbufferEXT );
            API_CHECK_EXT( glBindRenderbufferEXT );
            API_CHECK_EXT( glDeleteRenderbuffersEXT );
            API_CHECK_EXT( glGenRenderbuffersEXT );
            API_CHECK_EXT( glRenderbufferStorageEXT );
            API_CHECK_EXT( glGetRenderbufferParameterivEXT );
            API_CHECK_EXT( glIsFramebufferEXT );
            API_CHECK_EXT( glBindFramebufferEXT );
            API_CHECK_EXT( glDeleteFramebuffersEXT );
            API_CHECK_EXT( glGenFramebuffersEXT );
            API_CHECK_EXT( glCheckFramebufferStatusEXT );
            API_CHECK_EXT( glFramebufferTexture1DEXT );
            API_CHECK_EXT( glFramebufferTexture2DEXT );
            API_CHECK_EXT( glFramebufferTexture3DEXT );
            API_CHECK_EXT( glFramebufferRenderbufferEXT );
            API_CHECK_EXT( glGetFramebufferAttachmentParameterivEXT );
            API_CHECK_EXT( glGenerateMipmapEXT );

            if ( apiFatal )
            {
                printf( "WARNING: GL_EXT_framebuffer_object found, but not all functions supported\n" );

                supports_FBO_EXT = false;
            }
        }

        // OpenGL ARB_shader_objects API
        if ( supports_ARB_shader_objects )
        {
            API_LOAD_EXT( glDeleteObjectARB );
            API_LOAD_EXT( glGetHandleARB );
            API_LOAD_EXT( glDetachObjectARB );
            API_LOAD_EXT( glCreateShaderObjectARB );
            API_LOAD_EXT( glShaderSourceARB );
            API_LOAD_EXT( glCompileShaderARB );
            API_LOAD_EXT( glCreateProgramObjectARB );
            API_LOAD_EXT( glAttachObjectARB );
            API_LOAD_EXT( glLinkProgramARB );
            API_LOAD_EXT( glUseProgramObjectARB );
            API_LOAD_EXT( glValidateProgramARB );
            API_LOAD_EXT( glUniform1fARB );
            API_LOAD_EXT( glUniform2fARB );
            API_LOAD_EXT( glUniform3fARB );
            API_LOAD_EXT( glUniform4fARB );
            API_LOAD_EXT( glUniform1iARB );
            API_LOAD_EXT( glUniform2iARB );
            API_LOAD_EXT( glUniform3iARB );
            API_LOAD_EXT( glUniform4iARB );
            API_LOAD_EXT( glUniform1fvARB );
            API_LOAD_EXT( glUniform2fvARB );
            API_LOAD_EXT( glUniform3fvARB );
            API_LOAD_EXT( glUniform4fvARB );
            API_LOAD_EXT( glUniform1ivARB );
            API_LOAD_EXT( glUniform2ivARB );
            API_LOAD_EXT( glUniform3ivARB );
            API_LOAD_EXT( glUniform4ivARB );
            API_LOAD_EXT( glUniformMatrix2fvARB );
            API_LOAD_EXT( glUniformMatrix3fvARB );
            API_LOAD_EXT( glUniformMatrix4fvARB );
            API_LOAD_EXT( glGetObjectParameterfvARB );
            API_LOAD_EXT( glGetObjectParameterivARB );
            API_LOAD_EXT( glGetInfoLogARB );
            API_LOAD_EXT( glGetAttachedObjectsARB );
            API_LOAD_EXT( glGetUniformLocationARB );
            API_LOAD_EXT( glGetActiveUniformARB );
            API_LOAD_EXT( glGetUniformfvARB );
            API_LOAD_EXT( glGetUniformivARB );
            API_LOAD_EXT( glGetShaderSourceARB );

            // Check whether all ARB_shader_objects functions are supported.
            bool apiFatal = false;

            API_CHECK_EXT( glDeleteObjectARB );
            API_CHECK_EXT( glGetHandleARB );
            API_CHECK_EXT( glDetachObjectARB );
            API_CHECK_EXT( glCreateShaderObjectARB );
            API_CHECK_EXT( glShaderSourceARB );
            API_CHECK_EXT( glCompileShaderARB );
            API_CHECK_EXT( glCreateProgramObjectARB );
            API_CHECK_EXT( glAttachObjectARB );
            API_CHECK_EXT( glLinkProgramARB );
            API_CHECK_EXT( glUseProgramObjectARB );
            API_CHECK_EXT( glValidateProgramARB );
            API_CHECK_EXT( glUniform1fARB );
            API_CHECK_EXT( glUniform2fARB );
            API_CHECK_EXT( glUniform3fARB );
            API_CHECK_EXT( glUniform4fARB );
            API_CHECK_EXT( glUniform1iARB );
            API_CHECK_EXT( glUniform2iARB );
            API_CHECK_EXT( glUniform3iARB );
            API_CHECK_EXT( glUniform4iARB );
            API_CHECK_EXT( glUniform1fvARB );
            API_CHECK_EXT( glUniform2fvARB );
            API_CHECK_EXT( glUniform3fvARB );
            API_CHECK_EXT( glUniform4fvARB );
            API_CHECK_EXT( glUniform1ivARB );
            API_CHECK_EXT( glUniform2ivARB );
            API_CHECK_EXT( glUniform3ivARB );
            API_CHECK_EXT( glUniform4ivARB );
            API_CHECK_EXT( glUniformMatrix2fvARB );
            API_CHECK_EXT( glUniformMatrix3fvARB );
            API_CHECK_EXT( glUniformMatrix4fvARB );
            API_CHECK_EXT( glGetObjectParameterfvARB );
            API_CHECK_EXT( glGetObjectParameterivARB );
            API_CHECK_EXT( glGetInfoLogARB );
            API_CHECK_EXT( glGetAttachedObjectsARB );
            API_CHECK_EXT( glGetUniformLocationARB );
            API_CHECK_EXT( glGetActiveUniformARB );
            API_CHECK_EXT( glGetUniformfvARB );
            API_CHECK_EXT( glGetUniformivARB );
            API_CHECK_EXT( glGetShaderSourceARB );

            supports_ARB_shader_objects = !apiFatal;

            if ( apiFatal )
                printf( "WARNING: GL_ARB_shader_objects found, but not all functions supported\n" );
        }

        // OpenGL ARB_vertex_program API
        if ( supports_ARB_vertex_program )
        {
            API_LOAD_EXT( glVertexAttrib1dARB );
            API_LOAD_EXT( glVertexAttrib1dvARB );
            API_LOAD_EXT( glVertexAttrib1fARB );
            API_LOAD_EXT( glVertexAttrib1fvARB );
            API_LOAD_EXT( glVertexAttrib1sARB );
            API_LOAD_EXT( glVertexAttrib1svARB );
            API_LOAD_EXT( glVertexAttrib2dARB );
            API_LOAD_EXT( glVertexAttrib2dvARB );
            API_LOAD_EXT( glVertexAttrib2fARB );
            API_LOAD_EXT( glVertexAttrib2fvARB );
            API_LOAD_EXT( glVertexAttrib2sARB );
            API_LOAD_EXT( glVertexAttrib2svARB );
            API_LOAD_EXT( glVertexAttrib3dARB );
            API_LOAD_EXT( glVertexAttrib3dvARB );
            API_LOAD_EXT( glVertexAttrib3fARB );
            API_LOAD_EXT( glVertexAttrib3fvARB );
            API_LOAD_EXT( glVertexAttrib3sARB );
            API_LOAD_EXT( glVertexAttrib3svARB );
            API_LOAD_EXT( glVertexAttrib4NbvARB );
            API_LOAD_EXT( glVertexAttrib4NivARB );
            API_LOAD_EXT( glVertexAttrib4NsvARB );
            API_LOAD_EXT( glVertexAttrib4NubARB );
            API_LOAD_EXT( glVertexAttrib4NubvARB );
            API_LOAD_EXT( glVertexAttrib4NuivARB );
            API_LOAD_EXT( glVertexAttrib4NusvARB );
            API_LOAD_EXT( glVertexAttrib4bvARB );
            API_LOAD_EXT( glVertexAttrib4dARB );
            API_LOAD_EXT( glVertexAttrib4dvARB );
            API_LOAD_EXT( glVertexAttrib4fARB );
            API_LOAD_EXT( glVertexAttrib4fvARB );
            API_LOAD_EXT( glVertexAttrib4ivARB );
            API_LOAD_EXT( glVertexAttrib4sARB );
            API_LOAD_EXT( glVertexAttrib4svARB );
            API_LOAD_EXT( glVertexAttrib4ubvARB );
            API_LOAD_EXT( glVertexAttrib4uivARB );
            API_LOAD_EXT( glVertexAttrib4usvARB );
            API_LOAD_EXT( glVertexAttribPointerARB );
            API_LOAD_EXT( glEnableVertexAttribArrayARB );
            API_LOAD_EXT( glDisableVertexAttribArrayARB );
            API_LOAD_EXT( glGetVertexAttribdvARB );
            API_LOAD_EXT( glGetVertexAttribfvARB );
            API_LOAD_EXT( glGetVertexAttribivARB );
            API_LOAD_EXT( glGetVertexAttribPointervARB );

            // Check for extension validity
            bool apiFatal = false;

            API_CHECK_EXT( glVertexAttrib1dARB );
            API_CHECK_EXT( glVertexAttrib1dvARB );
            API_CHECK_EXT( glVertexAttrib1fARB );
            API_CHECK_EXT( glVertexAttrib1fvARB );
            API_CHECK_EXT( glVertexAttrib1sARB );
            API_CHECK_EXT( glVertexAttrib1svARB );
            API_CHECK_EXT( glVertexAttrib2dARB );
            API_CHECK_EXT( glVertexAttrib2dvARB );
            API_CHECK_EXT( glVertexAttrib2fARB );
            API_CHECK_EXT( glVertexAttrib2fvARB );
            API_CHECK_EXT( glVertexAttrib2sARB );
            API_CHECK_EXT( glVertexAttrib2svARB );
            API_CHECK_EXT( glVertexAttrib3dARB );
            API_CHECK_EXT( glVertexAttrib3dvARB );
            API_CHECK_EXT( glVertexAttrib3fARB );
            API_CHECK_EXT( glVertexAttrib3fvARB );
            API_CHECK_EXT( glVertexAttrib3sARB );
            API_CHECK_EXT( glVertexAttrib3svARB );
            API_CHECK_EXT( glVertexAttrib4NbvARB );
            API_CHECK_EXT( glVertexAttrib4NivARB );
            API_CHECK_EXT( glVertexAttrib4NsvARB );
            API_CHECK_EXT( glVertexAttrib4NubARB );
            API_CHECK_EXT( glVertexAttrib4NubvARB );
            API_CHECK_EXT( glVertexAttrib4NuivARB );
            API_CHECK_EXT( glVertexAttrib4NusvARB );
            API_CHECK_EXT( glVertexAttrib4bvARB );
            API_CHECK_EXT( glVertexAttrib4dARB );
            API_CHECK_EXT( glVertexAttrib4dvARB );
            API_CHECK_EXT( glVertexAttrib4fARB );
            API_CHECK_EXT( glVertexAttrib4fvARB );
            API_CHECK_EXT( glVertexAttrib4ivARB );
            API_CHECK_EXT( glVertexAttrib4sARB );
            API_CHECK_EXT( glVertexAttrib4svARB );
            API_CHECK_EXT( glVertexAttrib4ubvARB );
            API_CHECK_EXT( glVertexAttrib4uivARB );
            API_CHECK_EXT( glVertexAttrib4usvARB );
            API_CHECK_EXT( glVertexAttribPointerARB );
            API_CHECK_EXT( glEnableVertexAttribArrayARB );
            API_CHECK_EXT( glDisableVertexAttribArrayARB );
            API_CHECK_EXT( glGetVertexAttribdvARB );
            API_CHECK_EXT( glGetVertexAttribfvARB );
            API_CHECK_EXT( glGetVertexAttribivARB );
            API_CHECK_EXT( glGetVertexAttribPointervARB );

            supports_ARB_vertex_program = false;

            if ( apiFatal )
                printf( "WARNING: OpenGL ARB_vertex_program extension found, but not all functions supported\n" );
        }

        // OpenGL ARB_vertex_shader extension
        if ( supports_ARB_vertex_shader )
        {
            API_LOAD_EXT( glBindAttribLocationARB );
            API_LOAD_EXT( glGetActiveAttribARB );
            API_LOAD_EXT( glGetAttribLocationARB );

            // Check for extension completeness.
            bool apiFatal = false;

            API_CHECK_EXT( glBindAttribLocationARB );
            API_CHECK_EXT( glGetActiveAttribARB );
            API_CHECK_EXT( glGetAttribLocationARB );

            supports_ARB_vertex_shader = !apiFatal;

            if ( apiFatal )
                printf( "WARNING: OpenGL ARB_vertex_shader extension found, but not all functions supported\n" );
        }

        // OpenGL 2.0 API
        if ( vMajor >= 2 )
        {
            API_LOAD_EXT( glBlendEquationSeparate );
            API_LOAD_EXT( glDrawBuffers );
            API_LOAD_EXT( glStencilOpSeparate );
            API_LOAD_EXT( glStencilFuncSeparate );
            API_LOAD_EXT( glStencilMaskSeparate );
            API_LOAD_EXT( glAttachShader );
            API_LOAD_EXT( glBindAttribLocation );
            API_LOAD_EXT( glCompileShader );
            API_LOAD_EXT( glCreateProgram );
            API_LOAD_EXT( glCreateShader );
            API_LOAD_EXT( glDeleteProgram );
            API_LOAD_EXT( glDeleteShader );
            API_LOAD_EXT( glDetachShader );
            API_LOAD_EXT( glDisableVertexAttribArray );
            API_LOAD_EXT( glEnableVertexAttribArray );
            API_LOAD_EXT( glGetActiveAttrib );
            API_LOAD_EXT( glGetActiveUniform );
            API_LOAD_EXT( glGetAttachedShaders );
            API_LOAD_EXT( glGetAttribLocation );
            API_LOAD_EXT( glGetProgramiv );
            API_LOAD_EXT( glGetProgramInfoLog );
            API_LOAD_EXT( glGetShaderiv );
            API_LOAD_EXT( glGetShaderInfoLog );
            API_LOAD_EXT( glGetShaderSource );
            API_LOAD_EXT( glGetUniformLocation );
            API_LOAD_EXT( glGetUniformfv );
            API_LOAD_EXT( glGetUniformiv );
            API_LOAD_EXT( glGetVertexAttribdv );
            API_LOAD_EXT( glGetVertexAttribfv );
            API_LOAD_EXT( glGetVertexAttribiv );
            API_LOAD_EXT( glGetVertexAttribPointerv );
            API_LOAD_EXT( glIsProgram );
            API_LOAD_EXT( glIsShader );
            API_LOAD_EXT( glLinkProgram );
            API_LOAD_EXT( glShaderSource );
            API_LOAD_EXT( glUseProgram );
            API_LOAD_EXT( glUniform1f );
            API_LOAD_EXT( glUniform2f );
            API_LOAD_EXT( glUniform3f );
            API_LOAD_EXT( glUniform4f );
            API_LOAD_EXT( glUniform1i );
            API_LOAD_EXT( glUniform2i );
            API_LOAD_EXT( glUniform3i );
            API_LOAD_EXT( glUniform4i );
            API_LOAD_EXT( glUniform1fv );
            API_LOAD_EXT( glUniform2fv );
            API_LOAD_EXT( glUniform3fv );
            API_LOAD_EXT( glUniform4fv );
            API_LOAD_EXT( glUniform1iv );
            API_LOAD_EXT( glUniform2iv );
            API_LOAD_EXT( glUniform3iv );
            API_LOAD_EXT( glUniform4iv );
            API_LOAD_EXT( glUniformMatrix2fv );
            API_LOAD_EXT( glUniformMatrix3fv );
            API_LOAD_EXT( glUniformMatrix4fv );
            API_LOAD_EXT( glValidateProgram );
            API_LOAD_EXT( glVertexAttrib1d );
            API_LOAD_EXT( glVertexAttrib1dv );
            API_LOAD_EXT( glVertexAttrib1f );
            API_LOAD_EXT( glVertexAttrib1fv );
            API_LOAD_EXT( glVertexAttrib1s );
            API_LOAD_EXT( glVertexAttrib1sv );
            API_LOAD_EXT( glVertexAttrib2d );
            API_LOAD_EXT( glVertexAttrib2dv );
            API_LOAD_EXT( glVertexAttrib2f );
            API_LOAD_EXT( glVertexAttrib2fv );
            API_LOAD_EXT( glVertexAttrib2s );
            API_LOAD_EXT( glVertexAttrib2sv );
            API_LOAD_EXT( glVertexAttrib3d );
            API_LOAD_EXT( glVertexAttrib3dv );
            API_LOAD_EXT( glVertexAttrib3f );
            API_LOAD_EXT( glVertexAttrib3fv );
            API_LOAD_EXT( glVertexAttrib3s );
            API_LOAD_EXT( glVertexAttrib3sv );
            API_LOAD_EXT( glVertexAttrib4Nbv );
            API_LOAD_EXT( glVertexAttrib4Niv );
            API_LOAD_EXT( glVertexAttrib4Nsv );
            API_LOAD_EXT( glVertexAttrib4Nub );
            API_LOAD_EXT( glVertexAttrib4Nubv );
            API_LOAD_EXT( glVertexAttrib4Nuiv );
            API_LOAD_EXT( glVertexAttrib4Nusv );
            API_LOAD_EXT( glVertexAttrib4bv );
            API_LOAD_EXT( glVertexAttrib4d );
            API_LOAD_EXT( glVertexAttrib4dv );
            API_LOAD_EXT( glVertexAttrib4f );
            API_LOAD_EXT( glVertexAttrib4fv );
            API_LOAD_EXT( glVertexAttrib4iv );
            API_LOAD_EXT( glVertexAttrib4s );
            API_LOAD_EXT( glVertexAttrib4sv );
            API_LOAD_EXT( glVertexAttrib4ubv );
            API_LOAD_EXT( glVertexAttrib4uiv );
            API_LOAD_EXT( glVertexAttrib4usv );
            API_LOAD_EXT( glVertexAttribPointer );

            // Check that all OpenGL 2.0 API is present.
            bool apiFatal = false;

            API_CHECK_EXT( glBlendEquationSeparate );
            API_CHECK_EXT( glDrawBuffers );
            API_CHECK_EXT( glStencilOpSeparate );
            API_CHECK_EXT( glStencilFuncSeparate );
            API_CHECK_EXT( glStencilMaskSeparate );
            API_CHECK_EXT( glAttachShader );
            API_CHECK_EXT( glBindAttribLocation );
            API_CHECK_EXT( glCompileShader );
            API_CHECK_EXT( glCreateProgram );
            API_CHECK_EXT( glCreateShader );
            API_CHECK_EXT( glDeleteProgram );
            API_CHECK_EXT( glDeleteShader );
            API_CHECK_EXT( glDetachShader );
            API_CHECK_EXT( glDisableVertexAttribArray );
            API_CHECK_EXT( glEnableVertexAttribArray );
            API_CHECK_EXT( glGetActiveAttrib );
            API_CHECK_EXT( glGetActiveUniform );
            API_CHECK_EXT( glGetAttachedShaders );
            API_CHECK_EXT( glGetAttribLocation );
            API_CHECK_EXT( glGetProgramiv );
            API_CHECK_EXT( glGetProgramInfoLog );
            API_CHECK_EXT( glGetShaderiv );
            API_CHECK_EXT( glGetShaderInfoLog );
            API_CHECK_EXT( glGetShaderSource );
            API_CHECK_EXT( glGetUniformLocation );
            API_CHECK_EXT( glGetUniformfv );
            API_CHECK_EXT( glGetUniformiv );
            API_CHECK_EXT( glGetVertexAttribdv );
            API_CHECK_EXT( glGetVertexAttribfv );
            API_CHECK_EXT( glGetVertexAttribiv );
            API_CHECK_EXT( glGetVertexAttribPointerv );
            API_CHECK_EXT( glIsProgram );
            API_CHECK_EXT( glIsShader );
            API_CHECK_EXT( glLinkProgram );
            API_CHECK_EXT( glShaderSource );
            API_CHECK_EXT( glUseProgram );
            API_CHECK_EXT( glUniform1f );
            API_CHECK_EXT( glUniform2f );
            API_CHECK_EXT( glUniform3f );
            API_CHECK_EXT( glUniform4f );
            API_CHECK_EXT( glUniform1i );
            API_CHECK_EXT( glUniform2i );
            API_CHECK_EXT( glUniform3i );
            API_CHECK_EXT( glUniform4i );
            API_CHECK_EXT( glUniform1fv );
            API_CHECK_EXT( glUniform2fv );
            API_CHECK_EXT( glUniform3fv );
            API_CHECK_EXT( glUniform4fv );
            API_CHECK_EXT( glUniform1iv );
            API_CHECK_EXT( glUniform2iv );
            API_CHECK_EXT( glUniform3iv );
            API_CHECK_EXT( glUniform4iv );
            API_CHECK_EXT( glUniformMatrix2fv );
            API_CHECK_EXT( glUniformMatrix3fv );
            API_CHECK_EXT( glUniformMatrix4fv );
            API_CHECK_EXT( glValidateProgram );
            API_CHECK_EXT( glVertexAttrib1d );
            API_CHECK_EXT( glVertexAttrib1dv );
            API_CHECK_EXT( glVertexAttrib1f );
            API_CHECK_EXT( glVertexAttrib1fv );
            API_CHECK_EXT( glVertexAttrib1s );
            API_CHECK_EXT( glVertexAttrib1sv );
            API_CHECK_EXT( glVertexAttrib2d );
            API_CHECK_EXT( glVertexAttrib2dv );
            API_CHECK_EXT( glVertexAttrib2f );
            API_CHECK_EXT( glVertexAttrib2fv );
            API_CHECK_EXT( glVertexAttrib2s );
            API_CHECK_EXT( glVertexAttrib2sv );
            API_CHECK_EXT( glVertexAttrib3d );
            API_CHECK_EXT( glVertexAttrib3dv );
            API_CHECK_EXT( glVertexAttrib3f );
            API_CHECK_EXT( glVertexAttrib3fv );
            API_CHECK_EXT( glVertexAttrib3s );
            API_CHECK_EXT( glVertexAttrib3sv );
            API_CHECK_EXT( glVertexAttrib4Nbv );
            API_CHECK_EXT( glVertexAttrib4Niv );
            API_CHECK_EXT( glVertexAttrib4Nsv );
            API_CHECK_EXT( glVertexAttrib4Nub );
            API_CHECK_EXT( glVertexAttrib4Nubv );
            API_CHECK_EXT( glVertexAttrib4Nuiv );
            API_CHECK_EXT( glVertexAttrib4Nusv );
            API_CHECK_EXT( glVertexAttrib4bv );
            API_CHECK_EXT( glVertexAttrib4d );
            API_CHECK_EXT( glVertexAttrib4dv );
            API_CHECK_EXT( glVertexAttrib4f );
            API_CHECK_EXT( glVertexAttrib4fv );
            API_CHECK_EXT( glVertexAttrib4iv );
            API_CHECK_EXT( glVertexAttrib4s );
            API_CHECK_EXT( glVertexAttrib4sv );
            API_CHECK_EXT( glVertexAttrib4ubv );
            API_CHECK_EXT( glVertexAttrib4uiv );
            API_CHECK_EXT( glVertexAttrib4usv );
            API_CHECK_EXT( glVertexAttribPointer );

            // Only support the framework if all functions are implemented.
            supports2_0 = !apiFatal;

            if ( apiFatal )
                printf( "WARNING: OpenGL 2.0 implementation found, but not all functions supported\n" );
        }
        else
            supports2_0 = false;

        // OpenGL 3.0 API
        if ( vMajor >= 3 )
        {
            // Obtain 3.0 parameters
            _glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &maxFBOColorAttachments );

            // OpenGL 3.0 prototypes
            API_LOAD_EXT( glColorMaski );
            API_LOAD_EXT( glGetBooleani_v );
            API_LOAD_EXT( glGetIntegeri_v );
            API_LOAD_EXT( glEnablei );
            API_LOAD_EXT( glDisablei );
            API_LOAD_EXT( glIsEnabledi );
            API_LOAD_EXT( glBeginTransformFeedback );
            API_LOAD_EXT( glEndTransformFeedback );
            API_LOAD_EXT( glBindBufferRange );
            API_LOAD_EXT( glBindBufferBase );
            API_LOAD_EXT( glTransformFeedbackVaryings );
            API_LOAD_EXT( glGetTransformFeedbackVarying );
            API_LOAD_EXT( glClampColor );
            API_LOAD_EXT( glBeginConditionalRender );
            API_LOAD_EXT( glEndConditionalRender );
            API_LOAD_EXT( glVertexAttribIPointer );
            API_LOAD_EXT( glGetVertexAttribIiv );
            API_LOAD_EXT( glGetVertexAttribIuiv );
            API_LOAD_EXT( glVertexAttribI1i );
            API_LOAD_EXT( glVertexAttribI2i );
            API_LOAD_EXT( glVertexAttribI3i );
            API_LOAD_EXT( glVertexAttribI4i );
            API_LOAD_EXT( glVertexAttribI1ui );
            API_LOAD_EXT( glVertexAttribI2ui );
            API_LOAD_EXT( glVertexAttribI3ui );
            API_LOAD_EXT( glVertexAttribI4ui );
            API_LOAD_EXT( glVertexAttribI1iv );
            API_LOAD_EXT( glVertexAttribI2iv );
            API_LOAD_EXT( glVertexAttribI3iv );
            API_LOAD_EXT( glVertexAttribI4iv );
            API_LOAD_EXT( glVertexAttribI1uiv );
            API_LOAD_EXT( glVertexAttribI2uiv );
            API_LOAD_EXT( glVertexAttribI3uiv );
            API_LOAD_EXT( glVertexAttribI4uiv );
            API_LOAD_EXT( glVertexAttribI4bv );
            API_LOAD_EXT( glVertexAttribI4sv );
            API_LOAD_EXT( glVertexAttribI4ubv );
            API_LOAD_EXT( glVertexAttribI4usv );
            API_LOAD_EXT( glGetUniformuiv );
            API_LOAD_EXT( glBindFragDataLocation );
            API_LOAD_EXT( glGetFragDataLocation );
            API_LOAD_EXT( glUniform1ui );
            API_LOAD_EXT( glUniform2ui );
            API_LOAD_EXT( glUniform3ui );
            API_LOAD_EXT( glUniform4ui );
            API_LOAD_EXT( glUniform1uiv );
            API_LOAD_EXT( glUniform2uiv );
            API_LOAD_EXT( glUniform3uiv );
            API_LOAD_EXT( glUniform4uiv );
            API_LOAD_EXT( glTexParameterIiv );
            API_LOAD_EXT( glTexParameterIuiv );
            API_LOAD_EXT( glGetTexParameterIiv );
            API_LOAD_EXT( glGetTexParameterIuiv );
            API_LOAD_EXT( glClearBufferiv );
            API_LOAD_EXT( glClearBufferuiv );
            API_LOAD_EXT( glClearBufferfv );
            API_LOAD_EXT( glClearBufferfi );
            API_LOAD_EXT( glGetStringi );
            API_LOAD_EXT( glIsRenderbuffer );
            API_LOAD_EXT( glBindRenderbuffer );
            API_LOAD_EXT( glDeleteRenderbuffers );
            API_LOAD_EXT( glGenRenderbuffers );
            API_LOAD_EXT( glRenderbufferStorage );
            API_LOAD_EXT( glGetRenderbufferParameteriv );
            API_LOAD_EXT( glIsFramebuffer );
            API_LOAD_EXT( glBindFramebuffer );
            API_LOAD_EXT( glDeleteFramebuffers );
            API_LOAD_EXT( glGenFramebuffers );
            API_LOAD_EXT( glCheckFramebufferStatus );
            API_LOAD_EXT( glFramebufferTexture1D );
            API_LOAD_EXT( glFramebufferTexture2D );
            API_LOAD_EXT( glFramebufferTexture3D );
            API_LOAD_EXT( glFramebufferRenderbuffer );
            API_LOAD_EXT( glGetFramebufferAttachmentParameteriv );
            API_LOAD_EXT( glGenerateMipmap );
            API_LOAD_EXT( glBlitFramebuffer );
            API_LOAD_EXT( glRenderbufferStorageMultisample );
            API_LOAD_EXT( glFramebufferTextureLayer );
            API_LOAD_EXT( glMapBufferRange );
            API_LOAD_EXT( glFlushMappedBufferRange );
            API_LOAD_EXT( glBindVertexArray );
            API_LOAD_EXT( glDeleteVertexArrays );
            API_LOAD_EXT( glGenVertexArrays );
            API_LOAD_EXT( glIsVertexArray );

            // Check that all OpenGL 3.0 API is present
            bool apiFatal = false;

            API_CHECK_EXT( glColorMaski );
            API_CHECK_EXT( glGetBooleani_v );
            API_CHECK_EXT( glGetIntegeri_v );
            API_CHECK_EXT( glEnablei );
            API_CHECK_EXT( glDisablei );
            API_CHECK_EXT( glIsEnabledi );
            API_CHECK_EXT( glBeginTransformFeedback );
            API_CHECK_EXT( glEndTransformFeedback );
            API_CHECK_EXT( glBindBufferRange );
            API_CHECK_EXT( glBindBufferBase );
            API_CHECK_EXT( glTransformFeedbackVaryings );
            API_CHECK_EXT( glGetTransformFeedbackVarying );
            API_CHECK_EXT( glClampColor );
            API_CHECK_EXT( glBeginConditionalRender );
            API_CHECK_EXT( glEndConditionalRender );
            API_CHECK_EXT( glVertexAttribIPointer );
            API_CHECK_EXT( glGetVertexAttribIiv );
            API_CHECK_EXT( glGetVertexAttribIuiv );
            API_CHECK_EXT( glVertexAttribI1i );
            API_CHECK_EXT( glVertexAttribI2i );
            API_CHECK_EXT( glVertexAttribI3i );
            API_CHECK_EXT( glVertexAttribI4i );
            API_CHECK_EXT( glVertexAttribI1ui );
            API_CHECK_EXT( glVertexAttribI2ui );
            API_CHECK_EXT( glVertexAttribI3ui );
            API_CHECK_EXT( glVertexAttribI4ui );
            API_CHECK_EXT( glVertexAttribI1iv );
            API_CHECK_EXT( glVertexAttribI2iv );
            API_CHECK_EXT( glVertexAttribI3iv );
            API_CHECK_EXT( glVertexAttribI4iv );
            API_CHECK_EXT( glVertexAttribI1uiv );
            API_CHECK_EXT( glVertexAttribI2uiv );
            API_CHECK_EXT( glVertexAttribI3uiv );
            API_CHECK_EXT( glVertexAttribI4uiv );
            API_CHECK_EXT( glVertexAttribI4bv );
            API_CHECK_EXT( glVertexAttribI4sv );
            API_CHECK_EXT( glVertexAttribI4ubv );
            API_CHECK_EXT( glVertexAttribI4usv );
            API_CHECK_EXT( glGetUniformuiv );
            API_CHECK_EXT( glBindFragDataLocation );
            API_CHECK_EXT( glGetFragDataLocation );
            API_CHECK_EXT( glUniform1ui );
            API_CHECK_EXT( glUniform2ui );
            API_CHECK_EXT( glUniform3ui );
            API_CHECK_EXT( glUniform4ui );
            API_CHECK_EXT( glUniform1uiv );
            API_CHECK_EXT( glUniform2uiv );
            API_CHECK_EXT( glUniform3uiv );
            API_CHECK_EXT( glUniform4uiv );
            API_CHECK_EXT( glTexParameterIiv );
            API_CHECK_EXT( glTexParameterIuiv );
            API_CHECK_EXT( glGetTexParameterIiv );
            API_CHECK_EXT( glGetTexParameterIuiv );
            API_CHECK_EXT( glClearBufferiv );
            API_CHECK_EXT( glClearBufferuiv );
            API_CHECK_EXT( glClearBufferfv );
            API_CHECK_EXT( glClearBufferfi );
            API_CHECK_EXT( glGetStringi );
            API_CHECK_EXT( glIsRenderbuffer );
            API_CHECK_EXT( glBindRenderbuffer );
            API_CHECK_EXT( glDeleteRenderbuffers );
            API_CHECK_EXT( glGenRenderbuffers );
            API_CHECK_EXT( glRenderbufferStorage );
            API_CHECK_EXT( glGetRenderbufferParameteriv );
            API_CHECK_EXT( glIsFramebuffer );
            API_CHECK_EXT( glBindFramebuffer );
            API_CHECK_EXT( glDeleteFramebuffers );
            API_CHECK_EXT( glGenFramebuffers );
            API_CHECK_EXT( glCheckFramebufferStatus );
            API_CHECK_EXT( glFramebufferTexture1D );
            API_CHECK_EXT( glFramebufferTexture2D );
            API_CHECK_EXT( glFramebufferTexture3D );
            API_CHECK_EXT( glFramebufferRenderbuffer );
            API_CHECK_EXT( glGetFramebufferAttachmentParameteriv );
            API_CHECK_EXT( glGenerateMipmap );
            API_CHECK_EXT( glBlitFramebuffer );
            API_CHECK_EXT( glRenderbufferStorageMultisample );
            API_CHECK_EXT( glFramebufferTextureLayer );
            API_CHECK_EXT( glMapBufferRange );
            API_CHECK_EXT( glFlushMappedBufferRange );
            API_CHECK_EXT( glBindVertexArray );
            API_CHECK_EXT( glDeleteVertexArrays );
            API_CHECK_EXT( glGenVertexArrays );
            API_CHECK_EXT( glIsVertexArray );

            supports3_0 = !apiFatal;

            if ( apiFatal )
                printf( "WARNING: OpenGL 3.0 implementation found, but not all functions supported\n" );
        }
        else
            supports3_0 = false;
    }
}

glDriver::~glDriver( void )
{
    delete contextInfo;

    LIST_REMOVE( rootNode );

    // Destroy OpenGL environment
    _wglDeleteContext( glContext );
    ReleaseDC( m_window->handle, deviceContext );

    m_window->DecrementMethodStack();
}

#define API_LOAD( lib, name ) \
    _##name = (##name##_t)GetProcAddress( lib, #name )
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
        API_LOAD( glMainLibrary, glAccum );
        API_LOAD( glMainLibrary, glAlphaFunc );
        API_LOAD( glMainLibrary, glAreTexturesResident );
        API_LOAD( glMainLibrary, glArrayElement );
        API_LOAD( glMainLibrary, glBegin );
        API_LOAD( glMainLibrary, glBindTexture );
        API_LOAD( glMainLibrary, glBitmap );
        API_LOAD( glMainLibrary, glBlendFunc );
        API_LOAD( glMainLibrary, glCallList );
        API_LOAD( glMainLibrary, glCallLists );
        API_LOAD( glMainLibrary, glClear );
        API_LOAD( glMainLibrary, glClearAccum );
        API_LOAD( glMainLibrary, glClearColor );
        API_LOAD( glMainLibrary, glClearDepth );
        API_LOAD( glMainLibrary, glClearIndex );
        API_LOAD( glMainLibrary, glClearStencil );
        API_LOAD( glMainLibrary, glClipPlane );
        API_LOAD( glMainLibrary, glColor3b );
        API_LOAD( glMainLibrary, glColor3bv );
        API_LOAD( glMainLibrary, glColor3d );
        API_LOAD( glMainLibrary, glColor3dv );
        API_LOAD( glMainLibrary, glColor3f );
        API_LOAD( glMainLibrary, glColor3fv );
        API_LOAD( glMainLibrary, glColor3i );
        API_LOAD( glMainLibrary, glColor3iv );
        API_LOAD( glMainLibrary, glColor3s );
        API_LOAD( glMainLibrary, glColor3sv );
        API_LOAD( glMainLibrary, glColor3ub );
        API_LOAD( glMainLibrary, glColor3ubv );
        API_LOAD( glMainLibrary, glColor3ui );
        API_LOAD( glMainLibrary, glColor3uiv );
        API_LOAD( glMainLibrary, glColor3us );
        API_LOAD( glMainLibrary, glColor3usv );
        API_LOAD( glMainLibrary, glColor4b );
        API_LOAD( glMainLibrary, glColor4bv );
        API_LOAD( glMainLibrary, glColor4d );
        API_LOAD( glMainLibrary, glColor4dv );
        API_LOAD( glMainLibrary, glColor4f );
        API_LOAD( glMainLibrary, glColor4fv );
        API_LOAD( glMainLibrary, glColor4i );
        API_LOAD( glMainLibrary, glColor4iv );
        API_LOAD( glMainLibrary, glColor4s );
        API_LOAD( glMainLibrary, glColor4sv );
        API_LOAD( glMainLibrary, glColor4ub );
        API_LOAD( glMainLibrary, glColor4ubv );
        API_LOAD( glMainLibrary, glColor4ui );
        API_LOAD( glMainLibrary, glColor4uiv );
        API_LOAD( glMainLibrary, glColor4us );
        API_LOAD( glMainLibrary, glColor4usv );
        API_LOAD( glMainLibrary, glColorMask );
        API_LOAD( glMainLibrary, glColorMaterial );
        API_LOAD( glMainLibrary, glColorPointer );
        API_LOAD( glMainLibrary, glCopyPixels );
        API_LOAD( glMainLibrary, glCopyTexImage1D );
        API_LOAD( glMainLibrary, glCopyTexImage2D );
        API_LOAD( glMainLibrary, glCopyTexSubImage1D );
        API_LOAD( glMainLibrary, glCopyTexSubImage2D );
        API_LOAD( glMainLibrary, glCullFace );
        API_LOAD( glMainLibrary, glDeleteLists );
        API_LOAD( glMainLibrary, glDeleteTextures );
        API_LOAD( glMainLibrary, glDepthFunc );
        API_LOAD( glMainLibrary, glDepthMask );
        API_LOAD( glMainLibrary, glDepthRange );
        API_LOAD( glMainLibrary, glDisable );
        API_LOAD( glMainLibrary, glDisableClientState );
        API_LOAD( glMainLibrary, glDrawArrays );
        API_LOAD( glMainLibrary, glDrawBuffer );
        API_LOAD( glMainLibrary, glDrawElements );
        API_LOAD( glMainLibrary, glDrawPixels );
        API_LOAD( glMainLibrary, glEdgeFlag );
        API_LOAD( glMainLibrary, glEdgeFlagPointer );
        API_LOAD( glMainLibrary, glEdgeFlagv );
        API_LOAD( glMainLibrary, glEnable );
        API_LOAD( glMainLibrary, glEnableClientState );
        API_LOAD( glMainLibrary, glEnd );
        API_LOAD( glMainLibrary, glEndList );
        API_LOAD( glMainLibrary, glEvalCoord1d );
        API_LOAD( glMainLibrary, glEvalCoord1dv );
        API_LOAD( glMainLibrary, glEvalCoord1f );
        API_LOAD( glMainLibrary, glEvalCoord1fv );
        API_LOAD( glMainLibrary, glEvalCoord2d );
        API_LOAD( glMainLibrary, glEvalCoord2dv );
        API_LOAD( glMainLibrary, glEvalCoord2f );
        API_LOAD( glMainLibrary, glEvalCoord2fv );
        API_LOAD( glMainLibrary, glEvalMesh1 );
        API_LOAD( glMainLibrary, glEvalMesh2 );
        API_LOAD( glMainLibrary, glEvalPoint1 );
        API_LOAD( glMainLibrary, glEvalPoint2 );
        API_LOAD( glMainLibrary, glFeedbackBuffer );
        API_LOAD( glMainLibrary, glFinish );
        API_LOAD( glMainLibrary, glFlush );
        API_LOAD( glMainLibrary, glFogf );
        API_LOAD( glMainLibrary, glFogfv );
        API_LOAD( glMainLibrary, glFogi );
        API_LOAD( glMainLibrary, glFogiv );
        API_LOAD( glMainLibrary, glFrontFace );
        API_LOAD( glMainLibrary, glFrustum );
        API_LOAD( glMainLibrary, glGenLists );
        API_LOAD( glMainLibrary, glGenTextures );
        API_LOAD( glMainLibrary, glGetBooleanv );
        API_LOAD( glMainLibrary, glGetClipPlane );
        API_LOAD( glMainLibrary, glGetDoublev );
        API_LOAD( glMainLibrary, glGetError );
        API_LOAD( glMainLibrary, glGetFloatv );
        API_LOAD( glMainLibrary, glGetIntegerv );
        API_LOAD( glMainLibrary, glGetLightfv );
        API_LOAD( glMainLibrary, glGetLightiv );
        API_LOAD( glMainLibrary, glGetMapdv );
        API_LOAD( glMainLibrary, glGetMapfv );
        API_LOAD( glMainLibrary, glGetMapiv );
        API_LOAD( glMainLibrary, glGetMaterialfv );
        API_LOAD( glMainLibrary, glGetMaterialiv );
        API_LOAD( glMainLibrary, glGetPixelMapfv );
        API_LOAD( glMainLibrary, glGetPixelMapuiv );
        API_LOAD( glMainLibrary, glGetPixelMapusv );
        API_LOAD( glMainLibrary, glGetPointerv );
        API_LOAD( glMainLibrary, glGetPolygonStipple );
        API_LOAD( glMainLibrary, glGetString );
        API_LOAD( glMainLibrary, glGetTexEnvfv );
        API_LOAD( glMainLibrary, glGetTexEnviv );
        API_LOAD( glMainLibrary, glGetTexGendv );
        API_LOAD( glMainLibrary, glGetTexGenfv );
        API_LOAD( glMainLibrary, glGetTexGeniv );
        API_LOAD( glMainLibrary, glGetTexImage );
        API_LOAD( glMainLibrary, glGetTexLevelParameterfv );
        API_LOAD( glMainLibrary, glGetTexLevelParameteriv );
        API_LOAD( glMainLibrary, glGetTexParameterfv );
        API_LOAD( glMainLibrary, glGetTexParameteriv );
        API_LOAD( glMainLibrary, glHint );
        API_LOAD( glMainLibrary, glIndexMask );
        API_LOAD( glMainLibrary, glIndexPointer );
        API_LOAD( glMainLibrary, glIndexd );
        API_LOAD( glMainLibrary, glIndexdv );
        API_LOAD( glMainLibrary, glIndexf );
        API_LOAD( glMainLibrary, glIndexfv );
        API_LOAD( glMainLibrary, glIndexi );
        API_LOAD( glMainLibrary, glIndexiv );
        API_LOAD( glMainLibrary, glIndexs );
        API_LOAD( glMainLibrary, glIndexsv );
        API_LOAD( glMainLibrary, glIndexub );
        API_LOAD( glMainLibrary, glIndexubv );
        API_LOAD( glMainLibrary, glInitNames );
        API_LOAD( glMainLibrary, glInterleavedArrays );
        API_LOAD( glMainLibrary, glIsEnabled );
        API_LOAD( glMainLibrary, glIsList );
        API_LOAD( glMainLibrary, glIsTexture );
        API_LOAD( glMainLibrary, glLightModelf );
        API_LOAD( glMainLibrary, glLightModelfv );
        API_LOAD( glMainLibrary, glLightModeli );
        API_LOAD( glMainLibrary, glLightModeliv );
        API_LOAD( glMainLibrary, glLightf );
        API_LOAD( glMainLibrary, glLightfv );
        API_LOAD( glMainLibrary, glLighti );
        API_LOAD( glMainLibrary, glLightiv );
        API_LOAD( glMainLibrary, glLineStipple );
        API_LOAD( glMainLibrary, glLineWidth );
        API_LOAD( glMainLibrary, glListBase );
        API_LOAD( glMainLibrary, glLoadIdentity );
        API_LOAD( glMainLibrary, glLoadMatrixd );
        API_LOAD( glMainLibrary, glLoadMatrixf );
        API_LOAD( glMainLibrary, glLoadName );
        API_LOAD( glMainLibrary, glLogicOp );
        API_LOAD( glMainLibrary, glMap1d );
        API_LOAD( glMainLibrary, glMap1f );
        API_LOAD( glMainLibrary, glMap2d );
        API_LOAD( glMainLibrary, glMap2f );
        API_LOAD( glMainLibrary, glMapGrid1d );
        API_LOAD( glMainLibrary, glMapGrid1f );
        API_LOAD( glMainLibrary, glMapGrid2d );
        API_LOAD( glMainLibrary, glMapGrid2f );
        API_LOAD( glMainLibrary, glMaterialf );
        API_LOAD( glMainLibrary, glMaterialfv );
        API_LOAD( glMainLibrary, glMateriali );
        API_LOAD( glMainLibrary, glMaterialiv );
        API_LOAD( glMainLibrary, glMatrixMode );
        API_LOAD( glMainLibrary, glMultMatrixd );
        API_LOAD( glMainLibrary, glMultMatrixf );
        API_LOAD( glMainLibrary, glNewList );
        API_LOAD( glMainLibrary, glNormal3b );
        API_LOAD( glMainLibrary, glNormal3bv );
        API_LOAD( glMainLibrary, glNormal3d );
        API_LOAD( glMainLibrary, glNormal3dv );
        API_LOAD( glMainLibrary, glNormal3f );
        API_LOAD( glMainLibrary, glNormal3fv );
        API_LOAD( glMainLibrary, glNormal3i );
        API_LOAD( glMainLibrary, glNormal3iv );
        API_LOAD( glMainLibrary, glNormal3s );
        API_LOAD( glMainLibrary, glNormal3sv );
        API_LOAD( glMainLibrary, glNormalPointer );
        API_LOAD( glMainLibrary, glOrtho );
        API_LOAD( glMainLibrary, glPassThrough );
        API_LOAD( glMainLibrary, glPixelMapfv );
        API_LOAD( glMainLibrary, glPixelMapuiv );
        API_LOAD( glMainLibrary, glPixelMapusv );
        API_LOAD( glMainLibrary, glPixelStoref );
        API_LOAD( glMainLibrary, glPixelStorei );
        API_LOAD( glMainLibrary, glPixelTransferf );
        API_LOAD( glMainLibrary, glPixelTransferi );
        API_LOAD( glMainLibrary, glPixelZoom );
        API_LOAD( glMainLibrary, glPointSize );
        API_LOAD( glMainLibrary, glPolygonMode );
        API_LOAD( glMainLibrary, glPolygonOffset );
        API_LOAD( glMainLibrary, glPolygonStipple );
        API_LOAD( glMainLibrary, glPopAttrib );
        API_LOAD( glMainLibrary, glPopClientAttrib );
        API_LOAD( glMainLibrary, glPopMatrix );
        API_LOAD( glMainLibrary, glPopName );
        API_LOAD( glMainLibrary, glPrioritizeTextures );
        API_LOAD( glMainLibrary, glPushAttrib );
        API_LOAD( glMainLibrary, glPushClientAttrib );
        API_LOAD( glMainLibrary, glPushMatrix );
        API_LOAD( glMainLibrary, glPushName );
        API_LOAD( glMainLibrary, glRasterPos2d );
        API_LOAD( glMainLibrary, glRasterPos2dv );
        API_LOAD( glMainLibrary, glRasterPos2f );
        API_LOAD( glMainLibrary, glRasterPos2fv );
        API_LOAD( glMainLibrary, glRasterPos2i );
        API_LOAD( glMainLibrary, glRasterPos2iv );
        API_LOAD( glMainLibrary, glRasterPos2s );
        API_LOAD( glMainLibrary, glRasterPos2sv );
        API_LOAD( glMainLibrary, glRasterPos3d );
        API_LOAD( glMainLibrary, glRasterPos3dv );
        API_LOAD( glMainLibrary, glRasterPos3f );
        API_LOAD( glMainLibrary, glRasterPos3fv );
        API_LOAD( glMainLibrary, glRasterPos3i );
        API_LOAD( glMainLibrary, glRasterPos3iv );
        API_LOAD( glMainLibrary, glRasterPos3s );
        API_LOAD( glMainLibrary, glRasterPos3sv );
        API_LOAD( glMainLibrary, glRasterPos4d );
        API_LOAD( glMainLibrary, glRasterPos4dv );
        API_LOAD( glMainLibrary, glRasterPos4f );
        API_LOAD( glMainLibrary, glRasterPos4fv );
        API_LOAD( glMainLibrary, glRasterPos4i );
        API_LOAD( glMainLibrary, glRasterPos4iv );
        API_LOAD( glMainLibrary, glRasterPos4s );
        API_LOAD( glMainLibrary, glRasterPos4sv );
        API_LOAD( glMainLibrary, glReadBuffer );
        API_LOAD( glMainLibrary, glReadPixels );
        API_LOAD( glMainLibrary, glRectd );
        API_LOAD( glMainLibrary, glRectdv );
        API_LOAD( glMainLibrary, glRectf );
        API_LOAD( glMainLibrary, glRectfv );
        API_LOAD( glMainLibrary, glRecti );
        API_LOAD( glMainLibrary, glRectiv );
        API_LOAD( glMainLibrary, glRects );
        API_LOAD( glMainLibrary, glRectsv );
        API_LOAD( glMainLibrary, glRenderMode );
        API_LOAD( glMainLibrary, glRotated );
        API_LOAD( glMainLibrary, glRotatef );
        API_LOAD( glMainLibrary, glScaled );
        API_LOAD( glMainLibrary, glScalef );
        API_LOAD( glMainLibrary, glScissor );
        API_LOAD( glMainLibrary, glSelectBuffer );
        API_LOAD( glMainLibrary, glShadeModel );
        API_LOAD( glMainLibrary, glStencilFunc );
        API_LOAD( glMainLibrary, glStencilMask );
        API_LOAD( glMainLibrary, glStencilOp );
        API_LOAD( glMainLibrary, glTexCoord1d );
        API_LOAD( glMainLibrary, glTexCoord1dv );
        API_LOAD( glMainLibrary, glTexCoord1f );
        API_LOAD( glMainLibrary, glTexCoord1fv );
        API_LOAD( glMainLibrary, glTexCoord1i );
        API_LOAD( glMainLibrary, glTexCoord1iv );
        API_LOAD( glMainLibrary, glTexCoord1s );
        API_LOAD( glMainLibrary, glTexCoord1sv );
        API_LOAD( glMainLibrary, glTexCoord2d );
        API_LOAD( glMainLibrary, glTexCoord2dv );
        API_LOAD( glMainLibrary, glTexCoord2f );
        API_LOAD( glMainLibrary, glTexCoord2fv );
        API_LOAD( glMainLibrary, glTexCoord2i );
        API_LOAD( glMainLibrary, glTexCoord2iv );
        API_LOAD( glMainLibrary, glTexCoord2s );
        API_LOAD( glMainLibrary, glTexCoord2sv );
        API_LOAD( glMainLibrary, glTexCoord3d );
        API_LOAD( glMainLibrary, glTexCoord3dv );
        API_LOAD( glMainLibrary, glTexCoord3f );
        API_LOAD( glMainLibrary, glTexCoord3fv );
        API_LOAD( glMainLibrary, glTexCoord3i );
        API_LOAD( glMainLibrary, glTexCoord3iv );
        API_LOAD( glMainLibrary, glTexCoord3s );
        API_LOAD( glMainLibrary, glTexCoord3sv );
        API_LOAD( glMainLibrary, glTexCoord4d );
        API_LOAD( glMainLibrary, glTexCoord4dv );
        API_LOAD( glMainLibrary, glTexCoord4f );
        API_LOAD( glMainLibrary, glTexCoord4fv );
        API_LOAD( glMainLibrary, glTexCoord4i );
        API_LOAD( glMainLibrary, glTexCoord4iv );
        API_LOAD( glMainLibrary, glTexCoord4s );
        API_LOAD( glMainLibrary, glTexCoord4sv );
        API_LOAD( glMainLibrary, glTexCoordPointer );
        API_LOAD( glMainLibrary, glTexEnvf );
        API_LOAD( glMainLibrary, glTexEnvfv );
        API_LOAD( glMainLibrary, glTexEnvi );
        API_LOAD( glMainLibrary, glTexEnviv );
        API_LOAD( glMainLibrary, glTexGend );
        API_LOAD( glMainLibrary, glTexGendv );
        API_LOAD( glMainLibrary, glTexGenf );
        API_LOAD( glMainLibrary, glTexGenfv );
        API_LOAD( glMainLibrary, glTexGeni );
        API_LOAD( glMainLibrary, glTexGeniv );
        API_LOAD( glMainLibrary, glTexImage1D );
        API_LOAD( glMainLibrary, glTexImage2D );
        API_LOAD( glMainLibrary, glTexParameterf );
        API_LOAD( glMainLibrary, glTexParameterfv );
        API_LOAD( glMainLibrary, glTexParameteri );
        API_LOAD( glMainLibrary, glTexParameteriv );
        API_LOAD( glMainLibrary, glTexSubImage1D );
        API_LOAD( glMainLibrary, glTexSubImage2D );
        API_LOAD( glMainLibrary, glTranslated );
        API_LOAD( glMainLibrary, glTranslatef );
        API_LOAD( glMainLibrary, glVertex2d );
        API_LOAD( glMainLibrary, glVertex2dv );
        API_LOAD( glMainLibrary, glVertex2f );
        API_LOAD( glMainLibrary, glVertex2fv );
        API_LOAD( glMainLibrary, glVertex2i );
        API_LOAD( glMainLibrary, glVertex2iv );
        API_LOAD( glMainLibrary, glVertex2s );
        API_LOAD( glMainLibrary, glVertex2sv );
        API_LOAD( glMainLibrary, glVertex3d );
        API_LOAD( glMainLibrary, glVertex3dv );
        API_LOAD( glMainLibrary, glVertex3f );
        API_LOAD( glMainLibrary, glVertex3fv );
        API_LOAD( glMainLibrary, glVertex3i );
        API_LOAD( glMainLibrary, glVertex3iv );
        API_LOAD( glMainLibrary, glVertex3s );
        API_LOAD( glMainLibrary, glVertex3sv );
        API_LOAD( glMainLibrary, glVertex4d );
        API_LOAD( glMainLibrary, glVertex4dv );
        API_LOAD( glMainLibrary, glVertex4f );
        API_LOAD( glMainLibrary, glVertex4fv );
        API_LOAD( glMainLibrary, glVertex4i );
        API_LOAD( glMainLibrary, glVertex4iv );
        API_LOAD( glMainLibrary, glVertex4s );
        API_LOAD( glMainLibrary, glVertex4sv );
        API_LOAD( glMainLibrary, glVertexPointer );
        API_LOAD( glMainLibrary, glViewport );

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

        // Have we failed initializing?
        if ( apiFatal )
        {
            printf( "failed to load 'opengl32.dll'\n\n" );

            FreeLibrary( glMainLibrary );
            glMainLibrary = NULL;
        }
    }

    if ( glMainLibrary )
    {
        // OpenGL glu utility library
        gluLibrary = LoadLibrary( "Glu32.dll" );

        if ( gluLibrary )
        {
            API_LOAD( gluLibrary, gluErrorString );
            API_LOAD( gluLibrary, gluErrorUnicodeStringEXT );
            API_LOAD( gluLibrary, gluGetString );
            API_LOAD( gluLibrary, gluOrtho2D );
            API_LOAD( gluLibrary, gluPerspective );
            API_LOAD( gluLibrary, gluPickMatrix );
            API_LOAD( gluLibrary, gluLookAt );
            API_LOAD( gluLibrary, gluProject );
            API_LOAD( gluLibrary, gluUnProject );
            API_LOAD( gluLibrary, gluScaleImage );
            API_LOAD( gluLibrary, gluBuild1DMipmaps );
            API_LOAD( gluLibrary, gluBuild2DMipmaps );
            API_LOAD( gluLibrary, gluNewQuadric );
            API_LOAD( gluLibrary, gluDeleteQuadric );
            API_LOAD( gluLibrary, gluQuadricNormals );
            API_LOAD( gluLibrary, gluQuadricTexture );
            API_LOAD( gluLibrary, gluQuadricOrientation );
            API_LOAD( gluLibrary, gluQuadricDrawStyle );
            API_LOAD( gluLibrary, gluCylinder );
            API_LOAD( gluLibrary, gluDisk );
            API_LOAD( gluLibrary, gluPartialDisk );
            API_LOAD( gluLibrary, gluSphere );
            API_LOAD( gluLibrary, gluQuadricCallback );
            API_LOAD( gluLibrary, gluNewTess );
            API_LOAD( gluLibrary, gluDeleteTess );
            API_LOAD( gluLibrary, gluTessBeginPolygon );
            API_LOAD( gluLibrary, gluTessBeginContour );
            API_LOAD( gluLibrary, gluTessVertex );
            API_LOAD( gluLibrary, gluTessEndContour );
            API_LOAD( gluLibrary, gluTessEndPolygon );
            API_LOAD( gluLibrary, gluTessProperty );
            API_LOAD( gluLibrary, gluTessNormal );
            API_LOAD( gluLibrary, gluTessCallback );
            API_LOAD( gluLibrary, gluGetTessProperty );
            API_LOAD( gluLibrary, gluNewNurbsRenderer );
            API_LOAD( gluLibrary, gluDeleteNurbsRenderer );
            API_LOAD( gluLibrary, gluBeginSurface );
            API_LOAD( gluLibrary, gluBeginCurve );
            API_LOAD( gluLibrary, gluEndCurve );
            API_LOAD( gluLibrary, gluEndSurface );
            API_LOAD( gluLibrary, gluBeginTrim );
            API_LOAD( gluLibrary, gluEndTrim );
            API_LOAD( gluLibrary, gluPwlCurve );
            API_LOAD( gluLibrary, gluNurbsCurve );
            API_LOAD( gluLibrary, gluNurbsSurface );
            API_LOAD( gluLibrary, gluLoadSamplingMatrices );
            API_LOAD( gluLibrary, gluNurbsProperty );
            API_LOAD( gluLibrary, gluGetNurbsProperty );
            API_LOAD( gluLibrary, gluNurbsCallback );
            API_LOAD( gluLibrary, gluBeginPolygon );
            API_LOAD( gluLibrary, gluNextContour );
            API_LOAD( gluLibrary, gluEndPolygon );

            // Check that all API is linked
            bool apiFatal = false;

            API_CHECK( "Glu32.dll", _gluErrorString );
            API_CHECK( "Glu32.dll", _gluErrorUnicodeStringEXT );
            API_CHECK( "Glu32.dll", _gluGetString );
            API_CHECK( "Glu32.dll", _gluOrtho2D );
            API_CHECK( "Glu32.dll", _gluPerspective );
            API_CHECK( "Glu32.dll", _gluPickMatrix );
            API_CHECK( "Glu32.dll", _gluLookAt );
            API_CHECK( "Glu32.dll", _gluProject );
            API_CHECK( "Glu32.dll", _gluUnProject );
            API_CHECK( "Glu32.dll", _gluScaleImage );
            API_CHECK( "Glu32.dll", _gluBuild1DMipmaps );
            API_CHECK( "Glu32.dll", _gluBuild2DMipmaps );
            API_CHECK( "Glu32.dll", _gluNewQuadric );
            API_CHECK( "Glu32.dll", _gluDeleteQuadric );
            API_CHECK( "Glu32.dll", _gluQuadricNormals );
            API_CHECK( "Glu32.dll", _gluQuadricTexture );
            API_CHECK( "Glu32.dll", _gluQuadricOrientation );
            API_CHECK( "Glu32.dll", _gluQuadricDrawStyle );
            API_CHECK( "Glu32.dll", _gluCylinder );
            API_CHECK( "Glu32.dll", _gluDisk );
            API_CHECK( "Glu32.dll", _gluPartialDisk );
            API_CHECK( "Glu32.dll", _gluSphere );
            API_CHECK( "Glu32.dll", _gluQuadricCallback );
            API_CHECK( "Glu32.dll", _gluNewTess );
            API_CHECK( "Glu32.dll", _gluDeleteTess );
            API_CHECK( "Glu32.dll", _gluTessBeginPolygon );
            API_CHECK( "Glu32.dll", _gluTessBeginContour );
            API_CHECK( "Glu32.dll", _gluTessVertex );
            API_CHECK( "Glu32.dll", _gluTessEndContour );
            API_CHECK( "Glu32.dll", _gluTessEndPolygon );
            API_CHECK( "Glu32.dll", _gluTessProperty );
            API_CHECK( "Glu32.dll", _gluTessNormal );
            API_CHECK( "Glu32.dll", _gluTessCallback );
            API_CHECK( "Glu32.dll", _gluGetTessProperty );
            API_CHECK( "Glu32.dll", _gluNewNurbsRenderer );
            API_CHECK( "Glu32.dll", _gluDeleteNurbsRenderer );
            API_CHECK( "Glu32.dll", _gluBeginSurface );
            API_CHECK( "Glu32.dll", _gluBeginCurve );
            API_CHECK( "Glu32.dll", _gluEndCurve );
            API_CHECK( "Glu32.dll", _gluEndSurface );
            API_CHECK( "Glu32.dll", _gluBeginTrim );
            API_CHECK( "Glu32.dll", _gluEndTrim );
            API_CHECK( "Glu32.dll", _gluPwlCurve );
            API_CHECK( "Glu32.dll", _gluNurbsCurve );
            API_CHECK( "Glu32.dll", _gluNurbsSurface );
            API_CHECK( "Glu32.dll", _gluLoadSamplingMatrices );
            API_CHECK( "Glu32.dll", _gluNurbsProperty );
            API_CHECK( "Glu32.dll", _gluGetNurbsProperty );
            API_CHECK( "Glu32.dll", _gluNurbsCallback );
            API_CHECK( "Glu32.dll", _gluBeginPolygon );
            API_CHECK( "Glu32.dll", _gluNextContour );
            API_CHECK( "Glu32.dll", _gluEndPolygon );

            // Have we failed initializing?
            if ( apiFatal )
            {
                printf( "failed to load 'Glu32.dll'\n\n" );

                FreeLibrary( gluLibrary );
                gluLibrary = NULL;
            }
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

void luagl_pulseDrivers( lua_State* )
{
    lua_State *L = g_L;
    int top = lua_gettop( L );

    try
    {
        // Frame the drivers
        LIST_FOREACH_BEGIN( glDriver, allDevices.root, rootNode )
            glContextStack stack( item );

            item->PushMethod( L, "triggerEvent" );
            lua_pushcstring( L, "onFrame" );
            lua_call( L, 1, 0 );
        LIST_FOREACH_END
    }
    catch( ... )
    {
        lua_settop( L, top );
        throw;
    }
}