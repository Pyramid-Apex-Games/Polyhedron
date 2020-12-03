#pragma once
#include "AppConfig.h"
#include "engine/console.h"
#include "shared/tools/stream.h"
#include "shared/stream.h"
#include "shared/python/PythonScript.h"
#include <nlohmann/json.hpp>
#include <SDL_events.h>
#include <list>
#include <cstring>

class CommandlineArguments;
class Input;
class Renderer;
class Window;
class SoundConfig;
class Console;
class State;

#ifdef __APPLE__
#define main SDL_main
#endif

extern int numcpus;

class Application {
public:
    Application(const CommandlineArguments& commandlineArguments);
    ~Application();

    enum class PeekEventAction {
        Cache, Drop
    };
    using PeekEventCallback = std::function<PeekEventAction(SDL_Event&)>;

    static Application& Instance();

    Window& GetWindow() const;
    Renderer& GetRenderer() const;
    Input& GetInput() const;
    SoundConfig& GetSoundConfig() const;
    const AppState& GetAppState() const;
#ifdef BUILD_WITH_PYTHON
    PythonScript& GetPython() const;
#endif
    Console& GetConsole() const;

    void RunFrame();
    void ProcessEvents();
    void PeekEvents(const PeekEventCallback& peekCallback);

    template<class T>
    void SetState()
    {
        m_ActiveState = std::make_unique<T>();
    }

    template<class T = State>
    T& GetState() const
    {
        if (!m_ActiveState)
        {
            Instance().Fatal("ActiveState == nullptr!");
        }
        return static_cast<T&>(*m_ActiveState.get());
    }

    template <class... Args>
    void Fatal(const std::string& s, Args&&...);
    void Fatal(const std::string& s);

    void Quit();
    bool GetQuitRequested() const;
private:

    bool m_QuitRequested = false;
    AppConfig m_AppConfig;
    AppState m_AppState;
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<Input> m_Input;
    std::unique_ptr<SoundConfig> m_SoundConfig;
    std::unique_ptr<State> m_ActiveState;
#ifdef BUILD_WITH_PYTHON
    std::unique_ptr<PythonScript> m_Python;
#endif
    std::unique_ptr<Console> m_Console;

    std::list<SDL_Event> m_CachedEvents;
};


template <typename T>
bool LoadJson(Application& app, const std::string& filename, T& intoObject, bool required = false)
{
    std::function<void(std::string, std::string)> report;
    if (required)
    {
        report = [&app](std::string filename, std::string msg){
            app.Fatal("Unable to read json %s: %s", filename.c_str(), msg.c_str());
        };
    }
    else
    {
        report = [](std::string filename, std::string msg){
            conoutf(CON_ERROR, "Unable to read json %s: %s", filename.c_str(), msg.c_str());
        };
    }

    stream* fileHandle = openrawfile(filename.c_str(), "r");
    if (fileHandle)
    {
        auto size = fileHandle->size();
        std::string buffer;
        buffer.resize(size);
        if (fileHandle->read(buffer.data(), size))
        {
            try {
                auto document = nlohmann::json::parse(buffer);
                document.get_to(intoObject);

                return true;
            }
            catch(nlohmann::json::exception& error)
            {
                report(filename, error.what());
            }
            catch(...)
            {
                report(filename, "unknown exception occurred");
            }
        }
    }

    report(filename, "unable to read file");
    return !required;
}
