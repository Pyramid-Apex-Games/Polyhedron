#include "Input.h"
#include "Application.h"
#include "Window.h"
#include "engine/console.h"
#include "shared/types.h"
#include "engine/physics.h"
#include <SDL_events.h>

//FIXME: extern
extern size_t decodeutf8(uchar *dst, size_t dstlen, const uchar *src, size_t srclen, size_t *carry);

Input::Input(Application& app)
    : m_App(app)
{
}

Input::~Input()
{
    SDL_ShowCursor(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    if (m_App.GetWindow().GetWindowHandle())
    {
        SDL_SetWindowGrab(m_App.GetWindow().GetWindowHandle(), SDL_FALSE);
    }
}

void Input::HandleEvent(const SDL_Event& event)
{
    switch(event.type)
    {
        case SDL_TEXTINPUT:
            if (m_IsTyping)
            {
                uchar buf[SDL_TEXTINPUTEVENT_TEXT_SIZE+1];
                size_t len = decodeutf8(buf, sizeof(buf)-1, (const uchar *)event.text.text, strlen(event.text.text));
                if(len > 0)
                {
                    buf[len] = '\0';
                    processtextinput((const char *)buf, len);
                }
            }
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (!event.key.repeat)
            {
                processkey(event.key.keysym.sym, event.key.state == SDL_PRESSED);
            }
            break;

        case SDL_MOUSEMOTION:
            {
                if (m_GrabMode)
                {
                    mousemove(event.motion.xrel, event.motion.yrel);
                    SDL_WarpMouseInWindow(m_App.GetWindow().GetWindowHandle(), m_App.GetWindow().GetWidth() / 2, m_App.GetWindow().GetHeight() / 2);
                }
                else
                {
                    m_PointerPosition.x = event.motion.x;
                    m_PointerPosition.y = event.motion.y;
                }
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            switch(event.button.button)
            {
                case SDL_BUTTON_LEFT: processkey(-1, event.button.state==SDL_PRESSED); break;
                case SDL_BUTTON_MIDDLE: processkey(-2, event.button.state==SDL_PRESSED); break;
                case SDL_BUTTON_RIGHT: processkey(-3, event.button.state==SDL_PRESSED); break;
                case SDL_BUTTON_X1: processkey(-6, event.button.state==SDL_PRESSED); break;
                case SDL_BUTTON_X2: processkey(-7, event.button.state==SDL_PRESSED); break;
            }
            break;

        case SDL_MOUSEWHEEL: {
            if(event.wheel.y > 0)
            {
                processkey(-4, true);
                processkey(-4, false);
            }
            else if(event.wheel.y < 0)
            {
                processkey(-5, true);
                processkey(-5, false);
            }
        } break;
    }
}

void Input::Grab(bool enable)
{
    m_GrabMode = enable;

    if (enable)
    {
        if (SDL_SetRelativeMouseMode(SDL_TRUE) >= 0)
        {
            SDL_SetWindowGrab(m_App.GetWindow().GetWindowHandle(), SDL_TRUE);
        }
        else
        {
            SDL_SetWindowGrab(m_App.GetWindow().GetWindowHandle(), SDL_FALSE);
            m_GrabMode = false;
        }
    }
    else
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        SDL_SetWindowGrab(m_App.GetWindow().GetWindowHandle(), SDL_FALSE);
    }
}

void Input::WantGrab(bool wantGrab)
{
    if (wantGrab)
    {
        if (m_GrabMode)
        {
            Grab(m_GrabMode);
        }
    }
    else
    {
        if (m_GrabMode)
        {
            Grab(false);
            m_GrabMode = true;
        }
    }
}

void Input::Text(bool enable)
{
    if (m_IsTyping != enable)
    {
        if (enable)
        {
            SDL_StartTextInput();
        }
        else
        {
            SDL_StopTextInput();
        }

        m_IsTyping = enable;
    }
}

bool Input::InterceptKey(SDL_KeyCode key)
{
    bool intercepted = false;
    m_App.PeekEvents([&](SDL_Event& event) -> Application::PeekEventAction {
        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == key)
            {
                intercepted = true;
                return Application::PeekEventAction::Drop;
            }
        }

        return Application::PeekEventAction::Cache;
    });

    return intercepted;
}

ivec2 Input::GetMousePosition() const
{
    return m_PointerPosition;
}

bool Input::IsMouseGrabbed() const
{
    return m_GrabMode;
}
