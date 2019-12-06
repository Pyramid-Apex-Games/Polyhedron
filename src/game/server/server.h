#pragma once
#include "../game.h"

namespace server
{
    struct ClientInfo {
		int clientnumber = 0;
        int ownernumber = 0;
        int connectmillis = 0;
        int sessionid = 0;
        int overflow = 0;
        int playermodel = 0;
        cubestr name;
        int privilege;
        bool connected = false;
        bool local = false;
        bool timesync = false;
        int gameoffset = 0;
        int lastevent = 0;
        int pushed = 0;
        int exceeded = 0;
	};

    // Server time related functions
    void serverinit();
    bool ispaused();
    int scaletime(int t);
    void serverupdate();

    // Server client related functions.
    void *newclientinfo();
    void deleteclientinfo(void *ci);
    int reserveclients();
    void clientdisconnect(int n);
    int clientconnect(int n, uint ip);
    void localdisconnect(int n);
    void localconnect(int n);

    // Broadcasting related functions.
    bool allowbroadcast(int n);
    void recordpacket(int chan, void *data, int len);
    void parsepacket(int sender, int chan, packetbuf &p);
    void sendservmsg(const char *s);
    bool sendpackets(bool force);
    void serverinforeply(ucharbuf &req, ucharbuf &p);
    int numchannels();
    
    // Protocol related functions.
    int protocolversion();
    int serverinfoport(int servport);
    int serverport();
    int masterport();
    int laninfoport();

    // Master server related functions.
    const char *defaultmaster();
    void processmasterinput(const char *cmd, int cmdlen, const char *args);
    void masterconnected();
    void masterdisconnected();

    // Are servers compatible?
    bool servercompatible(char *name, char *sdec, char *map, int ping, const vector<int> &attr, int np);
}