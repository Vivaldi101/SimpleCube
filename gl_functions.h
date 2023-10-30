#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <GL/gl.h>
#include "glcorearb.h"  // download from https://www.khronos.org/registry/OpenGL/api/GL/glcorearb.h
#include "wglext.h"     // download from https://www.khronos.org/registry/OpenGL/api/GL/wglext.h

// make sure you use functions that are valid for selected GL version (specified when context is created)
#define GL_FUNCTIONS(X) \
    X(PFNGLGENBUFFERSPROC,				 glGenBuffers               ) \
    X(PFNGLBINDBUFFERPROC,				 glBindBuffer               ) \
    X(PFNGLCREATEBUFFERSPROC,            glCreateBuffers            ) \
    X(PFNGLNAMEDBUFFERSTORAGEPROC,       glNamedBufferStorage       ) \
    X(PFNGLBUFFERSTORAGEPROC,			 glBufferStorage			) \
    X(PFNGLBINDVERTEXARRAYPROC,          glBindVertexArray          ) \
    X(PFNGLISVERTEXARRAYPROC,			 glIsVertexArray            ) \
    X(PFNGLISBUFFERPROC,			     glIsBuffer            ) \
    X(PFNGLBINDVERTEXBUFFERPROC,         glBindVertexBuffer         ) \
    X(PFNGLGENVERTEXARRAYSPROC,			 glGenVertexArrays			) \
    X(PFNGLCREATEVERTEXARRAYSPROC,       glCreateVertexArrays       ) \
    X(PFNGLVERTEXARRAYATTRIBBINDINGPROC, glVertexArrayAttribBinding ) \
    X(PFNGLVERTEXATTRIBBINDINGPROC,		 glVertexAttribBinding		) \
    X(PFNGLVERTEXARRAYVERTEXBUFFERPROC,  glVertexArrayVertexBuffer  ) \
    X(PFNGLVERTEXARRAYATTRIBFORMATPROC,  glVertexArrayAttribFormat  ) \
    X(PFNGLVERTEXATTRIBFORMATPROC,		 glVertexAttribFormat		) \
    X(PFNGLENABLEVERTEXARRAYATTRIBPROC,  glEnableVertexArrayAttrib  ) \
    X(PFNGLENABLEVERTEXATTRIBARRAYPROC,	 glEnableVertexAttribArray	) \
    X(PFNGLCREATESHADERPROGRAMVPROC,     glCreateShaderProgramv     ) \
    X(PFNGLGETPROGRAMIVPROC,             glGetProgramiv             ) \
    X(PFNGLGETPROGRAMINFOLOGPROC,        glGetProgramInfoLog        ) \
    X(PFNGLGENPROGRAMPIPELINESPROC,      glGenProgramPipelines      ) \
    X(PFNGLUSEPROGRAMSTAGESPROC,         glUseProgramStages         ) \
    X(PFNGLBINDPROGRAMPIPELINEPROC,      glBindProgramPipeline      ) \
    X(PFNGLISPROGRAMPIPELINEPROC,        glIsProgramPipeline        ) \
    X(PFNGLISPROGRAMPROC,                glIsProgram        ) \
    X(PFNGLPROGRAMUNIFORMMATRIX2FVPROC,  glProgramUniformMatrix2fv  ) \
    X(PFNGLPROGRAMUNIFORMMATRIX4FVPROC,  glProgramUniformMatrix4fv  ) \
    X(PFNGLUNIFORMMATRIX4FVPROC,  glUniformMatrix4fv  ) \
    X(PFNGLBINDTEXTUREUNITPROC,          glBindTextureUnit          ) \
    X(PFNGLCREATETEXTURESPROC,           glCreateTextures           ) \
    X(PFNGLTEXTUREPARAMETERIPROC,        glTextureParameteri        ) \
    X(PFNGLTEXTURESTORAGE2DPROC,         glTextureStorage2D         ) \
    X(PFNGLTEXTURESUBIMAGE2DPROC,        glTextureSubImage2D        ) \
    X(PFNGLGENFRAMEBUFFERSPROC,			 glGenFramebuffers			) \
    X(PFNGLBINDFRAMEBUFFERPROC,			 glBindFramebuffer			) \
    X(PFNGLFRAMEBUFFERTEXTURE2DPROC,	 glFramebufferTexture2D		) \
    X(PFNGLDRAWBUFFERSPROC,				 glDrawBuffers				) \
    X(PFNGLCHECKFRAMEBUFFERSTATUSPROC,	 glCheckFramebufferStatus   ) \
    X(PFNGLUSEPROGRAMPROC,				 glUseProgram				) \
    X(PFNGLLINKPROGRAMPROC,				 glLinkProgram				) \
    X(PFNGLPROGRAMUNIFORM3FPROC,		 glProgramUniform3f			) \
    X(PFNGLPROGRAMUNIFORM2IVPROC,		 glProgramUniform2iv		) \
    X(PFNGLPROGRAMUNIFORM2FVPROC,		 glProgramUniform2fv		) \
    X(PFNGLPROGRAMUNIFORM3UIPROC,		 glProgramUniform3ui		) \
    X(PFNGLPROGRAMUNIFORM2UIPROC,		 glProgramUniform2ui		) \
    X(PFNGLPROGRAMUNIFORM1UIPROC,		 glProgramUniform1ui		) \
    X(PFNGLPROGRAMUNIFORM1FPROC,		 glProgramUniform1f		) \
    X(PFNGLPROGRAMUNIFORM1IPROC,		 glProgramUniform1i		) \
    X(PFNGLUNIFORM1FPROC,		 glUniform1f		) \
    X(PFNGLUNIFORM1UIPROC,		 glUniform1ui		) \
    X(PFNGLUNIFORM2IVPROC,		 glUniform2iv		) \
    X(PFNGLGETATTACHEDSHADERSPROC,	     glGetAttachedShaders	    ) \
    X(PFNGLDELETESHADERPROC,	     glDeleteShader	    ) \
    X(PFNGLCREATESHADERPROC,	     glCreateShader	    ) \
    X(PFNGLSHADERSOURCEPROC,	     glShaderSource	    ) \
    X(PFNGLCOMPILESHADERPROC,	     glCompileShader	    ) \
    X(PFNGLGETSHADERIVPROC,	     glGetShaderiv	    ) \
    X(PFNGLGETSHADERINFOLOGPROC,	     glGetShaderInfoLog	    ) \
    X(PFNGLCREATEPROGRAMPROC,	     glCreateProgram	    ) \
    X(PFNGLDELETEPROGRAMPROC,	     glDeleteProgram	    ) \
    X(PFNGLVALIDATEPROGRAMPROC,	     glValidateProgram	    ) \
    X(PFNGLATTACHSHADERPROC,	     glAttachShader	    ) \
    X(PFNGLDETACHSHADERPROC,	     glDetachShader	    ) \
    X(PFNGLISSHADERPROC,	     glIsShader	    ) \
    X(PFNGLGETUNIFORMLOCATIONPROC,	     glGetUniformLocation	    ) \
    X(PFNGLVERTEXATTRIBPOINTERPROC,	     glVertexAttribPointer	    ) \
    X(PFNGLBUFFERDATAPROC,	     glBufferData	    ) \
    X(PFNGLBUFFERSUBDATAPROC,	     glBufferSubData	    ) \
    X(PFNGLDEBUGMESSAGECALLBACKPROC,     glDebugMessageCallback     )

#define X(type, name) static type name;
GL_FUNCTIONS(X)
#undef X

