
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
struct DeviceServiceRoutine* currentDsr;
char currentPath[256];

void initGlobals() {
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
  cprintf("TIPIFM v%s\n", TIPIMAN_VER);
  cprintf("File Manager for TIPI\n");
  cprintf("www.jedimatt42.com\n\n");
}

void main()
{
  initGlobals();
  setupScreen(40);
  titleScreen();
  loadDriveDSRs();
  currentDsr = dsrList;
  strcpy(currentPath, currentDsr->name);
  strcat(currentPath, ".");

  while(1) {
    strset(buffer, 0, 255);
    cprintf("\n[%x.%s]\n$ ", currentDsr->crubase, currentPath);
    getstr(2, 23, buffer, 255);
    cprintf("\n");
    handleCommand(buffer);
  }
}

#define MATCH(x,y) (!(strcmpi(x,y)))

#define COMMAND(x, y) if (MATCH(tok, x)) y();

void handleCommand(char *buffer) {
  char* tok = strtok(buffer, " ");
  COMMAND("dir", handleDir)
  else COMMAND("drives", handleDrives)
  else COMMAND("width", handleWidth)
  else COMMAND("quit", handleQuit)
  else COMMAND("ver", handleVer)
  else cprintf("unknown command: %s\n", tok);
}

void handleVer() {
  titleScreen();
}

void handleDrives() {
  int i = 0;
  int cb = 0;
  while(dsrList[i].name[0] != 0) {
    if (cb != dsrList[i].crubase) {
      if (cb == 0) {
        cprintf("\n");
      }
      cb = dsrList[i].crubase;
      cprintf("%x -", cb);
    }
    cprintf(" %s", dsrList[i].name);
    i++;
  }
  cprintf("\n");
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
  char* path = strtok(0, " ");
  struct DeviceServiceRoutine* dsr = currentDsr;
  if (path == 0) {
    path = currentPath;
  } else {
    dsr = findDsr(path);
    if (dsr == 0) {
      cprintf("error: no device found");
      return;
    }
  }

  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }
  loadDir(dsr, path, onVolInfo, onDirEntry);
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