#include "renderdoc_api.h"
#ifdef HAVE_RENDERDOC_H
#include <dlfcn.h>
#include <renderdoc.h>
#include <cassert>

namespace {
    RENDERDOC_API_1_4_0 *rdoc_api = nullptr;
}

void RenderdocApi::Initialize()
{
    if(void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_0, (void **)&rdoc_api);
        assert(ret == 1);
    }
}

void RenderdocApi::StartCapture()
{
    if(rdoc_api)
    {
        rdoc_api->StartFrameCapture(nullptr, nullptr);
    }
}

void RenderdocApi::StopCapture()
{
    if(rdoc_api)
    {
        rdoc_api->EndFrameCapture(nullptr, nullptr);
    }
}

#else

void RenderdocApi::Initialize()
{
}

void RenderdocApi::StartCapture()
{
}

void RenderdocApi::StopCapture()
{
}
#endif