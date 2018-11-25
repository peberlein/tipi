#ifndef _MAIN_H
#define _MAIN_H 1

void initGlobals();
void setupScreen(int width);
void titleScreen();
void main();
void sleep(int jiffies);
void handleCommand(char * buffer);

void handleDir();
void handleDrives();
void handleWidth();
void handleQuit();
void handleVer();

#endif