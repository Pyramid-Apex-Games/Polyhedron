#include "nui.h"
#include "shared/entities/EntityEditorMenu.h"
#include <cassert>

//FIXME: use some kind of gamestate/editorstate controller for this
extern bool editmode;

namespace engine { namespace nui {

    std::unique_ptr<NkPolyhedron> NuklearPolyhedronDevice;

    void Initialize()
    {
        NuklearPolyhedronDevice.reset(new NkPolyhedron);
    }

    void Render()
    {
        NuklearPolyhedronDevice->Render();
    }

    void Update()
    {
    }

    void InputProcessBegin()
    {
        NuklearPolyhedronDevice->InputProcessBegin();
    }

    InputEventProcessState InputEvent(const SDL_Event &evt)
    {
        if (NuklearPolyhedronDevice->InputEvent(evt) == InputEventProcessState::Handled)
        {
            //filter events, ignore NO_INPUT windows
            auto iter = NuklearPolyhedronDevice->GetContext()->begin;
            auto* input = &NuklearPolyhedronDevice->GetContext()->input;
            while (iter)
            {
                /* check if window is being hovered */
                if(!(iter->flags & NK_WINDOW_NO_INPUT) && !(iter->flags & NK_WINDOW_HIDDEN))
                {
                    /* check if window popup is being hovered */
                    if (
                        iter->popup.active &&
                        iter->popup.win &&
                        nk_input_is_mouse_hovering_rect(input, iter->popup.win->bounds)
                    )
                        return InputEventProcessState::Handled;

                    if (iter->flags & NK_WINDOW_MINIMIZED)
                    {
                        struct nk_rect header = iter->bounds;
                        header.h = NuklearPolyhedronDevice->GetContext()->style.font->height +
                                2 * NuklearPolyhedronDevice->GetContext()->style.window.header.padding.y;
                        if (nk_input_is_mouse_hovering_rect(input, header))
                            return InputEventProcessState::Handled;
                    }
                    else if (nk_input_is_mouse_hovering_rect(input, iter->bounds))
                    {
                        return InputEventProcessState::Handled;
                    }
                }
                iter = iter->next;
            }
        }

        return InputEventProcessState::NotHandled;
    }
    void InputProcessEnd()
    {
        NuklearPolyhedronDevice->InputProcessEnd();
    }

    void Destroy()
    {
        NuklearPolyhedronDevice.reset();
    }

    nk_context *GetNKContext()
    {
        return NuklearPolyhedronDevice->GetContext();
    }

    NkPolyhedron& GetDevice()
    {
        assert(NuklearPolyhedronDevice && "NuklearPolyhedronDevice destroyed or not initialized!");

        return *NuklearPolyhedronDevice.get();
    }

}
}