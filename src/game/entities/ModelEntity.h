#pragma once
#include "shared/entities/MovableEntity.h"
#include "engine/engine.h"
#include "engine/rendermodel.h"


class ModelEntity : public MovableEntity {
    ENTITY_FACTORY_IMPL(ModelEntity);
public:
    ModelEntity() = default;
    ModelEntity(const std::string &filename);

    virtual void preload();
    virtual void think();

    virtual void onAnimate(int &anim, int &basetime);

    virtual bool getBoundingBox(int entselradius, vec &minbb, vec &maxbb) const;
public:
    void preloadMapModel(const std::string &filename);
    const std::string& getModelName() const;
    model* getModel() const;

private:
    void loadModelAttributes();

private:
    DONTSERIALIZE mapmodelinfo mmi;
    PHUI_INPUT("Animation id") int animation = 5;
    PHUI_INPUT("Size") float size = 1.0f;
    DONTSERIALIZE vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    PHUI_INPUT("Model") std::string modelname;
public:
    DONTSERIALIZE int model_idx = 0;
};
