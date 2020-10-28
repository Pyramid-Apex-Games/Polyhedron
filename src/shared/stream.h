#pragma once
#include "shared/tools/vector.h"
#include <string>

char *makerelpath(const char *dir, const char *file, const char *prefix = NULL, const char *cmd = NULL);
char *path(char *s);
char *path(const char *s, bool copy);
const char *parentdir(const char *directory);
bool fileexists(const char *path, const char *mode);
bool createdir(const char *path);
size_t fixpackagedir(char *dir);
const char *sethomedir(const char *dir);
const char *addpackagedir(const char *dir);
const char *findfile(const char *filename, const char *mode);
bool findzipfile(const char *filename);
stream *openrawfile(const char *filename, const char *mode);
stream *openfile(const char *filename, const char *mode);
stream *opentempfile(const char *filename, const char *mode);
stream *opengzfile(const char *filename, const char *mode, stream *file = NULL, int level = 9);
stream *openutf8file(const char *filename, const char *mode, stream *file = NULL);
char *loadfile(const char *fn, size_t *size, bool utf8 = true);
bool listdir(const std::string& dir, bool rel, const std::string& ext, std::vector<std::string> &files);
int listfiles(const std::string& dir, const std::string& ext, std::vector<std::string> &files);
