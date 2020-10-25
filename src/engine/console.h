#pragma once
#include "shared/tools/macros.h"
#include <string>
#include <vector>
#include <stdarg.h>

enum LogLevel
{
    CON_INFO  = 1u << 0u,
    CON_WARN  = 1u << 1u,
    CON_ERROR = 1u << 2u,
    CON_DEBUG = 1u << 3u,
    CON_INIT  = 1u << 4u,
    CON_ECHO  = 1u << 5u
};


struct ConsoleOptions
{
    int LineNum = 5;
    int LineWidth = 0;
    int FadeDelay = 30;
    unsigned int Filter = 0xFFFFFFFF;
    ConsoleOptions(int lineNum, int lineWidth, int fadeDelay, unsigned int filter);
};

struct ConsoleConfig
{
    ConsoleOptions Regular {12, 0, 30, 0xFFFFFFFF};
    ConsoleOptions Mini {5, 0, 20, 0xFFFFFFFF};
    ConsoleOptions Full {0, 0, 0, 0xFFFFFFFF};
};

class Console
{
public:
    Console();
    ~Console();

    void Update();

    void AddPiecewise(LogLevel level, const std::string& text);
    void AddPiecewise(const std::string& text);

    void Add(LogLevel level, const std::string& text);
    void Add(const std::string& text);

    void Clear();
//    void TextInput();

private:
    struct ConsoleLine
    {
        std::string m_Line;
        LogLevel m_Level;
        int m_AddedTime;

        ConsoleLine(LogLevel level, std::string line);
    };

    static void LowLevelPrint(const ConsoleLine& line);

    std::vector<std::string> m_CommandHistory;
    std::vector<ConsoleLine> m_ConsoleLines;
    ConsoleConfig m_Config;

    int m_BufferedWidth = 0;
    int m_BufferedHeight = 0;
//    float m_LineHeight = 32.0f;
};

extern float conscale;
struct tagval;
struct ident;
struct stream;

void processkey(int code, bool isdown);
void processtextinput(const char *str, int len);
float rendercommand(float x, float y, float w);
float renderfullconsole(float w, float h);

void conoutf(const char *s, ...) PRINTFARGS(1, 2);
void conoutf(int type, const char *s, ...) PRINTFARGS(2, 3);
extern void conoutfv(int type, const char *fmt, va_list args);
void resetcomplete();
void complete(char *s, int maxlen, const char *cmdprefix);
const char *getkeyname(int code);
const char *addreleaseaction(char *s);
tagval *addreleaseaction(ident *id, int numargs);
void writebinds(stream *f);
void writecompletions(stream *f);
void conline(int type, const char *sf);

void ConsoleBuffered(int type, std::string& text);
void ConsoleBuffered(std::string& text);
