#include "Renderer.h"
#include "Window.h"
#include "GLContext.h"
#include "Application.h"
#include "AppConfig.h"
#include "shared/glemu.h"
#include "engine/font.h"
#include "engine/client.h"
#include "engine/console.h"
#include "engine/rendergl.h"
#include "engine/texture.h"
#include "engine/renderlights.h"
#include "engine/renderparticles.h"
#include "engine/SoundConfig.h"
#include "engine/stain.h"
#include "engine/menus.h"
#include "engine/nui/nui.h"
#include "engine/hud.h"
#include "engine/GLFeatures.h"
#include "engine/editor/ui.h"
#include "renderdoc_api.h"

extern Texture *notexture;

float loadprogress = 0;
VAR(progressbackground, 0, 0, 1);

bool inbetweenframes = false, renderedframe = true;


Renderer::Renderer(Window& window)
    : m_Window(window)
{
    conoutf("init: check OpenGL Features");
    GLFeatures::Initialize();

    conoutf("init: gl_init");
    gl_init();
}

void Renderer::Initialize()
{
    notexture = textureload("media/texture/game/notexture.png");
    if(!notexture)
    {
        Application::Instance().Fatal("could not find core textures");
    }

//    if(!execfile("media/interface/font/default.cfg", false))
//    {
//        Application::Instance().Fatal("cannot find default font");
//    }
//    if(!execfile("config/test.cfg", false))
//    {
//        Application::Instance().Fatal("test.cfg failed");
//    }
    if(!execfile("config/font.cfg", false))
    {
        Application::Instance().Fatal("cannot find default font");
    }
    if(!setfont("default"))
    {
        Application::Instance().Fatal("no default font specified");
    }

//    m_Window.SetGamma();
    m_Window.ToggleVSync();

    initgbuffer();
    loadshaders();
    initparticles();
    initstains();

    inbetweenframes = true;
}

void Renderer::RunFrame()
{
    recomputecamera();
    updateparticles();
    updatesounds();

    if (Application::Instance().GetAppState().Minimized)
        return;

    gl_setupframe(true);

    inbetweenframes = false;
    gl_drawframe();
    engine::nui::Render();
    EditorUI::Render();

    if (m_FrameCount == 4)
    {
        RenderdocApi::StopCapture();
    }

    m_FrameCount++;
    renderedframe = inbetweenframes = true;
}

namespace {
    void getbackgroundres(int &w, int &h)
    {
        float wk = 1, hk = 1;
        if(w < 1024) wk = 1024.0f/w;
        if(h < 768) hk = 768.0f/h;
        wk = hk = max(wk, hk);
        w = int(ceil(w*wk));
        h = int(ceil(h*hk));
    }
}

void Renderer::RenderBackgroundQuad(float x, float y, float w, float h, float tx, float ty, float tw, float th)
{
    gle::begin(GL_TRIANGLE_STRIP);
    gle::attribf(x,   y);   gle::attribf(tx,      ty);
    gle::attribf(x+w, y);   gle::attribf(tx + tw, ty);
    gle::attribf(x,   y+h); gle::attribf(tx,      ty + th);
    gle::attribf(x+w, y+h); gle::attribf(tx + tw, ty + th);
    gle::end();
}

void Renderer::RenderProgressView(float bar, const std::string& text)
{
    int w = 0;
    int h = 0;
    m_Window.GetContext().GetFramebufferSize(w, h);

    hudmatrix.ortho(0, w, h, 0, -1, 1);
    resethudmatrix();
    resethudshader();

    gle::defvertex(2);
    gle::deftexcoord0();

    float fh = 0.060f*min(w, h), fw = fh*15,
            fx = renderedframe ? w - fw - fh/4 : 0.5f*(w - fw),
            fy = renderedframe ? fh/4 : h - fh*1.5f;
    settexture("media/interface/loading_frame.png", 3);
    RenderBackgroundQuad(fx, fy, fw, fh);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float bw = fw*(512 - 2*8)/512.0f, bh = fh*20/32.0f,
            bx = fx + fw*8/512.0f, by = fy + fh*6/32.0f,
            su1 = 0/32.0f, su2 = 8/32.0f, sw = fw*8/512.0f,
            eu1 = 24/32.0f, eu2 = 32/32.0f, ew = fw*8/512.0f,
            mw = bw - sw - ew,
            ex = bx+sw + max(mw*bar, fw*8/512.0f);
    if (bar > 0)
    {
        settexture("media/interface/loading_bar.png", 3);
        RenderBackgroundQuad(bx, by, sw, bh, su1, 0, su2-su1, 1);
        RenderBackgroundQuad(bx+sw, by, ex-(bx+sw), bh, su2, 0, eu1-su2, 1);
        RenderBackgroundQuad(ex, by, ew, bh, eu1, 0, eu2-eu1, 1);
    }

    if (!text.empty())
    {
        int tw = text_width(text.c_str());
        float tsz = bh*0.6f/FONTH;
        if(tw*tsz > mw) tsz = mw/tw;

        pushhudtranslate(bx+sw, by + (bh - FONTH*tsz)/2, tsz);
        draw_text(text.c_str(), 0, 0);
        pophudmatrix();
    }

    glDisable(GL_BLEND);
}

extern int drawtex;

void Renderer::RenderProgress(float bar, const std::string& text, bool background)
{
    if(!inbetweenframes || drawtex) return;

    extern int menufps, maxfps;
    int fps = menufps ? (maxfps ? min(maxfps, menufps) : menufps) : maxfps;
    if(fps)
    {
        static int lastprogress = 0;
        int ticks = SDL_GetTicks(), diff = ticks - lastprogress;
        if(bar > 0 && diff >= 0 && diff < (1000 + fps-1)/fps) return;
        lastprogress = ticks;
    }

    clientkeepalive();      // make sure our connection doesn't time out while loading maps etc.

//#ifdef __APPLE__
//    interceptkey(SDLK_UNKNOWN); // keep the event queue awake to avoid 'beachball' cursor
//#endif

    int w = 0, h = 0;
    m_Window.GetContext().GetFramebufferSize(w, h);
    if(forceaspect) w = int(ceil(h*forceaspect));
    getbackgroundres(w, h);
    gettextres(w, h);

    extern int mesa_swap_bug;
    int curvsync = Application::Instance().GetAppState().VSyncEnabled;
    bool forcebackground = progressbackground || (mesa_swap_bug && (curvsync || totalmillis==1));
    if(background || forcebackground) RestoreBackground(forcebackground);

    RenderProgressView(bar, text);
    m_Window.Swap();
}

void Renderer::SetBackgroundInfo(
    const std::string& caption, Texture* texture,
    const std::string& mapName, const std::string& mapInfo
)
{
    m_BackgroundInfo.caption = caption;
    m_BackgroundInfo.texture = texture;
    m_BackgroundInfo.mapName = mapName;
    m_BackgroundInfo.mapInfo = mapInfo;
}

void Renderer::RestoreBackground(bool force)
{
    if(renderedframe)
    {
        if(!force) return;
        m_BackgroundInfo.caption.clear();
        m_BackgroundInfo.texture = nullptr;
        m_BackgroundInfo.mapName.clear();
        m_BackgroundInfo.mapInfo.clear();
    }
    RenderBackgroundView(m_BackgroundInfo);
}

void Renderer::RenderBackground(const std::string &caption)
{
    auto bg = BackgroundInfo { caption };
    RenderBackground(bg);
}

void Renderer::RenderBackground(BackgroundInfo &info, bool force)
{
    if (!inbetweenframes && !force)
    {
        return;
    }

    int w = 0, h = 0;
    m_Window.GetContext().GetFramebufferSize(w, h);

    if(forceaspect) w = int(ceil(h*forceaspect));
    getbackgroundres(w, h);
    gettextres(w, h);

    if(force)
    {
        RenderBackgroundView(info);
        return;
    }

    loopi(3)
    {
        RenderBackgroundView(info);
        m_Window.Swap();
    }

    SetBackgroundInfo(info.caption, info.texture, info.mapName, info.mapInfo);
}

void Renderer::RenderBackgroundView(BackgroundInfo &info)
{
    static int lastupdate = -1, lastw = -1, lasth = -1;
    static float backgroundu = 0, backgroundv = 0;
    int w = 0, h = 0;
    int hudw = 0, hudh = 0;
    m_Window.GetContext().GetFramebufferSize(w, h);
    m_Window.GetContext().GetFramebufferSize(hudw, hudh);

    if((renderedframe && !mainmenu && lastupdate != lastmillis) || lastw != w || lasth != h)
    {
        lastupdate = lastmillis;
        lastw = w;
        lasth = h;

        backgroundu = rndscale(1);
        backgroundv = rndscale(1);
    }
    else if(lastupdate != lastmillis) lastupdate = lastmillis;
    hudmatrix.ortho(0, w, h, 0, -1, 1);
    resethudmatrix();
    resethudshader();

    gle::defvertex(2);
    gle::deftexcoord0();

    settexture("media/interface/background.png", 0);
    float bu = w*0.67f/256.0f, bv = h*0.67f/256.0f;
    RenderBackgroundQuad(0, 0, w, h, backgroundu, backgroundv, bu, bv);

    glCheckError(glEnable(GL_BLEND));
    glCheckError(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    settexture("media/interface/shadow.png", 3);
    RenderBackgroundQuad(0, 0, w, h);

    glCheckError(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

    float lh = 0.5f*min(w, h), lw = lh*2,
            lx = 0.5f*(w - lw), ly = 0.5f*(h*0.5f - lh);
    settexture(
        (maxtexsize ? min(maxtexsize, hwtexsize) : hwtexsize) >= 1024 && (hudw > 1280 || hudh > 800) ?
        "<premul>media/interface/logo_1024.png" :
        "<premul>media/interface/logo.png",
        3
    );
    RenderBackgroundQuad(lx, ly, lw, lh);

    glCheckError(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    if (!info.caption.empty())
    {
        int tw = text_width(info.caption.c_str());
        float tsz = 0.04f*min(w, h)/FONTH,
                tx = 0.5f*(w - tw*tsz), ty = h - 0.075f*1.5f*min(w, h) - FONTH*tsz;
        pushhudtranslate(tx, ty, tsz);
        draw_text(info.caption.c_str(), 0, 0);
        pophudmatrix();
    }
    if (info.texture || !info.mapName.empty())
    {
        float infowidth = 14*FONTH, sz = 0.35f*min(w, h), msz = (0.85f*min(w, h) - sz)/(infowidth + FONTH);
        float x = 0.5f*w, y = ly+lh - sz/15, mx = 0, my = 0, mw = 0, mh = 0;

        if (!info.mapInfo.empty())
        {
            text_boundsf(info.mapInfo.c_str(), mw, mh, infowidth);
            x -= 0.5f*mw*msz;
            if(info.texture && info.texture != notexture)
            {
                x -= 0.5f*FONTH*msz;
                mx = sz + FONTH*msz;
            }
        }
        if (info.texture && info.texture != notexture)
        {
            x -= 0.5f*sz;
            resethudshader();
            glCheckError(glBindTexture(GL_TEXTURE_2D, info.texture->id));
            RenderBackgroundQuad(x, y, sz, sz);
        }
        if (!info.mapName.empty())
        {
            float tw = text_widthf(info.mapName.c_str());
            float tsz = sz/(8*FONTH);
            float tx = max(0.5f*(mw*msz - tw*tsz), 0.0f);
            pushhudtranslate(x+mx+tx, y, tsz);
            draw_text(info.mapName.c_str(), 0, 0);
            pophudmatrix();
            my = 1.5f*FONTH*tsz;
        }
        if (!info.mapInfo.empty())
        {
            pushhudtranslate(x+mx, y+my, msz);
            draw_text(info.mapInfo.c_str(), 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, -1, infowidth);
            pophudmatrix();
        }
    }

    glCheckError(glDisable(GL_BLEND));
}
