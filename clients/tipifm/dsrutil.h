#ifndef _DSR_H
#define _DSR_H 1

#include <files.h>

#define VPAB 0x3200
#define FBUF 0x3000

#define DSR_STATUS_EOF DST_STATUS_EOF

// Casting rom locations to the next 3 structs should ease 
// reasoning about any code accessing the rom header and
// lists.

struct __attribute__((__packed__)) EntryLink {
  struct EntryLink* next;
  unsigned int* routine;
};

struct __attribute__((__packed__)) NameLink {
  struct NameLink* next;
  unsigned int routine;
  char name[8]; // length byte + [upto] 7 characters. 
};

struct __attribute__((__packed__)) DeviceRomHeader {
  unsigned char flag;
  unsigned char version;
  unsigned int reserved1;
  struct EntryLink* powerlnk;
  struct NameLink* userlnk;
  struct NameLink* dsrlnk;
  struct NameLink* basiclnk;
  struct EntryLink* interruptlnk;
  unsigned int* gromsomething;
};

// fun with 'drives'
struct __attribute__((__packed__)) DeviceServiceRoutine {
  char name[8];
  int crubase;
  unsigned int addr;
  char unit;
}; 

// A cache of dsr names and addresses.
extern struct DeviceServiceRoutine dsrList[40];

struct __attribute__((__packed__)) DirEntry {
  char name[11];
  int type;
  int sectors;
  int reclen;
}; 

extern struct DirEntry lentries[128];
extern struct DirEntry rentries[128];

struct __attribute__((__packed__)) VolInfo {
  char name[11];
};

extern struct VolInfo lvol;
extern struct VolInfo rvol;

unsigned char dsr_open(struct PAB* pab, const char* fname, int vdpbuffer, unsigned char flags, int reclen);
unsigned char dsr_close(struct PAB* pab);
unsigned char dsr_read(struct PAB* pab, int recordNumber);
unsigned char dsr_write(struct PAB* pab, unsigned char* record);

unsigned char loadDir(const char* pathname, int leftOrRight);

void loadDriveDSRs();

void enableROM(int crubase);
void disableROM(int crubase);
int isDrive(char *basicstr);
unsigned char callLevel3(int crubase, struct PAB* pab, unsigned int vdp);
unsigned int findLevel3(int crubase, char* devicename);

#endif
