// world.cpp: core map management stuff
#include "shared/cube.h"
#include "shared/ents.h"
#include "shared/entities/Entity.h"
#include "shared/entities/DecalEntity.h"

#include "engine/light.h"
#include "engine/texture.h"
#include "engine/model.h"
#include "engine/pvs.h"
#include "engine/rendergl.h"
#include "engine/renderlights.h"
#include "engine/octaedit.h"
#include "engine/octarender.h"
#include "engine/command.h"
#include "engine/physics.h"
#include "engine/renderparticles.h"
#include "engine/stain.h"
#include "engine/menus.h"
#include "engine/blend.h"
#include "engine/SoundConfig.h"
#include "engine/main/Compatibility.h"
#include "engine/Camera.h"

#include "game/entities/SkeletalEntity.h"
#include "game/entities/PlayerSpawnEntity.h"
#include "game/entities/ModelEntity.h"
#include "game/game.h"


#include <cassert>
#include <game/entities/LightEntity.h>

VARR(mapversion, 1, MAPVERSION, 0);
VARNR(mapscale, worldscale, 1, 0, 0);
VARNR(mapsize, worldsize, 1, 0, 0);
SVARR(maptitle, "Untitled Map by Unknown");
VARNR(emptymap, _emptymap, 1, 0, 0);

VAR(octaentsize, 0, 64, 1024);
VAR(entselradius, 0, 2, 10);

inline void transformbb(const Entity *e, vec &center, vec &radius)
{
	if(e->scale > 0) { float scale = e->scale/100.0f; center.mul(scale); radius.mul(scale); }
	rotatebb(center, radius, e->d.z, e->d.y, e->d.x);
}

void mmboundbox(const Entity *e, model *m, vec &center, vec &radius)
{
    m->boundbox(center, radius);
    transformbb(e, center, radius);
}

void mmcollisionbox(const Entity *e, model *m, vec &center, vec &radius)
{
    m->collisionbox(center, radius);
    transformbb(e, center, radius);
}

static inline void decalboundbox(const Entity *e, DecalSlot &s, vec &center, vec &radius)
{
	float size = max(float(e->scale), 1.0f);
    center = vec(0, s.depth * size/2, 0);
    radius = vec(size/2, s.depth * size/2, size/2);
	rotatebb(center, radius, e->d.x, e->d.y, e->d.z);
}

bool getentboundingbox(const Entity *e, ivec &o, ivec &r)
{
	vec fo;
	vec fr;
	auto result = e->getBoundingBox(entselradius, fo, fr);
	o = ivec(fo);
	r = ivec(fr);
	return result;
}

enum
{
    MODOE_ADD      = 1<<0,
    MODOE_UPDATEBB = 1<<1,
    MODOE_CHANGED  = 1<<2
};

void modifyoctaentity(int flags, int id, Entity *e, cube *c, const ivec &cor, int size, const ivec &bo, const ivec &br, int leafsize, vtxarray *lastva = NULL)
{
    loopoctabox(cor, size, bo, br)
    {
        ivec o(i, cor, size);
        vtxarray *va = c[i].ext && c[i].ext->va ? c[i].ext->va : lastva;
        if(c[i].children != NULL && size > leafsize)
        {
            modifyoctaentity(flags, id, e, c[i].children, o, size>>1, bo, br, leafsize, va);
		}
        else if(flags&MODOE_ADD)
        {
            if(!c[i].ext || !c[i].ext->ents)
            {
				ext(c[i]).ents = new octaentities(o, size);
			}
            octaentities &oe = *c[i].ext->ents;
            
            auto e_decal = dynamic_cast<DecalEntity*>(e);
            auto e_mapmodel = dynamic_cast<ModelEntity*>(e);
            
            if (e_decal)
            {
				if(va)
				{
					va->bbmin.x = -1;
					if(oe.decals.empty()) va->decals.emplace_back(&oe);
				}
				oe.decals.push_back(id);
				oe.bbmin.min(bo).max(oe.o);
				oe.bbmax.max(br).min(ivec(oe.o).add(oe.size));
			}
			else if (e_mapmodel)
			{
			    e_mapmodel->preload();
				if(loadmapmodel(e_mapmodel))
				{
					if(va)
					{
						va->bbmin.x = -1;
						if(oe.mapmodels.empty()) va->mapmodels.emplace_back(&oe);
					}
					oe.mapmodels.push_back(id);
					oe.bbmin.min(bo).max(oe.o);
					oe.bbmax.max(br).min(ivec(oe.o).add(oe.size));
					break;
				}
			}
			else
			{
				oe.other.push_back(id);
			}
        }
        else if(c[i].ext && c[i].ext->ents)
        {
            octaentities &oe = *c[i].ext->ents;
			const auto& ents = getents();
			
            auto e_decal = dynamic_cast<DecalEntity*>(e);
            auto e_mapmodel = dynamic_cast<ModelEntity*>(e);

			if (e_decal)
			{
			    oe.decals.erase(std::remove(oe.decals.begin(), oe.decals.end(), id), oe.decals.end());

				if(va)
				{
					va->bbmin.x = -1;
					if(oe.decals.empty()) va->decals.removeobj(&oe);
				}
				oe.bbmin = oe.bbmax = oe.o;
				oe.bbmin.add(oe.size);
				for(auto& decalIdx : oe.decals)
				{
					auto e = ents[decalIdx];
					ivec eo, er;
					if(getentboundingbox(e, eo, er))
					{
						oe.bbmin.min(eo);
						oe.bbmax.max(er);
					}
				}
				oe.bbmin.max(oe.o);
				oe.bbmax.min(ivec(oe.o).add(oe.size));
			}
			else if (e_mapmodel)
			{
			    e_mapmodel->preload();
				if(loadmapmodel(e_mapmodel))
				{
				    oe.mapmodels.erase(std::remove(oe.mapmodels.begin(), oe.mapmodels.end(), id), oe.mapmodels.end());

					if(va)
					{
						va->bbmin.x = -1;
						if(oe.mapmodels.empty()) va->mapmodels.removeobj(&oe);
					}
					oe.bbmin = oe.bbmax = oe.o;
					oe.bbmin.add(oe.size);
					for(auto& modelIdx : oe.mapmodels)
					{
						Entity *e = getents()[modelIdx];
						ivec eo, er;
						if(getentboundingbox(e, eo, er))
						{
							oe.bbmin.min(eo);
							oe.bbmax.max(er);
						}
					}
					oe.bbmin.max(oe.o);
					oe.bbmax.min(ivec(oe.o).add(oe.size));
				}
			}
			else
			{
                oe.other.erase(std::remove(oe.other.begin(), oe.other.end(), id), oe.other.end());
			}

            if(oe.mapmodels.empty() && oe.decals.empty() && oe.other.empty())
            {
                freeoctaentities(c[i]);
			}
        }
        if(c[i].ext && c[i].ext->ents)
        {
			c[i].ext->ents->query = NULL;
		}
        if(va && va!=lastva)
        {
            if(lastva)
            {
                if(va->bbmin.x < 0)
                {
					lastva->bbmin.x = -1;
				}
            }
            else if(flags&MODOE_UPDATEBB)
            {
				updatevabb(va);
			}
        }
    }
}

vector<int> outsideents;
int spotlights = 0, volumetriclights = 0, nospeclights = 0;

static bool modifyoctaent(int flags, int id, Entity *e)
{
	if(flags&MODOE_ADD)
	{
		if (e->flags&EntityFlags::EF_OCTA)
		{
			return false;
		}
	}
	else
	{
		if (!(e->flags&EntityFlags::EF_OCTA))
		{
			return false;
		}
	}

    ivec o, r;
    if(!getentboundingbox(e, o, r)) return false;

	if(!insideworld(e->o))
    {
        int idx = outsideents.find(id);
        if(flags&MODOE_ADD)
        {
            if(idx < 0) outsideents.emplace_back(id);
        }
        else if(idx >= 0) outsideents.removeunordered(idx);
    }
    else
    {
        int leafsize = octaentsize, limit = max(r.x - o.x, max(r.y - o.y, r.z - o.z));
        while(leafsize < limit) leafsize *= 2;
        int diff = ~(leafsize-1) & ((o.x^r.x)|(o.y^r.y)|(o.z^r.z));
        if(diff && (limit > octaentsize/2 || diff < leafsize*2)) leafsize *= 2;
        modifyoctaentity(flags, id, e, worldroot, ivec(0, 0, 0), worldsize>>1, o, r, leafsize);
    }
	e->flags ^= EntityFlags::EF_OCTA;
    if (auto* el = dynamic_cast<LightEntity*>(e); el)
    {
        clearlightcache(id);
        if(el->type&L_VOLUMETRIC) { if(flags&MODOE_ADD) volumetriclights++; else --volumetriclights; }
        if(el->type&L_NOSPEC) { if(!(flags&MODOE_ADD ? nospeclights++ : --nospeclights)) cleardeferredlightshaders(); }
    }
//FIXME: implement SpotlightEntity and ParticleEntity
//        case ET_SPOTLIGHT: if(!(flags&MODOE_ADD ? spotlights++ : --spotlights)) { cleardeferredlightshaders(); cleanupvolumetric(); } break;
//        case ET_PARTICLES: clearparticleemitters(); break;
    if (auto* ed = dynamic_cast<DecalEntity*>(e); ed)
    {
        if(flags&MODOE_CHANGED) changed(o, r, false);
    }
    return true;
}

static inline bool modifyoctaent(int flags, int id)
{
    auto &ents = getents();
	return ents.inrange(id) && modifyoctaent(flags, id, ents[id]);
}

static inline void addentity(int id)        { modifyoctaent(MODOE_ADD|MODOE_UPDATEBB, id); }
void addentityedit(int id)    { modifyoctaent(MODOE_ADD|MODOE_UPDATEBB|MODOE_CHANGED, id); }
static inline void removeentity(int id)     { modifyoctaent(MODOE_UPDATEBB, id); }
static inline void removeentityedit(int id) { modifyoctaent(MODOE_UPDATEBB|MODOE_CHANGED, id); }

void freeoctaentities(cube &c)
{
    if(!c.ext) return;
    if(getents().size())
    {
        while(c.ext->ents && !c.ext->ents->mapmodels.empty())
        {
            auto& last = c.ext->ents->mapmodels.back();
            c.ext->ents->mapmodels.pop_back();
            removeentity(last);
        }
        while(c.ext->ents && !c.ext->ents->decals.empty())
        {
            auto& last = c.ext->ents->decals.back();
            c.ext->ents->decals.pop_back();
            removeentity(last);
        }
        while(c.ext->ents && !c.ext->ents->other.empty())
        {
            auto& last = c.ext->ents->other.back();
            c.ext->ents->other.pop_back();
            removeentity(last);
        }
    }
    if(c.ext->ents)
    {
        delete c.ext->ents;
        c.ext->ents = NULL;
    }
}

void entitiesinoctanodes()
{
    auto &ents = getents();

    loopv(ents) {
        if (ents.inrange(i)) {
            if (ents[i] != NULL)
            {
				modifyoctaent(MODOE_ADD, i, ents[i]);
                conoutf("entitiesinoctanodes: %d", ents.size());
            }
        }
    }
}

static inline void findents(octaentities &oe, int low, int high, bool notspawned, const vec &pos, const vec &invradius, vector<int> &found)
{
    auto &ents = getents();
    for(auto& id : oe.other)
    {
        if (ents.inrange(id)) {
            auto e = ents[id];
            // TODO: Fix this, et_type? and ent_type?
			if(/*e->et_type >= low && e->et_type <= high && */ (e->spawned || notspawned) && vec(e->o).sub(pos).mul(invradius).squaredlen() <= 1)
                found.emplace_back(id);
        }
    }
}

static inline void findents(cube *c, const ivec &o, int size, const ivec &bo, const ivec &br, int low, int high, bool notspawned, const vec &pos, const vec &invradius, vector<int> &found)
{
    loopoctabox(o, size, bo, br)
    {
        if(c[i].ext && c[i].ext->ents) findents(*c[i].ext->ents, low, high, notspawned, pos, invradius, found);
        if(c[i].children && size > octaentsize)
        {
            ivec co(i, o, size);
            findents(c[i].children, co, size>>1, bo, br, low, high, notspawned, pos, invradius, found);
        }
    }
}

void findents(int low, int high, bool notspawned, const vec &pos, const vec &radius, vector<int> &found)
{
    vec invradius(1/radius.x, 1/radius.y, 1/radius.z);
    ivec bo(vec(pos).sub(radius).sub(1)),
         br(vec(pos).add(radius).add(1));
    int diff = (bo.x^br.x) | (bo.y^br.y) | (bo.z^br.z) | octaentsize,
        scale = worldscale-1;
    if(diff&~((1<<scale)-1) || uint(bo.x|bo.y|bo.z|br.x|br.y|br.z) >= uint(worldsize))
    {
        findents(worldroot, ivec(0, 0, 0), 1<<scale, bo, br, low, high, notspawned, pos, invradius, found);
        return;
    }
    cube *c = &worldroot[octastep(bo.x, bo.y, bo.z, scale)];
    if(c->ext && c->ext->ents) findents(*c->ext->ents, low, high, notspawned, pos, invradius, found);
    scale--;
    while(c->children && !(diff&(1<<scale)))
    {
        c = &c->children[octastep(bo.x, bo.y, bo.z, scale)];
        if(c->ext && c->ext->ents) findents(*c->ext->ents, low, high, notspawned, pos, invradius, found);
        scale--;
    }
    if(c->children && 1<<scale >= octaentsize) findents(c->children, ivec(bo).mask(~((2<<scale)-1)), 1<<scale, bo, br, low, high, notspawned, pos, invradius, found);
}

const char *entname(Entity *e)
{
    return e->name.c_str();
}

extern selinfo sel;
extern bool havesel;
int entlooplevel = 0;
int efocus = -1, enthover = -1, entorient = -1, oldhover = -1;
bool undonext = true;

VARF(entediting, 0, 0, 1, {
	if(!entediting)
	{
		send_entity_event(enthover, EntityEventHoverStop());
		entcancel();
		efocus = enthover = -1;
	}
});

bool noentedit()
{
    if(!editmode) { conoutf(CON_ERROR, "operation only allowed in edit mode"); return true; }
    return !entediting;
}

bool pointinsel(const selinfo &sel, const vec &o)
{
    return(o.x <= sel.o.x+sel.s.x*sel.grid
        && o.x >= sel.o.x
        && o.y <= sel.o.y+sel.s.y*sel.grid
        && o.y >= sel.o.y
        && o.z <= sel.o.z+sel.s.z*sel.grid
        && o.z >= sel.o.z);
}

vector<int> entgroup;

bool haveselent()
{
    return entgroup.size() > 0;
}

SCRIPTEXPORT void entcancel()
{
    loopv(entgroup)
    {
        send_entity_event(entgroup[i], EntityEventSelectStop());
    }
    entgroup.shrink(0);
}

void entadd(int id)
{
    undonext = true;
    entgroup.emplace_back(id);
	send_entity_event(id, EntityEventSelectStart());
}

undoblock *newundoent()
{
    int numents = entgroup.size();
    if(numents <= 0) return NULL;
    undoblock *u = (undoblock *)new uchar[sizeof(undoblock) + numents*sizeof(undoent)];
    u->numents = numents;
    undoent *e = (undoent *)(u + 1);
    auto& ents = getents();
    loopv(entgroup)
    {
        e->i = entgroup[i];
        assert(ents.size() > entgroup[i]);
        e->e = ents[entgroup[i]];
        e++;
    }
    return u;
}

void makeundoent()
{
    if(!undonext) return;
    undonext = false;
    oldhover = enthover;
    undoblock *u = newundoent();
    if(u) addundo(u);
}

void detachentity(Entity *e)
{
    //FIXME: Attachement feature
//	if(!e->attached) return;
//	e->attached->attached = NULL;
//	e->attached = NULL;
}

VAR(attachradius, 1, 100, 1000);

void attachentity(Entity *e)
{
    //FIXME: Attache entity feature
//    switch(e->et_type)
//    {
//        case ET_SPOTLIGHT:
//            break;
//
//        default:
//            if(e->et_type<ET_GAMESPECIFIC || !mayattach(e)) return;
//            break;
//    }
//
//	detachentity(e);
//
//    auto &ents = getents();
//    int closest = -1;
//    float closedist = 1e10f;
//    loopv(ents)
//    {
//        auto a = ents[i];
//        if (!a) continue;
//
//        if(a->attached) continue;
//        switch(e->et_type)
//        {
//            case ET_SPOTLIGHT:
//                if(a->et_type!=ET_LIGHT) continue;
//                break;
//
//            default:
//                if(e->et_type<ET_GAMESPECIFIC || !entitities_attachable(e, a)) continue;
//                break;
//        }
//        float dist = e->o.dist(a->o);
//        if(dist < closedist)
//        {
//            closest = i;
//            closedist = dist;
//        }
//    }
//    if(closedist>attachradius) return;
//    e->attached = ents[closest];
//    ents[closest]->attached = e;
}

void attachentities()
{
    auto  &ents = getents();
    loopv(ents) attachentity(ents[i]);
}

// convenience macros implicitly define:
// e         entity, currently edited ent
// n         int,    index to currently edited ent
#define addimplicit(f)    { if(entgroup.empty() && enthover>=0) { entadd(enthover); undonext = (enthover != oldhover); f; entgroup.erase(entgroup.end() - 1, entgroup.end()); } else f; }
#define entfocusv(i, f, v){ int n = efocus = (i); if(n>=0) { Entity *e = v[n]; f; } }
#define entfocus(i, f)    entfocusv(i, f, getents())
#define enteditv(i, f, v) \
{ \
    entfocusv(i, \
    { \
        /*int old_et_type = e->et_type; \
        int old_ent_type = e->ent_type; \
        int old_game_type = e->game_type;*/ \
        removeentityedit(n);  \
        f; \
        /*if(old_et_type!=e->et_type)*/ detachentity(e); \
        if(e->classname!="core_entity") { addentityedit(n); /*if(old_et_type!=e->et_type)*/ attachentity(e); } \
        editent(n, true); \
        clearshadowcache(); \
    }, v); \
}
#define entedit(i, f)   enteditv(i, f, getents())
#define addgroup(exp)   { auto &ents = getents(); loopv(ents) entfocusv(i, if(exp) entadd(n), ents); }
#define setgroup(exp)   { entcancel(); addgroup(exp); }
#define groupeditloop(f){ auto &ents = getents(); entlooplevel++; int _ = efocus; loopv(entgroup) enteditv(entgroup[i], f, ents); efocus = _; entlooplevel--; }
#define groupeditpure(f){ if(entlooplevel>0) { entedit(efocus, f); } else { groupeditloop(f); commitchanges(); } }
#define groupeditundo(f){ makeundoent(); groupeditpure(f); }
#define groupedit(f)    { addimplicit(groupeditundo(f)); }

vec getselpos()
{
    auto &ents = getents();
    if(entgroup.size() && ents.inrange(entgroup[0])) return ents[entgroup[0]]->o;
    if(ents.inrange(enthover)) return ents[enthover]->o;
    return vec(sel.o);
}

undoblock *copyundoents(undoblock *u)
{
    entcancel();
    undoent *e = u->ents();
    loopi(u->numents)
        entadd(e[i].i);
    undoblock *c = newundoent();
    loopi(u->numents)
    {
//		if(e[i].e->et_type==ET_EMPTY)
        if(e[i].e->classname == "core_entity")
		{
			send_entity_event(e[i].i, EntityEventSelectStop());
			entgroup.removeobj(e[i].i);
		}
	}
    return c;
}

void pasteundoent(int idx, Entity* ue)
{
    if(idx < 0 || idx >= MAXENTS) return;
    auto &ents = getents();
    while(ents.size() < idx)
    {
		auto ne = new_game_entity(true, ue->o, idx, "core_entity");
        ents.emplace_back(ne);
	}
    int efocus = -1;
    entedit(idx, e = ue);
}

void pasteundoents(undoblock *u)
{
    undoent *ue = u->ents();
    loopi(u->numents) pasteundoent(ue[i].i, ue[i].e);
}

SCRIPTEXPORT void entflip()
{
    if(noentedit()) return;
    int d = dimension(sel.orient);
    float mid = sel.s[d]*sel.grid/2+sel.o[d];
	groupeditundo(e->o[d] -= (e->o[d]-mid)*2);
}

SCRIPTEXPORT void entrotate(int *cw)
{
    if(noentedit()) return;
    int d = dimension(sel.orient);
    int dd = (*cw<0) == dimcoord(sel.orient) ? R[d] : C[d];
    float mid = sel.s[dd]*sel.grid/2+sel.o[dd];
    vec s(sel.o.v);
    groupeditundo(
        e->o[dd] -= (e->o[dd]-mid)*2;
        e->o.sub(s);
        swap(e->o[R[d]], e->o[C[d]]);
            e->o.add(s);
    );
}

void entselectionbox(const Entity *e, vec &eo, vec &es)
{
	vec bbmin;
	vec bbmax;
	e->getBoundingBox(entselradius, bbmin, bbmax);
	
	eo.x = bbmin.x;
	eo.y = bbmin.y;
	eo.z = bbmin.z;
	es.x = bbmax.x - bbmin.x;
	es.y = bbmax.y - bbmin.y;
	es.z = bbmax.z - bbmin.z;
	
	return;

    std::string mname = entmodel(e);
    if(!mname.empty())
    {
        auto [m, name] = loadmodel(mname);
        if(m)
        {
            m->collisionbox(eo, es);
            if(es.x > es.y) es.y = es.x; else es.x = es.y; // square
            es.z = (es.z + eo.z + 1 + entselradius)/2; // enclose ent radius box and model box
            eo.x += e->o.x;
            eo.y += e->o.y;
            eo.z = e->o.z - entselradius + es.z;
        }
        else if(auto em = dynamic_cast<const ModelEntity*>(e); em && (m = loadmapmodel(em->model_idx)))
        {
            mmcollisionbox(e, m, eo, es);
            es.max(entselradius);
            eo.add(e->o);
        }
        else if(auto ed = dynamic_cast<const DecalEntity*>(e); ed)
        {
            DecalSlot &s = lookupdecalslot(ed->m_DecalSlot, false);
            decalboundbox(e, s, eo, es);
            es.max(entselradius);
            eo.add(e->o);
        }
        else
        {
            es = vec(entselradius);
            eo = e->o;
        }
        eo.sub(es);
        es.mul(2);
    }
}

VAR(entselsnap, 0, 0, 1);
VAR(entmovingshadow, 0, 1, 1);

extern void boxs(int orient, vec o, const vec &s, float size);
extern void boxs(int orient, vec o, const vec &s);
extern void boxs3D(const vec &o, vec s, int g);
extern bool editmoveplane(const vec &camPos, const vec &o, const vec &ray, int d, float off, vec &handle, vec &dest, bool first);

int entmoving = 0;

void entdrag(const vec &rayOrigin, const vec &ray)
{
    if(noentedit() || !haveselent()) return;

    float r = 0, c = 0;
    static vec dest, handle;
    vec eo, es;
    int d = dimension(entorient),
        dc= dimcoord(entorient);

    entfocus(entgroup.back(),
        entselectionbox(e, eo, es);

        if(!editmoveplane(rayOrigin, e->o, ray, d, eo[d] + (dc ? es[d] : 0), handle, dest, entmoving==1))
            return;

        ivec g(dest);
        int z = g[d]&(~(sel.grid-1));
                g.add(sel.grid / 2).mask(~(sel.grid - 1));
        g[d] = z;

        r = (entselsnap ? g[R[d]] : dest[R[d]]) - e->o[R[d]];
        c = (entselsnap ? g[C[d]] : dest[C[d]]) - e->o[C[d]];
    );

    if(entmoving==1) makeundoent();
    groupeditpure(e->o[R[d]] += r; e->o[C[d]] += c);
    entmoving = 2;
}

VAR(showentradius, 0, 1, 1);

void renderentring(const Entity *e, float radius, int axis)
{
    if(radius <= 0) return;
    gle::defvertex();
    gle::begin(GL_LINE_LOOP);
    loopi(15)
    {
        vec p(e->o);
        const vec2 &sc = sincos360[i*(360/15)];
        p[axis>=2 ? 1 : 0] += radius*sc.x;
        p[axis>=1 ? 2 : 1] += radius*sc.y;
        gle::attrib(p);
    }
    xtraverts += gle::end();
}

void renderentsphere(const Entity *e, float radius)
{
    if(radius <= 0) return;
    loopk(3) renderentring(e, radius, k);
}

void renderentattachment(const Entity *e)
{
    //FIXME: Attachement feature
//    if(!e->attached) return;
//    gle::defvertex();
//    gle::begin(GL_LINES);
//    gle::attrib(e->o);
//    gle::attrib(e->attached->o);
//    xtraverts += gle::end();
}

void renderentarrow(const Entity *e, const vec &dir, float radius)
{
    if(radius <= 0) return;
    float arrowsize = min(radius/8, 0.5f);
    vec target = vec(dir).mul(radius).add(e->o), arrowbase = vec(dir).mul(radius - arrowsize).add(e->o), spoke;
    spoke.orthogonal(dir);
    spoke.normalize();
    spoke.mul(arrowsize);

    gle::defvertex();

    gle::begin(GL_LINES);
    gle::attrib(e->o);
    gle::attrib(target);
    xtraverts += gle::end();

    gle::begin(GL_TRIANGLE_FAN);
    gle::attrib(target);
    loopi(5) gle::attrib(vec(spoke).rotate(2*M_PI*i/4.0f, dir).add(arrowbase));
    xtraverts += gle::end();
}

void renderentcone(const Entity *e, const vec &dir, float radius, float angle)
{
    if(radius <= 0) return;
    vec spot = vec(dir).mul(radius*cosf(angle*RAD)).add(e->o), spoke;
    spoke.orthogonal(dir);
    spoke.normalize();
    spoke.mul(radius*sinf(angle*RAD));

    gle::defvertex();

    gle::begin(GL_LINES);
    loopi(8)
    {
        gle::attrib(e->o);
        gle::attrib(vec(spoke).rotate(2*M_PI*i/8.0f, dir).add(spot));
    }
    xtraverts += gle::end();

    gle::begin(GL_LINE_LOOP);
    loopi(8) gle::attrib(vec(spoke).rotate(2*M_PI*i/8.0f, dir).add(spot));
    xtraverts += gle::end();
}

void renderentbox(const Entity *e, const vec &center, const vec &radius, int yaw, int pitch, int roll)
{
    matrix4x3 orient;
    orient.identity();
    orient.settranslation(e->o);
    if(yaw) orient.rotate_around_z(sincosmod360(yaw));
    if(pitch) orient.rotate_around_x(sincosmod360(pitch));
    if(roll) orient.rotate_around_y(sincosmod360(-roll));
    orient.translate(center);

    gle::defvertex();

    vec front[4] = { vec(-radius.x, -radius.y, -radius.z), vec( radius.x, -radius.y, -radius.z), vec( radius.x, -radius.y,  radius.z), vec(-radius.x, -radius.y,  radius.z) },
        back[4] = { vec(-radius.x, radius.y, -radius.z), vec( radius.x, radius.y, -radius.z), vec( radius.x, radius.y,  radius.z), vec(-radius.x, radius.y,  radius.z) };
    loopi(4)
    {
        front[i] = orient.transform(front[i]);
        back[i] = orient.transform(back[i]);
    }

    gle::begin(GL_LINE_LOOP);
    loopi(4) gle::attrib(front[i]);
    xtraverts += gle::end();

    gle::begin(GL_LINES);
    gle::attrib(front[0]);
        gle::attrib(front[2]);
    gle::attrib(front[1]);
        gle::attrib(front[3]);
    loopi(4)
    {
        gle::attrib(front[i]);
        gle::attrib(back[i]);
    }
    xtraverts += gle::end();

    gle::begin(GL_LINE_LOOP);
    loopi(4) gle::attrib(back[i]);
    xtraverts += gle::end();
}

void renderentradius(Entity *e, bool color)
{
/*    switch(e->et_type)
    {
        case ET_LIGHT:
            if(e->attr1 <= 0) break;
            if(color) gle::colorf(e->attr2/255.0f, e->attr3/255.0f, e->attr4/255.0f);
            renderentsphere(e, e->attr1);
            break;

        case ET_SPOTLIGHT:
            if(e->attached)
            {
                if(color) gle::colorf(0, 1, 1);
                float radius = e->attached->attr1;
                if(radius <= 0) break;
                vec dir = vec(e->o).sub(e->attached->o).normalize();
                float angle = clamp(int(e->attr1), 1, 89);
                renderentattachment(e);
                renderentcone(e->attached, dir, radius, angle);
            }
            break;

        case ET_SOUND:
            if(color) gle::colorf(0, 1, 1);
            renderentsphere(e, e->attr2);
            break;

        case ET_ENVMAP:
        {
            extern int envmapradius;
            if(color) gle::colorf(0, 1, 1);
            renderentsphere(e, e->attr1 ? max(0, min(10000, int(e->attr1))) : envmapradius);
            break;
        }

        case ET_MAPMODEL:
        {
            if(color) gle::colorf(0, 1, 1);
            entradius(e, color);
            vec dir;
            vecfromyawpitch(e->attr2, e->attr3, 1, 0, dir);
            renderentarrow(e, dir, 4);
            break;
        }

        case ET_PLAYERSTART:
        {
            if(color) gle::colorf(0, 1, 1);
            entradius(e, color);
            vec dir;
            vecfromyawpitch(e->attr1, 0, 1, 0, dir);
            renderentarrow(e, dir, 4);
            break;
        }

        case ET_DECAL:
        {
            if(color) gle::colorf(0, 1, 1);
            DecalSlot &s = lookupdecalslot(e->attr1, false);
            float size = max(float(e->attr5), 1.0f);
            renderentbox(e, vec(0, s.depth * size/2, 0), vec(size/2, s.depth * size/2, size/2), e->attr2, e->attr3, e->attr4);
            break;
        }

        default:
            if(e->et_type>=ET_GAMESPECIFIC)
            {
                if(color) gle::colorf(0, 1, 1);
                entradius(e, color);
            }
            break;
    }*/
}

static void renderenticosahedron(const vec &eo, vec es)
{
	static const float x_factor = .525731112119133606f;
	static const float z_factor = .850650808352039932;
	vec es2 = es;
	es2.mul(0.5f);
	static const vec esx_factor(es2.x * x_factor, es2.y * x_factor, es2.z * x_factor);
	static const vec esz_factor(es2.x * z_factor, es2.y * z_factor, es2.z * z_factor);
	static const vec vertices[12] = {
		{-esx_factor.x, 0.0f, esz_factor.z}, {esx_factor.x, 0.0f, esz_factor.z}, {-esx_factor.x, 0.0f, -esz_factor.z}, {esx_factor.x, 0.0f, -esz_factor.z},
		{0.0f, esz_factor.y, esx_factor.z}, {0.0f, esz_factor.y, -esx_factor.z}, {0.0f, -esz_factor.y, esx_factor.z}, {0.0f, -esz_factor.y, -esx_factor.z},
		{esz_factor.x, esx_factor.y, 0.0f}, {-esz_factor.x, esx_factor.y, 0.0f}, {esz_factor.x, -esx_factor.y, 0.0f}, {-esz_factor.x, -esx_factor.y, 0.0f}
	};
//	static const vec vertices[12] = {
//		{-x_factor, 0.0f, z_factor}, {x_factor, 0.0f, z_factor}, {-x_factor, 0.0f, -z_factor}, {x_factor, 0.0f, -z_factor},
//		{0.0f, z_factor, x_factor}, {0.0f, z_factor, -x_factor}, {0.0f, -z_factor, x_factor}, {0.0f, -z_factor, -x_factor},
//		{z_factor, x_factor, 0.0f}, {-z_factor, x_factor, 0.0f}, {z_factor, -x_factor, 0.0f}, {-z_factor, -x_factor, 0.0f}
//	};
	static int indices[20][3] = {
		{0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
		{8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
		{7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
		{6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
	};
	
	for (auto tri : indices)
	{
		vec p0(vertices[tri[0]]);
		vec p1(vertices[tri[1]]);
		vec p2(vertices[tri[2]]);
		p0.add(eo);
		p1.add(eo);
		p2.add(eo);
		p0.add(es2);
		p1.add(es2);
		p2.add(es2);
		gle::attrib(p0); gle::attrib(p1);
		gle::attrib(p1); gle::attrib(p2);
		gle::attrib(p2); gle::attrib(p0);
	}
}

static void renderentbox(const vec &eo, vec es)
{
	renderenticosahedron(eo, es);
	return;
	
    es.add(eo);

    // bottom quad
    gle::attrib(eo.x, eo.y, eo.z); gle::attrib(es.x, eo.y, eo.z);
    gle::attrib(es.x, eo.y, eo.z); gle::attrib(es.x, es.y, eo.z);
    gle::attrib(es.x, es.y, eo.z); gle::attrib(eo.x, es.y, eo.z);
    gle::attrib(eo.x, es.y, eo.z); gle::attrib(eo.x, eo.y, eo.z);

    // top quad
    gle::attrib(eo.x, eo.y, es.z); gle::attrib(es.x, eo.y, es.z);
    gle::attrib(es.x, eo.y, es.z); gle::attrib(es.x, es.y, es.z);
    gle::attrib(es.x, es.y, es.z); gle::attrib(eo.x, es.y, es.z);
    gle::attrib(eo.x, es.y, es.z); gle::attrib(eo.x, eo.y, es.z);

    // sides
    gle::attrib(eo.x, eo.y, eo.z); gle::attrib(eo.x, eo.y, es.z);
    gle::attrib(es.x, eo.y, eo.z); gle::attrib(es.x, eo.y, es.z);
    gle::attrib(es.x, es.y, eo.z); gle::attrib(es.x, es.y, es.z);
    gle::attrib(eo.x, es.y, eo.z); gle::attrib(eo.x, es.y, es.z);
}

void renderentselection(const vec &o, const vec &ray, bool entmoving)
{
	extern int xtraverts;
	
    if(noentedit() || (entgroup.empty() && enthover < 0)) return;
    vec eo, es;

    if(entgroup.size())
    {
//        gle::colorub(0, 0, 40);
//        gle::defvertex();
//        gle::begin(GL_LINES, entgroup.size()*20*6);
//        loopv(entgroup) entfocus(entgroup[i],
//            entselectionbox(e, eo, es);
//            renderentbox(eo, es);
//        );
//        xtraverts += gle::end();
    }

    if(enthover >= 0 && enthover < getents().size())
    {
//		auto highlighted_ent = getents()[enthover];
//		float cameraRelativeTickness = clamp(0.015f*camera1->o.dist(highlighted_ent->o)*tan(fovy*0.5f*RAD), 0.1f, 1.0f);
//		highlighted_ent->renderHighlight(entselradius, entorient, cameraRelativeTickness);
//
//        if(entmoving && entmovingshadow==1)
//        {
//			highlighted_ent->renderMoveShadow(entselradius, worldsize);
//        }
    }

    if(showentradius)
    {
//        glDepthFunc(GL_GREATER);
//        gle::colorf(0.25f, 0.25f, 0.25f);
//        loopv(entgroup) entfocus(entgroup[i], renderentradius(e, false));
//        if(enthover>=0) entfocus(enthover, renderentradius(e, false));
//        glDepthFunc(GL_LESS);
//        loopv(entgroup) entfocus(entgroup[i], renderentradius(e, true));
//        if(enthover>=0) entfocus(enthover, renderentradius(e, true));
    }
}

bool enttoggle(int id)
{
    undonext = true;
    int i = entgroup.find(id);
    if(i < 0)
    {
        entadd(id);
	}
    else
    {
		send_entity_event(i, EntityEventSelectStop());
        entgroup.remove(i);
	}
    return i < 0;
}

bool hoveringonent(int ent, int orient)
{
    if(noentedit()) return false;
    auto oldOrient = entorient;
    entorient = orient;
    if(ent >= 0)
    {
		if (enthover != ent)
		{
			send_entity_event(ent, EntityEventHoverStart(entorient));
			if (enthover >= 0)
			{
				send_entity_event(enthover, EntityEventHoverStop());
			}
		}
		else if (oldOrient != entorient)
		{
			send_entity_event(ent, EntityEventHoverStart(entorient));
		}
		efocus = enthover = ent;
        return true;
	}
    efocus = entgroup.empty() ? -1 : entgroup.back();
    if (enthover >= 0)
    {
		send_entity_event(enthover, EntityEventHoverStop());
	}
    enthover = -1;
    return false;
}

VAR(entitysurf, 0, 0, 1);

SCRIPTEXPORT_AS(entadd) void entadd_scriptimpl()
{
    if(enthover >= 0 && !noentedit())
    {
        if(entgroup.find(enthover) < 0)
        {
			entadd(enthover);
			send_entity_event(enthover, EntityEventMoveStart());
		}
        if(entmoving > 1) entmoving = 1;
    }
}

SCRIPTEXPORT_AS(enttoggle) void enttoggle_scriptimpl()
{
    if(enthover < 0 || noentedit() || !enttoggle(enthover))
    {
		entmoving = 0; intret(0);
	}
    else
    {
		if(entmoving > 1)
		{
			entmoving = 1; intret(1);
		}
	}
}

SCRIPTEXPORT_AS(entmoving) void entmoving_scriptimpl(CommandTypes::Boolean n)
{
    if(*n >= 0)
    {
		auto hasNoEntover = enthover < 0;
        if(!*n || hasNoEntover || noentedit())
        {
			entmoving = 0;
			if (!hasNoEntover)
			{
				send_entity_event(enthover, EntityEventMoveStop());
			}
		}
        else
        {
            if(entgroup.find(enthover) < 0)
            {
				entadd(enthover); entmoving = 1;
			}
            else if(!entmoving)
            {
				entmoving = 1;
			}
			send_entity_event(enthover, EntityEventMoveStart());
        }
    }
    intret(entmoving);
}

SCRIPTEXPORT void entpush(int *dir)
{
    if(noentedit()) return;
    int d = dimension(entorient);
    int s = dimcoord(entorient) ? -*dir : *dir;
    if(entmoving)
    {
        groupeditpure(e->o[d] += float(s*sel.grid)); // editdrag supplies the undo
    }
    else
        groupedit(e->o[d] += float(s*sel.grid));
    if(entitysurf==1)
    {
        player->o[d] += float(s*sel.grid);
        player->resetinterp();
    }
}

VAR(entautoviewdist, 0, 25, 100);
SCRIPTEXPORT void entautoview(int *dir)
{
    if(!haveselent()) return;
    const auto& activeCamera = Camera::GetActiveCamera();
    if (!activeCamera) return;

    static int s = 0;
    vec v(player->o);
    v.sub(activeCamera->GetWorldPos());
    v.normalize();
    v.mul(entautoviewdist);
    int t = s + *dir;
    s = abs(t) % entgroup.size();
    if(t<0 && s>0) s = entgroup.size() - s;
    entfocus(entgroup[s],
        v.add(e->o);
        player->o = v;
        player->resetinterp();
    );
}


SCRIPTEXPORT void delent()
{
    if(noentedit()) return;
    std::vector<Entity*> entitiesToDelete;

    groupedit(entitiesToDelete.push_back(e););
    entcancel();

    for (auto& e : entitiesToDelete)
    {
        delete e;
    }
}

int findtype(char *what)
{
    for(int i = 0; *entname(i); i++) if(strcmp(what, entname(i))==0) return i;
    conoutf(CON_ERROR, "unknown entity type \"%s\"", what);
    return ET_EMPTY;
}

VAR(entdrop, 0, 2, 3);

bool dropentity(Entity *e, int drop = -1)
{
    if (!e) {
        return false;
    }

    vec radius(4.0f, 4.0f, 4.0f);
    if(drop<0) drop = entdrop;
    auto em = dynamic_cast<ModelEntity*>(e);
    auto el = dynamic_cast<LightEntity*>(e);
    if(em)
    {
        model *m = loadmapmodel(em->model_idx);
        if(m)
        {
            vec center;
            mmboundbox(e, m, center, radius);
            radius.x += fabs(center.x);
            radius.y += fabs(center.y);
        }
        radius.z = 0.0f;
    }
    switch(drop)
    {
    case 1:
        if(el /*&& e->et_type != ET_SPOTLIGHT*/)
           dropenttofloor(e);
        break;
    case 2:
    case 3:
        int cx = 0, cy = 0;
        if(sel.cxs == 1 && sel.cys == 1)
        {
            cx = (sel.cx ? 1 : -1) * sel.grid / 2;
            cy = (sel.cy ? 1 : -1) * sel.grid / 2;
        }
        e->o = vec(sel.o);
        int d = dimension(sel.orient), dc = dimcoord(sel.orient);
        e->o[R[d]] += sel.grid / 2 + cx;
        e->o[C[d]] += sel.grid / 2 + cy;
        if(!dc)
            e->o[D[d]] -= radius[D[d]];
        else
            e->o[D[d]] += sel.grid + radius[D[d]];

        if(el /*&& e->et_type != ET_SPOTLIGHT*/)
            dropenttofloor(e);
        break;
    }
    return true;
}

SCRIPTEXPORT void dropent()
{
    if(noentedit()) return;
    groupedit(dropentity(e));
}

SCRIPTEXPORT void attachent()
{
    if(noentedit()) return;
    groupedit(attachentity((MovableEntity*)&e));
}

static int keepents = 0;
//
//Entity *newentity(bool local, const vec &o, int type, int v1, int v2, int v3, int v4, int v5, int &idx, bool fix = true)
//{
//    auto &ents = getents();
//
//    // If local, we ensure it is not out of bounds, if it is, we return NULL and warn our player.
//    if(local)
//    {
//        idx = -1;
//        for(int i = keepents; i < ents.size(); i++)
//        {
//			if(ents[i]->et_type == ET_EMPTY)
//			{
//				idx = i; break;
//			}
//		}
//        if(idx < 0 && ents.size() >= MAXENTS)
//        {
//			conoutf("too many entities");
//			return NULL;
//		}
//    } else {
//        while(ents.size() < idx)
//        {
//            ents.emplace_back(newgameentity(""))->et_type = ET_EMPTY;
//        }
//    }
//
//    auto e = newgameentity("");
//    e->o = o;
//    e->attr1 = v1;
//    e->attr2 = v2;
//    e->attr3 = v3;
//    e->attr4 = v4;
//    e->attr5 = v5;
//    e->et_type = type;
//    e->ent_type = ENT_INANIMATE;
//    e->game_type = type;
//    e->reserved = 0;
//	e->name = "tesseract_ent_" + std::to_string(idx);
//
//    if(ents.inrange(idx))
//    {
//		deletegameentity(ents[idx]);
//		ents[idx] = e;
//	}
//    else
//    {
//		idx = ents.size();
//		ents.emplace_back(e);
//	}
//	return e;
//}
//
//void newentity(int type, int a1, int a2, int a3, int a4, int a5, bool fix = true)
//{
//    int idx;
//    auto t = newentity(true, player->o, type, a1, a2, a3, a4, a5, idx, fix);
//    if(!t) return;
//    dropentity(t);
//    t->et_type = ET_EMPTY;
//    enttoggle(idx);
//    makeundoent();
//    entedit(idx, e->et_type = type);
//    commitchanges();
//}
//
//SCRIPTEXPORT void newent(char *what, int *a1, int *a2, int *a3, int *a4, int *a5)
//{
//    if(noentedit()) return;
//    int type = findtype(what);
//    if(type != ET_EMPTY)
//        newentity(type, *a1, *a2, *a3, *a4, *a5);
//}
namespace {
	void find_next_entity_index(int &idx)
	{
		auto &ents = getents();
		idx = -1;
		
//		for (int i = keepents; i < ents.size(); i++)
//		{
//			if(ents[i]->et_type == ET_EMPTY)
//			{
//				conoutf("idx = %i", idx);
//				idx = i;
//				break;
//			}
//		}
	}
}

Entity *new_game_entity(bool local, const vec &o, int &idx, const char *strclass)
{
    // Retreive the list of entities.
    auto &ents = getents();

    // If local, we ensure it is not out of bounds, if it is, we return NULL and warn our player.
    if (local)
    {
		find_next_entity_index(idx);
        
        if (idx < 0 && ents.size() >= MAXENTS)
        {
            conoutf("too many entities"); return nullptr;
        }
    }
    else
    {
        if (ents.size() < idx)
        {
			vec mapcenter;
			mapcenter.x = mapcenter.y = mapcenter.z = 0.5f*worldsize;
			mapcenter.z += 1;
			mapcenter.x += idx * 2.5f;
            ents.emplace_back(new_game_entity(false, mapcenter, idx, "core_entity"));
        }
    }

    Entity *ent = EntityFactory::constructEntity(std::string(strclass));

    ent->o = o;

	if (ents.inrange(idx))
	{
		deletegameentity(ents[idx]);
        ents[idx] = ent;
	}
	else
	{
		idx = ents.size();
        ents.emplace_back(ent);
	}

    ent->entityId = idx;

	enttoggle(idx);
	makeundoent();
	removeentityedit(idx);
	addentityedit(idx);
	editent(idx, true);
	clearshadowcache();
	commitchanges();
	
    return ent;
}

Entity *new_game_entity(int &idx, Entity *copy)
{
	find_next_entity_index(idx);
	auto ent = new_game_entity(true, copy->o, idx, copy->currentClassname().c_str());
	
	nlohmann::json data {};
	copy->saveToJson(data);
	ent->loadFromJson(data);
	
	return ent;
}

SCRIPTEXPORT void newgent(char *classname)
{
	int idx = 0;
	int surface_axis = dimension(sel.orient);
	int dc = dimcoord(sel.orient);
    vec selected_center(sel.o);
	selected_center.add(vec(sel.s).mul(sel.grid * 0.5f));
	vec on_surface(0,0,0);
	on_surface[surface_axis] = (sel.grid * 0.5f + entselradius) * (dc ? 1.f : -1.f);
	selected_center.add(on_surface);
    new_game_entity(true, selected_center, idx, classname);
}

int entcopygrid;
vector<Entity*> entcopybuf;

SCRIPTEXPORT void entcopy()
{
    if(noentedit()) return;
    entcopygrid = sel.grid;
    entcopybuf.shrink(0);
    addimplicit({
        loopv(entgroup) entfocus(entgroup[i], entcopybuf.emplace_back(e)->o.sub(vec(sel.o)));
    });
}

SCRIPTEXPORT void entpaste()
{
    //FIXME: Copy/Paste Entities feature
    /*if(noentedit() || entcopybuf.empty()) return;
    entcancel();
    float m = float(sel.grid)/float(entcopygrid);
    loopv(entcopybuf)
    {
        const auto c = entcopybuf[i];
        vec o = vec(c->o).mul(m).emplace_back(vec(sel.o));
        int idx = -1;
        Entity *e = new_game_entity(idx, c);
        if(!e) continue;
        e->o = o;
        entadd(idx);
        keepents = max(keepents, idx+1);
    }
    keepents = 0;
    int j = 0;
    groupeditundo(e->et_type = entcopybuf[j++]->et_type;);*/
}

SCRIPTEXPORT void entreplace()
{
    if(noentedit() || entcopybuf.empty()) return;
    const auto c = entcopybuf[0];
    if(entgroup.size() || enthover >= 0)
    {
        groupedit({
			nlohmann::json data {};
			c->saveToJson(data);
			e->loadFromJson(data);
        });
    }
    else
    {
        int idx = -1;
        new_game_entity(idx, c);
    }
}

/* Used by F3 map model menu, replaceents, enttypeselect*/
SCRIPTEXPORT void entset(char *what, int *a1, int *a2, int *a3, int *a4, int *a5)
{
    assert(false);
}

//void printent(Entity *e, char *buf, int len)
//{
//	nformatcubestr(buf, len, "%s", e->name.c_str());
//}

SCRIPTEXPORT void nearestent()
{
    if(noentedit()) return;
    int closest = -1;
    float closedist = 1e16f;
    auto &ents = getents();
    loopv(ents)
    {
        Entity *e = ents[i];
//        if(e->et_type == ET_EMPTY) continue;
        float dist = e->o.dist(player->o);
        if(dist < closedist)
        {
            closest = i;
            closedist = dist;
        }
    }
    if(closest >= 0) entadd(closest);
}

SCRIPTEXPORT void enthavesel()
{
    addimplicit(intret(entgroup.size()));
}

SCRIPTEXPORT void entselect(CommandTypes::Expression body)
{
    if(!noentedit()) addgroup(/*e->et_type != ET_EMPTY && */ entgroup.find(n)<0 && executebool(body));
}

SCRIPTEXPORT void entloop(CommandTypes::Expression body)
{
    if(!noentedit()) addimplicit(groupeditloop(((void)e, execute(body))));
}

SCRIPTEXPORT void insel()
{
    entfocus(efocus, intret(pointinsel(sel, e->o)));
}

//SCRIPTEXPORT void entget()
//{
//    entfocus(efocus, cubestr s; printent(e, s, sizeof(s)); result(s));
//}

SCRIPTEXPORT void entindex()
{
    intret(efocus);
}

SCRIPTEXPORT void enttype(char *type, CommandTypes::ArgLen numargs)
{
    if(*numargs >= 1)
    {
        int typeidx = findtype(type);
//        if(typeidx != ET_EMPTY) groupedit(e->et_type = typeidx);
    }
    else entfocus(efocus,
    {
        result(e->classname.c_str());
    })
}

SCRIPTEXPORT void entattr(int *attr, int *val, CommandTypes::ArgLen numargs)
{
    /*if(*numargs >= 2)
    {
        if(*attr >= 0 && *attr <= 4)
            groupedit(
                switch(*attr)
                {
                    case 0: e->attr1 = *val; break;
                    case 1: e->attr2 = *val; break;
                    case 2: e->attr3 = *val; break;
                    case 3: e->attr4 = *val; break;
                    case 4: e->attr5 = *val; break;
                }
            )
    }
    else entfocus(efocus,
    {
        switch(*attr)
        {
            case 0: intret(e->attr1); break;
            case 1: intret(e->attr2); break;
            case 2: intret(e->attr3); break;
            case 3: intret(e->attr4); break;
            case 4: intret(e->attr5); break;
        }
    })*/
}

// TODO: Is this still needed?
// MvK: pretty sure, no
/*
int findentity(int type, int index, int attr1, int attr2)
{
    const auto &ents = getents();
    if(index > ents.size()) index = ents.size();
    else for(int i = index; i<ents.size(); i++)
    {
        Entity *e = ents[i];
        if (e->et_type == ET_MAPMODEL && (attr1 < 0 || e->model_idx == attr1) && (attr2 < 0 || e->attr2 == attr2))
            return i;
        if(e->et_type==type && (attr1<0 || e->attr1==attr1) && (attr2<0 || e->attr2==attr2))
            return i;
    }
    loopj(index)
    {
        Entity *e = ents[j];
        if (e->et_type == ET_MAPMODEL && (attr1 < 0 || e->model_idx == attr1) && (attr2 < 0 || e->attr2 == attr2))
            return j;
        if(e->et_type==type && (attr1<0 || e->attr1==attr1) && (attr2<0 || e->attr2==attr2))
            return j;
    }
    return -1;
}*/

int findentity_byclass(const std::string &classname)
{
	const auto &ents = getents();
	for(int i = 0; i <ents.size(); i++)
	{
		if (ents[i]->classname != classname) continue;
		
		return i;
	}

	return -1;
}


// We do not need forceent = -1 anymore atm, neither do we need tag = 0 for now. But it's here for backwards reasons.
void findplayerspawn(SkeletalEntity *d, int forceent, int tag) // Place at spawn (some day, random spawn).
{
	auto startEntity = getentitybytype<PlayerSpawnEntity>();

	if (startEntity)
	{
		d->o = startEntity->o;
		d->o.z += 1;
		d->d.x = startEntity->d.x;
	}
	else
	{
		conoutf(CON_WARN, "Unable to find a PlayerStart, defaulting to mapcenter.. ");
		
		d->o.x = d->o.y = d->o.z = 0.5f*worldsize;
		d->o.z += 1;
		d->d.x = 0.0f;
	}
	
	d->resetinterp();
	entinmap(d);
}

//=====================
// WatIsDeze: Old findplayerspawn codes. We don't need these for now.
//
//=====================
//int spawncycle = -1;

//void findplayerspawn(dynent *d, int forceent, int tag) // place at random spawn
//{
//    int pick = forceent;
//    if(pick<0)
//    {
//        int r = rnd(10)+1;
//        pick = spawncycle;
//        loopi(r)
//        {
//            pick = findentity(ET_PLAYERSTART, pick+1, -1, tag);
//            if(pick < 0) break;
//        }
//        if(pick < 0 && tag)
//        {
//            pick = spawncycle;
//            loopi(r)
//            {
//                pick = findentity(ET_PLAYERSTART, pick+1, -1, 0);
//                if(pick < 0) break;
//            }
//        }
//        if(pick >= 0) spawncycle = pick;
//    }
//    if(pick>=0)
//    {
//        const vector<extentity *> &ents = getents();
//        d->d.y = 0;
//        d->d.z = 0;
//        for(int attempt = pick;;)
//        {
//            d->o = ents[attempt]->o;
//            d->d.x = ents[attempt]->attr1;
//            if(entinmap(d, true)) break;
//            attempt = findentity(ET_PLAYERSTART, attempt+1, -1, tag);
//            if(attempt<0 || attempt==pick)
//            {
//                d->o = ents[pick]->o;
//                d->d.x = ents[pick]->attr1;
//                entinmap(d);
//                break;
//            }
//        }
//    }
//    else
//    {
//        d->o.x = d->o.y = d->o.z = 0.5f*worldsize;
//        d->o.z += 1;
//        entinmap(d);
//    }
//}
//int findentity_byclass(const std::string &classname, int index, int attr1, int attr2)
//{
//    const auto &ents = getents();
//    if(index > ents.size()) index = ents.size();
//    else {
//        for(int i = index; i<ents.size(); i++)
//        {
//            Entity *e = ents[i];

//            if(e->classname == classname) {
//				conoutf("Found Entity by Class: %s , %s", classname.c_str(), e->classname.c_str());
//                return i;
//            }
//        }
//    }

//    return index;
//}

//void findplayerspawn(SkeletalEntity *d, int forceent, int tag) // place at random spawn
//{
//	int pick = forceent;
//	if(pick < 0)
//	{
//		int r = rnd(10)+1;
//		pick = spawncycle;
//		loopi(r)
//		{
//			pick = findentity_byclass("playerstart", pick+1, -1, tag);
//			if(pick < 0) break;
//		}
//		if(pick < 0 && tag)
//		{
//			pick = spawncycle;
//			loopi(r)
//			{
//				pick = findentity_byclass("playerstart", pick+1, -1, 0);
//				if(pick < 0) break;
//			}
//		}
//		if(pick >= 0) spawncycle = pick;
//	}
//	if(pick>=0)
//	{
//		const auto &ents = getents();
//		d->d.y = 0;
//		d->d.z = 0;
//		for(int attempt = pick; attempt < ents.size(); attempt++ )
//		{
//			d->o = ents[attempt]->o;
//			d->d.x = ents[attempt]->attr1;
//			if(entinmap(d, true)) break;
//			attempt = findentity_byclass("playerstart", attempt+1, -1, tag);
//			if(attempt < 0 || attempt==pick)
//			{
//				d->o = ents[pick]->o;
//				d->d.x = ents[pick]->attr1;
//				entinmap(d);
//				break;
//			}
//		}
//	}
//	else
//	{
//        d->o.x = d->o.y = d->o.z = 0.5f*worldsize;
//        d->o.z += 1;
//        entinmap(d);
//	}
//}

void splitocta(cube *c, int size)
{
    if(size <= 0x1000) return;
    loopi(8)
    {
        if(!c[i].children) c[i].children = newcubes(isempty(c[i]) ? F_EMPTY : F_SOLID);
        splitocta(c[i].children, size>>1);
    }
}

void resetmap()
{
    clearoverrides();
    clearmapsounds();
    resetblendmap();
    clearlights();
    clearpvs();
    clearslots();
    clearparticles();
    clearstains();
    clearsleep();
    cancelsel();
    pruneundos();
    clearmapcrc();

    clearents();
    outsideents.setsize(0);
    spotlights = 0;
    volumetriclights = 0;
    nospeclights = 0;
}

void startmap(const char *name)
{
    extern void mouselook(CommandTypes::Boolean);
    int on = 1;
    if (editmode)
    {
        on = 0;
    }
    mouselook(&on);

    game::startmap(name);
}

bool emptymap(int scale, bool force, const char *mname, bool usecfg)    // main empty world creation routine
{
    if(!force && !editmode)
    {
        conoutf(CON_ERROR, "newmap only allowed in edit mode");
        return false;
    }

    logoutf("reset map");
    resetmap();

    setvar("mapscale", scale<10 ? 10 : (scale>16 ? 16 : scale), true, false);
    setvar("mapsize", 1<<worldscale, true, false);
    setvar("emptymap", 1, true, false);

    texmru.shrink(0);
    logoutf("freeocta worldroot1");
    freeocta(worldroot);
    worldroot = newcubes(F_EMPTY);
    loopi(4) solidfaces(worldroot[i]);
    logoutf("worldroot = new cubes(F_EMPTY)");

    if(worldsize > 0x1000) splitocta(worldroot, worldsize>>1);

    clearmainmenu();
    logoutf("clearmenu");

    if(usecfg)
    {
        identflags |= IDF_OVERRIDDEN;
        execfile("config/default_map_settings.cfg", false);
        identflags &= ~IDF_OVERRIDDEN;
    }

    allchanged(true);
    logoutf("allchanged true");

    logoutf("startmap %s", mname);
    startmap(mname);

    return true;
}

bool enlargemap(bool force)
{
    if(!force && !editmode)
    {
        conoutf(CON_ERROR, "mapenlarge only allowed in edit mode");
        return false;
    }
    if(worldsize >= 1<<16) return false;

    while(outsideents.size()) removeentity(outsideents.pop_back());

    worldscale++;
    worldsize *= 2;
    cube *c = newcubes(F_EMPTY);
    c[0].children = worldroot;
    loopi(3) solidfaces(c[i+1]);
    worldroot = c;

    if(worldsize > 0x1000) splitocta(worldroot, worldsize>>1);

    enlargeblendmap();

    allchanged();

    return true;
}

static bool isallempty(cube &c)
{
    if(!c.children) return isempty(c);
    loopi(8) if(!isallempty(c.children[i])) return false;
    return true;
}

SCRIPTEXPORT void shrinkmap()
{
    extern int nompedit;
    if(noedit(true) || (nompedit && multiplayer())) return;
    if(worldsize <= 1<<10) return;

    int octant = -1;
    loopi(8) if(!isallempty(worldroot[i]))
    {
        if(octant >= 0) return;
        octant = i;
    }
    if(octant < 0) return;

    while(outsideents.size()) removeentity(outsideents.pop_back());

    if(!worldroot[octant].children) subdividecube(worldroot[octant], false, false);
    cube *root = worldroot[octant].children;
    worldroot[octant].children = NULL;
    freeocta(worldroot);
    worldroot = root;
    worldscale--;
    worldsize /= 2;

    ivec offset(octant, ivec(0, 0, 0), worldsize);
    auto &ents = getents();
    loopv(ents) ents[i]->o.sub(vec(offset));

    shrinkblendmap(octant);

    allchanged();

    conoutf("shrunk map to size %d", worldscale);
}

SCRIPTEXPORT void newmap(int *i) { bool force = !isconnected(); if(force) game::forceedit(""); if(emptymap(*i, force, NULL)) game::newmap(max(*i, 0)); }
SCRIPTEXPORT void mapenlarge() { if(enlargemap(false)) game::newmap(-1); }

SCRIPTEXPORT void mapname()
{
    result(game::getclientmap());
}

void mpeditent(int i, const vec &o, int type, int attr1, int attr2, int attr3, int attr4, int attr5, bool local)
{
    assert(false);
}

int getworldsize() { return worldsize; }
int getmapversion() { return mapversion; }


// >>>>>>>>>> SCRIPTBIND >>>>>>>>>>>>>> //
#if 0
#include "/Users/micha/dev/ScMaMike/src/build/binding/..+engine+world.binding.cpp"
#endif
// <<<<<<<<<< SCRIPTBIND <<<<<<<<<<<<<< //
