#include "DecalEntity.h"
#include "engine/texture.h"
#include "engine/engine.h"
#include "engine/physics.h"

bool DecalEntity::getBoundingBox(int entselradius, vec &minbb, vec &maxbb) const
{
	if (Entity::getBoundingBox(entselradius, minbb, maxbb))
	{
		DecalSlot &s = lookupdecalslot(m_DecalSlot, false);
		vec center, radius;
		
		float size = max(m_Size, 1.0f);
		center = vec(0, s.depth * size/2, 0);
		radius = vec(size/2, s.depth * size/2, size/2);
		rotatebb(center, radius, d.x, d.y, d.z);
		
		center.add(o);
		radius.max(entselradius);
		minbb = vec(center).sub(radius);
		maxbb = vec(center).add(radius).add(1);
		return true;
	}
	return false;
}

void DecalEntity::On(const Event& event)
{
}



ADD_ENTITY_TO_FACTORY_SERIALIZED(DecalEntity, "decal", Entity);

