#pragma once
#include "nuklear_polyhedron.h"

struct nk_context;

namespace engine {
	namespace nui {
        using InputEventProcessState = NkPolyhedron::InputEventProcessState;

		void Initialize();

		void Render();

		void InputProcessBegin();
        InputEventProcessState InputEvent(const SDL_Event &evt);
		void InputProcessEnd();

		void Destroy();

		nk_context *GetNKContext();

		void DemoUI();
	}
}
