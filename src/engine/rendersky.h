#pragma once

extern int skytexture, skyshadow, explicitsky;

void drawskybox(bool clear = false);
bool hasskybox();
bool limitsky();
bool renderexplicitsky(bool outline = false);
void cleanupsky();