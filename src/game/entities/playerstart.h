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

            void reset();

			float yaw = 0.0f;

        private:
        };
    } // classes
} // entities

