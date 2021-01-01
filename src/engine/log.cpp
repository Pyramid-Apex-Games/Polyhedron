#include "shared/tools.h"
#include "shared/stream.h"
#include "engine/log.h"

#ifdef ANDROID
#include <android/log.h>
#endif


static FILE *logfile = NULL;

void closelogfile()
{
    if(logfile)
    {
        fclose(logfile);
        logfile = NULL;
    }
}

FILE *getlogfile()
{
#ifdef WIN32
    return logfile;
#else
	return logfile ? logfile : stdout;
#endif
}

void setlogfile(const char *fname)
{
    closelogfile();
    if(fname && fname[0])
    {
        fname = findfile(fname, "w");
        if(fname) logfile = fopen(fname, "w");
    }
    FILE *f = getlogfile();
    if(f) setvbuf(f, NULL, _IOLBF, BUFSIZ);
}

void logoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logoutfv(fmt, args);
    va_end(args);
}


static void writelog(FILE *file, const char *buf)
{
    static uchar ubuf[512];
    size_t len = strlen(buf), carry = 0;
    while(carry < len)
    {
        size_t numu = encodeutf8(ubuf, sizeof(ubuf)-1, &((const uchar *)buf)[carry], len - carry, &carry);
        if(carry >= len) ubuf[numu++] = '\n';
        fwrite(ubuf, 1, numu, file);
    }
}

static void writelogv(FILE *file, const char *fmt, va_list args)
{
    static char buf[LOGSTRLEN];
    vformatcubestr(buf, fmt, args, sizeof(buf));
    writelog(file, buf);
}

void logoutfv(const char *fmt, va_list args)
{
#if defined(ANDROID)
    __android_log_vprint(ANDROID_LOG_WARN, "POLYHDRN", fmt, args);
#elif defined(WIN32)
    char buf[1024]={0};
    vsnprintf(buf, 1024, fmt, args);
    OutputDebugString(buf);
    OutputDebugString("\n");
#else
    FILE* f = getlogfile();
    if (f) writelogv(f, fmt, args);
#endif
}


//void conoutfv(int type, const char *fmt, va_list args)
//{
//   logoutfv(fmt, args);
//}
//
//void conoutf(const char *fmt, ...)
//{
//   va_list args;
//   va_start(args, fmt);
//   conoutfv(CON_INFO, fmt, args);
//   va_end(args);
//}
//
//void conoutf(int type, const char *fmt, ...)
//{
//   va_list args;
//   va_start(args, fmt);
//   conoutfv(type, fmt, args);
//   va_end(args);
//}
