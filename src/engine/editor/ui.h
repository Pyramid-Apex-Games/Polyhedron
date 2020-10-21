#pragma once
#include <SDL_events.h>

class Entity;

namespace EditorUI
{
    enum class InputEventProcessState {
        Handled,
        NotHandled
    };

    void Initialize();
    void Update();
    void Render();
    void Reset();

    void StartEntityEditor(size_t entityId);
    void StopEntityEditor(size_t entityId);

    void InputProcessBegin();
    InputEventProcessState InputEvent(const SDL_Event &evt);
    void InputProcessEnd();

    void Destroy();
}