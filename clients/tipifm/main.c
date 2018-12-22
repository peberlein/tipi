
#include "dsrutil.h"
#include "lvl2.h"
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

#define OPTIONAL 0
#define REQUIRED 1

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
int displayWidth;
int column;

void initGlobals() {
  buffer[0] = 0;
  displayWidth = 40;
  column = 0;
}

void sleep(int jiffies) {
  for(int i=0; i<jiffies;i++) {
    VDP_WAIT_VBLANK_CRU;
  }
}

int isF18A() {
  unlock_f18a();
  unsigned char testcode[6] = { 0x04, 0xE0, 0x3F, 0x00, 0x03, 0x40 };
  vdpmemcpy(0x3F00, testcode, 6);
  VDP_SET_REGISTER(0x36, 0x3F);
  VDP_SET_REGISTER(0x37, 0x00);
  return !vdpreadchar(0x3F00);
}

void resetF18A() {
  lock_f18a();
  set_graphics(0); // just to reset EVERYTHING
}

void setupScreen(int width) {
  resetF18A();
  bgcolor(COLOR_CYAN);
  textcolor(COLOR_BLACK);
  if (width == 80) {
    displayWidth = 80;
    set_text80_color();
  } else if(width == 40) {
    displayWidth = 40;
    set_text();
  }

  defineChars();
  clrscr();
  gotoxy(0,23);
}

void titleScreen() {
  cprintf("TIPIFM v%s\n", TIPIMAN_VER);
  cprintf("www.jedimatt42.com\n");
}

void main()
{
  initGlobals();
  setupScreen(isF18A() ? 80 : 40);
  titleScreen();
  loadDriveDSRs();
  currentDsr = dsrList;
  strcpy(currentPath, currentDsr->name);
  strcat(currentPath, ".");

  while(1) {
    VDP_INT_POLL;
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
  COMMAND("cd", handleCd)
  else COMMAND("dir", handleDir)
  else COMMAND("drives", handleDrives)
  else COMMAND("exit", handleQuit)
  else COMMAND("help", handleHelp)
  else COMMAND("mkdir", handleMkdir)
  else COMMAND("quit", handleQuit)
  else COMMAND("lvl2", handleLvl2)
  else COMMAND("ver", handleVer)
  else COMMAND("width", handleWidth)
  else cprintf("unknown command: %s\n", tok);
}

void handleCd() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  parsePathParam(&dsr, path, REQUIRED);
  if (dsr == 0) {
    cprintf("no path: drive or folder specified\n");
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }
  unsigned char stat = existsDir(dsr, path);
  if (stat != 0) {
    cprintf("error, device/folder not found: %s\n", path);
    return;
  }
  
  currentDsr = dsr;
  strcpy(currentPath, path);
}

void handleVer() {
  titleScreen();
}

void handleDrives() {
  int i = 0;
  int cb = 0;
  
  while(dsrList[i].name[0] != 0) {
    cb = dsrList[i].crubase;
    cprintf("%x -", cb);
    while (cb == dsrList[i].crubase) {
      cprintf(" %s", dsrList[i].name);
      i++;
    }
    cprintf("\n");
  }
}

void handleLvl2() {
  char* tok = strtok(0, " ");
  int crubase = htoi(tok);

  if (crubase == 0) {
    cprintf("no crubase specified\n");
    return;
  }

  enableROM(crubase);
  struct DeviceRomHeader* rom = (struct DeviceRomHeader*)0x4000;

  struct NameLink* link = rom->basiclnk;
  while(link != 0) {
    if (link->name[0] == 1) {
      cprintf(" >%x", link->name[1]);
    }
    link = link->next;
  }
  cprintf("\n");

  disableROM(crubase);
}

void onVolInfo(struct VolInfo* volInfo) {
  cprintf("Vol: %s\n", volInfo->volname);
  column = 0;
}

void onDirEntry(struct DirEntry* dirEntry) {
  gotoxy(column,23);
  cprintf("%s", dirEntry->name);
  gotoxy(column + 11,23);
  cprintf(ftypes[dirEntry->type - 1]);
  if (dirEntry->reclen != 0) {
    cprintf(" %d", dirEntry->reclen);
  }
  column += 20;
  if (column == displayWidth) {
    cprintf("\n");
    column = 0;
  }
}

int parsePath(char* path, char* devicename) {
  char workbuf[14];
  int crubase = 0;
  strncpy(workbuf, path, 14);
  char* tok = strtok(workbuf, ". ");
  if (tok != 0 && tok[0] == '1' && strlen(tok) == 4) {
    crubase = htoi(tok);
    tok = strtok(0, ". ");
    strcpy(devicename, tok);
  } else {
    strcpy(devicename, tok);
  }
  return crubase;
}

void parsePathParam(struct DeviceServiceRoutine** dsr, char* buffer, int requirements) {
  buffer[0] = 0; // null terminate so later we can tell if it is prepared or not.
  char* path = strtok(0, " ");
  *dsr = currentDsr;
  if (path == 0) {
    if (requirements & REQ_DEFAULT == 0) {
      *dsr = 0;
      return;
    }
    path = currentPath;
  } else {
    char devicename[8];
    if (0 == strcmp("..", path)) {
      int ldot = lindexof(currentPath, '.', strlen(currentPath) - 2);
      if (ldot == -1) {
        *dsr = 0;
        cprintf("No parent folder\n");
        return;
      }
      strncpy(buffer, currentPath, ldot + 1);
      return;
    } else {
      int crubase = parsePath(path, devicename);
      *dsr = findDsr(devicename, crubase);
      if (*dsr == 0) {
        // not a base device, so try subdir
        strcpy(buffer, currentPath);
        strcat(buffer, path);
        crubase = parsePath(buffer, devicename);
        *dsr = findDsr(devicename, crubase);
        // if still not found, then give up.
        if (*dsr == 0) {  
          cprintf("device not found.\n");
          return;
        }
      }
      if (crubase != 0) {
        path = strtok(path, ".");
        path = strtok(0, " ");
      }
    }
  }
  // Todo: test for existance and matching requirements
  if (buffer[0] == 0) {
    strcpy(buffer, path);
  }
}

void handleDir() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  parsePathParam(&dsr, path, OPTIONAL);
  if (dsr == 0) {
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }

  unsigned char stat = existsDir(dsr, path);
  if (stat != 0) {
    cprintf("error, device/folder not found: %s\n", path);
    return;
  }

  loadDir(dsr, path, onVolInfo, onDirEntry);
  column = 0;
  cprintf("\n");
}

void handleMkdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    cprintf("error, must specify a directory name\n");
    return;
  }

  char unit = path2unit(currentPath);
  cprintf("mkdir, unit %d, dirname %s\n", unit, dirname);

  unsigned char err = lvl2_mkdir(currentDsr->crubase, unit, dirname);
  cprintf("mkdir err: %d\n", err);
  if (err) {
    cprintf("cannot create directory %s%s\n", currentPath, dirname);
  }
}

void handleWidth() {
  char* tok = strtok(0, " ");
  int width = atoi(tok);

  if (width == 40 || width == 80) {
    setupScreen(width);
  } else {
    cprintf("no width specified\n");
  }
}

void handleQuit() {
  resetF18A();
  exit();
}