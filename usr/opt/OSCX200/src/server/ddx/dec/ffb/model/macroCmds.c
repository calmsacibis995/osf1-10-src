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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: macroCmds.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:41:03 $";
#endif
/*
 * HISTORY
 * $Log: macroCmds.c,v $
 * Revision 1.1.2.2  1993/11/19  21:41:03  Robert_Lembree
 * 	SFB+ Initial version
 * 	[1993/11/19  16:33:41  Robert_Lembree]
 *
 * Revision 1.1.1.2  1993/11/19  16:33:41  Robert_Lembree
 * 	SFB+ Initial version
 *
 * Revision 2.9  1993/05/13  20:32:43  rsm
 * subtrace AddrBase() from address when writing copy64 registers
 *
 * Revision 2.8  1993/04/26  05:27:42  chris
 * Remove references to DmaData call
 *
 * Revision 2.7  1993/04/16  01:59:05  chris
 * depth/rotate visual changes
 *
 */
#include <stdio.h>

#include "defs.h"
#include "types.h"
#include "vars.h"
#include "vram.h"

extern VramPointType pointBuf[MAXPOINTS];
extern int    npoints;
extern SFBREG sfbreg;

#define ADDRMASK 0x1fffff

DumpRegisters()
{
  COLORS plmsk;
#ifdef HARDWARE
  printf("can't dump registers in hardware model\n");
#else
  printf("\nbackground\t= %-.2x %-.2x %-.2x %-.2x\n", sfbreg.background.byte[0],
	 sfbreg.background.byte[1], sfbreg.background.byte[2], sfbreg.background.byte[3]);
  printf("foreground\t= %-.2x %-.2x %-.2x %-.2x\n", sfbreg.foreground.byte[0],
	 sfbreg.foreground.byte[1], sfbreg.foreground.byte[2], sfbreg.foreground.byte[3]);
  plmsk.data = sfbreg.planeMask;
  printf("planeMask\t= %-.2x %-.2x %-.2x %-.2x\n", plmsk.byte[0],
	 plmsk.byte[1], plmsk.byte[2], plmsk.byte[3]);
  printf("rasterOp\t= %x\n", sfbreg.rop);
  printf("mode\t\t= %x\n", sfbreg.oldMode);
  printf("deep\t\t= %x\n", sfbreg.depth);
/*  printf("readFlag\t= %x\n\n", readFlag); */
  printf("pixShift\t= %x\n", sfbreg.shift);
  printf("pixMask\t\t= %x\n", sfbreg.pixelMask);
  printf("bres1.a1\t= %-.4x\n", sfbreg.bres1.reg.a1);
  printf("bres1.e1\t= %-.4x\n", sfbreg.bres1.reg.e1);
  printf("bres2.a2\t= %-.4x\n", sfbreg.bres2.reg.a2);
  printf("bres2.e2\t= %-.4x\n", sfbreg.bres2.reg.e2); 
  printf("bres3.e\t\t= %-.5x\n", sfbreg.bres3.reg.e); 
  printf("bres3.count\t= %x\n\n", sfbreg.bres3.reg.lineLength); 
#endif
}

extern ROPREG    shadowRop;
extern MODEREG   shadowMode;
extern BWIDTHREG shadowWidth;
extern DEEPREG   shadowDeep;


AddrBase()
{
  switch (shadowDeep.reg.deep&1) {
  case DEPTH8:	return (0x200000);
  case DEPTH32:	return (0x800000);
  }

  fprintf (stderr, "AddrBase: unknown depth (%d)\n", shadowDeep.reg.deep&1);
  exit (1);

  return (0);
}

SFBPIXELBYTES()
{
  switch (shadowDeep.reg.deep&1) {
  case DEPTH8:	return (1);
  case DEPTH32:	return (4);
  }

  fprintf (stderr, "SFBPIXELBYTES: unknown depth (%d)\n", shadowDeep.reg.deep&1);
  exit (1);

  return (0);
}

PixelsPerOp()
{
  switch (shadowDeep.reg.deep&1) {
  case DEPTH8:	return (32);
  case DEPTH32:	return (8);
  }

  fprintf (stderr, "PixelsPerOp: unknown depth (%d)\n", shadowDeep.reg.deep&1);
  exit (1);

  return (0);
}

SFBCOPYBITS()
{
  switch (shadowDeep.reg.deep&1) {
  case DEPTH8:	return (32);
  case DEPTH32:	return (16);
  }

  fprintf (stderr, "SFBCOPYBITS: unknown depth (%d)\n", shadowDeep.reg.deep&1);
  exit (1);

  return (0);
}

SFBCOPYBYTESDONE()
{
  return (SFBCOPYBITS() * SFBPIXELBYTES());
}


SFBALIGNMASK()
{
  switch (shadowDeep.reg.deep&1) {
  case DEPTH8:	return (7);
  case DEPTH32:	return (1);
  }

  fprintf (stderr, "SFBALIGNMASK: unknown depth (%d)\n", shadowDeep.reg.deep&1);
  exit (1);

  return (0);
}

SFBALIGNMENT()
{
  switch (shadowDeep.reg.deep&1) {
  case DEPTH8:	return (8);
  case DEPTH32:	return (2);
  }

  fprintf (stderr, "SFBALIGNMENT: unknown depth (%d)\n", shadowDeep.reg.deep&1);
  exit (1);

  return (0);
}

OpMask()
{
  switch (shadowDeep.reg.deep&1) {
  case DEPTH8:	return (0xff);
  case DEPTH32:	return (0x03);
  }

  fprintf (stderr, "OpMask: unknown depth (%d)\n", shadowDeep.reg.deep&1);
  exit (1);

  return (0);
}

CopyMask()
{
  switch (shadowDeep.reg.deep&1) {
  case DEPTH8:	return (0xffffffff);
  case DEPTH32:	return (0x000000ff);
  }

  fprintf (stderr, "CopyMask: unknown depth (%d)\n", shadowDeep.reg.deep&1);
  exit (1);

  return (0);
}

OpsPerCmd()
{
  switch (shadowDeep.reg.deep&1) {
  case DEPTH8:	return (4);
  case DEPTH32:	return (4);
  }

  fprintf (stderr, "OpPerCmd: unknown depth (%d)\n", shadowDeep.reg.deep&1);
  exit (1);

  return (0);
}


Fill(x0, y0, x1, y1, pattern)
unsigned pattern;
{
  unsigned startAddr, mask;
  int llx, lly, ury, urx, x, y, data;

  llx = (x0 < x1 ? x0 : x1);
  lly = (y0 < y1 ? y0 : y1);
  ury = (y0 > y1 ? y0 : y1);
  urx = (x0 > x1 ? x0 : x1);

  shadowMode.reg.mode = OPAQUESTIPPLE | (shadowMode.reg.mode & TRANSPMASK);
  BusWrite (MODE_ADDRESS, shadowMode.u32, LWMASK);

  for (y=lly; y<=ury; ++y) {
    for (x=(llx & ~SFBALIGNMASK()); x<((urx+(1+SFBALIGNMASK())) & ~SFBALIGNMASK()); x += PixelsPerOp()) {
      startAddr = AddrBase() + (x + y*RAM_WIDTH) * SFBPIXELBYTES();

      data = pattern << (llx & SFBALIGNMASK());
      data |= pattern >> (PixelsPerOp() - (llx & SFBALIGNMASK()));

      mask = MakeFillMask(x, llx, urx);
      if ((shadowMode.reg.mode & TRANSPMASK) == 0)
	BusWrite (PIXMSK_ADDRESS, mask, LWMASK);
#if 0
      BusWrite (startAddr, data&mask, LWMASK);
#else
      BusWrite (ADDRREG_ADDRESS, startAddr & ADDRMASK, LWMASK);
      BusWrite (START_ADDRESS, data&mask, LWMASK);
#endif
    }
  }
}

#if 0
BlockFill(x0, y0, x1, y1, pattern)
unsigned pattern;
{
  unsigned startAddr, data, mask;
  int llx, lly, ury, urx, x, y;

  llx = (x0 < x1 ? x0 : x1);
  lly = (y0 < y1 ? y0 : y1);
  ury = (y0 > y1 ? y0 : y1);
  urx = (x0 > x1 ? x0 : x1);

  shadowMode.reg.mode = TRANSPBLKSTIPPL;
  BusWrite (MODE_ADDRESS, shadowMode.u32, LWMASK);

  for (y=lly; y<=ury; ++y) {
    for (x=(llx & ~SFBALIGNMASK()); x<((urx+(1+SFBALIGNMASK())) & ~SFBALIGNMASK()); x += 32) {
      startAddr = AddrBase() + (x + y*RAM_WIDTH) * SFBPIXELBYTES();

      data = pattern << (llx & SFBALIGNMASK());
      data |= pattern >> (PixelsPerOp() - (llx & SFBALIGNMASK()));

      mask = MakeFillMask(x, llx, urx);

      data &= mask;

      {
	unsigned d0, d1, shift;

	shift = (startAddr & 0x1c);

	if (shift != 0) {
	  d0 = data << shift;
	  d1 = data >> (32 - shift);
	  data = d0 | d1;
	}
      }
#if 1
      BusWrite (startAddr, data, LWMASK);
#else
      BusWrite (ADDRREG_ADDRESS, startAddr & ADDRMASK, LWMASK);
      BusWrite (START_ADDRESS, data, LWMASK);
#endif
    }
  }
}
#else
BlockFill(x0, y0, x1, y1, pattern)
unsigned pattern;
{
  unsigned startAddr, data;
  int llx, lly, ury, urx, y;

  llx = (x0 < x1 ? x0 : x1);
  lly = (y0 < y1 ? y0 : y1);
  ury = (y0 > y1 ? y0 : y1);
  urx = (x0 > x1 ? x0 : x1);

  shadowMode.reg.mode = BLOCKFILL;
  BusWrite (MODE_ADDRESS, shadowMode.u32, LWMASK);

  for (y=lly; y<=ury; ++y) {
    startAddr = AddrBase() + (llx + y*RAM_WIDTH) * SFBPIXELBYTES();

    data = (urx - llx) & 0x7ff;
    data |= (startAddr & 3) << 16;

#if 1
    BusWrite (startAddr & ~3, data, LWMASK);
#else
    BusWrite (ADDRREG_ADDRESS, startAddr & ADDRMASK, LWMASK);
    BusWrite (START_ADDRESS, data, LWMASK);
#endif
  }
}
#endif

MakeFillMask(x, llx, urx)
{
  int dx, mask, i, unmask;

  dx = urx - x + 1;
  if (dx > PixelsPerOp()) dx = PixelsPerOp();

  mask = 0;
  for (i=0; i<dx; ++i)
    mask = (mask << 1) + 1;

  unmask = 0;
  for (i=0; i<llx-x; ++i)
    unmask = (unmask << 1) + 1;

  return (mask & ~unmask);
}

int lastx = -1;
int lasty = -1;

PrintBres(xa, ya, xb, yb)
{
  int dx, dy, len;
  int e1, e2, e, a1, a2;

  dx = xb - xa;
  dy = yb - ya;

  if (iabs(dy) > iabs(dx)) {
    e1 = 2*iabs(dx);
    e2 = 2*iabs(dx) - 2*iabs(dy);
    e = 2*iabs(dx) - iabs(dy);

    a1 = (dy>0 ? RAM_WIDTH : -RAM_WIDTH);
    a2 = a1 + (dx>0 ? 1 : -1);
    len = iabs(dy) + 1;
  } else {
    e1 = 2*iabs(dy);
    e2 = 2*iabs(dy) - 2*iabs(dx);
    e = 2*iabs(dy) - iabs(dx);

    a1 = (dx>0 ? 1 : -1);
    a2 = a1 + (dy>0 ? RAM_WIDTH : -RAM_WIDTH);
    len = iabs(dx) + 1;
  }

  a1 *= SFBPIXELBYTES();
  a2 *= SFBPIXELBYTES();

  fprintf(stderr, ">>> bres1: a1 = %-.8x, e1 = %-.8x\n", a1, (e1>>1) );
  fprintf(stderr, ">>> bres2: a2 = %-.8x, e2 = %-.8x\n", a2, ((-e2) >> 1));

  if ((len & 0xf) == 0) {
    len -= 16;
  }

  fprintf(stderr, ">>> bres3: e = %-.8x, len = %d\n", (e >> 1), len & 0xf);
}



OldLine(xa, ya, xb, yb, pattern)
unsigned pattern;
{
  int dx, dy, len, nextx, nexty;
  int e1, e2, e, a1, a2;
  unsigned addr;
  unsigned stipple;
  REGS u;

  dx = xb - xa;
  dy = yb - ya;

  if (iabs(dy) > iabs(dx)) {
    e1 = 2*iabs(dx);
    e2 = 2*iabs(dx) - 2*iabs(dy);
    e = 2*iabs(dx) - iabs(dy);

    a1 = (dy>0 ? RAM_WIDTH : -RAM_WIDTH);
    a2 = a1 + (dx>0 ? 1 : -1);
    len = iabs(dy) + 1;

    nextx = xb + (e < 0 ? 0 : (dx > 0 ? 1 : -1));
    nexty = yb + (dy > 0 ? 1 : -1);
  } else {
    e1 = 2*iabs(dy);
    e2 = 2*iabs(dy) - 2*iabs(dx);
    e = 2*iabs(dy) - iabs(dx);

    a1 = (dx>0 ? 1 : -1);
    a2 = a1 + (dy>0 ? RAM_WIDTH : -RAM_WIDTH);
    len = iabs(dx) + 1;

    nextx = xb + a1;
    nexty = yb + (e < 0 ? 0 : (dy > 0 ? 1 : -1));
  }

/*
  a1 *= SFBPIXELBYTES();
  a2 *= SFBPIXELBYTES();
*/

  MakeIdle();
  shadowMode.reg.mode = OPAQUELINE | (shadowMode.reg.mode & TRANSPMASK);
  BusWrite (MODE_ADDRESS, shadowMode.u32, LWMASK);

  u.bres1.e1 = (e1 >> 1);
  u.bres1.a1 = a1;
  LineWrite (BRES1_ADDRESS, u.data);

  u.bres2.e2 = ((-e2) >> 1);
  u.bres2.a2 = a2;
  LineWrite (BRES2_ADDRESS, u.data);

  if ((len & 0xf) == 0) {
    len -= 16;
    stipple = (pattern >> 16) | (pattern << 16);
  } else
    stipple = (pattern >> (len & 0xf)) | (pattern << (32 - (len & 0xf)));

  u.bres3.e = (e >> 1);
  u.bres3.lineLength = len & 0xf;
  LineWrite (BRES3_ADDRESS, u.data);

  len -= (len & 0xf);
  addr = AddrBase() + (xa + ya*RAM_WIDTH) * SFBPIXELBYTES();
  
  npoints = 0;
  if (lastx == xa && lasty == ya) {
    LineWrite (BCONT_ADDRESS, pattern & 0xffff);

/*    fprintf (stderr, "segment continuation (%d, %d) --> (%d, %d)\n",
	     xa, ya, xb, yb);*/

  } else {
#if 0
    u.data = 0;
    u.bstart.linedata = pattern & 0xffff;
    u.bstart.addrLo = addr & 3;
    LineWrite (addr & ~3, u.data);
#else
    LineWrite (ADDRREG_ADDRESS, addr & ADDRMASK);

    u.data = 0;
    u.bstart.linedata = pattern & 0xffff;
/*    LineWrite (START_ADDRESS, u.data);*/
    LineWrite (BCONT_ADDRESS, u.data);
#endif
  }

  lastx = nextx;
  lasty = nexty;
/*  fprintf (stderr, "(%d, %d) --> (%d, %d) [%d, %d]\n", xa, ya, xb, yb, nextx, nexty);*/
  
  while (len > 0) {
    LineWrite (BCONT_ADDRESS, stipple & 0xffff);
    len -= 16;
    stipple = (stipple >> 16) | (stipple << 16);
  }
  MakeIdle();

  {
    int dx = xa - xb;
    int dy = ya - yb;
    int len;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    if (dx > dy) len = dx + 1;
    else len = dy + 1;

    if (npoints != len) {
      fprintf (stderr, "****request (%d, %d) -> (%d, %d)\n", xa, ya, xb, yb);
      fprintf (stderr, "****actual  wrote %d pixels, expected %d pixels\n",
	       npoints, len);
    }
    if (npoints != 0 && (pointBuf[0].x != xa
	|| pointBuf[0].y != ya
	|| pointBuf[npoints-1].x != xb
	|| pointBuf[npoints-1].y != yb)) {
      int XYCompare();
      int lx, ly, rx, ry;
      
      if (xa > xb) {
	lx = xb; ly = yb; rx = xa; ry = ya;
      } else {
	lx = xa; ly = ya; rx = xb; ry = yb;
      }

      qsort(pointBuf, npoints, sizeof(VramPointType), XYCompare);

      if (pointBuf[0].x != lx
	  || pointBuf[0].y != ly
	  || pointBuf[npoints-1].x != rx
	  || pointBuf[npoints-1].y != ry) {
	if (npoints == len) {
	  fprintf (stderr, "****request (%d, %d) -> (%d, %d)\n", xa, ya, xb, yb);    
	  fprintf (stderr, "****actual  ");
	} else
	  fprintf (stderr, "            ");

	fprintf (stderr, "(%d, %d) -> (%d, %d)\n",
		 pointBuf[0].x, pointBuf[0].y,
		 pointBuf[npoints-1].x, pointBuf[npoints-1].y);
      }
    }
  }
}

int XYCompare(VramPointType *p, VramPointType *q)
{
  if (p->x == q->x)
    return (p->y - q->y);
  else
    return (p->x - q->x);
}

SmoothShadedLine(xa, ya, xb, yb, r0, g0, b0, r1, g1, b1)
{
  int dx, dy, rinc, ginc, binc, len;

  dx = xb - xa;
  dy = yb - ya;

  if (iabs(dx) >= iabs(dy))
    len = iabs(dx);
  else
    len = iabs(dy);

  r0 <<= 12;
  g0 <<= 12;
  b0 <<= 12;
  r1 <<= 12;
  g1 <<= 12;
  b1 <<= 12;

  if (len != 0) {
    rinc = (r1 - r0) / len;
    ginc = (g1 - g0) / len;
    binc = (b1 - b0) / len;
/*
  fprintf(stderr, "rinc = %-.8x, ginc = %-.8x, binc = %-.8x\n", rinc, ginc, binc);
*/
  }

  BusWrite (RVAL_ADDRESS, r0 | ((ya&31) << 27), LWMASK);
  BusWrite (GVAL_ADDRESS, g0 | ((xa&31) << 27), LWMASK);
  BusWrite (BVAL_ADDRESS, b0, LWMASK);

  BusWrite (RINC_ADDRESS, rinc, LWMASK);
  BusWrite (GINC_ADDRESS, ginc, LWMASK);
  BusWrite (BINC_ADDRESS, binc, LWMASK);

  Line (xa, ya, xb, yb, -1, /* smooth? */ 1);
}

InterpolatedLine(xa, ya, xb, yb, r0, g0, b0, r1, g1, b1)
{
  int dx, dy, rinc, ginc, binc, len;

  dx=xb-xa; dy=yb-ya;

  if (abs(dx) >= abs(dy)) len=abs(dx);
  else                    len=abs(dy);

  r0<<=12; g0<<=12; b0<<=12;
  r1<<=12; g1<<=12; b1<<=12;

  if (len != 0) {
    rinc = (r1 - r0) / len; 
    ginc = (g1 - g0) / len; 
    binc = (b1 - b0) / len; }

  BusWrite (RVAL_ADDRESS, r0 | ((ya&31) << 27), LWMASK);
  BusWrite (GVAL_ADDRESS, g0 | ((xa&31) << 27), LWMASK);
  BusWrite (BVAL_ADDRESS, b0, LWMASK);

  BusWrite (RINC_ADDRESS, rinc, LWMASK);
  BusWrite (GINC_ADDRESS, ginc, LWMASK);
  BusWrite (BINC_ADDRESS, binc, LWMASK);

  Line (xa, ya, xb, yb, -1, /* smooth? */ 1);
}


Line(xa, ya, xb, yb, pattern, smooth)
unsigned pattern;
{
  int dx, dy, len, nextx, nexty;
  int e;
  unsigned regOffset;
  unsigned addr;

  dx = xb - xa;
  dy = yb - ya;

  if (dx == 0 && dy == 0) {
    OldLine (xa, ya, xb, yb, pattern);
    return;
  }

/*
  PrintBres(xa, ya, xb, yb);
*/

  regOffset = 0;
  if (dy >= 0) regOffset |= 0x04;
  if (dx >= 0) regOffset |= 0x08;
  if (iabs(dx) >= iabs(dy)) {
    regOffset |= 0x10;
    e = 2*iabs(dy) - iabs(dx);

    len = iabs(dx) + 1;

    nextx = xb + (dx>0 ? 1 : -1);
    nexty = yb + (e < 0 ? 0 : (dy > 0 ? 1 : -1));
  } else {
    e = 2*iabs(dx) - iabs(dy);

    len = iabs(dy) + 1;

    nextx = xb + (e < 0 ? 0 : (dx > 0 ? 1 : -1));
    nexty = yb + (dy > 0 ? 1 : -1);
  }

  MakeIdle();
  shadowMode.reg.mode = (smooth ? 0x28 : 0) | OPAQUELINE | (shadowMode.reg.mode & TRANSPMASK);

  shadowMode.reg.capEnds = 1;

  BusWrite (MODE_ADDRESS, shadowMode.u32, LWMASK);

  if ((len & 0xf) == 0)
    len -= 16;

  len -= (len & 0xf);
  addr = AddrBase() + (xa + ya*RAM_WIDTH) * SFBPIXELBYTES();
  
  npoints = 0;
  if (lastx != xa || lasty != ya) {
    LineWrite (ADDRREG_ADDRESS, addr & ADDRMASK);
  } else {
    fprintf (stderr, "segment continuation (%d, %d) --> (%d, %d)\n",
	  xa, ya, xb, yb);
  }
  if (shadowWidth.reg.drawable != RAM_WIDTH) {
    shadowWidth.reg.drawable = RAM_WIDTH;
    BusWrite (BRESWIDTH_ADDRESS, shadowWidth.u32, LWMASK);
  }
  LineWrite (SLOPE_000 + regOffset, (iabs(dy)<<16) + iabs(dx));

  lastx = nextx;
  lasty = nexty;
/*  fprintf (stderr, "(%d, %d) --> (%d, %d) [%d, %d]\n", xa, ya, xb, yb, nextx, nexty);*/
  
  while (len > 0) {
    LineWrite (BCONT_ADDRESS, 0xffff);
    len -= 16;
  }
  MakeIdle();

  {
    int dx = xa - xb;
    int dy = ya - yb;
    int len;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    if (dx > dy) len = dx + 1;
    else len = dy + 1;

    if (npoints != len) {
      fprintf (stderr, "****request (%d, %d) -> (%d, %d)\n", xa, ya, xb, yb);
      fprintf (stderr, "****actual  wrote %d pixels, expected %d pixels\n",
	       npoints, len);
    }
    if (npoints != 0 && (pointBuf[0].x != xa
	|| pointBuf[0].y != ya
	|| pointBuf[npoints-1].x != xb
	|| pointBuf[npoints-1].y != yb)) {
      int XYCompare();
      int lx, ly, rx, ry;
      
      if (xa > xb) {
	lx = xb; ly = yb; rx = xa; ry = ya;
      } else {
	lx = xa; ly = ya; rx = xb; ry = yb;
      }

      qsort(pointBuf, npoints, sizeof(VramPointType), XYCompare);

      if (pointBuf[0].x != lx
	  || pointBuf[0].y != ly
	  || pointBuf[npoints-1].x != rx
	  || pointBuf[npoints-1].y != ry) {
	if (npoints == len) {
	  fprintf (stderr, "****request (%d, %d) -> (%d, %d)\n", xa, ya, xb, yb);    
	  fprintf (stderr, "****actual  ");
	} else
	  fprintf (stderr, "            ");

	fprintf (stderr, "(%d, %d) -> (%d, %d)\n",
		 pointBuf[0].x, pointBuf[0].y,
		 pointBuf[npoints-1].x, pointBuf[npoints-1].y);
      }
    }
  }
}

iabs(x)
  {
    if (x < 0) return (-x);
    else return (x);
  }

SFBLEFTCOPYMASK(unsigned align, unsigned ones)
{
  unsigned mask = (((ones) << (align)) & (ones));
#if 0
  printf ("**** LeftCopyMask(%x, %x) = %-.8x\n", align, ones, mask);
#endif
  return mask;
}

SFBRIGHTCOPYMASK(unsigned alignedWidth, unsigned ones)
{
  unsigned mask = ((ones) >> (32-alignedWidth));
#if 0
  printf ("**** RightCopyMask(%x, %x) = %-.8x\n", alignedWidth, ones, mask);
#endif
  return mask;
}


#define SFBBYTESTOPIXELS(n) (n) /= SFBPIXELBYTES()

typedef int Bits32;
typedef Pixel32     PixelWord;
typedef Bits32	    CommandWord;

CommandWord sfbCopyAll1 = ((CommandWord)0xffffffff);
/* Command registers. */
typedef volatile struct {
    PixelWord   buffer[8];	/* Port to read/write copy buffer    */
    PixelWord   foreground;     /* Foreground register (minimum 32 bits)    */
    PixelWord   background;     /* Background register (minimum 32 bits)    */
    PixelWord   planemask;      /* Planemask (minimum 32 bits)		    */
    CommandWord	pixelmask;      /* Pixel mask register			    */
    SFBMode     mode;		/* Hardware mode			    */
    unsigned    rop;		/* Raster op for combining src, dst	    */
    int		shift;		/* -SFBALIGNMASK..+SFBALIGNMASK copy shift  */
    Pixel8      *address;	/* Pixel address register		    */
    Bits32      bres1;		/* a1, e1				    */
    Bits32	bres2;		/* a2, e2				    */
    Bits32	bres3;		/* e, count				    */
    Bits32	brescont;	/* Continuation data for lines		    */
    SFBDepth	depth;		/* 8, 16, or 32 bits/pixel?		    */
    CommandWord	start;		/* Start operation if using address reg     */
    CommandWord clear_interrupt;/* Clear Interrupt register		    */
    Bits32 	test_register;	/* ??? 					    */
    Bits32 	refresh_count;  /* interval between refresh reads	    */
    Bits32 	horizontal_setup;/* horizontal video state machine	    */
    Bits32	vertical_setup; /* vertical video state machine		    */
    Bits32	base_address;	/* base row address for starting scan line  */
    CommandWord video_valid;	/* writes to video registers have completed */
    CommandWord enable_disable_interrupt; /* low order bit determines       */
    Bits32	tcclk_counter;	/* oscillator clock counters		    */
    Bits32	vidclk_counter; /* oscillator clock counters		    */
} *SFB;

/*
 * A command word in DMAREAD mode looks like:
 *
 *	31          25 24             16 15   12 11    8 7     4 3     0
 *	+-------------+-----------------+-------+-------+-------+-------+
 *	| don't care  |  DMA word count |  rm1  |  rm0  |  lm1  |  lm0  |
 *	+-------------+-----------------+-------+-------+-------+-------+
 */
#define SFBBUSBYTES		4
#define SFBDMAREADALIGN		4
#define SFBDMAREADALIGNMASK     (SFBDMAREADALIGN-1)
#define SFBDMAALL1		0xff
#define SFBDMAMAXWORDS		512

/* ||| Can get rid of & (ones) here if put left mask at TOP of command word! */
SFBLEFTDMAREADMASK(unsigned align, unsigned ones)
{
  unsigned mask = (((ones) << (align)) & (ones));

  if (1)
    printf("****SfbLeftDmaReadMask (%x, %x) = %x\n", align, ones, mask);
  return mask;
}

SFBRIGHTDMAREADMASK(unsigned alignedWidth, unsigned ones)
{
  unsigned mask = ((ones) >> (-(alignedWidth) & SFBDMAREADALIGNMASK));

  if (1)
    printf("****SfbRightDmaReadMask (%x, %x) = %x\n", alignedWidth, ones, mask);
  return mask;
}

#define SFBDMACOMMAND(leftMask, rightMask, wordCount) \
  (((wordCount) << 16) | ((rightMask & 0xff) << 8) | (leftMask & 0xff))

#define SFBVIRTUALTOBUS(n) (n)

DmaReadBlt(Pixel8 *psrcLine, int dstx, int dsty, int width)
{
  register int	dstAlign;	/* Last few bits of destination ptr */
  register int	srcAlign;       /* last few bits of source ptr      */
  register int	shift;		/* Mostly dstAlign-srcAlign	    */

  unsigned wordCount;		/* count of words to read on I/O bus */
  Pixel8		*pdstLine; 		/* Current dest scanline    */
  CommandWord		onesDMA = SFBDMAALL1;
  unsigned		leftMask, rightMask;
  SFB			sfb;
  unsigned		command;
  int                   i;

  if (1)
    printf("***DmaReadBlt(%x, %d, %d, %d)\n", psrcLine, dstx, dsty, width);
  SFBMODE(sfb, DMAREAD);

  if (shadowRop.reg.visual != 0) {
    width <<= 2;
    pdstLine = (Pixel8 *) (AddrBase() + (dsty * RAM_WIDTH + dstx) * 4);
  } else {
    pdstLine = (Pixel8 *) (AddrBase() + (dsty * RAM_WIDTH) + dstx);
  }
  dstAlign = (int)pdstLine & SFBDMAREADALIGNMASK;
  srcAlign = (int)psrcLine & SFBDMAREADALIGNMASK;
  
  shift = dstAlign - srcAlign;
  if (shift < 0) {
    /* Ooops.  First source word has less data than destination
       needs, so first word written is junk that primes the pump.
       Adjust shift and dstAlign to reflect this fact. */
    shift += SFBDMAREADALIGN;
    dstAlign += SFBDMAREADALIGN;
  }
  SFBSHIFT(sfb, shift);
  psrcLine -= srcAlign;
  pdstLine -= dstAlign;
  width += dstAlign;
  wordCount = (width - 1) / SFBBUSBYTES;
  leftMask = SFBLEFTDMAREADMASK(dstAlign, onesDMA);
  rightMask = SFBRIGHTDMAREADMASK(width, onesDMA);
  if ((rightMask >> (SFBDMAREADALIGN + shift)) != 0) {
    /* Don't drain residue case, adjust right mask and wordCount */
    rightMask >>= SFBDMAREADALIGN;
    wordCount += 1;
  }
  if (wordCount == 1)
    /* Only right mask will be used - merge in left mask */
    rightMask &= leftMask;
  command = SFBDMACOMMAND(leftMask, rightMask, wordCount-1);
  psrcLine = SFBVIRTUALTOBUS(psrcLine);

  SFBDMAADDRESS(psrcLine);
printf("**** DmaReadBlt: addr = %-.8x, command = %-.8x\n", pdstLine, command);

  SFBWRITE(pdstLine, command);
}

#define SFBDMAWRITEALIGN		8
#define SFBDMAWRITEALIGNMASK     (SFBDMAWRITEALIGN-1)

#define SFB_DST_SLEAZEDMAWRITEALIGN		4
#define SFB_DST_SLEAZEDMAWRITEALIGNMASK     (SFB_DST_SLEAZEDMAWRITEALIGN-1)

#define SFBVRAMBYTES		8

/* ||| Can get rid of & (ones) here if put left mask at TOP of command word! */
SFBLEFTDMAWRITEMASK(unsigned align, unsigned ones)
{
  unsigned mask = (((ones) << (align)) & (ones));

  if (1)
    printf("****SfbLeftDmaWriteMask (%x, %x) = %x\n", align, ones, mask);
  return mask;
}

SFBRIGHTDMAWRITEMASK(unsigned alignedWidth, unsigned ones)
{
  unsigned mask = ((ones) >> (-(alignedWidth) & SFBDMAWRITEALIGNMASK));

  if (1)
    printf("****SfbRightDmaWriteMask (%x, %x) = %x\n", alignedWidth, ones, mask);
  return mask;
}

DmaWriteBlt(Pixel8 *pdstLine, int srcx, int srcy, int width)
{
  register int	dstAlign;	/* Last few bits of destination ptr */
  register int	srcAlign;       /* last few bits of source ptr      */
  register int	shift;		/* Mostly dstAlign-srcAlign	    */

  unsigned wordCount;		/* count of words to read on I/O bus */
  Pixel8		*psrcLine; 		/* Current dest scanline    */
  CommandWord		onesDMA = SFBDMAALL1;
  unsigned		leftMask, rightMask;
  SFB			sfb;
  unsigned		command;
  int                   i;

  if (1)
    printf("***DmaWriteBlt(%x, %d, %d, %d)\n", pdstLine, srcx, srcy, width);
  SFBMODE(sfb, DMAWRITE);

  if (shadowMode.reg.visual != 0) {
    width <<= 2;
    psrcLine = (Pixel8 *) (AddrBase() + (srcy * RAM_WIDTH + srcx) * 4);
  } else {
    psrcLine = (Pixel8 *) (AddrBase() + (srcy * RAM_WIDTH) + srcx);
  }
  srcAlign = (int)psrcLine & SFBDMAWRITEALIGNMASK;
  dstAlign = (int)pdstLine & SFB_DST_SLEAZEDMAWRITEALIGNMASK;
  
  shift = dstAlign - srcAlign;
  if (shift < 0) {
    /* Ooops.  First source word has less data than destination
       needs, so first word written is junk that primes the pump.
       Adjust shift and dstAlign to reflect this fact. */
    shift += SFBDMAWRITEALIGN;
  } else {
    /* First source word has enough data, so priming the pump isn't
       necessary.  Hardware primes the pump regardless, so we'll have
       to back up the source address to compensate. */
    srcAlign += SFBDMAWRITEALIGN;
  }
  SFBSHIFT(sfb, shift);
  /* !!!! if 8-bit unpacked this adjustment should be *4 !!!! */
  psrcLine -= srcAlign;
  pdstLine -= dstAlign;
  width += dstAlign + SFBDMAWRITEALIGN;
  wordCount = (width - 1) / SFBVRAMBYTES;
  leftMask = SFBLEFTDMAWRITEMASK(dstAlign, onesDMA);
  rightMask = SFBRIGHTDMAWRITEMASK(width, onesDMA);
  if (wordCount <= 1)
    /* Only right mask will be used - merge in left mask */
    rightMask &= leftMask;
  command = SFBDMACOMMAND(leftMask, rightMask, wordCount);
  pdstLine = SFBVIRTUALTOBUS(pdstLine);

  SFBDMAADDRESS(pdstLine);
printf("**** DmaWriteBlt: addr = %-.8x, command = %-.8x\n", psrcLine, command);

  SFBWRITE(psrcLine, command);
}

NewAreaCopy(srcx, srcy, dstx, dsty, deltax, deltay)
{
  register int	dstAlign;	/* Last few bits of destination ptr */
  register int	srcAlign;       /* last few bits of source ptr      */
  register int	shift;		/* Mostly dstAlign-srcAlign	    */
  register Pixel8	*psrc;	/* pointer to current src longword  */
  register Pixel8	*pdst;	/* pointer to current dst longword  */
  register int	width;		/* width to blt			    */
  register int	h;		/* height to blt		    */

  Pixel8		*psrcLine;		/* Current source scanline  */
  Pixel8		*pdstLine; 		/* Current dest scanline    */
  CommandWord		ones = sfbCopyAll1;
  CommandWord		mask, leftMask, rightMask;
  int			m;
  SFB			sfb;

  width = deltax;
  h = deltay;

  psrcLine = (Pixel8 *) (AddrBase() + (srcy * RAM_WIDTH));
  pdstLine = (Pixel8 *) (AddrBase() + (dsty * RAM_WIDTH)); 

  psrcLine += srcx * SFBPIXELBYTES(); 
  pdstLine += dstx * SFBPIXELBYTES(); 
  srcAlign = (int)psrcLine & SFBALIGNMASK();
  dstAlign = (int)pdstLine & SFBALIGNMASK();
  shift = dstAlign - srcAlign;

  if (shift < 0) {
    /*
     * Ooops.  First source word has less data in it than we need
     * to write to destination, so first word written to internal
     * sfb copy buffer will be junk that just primes the pump.
     * Adjust shift and dstAlign to reflect this fact.
     */
    shift += SFBALIGNMENT();
    dstAlign += SFBALIGNMENT();
  }

  SFBMODE(sfb, COPY);

  SFBSHIFT(sfb, shift);
  psrcLine -= srcAlign;
  pdstLine -= dstAlign;
  SFBBYTESTOPIXELS(dstAlign);
  width += dstAlign;
  leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
  rightMask = SFBRIGHTCOPYMASK(width, ones);
  if (width <= SFBCOPYBITS()) {
    /* The mask fits into a single word */
    mask = leftMask & rightMask;
    do {
      SFBWRITE(psrcLine, rightMask);
      SFBWRITE(pdstLine, mask);
      psrcLine += RAM_WIDTH;
      pdstLine += RAM_WIDTH;
      h--;
    } while (h != 0);
  } else {
    /* Mask requires multiple words */
    do {
      psrc = psrcLine;
      pdst = pdstLine;
      SFBWRITE(psrcLine, ones);
      SFBWRITE(pdstLine, leftMask);
      for (m = width - 2*SFBCOPYBITS(); m > 0; m -= SFBCOPYBITS()) {
	psrc += SFBCOPYBYTESDONE();
	pdst += SFBCOPYBYTESDONE();
	SFBWRITE(psrc, ones);
	SFBWRITE(pdst, ones);
      }
      SFBWRITE(psrc+SFBCOPYBYTESDONE(), rightMask);
      SFBWRITE(pdst+SFBCOPYBYTESDONE(), rightMask);
      psrcLine += RAM_WIDTH;
      pdstLine += RAM_WIDTH;
      h--;
    } while (h != 0);
  } /* if small copy else big copy */
}

SFBWRITE(char *psfb, unsigned data)
{
#if 0
  fprintf(stderr, "**** SFBWRITE(0x%-.8x, 0x%-.8x)\n", psfb, data);
#endif
  BusWrite (psfb, data, LWMASK);
}


SFBSHIFT(sfb, shift)
{
  if (0)
    fprintf (stderr, "**** shft %d\n", shift);

  BusWrite (PIXSHFT_ADDRESS, (unsigned) shift, LWMASK);
}


SFBMODE(unsigned sfb, unsigned mode)
{
  if (0)
    fprintf (stderr, "**** mode %d\n", mode);

  shadowMode.reg.mode = mode;
  BusWrite (MODE_ADDRESS, shadowMode.u32, LWMASK);
}

SFBDMAADDRESS(unsigned pdma)
{
  if (0)
    fprintf (stderr, "**** dmaAddr %d\n", pdma);

  BusWrite (DMABASE, pdma, LWMASK);
}


AreaCopy(srcx, srcy, dstx, dsty, deltax, deltay)
{
  int i, dy;
  int shiftCnt;	/* shift count */

  unsigned
    ldm,	/* left destination mask */
    rdm,	/* right destination mask */
    dx,		/* delta-x word count */
    lc,		/* left count */
    rc,		/* right count */
    mc,		/* middle count */
    lnc,	/* left nibble count */
    lnm,	/* left nibble mask */
    rnm,	/* right nibble mask */
    srcAddr,	/* source address */
    dstAddr;	/* destination address */

  shiftCnt = (dstx%(1+SFBALIGNMASK())) - (srcx%(1+SFBALIGNMASK()));

  if (shiftCnt < 0) {
    /*
     * can't use residue. 
     * adjust destination mask and destination address.
     */
    shiftCnt += (1+SFBALIGNMASK());
    lnm = 0;
    dstx -= (1+SFBALIGNMASK());
    deltax += (1+SFBALIGNMASK());
  } else
    lnm = OpMask() & (OpMask() << (dstx & SFBALIGNMASK()));

  shadowMode.reg.mode = COPY;
  BusWrite (MODE_ADDRESS, shadowMode.u32, LWMASK);
  if (0)
    fprintf (stderr, "**** shft %d\n", shiftCnt);
  BusWrite (PIXSHFT_ADDRESS, (unsigned) shiftCnt, LWMASK);

  lnc = (1+SFBALIGNMASK()) - (dstx & SFBALIGNMASK());
  dx = (lnc+SFBALIGNMASK())/(1+SFBALIGNMASK()) + (deltax-lnc+SFBALIGNMASK())/(1+SFBALIGNMASK());

  lc = (dx < OpsPerCmd() ? dx : OpsPerCmd());

  rnm = OpMask() >> ((SFBALIGNMASK()-(dstx+deltax-1)) & SFBALIGNMASK());

  for (i=1; i<=lc; ++i) {
    if (i == dx)	/* it's the rightmost nibble */
      ldm |= rnm << (1+SFBALIGNMASK())*(i-1);
    else if (i == 1)	/* it's the leftmost nibble */
      ldm = lnm;
    else		/* it's a nibble between left and right */
      ldm |= OpMask() << (1+SFBALIGNMASK())*(i-1);
  }

  rc = (dx - lc) % OpsPerCmd();
  mc = dx - (lc + rc);
  if (mc % OpsPerCmd() != 0)
    fprintf(stderr, "AreaCopy: middle count is not a multiple of %d (%d)\n",
	    OpsPerCmd(), mc);
  mc /= OpsPerCmd();

  rdm = 0;
  for (i=1; i<rc; ++i)
    rdm |= OpMask() << (1+SFBALIGNMASK())*(i-1);

  rdm |= rnm << (1+SFBALIGNMASK())*(rc-1);

  for (dy=0; dy<deltay; ++dy) {
    srcAddr = AddrBase() + SFBPIXELBYTES() * ((srcx & ~SFBALIGNMASK()) + (dy + srcy)*RAM_WIDTH);
    dstAddr = AddrBase() + SFBPIXELBYTES() * ((dstx & ~SFBALIGNMASK()) + (dy + dsty)*RAM_WIDTH);
    RasterCopy (shiftCnt, srcAddr, dstAddr, ldm, mc, rc, rdm);
  }
}

unsigned cpydata[128];

RasterCopy (shiftCnt, srcAddr, dstAddr, ldm, mc, rc, rdm)
{
  int i;

  BusWrite (PIXSHFT_ADDRESS, (unsigned) shiftCnt, LWMASK);
  /*
   * left edge.
   */
  CopyBytes (srcAddr, ldm|OpMask(), dstAddr, ldm);
  
  /*
   * middle area.
   */
  for (i=1; i<=mc; i+=2) {
    BusWrite (COPY64SRC, srcAddr + i*32 - AddrBase(), LWMASK);
    BusWrite (COPY64DST, dstAddr + i*32 - AddrBase(), LWMASK);
  }

  if (mc & 1 == 0)
    CopyBytes (srcAddr + mc*32, CopyMask(), dstAddr + mc*32, CopyMask());

  /*
   * right edge.
   */
  if (rc > 0) {
    CopyBytes (srcAddr + i*32, rdm, dstAddr + i*32, rdm);
  }
}


RasterCopy2 (shiftCnt, srcAddr, dstAddr, ldm, mc, rc, rdm)
{
  int i;

  BusWrite (PIXSHFT_ADDRESS, (unsigned) shiftCnt, LWMASK);
  /*
   * left edge.
   */
  ReadBytes (0, srcAddr, ldm|OpMask());
  
  /*
   * middle area.
   */
  for (i=1; i<=mc; ++i)
    ReadBytes (i*(1+SFBALIGNMASK()), srcAddr + i*32, CopyMask());

  /*
   * right edge.
   */
  if (rc > 0) {
    ReadBytes (i*(1+SFBALIGNMASK()), srcAddr + i*32, rdm);
  }

  BusWrite (PIXSHFT_ADDRESS, (unsigned) 0, LWMASK);
  /*
   * left edge.
   */
  WriteBytes (0, dstAddr, ldm);
  
  /*
   * middle area.
   */
  for (i=1; i<=mc; ++i)
    WriteBytes (i*(1+SFBALIGNMASK()), dstAddr + i*32, CopyMask());

  /*
   * right edge.
   */
  if (rc > 0) {
    WriteBytes (i*(1+SFBALIGNMASK()), dstAddr + i*32, rdm);
  }
}


CopyBytes (srcAddr, srcMask, dstAddr, dstMask)
unsigned srcAddr, dstAddr;
unsigned srcMask, dstMask;
{
  if (0)
    fprintf (stderr, "**** copy %-.8x %-.8x %-.8x %-.8x\n",
	     srcAddr&0x1fffff, srcMask, dstAddr&0x1fffff, dstMask);
  BusWrite (srcAddr, srcMask, LWMASK);
  BusWrite (dstAddr, dstMask, LWMASK);
}

ReadBytes (offset, srcAddr, srcMask)
unsigned srcAddr, srcMask;
{
  int i;

  BusWrite (srcAddr, srcMask, LWMASK);
  for (i=0; i<8; ++i)
    cpydata [offset + i] = BusRead (CPYBF0_ADDRESS + 4*i);
}

WriteBytes (offset, dstAddr, dstMask)
unsigned dstAddr, dstMask;
{
  int i;

  for (i=0; i<8; ++i)
    BusWrite (CPYBF0_ADDRESS + 4*i, 0x01010101 + cpydata [offset + i], LWMASK);

  BusWrite (dstAddr, dstMask, LWMASK);
}

PutBytes (x)
unsigned x;
{
  int i;

  for (i=0; i<4; ++i)
    fprintf (stderr, "%c", x & (0xff << (i*8)) ? 'x' : ' ');
}

LineWrite (addr, data)
unsigned addr, data;
{
  if (0)
    fprintf (stderr, "**** line %-.8x %-.8x\n", addr, data);
  BusWrite (addr, data, LWMASK);
}
