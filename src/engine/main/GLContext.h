#pragma once
#include "AppConfig.h"
#include <SDL_video.h>

class Window;

class GLContext {
public:
    GLContext(Window& window, const GLConfig& config);
    ~GLContext();

    bool CreateContext();
    void Resize();
    void ApplyValuesToCubeScript();

    void GetFramebufferSize(int& w, int& h) const;

    SDL_GLContext GetContextHandle() const;
private:
    GLConfig m_Config;
    GLContextState m_ContextState;
    SDL_GLContext m_Context;
    Window& m_Window;
};


