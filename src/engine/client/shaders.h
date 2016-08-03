#ifndef ENGINE_CLIENT_SHADERS_H
#define ENGINE_CLIENT_SHADERS_H

#include "SDL.h"
#include "SDL_opengl.h"
 

typedef struct {
	GLhandleARB program;
	GLhandleARB vert_shader;
	GLhandleARB frag_shader;
	const char *vert_source;
    const char *frag_source;
} ShaderData;

static PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
static PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
static PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
static PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
static PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
static PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
static PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
static PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
static PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
static PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
static PFNGLUNIFORM1IARBPROC glUniform1iARB;
static PFNGLUNIFORM1FARBPROC glUniform1fARB;
static PFNGLUNIFORM2FVPROC glUniform2fv;
static PFNGLUNIFORM4FVPROC glUniform4fv;
static PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;



GLuint LoadShader(const char *vertex_path, const char *fragment_path);
	

#endif