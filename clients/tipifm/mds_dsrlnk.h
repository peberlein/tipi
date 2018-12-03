#ifndef mds_dsrlnk_h
#define mds_dsrlnk_h

unsigned char mds_dsrlnk(int crubase, struct PAB *pab, unsigned int vdp);
void __attribute__((noinline)) mds_dsrlnkraw(int crubase, unsigned int vdp);

#endif