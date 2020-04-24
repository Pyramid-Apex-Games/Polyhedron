#pragma once
#include "shared/entities/basephysicalentity.h"

namespace entities {
    namespace classes {
        class DynamicLight : public BasePhysicalEntity {
			ENTITY_FACTORY_IMPL(DynamicLight);
        public:
            DynamicLight();

            void preload();
            void think();
            void render();

            enum DYNAMIC_LIGHT_STATE {
                ON,
                OFF,
            };
            
            enum DYNAMIC_LIGHT_MODE {
                // Simple.
                FLICKERING,
                FADING,

                // Slow.
                SLOW_FADE,
                SLOW_FLICKER,

                // Fast.
                FAST_FADE,
                FAST_FLICKER,
            };

			NLOHMANN_JSON_SERIALIZE_ENUM( DYNAMIC_LIGHT_STATE, {
                // Is it on?
				{ON, "ON"},
				{OFF, "OFF"},
			});

			NLOHMANN_JSON_SERIALIZE_ENUM( DYNAMIC_LIGHT_MODE, {
                // Simple.
				{FLICKERING, "FLICKERING"},
				{FADING, "FADING"},

                // Slow.
                {SLOW_FADE, "SLOW_FADE"},
                {SLOW_FLICKER, "SLOW_FLICKER"},

                // Fast.
                {FAST_FADE, "FAST_FADE"},
                {FAST_FLICKER, "FAST_FLICKER"},
			});

            // Set its state.
            void setState(DynamicLight::DYNAMIC_LIGHT_STATE &_lightState);

            // Set its mode.
            void setMode(DynamicLight::DYNAMIC_LIGHT_MODE &_lightMode);
        public:
            //
            // Light states.
            //
            // Stores the current state of the dynamic light. (On/Off.. Or??)
            DynamicLight::DYNAMIC_LIGHT_STATE lightState;

            // Stores the current mode state of the light. (Flickering etc.)
            DynamicLight::DYNAMIC_LIGHT_MODE lightMode;

            // Obviously speaks for itself, the color.
            vec4 lightColor;
			
        };
    } // classes
} // entities	
