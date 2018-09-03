#ifndef _MAIN_H
#define _MAIN_H 1

void initGlobals();
void sleep(int jiffies);

void setupScreen();
void titleScreen();

void printPadded(int x, int y, char* str, int width);
void headings(int x);
void showMenu();

void layoutScreen();
void showVolInfo(int leftOrRight);

void drawEntries(int start, int leftOrRight);
void catalogDrive(char* drive, int leftOrRight);

#endif