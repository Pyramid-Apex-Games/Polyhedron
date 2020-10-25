#pragma once

//#define OPEN_GL_ES

#include <glad/glad.h>
#include <SDL.h>

#if defined(glMultiDrawArraysEXT) && !defined(glMultiDrawArrays)
#define glMultiDrawArrays glMultiDrawArraysEXT
#endif
#define glMultiDrawArrays_ glMultiDrawArrays

#if defined(glMultiDrawElementsEXT) && !defined(glMultiDrawElements)
#define glMultiDrawElements glMultiDrawElementsEXT
#endif
#define glMultiDrawElements_ glMultiDrawElements

#define glBlendFuncSeparate_ glBlendFuncSeparate
#define glBlendEquationSeparate_ glBlendEquationSeparate
#define glStencilOpSeparate_ glStencilOpSeparate
#define glStencilFuncSeparate_ glStencilFuncSeparate
#define glStencilMaskSeparate_ glStencilMaskSeparate

#define glGenBuffers_ glGenBuffers
#define glBindBuffer_ glBindBuffer
#define glMapBuffer_ glMapBuffer
#define glUnmapBuffer_ glUnmapBuffer
#define glBufferData_ glBufferData
#define glBufferSubData_ glBufferSubData
#define glDeleteBuffers_ glDeleteBuffers
#define glGetBufferSubData_ glGetBufferSubData

#define glGenQueries_ glGenQueries
#define glDeleteQueries_ glDeleteQueries
#define glBeginQuery_ glBeginQuery
#define glEndQuery_ glEndQuery
#define glGetQueryiv_ glGetQueryiv
#define glGetQueryObjectiv_ glGetQueryObjectiv
#define glGetQueryObjectuiv_ glGetQueryObjectuiv

#define glCreateProgram_ glCreateProgram
#define glDeleteProgram_ glDeleteProgram
#define glUseProgram_ glUseProgram
#define glCreateShader_ glCreateShader
#define glDeleteShader_ glDeleteShader
#define glShaderSource_ glShaderSource
#define glCompileShader_ glCompileShader
#define glGetShaderiv_ glGetShaderiv
#define glGetProgramiv_ glGetProgramiv
#define glAttachShader_ glAttachShader
#define glDetachShader_ glDetachShader
#define glGetProgramInfoLog_ glGetProgramInfoLog
#define glGetShaderInfoLog_ glGetShaderInfoLog
#define glLinkProgram_ glLinkProgram
#define glGetUniformLocation_ glGetUniformLocation
#define glGetAttribLocation_ glGetAttribLocation
#define glUniform1f_ glUniform1f
#define glUniform2f_ glUniform2f
#define glUniform3f_ glUniform3f
#define glUniform4f_ glUniform4f
#define glUniform1fv_ glUniform1fv
#define glUniform2fv_ glUniform2fv
#define glUniform3fv_ glUniform3fv
#define glUniform4fv_ glUniform4fv
#define glUniform1i_ glUniform1i
#define glUniform2i_ glUniform2i
#define glUniform3i_ glUniform3i
#define glUniform4i_ glUniform4i
#define glUniform1iv_ glUniform1iv
#define glUniform2iv_ glUniform2iv
#define glUniform3iv_ glUniform3iv
#define glUniform4iv_ glUniform4iv
#define glUniformMatrix2fv_ glUniformMatrix2fv
#define glUniformMatrix3fv_ glUniformMatrix3fv
#define glUniformMatrix4fv_ glUniformMatrix4fv
#define glBindAttribLocation_ glBindAttribLocation
#define glGetActiveUniform_ glGetActiveUniform
#define glEnableVertexAttribArray_ glEnableVertexAttribArray
#define glDisableVertexAttribArray_ glDisableVertexAttribArray

#define glVertexAttrib1f_ glVertexAttrib1f
#define glVertexAttrib1fv_ glVertexAttrib1fv
#define glVertexAttrib1s_ glVertexAttrib1s
#define glVertexAttrib1sv_ glVertexAttrib1sv
#define glVertexAttrib2f_ glVertexAttrib2f
#define glVertexAttrib2fv_ glVertexAttrib2fv
#define glVertexAttrib2s_ glVertexAttrib2s
#define glVertexAttrib2sv_ glVertexAttrib2sv
#define glVertexAttrib3f_ glVertexAttrib3f
#define glVertexAttrib3fv_ glVertexAttrib3fv
#define glVertexAttrib3s_ glVertexAttrib3s
#define glVertexAttrib3sv_ glVertexAttrib3sv
#define glVertexAttrib4f_ glVertexAttrib4f
#define glVertexAttrib4fv_ glVertexAttrib4fv
#define glVertexAttrib4s_ glVertexAttrib4s
#define glVertexAttrib4sv_ glVertexAttrib4sv
#define glVertexAttrib4bv_ glVertexAttrib4bv
#define glVertexAttrib4iv_ glVertexAttrib4iv
#define glVertexAttrib4ubv_ glVertexAttrib4ubv
#define glVertexAttrib4uiv_ glVertexAttrib4uiv
#define glVertexAttrib4usv_ glVertexAttrib4usv
#define glVertexAttrib4Nbv_ glVertexAttrib4Nbv
#define glVertexAttrib4Niv_ glVertexAttrib4Niv
#define glVertexAttrib4Nub_ glVertexAttrib4Nub
#define glVertexAttrib4Nubv_ glVertexAttrib4Nubv
#define glVertexAttrib4Nuiv_ glVertexAttrib4Nuiv
#define glVertexAttrib4Nusv_ glVertexAttrib4Nusv
#define glVertexAttribPointer_ glVertexAttribPointer

#define glUniform1uiv_ glUniform1uiv
#define glUniform2uiv_ glUniform2uiv
#define glUniform3uiv_ glUniform3uiv
#define glUniform4uiv_ glUniform4uiv
#define glUniform1ui_ glUniform1ui
#define glUniform2ui_ glUniform2ui
#define glUniform3ui_ glUniform3ui
#define glUniform4ui_ glUniform4ui

#define glActiveTexture_ glActiveTexture

#define glBindRenderbuffer_ glBindRenderbuffer
#define glDeleteRenderbuffers_ glDeleteRenderbuffers
#define glGenRenderbuffers_ glGenRenderbuffers
#define glRenderbufferStorage_ glRenderbufferStorage
#define glCheckFramebufferStatus_ glCheckFramebufferStatus
#define glBindFramebuffer_ glBindFramebuffer
#define glDeleteFramebuffers_ glDeleteFramebuffers
#define glGenFramebuffers_ glGenFramebuffers
#define glFramebufferTexture2D_ glFramebufferTexture2D
#define glFramebufferRenderbuffer_ glFramebufferRenderbuffer

#define glDrawBuffers_ glDrawBuffers

#define glGetStringi_ glGetStringi

#define glCopyTexSubImage3D_ glCopyTexSubImage3D
#define glClearDepth_ glClearDepthf

#define glMinSampleShading_ glMinSampleShading
#define glBindVertexArray_ glBindVertexArray
#define glDeleteVertexArrays_ glDeleteVertexArrays
#define glGenVertexArrays_ glGenVertexArrays
#define glIsVertexArray_ glIsVertexArray
#define glBindFragDataLocation_ glBindFragDataLocation
#define glClearBufferiv_ glClearBufferiv
#define glClearBufferuiv_ glClearBufferuiv
#define glClearBufferfv_ glClearBufferfv
#define glClearBufferfi_ glClearBufferfi
#define glClampColor_ glClampColor
#define glColorMaski_ glColorMaski
#define glEnablei_ glEnablei
#define glDisablei_ glDisablei
#define glBeginConditionalRender_ glBeginConditionalRender
#define glEndConditionalRender_ glEndConditionalRender
#define glTexParameterIiv_ glTexParameterIiv
#define glTexParameterIuiv_ glTexParameterIuiv
#define glGetTexParameterIiv_ glGetTexParameterIiv
#define glGetTexParameterIuiv_ glGetTexParameterIuiv
#define glClearColorIi_ glClearColorIi
#define glClearColorIui_ glClearColorIui
#define glFramebufferTexture1D_ glFramebufferTexture1D
#define glGenerateMipmap_ glGenerateMipmap
#define glBlitFramebuffer_ glBlitFramebuffer
#define glRenderbufferStorageMultisample_ glRenderbufferStorageMultisample
#define glMapBufferRange_ glMapBufferRange
#define glFlushMappedBufferRange_ glFlushMappedBufferRange
#define glGetUniformIndices_ glGetUniformIndices
#define glGetActiveUniformsiv_ glGetActiveUniformsiv
#define glGetUniformBlockIndex_ glGetUniformBlockIndex
#define glGetActiveUniformBlockiv_ glGetActiveUniformBlockiv
#define glUniformBlockBinding_ glUniformBlockBinding
#define glBindBufferBase_ glBindBufferBase
#define glBindBufferRange_ glBindBufferRange
#define glCopyBufferSubData_ glCopyBufferSubData
#define glTexImage2DMultisample_ glTexImage2DMultisample
#define glTexImage3DMultisample_ glTexImage3DMultisample
#define glGetMultisamplefv_ glGetMultisamplefv
#define glSampleMaski_ glSampleMaski
#define glGetQueryObjecti64v_ glGetQueryObjecti64v
#define glGetQueryObjectui64v_ glGetQueryObjectui64v
#if defined(glDepthBoundsf) && !defined(glDepthBounds)
#define glDepthBounds glDepthBoundsf
#endif
#define glDepthBounds_ glDepthBounds
#define glBindFragDataLocationIndexed_ glBindFragDataLocationIndexed
#define glBlendEquationi_ glBlendEquationi
#define glBlendEquationSeparatei_ glBlendEquationSeparatei
#define glBlendFunci_ glBlendFunci
#define glBlendFuncSeparatei_ glBlendFuncSeparatei
#define glDebugMessageControl_ glDebugMessageControl
#define glDebugMessageInsert_ glDebugMessageInsert
#define glDebugMessageCallback_ glDebugMessageCallback
#define glGetDebugMessageLog_ glGetDebugMessageLog
#define glCopyImageSubData_ glCopyImageSubData

#define glDrawRangeElements_ glDrawRangeElements
#define glFramebufferTexture3D_ glFramebufferTexture3DOES

#define glCompressedTexImage1D_ glCompressedTexImage1D
#define glCompressedTexImage2D_ glCompressedTexImage2D
#define glGetCompressedTexImage_ glGetCompressedTexImage
#define glTexImage3D_ glTexImage3D

#ifdef OPEN_GL_ES
#define glDepthRange_ glDepthRangef
#else
#define glDepthRange_ glDepthRange
#endif

#ifndef GL_TEXTURE_RECTANGLE
#ifdef GL_TEXTURE_RECTANGLE_ARB
#define GL_TEXTURE_RECTANGLE GL_TEXTURE_RECTANGLE_ARB
#endif
#ifdef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE GL_TEXTURE_RECTANGLE_NV
#endif
#endif

#ifndef GL_RGB5_EXT
#define GL_RGB5_EXT 0x8050
#endif

#ifndef GL_RGB16_EXT
#define GL_RGB16_EXT 0x8054
#endif

#ifndef GL_R16_EXT
#define GL_R16_EXT 0x822A
#endif

#ifdef GL_LUMINANCE8_OES
#define GL_LUMINANCE8 GL_LUMINANCE8_OES
#endif
#ifdef GL_LUMINANCE8_EXT
#define GL_LUMINANCE8 GL_LUMINANCE8_EXT
#endif
#ifdef GL_LUMINANCE8_ALPHA8_OES
#define GL_LUMINANCE8_ALPHA8 GL_LUMINANCE8_ALPHA8_OES
#endif
#ifdef GL_LUMINANCE8_ALPHA8_EXT
#define GL_LUMINANCE8_ALPHA8 GL_LUMINANCE8_ALPHA8_EXT
#endif
#if defined(GL_RGB10_EXT) && !defined(GL_RGB10)
#define GL_RGB10 GL_RGB10_EXT
#endif
#if defined(GL_R16_EXT) && !defined(GL_R16)
#define GL_R16 GL_R16_EXT
#endif
#if defined(GL_RGB16_EXT) && !defined(GL_RGB16)
#define GL_RGB16 GL_RGB16_EXT
#endif
#if defined(GL_DEPTH_COMPONENT32_OES) && !defined(GL_DEPTH_COMPONENT32)
#define GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
#endif

#if defined(GL_COMPRESSED_RED_RGTC1_EXT) && !defined(GL_COMPRESSED_RED_RGTC1)
#define GL_COMPRESSED_RED_RGTC1 GL_COMPRESSED_RED_RGTC1_EXT
#endif

#ifdef GL_ONE_MINUS_SRC1_ALPHA_EXT
#define GL_ONE_MINUS_SRC1_ALPHA GL_ONE_MINUS_SRC1_ALPHA_EXT
#endif
#ifdef GL_SRC1_ALPHA_EXT
#define GL_SRC1_ALPHA GL_SRC1_ALPHA_EXT
#endif
#ifndef GL_RGB5
#define GL_RGB5 GL_RGB5_EXT
#endif
//#define GL_COMPRESSED_RG_RGTC2 GL_COMPRESSED_RG_RGTC2_EXT
#define GL_COMPRESSED_LUMINANCE_LATC1 GL_COMPRESSED_LUMINANCE_LATC1_EXT
#define GL_COMPRESSED_LUMINANCE_ALPHA_LATC2 GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1 GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#ifndef GL_BGRA
#define GL_BGRA GL_BGRA_EXT
#endif

#ifndef GL_LUMINANCE16
#define GL_LUMINANCE16 GL_LUMINANCE16_EXT
#endif

#ifndef GL_LUMINANCE16_ALPHA16
#define GL_LUMINANCE16_ALPHA16 GL_LUMINANCE16_ALPHA16_EXT
#endif

#ifndef GL_ALPHA8
#define GL_ALPHA8 GL_ALPHA8_EXT
#endif

#ifndef GL_ALPHA16
#define GL_ALPHA16 GL_ALPHA16_EXT
#endif

#ifndef GL_COMPRESSED_ALPHA
#define GL_COMPRESSED_ALPHA					0x84E9
#endif
#ifndef GL_COMPRESSED_LUMINANCE
#define GL_COMPRESSED_LUMINANCE				0x84EA
#endif
#ifndef GL_COMPRESSED_LUMINANCE_ALPHA
#define GL_COMPRESSED_LUMINANCE_ALPHA		0x84EB
#endif
#define GL_COMPRESSED_RED					0x8225
#define GL_COMPRESSED_RG					0x8226

#ifdef GL_TIME_ELAPSED_EXT
#define GL_TIME_ELAPSED GL_TIME_ELAPSED_EXT
#endif

#ifdef GL_SCALED_RESOLVE_FASTEST_EXT
#define GL_SCALED_RESOLVE_FASTEST GL_SCALED_RESOLVE_FASTEST_EXT
#endif
#ifndef GL_SCALED_RESOLVE_FASTEST
#define GL_SCALED_RESOLVE_FASTEST GL_NEAREST
#endif

//#ifdef DEBUG
GLenum DebugOpenGL(const char *expression, const char *file, int line);
#define glCheckError(EXPR) EXPR; DebugOpenGL(#EXPR, __FILE__, __LINE__)
//#else
//#define glCheckError(EXPR)
//#endif