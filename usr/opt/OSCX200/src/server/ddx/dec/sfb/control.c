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
 * behavioral model of sfb control logic.
 *
 * Lindsay Gage, Sep-90 
 * Bob McNamara, Oct-90
 */

#include <stdio.h>

#include "defs.h"
#include "types.h"
#include "vars.h"

extern FILE *cmdFile;
extern int logFlag;
int debugFlag = 0;

/*
 * read the VRAM's and return 64-bits of data.
 */
ULONG64
ReadVRAM (addr, mask)
unsigned addr;
unsigned mask;
{
  ULONG64 zero, data;

  zero.lo = zero.hi = 0;
  data = do_rams (addr, zero, sfbreg.planemask, mask, 1);
  if (debugFlag)
    fprintf (stderr, "**** read : %08x%08x, address = %08x\n",
	     data.hi, data.lo, addr<<3);
  return (data);
}

/*
 * write 64-bits of data to the VRAM's.
 */
WriteVRAM (addr, bytmsk, src)
unsigned addr, bytmsk;
ULONG64 src;
{
  ULONG64 dst, data, dummy;

  dst = ReadVRAM (addr, bytmsk);
  data = BoolOp (src, dst);
  if (debugFlag)
  fprintf (stderr, "**** write: %08x%08x, address = %08x, cas = %02x, planemask = %08x\n",
	   data.hi, data.lo, addr<<3, bytmsk, sfbreg.planemask);
  dummy = do_rams (addr, data, sfbreg.planemask, bytmsk, 0);
}

/*
 * simulate until everything becomes idle.
 */
MakeIdle()
{
  do {
    CombControl ();
    (void) Datapath (0, 0);
    SfbControl ();
    ClockEdge ();
  } while (ctl.sig.q0 || ctl.sig.q1 || ctl.sig.wait ||
	   ctl.sig.busy || ctl.sig.tcreq || ctl.sig.rq
	   || !ctl.sig.memidle);
}


#define SFBALIGNMENT    8
#define SFBALIGNMASK    (SFBALIGNMENT-1)
#define ALL1		0xff

void FillMemory(addr, width, height, scanlineWidth, pixel)
    unsigned addr;		/* Byte address		    */
    int     width;		/* width in pixels	    */
    int     height;		/* height in scanlines	    */
    int     scanlineWidth;      /* # bytes to next scanline */
    int     pixel;		/* 32-bit pixel value       */
{
    ULONG64     data;
    unsigned    p;
    unsigned    leftMask, rightMask, mask;
    int		saveLogFlag;
    int		m, align;
    int		SFBPIXELBYTES;
    
    MakeIdle();
    if (logFlag)
	fprintf (cmdFile, "2 %x %x %x %x %x\n",
	    addr, width, height, scanlineWidth, pixel);

    switch (sfbreg.depth) {
	case DEPTH8:  SFBPIXELBYTES = 1; break;
	case DEPTH16: SFBPIXELBYTES = 2; break;
	case DEPTH32: SFBPIXELBYTES = 4; break;
    }
    data.lo = pixel;
    data.hi = pixel;
    saveLogFlag = logFlag;
    logFlag = FALSE;
    addr /= SFBPIXELBYTES;
    scanlineWidth /= SFBPIXELBYTES;
    align = addr & SFBALIGNMASK;
    addr -= align;
    width += align;
    leftMask = (ALL1 << align) & ALL1;
    rightMask = ALL1 >> ((-width) & SFBALIGNMASK);

    if (width <= SFBALIGNMENT) {
	mask = leftMask & rightMask;
	switch (sfbreg.depth) {
	    case DEPTH8:
		do {
		    do_rams(addr/SFBALIGNMENT, data, ~0, mask, 0);
		    addr += scanlineWidth;
		    height--;
		} while (height > 0);
		break;
	    case DEPTH16:
		do {
		    do_rams(addr/SFBALIGNMENT, data, ~0, mask & 0x0f, 0);
		    do_rams(addr/SFBALIGNMENT, data, ~0, mask & 0xf0, 0);
		    addr += scanlineWidth;
		    height--;
		} while (height > 0);
		break;
	    case DEPTH32:
		do {
		    do_rams(addr/SFBALIGNMENT, data, ~0, mask & 0x03, 0);
		    do_rams(addr/SFBALIGNMENT, data, ~0, mask & 0x0c, 0);
		    do_rams(addr/SFBALIGNMENT, data, ~0, mask & 0x30, 0);
		    do_rams(addr/SFBALIGNMENT, data, ~0, mask & 0xc0, 0);
		    addr += scanlineWidth;
		    height--;
		} while (height > 0);
		break;
	}
    } else {
	switch (sfbreg.depth) {
	    case DEPTH8:
		do {
		    p = addr/SFBALIGNMENT;
		    do_rams(p, data, ~0, leftMask, 0);
		    for (m = width - 2*SFBALIGNMENT; m > 0; m -= SFBALIGNMENT) {
			p += 1;
			do_rams(p, data, ~0, ALL1, 0);
		    }
		    do_rams(p+1, data, ~0, rightMask, 0);
		    addr += scanlineWidth;
		    height--;
		} while (height > 0);
		break;
	    case DEPTH16:
		do {
		    p = addr/SFBALIGNMENT;
		    do_rams(p, data, ~0, leftMask & 0x0f, 0);
		    do_rams(p, data, ~0, leftMask & 0xf0, 0);
		    for (m = width - 2*SFBALIGNMENT; m > 0; m -= SFBALIGNMENT) {
			p += 1;
			do_rams(p, data, ~0, 0x0f, 0);
			do_rams(p, data, ~0, 0xf0, 0);
		    }
		    do_rams(p+1, data, ~0, rightMask & 0x0f, 0);
		    do_rams(p+1, data, ~0, rightMask & 0xf0, 0);
		    addr += scanlineWidth;
		    height--;
		} while (height > 0);
		break;
	    case DEPTH32:
		do {
		    p = addr/SFBALIGNMENT;
		    do_rams(p, data, ~0, leftMask & 0x03, 0);
		    do_rams(p, data, ~0, leftMask & 0x0c, 0);
		    do_rams(p, data, ~0, leftMask & 0x30, 0);
		    do_rams(p, data, ~0, leftMask & 0xc0, 0);
		    for (m = width - 2*SFBALIGNMENT; m > 0; m -= SFBALIGNMENT) {
			p += 1;
			do_rams(p, data, ~0, 0x03, 0);
			do_rams(p, data, ~0, 0x0c, 0);
			do_rams(p, data, ~0, 0x30, 0);
			do_rams(p, data, ~0, 0xc0, 0);
		    }
		    do_rams(p+1, data, ~0, rightMask & 0x03, 0);
		    do_rams(p+1, data, ~0, rightMask & 0x0c, 0);
		    do_rams(p+1, data, ~0, rightMask & 0x30, 0);
		    do_rams(p+1, data, ~0, rightMask & 0xc0, 0);
		    addr += scanlineWidth;
		    height--;
		} while (height > 0);
		break;
	}
    }
    logFlag = saveLogFlag;
}

/*
 * simulate a TURBOchannel read transaction.
 */
unsigned
BusRead (addr)
unsigned addr;
{
  IOADDR a;
  unsigned readData;

/*  fprintf (stderr, "reading %08x\n", addr);*/
  if (logFlag)
    fprintf (cmdFile, "0 %x ", addr);
  a.un = 0;
  a.io.addr = addr >> 2;
  a.io.mask = 0;

  /*
   * assert TURBOchannel sel.
   * send address for 1 tick.
   */
  sel = 1;
  wr = 0;
  CombControl ();
  (void) Datapath (a, wr);
  SfbControl ();

  /*
   * send data and keep ticking until rdy asserts.
   */
  do {
    ClockEdge ();
    CombControl ();
    readData = Datapath (0, wr);
    SfbControl ();
  } while (ctl.sig.tcRdy == 0);

  ClockEdge ();

  /*
   * then deassert sel for one tick.
   */
  sel = 0;
  wr = 0;
  CombControl ();
  (void) Datapath (0, wr);
  SfbControl ();
  ClockEdge ();

  if (logFlag)
    fprintf (cmdFile, "%x\n", readData);
  return (readData);
}

/*
 * simulate a TURBOchannel write transaction.
 */
void
BusWrite (addr, data, mask)
unsigned addr, data, mask;
{
  IOADDR a;

/*  fprintf (stderr, "writing %08x to %08x\n", data, addr); */
  if (logFlag)
    fprintf (cmdFile, "1 %x %x %x\n", addr, data, mask);
  a.un = 0;
  a.io.addr = addr >> 2;
  a.io.mask = mask;

  /*
   * assert TURBOchannel sel and wr.
   * send address for 1 tick.
   */
  sel = 1;
  wr = 1;
  CombControl ();
  (void) Datapath (a, wr);
  SfbControl ();

  /*
   * send data and keep ticking until rdy asserts.
   */
  do {
    ClockEdge ();
    CombControl ();
    (void) Datapath (data, wr);
    SfbControl ();
  } while (ctl.sig.tcRdy == 0);

  ClockEdge ();
  /*
   * then deassert sel and wr for one tick.
   */
  sel = 0;
  wr = 0;
  CombControl ();
  (void) Datapath (0, wr);
  SfbControl ();
  ClockEdge ();
}

int i_doRead, i_doWrite;

/*
 * simulation of the FSM controlling
 * the majority of the datapath.
 */
SfbControl ()
{
  int writeReg;
  int readReg;
  int writeFB;
  int readFB;
  int next_sfbctl_shift, next_memctl_shift;

#define CLOCKED
#define COMBINATIONAL

  writeReg = (ctl.sig.tcreq && tcWrite
	      && (tcAddr & REGMASK) == REGVAL);
  readReg = (ctl.sig.tcreq && !tcWrite
	     && (tcAddr & REGMASK) == REGVAL);
  writeFB = (ctl.sig.tcreq && tcWrite
	     && (tcAddr & REGMASK) != REGVAL);
  readFB = (ctl.sig.tcreq && !tcWrite
	     && (tcAddr & REGMASK) != REGVAL);

#include "sfbcontrol.c"
#include "fasttc.c"
#include "memctl.c"
#undef CLOCKED
#undef COMBINATIONAL

  /*
   * merge ready's from tccontrol and sfbcontrol.
   */
  next_ctl.sig.tcRdy = next_ctl.sig.wrRdy || next_ctl.sig.rdRdy;

  /*
   * make actual "d1busy" from "busy".
   */
  next_ctl.sig.d1busy = ctl.sig.busy;

  /*
   * merge "shift" from memctl and sfbcontrol.
   */
  next_ctl.sig.shift = next_memctl_shift || next_sfbctl_shift;

  if (i_doRead) next_dstData = ReadVRAM (memaddr, membm);
  if (i_doWrite) WriteVRAM (memaddr, membm, memdata);

  next_ctl.sig.selBuff = actl.sig.i_selBuff;

  if (next_ctl.sig.setReadFlag)
    next_readFlag = 1;
  else if (next_ctl.sig.resetReadFlag)
    next_readFlag = 0;
  else
    next_readFlag = readFlag;
}

/*
 * simulate the combinational control logic.
 */
CombControl()
{
  int writeReg;
  int readReg;
  int writeFB;
  int readFB;

  writeReg = (ctl.sig.tcreq && tcWrite
	      && (tcAddr & REGMASK) == REGVAL);
  readReg = (ctl.sig.tcreq && !tcWrite
	     && (tcAddr & REGMASK) == REGVAL);
  writeFB = (ctl.sig.tcreq && tcWrite
	     && (tcAddr & REGMASK) != REGVAL);
  readFB = (ctl.sig.tcreq && !tcWrite
	     && (tcAddr & REGMASK) != REGVAL);

#undef CLOCKED
#define COMBINATIONAL
#include "sfbcontrol.c"
#include "fasttc.c"
#include "memctl.c"
#undef CLOCKED
#undef COMBINATIONAL
}

/*
 * simulate the edge triggered flops in
 * both the control and the datapath.
 */
ClockEdge()
{
  PrintState();

  ctl = next_ctl;
  sfbAddr = next_sfbAddr;
  bresError = next_bresError;
  nextBmask = next_nextBmask;
  buf[0] = next_buf[0];
  buf[1] = next_buf[1];
  wrnext = next_wrnext;
  wrptr = next_wrptr;
  readFlag = next_readFlag;
  dstData = next_dstData;
  memaddr = next_memaddr;
  membm = next_membm;
  memdata = next_memdata;
  addrIn = next_addrIn;
  dataIn = next_dataIn;
  bmIn = next_bmIn;
  hldAD = next_hldAD;
  buffA = next_buffA;
  buffD = next_buffD;
  lineLength = next_lineLength & 0xf;
  lineDone = next_lineDone;
  hldWr = next_hldWr;
  buffWr = next_buffWr;
  writeIn = next_writeIn;
  iterCount = next_iterCount;
}

PrintState()
{
#if 1
  int	i;

  if (debugFlag) {
    fprintf (stderr, "%d ", sel);
    fprintf (stderr, "%d ", wr);
    fprintf (stderr, "%d ", ctl.sig.tcRdy);

    i = ctl.sig.tc3*8 + ctl.sig.tc2*4 + ctl.sig.tc1*2 + ctl.sig.tc0;
    fprintf (stderr, "%s ", 
	     (i==0x0 ? "IDLE   " :
	      i==0x1 ? "CMDHOLD" :
	      i==0x2 ? "WRADDR " :
	      i==0x8 ? "RDADDR " :
	      i==0x4 ? "DATAHLD" :
	      i==0x6 ? "TCSTALL" : "???????"));

    fprintf (stderr, "%d ", ctl.sig.tcreq);
    fprintf (stderr, "%d ", ctl.sig.busy);
    fprintf (stderr, "%d ", actl.sig.i_busy);
    
    i = ctl.sig.busy*8 + ctl.sig.q2*4 + ctl.sig.q1*2 + ctl.sig.q0;
    fprintf (stderr, "%s ", 
	     (i==0x0 ? "IDLE" :
	      i==0x9 ? "OP1 " :
	      i==0xb ? "OP2 " :
	      i==0x8 ? "PEND" :
	      i==0xa ? "WRRG" :
	      i==0xc ? "RDRG" :
	      i==0xd ? "RDR1" :
	      i==0xf ? "RDR2" :
	      i==0xe ? "RDR3" : "????"));

    fprintf (stderr, "%2d ", lineLength);
    fprintf (stderr, "%d ", ctl.sig.rq);
    fprintf (stderr, "%d ", ctl.sig.next);
    fprintf (stderr, "%d ", ctl.sig.ldBresErr);
    fprintf (stderr, "%d ", ctl.sig.wait);
    fprintf (stderr, "%d ", pixptr);
    fprintf (stderr, "%d ", ctl.sig.ldPixelMask);
    fprintf (stderr, "%d ", ctl.sig.setMask);
    
    i = ctl.sig.memidle*16 +
      ctl.sig.mc3*8 + ctl.sig.mc2*4 + ctl.sig.mc1*2 + ctl.sig.mc0;
    fprintf (stderr, "%s ", 
	     (i==0x10 ? "IDLE" :
	      i==0x01 ? "RDPR" :
	      i==0x03 ? "RRAS" :
	      i==0x07 ? "RCOL" :
	      i==0x06 ? "RCAS" :
	      i==0x09 ? "WRPR" :
	      i==0x0b ? "WRAS" :
	      i==0x0f ? "WCOL" :
	      i==0x0e ? "WCAS" :
	      i==0x14 ? "PAGE" : "????"));

    fprintf (stderr, "%08x %02x ", memaddr, membm);
    
    fprintf (stderr, "\n");
  }
#endif
#if 0
  int	i;

  i = ctl.sig.tc3*8 + ctl.sig.tc2*4 + ctl.sig.tc1*2 + ctl.sig.tc0;
  switch (i) {
  case 0: fprintf (stderr, "TCIDLE  "); break;
  case 1: fprintf (stderr, "CMDHOLD "); break;
  case 2: fprintf (stderr, "WRADDR  "); break;
  case 8: fprintf (stderr, "RDADDR  "); break;
  case 4: fprintf (stderr, "DATAHLD "); break;
  case 6: fprintf (stderr, "TCSTALL "); break;
  default: fprintf(stderr, "???     "); break;
  }

  fprintf (stderr, "%ctcreq ", ctl.sig.tcreq ? ' ' : '-');
  fprintf (stderr, "%ctcRdy ", ctl.sig.tcRdy ? ' ' : '-');
  fprintf (stderr, "%ctcsel ", sel ? ' ' : '-');


  switch (i) {
  case 0:
    fprintf(stderr, "IDLE");
    break;

  case 1:
    fprintf(stderr, "OP1 ");
    break;

  case 2:
    fprintf(stderr, "WREG");
    break;

  case 3:
    fprintf(stderr, "OP2 ");
    break;

  default:
    fprintf (stderr, "??\n");
    exit(1);
  }

  fprintf (stderr, " %cwait", ctl.sig.wait ? ' ' : '-');
  fprintf (stderr, " %cnext", ctl.sig.next ? ' ' : '-');
  fprintf (stderr, " %crq", ctl.sig.rq ? ' ' : '-');
/*
  fprintf (stderr, " %cwait", ctl.sig.wait ? ' ' : '-');
  fprintf (stderr, " %cwait", ctl.sig.wait ? ' ' : '-');
  fprintf (stderr, " %cwait", ctl.sig.wait ? ' ' : '-');
  fprintf (stderr, " %cwait", ctl.sig.wait ? ' ' : '-');
*/

  { IOADDR a;
    a.un = buffA;
    fprintf (stderr, " %08x", a.io.addr<<2);
    a.un = hldAD;
    fprintf (stderr, " %08x %08x", a.io.addr<<2, addrIn);
  }
  fprintf (stderr, " %cendat ", actl.sig.i_endat ? ' ' : '-');
  fprintf (stderr, "\n");
#endif
}
