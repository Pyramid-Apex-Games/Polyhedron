#pragma once

struct Texture;
struct occludequery;
struct mapmodelinfo { cubestr name; model *m = nullptr, *collide = nullptr; };

extern vector<mapmodelinfo> mapmodels;

extern float transmdlsx1, transmdlsy1, transmdlsx2, transmdlsy2;
extern uint transmdltiles[LIGHTTILE_MAXH];

void loadskin(const char *dir, const char *altdir, Texture *&skin, Texture *&masks);
void resetmodelbatches();
void startmodelquery(occludequery *query);
void endmodelquery();
void rendershadowmodelbatches(bool dynmodel = true);
void shadowmaskbatchedmodels(bool dynshadow = true);
void rendermapmodelbatches();
void rendermodelbatches();
void rendertransparentmodelbatches(int stencil = 0);
void rendermapmodel(int idx, int anim, const vec &o, float yaw = 0, float pitch = 0, float roll = 0, int flags = MDL_CULL_VFC | MDL_CULL_DIST, int basetime = 0, float size = 1);
void clearbatchedmapmodels();
void preloadusedmapmodels(bool msg = false, bool bih = false);
int batcheddynamicmodels();
int batcheddynamicmodelbounds(int mask, vec &bbmin, vec &bbmax);
void cleanupmodels();

model *loadmapmodel(int n);


model *loadmapmodel(const char *filename);


// WatIsDeze: Added so we can load mapmodels by string filename
/*static inline int loadmapmodel(const char *filename, entities::classes::CoreEntity *ent)
{
    int idx = -1;

    // Check if it already exists.
    if (ent == NULL) {
        return -1;
    }

    // WatIsDeze: TODO: Check if this is actually... smart.
    if (ent->model_idx != -1) {
        loopv(mapmodels) if(!strcmp(mapmodels[i].name, filename)) {
            ent->model_idx = i;
        }
    } else {
        //loopv(mapmodels)
        if (mapmodels.inrange(e->model_idx))
            return e->model_idx;
        else
            return -1;
    }

    // MapModelInfo struct.
    mapmodelinfo &mmi = mapmodels.add();
    mmi.m = NULL;
    mmi.collide = NULL;

    // Setup the name.
    if(filename[0])
        copycubestr(mmi.name, filename);
    else
        mmi.name[0] = '\0';

    // Try and load the model.
    model *mdl = loadmodel(filename);

    if (mdl) {
        conoutf("%s %s %i", "Succesfully loaded MapModel: ", filename, mapmodels.length() - 1);
        return mapmodels.length() - 1;
    } else {
        conoutf("%s %s", "Failed to load MapModel: ", filename);
        return -1;
    }
}*/

mapmodelinfo *getmminfo(int n);
