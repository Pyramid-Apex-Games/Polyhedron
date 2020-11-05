#include <fmt/format.h>
#include "shared/cube.h"
#include "shared/ents.h"
#include "shared/entities/MovableEntity.h"
#include "engine/pvs.h"
#include "engine/rendergl.h"
#include "engine/renderlights.h"
#include "engine/aa.h"
#include "engine/renderva.h"
#include "engine/main/Compatibility.h"
#include "engine/main/Renderer.h"
#include "engine/Camera.h"
#include "game/entities/ModelEntity.h"

//extern from command.h
extern int identflags;

VAR(oqdynent, 0, 1, 1);
VAR(animationinterpolationtime, 0, 200, 1000);

model *loadingmodel = NULL;

#include "ragdoll.h"
#include "animmodel.h"
#include "vertmodel.h"
#include "skelmodel.h"
#include "hitzone.h"

static model *(__cdecl *modeltypes[NUMMODELTYPES])(const char *);

static int addmodeltype(int type, model *(__cdecl *loader)(const char *))
{
	modeltypes[type] = loader;
	return type;
}

#define MODELTYPE(modeltype, modelclass) \
static model *__loadmodel__##modelclass(const char *filename) \
{ \
	return new modelclass(filename); \
} \
UNUSED static int __dummy__##modelclass = addmodeltype((modeltype), __loadmodel__##modelclass);

#include "md2.h"
#include "md3.h"
#include "md5.h"
#include "obj.h"
#include "smd.h"
#include "iqm.h"

MODELTYPE(MDL_MD2, md2);
MODELTYPE(MDL_MD3, md3);
MODELTYPE(MDL_MD5, md5);
MODELTYPE(MDL_OBJ, obj);
MODELTYPE(MDL_SMD, smd);
MODELTYPE(MDL_IQM, iqm);

#define checkmdl if(!loadingmodel) { conoutf(CON_ERROR, "not loading a model"); return; }

SCRIPTEXPORT void mdlcullface(int *cullface)
{
	checkmdl;
	loadingmodel->setcullface(*cullface);
}

SCRIPTEXPORT void mdlcolor(float *r, float *g, float *b)
{
	checkmdl;
	loadingmodel->setcolor(vec(*r, *g, *b));
}

SCRIPTEXPORT void mdlcollide(int *collide)
{
	checkmdl;
	loadingmodel->collide = *collide!=0 ? (loadingmodel->collide ? loadingmodel->collide : COLLIDE_OBB) : COLLIDE_NONE;
}

SCRIPTEXPORT void mdlellipsecollide(int *collide)
{
	checkmdl;
	loadingmodel->collide = *collide!=0 ? COLLIDE_ELLIPSE : COLLIDE_NONE;
}

SCRIPTEXPORT void mdltricollide(char *collide)
{
	checkmdl;
	char *end = NULL;
	int val = strtol(collide, &end, 0);
	if(*end) { val = 1; loadingmodel->collidemodel = collide; }
	loadingmodel->collide = val ? COLLIDE_TRI : COLLIDE_NONE;
}

SCRIPTEXPORT void mdlspec(float *percent)
{
	checkmdl;
	float spec = *percent > 0 ? *percent/100.0f : 0.0f;
	loadingmodel->setspec(spec);
}

SCRIPTEXPORT void mdlgloss(int *gloss)
{
	checkmdl;
	loadingmodel->setgloss(clamp(*gloss, 0, 2));
}

SCRIPTEXPORT void mdlalphatest(float *cutoff)
{
	checkmdl;
	loadingmodel->setalphatest(max(0.0f, min(1.0f, *cutoff)));
}

SCRIPTEXPORT void mdldepthoffset(int *offset)
{
	checkmdl;
	loadingmodel->depthoffset = *offset!=0;
}

SCRIPTEXPORT void mdlglow(float *percent, float *delta, float *pulse)
{
	checkmdl;
	float glow = *percent > 0 ? *percent/100.0f : 0.0f, glowdelta = *delta/100.0f, glowpulse = *pulse > 0 ? *pulse/1000.0f : 0;
	glowdelta -= glow;
	loadingmodel->setglow(glow, glowdelta, glowpulse);
}

SCRIPTEXPORT void mdlenvmap(float *envmapmax, float *envmapmin, char *envmap)
{
	checkmdl;
	loadingmodel->setenvmap(*envmapmin, *envmapmax, envmap[0] ? cubemapload(envmap) : NULL);
}

SCRIPTEXPORT void mdlfullbright(float *fullbright)
{
	checkmdl;
	loadingmodel->setfullbright(*fullbright);
}

SCRIPTEXPORT void mdlshader(char *shader)
{
	checkmdl;
	loadingmodel->setshader(lookupshaderbyname(shader));
}

SCRIPTEXPORT void mdlspin(float *yaw, float *pitch, float *roll)
{
	checkmdl;
	loadingmodel->spinyaw = *yaw;
	loadingmodel->spinpitch = *pitch;
	loadingmodel->spinroll = *roll;
}

SCRIPTEXPORT void mdlscale(float *percent)
{
	checkmdl;
	float scale = *percent > 0 ? *percent/100.0f : 1.0f;
	loadingmodel->scale = scale;
}

SCRIPTEXPORT void mdltrans(float *x, float *y, float *z)
{
	checkmdl;
	loadingmodel->translate = vec(*x, *y, *z);
}

SCRIPTEXPORT void mdlyaw(float *angle)
{
	checkmdl;
	loadingmodel->offsetyaw = *angle;
}

SCRIPTEXPORT void mdlpitch(float *angle)
{
	checkmdl;
	loadingmodel->offsetpitch = *angle;
}

SCRIPTEXPORT void mdlroll(float *angle)
{
	checkmdl;
	loadingmodel->offsetroll = *angle;
}

SCRIPTEXPORT void mdlshadow(int *shadow)
{
	checkmdl;
	loadingmodel->shadow = *shadow!=0;
}

SCRIPTEXPORT void mdlalphashadow(int *alphashadow)
{
	checkmdl;
	loadingmodel->alphashadow = *alphashadow!=0;
}

SCRIPTEXPORT void mdlbb(float *rad, float *h, float *eyeheight)
{
	checkmdl;
	loadingmodel->collidexyradius = *rad;
	loadingmodel->collideheight = *h;
	loadingmodel->eyeheight = *eyeheight;
}

SCRIPTEXPORT void mdlextendbb(float *x, float *y, float *z)
{
	checkmdl;
	loadingmodel->bbextend = vec(*x, *y, *z);
}

SCRIPTEXPORT void mdlname()
{
	checkmdl;
	result(loadingmodel->name.c_str());
}

#define checkragdoll \
	checkmdl; \
	if(!loadingmodel->skeletal()) { conoutf(CON_ERROR, "not loading a skeletal model"); return; } \
	skelmodel *m = (skelmodel *)loadingmodel; \
	if(m->parts.empty()) return; \
	skelmodel::skelmeshgroup *meshes = (skelmodel::skelmeshgroup *)m->parts.back()->meshes; \
	if(!meshes) return; \
	skelmodel::skeleton *skel = meshes->skel; \
	if(!skel->ragdoll) skel->ragdoll = new ragdollskel; \
	ragdollskel *ragdoll = skel->ragdoll; \
	if(ragdoll->loaded) return;


SCRIPTEXPORT void rdvert(float *x, float *y, float *z, float *radius)
{
	checkragdoll;
	ragdollskel::vert &v = ragdoll->verts.emplace_back();
	v.pos = vec(*x, *y, *z);
	v.radius = *radius > 0 ? *radius : 1;
}

SCRIPTEXPORT void rdeye(int *v)
{
	checkragdoll;
	ragdoll->eye = *v;
}

SCRIPTEXPORT void rdtri(int *v1, int *v2, int *v3)
{
	checkragdoll;
	ragdollskel::tri &t = ragdoll->tris.emplace_back();
	t.vert[0] = *v1;
	t.vert[1] = *v2;
	t.vert[2] = *v3;
}

SCRIPTEXPORT void rdjoint(int *n, int *t, int *v1, int *v2, int *v3)
{
	checkragdoll;
	if(*n < 0 || *n >= skel->numbones) return;
	ragdollskel::joint &j = ragdoll->joints.emplace_back();
	j.bone = *n;
	j.tri = *t;
	j.vert[0] = *v1;
	j.vert[1] = *v2;
	j.vert[2] = *v3;
}

SCRIPTEXPORT void rdlimitdist(int *v1, int *v2, float *mindist, float *maxdist)
{
	checkragdoll;
	ragdollskel::distlimit &d = ragdoll->distlimits.emplace_back();
	d.vert[0] = *v1;
	d.vert[1] = *v2;
	d.mindist = *mindist;
	d.maxdist = max(*maxdist, *mindist);
}

SCRIPTEXPORT void rdlimitrot(int *t1, int *t2, float *maxangle, float *qx, float *qy, float *qz, float *qw)
{
	checkragdoll;
	ragdollskel::rotlimit &r = ragdoll->rotlimits.emplace_back();
	r.tri[0] = *t1;
	r.tri[1] = *t2;
	r.maxangle = *maxangle * RAD;
	r.maxtrace = 1 + 2*cos(r.maxangle);
	r.middle = matrix3(quat(*qx, *qy, *qz, *qw));
}

SCRIPTEXPORT void rdanimjoints(int *on)
{
	checkragdoll;
	ragdoll->animjoints = *on!=0;
}

// mapmodels
std::vector<mapmodelinfo> mapmodels;

SCRIPTEXPORT void mapmodel(char *name)
{
	// Returns a reference to the added mapmodel info in the list.
	auto path = std::string("model/") + name;
	if (fileexists(path.c_str(), "a"))
	{
		mapmodelinfo &mmi = loadmodelinfo(path.c_str());
		
		// Setup the name.
		if(name[0])
		{
			mmi.name = name;
		}
		else
		{
			mmi.name = "";
		}

		// Set all to NULL.
		mmi.m = mmi.collide = nullptr;
		conoutf(CON_INFO, "mapmodel prepared: %s", name);
	}
	else
	{
		conoutf(CON_WARN, "No such mapmodel: %s", name);
	}
}

SCRIPTEXPORT void mmodel(char *name)
{
    mapmodel(name);
}

SCRIPTEXPORT void mapmodelreset(int *n)
{
	if(!(identflags&IDF_OVERRIDDEN) && !game::allowedittoggle())
	    return;
	for(int i = clamp(*n, 0, (int)mapmodels.size()); i >= 0 && !mapmodels.empty(); --i)
    {
        mapmodels.pop_back();
    }
}

std::string mapmodelname(int i)
{
    return (mapmodels.size() > i && i >= 0) ? mapmodels[i].name : "";
}

SCRIPTEXPORT_AS(mapmodelname) void mapmodelname_scriptimpl(int *index, int *prefix)
{
    if(mapmodels.size() > *index && *index >= 0)
    {
        result(!mapmodels[*index].name.empty() ? mapmodels[*index].name.c_str() : "");
    }
}

SCRIPTEXPORT void nummapmodels()
{
    intret(mapmodels.size());
}

//static inlines from rendermodel.h
model *loadmapmodel(ModelEntity* entity)
{
    return entity->getModel();
}

model *loadmapmodel(int n)
{
    if(mapmodels.size() > n && n >= 0)
    {
        model *m = mapmodels[n].m;
        return m ? m : getmodel(n);
    }
    return NULL;
}

model *loadmapmodel(const char *filename)
{
    for(auto& mm : mapmodels)
    {
        // Compare if the mapmodel's values equal each other.
        model *m = mm.m;

        if (m->name == filename)
        {
            // If they equal each other, it means we don't have to load it in again. Just return the pointer.
            if (!m)
            {
                auto [model, name] = loadmodel(filename);
                return model;
            }
            return m;
        }
        else
            return nullptr;
    }
    return nullptr;
}


mapmodelinfo *getmminfo(int n)
{
    return (mapmodels.size() > n && n >= 0) ? &mapmodels[n] : NULL;
}
// model registry

hashnameset<model *> models;
vector<std::string> preloadmodels;
hashset<char*> failedmodels;

void preloadmodel(const char *name)
{
	if(!name || !name[0] || models.access(name)) return;
	auto itr = std::find(preloadmodels.begin(), preloadmodels.end(), name);
	if (itr == preloadmodels.end()) return;
    preloadmodels.emplace_back(name);

}

void flushpreloadedmodels(bool msg)
{
	loopv(preloadmodels)
	{
		loadprogress = float(i+1)/preloadmodels.size();
		auto [m, name] = loadmodel(preloadmodels[i], -1, msg);
		if(!m)
		{
		    if(msg)
            {
		        conoutf(CON_WARN, "could not load model: %s", preloadmodels[i].c_str());
            }
		}
		else
		{
			m->preloadmeshes();
			m->preloadshaders();
		}
	}
	preloadmodels.clear();
	loadprogress = 0;
}

void preloadusedmapmodels(bool msg, bool bih)
{
	auto &ents = getents();
	vector<int> used;
	loopv(ents)
	{
		auto e = dynamic_cast<ModelEntity*>(ents[i]);
		if(e && !in_list(e->model_idx, used)) used.emplace_back(e->model_idx);
	}

	vector<std::string> col;
	loopv(used)
	{
		loadprogress = float(i+1)/used.size();
		int mmindex = used[i];
		if(mapmodels.size() >= mmindex || mmindex <= 0) { if(msg) conoutf(CON_WARN, "could not find map model: %d", mmindex); continue; }
		mapmodelinfo &mmi = mapmodels[mmindex];
		if(!mmi.name[0]) continue;
		model *m = getmodel(mmindex, msg);
		if(!m) { if(msg) conoutf(CON_WARN, "could not load map model: %s", mmi.name.c_str()); }
		else
		{
			if(bih) m->preloadBIH();
			else if(m->collide == COLLIDE_TRI && m->collidemodel.empty() && m->bih) m->setBIH();
			m->preloadmeshes();
			m->preloadshaders();
			if(!m->collidemodel.empty())
            {
			    if (std::find(col.begin(), col.end(), m->collidemodel) == col.end())
                    col.emplace_back(m->collidemodel.c_str());
            }
		}
	}

	loopv(col)
	{
		loadprogress = float(i+1)/col.size();
		auto [m, n] = loadmodel(col[i], -1, msg);
		if(!m) { if(msg) conoutf(CON_WARN, "could not load collide model: %s", col[i].c_str()); }
		else if(!m->bih) m->setBIH();
	}

	loadprogress = 0;
}

model* getmodel(int i, bool msg)
{
    auto [model, name] = loadmodel("", i, msg);
    return model;
}

std::tuple<model *, std::string> loadmodel(const std::string& _name, int i, bool msg)
{
    std::string name = _name;
	model *m = nullptr;
	if(name.empty())
	{
		if(mapmodels.size() <= i || i < 0) return std::tie(m, name);
		mapmodelinfo &mmi = mapmodels[i];
		if(mmi.m) return std::tie(mmi.m, name);
		name = mmi.name;
	}
	model **mm = models.access(name.c_str());
	if(mm) m = *mm;
	else
	{
		if(
			name.empty() ||
			loadingmodel ||
			failedmodels.find(name.c_str(), NULL)
		)
			return std::tie(m, name);
		if(msg)
		{
		    auto filename = fmt::format("media/model/{}", name);
			renderprogress(loadprogress, filename.c_str());
		}
		loopi(NUMMODELTYPES)
		{
			m = modeltypes[i](name.c_str());
			if(!m)
            {
			    continue;
            }
			loadingmodel = m;
			if(m->load())
            {
			    break;
            }
			DELETEP(m);
		}
		loadingmodel = NULL;
		if(!m)
		{
			failedmodels.add(newcubestr(name.c_str()));
			return std::tie(m, name);
		}
		models.access(m->name.c_str(), m);
	}
	if(mapmodels.size() > i && i >= 0 && !mapmodels[i].m) mapmodels[i].m = m;
	return std::tie(m, name);
}

mapmodelinfo& loadmodelinfo(const char *name)
{
	// Preload first.
	preloadmodel(name);

	mapmodelinfo &mmi = mapmodels.emplace_back();

	// Load in the model.
	std::string tmp;
	std::tie(mmi.m, tmp) = loadmodel(name, -1, true);

	return mmi;
}

void clear_models()
{
	enumerate(models, model *, m, delete m);
}

void cleanupmodels()
{
	enumerate(models, model *, m, m->cleanup());
}

SCRIPTEXPORT void clearmodel(char *name)
{
	model *m = models.find(name, NULL);
	if(!m) { conoutf("model %s is not loaded", name); return; }
	for(mapmodelinfo &mmi : mapmodels)
	{
		if(mmi.m == m) mmi.m = NULL;
		if(mmi.collide == m) mmi.collide = NULL;
	}
	models.remove(name);
	m->cleanup();
	delete m;
	conoutf("cleared model %s", name);
}

bool modeloccluded(const vec &center, float radius)
{
	ivec bbmin(vec(center).sub(radius)), bbmax(vec(center).add(radius+1));
	return pvsoccluded(bbmin, bbmax) || bboccluded(bbmin, bbmax);
}

struct batchedmodel
{
	vec pos, center;
	float radius, yaw, pitch, roll, sizescale;
	vec4 colorscale;
	int anim, basetime, basetime2, flags, attached;
	union
	{
		int visible;
		int culled;
	};
	ModelEntity *d;
	int next;
};
struct modelbatch
{
	model *m;
	int flags, batched;
};
static vector<batchedmodel> batchedmodels;
static vector<modelbatch> batches;
static vector<modelattach> modelattached;

void resetmodelbatches()
{
    batchedmodels.resize(0);
    batches.resize(0);
    modelattached.resize(0);
}

void addbatchedmodel(model *m, batchedmodel &bm, int idx)
{
	modelbatch *b = NULL;
	if(in_range(m->batch, batches))
	{
		b = &batches[m->batch];
		if(b->m == m && (b->flags & MDL_MAPMODEL) == (bm.flags & MDL_MAPMODEL))
			goto foundbatch;
	}

	m->batch = batches.size();
	b = &batches.emplace_back();
	b->m = m;
	b->flags = 0;
	b->batched = -1;

foundbatch:
	b->flags |= bm.flags;
	bm.next = b->batched;
	b->batched = idx;
}

static inline void renderbatchedmodel(model *m, const batchedmodel &b)
{
	modelattach *a = NULL;
	if(b.attached>=0) a = &modelattached[b.attached];

	int anim = b.anim;
	if(shadowmapping > SM_REFLECT)
	{
		anim |= ANIM_NOSKIN;
	}
	else
	{
		if(b.flags&MDL_FULLBRIGHT) anim |= ANIM_FULLBRIGHT;
	}

	m->render(anim, b.basetime, b.basetime2, b.pos, b.yaw, b.pitch, b.roll, (MovableEntity*)b.d, a, b.sizescale, b.colorscale);
}

VAR(maxmodelradiusdistance, 10, 200, 1000);

static inline void enablecullmodelquery()
{
	startbb();
}

static inline void rendercullmodelquery(model *m, MovableEntity *d, const vec &center, float radius)
{
    assert(Camera::GetActiveCamera());
    const auto& o = Camera::GetActiveCamera()->o;
	if(fabs(o.x-center.x) < radius+1 &&
	   fabs(o.y-center.y) < radius+1 &&
	   fabs(o.z-center.z) < radius+1)
	{
		d->query = NULL;
		return;
	}
	d->query = newquery(d);
	if(!d->query) return;
	startquery(d->query);
	int br = int(radius*2)+1;
	drawbb(ivec(int(center.x-radius), int(center.y-radius), int(center.z-radius)), ivec(br, br, br));
	endquery(d->query);
}

static inline void disablecullmodelquery()
{
	endbb();
}

static inline int cullmodel(model *m, const vec &center, float radius, int flags, ModelEntity *d = NULL)
{
    assert(Camera::GetActiveCamera());
	if(flags&MDL_CULL_DIST && center.dist(Camera::GetActiveCamera()->o)/radius>maxmodelradiusdistance) return MDL_CULL_DIST;
	if(flags&MDL_CULL_VFC && isfoggedsphere(radius, center)) return MDL_CULL_VFC;
	if(flags&MDL_CULL_OCCLUDED && modeloccluded(center, radius)) return MDL_CULL_OCCLUDED;
	else if(flags&MDL_CULL_QUERY && d->query && d->query->owner==d && checkquery(d->query)) return MDL_CULL_QUERY;
	return 0;
}

static inline int shadowmaskmodel(const vec &center, float radius)
{
	switch(shadowmapping)
	{
		case SM_REFLECT:
			return calcspherersmsplits(center, radius);
		case SM_CUBEMAP:
		{
			vec scenter = vec(center).sub(shadoworigin);
			float sradius = radius + shadowradius;
			if(scenter.squaredlen() >= sradius*sradius) return 0;
			return calcspheresidemask(scenter, radius, shadowbias);
		}
		case SM_CASCADE:
			return calcspherecsmsplits(center, radius);
		case SM_SPOT:
		{
			vec scenter = vec(center).sub(shadoworigin);
			float sradius = radius + shadowradius;
			return scenter.squaredlen() < sradius*sradius && sphereinsidespot(shadowdir, shadowspot, scenter, radius) ? 1 : 0;
		}
	}
	return 0;
}

void shadowmaskbatchedmodels(bool dynshadow)
{
	loopv(batchedmodels)
	{
		batchedmodel &b = batchedmodels[i];
		if(b.flags&(MDL_MAPMODEL | MDL_NOSHADOW)) break;
		b.visible = dynshadow && (b.colorscale.a >= 1 || b.flags&(MDL_ONLYSHADOW | MDL_FORCESHADOW)) ? shadowmaskmodel(b.center, b.radius) : 0;
	}
}

int batcheddynamicmodels()
{
	int visible = 0;
	loopv(batchedmodels)
	{
		batchedmodel &b = batchedmodels[i];
		if(b.flags&MDL_MAPMODEL) break;
		visible |= b.visible;
	}
	loopv(batches)
	{
		modelbatch &b = batches[i];
		if(!(b.flags&MDL_MAPMODEL) || !b.m->animated()) continue;
		for(int j = b.batched; j >= 0;)
		{
			batchedmodel &bm = batchedmodels[j];
			j = bm.next;
			visible |= bm.visible;
		}
	}
	return visible;
}

int batcheddynamicmodelbounds(int mask, vec &bbmin, vec &bbmax)
{
	int vis = 0;
	loopv(batchedmodels)
	{
		batchedmodel &b = batchedmodels[i];
		if(b.flags&MDL_MAPMODEL) break;
		if(b.visible&mask)
		{
			bbmin.min(vec(b.center).sub(b.radius));
			bbmax.max(vec(b.center).add(b.radius));
			++vis;
		}
	}
	loopv(batches)
	{
		modelbatch &b = batches[i];
		if(!(b.flags&MDL_MAPMODEL) || !b.m->animated()) continue;
		for(int j = b.batched; j >= 0;)
		{
			batchedmodel &bm = batchedmodels[j];
			j = bm.next;
			if(bm.visible&mask)
			{
				bbmin.min(vec(bm.center).sub(bm.radius));
				bbmax.max(vec(bm.center).add(bm.radius));
				++vis;
			}
		}
	}
	return vis;
}

void rendershadowmodelbatches(bool dynmodel)
{
	loopv(batches)
	{
		modelbatch &b = batches[i];
		if(!b.m->shadow || (!dynmodel && (!(b.flags&MDL_MAPMODEL) || b.m->animated()))) continue;
		bool rendered = false;
		for(int j = b.batched; j >= 0;)
		{
			batchedmodel &bm = batchedmodels[j];
			j = bm.next;
			if(!(bm.visible&(1<<shadowside))) continue;
			if(!rendered) { b.m->startrender(); rendered = true; }
			renderbatchedmodel(b.m, bm);
		}
		if(rendered) b.m->endrender();
	}
}

void rendermapmodelbatches()
{
	enableaamask();
	loopv(batches)
	{
		modelbatch &b = batches[i];
		if(!(b.flags&MDL_MAPMODEL)) continue;
		b.m->startrender();
		setaamask(b.m->animated());
		for(int j = b.batched; j >= 0;)
		{
			batchedmodel &bm = batchedmodels[j];
			renderbatchedmodel(b.m, bm);
			j = bm.next;
		}
		b.m->endrender();
	}
	disableaamask();
}

float transmdlsx1 = -1, transmdlsy1 = -1, transmdlsx2 = 1, transmdlsy2 = 1;
uint transmdltiles[LIGHTTILE_MAXH];

void rendermodelbatches()
{
	transmdlsx1 = transmdlsy1 = 1;
	transmdlsx2 = transmdlsy2 = -1;
	memset(transmdltiles, 0, sizeof(transmdltiles));

	enableaamask();
	loopv(batches)
	{
		modelbatch &b = batches[i];
		if(b.flags&MDL_MAPMODEL) continue;
		bool rendered = false;
		for(int j = b.batched; j >= 0;)
		{
			batchedmodel &bm = batchedmodels[j];
			j = bm.next;
			bm.culled = cullmodel(b.m, bm.center, bm.radius, bm.flags, bm.d);
			if(bm.culled || bm.flags&MDL_ONLYSHADOW) continue;
			if(bm.colorscale.a < 1 || bm.flags&MDL_FORCETRANSPARENT)
			{
				float sx1, sy1, sx2, sy2;
				ivec bbmin(vec(bm.center).sub(bm.radius)), bbmax(vec(bm.center).add(bm.radius+1));
				if(calcbbscissor(bbmin, bbmax, sx1, sy1, sx2, sy2))
				{
					transmdlsx1 = min(transmdlsx1, sx1);
					transmdlsy1 = min(transmdlsy1, sy1);
					transmdlsx2 = max(transmdlsx2, sx2);
					transmdlsy2 = max(transmdlsy2, sy2);
					masktiles(transmdltiles, sx1, sy1, sx2, sy2);
				}
				continue;
			}
			if(!rendered)
			{
				b.m->startrender();
				rendered = true;
				setaamask(true);
			}
			if(bm.flags&MDL_CULL_QUERY)
			{
				bm.d->query = newquery(bm.d);
				if(bm.d->query)
				{
					startquery(bm.d->query);
					renderbatchedmodel(b.m, bm);
					endquery(bm.d->query);
					continue;
				}
			}
			renderbatchedmodel(b.m, bm);
		}
		if(rendered) b.m->endrender();
		if(b.flags&MDL_CULL_QUERY)
		{
			bool queried = false;
			for(int j = b.batched; j >= 0;)
			{
				batchedmodel &bm = batchedmodels[j];
				j = bm.next;
				if(bm.culled&(MDL_CULL_OCCLUDED|MDL_CULL_QUERY) && bm.flags&MDL_CULL_QUERY)
				{
					if(!queried)
					{
						if(rendered) setaamask(false);
						enablecullmodelquery();
						queried = true;
					}
					rendercullmodelquery(b.m, bm.d, bm.center, bm.radius);
				}
			}
			if(queried) disablecullmodelquery();
		}
	}
	disableaamask();
}

void rendertransparentmodelbatches(int stencil)
{
	enableaamask(stencil);
	loopv(batches)
	{
		modelbatch &b = batches[i];
		if(b.flags&MDL_MAPMODEL) continue;
		bool rendered = false;
		for(int j = b.batched; j >= 0;)
		{
			batchedmodel &bm = batchedmodels[j];
			j = bm.next;
			bm.culled = cullmodel(b.m, bm.center, bm.radius, bm.flags, bm.d);
			if(bm.culled || !(bm.colorscale.a < 1 || bm.flags&MDL_FORCETRANSPARENT) || bm.flags&MDL_ONLYSHADOW) continue;
			if(!rendered)
			{
				b.m->startrender();
				rendered = true;
				setaamask(true);
			}
			if(bm.flags&MDL_CULL_QUERY)
			{
				bm.d->query = newquery(bm.d);
				if(bm.d->query)
				{
					startquery(bm.d->query);
					renderbatchedmodel(b.m, bm);
					endquery(bm.d->query);
					continue;
				}
			}
			renderbatchedmodel(b.m, bm);
		}
		if(rendered) b.m->endrender();
	}
	disableaamask();
}

static occludequery *modelquery = NULL;
static int modelquerybatches = -1, modelquerymodels = -1, modelqueryattached = -1;

void startmodelquery(occludequery *query)
{
	modelquery = query;
	modelquerybatches = batches.size();
	modelquerymodels = batchedmodels.size();
	modelqueryattached = modelattached.size();
}

void endmodelquery()
{
	if(batchedmodels.size() == modelquerymodels)
	{
		modelquery->fragments = 0;
		modelquery = NULL;
		return;
	}
	enableaamask();
	startquery(modelquery);
	loopv(batches)
	{
		modelbatch &b = batches[i];
		int j = b.batched;
		if(j < modelquerymodels) continue;
		b.m->startrender();
		setaamask(!(b.flags&MDL_MAPMODEL) || b.m->animated());
		do
		{
			batchedmodel &bm = batchedmodels[j];
			renderbatchedmodel(b.m, bm);
			j = bm.next;
		}
		while(j >= modelquerymodels);
		b.batched = j;
		b.m->endrender();
	}
	endquery(modelquery);
	modelquery = NULL;
    batches.resize(modelquerybatches);
    batchedmodels.resize(modelquerymodels);
    modelattached.resize(modelqueryattached);
	disableaamask();
}

void clearbatchedmapmodels()
{
	loopv(batches)
	{
		modelbatch &b = batches[i];
		if(b.flags&MDL_MAPMODEL)
		{
            batchedmodels.resize(b.batched);
            batches.resize(i);
			break;
		}
	}
}

void rendermapmodel(int idx, int anim, const vec &o, float yaw, float pitch, float roll, int flags, int basetime, float size)
{
	// WatIsDeze: TODO: Remove.
//	conoutf("rendermapmodel: %d (%.2f/%.2f/%.2f) %s", idx, o.x, o.y, o.z, mapmodels.inrange(idx) ? "found" : "not found");

	if(mapmodels.size() <= idx && idx < 0) return;
	mapmodelinfo &mmi = mapmodels[idx];
	model *m = mmi.m ? mmi.m : nullptr;
	if (!m)
    {
    	std::string tmp;
	    std::tie(m, tmp) = loadmodel(mmi.name);
    }
	if(!m) return;

	vec center, bbradius;
	m->boundbox(center, bbradius);
	float radius = bbradius.magnitude();
	center.mul(size);
	if(roll) center.rotate_around_y(-roll*RAD);
	if(pitch && m->pitched()) center.rotate_around_x(pitch*RAD);
	center.rotate_around_z(yaw*RAD);
	center.add(o);
	radius *= size;

	int visible = 0;
	if(shadowmapping)
	{
		if(!m->shadow) return;
		visible = shadowmaskmodel(center, radius);
		if(!visible) return;
	}
	else if(flags&(MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED) && cullmodel(m, center, radius, flags))
		return;

	batchedmodel &b = batchedmodels.emplace_back();
	b.pos = o;
	b.center = center;
	b.radius = radius;
	b.anim = anim;
	b.yaw = yaw;
	b.pitch = pitch;
	b.roll = roll;
	b.basetime = basetime;
	b.basetime2 = 0;
	b.sizescale = size;
	b.colorscale = vec4(1, 1, 1, 1);
	b.flags = flags | MDL_MAPMODEL;
	b.visible = visible;
	b.d = NULL;
	b.attached = -1;
	addbatchedmodel(m, b, batchedmodels.size()-1);
}

void rendermodel(const char *mdl, int anim, const vec &o, float yaw, float pitch, float roll, int flags, ModelEntity *d, modelattach *a, int basetime, int basetime2, float size, const vec4 &color)
{
	auto [m, name] = loadmodel(mdl);
	if(!m) return;

	vec center, bbradius;
	m->boundbox(center, bbradius);
	float radius = bbradius.magnitude();
	MovableEntity *dynent = dynamic_cast<MovableEntity*>(d);
	if(dynent)
	{
		if(dynent->ragdoll)
		{
			if(anim&ANIM_RAGDOLL && dynent->ragdoll->millis >= basetime)
			{
				radius = max(radius, d->ragdoll->radius);
				center = dynent->ragdoll->center;
				goto hasboundbox;
			}
			DELETEP(dynent->ragdoll);
		}
		if(anim&ANIM_RAGDOLL) flags &= ~(MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY);
	}
	center.mul(size);
	if(roll) center.rotate_around_y(-roll*RAD);
	if(pitch && m->pitched()) center.rotate_around_x(pitch*RAD);
	center.rotate_around_z(yaw*RAD);
	center.add(o);
hasboundbox:
	radius *= size;

	if(flags&MDL_NORENDER) anim |= ANIM_NORENDER;

	if(a) for(int i = 0; a[i].tag; i++)
	{
		if(a[i].name)
        {
		    std::string tmp;
		    std::tie(a[i].m, tmp) = loadmodel(a[i].name);
        }
	}

	if(flags&MDL_CULL_QUERY)
	{
		if(!oqfrags || !oqdynent || !d) flags &= ~MDL_CULL_QUERY;
	}

	if(flags&MDL_NOBATCH)
	{
		int culled = cullmodel(m, center, radius, flags, d);
		if(culled)
		{
			if(culled&(MDL_CULL_OCCLUDED|MDL_CULL_QUERY) && flags&MDL_CULL_QUERY)
			{
				enablecullmodelquery();
				rendercullmodelquery(m, d, center, radius);
				disablecullmodelquery();
			}
			return;
		}
		enableaamask();
		if(flags&MDL_CULL_QUERY)
		{
			d->query = newquery(d);
			if(d->query) startquery(d->query);
		}
		m->startrender();
		setaamask(true);
		if(flags&MDL_FULLBRIGHT) anim |= ANIM_FULLBRIGHT;
		m->render(anim, basetime, basetime2, o, yaw, pitch, roll, d, a, size, color);
		m->endrender();
		if(flags&MDL_CULL_QUERY && d->query) endquery(d->query);
		disableaamask();
		return;
	}

	batchedmodel &b = batchedmodels.emplace_back();
	b.pos = o;
	b.center = center;
	b.radius = radius;
	b.anim = anim;
	b.yaw = yaw;
	b.pitch = pitch;
	b.roll = roll;
	b.basetime = basetime;
	b.basetime2 = basetime2;
	b.sizescale = size;
	b.colorscale = color;
	b.flags = flags;
	b.visible = 0;
	b.d = d;
	b.attached = a ? modelattached.size() : -1;
	if(a) for(int i = 0;; i++) { modelattached.emplace_back(a[i]); if(!a[i].tag) break; }
	addbatchedmodel(m, b, batchedmodels.size()-1);
}

int intersectmodel(const std::string &mdl, int anim, const vec &pos, float yaw, float pitch, float roll, const vec &o, const vec &ray, float &dist, int mode, MovableEntity *d, modelattach *a, int basetime, int basetime2, float size)
{
	auto [m, name] = loadmodel(mdl.c_str());
	if(!m) return -1;
	if(d && d->ragdoll && (!(anim&ANIM_RAGDOLL) || d->ragdoll->millis < basetime)) DELETEP(d->ragdoll);
	if(a) for(int i = 0; a[i].tag; i++)
	{
		if(a[i].name)
        {
		    std::string tmp;
		    std::tie(a[i].m, tmp) = loadmodel(a[i].name);
        }
	}
	return m->intersect(anim, basetime, basetime2, pos, yaw, pitch, roll, d, a, size, o, ray, dist, mode);
}

void abovemodel(vec &o, const std::string &mdl)
{
	auto [m, tmp] = loadmodel(mdl.c_str());
	if(!m) return;
	o.z += m->above();
}

bool matchanim(const std::string &name, const char *pattern)
{
	for(;; pattern++)
	{
		const char *s = name.c_str();
		char c;
		for(;; pattern++)
		{
			c = *pattern;
			if(!c || c=='|') break;
			else if(c=='*')
			{
				if(!*s || iscubespace(*s)) break;
				do s++; while(*s && !iscubespace(*s));
			}
			else if(c!=*s) break;
			else s++;
		}
		if(!*s && (!c || c=='|')) return true;
		pattern = strchr(pattern, '|');
		if(!pattern) break;
	}
	return false;
}

SCRIPTEXPORT void findanims(char *name)
{
    vector<int> anims;
    game::findanims(name, anims);
    vector<char> buf;
    cubestr num;
    loopv(anims)
    {
        formatcubestr(num, "%d", anims[i]);
        if(i > 0) buf.emplace_back(' ');
        put(num, strlen(num), buf);
    }
    buf.emplace_back('\0');
    result(buf.data());
}

void loadskin(const char *dir, const char *altdir, Texture *&skin, Texture *&masks) // model skin sharing
{
#define ifnoload(tex, path) if((tex = textureload(path, 0, true, false))==notexture)
#define tryload(tex, prefix, cmd, name) \
	ifnoload(tex, makerelpath(mdir, name ".jpg", prefix, cmd)) \
	{ \
		ifnoload(tex, makerelpath(mdir, name ".png", prefix, cmd)) \
		{ \
			ifnoload(tex, makerelpath(maltdir, name ".jpg", prefix, cmd)) \
			{ \
				ifnoload(tex, makerelpath(maltdir, name ".png", prefix, cmd)) return; \
			} \
		} \
	}

	defformatcubestr(mdir, "media/model/%s", dir);
	defformatcubestr(maltdir, "media/model/%s", altdir);
	masks = notexture;
	tryload(skin, NULL, NULL, "skin");
	tryload(masks, NULL, NULL, "masks");
}

void setbbfrommodel(MovableEntity *d, const std::string &mdl)
{
	auto [m, name] = loadmodel(mdl.c_str());
	if(!m) return;
	vec center, radius;
	m->collisionbox(center, radius);
	if(m->collide != COLLIDE_ELLIPSE) d->collidetype = COLLIDE_OBB;
	d->xradius   = radius.x + fabs(center.x);
	d->yradius   = radius.y + fabs(center.y);
	d->radius    = d->collidetype==COLLIDE_OBB ? sqrtf(d->xradius*d->xradius + d->yradius*d->yradius) : max(d->xradius, d->yradius);
	d->eyeheight = (center.z-radius.z) + radius.z*2*m->eyeheight;
	d->aboveeye  = radius.z*2*(1.0f-m->eyeheight);
}
