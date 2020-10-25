#pragma once
#include "AppConfig.h"
#include <SDL_video.h>
#include <SDL_events.h>

class GLContext;

class Window
{
public:
    Window(AppConfig& config);
    ~Window();

    void Initialize();

    void HandleEvent(const SDL_Event& event);

    void ToggleFullscreen(bool enable = true);
    void ToggleVSync(bool enable = true);
    void SetGamma(int value = 100);

    void Swap();
    GLContext& GetContext() const;
    SDL_Window* GetWindowHandle() const;

    int GetWidth() const;
    int GetHeight() const;
private:
    std::string m_ConfigFilename;
    AppConfig& m_AppConfig;
    AppState m_State;

    SDL_Window* m_Window = nullptr;
    std::unique_ptr<GLContext> m_GLContext;
};


