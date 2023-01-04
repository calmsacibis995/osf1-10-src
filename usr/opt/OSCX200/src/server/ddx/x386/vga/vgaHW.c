/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
 * $XConsortium: vgaHW.c,v 1.3 91/08/26 15:40:56 gildea Exp $
 *
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 */

#define _NEED_SYSI86

#include "compiler.h"

#include "x386OSD.h"
#include "vga.h"
#include "scrnintstr.h"

#define new ((vgaHWPtr)vgaNewVideoState)


/*
 * vgaSaveScreen -- 
 *      Disable the video on the frame buffer to save the screen.
 */
Bool
vgaSaveScreen (pScreen, on)
     ScreenPtr     pScreen;
     Bool          on;
{
  unsigned char   state;

  if (x386VTSema) {
    outb(0x3C4,1);
    state = inb(0x3C5);
  
    if (on) state &= 0xDF;
    else    state |= 0x20;
    
    /*
     * turn off srceen if necessary
     */
    outw(0x3C4, 0x0100);              /* syncronous reset */
    outw(0x3C4, (state << 8) | 0x01); /* change mode */
    outw(0x3C4, 0x0300);              /* syncronous reset */

  } else {
    if (on)
      ((vgaHWPtr)vgaNewVideoState)->Sequencer[1] &= 0xDF;
    else
      ((vgaHWPtr)vgaNewVideoState)->Sequencer[1] |= 0x20;
  }

  return(TRUE);
}



/*
 * vgaHWRestore --
 *      restore a video mode
 */

void
vgaHWRestore(restore)
     vgaHWPtr restore;
{
  int i;

  outw(0x3C4, ((restore->Sequencer[0] & 0xFD) << 8) | 0x00);

  if (vgaIOBase == 0x3B0)
    restore->MiscOutReg &= 0xFE;
  else
    restore->MiscOutReg |= 0x01;

  outb(0x3C2, restore->MiscOutReg);

  /*
   * This here is a workaround a bug in the kd-driver. We MUST explicitely
   * restore the font we got, when we entered graphics mode.
   * The bug was seen on ESIX, and ISC 2.0.2 when using a monochrome
   * monitor. 
   *
   * BTW, also GIO_FONT seems to have a bug, so we cannot use it, to get
   * a font.
   */
  
  if(restore->FontInfo) {
    /*
     * here we switch temporary to 16 color-plane-mode, to simply
     * copy the font-info
     *
     * BUGALLERT: The vga's segment-select register MUST be set appropriate !
     */
    i = inb(vgaIOBase + 0x0A); /* reset flip-flop */
    outb(0x3C0,0x30); outb(0x3C0, 0x01); /* graphics mode */
    outw(0x3C4, 0x0402);    /* write to plane 2 */
    outw(0x3C4, 0x0604);    /* enable plane graphics */
    outw(0x3CE, 0x0204);    /* read plane 2 */
    outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
    outw(0x3CE, 0x0506);    /* set graphics */
    bcopy(restore->FontInfo, vgaBase, 8192);

  }
  
  for (i=0; i<5;  i++) outw(0x3C4, (restore->Sequencer[i] << 8) | i);
  i = inb(vgaIOBase + 0x0A); /* reset flip-flop */
  for (i=0; i<16; i++) { outb(0x3C0,i); outb(0x3C0, restore->Attribute[i]); }
  for (i=16; i<21;i++) { outb(0x3C0,i | 0x20);
			 outb(0x3C0, restore->Attribute[i]); }
  for (i=0; i<24; i++) outw(vgaIOBase + 4,(restore->CRTC[i] << 8) | i);
  for (i=0; i<9;  i++) outw(0x3CE, (restore->Graphics[i] << 8) | i);
  
  outb(0x3C6,0xFF);
  outb(0x3C8,0x00);
  for (i=0; i<768; i++) outb(0x3C9, restore->DAC[i]);

}



/*
 * vgaHWSave --
 *      save the current video mode
 */

void *
vgaHWSave(save, size)
     vgaHWPtr save;
     int          size;
{
  int           i;

  if (save == NULL) {
    save = (vgaHWPtr)Xcalloc(size);
    /*
     * Here we are, when we first save the videostate. This means we came here
     * to save the original Text mode. Because some drivers may depend
     * on NoClock we set it here to a resonable value.
     */
    save->NoClock = (inb(0x3CC) >> 2) & 3;
  }

  /*
   * now get the fuck'in register
   */
  save->MiscOutReg = inb(0x3CC);

  vgaIOBase = (save->MiscOutReg & 0x01) ? 0x3D0 : 0x3B0;
  for (i=0; i<24; i++) { outb(vgaIOBase + 4,i);
			 save->CRTC[i] = inb(vgaIOBase + 5); }
  for (i=0; i<5;  i++) { outb(0x3C4,i); save->Sequencer[i]   = inb(0x3C5); }
  for (i=0; i<9;  i++) { outb(0x3CE,i); save->Graphics[i]  = inb(0x3CF); }
  i = inb(vgaIOBase + 0x0A); /* reset flip-flop */
  for (i=0; i<16; i++) { outb(0x3C0,i); save->Attribute[i] = inb(0x3C1);
			 outb(0x3C0, save->Attribute[i]); }
  for (i=16; i<21; i++) { outb(0x3C0,i | 0x20); 
			  save->Attribute[i] = inb(0x3C1);
			  outb(0x3C0, save->Attribute[i]); }
  
#ifdef GOOD_ET4000
  /* Some recent (1991) ET4000 chips have a HW bug that prevents the reading
     of the color lookup table.  Mask rev 9042EAI is known to have this bug.

     X386 already keeps track of the contents of the color lookup table so
     reading the HW isn't needed.  Therefore, as a workaround for this HW
     bug, the following (correct) code has been #ifdef'ed out.  This is also
     a valid change for ET4000 chips that don't have the HW bug.  The code
     is really just being retained for reference.  MWS 22-Aug-91
  */

  /*			 
   * save the colorlookuptable 
   */
  outb(0x3C6,0xFF);
  outb(0x3C7,0x00);
  for (i=0; i<768; i++) save->DAC[i] = inb(0x3C9); 
#endif /* GOOD_ET4000 */

  /*
   * get the character set of the first non-graphics application
   */
  if (((save->Attribute[0x10] & 0x01) == 0) &&
      (save->FontInfo == NULL)) {
    /*
     * Here we switch temporary to 16 color-plane-mode, to simply
     * copy the font-info
     *
     * BUGALLERT: The vga's segment-select register MUST be set appropriate !
     */
    i = inb(vgaIOBase + 0x0A); /* reset flip-flop */
    outb(0x3C0,0x30); outb(0x3C0, 0x01); /* graphics mode */
    outw(0x3C4, 0x0402);    /* write to plane 2 */
    outw(0x3C4, 0x0604);    /* enable plane graphics */
    outw(0x3CE, 0x0204);    /* read plane 2 */
    outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
    outw(0x3CE, 0x0506);    /* set graphics */

    save->FontInfo = (pointer)xalloc(8192);
    bcopy(vgaBase, save->FontInfo, 8192);

  }
  
  return ((void *) save);
}



/*
 * vgaHWInit --
 *      Handle the initialization, etc. of a screen.
 */

void
vgaHWInit(mode,size)
     int             size;
     DisplayModePtr      mode;
{
  int                i;

  if (vgaNewVideoState == NULL) {
    vgaNewVideoState = (void *)Xcalloc(size);

    /*
     * initialize default colormap for monochrome
     */
    for (i=0; i<3;   i++) new->DAC[i] = 0x00;
    for (i=3; i<768; i++) new->DAC[i] = 0x3F;

  }

  /*
   * Get NoClock
   */
  new->NoClock   = mode->Clock;

  /*
   * compute correct Hsync & Vsync polarity 
   */
  if ((mode->Flags & (V_PHSYNC | V_NHSYNC))
      && (mode->Flags & (V_PVSYNC | V_NVSYNC)))
      {
	new->MiscOutReg = 0x23;
	if (mode->Flags & V_NHSYNC) new->MiscOutReg |= 0x40;
	if (mode->Flags & V_NVSYNC) new->MiscOutReg |= 0x80;
      }
      else
      {
	if      (mode->VDisplay < 400) new->MiscOutReg = 0xA3;
	else if (mode->VDisplay < 480) new->MiscOutReg = 0x63;
	else if (mode->VDisplay < 768) new->MiscOutReg = 0xE3;
	else                           new->MiscOutReg = 0x23;
      }
  new->MiscOutReg |= (new->NoClock & 0x03) << 2;
  
  /*
   * Time Sequencer
   */
  new->Sequencer[0] = 0x00;
  new->Sequencer[1] = 0x01;
  new->Sequencer[2] = 0x0F;
  new->Sequencer[3] = 0x00;                             /* Font select */
  new->Sequencer[4] = 0x0E;                             /* Misc */

  /*
   * CRTC Controller
   */
  new->CRTC[0]  = (mode->HTotal >> 3) - 5;
  new->CRTC[1]  = (mode->HDisplay >> 3) - 1;
  new->CRTC[2]  = (mode->HSyncStart >> 3) -1;
  new->CRTC[3]  = ((mode->HSyncEnd >> 3) & 0x1F) | 0x80;
  new->CRTC[4]  = (mode->HSyncStart >> 3);
  new->CRTC[5]  = (((mode->HSyncEnd >> 3) & 0x20 ) << 2 )
    | (((mode->HSyncEnd >> 3)) & 0x1F);
  new->CRTC[6]  = (mode->VTotal - 2) & 0xFF;
  new->CRTC[7]  = (((mode->VTotal -2) & 0x100) >> 8 )
    | (((mode->VDisplay -1) & 0x100) >> 7 )
      | ((mode->VSyncStart & 0x100) >> 6 )
	| (((mode->VSyncStart) & 0x100) >> 5 )
	  | 0x10
	    | (((mode->VTotal -2) & 0x200)   >> 4 )
	      | (((mode->VDisplay -1) & 0x200) >> 3 )
		| ((mode->VSyncStart & 0x200) >> 2 );
  new->CRTC[8]  = 0x00;
  new->CRTC[9]  = ((mode->VSyncStart & 0x200) >>4) | 0x40;
  new->CRTC[10] = 0x00;
  new->CRTC[11] = 0x00;
  new->CRTC[12] = 0x00;
  new->CRTC[13] = 0x00;
  new->CRTC[14] = 0x00;
  new->CRTC[15] = 0x00;
  new->CRTC[16] = mode->VSyncStart & 0xFF;
  new->CRTC[17] = (mode->VSyncEnd & 0x0F) | 0x20;
  new->CRTC[18] = (mode->VDisplay -1) & 0xFF;
  new->CRTC[19] = vga256InfoRec.virtualX >> 4;  /* just a guess */
  new->CRTC[20] = 0x00;
  new->CRTC[21] = mode->VSyncStart & 0xFF; 
  new->CRTC[22] = (mode->VSyncStart +1) & 0xFF;
  new->CRTC[23] = 0xC3;
  new->CRTC[24] = 0xFF;

  /*
   * Graphics Display Controller
   */
  new->Graphics[0] = 0x00;
  new->Graphics[1] = 0x00;
  new->Graphics[2] = 0x00;
  new->Graphics[3] = 0x00;
  new->Graphics[4] = 0x00;
  new->Graphics[5] = 0x40;
  new->Graphics[6] = 0x05;   /* only map 64k VGA memory !!!! */
  new->Graphics[7] = 0x0F;
  new->Graphics[8] = 0xFF;
  
  new->Attribute[0]  = 0x00; /* standart colormap translation */
  new->Attribute[1]  = 0x01;
  new->Attribute[2]  = 0x02;
  new->Attribute[3]  = 0x03;
  new->Attribute[4]  = 0x04;
  new->Attribute[5]  = 0x05;
  new->Attribute[6]  = 0x06;
  new->Attribute[7]  = 0x07;
  new->Attribute[8]  = 0x08;
  new->Attribute[9]  = 0x09;
  new->Attribute[10] = 0x0A;
  new->Attribute[11] = 0x0B;
  new->Attribute[12] = 0x0C;
  new->Attribute[13] = 0x0D;
  new->Attribute[14] = 0x0E;
  new->Attribute[15] = 0x0F;
  new->Attribute[16] = 0x41; /* wrong for the ET4000 */
  new->Attribute[17] = 0x01;
  new->Attribute[18] = 0x0F;
  new->Attribute[19] = 0x00;
  new->Attribute[20] = 0x00;
}


/*
 * vgaGetClocks --
 *      get the dot-clocks via a BIG BAD hack ...
 */

void
vgaGetClocks(num, ClockFunc)
     int num;
     void (*ClockFunc)();
{
  int          norm;
  register int status = vgaIOBase + 0x0A;
  unsigned long       i, j, cnt, rcnt, sync;

  sysi86(SI86V86, V86SC_IOPL, PS_IOPL);

  for (i = 1; i < num; i++) {
    
    (*ClockFunc)(i);

    cnt  = 0;
    sync = 200000;

    intr_disable();
    while ((inb(status) & 0x08) == 0x00) if (sync-- == 0) goto finish;
    while ((inb(status) & 0x08) == 0x08) if (sync-- == 0) goto finish;
    while ((inb(status) & 0x08) == 0x00) if (sync-- == 0) goto finish;
    
    for (rcnt = 0; rcnt < 5; rcnt++) {
      while (!(inb(status) & 0x08)) cnt++;
      while ((inb(status) & 0x08)) cnt++;
    }
    
  finish:
    intr_enable();

    vga256InfoRec.clock[i] = cnt ? cnt : 1000000;
  }

  for (i = 2; i < num; i++)
    vga256InfoRec.clock[i] = (int)(0.5 +
      (28.322 * vga256InfoRec.clock[1]) / (vga256InfoRec.clock[i]));

  vga256InfoRec.clock[0] = 25;
  vga256InfoRec.clock[1] = 28;

  for (i=0; i < num; i++)
    for (j=i+1; j < num; j++)
      if (vga256InfoRec.clock[i] == vga256InfoRec.clock[j]) 
	vga256InfoRec.clock[j] = 0;

  vga256InfoRec.clocks = num;
  (ClockFunc)(0);
}


