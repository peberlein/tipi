
#include <vdp.h>
#include <system.h>
#include <kscan.h>

#include "patterns.h"
#include "tipi_mouse.h"
#include "bitmap.h"

#define SCREEN_COLOR (COLOR_BLACK << 4) + COLOR_CYAN

#define SPR_MOUSE0 0
#define SPR_MOUSE1 1
#define true 1
#define false 0

#define FCTNERASE 7
#define FCTNREDO 6
#define FCTNQUIT 5

unsigned int pointerx;
unsigned int pointery;

void sprite_pos(unsigned int n, unsigned int r, unsigned int c) {
  unsigned int addr=gSprite+(n<<2);
  VDP_SET_ADDRESS_WRITE(addr);
  VDPWD=r;
  VDPWD=c;
}



void main() {
  bm_loadFont();

  // now go to bitmap/graphics mode 2
  set_bitmap(VDP_SPR_16x16);
  vdpwriteinc(gImage,0,768);
  vdpmemset(gColor,SCREEN_COLOR,768*8);  
  vdpmemset(gPattern,0,768*8);

  // Load Sprite patterns
  vdpmemcpy(gSpritePat, gfx_point0, 32);
  vdpmemcpy(gSpritePat + 32, gfx_point1, 32);

  bm_putsxy(0,0, "0123456789");
  bm_putsxy(0,1, "ABCDEFGHIJ");
  bm_putsxy(0,2, "KLMNOPQRST");
  bm_putsxy(0,3, "W  X  Y  Z");

  pointerx = 256/2;
  pointery = 192/2;

  sprite(SPR_MOUSE0, 0, COLOR_BLACK, pointery - 1, pointerx);
  sprite(SPR_MOUSE1, 4, COLOR_WHITE, pointery - 1, pointerx);

  halt();

  while(true) {
    unsigned char k = kscan(0);
    if (k == FCTNQUIT) {
      __asm__("blwp @>0000");
    }
    VDP_WAIT_VBLANK_CRU;
    
    tipiMouseRead();

    if (mousex != 0 || mousey != 0) {
      pointerx += (2 * mousex) / 3;
      pointery += (2 * mousey) / 3;

      if (pointerx > 0xF000) {
        pointerx = 0;
      } else if (pointerx > 255) {
        pointerx = 255;
      }

      if (pointery > 0xF000) {
        pointery = 0;
      } else if (pointery > 191) {
        pointery = 191;
      }

      sprite_pos(SPR_MOUSE0, pointery - 1, pointerx);
      sprite_pos(SPR_MOUSE1, pointery - 1, pointerx);
    }

    if (mouseb & MB_LEFT) {
      bm_plotBit(pointerx,pointery);
    }
    if (mouseb & MB_RIGHT) {
      vdpmemset(gPattern,0,768*8);
    }
  }
}
