#pragma once

int listzipfiles(const char *dir, const char *ext, vector<char *> &files);
stream *openzipfile(const char *filename, const char *mode);
