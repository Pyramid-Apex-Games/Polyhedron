// shader.cpp: OpenGL GLSL shader management

#include "engine.h"
#include "engine/texture.h"
#include "engine/command.h"
#include "engine/rendergl.h"
#include "engine/renderlights.h"
#include "engine/octarender.h"
#include "engine/rendermodel.h"
#include "engine/menus.h"
#include "engine/GLFeatures.h"
#include "engine/main/Renderer.h"
#include "engine/main/Compatibility.h"

Shader *Shader::lastshader = NULL;

Shader *nullshader = NULL, *hudshader = NULL, *hudtextshader = NULL, *hudnotextureshader = NULL, *nocolorshader = NULL, *foggedshader = NULL, *foggednotextureshader = NULL, *ldrshader = NULL, *ldrnotextureshader = NULL, *stdworldshader = NULL;

static hashnameset<GlobalShaderParamState> globalparams(256);
static hashtable<const char *, int> localparams(256);
static hashnameset<Shader> shaders(256);
static Shader *slotshader = NULL;
static vector<SlotShaderParam> slotparams;
static bool standardshaders = false, forceshaders = true, loadedshaders = false;

VAR(maxvsuniforms, 1, 0, 0);
VAR(maxfsuniforms, 1, 0, 0);
VAR(mintexoffset, 1, 0, 0);
VAR(maxtexoffset, 1, 0, 0);
VAR(mintexrectoffset, 1, 0, 0);
VAR(maxtexrectoffset, 1, 0, 0);
//VAR(dbgshader, 0, 0, 2);
VAR(dbgshader, 0, 1, 2);

void loadshaders()
{
    standardshaders = true;
    execfile("config/glsl.cfg");
    standardshaders = false;

    nullshader = lookupshaderbyname("null");
    hudshader = lookupshaderbyname("hud");
    hudtextshader = lookupshaderbyname("hudtext");
    hudnotextureshader = lookupshaderbyname("hudnotexture");
    stdworldshader = lookupshaderbyname("stdworld");
    if(!nullshader || !hudshader || !hudtextshader || !hudnotextureshader || !stdworldshader) fatal("cannot find shader definitions");

    dummyslot.shader = stdworldshader;
    dummydecalslot.shader = nullshader;

    nocolorshader = lookupshaderbyname("nocolor");
    foggedshader = lookupshaderbyname("fogged");
    foggednotextureshader = lookupshaderbyname("foggednotexture");
    ldrshader = lookupshaderbyname("ldr");
    ldrnotextureshader = lookupshaderbyname("ldrnotexture");

    nullshader->set();

    loadedshaders = true;
}

Shader *lookupshaderbyname(const char *name)
{
    Shader *s = shaders.access(name);
    return s && s->loaded() ? s : NULL;
}

Shader *generateshader(const char *name, const char *fmt, ...)
{
    if(!loadedshaders) return NULL;
    Shader *s = name ? lookupshaderbyname(name) : NULL;
    if(!s)
    {
        defvformatcubestr(cmd, fmt, fmt);
        bool wasstandard = standardshaders;
        standardshaders = true;
        execute(cmd);
        standardshaders = wasstandard;
        s = name ? lookupshaderbyname(name) : NULL;
        if(!s) s = nullshader;
    }
    return s;
}

static void showglslinfo(GLenum type, GLuint obj, const char *name, const char **parts = NULL, int numparts = 0)
{
    GLint length = 0;
    if(type){
        glCheckError(glGetShaderiv_(obj, GL_INFO_LOG_LENGTH, &length));
    }
    else
    {
        glCheckError(glGetProgramiv_(obj, GL_INFO_LOG_LENGTH, &length));
    }
    if(length > 1)
    {
        conoutf(CON_ERROR, "GLSL ERROR (%s:%s)", type == GL_VERTEX_SHADER ? "VS" : (type == GL_FRAGMENT_SHADER ? "FS" : "PROG"), name);

		std::string log;
		log.reserve(length + 1);
		if(type)
		{
			glCheckError(glGetShaderInfoLog_(obj, length, &length, log.data()));
		}
		else
		{
			glCheckError(glGetProgramInfoLog_(obj, length, &length, log.data()));
		}

		conoutf(CON_ERROR, ">> %s", log.c_str());

		for (int i = 0; i < numparts; ++i)
		{
			int line = 0;
			const char *part = parts[i];
			while(*part)
			{
				const char *next = strchr(part, '\n');
				if(++line > 1000) return;

				if (!next)
				{
					conoutf(CON_ERROR, ">>% 2d:%02d: %s<<<", i, line, part);
					break;
				}
				else
				{
					std::string strLine(part, next - part);
					conoutf(CON_ERROR, ">>% 2d:%02d: %s", i, line, strLine.c_str());
				}
				part = next + 1;
			}
		}
    }
}

static const char *finddecls(const char *line)
{
    for(;;)
    {
        const char *start = line + strspn(line, " \t\r");
        switch(*start)
        {
            case '\n':
                line = start + 1;
                continue;
            case '#':
                do
                {
                    start = strchr(start + 1, '\n');
                    if(!start) return NULL;
                } while(start[-1] == '\\');
                line = start + 1;
                continue;
            case '/':
                switch(start[1])
                {
                    case '/':
                        start = strchr(start + 2, '\n');
                        if(!start) return NULL;
                        line = start + 1;
                        continue;
                    case '*':
                        start = strstr(start + 2, "*/");
                        if(!start) return NULL;
                        line = start + 2;
                        continue;
                }
                // fall-through
            default:
                return line;
        }
    }
}

extern int amd_eal_bug;

std::string GetShaderVersionHeader()
{
    struct OpenGLShaderVersion {
        int version;
        std::string header;
        OpenGLShaderVersion(int version, const std::string &header)
            : version(version)
            , header(header)
        {}
    };

#ifndef OPEN_GL_ES
    static const std::array<OpenGLShaderVersion, 7> glslVersions {
        OpenGLShaderVersion { 400, "#version 400\n" },
        OpenGLShaderVersion { 330, "#version 330\n" },
        OpenGLShaderVersion { 320, "#version 150\n" },
        OpenGLShaderVersion { 150, "#version 150\n" },
        OpenGLShaderVersion { 140, "#version 140\n" },
        OpenGLShaderVersion { 130, "#version 130\n" },
        OpenGLShaderVersion { 120, "#version 120\n" }
    };
#else
    static const std::array<OpenGLShaderVersion, 6> glslVersions {
        OpenGLShaderVersion { 400, "#version 400\n" },
        OpenGLShaderVersion { 330, "#version 330 es\nprecision mediump float;\nprecision mediump sampler3D;\n" },
        OpenGLShaderVersion { 320, "#version 320 es\nprecision mediump float;\nprecision mediump sampler3D;\n" },
        OpenGLShaderVersion { 310, "#version 310 es\nprecision mediump float;\nprecision mediump sampler3D;\n" },
        OpenGLShaderVersion { 300, "#version 300 es\nprecision mediump float;\nprecision mediump sampler3D;\n" },
        OpenGLShaderVersion { 200, "#version 100\nprecision mediump float;\nprecision mediump sampler3D;\n" }
    };
#endif
    for(auto glslVersion : glslVersions) {
        if (GLFeatures::ShaderVersion() >= glslVersion.version) {
            return glslVersion.header;
        }
    }

    return "#version 100";
}

static void compileglslshader(Shader &s, GLenum type, GLuint &obj, const char *def, const char *name, bool msg = true)
{
    const char *source = def + strspn(def, " \t\r\n");
    char *modsource = NULL;
    const char *parts[16];
    int numparts = 0;

    parts[numparts++] = GetShaderVersionHeader().c_str();

    if(GLFeatures::ShaderVersion() < 140)
    {
        parts[numparts++] = "#extension GL_ARB_texture_rectangle : enable\n";
        if(GLFeatures::HasEGPU4())
            parts[numparts++] = "#extension GL_EXT_gpu_shader4 : enable\n";
    }
    if(GLFeatures::ShaderVersion() < 150 && GLFeatures::HasTMS())
        parts[numparts++] = "#extension GL_ARB_texture_multisample : enable\n";
    if(GLFeatures::ShaderVersion() >= 150 && GLFeatures::ShaderVersion() < 330 && GLFeatures::HasEAL() && !amd_eal_bug)
        parts[numparts++] = "#extension GL_ARB_explicit_attrib_location : enable\n";
    if(GLFeatures::ShaderVersion() < 400)
    {
        if(GLFeatures::HasTG()) parts[numparts++] = "#extension GL_ARB_texture_gather : enable\n";
        if(GLFeatures::HasGPU5()) parts[numparts++] = "#extension GL_ARB_gpu_shader5 : enable\n";
    }
    if(GLFeatures::ShaderVersion() >= 130)
    {
        if(type == GL_VERTEX_SHADER) parts[numparts++] =
            "#define attribute in\n"
            "#define varying out\n";
        else if(type == GL_FRAGMENT_SHADER)
        {
            parts[numparts++] = "#define varying in\n";
            parts[numparts++] = (GLFeatures::ShaderVersion() >= 330 || (GLFeatures::ShaderVersion() >= 150 &&
                    GLFeatures::HasEAL()) || (GLFeatures::ShaderVersion() >= 300 && GLFeatures::HasGLES())) && !amd_eal_bug ?
                "#define fragdata(loc) layout(location = loc) out\n"
                "#define fragblend(loc) layout(location = loc, index = 1) out\n" :
                "#define fragdata(loc) out\n"
                "#define fragblend(loc) out\n";
            if(GLFeatures::ShaderVersion() < 150)
            {
                const char *decls = finddecls(source);
                if(decls)
                {
                    static const char * const prec = "precision highp float;\n";
                    if(decls != source)
                    {
                        static const int preclen = strlen(prec);
                        int beforelen = int(decls-source), afterlen = strlen(decls);
                        modsource = newcubestr(beforelen + preclen + afterlen);
                        memcpy(modsource, source, beforelen);
                        memcpy(&modsource[beforelen], prec, preclen);
                        memcpy(&modsource[beforelen + preclen], decls, afterlen);
                        modsource[beforelen + preclen + afterlen] = '\0';
                    }
                    else parts[numparts++] = prec;
                }
            }
        }
        parts[numparts++] =
            "#define texture1D(sampler, coords) texture(sampler, coords)\n"
            "#define texture2D(sampler, coords) texture(sampler, coords)\n"
            "#define texture2DOffset(sampler, coords, offset) textureOffset(sampler, coords, offset)\n"
            "#define texture2DProj(sampler, coords) textureProj(sampler, coords)\n"
            "#define shadow2D(sampler, coords) texture(sampler, coords)\n"
            "#define shadow2DOffset(sampler, coords, offset) textureOffset(sampler, coords, offset)\n"
            "#define texture3D(sampler, coords) texture(sampler, coords)\n"
            "#define textureCube(sampler, coords) texture(sampler, coords)\n";
        if(GLFeatures::ShaderVersion() >= 140)
        {
            parts[numparts++] =
                "#define texture2DRect(sampler, coords) texture(sampler, coords)\n"
                "#define texture2DRectProj(sampler, coords) textureProj(sampler, coords)\n"
                "#define shadow2DRect(sampler, coords) texture(sampler, coords)\n";
            extern int mesa_texrectoffset_bug;
            parts[numparts++] = mesa_texrectoffset_bug ?
                "#define texture2DRectOffset(sampler, coords, offset) texture(sampler, coords + vec2(offset))\n"
                "#define shadow2DRectOffset(sampler, coords, offset) texture(sampler, coords + vec2(offset))\n" :
                "#define texture2DRectOffset(sampler, coords, offset) textureOffset(sampler, coords, offset)\n"
                "#define shadow2DRectOffset(sampler, coords, offset) textureOffset(sampler, coords, offset)\n";
        }
    }
    if(GLFeatures::ShaderVersion() < 130 && GLFeatures::HasEGPU4()) parts[numparts++] = "#define uint unsigned int\n";
    else if(GLFeatures::ShaderVersion() < 140 && !GLFeatures::HasEGPU4())
    {
        if(GLFeatures::ShaderVersion() < 130) parts[numparts++] = "#define flat\n";
        parts[numparts++] =
            "#define texture2DRectOffset(sampler, coords, offset) texture2DRect(sampler, coords + vec2(offset))\n"
            "#define shadow2DRectOffset(sampler, coords, offset) shadow2DRect(sampler, coords + vec2(offset))\n";
    }
    if(GLFeatures::ShaderVersion() < 130 && type == GL_FRAGMENT_SHADER)
    {
        if(GLFeatures::HasEGPU4())
        {
            parts[numparts++] =
                "#define fragdata(loc) varying out\n"
                "#define fragblend(loc) varying out\n";
        }
        else
        {
            loopv(s.fragdatalocs)
            {
                FragDataLoc &d = s.fragdatalocs[i];
                if(d.index) continue;
                if(i >= 4) break;
                static cubestr defs[4];
                const char *swizzle = "";
                switch(d.format)
                {
                    case GL_UNSIGNED_INT_VEC2:
                    case GL_INT_VEC2:
                    case GL_FLOAT_VEC2: swizzle = ".rg"; break;
                    case GL_UNSIGNED_INT_VEC3:
                    case GL_INT_VEC3:
                    case GL_FLOAT_VEC3: swizzle = ".rgb"; break;
                    case GL_UNSIGNED_INT:
                    case GL_INT:
                    case GL_FLOAT: swizzle = ".r"; break;
                }
                formatcubestr(defs[i], "#define %s gl_FragData[%d]%s\n", d.name, d.loc, swizzle);
                parts[numparts++] = defs[i];
            }
        }
    }
    parts[numparts++] = modsource ? modsource : source;

    obj = glCheckError(glCreateShader_(type));
    glCheckError(glShaderSource_(obj, numparts, (const GLchar **)parts, NULL));
    glCheckError(glCompileShader_(obj));
    GLint success;
    glCheckError(glGetShaderiv_(obj, GL_COMPILE_STATUS, &success));
    if(!success)
    {
        if(msg) showglslinfo(type, obj, name, parts, numparts);
        glCheckError(glDeleteShader_(obj));
        obj = 0;
    }
    else if(dbgshader > 1 && msg) showglslinfo(type, obj, name, parts, numparts);

    if(modsource) delete[] modsource;
}

VAR(dbgubo, 0, 0, 1);

static void bindglsluniform(Shader &s, UniformLoc &u)
{
    u.loc = glCheckError(glGetUniformLocation_(s.program, u.name));
    if(!u.blockname || !GLFeatures::HasUBO()) return;
    GLuint bidx = glCheckError(glGetUniformBlockIndex_(s.program, u.blockname));
    GLuint uidx = GL_INVALID_INDEX;
    glCheckError(glGetUniformIndices_(s.program, 1, &u.name, &uidx));
    if(bidx != GL_INVALID_INDEX && uidx != GL_INVALID_INDEX)
    {
        GLint sizeval = 0, offsetval = 0, strideval = 0;
        glCheckError(glGetActiveUniformBlockiv_(s.program, bidx, GL_UNIFORM_BLOCK_DATA_SIZE, &sizeval));
        if(sizeval <= 0) return;
        glCheckError(glGetActiveUniformsiv_(s.program, 1, &uidx, GL_UNIFORM_OFFSET, &offsetval));
        if(u.stride > 0)
        {
            glCheckError(glGetActiveUniformsiv_(s.program, 1, &uidx, GL_UNIFORM_ARRAY_STRIDE, &strideval));
            if(strideval > u.stride) return;
        }
        u.offset = offsetval;
        u.size = sizeval;
        glCheckError(glUniformBlockBinding_(s.program, bidx, u.binding));
        if(dbgubo) conoutf(CON_DEBUG, "UBO: %s:%s:%d, offset: %d, size: %d, stride: %d", u.name, u.blockname, u.binding, offsetval, sizeval, strideval);
    }
}

static void bindworldtexlocs(Shader &s)
{
#define UNIFORMTEX(name, tmu) \
    do { \
        int loc = glCheckError(glGetUniformLocation_(s.program, name)); \
        if(loc != -1) { glCheckError(glUniform1i_(loc, tmu)); } \
    } while(0)
    UNIFORMTEX("diffusemap", TEX_DIFFUSE);
    UNIFORMTEX("normalmap", TEX_NORMAL);
    UNIFORMTEX("glowmap", TEX_GLOW);
    UNIFORMTEX("envmap", TEX_ENVMAP);
    UNIFORMTEX("detaildiffusemap", TEX_DETAIL+TEX_DIFFUSE);
    UNIFORMTEX("detailnormalmap", TEX_DETAIL+TEX_NORMAL);
    UNIFORMTEX("blendmap", 7);
    UNIFORMTEX("refractmask", 7);
    UNIFORMTEX("refractlight", 8);
}

static void linkglslprogram(Shader &s, bool msg = true)
{
    s.program = s.vsobj && s.psobj ? glCreateProgram_() : 0;
    GLint success = 0;
    if(s.program)
    {
        glCheckError(glAttachShader_(s.program, s.vsobj));
        glCheckError(glAttachShader_(s.program, s.psobj));
        uint attribs = 0;
        loopv(s.attriblocs)
        {
            AttribLoc &a = s.attriblocs[i];
            glCheckError(glBindAttribLocation_(s.program, a.loc, a.name));
            attribs |= 1<<a.loc;
        }
        loopi(gle::MAXATTRIBS) if(!(attribs&(1<<i))){
            glCheckError(glBindAttribLocation_(s.program, i, gle::attribnames[i]));
        }
#ifndef OPEN_GL_ES
        if(GLFeatures::HasGPU4() && ((GLFeatures::ShaderVersion() < 330 && (GLFeatures::ShaderVersion() < 150 || !GLFeatures::HasEAL())) || amd_eal_bug)) loopv(s.fragdatalocs)
        {
            FragDataLoc &d = s.fragdatalocs[i];
            if(d.index)
            {
                if(maxdualdrawbufs){
                    glCheckError(glBindFragDataLocationIndexed_(s.program, d.loc, d.index, d.name));
                }
            }
            else
            {
                glCheckError(glBindFragDataLocation_(s.program, d.loc, d.name));
            }
        }
#endif
        glCheckError(glLinkProgram_(s.program));
        glCheckError(glGetProgramiv_(s.program, GL_LINK_STATUS, &success));
    }
    if(success)
    {
        glCheckError(glUseProgram_(s.program));
        loopi(16)
        {
            static const char * const texnames[16] = { "tex0", "tex1", "tex2", "tex3", "tex4", "tex5", "tex6", "tex7", "tex8", "tex9", "tex10", "tex11", "tex12", "tex13", "tex14", "tex15" };
            GLint loc = glCheckError(glGetUniformLocation_(s.program, texnames[i]));
            if(loc != -1){
                glCheckError(glUniform1i_(loc, i));
            }
        }
        if(s.type & SHADER_WORLD) bindworldtexlocs(s);
        loopv(s.defaultparams)
        {
            SlotShaderParamState &param = s.defaultparams[i];
            param.loc = glCheckError(glGetUniformLocation_(s.program, param.name));
        }
        loopv(s.uniformlocs) bindglsluniform(s, s.uniformlocs[i]);
        glCheckError(glUseProgram_(0));
    }
    else if(s.program)
    {
        if(msg) showglslinfo(GL_FALSE, s.program, s.name);
        glCheckError(glDeleteProgram_(s.program));
        s.program = 0;
    }
}

static void findfragdatalocs(Shader &s, char *ps, const char *macroname, int index)
{
    int macrolen = strlen(macroname); 
    bool clear = GLFeatures::ShaderVersion() < 130 && !GLFeatures::HasEGPU4();
    while((ps = strstr(ps, macroname)))
    {
        char *start = ps;
        int loc = strtol(ps + macrolen, (char **)&ps, 0);
        if(loc < 0 || loc > 3) continue;

        ps += strspn(ps, ") \t\r\n");
        const char *type = ps;
        ps += strcspn(ps, "; \t\r\n");
        GLenum format = GL_FLOAT_VEC4;
        switch(type[0])
        {
            case 'v':
                if(matchcubestr(type, ps-type, "vec3")) format = GL_FLOAT_VEC3;
                else if(matchcubestr(type, ps-type, "vec2")) format = GL_FLOAT_VEC2;
                break;
            case 'f':
                if(matchcubestr(type, ps-type, "float")) format = GL_FLOAT;
                break;
            case 'i':
                if(matchcubestr(type, ps-type, "ivec4")) format = GL_INT_VEC4;
                else if(matchcubestr(type, ps-type, "ivec3")) format = GL_INT_VEC3;
                else if(matchcubestr(type, ps-type, "ivec2")) format = GL_INT_VEC2;
                else if(matchcubestr(type, ps-type, "int")) format = GL_INT;
                break;
            case 'u':
                if(matchcubestr(type, ps-type, "uvec4")) format = GL_UNSIGNED_INT_VEC4;
                else if(matchcubestr(type, ps-type, "uvec3")) format = GL_UNSIGNED_INT_VEC3;
                else if(matchcubestr(type, ps-type, "uvec2")) format = GL_UNSIGNED_INT_VEC2;
                else if(matchcubestr(type, ps-type, "uint")) format = GL_UNSIGNED_INT;
                break;
        }

        ps += strspn(ps, " \t\r\n");
        const char *name = ps;
        ps += strcspn(ps, "; \t\r\n");

        if(ps > name)
        {
            char end = *ps;
            *ps = '\0';
            s.fragdatalocs.emplace_back(FragDataLoc(getshaderparamname(name), loc, format, index));
            *ps = end;
        }

        if(clear)
        {
            ps += strspn(ps, "; \t\r\n");
            memset(start, ' ', ps - start);
        }
    }
}

void findfragdatalocs(Shader &s, char *psstr)
{
    if(!psstr || ((GLFeatures::ShaderVersion() >= 330 || (GLFeatures::ShaderVersion() >= 150 && GLFeatures::HasEAL())) && !amd_eal_bug)) return;

    findfragdatalocs(s, psstr, "fragdata(", 0);
    if(maxdualdrawbufs) findfragdatalocs(s, psstr, "fragblend(", 1);
}

int getlocalparam(const char *name)
{
    return localparams.access(name, int(localparams.numelems));
}

static int addlocalparam(Shader &s, const char *name, int loc, int size, GLenum format)
{
    int idx = getlocalparam(name);
    if(idx >= s.localparamremap.size())
    {
        int n = idx + 1 - s.localparamremap.size();
        s.localparamremap.resize(idx + 1);
        memset(s.localparamremap.data(), 0xFF, n);
    }
    s.localparamremap[idx] = s.localparams.size();

    LocalShaderParamState &l = s.localparams.emplace_back();
    l.name = name;
    l.loc = loc;
    l.size = size;
    l.format = format;
    return idx;
}

GlobalShaderParamState *getglobalparam(const char *name)
{
    GlobalShaderParamState *param = globalparams.access(name);
    if(!param)
    {
        param = &globalparams[name];
        param->name = name;
        memset(param->buf, -1, sizeof(param->buf));
        param->version = -1;
    }
    return param;
}

static GlobalShaderParamUse *addglobalparam(Shader &s, GlobalShaderParamState *param, int loc, int size, GLenum format)
{
    GlobalShaderParamUse &g = s.globalparams.emplace_back();
    g.param = param;
    g.version = -2;
    g.loc = loc;
    g.size = size;
    g.format = format;
    return &g;
}

static void setglsluniformformat(Shader &s, const char *name, GLenum format, int size)
{
    switch(format)
    {
        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
        case GL_INT:
        case GL_INT_VEC2:
        case GL_INT_VEC3:
        case GL_INT_VEC4:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_VEC2:
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
        case GL_BOOL:
        case GL_BOOL_VEC2:
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
            break;
        default:
            return;
    }
    if(!strncmp(name, "gl_", 3)) return;

    int loc = glCheckError(glGetUniformLocation_(s.program, name));
    if(loc < 0) return;
    loopvj(s.defaultparams) if(s.defaultparams[j].loc == loc)
    {
        s.defaultparams[j].format = format;
        return;
    }
    loopvj(s.uniformlocs) if(s.uniformlocs[j].loc == loc) return;
    loopvj(s.globalparams) if(s.globalparams[j].loc == loc) return;
    loopvj(s.localparams) if(s.localparams[j].loc == loc) return;

    name = getshaderparamname(name);
    GlobalShaderParamState *param = globalparams.access(name);
    if(param) addglobalparam(s, param, loc, size, format);
    else addlocalparam(s, name, loc, size, format);
}

static void allocglslactiveuniforms(Shader &s)
{
    GLint numactive = 0;
    glCheckError(glGetProgramiv_(s.program, GL_ACTIVE_UNIFORMS, &numactive));
    cubestr name;
    loopi(numactive)
    {
        GLsizei namelen = 0;
        GLint size = 0;
        GLenum format = GL_FLOAT_VEC4;
        name[0] = '\0';
        glCheckError(glGetActiveUniform_(s.program, i, sizeof(name)-1, &namelen, &size, &format, name));
        if(namelen <= 0 || size <= 0) continue;
        name[clamp(int(namelen), 0, (int)sizeof(name)-2)] = '\0';
        char *brak = strchr(name, '[');
        if(brak) *brak = '\0';
        setglsluniformformat(s, name, format, size);
    }
}

void Shader::allocparams(Slot *slot)
{
    allocglslactiveuniforms(*this);
}

int GlobalShaderParamState::nextversion = 0;

void GlobalShaderParamState::resetversions()
{
    enumerate(shaders, Shader, s,
    {
        loopv(s.globalparams)
        {
            GlobalShaderParamUse &u = s.globalparams[i];
            if(u.version != u.param->version) u.version = -2;
        }
    });
    nextversion = 0;
    enumerate(globalparams, GlobalShaderParamState, g, { g.version = ++nextversion; });
    enumerate(shaders, Shader, s,
    {
        loopv(s.globalparams)
        {
            GlobalShaderParamUse &u = s.globalparams[i];
            if(u.version >= 0) u.version = u.param->version;
        }
    });
}

static float *findslotparam(Slot &s, const char *name, float *noval = NULL)
{
    loopv(s.params)
    {
        SlotShaderParam &param = s.params[i];
        if(name == param.name) return param.val;
    }
    loopv(s.shader->defaultparams)
    {
        SlotShaderParamState &param = s.shader->defaultparams[i];
        if(name == param.name) return param.val;
    }
    return noval;
}

static float *findslotparam(VSlot &s, const char *name, float *noval = NULL)
{
    loopv(s.params)
    {
        SlotShaderParam &param = s.params[i];
        if(name == param.name) return param.val;
    }
    return findslotparam(*s.slot, name, noval);
}

static inline void setslotparam(SlotShaderParamState &l, const float *val)
{
    switch(l.format)
    {
        case GL_BOOL:
        case GL_FLOAT:      glCheckError(glUniform1fv_(l.loc, 1, val)); break;
        case GL_BOOL_VEC2:
        case GL_FLOAT_VEC2: glCheckError(glUniform2fv_(l.loc, 1, val)); break;
        case GL_BOOL_VEC3:
        case GL_FLOAT_VEC3: glCheckError(glUniform3fv_(l.loc, 1, val)); break;
        case GL_BOOL_VEC4:
        case GL_FLOAT_VEC4: glCheckError(glUniform4fv_(l.loc, 1, val)); break;
        case GL_INT:      glCheckError(glUniform1i_(l.loc, int(val[0]))); break;
        case GL_INT_VEC2: glCheckError(glUniform2i_(l.loc, int(val[0]), int(val[1]))); break;
        case GL_INT_VEC3: glCheckError(glUniform3i_(l.loc, int(val[0]), int(val[1]), int(val[2]))); break;
        case GL_INT_VEC4: glCheckError(glUniform4i_(l.loc, int(val[0]), int(val[1]), int(val[2]), int(val[3]))); break;
        case GL_UNSIGNED_INT:      glCheckError(glUniform1ui_(l.loc, uint(val[0]))); break;
        case GL_UNSIGNED_INT_VEC2: glCheckError(glUniform2ui_(l.loc, uint(val[0]), uint(val[1]))); break;
        case GL_UNSIGNED_INT_VEC3: glCheckError(glUniform3ui_(l.loc, uint(val[0]), uint(val[1]), uint(val[2]))); break;
        case GL_UNSIGNED_INT_VEC4: glCheckError(glUniform4ui_(l.loc, uint(val[0]), uint(val[1]), uint(val[2]), uint(val[3]))); break;
    }
}

#define SETSLOTPARAM(l, mask, i, val) do { \
    if(!(mask&(1<<i))) { \
        mask |= 1<<i; \
        setslotparam(l, val); \
    } \
} while(0)

#define SETSLOTPARAMS(slotparams) \
    loopv(slotparams) \
    { \
        SlotShaderParam &p = slotparams[i]; \
        if(!in_range(p.loc, defaultparams)) continue; \
        SlotShaderParamState &l = defaultparams[p.loc]; \
        SETSLOTPARAM(l, unimask, p.loc, p.val); \
    }
#define SETDEFAULTPARAMS \
    loopv(defaultparams) \
    { \
        SlotShaderParamState &l = defaultparams[i]; \
        SETSLOTPARAM(l, unimask, i, l.val); \
    }

void Shader::setslotparams(Slot &slot)
{
    uint unimask = 0;
    SETSLOTPARAMS(slot.params)
    SETDEFAULTPARAMS
}

void Shader::setslotparams(Slot &slot, VSlot &vslot)
{
    uint unimask = 0;
    if(vslot.slot == &slot)
    {
        SETSLOTPARAMS(vslot.params)
        SETSLOTPARAMS(slot.params)
        SETDEFAULTPARAMS
    }
    else
    {
        SETSLOTPARAMS(slot.params)
        loopv(defaultparams)
        {
            SlotShaderParamState &l = defaultparams[i];
            SETSLOTPARAM(l, unimask, i, l.flags&SlotShaderParam::REUSE ? findslotparam(vslot, l.name, l.val) : l.val);
        }
    }
}

void Shader::bindprograms()
{
    if(this == lastshader || !loaded()) return;
    glCheckError(glUseProgram_(program));
    lastshader = this;
}

bool Shader::compile()
{
    if(!vsstr) vsobj = !reusevs || reusevs->invalid() ? 0 : reusevs->vsobj;
    else compileglslshader(*this, GL_VERTEX_SHADER,   vsobj, vsstr, name, dbgshader || !variantshader);
    if(!psstr) psobj = !reuseps || reuseps->invalid() ? 0 : reuseps->psobj;
    else compileglslshader(*this, GL_FRAGMENT_SHADER, psobj, psstr, name, dbgshader || !variantshader);
    linkglslprogram(*this, !variantshader);
    return program!=0;
}

void Shader::cleanup(bool full)
{
    used = false;
    if(vsobj) { if(!reusevs){
        glCheckError(glDeleteShader_(vsobj)); vsobj = 0; }
    }
    if(psobj) { if(!reuseps){
        glCheckError(glDeleteShader_(psobj)); psobj = 0; }
    }
    if(program) { glCheckError(glDeleteProgram_(program)); program = 0; }
    localparams.clear();
    localparamremap.clear();
    globalparams.clear();
    if(standard || full)
    {
        type = SHADER_INVALID;
        DELETEA(vsstr);
        DELETEA(psstr);
        DELETEA(defer);
        variants.clear();
        DELETEA(variantrows);
        defaultparams.clear();
        attriblocs.clear();
        fragdatalocs.clear();
        uniformlocs.clear();
        reusevs = reuseps = NULL;
    }
    else loopv(defaultparams) defaultparams[i].loc = -1;
}

static void genattriblocs(Shader &s, const char *vs, const char *ps, Shader *reusevs, Shader *reuseps)
{
    static int len = strlen("//:attrib");
    cubestr name;
    int loc;
    if(reusevs) s.attriblocs = reusevs->attriblocs;
    else while((vs = strstr(vs, "//:attrib")))
    {
        if(sscanf(vs, "//:attrib %100s %d", name, &loc) == 2)
            s.attriblocs.emplace_back(AttribLoc(getshaderparamname(name), loc));
        vs += len;
    }
}

static void genuniformlocs(Shader &s, const char *vs, const char *ps, Shader *reusevs, Shader *reuseps)
{
    static int len = strlen("//:uniform");
    cubestr name, blockname;
    int binding, stride;
    if(reusevs) s.uniformlocs = reusevs->uniformlocs;
    else while((vs = strstr(vs, "//:uniform")))
    {
        int numargs = sscanf(vs, "//:uniform %100s %100s %d %d", name, blockname, &binding, &stride);
        if(numargs >= 3) s.uniformlocs.emplace_back(
                    UniformLoc(getshaderparamname(name), getshaderparamname(blockname), binding,
                               numargs >= 4 ? stride : 0));
        else if(numargs >= 1) s.uniformlocs.emplace_back(UniformLoc(getshaderparamname(name)));
        vs += len;
    }
}

Shader *newshader(int type, const char *name, const char *vs, const char *ps, Shader *variant = NULL, int row = 0)
{
    if(Shader::lastshader)
    {
        glCheckError(glUseProgram_(0));
        Shader::lastshader = NULL;
    }

    Shader *exists = shaders.access(name);
    char *rname = exists ? exists->name : newcubestr(name);
    Shader &s = shaders[rname];
    s.name = rname;
    s.vsstr = newcubestr(vs);
    s.psstr = newcubestr(ps);
    DELETEA(s.defer);
    s.type = type & ~(SHADER_INVALID | SHADER_DEFERRED);
    s.variantshader = variant;
    s.standard = standardshaders;
    if(forceshaders) s.forced = true;
    s.reusevs = s.reuseps = NULL;
    if(variant)
    {
        int row = 0, col = 0;
        if(!vs[0] || sscanf(vs, "%d , %d", &row, &col) >= 1)
        {
            DELETEA(s.vsstr);
            s.reusevs = !vs[0] ? variant : variant->getvariant(col, row);
        }
        row = col = 0;
        if(!ps[0] || sscanf(ps, "%d , %d", &row, &col) >= 1)
        {
            DELETEA(s.psstr);
            s.reuseps = !ps[0] ? variant : variant->getvariant(col, row);
        }
    }
    if(variant) loopv(variant->defaultparams) s.defaultparams.emplace_back(variant->defaultparams[i]);
    else loopv(slotparams) s.defaultparams.emplace_back(slotparams[i]);
    s.attriblocs.clear();
    s.uniformlocs.clear();
    genattriblocs(s, vs, ps, s.reusevs, s.reuseps);
    genuniformlocs(s, vs, ps, s.reusevs, s.reuseps);
    s.fragdatalocs.clear();
    if(s.reuseps) s.fragdatalocs = s.reuseps->fragdatalocs;
    else findfragdatalocs(s, s.psstr);
    if(!s.compile())
    {
        s.cleanup(true);
        if(variant) shaders.remove(rname);
        return NULL;
    }
    if(variant) variant->addvariant(row, &s);
    return &s;
}

static const char *findglslmain(const char *s)
{
    const char *main = strstr(s, "main");
    if(!main) return NULL;
    for(; main >= s; main--) switch(*main) { case '\r': case '\n': case ';': return main + 1; }
    return s;
}

static void gengenericvariant(Shader &s, const char *sname, const char *vs, const char *ps, int row = 0)
{
    int rowoffset = 0;
    bool vschanged = false, pschanged = false;
    vector<char> vsv, psv;

    put(vs, strlen(vs)+1, vsv);
    put(ps, strlen(ps)+1, psv);

    static const int len = strlen("//:variant"), olen = strlen("override");
    for(char *vspragma = vsv.data();; vschanged = true)
    {
        vspragma = strstr(vspragma, "//:variant");
        if(!vspragma) break;
        if(sscanf(vspragma + len, "row %d", &rowoffset) == 1) continue;
        memset(vspragma, ' ', len);
        vspragma += len;
        if(!strncmp(vspragma, "override", olen))
        {
            memset(vspragma, ' ', olen);
            vspragma += olen;
            char *end = vspragma + strcspn(vspragma, "\n\r");
            end += strspn(end, "\n\r");
            int endlen = strcspn(end, "\n\r");
            memset(end, ' ', endlen);
        }
    }
    for(char *pspragma = psv.data();; pschanged = true)
    {
        pspragma = strstr(pspragma, "//:variant");
        if(!pspragma) break;
        if(sscanf(pspragma + len, "row %d", &rowoffset) == 1) continue;
        memset(pspragma, ' ', len);
        pspragma += len;
        if(!strncmp(pspragma, "override", olen))
        {
            memset(pspragma, ' ', olen);
            pspragma += olen;
            char *end = pspragma + strcspn(pspragma, "\n\r");
            end += strspn(end, "\n\r");
            int endlen = strcspn(end, "\n\r");
            memset(end, ' ', endlen);
        }
    }
    row += rowoffset;
    if(row < 0 || row >= MAXVARIANTROWS) return;
    int col = s.numvariants(row);
    defformatcubestr(varname, "<variant:%d,%d>%s", col, row, sname);
    cubestr reuse;
    if(col) formatcubestr(reuse, "%d", row);
    else copycubestr(reuse, "");
    newshader(s.type, varname, vschanged ? vsv.data() : reuse, pschanged ? psv.data() : reuse, &s, row);
}

static void genfogshader(vector<char> &vsbuf, vector<char> &psbuf, const char *vs, const char *ps)
{
    const char *vspragma = strstr(vs, "//:fog"), *pspragma = strstr(ps, "//:fog");
    if(!vspragma && !pspragma) return;
    static const int pragmalen = strlen("//:fog");
    const char *vsmain = findglslmain(vs), *vsend = strrchr(vs, '}');
    if(vsmain && vsend)
    {
        if(!strstr(vs, "lineardepth"))
        {
            put(vs, vsmain - vs, vsbuf);
            const char *fogparams = "\nuniform vec2 lineardepthscale;\nvarying float lineardepth;\n";
            put(fogparams, strlen(fogparams), vsbuf);
            put(vsmain, vsend - vsmain, vsbuf);
            const char *vsfog = "\nlineardepth = dot(lineardepthscale, gl_Position.zw);\n";
            put(vsfog, strlen(vsfog), vsbuf);
            put(vsend, strlen(vsend)+1, vsbuf);
        }
    }
    const char *psmain = findglslmain(ps), *psend = strrchr(ps, '}');
    if(psmain && psend)
    {
        put(ps, psmain - ps, psbuf);
        if(!strstr(ps, "lineardepth"))
        {
            const char *foginterp = "\nvarying float lineardepth;\n";
            put(foginterp, strlen(foginterp), psbuf);
        }
        const char *fogparams =
            "\nuniform vec3 fogcolor;\n"
            "uniform vec2 fogdensity;\n"
            "uniform vec4 radialfogscale;\n"
            "#define fogcoord lineardepth*length(vec3(gl_FragCoord.xy*radialfogscale.xy + radialfogscale.zw, 1.0))\n";
        put(fogparams, strlen(fogparams), psbuf);
        put(psmain, psend - psmain, psbuf);
        const char *psdef = "\n#define FOG_COLOR ";
        const char *psfog =
            pspragma && !strncmp(pspragma+pragmalen, "rgba", 4) ?
                "\nfragcolor = mix((FOG_COLOR), fragcolor, clamp(exp2(fogcoord*-fogdensity.x)*fogdensity.y, 0.0, 1.0));\n" :
                "\nfragcolor.rgb = mix((FOG_COLOR).rgb, fragcolor.rgb, clamp(exp2(fogcoord*-fogdensity.x)*fogdensity.y, 0.0, 1.0));\n";
        int clen = 0;
        if(pspragma)
        {
            pspragma += pragmalen;
            while(iscubealpha(*pspragma)) pspragma++;
            while(*pspragma && !iscubespace(*pspragma)) pspragma++;
            pspragma += strspn(pspragma, " \t\v\f");
            clen = strcspn(pspragma, "\r\n");
        }
        if(clen <= 0) { pspragma = "fogcolor"; clen = strlen(pspragma); }
        put(psdef, strlen(psdef), psbuf);
        put(pspragma, clen, psbuf);
        put(psfog, strlen(psfog), psbuf);
        put(psend, strlen(psend)+1, psbuf);
    }
}

static void genuniformdefs(vector<char> &vsbuf, vector<char> &psbuf, const char *vs, const char *ps, Shader *variant = NULL)
{
    if(variant ? variant->defaultparams.empty() : slotparams.empty()) return;
    const char *vsmain = findglslmain(vs), *psmain = findglslmain(ps);
    if(!vsmain || !psmain) return;
    put(vs, vsmain - vs, vsbuf);
    put(ps, psmain - ps, psbuf);
    if(variant) loopv(variant->defaultparams)
    {
        defformatcubestr(uni, "\nuniform vec4 %s;\n", variant->defaultparams[i].name);
        put(uni, strlen(uni), vsbuf);
        put(uni, strlen(uni), psbuf);
    }
    else loopv(slotparams)
    {
        defformatcubestr(uni, "\nuniform vec4 %s;\n", slotparams[i].name);
        put(uni, strlen(uni), vsbuf);
        put(uni, strlen(uni), psbuf);
    }
    put(vsmain, strlen(vsmain)+1, vsbuf);
    put(psmain, strlen(psmain)+1, psbuf);
}

void setupshaders()
{
    GLint val;
    glCheckError(glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &val));
    maxvsuniforms = val/4;
    glCheckError(glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &val));
    maxfsuniforms = val/4;
    if(GLFeatures::HasGPU4())
    {
        glCheckError(glGetIntegerv(GL_MIN_PROGRAM_TEXEL_OFFSET, &val));
        mintexoffset = val;
        glCheckError(glGetIntegerv(GL_MAX_PROGRAM_TEXEL_OFFSET, &val));
        maxtexoffset = val;
    }
    else mintexoffset = maxtexoffset = 0;
    if(GLFeatures::ShaderVersion() >= 140 || GLFeatures::HasEGPU4())
    {
        mintexrectoffset = mintexoffset;
        maxtexrectoffset = maxtexoffset;
    }
    else mintexrectoffset = maxtexrectoffset = 0;

    standardshaders = true;
    nullshader = newshader(0, "<init>null",
        "attribute vec4 vvertex;\n"
        "void main(void) {\n"
        "   gl_Position = vvertex;\n"
        "}\n",
        "fragdata(0) vec4 fragcolor;\n"
        "void main(void) {\n"
        "   fragcolor = vec4(1.0, 0.0, 1.0, 1.0);\n"
        "}\n");
    hudshader = newshader(0, "<init>hud",
        "attribute vec4 vvertex, vcolor;\n"
        "attribute vec2 vtexcoord0;\n"
        "uniform mat4 hudmatrix;\n"
        "varying vec2 texcoord0;\n"
        "varying vec4 colorscale;\n"
        "void main(void) {\n"
        "    gl_Position = hudmatrix * vvertex;\n"
        "    texcoord0 = vtexcoord0;\n"
        "    colorscale = vcolor;\n"
        "}\n",
        "uniform sampler2D tex0;\n"
        "varying vec2 texcoord0;\n"
        "varying vec4 colorscale;\n"
        "fragdata(0) vec4 fragcolor;\n"
        "void main(void) {\n"
        "    vec4 color = texture2D(tex0, texcoord0);\n"
        "    fragcolor = colorscale * color;\n"
        "}\n");
    hudtextshader = newshader(0, "<init>hudtext",
        "attribute vec4 vvertex, vcolor;\n"
        "attribute vec2 vtexcoord0;\n"
        "uniform mat4 hudmatrix;\n"
        "varying vec2 texcoord0;\n"
        "varying vec4 colorscale;\n"
        "void main(void) {\n"
        "    gl_Position = hudmatrix * vvertex;\n"
        "    texcoord0 = vtexcoord0;\n"
        "    colorscale = vcolor;\n"
        "}\n",
        "uniform sampler2D tex0;\n"
        "uniform vec4 textparams;\n"
        "varying vec2 texcoord0;\n"
        "varying vec4 colorscale;\n"
        "fragdata(0) vec4 fragcolor;\n"
        "void main(void) {\n"
        "    float dist = texture2D(tex0, texcoord0).r;\n"
        "    float border = smoothstep(textparams.x, textparams.y, dist);\n"
        "    float outline = smoothstep(textparams.z, textparams.w, dist);\n"
        "    fragcolor = vec4(colorscale.rgb * outline, colorscale.a * border);\n"
        "}\n");
    hudnotextureshader = newshader(0, "<init>hudnotexture",
        "attribute vec4 vvertex, vcolor;\n"
        "uniform mat4 hudmatrix;"
        "varying vec4 color;\n"
        "void main(void) {\n"
        "    gl_Position = hudmatrix * vvertex;\n"
        "    color = vcolor;\n"
        "}\n",
        "varying vec4 color;\n"
        "fragdata(0) vec4 fragcolor;\n"
        "void main(void) {\n"
        "    fragcolor = color;\n"
        "}\n");
    standardshaders = false;

    if(!nullshader || !hudshader || !hudtextshader || !hudnotextureshader) fatal("failed to setup shaders");

    dummyslot.shader = nullshader;
}

VAR(defershaders, 0, 1, 1);

SCRIPTEXPORT void defershader(int *type, const char *name, const char *contents)
{
    Shader *exists = shaders.access(name);
    if(exists && !exists->invalid()) return;
    if(!defershaders) { execute(contents); return; }
    char *rname = exists ? exists->name : newcubestr(name);
    Shader &s = shaders[rname];
    s.name = rname;
    DELETEA(s.defer);
    s.defer = newcubestr(contents);
    s.type = SHADER_DEFERRED | (*type & ~SHADER_INVALID);
    s.standard = standardshaders;
}

void Shader::force()
{
    if(!deferred() || !defer) return;

    char *cmd = defer;
    defer = NULL;
    bool wasstandard = standardshaders, wasforcing = forceshaders;
    int oldflags = identflags;
    standardshaders = standard;
    forceshaders = false;
    identflags &= ~IDF_PERSIST;
    slotparams.clear();
    execute(cmd);
    identflags = oldflags;
    forceshaders = wasforcing;
    standardshaders = wasstandard;
    delete[] cmd;

    if(deferred())
    {
        DELETEA(defer);
        type = SHADER_INVALID;
    }
}

int Shader::uniformlocversion()
{
    static int version = 0;
    if(++version >= 0) return version;
    version = 0;
    enumerate(shaders, Shader, s, { loopvj(s.uniformlocs) s.uniformlocs[j].version = -1; });
    return version;
}

Shader *useshaderbyname(const char *name)
{
    Shader *s = shaders.access(name);
    if(!s) return NULL;
    if(s->deferred()) s->force();
    s->forced = true;
    return s;
}
SCRIPTEXPORT void forceshader(const char *name)
{
    useshaderbyname(name);
}

SCRIPTEXPORT void shader(int *type, char *name, char *vs, char *ps)
{
    if(lookupshaderbyname(name)) return;
    conoutf("Loading shader %s", name);

    defformatcubestr(info, "shader %s", name);
    renderprogress(loadprogress, info);
    vector<char> vsbuf, psbuf, vsbak, psbak;
#define GENSHADER(cond, body) \
    if(cond) \
    { \
        if(vsbuf.size()) { vsbak.clear(); put(vs, strlen(vs)+1, vsbak); vs = vsbak.data(); vsbuf.clear(); } \
        if(psbuf.size()) { psbak.clear(); put(ps, strlen(ps)+1, psbak); ps = psbak.data(); psbuf.clear(); } \
        body; \
        if(vsbuf.size()) vs = vsbuf.data(); \
        if(psbuf.size()) ps = psbuf.data(); \
    }
    GENSHADER(slotparams.size(), genuniformdefs(vsbuf, psbuf, vs, ps));
    GENSHADER(strstr(vs, "//:fog") || strstr(ps, "//:fog"), genfogshader(vsbuf, psbuf, vs, ps));
    Shader *s = newshader(*type, name, vs, ps);
    if(s)
    {
        if(strstr(ps, "//:variant") || strstr(vs, "//:variant")) gengenericvariant(*s, name, vs, ps);
    }
    slotparams.clear();
}

SCRIPTEXPORT void variantshader(int *type, char *name, int *row, char *vs, char *ps, int *maxvariants)
{
    if(*row < 0)
    {
        shader(type, name, vs, ps);
        return;
    }
    else if(*row >= MAXVARIANTROWS) return;

    Shader *s = lookupshaderbyname(name);
    if(!s) return;

    defformatcubestr(varname, "<variant:%d,%d>%s", s->numvariants(*row), *row, name);
    if(*maxvariants > 0)
    {
        defformatcubestr(info, "shader %s", name);
        renderprogress(min(s->variants.size() / float(*maxvariants), 1.0f), info);
    }
    vector<char> vsbuf, psbuf, vsbak, psbak;
    GENSHADER(s->defaultparams.size(), genuniformdefs(vsbuf, psbuf, vs, ps, s));
    GENSHADER(strstr(vs, "//:fog") || strstr(ps, "//:fog"), genfogshader(vsbuf, psbuf, vs, ps));
    Shader *v = newshader(*type, varname, vs, ps, s, *row);
    if(v)
    {
        if(strstr(ps, "//:variant") || strstr(vs, "//:variant")) gengenericvariant(*s, varname, vs, ps, *row);
    }
}

SCRIPTEXPORT void setshader(char *name)
{
    slotparams.clear();
    Shader *s = shaders.access(name);
    if(!s)
    {
        conoutf(CON_ERROR, "no such shader: %s", name);
    }
    else slotshader = s;
}

void resetslotshader()
{
    slotshader = NULL;
    slotparams.clear();
}

void setslotshader(Slot &s)
{
    s.shader = slotshader;
    if(!s.shader)
    {
        s.shader = stdworldshader;
        return;
    }
    loopv(slotparams) s.params.emplace_back(slotparams[i]);
}

static void linkslotshaderparams(vector<SlotShaderParam> &params, Shader *sh, bool load)
{
    if(sh->loaded())
        loopv(params)
    {
        int loc = -1;
        SlotShaderParam &param = params[i];
        loopv(sh->defaultparams)
        {
            SlotShaderParamState &dparam = sh->defaultparams[i];
            if(dparam.name==param.name)
            {
                if(memcmp(param.val, dparam.val, sizeof(param.val))) loc = i;
                break;
            }
        }
        param.loc = loc;
    }
    else if(load) loopv(params) params[i].loc = -1;
}

void linkslotshader(Slot &s, bool load)
{
    if(!s.shader) return;

    if(load && s.shader->deferred()) s.shader->force();

    linkslotshaderparams(s.params, s.shader, load);
}

void linkvslotshader(VSlot &s, bool load)
{
    if(!s.slot->shader) return;

    linkslotshaderparams(s.params, s.slot->shader, load);

    if(!s.slot->shader->loaded()) return;

    if(s.slot->texmask&(1<<TEX_GLOW))
    {
        static const char *paramname = getshaderparamname("glowcolor");
        const float *param = findslotparam(s, paramname);
        if(param) s.glowcolor = vec(param).clamp(0, 1);
    }
}

bool shouldreuseparams(Slot &s, VSlot &p)
{
    if(!s.shader) return false;
    
    Shader &sh = *s.shader;
    loopv(sh.defaultparams)
    {
        SlotShaderParamState &param = sh.defaultparams[i];
        if(param.flags & SlotShaderParam::REUSE)
        {
            const float *val = findslotparam(p, param.name);
            if(val && memcmp(param.val, val, sizeof(param.val)))
            {
                loopvj(s.params) if(s.params[j].name == param.name) goto notreused; 
                return true;
            notreused:;
            }
        }
    }
    return false;
}

SCRIPTEXPORT void dumpshader(const char *name, int *col, int *row)
{
    Shader *s = lookupshaderbyname(name);
    FILE *l = getlogfile();
    if(!s || !l) return;
    if(*col >= 0)
    {
        s = s->getvariant(*col, *row);
        if(!s) return;
    }
    if(s->vsstr) fprintf(l, "%s:%s\n%s\n", s->name, "VS", s->vsstr);
    if(s->psstr) fprintf(l, "%s:%s\n%s\n", s->name, "FS", s->psstr);
}

SCRIPTEXPORT void isshaderdefined(char *name)
{
    intret(lookupshaderbyname(name) ? 1 : 0);
}

static hashset<const char *> shaderparamnames(256);

const char *getshaderparamname(const char *name, bool insert)
{
    const char *exists = shaderparamnames.find(name, NULL);
    if(exists || !insert) return exists;
    return shaderparamnames.add(newcubestr(name));
}

void addslotparam(const char *name, float x, float y, float z, float w, int flags = 0)
{
    if(name) name = getshaderparamname(name);
    loopv(slotparams)
    {
        SlotShaderParam &param = slotparams[i];
        if(param.name==name)
        {
            param.val[0] = x;
            param.val[1] = y;
            param.val[2] = z;
            param.val[3] = w;
            param.flags |= flags;
            return;
        }
    }
    SlotShaderParam param = {name, -1, flags, {x, y, z, w}};
    slotparams.emplace_back(param);
}

SCRIPTEXPORT void setuniformparam(char *name, float *x, CommandTypes::OptionalFloat y, CommandTypes::OptionalFloat z, float *w)
{
    addslotparam(name, *x, *y, *z, *w);
}

SCRIPTEXPORT void setshaderparam(char *name, float *x, CommandTypes::OptionalFloat y, CommandTypes::OptionalFloat z, float *w)
{
    addslotparam(name, *x, *y, *z, *w);
}

SCRIPTEXPORT void defuniformparam(char *name, float *x, CommandTypes::OptionalFloat y, CommandTypes::OptionalFloat z, float *w)
{
    addslotparam(name, *x, *y, *z, *w);
}

SCRIPTEXPORT void reuseuniformparam(char *name, float *x, CommandTypes::OptionalFloat y, CommandTypes::OptionalFloat z, float *w)
{
    addslotparam(name, *x, *y, *z, *w, SlotShaderParam::REUSE);
}


#define NUMPOSTFXBINDS 10

struct postfxtex
{
    GLuint id;
    int scale, used;

    postfxtex() : id(0), scale(0), used(-1) {}
};
vector<postfxtex> postfxtexs;
int postfxbinds[NUMPOSTFXBINDS];
GLuint postfxfb = 0;
int postfxw = 0, postfxh = 0;

struct postfxpass
{
    Shader *shader;
    vec4 params;
    uint inputs, freeinputs;
    int outputbind, outputscale;

    postfxpass() : shader(NULL), inputs(1), freeinputs(1), outputbind(0), outputscale(0) {}
};
vector<postfxpass> postfxpasses;

static int allocatepostfxtex(int scale)
{
    loopv(postfxtexs)
    {
        postfxtex &t = postfxtexs[i];
        if(t.scale==scale && t.used < 0) return i;
    }
    postfxtex &t = postfxtexs.emplace_back();
    t.scale = scale;
    glCheckError(glGenTextures(1, &t.id));
    createtexture(t.id, max(postfxw>>scale, 1), max(postfxh>>scale, 1), NULL, 3, 1, GL_RGB, GL_TEXTURE_RECTANGLE);
    return postfxtexs.size() - 1;
}

void cleanuppostfx(bool fullclean)
{
    if(fullclean && postfxfb)
    {
        glCheckError(glDeleteFramebuffers_(1, &postfxfb));
        postfxfb = 0;
    }

    loopv(postfxtexs){
        glCheckError(glDeleteTextures(1, &postfxtexs[i].id));
    }
    postfxtexs.clear();

    postfxw = 0;
    postfxh = 0;
}

GLuint setuppostfx(int w, int h, GLuint outfbo)
{
    if(postfxpasses.empty()) return outfbo;

    if(postfxw != w || postfxh != h)
    {
        cleanuppostfx(false);
        postfxw = w;
        postfxh = h;
    }

    loopi(NUMPOSTFXBINDS) postfxbinds[i] = -1;
    loopv(postfxtexs) postfxtexs[i].used = -1;

    if(!postfxfb){
        glCheckError(glGenFramebuffers_(1, &postfxfb));
    }
    glCheckError(glBindFramebuffer_(GL_FRAMEBUFFER, postfxfb));
    int tex = allocatepostfxtex(0);
    glCheckError(glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, postfxtexs[tex].id, 0));
    bindgdepth();

    postfxbinds[0] = tex;
    postfxtexs[tex].used = 0;

    return postfxfb;
}

void renderpostfx(GLuint outfbo)
{
    if(postfxpasses.empty()) return;

    timer *postfxtimer = begintimer("postfx");
    loopv(postfxpasses)
    {
        postfxpass &p = postfxpasses[i];

        int tex = -1;
        if(!in_range(i+1, postfxpasses))
        {
            glCheckError(glBindFramebuffer_(GL_FRAMEBUFFER, outfbo));
        }
        else
        {
            tex = allocatepostfxtex(p.outputscale);
            glCheckError(glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, postfxtexs[tex].id, 0));
        }

        int w = tex >= 0 ? max(postfxw>>postfxtexs[tex].scale, 1) : postfxw,
            h = tex >= 0 ? max(postfxh>>postfxtexs[tex].scale, 1) : postfxh;
        glCheckError(glViewport(0, 0, w, h));
        p.shader->set();
        LOCALPARAM(params, p.params);
        int tw = w, th = h, tmu = 0;
        loopj(NUMPOSTFXBINDS) if(p.inputs&(1<<j) && postfxbinds[j] >= 0)
        {
            if(!tmu)
            {
                tw = max(postfxw>>postfxtexs[postfxbinds[j]].scale, 1);
                th = max(postfxh>>postfxtexs[postfxbinds[j]].scale, 1);
            }
            else
            {
                glCheckError(glActiveTexture_(GL_TEXTURE0 + tmu));
            }
            glCheckError(glBindTexture(GL_TEXTURE_RECTANGLE, postfxtexs[postfxbinds[j]].id));
            ++tmu;
        }
        if(tmu)
        {
            glCheckError(glActiveTexture_(GL_TEXTURE0));
        }
        screenquad(tw, th);

        loopj(NUMPOSTFXBINDS) if(p.freeinputs&(1<<j) && postfxbinds[j] >= 0)
        {
            postfxtexs[postfxbinds[j]].used = -1;
            postfxbinds[j] = -1;
        }
        if(tex >= 0)
        {
            if(postfxbinds[p.outputbind] >= 0) postfxtexs[postfxbinds[p.outputbind]].used = -1;
            postfxbinds[p.outputbind] = tex;
            postfxtexs[tex].used = p.outputbind;
        }
    }
    endtimer(postfxtimer);
}

static bool addpostfx(const char *name, int outputbind, int outputscale, uint inputs, uint freeinputs, const vec4 &params)
{
    if(!*name) return false;
    Shader *s = useshaderbyname(name);
    if(!s)
    {
        conoutf(CON_ERROR, "no such postfx shader: %s", name);
        return false;
    }
    postfxpass &p = postfxpasses.emplace_back();
    p.shader = s;
    p.outputbind = outputbind;
    p.outputscale = outputscale;
    p.inputs = inputs;
    p.freeinputs = freeinputs;
    p.params = params;
    return true;
}

SCRIPTEXPORT void clearpostfx()
{
    postfxpasses.clear();
    cleanuppostfx(false);
}

SCRIPTEXPORT_AS(addpostfx) void addpostfx_scriptimpl(char *name, int *bind, int *scale, char *inputs, float *x, float *y, float *z, float *w)
{
    int inputmask = inputs[0] ? 0 : 1;
    int freemask = inputs[0] ? 0 : 1;
    bool freeinputs = true;
    for(; *inputs; inputs++)
    {
        if(isdigit(*inputs))
        {
            inputmask |= 1<<(*inputs-'0');
            if(freeinputs) freemask |= 1<<(*inputs-'0');
        }
        else if(*inputs=='+') freeinputs = false;
        else if(*inputs=='-') freeinputs = true;
    }
    inputmask &= (1<<NUMPOSTFXBINDS)-1;
    freemask &= (1<<NUMPOSTFXBINDS)-1;
    addpostfx(name, clamp(*bind, 0, NUMPOSTFXBINDS-1), max(*scale, 0), inputmask, freemask, vec4(*x, *y, *z, *w));
}

SCRIPTEXPORT void setpostfx(char *name, float *x, float *y, float *z, float *w)
{
    clearpostfx();
    if(name[0]) addpostfx(name, 0, 0, 1, 1, vec4(*x, *y, *z, *w));
}

void cleanupshaders()
{
    cleanuppostfx(true);

    loadedshaders = false;
    nullshader = hudshader = hudnotextureshader = NULL;
    enumerate(shaders, Shader, s, s.cleanup());
    Shader::lastshader = NULL;
    glCheckError(glUseProgram_(0));
}

void reloadshaders()
{
    identflags &= ~IDF_PERSIST;
    loadshaders();
    identflags |= IDF_PERSIST;

    linkslotshaders();
    enumerate(shaders, Shader, s,
    {
        if(!s.standard && s.loaded() && !s.variantshader)
        {
            defformatcubestr(info, "shader %s", s.name);
            renderprogress(0.0, info);
            if(!s.compile()) s.cleanup(true);
            loopv(s.variants)
            {
                Shader *v = s.variants[i];
                if((v->reusevs && v->reusevs->invalid()) ||
                   (v->reuseps && v->reuseps->invalid()) ||
                   !v->compile())
                    v->cleanup(true);
            }
        }
        if(s.forced && s.deferred()) s.force();
    });
}

SCRIPTEXPORT void resetshaders()
{
    clearchanges(CHANGE_SHADERS);

    cleanuplights();
    cleanupmodels();
    cleanupshaders();
    setupshaders();
    initgbuffer();
    reloadshaders();
    allchanged(true);
    GLERROR;
}

FVAR(blursigma, 0.005f, 0.5f, 2.0f);

void setupblurkernel(int radius, float *weights, float *offsets)
{
    if(radius<1 || radius>MAXBLURRADIUS) return;
    float sigma = blursigma*2*radius, total = 1.0f/sigma;
    weights[0] = total;
    offsets[0] = 0;
    // rely on bilinear filtering to sample 2 pixels at once
    // transforms a*X + b*Y into (u+v)*[X*u/(u+v) + Y*(1 - u/(u+v))]
    loopi(radius)
    {
        float weight1 = exp(-((2*i)*(2*i)) / (2*sigma*sigma)) / sigma,
              weight2 = exp(-((2*i+1)*(2*i+1)) / (2*sigma*sigma)) / sigma,
              scale = weight1 + weight2,
              offset = 2*i+1 + weight2 / scale;
        weights[i+1] = scale;
        offsets[i+1] = offset;
        total += 2*scale;
    }
    loopi(radius+1) weights[i] /= total;
    for(int i = radius+1; i <= MAXBLURRADIUS; i++) weights[i] = offsets[i] = 0;
}

void setblurshader(int pass, int size, int radius, float *weights, float *offsets, GLenum target)
{
    if(radius<1 || radius>MAXBLURRADIUS) return;
    static Shader *blurshader[7][2] = { { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL } },
                  *blurrectshader[7][2] = { { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL } };
    Shader *&s = (target == GL_TEXTURE_RECTANGLE ? blurrectshader : blurshader)[radius-1][pass];
    if(!s)
    {
        defformatcubestr(name, "blur%c%d%s", 'x'+pass, radius, target == GL_TEXTURE_RECTANGLE ? "rect" : "");
        s = lookupshaderbyname(name);
    }
    s->set();
    LOCALPARAMV(weights, weights, 8);
    float scaledoffsets[8];
    loopk(8) scaledoffsets[k] = offsets[k]/size;
    LOCALPARAMV(offsets, scaledoffsets, 8);
}


// >>>>>>>>>> SCRIPTBIND >>>>>>>>>>>>>> //
#if 0
#include "/Users/micha/dev/ScMaMike/src/build/binding/..+engine+shader.binding.cpp"
#endif
// <<<<<<<<<< SCRIPTBIND <<<<<<<<<<<<<< //
