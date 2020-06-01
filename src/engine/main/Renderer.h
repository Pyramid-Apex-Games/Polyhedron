#pragma once
#include <string>

class Window;
class Texture;
class Application;

struct BackgroundInfo
{
    std::string caption;
    Texture* texture = nullptr;
    std::string mapName;
    std::string mapInfo;
};

class Renderer {
public:
    Renderer(Window& window);

    void Initialize();
    void RunFrame();

    void RenderBackground(const std::string& caption);
    void RenderBackground(BackgroundInfo& info, bool force = false);
    void RenderProgress(float bar, const std::string& text, bool background);
    void SetBackgroundInfo(
        const std::string& caption, Texture* texture,
        const std::string& mapName, const std::string& mapInfo
    );

private:
    void RenderBackgroundQuad(
        float x, float y, float w, float h,
        float tx = 0, float ty = 0, float tw = 1, float th = 1
    );
    void RenderProgressView(float bar, const std::string& text);
    void RestoreBackground(bool force = false);
    void RenderBackgroundView(BackgroundInfo& info);

    long m_FrameCount = 0;
    Window& m_Window;
    BackgroundInfo m_BackgroundInfo;
    BackgroundInfo m_BackgroundMapInfo;
};

extern bool inbetweenframes, renderedframe;

extern float loadprogress;

