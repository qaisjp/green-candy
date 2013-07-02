/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_context.h
*  PURPOSE:     OpenGL driver internal/private header file
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _OPENGL_CONTEXT_DESCRIPTOR_
#define _OPENGL_CONTEXT_DESCRIPTOR_

struct glContextDescriptor
{
    // OpenGL 1.3 prototypes
    glActiveTexture_t               glActiveTexture;
    glClientActiveTexture_t         glClientActiveTexture;
    glMultiTexCoord1d_t             glMultiTexCoord1d;
    glMultiTexCoord1dv_t            glMultiTexCoord1dv;
    glMultiTexCoord1f_t             glMultiTexCoord1f;
    glMultiTexCoord1fv_t            glMultiTexCoord1fv;
    glMultiTexCoord1i_t             glMultiTexCoord1i;
    glMultiTexCoord1iv_t            glMultiTexCoord1iv;
    glMultiTexCoord1s_t             glMultiTexCoord1s;
    glMultiTexCoord1sv_t            glMultiTexCoord1sv;
    glMultiTexCoord2d_t             glMultiTexCoord2d;
    glMultiTexCoord2dv_t            glMultiTexCoord2dv;
    glMultiTexCoord2f_t             glMultiTexCoord2f;
    glMultiTexCoord2fv_t            glMultiTexCoord2fv;
    glMultiTexCoord2i_t             glMultiTexCoord2i;
    glMultiTexCoord2iv_t            glMultiTexCoord2iv;
    glMultiTexCoord2s_t             glMultiTexCoord2s;
    glMultiTexCoord2sv_t            glMultiTexCoord2sv;
    glMultiTexCoord3d_t             glMultiTexCoord3d;
    glMultiTexCoord3dv_t            glMultiTexCoord3dv;
    glMultiTexCoord3f_t             glMultiTexCoord3f;
    glMultiTexCoord3fv_t            glMultiTexCoord3fv;
    glMultiTexCoord3i_t             glMultiTexCoord3i;
    glMultiTexCoord3iv_t            glMultiTexCoord3iv;
    glMultiTexCoord3s_t             glMultiTexCoord3s;
    glMultiTexCoord3sv_t            glMultiTexCoord3sv;
    glMultiTexCoord4d_t             glMultiTexCoord4d;
    glMultiTexCoord4dv_t            glMultiTexCoord4dv;
    glMultiTexCoord4f_t             glMultiTexCoord4f;
    glMultiTexCoord4fv_t            glMultiTexCoord4fv;
    glMultiTexCoord4i_t             glMultiTexCoord4i;
    glMultiTexCoord4iv_t            glMultiTexCoord4iv;
    glMultiTexCoord4s_t             glMultiTexCoord4s;
    glMultiTexCoord4sv_t            glMultiTexCoord4sv;
    glLoadTransposeMatrixf_t        glLoadTransposeMatrixf;
    glLoadTransposeMatrixd_t        glLoadTransposeMatrixd;
    glMultTransposeMatrixf_t        glMultTransposeMatrixf;
    glMultTransposeMatrixd_t        glMultTransposeMatrixd;
    glSampleCoverage_t              glSampleCoverage;
    glCompressedTexImage3D_t        glCompressedTexImage3D;
    glCompressedTexImage2D_t        glCompressedTexImage2D;
    glCompressedTexImage1D_t        glCompressedTexImage1D;
    glCompressedTexSubImage3D_t     glCompressedTexSubImage3D;
    glCompressedTexSubImage2D_t     glCompressedTexSubImage2D;
    glCompressedTexSubImage1D_t     glCompressedTexSubImage1D;
    glGetCompressedTexImage_t       glGetCompressedTexImage;

    // OpenGL EXT render buffer prototypes
    glIsRenderbufferEXT_t                           glIsRenderbufferEXT;
    glBindRenderbufferEXT_t                         glBindRenderbufferEXT;
    glDeleteRenderbuffersEXT_t                      glDeleteRenderbuffersEXT;
    glGenRenderbuffersEXT_t                         glGenRenderbuffersEXT;
    glRenderbufferStorageEXT_t                      glRenderbufferStorageEXT;
    glGetRenderbufferParameterivEXT_t               glGetRenderbufferParameterivEXT;
    glIsFramebufferEXT_t                            glIsFramebufferEXT;
    glBindFramebufferEXT_t                          glBindFramebufferEXT;
    glDeleteFramebuffersEXT_t                       glDeleteFramebuffersEXT;
    glGenFramebuffersEXT_t                          glGenFramebuffersEXT;
    glCheckFramebufferStatusEXT_t                   glCheckFramebufferStatusEXT;
    glFramebufferTexture1DEXT_t                     glFramebufferTexture1DEXT;
    glFramebufferTexture2DEXT_t                     glFramebufferTexture2DEXT;
    glFramebufferTexture3DEXT_t                     glFramebufferTexture3DEXT;
    glFramebufferRenderbufferEXT_t                  glFramebufferRenderbufferEXT;
    glGetFramebufferAttachmentParameterivEXT_t      glGetFramebufferAttachmentParameterivEXT;
    glGenerateMipmapEXT_t                           glGenerateMipmapEXT;

    // OpenGL 3.0 prototypes
    glColorMaski_t                              glColorMaski;
    glGetBooleani_v_t                           glGetBooleani_v;
    glGetIntegeri_v_t                           glGetIntegeri_v;
    glEnablei_t                                 glEnablei;
    glDisablei_t                                glDisablei;
    glIsEnabledi_t                              glIsEnabledi;
    glBeginTransformFeedback_t                  glBeginTransformFeedback;
    glEndTransformFeedback_t                    glEndTransformFeedback;
    glBindBufferRange_t                         glBindBufferRange;
    glBindBufferBase_t                          glBindBufferBase;
    glTransformFeedbackVaryings_t               glTransformFeedbackVaryings;
    glGetTransformFeedbackVarying_t             glGetTransformFeedbackVarying;
    glClampColor_t                              glClampColor;
    glBeginConditionalRender_t                  glBeginConditionalRender;
    glEndConditionalRender_t                    glEndConditionalRender;
    glVertexAttribIPointer_t                    glVertexAttribIPointer;
    glGetVertexAttribIiv_t                      glGetVertexAttribIiv;
    glGetVertexAttribIuiv_t                     glGetVertexAttribIuiv;
    glVertexAttribI1i_t                         glVertexAttribI1i;
    glVertexAttribI2i_t                         glVertexAttribI2i;
    glVertexAttribI3i_t                         glVertexAttribI3i;
    glVertexAttribI4i_t                         glVertexAttribI4i;
    glVertexAttribI1ui_t                        glVertexAttribI1ui;
    glVertexAttribI2ui_t                        glVertexAttribI2ui;
    glVertexAttribI3ui_t                        glVertexAttribI3ui;
    glVertexAttribI4ui_t                        glVertexAttribI4ui;
    glVertexAttribI1iv_t                        glVertexAttribI1iv;
    glVertexAttribI2iv_t                        glVertexAttribI2iv;
    glVertexAttribI3iv_t                        glVertexAttribI3iv;
    glVertexAttribI4iv_t                        glVertexAttribI4iv;
    glVertexAttribI1uiv_t                       glVertexAttribI1uiv;
    glVertexAttribI2uiv_t                       glVertexAttribI2uiv;
    glVertexAttribI3uiv_t                       glVertexAttribI3uiv;
    glVertexAttribI4uiv_t                       glVertexAttribI4uiv;
    glVertexAttribI4bv_t                        glVertexAttribI4bv;
    glVertexAttribI4sv_t                        glVertexAttribI4sv;
    glVertexAttribI4ubv_t                       glVertexAttribI4ubv;
    glVertexAttribI4usv_t                       glVertexAttribI4usv;
    glGetUniformuiv_t                           glGetUniformuiv;
    glBindFragDataLocation_t                    glBindFragDataLocation;
    glGetFragDataLocation_t                     glGetFragDataLocation;
    glUniform1ui_t                              glUniform1ui;
    glUniform2ui_t                              glUniform2ui;
    glUniform3ui_t                              glUniform3ui;
    glUniform4ui_t                              glUniform4ui;
    glUniform1uiv_t                             glUniform1uiv;
    glUniform2uiv_t                             glUniform2uiv;
    glUniform3uiv_t                             glUniform3uiv;
    glUniform4uiv_t                             glUniform4uiv;
    glTexParameterIiv_t                         glTexParameterIiv;
    glTexParameterIuiv_t                        glTexParameterIuiv;
    glGetTexParameterIiv_t                      glGetTexParameterIiv;
    glGetTexParameterIuiv_t                     glGetTexParameterIuiv;
    glClearBufferiv_t                           glClearBufferiv;
    glClearBufferuiv_t                          glClearBufferuiv;
    glClearBufferfv_t                           glClearBufferfv;
    glClearBufferfi_t                           glClearBufferfi;
    glGetStringi_t                              glGetStringi;
    glIsRenderbuffer_t                          glIsRenderbuffer;
    glBindRenderbuffer_t                        glBindRenderbuffer;
    glDeleteRenderbuffers_t                     glDeleteRenderbuffers;
    glGenRenderbuffers_t                        glGenRenderbuffers;
    glRenderbufferStorage_t                     glRenderbufferStorage;
    glGetRenderbufferParameteriv_t              glGetRenderbufferParameteriv;
    glIsFramebuffer_t                           glIsFramebuffer;
    glBindFramebuffer_t                         glBindFramebuffer;
    glDeleteFramebuffers_t                      glDeleteFramebuffers;
    glGenFramebuffers_t                         glGenFramebuffers;
    glCheckFramebufferStatus_t                  glCheckFramebufferStatus;
    glFramebufferTexture1D_t                    glFramebufferTexture1D;
    glFramebufferTexture2D_t                    glFramebufferTexture2D;
    glFramebufferTexture3D_t                    glFramebufferTexture3D;
    glFramebufferRenderbuffer_t                 glFramebufferRenderbuffer;
    glGetFramebufferAttachmentParameteriv_t     glGetFramebufferAttachmentParameteriv;
    glGenerateMipmap_t                          glGenerateMipmap;
    glBlitFramebuffer_t                         glBlitFramebuffer;
    glRenderbufferStorageMultisample_t          glRenderbufferStorageMultisample;
    glFramebufferTextureLayer_t                 glFramebufferTextureLayer;
    glMapBufferRange_t                          glMapBufferRange;
    glFlushMappedBufferRange_t                  glFlushMappedBufferRange;
    glBindVertexArray_t                         glBindVertexArray;
    glDeleteVertexArrays_t                      glDeleteVertexArrays;
    glGenVertexArrays_t                         glGenVertexArrays;
    glIsVertexArray_t                           glIsVertexArray;

};

#endif //_OPENGL_CONTEXT_DESCRIPTOR_