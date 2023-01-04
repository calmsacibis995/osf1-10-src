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
 * behavioral model of sfb datapath.
 *
 * Lindsay Gage, Sep-90 
 * Bob McNamara, Oct-90
 */


#include <stdio.h>

#include "defs.h"
#include "types.h"
#include "vars.h"

/*
 * simulate the majority of the datapath.
 */
unsigned
Datapath (tcAD, tcWr)
unsigned tcAD;
{
  unsigned tcData, tcMask;
  ULONG64 stippleData, copyData, dataIn64;

  unsigned bitMask, stippleBits, lineBits, stipple;
  unsigned curAddr, nxtAddr, addrOut, adrLo3, errorSign;

  int lineInc, pixInc, addrInc, errorInc, curError, nxtError;

  REGS wdata;
  IOADDR a;
  
  /*
   * TURBOchannel interface.
   */
  if (actl.sig.i_enhld) next_hldAD = tcAD;
  if (actl.sig.i_enhld) next_hldWr = tcWr;
  if (actl.sig.i_enadr) next_buffA = hldAD;
  if (actl.sig.i_enadr) next_buffWr = hldWr;
  if (ctl.sig.endat) next_buffD = hldAD;

  a.un   = (actl.sig.i_selBuff ? buffA : hldAD);
  tcData = (actl.sig.i_selBuff && ctl.sig.selBuff    ? buffD : hldAD);
  tcWrite= (actl.sig.i_selBuff ? buffWr : hldWr);

  tcAddr = a.io.addr << 2;
  tcMask = a.io.mask;

  /*
   * internal registers
   */
  wdata.data = dataIn;
  if (ctl.sig.ldMode) sfbreg.mode = wdata.data;
  if (ctl.sig.ldPlnmsk) sfbreg.planemask = wdata.data;
  if (ctl.sig.ldBoolop) sfbreg.rop = wdata.data;
  if (ctl.sig.ldDeep) sfbreg.depth = wdata.data;
  if (ctl.sig.ldPixelShift) sfbreg.shift = wdata.data & 0xf;

  if (ctl.sig.ldPixelMask) sfbreg.pixelMask =
    (ctl.sig.setMask ? 0xffffffff : wdata.data);

  if (ctl.sig.ldPixelMask) next_lineLength =
    (ctl.sig.setMask ? 0 : wdata.bres3.lineLength);

  if (ctl.sig.ldPixelMask) next_iterCount = 0;

  if (ctl.sig.ldBres1) sfbreg.bres1 = wdata.bres1;;
  if (ctl.sig.ldBres2) sfbreg.bres2 = wdata.bres2;
  if (ctl.sig.ldFore) sfbreg.foreground.data = wdata.data;
  if (ctl.sig.ldBack) sfbreg.background.data = wdata.data;
  if (ctl.sig.ldCopyBuffHi) cpuData.hi = wdata.data;
  if (ctl.sig.ldCopyBuffLo) cpuData.lo = wdata.data;
  if (ctl.sig.ldAddrReg) addrReg = wdata.data;

  /*
   * sfb data path
   */
  bitMask = NibbleMask (sfbreg.pixelMask, sfbreg.depth);

  pixptr = NextNibble (bitMask);
  /*!!!! I would like to get rid of done (like Lindsay does) !!!!*/
  done = (sfbreg.mode == OPAQUELINE || sfbreg.mode == TRANSPARENTLINE
	  ? lineDone : (pixptr&8) != 0);
  lastOne = (sfbreg.mode == OPAQUELINE || sfbreg.mode == TRANSPARENTLINE
	  ? lineLength==1 : (pixptr&16) != 0);
  pixptr &= 07;

  stippleBits = (sfbreg.depth == DEPTH32 ? 0x03 & (dataIn >> (pixptr*2))
		 : sfbreg.depth == DEPTH16 ? 0x0f & (dataIn >> (pixptr*4))
		 : 0xff & (dataIn >> (pixptr*8)));
  lineBits = (0x1 & (dataIn >> iterCount) ? 0xff : 0);

  stipple = (sfbreg.mode == OPAQUELINE || sfbreg.mode == TRANSPARENTLINE
	     ? lineBits
	     : stippleBits);

  stippleData = StippleData (stipple);

  if (ctl.sig.ldCopyBuffHi) {
    copyData = Buffer (cpuData, ctl.sig.flush, ctl.sig.shift);
  } else {
    copyData = Buffer (dstData, ctl.sig.flush, ctl.sig.shift);
  }

  dataIn64.lo = dataIn64.hi = dataIn;

  srcData = (sfbreg.mode == COPY ? copyData :
	     sfbreg.mode == SIMPLE ? dataIn64 : stippleData);

  /*
   * address path.
   */
  errorSign = (bresError < 0);

  lineInc = (errorSign ? sfbreg.bres1.a1 : sfbreg.bres2.a2);

  pixInc = (pixptr << 3);
  if (sfbreg.mode == COPY && (sfbreg.shift & 8))
    pixInc = ~pixInc;

  addrInc = (ctl.sig.stepBres ? lineInc : pixInc);

  if (sfbreg.mode == OPAQUELINE || sfbreg.mode == TRANSPARENTLINE)
    addrIn |= wdata.bstart.addrLo;

  curAddr = (ctl.sig.selAddr ? sfbAddr
	  : (ctl.sig.useAddrReg ? addrReg : addrIn));

  nxtAddr = (curAddr + addrInc +
	     (sfbreg.mode == COPY
	      && (sfbreg.shift & 8) != 0));

  /*
   * Bresenham error term.
   */
  errorInc = (errorSign
	      ? sfbreg.bres1.e1
	      : ~sfbreg.bres2.e2);
  nxtError = bresError + errorInc + !errorSign;

  if (ctl.sig.stepBres == 0)
    curError = wdata.bres3.e;
  else
    curError = nxtError;


  addrOut = (sfbreg.mode == OPAQUELINE || sfbreg.mode == TRANSPARENTLINE
	     ? curAddr : nxtAddr) & 0x7fffff;

  adrLo3 = 0x07 &
    (sfbreg.depth == DEPTH8 ? addrOut
     : sfbreg.depth == DEPTH16 ? (addrOut>>1)
     : (addrOut>>2));

  addrOut >>= (sfbreg.depth == DEPTH8 ? 3 :
	       sfbreg.depth == DEPTH16 ? 4 : 5);

  /*
   * bytemask.
   */
  switch (sfbreg.mode) {

  case OPAQUELINE:
    bytemask = 1 << adrLo3;
    break;

  case TRANSPARENTLINE:
    bytemask = ((dataIn >> iterCount) & 1
		? 1 << adrLo3
		: 0);
    break;

  case COPY:
    if (!ctl.sig.readRq)
      bytemask =
	(sfbreg.depth == DEPTH8 ? 0xff & (sfbreg.pixelMask >> (8*pixptr)) :
	 (sfbreg.depth == DEPTH16 ?
	  (0x0f & (sfbreg.pixelMask >> (4*pixptr))) << (adrLo3&4) :
	  (0x03 & (sfbreg.pixelMask >> (2*pixptr))) << (adrLo3&6)));
    else
      bytemask =
	(sfbreg.depth == DEPTH8 ? 0xff & (0xffffffff >> (8*pixptr)) :
	 (sfbreg.depth == DEPTH16 ?
	  (0x0f & (0xffffffff >> (4*pixptr))) << (adrLo3&4) :
	  (0x03 & (0xffffffff >> (2*pixptr))) << (adrLo3&6)));
    break;

  case OPAQUESTIPPLE:
  case TRANSPARENTSTIPPLE:
    bytemask =
      (sfbreg.depth == DEPTH8 ? 0xff & (sfbreg.pixelMask >> (8*pixptr)) :
       (sfbreg.depth == DEPTH16 ?
	(0x0f & (sfbreg.pixelMask >> (4*pixptr))) << (adrLo3&4) :
       (0x03 & (sfbreg.pixelMask >> (2*pixptr))) << (adrLo3&6)));
    break;

  default: /* SIMPLE */
    bytemask = 
      (sfbreg.depth == DEPTH8 ? bmIn << (adrLo3&4) :
       sfbreg.depth == DEPTH16 ? (((bmIn&4)>>1) | (bmIn&1)) << (adrLo3&6) :
       1 << (adrLo3&7));
  }

  /*
   * input register devices.
   */
  if (ctl.sig.next || !ctl.sig.busy) {
    next_nextBmask =
      (ctl.sig.busy ? (nextBmask & ~bmaskMask) : 0xff);
    next_lineLength = (ctl.sig.rq ? lineLength - 1 : lineLength) & 0xf;
    next_lineDone = (ctl.sig.busy && next_lineLength == 0);
  }
  if (ctl.sig.tcreq && !ctl.sig.busy) {
    next_addrIn = tcAddr;
    next_writeIn = tcWrite;
    next_dataIn = tcData;
    next_bmIn = tcMask;
/*
    fprintf (stderr, "wrote   %08x to %08x\n",
	       tcData, tcAddr);
*//*
      fprintf (stderr, "wrote   %08x to %08x (%08x %08x %08x)\n",
	       tcData, tcAddr, buffD, hldAD, tcAD);
*/
  }

  /*
   * output register devices.
   */
  if (ctl.sig.ldBresErr) next_bresError = curError;

  if (ctl.sig.rq)
    next_sfbAddr = nxtAddr;
  if (ctl.sig.ldAddrReg)
    next_sfbAddr = wdata.data;

  if (ctl.sig.rq) {
    next_iterCount = iterCount + 1;
    next_memaddr = addrOut;
    next_membm = bytemask;
    next_memdata = srcData;
  }
  actl.sig.rowMatch = ((memaddr ^ addrOut) & ROWMASK) == 0;
  /*
   * read data return.
   */
  if (ctl.sig.rdCopyBuff) {
    if (addrIn & 4)
      return (copyData.hi);
    else
      return (copyData.lo);
  } else
    return ((unsigned) 0);
}


struct {
  int bufnum;
  int bytnum;
} shifter[8][8] = {
  0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7,
  1, 7, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6,
  1, 6, 1, 7, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5,
  1, 5, 1, 6, 1, 7, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4,
  1, 4, 1, 5, 1, 6, 1, 7, 0, 0, 0, 1, 0, 2, 0, 3,
  1, 3, 1, 4, 1, 5, 1, 6, 1, 7, 0, 0, 0, 1, 0, 2,
  1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 7, 0, 0, 0, 1,
  1, 1, 1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 7, 0, 0,
  };

/*
 * this routine models the buffer used in COPYMODE.
 * 
 * the buffer is comprised of two incoming registers
 * connected to an alignment shifter which feeds a FIFO
 * made from 7 latches w/ tristate buffers (RAM1 cells).
 */
ULONG64
Buffer (data, flush, shift)
ULONG64 data;
unsigned flush, shift;
{
  static ULONG64 fifo[4];
  register i, j, k;
  ULONG64 d;

  BYTES align;

  /*
   * storage register and residue register
   * in front of alignment shifter.
   */
  next_wrnext = 0;

  if (shift) {
    if (sfbreg.shift & 0x8) {
      next_buf[1].l = data;
      next_buf[0].l = buf[1].l;
    } else {
      next_buf[1].l = buf[0].l;
      next_buf[0].l = data;
    }
    next_wrnext = 1;
  }
  if (wrnext)
    next_wrptr <<= 1;

  /*
   * alignment shifter.
   * an 8 to 1 mux.
   */
  for (i=0; i<8; ++i) {
    j = shifter[sfbreg.shift & 7][i].bufnum;
    k = shifter[sfbreg.shift & 7][i].bytnum;
    align.b[i] = buf[j].b[k];
  }

  if (flush)
    next_wrptr = 0x0f;

  for (i=0; i<4; ++i)
    if (wrptr & (1<<i)) fifo[i] = align.l;

  if (ctl.sig.rdCopyBuff)
    d = fifo [(addrIn >> 3) & 3];
  else
    d = fifo [pixptr & 3];
  return (d);
}

/*************************************************************/

ULONG64
StippleData (fgbgsel)
unsigned fgbgsel;
{
  register i;
  BYTES fg, bg, d;

  for (i=0; i<8; ++i) {
    fg.b[i] = sfbreg.foreground.byte[i%4];
    bg.b[i] = sfbreg.background.byte[i%4];
  }
      
  switch (sfbreg.depth) {
  case DEPTH8:
    d.b[0] = (fgbgsel & 0x1  ? fg.b[0] : bg.b[0]);
    d.b[1] = (fgbgsel & 0x2  ? fg.b[1] : bg.b[1]);
    d.b[2] = (fgbgsel & 0x4  ? fg.b[2] : bg.b[2]);
    d.b[3] = (fgbgsel & 0x8  ? fg.b[3] : bg.b[3]);
    d.b[4] = (fgbgsel & 0x10 ? fg.b[0] : bg.b[0]);
    d.b[5] = (fgbgsel & 0x20 ? fg.b[1] : bg.b[1]);
    d.b[6] = (fgbgsel & 0x40 ? fg.b[2] : bg.b[2]);
    d.b[7] = (fgbgsel & 0x80 ? fg.b[3] : bg.b[3]);
    break;

  case DEPTH16:
    d.p16[0] = (fgbgsel & 0x1  ? fg.p16[0] : bg.p16[0]);
    d.p16[1] = (fgbgsel & 0x2  ? fg.p16[1] : bg.p16[1]);
    d.p16[2] = (fgbgsel & 0x4  ? fg.p16[2] : bg.p16[2]);
    d.p16[3] = (fgbgsel & 0x8  ? fg.p16[3] : bg.p16[3]);
    break;

  case DEPTH32:
    d.p32[0] = (fgbgsel & 0x1  ? fg.p32[0] : bg.p32[0]);
    d.p32[1] = (fgbgsel & 0x2  ? fg.p32[1] : bg.p32[1]);
    break;
  }
  
  return (d.l);
}	
	
/*************************************************************/

/*
 * find the number of the next nonzero bit in bitmask.
 * if first is true, returns the number of the first nonzero bit.
 *
 * the returned bit number will be in the range 0 to 7 if a
 * nonzero bit was found, else 8 is returned.
 */
NextNibble (bitmask)
unsigned bitmask;
{	
  unsigned inputBmask, bitptr;
  int zeroSet, oneSet, twoSet, threeSet, fourSet, fiveSet, sixSet, sevenSet;

  inputBmask = (bitmask & nextBmask);

  zeroSet  = (inputBmask & 0x01) == 0x01;
  oneSet   = (inputBmask & 0x03) == 0x02;
  twoSet   = (inputBmask & 0x07) == 0x04;
  threeSet = (inputBmask & 0x0f) == 0x08;
  fourSet  = (inputBmask & 0x1f) == 0x10;
  fiveSet  = (inputBmask & 0x3f) == 0x20;
  sixSet   = (inputBmask & 0x7f) == 0x40;
  sevenSet = (inputBmask & 0xff) == 0x80;

  bmaskMask = ((sevenSet << 7) |
	       (sixSet   << 6) |
	       (fiveSet  << 5) |
	       (fourSet  << 4) |
	       (threeSet << 3) |
	       (twoSet   << 2) |
	       (oneSet   << 1) |
	       (zeroSet  << 0));

  bitptr = (((sevenSet || sixSet || fiveSet || fourSet) << 2) |
	    ((sevenSet || sixSet || threeSet || twoSet) << 1) |
	    ((sevenSet || fiveSet || threeSet || oneSet)));

  if (zeroSet + oneSet + twoSet + threeSet + 
      fourSet + fiveSet + sixSet + sevenSet == 0) bitptr |= 8;

  if (zeroSet  && (inputBmask & ~0x01) == 0
      || oneSet   && (inputBmask & ~0x03) == 0
      || twoSet   && (inputBmask & ~0x07) == 0
      || threeSet && (inputBmask & ~0x0f) == 0
      || fourSet  && (inputBmask & ~0x1f) == 0
      || fiveSet  && (inputBmask & ~0x3f) == 0
      || sixSet   && (inputBmask & ~0x7f) == 0
      || sevenSet && (inputBmask & ~0xff) == 0)
    bitptr |= 16;

  return (bitptr);
}

/*************************************************************/

/*
 * return a 4 bit mask where each bit of the mask reflects the
 * zero/nonzero state of the corresponding byte in "bits".
 *
 * h/w implementation is:
 *	4 8-input OR's. (8 plane)
 *	8 4-input OR's. (16 plane)
 *	8 2-input OR's. (32 plane)
 */
NibbleMask (bits, depth)
unsigned bits;
SFBDepth depth;
{	
  int i, bm;
  int mask = 0;

  if (depth == DEPTH8) {
    for (i=3; i>=0; --i) {
      mask <<= 1;
      bm = 0xff << (i*8);
      mask |= ((bits & bm) != 0);
    }
  } else if (depth == DEPTH16) {
    for (i=7; i>=0; --i) {
      mask <<= 1;
      bm = 0xf << (i*4);
      mask |= ((bits & bm) != 0);
    }
  } else if (depth == DEPTH32) {
    for (i=7; i>=0; --i) {
      mask <<= 1;
      bm = 0x3 << (i*2);
      mask |= ((bits & bm) != 0);
    }
  } else {
    fprintf (stderr, "PANIC! unexpected depth value (%d) in NibbleMask\n",
	     depth);
    exit (1);
  }

  return (mask);
}

/**************************************************************/

/*
 *
 * given the boolean function, return the correct answer
 * the hardware actually looks like:
 *
 *	op.3	-----|-\
 *	op.2	-----|  \
 *	op.1	-----|  /
 *	op.0	-----|-/
 *		     ||
 *		    s d
 *
 */

ULONG64
BoolOp (src, dst)
ULONG64 src, dst;
{
  unsigned opbit3;
  unsigned opbit2;
  unsigned opbit1;
  unsigned opbit0;
  ULONG64 res;

  /*
   * separate out the bool operation bits
   */
  opbit0 = (sfbreg.rop & 0x08 ? 0xffffffff : 0);
  opbit1 = (sfbreg.rop & 0x04 ? 0xffffffff : 0);
  opbit2 = (sfbreg.rop & 0x02 ? 0xffffffff : 0);
  opbit3 = (sfbreg.rop & 0x01 ? 0xffffffff : 0);
  
  /*
   * perform the muxing action
   */
  res.lo = (~src.lo & ~dst.lo & opbit0)
      | (~src.lo & dst.lo & opbit1)
      | (src.lo & ~dst.lo & opbit2)
      | (src.lo & dst.lo & opbit3);

  res.hi = (~src.hi & ~dst.hi & opbit0)
      | (~src.hi & dst.hi & opbit1)
      | (src.hi & ~dst.hi & opbit2)
      | (src.hi & dst.hi & opbit3);
  
  return (res);
}
