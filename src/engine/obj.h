struct obj;

struct obj : vertloader<obj>
{
    obj(const char *name) : vertloader(name) {}

    static const char *formatname() { return "obj"; }
    static bool cananimate() { return false; }
    bool flipy() const { return true; }
    int type() const { return MDL_OBJ; }

    struct objmeshgroup : vertmeshgroup
    {
        void parsevert(char *s, vector<vec> &out)
        {
            vec &v = out.emplace_back(vec(0, 0, 0));
            while(isalpha(*s)) s++;
            loopi(3)
            {
                v[i] = strtod(s, &s);
                while(isspace(*s)) s++;
                if(!*s) break;
            }
        }

        bool load(const char *filename, float smooth)
        {
            int len = strlen(filename);
            if(len < 4 || strcasecmp(&filename[len-4], ".obj")) return false;

            stream *file = openfile(filename, "rb");
            if(!file) return false;

            name = newcubestr(filename);

            numframes = 1;

            vector<vec> attrib[3];
            char buf[512];

            hashtable<ivec, int> verthash(1<<11);
            vector<vert> verts;
            vector<tcvert> tcverts;
            vector<tri> tris;

            #define FLUSHMESH do { \
                curmesh->numverts = verts.size(); \
                if(verts.size()) \
                { \
                    curmesh->verts = new vert[verts.size()]; \
                    memcpy(curmesh->verts, verts.data(), verts.size()*sizeof(vert)); \
                    curmesh->tcverts = new tcvert[verts.size()]; \
                    memcpy(curmesh->tcverts, tcverts.data(), tcverts.size()*sizeof(tcvert)); \
                } \
                curmesh->numtris = tris.size(); \
                if(tris.size()) \
                { \
                    curmesh->tris = new tri[tris.size()]; \
                    memcpy(curmesh->tris, tris.data(), tris.size()*sizeof(tri)); \
                } \
                if(attrib[2].empty()) \
                { \
                    if(smooth <= 1) curmesh->smoothnorms(smooth); \
                    else curmesh->buildnorms(); \
                } \
                curmesh->calctangents(); \
            } while(0)

            cubestr meshname = "";
            vertmesh *curmesh = NULL;
            while(file->getline(buf, sizeof(buf)))
            {
                char *c = buf;
                while(isspace(*c)) c++;
                switch(*c)
                {
                    case '#': continue;
                    case 'v':
                        if(isspace(c[1])) parsevert(c, attrib[0]);
                        else if(c[1]=='t') parsevert(c, attrib[1]);
                        else if(c[1]=='n') parsevert(c, attrib[2]);
                        break;
                    case 'g':
                    {
                        while(isalpha(*c)) c++;
                        while(isspace(*c)) c++;
                        char *name = c;
                        size_t namelen = strlen(name);
                        while(namelen > 0 && isspace(name[namelen-1])) namelen--;
                        copycubestr(meshname, name, min(namelen+1, sizeof(meshname)));

                        if(curmesh) FLUSHMESH;
                        curmesh = NULL;
                        break;
                    }
                    case 'f':
                    {
                        if(!curmesh)
                        {
                            auto m = new vertmesh;
                            m->group = this;
                            m->name = meshname[0] ? meshname : "";
                            meshes.emplace_back(m);
                            curmesh = m;

                            verthash.clear();
                            verts.setsize(0);
                            tcverts.setsize(0);
                            tris.setsize(0);
                        }
                        int v0 = -1, v1 = -1;
                        while(isalpha(*c)) c++;
                        for(;;)
                        {
                            while(isspace(*c)) c++;
                            if(!*c) break;
                            ivec vkey(-1, -1, -1);
                            loopi(3)
                            {
                                vkey[i] = strtol(c, &c, 10);
                                if(vkey[i] < 0) vkey[i] = attrib[i].size() + vkey[i];
                                else vkey[i]--;
                                if(!attrib[i].inrange(vkey[i])) vkey[i] = -1;
                                if(*c!='/') break;
                                c++;
                            }
                            int *index = verthash.access(vkey);
                            if(!index)
                            {
                                index = &verthash[vkey];
                                *index = verts.size();
                                vert &v = verts.emplace_back();
                                v.pos = vkey.x < 0 ? vec(0, 0, 0) : attrib[0][vkey.x];
                                v.pos = vec(v.pos.z, -v.pos.x, v.pos.y);
                                v.norm = vkey.z < 0 ? vec(0, 0, 0) : attrib[2][vkey.z];
                                v.norm = vec(v.norm.z, -v.norm.x, v.norm.y);
                                tcvert &tcv = tcverts.emplace_back();
                                tcv.tc = vkey.y < 0 ? vec2(0, 0) : vec2(attrib[1][vkey.y].x, 1-attrib[1][vkey.y].y);
                            }
                            if(v0 < 0) v0 = *index;
                            else if(v1 < 0) v1 = *index;
                            else
                            {
                                tri &t = tris.emplace_back();
                                t.vert[0] = ushort(*index);
                                t.vert[1] = ushort(v1);
                                t.vert[2] = ushort(v0);
                                v1 = *index;
                            }
                        }
                        break;
                    }
                }
            }

            if(curmesh) FLUSHMESH;

            delete file;

            return true;
        }
    };

    vertmeshgroup *newmeshes() { return new objmeshgroup; }

    bool loaddefaultparts()
    {
        part &mdl = addpart();
        const char *pname = parentdir(name.c_str());
        auto name1 = fmt::format("media/model/{}/tris.obj", name);
        mdl.meshes = sharemeshes(path(name1.c_str(), true));
        if(!mdl.meshes)
        {
            auto name2 = fmt::format("media/model/{}/tris.obj", pname);    // try obj in parent folder (vert sharing)
            mdl.meshes = sharemeshes(path(name2.c_str(), true));
            if(!mdl.meshes) return false;
        }
        Texture *tex, *masks;
        loadskin(name.c_str(), pname, tex, masks);
        mdl.initskins(tex, masks);
        if(tex==notexture) conoutf("could not load model skin for %s", name1.c_str());
        return true;
    }
};

vertcommands<obj> objcommands;

