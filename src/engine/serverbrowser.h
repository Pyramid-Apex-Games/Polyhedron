#pragma once

bool resolverwait(const char *name, ENetAddress *address);
int connectwithtimeout(ENetSocket sock, const char *hostname, const ENetAddress &address);
void addserver(const char *name, int port = 0, const char *password = NULL, bool keep = false);
void writeservercfg();