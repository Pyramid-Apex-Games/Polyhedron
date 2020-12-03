#include "engine/engine.h"
#include "game/game.h"
#include "engine/world.h"
#include "engine/model.h"
#include "engine/animmodel.h"

#include "ModelEntity.h"
#include "shared/entities/EntityFactory.h"

//namespace {
//	static inline void transformbb(const Entity *e, vec &center, vec &radius)
//	{
//		if(e->attr5 > 0) { float scale = e->attr5/100.0f; center.mul(scale); radius.mul(scale); }
//		rotatebb(center, radius, e->attr2, e->attr3, e->attr4);
//	}
//
//	static inline void mmboundbox(const Entity *e, model *m, vec &center, vec &radius)
//	{
//		m->boundbox(center, radius);
//		transformbb(e, center, radius);
//	}
//}

//extern inline void transformbb(const Entity *e, vec &center, vec &radius);
//extern inline void mmboundbox(const Entity *e, model *m, vec &center, vec &radius);

ModelEntity::ModelEntity(const std::string &modelname)
    : ModelEntity()
{
    this->modelname = modelname;
	preloadMapModel(modelname);
}


void ModelEntity::preload()
{
	preloadMapModel(modelname);
}

void ModelEntity::think()
{

}

void ModelEntity::render(RenderPass pass)
{
	if (pass == RenderPass::Main)
	{
		rendermodel(modelname.c_str(), animation, o, d.x, d.y, d.z, MDL_NOBATCH/* MDL_NOBATCH | MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED*/, this, nullptr, curtime, curtime, size, color);
	}
}

extern void mmcollisionbox(const Entity *e, model *m, vec &center, vec &radius);

bool ModelEntity::getBoundingBox(int entselradius, vec &minbb, vec &maxbb) const
{
    if(mmi.m)
    {
        vec center, radius;
        mmcollisionbox(this, mmi.m, center, radius);
        radius.max(entselradius);
        center.add(o);

        minbb = center;
        maxbb = center;
        minbb.sub(radius);
        maxbb.add(radius);

        return true;
    }

    return MovableEntity::getBoundingBox(entselradius, minbb, maxbb);
}

void ModelEntity::onAnimate(int &anim, int &basetime) {
//    conoutf("OnAnimate: %i %i", anim, basetime);
}

void ModelEntity::preloadMapModel(const std::string &modelname)
{
	if (model_idx >= 0)
	{
		// In case this is the first time, a filename will be supplied for sure.
		if (!modelname.empty())
		{
			mmi = mapmodels.emplace_back();
			auto [modelPtr, loadedModelName] = loadmodel(modelname, -1, true);
			if (modelPtr)
			{
			    mmi.m = modelPtr;
			    auto [colPtr, loadedColName] = loadmodel(modelPtr->collidemodel, -1, true);
				mmi.collide = colPtr;
				mmi.name = loadedModelName;

				model_idx = mapmodels.size() - 1;

				if (auto animModelPtr = dynamic_cast<animmodel*>(modelPtr); animModelPtr)
                {
				    //find the animation names
				    for (int i = 0; i < animModelPtr->parts.size(); ++i)
                    {

                    }
                }
			}
			else
			{
			    mapmodels.pop_back();
				conoutf("BaseMapModel::preloadMapModel: couldn't load model: %s\n", modelname.c_str());
			}
		}
		else
		{
			preloadMapModel("world/box");
		}
	}
}


const std::string& ModelEntity::getModelName() const
{
	return modelname;
}

void ModelEntity::On(const Event& event)
{
	switch(event.type)
	{
		case EventType::AttributeChanged:
		{
			const EntityEventAttributeChanged& attrChangeEvent = static_cast<const EntityEventAttributeChanged&>(event);
			
			if (attrChangeEvent.payload == "modelname")
			{
				preload();
			}
		} break;
		case EventType::HoverStart:
		break;
		case EventType::HoverStop:
		break;
		case EventType::SelectStart:
		break;
		case EventType::SelectStop:
		break;
		case EventType::TouchStart:
		break;
		case EventType::TouchStop:
		break;
		case EventType::Tick:
		break;
		case EventType::Use:
		break;
		case EventType::Trigger:
		break;
		case EventType::Precache:
			preload();
		break;

		default:
		case EventType::None:
		case EventType::Count:
		break;
	}
}

model *ModelEntity::getModel() const
{
    if (mmi.m)
    {
        return mmi.m;
    }

    auto [model, name] = loadmodel(getModelName(), -1, true);
    return model;
};

ADD_ENTITY_TO_FACTORY_SERIALIZED(ModelEntity, "model", MovableEntity)


