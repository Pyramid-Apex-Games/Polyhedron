#pragma once
#include "shared/ents.h"
#include "shared/entities/animinfo.h"
#include "shared/geom/triangle.h"
#include "shared/geom/vec4.h"
#include "engine/bih.h"
#include <string>
enum { MDL_MD2 = 0, MDL_MD3, MDL_MD5, MDL_OBJ, MDL_SMD, MDL_IQM, NUMMODELTYPES };

struct Shader;
struct modelattach;

struct model
{
    std::string name;
    float spinyaw = 0.0f;
    float spinpitch = 0.0f;
    float spinroll = 0.0f;
    float offsetyaw = 0.0f;
    float offsetpitch = 0.0f;
    float offsetroll = 0.0f;

    bool shadow = true;
    bool alphashadow = true;
    bool depthoffset = false;

    float scale = 1.0f;
    vec translate = {0.0f, 0.0f, 0.0f};
    BIH *bih = nullptr;

    vec bbcenter = {0.0f, 0.0f, 0.0f};
    vec bbradius = {-1.0f, -1.0f, -1.0f};
    vec bbextend = {0.0f, 0.0f, 0.0f};
    vec collidecenter = {0.0f, 0.0f, 0.0f};
    vec collideradius = {-1.0f, -1.0f, -1.0f};

    float rejectradius = -1.0f;
    float eyeheight = 0.9f;
    float collidexyradius = 0.0f;
    float collideheight = 0.0f;
    std::string collidemodel;
    int collide = COLLIDE_OBB;
    int batch = -1;

    model() = default;
    model(const char *name) : model() {
        if (name)
        {
            this->name = name;
        }
    }
    virtual ~model() { DELETEP(bih); }
    virtual void calcbb(vec &center, vec &radius) = 0;
    virtual void calctransform(matrix4x3 &m) = 0;
    virtual int intersect(int anim, int basetime, int basetime2, const vec &pos, float yaw, float pitch, float roll, MovableEntity *d, modelattach *a, float size, const vec &o, const vec &ray, float &dist, int mode) = 0;
    virtual void render(int anim, int basetime, int basetime2, const vec &o, float yaw, float pitch, float roll, MovableEntity *d, modelattach *a = NULL, float size = 1, const vec4 &color = vec4(1, 1, 1, 1)) = 0;
    virtual bool load() = 0;
    virtual int type() const = 0;
    virtual BIH *setBIH() { return NULL; }
    virtual bool envmapped() const { return false; }
    virtual bool skeletal() const { return false; }
    virtual bool animated() const { return false; }
    virtual bool pitched() const { return true; }
    virtual bool alphatested() const { return false; }

    virtual void setshader(Shader *shader) {}
    virtual void setenvmap(float envmapmin, float envmapmax, Texture *envmap) {}
    virtual void setspec(float spec) {}
    virtual void setgloss(int gloss) {}
    virtual void setglow(float glow, float glowdelta, float glowpulse) {}
    virtual void setalphatest(float alpha) {}
    virtual void setfullbright(float fullbright) {}
    virtual void setcullface(int cullface) {}
    virtual void setcolor(const vec &color) {}

    virtual void genshadowmesh(vector<triangle> &tris, const matrix4x3 &orient) {}
    virtual void preloadBIH() { if(!bih) setBIH(); }
    virtual void preloadshaders() {}
    virtual void preloadmeshes() {}
    virtual void cleanup() {}

    virtual void startrender() {}
    virtual void endrender() {}

    void boundbox(vec &center, vec &radius)
    {
        if(bbradius.x < 0)
        {
            calcbb(bbcenter, bbradius);
            bbradius.add(bbextend);
        }
        center = bbcenter;
        radius = bbradius;
    }

    float collisionbox(vec &center, vec &radius)
    {
        if(collideradius.x < 0)
        {
            boundbox(collidecenter, collideradius);
            if(collidexyradius)
            {
                collidecenter.x = collidecenter.y = 0;
                collideradius.x = collideradius.y = collidexyradius;
            }
            if(collideheight)
            {
                collidecenter.z = collideradius.z = collideheight/2;
            }
            rejectradius = collideradius.magnitude();
        }
        center = collidecenter;
        radius = collideradius;
        return rejectradius;
    }

    float boundsphere(vec &center)
    {
        vec radius;
        boundbox(center, radius);
        return radius.magnitude();
    }

    float above()
    {
        vec center, radius;
        boundbox(center, radius);
        return center.z+radius.z;
    }
};

