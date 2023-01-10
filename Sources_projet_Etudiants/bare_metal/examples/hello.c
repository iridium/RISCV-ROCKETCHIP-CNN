// See LICENSE.Cambridge for license details.
// A hello world program

#include <stdio.h>
#include <stdint.h>
#include "lowrisc_memory_map.h"
#include "hid.h"
#include "mini-printf.h"
#include "minion_lib.h"

const struct { char scan,lwr,upr; } scancode[] = {
#include "scancode.h"
  };

extern char dos[];

int main() {
  int i, j;
  int height = 11;
  int width = 5;
  int scroll = 0;
  int ghlimit = 40; // 32; // 40;
  hid_send_string("\nBare metal HID access\n");
  printf("Hello World! "__DATE__" "__TIME__"\n");
  hid_reg_ptr[LOWRISC_REGS_GHLIMIT] = ghlimit;
  hid_reg_ptr[LOWRISC_REGS_MODE] = 2;
  for (i = 0; i < 768*32; i++)
    hid_fb_ptr[i] = 0;
#if 0  
  for (i = 0; i < 32; i++)
    {
      for (j = 0; j < 128; j++)
        hid_vga_ptr[128*i+j] = 0x8080|(i<<8);
    }
  for (i = 0; i < 1024; i++)
    {
      for (j = 0; j < ghlimit; j++)
        {
          hid_fb_ptr[i*ghlimit+j] = 0xF0000005ULL;
        }
    }
  for (i = 0; i < 1024; i+=32)
    {
      for (j = 0; j < ghlimit; j++)
        {
          hid_fb_ptr[i*ghlimit+j] = ~(0LL);
        }
    }
  for (i = 0; i < 1024; i++)
    {
      hid_fb_ptr[i*ghlimit+i/16] = 0xAULL << ((i&15)<<2);
    }
  for (i = 0; i < 256; i++)
    {
      hid_fb_ptr[i*ghlimit+i/16+16] = 0x5ULL << ((i&15)<<2);
    }
#endif  
  hid_reg_ptr[LOWRISC_REGS_HPIX] = width;
  hid_reg_ptr[LOWRISC_REGS_VPIX] = height;
  for (i = 0; i < 32; i++)
    {
      hid_vga_ptr[128*i+100] = i/10+'0'+0x8F00;
      hid_vga_ptr[128*i+101] = i%10+'0'+0x8F00;
    }
  for (i = 0; i < 32; i++)
    {
      for (j = 96; j < 112; j++)
        hid_vga_ptr[128*(i+14)+j] = 128|(i<<8);
      hid_vga_ptr[128*(i+14)+j] = i/10+'0'+0x8F00;
      hid_vga_ptr[128*(i+14)+j+1] = i%10+'0'+0x8F00;
    }
  draw_logo(ghlimit);
  for (;;)
    {
      int scan, ascii, event = *keyb_base;
      if (0x200 & ~event)
        {
          *keyb_base = 0; // pop FIFO
          event = *keyb_base & ~0x200;
          scan = scancode[event&~0x100].scan;
          ascii = scancode[event&~0x100].lwr;
          printf("Keyboard event = %X, scancode = %X, ascii = '%c'\n", event, scan, ascii);
          if (0x100 & ~event) switch(scan)
            {
            case 0x50: hid_reg_ptr[LOWRISC_REGS_VPIX] = ++height; printf(" %d,%d", height, width); break;
            case 0x48: hid_reg_ptr[LOWRISC_REGS_VPIX] = --height; printf(" %d,%d", height, width); break;
            case 0x4D: hid_reg_ptr[LOWRISC_REGS_HPIX] = ++width; printf(" %d,%d", height, width); break;
            case 0x4B: hid_reg_ptr[LOWRISC_REGS_HPIX] = --width; printf(" %d,%d", height, width); break;
            case 0xE0: break;
            case 0x39: for (i = 33; i < 47; i++) hid_reg_ptr[i] = rand32(); break;
            default: printf("?%x", scan); break;
            }
        }
    }
}
char dos[]={
/*-- ^@  --*/
0x00,0x00,0x20,0x60,0xfc,0xfc,0x60,0x20,0x00,0x00,0x00,0x00,
/*-- ^A  --*/
0x00,0x00,0x10,0x18,0xfc,0xfc,0x18,0x10,0x00,0x00,0x00,0x00,
/*-- ^B  --*/
0x00,0x30,0x78,0xfc,0x30,0x30,0x30,0x30,0x30,0x30,0x00,0x00,
/*-- ^C  --*/
0x00,0x30,0x30,0x30,0x30,0x30,0x30,0xfc,0x78,0x30,0x00,0x00,
/*-- ^D  --*/
0x00,0x78,0x84,0x82,0x82,0x82,0x82,0x82,0x84,0x78,0x00,0x00,
/*-- ^E  --*/
0x00,0x78,0x84,0xb2,0xfa,0xfa,0xfa,0xb2,0x84,0x78,0x00,0x00,
/*-- ^F  --*/
0x00,0xfe,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0xfe,0x00,0x00,
/*-- ^G  --*/
0x00,0xfe,0x86,0xce,0xfa,0xb2,0xfa,0xce,0x86,0xfe,0x00,0x00,
/*-- ^H  --*/
0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xf8,0xf0,0xe0,0xc0,0x80,0x00,
/*-- ^I  --*/
0x00,0x00,0x00,0x70,0xf8,0xf8,0xf8,0x70,0x00,0x00,0x00,0x00,
/*-- ^J  --*/
0x00,0x00,0x00,0x00,0x00,0x06,0x1e,0x18,0x30,0x30,0x30,0x30,
/*-- ^K  --*/
0x00,0x00,0x00,0x00,0x00,0x80,0xe0,0x60,0x30,0x30,0x30,0x30,
/*-- ^L  --*/
0x30,0x30,0x30,0x30,0x18,0x1e,0x06,0x00,0x00,0x00,0x00,0x00,
/*-- ^M  --*/
0x30,0x30,0x30,0x30,0x60,0xe0,0x80,0x00,0x00,0x00,0x00,0x00,
/*-- ^N  --*/
0x00,0x00,0x00,0x00,0x00,0xfe,0xfe,0x00,0x00,0x00,0x00,0x00,
/*-- ^O  --*/
0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
/*-- ^P  --*/
0x00,0x00,0x00,0x00,0x00,0xfe,0xfe,0x30,0x30,0x30,0x30,0x30,
/*-- ^Q  --*/
0x30,0x30,0x30,0x30,0x30,0xfe,0xfe,0x00,0x00,0x00,0x00,0x00,
/*-- ^R  --*/
0x30,0x30,0x30,0x30,0x30,0x3e,0x3e,0x30,0x30,0x30,0x30,0x30,
/*-- ^S  --*/
0x30,0x30,0x30,0x30,0x30,0xf0,0xf0,0x30,0x30,0x30,0x30,0x30,
/*-- ^T  --*/
0x30,0x30,0x30,0x30,0x30,0xfe,0xfe,0x30,0x30,0x30,0x30,0x30,
/*-- ^U  --*/
0x00,0xfe,0xfe,0x00,0x00,0xfe,0xfe,0x00,0x00,0xfe,0xfe,0x00,
/*-- ^V  --*/
0xcc,0x98,0xb2,0x66,0xcc,0x98,0xb2,0x66,0xcc,0x98,0xb2,0x66,
/*-- ^W  --*/
0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,
/*-- ^X  --*/
0xd4,0xaa,0xd4,0xaa,0xd4,0xaa,0xd4,0xaa,0xd4,0xaa,0xd4,0xaa,
/*-- ^Y  --*/
0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,
/*-- ^Z  --*/
0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0x1e,0x1e,0x1e,0x1e,0x1e,0x1e,
/*-- ^[  --*/
0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
/*-- ^\  --*/
0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,
/*-- ^]  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,
/*-- ^^  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x1e,0x1e,0x1e,0x1e,0x1e,0x1e,
/*-- ^_  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
/*--    --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*-- !  --*/
0x00,0x30,0x78,0x78,0x78,0x30,0x30,0x00,0x30,0x30,0x00,0x00,
/*-- "  --*/
0x00,0xcc,0xcc,0xcc,0xcc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*-- #  --*/
0x00,0xd8,0xd8,0xfc,0xd8,0xd8,0xd8,0xfc,0xd8,0xd8,0x00,0x00,
/*-- $  --*/
0x30,0x30,0x7c,0xcc,0xc0,0x78,0x0c,0xcc,0xf8,0x30,0x30,0x00,
/*-- %  --*/
0x00,0x00,0x00,0xc4,0xcc,0x18,0x30,0x60,0xcc,0x8c,0x00,0x00,
/*-- &  --*/
0x00,0xe0,0xb0,0xb0,0xe0,0xb4,0xbc,0x98,0xb8,0xec,0x00,0x00,
/*-- '  --*/
0x00,0x30,0x30,0x70,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*-- (  --*/
0x00,0x18,0x30,0x60,0xc0,0xc0,0xc0,0x60,0x30,0x18,0x00,0x00,
/*-- )  --*/
0x00,0xc0,0x60,0x30,0x18,0x18,0x18,0x30,0x60,0xc0,0x00,0x00,
/*-- *  --*/
0x00,0x00,0x00,0xcc,0x78,0xfe,0x78,0xcc,0x00,0x00,0x00,0x00,
/*-- +  --*/
0x00,0x00,0x00,0x30,0x30,0xfc,0x30,0x30,0x00,0x00,0x00,0x00,
/*-- ,  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x70,0x30,0x60,
/*-- -  --*/
0x00,0x00,0x00,0x00,0x00,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,
/*-- .  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x70,0x00,0x00,
/*-- /  --*/
0x00,0x00,0x04,0x0c,0x18,0x30,0x60,0xc0,0x80,0x80,0x00,0x00,
/*-- 0  --*/
0x00,0xf8,0x8c,0x9c,0xbc,0xac,0xec,0xcc,0x8c,0xf8,0x00,0x00,
/*-- 1  --*/
0x00,0x10,0x30,0xf0,0x30,0x30,0x30,0x30,0x30,0xfc,0x00,0x00,
/*-- 2  --*/
0x00,0x78,0xcc,0xcc,0x0c,0x18,0x30,0x60,0xcc,0xfc,0x00,0x00,
/*-- 3  --*/
0x00,0x78,0xcc,0x0c,0x0c,0x38,0x0c,0x0c,0xcc,0x78,0x00,0x00,
/*-- 4  --*/
0x00,0x18,0x38,0x78,0xd8,0x98,0xfc,0x18,0x18,0x3c,0x00,0x00,
/*-- 5  --*/
0x00,0xfc,0xc0,0xc0,0xc0,0xf8,0x0c,0x0c,0xcc,0x78,0x00,0x00,
/*-- 6  --*/
0x00,0x38,0x60,0xc0,0xc0,0xf8,0xcc,0xcc,0xcc,0x78,0x00,0x00,
/*-- 7  --*/
0x00,0xfc,0x8c,0x8c,0x0c,0x18,0x30,0x60,0x60,0x60,0x00,0x00,
/*-- 8  --*/
0x00,0x78,0xcc,0xcc,0xec,0x78,0xdc,0xcc,0xcc,0x78,0x00,0x00,
/*-- 9  --*/
0x00,0x78,0xcc,0xcc,0xcc,0x7c,0x0c,0x0c,0x18,0x70,0x00,0x00,
/*-- :  --*/
0x00,0x00,0x00,0x70,0x70,0x00,0x00,0x70,0x70,0x00,0x00,0x00,
/*-- ;  --*/
0x00,0x00,0x00,0x00,0x70,0x70,0x00,0x00,0x70,0x70,0x30,0x60,
/*-- <  --*/
0x00,0x0c,0x18,0x30,0x60,0xc0,0x60,0x30,0x18,0x0c,0x00,0x00,
/*-- =  --*/
0x00,0x00,0x00,0x00,0xfc,0x00,0xfc,0x00,0x00,0x00,0x00,0x00,
/*-- >  --*/
0x00,0xc0,0x60,0x30,0x18,0x0c,0x18,0x30,0x60,0xc0,0x00,0x00,
/*-- ?  --*/
0x00,0x78,0xcc,0x0c,0x18,0x30,0x30,0x00,0x30,0x30,0x00,0x00,
/*-- @  --*/
0x00,0xf8,0xcc,0x8c,0xbc,0xbc,0xb8,0x80,0xc0,0xf8,0x00,0x00,
/*-- A  --*/
0x00,0x78,0xcc,0xcc,0xcc,0xfc,0xcc,0xcc,0xcc,0xcc,0x00,0x00,
/*-- B  --*/
0x00,0xf8,0xcc,0xcc,0xcc,0xf8,0xcc,0xcc,0xcc,0xf8,0x00,0x00,
/*-- C  --*/
0x00,0x78,0xcc,0x8c,0x80,0x80,0x80,0x8c,0xcc,0x78,0x00,0x00,
/*-- D  --*/
0x00,0xf0,0xd8,0xcc,0xcc,0xcc,0xcc,0xcc,0xd8,0xf0,0x00,0x00,
/*-- E  --*/
0x00,0xfc,0xc4,0xc0,0xc8,0xf8,0xc8,0xc0,0xc4,0xfc,0x00,0x00,
/*-- F  --*/
0x00,0xfc,0xc4,0xc0,0xc8,0xf8,0xc8,0xc0,0xc0,0xe0,0x00,0x00,
/*-- G  --*/
0x00,0x78,0xcc,0x8c,0x80,0x80,0x9c,0x8c,0xcc,0x7c,0x00,0x00,
/*-- H  --*/
0x00,0x8c,0x8c,0x8c,0x8c,0xfc,0x8c,0x8c,0x8c,0x8c,0x00,0x00,
/*-- I  --*/
0x00,0x78,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x78,0x00,0x00,
/*-- J  --*/
0x00,0x3c,0x18,0x18,0x18,0x18,0x98,0x98,0x98,0xf0,0x00,0x00,
/*-- K  --*/
0x00,0xcc,0xcc,0xd8,0xd8,0xf0,0xd8,0xd8,0xcc,0xcc,0x00,0x00,
/*-- L  --*/
0x00,0xe0,0xc0,0xc0,0xc0,0xc0,0xc4,0xcc,0xcc,0xfc,0x00,0x00,
/*-- M  --*/
0x00,0x8c,0xdc,0xfc,0xfc,0xac,0x8c,0x8c,0x8c,0x8c,0x00,0x00,
/*-- N  --*/
0x00,0x8c,0x8c,0xcc,0xec,0xfc,0xbc,0x9c,0x8c,0x8c,0x00,0x00,
/*-- O  --*/
0x00,0x70,0xd8,0x8c,0x8c,0x8c,0x8c,0x8c,0xd8,0x70,0x00,0x00,
/*-- P  --*/
0x00,0xf8,0xcc,0xcc,0xcc,0xf8,0xc0,0xc0,0xc0,0xe0,0x00,0x00,
/*-- Q  --*/
0x00,0x70,0xd8,0x8c,0x8c,0x8c,0x8c,0xbc,0xd8,0x78,0x0c,0x00,
/*-- R  --*/
0x00,0xf8,0xcc,0xcc,0xcc,0xf8,0xd8,0xcc,0xcc,0xcc,0x00,0x00,
/*-- S  --*/
0x00,0x78,0xcc,0xcc,0xc0,0x70,0x18,0xcc,0xcc,0x78,0x00,0x00,
/*-- T  --*/
0x00,0xfc,0xb4,0x30,0x30,0x30,0x30,0x30,0x30,0x78,0x00,0x00,
/*-- U  --*/
0x00,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x78,0x00,0x00,
/*-- V  --*/
0x00,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x78,0x30,0x00,0x00,
/*-- W  --*/
0x00,0x8c,0x8c,0x8c,0x8c,0xac,0xac,0xd8,0xd8,0xd8,0x00,0x00,
/*-- X  --*/
0x00,0xcc,0xcc,0xcc,0x78,0x30,0x78,0xcc,0xcc,0xcc,0x00,0x00,
/*-- Y  --*/
0x00,0xcc,0xcc,0xcc,0xcc,0x78,0x30,0x30,0x30,0x78,0x00,0x00,
/*-- Z  --*/
0x00,0xfc,0x8c,0x98,0x30,0x60,0xc0,0x84,0x8c,0xfc,0x00,0x00,
/*-- [  --*/
0x00,0x78,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x78,0x00,0x00,
/*-- \  --*/
0x00,0x00,0x80,0x80,0xc0,0x60,0x30,0x18,0x0c,0x04,0x00,0x00,
/*-- ]  --*/
0x00,0x78,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x78,0x00,0x00,
/*-- ^  --*/
0x00,0x20,0x70,0xd8,0x8c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*-- _  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xfe,
/*-- `  --*/
0x00,0x00,0x00,0x60,0x70,0x38,0x18,0x00,0x00,0x00,0x00,0x00,
/*-- a  --*/
0x00,0x00,0x00,0x00,0xf0,0x18,0xf8,0x98,0x98,0xec,0x00,0x00,
/*-- b  --*/
0x00,0xc0,0xc0,0xc0,0xf8,0xcc,0xcc,0xcc,0xcc,0xb8,0x00,0x00,
/*-- c  --*/
0x00,0x00,0x00,0x00,0x78,0xcc,0xc0,0xc0,0xcc,0x78,0x00,0x00,
/*-- d  --*/
0x00,0x38,0x18,0x18,0xf8,0x98,0x98,0x98,0x98,0xec,0x00,0x00,
/*-- e  --*/
0x00,0x00,0x00,0x00,0x78,0xcc,0xfc,0xc0,0xcc,0x78,0x00,0x00,
/*-- f  --*/
0x00,0x38,0x6c,0x60,0x60,0xf8,0x60,0x60,0x60,0xf0,0x00,0x00,
/*-- g  --*/
0x00,0x00,0x00,0x00,0xec,0x98,0x98,0x98,0xf8,0x18,0x98,0xf0,
/*-- h  --*/
0x00,0xc0,0xc0,0xc0,0xd8,0xec,0xcc,0xcc,0xcc,0xcc,0x00,0x00,
/*-- i  --*/
0x00,0x30,0x30,0x00,0x70,0x30,0x30,0x30,0x30,0xfc,0x00,0x00,
/*-- j  --*/
0x00,0x0c,0x0c,0x00,0x1c,0x0c,0x0c,0x0c,0x0c,0xcc,0xcc,0x78,
/*-- k  --*/
0x00,0xc0,0xc0,0xc0,0xcc,0xd8,0xf0,0xd8,0xcc,0xcc,0x00,0x00,
/*-- l  --*/
0x00,0xf0,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0xfc,0x00,0x00,
/*-- m  --*/
0x00,0x00,0x00,0x00,0xf8,0xac,0xac,0xac,0xac,0xac,0x00,0x00,
/*-- n  --*/
0x00,0x00,0x00,0x00,0xf8,0xcc,0xcc,0xcc,0xcc,0xcc,0x00,0x00,
/*-- o  --*/
0x00,0x00,0x00,0x00,0x78,0xcc,0xcc,0xcc,0xcc,0x78,0x00,0x00,
/*-- p  --*/
0x00,0x00,0x00,0x00,0xb8,0xcc,0xcc,0xcc,0xcc,0xf8,0xc0,0xe0,
/*-- q  --*/
0x00,0x00,0x00,0x00,0xec,0x98,0x98,0x98,0x98,0xf8,0x18,0x3c,
/*-- r  --*/
0x00,0x00,0x00,0x00,0xd8,0xdc,0xec,0xc0,0xc0,0xe0,0x00,0x00,
/*-- s  --*/
0x00,0x00,0x00,0x00,0x78,0xcc,0x60,0x18,0xcc,0x78,0x00,0x00,
/*-- t  --*/
0x00,0x00,0x20,0x60,0xfc,0x60,0x60,0x60,0x6c,0x38,0x00,0x00,
/*-- u  --*/
0x00,0x00,0x00,0x00,0x98,0x98,0x98,0x98,0x98,0xec,0x00,0x00,
/*-- v  --*/
0x00,0x00,0x00,0x00,0xcc,0xcc,0xcc,0xcc,0x78,0x30,0x00,0x00,
/*-- w  --*/
0x00,0x00,0x00,0x00,0x8c,0x8c,0xac,0xac,0xd8,0xd8,0x00,0x00,
/*-- x  --*/
0x00,0x00,0x00,0x00,0x8c,0xd8,0x70,0x70,0xd8,0x8c,0x00,0x00,
/*-- y  --*/
0x00,0x00,0x00,0x00,0xcc,0xcc,0xcc,0xcc,0x7c,0x0c,0x18,0x70,
/*-- z  --*/
0x00,0x00,0x00,0x00,0xfc,0x8c,0x18,0x60,0xc4,0xfc,0x00,0x00,
/*-- {  --*/
0x00,0x1c,0x30,0x30,0x60,0xc0,0x60,0x30,0x30,0x1c,0x00,0x00,
/*-- |  --*/
0x00,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x00,0x00,
/*-- }  --*/
0x00,0xe0,0x30,0x30,0x18,0x0c,0x18,0x30,0x30,0xe0,0x00,0x00,
/*-- ~  --*/
0x00,0xe6,0xb6,0x9c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
