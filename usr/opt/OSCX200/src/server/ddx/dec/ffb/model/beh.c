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
static char *rcsid = "@(#)$RCSfile: beh.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:21:37 $";
#endif
/*
 * HISTORY
 * $Log: beh.c,v $
 * Revision 1.1.2.2  1993/11/19  21:21:37  Robert_Lembree
 * 	SFB+ Initial version
 * 	[1993/11/19  16:33:08  Robert_Lembree]
 *
 * Revision 1.1.1.2  1993/11/19  16:33:08  Robert_Lembree
 * 	SFB+ Initial version
 *
 * Revision 2.76  1993/06/16  19:28:59  rsm
 * fixed bug in DmaWrite (extra set of parenthesis)
 *
 * Revision 2.75  1993/06/11  03:37:52  rsm
 * added missing parenthesis
 *
 * Revision 2.74  1993/06/11  02:09:12  rsm
 * changed my mind on how to fix the lockreg problem
 *
 * Revision 2.73  1993/06/11  01:54:07  rsm
 * removed fg from Reduce and random noise from Dither
 * added in reset terms here and there
 * changed the lockReg that I send Chris so that it doesn't
 * assert in copy modes (copy, dmard, dmawr)
 *
 * Revision 2.72  1993/06/10  17:05:01  chris
 * invert fg and bg into MakeStipple
 *
 * Revision 2.71  1993/06/09  23:35:48  rsm
 * removed req0 & mode.dmaRd term from wrMemData
 * added lockReg1 output for Chris (without req0 term)
 * removed done from logic for i_idle
 *
 * Revision 2.70  1993/06/09  18:13:42  rsm
 * got rid of mode.big; it's cheaper and easier to generate my own
 * version than to have Chris generate it and then pipe it along
 *
 * Revision 2.69  1993/06/05  20:44:20  rsm
 * inhibit readFlag if we're writing block color or planemask registers.
 *
 * Revision 2.68  1993/06/04  15:58:40  rsm
 * write planemask 4 times in 8-bit packed mode
 *
 * Revision 2.67  1993/06/01  22:33:05  rsm
 * fixed byte mask bug in 8bit unpacked mode writing color registers
 *
 * Revision 2.66  1993/05/27  19:24:35  chris
 * changed 12/24 plane sequential interpolation of the high byte
 *
 * Revision 2.65  1993/05/25  19:06:03  rsm
 * added a missing break in Dither()
 *
 * Revision 2.64  1993/05/25  00:51:43  rsm
 * change to use address directly out of pipe stage 1 to determine block
 * write style for a particular bank
 *
 * Revision 2.63  1993/05/24  19:37:19  pcharles
 * additional mode decoding for better fault coverage,
 * dither visibility
 *
 * Revision 2.62  1993/05/21  16:08:34  rsm
 * make req1 assert at reset to get flops init'ed
 * change color.seladdr logic to allow faster implementation
 * connect byteMask.z16Sel to mode.z16
 *
 * Revision 2.61  1993/05/20  20:30:36  rsm
 * fixed bug in new logic for last and done.
 * added reset into AddrCtl.
 *
 * Revision 2.60  1993/05/19  15:29:36  rsm
 * changed MoreThan2 and FindFirstSet to take low asserted input
 *
 * Revision 2.59  1993/05/18  18:44:19  rsm
 * removed "done" term from i_finished
 *
 * Revision 2.58  1993/05/18  15:59:20  pcharles
 * changed Dither() to do low-true color i/o
 *
 * Revision 2.57  1993/05/18  15:35:39  rsm
 * logic changes: only req1 depends on done now; other terms use last
 * added code to allow dma mismatches to analyze traces made on the broken model
 * made changes so AddrCtl could be converted for Synopsys with a simple ed script
 *
 * Revision 2.56  1993/05/17  18:30:38  rsm
 * zbuffering bug fix
 * added MoreThan2 routine
 * added low-true input to BuildOpMask
 *
 * Revision 2.55  1993/05/13  22:45:24  pcharles
 * proper address increment for dma dithered reads
 * corrected selDataMux.notData32 to include dma and dma dithered term
 * (.notLine)
 *
 * Revision 2.54  1993/05/12  16:16:20  chris
 * Do not assert lockReg1 when in dmaRead modes
 *
 * Revision 2.53  1993/05/11  19:59:04  rsm
 * changed dmaBase type from B32 to int
 *
 * Revision 2.52  1993/05/11  17:26:14  pcharles
 * fixed default error case output in Dither()
 *
 * Revision 2.51  1993/05/11  17:19:45  pcharles
 * cleaned up Reduce()
 * #ifdef'd out some dma debugging info.
 * added _notLine terms for dma dithering
 *
 * Revision 2.50  1993/05/11  14:55:31  rsm
 * changed dmaMask from B2x4 to B8 in PrintDmaWrite
 *
 * Revision 2.49  1993/05/11  14:04:46  rsm
 * dma changes (using dmaBase for dma addresses instead of zbase)
 * move behavioral RAM to BehRAM.c
 * don't generate flush control during dma writes
 *
 * Revision 2.48  1993/05/06  20:28:19  rsm
 * bug fix for 8-bit unpacked visuals during dma reads
 *
 * Revision 2.47  1993/05/05  19:50:04  rsm
 * 64-bit fix for DMA on Alpha OSF
 *
 * Revision 2.46  1993/05/05  17:17:44  rsm
 * added optimization for sparse block stipples with 32-bit visuals
 * fixed bug which was screwing up addresses for z operations
 *
 * Revision 2.45  1993/05/04  14:12:30  rsm
 * bug fix for dma write in 32-plane mode
 * bug fix for memCmd.{,un}packed8bit during color reg/planemask writes
 *
 * Revision 2.44  1993/04/30  14:20:36  rsm
 * bug fix for writing color regs/planemask in dmawrite mode
 * set memCmd.line field
 *
 * Revision 2.43  1993/04/28  21:50:14  rsm
 * fixed control logic to use dmaStatus.first instead of cmd.newAddr
 * in dma modes
 *
 * Revision 2.42  1993/04/26  20:48:17  rsm
 * fixed bug with unaligned stipples to 8-bit unpacked visuals
 *
 * Revision 2.41  1993/04/22  17:26:59  rsm
 * fixed bug generating bytemasks for z-read/write on 32-plane systems
 *
 * Revision 2.40  1993/04/21  14:14:22  rsm
 * dma changes to permit software model to use 64-bit addressing on Alpha/OSF
 * bug fixes to ctl->addr.visual32 relating to 8-bit unpacked visuals
 *
 * Revision 2.39  1993/04/16  01:58:47  chris
 * depth/rotate visual changes
 *
 */
#include <stdio.h>
#include "lyreTypes.h"
#include "parts_c.h"
#include "types.h"
#include "vars.h"
#include "dithermatrix.h"

#ifndef PRESTO
#include "BehAdder_parts.c"
BEHAVIORAL BehAdder (int in1, int in2, signal cin, int *out)
{
  *out = in1 + in2 + (cin ? 1 : 0);
}

#include "BehAdder16_parts.c"
BEHAVIORAL BehAdder16 (int in1, int in2, signal cin, int *out, signal *cout)
{
  int sum;

  sum = (in1 & 0xffff) + (in2 & 0xffff) + (cin ? 1 : 0);

  *out = sum & 0xffff;
  *cout = (sum & 0x10000 ? -1 : 0);
}
#endif

#include "BehAdder36_parts.c"
BEHAVIORAL BehAdder36 (B36 in1,
		       B36 in2,
		       B36 *out)
{
  register int lo;
  int a1, a2, s, carry;

  lo = in1.lo + in2.lo;
  a1 = in1.lo >> 31;
  a2 = in2.lo >> 31;
  s = lo >> 31;
  carry = a1 & a2 | (a1 ^ a2) & ~s;
  out->lo = lo;
  out->hi = (in1.hi + in2.hi + (carry ? 1 : 0)) & 0xf;
}

#include "MoreThan1_parts.c"
BEHAVIORAL MoreThan1 (Bits16 in,
		      signal *mt1)
{
  int i, ones;

  in &= 0xffff;

  ones = 0;
  for (i=0; i<16; ++i)
    if (in & (1 << i))
      if (ones) {
	*mt1 = -1;
	return;
      } else
	++ones;

  *mt1 = 0;
}

#include "FindWork_parts.c"
BEHAVIORAL FindWork (Bits16 in_L,
		     Bits4 *out,
		     signal *gs,
		     signal *mt2)
{
  int i, ones;
  int p3, p2, p1, p0;
  Bits16 in = ~in_L;

  in &= 0xffff;

  ones = 0;
  for (i=0; i<16; ++i)
    if (in & (1 << i))
      ++ones;

  *mt2 = (ones > 2 ? -1 : 0);

  p3 = 0; p2 = 0; p1 = 0; p0 = 0;

  *gs = (in ? -1 : 0);

  if (in != 0) {
    if ((in & 0xff) == 0) {
      p3 = 8;
      in >>= 8;
    }
    if ((in & 0xf) == 0) {
      p2 = 4;
      in >>= 4;
    }
    if ((in & 0x3) == 0) {
      p1 = 2;
      in >>= 2;
    }
    if ((in & 0x1) == 0) {
      p0 = 1;
    }
  }

  *out = p3 | p2 | p1 | p0;
}


#include "MakeMask_parts.c"
BEHAVIORAL MakeMask (B4 in,
		     Signal init,
		     B16 *out)
{
  in &= 0xf;

  if (init)
    *out = -1;
  else
    *out = (-1) << (in+1);
}

int blockMask[4] = {
  0x03030303,
  0x0c0c0c0c,
  0x30303030,
  0xc0c0c0c0
  };

#include "BuildOpMask_parts.c"
BEHAVIORAL BuildOpMask (B32 in_L,
			CtlBuildOpMaskType ctl,
			Bits16 *out,
			signal *fastFill)
{
  B32 in = ~in_L;
  int i, bm;
  int mask = 0;
  int nibMask = 0;

  *fastFill = 0;

  if (ctl.blockMode) {	/* block mode: either block stipple or block fill */
    if (ctl.visual32) { 
      /*
       * 32 plane, block mode
       */
      if (in == -1) {
	/* fill all 32 pixels; do it in one operation */
	*fastFill = -1;
	mask = 1;
      } else 
	/* fill fewer than 32 pixels; potentially requires 4 operations */
	for (i=3; i>=0; --i) {
	  mask <<= 1;
	  bm = blockMask[i];
	  mask |= ((in & bm) != 0);
	}
    } else {
      /*
       * 8 plane, block mode
       */
      /* optimize out the all 0's case */
      mask = (in != 0);
    }
  } else {
    if (ctl.visual32) { 
      /*
       * 32 plane, non-block mode
       */
      for (i=15; i>=0; --i) {
	mask <<= 1;
	bm = 0x3 << (i*2);
	mask |= ((in & bm) != 0);
      }
    } else {
      /*
       * 8 plane, non-block mode
       */
      for (i=7; i>=0; --i) {
	nibMask <<= 1;
	bm = 0xf << (i*4);
	nibMask |= ((in & bm) != 0);
      }
      if (ctl.unaligned)
	nibMask <<= 1;
      
      for (i=4; i>=0; --i) {
	mask <<= 1;
	bm = 3 << (i*2);
	mask |= ((nibMask & bm) != 0);
      }
    }
  }

  *out = mask;
}

#include "MakeStipple_parts.c"
BEHAVIORAL MakeStipple (BlkStyleType blkStyle,
			CtlMakeStippleType ctl,
			B32 dataIn,
			Bits4 iter,
			B32 bg_L,
			B32 fg_L,

			signal *zenb,
			OUTPUT B64 stipplePattern,
			OUTPUT B64 blockStipple)
{
  unsigned stippleBits, lineBits, stipple;
  ULONG64 stippleData, StippleData();

  iter &= 0xf;

  lineBits = (0x1 & (dataIn >> iter) ? 0xff : 0);

/*
  fprintf(stderr, "dataIn = %-.8x, iter = %-.1x, lineBits = %-.2x\n",
	  dataIn, iter, lineBits & 0xff);
*/

  if (ctl.unaligned)
    dataIn = (((unsigned) dataIn & 0xf0000000) >> 28) | (dataIn << 4);

  stippleBits = (ctl.visual32
		 ? 0x03 & (dataIn >> (iter*2))
		 : 0xff & (dataIn >> ((iter%4)*8)));

  stipple = (ctl.lineMode ? lineBits : stippleBits);

  stippleData = StippleData (stipple, ctl.visual32, ~fg_L, ~bg_L);

  if (ctl.transparent) {
    /* transparency bit turned on */
    *zenb = (lineBits ? -1 : 0);
  } else {
    /* opaque - write something no matter what */
    *zenb = -1;
  }

  {
    /* create block stipple or block fill data pattern */
    int i, j, bit;
    PIXELS d, m;

    d.l.lo = 0;
    d.l.hi = 0;

    if (ctl.visual32) {
      /* 32 plane block mode */
      for (i=0; i<8; ++i) {	/* bit number */
	for (j=0; j<8; ++j) {	/* byte number */
	  bit = (i%4)*2 + j/4;
	  bit = iter*2 + (bit >> 1)*8 + (bit & 1);
	  if (dataIn & (1 << bit)) {
	    d.p8[j] |= (1 << i);
	  }
	}
      }
    } else {
      /* 8 plane block mode */
      for (i=0; i<8; ++i) {	/* bit number */
	for (j=0; j<8; ++j) {	/* byte number */
	  if (dataIn & (1 << (j+(i%4)*8))) {
	    d.p8[j] |= (1 << i);
	  }
	}
      }
    }

    m.l.lo = (blkStyle.maskHighNibble ? 0 : 0xf0f0f0f0);
    m.l.hi = (blkStyle.maskHighNibble ? 0 : 0xf0f0f0f0);

    m.l.lo |= (blkStyle.maskLowNibble ? 0 : 0x0f0f0f0f);
    m.l.hi |= (blkStyle.maskLowNibble ? 0 : 0x0f0f0f0f);

    blockStipple[0] = d.l.lo & m.l.lo;
    blockStipple[1] = d.l.hi & m.l.hi;
  }

  stipplePattern[0] = stippleData.lo;
  stipplePattern[1] = stippleData.hi;
#if 0
  printf ("MakeStipple: data = %-.8x %-.8x\n", stippleData.lo, stippleData.hi);
#endif
}

ULONG64
StippleData (unsigned fgbgsel, int visual32, int fore, int back)
{
  register i;
  BYTES fg, bg, d;
  COLORS foreground, background;

  foreground.data = fore;
  background.data = back;

  for (i=0; i<8; ++i) {
    fg.b[i] = foreground.byte[i%4];
    bg.b[i] = background.byte[i%4];
  }
      
  if (visual32) {
    d.p32[0] = (fgbgsel & 0x1  ? fg.p32[0] : bg.p32[0]);
    d.p32[1] = (fgbgsel & 0x2  ? fg.p32[1] : bg.p32[1]);
  } else {
    d.b[0] = (fgbgsel & 0x1  ? fg.b[0] : bg.b[0]);
    d.b[1] = (fgbgsel & 0x2  ? fg.b[1] : bg.b[1]);
    d.b[2] = (fgbgsel & 0x4  ? fg.b[2] : bg.b[2]);
    d.b[3] = (fgbgsel & 0x8  ? fg.b[3] : bg.b[3]);
    d.b[4] = (fgbgsel & 0x10 ? fg.b[0] : bg.b[0]);
    d.b[5] = (fgbgsel & 0x20 ? fg.b[1] : bg.b[1]);
    d.b[6] = (fgbgsel & 0x40 ? fg.b[2] : bg.b[2]);
    d.b[7] = (fgbgsel & 0x80 ? fg.b[3] : bg.b[3]);
  }
  
  return (d.l);
}	

#ifndef PRESTO	
#include "BehIncr_parts.c"
BEHAVIORAL BehIncr (int in, int *out)
{
  *out = in + 1;
}

#include "BehDecr_parts.c"
BEHAVIORAL BehDecr (int in, int *out)
{
  *out = in - 1;
}
#endif

#include "WordToBytes_parts.c"
BEHAVIORAL WordToBytes (B32 in,
			B8 *out0,
			B8 *out1,
			B8 *out2,
			B8 *out3)
{
  PIXELS32 d;

  d.p32 = in;

  *out0 = d.p8[0];
  *out1 = d.p8[1];
  *out2 = d.p8[2];
  *out3 = d.p8[3];
}


#include "BytesToWord_parts.c"
BEHAVIORAL BytesToWord (B8 in0,
			B8 in1,
			B8 in2,
			B8 in3,
			B32 *out)
{
  PIXELS32 d;

  d.p8[0] = in0;
  d.p8[1] = in1;
  d.p8[2] = in2;
  d.p8[3] = in3;

  *out = d.p32;
}


#include "BusyLogic_parts.c"
BEHAVIORAL BusyLogic (signal first,
		      signal pend1,
		      signal pend0,
		      signal zrd,
		      signal zop,
		      signal memBusy,
		      signal memIdle,
		      signal zSafe,
		      signal copySafe,
		      signal last,
		      signal done,
		      ModeType mode,
		      CmdType cmd,
		      DmaStatusType dmaStatus,
		      signal readFlag1,

		      signal *i_busy0,
		      signal *i_idle)
{
  int req2, req1, i_next, i_finished, i_writeReg, i_pend1;
  int memStall, copyStall, zStall;

  memStall  = memBusy;
  copyStall = mode.copy & ~readFlag1 & ~copySafe & ~memIdle;
  zStall    = zop & ~zrd & ~zSafe;	/* memIdle shouldn't be necessary here */

  i_writeReg	= cmd.color | cmd.planeMask;

  i_next = pend0 & (~(memStall | copyStall) | ~pend1) & ~zStall;

  req2 = pend1 & ~(memStall | copyStall);
  req1 = i_next;

  i_finished	= (last | mode.dmaWr | mode.dmaRd & (~dmaStatus.last | ~first)) & i_next;
  
  i_pend1	= req1 | pend1 & ~req2;

  /*
   * i_busy0 has the same logic as i_pend0 except that req0
   * is left out to prevent a logic loop.
   */
  *i_busy0 = pend0 & ~i_finished
           | /*~i_writeReg & */zop;

  *i_idle       = memIdle
                & ~(req2
		    | i_pend1
		    | *i_busy0);
}

/*
 * copy buffer register read:
 *   command parser/bus interface performs read iff address generator is idle.
 *
 * copy buffer register write:
 *   command parser/bus interface performs write iff address generator is idle.
 *
 * video memory reads -> copy buffer writes
 *   address generator puts out addresses for memory reads as fast as possible
 *   (when not stalled by memBusy).
 *
 * copy buffer reads -> video memory writes
 *   address generator puts out addresses for memory writes.
 *   mask is provided by stipple mask logic.
 *   64-bit data is muxed into dataOut flop from copy buffer
 *   indexed by iter.
 * 
 * destData - provided by memory interface 
 * 
 * loadHiBuff - generated by bus interface/command parser
 * 
 * rdCopyBuff - generated by bus interface/command parser
 * 
 * wrCopyBuff = loadHiBuff + dataReady * copyMode
 * 
 * flush - maintained by bus interface/command parser.
 *
 *	= copyMode * readFlag * write * (START + CONTINUE + FB)
 * 	+ write * CPYBF0ADDRS
 *
 * readFlag - maintained by bus interface/command parser
 *            and piped along in address/pixel generator.
 * 
 * toggles with each frame buffer request (only in copyMode, of course).
 * set by writes to pixShift register, reads of copy buffer registers.
 * reset by writes to copy buffer register #0.
 * 
 */

#include "PrintReq_parts.c"
BEHAVIORAL PrintReq (B24 addr,
		     B32 data,
		     SfbRegType sfbReg,
		     CmdType cmd,
		     Signal  reset,
		     Signal  req0dly)
{
  if (req0dly) {
#if 0
    printf( "request:\n" );
    printf( "    addr = %-.8x, data = %-.8x, mask = %-.8x\n",
	    addr, data, sfbReg.tcMask );
    printf( "    pixelMask = %-.8x, planeMask = %-.8x, reset = %-.8x\n",
	    sfbReg.pixelMask, sfbReg.planeMask, reset );
    printf( "    line=%d, copy=%d, stipple=%d, trans=%d, mode=%d\n",
	    sfbReg.mode.line, sfbReg.mode.copy,
	    sfbReg.mode.stipple, sfbReg.mode.mode & 4,
	    sfbReg.mode.mode );
    printf( "    len=%d, len=%d, e=%-.8x, e1=%-.8x, e2=%-.8x\n",
	    sfbReg.lineLength, sfbReg.bres.len, sfbReg.bres.e,
	    sfbReg.bres.e1, sfbReg.bres.e2 );
    printf( "    a1=%-.8x, a2=%-.8x\n",
	    sfbReg.bres.a1, sfbReg.bres.a2 );
    printf( "    readFlag0=%d, newLine=%d, newError=%d\n",
	   cmd.readFlag0&1, cmd.newLine&1, cmd.newError&1 );
    printf( "    copy64=%d, color=%d, planeMask=%d\n",
	   cmd.copy64&1, cmd.color&1, cmd.planeMask&1 );
    printf( "    ramdac=%d, rom=%d\n",
	   cmd.ramdac&1, cmd.rom&1 );
#endif
#if 0
    printf( "val.red = %-.8x, val.grn = %-.8x, val.blu = %-.8x\n",
	    reg.colorVal.red,
	    reg.colorVal.green,
	    reg.colorVal.blue );
    printf( "inc.red = %-.8x, inc.grn = %-.8x, inc.blu = %-.8x\n",
	    reg.colorInc.red,
	    reg.colorInc.green,
	    reg.colorInc.blue );
    printf( "row = %-.2x, col = %-.2x\n",
	    reg.dither.row,
	    reg.dither.col );
    printf( "dxGEdy = %d, dxGE0 = %d, dyGE0 = %d\n",
	    reg.dither.dxGEdy,
	    reg.dither.dxGE0,
	    reg.dither.dyGE0 );
    sfbReg.rop
    sfbReg.pixelShift
    sfbReg.fg
    sfbReg.bg
    sfbReg.depth
    reg.rasterOp
    reg.pixelShift
    reg.planeMask
    reg.pixelMask
    reg.deep.block
    reg.deep.mask
    reg.deep.deep
    reg.mode.cap
    reg.mode.rotate
    reg.mode.visual
    reg.mode.mode
    reg.mode.big
    reg.mode.simple
    reg.mode.stipple
    reg.mode.line
    reg.mode.copy
    reg.dmaBase
    reg.bres.len
    reg.bres.e
    reg.bres.e1
    reg.bres.e2
    reg.bres.a1
    reg.bres.a2
    reg.foreground
    reg.background
    reg.z.base
    reg.z.val0
    reg.z.val1
    reg.z.inc0
    reg.z.inc1
    reg.z.a1
    reg.z.a2
    reg.address
    reg.data
    reg.bWidth
    reg.zWidth
    reg.hor.odd
    reg.hor.back
    reg.hor.sync
    reg.hor.front
    reg.hor.active
    reg.ver.back
    reg.ver.sync
    reg.ver.front
    reg.ver.active
    reg.video.base
    reg.video.valid
    reg.video.blank
    reg.cursor.valid
    reg.cursor.x
    reg.cursor.y
    reg.cursor.rows
    reg.cursor.base
    reg.count.status
    reg.count.value
    reg.intEnable
#endif
  }

}

static int dmaMisMatch;

unsigned
DmaWrite(unsigned src, unsigned *addr, unsigned mask, unsigned *dmaStart)
{
  extern unsigned long bogusDmaHigh32;
  extern FILE *vramFile;
  unsigned *pdst = addr;
  unsigned wordMask;

#ifdef PCI
  wordMask = (mask & 1 ? 0x000000ff : 0)
           | (mask & 2 ? 0x0000ff00 : 0)
           | (mask & 4 ? 0x00ff0000 : 0)
           | (mask & 8 ? 0xff000000 : 0);
#endif

  if ((unsigned) addr & 3) {
    /* trap any unaligned addresses here */
    fprintf (stderr, "PANIC! unaligned address in DmaWrite (%lx)\n",
	     addr);
    exit (1);
  }

  /* we expect an int to be 32 bits. */
  if (sizeof(int) != 4) {
    fprintf (stderr, "PANIC! DmaWrite assumption violated\n");
    exit (1);
  }

  if (vramFile == NULL) {
    extern FILE *cmdFile;
    extern char dmaBuff[];
    unsigned d;
    
    if (cmdFile != NULL)
      pdst = (unsigned *) dmaBuff + (addr - dmaStart);

#ifdef PCI
    d = (src & wordMask | *pdst & ~wordMask);
#else
    /* TRUBOchannel has no byte mask on DMAWRITE operations */
    d = src;
#endif
/*    printf("DmaWrite: writing address %x, data %x\n", (int) pdst, d);*/
    *pdst = d;
  } else {
    extern int cmdLine, vramLine;
    char *GetDmaLine ();
    char *p = GetDmaLine ();
    struct {
      unsigned a;	/* dma address */
      unsigned d;	/* dma data */
      unsigned m;	/* dma byte mask */
    } dma;

    if (p == NULL) {
      fprintf (stderr, "unexpected end of vramFile\n");
      fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
      exit (1);
    } else if (p[0] != 'w') {
      fprintf (stderr, "expected dma write: '%s'\n", p);
      fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
      exit (1);
    } else if (sscanf(p, "w %x %x %x", &dma.a, &dma.d, &dma.m) != 3) {
      fprintf (stderr, "couldn't parse dma write: '%s'\n", p);
      fprintf (stderr, "**** cmdLine = %d, vramLine = %d\n",
	       cmdLine, vramLine);
      exit (1);
    } else if (dma.a != (int) addr && dma.d == src && dma.m == mask) {
      fprintf (stderr, "**** incorrect dmawrite: ");
      fprintf (stderr, "cmdLine = %d, vramLine = %d\n", cmdLine, vramLine);
      fprintf (stderr, "expected: a=%x, d=%x, m=%x\n", dma.a, dma.d, dma.m);
      fprintf (stderr, "received: a=%x, d=%x, m=%x\n", (int) addr, src, mask);
      if (++dmaMisMatch > 1000) {
	fprintf (stderr, "PANIC! too many mismatches\n");
	exit (1);
      }
    } else if (dma.a != (int) addr || dma.d != src || dma.m != mask) {
      fprintf (stderr, "**** incorrect dmawrite: ");
      fprintf (stderr, "cmdLine = %d, vramLine = %d\n", cmdLine, vramLine);
      fprintf (stderr, "expected: a=%x, d=%x, m=%x\n", dma.a, dma.d, dma.m);
      fprintf (stderr, "received: a=%x, d=%x, m=%x\n", (int) addr, src, mask);
      exit (1);
    }
  }
}


#include "PrintDmaWrite_parts.c"
BEHAVIORAL PrintDmaWrite (B2x32 dmaData,
			  B8 dmaMask,
			  signal first,
			  signal write,
			  B32 dataReg,
			  int dmaBase,
			  signal tickIn,
			  signal *tickOut)
{
  static int dataPhase = 0;
  static unsigned *dmaAddr;
  static unsigned *dmaStart;
  extern SFBREG sfbreg;
  extern FILE *vramOut;
  extern int logFlag;
  extern unsigned long bogusDmaHigh32;
  extern MODEREG   shadowMode;
  
  *tickOut = tickIn + 1;

  if (dataPhase) {
    if (dmaMask & 0xf) {
#if 0
      printf ("*** DMA write: addr = %lx, data = %-.8x, mask = %-.8x\n",
	      dmaAddr, dmaData[0], dmaMask & 0xf);
#endif
      DmaWrite(dmaData[0] & dataReg, dmaAddr, dmaMask & 0xf, dmaStart);

      if (logFlag)
	fprintf (vramOut, "w %x %x %x\n", (int) dmaAddr, dmaData[0] & dataReg, dmaMask & 0xf);

      ++dmaAddr;
    }
    if (dmaMask & 0xf0) {
#if 0
      printf ("*** DMA write: addr = %lx, data = %-.8x, mask = %-.8x\n",
	      dmaAddr, dmaData[1], dmaMask & 0xf0);
#endif
      DmaWrite(dmaData[1] & dataReg, dmaAddr, (dmaMask & 0xf0) >> 4, dmaStart);

      if (logFlag)
	fprintf (vramOut, "w %x %x %x\n", (int) dmaAddr, dmaData[1] & dataReg, (dmaMask & 0xf0) >> 4);

      ++dmaAddr;
    }
    dataPhase = 0;
  }

  if (write) {
    dataPhase = 1;
  }

  if (first) {
    dmaStart = dmaAddr = (unsigned *) ((unsigned long) dmaBase | bogusDmaHigh32);
    if ((int) dmaAddr & 3) {
      fprintf (stderr, "PANIC! unaligned address in PrintDmaWrite (%lx)\n",
	       dmaAddr);
      exit (1);
    }
  }
}

#ifndef PRESTO
#include "PrintXY_parts.c"
BEHAVIORAL PrintXY (int x,
		    int y,
		    signal stepBres,
		    color24 drom,
		    color24 dithered)
{
#if 0
  if (stepBres)
    printf ("(%d, %d) rom.red = %d, rom.grn = %d, rom.blu = %d, col.red = %d, col.grn = %d, col.blu = %d\n",
    x, y, drom.red8, drom.green8, drom.blue8,
    dithered.red8, dithered.green8, dithered.blue8);
#endif
}

#include "PrintError_parts.c"
BEHAVIORAL PrintError (int lastError,
		       signal stepBres,
		       signal selAddr,
		       signal saveCurrentVals,
		       signal selSavedVals)
{
#if 0
  static int startPrinting = 0;

  if (stepBres)
    startPrinting = 1;

  if (startPrinting) {
    if (selAddr || saveCurrentVals || selSavedVals)
      printf ("\n");

    printf ("lastError = %-.5x ", lastError);

    printf ("stepBres = %d ", stepBres);
    printf ("selAddr = %d ", selAddr);
    printf ("saveCurrentVals = %d ", saveCurrentVals);
    printf ("selSavedVals = %d ", selSavedVals);
    printf ("\n");
  }
#endif
}
#endif

#include "Drom_parts.c"
BEHAVIORAL Drom(B5 xx, B5 yy, color19 *matrix)
{
  xx &= 0x1f;
  yy &= 0x1f;

#if 1	/* new phase offset */
  matrix->red6   = DROM[yy][(xx+pR)%M] >> 1;
  matrix->green6 = DROM[yy][(xx+pG)%M] >> 1;
  matrix->blue7  = DROM[yy][(xx+pB)%M];
#else	/* old phase offset */
  matrix->red8   = DROM[(yy+pR)%N][xx] << 1;
  matrix->green8 = DROM[(yy+pG)%N][xx] << 1;
  matrix->blue8  = DROM[(yy+pB)%N][xx] << 1;
#endif
/*
  fprintf(stderr,"point (%x,%x); DROM[%x][%x] = %x\n",xx,yy,
	  yy,(xx+pB)%M,DROM[yy][(xx+pB)%M]);
*/
}

#include "StripHigh_parts.c"
BEHAVIORAL StripHigh(RGB TwentyFourBit,
		     color24 *EightBit,
		     color27 *NineBit)
{
/*
  fprintf(stderr, "r=%-.2x, g=%-.2x, b=%-.2x\n",
	  TwentyFourBit.red & 0xfffff,
	  TwentyFourBit.green & 0xfffff,
	  TwentyFourBit.blue & 0xfffff);
*/
#if 1	/* 20-bit color values */
  EightBit->red8   =   TwentyFourBit.red >> 12;
  EightBit->green8 = TwentyFourBit.green >> 12;
  EightBit->blue8  =  TwentyFourBit.blue >> 12;

  NineBit->red9    =   TwentyFourBit.red >> 11;
  NineBit->green9  = TwentyFourBit.green >> 11;
  NineBit->blue9   =  TwentyFourBit.blue >> 11;
#else	/* 24-bit color values */
  EightBit->red8   =   TwentyFourBit.red >> 16;
  EightBit->green8 = TwentyFourBit.green >> 16;
  EightBit->blue8  =  TwentyFourBit.blue >> 16;

  NineBit->red9    =   TwentyFourBit.red >> 15;
  NineBit->green9  = TwentyFourBit.green >> 15;
  NineBit->blue9   =  TwentyFourBit.blue >> 15;
#endif
}

#include "Reduce_parts.c"
BEHAVIORAL Reduce(color24 trueColor,
		  Bits7 modeReg,
		  Bits2 depth,
		  B32 *visual)
{
  Bits32 result;
  Bits32 regster;

  regster  = (unsigned)trueColor.red8        & 0xe0; /* 11100000 */
  regster |= (unsigned)trueColor.green8 >> 3 & 0x1c; /* 00011100 */
  regster |= (unsigned)trueColor.blue8  >> 6 & 0x03; /* 00000011 */

  if(modeReg & 0x40) /* sequential interpolation */
    regster  = trueColor.red8 & 0xff;

  switch(depth) {
  case VISUAL8P:  /* 8-bit visuals */
  case VISUAL8U:
    result   = regster | regster << 8 | regster << 16 | regster << 24;
    break;
  case VISUAL12:
    result  = (unsigned)trueColor.blue8 >>  4   & 0x0000000f;
    result |= (unsigned)trueColor.blue8         & 0x000000f0;
    result |= (unsigned)trueColor.green8<<  4   & 0x00000f00;
    result |= (unsigned)trueColor.green8<<  8   & 0x0000f000;
    result |= (unsigned)trueColor.red8  << 12   & 0x000f0000;
    result |= (unsigned)trueColor.red8  << 16   & 0x00f00000;
    result |= regster << 24;                  /*  0xff000000 */
    break;
  case VISUAL24: /* 24-bit visuals */
    result  = (unsigned)trueColor.red8   << 16  & 0x00ff0000;
    result |= (unsigned)trueColor.green8 <<  8  & 0x0000ff00;
    result |= (unsigned)trueColor.blue8         & 0x000000ff;
    result |= regster << 24;                 /*   0xff000000 */
    break; 
  default: 
    fprintf(stderr,"Reduce received illegal depth (%x)\n", depth);
    exit(1);
    break;
  }
*visual=result;
}

#include "Watch_parts.c"
BEHAVIORAL Watch(signal TestPoint)
{
  fprintf(stderr,"TestPoint = %x\n",TestPoint);
}

#include "Dither_parts.c"
BEHAVIORAL Dither(color19 matrixIn,
		  color27 ditherIn,
		  color24 *colorDither,
                  Bits2 modeReg)
{
unsigned redM, greenM, blueM;

  ditherIn.red9   = ~(unsigned)ditherIn.red9;    /* low-true color input     */
  ditherIn.green9 = ~(unsigned)ditherIn.green9;
  ditherIn.blue9  = ~(unsigned)ditherIn.blue9;

  ditherIn.red9   &= 0x1ff;	           /* make sure we have 9-bit values */
  ditherIn.green9 &= 0x1ff;
  ditherIn.blue9  &= 0x1ff;

  switch (modeReg) {
  case VISUAL8P: /* 8-bit visuals  */
  case VISUAL8U:
    /* simulate the 9-bit adder for scaling */
    ditherIn.red9   = ditherIn.red9   - (ditherIn.red9   >> 3);
    ditherIn.green9 = ditherIn.green9 - (ditherIn.green9 >> 3);
    ditherIn.blue9  = ditherIn.blue9  - (ditherIn.blue9  >> 2);
    redM            = (unsigned)matrixIn.red6;
    greenM          = (unsigned)matrixIn.green6;
    blueM           = (unsigned)matrixIn.blue7;
    break;
  
  case VISUAL12: /* 12-bit visuals */
    ditherIn.red9   = ditherIn.red9   - (ditherIn.red9   >> 4);
    ditherIn.green9 = ditherIn.green9 - (ditherIn.green9 >> 4);
    ditherIn.blue9  = ditherIn.blue9  - (ditherIn.blue9  >> 4);
    redM            = (unsigned)matrixIn.red6   >> 1;
    greenM          = (unsigned)matrixIn.green6 >> 1;
    blueM           = (unsigned)matrixIn.blue7  >> 2;
    break; 

  case VISUAL24: /* 24-bit dithering not useful except for testing */
    ditherIn.red9   = ditherIn.red9   - (ditherIn.red9   >> 3);
    ditherIn.green9 = ditherIn.green9 - (ditherIn.green9 >> 3);
    ditherIn.blue9  = ditherIn.blue9  - (ditherIn.blue9  >> 2);
    redM            = (unsigned)matrixIn.red6;
    greenM          = (unsigned)matrixIn.green6;
    blueM           = (unsigned)matrixIn.blue7;
    break;

  default:
    fprintf(stderr,"  Dither() in beh.c got illegal mode (%x)\n", modeReg);
    exit(1);
    break;
  }
  colorDither->red8   = (unsigned)ditherIn.red9   + redM   >> 1;
  colorDither->green8 = (unsigned)ditherIn.green9 + greenM >> 1;
  colorDither->blue8  = (unsigned)ditherIn.blue9  + blueM  >> 1;

  colorDither->red8   = ~(unsigned)colorDither->red8;   /* low-true color out*/
  colorDither->green8 = ~(unsigned)colorDither->green8;
  colorDither->blue8  = ~(unsigned)colorDither->blue8;
}

#include "PixelGenControlNoLoop_parts.c"
BEHAVIORAL PixelGenControlNoLoop(ModeType mode,
				 Signal depth32,
				 CmdType cmd, 
				 B24 addrIn,
				 CtlAddrGen2Type *ctl,
				 Signal *selCounter,
				 Signal *selConstant,
				 Signal *sel4,
				 Signal *sel1or4)
{
  *selCounter			= mode.dmaRd | mode.dmaWr | mode.line | mode.simple | cmd.copy64 | cmd.color | cmd.planeMask;
  *selConstant			= cmd.copy64 | cmd.color | cmd.planeMask | mode.simple;
  *sel4				= cmd.planeMask && depth32 && (mode.visual.dst != VISUAL8U) ? -1 : 0;
  *sel1or4			= ~cmd.copy64 | mode.simple;

  ctl->buildOpMask.visual32	= (mode.visual.dst & VISUAL32MASK ? -1 : 0);
  ctl->buildOpMask.blockMode	= ((mode.mode & BLOCKMASK) == TRANSPBLKSTIPPL ? -1 : 0);
  ctl->buildOpMask.unaligned	= (mode.mode & ~4) == OPAQUESTIPPLE
    				   && (mode.visual.dst & VISUAL32MASK) == 0
				   && (addrIn & 4) ? -1 : 0;

  /* !!!! should this be changed to mode.mode & 0x3f ???? */
  ctl->selData			= ~(cmd.copy64 | mode.simple | ((mode.mode & 7) == OPAQUESTIPPLE ? -1 : 0));
}

#include "PerBankBlockWriteStyle_parts.c"
BEHAVIORAL PerBankBlockWriteStyle(Bits4 blkStyle,
				 B24 addr1,
				 BlkStyleType *blkCtl)
{
  unsigned block8;	/* asserted if the RAM bank supports 8x8 blocks */
  unsigned rasBank;	/* ram bank (in range 0-3) */
  unsigned addr5;	/* bit 5 of pixel address */

  /* although the following address shifting isn't correct for
     packed 8-bit visuals, we don't care because block operations
     aren't supported for packed 8-bit visuals anyhow */
  addr5 = (addr1 >> 5) & 1;
  rasBank = (addr1 >> 21) & 3;
  block8 = (blkStyle >> rasBank) & 1 ? -1 : 0;
  
  blkCtl->maskHighNibble = ~addr5 & block8;
  blkCtl->maskLowNibble  = addr5 & block8;
}

#define begin {
#define end   }

#include "AddrCtl_parts.c"
BEHAVIORAL AddrCtl (signal first,
		    signal pend0,
		    signal zrd,
		    signal zop,
		    signal pend1,
		    signal lockreg,
		    signal memBusy,
		    signal memIdle,
		    signal dataReady,
		    signal zSafe,
		    signal copySafe,
		    signal last,
		    signal done,
		    signal errorSign,
		    signal zWrite,
		    signal fastFill,

		    ModeType mode,
		    signal pixelShift3,
		    signal req0,
		    CmdType cmd,
		    DmaStatusType dmaStatus,
		    B24 addrIn,
		    Signal reset,
		    CtlPipe2Type pipe2,

		    signal *i_first,
		    signal *i_pend0,
		    signal *i_zrd,
		    signal *i_zop,
		    signal *i_pend1,
		    signal *i_lockreg,
		    signal *lockreg1,
		    signal *i_req2,
		    signal *i_req1,
		    signal *i_loadReg,
		    CtlCopyLogicType *ctlCopyLogic,
		    signal *i_init,
		    CtlAddrGen1Type *ctl,
		    CtlPipe2Type *pipe1,
		    signal *i_next)
{
  int req2, replay, i_finished, i_writeReg;
  int selError, stepError, drainResidue;
  int memStall, copyStall, zStall;

    int selAddrMask1, selAddrMask0;
    int selMode1, selMode0;
    int mode1, mode0;
    int visual8;

  mode0 = mode.mode & 1 ? -1 : 0;
  mode1 = mode.mode & 2 ? -1 : 0;
  visual8   = (mode.visual.dst & VISUAL32MASK) ? 0 : -1;

  memStall  = memBusy;
  copyStall = mode.copy & ~pipe2.readFlag & ~copySafe & ~memIdle;
  zStall    = zop & ~zrd & ~zSafe;	/* memIdle shouldn't be necessary here */

  i_writeReg = cmd.color | cmd.planeMask;

  *i_next = pend0 & (~(memStall | copyStall) | ~pend1) & ~zStall;
  req2 = pend1 & ~(memStall | copyStall);
  *i_req2 = req2;
  *i_req1 = *i_next & ~done | reset;

  /*
   * The second term here allows the registers to update anytime
   * the pixel generator is idle.  This gets things like pixelShift
   * updated before the command parser starts writing the copy buffer.
   * It seems that Chris has no need for this extra term when I 
   * interface with his logic, but my ChrisStub implementation does
   * need it.
   *
   *	*i_loadReg = first1 & req1 | ~(pend0 & (~done | mode.line));
   */
  *i_loadReg 		= (zop & zrd & pend0 | ~mode.z) & first & *i_next
                        | ~pend0 & ~pend1;

  *i_pend1		= ~reset & (*i_req1 | pend1 & ~req2);

  selError		= cmd.newError & first;
  stepError		= mode.line & *i_next & ~i_writeReg;

  pipe1->stepZ		= mode.line & zop & ~i_writeReg;
  pipe1->stepBres	= (mode.line | mode.dmaRd | mode.dmaWr) & ~zop & ~i_writeReg;
  pipe1->newAddr	= dmaStatus.first & ~i_writeReg;
  pipe1->selAddr	= (~(mode.line | mode.dmaRd | mode.dmaWr) 
			| cmd.newAddr & first & mode.line
			| dmaStatus.first & first & ~mode.line
			| i_writeReg);
  pipe1->selectZ	= zop & ~i_writeReg;
  pipe1->readZ		= zop & zrd & ~i_writeReg;
  pipe1->enableZ	= ~mode.z | i_writeReg;
  pipe1->readFlag	= ((cmd.readFlag0 & mode.copy) | mode.dmaWr) & ~i_writeReg;
  pipe1->color		= cmd.color;
  pipe1->planeMask	= cmd.planeMask;
  pipe1->block		= ((mode.mode & BLOCKMASK) == TRANSPBLKSTIPPL && i_writeReg == 0 ? -1 : 0);

  pipe1->selSavedVals    = zop & ~zrd & pend0 & first & ~i_writeReg		  /* ZWRFIRST */
                         | ~zop & ~zrd & pend0 & first & mode.z & ~i_writeReg;	  /* FIRST & mode.z */
  pipe1->saveCurrentVals = zop & zrd & pend0 & first & ~i_writeReg;		  /* ZRDFIRST */

  /*
   * I think just the "lineMode & req1 & lineLast"
   * should be sufficient, but for now let's include
   * all the terms.
   *
   * now that lineLast no longer exists (it's just last)
   * the term should probably just be "req1 & last".
   */
  replay = zop & ~i_writeReg & *i_next & (mode.simple | last);

  *i_init = req0 | replay;

  i_finished	= ((last | mode.dmaWr | mode.dmaRd & (~dmaStatus.last | ~first)) & *i_next);
  
  drainResidue  = mode.dmaRd & dmaStatus.last & first & *i_next;

  *i_zop	= ~reset & (~i_writeReg & zrd
                | ~i_writeReg & zop & ~i_finished
                | req0 & mode.z);

  *i_zrd	= ~reset & (~i_writeReg & zrd & ~i_finished
                | req0 & mode.z);

  *i_pend0	= ~reset & (req0
		| pend0 & ~i_finished
                | ~i_writeReg & zop);

  *i_first	= ~reset & (~*i_next & first & ~i_finished
                | ~i_writeReg & zop & i_finished
                | req0);

  /*
   * don't assert lockreg in dmard, dmawr, or copy mode
   * because it isn't necessary and it screws up Chris
   * during dma.
   */
  *i_lockreg	= ~reset & (~*i_next & lockreg & ~i_finished
                | req0 & ((mode.mode & 3) != 3 ? -1 : 0));

  /*
   * lockreg1 is identical to i_lockreg except it doesn't
   * depend on req0.  this fixes a long timing path.
   */
  *lockreg1	= ~reset & (~*i_next & lockreg & ~i_finished);

  pipe1->lastDma = dmaStatus.last & ~req0;

  pipe1->first  = first;

  ctlCopyLogic->flush       = ~mode.dmaWr & pipe2.first & req2 & pipe2.readFlag /* | sfbRequest.reset */;
  ctlCopyLogic->wrMemData   = dataReady & (mode.copy | mode.dmaWr)
                            | drainResidue
			    | reset;

  ctl->req1			= *i_req1;
  ctl->req2			= req2;
  ctl->copy64			= cmd.copy64;

  ctl->selDmaRdData		= ~i_writeReg & mode.dmaRd;
  ctl->selOnes			= ~(dmaStatus.first | dmaStatus.last | dmaStatus.second);
  ctl->selEdge			= (pipe2.lastDma & ~first ? 3
				   : dmaStatus.last ? 2
				   : dmaStatus.first ? 0
				   : 1);
  ctl->bresError.saveCurrError	= pipe1->saveCurrentVals & *i_next | reset;
  ctl->addr.saveCurrentVals	= pipe2.saveCurrentVals & req2 | reset;
  ctl->color.saveCurrentVals	= pipe2.saveCurrentVals & req2 | reset;

  ctl->bresError.selSavedError	= pipe1->selSavedVals;
  ctl->addr.selSavedVals	= pipe2.selSavedVals;
  ctl->color.selSavedVals	= pipe2.selSavedVals;

  ctl->addr.selAddr		= pipe2.selAddr;
  ctl->color.selAddr0		= cmd.newAddr & first & mode.line
				| dmaStatus.first & first & ~mode.line
				| reset;
  ctl->color.selAddr1		= pipe2.selAddr;

  ctl->addr.selCurAddr		= (mode.line | mode.dmaRd | mode.dmaWr) & ~(pipe2.color | pipe2.planeMask);
  ctl->color.notLine	        = mode.mode == DMAREADDITHERED ? -1 : 0;
  ctl->addr.plus8		= (mode.dmaWr | mode.dmaRd) & ~(pipe2.color | pipe2.planeMask);
  ctl->addr.plus4		= mode.dmaRd & ~(pipe2.color | pipe2.planeMask);
  ctl->addr.plus1		= mode.dmaRd & (mode.visual.dst & VISUAL32MASK ? -1 : 0) & ~(pipe2.color | pipe2.planeMask);
  ctl->addr.plus1               = ctl->color.notLine ? -1 : ctl->addr.plus1;

  ctl->bresError.selError	= reset | selError;

  ctl->addr.selectZ		= pipe2.selectZ;

  ctl->addr.stepBres		= reset | pipe2.stepBres & req2;
  ctl->color.stepIndex		= reset | pipe1->stepBres & *i_next;
  ctl->color.stepBres		= reset | pipe2.stepBres & req2;
  ctl->bresError.stepBres	= reset | stepError;

  ctl->addr.stepZ		= reset | pipe2.stepZ & req2;
  ctl->color.stepZ		= reset | pipe2.stepZ & req2;

  ctl->stipMask.blockMode	= ((mode.mode & BLOCKMASK) == TRANSPBLKSTIPPL ? -1 : 0)
				| (mode.dmaWr & ~(dmaStatus.first | dmaStatus.second | dmaStatus.last))
                                | (cmd.copy64)
                                | (cmd.color)
                                | (cmd.planeMask)
                                | (mode.dmaRd);

  ctl->addr.lineMode		= mode.line & ~(pipe2.color | pipe2.planeMask);
  ctl->makeStipple.lineMode	= mode.line;

  ctl->addr.visual32		= (pipe2.readFlag ? mode.visual.src : mode.visual.dst) & VISUAL32MASK ? -1 : 0;

  ctl->makeStipple.visual32	= (mode.visual.dst & VISUAL32MASK ? -1 : 0);
  ctl->stipMask.visual32	= (mode.visual.dst & VISUAL32MASK ? -1 : 0)
    				& ((mode.mode & DMAMASK) != DMAMASK ? -1 : 0);

  ctl->stipMask.unaligned	= ((mode.mode & ~4) == OPAQUESTIPPLE)
    				   && (mode.visual.dst & VISUAL32MASK) == 0
				   && (addrIn & 4) ? -1 : 0;
  pipe1->unaligned		= ctl->stipMask.unaligned;
  ctl->makeStipple.unaligned	= pipe2.unaligned;
  ctl->color.bresError		= errorSign;
  ctl->addr.errorSign		= errorSign;

  ctl->makeStipple.transparent	= (mode.mode & 4 ? -1 : 0);
  ctl->color.dstVisual     	= mode.visual.dst;
  ctl->color.mode               = mode.mode;
  ctl->addr.negateIncVal	= mode.copy & pixelShift3 & ~(pipe2.color | pipe2.planeMask);
  ctl->color.selDither		= (mode.mode & DITHEREDMASK ? -1 : 0);

    selMode1  = (~mode1 | mode.dmaRd)
              & ~pipe2.selectZ
              & ~mode.dmaWr
              & ~pipe2.color
              & ~pipe2.planeMask;

    selMode0  = (~mode0 | mode.dmaRd)
              & ~mode.dmaWr
              & ~pipe2.color
              & ~pipe2.planeMask;

    ctl->byteMask.selMode       = (selMode1 ? 2 : 0) | (selMode0 ? 1 : 0);

  ctl->memCmd.color		= pipe2.color;
  ctl->memCmd.planeMask		= pipe2.planeMask;
  ctl->memCmd.line		= mode.line;
  ctl->memCmd.packed8bit	= (pipe2.readFlag ? (mode.visual.src == VISUAL8P) : (mode.visual.dst == VISUAL8P)) ? -1 : 0;
  ctl->memCmd.unpacked8bit	= (pipe2.readFlag ? (mode.visual.src == VISUAL8U) : (mode.visual.dst == VISUAL8U)) ? -1 : 0;

  if (pipe2.color || pipe2.planeMask) begin
    ctl->memCmd.readFlag	= 0;
    ctl->memCmd.selectZ		= 0;
    ctl->memCmd.readZ		= 0;
    ctl->memCmd.block		= 0;
    ctl->memCmd.fastFill	= 0;
  end else begin
    ctl->memCmd.readFlag	= pipe2.readFlag;
    ctl->memCmd.selectZ		= pipe2.selectZ;
    ctl->memCmd.readZ		= pipe2.readZ;
    ctl->memCmd.block		= (mode.mode & BLOCKMASK) == TRANSPBLKSTIPPL;
    ctl->memCmd.fastFill	= fastFill;
  end

  ctl->byteMask.z16Sel = mode.z16;
  ctl->byteMask.readZ  = ctl->memCmd.readZ;
  ctl->byteMask.enable = (ctl->memCmd.selectZ | zWrite | pipe2.enableZ)
                         & ~(mode.dmaWr & pipe2.newAddr);
    /*
     * SELnnn	= sel/00
     * SELnXX	= sel/01
     * SELXXX	= sel/10
     */

  selAddrMask1  = ~pipe2.color & ~pipe2.selectZ & ~mode1 & mode0
                | ~pipe2.color & ~mode.dmaRd & ~pipe2.selectZ & mode0
                | ~pipe2.color & mode.dmaWr & ~pipe2.selectZ
                | pipe2.planeMask & ~pipe2.selectZ;

  selAddrMask0  = ~pipe2.planeMask & ~mode1 & ~mode0
                | ~pipe2.planeMask & ~mode0 & ~visual8
                | ~pipe2.planeMask & mode.dmaRd & ~ctl->color.notLine
                | pipe2.selectZ
                | pipe2.color;

  ctl->byteMask.selAddrMask	= (selAddrMask1 ? 2 : 0) | (selAddrMask0 ? 1 : 0);

  ctl->byteMask.bigPixels       = mode.visual.dst != VISUAL8P
    				&& (!visual8 || pipe2.selectZ || pipe2.color)
				  ? -1 : 0;

  ctl->selDataMux.data64 	= (mode.copy ? 3
				   : (mode.mode & BLOCKMASK) == TRANSPBLKSTIPPL ? 2
				   : (mode.mode & COLORINTERPMASK) ? 0
				   : 1);
  ctl->selDataMux.data64        = (pipe2.color
				   | pipe2.planeMask
				   | mode.simple
				   | pipe2.selectZ
				   | mode.dmaRd
				   | ctl->color.notLine) ? 0 : ctl->selDataMux.data64;
  ctl->selDataMux.notZ		= ~pipe2.selectZ;
  ctl->selDataMux.notData32	= ~(pipe2.color | pipe2.planeMask |
				    mode.simple & ~pipe2.selectZ | 
				    mode.dmaRd ^ ctl->color.notLine);
}
