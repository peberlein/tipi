#ifndef _MAIN_H
#define _MAIN_H 1

#include "dsrutil.h"

void initGlobals();
void setupScreen(int width);
void resetF18A();
void titleScreen();
void main();
void sleep(int jiffies);
void handleCommand(char * buffer);

void handleCd();
void handleDir();
void handleDrives();
void handleWidth();
void handleQuit();
void handleVer();
void handleHelp();

int parsePath(char* path, char* devicename);
void parsePathParam(struct DeviceServiceRoutine** dsr, char* buffer, int requirements);

#define REQ_DEFAULT 0x8000
#define REQ_EXISTS 0x0800
#define REQ_FOLDER 0x0400
#define REQ_FILE   0x0200

#endif