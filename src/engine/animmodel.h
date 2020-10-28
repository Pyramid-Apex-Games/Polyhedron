#include "engine/model.h"
#include "engine/texture.h"
#include "engine/renderlights.h"
#include "shared/stream.h"
#include <fmt/format.h>


//extern from command.h
extern int identflags;

extern matrix4 camprojmatrix;

void enablepolygonoffset(GLenum type);
void disablepolygonoffset(GLenum type);

struct animinfo;

struct animmodel : model
{
    struct animspec
    {
        int frame = 0, range = 0;
        float speed = 0.0f;
        int priority = 0;
    };

    struct animpos
    {
        int anim = 0, fr1 = 0, fr2 = 0;
        float t = 0.0f;

        void setframes(const animinfo &info);
        bool operator==(const animpos &a) const;
        bool operator!=(const animpos &a) const;
    };

    struct part;

    struct animstate
    {
        part *owner = nullptr;
        animpos cur, prev;
        float interp = 0.0f;

        bool operator==(const animstate &a) const;
        bool operator!=(const animstate &a) const;
    };

    struct linkedpart;
    struct mesh;

    struct shaderparams
    {
        float spec = 1.0f;
        float gloss = 1.0f;
        float glow = 3.0f;
        float glowdelta = 0.0f;
        float glowpulse = 0.0f;
        float fullbright = 0.0f;
        float envmapmin = 0.0f;
        float envmapmax = 0.0f;
        float scrollu = 0.0f;
        float scrollv = 0.0f;
        float alphatest = 0.9f;
        vec color = {1.0f, 1.0f, 1.0f};
    };

    struct shaderparamskey
    {
        static hashtable<shaderparams, shaderparamskey> keys;
        static int firstversion, lastversion;

        int version = -1;

        bool checkversion();

        static inline void invalidate()
        {
            firstversion = lastversion;
        }
    };

    struct skin : shaderparams
    {
        part *owner = nullptr;
        Texture *tex = notexture, *decal = nullptr, *masks = notexture, *envmap = nullptr, *normalmap = nullptr;
        Shader *shader = nullptr, *rsmshader = nullptr;
        int cullface = 1;
        shaderparamskey *key = nullptr;

        bool masked() const;
        bool envmapped() const;
        bool bumpmapped() const;
        bool alphatested() const;
        bool decaled() const;

        void setkey();

        void setshaderparams(mesh &m, const animstate *as, bool skinned = true);

        Shader *loadshader();
        void cleanup();
        void preloadBIH();
        void preloadshader();
        void setshader(mesh &m, const animstate *as);
        void bind(mesh &b, const animstate *as);
    };

    struct meshgroup;

    struct mesh
    {
        meshgroup *group = nullptr;
        std::string name;
        bool cancollide = true;
        bool canrender = true;
        bool noclip = false;

        mesh() = default;
        virtual ~mesh() = default;

        virtual void calcbb(vec &bbmin, vec &bbmax, const matrix4x3 &m) {}
        virtual void genBIH(BIH::mesh &m) {}
        virtual void genshadowmesh(vector<triangle> &tris, const matrix4x3 &m) {}
        void genBIH(skin &s, vector<BIH::mesh> &bih, const matrix4x3 &t);
        virtual void setshader(Shader *s, int row = 0);

        struct smoothdata
        {
            vec norm = {0.0f, 0.0f, 0.0f};
            int next = -1;
        };

        template<class V, class T> void smoothnorms(V *verts, int numverts, T *tris, int numtris, float limit, bool areaweight)
        {
            if(!numverts) return;
            smoothdata *smooth = new smoothdata[numverts];
            hashtable<vec, int> share;
            loopi(numverts)
            {
                V &v = verts[i];
                int &idx = share.access(v.pos, i);
                if(idx != i) { smooth[i].next = idx; idx = i; }
            }
            loopi(numtris)
            {
                T &t = tris[i];
                int v1 = t.vert[0], v2 = t.vert[1], v3 = t.vert[2];
                vec norm;
                norm.cross(verts[v1].pos, verts[v2].pos, verts[v3].pos);
                if(!areaweight) norm.normalize();
                smooth[v1].norm.add(norm);
                smooth[v2].norm.add(norm);
                smooth[v3].norm.add(norm);
            }
            loopi(numverts) verts[i].norm = vec(0, 0, 0);
            loopi(numverts)
            {
                const smoothdata &n = smooth[i];
                verts[i].norm.add(n.norm);
                if(n.next >= 0)
                {
                    float vlimit = limit*n.norm.magnitude();
                    for(int j = n.next; j >= 0;)
                    {
                        const smoothdata &o = smooth[j];
                        if(n.norm.dot(o.norm) >= vlimit*o.norm.magnitude())
                        {
                            verts[i].norm.add(o.norm);
                            verts[j].norm.add(n.norm);
                        }
                        j = o.next;
                    }
                }
            }
            loopi(numverts) verts[i].norm.normalize();
            delete[] smooth;
        }

        template<class V, class T> void buildnorms(V *verts, int numverts, T *tris, int numtris, bool areaweight)
        {
            if(!numverts) return;
            loopi(numverts) verts[i].norm = vec(0, 0, 0);
            loopi(numtris)
            {
                T &t = tris[i];
                V &v1 = verts[t.vert[0]], &v2 = verts[t.vert[1]], &v3 = verts[t.vert[2]];
                vec norm;
                norm.cross(v1.pos, v2.pos, v3.pos);
                if(!areaweight) norm.normalize();
                v1.norm.add(norm);
                v2.norm.add(norm);
                v3.norm.add(norm);
            }
            loopi(numverts) verts[i].norm.normalize();
        }

        template<class V, class T> void buildnorms(V *verts, int numverts, T *tris, int numtris, bool areaweight, int numframes)
        {
            if(!numverts) return;
            loopi(numframes) buildnorms(&verts[i*numverts], numverts, tris, numtris, areaweight);
        }

        static inline void fixqtangent(quat &q, float bt)
        {
            static const float bias = -1.5f/65535, biasscale = sqrtf(1 - bias*bias);
            if(bt < 0)
            {
                if(q.w >= 0) q.neg();
                if(q.w > bias) { q.mul3(biasscale); q.w = bias; }
            }
            else if(q.w < 0) q.neg();
        }

        template<class V> static inline void calctangent(V &v, const vec &n, const vec &t, float bt)
        {
            matrix3 m;
            m.c = n;
            m.a = t;
            m.b.cross(m.c, m.a);
            quat q(m);
            fixqtangent(q, bt);
            v.tangent = q;
        }

        template<class V, class TC, class T> void calctangents(V *verts, TC *tcverts, int numverts, T *tris, int numtris, bool areaweight)
        {
            vec *tangent = new vec[2*numverts], *bitangent = tangent+numverts;
            memset(tangent, 0, 2*numverts*sizeof(vec));
            loopi(numtris)
            {
                const T &t = tris[i];
                const vec &e0 = verts[t.vert[0]].pos;
                vec e1 = vec(verts[t.vert[1]].pos).sub(e0), e2 = vec(verts[t.vert[2]].pos).sub(e0);

                const vec2 &tc0 = tcverts[t.vert[0]].tc,
                           &tc1 = tcverts[t.vert[1]].tc,
                           &tc2 = tcverts[t.vert[2]].tc;
                float u1 = tc1.x - tc0.x, v1 = tc1.y - tc0.y,
                      u2 = tc2.x - tc0.x, v2 = tc2.y - tc0.y;
                vec u(e2), v(e2);
                u.mul(v1).sub(vec(e1).mul(v2));
                v.mul(u1).sub(vec(e1).mul(u2));

                if(vec().cross(e2, e1).dot(vec().cross(v, u)) >= 0)
                {
                    u.neg();
                    v.neg();
                }

                if(!areaweight)
                {
                    u.normalize();
                    v.normalize();
                }

                loopj(3)
                {
                    tangent[t.vert[j]].sub(u);
                    bitangent[t.vert[j]].add(v);
                }
            }
            loopi(numverts)
            {
                V &v = verts[i];
                const vec &t = tangent[i],
                          &bt = bitangent[i];
                matrix3 m;
                m.c = v.norm;
                (m.a = t).project(m.c).normalize();
                m.b.cross(m.c, m.a);
                quat q(m);
                fixqtangent(q, m.b.dot(bt));
                v.tangent = q;
            }
            delete[] tangent;
        }

        template<class V, class TC, class T> void calctangents(V *verts, TC *tcverts, int numverts, T *tris, int numtris, bool areaweight, int numframes)
        {
            loopi(numframes) calctangents(&verts[i*numverts], tcverts, numverts, tris, numtris, areaweight);
        }
    };

    struct meshgroup
    {
        meshgroup *next = nullptr;
        int shared = 0;
        std::string name;
        vector<mesh *> meshes;

        virtual ~meshgroup();
        virtual int findtag(const char *name);
        virtual void concattagtransform(part *p, int i, const matrix4x3 &m, matrix4x3 &n);

        #define looprendermeshes(type, name, body) do { \
            loopv(meshes) \
            { \
                type &name = *(type *)meshes[i]; \
                if(name.canrender || dbgcolmesh) { body; } \
            } \
        } while(0)

        template <class T>
        using forEachMeshIteration = std::function<void(T&)>;

        template <class T>
        using forEachMeshIterationIndexed = std::function<void(T&, int)>;

        template <class T>
        void forEachRenderMesh(const forEachMeshIterationIndexed<T>& iteration)
        {
            extern int dbgcolmesh;

//            for (auto mesh : meshes)
            for (int i = 0; i < meshes.size(); i++)
            {
                auto& mesh = meshes[i];

                T* meshT = static_cast<T*>(mesh);

                if (meshT->canrender || dbgcolmesh)
                {
                    iteration(*meshT, i);
                }
            }
        }

        template <class T>
        void forEachRenderMesh(const forEachMeshIteration<T>& iteration)
        {
            forEachRenderMesh<T>([&](T& t, int){ iteration(t); });
        }

        void calcbb(vec &bbmin, vec &bbmax, const matrix4x3 &t);
        void genBIH(vector<skin> &skins, vector<BIH::mesh> &bih, const matrix4x3 &t);
        void genshadowmesh(vector<triangle> &tris, const matrix4x3 &t);

        virtual void *animkey();
        virtual int totalframes() const;
        bool hasframe(int i) const;
        bool hasframes(int i, int n) const;
        int clipframes(int i, int n) const;

        virtual void cleanup();
        virtual void preload(part *p);
        virtual void render(const animstate *as, float pitch, const vec &axis, const vec &forward, MovableEntity *d, part *p);
        virtual void intersect(const animstate *as, float pitch, const vec &axis, const vec &forward, MovableEntity *d, part *p, const vec &o, const vec &ray);

        void bindpos(GLuint ebuf, GLuint vbuf, void *v, int stride, int type, int size);
        void bindpos(GLuint ebuf, GLuint vbuf, vec *v, int stride);
        void bindpos(GLuint ebuf, GLuint vbuf, hvec4 *v, int stride);

        void bindtc(void *v, int stride);
        void bindtangents(void *v, int stride);
        void bindbones(void *wv, void *bv, int stride);
    };

    static hashnameset<meshgroup *> meshgroups;

    struct linkedpart
    {
        part *p = nullptr;
        int tag = -1, anim = -1, basetime = 0;
        vec translate = {0.0f, 0.0f, 0.0f};
        vec *pos = nullptr;
        matrix4 matrix;
    };

    struct part
    {
        animmodel *model = nullptr;
        int index = 0;
        meshgroup *meshes = nullptr;
        vector<linkedpart> links;
        vector<skin> skins;
        vector<animspec> *anims[MAXANIMPARTS];
        int numanimparts = 1;
        float pitchscale = 1.0f, pitchoffset = 0.0f, pitchmin = 0.0f, pitchmax = 0.0f;

        part(animmodel *model, int index = 0);
        virtual ~part();

        virtual void cleanup();
        void disablepitch();
        void calcbb(vec &bbmin, vec &bbmax, const matrix4x3 &m);
        void genBIH(vector<BIH::mesh> &bih, const matrix4x3 &m);
        void genshadowmesh(vector<triangle> &tris, const matrix4x3 &m);
        bool link(part *p, const char *tag, const vec &translate = vec(0, 0, 0), int anim = -1, int basetime = 0, vec *pos = nullptr);
        bool unlink(part *p);
        void initskins(Texture *tex = notexture, Texture *masks = notexture, int limit = 0);
        bool alphatested() const;
        void preloadBIH();
        void preloadshaders();
        void preloadmeshes();
        virtual void getdefaultanim(animinfo &info, int anim, uint varseed, MovableEntity *d);
        bool calcanim(int animpart, int anim, int basetime, int basetime2, MovableEntity *d, int interp, animinfo &info, int &animinterptime);
        void intersect(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, const vec &o, const vec &ray);
        void intersect(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, const vec &o, const vec &ray, animstate *as);
        void render(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d);
        void render(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, animstate *as);
        void setanim(int animpart, int num, int frame, int range, float speed, int priority = 0);
        bool animated() const;
        virtual void loaded();
    };

    enum
    {
        LINK_TAG = 0,
        LINK_COOP,
        LINK_REUSE
    };

    virtual int linktype(animmodel *m, part *p) const;
    void intersect(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, modelattach *a, const vec &o, const vec &ray);

    static int intersectresult, intersectmode;
    static float intersectdist, intersectscale;

    int intersect(int anim, int basetime, int basetime2, const vec &pos, float yaw, float pitch, float roll, MovableEntity *d, modelattach *a, float size, const vec &o, const vec &ray, float &dist, int mode);
    void render(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, modelattach *a);
    void render(int anim, int basetime, int basetime2, const vec &o, float yaw, float pitch, float roll, MovableEntity *d, modelattach *a, float size, const vec4 &color);

    vector<part *> parts;

    animmodel(const char *name);
    virtual ~animmodel();
    void cleanup();
    virtual void flushpart();
    part &addpart();
    void initmatrix(matrix4x3 &m);
    void genBIH(vector<BIH::mesh> &bih);
    void genshadowmesh(vector<triangle> &tris, const matrix4x3 &orient);
    void preloadBIH();
    BIH *setBIH();
    bool link(part *p, const char *tag, const vec &translate = vec(0, 0, 0), int anim = -1, int basetime = 0, vec *pos = nullptr);
    bool unlink(part *p);
    bool envmapped() const;
    bool animated() const;
    bool pitched() const;
    bool alphatested() const;
    virtual bool flipy() const;
    virtual bool loadconfig();
    virtual bool loaddefaultparts();
    virtual void startload();
    virtual void endload();

    bool load();
    void preloadshaders();
    void preloadmeshes();
    void setshader(Shader *shader);
    void setenvmap(float envmapmin, float envmapmax, Texture *envmap);
    void setspec(float spec);
    void setgloss(int gloss);
    void setglow(float glow, float delta, float pulse);
    void setalphatest(float alphatest);
    void setfullbright(float fullbright);
    void setcullface(int cullface);
    void setcolor(const vec &color);
    void calcbb(vec &center, vec &radius);
    void calctransform(matrix4x3 &m);
    virtual void loaded();

    static bool enabletc, enablecullface, enabletangents, enablebones, enabledepthoffset;
    static float sizescale;
    static vec4 colorscale;
    static GLuint lastvbuf, lasttcbuf, lastxbuf, lastbbuf, lastebuf, lastenvmaptex, closestenvmaptex;
    static Texture *lasttex, *lastdecal, *lastmasks, *lastnormalmap;
    static int matrixpos;
    static matrix4 matrixstack[64];

    void startrender()
    {
        enabletc = enabletangents = enablebones = enabledepthoffset = false;
        enablecullface = true;
        lastvbuf = lasttcbuf = lastxbuf = lastbbuf = lastebuf = lastenvmaptex = closestenvmaptex = 0;
        lasttex = lastdecal = lastmasks = lastnormalmap = nullptr;
        shaderparamskey::invalidate();
    }

    static void disablebones()
    {
        gle::disableboneweight();
        gle::disableboneindex();
        enablebones = false;
    }

    static void disabletangents()
    {
        gle::disabletangent();
        enabletangents = false;
    }

    static void disabletc()
    {
        gle::disabletexcoord0();
        enabletc = false;
    }

    static void disablevbo()
    {
        if(lastebuf) gle::clearebo();
        if(lastvbuf)
        {
            gle::clearvbo();
            gle::disablevertex();
        }
        if(enabletc) disabletc();
        if(enabletangents) disabletangents();
        if(enablebones) disablebones();
        lastvbuf = lasttcbuf = lastxbuf = lastbbuf = lastebuf = 0;
    }

    void endrender()
    {
        if(lastvbuf || lastebuf) disablevbo();
        if(!enablecullface) glEnable(GL_CULL_FACE);
        if(enabledepthoffset) disablepolygonoffset(GL_POLYGON_OFFSET_FILL);
    }
};

static inline uint hthash(const animmodel::shaderparams &k)
{
    return memhash(&k, sizeof(k));
}

static inline bool htcmp(const animmodel::shaderparams &x, const animmodel::shaderparams &y)
{
    return !memcmp(&x, &y, sizeof(animmodel::shaderparams));
}

template<class MDL, class BASE> struct modelloader : BASE
{
    static MDL *loading;
    static std::string dir;

    modelloader(const char *name) : BASE(name) {}

    static bool cananimate() { return true; }
    static bool multiparted() { return true; }
    static bool multimeshed() { return true; }

    void startload()
    {
        loading = (MDL *)this;
    }

    void endload()
    {
        loading = NULL;
    }

    bool loadconfig()
    {
        dir = fmt::format("media/model/{}", BASE::name).c_str();
        auto cfgname = fmt::format("media/model/{}/{}.cfg", BASE::name, MDL::formatname());

        identflags &= ~IDF_PERSIST;
        bool success = execfile(cfgname.c_str(), false);
        identflags |= IDF_PERSIST;
        return success;
    }
};

template<class MDL, class BASE> MDL *modelloader<MDL, BASE>::loading = nullptr;
template<class MDL, class BASE> std::string modelloader<MDL, BASE>::dir;

template<class MDL, class MESH> struct modelcommands
{
    typedef struct MDL::part part;
    typedef struct MDL::skin skin;

    static void setdir(char *name)
    {
        if(!MDL::loading)
        {
            conoutf(CON_ERROR, "model::setdir: not loading an %s", MDL::formatname());
            return;
        }
        MDL::dir = fmt::format("media/model/{}", name);
    }

    static void ForEachMesh(const std::string& meshname, const std::function<void(MESH&)>& forEachCallback)
    {
        if (!MDL::loading || MDL::loading->parts.empty())
        {
            conoutf(CON_ERROR, "ForEachMesh: not loading an %s", MDL::formatname());
            return;
        }

        part &mdl = *MDL::loading->parts.back();
        if(!mdl.meshes) return;

        const bool wildcard = meshname == "*";

        for(int i = 0; i < mdl.meshes->meshes.size(); ++i)
        {
            auto& mesh = mdl.meshes->meshes[i];
            if (wildcard || mesh->name == meshname)
            {
                forEachCallback(*static_cast<MESH*>(mesh));
            }
        }
    }

    static void ForEachSkin(const std::string& meshname, const std::function<void(skin&)>& forEachCallback)
    {
        if (!MDL::loading || MDL::loading->parts.empty())
        {
            conoutf(CON_ERROR, "ForEachSkin: not loading an %s", MDL::formatname());
            return;
        }

        part &mdl = *MDL::loading->parts.back();
        if(!mdl.meshes) return;

        const bool wildcard = meshname == "*";

        for(int i = 0; i < mdl.skins.size() && i < mdl.meshes->meshes.size(); ++i)
        {
            auto& skin = mdl.skins[i];
            auto& mesh = mdl.meshes->meshes[i];
            if (wildcard || mesh->name == meshname)
            {
                forEachCallback(skin);
            }
        }
    }

//    #define loopmeshes(meshname, m, body) do { \
//        if(!MDL::loading || MDL::loading->parts.empty()) { conoutf(CON_ERROR, "loopmeshes: not loading an %s", MDL::formatname()); return; } \
//        part &mdl = *MDL::loading->parts.last(); \
//        if(!mdl.meshes) return; \
//        loopv(mdl.meshes->meshes) \
//        { \
//            MESH &m = *(MESH *)mdl.meshes->meshes[i]; \
//            if(meshname == "*" || m.name == meshname) \
//            { \
//                body; \
//            } \
//        } \
//    } while(0)
//
//    #define loopskins(meshname, s, body) loopmeshes(meshname, m, { skin &s = mdl.skins[i]; body; })

    static void setskin(char *meshname, char *tex, char *masks, float *envmapmax, float *envmapmin)
    {
        ForEachSkin(meshname, [&](skin& s)
        {
            s.tex = textureload(makerelpath(MDL::dir.c_str(), tex), 0, true, false);
            if(*masks)
            {
                s.masks = textureload(makerelpath(MDL::dir.c_str(), masks), 0, true, false);
                s.envmapmax = *envmapmax;
                s.envmapmin = *envmapmin;
            }
        });
    }

    static void setspec(char *meshname, float *percent)
    {
        float spec = *percent > 0 ? *percent/100.0f : 0.0f;
        ForEachSkin(meshname, [&](skin& s)
        {
            s.spec = spec;
        });
    }

    static void setgloss(char *meshname, int *gloss)
    {
        ForEachSkin(meshname, [&](skin& s)
        {
            s.gloss = clamp(*gloss, 0, 2);
        });
    }

    static void setglow(char *meshname, float *percent, float *delta, float *pulse)
    {
        float glow = *percent > 0 ? *percent/100.0f : 0.0f;
        float glowdelta = *delta/100.0f;
        float glowpulse = *pulse > 0 ? *pulse/1000.0f : 0;
        glowdelta -= glow;
        ForEachSkin(meshname, [&](skin& s)
        {
            s.glow = glow;
            s.glowdelta = glowdelta;
            s.glowpulse = glowpulse;
        });
    }

    static void setalphatest(char *meshname, float *cutoff)
    {
        ForEachSkin(meshname, [&](skin& s)
        {
            s.alphatest = max(0.0f, min(1.0f, *cutoff));
        });
    }

    static void setcullface(char *meshname, int *cullface)
    {
        ForEachSkin(meshname, [&](skin& s)
        {
            s.cullface = *cullface;
        });
    }

    static void setcolor(char *meshname, float *r, float *g, float *b)
    {
        ForEachSkin(meshname, [&](skin& s)
        {
            s.color = vec(*r, *g, *b);
        });
    }

    static void setenvmap(char *meshname, char *envmap)
    {
        Texture *tex = cubemapload(envmap);
        ForEachSkin(meshname, [&](skin& s)
        {
            s.envmap = tex;
        });
    }

    static void setbumpmap(char *meshname, char *normalmapfile)
    {
        Texture *normalmaptex = textureload(makerelpath(MDL::dir.c_str(), normalmapfile), 0, true, false);
        ForEachSkin(meshname, [&](skin& s)
        {
            s.normalmap = normalmaptex;
        });
    }

    static void setdecal(char *meshname, char *decal)
    {
        ForEachSkin(meshname, [&](skin& s)
        {
            s.decal = textureload(makerelpath(MDL::dir.c_str(), decal), 0, true, false);
        });
    }

    static void setfullbright(char *meshname, float *fullbright)
    {
        ForEachSkin(meshname, [&](skin& s)
        {
            s.fullbright = *fullbright;
        });
    }

    static void setshader(char *meshname, char *shader)
    {
        ForEachSkin(meshname, [&](skin& s)
        {
            s.shader = lookupshaderbyname(shader);
        });
    }

    static void setscroll(char *meshname, float *scrollu, float *scrollv)
    {
        ForEachSkin(meshname, [&](skin& s)
        {
            s.scrollu = *scrollu;
            s.scrollv = *scrollv;
        });
    }

    static void setnoclip(char *meshname, int *noclip)
    {
        ForEachMesh(meshname, [&](MESH& m)
        {
            m.noclip = *noclip!=0;
        });
    }

    static void settricollide(char *meshname)
    {
        bool init = true;
        ForEachMesh(meshname, [&](MESH& m)
        {
            if(!m.cancollide)
            {
                init = false;
            }
        });

        if(init)
        {
            ForEachMesh("*", [](MESH& m)
            {
                m.cancollide = false;
            });
        }

        ForEachMesh(meshname, [](MESH& m)
        {
            m.cancollide = true;
            m.canrender = false;
        });

        MDL::loading->collide = COLLIDE_TRI;
    }

    static void setlink(int *parent, int *child, char *tagname, float *x, float *y, float *z)
    {
        if(!MDL::loading)
        {
            conoutf(CON_ERROR, "skelmodel: not loading an %s", MDL::formatname());
            return;
        }
        if(!MDL::loading->parts.inrange(*parent) || !MDL::loading->parts.inrange(*child))
        {
            conoutf(CON_ERROR, "no models loaded to link");
            return;
        }
        if(!MDL::loading->parts[*parent]->link(MDL::loading->parts[*child], tagname, vec(*x, *y, *z)))
        {
            conoutf(CON_ERROR, "could not link model %s", MDL::loading->name.c_str());
        }
    }

    template<class F> void modelcommand(F *fun, const char *suffix, const char *args)
    {
        auto name = fmt::format("{}{}", MDL::formatname(), suffix);
        addcommand(newcubestr(name.c_str()), (identfun)fun, args);
    }

    modelcommands()
    {
        modelcommand(setdir, "dir", "s");
        if(MDL::multimeshed())
        {
            modelcommand(setskin, "skin", "sssff");
            modelcommand(setspec, "spec", "sf");
            modelcommand(setgloss, "gloss", "si");
            modelcommand(setglow, "glow", "sfff");
            modelcommand(setalphatest, "alphatest", "sf");
            modelcommand(setcullface, "cullface", "si");
            modelcommand(setcolor, "color", "sfff");
            modelcommand(setenvmap, "envmap", "ss");
            modelcommand(setbumpmap, "bumpmap", "ss");
            modelcommand(setdecal, "decal", "ss");
            modelcommand(setfullbright, "fullbright", "sf");
            modelcommand(setshader, "shader", "ss");
            modelcommand(setscroll, "scroll", "sff");
            modelcommand(setnoclip, "noclip", "si");
            modelcommand(settricollide, "tricollide", "s");
        }
        if(MDL::multiparted()) modelcommand(setlink, "link", "iisfff");
    }
};

