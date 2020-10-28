//extern from command.h
extern int identflags;

struct md2;

static const float md2normaltable[256][3] =
{
    { -0.525731f,  0.000000f,  0.850651f },     { -0.442863f,  0.238856f,  0.864188f },     { -0.295242f,  0.000000f,  0.955423f },     { -0.309017f,  0.500000f,  0.809017f },
    { -0.162460f,  0.262866f,  0.951056f },     {  0.000000f,  0.000000f,  1.000000f },     {  0.000000f,  0.850651f,  0.525731f },     { -0.147621f,  0.716567f,  0.681718f },
    {  0.147621f,  0.716567f,  0.681718f },     {  0.000000f,  0.525731f,  0.850651f },     {  0.309017f,  0.500000f,  0.809017f },     {  0.525731f,  0.000000f,  0.850651f },
    {  0.295242f,  0.000000f,  0.955423f },     {  0.442863f,  0.238856f,  0.864188f },     {  0.162460f,  0.262866f,  0.951056f },     { -0.681718f,  0.147621f,  0.716567f },
    { -0.809017f,  0.309017f,  0.500000f },     { -0.587785f,  0.425325f,  0.688191f },     { -0.850651f,  0.525731f,  0.000000f },     { -0.864188f,  0.442863f,  0.238856f },
    { -0.716567f,  0.681718f,  0.147621f },     { -0.688191f,  0.587785f,  0.425325f },     { -0.500000f,  0.809017f,  0.309017f },     { -0.238856f,  0.864188f,  0.442863f },
    { -0.425325f,  0.688191f,  0.587785f },     { -0.716567f,  0.681718f, -0.147621f },     { -0.500000f,  0.809017f, -0.309017f },     { -0.525731f,  0.850651f,  0.000000f },
    {  0.000000f,  0.850651f, -0.525731f },     { -0.238856f,  0.864188f, -0.442863f },     {  0.000000f,  0.955423f, -0.295242f },     { -0.262866f,  0.951056f, -0.162460f },
    {  0.000000f,  1.000000f,  0.000000f },     {  0.000000f,  0.955423f,  0.295242f },     { -0.262866f,  0.951056f,  0.162460f },     {  0.238856f,  0.864188f,  0.442863f },
    {  0.262866f,  0.951056f,  0.162460f },     {  0.500000f,  0.809017f,  0.309017f },     {  0.238856f,  0.864188f, -0.442863f },     {  0.262866f,  0.951056f, -0.162460f },
    {  0.500000f,  0.809017f, -0.309017f },     {  0.850651f,  0.525731f,  0.000000f },     {  0.716567f,  0.681718f,  0.147621f },     {  0.716567f,  0.681718f, -0.147621f },
    {  0.525731f,  0.850651f,  0.000000f },     {  0.425325f,  0.688191f,  0.587785f },     {  0.864188f,  0.442863f,  0.238856f },     {  0.688191f,  0.587785f,  0.425325f },
    {  0.809017f,  0.309017f,  0.500000f },     {  0.681718f,  0.147621f,  0.716567f },     {  0.587785f,  0.425325f,  0.688191f },     {  0.955423f,  0.295242f,  0.000000f },
    {  1.000000f,  0.000000f,  0.000000f },     {  0.951056f,  0.162460f,  0.262866f },     {  0.850651f, -0.525731f,  0.000000f },     {  0.955423f, -0.295242f,  0.000000f },
    {  0.864188f, -0.442863f,  0.238856f },     {  0.951056f, -0.162460f,  0.262866f },     {  0.809017f, -0.309017f,  0.500000f },     {  0.681718f, -0.147621f,  0.716567f },
    {  0.850651f,  0.000000f,  0.525731f },     {  0.864188f,  0.442863f, -0.238856f },     {  0.809017f,  0.309017f, -0.500000f },     {  0.951056f,  0.162460f, -0.262866f },
    {  0.525731f,  0.000000f, -0.850651f },     {  0.681718f,  0.147621f, -0.716567f },     {  0.681718f, -0.147621f, -0.716567f },     {  0.850651f,  0.000000f, -0.525731f },
    {  0.809017f, -0.309017f, -0.500000f },     {  0.864188f, -0.442863f, -0.238856f },     {  0.951056f, -0.162460f, -0.262866f },     {  0.147621f,  0.716567f, -0.681718f },
    {  0.309017f,  0.500000f, -0.809017f },     {  0.425325f,  0.688191f, -0.587785f },     {  0.442863f,  0.238856f, -0.864188f },     {  0.587785f,  0.425325f, -0.688191f },
    {  0.688191f,  0.587785f, -0.425325f },     { -0.147621f,  0.716567f, -0.681718f },     { -0.309017f,  0.500000f, -0.809017f },     {  0.000000f,  0.525731f, -0.850651f },
    { -0.525731f,  0.000000f, -0.850651f },     { -0.442863f,  0.238856f, -0.864188f },     { -0.295242f,  0.000000f, -0.955423f },     { -0.162460f,  0.262866f, -0.951056f },
    {  0.000000f,  0.000000f, -1.000000f },     {  0.295242f,  0.000000f, -0.955423f },     {  0.162460f,  0.262866f, -0.951056f },     { -0.442863f, -0.238856f, -0.864188f },
    { -0.309017f, -0.500000f, -0.809017f },     { -0.162460f, -0.262866f, -0.951056f },     {  0.000000f, -0.850651f, -0.525731f },     { -0.147621f, -0.716567f, -0.681718f },
    {  0.147621f, -0.716567f, -0.681718f },     {  0.000000f, -0.525731f, -0.850651f },     {  0.309017f, -0.500000f, -0.809017f },     {  0.442863f, -0.238856f, -0.864188f },
    {  0.162460f, -0.262866f, -0.951056f },     {  0.238856f, -0.864188f, -0.442863f },     {  0.500000f, -0.809017f, -0.309017f },     {  0.425325f, -0.688191f, -0.587785f },
    {  0.716567f, -0.681718f, -0.147621f },     {  0.688191f, -0.587785f, -0.425325f },     {  0.587785f, -0.425325f, -0.688191f },     {  0.000000f, -0.955423f, -0.295242f },
    {  0.000000f, -1.000000f,  0.000000f },     {  0.262866f, -0.951056f, -0.162460f },     {  0.000000f, -0.850651f,  0.525731f },     {  0.000000f, -0.955423f,  0.295242f },
    {  0.238856f, -0.864188f,  0.442863f },     {  0.262866f, -0.951056f,  0.162460f },     {  0.500000f, -0.809017f,  0.309017f },     {  0.716567f, -0.681718f,  0.147621f },
    {  0.525731f, -0.850651f,  0.000000f },     { -0.238856f, -0.864188f, -0.442863f },     { -0.500000f, -0.809017f, -0.309017f },     { -0.262866f, -0.951056f, -0.162460f },
    { -0.850651f, -0.525731f,  0.000000f },     { -0.716567f, -0.681718f, -0.147621f },     { -0.716567f, -0.681718f,  0.147621f },     { -0.525731f, -0.850651f,  0.000000f },
    { -0.500000f, -0.809017f,  0.309017f },     { -0.238856f, -0.864188f,  0.442863f },     { -0.262866f, -0.951056f,  0.162460f },     { -0.864188f, -0.442863f,  0.238856f },
    { -0.809017f, -0.309017f,  0.500000f },     { -0.688191f, -0.587785f,  0.425325f },     { -0.681718f, -0.147621f,  0.716567f },     { -0.442863f, -0.238856f,  0.864188f },
    { -0.587785f, -0.425325f,  0.688191f },     { -0.309017f, -0.500000f,  0.809017f },     { -0.147621f, -0.716567f,  0.681718f },     { -0.425325f, -0.688191f,  0.587785f },
    { -0.162460f, -0.262866f,  0.951056f },     {  0.442863f, -0.238856f,  0.864188f },     {  0.162460f, -0.262866f,  0.951056f },     {  0.309017f, -0.500000f,  0.809017f },
    {  0.147621f, -0.716567f,  0.681718f },     {  0.000000f, -0.525731f,  0.850651f },     {  0.425325f, -0.688191f,  0.587785f },     {  0.587785f, -0.425325f,  0.688191f },
    {  0.688191f, -0.587785f,  0.425325f },     { -0.955423f,  0.295242f,  0.000000f },     { -0.951056f,  0.162460f,  0.262866f },     { -1.000000f,  0.000000f,  0.000000f },
    { -0.850651f,  0.000000f,  0.525731f },     { -0.955423f, -0.295242f,  0.000000f },     { -0.951056f, -0.162460f,  0.262866f },     { -0.864188f,  0.442863f, -0.238856f },
    { -0.951056f,  0.162460f, -0.262866f },     { -0.809017f,  0.309017f, -0.500000f },     { -0.864188f, -0.442863f, -0.238856f },     { -0.951056f, -0.162460f, -0.262866f },
    { -0.809017f, -0.309017f, -0.500000f },     { -0.681718f,  0.147621f, -0.716567f },     { -0.681718f, -0.147621f, -0.716567f },     { -0.850651f,  0.000000f, -0.525731f },
    { -0.688191f,  0.587785f, -0.425325f },     { -0.587785f,  0.425325f, -0.688191f },     { -0.425325f,  0.688191f, -0.587785f },     { -0.425325f, -0.688191f, -0.587785f },
    { -0.587785f, -0.425325f, -0.688191f },     { -0.688191f, -0.587785f, -0.425325f }
};

struct md2 : vertloader<md2>
{
    struct md2_header
    {
        int magic;
        int version;
        int skinwidth, skinheight;
        int framesize;
        int numskins, numvertices, numtexcoords;
        int numtriangles, numglcommands, numframes;
        int offsetskins, offsettexcoords, offsettriangles;
        int offsetframes, offsetglcommands, offsetend;
    };

    struct md2_vertex
    {
        uchar vertex[3], normalindex;
    };

    struct md2_frame
    {
        float      scale[3];
        float      translate[3];
        char       name[16];
    };

    md2(const char *name) : vertloader(name) {}

    static const char *formatname() { return "md2"; }
    static bool multiparted() { return false; }
    static bool multimeshed() { return false; }
    bool flipy() const { return true; }
    int type() const { return MDL_MD2; }

    int linktype(animmodel *m, part *p) const { return LINK_COOP; }

    struct md2meshgroup : vertmeshgroup
    {
        void genverts(int *glcommands, vector<tcvert> &tcverts, vector<ushort> &vindexes, vector<tri> &tris)
        {
            hashtable<ivec, int> tchash;
            vector<ushort> idxs;
            for(int *command = glcommands; (*command)!=0;)
            {
                int numvertex = *command++;
                bool isfan = numvertex<0;
                if(isfan) numvertex = -numvertex;
                idxs.resize(0);
                loopi(numvertex)
                {
                    union { int i; float f; } u, v;
                    u.i = *command++;
                    v.i = *command++;
                    int vindex = *command++;
                    ivec tckey(u.i, v.i, vindex);
                    int *idx = tchash.access(tckey);
                    if(!idx)
                    {
                        idx = &tchash[tckey];
                        *idx = tcverts.size();
                        tcvert &tc = tcverts.emplace_back();
                        tc.tc = vec2(u.f, v.f);
                        vindexes.emplace_back((ushort) vindex);
                    }
                    idxs.emplace_back(*idx);
                }
                loopi(numvertex-2)
                {
                    tri &t = tris.emplace_back();
                    if(isfan)
                    {
                        t.vert[0] = idxs[0];
                        t.vert[1] = idxs[i+1];
                        t.vert[2] = idxs[i+2];
                    }
                    else loopk(3) t.vert[k] = idxs[i&1 && k ? i+(1-(k-1))+1 : i+k];
                }
            }
        }

        bool load(const char *filename, float smooth)
        {
            stream *file = openfile(filename, "rb");
            if(!file) return false;

            md2_header header;
            file->read(&header, sizeof(md2_header));
            lilswap(&header.magic, sizeof(md2_header)/sizeof(int));

            if(header.magic!=844121161 || header.version!=8 ||
               header.numframes <= 0 || header.numframes > 1000 ||
               header.numglcommands <= 0 || header.numglcommands > (1<<20) ||
               header.numvertices <= 0 || header.numvertices > (1<<20))
            {
                delete file;
                return false;
            }

            name = newcubestr(filename);

            numframes = header.numframes;

            vertmesh &m = *new vertmesh;
            m.group = this;
            meshes.emplace_back(&m);

            int *glcommands = new int[header.numglcommands];
            file->seek(header.offsetglcommands, SEEK_SET);
            int numglcommands = file->read(glcommands, header.numglcommands*sizeof(int))/sizeof(int);
            lilswap(glcommands, numglcommands);
            if(numglcommands < header.numglcommands) memset(&glcommands[numglcommands], 0, (header.numglcommands-numglcommands)*sizeof(int));

            vector<tcvert> tcgen;
            vector<ushort> vgen;
            vector<tri> trigen;
            genverts(glcommands, tcgen, vgen,trigen);
            delete[] glcommands;

            m.numverts = tcgen.size();

            m.tcverts = new tcvert[m.numverts];
            memcpy(m.tcverts, tcgen.data(), m.numverts*sizeof(tcvert));
            m.numtris = trigen.size();
            m.tris = new tri[m.numtris];
            memcpy(m.tris, trigen.data(), m.numtris*sizeof(tri));

            m.verts = new vert[m.numverts*numframes];

            md2_vertex *tmpverts = new md2_vertex[header.numvertices];
            int frame_offset = header.offsetframes;
            vert *curvert = m.verts;
            loopi(header.numframes)
            {
                md2_frame frame;
                file->seek(frame_offset, SEEK_SET);
                file->read(&frame, sizeof(md2_frame));
                lilswap(frame.scale, 6);

                file->read(tmpverts, header.numvertices*sizeof(md2_vertex));
                loopj(m.numverts)
                {
                    const md2_vertex &v = tmpverts[vgen[j]];
                    curvert->pos = vec(v.vertex[0]*frame.scale[0]+frame.translate[0],
                                       -(v.vertex[1]*frame.scale[1]+frame.translate[1]),
                                       v.vertex[2]*frame.scale[2]+frame.translate[2]);
                    const float *norm = md2normaltable[v.normalindex];
                    curvert->norm = vec(norm[0], -norm[1], norm[2]);
                    ++curvert;
                }
                frame_offset += header.framesize;
            }
            delete[] tmpverts;

            m.calctangents();

            delete file;

            return true;
        }
    };

    vertmeshgroup *newmeshes() { return new md2meshgroup; }

    bool loadconfig() { return false; }

    bool loaddefaultparts()
    {
        part &mdl = addpart();
        const char *pname = parentdir(name.c_str());
        auto name1 = fmt::format("media/model/{}/tris.md2", name);
        mdl.meshes = sharemeshes(path(name1.c_str(), true));
        if(!mdl.meshes)
        {
            auto name2 = fmt::format("media/model/{}/tris.md2", pname);
            mdl.meshes = sharemeshes(path(name2.c_str(), true));
            if(!mdl.meshes) return false;
        }
        Texture *tex, *masks;
        loadskin(name.c_str(), pname, tex, masks);
        mdl.initskins(tex, masks);
        if(tex==notexture) conoutf("could not load model skin for %s", name1.c_str());
        identflags &= ~IDF_PERSIST;
        auto name3 = fmt::format("media/model/{}/md2.cfg", name);
        if(!execfile(name3.c_str(), false))
        {
            name3 = fmt::format("media/model/{}/md2.cfg", name);
            execfile(name3.c_str(), false);
        }
        identflags |= IDF_PERSIST;
        return true;
    }
};

vertcommands<md2> md2commands;

