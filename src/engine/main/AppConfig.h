#pragma once
#include <string>
#include <nlohmann/json_fwd.hpp>

struct AppConfig {
    bool Fullscreen = false;
    int ScreenWidth = 1024;
    int ScreenHeight = 768;

    bool SoundEnabled = true;
    int SoundChannels = 32;
    int SoundFrequency = 44100;
    int SoundBufferLength = 1024;
};

struct AppState {
    std::string ApplicationName = "Polyhedron";
    int DesktopWidth = 0;
    int DesktopHeight = 0;
    int WindowPositionX = 0;
    int WindowPositionY = 0;
    int WindowWidth = 0;
    int WindowHeight = 0;
    int WindowFlags = 0;
    bool InitialWindowPosition = false;
    bool Minimized = false;
    bool VSyncEnabled = true;
};

struct AppConstants {
    static const int MinimumWindowWidth = 640;
    static const int MinimumWindowHeight = 480;
};

struct GLConfig {
    bool DoubleBuffering = true;
    int FrameBufferSize = 1;
    int StencilBufferSize = 0;
    int DepthBufferSize = 0;
};

struct GLContextState {
    int FrameBufferWidth = 0;
    int FrameBufferHeight = 0;
};

static AppConfig AppConfigDefaults;

void from_json(const nlohmann::json& document, AppConfig& appConfig);
void to_json(nlohmann::json& document, const AppConfig& appConfig);

void from_json(const nlohmann::json& document, GLConfig& glConfig);
void to_json(nlohmann::json& document, const GLConfig& glConfig);
