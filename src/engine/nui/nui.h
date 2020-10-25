#pragma once
#include "nuklear_polyhedron.h"

struct nk_context;
class Entity;

namespace engine {
	namespace nui {
        using InputEventProcessState = NkPolyhedron::InputEventProcessState;

		void Initialize();

		void Render();
		void Update();

		void InputProcessBegin();
        InputEventProcessState InputEvent(const SDL_Event &evt);
		void InputProcessEnd();

		void Destroy();

		nk_context* GetNKContext();
		NkPolyhedron& GetDevice();
	}
}
