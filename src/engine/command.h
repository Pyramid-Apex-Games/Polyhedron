#pragma once

extern hashnameset<ident> idents;
extern int identflags;

void clearoverrides();
void writecfg(const char *name = NULL);

void checksleep(int millis);
void clearsleep(bool clearoverrides = true);