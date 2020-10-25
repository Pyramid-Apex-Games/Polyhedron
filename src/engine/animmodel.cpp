#include "animmodel.h"
#include "shared/entities/animinfo.h"
#include "shared/entities/MovableEntity.h"
#include "game/entities/SkeletalEntity.h"
#include "engine/ragdoll.h"
#include "engine/Camera.h"

VARP(fullbrightmodels, 0, 0, 200);
VAR(testtags, 0, 0, 1);
VARF(dbgcolmesh, 0, 0, 1,
{
    extern void cleanupmodels();
    cleanupmodels();
});

hashnameset<animmodel::meshgroup *> animmodel::meshgroups;
int animmodel::intersectresult = -1, animmodel::intersectmode = 0;
float animmodel::intersectdist = 0, animmodel::intersectscale = 1;
bool animmodel::enabletc = false, animmodel::enabletangents = false, animmodel::enablebones = false,
     animmodel::enablecullface = true, animmodel::enabledepthoffset = false;
float animmodel::sizescale = 1;
vec4 animmodel::colorscale(1, 1, 1, 1);
GLuint animmodel::lastvbuf = 0, animmodel::lasttcbuf = 0, animmodel::lastxbuf = 0, animmodel::lastbbuf = 0, animmodel::lastebuf = 0,
       animmodel::lastenvmaptex = 0, animmodel::closestenvmaptex = 0;
Texture *animmodel::lasttex = NULL, *animmodel::lastdecal = NULL, *animmodel::lastmasks = NULL, *animmodel::lastnormalmap = NULL;
int animmodel::matrixpos = 0;
matrix4 animmodel::matrixstack[64];

hashtable<animmodel::shaderparams, animmodel::shaderparamskey> animmodel::shaderparamskey::keys;
int animmodel::shaderparamskey::firstversion = 0, animmodel::shaderparamskey::lastversion = 1;

void animmodel::animpos::setframes(const animinfo &info)
{
    anim = info.anim;
    if(info.range<=1)
    {
        fr1 = 0;
        t = 0;
    }
    else
    {
        int time = info.anim&ANIM_SETTIME ? info.basetime : lastmillis-info.basetime;
        fr1 = (int)(time/info.speed); // round to full frames
        t = (time-fr1*info.speed)/info.speed; // progress of the frame, value from 0.0f to 1.0f
    }
    if(info.anim&ANIM_LOOP)
    {
        fr1 = fr1%info.range+info.frame;
        fr2 = fr1+1;
        if(fr2>=info.frame+info.range) fr2 = info.frame;
    }
    else
    {
        fr1 = min(fr1, info.range-1)+info.frame;
        fr2 = min(fr1+1, info.frame+info.range-1);
    }
    if(info.anim&ANIM_REVERSE)
    {
        fr1 = (info.frame+info.range-1)-(fr1-info.frame);
        fr2 = (info.frame+info.range-1)-(fr2-info.frame);
    }
}

bool animmodel::animpos::operator==(const animpos &a) const
{
    return fr1==a.fr1 && fr2==a.fr2 && (fr1==fr2 || t==a.t);
}

bool animmodel::animpos::operator!=(const animpos &a) const
{
    return fr1!=a.fr1 || fr2!=a.fr2 || (fr1!=fr2 && t!=a.t);
}

bool animmodel::animstate::operator==(const animstate &a) const
{
    return cur==a.cur && (interp<1 ? interp==a.interp && prev==a.prev : a.interp>=1);
}

bool animmodel::animstate::operator!=(const animstate &a) const
{
    return cur!=a.cur || (interp<1 ? interp!=a.interp || prev!=a.prev : a.interp<1);
}

bool animmodel::shaderparamskey::checkversion()
{
    if(version >= firstversion)
    {
        return true;
    }

    version = lastversion;
    if(++lastversion <= 0)
    {
        enumerate(keys, shaderparamskey, key, key.version = -1);
        firstversion = 0;
        lastversion = 1;
        version = 0;
    }

    return false;
}


bool animmodel::skin::masked() const { return masks != notexture; }
bool animmodel::skin::envmapped() const { return envmapmax>0; }
bool animmodel::skin::bumpmapped() const { return normalmap != nullptr; }
bool animmodel::skin::alphatested() const { return alphatest > 0 && tex->type&Texture::ALPHA; }
bool animmodel::skin::decaled() const { return decal != nullptr; }

void animmodel::skin::setkey()
{
    key = &shaderparamskey::keys[*this];
}

void animmodel::skin::setshaderparams(mesh &m, const animstate *as, bool skinned)
{
    if(!Shader::lastshader) return;
    if(key->checkversion() && Shader::lastshader->owner == key) return;
    Shader::lastshader->owner = key;

    LOCALPARAMF(texscroll, scrollu*lastmillis/1000.0f, scrollv*lastmillis/1000.0f);
    if(alphatested()) LOCALPARAMF(alphatest, alphatest);

    if(!skinned) return;

    if(color.r < 0) LOCALPARAM(colorscale, colorscale);
    else LOCALPARAMF(colorscale, color.r, color.g, color.b, colorscale.a);

    if(fullbright) LOCALPARAMF(fullbright, 0.0f, fullbright);
    else LOCALPARAMF(fullbright, 1.0f, as->cur.anim&ANIM_FULLBRIGHT ? 0.5f*fullbrightmodels/100.0f : 0.0f);

    float curglow = glow;
    if(glowpulse > 0)
    {
        float curpulse = lastmillis*glowpulse;
        curpulse -= floor(curpulse);
        curglow += glowdelta*2*fabs(curpulse - 0.5f);
    }
    LOCALPARAMF(maskscale, spec, gloss, curglow);
    if(envmapped()) LOCALPARAMF(envmapscale, envmapmin-envmapmax, envmapmax);
}

Shader* animmodel::skin::loadshader()
{
    #define DOMODELSHADER(name, body) \
        do { \
            static Shader *name##shader = NULL; \
            if(!name##shader) name##shader = useshaderbyname(#name); \
            body; \
        } while(0)
    #define LOADMODELSHADER(name) DOMODELSHADER(name, return name##shader)
    #define SETMODELSHADER(m, name) DOMODELSHADER(name, (m).setshader(name##shader))

    if(shadowmapping == SM_REFLECT)
    {
        if(rsmshader) return rsmshader;

        cubestr opts;
        int optslen = 0;
        if(alphatested()) opts[optslen++] = 'a';
        if(!cullface) opts[optslen++] = 'c';
        opts[optslen++] = '\0';

        defformatcubestr(name, "rsmmodel%s", opts);
        rsmshader = generateshader(name, "rsmmodelshader \"%s\"", opts);
        return rsmshader;
    }

    if(shader) return shader;

    cubestr opts;
    int optslen = 0;
    if(alphatested()) opts[optslen++] = 'a';
    if(decaled()) opts[optslen++] = decal->type&Texture::ALPHA ? 'D' : 'd';
    if(bumpmapped()) opts[optslen++] = 'n';
    if(envmapped()) { opts[optslen++] = 'm'; opts[optslen++] = 'e'; }
    else if(masked()) opts[optslen++] = 'm';
    if(!cullface) opts[optslen++] = 'c';
    opts[optslen++] = '\0';

    defformatcubestr(name, "model%s", opts);
    shader = generateshader(name, "modelshader \"%s\"", opts);
    return shader;
}

void animmodel::skin::cleanup()
{
    if(shader && shader->standard) shader = NULL;
}

void animmodel::skin::preloadBIH()
{
    if(alphatested() && !tex->alphamask) loadalphamask(tex);
}

void animmodel::skin::preloadshader()
{
    loadshader();
    useshaderbyname(alphatested() && owner->model->alphashadow ? "alphashadowmodel" : "shadowmodel");
    if(useradiancehints()) useshaderbyname(alphatested() ? "rsmalphamodel" : "rsmmodel");
}

void animmodel::skin::setshader(mesh &m, const animstate *as)
{
    m.setshader(loadshader(), transparentlayer ? 1 : 0);
}

void animmodel::skin::bind(mesh &b, const animstate *as)
{
    if(cullface > 0)
    {
        if(!enablecullface) { glEnable(GL_CULL_FACE); enablecullface = true; }
    }
    else if(enablecullface) { glDisable(GL_CULL_FACE); enablecullface = false; }

    if(as->cur.anim&ANIM_NOSKIN)
    {
        if(alphatested() && owner->model->alphashadow)
        {
            if(tex!=lasttex)
            {
                glBindTexture(GL_TEXTURE_2D, tex->id);
                lasttex = tex;
            }
            SETMODELSHADER(b, alphashadowmodel);
            setshaderparams(b, as, false);
        }
        else
        {
            SETMODELSHADER(b, shadowmodel);
        }
        return;
    }
    int activetmu = 0;
    if(tex!=lasttex)
    {
        glBindTexture(GL_TEXTURE_2D, tex->id);
        lasttex = tex;
    }
    if(bumpmapped() && normalmap!=lastnormalmap)
    {
        glActiveTexture_(GL_TEXTURE3);
        activetmu = 3;
        glBindTexture(GL_TEXTURE_2D, normalmap->id);
        lastnormalmap = normalmap;
    }
    if(decaled() && decal!=lastdecal)
    {
        glActiveTexture_(GL_TEXTURE4);
        activetmu = 4;
        glBindTexture(GL_TEXTURE_2D, decal->id);
        lastdecal = decal;
    }
    if(masked() && masks!=lastmasks)
    {
        glActiveTexture_(GL_TEXTURE1);
        activetmu = 1;
        glBindTexture(GL_TEXTURE_2D, masks->id);
        lastmasks = masks;
    }
    if(envmapped())
    {
        GLuint emtex = envmap ? envmap->id : closestenvmaptex;
        if(lastenvmaptex!=emtex)
        {
            glActiveTexture_(GL_TEXTURE2);
            activetmu = 2;
            glBindTexture(GL_TEXTURE_CUBE_MAP, emtex);
            lastenvmaptex = emtex;
        }
    }
    if(activetmu != 0) glActiveTexture_(GL_TEXTURE0);
    setshader(b, as);
    setshaderparams(b, as);
}


void animmodel::mesh::genBIH(skin &s, vector<BIH::mesh> &bih, const matrix4x3 &t)
{
    BIH::mesh &m = bih.add();
    m.xform = t;
    m.tex = s.tex;
    if(canrender) m.flags |= BIH::MESH_RENDER;
    if(cancollide) m.flags |= BIH::MESH_COLLIDE;
    if(s.alphatested()) m.flags |= BIH::MESH_ALPHA;
    if(noclip) m.flags |= BIH::MESH_NOCLIP;
    if(s.cullface > 0) m.flags |= BIH::MESH_CULLFACE;
    genBIH(m);
    while(bih.last().numtris > BIH::mesh::MAXTRIS)
    {
        BIH::mesh &overflow = bih.dup();
        overflow.tris += BIH::mesh::MAXTRIS;
        overflow.numtris -= BIH::mesh::MAXTRIS;
        bih[bih.length()-2].numtris = BIH::mesh::MAXTRIS;
    }
}


void animmodel::mesh::setshader(Shader *s, int row)
{
    if(row) s->setvariant(0, row);
    else s->set();
}

animmodel::meshgroup::~meshgroup()
{
    meshes.deletecontents();
    DELETEP(next);
}
int animmodel::meshgroup::findtag(const char *name)
{
    return -1;
}

void animmodel::meshgroup::concattagtransform(part *p, int i, const matrix4x3 &m, matrix4x3 &n)
{
}


void animmodel::meshgroup::calcbb(vec &bbmin, vec &bbmax, const matrix4x3 &t)
{
    forEachRenderMesh<mesh>([&](mesh& m){
        m.calcbb(bbmin, bbmax, t);
    });
}

void animmodel::meshgroup::genBIH(vector<skin> &skins, vector<BIH::mesh> &bih, const matrix4x3 &t)
{
    loopv(meshes) meshes[i]->genBIH(skins[i], bih, t);
}

void animmodel::meshgroup::genshadowmesh(vector<triangle> &tris, const matrix4x3 &t)
{
    forEachRenderMesh<mesh>([&](mesh& m){
        m.genshadowmesh(tris, t);
    });
}

void* animmodel::meshgroup::animkey()
{
    return this;
}

int animmodel::meshgroup::totalframes() const
{
    return 1;
}

bool animmodel::meshgroup::hasframe(int i) const
{
    return i>=0 && i<totalframes();
}

bool animmodel::meshgroup::hasframes(int i, int n) const
{
    return i>=0 && i+n<=totalframes();
}

int animmodel::meshgroup::clipframes(int i, int n) const
{
    return min(n, totalframes() - i);
}

void animmodel::meshgroup::cleanup()
{
}

void animmodel::meshgroup::preload(part *p)
{
}

void animmodel::meshgroup::render(const animstate *as, float pitch, const vec &axis, const vec &forward, MovableEntity *d, part *p)
{
}

void animmodel::meshgroup::intersect(const animstate *as, float pitch, const vec &axis, const vec &forward, MovableEntity *d, part *p, const vec &o, const vec &ray)
{
}

void animmodel::meshgroup::bindpos(GLuint ebuf, GLuint vbuf, void *v, int stride, int type, int size)
{
    if(lastebuf!=ebuf)
    {
        gle::bindebo(ebuf);
        lastebuf = ebuf;
    }
    if(lastvbuf!=vbuf)
    {
        gle::bindvbo(vbuf);
        if(!lastvbuf) gle::enablevertex();
        gle::vertexpointer(stride, v, type, size);
        lastvbuf = vbuf;
    }
}
void animmodel::meshgroup::bindpos(GLuint ebuf, GLuint vbuf, vec *v, int stride)
{
    bindpos(ebuf, vbuf, v, stride, GL_FLOAT, 3);
}

void animmodel::meshgroup::bindpos(GLuint ebuf, GLuint vbuf, hvec4 *v, int stride)
{
    bindpos(ebuf, vbuf, v, stride, GL_HALF_FLOAT, 4);
}

void animmodel::meshgroup::bindtc(void *v, int stride)
{
    if(!enabletc)
    {
        gle::enabletexcoord0();
        enabletc = true;
    }
    if(lasttcbuf!=lastvbuf)
    {
        gle::texcoord0pointer(stride, v, GL_HALF_FLOAT);
        lasttcbuf = lastvbuf;
    }
}

void animmodel::meshgroup::bindtangents(void *v, int stride)
{
    if(!enabletangents)
    {
        gle::enabletangent();
        enabletangents = true;
    }
    if(lastxbuf!=lastvbuf)
    {
        gle::tangentpointer(stride, v, GL_SHORT);
        lastxbuf = lastvbuf;
    }
}

void animmodel::meshgroup::bindbones(void *wv, void *bv, int stride)
{
    if(!enablebones)
    {
        gle::enableboneweight();
        gle::enableboneindex();
        enablebones = true;
    }
    if(lastbbuf!=lastvbuf)
    {
        gle::boneweightpointer(stride, wv);
        gle::boneindexpointer(stride, bv);
        lastbbuf = lastvbuf;
    }
}

animmodel::part::part(animmodel *model, int index) : model(model), index(index)
{
    loopk(MAXANIMPARTS) anims[k] = NULL;
}
animmodel::part::~part()
{
    loopk(MAXANIMPARTS) DELETEA(anims[k]);
}

void animmodel::part::cleanup()
{
    if(meshes) meshes->cleanup();
    loopv(skins) skins[i].cleanup();
}

void animmodel::part::disablepitch()
{
    pitchscale = pitchoffset = pitchmin = pitchmax = 0;
}

void animmodel::part::calcbb(vec &bbmin, vec &bbmax, const matrix4x3 &m)
{
    matrix4x3 t = m;
    t.scale(model->scale);
    meshes->calcbb(bbmin, bbmax, t);
    loopv(links)
    {
        matrix4x3 n;
        meshes->concattagtransform(this, links[i].tag, m, n);
        n.translate(links[i].translate, model->scale);
        links[i].p->calcbb(bbmin, bbmax, n);
    }
}

void animmodel::part::genBIH(vector<BIH::mesh> &bih, const matrix4x3 &m)
{
    matrix4x3 t = m;
    t.scale(model->scale);
    meshes->genBIH(skins, bih, t);
    loopv(links)
    {
        matrix4x3 n;
        meshes->concattagtransform(this, links[i].tag, m, n);
        n.translate(links[i].translate, model->scale);
        links[i].p->genBIH(bih, n);
    }
}

void animmodel::part::genshadowmesh(vector<triangle> &tris, const matrix4x3 &m)
{
    matrix4x3 t = m;
    t.scale(model->scale);
    meshes->genshadowmesh(tris, t);
    loopv(links)
    {
        matrix4x3 n;
        meshes->concattagtransform(this, links[i].tag, m, n);
        n.translate(links[i].translate, model->scale);
        links[i].p->genshadowmesh(tris, n);
    }
}

bool animmodel::part::link(part *p, const char *tag, const vec &translate, int anim, int basetime, vec *pos)
{
    int i = meshes ? meshes->findtag(tag) : -1;
    if(i<0)
    {
        loopv(links) if(links[i].p && links[i].p->link(p, tag, translate, anim, basetime, pos)) return true;
        return false;
    }
    linkedpart &l = links.add();
    l.p = p;
    l.tag = i;
    l.anim = anim;
    l.basetime = basetime;
    l.translate = translate;
    l.pos = pos;
    return true;
}

bool animmodel::part::unlink(part *p)
{
    loopvrev(links) if(links[i].p==p) { links.remove(i, 1); return true; }
    loopv(links) if(links[i].p && links[i].p->unlink(p)) return true;
    return false;
}

void animmodel::part::initskins(Texture *tex, Texture *masks, int limit)
{
    if(!limit)
    {
        if(!meshes) return;
        limit = meshes->meshes.length();
    }
    while(skins.length() < limit)
    {
        skin &s = skins.add();
        s.owner = this;
        s.tex = tex;
        s.masks = masks;
    }
}

bool animmodel::part::alphatested() const
{
    loopv(skins) if(skins[i].alphatested()) return true;
    return false;
}

void animmodel::part::preloadBIH()
{
    loopv(skins) skins[i].preloadBIH();
}

void animmodel::part::preloadshaders()
{
    loopv(skins) skins[i].preloadshader();
}

void animmodel::part::preloadmeshes()
{
    if(meshes) meshes->preload(this);
}

void animmodel::part::getdefaultanim(animinfo &info, int anim, uint varseed, MovableEntity *d)
{
    info.frame = 0;
    info.range = 1;
}

bool animmodel::part::calcanim(int animpart, int anim, int basetime, int basetime2, MovableEntity *d, int interp, animinfo &info, int &animinterptime)
{
    uint varseed = uint((size_t)d);
    info.anim = anim;
    info.basetime = basetime;
    info.varseed = varseed;
    info.speed = anim&ANIM_SETSPEED ? basetime2 : 100.0f;
    if((anim&ANIM_INDEX)==ANIM_ALL)
    {
        info.frame = 0;
        info.range = meshes->totalframes();
    }
    else
    {
        animspec *spec = NULL;
        if(anims[animpart])
        {
            vector<animspec> &primary = anims[animpart][anim&ANIM_INDEX];
            if(primary.length()) spec = &primary[uint(varseed + basetime)%primary.length()];
            if((anim>>ANIM_SECONDARY)&(ANIM_INDEX|ANIM_DIR))
            {
                vector<animspec> &secondary = anims[animpart][(anim>>ANIM_SECONDARY)&ANIM_INDEX];
                if(secondary.length())
                {
                    animspec &spec2 = secondary[uint(varseed + basetime2)%secondary.length()];
                    if(!spec || spec2.priority > spec->priority)
                    {
                        spec = &spec2;
                        info.anim >>= ANIM_SECONDARY;
                        info.basetime = basetime2;
                    }
                }
            }
        }
        if(spec)
        {
            info.frame = spec->frame;
            info.range = spec->range;
            if(spec->speed>0) info.speed = 1000.0f/spec->speed;
        }
        else getdefaultanim(info, anim, uint(varseed + info.basetime), d);
    }

    info.anim &= (1<<ANIM_SECONDARY)-1;
    info.anim |= anim&ANIM_FLAGS;
    if(info.anim&ANIM_LOOP)
    {
        info.anim &= ~ANIM_SETTIME;
        if(!info.basetime) info.basetime = -((int)(size_t)d&0xFFF);

        if(info.anim&ANIM_CLAMP)
        {
            if(info.anim&ANIM_REVERSE) info.frame += info.range-1;
            info.range = 1;
        }
    }

    if(!meshes->hasframes(info.frame, info.range))
    {
        if(!meshes->hasframe(info.frame)) return false;
        info.range = meshes->clipframes(info.frame, info.range);
    }

    if(d && interp>=0)
    {
        animinterpinfo &ai = d->animinterp[interp];
        if((info.anim&(ANIM_LOOP|ANIM_CLAMP))==ANIM_CLAMP)
        {
            animinterptime = min(animinterptime, int(info.range*info.speed*0.5e-3f));
        }
        void *ak = meshes->animkey();
        if(d->ragdoll && d->ragdoll->millis != lastmillis)
        {
            ai.prev.range = ai.cur.range = 0;
            ai.lastswitch = -1;
        }
        else if(ai.lastmodel!=ak || ai.lastswitch<0 || lastmillis-d->lastrendered>animinterptime)
        {
            ai.prev = ai.cur = info;
            ai.lastswitch = lastmillis-animinterptime*2;
        }
        else if(ai.cur!=info)
        {
            if(lastmillis-ai.lastswitch>animinterptime/2) ai.prev = ai.cur;
            ai.cur = info;
            ai.lastswitch = lastmillis;
        }
        else if(info.anim&ANIM_SETTIME) ai.cur.basetime = info.basetime;
        ai.lastmodel = ak;
    }
    return true;
}

void animmodel::part::intersect(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, const vec &o, const vec &ray)
{
    animstate as[MAXANIMPARTS];
    intersect(anim, basetime, basetime2, pitch, axis, forward, d, o, ray, as);
}

void animmodel::part::intersect(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, const vec &o, const vec &ray, animstate *as)
{
    extern int animationinterpolationtime;

    if((anim&ANIM_REUSE) != ANIM_REUSE) loopi(numanimparts)
    {
        animinfo info;
        int interp = d && index+numanimparts<=MAXANIMPARTS ? index+i : -1;
        int aitime = animationinterpolationtime;
        if(!calcanim(i, anim, basetime, basetime2, d, interp, info, aitime)) return;
        animstate &p = as[i];
        p.owner = this;
        p.cur.setframes(info);
        p.interp = 1;
        if(interp>=0 && d->animinterp[interp].prev.range>0)
        {
            int diff = lastmillis-d->animinterp[interp].lastswitch;
            if(diff<aitime)
            {
                p.prev.setframes(d->animinterp[interp].prev);
                p.interp = diff/float(aitime);
            }
        }
    }

    float resize = model->scale * sizescale;
    int oldpos = matrixpos;
    vec oaxis, oforward, oo, oray;
    matrixstack[matrixpos].transposedtransformnormal(axis, oaxis);
    float pitchamount = pitchscale*pitch + pitchoffset;
    if((pitchmin || pitchmax) && pitchmin <= pitchmax) pitchamount = clamp(pitchamount, pitchmin, pitchmax);
    if(as->cur.anim&ANIM_NOPITCH || (as->interp < 1 && as->prev.anim&ANIM_NOPITCH))
        pitchamount *= (as->cur.anim&ANIM_NOPITCH ? 0 : as->interp) + (as->interp < 1 && as->prev.anim&ANIM_NOPITCH ? 0 : 1-as->interp);
    if(pitchamount)
    {
        ++matrixpos;
        matrixstack[matrixpos] = matrixstack[matrixpos-1];
        matrixstack[matrixpos].rotate(pitchamount*RAD, oaxis);
    }
    if(this == model->parts[0] && !model->translate.iszero())
    {
        if(oldpos == matrixpos)
        {
            ++matrixpos;
            matrixstack[matrixpos] = matrixstack[matrixpos-1];
        }
        matrixstack[matrixpos].translate(model->translate, resize);
    }
    matrixstack[matrixpos].transposedtransformnormal(forward, oforward);
    matrixstack[matrixpos].transposedtransform(o, oo);
    oo.div(resize);
    matrixstack[matrixpos].transposedtransformnormal(ray, oray);

    intersectscale = resize;
    meshes->intersect(as, pitch, oaxis, oforward, d, this, oo, oray);

    if((anim&ANIM_REUSE) != ANIM_REUSE)
    {
        loopv(links)
        {
            linkedpart &link = links[i];
            if(!link.p) continue;
            link.matrix.translate(link.translate, resize);

            matrixpos++;
            matrixstack[matrixpos].mul(matrixstack[matrixpos-1], link.matrix);

            int nanim = anim, nbasetime = basetime, nbasetime2 = basetime2;
            if(link.anim>=0)
            {
                nanim = link.anim | (anim&ANIM_FLAGS);
                nbasetime = link.basetime;
                nbasetime2 = 0;
            }
            link.p->intersect(nanim, nbasetime, nbasetime2, pitch, axis, forward, d, o, ray);

            matrixpos--;
        }
    }

    matrixpos = oldpos;
}

void animmodel::part::render(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d)
{
    animstate as[MAXANIMPARTS];
    render(anim, basetime, basetime2, pitch, axis, forward, d, as);
}

void animmodel::part::render(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, animstate *as)
{
    extern int animationinterpolationtime;

    if((anim&ANIM_REUSE) != ANIM_REUSE) loopi(numanimparts)
    {
        animinfo info;
        int interp = d && index+numanimparts<=MAXANIMPARTS ? index+i : -1;
        int aitime = animationinterpolationtime;
        if(!calcanim(i, anim, basetime, basetime2, d, interp, info, aitime)) return;
        animstate &p = as[i];
        p.owner = this;
        p.cur.setframes(info);
        p.interp = 1;
        if(interp>=0 && d->animinterp[interp].prev.range>0)
        {
            int diff = lastmillis-d->animinterp[interp].lastswitch;
            if(diff<aitime)
            {
                p.prev.setframes(d->animinterp[interp].prev);
                p.interp = diff/float(aitime);
            }
        }
    }

    float resize = model->scale * sizescale;
    int oldpos = matrixpos;
    vec oaxis, oforward;
    matrixstack[matrixpos].transposedtransformnormal(axis, oaxis);
    float pitchamount = pitchscale*pitch + pitchoffset;
    if(pitchmin || pitchmax) pitchamount = clamp(pitchamount, pitchmin, pitchmax);
    if(as->cur.anim&ANIM_NOPITCH || (as->interp < 1 && as->prev.anim&ANIM_NOPITCH))
        pitchamount *= (as->cur.anim&ANIM_NOPITCH ? 0 : as->interp) + (as->interp < 1 && as->prev.anim&ANIM_NOPITCH ? 0 : 1-as->interp);
    if(pitchamount)
    {
        ++matrixpos;
        matrixstack[matrixpos] = matrixstack[matrixpos-1];
        matrixstack[matrixpos].rotate(pitchamount*RAD, oaxis);
    }
    if(this == model->parts[0] && !model->translate.iszero())
    {
        if(oldpos == matrixpos)
        {
            ++matrixpos;
            matrixstack[matrixpos] = matrixstack[matrixpos-1];
        }
        matrixstack[matrixpos].translate(model->translate, resize);
    }
    matrixstack[matrixpos].transposedtransformnormal(forward, oforward);

    if(!(anim&ANIM_NORENDER))
    {
        matrix4 modelmatrix;
        modelmatrix.mul(shadowmapping ? shadowmatrix : camprojmatrix, matrixstack[matrixpos]);
        if(resize!=1) modelmatrix.scale(resize);
        GLOBALPARAM(modelmatrix, modelmatrix);

        const auto& activeCamera = Camera::GetActiveCamera();

        if(!(anim&ANIM_NOSKIN) && activeCamera)
        {
            GLOBALPARAM(modelworld, matrix3(matrixstack[matrixpos]));

            vec modelcamera;
            matrixstack[matrixpos].transposedtransform(activeCamera->o, modelcamera);
            modelcamera.div(resize);
            GLOBALPARAM(modelcamera, modelcamera);
        }
    }

    meshes->render(as, pitch, oaxis, oforward, d, this);

    if((anim&ANIM_REUSE) != ANIM_REUSE)
    {
        loopv(links)
        {
            linkedpart &link = links[i];
            link.matrix.translate(link.translate, resize);

            matrixpos++;
            matrixstack[matrixpos].mul(matrixstack[matrixpos-1], link.matrix);

            if(link.pos) *link.pos = matrixstack[matrixpos].gettranslation();

            if(!link.p)
            {
                matrixpos--;
                continue;
            }

            int nanim = anim, nbasetime = basetime, nbasetime2 = basetime2;
            if(link.anim>=0)
            {
                nanim = link.anim | (anim&ANIM_FLAGS);
                nbasetime = link.basetime;
                nbasetime2 = 0;
            }
            link.p->render(nanim, nbasetime, nbasetime2, pitch, axis, forward, d);

            matrixpos--;
        }
    }

    matrixpos = oldpos;
}

void animmodel::part::setanim(int animpart, int num, int frame, int range, float speed, int priority)
{
    if(animpart<0 || animpart>=MAXANIMPARTS || num<0 || num>=SkeletalEntity::AnimationsCount())
    {
        return;
    }
    if(frame<0 || range<=0 || !meshes || !meshes->hasframes(frame, range))
    {
        conoutf(CON_ERROR, "invalid frame %d, range %d in model %s", frame, range, model->name.c_str());
        return;
    }
    if(!anims[animpart])
    {
        anims[animpart] = new vector<animspec>[SkeletalEntity::AnimationsCount()];
    }
    animspec &spec = anims[animpart][num].add();
    spec.frame = frame;
    spec.range = range;
    spec.speed = speed;
    spec.priority = priority;
}

bool animmodel::part::animated() const
{
    loopi(MAXANIMPARTS) if(anims[i]) return true;
    return false;
}

void animmodel::part::loaded()
{
    meshes->shared++;
    loopv(skins) skins[i].setkey();
}


int animmodel::linktype(animmodel *m, part *p) const
{
    return LINK_TAG;
}

void animmodel::intersect(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, modelattach *a, const vec &o, const vec &ray)
{
    int numtags = 0;
    if(a)
    {
        int index = parts.last()->index + parts.last()->numanimparts;
        for(int i = 0; a[i].tag; i++)
        {
            numtags++;

            animmodel *m = (animmodel *)a[i].m;
            if(!m) continue;
            part *p = m->parts[0];
            switch(linktype(m, p))
            {
                case LINK_TAG:
                    p->index = link(p, a[i].tag, vec(0, 0, 0), a[i].anim, a[i].basetime, a[i].pos) ? index : -1;
                    break;

                case LINK_COOP:
                    p->index = index;
                    break;

                default:
                    continue;
            }
            index += p->numanimparts;
        }
    }

    animstate as[MAXANIMPARTS];
    parts[0]->intersect(anim, basetime, basetime2, pitch, axis, forward, d, o, ray, as);

    for(int i = 1; i < parts.length(); i++)
    {
        part *p = parts[i];
        switch(linktype(this, p))
        {
            case LINK_COOP:
                p->intersect(anim, basetime, basetime2, pitch, axis, forward, d, o, ray);
                break;

            case LINK_REUSE:
                p->intersect(anim | ANIM_REUSE, basetime, basetime2, pitch, axis, forward, d, o, ray, as);
                break;
        }
    }

    if(a) for(int i = numtags-1; i >= 0; i--)
    {
        animmodel *m = (animmodel *)a[i].m;
        if(!m) continue;
        part *p = m->parts[0];
        switch(linktype(m, p))
        {
            case LINK_TAG:
                if(p->index >= 0) unlink(p);
                p->index = 0;
                break;

            case LINK_COOP:
                p->intersect(anim, basetime, basetime2, pitch, axis, forward, d, o, ray);
                p->index = 0;
                break;

            case LINK_REUSE:
                p->intersect(anim | ANIM_REUSE, basetime, basetime2, pitch, axis, forward, d, o, ray, as);
                break;
        }
    }
}

int animmodel::intersect(int anim, int basetime, int basetime2, const vec &pos, float yaw, float pitch, float roll, MovableEntity *d, modelattach *a, float size, const vec &o, const vec &ray, float &dist, int mode)
{
    vec axis(1, 0, 0), forward(0, 1, 0);

    matrixpos = 0;
    matrixstack[0].identity();
    if(!d || !d->ragdoll || d->ragdoll->millis == lastmillis)
    {
        float secs = lastmillis/1000.0f;
        yaw += spinyaw*secs;
        pitch += spinpitch*secs;
        roll += spinroll*secs;

        matrixstack[0].settranslation(pos);
        matrixstack[0].rotate_around_z(yaw*RAD);
        bool usepitch = pitched();
        if(roll && !usepitch) matrixstack[0].rotate_around_y(-roll*RAD);
        matrixstack[0].transformnormal(vec(axis), axis);
        matrixstack[0].transformnormal(vec(forward), forward);
        if(roll && usepitch) matrixstack[0].rotate_around_y(-roll*RAD);
        if(offsetyaw) matrixstack[0].rotate_around_z(offsetyaw*RAD);
        if(offsetpitch) matrixstack[0].rotate_around_x(offsetpitch*RAD);
        if(offsetroll) matrixstack[0].rotate_around_y(-offsetroll*RAD);
    }
    else
    {
        matrixstack[0].settranslation(d->ragdoll->center);
        pitch = 0;
    }

    sizescale = size;
    intersectresult = -1;
    intersectmode = mode;
    intersectdist = dist;

    intersect(anim, basetime, basetime2, pitch, axis, forward, d, a, o, ray);

    if(intersectresult >= 0) dist = intersectdist;
    return intersectresult;
}

void animmodel::render(int anim, int basetime, int basetime2, float pitch, const vec &axis, const vec &forward, MovableEntity *d, modelattach *a)
{
    int numtags = 0;
    if(a)
    {
        int index = parts.last()->index + parts.last()->numanimparts;
        for(int i = 0; a[i].tag; i++)
        {
            numtags++;

            animmodel *m = (animmodel *)a[i].m;
            if(!m)
            {
                if(a[i].pos) link(NULL, a[i].tag, vec(0, 0, 0), 0, 0, a[i].pos);
                continue;
            }
            part *p = m->parts[0];
            switch(linktype(m, p))
            {
                case LINK_TAG:
                    p->index = link(p, a[i].tag, vec(0, 0, 0), a[i].anim, a[i].basetime, a[i].pos) ? index : -1;
                    break;

                case LINK_COOP:
                    p->index = index;
                    break;

                default:
                    continue;
            }
            index += p->numanimparts;
        }
    }

    animstate as[MAXANIMPARTS];
    parts[0]->render(anim, basetime, basetime2, pitch, axis, forward, d, as);

    for(int i = 1; i < parts.length(); i++)
    {
        part *p = parts[i];
        switch(linktype(this, p))
        {
            case LINK_COOP:
                p->render(anim, basetime, basetime2, pitch, axis, forward, d);
                break;

            case LINK_REUSE:
                p->render(anim | ANIM_REUSE, basetime, basetime2, pitch, axis, forward, d, as);
                break;
        }
    }

    if(a) for(int i = numtags-1; i >= 0; i--)
    {
        animmodel *m = (animmodel *)a[i].m;
        if(!m)
        {
            if(a[i].pos) unlink(NULL);
            continue;
        }
        part *p = m->parts[0];
        switch(linktype(m, p))
        {
            case LINK_TAG:
                if(p->index >= 0) unlink(p);
                p->index = 0;
                break;

            case LINK_COOP:
                p->render(anim, basetime, basetime2, pitch, axis, forward, d);
                p->index = 0;
                break;

            case LINK_REUSE:
                p->render(anim | ANIM_REUSE, basetime, basetime2, pitch, axis, forward, d, as);
                break;
        }
    }
}

void animmodel::render(int anim, int basetime, int basetime2, const vec &o, float yaw, float pitch, float roll, MovableEntity *d, modelattach *a, float size, const vec4 &color)
{
    vec axis(1, 0, 0), forward(0, 1, 0);

    matrixpos = 0;
    matrixstack[0].identity();
    if(!d || !d->ragdoll || d->ragdoll->millis == lastmillis)
    {
        float secs = lastmillis/1000.0f;
        yaw += spinyaw*secs;
        pitch += spinpitch*secs;
        roll += spinroll*secs;

        matrixstack[0].settranslation(o);
        matrixstack[0].rotate_around_z(yaw*RAD);
        bool usepitch = pitched();
        if(roll && !usepitch) matrixstack[0].rotate_around_y(-roll*RAD);
        matrixstack[0].transformnormal(vec(axis), axis);
        matrixstack[0].transformnormal(vec(forward), forward);
        if(roll && usepitch) matrixstack[0].rotate_around_y(-roll*RAD);
        if(offsetyaw) matrixstack[0].rotate_around_z(offsetyaw*RAD);
        if(offsetpitch) matrixstack[0].rotate_around_x(offsetpitch*RAD);
        if(offsetroll) matrixstack[0].rotate_around_y(-offsetroll*RAD);
    }
    else
    {
        matrixstack[0].settranslation(d->ragdoll->center);
        pitch = 0;
    }

    sizescale = size;

    if(anim&ANIM_NORENDER)
    {
        render(anim, basetime, basetime2, pitch, axis, forward, d, a);
        if(d) d->lastrendered = lastmillis;
        return;
    }

    if(!(anim&ANIM_NOSKIN))
    {
        if(colorscale != color)
        {
            colorscale = color;
            shaderparamskey::invalidate();
        }

        if(envmapped()) closestenvmaptex = lookupenvmap(closestenvmap(o));
        else if(a) for(int i = 0; a[i].tag; i++) if(a[i].m && a[i].m->envmapped())
        {
            closestenvmaptex = lookupenvmap(closestenvmap(o));
            break;
        }
    }

    if(depthoffset && !enabledepthoffset)
    {
        enablepolygonoffset(GL_POLYGON_OFFSET_FILL);
        enabledepthoffset = true;
    }

    render(anim, basetime, basetime2, pitch, axis, forward, d, a);

    if(d) d->lastrendered = lastmillis;
}

animmodel::animmodel(const char *name) : model(name)
{
}

animmodel::~animmodel()
{
    parts.deletecontents();
}

void animmodel::cleanup()
{
    loopv(parts) parts[i]->cleanup();
}

void animmodel::flushpart()
{
}

animmodel::part& animmodel::addpart()
{
    flushpart();
    part *p = new part(this, parts.length());
    parts.add(p);
    return *p;
}

void animmodel::initmatrix(matrix4x3 &m)
{
    m.identity();
    if(offsetyaw) m.rotate_around_z(offsetyaw*RAD);
    if(offsetpitch) m.rotate_around_x(offsetpitch*RAD);
    if(offsetroll) m.rotate_around_y(-offsetroll*RAD);
    m.translate(translate, scale);
}

void animmodel::genBIH(vector<BIH::mesh> &bih)
{
    if(parts.empty()) return;
    matrix4x3 m;
    initmatrix(m);
    parts[0]->genBIH(bih, m);
    for(int i = 1; i < parts.length(); i++)
    {
        part *p = parts[i];
        switch(linktype(this, p))
        {
            case LINK_COOP:
            case LINK_REUSE:
                p->genBIH(bih, m);
                break;
        }
    }
}

void animmodel::genshadowmesh(vector<triangle> &tris, const matrix4x3 &orient)
{
    if(parts.empty()) return;
    matrix4x3 m;
    initmatrix(m);
    m.mul(orient, matrix4x3(m));
    parts[0]->genshadowmesh(tris, m);
    for(int i = 1; i < parts.length(); i++)
    {
        part *p = parts[i];
        switch(linktype(this, p))
        {
            case LINK_COOP:
            case LINK_REUSE:
                p->genshadowmesh(tris, m);
                break;
        }
    }
}

void animmodel::preloadBIH()
{
    model::preloadBIH();
    if(bih) loopv(parts) parts[i]->preloadBIH();
}

BIH* animmodel::setBIH()
{
    if(bih) return bih;
    vector<BIH::mesh> meshes;
    genBIH(meshes);
    bih = new BIH(meshes);
    return bih;
}

bool animmodel::link(part *p, const char *tag, const vec &translate, int anim, int basetime, vec *pos)
{
    if(parts.empty()) return false;
    return parts[0]->link(p, tag, translate, anim, basetime, pos);
}

bool animmodel::unlink(part *p)
{
    if(parts.empty()) return false;
    return parts[0]->unlink(p);
}

bool animmodel::envmapped() const
{
    loopv(parts) loopvj(parts[i]->skins) if(parts[i]->skins[j].envmapped()) return true;
    return false;
}

bool animmodel::animated() const
{
    if(spinyaw || spinpitch || spinroll) return true;
    loopv(parts) if(parts[i]->animated()) return true;
    return false;
}

bool animmodel::pitched() const
{
    return parts[0]->pitchscale != 0;
}

bool animmodel::alphatested() const
{
    loopv(parts) if(parts[i]->alphatested()) return true;
    return false;
}

bool animmodel::flipy() const
{
    return false;
}

bool animmodel::loadconfig()
{
    return false;
}

bool animmodel::loaddefaultparts()
{
    return false;
}

void animmodel::startload()
{
}

void animmodel::endload()
{
}

bool animmodel::load()
{
    startload();
    bool success = loadconfig() && parts.length(); // configured model, will call the model commands below
    if(!success)
        success = loaddefaultparts(); // model without configuration, try default tris and skin
    flushpart();
    endload();
    if(flipy()) translate.y = -translate.y;

    if(!success) return false;
    loopv(parts) if(!parts[i]->meshes) return false;

    loaded();
    return true;
}

void animmodel::preloadshaders()
{
    loopv(parts) parts[i]->preloadshaders();
}

void animmodel::preloadmeshes()
{
    loopv(parts) parts[i]->preloadmeshes();
}

void animmodel::setshader(Shader *shader)
{
    if(parts.empty())
    {
        loaddefaultparts();
    }

    loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].shader = shader;
}

void animmodel::setenvmap(float envmapmin, float envmapmax, Texture *envmap)
{
    if(parts.empty()) loaddefaultparts();
    loopv(parts) loopvj(parts[i]->skins)
    {
        skin &s = parts[i]->skins[j];
        if(envmapmax)
        {
            s.envmapmin = envmapmin;
            s.envmapmax = envmapmax;
        }
        if(envmap) s.envmap = envmap;
    }
}

void animmodel::setspec(float spec)
{
    if(parts.empty()) loaddefaultparts();
    loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].spec = spec;
}

void animmodel::setgloss(int gloss)
{
    if(parts.empty()) loaddefaultparts();
    loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].gloss = gloss;
}

void animmodel::setglow(float glow, float delta, float pulse)
{
    if(parts.empty()) loaddefaultparts();
    loopv(parts) loopvj(parts[i]->skins)
    {
        skin &s = parts[i]->skins[j];
        s.glow = glow;
        s.glowdelta = delta;
        s.glowpulse = pulse;
    }
}

void animmodel::setalphatest(float alphatest)
{
    if(parts.empty()) loaddefaultparts();
    loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].alphatest = alphatest;
}

void animmodel::setfullbright(float fullbright)
{
    if(parts.empty()) loaddefaultparts();
    loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].fullbright = fullbright;
}

void animmodel::setcullface(int cullface)
{
    if(parts.empty()) loaddefaultparts();
    loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].cullface = cullface;
}

void animmodel::setcolor(const vec &color)
{
    if(parts.empty()) loaddefaultparts();
    loopv(parts) loopvj(parts[i]->skins) parts[i]->skins[j].color = color;
}

void animmodel::calcbb(vec &center, vec &radius)
{
    if(parts.empty()) return;
    vec bbmin(1e16f, 1e16f, 1e16f), bbmax(-1e16f, -1e16f, -1e16f);
    matrix4x3 m;
    initmatrix(m);
    parts[0]->calcbb(bbmin, bbmax, m);
    for(int i = 1; i < parts.length(); i++)
    {
        part *p = parts[i];
        switch(linktype(this, p))
        {
            case LINK_COOP:
            case LINK_REUSE:
                p->calcbb(bbmin, bbmax, m);
                break;
        }
    }
    radius = bbmax;
    radius.sub(bbmin);
    radius.mul(0.5f);
    center = bbmin;
    center.add(radius);
}

void animmodel::calctransform(matrix4x3 &m)
{
    initmatrix(m);
    m.scale(scale);
}

void animmodel::loaded()
{
    loopv(parts) parts[i]->loaded();
}
