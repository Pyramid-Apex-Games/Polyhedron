#pragma once
#include <string>

extern int glcompat;

extern int amd_pf_bug;
extern int amd_eal_bug;
extern int mesa_texrectoffset_bug;
extern int intel_texalpha_bug;
extern int intel_mapbufferrange_bug;
extern int mesa_swap_bug;

extern int useubo;
extern int usetexgather;
extern int usetexcompress;
extern int maxdrawbufs;
extern int maxdualdrawbufs;

static bool checkseries(const char *s, const char *name, int low, int high);
static bool checkmesaversion(const char *s, int major, int minor, int patch);
bool checkdepthtexstencilrb();

namespace GLFeatures {
    const std::string &VersionString();
    const std::string &ShaderVersionString();
    const std::string &VendorString();
    const std::string &RendererString();

    int Version();
    int ShaderVersion();

    enum class Vendor {
        Mesa,
        Intel,
        Nvidia,
        Amd
    };

    bool hasVendor(Vendor v);

    namespace __util {
        template<int V>
        bool HasGlVersionGrEqThan() {
            static bool __vercheck = Version() >= V;
            return __vercheck;
        }

        template<int V>
        bool HasGlVersionLowerThan() {
            static bool __vercheck = Version() < V;
            return __vercheck;
        }
    }

    bool HasVAO();
    bool HasPBO();
    bool HasTFW();
    bool HasAFBO();
    bool HasDS();
    bool HasFBB();
    bool HasFBMS();
    bool HasTF();
    bool HasRGTC();
    bool HasPF();
    bool HasHFV();
    bool HasHFP();
    bool HasS3TC();
    bool HasLATC();
    bool HasGPU4();
    bool HasEGPU4();
    bool HasGPU5();
    bool HasAF();
    bool HasTMS();
    bool HasFBMSBS();
    bool HasMSS();
    bool HasDBB();
    bool HasDBT();
    bool HasDC();
    bool HasUBO();
    bool HasTG();
    bool HasMBR();
    bool HasTQ();
    bool HasTRG();
    bool HasCBF();
    bool HasCB();
    bool HasDB2();
    bool HasCR();
    bool HasTI();
    bool HasDBGO();
    bool HasCI();
    bool HasBFE();
    bool HasEAL();
    bool HasTSW();
    bool HasOQ2();
    bool HasGLES();
    bool HasES3();

    int GetMaxTextureSize();
    int GetMaxTextureUnits();
    int GetMaxVertexTextureUnits();
    int GetMaxCubeMapTextureSize();
    int GetMaxDrawBuffers();

    void Checks();
    void Initialize();
}