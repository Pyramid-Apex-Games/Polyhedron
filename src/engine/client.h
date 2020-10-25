#pragma once

void localdisconnect(bool cleanup = true);
void localservertoclient(int chan, ENetPacket *packet);
void connectserv(const char *servername, int port, const char *serverpassword);
void abortconnect();
void clientkeepalive();