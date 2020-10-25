#pragma once
#include "Entity.h"

class DecalEntity : public Entity
{
    ENTITY_FACTORY_IMPL(DecalEntity);
    friend struct verthash;
    friend struct vacollect;
public:

    virtual bool getBoundingBox(int entselradius, vec &minbb, vec &maxbb) const;

public:
    int m_DecalSlot = 0;
    float m_Size = 1.0f;
};

