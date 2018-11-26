#ifndef _MAIN_H
#define _MAIN_H 1

void initGlobals();
void setupScreen(int width);
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

int parsePath(char* path, char* devicename);
char* parsePathParam(struct DeviceServiceRoutine** dsr);

#endif