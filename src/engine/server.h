#pragma once

extern vector<const char *> gameargs;

void initserver(bool listen, bool dedicated);
void cleanupserver();
void serverslice(bool dedicated, uint timeout);
void updatetime();

ENetSocket connectmaster(bool wait);
void localclienttoserver(int chan, ENetPacket *);
void localconnect();
bool serveroption(char *opt);