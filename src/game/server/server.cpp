#include "game/game.h"
#include "server.h"
#include "shared/tools.h"

// This file only servers as an empty basic server implementation.
namespace server
{
	void *newclientinfo() {
		return new ClientInfo;
	}
	void deleteclientinfo(void *ci) {
		delete (ClientInfo *)ci;
	}
	void serverinit() {

	}
	int reserveclients() {
		return 3; // Return 3, no clue why, but their default function does.
	}
	int numchannels() {
		return 0;
	}
	void clientdisconnect(int n) {

	}
	int clientconnect(int n, uint ip) {
		
		return DISC_NONE;
	}
	void localdisconnect(int n) {

	}
	void localconnect(int n) {

	}
	bool allowbroadcast(int n) {
		return true;
	}
	void recordpacket(int chan, void *data, int len) {

	}
	void parsepacket(int sender, int chan, packetbuf &p) {

	}
	void sendservmsg(const char *s) {
		
	}
	bool sendpackets(bool force) {

		return false;
	}
	void serverinforeply(ucharbuf &req, ucharbuf &p) {

	}
	void serverupdate() {

	}
	bool servercompatible(char *name, char *sdec, char *map, int ping, const vector<int> &attr, int np) {
		return true;
	}

	int protocolversion() {
		return 1;
	}
	int serverinfoport(int servport) {
		return 0;
	}
	int serverport() {
		return 0;
	}
	const char *defaultmaster() {
		return "";
	}
	int masterport() {
		return 0;
	}
	int laninfoport() {
		return 0;
	}
	void processmasterinput(const char *cmd, int cmdlen, const char *args) {

	}
	void masterconnected() {

	}
	void masterdisconnected() {

	}
	bool ispaused() {
		return false;
	}
	int scaletime(int t) {
		return t*100;
	}
}


// >>>>>>>>>> SCRIPTBIND >>>>>>>>>>>>>> //
// <<<<<<<<<< SCRIPTBIND <<<<<<<<<<<<<< //