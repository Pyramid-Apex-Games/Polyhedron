// console.cpp: the console buffer, its display, and command line control

#include "shared/cube.h"
#include "shared/stream.h"
#include "game/entities/SkeletalEntity.h"
#include "engine/nui/nui.h"
#include "engine/main/Application.h"
#include "engine/main/Window.h"
#include "engine/main/GLContext.h"
#include "engine/main/Compatibility.h"
#include "engine/engine/font.h"
#include "engine/command.h"
#include "engine/menus.h"
#include "engine/ui.h"
#include "console.h"
#include <list>

//#define MAXCONLINES 1000



//reversequeue<cline, MAXCONLINES> conlines;

int commandmillis = -1;
cubestr commandbuf;
char *commandaction = NULL, *commandprompt = NULL;
enum { CF_COMPLETE = 1<<0, CF_EXECUTE = 1<<1 };
int commandflags = 0, commandpos = -1;


void conoutfv(int type, const char *fmt, va_list args)
{
    static char buf[512];
    vformatcubestr(buf, fmt, args, sizeof(buf));
    Application::Instance().GetConsole().Add(buf);
    logoutf("%s", buf);
}

void conoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(CON_INFO, fmt, args);
    va_end(args);
}

void conoutf(int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(type, fmt, args);
    va_end(args);
}

SCRIPTEXPORT SCRIPTBIND_OPT(BINDOPT_GENERATORS, BINDOPER_DROP, BINDGENERATOR_PYTHON)
void fullconsole(int *val, CommandTypes::ArgLen numargs, ident *id)
{
    if(*numargs > 0)
    {
        UI::holdui("fullconsole", *val!=0);
    }
    else
    {
        int vis = UI::uivisible("fullconsole") ? 1 : 0;
        if(*numargs < 0)
        {
            intret(vis);
        }
        else
        {
            printvar(id, vis);
        }
    }
}

SCRIPTEXPORT void toggleconsole()
{
    UI::toggleui("fullconsole");
}

float rendercommand(float x, float y, float w)
{
    if(commandmillis < 0) return 0;

    char buf[512];
    const char *prompt = commandprompt ? commandprompt : ">";
    formatcubestr(buf, "%s %s", prompt, commandbuf);

    float width, height;
    text_boundsf(buf, width, height, w);
    y -= height;
    draw_text(buf, x, y, 0xFF, 0xFF, 0xFF, 0xFF, commandpos>=0 ? commandpos+1 + strlen(prompt) : strlen(buf), w);
    return height;
}

VARP(consize, 0, 10, 100);
VARP(miniconsize, 0, 5, 100);
VARP(miniconwidth, 0, 40, 100);
VARP(confade, 0, 0, 60);
VARP(miniconfade, 0, 30, 60);
VARP(fullconsize, 0, 75, 100);
HVARP(confilter, 0, 0xFFFFFF, 0xFFFFFF);
HVARP(fullconfilter, 0, 0xFFFFFF, 0xFFFFFF);
HVARP(miniconfilter, 0, 0, 0xFFFFFF);

int conskip = 0, miniconskip = 0;

void setconskip(int &skip, int filter, int n)
{

}

SCRIPTEXPORT_AS(conskip) void conskip_scriptimpl(int *n)
{
    setconskip(conskip, UI::uivisible("fullconsole") ? fullconfilter : confilter, *n);
}

SCRIPTEXPORT_AS(miniconskip) void miniconskip_scriptimpl(int *n)
{
    setconskip(miniconskip, miniconfilter, *n);
}

SCRIPTEXPORT void clearconsole()
{
    Application::Instance().GetConsole().Clear();
}

float renderfullconsole(float w, float h)
{
    float conpad = FONTH/2,
          conheight = h - 2*conpad,
          conwidth = w - 2*conpad;
    return conheight + 2*conpad;
}



// keymap is defined externally in keymap.cfg

struct keym
{
    enum
    {
        ACTION_DEFAULT = 0,
        ACTION_SPECTATOR,
        ACTION_EDITING,
        NUMACTIONS
    };

    int code;
    char *name;
    char *actions[NUMACTIONS];
    bool pressed;

    keym() : code(-1), name(NULL), pressed(false) { loopi(NUMACTIONS) actions[i] = newcubestr(""); }
    ~keym() { DELETEA(name); loopi(NUMACTIONS) DELETEA(actions[i]); }

    void clear(int type);
    void clear() { loopi(NUMACTIONS) clear(i); }
};

hashtable<int, keym> keyms(128);

SCRIPTEXPORT void keymap(int *code, char *key)
{
    if(identflags&IDF_OVERRIDDEN) { conoutf(CON_ERROR, "cannot override keymap %d", *code); return; }
    keym &km = keyms[*code];
    km.code = *code;
    DELETEA(km.name);
    km.name = newcubestr(key);
}

keym *keypressed = NULL;
char *keyaction = NULL;

const char *getkeyname(int code)
{
    keym *km = keyms.access(code);
    return km ? km->name : NULL;
}

void searchbinds(char *action, int type)
{
    vector<char> names;
    enumerate(keyms, keym, km,
    {
        if(!strcmp(km.actions[type], action))
        {
            if(names.size()) names.emplace_back(' ');
            put(km.name, 1, names);
        }
    });
    names.emplace_back('\0');
    result(names.data());
}

keym *findbind(char *key)
{
    enumerate(keyms, keym, km,
    {
        if(!strcasecmp(km.name, key)) return &km;
    });
    return NULL;
}

void getbind(char *key, int type)
{
    keym *km = findbind(key);
    result(km ? km->actions[type] : "");
}

void bindkey(char *key, char *action, int state, const char *cmd)
{
    if(identflags&IDF_OVERRIDDEN) { conoutf(CON_ERROR, "cannot override %s \"%s\"", cmd, key); return; }
    keym *km = findbind(key);
    if(!km) { conoutf(CON_ERROR, "unknown key \"%s\"", key); return; }
    char *&binding = km->actions[state];
    if(!keypressed || keyaction!=binding) delete[] binding;
    // trim white-space to make searchbinds more reliable
    while(iscubespace(*action)) action++;
    int len = strlen(action);
    while(len>0 && iscubespace(action[len-1])) len--;
    binding = newcubestr(action, len);
}

SCRIPTEXPORT_AS(bind) void bind_scriptimpl(char *key, char *action)
{
    bindkey(key, action, keym::ACTION_DEFAULT, "bind");
}

SCRIPTEXPORT void specbind(char *key, char *action)
{
    bindkey(key, action, keym::ACTION_SPECTATOR, "specbind");
}

SCRIPTEXPORT void editbind(char *key, char *action)
{
    bindkey(key, action, keym::ACTION_EDITING, "editbind");
}

SCRIPTEXPORT_AS(getbind) void getbind_scriptimpl(char *key)
{
    getbind(key, keym::ACTION_DEFAULT);
}

SCRIPTEXPORT void getspecbind(char *key)
{
    getbind(key, keym::ACTION_SPECTATOR);
}

SCRIPTEXPORT void geteditbind(char *key)
{
    getbind(key, keym::ACTION_EDITING);
}

SCRIPTEXPORT_AS(searchbinds) void searchbinds_scriptimpl(char *action)
{
    searchbinds(action, keym::ACTION_DEFAULT);
}

SCRIPTEXPORT void searchspecbinds(char *action)
{
    searchbinds(action, keym::ACTION_SPECTATOR);
}

SCRIPTEXPORT void searcheditbinds(char *action)
{
    searchbinds(action, keym::ACTION_EDITING);
}

void keym::clear(int type)
{
    char *&binding = actions[type];
    if(binding[0])
    {
        if(!keypressed || keyaction!=binding) delete[] binding;
        binding = newcubestr("");
    }
}

SCRIPTEXPORT void clearbinds()
{
    enumerate(keyms, keym, km, km.clear(keym::ACTION_DEFAULT));
}

SCRIPTEXPORT void clearspecbinds()
{
    enumerate(keyms, keym, km, km.clear(keym::ACTION_SPECTATOR));
}

SCRIPTEXPORT void cleareditbinds()
{
    enumerate(keyms, keym, km, km.clear(keym::ACTION_EDITING));
}

SCRIPTEXPORT void clearallbinds()
{
    enumerate(keyms, keym, km, km.clear());
}


SCRIPTEXPORT void inputcommand(char *init, char *action =  NULL, char *prompt = NULL, char *flags = NULL) // turns input to the command line on or off
{
    commandmillis = init ? totalmillis : -1;
    textinput(commandmillis >= 0, TI_CONSOLE);
    keyrepeat(commandmillis >= 0, KR_CONSOLE);
    copycubestr(commandbuf, init ? init : "");
    DELETEA(commandaction);
    DELETEA(commandprompt);
    commandpos = -1;
    if(action && action[0]) commandaction = newcubestr(action);
    if(prompt && prompt[0]) commandprompt = newcubestr(prompt);
    commandflags = 0;
    if(flags) while(*flags) switch(*flags++)
    {
        case 'c': commandflags |= CF_COMPLETE; break;
        case 'x': commandflags |= CF_EXECUTE; break;
        case 's': commandflags |= CF_COMPLETE|CF_EXECUTE; break;
    }
    else if(init) commandflags |= CF_COMPLETE|CF_EXECUTE;
}

SCRIPTEXPORT void saycommand(char *init)
{
    inputcommand(init);
}

void pasteconsole()
{
    if(!SDL_HasClipboardText()) return;
    char *cb = SDL_GetClipboardText();
    if(!cb) return;
    size_t cblen = strlen(cb),
           commandlen = strlen(commandbuf),
           decoded = decodeutf8((uchar *)&commandbuf[commandlen], sizeof(commandbuf)-1-commandlen, (const uchar *)cb, cblen);
    commandbuf[commandlen + decoded] = '\0';
    SDL_free(cb);
}

struct hline
{
    char *buf, *action, *prompt;
    int flags;

    hline() : buf(NULL), action(NULL), prompt(NULL), flags(0) {}
    ~hline()
    {
        DELETEA(buf);
        DELETEA(action);
        DELETEA(prompt);
    }

    void restore()
    {
        copycubestr(commandbuf, buf);
        if(commandpos >= (int)strlen(commandbuf)) commandpos = -1;
        DELETEA(commandaction);
        DELETEA(commandprompt);
        if(action) commandaction = newcubestr(action);
        if(prompt) commandprompt = newcubestr(prompt);
        commandflags = flags;
    }

    bool shouldsave()
    {
        return strcmp(commandbuf, buf) ||
               (commandaction ? !action || strcmp(commandaction, action) : action!=NULL) ||
               (commandprompt ? !prompt || strcmp(commandprompt, prompt) : prompt!=NULL) ||
               commandflags != flags;
    }

    void save()
    {
        buf = newcubestr(commandbuf);
        if(commandaction) action = newcubestr(commandaction);
        if(commandprompt) prompt = newcubestr(commandprompt);
        flags = commandflags;
    }

    void run()
    {
		printf("console executing: %s\n", buf);

        if(flags&CF_EXECUTE && buf[0]=='/') execute(buf+1);
        else if(action)
        {
            alias("commandbuf", buf);
            execute(action);
        }
        else game::toserver(buf);
    }
};
vector<hline *> history;
int histpos = 0;

VARP(maxhistory, 0, 1000, 10000);

SCRIPTEXPORT_AS(history) void history_(int *n)
{
    static bool inhistory = false;
    if(!inhistory && in_range(*n, history))
    {
        inhistory = true;
        history[history.size()-*n-1]->run();
        inhistory = false;
    }
}

struct releaseaction
{
    keym *key = nullptr;
    union
    {
        char *action = nullptr;
        ident *id;
    };
    int numargs;
    tagval args[3] = {0};
};
vector<releaseaction> releaseactions;

const char *addreleaseaction(char *s)
{
    if(!keypressed) { delete[] s; return NULL; }
    releaseaction &ra = releaseactions.emplace_back();
    ra.key = keypressed;
    ra.action = s;
    ra.numargs = -1;
    return keypressed->name;
}

tagval *addreleaseaction(ident *id, int numargs)
{
    if(!keypressed || numargs > 3) return NULL;
    releaseaction &ra = releaseactions.emplace_back();
    ra.key = keypressed;
    ra.id = id;
    ra.numargs = numargs;
    return ra.args;
}

SCRIPTEXPORT void onrelease(const char *s)
{
    addreleaseaction(newcubestr(s));
}

void execbind(keym &k, bool isdown)
{
    loopv(releaseactions)
    {
        releaseaction &ra = releaseactions[i];
        if(ra.key==&k)
        {
            if(ra.numargs < 0)
            {
                if(!isdown)
                {
                    execute(ra.action);
                }
                delete[] ra.action;
            }
            else execute(isdown ? NULL : ra.id, ra.args, ra.numargs);
        }
    }

    releaseactions.erase(
        std::remove_if(
            releaseactions.begin(), releaseactions.end(),
            [&k](const releaseaction& ra){
                return ra.key == &k;
            }
        ),
        releaseactions.end()
    );

    if(isdown)
    {
        int state = keym::ACTION_DEFAULT;
        if(!mainmenu)
        {
            if(editmode) state = keym::ACTION_EDITING;
            else if(player->state==CS_SPECTATOR) state = keym::ACTION_SPECTATOR;
        }
        char *&action = k.actions[state][0] ? k.actions[state] : k.actions[keym::ACTION_DEFAULT];
        keyaction = action;
        keypressed = &k;
        execute(keyaction);
        keypressed = NULL;
        if(keyaction!=action) delete[] keyaction;
    }
    k.pressed = isdown;
}

bool consoleinput(const char *str, int len)
{
    if(commandmillis < 0) return false;

    resetcomplete();
    int cmdlen = (int)strlen(commandbuf), cmdspace = int(sizeof(commandbuf)) - (cmdlen+1);
    len = min(len, cmdspace);
    if(commandpos<0)
    {
        memcpy(&commandbuf[cmdlen], str, len);
    }
    else
    {
        memmove(&commandbuf[commandpos+len], &commandbuf[commandpos], cmdlen - commandpos);
        memcpy(&commandbuf[commandpos], str, len);
        commandpos += len;
    }
    commandbuf[cmdlen + len] = '\0';

    return true;
}

bool consolekey(int code, bool isdown)
{
    if(commandmillis < 0) return false;

    #ifdef __APPLE__
        #define MOD_KEYS (KMOD_LGUI|KMOD_RGUI)
    #else
        #define MOD_KEYS (KMOD_LCTRL|KMOD_RCTRL)
    #endif

    if(isdown)
    {
        switch(code)
        {
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                break;

            case SDLK_HOME:
                if(strlen(commandbuf)) commandpos = 0;
                break;

            case SDLK_END:
                commandpos = -1;
                break;

            case SDLK_DELETE:
            {
                int len = (int)strlen(commandbuf);
                if(commandpos<0) break;
                memmove(&commandbuf[commandpos], &commandbuf[commandpos+1], len - commandpos);
                resetcomplete();
                if(commandpos>=len-1) commandpos = -1;
                break;
            }

            case SDLK_BACKSPACE:
            {
                int len = (int)strlen(commandbuf), i = commandpos>=0 ? commandpos : len;
                if(i<1) break;
                memmove(&commandbuf[i-1], &commandbuf[i], len - i + 1);
                resetcomplete();
                if(commandpos>0) commandpos--;
                else if(!commandpos && len<=1) commandpos = -1;
                break;
            }

            case SDLK_LEFT:
                if(commandpos>0) commandpos--;
                else if(commandpos<0) commandpos = (int)strlen(commandbuf)-1;
                break;

            case SDLK_RIGHT:
                if(commandpos>=0 && ++commandpos>=(int)strlen(commandbuf)) commandpos = -1;
                break;

            case SDLK_UP:
                if(histpos > history.size()) histpos = history.size();
                if(histpos > 0) history[--histpos]->restore();
                break;

            case SDLK_DOWN:
                if(histpos + 1 < history.size()) history[++histpos]->restore();
                break;

            case SDLK_TAB:
                if(commandflags&CF_COMPLETE)
                {
                    complete(commandbuf, sizeof(commandbuf), commandflags&CF_EXECUTE ? "/" : NULL);
                    if(commandpos>=0 && commandpos>=(int)strlen(commandbuf)) commandpos = -1;
                }
                break;

            case SDLK_v:
                if(SDL_GetModState()&MOD_KEYS) pasteconsole();
                break;
        }
    }
    else
    {
        if(code==SDLK_RETURN || code==SDLK_KP_ENTER)
        {
            hline *h = NULL;
            if(commandbuf[0])
            {
                if(history.empty() || history.back()->shouldsave())
                {
                    if(maxhistory && history.size() >= maxhistory)
                    {
                        loopi(history.size()-maxhistory+1) delete history[i];
                        history.erase(history.begin(), history.begin() + history.size()-maxhistory+1);
                    }
                    history.emplace_back(h = new hline)->save();
                }
                else h = history.back();
            }
            histpos = history.size();
            inputcommand(NULL);
            if(h) h->run();
        }
        else if(code==SDLK_ESCAPE)
        {
            histpos = history.size();
            inputcommand(NULL);
        }
    }

    return true;
}

void processtextinput(const char *str, int len)
{
    if(!UI::textinput(str, len))
        consoleinput(str, len);
}

void processkey(int code, bool isdown)
{
    keym *haskey = keyms.access(code);
    if(haskey && haskey->pressed) execbind(*haskey, isdown); // allow pressed keys to release
    else if(!UI::keypress(code, isdown)) // UI key intercept
    {
        if(!consolekey(code, isdown))
        {
            if(haskey) execbind(*haskey, isdown);
        }
    }
}

void clear_console()
{
    keyms.clear();
}

void writebinds(stream *f)
{
    static const char * const cmds[3] = { "bind", "specbind", "editbind" };
    vector<keym *> binds;
    enumerate(keyms, keym, km, binds.emplace_back(&km));
    std::sort(binds.begin(), binds.end(), sortnameless());
    loopj(3)
    {
        loopv(binds)
        {
            keym &km = *binds[i];
            if(*km.actions[j])
            {
                if(validateblock(km.actions[j])) f->printf("%s %s [%s]\n", cmds[j], escapecubestr(km.name), km.actions[j]);
                else f->printf("%s %s %s\n", cmds[j], escapecubestr(km.name), escapecubestr(km.actions[j]));
            }
        }
    }
}

// tab-completion of all idents and base maps

enum { FILES_DIR = 0, FILES_LIST };

struct fileskey
{
    int type;
    std::string dir, ext;

    fileskey() {}
    fileskey(int type, const char *dir, const char *ext) : type(type), dir(dir), ext(ext ? ext : "") {}
    fileskey(int type, const std::string& dir, const std::string& ext) : type(type), dir(dir), ext(ext) {}
};

struct filesval
{
    int type;
    std::string dir, ext;
    std::vector<std::string> files;
    int millis;

    filesval(int type, const char *dir, const char *ext) : type(type), dir(dir), ext(ext ? ext : ""), millis(-1) {}
    ~filesval() { }

    void update()
    {
        if(type!=FILES_DIR || millis >= commandmillis) return;
        files = {};
        listfiles(dir, ext, files);
        std::sort(files.begin(), files.end());

        files.erase(std::unique(files.begin(), files.end()), files.end());
        millis = totalmillis;
    }
};

static inline bool htcmp(const fileskey &x, const fileskey &y)
{
    return x.type == y.type && x.dir == y.dir && x.ext == y.ext;
}

static inline uint hthash(const fileskey &k)
{
    return hthash(k.dir.c_str());
}

static hashtable<fileskey, filesval *> completefiles;
static hashtable<char *, filesval *> completions;

int completesize = 0;
char *lastcomplete = NULL;

void resetcomplete() { completesize = 0; }

void addcomplete(char *command, int type, char *dir, char *ext)
{
    if(identflags&IDF_OVERRIDDEN)
    {
        conoutf(CON_ERROR, "cannot override complete %s", command);
        return;
    }
    if(!dir[0])
    {
        filesval **hasfiles = completions.access(command);
        if(hasfiles) *hasfiles = NULL;
        return;
    }
    if(type==FILES_DIR)
    {
        int dirlen = (int)strlen(dir);
        while(dirlen > 0 && (dir[dirlen-1] == '/' || dir[dirlen-1] == '\\'))
            dir[--dirlen] = '\0';
        if(ext)
        {
            if(strchr(ext, '*')) ext[0] = '\0';
            if(!ext[0]) ext = NULL;
        }
    }
    fileskey key(type, dir, ext);
    filesval **val = completefiles.access(key);
    if(!val)
    {
        filesval *f = new filesval(type, dir, ext);
        if(type==FILES_LIST) explodelist(dir, f->files);
        val = &completefiles[fileskey(type, f->dir, f->ext)];
        *val = f;
    }
    filesval **hasfiles = completions.access(command);
    if(hasfiles) *hasfiles = *val;
    else completions[newcubestr(command)] = *val;
}

SCRIPTEXPORT_AS(complete) void addfilecomplete(char *command, char *dir, char *ext)
{
    addcomplete(command, FILES_DIR, dir, ext);
}

SCRIPTEXPORT_AS(listcomplete) void addlistcomplete(char *command, char *list)
{
    addcomplete(command, FILES_LIST, list, NULL);
}

void complete(char *s, int maxlen, const char *cmdprefix)
{
    int cmdlen = 0;
    if(cmdprefix)
    {
        cmdlen = strlen(cmdprefix);
        if(strncmp(s, cmdprefix, cmdlen)) prependcubestr(s, cmdprefix, maxlen);
    }
    if(!s[cmdlen]) return;
    if(!completesize) { completesize = (int)strlen(&s[cmdlen]); DELETEA(lastcomplete); }

    filesval *f = NULL;
    if(completesize)
    {
        char *end = strchr(&s[cmdlen], ' ');
        if(end) f = completions.find(cubestrslice(&s[cmdlen], end), NULL);
    }

    const char *nextcomplete = NULL;
    if(f) // complete using filenames
    {
        int commandsize = strchr(&s[cmdlen], ' ')+1-s;
        f->update();
        loopv(f->files)
        {
            if(strncmp(f->files[i].c_str(), &s[commandsize], completesize+cmdlen-commandsize)==0 &&
               (!lastcomplete || strcmp(f->files[i].c_str(), lastcomplete) > 0) && (!nextcomplete || strcmp(f->files[i].c_str(), nextcomplete) < 0))
                nextcomplete = f->files[i].c_str();
        }
        cmdprefix = s;
        cmdlen = commandsize;
    }
    else // complete using command names
    {
        enumerate(idents, ident, id,
            if(strncmp(id.name, &s[cmdlen], completesize)==0 &&
               (!lastcomplete || strcmp(id.name, lastcomplete) > 0) && (!nextcomplete || strcmp(id.name, nextcomplete) < 0))
                nextcomplete = id.name;
        );
    }
    DELETEA(lastcomplete);
    if(nextcomplete)
    {
        cmdlen = min(cmdlen, maxlen-1);
        if(cmdlen) memmove(s, cmdprefix, cmdlen);
        copycubestr(&s[cmdlen], nextcomplete, maxlen-cmdlen);
        lastcomplete = newcubestr(nextcomplete);
    }
}

void writecompletions(stream *f)
{
    vector<char *> cmds;
    enumeratekt(completions, char *, k, filesval *, v, { if(v) cmds.emplace_back(k); });
    std::sort(cmds.begin(), cmds.end());
    loopv(cmds)
    {
        char *k = cmds[i];
        filesval *v = completions[k];
        if(v->type==FILES_LIST)
        {
            if(validateblock(v->dir.c_str())) f->printf("listcomplete %s [%s]\n", escapeid(k), v->dir.c_str());
            else f->printf("listcomplete %s %s\n", escapeid(k), escapestr(v->dir).c_str());
        }
        else f->printf("complete %s %s %s\n", escapeid(k), escapestr(v->dir).c_str(), escapestr(v->ext.empty() ? "*" :  v->ext).c_str());
    }
}

Console::Console()
{
}

Console::~Console()
{
}

namespace {
    std::string StripNewline(const std::string& line)
    {
        if (line.back() == '\n')
        {
            return line.substr(0, line.size() - 1);
        }

        return line;
    }

    nk_color LogLevelToColor(LogLevel level)
    {
        nk_color color {255, 255, 255, 255};
        if (level & CON_INFO)
        {
            //default color
        }
        else if (level & CON_ERROR)
        {
            color.r = 217;
            color.g = 58;
            color.b = 54;
        }
        else if (level & CON_WARN)
        {
            color.r = 215;
            color.g = 217;
            color.b = 54;
        }

        return color;
    }
}
void Console::Update()
{
    if (m_BufferedWidth == 0.0f && m_BufferedHeight == 0.0f)
    {
        Application::Instance()
            .GetWindow()
            .GetContext()
            .GetFramebufferSize(m_BufferedWidth, m_BufferedHeight);
    }

    auto lineNum = std::max(0ull, m_ConsoleLines.size() - m_Config.Regular.LineNum);
    auto height = (engine::nui::GetDevice().GetLineHeight() + 1.0f) * std::min((int)m_ConsoleLines.size(), m_Config.Regular.LineNum) + 2.0f;
    nk_style_push_style_item(
        engine::nui::GetNKContext(),
        &engine::nui::GetNKContext()->style.window.fixed_background,
        nk_style_item_hide()
    );
    nk_style_push_color(
        engine::nui::GetNKContext(),
        &engine::nui::GetNKContext()->style.window.border_color,
        nk_rgba(0,0,0,0)
    );
    nk_style_push_vec2(
        engine::nui::GetNKContext(),
        &engine::nui::GetNKContext()->style.window.spacing,
        nk_vec2(0,0)
    );
    nk_style_push_vec2(
        engine::nui::GetNKContext(),
        &engine::nui::GetNKContext()->style.window.padding,
        nk_vec2(4,4)
    );
    if (nk_begin(
        engine::nui::GetNKContext(),
        "",
        nk_rect(0.0f, 0.0f, m_BufferedWidth, height),
        NK_WINDOW_NOT_INTERACTIVE | NK_WINDOW_NO_SCROLLBAR
    ))
    {
        nk_layout_set_min_row_height(engine::nui::GetNKContext(), engine::nui::GetDevice().GetLineHeight());

        for (; lineNum < m_ConsoleLines.size(); ++lineNum)
        {
            nk_color color = LogLevelToColor(m_ConsoleLines[lineNum].m_Level);

            nk_layout_row_dynamic(engine::nui::GetNKContext(), engine::nui::GetDevice().GetLineHeight(), 1);
            nk_label_colored(engine::nui::GetNKContext(), StripNewline(m_ConsoleLines[lineNum].m_Line).c_str(), NK_TEXT_LEFT, color);
        }
    }
    nk_end(engine::nui::GetNKContext());
    nk_style_pop_style_item(engine::nui::GetNKContext());
    nk_style_pop_color(engine::nui::GetNKContext());
    nk_style_pop_vec2(engine::nui::GetNKContext());
    nk_style_pop_vec2(engine::nui::GetNKContext());
}

void Console::Add(LogLevel level, const std::string& text)
{
    auto newlinePos = text.find('\n');
    if (newlinePos == std::string::npos)
    {
        m_ConsoleLines.emplace_back(level, text + "\n");
    }
    else if (newlinePos == text.size() - 1)
    {
        m_ConsoleLines.emplace_back(level, text);
    }
    else
    {
        Add(level, text.substr(0, newlinePos));
        Add(level, text.substr(newlinePos + 1));
    }
}

void Console::Add(const std::string& text)
{
    auto level = LogLevel::CON_INFO;
    Add(level, text);
}

void Console::AddPiecewise(LogLevel level, const std::string& text)
{
    std::size_t current = text.find('\n'), previous = 0;
    if (m_ConsoleLines.empty())
    {
        if (current == std::string::npos)
        {
            m_ConsoleLines.emplace_back(level, text);
        }
        else
        {
            auto& addedLine = m_ConsoleLines.emplace_back(level, text.substr(previous, current + 1));
            LowLevelPrint(addedLine);
            AddPiecewise(level, text.substr(current + 1));
        }

        return;
    }

    auto& lastLine = m_ConsoleLines.back();
    if (lastLine.m_Line.back() == '\n')
    {
        auto& newLastLine = m_ConsoleLines.emplace_back(level, "");
        AddPiecewise(level, text);
        return;
    }
    if (lastLine.m_Level != level)
    {
        lastLine.m_Line += "\n";
        LowLevelPrint(lastLine);

        lastLine = m_ConsoleLines.emplace_back(level, "");
    }

    if (current == std::string::npos)
    {
        if (lastLine.m_Line.back() != '\n')
        {
            lastLine.m_Line += text;
        }
        else
        {
            m_ConsoleLines.emplace_back(level, text);
        }
        return;
    }

    std::list<std::string> lines;
    while(current != std::string::npos)
    {
        lines.push_back(text.substr(previous, current + 1));
        previous = current + 1;
        current = text.find('\n', previous);
    }
    if (previous != text.size())
    {
        lines.push_back(text.substr(previous));
    }

    lastLine.m_Line += lines.front();
    lines.pop_front();
    if (lastLine.m_Line.back() == '\n')
        LowLevelPrint(lastLine);

    for (auto& line : lines)
    {
        lastLine = m_ConsoleLines.emplace_back(level, line);
        if (lastLine.m_Line.back() == '\n')
            LowLevelPrint(lastLine);
    }
}

void Console::AddPiecewise(const std::string& text)
{
    auto level = LogLevel::CON_INFO;
    AddPiecewise(level, text);
}

void Console::LowLevelPrint(const Console::ConsoleLine & line)
{
    if (line.m_Level == CON_ERROR)
    {
        fprintf(stderr, "%s", line.m_Line.c_str());
    }
    else
    {
        printf("%s", line.m_Line.c_str());
    }
}

void Console::Clear()
{
    m_ConsoleLines.clear();
}


Console::ConsoleLine::ConsoleLine(LogLevel level, std::string line)
    : m_Level(level)
    , m_Line(std::move(line))
{
    m_AddedTime = totalmillis;
}


ConsoleOptions::ConsoleOptions(int lineNum, int lineWidth, int fadeDelay, unsigned int filter)
    : LineNum(lineNum)
    , LineWidth(lineWidth)
    , FadeDelay(fadeDelay)
    , Filter(filter)
{
}
