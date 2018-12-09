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

#define LVL2_OP_PROTECT 0x12
#define LVL2_OP_MKDIR 0x18

char lvl2_name[2];

void setPab(struct PAB* pab) {
  pab->OpCode = 0;
  pab->Status = 0;
  pab->RecordLength = 0;
  pab->RecordNumber = 0;
  pab->ScreenOffset = 0;
  pab->NameLength = 0;
  pab->CharCount = 0;
  pab->VDPBuffer = FBUF;
  pab->pName = lvl2_name;
}

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
  struct PAB pab;
  setPab(&pab);

  lvl2_name[1] = 0;
  lvl2_name[0] = LVL2_OP_PROTECT;

  LVL2_PARAMADDR1 = FBUF;
  strpad(filename, 10, ' ');
  vdpmemcpy(FBUF, filename, 10);

  LVL2_UNIT = unit;
  LVL2_STATUS = 0;
  LVL2_PROTECT = protect;

  unsigned char ferr = mds_dsrlnk(crubase, &pab, VPAB, DSR_MODE_LVL2);

  return LVL2_STATUS | ferr;
}

unsigned char lvl2_mkdir(int crubase, char unit, char* dirname) {
  struct PAB pab;
  setPab(&pab);

  lvl2_name[1] = 0;
  lvl2_name[0] = LVL2_OP_MKDIR;

  LVL2_PARAMADDR1 = FBUF;
  strpad(dirname, 10, ' ');
  vdpmemcpy(FBUF, dirname, 10);

  LVL2_UNIT = unit;
  LVL2_STATUS = 0;

  unsigned char ferr = mds_dsrlnk(crubase, &pab, VPAB, DSR_MODE_LVL2);

  return LVL2_STATUS | ferr;
}