#ifndef _BITMAP_H
#define _BITMAP_H


#define CHAR_COUNT ('Z' - ' ' + 1)

extern unsigned char gfx_font[8*CHAR_COUNT];

void bm_plotBit(unsigned int x, unsigned int y);

void bm_setColor(unsigned int x, unsigned int y, unsigned char foreground, unsigned char background);

// limited to 32 x 24 character placement grid.
void bm_placePattern(unsigned char x, unsigned char y, const unsigned char* pattern);

// assumes font is space, numbers, alphabet (no lowercase or symbols)
void bm_placeFontChar(unsigned char x, unsigned char y, unsigned char alphanum);

void bm_putsxy(unsigned char x, unsigned char y, unsigned char* str);

void bm_loadFont();

#endif
