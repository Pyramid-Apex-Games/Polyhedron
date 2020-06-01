#include "nuklear_polyhedron.h"
#include "shared/cube.h"
#include "engine/engine.h"
#define NK_IMPLEMENTATION
#include "nuklear.h"
#include "glemu.h"
#include "tools.h"
#include "texture.h"
#include "engine/font.h"
#include "engine/rendergl.h"
#include "engine/main/Application.h"
#include "engine/main/Window.h"
#include "engine/main/GLContext.h"

NkPolyhedron::NkFont::NkFont(std::string fontName)
{
    font* fontInstance = findfont(m_FontName.c_str());
    if (fontInstance)
    {
        m_FontName = fontName;
    }
    else
    {
        m_FontName = "default";
    }
    m_UserFont.userdata = nk_handle_ptr(this);
    auto fontScale = 1.0f;//float(fontInstance->c);
    m_UserFont.height = 22.0f;//fontInstance->scale;
    m_UserFont.width = &UserFontGetWidth;
}

float NkPolyhedron::UserFontGetWidth(nk_handle fontInstance, float h, const char* text, int len)
{
    NkFont *font = (NkFont*)fontInstance.ptr;
    if (!font || !text || len == 0)
    {
        return 0.0f;
    }

    auto fontObject = findfont(font->m_FontName.c_str());
    auto fontScale = float(fontObject->scale);
    float scale = h / fontScale;

    auto fontLen = text_widthf(std::string(text, len).c_str());

    return fontLen * scale;
}

NkPolyhedron::NkPolyhedron()
    : m_DefaultFont(std::make_unique<NkFont>("default_outline"))
{
    //m_Shader = lookupshaderbyname("hud");
    nk_buffer_init_default(&m_NkBuffer);
    nk_init_default(&m_Context, &m_DefaultFont->m_UserFont);
}

void NkPolyhedron::Render()
{
    const nk_command* cmd;
    int screenw = 0, screenh = 0;
    Application::Instance().GetWindow().GetContext().GetFramebufferSize(screenw, screenh);
    nk_foreach(cmd, &m_Context)
    {
        switch (cmd->type) {
            case NK_COMMAND_NOP: break;
            case NK_COMMAND_SCISSOR: {
                const auto* scissorCmd = reinterpret_cast<const nk_command_scissor*>(cmd);
                glCheckError(glEnable(GL_SCISSOR_TEST));
                glCheckError(
                    glScissor(
                        scissorCmd->x,
                        screenh + (scissorCmd->y + scissorCmd->h) * -1,
                        scissorCmd->w,
                        scissorCmd->h
                    )
                );
            } break;
            case NK_COMMAND_LINE: {
                const auto* lineCmd = reinterpret_cast<const nk_command_line*>(cmd);

                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);
                glCheckError(glLineWidth(lineCmd->line_thickness));

                gle::begin(GL_LINE_LOOP);
                gle::attribf(float(lineCmd->begin.x), float(lineCmd->begin.y));
                gle::attribub(lineCmd->color.r, lineCmd->color.g, lineCmd->color.b, lineCmd->color.a);

                gle::attribf(float(lineCmd->end.x), float(lineCmd->end.y));
                gle::attribub(lineCmd->color.r, lineCmd->color.g, lineCmd->color.b, lineCmd->color.a);

                xtraverts += gle::end();
            } break;
            case NK_COMMAND_RECT: {
                const auto* rectCmd = reinterpret_cast<const nk_command_rect*>(cmd);

                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);

                gle::begin(GL_LINE_LOOP);

                gle::attribf(rectCmd->x             , rectCmd->y);
                gle::attribub(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b, rectCmd->color.a);

                gle::attribf(rectCmd->x + rectCmd->w, rectCmd->y);
                gle::attribub(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b, rectCmd->color.a);

                gle::attribf(rectCmd->x + rectCmd->w, rectCmd->y + rectCmd->h);
                gle::attribub(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b, rectCmd->color.a);

                gle::attribf(rectCmd->x             , rectCmd->y + rectCmd->h);
                gle::attribub(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b, rectCmd->color.a);

                xtraverts += gle::end();
            } break;
            case NK_COMMAND_RECT_FILLED: {
                const auto* rectCmd = reinterpret_cast<const nk_command_rect_filled*>(cmd);

                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);

                gle::begin(GL_TRIANGLE_STRIP);

                gle::attribf(rectCmd->x + rectCmd->w, rectCmd->y);
                gle::attribub(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b, rectCmd->color.a);

                gle::attribf(rectCmd->x             , rectCmd->y);
                gle::attribub(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b, rectCmd->color.a);

                gle::attribf(rectCmd->x + rectCmd->w, rectCmd->y + rectCmd->h);
                gle::attribub(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b, rectCmd->color.a);

                gle::attribf(rectCmd->x             , rectCmd->y + rectCmd->h);
                gle::attribub(rectCmd->color.r, rectCmd->color.g, rectCmd->color.b, rectCmd->color.a);

                xtraverts += gle::end();
            } break;
            case NK_COMMAND_CIRCLE: {
                const auto* circleCmd = reinterpret_cast<const nk_command_circle*>(cmd);
                vec2 ptr (1.0f, 0.0f);
                float w = circleCmd->w;
                float h = circleCmd->h;
                float rw = w / 2.0f;
                float rh = h / 2.0f;
                float x = float(circleCmd->x) + rw;
                float y = float(circleCmd->y) + rh;

                float circum = sqrt(rw * rw + rh * rh) * PI * 2.0f;
                int segmentCount = std::max(8, int(circum / 15.0f));
                float anglePerSegment = (PI * 2) / float(segmentCount);

                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);
                glCheckError(glLineWidth(circleCmd->line_thickness));

                gle::begin(GL_LINE_LOOP);

                for (int i = 0; i < segmentCount; ++i)
                {
                    gle::attribf(x + ptr.x * rw, y + ptr.y * rh);
                    gle::attribub(circleCmd->color.r, circleCmd->color.g, circleCmd->color.b, circleCmd->color.a);

                    ptr.rotate_around_z(anglePerSegment);
                }

                xtraverts += gle::end();
            } break;
            case NK_COMMAND_CIRCLE_FILLED: {
                const auto* circleCmd = reinterpret_cast<const nk_command_circle_filled*>(cmd);
                vec2 ptr (1.0f, 0.0f);
                float w = circleCmd->w;
                float h = circleCmd->h;
                float rw = w / 2.0f;
                float rh = h / 2.0f;
                float x = float(circleCmd->x) + rw;
                float y = float(circleCmd->y) + rh;

                float circum = sqrt(rw * rw + rh * rh) * PI * 2.0f;
                int segmentCount = std::max(8, int(circum / 15.0f));
                float anglePerSegment = (PI * 2) / float(segmentCount);

                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);

                gle::begin(GL_TRIANGLE_FAN);

                gle::attribf(x, y);
                gle::attribub(circleCmd->color.r, circleCmd->color.g, circleCmd->color.b, circleCmd->color.a);

                gle::attribf(x + ptr.x * rw, y + ptr.y * rh);
                gle::attribub(circleCmd->color.r, circleCmd->color.g, circleCmd->color.b, circleCmd->color.a);

                for (int i = 0; i < segmentCount; ++i)
                {
                    ptr.rotate_around_z(anglePerSegment);

                    gle::attribf(x + ptr.x * rw, y + ptr.y * rh);
                    gle::attribub(circleCmd->color.r, circleCmd->color.g, circleCmd->color.b, circleCmd->color.a);
                }

                xtraverts += gle::end();
            } break;
            case NK_COMMAND_TRIANGLE: {
                const auto* triangleCmd = reinterpret_cast<const nk_command_triangle*>(cmd);
                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);

                gle::begin(GL_TRIANGLE_FAN);

                gle::attribf(triangleCmd->a.x, triangleCmd->a.y);
                gle::attribub(triangleCmd->color.r, triangleCmd->color.g, triangleCmd->color.b, triangleCmd->color.a);

                gle::attribf(triangleCmd->b.x, triangleCmd->b.y);
                gle::attribub(triangleCmd->color.r, triangleCmd->color.g, triangleCmd->color.b, triangleCmd->color.a);

                gle::attribf(triangleCmd->c.x, triangleCmd->c.y);
                gle::attribub(triangleCmd->color.r, triangleCmd->color.g, triangleCmd->color.b, triangleCmd->color.a);

                xtraverts += gle::end();
            } break;
            case NK_COMMAND_TRIANGLE_FILLED: {
                const auto* triangleFilledCmd = reinterpret_cast<const nk_command_triangle_filled*>(cmd);
                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);

                gle::begin(GL_LINE_LOOP);

                gle::attribf(triangleFilledCmd->a.x, triangleFilledCmd->a.y);
                gle::attribub(triangleFilledCmd->color.r, triangleFilledCmd->color.g, triangleFilledCmd->color.b, triangleFilledCmd->color.a);

                gle::attribf(triangleFilledCmd->b.x, triangleFilledCmd->b.y);
                gle::attribub(triangleFilledCmd->color.r, triangleFilledCmd->color.g, triangleFilledCmd->color.b, triangleFilledCmd->color.a);

                gle::attribf(triangleFilledCmd->c.x, triangleFilledCmd->c.y);
                gle::attribub(triangleFilledCmd->color.r, triangleFilledCmd->color.g, triangleFilledCmd->color.b, triangleFilledCmd->color.a);

                xtraverts += gle::end();
            } break;
            case NK_COMMAND_POLYGON: {
                const auto* polygonCmd = reinterpret_cast<const nk_command_polygon*>(cmd);
                if (polygonCmd->point_count > 0) {
                    gle::defvertex(2);
                    gle::defcolor(4, GL_UNSIGNED_BYTE);
                    glCheckError(glLineWidth(polygonCmd->line_thickness));

                    gle::begin(GL_LINE_LOOP);
                    const struct nk_vec2i *points = &polygonCmd->points[0];
                    for (int i = 1; i < polygonCmd->point_count; i++) {
                        gle::attribf(float(points[i - 1].x), float(points[i - 1].y));
                        gle::attribub(polygonCmd->color.r, polygonCmd->color.g, polygonCmd->color.b, polygonCmd->color.a);

                        gle::attribf(float(points[i].x), float(points[i].x));
                        gle::attribub(polygonCmd->color.r, polygonCmd->color.g, polygonCmd->color.b, polygonCmd->color.a);
                    }
                    gle::attribf(float(points[polygonCmd->point_count - 1].x), float(points[polygonCmd->point_count - 1].x));
                    gle::attribub(polygonCmd->color.r, polygonCmd->color.g, polygonCmd->color.b, polygonCmd->color.a);
                    xtraverts += gle::end();
                }
            } break;
            case NK_COMMAND_POLYGON_FILLED: {
                const auto* polygonFilledCmd = reinterpret_cast<const nk_command_polygon_filled*>(cmd);
                if (polygonFilledCmd->point_count > 0) {
                    gle::defvertex(2);
                    gle::defcolor(4, GL_UNSIGNED_BYTE);


                    gle::begin(GL_TRIANGLE_FAN);
                    const struct nk_vec2i *points = &polygonFilledCmd->points[0];
                    for (int i = 1; i < polygonFilledCmd->point_count; i++) {
                        gle::attribf(float(points[i - 1].x), float(points[i - 1].y));
                        gle::attribub(polygonFilledCmd->color.r, polygonFilledCmd->color.g, polygonFilledCmd->color.b, polygonFilledCmd->color.a);

                        gle::attribf(float(points[i].x), float(points[i].x));
                        gle::attribub(polygonFilledCmd->color.r, polygonFilledCmd->color.g, polygonFilledCmd->color.b, polygonFilledCmd->color.a);
                    }

                    xtraverts += gle::end();
                }
            } break;
            case NK_COMMAND_POLYLINE: {
                const auto* polylineCmd = reinterpret_cast<const nk_command_polyline*>(cmd);
                if (polylineCmd->point_count > 0) {
                    gle::defvertex(2);
                    gle::defcolor(4, GL_UNSIGNED_BYTE);
                    glCheckError(glLineWidth(polylineCmd->line_thickness));

                    gle::begin(GL_LINE_LOOP);
                    const struct nk_vec2i *points = &polylineCmd->points[0];
                    for (int i = 1; i < polylineCmd->point_count; i++) {
                        gle::attribf(float(points[i - 1].x), float(points[i - 1].y));
                        gle::attribub(polylineCmd->color.r, polylineCmd->color.g, polylineCmd->color.b, polylineCmd->color.a);

                        gle::attribf(float(points[i].x), float(points[i].x));
                        gle::attribub(polylineCmd->color.r, polylineCmd->color.g, polylineCmd->color.b, polylineCmd->color.a);
                    }
                    xtraverts += gle::end();
                }
            } break;
            case NK_COMMAND_TEXT: {
                const auto* textCmd = reinterpret_cast<const nk_command_text*>(cmd);
                float x = textCmd->x;
                float y = textCmd->y;
                float w = textCmd->w;
                float h = textCmd->h;

                std::string str(textCmd->string, textCmd->length);
                pushfont();
                auto textFont = reinterpret_cast<NkFont*>(textCmd->font->userdata.ptr);
                auto fontObject = findfont(textFont->m_FontName.c_str());
                auto fontScale = float(fontObject->scale);
                setfont(fontObject);
                float oldscale = textscale;
//                textscale = h / float(fontObject->defaulth);
                textscale = float(textCmd->font->height) / fontScale;
                float textscaleInv = 1.0f / textscale;
                glCheckError(glEnable(GL_BLEND));
                draw_text(
                    str.c_str(),
                    x * textscaleInv, y * textscaleInv,
                    textCmd->foreground.r,
                    textCmd->foreground.g,
                    textCmd->foreground.b,
                    textCmd->foreground.a
                );
                glCheckError(glDisable(GL_BLEND));
                textscale = oldscale;
                popfont();
            } break;
            case NK_COMMAND_IMAGE: {
                const auto* imageCmd = reinterpret_cast<const nk_command_image*>(cmd);

                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);
                gle::deftexcoord0();

                gle::begin(GL_TRIANGLE_STRIP);

                gle::attribf(imageCmd->x + imageCmd->w, imageCmd->y);
                gle::attribub(imageCmd->col.r, imageCmd->col.g, imageCmd->col.b, imageCmd->col.a);
                gle::attribf(imageCmd->img.region[0], imageCmd->img.region[0]);

                gle::attribf(imageCmd->x             , imageCmd->y);
                gle::attribub(imageCmd->col.r, imageCmd->col.g, imageCmd->col.b, imageCmd->col.a);
                gle::attribf(imageCmd->img.region[0], imageCmd->img.region[1]);

                gle::attribf(imageCmd->x + imageCmd->w, imageCmd->y + imageCmd->h);
                gle::attribub(imageCmd->col.r, imageCmd->col.g, imageCmd->col.b, imageCmd->col.a);
                gle::attribf(imageCmd->img.region[1], imageCmd->img.region[1]);

                gle::attribf(imageCmd->x             , imageCmd->y + imageCmd->h);
                gle::attribub(imageCmd->col.r, imageCmd->col.g, imageCmd->col.b, imageCmd->col.a);
                gle::attribf(imageCmd->img.region[1], imageCmd->img.region[0]);

                xtraverts += gle::end();
            } break;
            case NK_COMMAND_CURVE: {
                // How to draw a spline, since this seems to be used for that. (Check out the node_editor.c)
                // https://community.khronos.org/t/drawing-a-bezier-spline/73966
                const auto* curveCmd = reinterpret_cast<const nk_command_curve*>(cmd);
                int lod = 20;

                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);
                glCheckError(glLineWidth(curveCmd->line_thickness));

                gle::begin(GL_LINE_STRIP);
                for (int i = 0; i < lod; i++) {
                    float t = (float)i / (lod - 1);
                    float it = 1.0f - t;

                    float b0 = t * t * t;
                    float b1 = 3 * t * t * it;
                    float b2 = 3 * t * it * it;
                    float b3 = it * it * it;

                    float x = b0 * curveCmd->begin.x +
                            b1 * curveCmd->ctrl[0].x +
                            b2 * curveCmd->ctrl[1].x +
                            b3 * curveCmd->end.x;
                    float y = b0 * curveCmd->begin.y +
                            b1 * curveCmd->ctrl[0].y +
                            b2 * curveCmd->ctrl[1].y +
                            b3 * curveCmd->end.y;

                    gle::attribf(x, y);
                    gle::attribub(curveCmd->color.r, curveCmd->color.g, curveCmd->color.b, curveCmd->color.a);
                }
                xtraverts += gle::end();
            } break;
            case NK_COMMAND_ARC: {
                conoutf(CON_WARN, "%s", "Unknown NK_COMMAND_ARC draw command detected.");
            } break;
            case NK_COMMAND_RECT_MULTI_COLOR: {
                const auto* rectMultiColorCmd = reinterpret_cast<const nk_command_rect_multi_color*>(cmd);

                gle::defvertex(2);
                gle::defcolor(4, GL_UNSIGNED_BYTE);

                gle::begin(GL_TRIANGLE_STRIP);

                gle::attribf(rectMultiColorCmd->x + rectMultiColorCmd->w, rectMultiColorCmd->y);
                gle::attribub(rectMultiColorCmd->left.r, rectMultiColorCmd->left.g, rectMultiColorCmd->left.b, rectMultiColorCmd->left.a);

                gle::attribf(rectMultiColorCmd->x             , rectMultiColorCmd->y);
                gle::attribub(rectMultiColorCmd->top.r, rectMultiColorCmd->top.g, rectMultiColorCmd->top.b, rectMultiColorCmd->top.a);

                gle::attribf(rectMultiColorCmd->x + rectMultiColorCmd->w, rectMultiColorCmd->y + rectMultiColorCmd->h);
                gle::attribub(rectMultiColorCmd->right.r, rectMultiColorCmd->right.g, rectMultiColorCmd->right.b, rectMultiColorCmd->right.a);

                gle::attribf(rectMultiColorCmd->x             , rectMultiColorCmd->y + rectMultiColorCmd->h);
                gle::attribub(rectMultiColorCmd->bottom.r, rectMultiColorCmd->bottom.g, rectMultiColorCmd->bottom.b, rectMultiColorCmd->bottom.a);

                xtraverts += gle::end();
            } break;
            case NK_COMMAND_ARC_FILLED: {
                conoutf(CON_WARN, "%s", "Unknown NK_COMMAND_ARC_FILLED draw command detected.");
            } break;
            default: break;
        }
    }

    nk_clear(&m_Context);
}

void NkPolyhedron::InputProcessBegin()
{
    nk_input_begin(&m_Context);
}

void NkPolyhedron::InputProcessEnd()
{
    nk_input_end(&m_Context);
}

NkPolyhedron::InputEventProcessState NkPolyhedron::InputEvent(const SDL_Event &evt)
{
    if (evt.type == SDL_KEYUP || evt.type == SDL_KEYDOWN)
    {
        /* key events */
        int down = evt.type == SDL_KEYDOWN;
        const Uint8* state = SDL_GetKeyboardState(0);
        SDL_Keycode sym = evt.key.keysym.sym;
        if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
            nk_input_key(&m_Context, NK_KEY_SHIFT, down);
        else if (sym == SDLK_DELETE)
            nk_input_key(&m_Context, NK_KEY_DEL, down);
        else if (sym == SDLK_RETURN)
            nk_input_key(&m_Context, NK_KEY_ENTER, down);
        else if (sym == SDLK_TAB)
            nk_input_key(&m_Context, NK_KEY_TAB, down);
        else if (sym == SDLK_BACKSPACE)
            nk_input_key(&m_Context, NK_KEY_BACKSPACE, down);
        else if (sym == SDLK_HOME)
        {
            nk_input_key(&m_Context, NK_KEY_TEXT_START, down);
            nk_input_key(&m_Context, NK_KEY_SCROLL_START, down);
        }
        else if (sym == SDLK_END)
        {
            nk_input_key(&m_Context, NK_KEY_TEXT_END, down);
            nk_input_key(&m_Context, NK_KEY_SCROLL_END, down);
        }
        else if (sym == SDLK_PAGEDOWN)
        {
            nk_input_key(&m_Context, NK_KEY_SCROLL_DOWN, down);
        }
        else if (sym == SDLK_PAGEUP)
        {
            nk_input_key(&m_Context, NK_KEY_SCROLL_UP, down);
        }
        else if (sym == SDLK_z)
            nk_input_key(&m_Context, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_r)
            nk_input_key(&m_Context, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_c)
            nk_input_key(&m_Context, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_v)
            nk_input_key(&m_Context, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_x)
            nk_input_key(&m_Context, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_b)
            nk_input_key(&m_Context, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_e)
            nk_input_key(&m_Context, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_UP)
            nk_input_key(&m_Context, NK_KEY_UP, down);
        else if (sym == SDLK_DOWN)
            nk_input_key(&m_Context, NK_KEY_DOWN, down);
        else if (sym == SDLK_LEFT)
        {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(&m_Context, NK_KEY_TEXT_WORD_LEFT, down);
            else nk_input_key(&m_Context, NK_KEY_LEFT, down);
        }
        else if (sym == SDLK_RIGHT)
        {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(&m_Context, NK_KEY_TEXT_WORD_RIGHT, down);
            else nk_input_key(&m_Context, NK_KEY_RIGHT, down);
        }
        else
        {
            return InputEventProcessState::NotHandled;
        }
        return nk_item_is_any_active(&m_Context) ? InputEventProcessState::Handled : InputEventProcessState::NotHandled;
    }
    else if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP)
    {
        /* mouse button */
        int down = evt.type == SDL_MOUSEBUTTONDOWN;
        const int x = evt.button.x, y = evt.button.y;
        if (evt.button.button == SDL_BUTTON_LEFT)
        {
            if (evt.button.clicks > 1)
                nk_input_button(&m_Context, NK_BUTTON_DOUBLE, x, y, down);
            nk_input_button(&m_Context, NK_BUTTON_LEFT, x, y, down);
        }
        else if (evt.button.button == SDL_BUTTON_MIDDLE)
            nk_input_button(&m_Context, NK_BUTTON_MIDDLE, x, y, down);
        else if (evt.button.button == SDL_BUTTON_RIGHT)
            nk_input_button(&m_Context, NK_BUTTON_RIGHT, x, y, down);
        else return InputEventProcessState::NotHandled;

        return nk_window_is_any_hovered(&m_Context) ? InputEventProcessState::Handled : InputEventProcessState::NotHandled;
    }
    else if (evt.type == SDL_MOUSEMOTION)
    {
        /* mouse motion */
        if (m_Context.input.mouse.grabbed)
        {
            int x = (int)m_Context.input.mouse.prev.x, y = (int)m_Context.input.mouse.prev.y;
            nk_input_motion(&m_Context, x + evt.motion.xrel, y + evt.motion.yrel);
        }
        else
        {
            nk_input_motion(&m_Context, evt.motion.x, evt.motion.y);
        }

        return nk_window_is_any_hovered(&m_Context) ? InputEventProcessState::Handled : InputEventProcessState::NotHandled;
    }
    else if (evt.type == SDL_TEXTINPUT)
    {
        /* text input */
        nk_glyph glyph;
        memcpy(glyph, evt.text.text, NK_UTF_SIZE);
        nk_input_glyph(&m_Context, glyph);
        return nk_item_is_any_active(&m_Context) ? InputEventProcessState::Handled : InputEventProcessState::NotHandled;;
    }
    else if (evt.type == SDL_MOUSEWHEEL)
    {
        /* mouse wheel */
        nk_input_scroll(&m_Context, nk_vec2((float)evt.wheel.x,(float)evt.wheel.y));
        return nk_item_is_any_active(&m_Context) ? InputEventProcessState::Handled : InputEventProcessState::NotHandled;;
    }

    return InputEventProcessState::NotHandled;
}

nk_context* NkPolyhedron::GetContext()
{
    return &m_Context;
}

float NkPolyhedron::GetLineHeight()
{
    auto userFont = m_DefaultFont->m_UserFont;
    return userFont.height;
//    auto fontObject = findfont(m_DefaultFont->m_FontName.c_str());
//    auto fontScale = float(fontObject->scale);
//    return float(userFont.height) / fontScale;
}
