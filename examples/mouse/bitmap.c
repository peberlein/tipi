
#include <vdp.h>
#include "bitmap.h"


unsigned char gfx_font[8*CHAR_COUNT];

void bm_plotBit(unsigned int x, unsigned int y) {
  unsigned int addr = (8 * (x/8)) + (256 * (y/8)) + (y%8);
  VDP_SET_ADDRESS(addr);
  unsigned char bits = VDPRD;
  bits = bits | (0x80 >> (x%8));
  VDP_SET_ADDRESS_WRITE(addr);
  VDPWD = bits;
}

void bm_setColor(unsigned int x, unsigned int y, unsigned char foreground, unsigned char background) {
  unsigned int addr = (8 * (x/8)) + (256 * (y/8)) + (y%8);
  VDP_SET_ADDRESS(gColor + addr);
  VDPWD = (foreground << 4) + (background & 0x0F);
}

// limited to 32 x 24 character placement grid.
void bm_placePattern(unsigned char x, unsigned char y, const unsigned char* pattern) {
  unsigned int blockAddr = gPattern + (x * 8) + (y * 8 * 32);
  vdpmemcpy(blockAddr, pattern, 8);
}

// assumes font is space, numbers, alphabet (no lowercase or symbols)
void bm_placeFontChar(unsigned char x, unsigned char y, unsigned char alphanum) {
  unsigned char* fontPattern = (unsigned char*) gfx_font + (((int) alphanum - 32) * 8);
  bm_placePattern(x, y, fontPattern);
}

void bm_putsxy(unsigned char x, unsigned char y, unsigned char* str) {
  unsigned char* cursor = str;
  while(*cursor != 0) {
    bm_placeFontChar(x++, y, *cursor++);
  }
}

void bm_loadFont() {
  // setup graphics mode and load the charset from grom
  // then copy those patterns out to RAM
  set_graphics(0);
  charset();
  vdpmemread(gPattern + (8 * 32), gfx_font, 8 * CHAR_COUNT);
}