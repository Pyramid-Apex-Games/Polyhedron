#include "Window.h"
#include "Application.h"
#include "GLContext.h"
#include "engine/console.h"
#include "engine/movie.h"
#include <SDL_events.h>
#include <algorithm>

Window::Window(AppConfig& config)
    : m_AppConfig(config)
{
}

Window::~Window()
{
    //Write config into configFilename

    //cleanup
#ifdef __APPLE__
    if(m_Window)
    {
        SDL_SetWindowFullscreen(m_Window, 0);
    }
#endif
    SDL_SetWindowBrightness(m_Window, 1.0f);
    SDL_DestroyWindow(m_Window);
}

void Window::Initialize()
{
    SDL_Rect desktop;
    if(SDL_GetDisplayBounds(0, &desktop) < 0)
    {
        conoutf("failed querying desktop bounds: %s", SDL_GetError());
        exit(-1);
    }

    m_State.DesktopWidth = desktop.w;
    m_State.DesktopHeight = desktop.h;

    if(m_AppConfig.ScreenHeight < 0)
    {
        m_AppConfig.ScreenHeight = AppConfigDefaults.ScreenHeight;
    }
    if(m_AppConfig.ScreenWidth < 0)
    {
        m_AppConfig.ScreenWidth = (m_AppConfig.ScreenHeight * m_State.DesktopWidth) / m_State.DesktopHeight;
    }
    m_AppConfig.ScreenWidth = std::min(m_AppConfig.ScreenWidth, m_State.DesktopWidth);
    m_AppConfig.ScreenHeight = std::min(m_AppConfig.ScreenHeight, m_State.DesktopHeight);

    m_State.WindowPositionX = SDL_WINDOWPOS_UNDEFINED;
    m_State.WindowPositionY = SDL_WINDOWPOS_UNDEFINED;
    m_State.WindowWidth = m_AppConfig.ScreenWidth;
    m_State.WindowHeight = m_AppConfig.ScreenHeight;
    m_State.WindowFlags = SDL_WINDOW_RESIZABLE;
    if(m_AppConfig.Fullscreen)
    {
        m_State.WindowWidth = m_State.DesktopWidth;
        m_State.WindowHeight = m_State.DesktopHeight;
        m_State.WindowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        m_State.InitialWindowPosition = true;
    }

    GLConfig contextConfig;
    m_GLContext = std::make_unique<GLContext>(*this, contextConfig);

    m_Window = SDL_CreateWindow(
        m_State.ApplicationName.c_str(),
        m_State.WindowPositionX, m_State.WindowPositionY,
        m_State.WindowWidth, m_State.WindowHeight,
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_SHOWN |
        SDL_WINDOW_INPUT_FOCUS |
        SDL_WINDOW_MOUSE_FOCUS |
//        SDL_WINDOW_ALLOW_HIGHDPI |
        m_State.WindowFlags
    );
    if(!m_Window)
    {
        Application::Instance().Fatal("failed to create OpenGL window: %s", SDL_GetError());
        exit(-1);
    }

    SDL_SetWindowMinimumSize(m_Window, AppConstants::MinimumWindowWidth, AppConstants::MinimumWindowHeight);

    m_GLContext->CreateContext();

    SDL_RaiseWindow(m_Window);
}

void Window::ToggleFullscreen(bool enable)
{
    if (!m_Window)
    {
        return;
    }

    SDL_SetWindowFullscreen(m_Window, enable ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    if (!enable)
    {
        SDL_SetWindowSize(m_Window, m_State.WindowWidth, m_State.WindowHeight);
        if (m_State.InitialWindowPosition)
        {
            int winx = SDL_WINDOWPOS_CENTERED, winy = SDL_WINDOWPOS_CENTERED;
            SDL_SetWindowPosition(m_Window, winx, winy);
            m_State.InitialWindowPosition = false;
        }
    }

    m_AppConfig.Fullscreen = enable;
}

void Window::HandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_WINDOWEVENT)
    {
        switch (event.window.event)
        {
//            case SDL_WINDOWEVENT_CLOSE:
//                quit();
//                break;
//
//            case SDL_WINDOWEVENT_FOCUS_GAINED:
//                shouldgrab = true;
//                break;
//            case SDL_WINDOWEVENT_ENTER:
//                inputgrab(grabinput = true);
//                break;
//
//            case SDL_WINDOWEVENT_LEAVE:
//            case SDL_WINDOWEVENT_FOCUS_LOST:
//                inputgrab(grabinput = false);
//                break;
//
            case SDL_WINDOWEVENT_MINIMIZED:
                m_State.Minimized = true;
                break;

            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESTORED:
                m_State.Minimized = false;
                break;

            case SDL_WINDOWEVENT_RESIZED:
                break;

            case SDL_WINDOWEVENT_SIZE_CHANGED:
                SDL_GetWindowSize(m_Window, &m_State.WindowWidth, &m_State.WindowHeight);
                m_AppConfig.Fullscreen = SDL_GetWindowFlags(m_Window) & SDL_WINDOW_FULLSCREEN;
                if (!m_AppConfig.Fullscreen)
                {
                    m_AppConfig.ScreenWidth = m_State.WindowWidth;
                    m_AppConfig.ScreenHeight = m_State.WindowHeight;
                }
                m_GLContext->Resize();
                break;
        }
    }
}

void Window::Swap()
{
//    recorder::capture(overlay);
    SDL_GL_SwapWindow(m_Window);
}

void Window::ToggleVSync(bool enable)
{
    //FIXME: maybe move to GLContext?
    if (!m_Window)
        return;

    SDL_GL_SetSwapInterval(enable ? 1 : 0);
    m_State.VSyncEnabled = enable;
}

void Window::SetGamma(int value)
{
    if (!m_Window)
        return;

    if (SDL_SetWindowBrightness(m_Window, value/100.0f) < 0)
    {
        conoutf(CON_ERROR, "Could not set gamma: %s", SDL_GetError());
    }
}

GLContext& Window::GetContext() const
{
    return *m_GLContext.get();
}

SDL_Window *Window::GetWindowHandle() const {
    return m_Window;
}

int Window::GetWidth() const {
    return m_State.WindowWidth;
}

int Window::GetHeight() const {
    return m_State.WindowHeight;
}
