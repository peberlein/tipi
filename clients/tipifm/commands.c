#include "commands.h"
#include "globals.h"
#include "main.h"
#include "parsing.h"
#include "strutil.h"
#include "lvl2.h"

#include <system.h>
#include <conio.h>
#include <string.h>

void handleCd() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  parsePathParam(&dsr, path, PR_REQUIRED);
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

void handleCopy() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }

  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  parsePathParam(&dsr, path, PR_REQUIRED);
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

void handleDir() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  parsePathParam(&dsr, path, PR_OPTIONAL);
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
  cprintf("\n");
}

void handleMkdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    cprintf("error, must specify a directory name\n");
    return;
  }

  char unit = path2unit(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_mkdir(currentDsr->crubase, unit, dirname);
  if (err) {
    cprintf("cannot create directory %s%s\n", currentPath, dirname);
  }
}

void handleProtect() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }

  char unit = path2unit(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_protect(currentDsr->crubase, unit, filename, 1);
  if (err) {
    cprintf("cannot protect file %s%s\n", currentPath, filename);
  }
}

void handleUnprotect() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }

  char unit = path2unit(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_protect(currentDsr->crubase, unit, filename, 0);
  if (err) {
    cprintf("cannot unprotect file %s%s\n", currentPath, filename);
  }
}

void handleDelete() {

}

void handleRename() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify source file name\n");
    return;
  }
  char* newname = strtok(0, " ");
  if (newname == 0) {
    cprintf("error, must specify new file name\n");
    return;
  }

  char unit = path2unit(currentPath);

  char path[256];
  strcpy(path, currentPath);
  strcat(path, ".");
  strcat(path, filename);

  unsigned char stat = existsDir(currentDsr, path);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);
  unsigned char err = 0xff;
  if (stat == 0) {
    err = lvl2_rendir(currentDsr->crubase, unit, filename, newname);
  } else {
    err = lvl2_rename(currentDsr->crubase, unit, filename, newname);
  }

  if (err) {
    cprintf("cannot rename file %s%s\n", currentPath, filename);
  }
}

void handleRmdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    cprintf("error, must specify a directory name\n");
    return;
  }

  char unit = path2unit(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_rmdir(currentDsr->crubase, unit, dirname);
  if (err) {
    cprintf("cannot remove directory %s%s\n", currentPath, dirname);
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

void handleExit() {
  resetF18A();
  exit();
}