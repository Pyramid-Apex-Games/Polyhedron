#include "DynamicEntity.h"


vec DynamicEntity::feetpos(float offset = 0) const {
    return vec(o).addz(offset - eyeheight);
}
vec DynamicEntity::headpos(float offset = 0) const {
    return vec(o).addz(offset);
}

bool DynamicEntity::crouched() const {
    return fabs(eyeheight - maxheight*CROUCHHEIGHT) < 1e-4f;
}

void DynamicEntity::on(const Event& event)
{
}

void DynamicEntity::render(game::RenderPass pass)
{
}


ADD_ENTITY_TO_FACTORY_SERIALIZED(DynamicEntity, "dynamic", Entity);
