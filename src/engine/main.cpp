#include "engine/main/Application.h"
#include "engine/main/CommandlineArguments.h"

#include <SDL_main.h>

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

#ifdef WIN32
#define WIN_WStringToUTF8(S) SDL_iconv_string("UTF-8", "UTF-16LE", (char *)(S), (SDL_wcslen(S)+1)*sizeof(WCHAR))

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
    LPWSTR *argvw;
    char **argv;
    int i, argc, result;

    argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argvw == NULL) {
        return -1;
    }

    /* Parse it into argv and argc */
    argv = (char **)SDL_calloc(argc + 1, sizeof(*argv));
    if (!argv) {
        return -1;
    }
    for (i = 0; i < argc; ++i) {
        argv[i] = WIN_WStringToUTF8(argvw[i]);
        if (!argv[i]) {
            return -1;
        }
    }
    argv[i] = NULL;
    LocalFree(argvw);

    SDL_SetMainReady();

    /* Run the application main() code */
    result = SDL_main(argc, argv);

    /* Free argv, to avoid memory leak */
    for (i = 0; i < argc; ++i) {
        SDL_free(argv[i]);
    }
    SDL_free(argv);

    return result;
}
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