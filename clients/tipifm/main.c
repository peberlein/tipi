
#include "dsrutil.h"
#include "strutil.h"
#include "oem.h"
#include "tifloat.h"
#include "main.h"

#include <string.h>
#include <conio.h>
#include <system.h>
#include <kscan.h>
#include <vdp.h>

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

char buffer[256];

void initGlobals() {
  lvol.volname[0] = 0;
  buffer[0] = 0;
}

void sleep(int jiffies) {
  for(int i=0; i<jiffies;i++) {
    VDP_WAIT_VBLANK_CRU;
  }
}

void setupScreen(int width) {
  if (width == 80) {
    set_text80();
  } else if(width == 40) {
    set_text();
  }

  defineChars();
  bgcolor(COLOR_CYAN);
  textcolor(COLOR_BLACK);
  clrscr();
  gotoxy(0,23);
}

void titleScreen() {
  cprintf("TIPIMAN v%s\n", TIPIMAN_VER);
  cprintf("File Manager for TIPI\n");
  cprintf("www.jedimatt42.com\n\n");
}

void main()
{
  initGlobals();
  setupScreen(40);
  titleScreen();
  loadDriveDSRs();

  while(1) {
    strset(buffer, 0, 255);
    cprintf("\n$ ");
    getstr(2, 23, buffer, 255);
    cprintf("\n");
    handleCommand(buffer);
  }
}

#define MATCH(x,y) (!(strcmpi(x,y)))

void handleCommand(char *buffer) {
  char* tok = strtok(buffer, " ");
  if (MATCH(tok, "dir")) {
    handleDir();
  } else if (MATCH(tok, "drives")) {
    handleDrives();
  } else if (MATCH(tok, "width")) {
    handleWidth();
  } else if (MATCH(tok, "quit")) {
    handleQuit();
  } else {
    cprintf("unknown command: %s\n", tok);
  }
}

void handleDrives() {
  int i = 0;
  while(dsrList[i].name[0] != 0) {
    cprintf("(%x) %s\n", dsrList[i].crubase, dsrList[i].name);
    i++;
  }
}

void onVolInfo(struct VolInfo* volInfo) {
  cprintf("Vol: %s\n", volInfo->volname);
}

void onDirEntry(struct DirEntry* dirEntry) {
  cprintf("%s", dirEntry->name);
  gotoxy(11,23);
  cprintf("%s %d\n", ftypes[dirEntry->type - 1], dirEntry->reclen);
}
 
void handleDir() {
  loadDir(&dsrList[0], "DSK0.", onVolInfo, onDirEntry);
}

void handleWidth() {
  char* tok = strtok(0, " ");
  if (MATCH(tok, "80")) {
    setupScreen(80);
  } else if (MATCH(tok, "40")) {
    setupScreen(40);
  } else {
    cprintf("help: width n\n  n: 80|40\n");
  }
}

void handleQuit() {
  exit();
}