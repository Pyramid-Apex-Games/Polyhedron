#include "Compatibility.h"
#include "Application.h"
#include "Renderer.h"
#include "Input.h"


SkeletalEntity *player = nullptr;

bool interceptkey(int sym)
{
    auto key = static_cast<SDL_KeyCode>(sym);
    return Application::Instance().GetInput().InterceptKey(key);
}

void renderprogress(float bar, const char *text, bool background)   // also used during loading
{
    return Application::Instance().GetRenderer().RenderProgress(bar, text, background);
}

void renderbackground(const char *caption, Texture *mapshot, const char *mapname, const char *mapinfo, bool force)
{
    BackgroundInfo bgInfo {
        caption ? caption : "",
        mapshot,
        mapname ? mapname : "",
        mapinfo ? mapinfo : ""
    };

    Application::Instance().GetRenderer().RenderBackground(bgInfo, force);
}

void quit()
{
    Application::Instance().Quit();
}

void fatal(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t size = snprintf(nullptr, 0, fmt, args) + 1;
    if (size <= 0)
    {
        printf(fmt, args);
        exit(-1);
    }
    else
    {
        std::vector<char> messageBuf(size, '\0');
        snprintf(messageBuf.data(), size, fmt, args);
        std::string message(messageBuf.begin(), messageBuf.end());
        Application::Instance().Fatal(message);
    }
    va_end(args);
}

void keyrepeat(bool on, int mask)
{
    //noop
    void;
    //FIXME: understand this function
}

void textinput(bool on, int mask)
{
    Application::Instance().GetInput().Text(on);
}

SCRIPTEXPORT void mouselook(CommandTypes::Boolean enable)
{
    Application::Instance().GetInput().Grab(*enable == 1);
}