#pragma once
#include <cstdio>
#include <cstdarg>
#define LOGSTRLEN 512

void closelogfile();
FILE *getlogfile();
void setlogfile(const char *fname);
void logoutf(const char *fmt, ...);

static void writelog(FILE *file, const char *buf);
static void writelogv(FILE *file, const char *fmt, va_list args);
void logoutfv(const char *fmt, va_list args);
//void conoutfv(int type, const char *fmt, va_list args);
//void conoutf(const char *fmt, ...);
//void conoutf(int type, const char *fmt, ...);
