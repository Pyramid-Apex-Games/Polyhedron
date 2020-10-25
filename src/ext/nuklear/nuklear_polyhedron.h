#pragma once
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
//#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"
#include <SDL_events.h>
#include <string>
#include <memory>

struct Shader;
struct font;

class NkPolyhedron
{
    nk_buffer m_NkBuffer;
    nk_context m_Context;
    Shader* m_Shader;

    struct NkFont {
        NkFont(std::string fontName);
        nk_user_font m_UserFont;
        std::string m_FontName;
    };

    std::unique_ptr<NkFont> m_DefaultFont;
    static float UserFontGetWidth(nk_handle fontInstance, float h, const char*, int len);
public:
    enum class InputEventProcessState {
        Handled,
        NotHandled
    };

    NkPolyhedron();

    void Render();
    void InputProcessBegin();
    void InputProcessEnd();
    InputEventProcessState InputEvent(const SDL_Event &evt);

    nk_context* GetContext();
    float GetLineHeight();
};
