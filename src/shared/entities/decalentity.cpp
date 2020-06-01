
#include "decalentity.h"
#include "engine/texture.h"
#include "engine/engine.h"
#include "engine/physics.h"

namespace entities {
namespace classes {

bool DecalEntity::getBoundingBox(int entselradius, vec &minbb, vec &maxbb) const
{
	if (CoreEntity::getBoundingBox(entselradius, minbb, maxbb))
	{
		DecalSlot &s = lookupdecalslot(attr1, false);
		vec center, radius;
		
		float size = max(float(attr5), 1.0f);
		center = vec(0, s.depth * size/2, 0);
		radius = vec(size/2, s.depth * size/2, size/2);
		rotatebb(center, radius, attr2, attr3, attr4);
		
		center.add(o);
		radius.max(entselradius);
		minbb = vec(center).sub(radius);
		maxbb = vec(center).add(radius).add(1);
		return true;
	}
	return false;
}

void DecalEntity::on(const Event& event)
{
}

void DecalEntity::render(game::RenderPass pass)
{
}

} // classes
} // entities

ADD_ENTITY_TO_FACTORY_SERIALIZED(DecalEntity, "decal_entity", CoreEntity);

