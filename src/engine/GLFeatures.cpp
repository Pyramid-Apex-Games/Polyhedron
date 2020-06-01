#include "shared/command.h"
#include "engine/GLFeatures.h"
#include "engine/includegl.h"
#include "engine/texture.h"
#include "engine/main/Application.h"

VAR(glcompat, 1, 0, 0);

VAR(amd_pf_bug, 0, 0, 1);
VAR(amd_eal_bug, 0, 0, 1);
VAR(mesa_texrectoffset_bug, 0, 0, 1);
VAR(intel_texalpha_bug, 0, 0, 1);
VAR(intel_mapbufferrange_bug, 0, 0, 1);
VAR(mesa_swap_bug, 0, 0, 1);

VAR(useubo, 1, 0, 0);
VAR(usetexgather, 1, 0, 0);
VAR(usetexcompress, 1, 0, 0);
VAR(maxdrawbufs, 1, 0, 0);
VAR(maxdualdrawbufs, 1, 0, 0);

extern int gdepthstencil, gstencil, glineardepth, msaadepthstencil, msaalineardepth, batchsunlight, smgather, rhrect, tqaaresolvegather;
extern int usetexgather, tqaaresolvegather;

bool checkseries(const char *s, const char *name, int low, int high)
{
    if(name) s = strstr(s, name);
    if(!s) return false;
    while(*s && !isdigit(*s)) ++s;
    if(!*s) return false;
    int n = 0;
    while(isdigit(*s)) n = n*10 + (*s++ - '0');
    return n >= low && n <= high;
}

bool checkmesaversion(const char *s, int major, int minor, int patch)
{
    const char *v = strstr(s, "Mesa");
    if(!v) return false;
    int vmajor = 0, vminor = 0, vpatch = 0;
    if(sscanf(v, "Mesa %d.%d.%d", &vmajor, &vminor, &vpatch) < 1) return false;
    if(vmajor > major) return true; else if(vmajor < major) return false;
    if(vminor > minor) return true; else if(vminor < minor) return false;
    return vpatch >= patch;
}

bool checkdepthtexstencilrb()
{
    int w = 256, h = 256;
    GLuint fbo = 0;
    glCheckError(glGenFramebuffers_(1, &fbo));
    glCheckError(glBindFramebuffer_(GL_FRAMEBUFFER, fbo));

    GLuint depthtex = 0;
    glCheckError(glGenTextures(1, &depthtex));
    createtexture(depthtex, w, h, NULL, 3, 0, GL_DEPTH_COMPONENT24, GL_TEXTURE_RECTANGLE);
    glCheckError(glBindTexture(GL_TEXTURE_RECTANGLE, 0));
    glCheckError(glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, depthtex, 0));

    GLuint stencilrb = 0;
    glCheckError(glGenRenderbuffers_(1, &stencilrb));
    glCheckError(glBindRenderbuffer_(GL_RENDERBUFFER, stencilrb));
    glCheckError(glRenderbufferStorage_(GL_RENDERBUFFER, GL_STENCIL_INDEX8, w, h));
    glCheckError(glBindRenderbuffer_(GL_RENDERBUFFER, 0));
    glCheckError(glFramebufferRenderbuffer_(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilrb));

    bool supported = glCheckFramebufferStatus_(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    glCheckError(glBindFramebuffer_(GL_FRAMEBUFFER, 0));
    glCheckError(glDeleteFramebuffers_(1, &fbo));
    glCheckError(glDeleteTextures(1, &depthtex));
    glCheckError(glDeleteRenderbuffers_(1, &stencilrb));

    return supported;
}

const std::string &GLFeatures::VersionString() {
    static std::string version = (const char *) glCheckError(glGetString(GL_VERSION));
    return version;
}

const std::string &GLFeatures::ShaderVersionString() {
    static std::string shaderversion = (const char *) glCheckError(glGetString(GL_SHADING_LANGUAGE_VERSION));
    return shaderversion;
}

const std::string &GLFeatures::VendorString() {
    static std::string vendor = (const char *) glCheckError(glGetString(GL_VENDOR));
    return vendor;
}

const std::string &GLFeatures::RendererString() {
    static std::string renderer = (const char *) glCheckError(glGetString(GL_RENDERER));
    return renderer;
}

int GLFeatures::Version()
{
    return GLVersion.major*100 + GLVersion.minor * 10;
}

int GLFeatures::ShaderVersion()
{
    static int major = 0, minor = 0;
    if (major == 0 && minor == 0)
    {
        if (std::sscanf(ShaderVersionString().c_str(), "%d.%d", &major, &minor) != 2)
        {
            major = 0;
            minor = 1;
        }
    }

    return major*100 + minor;
}

bool GLFeatures::hasVendor(GLFeatures::Vendor v) {

    static bool mesa = RendererString().find("Mesa") != std::string::npos ||
                       VendorString().find("Mesa") != std::string::npos;
    static bool intel = RendererString().find("Intel") != std::string::npos;
    static bool nvidia = RendererString().find("NVIDIA") != std::string::npos;
    static bool amd = RendererString().find("ATI") != std::string::npos ||
                      RendererString().find("Advanced Micro Devices") != std::string::npos;

    switch (v) {
        case GLFeatures::Vendor::Mesa:
            return mesa;
        case GLFeatures::Vendor::Intel:
            return intel;
        case GLFeatures::Vendor::Nvidia:
            return nvidia;
        case GLFeatures::Vendor::Amd:
            return amd;
    }

    return false;
}

bool GLFeatures::HasVAO() {
    //"GL_ARB_vertex_array_object"
    //"GL_APPLE_vertex_array_object"
    static bool __hasVAO = __util::HasGlVersionGrEqThan<300>()
#ifdef GL_OES_vertex_array_object
                           || GLAD_GL_OES_vertex_array_object
#endif
                           ;
    return __hasVAO;
}

bool GLFeatures::HasPBO() {
    //"GL_ARB_pixel_buffer_object"
    //"GL_EXT_pixel_buffer_object"
    static bool __hasPBO = __util::HasGlVersionGrEqThan<210>() ||
#ifdef GL_NV_pixel_buffer_object
                           GLAD_GL_NV_pixel_buffer_object ||
#endif
                           false ||
                           false;
    return __hasPBO;
}

bool GLFeatures::HasTFW() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasAFBO() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasDS() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasFBB() {
    static bool __hasFBB = __util::HasGlVersionGrEqThan<300>()
#ifdef GL_NV_framebuffer_blit
                           || GLAD_GL_NV_framebuffer_blit
#endif
#ifdef GL_ANGLE_framebuffer_blit
                           || GLAD_GL_ANGLE_framebuffer_blit
#endif
                                   ;
    return __hasFBB;
}

bool GLFeatures::HasFBMS() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasTF() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasRGTC() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasPF() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasHFV() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasHFP() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasS3TC() {
    static bool __hasS3TC = GLAD_GL_EXT_texture_compression_s3tc;
    return __hasS3TC;
}

bool GLFeatures::HasLATC() {
    return false;
}

bool GLFeatures::HasGPU4() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasEGPU4() {
    //"GL_EXT_gpu_shader4"
    return GLFeatures::HasGPU4() && false;
}

bool GLFeatures::HasGPU5() {
    static bool __hasGPU5 = __util::HasGlVersionGrEqThan<400>()
#ifdef GL_NV_gpu_shader5
                            || GLAD_GL_NV_gpu_shader5
#endif
#ifdef GL_EXT_gpu_shader5
                            || GLAD_GL_EXT_gpu_shader5
#endif
#ifdef GL_OES_gpu_shader5
                            || GLAD_GL_OES_gpu_shader5
#endif
                            ;
    return __hasGPU5;
}

bool GLFeatures::HasAF() {
    static bool __hasAF = GLAD_GL_EXT_texture_filter_anisotropic;
    return __hasAF;
}

bool GLFeatures::HasTMS() {
    return __util::HasGlVersionGrEqThan<320>();
}

bool GLFeatures::HasFBMSBS() {
    return false;
}

bool GLFeatures::HasMSS() {
    return __util::HasGlVersionGrEqThan<400>();
}

bool GLFeatures::HasDBB() {
    return __util::HasGlVersionGrEqThan<400>();
}

bool GLFeatures::HasDBT() {
    //"GL_EXT_depth_bounds_test"
    return false;
}

bool GLFeatures::HasDC() {
    //"GL_NV_depth_clamp"
    static bool __hasDC =
#ifdef GL_EXT_depth_clamp
            GLAD_GL_EXT_depth_clamp ||
#endif
            false;
    return __hasDC;
}

bool GLFeatures::HasUBO() {
    return __util::HasGlVersionGrEqThan<310>();
}

bool GLFeatures::HasTG() {
    static bool __hasTG = __util::HasGlVersionGrEqThan<400>();
    return __hasTG;
}

bool GLFeatures::HasMBR() {
    static bool __hasMBR =
#ifdef GL_EXT_map_buffer_range
            GLAD_GL_EXT_map_buffer_range ||
#endif
            __util::HasGlVersionGrEqThan<300>();
    return __hasMBR;
}

bool GLFeatures::HasTQ() {
    static bool __hasTQ =
#ifdef GL_EXT_disjoint_timer_query
            GLAD_GL_EXT_disjoint_timer_query ||
#endif
                    __util::HasGlVersionGrEqThan<330>();
    return __hasTQ;
}

bool GLFeatures::HasTRG() {
    //"GL_ARB_texture_rg"
    static bool __hasTRG = __util::HasGlVersionGrEqThan<300>() || false;
    return __hasTRG;
}

bool GLFeatures::HasCBF() {
    static bool __hasCBF =
#ifdef GL_EXT_color_buffer_float
            GLAD_GL_EXT_color_buffer_float ||
#endif
            __util::HasGlVersionGrEqThan<300>();
    return __hasCBF;
}

bool GLFeatures::HasCB() {
    //"GL_ARB_copy_buffer"
    static bool __hasCB = __util::HasGlVersionGrEqThan<310>() || false;
    return __hasCB;
}

bool GLFeatures::HasDB2() {
    //"GL_EXT_draw_buffers2"
    return __util::HasGlVersionGrEqThan<300>() || false;
}

bool GLFeatures::HasCR() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasTI() {
    return __util::HasGlVersionGrEqThan<300>();
}

bool GLFeatures::HasDBGO() {
    //"GL_ARB_debug_output"
    static bool __hasDBGO = __util::HasGlVersionGrEqThan<430>() || false;
    return __hasDBGO;
}

bool GLFeatures::HasCI() {
    //"GL_NV_copy_image"
    static bool __hasCI = __util::HasGlVersionGrEqThan<430>()
#ifdef GL_EXT_copy_image
                          || GLAD_GL_EXT_copy_image
#endif
#ifdef GL_EXT_copy_image
                          || GLAD_GL_OES_copy_image
#endif
                          || false;

    return __hasCI;
}

bool GLFeatures::HasBFE() {
    static bool __hasBFE = __util::HasGlVersionGrEqThan<330>()
#ifdef GL_EXT_blend_func_extended
                          || GLAD_GL_EXT_blend_func_extended
#endif
                                   ;
    return __hasBFE;
}

bool GLFeatures::HasEAL() {
    //"GL_ARB_explicit_attrib_location"
    static bool __hasEAL = __util::HasGlVersionGrEqThan<330>()
#ifdef GLAD_GL_NV_explicit_attrib_location
                           || GLAD_GL_NV_explicit_attrib_location
#endif
                           || false;
    return __hasEAL;
}

bool GLFeatures::HasTSW() {
    //"GL_ARB_texture_swizzle"
    //"GL_EXT_texture_swizzle"
    static bool __hasTSW = __util::HasGlVersionGrEqThan<330>() ||
                           false ||
                           false;
    return __hasTSW;
}

bool GLFeatures::HasOQ2() {
    //"GL_ARB_occlusion_query2"
    static bool __hasQQ2 = __util::HasGlVersionGrEqThan<330>() ||
                           false;
    return __hasQQ2;
}

bool GLFeatures::HasES3() {
    static bool __hasES3 = GLAD_GL_ES_VERSION_3_0 || GLAD_GL_ES_VERSION_3_1 || GLAD_GL_ES_VERSION_3_2;
    return __hasES3;
}

bool GLFeatures::HasGLES() {
    static bool __hasGLES = HasES3() || GLAD_GL_ES_VERSION_2_0;
    return __hasGLES;
}

int GLFeatures::GetMaxTextureSize()
{
    GLint val = 0;
    glCheckError(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val));
    return val;
}

int GLFeatures::GetMaxTextureUnits()
{
    GLint val = 0;
    glCheckError(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &val));
    return val;
}

int GLFeatures::GetMaxVertexTextureUnits()
{
    GLint val = 0;
    glCheckError(glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &val));
    return val;
}

int GLFeatures::GetMaxCubeMapTextureSize()
{
    GLint val = 0;
    glCheckError(glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &val));
    return val;
}

int GLFeatures::GetMaxDrawBuffers()
{
    GLint val = 0;
    glCheckError(glGetIntegerv(GL_MAX_DRAW_BUFFERS, &val));
    return val;
}

void GLFeatures::Checks()
{
    if (__util::HasGlVersionLowerThan<200>())
    {
        Application::Instance().Fatal("Open GL 2.0 or higher required");
    }
    if (!GLFeatures::HasPBO())
    {
        Application::Instance().Fatal("Missing required Pixel Buffer Object support");
    }
    if (ShaderVersion() < 110)
    {
        Application::Instance().Fatal("GLSL 1.10 or higher required");
    }

    hwtexsize = GetMaxTextureSize();
    if(hwtexsize < 2048)
    {
        Application::Instance().Fatal("Large texture support '>=2048' is required!");
    }

    hwtexunits = GetMaxTextureUnits();
    if(hwtexunits < 16)
    {
        Application::Instance().Fatal("Hardware does not support at least 16 texture units.");
    }

    hwvtexunits = GetMaxVertexTextureUnits();
    if(hwvtexunits < 4)
    {
        Application::Instance().Fatal("Hardware does not support at least 4 vertex texture units.");
    }

    hwcubetexsize = GetMaxCubeMapTextureSize();

    maxdrawbufs = GetMaxDrawBuffers();
    if(maxdrawbufs < 4)
    {
        Application::Instance().Fatal("Hardware does not support at least 4 draw buffers.");
    }
}

void GLFeatures::Initialize()
{
    Checks();

    if (GLFeatures::HasAF())
    {
        GLint val = 0;
        glCheckError(glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &val));
        hwmaxaniso = val;
    }

    if (GLFeatures::HasGPU5() && GLFeatures::HasTG())
    {
        tqaaresolvegather = 1;
    }

    if (GLFeatures::HasTG())
    {
        usetexgather =
                GLFeatures::HasGPU5() && !GLFeatures::hasVendor(Vendor::Intel) && !GLFeatures::hasVendor(Vendor::Nvidia) ? 2 : 1;
    }

    if (GLFeatures::hasVendor(Vendor::Amd))
    {
        msaalineardepth = glineardepth = 1; // reading back from depth-stencil still buggy on newer cards, and requires stencil for MSAA
        msaadepthstencil = gdepthstencil = 1; // some older AMD GPUs do not support reading from depth-stencil textures, so only use depth-stencil renderbuffer for now
        if(checkseries(RendererString().c_str(), "Radeon HD", 4000, 5199))
        {
            amd_pf_bug = 1;
        }

        if(__util::HasGlVersionLowerThan<400>())
        {
            amd_eal_bug = 1; // explicit_attrib_location broken when used with blend_func_extended on legacy Catalyst
            rhrect = 1; // bad cpu stalls on Catalyst 13.x when trying to use 3D textures previously bound to FBOs
        }
    }
    else if(GLFeatures::hasVendor(Vendor::Intel))
    {
        smgather = 1; // native shadow filter is slow
        if(GLFeatures::hasVendor(Vendor::Mesa))
        {
            batchsunlight = 0; // causes massive slowdown in linux driver
            if(!checkmesaversion(VersionString().c_str(), 10, 0, 3))
                mesa_texrectoffset_bug = 1; // mesa i965 driver has buggy textureOffset with texture rectangles
            msaalineardepth = 1; // MSAA depth texture access is buggy and resolves are slow
        }
        else
        {
            // causes massive slowdown in windows driver if reading depth-stencil texture
            if(checkdepthtexstencilrb())
            {
                gdepthstencil = 1;
                gstencil = 1;
            }
            // sampling alpha by itself from a texture generates garbage on Intel drivers on Windows
            intel_texalpha_bug = 1;
            // MapBufferRange is buggy on older Intel drivers on Windows
            if(__util::HasGlVersionLowerThan<310 + 1>()) {
                intel_mapbufferrange_bug = 1;
            }
        }
    }
    if(GLFeatures::hasVendor(Vendor::Mesa))
    {
        mesa_swap_bug = 1;
    }

    if (!GLAD_GL_OES_texture_3D)
    {
        rhrect = 1;
    }
}
