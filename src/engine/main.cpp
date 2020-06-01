#include "engine/main/Application.h"
#include "engine/main/CommandlineArguments.h"

#ifdef WIN32
// Force Optimus setups to use the NVIDIA GPU
extern "C"
{
#ifdef __GNUC__
__attribute__((dllexport))
#else
__declspec(dllexport)
#endif
	DWORD NvOptimusEnablement = 1;

#ifdef __GNUC__
__attribute__((dllexport))
#else
__declspec(dllexport)
#endif
	DWORD AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#ifdef __APPLE__
#define main SDL_main
#endif

extern "C" int main(int argc, char **argv)
{
    CommandlineArguments args(argc, argv);
    Application app(args);

    while (!app.GetQuitRequested())
    {
        app.RunFrame();
    }

    exit(EXIT_SUCCESS);
}