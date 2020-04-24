#pragma once
#include "shared/entities/baseentity.h"

namespace entities {
    namespace classes {
        class PlayerStart : public BaseEntity {
			ENTITY_FACTORY_IMPL(PlayerStart);
        public:
            PlayerStart();

            void preload();
            void think();
            void render();

            void reset();

			float yaw = 180.0f;

        private:
        };
    } // classes
} // entities

