#include "dsrutil.h"

#include <conio.h>
#include "strutil.h"
#include "tifloat.h"
#include <string.h>

struct VolInfo lvol;

struct DeviceServiceRoutine dsrList[40];

unsigned char loadDir(struct DeviceServiceRoutine* dsr, const char* pathname, vol_entry_cb vol_cb, dir_entry_cb dir_cb) {
  struct PAB pab;
  
  struct VolInfo volInfo;
  struct DirEntry dirEntry;

  unsigned char ferr = dsr_open(dsr, &pab, pathname, FBUF, DSR_TYPE_INPUT | DSR_TYPE_INTERNAL | DSR_TYPE_SEQUENTIAL, 38);
  if (ferr) {
    cputsxy(40,23,"open-err ");
    cputs(int2str(ferr));
    return ferr;
  }

  int recNo = 0;
  ferr = DSR_ERR_NONE;
  while(ferr == DSR_ERR_NONE) {
    unsigned char cbuf[38];
    ferr = dsr_read(dsr, &pab, recNo);
    if (ferr == DSR_ERR_NONE) {
      // Now FBUF has the data... 
      vdpmemread(FBUF, cbuf, pab.CharCount);
      // process Record
      if (recNo == 0) {
        basicToCstr(cbuf, volInfo.volname);
        vol_cb(&volInfo);
      } else {
        int namlen = basicToCstr(cbuf, dirEntry.name);
        int a = ti_floatToInt(cbuf+1+namlen);
        int j = ti_floatToInt(cbuf+10+namlen);
        int k = ti_floatToInt(cbuf+19+namlen);
        dirEntry.type = a;
        dirEntry.sectors = j;
        dirEntry.reclen = k;
        if (dirEntry.name[0] != 0) {
          dir_cb(&dirEntry);
        }
      }
      recNo++;
    }
  }

  ferr = dsr_close(dsr, &pab);
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

unsigned char dsr_open(struct DeviceServiceRoutine* dsr, struct PAB* pab, const char* fname, int vdpbuffer, unsigned char flags, int reclen) {
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

  return callLevel3(dsr, pab, VPAB);
}

unsigned char dsr_close(struct DeviceServiceRoutine* dsr, struct PAB* pab) {
  pab->OpCode = DSR_CLOSE;

  return callLevel3(dsr, pab, VPAB);
}

// the data read is in FBUF, the length read in pab->CharCount
// typically passing 0 in for record number will let the controller
// auto-increment it. 
unsigned char dsr_read(struct DeviceServiceRoutine* dsr, struct PAB* pab, int recordNumber) {
  pab->OpCode = DSR_READ;
  pab->RecordNumber = recordNumber;
  pab->CharCount = 0;

  unsigned char result = callLevel3(dsr, pab, VPAB);
  vdpmemread(VPAB + 5, (&pab->CharCount), 1);
  return result;
}

/*
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
*/

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

void invokeLevel3(struct DeviceServiceRoutine* dsr, char* devicename) {
  unsigned int routine = dsr->addr;

  if (routine != 0) {
    enableROM(dsr->crubase);
    __asm__("mov %0,@>83F2\n\t"
            "lwpi >83E0\n\t"
            "bl *r9\n\t"
            "nop\n\t"
            "lwpi >8300"
            : : "r"(routine) : "r9"
    );
    disableROM(dsr->crubase);
  }
}


#define DSR_NAME_LEN	*((volatile unsigned int*)0x8354)

unsigned char callLevel3(struct DeviceServiceRoutine* dsr, struct PAB* pab, unsigned int vdp) {
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
  invokeLevel3(dsr, buf);

	// now return the result
	return GET_ERROR(vdpreadchar(status));
}
