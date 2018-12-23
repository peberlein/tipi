#ifndef _LVL2_H
#define _LVL2_H 1

#include "dsrutil.h"

#define LVL2_OP_PROTECT 0x12
#define LVL2_OP_RENAME 0x13
#define LVL2_OP_INPUT 0x14
#define LVL2_OP_OUTPUT 0x15
#define LVL2_OP_SETDIR 0x17
#define LVL2_OP_MKDIR 0x18
#define LVL2_OP_DELDIR 0x19
#define LVL2_OP_RENDIR 0x1A

unsigned char lvl2_mkdir(int crubase, char unit, char* dirname);
unsigned char lvl2_protect(int crubase, char unit, char* filename, char protect);
unsigned char lvl2_rename(int crubase, char unit, char* oldname, char* newname);
unsigned char lvl2_rendir(int crubase, char unit, char* oldname, char* newname);
unsigned char lvl2_rmdir(int crubase, char unit, char* dirname);
unsigned char lvl2_setdir(int crubase, char unit, char* path);

char path2unit(char* currentPath);
unsigned int __attribute__((noinline)) subroutine(int crubase, unsigned char operation);
void __attribute__((noinline)) call_lvl2(int crubase, unsigned char operation);

#endif