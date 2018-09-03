#include "dsrutil.h"

#include <vdp.h>
#include "strutil.h"
#include "tifloat.h"
#include <string.h>

int volRecordHandler(char* buf, struct VolInfo* volInfo);
int catRecordHandler(char* buf, struct DirEntry* entry);

struct VolInfo lvol;
struct VolInfo rvol;

struct DeviceServiceRoutine dsrList[40];
struct DirEntry lentries[128];
struct DirEntry rentries[128];

unsigned char loadDir(const char* pathname, int leftOrRight) {
  struct PAB pab;
  
  unsigned char ferr = dsr_open(&pab, pathname, FBUF, DSR_TYPE_INPUT | DSR_TYPE_INTERNAL | DSR_TYPE_SEQUENTIAL, 38);
  if (ferr) {
    cputsxy(40,23,"open-err ");
    cputs(int2str(ferr));
    return ferr;
  }

  struct VolInfo* volInfo = &lvol;
  struct DirEntry* entryList = lentries; 
  if (leftOrRight) {
    volInfo = &rvol;
    entryList = rentries;
  }

  int recNo = 0;
  ferr = DSR_ERR_NONE;
  while(ferr == DSR_ERR_NONE) {
    unsigned char cbuf[38];
    ferr = dsr_read(&pab, recNo);
    if (ferr == DSR_ERR_NONE) {
      // Now FBUF has the data... 
      vdpmemread(FBUF, cbuf, pab.CharCount);
      // process Record
      if (recNo == 0) {
        volRecordHandler(cbuf, volInfo);
      } else {
        catRecordHandler(cbuf, &entryList[recNo - 1]);
        entryList[recNo].name[0] = 0;
      }
      recNo++;
    }
  }

  ferr = dsr_close(&pab);
  if (ferr) {
    return ferr;
  }
}


//---- the following are meant to be easy, not fast ----

void initPab(struct PAB* pab) {
  pab->OpCode = DSR_OPEN;
  pab->Status = DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL | DSR_TYPE_INPUT;
  pab->RecordLength = 80;
  pab->RecordNumber = 0;
  pab->ScreenOffset = 0;
  pab->NameLength = 0;
  pab->CharCount = 0;
}

unsigned char dsr_open(struct PAB* pab, const char* fname, int vdpbuffer, unsigned char flags, int reclen) {
  initPab(pab);
  pab->OpCode = DSR_OPEN;
  if (flags != 0) {
    pab->Status = flags;
  }
  if (reclen != 0) {
    pab->RecordLength = reclen;
  }
  pab->pName = (char*)fname;
  pab->VDPBuffer = vdpbuffer;

  return callLevel3(0x1100, pab, VPAB);
}

unsigned char dsr_close(struct PAB* pab) {
  pab->OpCode = DSR_CLOSE;

  return callLevel3(0x1100, pab, VPAB);
}

// the data read is in FBUF, the length read in pab->CharCount
// typically passing 0 in for record number will let the controller
// auto-increment it. 
unsigned char dsr_read(struct PAB* pab, int recordNumber) {
  pab->OpCode = DSR_READ;
  pab->RecordNumber = recordNumber;
  pab->CharCount = 0;

  unsigned char result = callLevel3(0x1100, pab, VPAB);
  vdpmemread(VPAB + 5, (&pab->CharCount), 1);
  return result;
}

int volRecordHandler(char* buf, struct VolInfo* volInfo) {
  basicToCstr(buf, volInfo->name);
  return 0;
}

int entry_row = 0;

int catRecordHandler(char* buf, struct DirEntry* entry) {
  int namlen = basicToCstr(buf, entry->name);
  int a = ti_floatToInt(buf+1+namlen);
  int j = ti_floatToInt(buf+10+namlen);
  int k = ti_floatToInt(buf+19+namlen);
  entry->type = a;
  entry->sectors = j;
  entry->reclen = k;
  entry_row++;
  return 0;
}

void loadDriveDSRs() {
  struct DeviceServiceRoutine* listHead = dsrList;

  int cruscan = 0x1000;
  while(cruscan < 0x2000) {
    enableROM(cruscan);
    struct DeviceRomHeader* dsrrom = (struct DeviceRomHeader*) 0x4000;
    if (dsrrom->flag == 0xAA) {

      struct NameLink* dsrlinks = dsrrom->dsrlnk;

      while(dsrlinks != 0) {
        
        unsigned char* dsrname = (unsigned char*) (dsrlinks + 2);
        if (isDrive(dsrlinks->name)) {
          basicToCstr(dsrlinks->name, listHead->name);
          listHead->crubase = cruscan;
          listHead->addr = dsrlinks->routine;
          listHead += 1;
        }
        
        dsrlinks = dsrlinks->next;
      }
    }

    disableROM(cruscan);
    cruscan += 0x0100;
  }
}

int isDrive(char* basicstr) {
  if (basicstr[0] == 4) {
    if (0 == basic_strcmp(basicstr, "TIPI")) {
      return 1;
    } else if (basicstr[1] >= 'A' && basicstr[1] <= 'Z' && basicstr[4] >= '0' && basicstr[4] <= '9') {
      return 1;
    }
  }
  return 0;
}

void enableROM(int crubase) {
  __asm__("mov %0,r12\n\tsbo 0" : : "r"(crubase) : "r12");
}

void disableROM(int crubase) {
  __asm__("mov %0,r12\n\tsbz 0" : : "r"(crubase) : "r12");
}

void invokeLevel3(int crubase, char* devicename) {
  unsigned int routine = findLevel3(crubase, devicename);

  if (routine != 0) {
    enableROM(crubase);
    __asm__("mov %0,@>83F2\n\t"
            "lwpi >83E0\n\t"
            "bl *r9\n\t"
            "nop\n\t"
            "lwpi >8300"
            : : "r"(routine) : "r9"
    );
    disableROM(crubase);
  }
}


#define DSR_NAME_LEN	*((volatile unsigned int*)0x8354)

unsigned char callLevel3(int crubase, struct PAB* pab, unsigned int vdp) {
  //--- borrowed from Tursi's dsrlnk
	if (pab->NameLength == 0) {
		pab->NameLength = strlen(pab->pName);
	}

	// copies your PAB to VDP and then executes the call
	vdpmemcpy(vdp, (const unsigned char*)pab, 9);
	// assumes vdpmemcpy leaves the VDP address in the right place!
	VDPWD = pab->NameLength;

	// and the filename itself after pab in vdp - note we assume 'x' is valid!
	unsigned char *p = pab->pName;
  int x = pab->NameLength;
	while (x--) {
		VDPWD = *(p++);
	}

	unsigned char buf[8];	// 8 bytes of memory for a name buffer
	unsigned int status = vdp + 1;

	vdp+=9;
	DSR_PAB_POINTER = vdp;

	unsigned char size = vdpreadchar(vdp);
	unsigned char cnt=0;
	while (cnt < 8) {
		buf[cnt] = VDPRD;	// still in the right place after the readchar above got the length
		if (buf[cnt] == '.') {
      // Jedimatt42: turn buf into a cstring
      buf[cnt] = 0;
			break;
		}
		cnt++;
	}
	if ((cnt == 0) || (cnt > 7)) {
		// illegal device name length
		vdpchar(status, DSR_ERR_FILEERROR);
		return GET_ERROR(vdpreadchar(status));
	}
	// save off the device name length (asm below uses it!)
	DSR_NAME_LEN = cnt;
	++cnt;
	DSR_PAB_POINTER += cnt;

  // Jedimatt42: Now call the stored routine from dsrList 
  invokeLevel3(crubase, buf);

	// now return the result
	return GET_ERROR(vdpreadchar(status));
}

unsigned int findLevel3(int crubase, char* devicename) {
  struct DeviceServiceRoutine* head = dsrList;
  while(head->name[0] != 0) {
    if (!strcmp(head->name, devicename)) {
      return head->addr;
    }
    head++;
  }
  return 0;
}
