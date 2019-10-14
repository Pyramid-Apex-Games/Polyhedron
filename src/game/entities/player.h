#pragma once

#include "shared/entities/basedynamicentity.h"

struct vec;

namespace entities {
    namespace classes {
        class Player : public BaseDynamicEntity {
            ENTITY_FACTORY_IMPL(Player)
        public:
			//
			// Constructors/Destructor.
			//
            Player();
            virtual ~Player();

			//
			// Base/Core entity functions.
			//
            void preload();
            void think();
            void render();

			//
			// Entity functions.
			//
            void reset();
            void respawn();

			//
			// onEvent functions.
			//
            bool onTrigger(entities::classes::CoreEntity *otherEnt, const vec &dir);
            bool onTouch(entities::classes::CoreEntity *otherEnt, const vec &dir);

			//
			// Entity member variables.
			//
            entities::classes::BasePhysicalEntity *camera;
			
        private:

        };
    } // classes
} // entities

