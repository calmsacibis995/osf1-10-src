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
#include <stdio.h>

#include "types.h"
#include "defs.h"
#include "sfbparams.h"

#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   1024

extern FILE *vramFile;
extern int logFlag;

BYTES *vram;

/*
 * read or write the VRAM's.
 *
 * always returns 8 BYTES of data from the addressed (8-PIXEL) word.
 */

#if SFBPIXELBITS == 32 && defined(SLEAZOID32)
ULONG64
do_rams(addr, data, planemask, pixelmask, we)
    unsigned addr;		/* 8-PIXEL ``word'' address		*/
    ULONG64 data;		/* 8-BYTE data to write (if any)	*/
    unsigned planemask;		/* planemask				*/
    unsigned pixelmask;		/* 8 bits of pixel enable		*/
    unsigned we;		/* write enable	(0 means write)		*/

{
  int i, pixel;
  BYTES d;

  planemask = CompressPixel(planemask);
  addr %= (SCREEN_WIDTH*SCREEN_HEIGHT/8);
  if (we == 0) {   /** asserted low, so write it **/
    if (logFlag)
      fprintf (vramFile, "%x %x %x %x %x\n", 
	addr, data.lo, data.hi, planemask, pixelmask);
    d.l = data;
    for (i=0; pixelmask != 0; i++, pixelmask >>= 1) {
      if (pixelmask & 1) {
	pixel = d.p32[i & 1];
	pixel = CompressPixel(pixel);
	vram[addr].b[i] = (vram[addr].b[i] & ~planemask) | (pixel & planemask);
      }
    }
  }

  for (i=0; pixelmask != 0; i++, pixelmask >>= 1) {
    if (pixelmask & 1) {
      pixel = vram[addr].b[i];
      pixel = ExpandPixel(pixel);
      d.p32[i & 1] = pixel;
    }
  }
  return (d.l);
}
 
#else

ULONG64
do_rams(addr, data, planemask, pixelmask, we)
    unsigned addr;		/* 8-PIXEL ``word'' address		*/
    ULONG64 data;		/* 8-BYTE data to write (if any)	*/
    unsigned planemask;		/* planemask				*/
    unsigned pixelmask;		/* 8 bits of pixel enable		*/
    unsigned we;		/* write enable	(0 means write)		*/

{
  int i, pixel;
  BYTES d;

  addr %= (SCREEN_WIDTH*SCREEN_HEIGHT/8);
  if (we == 0) {   /** asserted low, so write it **/
    if (logFlag)
      fprintf (vramFile, "%x %x %x %x %x\n", 
	addr, data.lo, data.hi, planemask, pixelmask);
    d.l = data;
    for (i=0; pixelmask != 0; i++, pixelmask >>= 1) {
      pixel = d.b[i];
      if (pixelmask & 1) {
	vram[addr].b[i] = (vram[addr].b[i] & ~planemask) | (pixel & planemask);
      }
    }
  }

  return (vram[addr].l);
}
#endif
