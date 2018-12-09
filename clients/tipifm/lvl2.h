#ifndef _LVL2_H
#define _LVL2_H 1

#include "dsrutil.h"

unsigned char lvl2_mkdir(int crubase, char unit, char* dirname);
unsigned char lvl2_setdir(int crubase, char unit, char* dirpath);
unsigned char lvl2_rmdir(int crubase, char unit, char* dirname);


char path2unit(char* currentPath);

#endif