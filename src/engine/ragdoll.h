#pragma once
#include "shared/entities/DynamicEntity.h"
#include "shared/entities/MovableEntity.h"

struct ragdollskel
{
    struct vert
    {
        vec pos;
        float radius, weight;
    };

    struct tri
    {
        int vert[3];

        bool shareverts(const tri &t) const;
    };

    struct distlimit
    {
        int vert[2];
        float mindist, maxdist;
    };

    struct rotlimit
    {
        int tri[2];
        float maxangle, maxtrace;
        matrix3 middle;
    };

    struct rotfriction
    {
        int tri[2];
        matrix3 middle;
    };

    struct joint
    {
        int bone, tri, vert[3];
        float weight;
        matrix4x3 orient;
    };

    struct reljoint
    {
        int bone, parent;
    };

    bool loaded, animjoints;
    int eye;
    vector<vert> verts;
    vector<tri> tris;
    vector<distlimit> distlimits;
    vector<rotlimit> rotlimits;
    vector<rotfriction> rotfrictions;
    vector<joint> joints;
    vector<reljoint> reljoints;

    ragdollskel();

    void setupjoints();
    void setuprotfrictions();
    void setup();
    void addreljoint(int bone, int parent);
};

struct ragdolldata
{
    struct vert
    {
        vec oldpos, pos, newpos, undo;
        float weight;
        bool collided, stuck;

        vert();
    };

    ragdollskel *skel;
    int millis, collidemillis, collisions, floating, lastmove, unsticks;
    vec offset, center;
    float radius, timestep, scale;
    vert *verts;
    matrix3 *tris;
    matrix4x3 *animjoints;
    dualquat *reljoints;

    ragdolldata(ragdollskel *skel, float scale = 1);
    ~ragdolldata();
    void calcanimjoint(int i, const matrix4x3 &anim);
    void calctris();
    void calcboundsphere();
    void init(MovableEntity *d);
    void move(MovableEntity *pl, float ts);
    void constrain();
    void constraindist();
    void applyrotlimit(ragdollskel::tri &t1, ragdollskel::tri &t2, float angle, const vec &axis);
    void constrainrot();
    void calcrotfriction();
    void applyrotfriction(float ts);
    void tryunstick(float speed);

    static inline bool collidevert(const vec &pos, const vec &dir, float radius);
};

/*
    seed particle position = avg(modelview * base2anim * spherepos)
    mapped transform = invert(curtri) * origtrig
    parented transform = parent{invert(curtri) * origtrig} * (invert(parent{base2anim}) * base2anim)
*/

void moveragdoll(MovableEntity *d);
void cleanragdoll(MovableEntity *d);
