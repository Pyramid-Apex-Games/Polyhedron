#pragma once

//config values from renderlights.h
#define LIGHTTILE_MAXW 16
#define LIGHTTILE_MAXH 16

enum class RenderPass {
    Main,
    Lights,
    Edit,
    Gui
};