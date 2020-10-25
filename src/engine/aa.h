#pragma once
// aa
extern matrix4 nojittermatrix;

void setupaa(int w, int h);
void jitteraa();
bool maskedaa();
bool multisampledaa();
void setaavelocityparams(GLenum tmu = GL_TEXTURE0);
void setaamask(bool val);
void enableaamask(int stencil = 0);
void disableaamask();
void doaa(GLuint outfbo, void (*resolve)(GLuint, int));
bool debugaa();
void cleanupaa();
