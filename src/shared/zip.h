#pragma once

int listzipfiles(const std::string& dir, const std::string& ext, std::vector<std::string> &files);
int listzipfiles(const char *dir, const char *ext, vector<char *> &files);
stream *openzipfile(const char *filename, const char *mode);
