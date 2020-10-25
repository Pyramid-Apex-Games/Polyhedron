struct cube;
struct surfaceinfo;
struct vertinfo;

struct PackNode
{
    PackNode *child1, *child2;
    ushort x, y, w, h;
    int available;

    PackNode(ushort x, ushort y, ushort w, ushort h) : child1(0), child2(0), x(x), y(y), w(w), h(h), available(min(w, h)) {}

    void discardchildren()
    {
        DELETEP(child1);
        DELETEP(child2);
    }

    void forceempty()
    {
        discardchildren();
        available = 0;
    }

    void reset()
    {
        discardchildren();
        available = min(w, h);
    }

    bool resize(int nw, int nh)
    {
        if(w == nw && h == nw) return false;
        discardchildren();
        w = nw;
        h = nh;
        available = min(w, h);
        return true;
    }

    ~PackNode()
    {
        discardchildren();
    }

    bool insert(ushort &tx, ushort &ty, ushort tw, ushort th);
    void reserve(ushort tx, ushort ty, ushort tw, ushort th);
};

extern bvec ambient, skylight, sunlight;
extern float ambientscale, skylightscale, sunlightscale;
extern float sunlightyaw, sunlightpitch;
extern vec sunlightdir;
extern int fullbright, fullbrightlevel;

void clearlights();
void initlights();
void clearlightcache(int id = -1);
void brightencube(cube &c);
void setsurfaces(cube &c, const surfaceinfo *surfs, const vertinfo *verts, int numverts);
void setsurface(cube &c, int orient, const surfaceinfo &surf, const vertinfo *verts, int numverts);
void previewblends(const ivec &bo, const ivec &bs);

void calcnormals(bool lerptjoints = false);
void clearnormals();
void resetsmoothgroups();
int smoothangle(int id, int angle);
void findnormal(const vec &key, int smooth, const vec &surface, vec &v);

#define CHECK_CALCLIGHT_PROGRESS_LOCKED(exit, show_calclight_progress, before, after) \
    if(check_calclight_progress) \
    { \
        if(!calclight_canceled) \
        { \
            before; \
            show_calclight_progress(); \
            check_calclight_canceled(); \
            after; \
        } \
        if(calclight_canceled) { exit; } \
    }
#define CHECK_CALCLIGHT_PROGRESS(exit, show_calclight_progress) CHECK_CALCLIGHT_PROGRESS_LOCKED(exit, show_calclight_progress, , )

extern bool calclight_canceled;
extern volatile bool check_calclight_progress;

void check_calclight_canceled();

const vector<int> &checklightcache(int x, int y);

