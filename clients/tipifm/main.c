
#include "dsrutil.h"
#include "strutil.h"
#include "oem.h"
#include "tifloat.h"
#include "main.h"

#define DSR_STATUS_EOF DST_STATUS_EOF

#include <string.h>
#include <conio.h>
#include <system.h>


#define GPLWS ((unsigned int*)0x83E0)
#define DSRTS ((unsigned char*)0x401A)

#define TIPIMAN_VER "1"

const char* const ftypes[] = {
  "D/F",
  "D/V",
  "I/F",
  "I/V",
  "PRG",
  "DIR"
};

void initGlobals() {
  lentries[0].name[0] = 0;
  rentries[0].name[0] = 0;
  lvol.name[0] = 0;
  rvol.name[0] = 0;
}

void sleep(int jiffies) {
  for(int i=0; i<jiffies;i++) {
    VDP_WAIT_VBLANK_CRU;
  }
}

void setupScreen() {
  set_text80();
  defineChars();
  bgcolor(COLOR_CYAN);
  textcolor(COLOR_BLACK);
}

void titleScreen() {
  clrscr();
  drawBox2(25,8,54,16);
  gotoxy(35,10);
  cputs("TIPIMAN v");
  cputs(TIPIMAN_VER);
  gotoxy(30,12);
  cputs("File Manager for TIPI");
  gotoxy(33,14);
  cputs("ti994a.cwfk.net");
}

void printPadded(int x, int y, char* str, int width) {
  cclearxy(x, y, width);
  cputsxy(x, y, str);
}

void headings(int x) {
  cputsxy(x+2,1, "Name");
  cputsxy(x+23,1, "Type");
  cputsxy(x+28,1, "Rec");
  cputsxy(x+33,1, "Sect");
}

void showMenu() {
  cclearxy(0, 23, 79);
  cputsxy(0,23,"[L]eft");
  cputsxy(10,23,"[R]ight");
}

void layoutScreen() {
  clrscr();
  drawBox1(0, 0, 39, 22);
  drawBox1(40, 0, 79, 22);
  
  cputcxy(2, 0, LEFT_T);
  printPadded(3,0, "", 10);
  cputcxy(13, 0, RIGHT_T);
  cputcxy(42, 0, LEFT_T);
  printPadded(43,0, "", 10);
  cputcxy(53, 0, RIGHT_T);
  headings(1);
  headings(41);
  showMenu();
  showVolInfo(0);
  drawEntries(0, 0);
  showVolInfo(1);
  drawEntries(0, 1);
}

void showVolInfo(int leftOrRight) {
  struct VolInfo* volInfo = leftOrRight ? &rvol : &rvol;
  int x = leftOrRight ? 43 : 3;
  cputsxy(x, 0, volInfo->name);
}

void drawEntries(int start, int leftOrRight) {
  int done = 0;
  int bx = 1;
  struct DirEntry* entries = lentries;
  if (leftOrRight) {
    bx = 41;
    entries = rentries;
  }
  for(int i=0;i<20;i++) {
    int yi = 2+i;
    gotoxy(bx, yi);
    cclear(38);
    struct DirEntry* entry = &(entries[i+start]);
    if (!done && entry->name[0] != 0) {
      gotoxy(bx+2, yi);
      cputs(entry->name);
      gotoxy(bx+23, yi);
      cputs(ftypes[entry->type - 1]);
      if (entry->type < 4) {
        gotoxy(bx+28, yi);
        cputs(int2str(entry->reclen));
      }
      if (entry->type != 6) {
        gotoxy(bx+33, yi);
        cputs(int2str(entry->sectors));
      }
    } else {
      done = 1;
    }
  }
}

void catalogDrive(char* drive, int leftOrRight) {
  loadDir(drive, leftOrRight);
  showVolInfo(leftOrRight);
  drawEntries(0, leftOrRight);
}

void main()
{
  initGlobals();
  setupScreen();
  titleScreen();
  loadDriveDSRs();
  sleep(30);
  layoutScreen();

  catalogDrive("DSK2.", 0);

  while(1) {
    // char drive[32];
    // strcpy(drive, "TIPI.");  
    // getstr(0,23, drive, 32);
    // catalogDrive(drive, 0);
    // getstr(0,23, drive, 32);
    // catalogDrive(drive, 1);
  }
}

