#include "FpsGameState.h"
#include "engine/rendergl.h"
#include "engine/renderva.h"
#include "engine/renderlights.h"
#include "engine/rendersky.h"
#include "engine/renderparticles.h"
#include "engine/grass.h"
#include "engine/water.h"
#include "engine/world.h"
#include "engine/material.h"
#include "engine/blend.h"
#include "engine/octaedit.h"
#include "engine/octa.h"
#include "engine/texture.h"
#include "engine/aa.h"
#include "engine/hud.h"
#include "engine/font.h"
#include "engine/Camera.h"
#include "engine/main/FPS.h"
#include "engine/main/Clock.h"
#include "engine/main/Compatibility.h"
#include "engine/nui/nui.h"
#include "engine/editor/ui.h"
#include "game/game.h"
#include "game/entities/SkeletalEntity.h"

void drawdamagescreen(int w, int h);
void drawdamagecompass(int w, int h);
void printtimers(int conw, int conh);
void drawcrosshair(int w, int h);

FpsGameState::FpsGameState()
{
    m_Player = new SkeletalEntity();
    m_Camera = new Camera();
    getents().emplace_back(m_Player);
    getents().emplace_back(m_Camera);
    if (thirdperson)
    {
        player = m_Player;
        extern float thirdpersonup;
        thirdpersonup = 18.f;
    }
    else
    {
        player = m_Camera;
    }
    m_Camera->o = vec(0, 0, 9.f);
    m_Player->SetAttribute("model", "actors/bones");
    findplayerspawn(m_Player);
}

FpsGameState::~FpsGameState()
{

}

unsigned int FpsGameState::ShouldScale()
{
    extern GLuint scalefbo[2];

    return scalefbo[0];
}

void FpsGameState::RenderGame(RenderPass pass)
{
    auto& ents = getents();

    for (int i = 0; in_range(i, ents); ++i)
    {
        auto& entity = ents[i];

        entity->render(pass);
    }
}

void FpsGameState::Render(RenderPass pass)
{
    if (pass == RenderPass::Main)
    {
        RenderMain();
    }

    if (pass == RenderPass::Gui)
    {
        RenderGui();
    }
}

void FpsGameState::RenderMain()
{
    extern int gw, gh;
    extern int fogoverlay;

    GLuint scalefbo = ShouldScale();
    if(scalefbo) { vieww = gw; viewh = gh; }

    const auto& activeCamera = Camera::GetActiveCamera();

    float fogmargin = 1 + WATER_AMPLITUDE + nearplane;
    int fogmat = lookupmaterial(vec(activeCamera->o.x, activeCamera->o.y, activeCamera->o.z - fogmargin))&(MATF_VOLUME|MATF_INDEX), abovemat = MAT_AIR;
    float fogbelow = 0;
    if(isliquid(fogmat&MATF_VOLUME))
    {
        float z = findsurface(fogmat, vec(activeCamera->o.x, activeCamera->o.y, activeCamera->o.z - fogmargin), abovemat) - WATER_OFFSET;
        if(activeCamera->o.z < z + fogmargin)
        {
            fogbelow = z - activeCamera->o.z;
        }
        else fogmat = abovemat;
    }
    else
    {
        fogmat = MAT_AIR;
    }
    setfog(abovemat);
    setfog(fogmat, fogbelow, 1, abovemat);

    farplane = worldsize*2;

    projmatrix.perspective(fovy, aspect, nearplane, farplane);
    setcamprojmatrix();

    glCheckError(glEnable(GL_CULL_FACE));
    glCheckError(glEnable(GL_DEPTH_TEST));

    ldrscale = 0.5f;
    ldrscaleb = ldrscale/255;

    visiblecubes(false);

    if(wireframe && editmode){
#ifndef OPEN_GL_ES
        glCheckError(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
#endif
    }

    timer *gcputimer = drawtex ? NULL : begintimer("g-buffer", false);
    timer *gtimer = drawtex ? NULL : begintimer("g-buffer");

    rendergbuffer_before();
    RenderGame(RenderPass::Main);
    rendergbuffer_after();

    endtimer(gtimer);
    endtimer(gcputimer);

    if(wireframe && editmode){
#ifndef OPEN_GL_ES
        glCheckError(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
#endif
    }
    else if(limitsky() && editmode) renderexplicitsky(true);

    renderao();
    GLERROR;

    // render avatar after AO to avoid weird contact shadows
    renderavatar();
    GLERROR;

    // render grass after AO to avoid disturbing shimmering patterns
    generategrass();
    rendergrass();
    GLERROR;

    glFlush();

    renderradiancehints();
    GLERROR;

    rendershadowatlas();
    GLERROR;

    shadegbuffer();
    GLERROR;

    if(fogmat)
    {
        setfog(fogmat, fogbelow, 1, abovemat);

        renderwaterfog(fogmat, fogbelow);

        setfog(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);
    }

    rendertransparent();
    GLERROR;

    if(fogmat) setfog(fogmat, fogbelow, 1, abovemat);

    rendervolumetric();
    GLERROR;

    if(editmode)
    {
        extern int outline;
        if(!wireframe && outline) renderoutline();
        GLERROR;
        rendereditmaterials();
        GLERROR;
        renderparticles();
        GLERROR;

        extern int hidehud;
        if(!hidehud)
        {
            glCheckError(glDepthMask(GL_FALSE));
            renderblendbrush();
            rendereditcursor();
            RenderGame(RenderPass::Edit);
            glCheckError(glDepthMask(GL_TRUE));
        }
    }

    glCheckError(glDisable(GL_CULL_FACE));
    glCheckError(glDisable(GL_DEPTH_TEST));

    if(fogoverlay && fogmat != MAT_AIR) drawfogoverlay(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);

    doaa(setuppostfx(vieww, viewh, scalefbo), processhdr);
    renderpostfx(scalefbo);
    if(scalefbo) doscale();
}

void FpsGameState::RenderGui()
{
    extern int hidehud, hidestats, showfps, statrate, showfpsrange;
    extern int wallclock, wallclocksecs, wallclock24;
    extern time_t walltime;
    extern int frametimer, framemillis;

    debuglights();

    glCheckError(glEnable(GL_BLEND));

    debugparticles();

//    drawdamagescreen(w, h);
//    drawdamagecompass(w, h);
//
//    float conw = w/conscale, conh = h/conscale, abovehud = conh - FONTH;
//    if(!hidehud)
//    {
//        if(!hidestats)
//        {
//            pushhudscale(conscale);
//
//            int roffset = 0;
//            if(showfps)
//            {
//                static int lastfps = 0, prevfps[3] = { 0, 0, 0 }, curfps[3] = { 0, 0, 0 };
//                if(totalmillis - lastfps >= statrate)
//                {
//                    memcpy(prevfps, curfps, sizeof(prevfps));
//                    lastfps = totalmillis - (totalmillis%statrate);
//                }
//                int nextfps[3];
//                getfps(nextfps[0], nextfps[1], nextfps[2]);
//                loopi(3) if(prevfps[i]==curfps[i]) curfps[i] = nextfps[i];
//                if(showfpsrange) draw_textf("fps %d+%d-%d", conw-7*FONTH, conh-FONTH*3/2, curfps[0], curfps[1], curfps[2]);
//                else draw_textf("fps %d", conw-5*FONTH, conh-FONTH*3/2, curfps[0]);
//                roffset += FONTH;
//            }
//
//            printtimers(conw, conh);
//
//            if(wallclock)
//            {
//                if(!walltime) { walltime = time(NULL); walltime -= totalmillis/1000; if(!walltime) walltime++; }
//                time_t walloffset = walltime + totalmillis/1000;
//                struct tm *localvals = localtime(&walloffset);
//                static cubestr buf;
//                if(localvals && strftime(buf, sizeof(buf), wallclocksecs ? (wallclock24 ? "%H:%M:%S" : "%I:%M:%S%p") : (wallclock24 ? "%H:%M" : "%I:%M%p"), localvals))
//                {
//                    // hack because not all platforms (windows) support %P lowercase option
//                    // also strip leading 0 from 12 hour time
//                    char *dst = buf;
//                    const char *src = &buf[!wallclock24 && buf[0]=='0' ? 1 : 0];
//                    while(*src) *dst++ = tolower(*src++);
//                    *dst++ = '\0';
//                    draw_text(buf, conw-5*FONTH, conh-FONTH*3/2-roffset);
//                    roffset += FONTH;
//                }
//            }
//
//            pophudmatrix();
//        }
//
//        if(!editmode)
//        {
//            resethudshader();
//            glCheckError(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
//            abovehud = min(abovehud, conh*game::abovegameplayhud(w, h));
//        }
//
//        rendertexturepanel(w, h);
//    }

    drawcrosshair(vieww, viewh);

    glCheckError(glDisable(GL_BLEND));
}

void FpsGameState::Update()
{
    if(!curtime) return;

    physicsframe();

    auto& ents = getents();
    EntityEventTick evt;

    m_Player->OnImpl(evt);
    for (auto& entity : ents)
    {
        entity->OnImpl(evt);
    }
    m_Camera->OnImpl(evt);
}

bool FpsGameState::OnEvent(const Event& event)
{
    switch(event.type)
    {
        case EventType::Move:
            m_Player->move = static_cast<const GameEventMove &>(event).payload;;
            break;
        case EventType::Strafe:
            m_Player->strafe = static_cast<const GameEventStrafe &>(event).payload;
            break;
        case EventType::Jump:
            m_Player->jumping = static_cast<const GameEventJump &>(event).payload;
            break;
        case EventType::Crouch:
            m_Player->crouching = static_cast<const GameEventCrouch &>(event).payload;
            break;
    }
}


