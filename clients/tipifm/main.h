#ifndef _MAIN_H
#define _MAIN_H 1

#include "dsrutil.h"

void initGlobals();
void main();
void resetF18A();
void setupScreen(int width);
void sleep(int jiffies);
void titleScreen();

void handleCommand(char * buffer);

void handleCd();
void handleDelete();
void handleDir();
void handleDrives();
void handleExit();
void handleHelp();
void handleLvl2();
void handleMkdir();
void handleProtect();
void handleRename();
void handleRmdir();
void handleUnprotect();
void handleVer();
void handleWidth();

int parsePath(char* path, char* devicename);
void parsePathParam(struct DeviceServiceRoutine** dsr, char* buffer, int requirements);

#define REQ_DEFAULT 0x8000
#define REQ_EXISTS 0x0800
#define REQ_FOLDER 0x0400
#define REQ_FILE   0x0200

#endif