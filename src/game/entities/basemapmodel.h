#pragma once
#include "shared/entities/basedynamicentity.h"
#include "engine/engine.h"
#include "engine/rendermodel.h"

namespace entities
{
    namespace classes
    {
        class BaseMapModel : public BaseDynamicEntity {
			ENTITY_FACTORY_IMPL(BaseMapModel);
        public:
            BaseMapModel();
            BaseMapModel(const std::string &filename);

            virtual void preload();
            virtual void think();

            virtual void onAnimate(int &anim, int &basetime);

            virtual bool getBoundingBox(int entselradius, vec &minbb, vec &maxbb) const;
        public:
            void preloadMapModel(const std::string &filename);
            const std::string& getModelName() const;

        private:
            void loadModelAttributes();

        private:
            DONTSERIALIZE mapmodelinfo mmi;
            PHUI_INPUT("Animation id") int animation = 0;
            PHUI_INPUT("Size") float size = 1.0f;
            DONTSERIALIZE vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
            PHUI_INPUT("Model") std::string modelname;
        };
		
    } // classes
} // entities
