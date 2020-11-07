#pragma once
#include "shared/geom/ivec2.h"
#include <SDL_events.h>

class Application;

class Input
{
public:
    Input(Application& app);
    ~Input();

    void HandleEvent(const SDL_Event& event);

    void Grab(bool enable = true);
    void WantGrab(bool enable);

    void Text(bool enable = true);

    bool InterceptKey(SDL_KeyCode key);

    ivec2 GetMousePosition() const;
    bool IsMouseGrabbed() const;
private:
    bool m_IsTyping = false;
    bool m_GrabMode = false;
    ivec2 m_PointerPosition = {0, 0};
    Application& m_App;
};


