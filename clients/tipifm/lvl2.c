#include "lvl2.h"

#include <vdp.h>
#include <conio.h>

#include "dsrutil.h"
#include "mds_dsrlnk.h"
#include "string.h"
#include "strutil.h"

#define LVL2_STATUS *((volatile unsigned char*)0x8350)
#define LVL2_UNIT *((volatile unsigned char*)0x834C)
#define LVL2_PROTECT *((volatile unsigned char*)0x834D)
#define LVL2_PARAMADDR1 *((volatile unsigned int*)0x834E)
#define LVL2_PARAMADDR2 *((volatile unsigned int*)0x8350)


char path2unit(char* currentPath) {
  char drive[9];
  strncpy(drive, currentPath, 9);
  int l = indexof(drive, '.');
  drive[l] = 0;
  if (0 == strcmp("TIPI", drive)) {
    return 0;
  }
  l = strlen(drive);
  return drive[l-1] - '0';
}

unsigned char lvl2_protect(int crubase, char unit, char* filename, char protect) {
  LVL2_PARAMADDR1 = FBUF;
  strpad(filename, 10, ' ');
  vdpmemcpy(FBUF, filename, 10);

  LVL2_UNIT = unit;
  LVL2_STATUS = 0;
  LVL2_PROTECT = protect ? 0xff : 0x00;

  call_lvl2(crubase, LVL2_OP_PROTECT);

  return LVL2_STATUS;
}

unsigned char lvl2_setdir(int crubase, char unit, char* path) {
  LVL2_PARAMADDR1 = FBUF;
  int len = strlen(path);
  if (len > 39) {
    return 0xFE;
  }
  vdpchar(FBUF,(unsigned char) len);
  vdpmemcpy(FBUF+1, path, len);

  LVL2_UNIT = unit;
  LVL2_STATUS = 0;

  call_lvl2(crubase, LVL2_OP_SETDIR);

  return LVL2_STATUS;
}

unsigned char lvl2_mkdir(int crubase, char unit, char* dirname) {
  LVL2_PARAMADDR1 = FBUF;
  strpad(dirname, 10, ' ');
  vdpmemcpy(FBUF, dirname, 10);

  LVL2_UNIT = unit;
  LVL2_STATUS = 0;

  call_lvl2(crubase, LVL2_OP_MKDIR);

  return LVL2_STATUS;
}

unsigned char lvl2_rmdir(int crubase, char unit, char* dirname) {
  LVL2_PARAMADDR1 = FBUF;
  strpad(dirname, 10, ' ');
  vdpmemcpy(FBUF, dirname, 10);

  LVL2_UNIT = unit;
  LVL2_STATUS = 0;

  call_lvl2(crubase, LVL2_OP_DELDIR);

  return LVL2_STATUS;
}

unsigned char lvl2_rename(int crubase, char unit, char* oldname, char* newname) {
  LVL2_PARAMADDR1 = FBUF;
  LVL2_PARAMADDR2 = FBUF + 10;

  strpad(oldname, 10, ' ');
  strpad(newname, 10, ' ');
  vdpmemcpy(LVL2_PARAMADDR1, newname, 10);
  vdpmemcpy(LVL2_PARAMADDR2, oldname, 10);

  LVL2_UNIT = unit;
  LVL2_STATUS = 0;

  call_lvl2(crubase, LVL2_OP_RENAME);

  return LVL2_STATUS;
}

unsigned char lvl2_rendir(int crubase, char unit, char* oldname, char* newname) {
  LVL2_PARAMADDR1 = FBUF;
  LVL2_PARAMADDR2 = FBUF + 10;

  strpad(oldname, 10, ' ');
  strpad(newname, 10, ' ');
  vdpmemcpy(LVL2_PARAMADDR1, newname, 10);
  vdpmemcpy(LVL2_PARAMADDR2, oldname, 10);

  LVL2_UNIT = unit;
  LVL2_STATUS = 0;

  call_lvl2(crubase, LVL2_OP_RENDIR);

  return LVL2_STATUS;
}

unsigned char lvl2_input(int crubase, char unit, char* filename, unsigned char blockcount, struct AddInfo* addInfoPtr) {
  LVL2_PARAMADDR1 = FBUF;
  strpad(filename, 10, ' ');
  vdpmemcpy(FBUF, filename, 10);

  LVL2_UNIT = unit;
  LVL2_PROTECT = blockcount;
  LVL2_STATUS = ((unsigned int) addInfoPtr) - 0x8300;

  addInfoPtr->buffer = FBUF + 10;

  call_lvl2(crubase, LVL2_OP_INPUT);

  return LVL2_STATUS;
}

unsigned char lvl2_output(int crubase, char unit, char* filename, unsigned char blockcount, struct AddInfo* addInfoPtr) {
  LVL2_PARAMADDR1 = FBUF;
  strpad(filename, 10, ' ');
  vdpmemcpy(FBUF, filename, 10);

  LVL2_UNIT = unit;
  LVL2_PROTECT = blockcount;
  LVL2_STATUS = ((unsigned int) addInfoPtr) - 0x8300;

  addInfoPtr->buffer = FBUF + 10;

  call_lvl2(crubase, LVL2_OP_OUTPUT);

  return LVL2_STATUS;
}

unsigned int __attribute__((noinline)) subroutine(int crubase, unsigned char operation) {
  enableROM(crubase);
  unsigned int addr = 0;
  struct DeviceRomHeader* dsrrom = (struct DeviceRomHeader*) 0x4000;
  struct NameLink* entry = (struct NameLink*) dsrrom->basiclnk;
  while(entry != 0) {
    if (entry->name[0] == 1 && entry->name[1] == operation) {
      addr = entry->routine;
      break;
    }
    entry = entry->next;
  }
  disableROM(crubase);
  return addr;
}

void __attribute__((noinline)) call_lvl2(int crubase, unsigned char operation) {
  unsigned int addr = subroutine(crubase, operation);
  if (addr == 0) {
    LVL2_STATUS = 0xFF;
    return;
  }

  __asm__(
    	" mov %0,@>83F8		; prepare GPLWS r12 with crubase\n"
      " mov %1,@>83F2   ; set r9 to subroutine address\n"
    	"	lwpi 0x83e0     ; get gplws\n"
      " sbo 0           ; turn on card dsr\n"
      " bl *r9          ; call rubroutine\n"
      " nop             ; lvl2 routines never 'skip' request\n"
      " sbz 0           ; turn off card dsr\n"
      " lwpi 0x8300     ; assuming gcc workspace is here\n"
      :
      : "r" (crubase), "r" (addr)
      : "r12"
  );
}