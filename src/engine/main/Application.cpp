#include "Application.h"
#include "CommandlineArguments.h"
#include "SoundConfig.h"
#include "Clock.h"
#include "FPS.h"
#include "Window.h"
#include "Renderer.h"
#include "Input.h"
#include "engine/console.h"
#include "Compatibility.h"
#include "renderdoc_api.h"
#include "shared/command.h"
#include "engine/log.h"
#include "engine/server.h"
#include "engine/SoundConfig.h"
#include "engine/ui.h"
#include "engine/nui/nui.h"
#include "engine/menus.h"
#include "engine/octaedit.h"
#include "engine/command.h"
#include "engine/renderparticles.h"
#include "shared/entities/EntityFactory.h"
#include "game/entities/SkeletalEntity.h"
#include "shared/entities/Entity.h"
#include <SDL.h>

VAR(numcpus, 1, 1, 16);

namespace {
    static Application* applicationInstance = nullptr;
}

Application::Application(const CommandlineArguments& commandlineArguments)
{
    assert(applicationInstance == nullptr && "Application is a singleton, only 1 instance allowed");
    applicationInstance = this;

    m_Console = std::make_unique<Console>();

    m_Input = std::make_unique<Input>(*this);

    RenderdocApi::Initialize();
    numcpus = SDL_GetCPUCount();

    if (auto homedirPath = commandlineArguments.Get(Argument::HomeDirectory); !homedirPath.empty())
    {
        sethomedir(homedirPath.c_str());
    }

    if (auto logfileName = commandlineArguments.Get(Argument::LogFilename); logfileName.empty())
    {
        setlogfile(logfileName.c_str());
        logoutf("Setting log file: %s", logfileName.c_str());
    }

    LoadJson(*this, "config/init.json", m_AppConfig);

    logoutf("init: sdl");
    if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
    {
        Fatal("Unable to initialize SDL: %s", SDL_GetError());
    }

#if !defined(WIN32) && !defined(__APPLE__)
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#endif
#if defined(ANDROID)
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif

    m_Window = std::make_unique<Window>(m_AppConfig);

    //TODO: we still need this?
    SDL_StopTextInput(); // workaround for spurious text-input events getting sent on first text input toggle?

    m_Window->Initialize();
    m_Renderer = std::make_unique<Renderer>(*m_Window);

    m_Renderer->Initialize();

    m_SoundConfig = std::make_unique<SoundConfig>();
    LoadJson(*this, "config/sound.json", *m_SoundConfig, false);
    m_SoundConfig->Load();

    m_Renderer->Initialize();

    m_Python = std::make_unique<PythonScript>(commandlineArguments.Get(Argument::Any));

//    gl_setupframe(true);
    engine::nui::Initialize();
    UI::setup();

    m_Renderer->RenderBackground("Initializing...");

    logoutf("init: mainloop");
    localconnect();

    resetfpshistory();

    //FIXME: move to game
    game::initclient();
    player = game::player1 = dynamic_cast<SkeletalEntity*>(game::iterdynents(0));
    camera1 = new MovableEntity();
    //emptymap(0, true, NULL, false);

    execfile("config/stdlib.cfg", true);
    execfile("config/keymap.cfg", true);
    execfile("config/default.cfg", true);
    execfile("config/ui/common.cfg", true);
    execfile("config/stdedit.cfg", true);
    execfile("config/saved.cfg", true);

    initmumble();

    m_Python->RunString("from time import time,ctime\n"
                       "print('Hello from Python, today is', ctime(time()))\n");

    m_Python->RunString("import sys\n"
                       "print('sys.path = ', sys.path)\n");

    m_Python->RunString("import sys\n"
                       "print('sys.path = ', sys.path)\n");

    m_Python->RunString("import sys\n"
                       "print('sys.path = ', sys.path)\n");

    m_Python->RunString("import sys\n"
                       "print('error message!', file=sys.stderr)\n");
}

Application::~Application()
{
    m_Python.release();
    m_Renderer.release();
    m_Window.release();
    delete camera1;
}

namespace game {
    extern void updateworld();
}

void Application::RunFrame()
{
    ClockFrame();

    ProcessEvents();
    m_Python->Update();
    m_Console->Update();

    UI::update();
    engine::nui::Update();
    menuprocess();
    tryedit();

    if(lastmillis) game::updateworld();

    checksleep(lastmillis);

    serverslice(false, 0);

    updatefpshistory(elapsedtime);

    m_Renderer->RunFrame();

    m_Window->Swap();
}

void Application::ProcessEvents()
{
    SDL_Event event;
    engine::nui::InputProcessBegin();
    while(true)
    {
        if (!m_CachedEvents.empty())
        {
            event = m_CachedEvents.front();
            m_CachedEvents.pop_front();
        }
        else
        {
            if (!SDL_PollEvent(&event))
            {
                break;
            }
        }

        if (event.type == SDL_QUIT)
            Quit();

        if (engine::nui::InputEvent(event) == engine::nui::InputEventProcessState::Handled)
        {
            continue;
        }

        switch(event.type)
        {

            case SDL_WINDOWEVENT:
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_CLOSE:
                        Quit();
                        break;

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
//                        shouldgrab = true;
                        break;
                    case SDL_WINDOWEVENT_ENTER:
                        m_Input->WantGrab(true);
                        break;

                    case SDL_WINDOWEVENT_LEAVE:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        m_Input->WantGrab(false);
//                        inputgrab(grabinput = false);
                        break;

                }
                break;
        }

        m_Window->HandleEvent(event);
        m_Input->HandleEvent(event);
    }
    engine::nui::InputProcessEnd();
}

void Application::Quit()
{
    m_QuitRequested = true;
}

template <class... Args>
void Application::Fatal(const std::string& s, Args&&... args)
{
    static int errors = 0;
    errors++;

    if(errors <= 2) // print up to one extra recursive error
    {
        logoutf(s.c_str(), std::forward<Args>(args)...);

        if(errors <= 1) // avoid recursion
        {
            if(SDL_WasInit(SDL_INIT_VIDEO))
            {
                if(m_Window)
                {
                    m_Window->SetGamma();
#ifdef __APPLE__
                    m_Window->ToggleFullscreen(false);
#endif
                }
            }

            //

            Quit();

            size_t size = snprintf(nullptr, 0, s.c_str(), args...) + 1;
            if (size <= 0)
            {
                SDL_ShowSimpleMessageBox(
                    SDL_MESSAGEBOX_ERROR,
                    (m_AppState.ApplicationName + " - fatal error").c_str(),
                    s.c_str(),
                    NULL
                );
            }
            else
            {
                std::vector<char> messageBuf(size, '\0');
                snprintf(messageBuf.data(), size, s.c_str(), args...);
                SDL_ShowSimpleMessageBox(
                    SDL_MESSAGEBOX_ERROR,
                    (m_AppState.ApplicationName + " - fatal error").c_str(),
                    messageBuf.data(),
                    NULL
                );
            }
        }
    }

    exit(EXIT_FAILURE);
}

void Application::Fatal(const std::string& s)
{
    Fatal(s, "");
}

bool Application::GetQuitRequested() const
{
    return m_QuitRequested;
}

void Application::PeekEvents(const Application::PeekEventCallback &peekCallback)
{
    SDL_Event event;
    engine::nui::InputProcessBegin();
    while(SDL_PollEvent(&event))
    {
        switch (peekCallback(event))
        {
            case PeekEventAction::Cache:
                m_CachedEvents.push_back(event);
                break;
            case PeekEventAction::Drop:
                break;
        }
    }
}

Application& Application::Instance()
{
    assert(applicationInstance && "Application instance not initialized!");

    return *applicationInstance;
}

Window& Application::GetWindow() const
{
    assert(m_Window && "Window not initialized!");

    return *m_Window.get();
}

Renderer& Application::GetRenderer() const
{
    assert(m_Renderer && "Renderer not initialized!");

    return *m_Renderer.get();
}

Input& Application::GetInput() const
{
    assert(m_Input && "Input not initialized!");

    return *m_Input.get();
}

SoundConfig& Application::GetSoundConfig() const
{
    assert(m_SoundConfig && "SoundConfig not initialized!");

    return *m_SoundConfig.get();
}

const AppState& Application::GetAppState() const
{
    return m_AppState;
}

Console& Application::GetConsole() const
{
    return *m_Console.get();
}
