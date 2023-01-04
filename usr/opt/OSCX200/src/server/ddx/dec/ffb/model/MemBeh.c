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
static char *rcsid = "@(#)$RCSfile: MemBeh.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:21:01 $";
#endif
/*
 * HISTORY
 * $Log: MemBeh.c,v $
 * Revision 1.1.2.2  1993/11/19  21:21:01  Robert_Lembree
 * 	SFB+ Initial version
 * 	[1993/11/19  16:32:26  Robert_Lembree]
 *
 * Revision 1.1.1.2  1993/11/19  16:32:26  Robert_Lembree
 * 	SFB+ Initial version
 *
 * Revision 1.20  1993/06/11  01:52:45  rsm
 * added init into MemIntfc
 *
 * Revision 1.19  1993/06/07  16:48:44  rsm
 * added more complete information in a debug printf
 *
 * Revision 1.18  1993/06/01  21:04:24  rsm
 * added checks for correct byte masks for planeMask and color register writes
 *
 * Revision 1.17  1993/05/14  20:08:27  rsm
 * made rasterVid declaration
 *
 * Revision 1.16  1993/05/10  20:01:02  chris
 * use dmaStall to hold off reads
 *
 * Revision 1.15  1993/05/04  12:54:00  rsm
 * added 12-bit data replication for framebuffer reads
 * fixed bug converting pixel address to 32-bit address
 * performance hack: call do_rams 4 times instead of 8 for 8-bit unpacked
 *
 * Revision 1.14  1993/04/21  13:57:03  rsm
 * zero out 2 lo-order bits of address for VRAM reads
 * from 8-bit unpacked visuals
 *
 * Revision 1.13  1993/04/16  15:57:57  rsm
 * fixed typo (VISUAL8P should have been VISUAL8U)
 *
 * Revision 1.12  1993/04/16  01:58:10  chris
 * depth/rotate visual changes
 *
 */
#include <stdio.h>
#include "lyreTypes.h"
#include "parts_c.h"
#include "types.h"
#include "vars.h"

int interactiveFlag;
int debugFlag;
FILE *vid;
FILE *rasterVid;

/*
 * function prototypes.
 */
#include "MemBeh.h"

ULONG64 do_rams (unsigned addr, ULONG64 data, unsigned planeMask,
		 unsigned byteMask, unsigned we_L, SFBDepth depth);

OpenVid()
{
#ifndef VMS
  extern DEEPREG shadowDeep;

  if( vid == NULL ) {
    if( (vid = popen("xapp", "w")) == NULL ) {
      fprintf( stderr, "couldn't open pipe to 'xapp'\n" );
      exit( 1 );
    }
    fprintf (vid, "i\nc\n");
    if (shadowDeep.reg.deep)
      fprintf (vid, "l 24\nv 8\nd 7\nn 0\n");
  }
#endif
}

#define FIFOSIZE 16

#include "MemIntfc_parts.c"
BEHAVIORAL MemIntfc (Signal dmaStall,
		     VisualType visual,
		     RotateType rotate,
		     MemRequestType memReq,
		     signal req,
		     B4 rop,
		     Signal depth32,
		     Signal init,
		     B32 ticksIn,
		     B32 headIn,
		     B32 tailIn,
		     MemStatusType *memStat,
		     B32 *ticksOut,
		     B32 *headOut,
		     B32 *tailOut)
{
  PIXELS dst;
  static unsigned planeMask;	/* !!!! this must be an array of 4 !!!! */
  static reading = 0;
  static unsigned lastStencil;
  static MemRequestType reqFIFO [FIFOSIZE];
  static unsigned colorReg [8];
  Visual srcVisual, dstVisual;

  if (init) {
    memStat->stencil = 0;
    memStat->dest.data[0] = 0;
    memStat->dest.data[1] = 0;
    memStat->dest.mask = 0;
    headIn = 0;
    ticksIn = 0;
    tailIn = 0;
  }

  srcVisual.depth32      = depth32;
  srcVisual.unpacked8bit = (visual.src == VISUAL8U);
  srcVisual.bytePos      = rotate.src;

  dstVisual.depth32      = depth32;
  dstVisual.unpacked8bit = (visual.dst == VISUAL8U);
  dstVisual.bytePos      = rotate.dst;

  /*
   * incoming address in memReq structure is a pixel address.
   * this is converted into a 32-bit address when it is put
   * into the reqFIFO.
   */

  memStat->dataReady = 0;
  memStat->stencilReady = 0;
  memStat->stencil = lastStencil;
  memStat->idle = -1;

  if (reading) {
    /*
     * read operations take two ticks.
     * this is the second tick.
     * do nothing.
     */
    reading = 0;
    memStat->idle = 0;
  } else if (dmaStall) {
    /*
     * pretend there is nothing to do...
     */
  } else if (headIn != tailIn) {
    /*
     * the request FIFO is not empty.
     * pull out a request and start processing it.
     */
    memStat->idle = 0;
/*    if ((random() & 3) >= 1) */{
      if (reqFIFO[tailIn].cmd.planeMask) {
	/*
	 * planeMask write.
	 */
	/* !!!! extend this for 4 planemasks !!!! */
	planeMask = reqFIFO[tailIn].data[0];
	if (reqFIFO[tailIn].mask != 0xff) {
	  extern int ticks;
	  extern int cmdLine, vramLine;

	  fprintf (stderr, "**** inconsistent mask during planemask write:\n");
	  fprintf (stderr, "     clock tick %d\n", ticks);
	  fprintf (stderr, "     cmdLine = %d, vramLine = %d\n", cmdLine, vramLine);
	  fprintf (stderr, "     write to planemask%d, mask %-.2x, data %-.8x %-.8x\n",
		   reqFIFO[tailIn].addr & 7,
		   reqFIFO[tailIn].mask,
		   reqFIFO[tailIn].data[1],
		   reqFIFO[tailIn].data[0]);
	}
#if 0
	printf( "*** write to planeMask%d, mask %-.2x, data %-.8x %-.8x\n",
	       reqFIFO[tailIn].addr & 7,
	       reqFIFO[tailIn].mask, reqFIFO[tailIn].data[1], reqFIFO[tailIn].data[0]);
#endif
      } else if (reqFIFO[tailIn].cmd.color) {
	/*
	 * color register write.
	 */
#if 0
	static lastColorReg;
	int curColorReg = reqFIFO[tailIn].addr & 7;
	extern int ticks;
	extern int cmdLine, vramLine;

	if (curColorReg == 4 && lastColorReg != 3) {
	  printf ("**** inconsistent colorreg write:\n");
	  printf ("     clock tick %d\n", ticks);
	  printf ("     cmdLine = %d, vramLine = %d\n", cmdLine, vramLine);
	  exit (1);
	}
	lastColorReg = curColorReg;
#endif
	colorReg[reqFIFO[tailIn].addr & 7] = reqFIFO[tailIn].data[0];
	if (reqFIFO[tailIn].addr & 1 != 0 && reqFIFO[tailIn].mask != 0xf0
	    || reqFIFO[tailIn].addr & 1 == 0 && reqFIFO[tailIn].mask != 0x0f) {
	  extern int ticks;
	  extern int cmdLine, vramLine;

	  fprintf (stderr, "**** inconsistent mask during colorreg write:\n");
	  fprintf (stderr, "     clock tick %d\n", ticks);
	  fprintf (stderr, "     cmdLine = %d, vramLine = %d\n", cmdLine, vramLine);
	  fprintf (stderr, "     write to BLOCKCOLOR%d, mask %-.2x, data %-.8x %-.8x\n",
		   reqFIFO[tailIn].addr & 7,
		   reqFIFO[tailIn].mask,
		   reqFIFO[tailIn].data[1],
		   reqFIFO[tailIn].data[0]);
	}
#if 0
	printf( "*** write to BLOCKCOLOR%d, mask %-.2x, data %-.8x %-.8x\n",
	       reqFIFO[tailIn].addr & 7,
	       reqFIFO[tailIn].mask,
	       reqFIFO[tailIn].data[1],
	       reqFIFO[tailIn].data[0]);
#endif
      } else {
	if (interactiveFlag && vid == NULL) OpenVid();
	
	if (reqFIFO[tailIn].cmd.selectZ) {
	  /*
	   * Z buffer request.
	   */
	  if (reqFIFO[tailIn].cmd.readZ) {
	    /*
	     * Z buffer read.
	     */
	    reading = 1;
	    memStat->idle = 0;
	    memStat->stencilReady = -1;
	    memStat->stencil = ZBufferRead (reqFIFO[tailIn].addr,
					    reqFIFO[tailIn].zTest,
					    reqFIFO[tailIn].data[0],
					    reqFIFO[tailIn].mask,
					    depth32);
	  
	    memStat->dest.data[0] = dst.l.lo;
	    memStat->dest.data[1] = dst.l.hi;
	    memStat->dest.mask = reqFIFO[tailIn].mask;

	    lastStencil = memStat->stencil;
	  } else {
	    /*
	     * Z buffer write.
	     */
	    ZBufferWrite (reqFIFO[tailIn].addr,
			  reqFIFO[tailIn].data[0],
			  reqFIFO[tailIn].sWrite,
			  reqFIFO[tailIn].zWrite,
			  reqFIFO[tailIn].mask,
			  depth32);
	  }
	} else {
	  /*
	   * It's not a Z buffer operation.
	   */

	  /* convert 32-bit address to 64-bit address */
	  reqFIFO[tailIn].addr >>= 1;

	  if (reqFIFO[tailIn].cmd.readFlag == 0) {
	    /*
	     * writing.
	     */
	    if (reqFIFO[tailIn].cmd.block) {
	      /*
	       * block mode write.
	       */
	      if (reqFIFO[tailIn].cmd.fastFill) {
		unsigned i;

		for (i=0; i<4; ++i)
		  BlockWriteVRAM (reqFIFO[tailIn].addr^i,
				  reqFIFO[tailIn].data[1], reqFIFO[tailIn].data[0],
				  reqFIFO[tailIn].mask, planeMask, colorReg, dstVisual);
	      } else {
		BlockWriteVRAM (reqFIFO[tailIn].addr,
				reqFIFO[tailIn].data[1], reqFIFO[tailIn].data[0],
				reqFIFO[tailIn].mask, planeMask, colorReg, dstVisual);
	      }
	    } else {
	      /*
	       * normal write.
	       */
	      WriteVRAM (reqFIFO[tailIn].addr,
			 reqFIFO[tailIn].data[1], reqFIFO[tailIn].data[0],
			 rop, reqFIFO[tailIn].mask, planeMask, dstVisual);
	    }
	  } else {
	    /*
	     * reading.
	     */
	    dst.l = ReadVRAM( reqFIFO[tailIn].addr, srcVisual );
	  
	    if (visual.src == VISUAL12) {
	      memStat->dest.data[0] = dst.l.lo & 0x000f0f0f;
	      memStat->dest.data[0] |= memStat->dest.data[0] << 4;
	      memStat->dest.data[0] |= dst.l.lo & 0xff000000;

	      memStat->dest.data[1] = dst.l.hi & 0x000f0f0f;
	      memStat->dest.data[1] |= memStat->dest.data[1] << 4;
	      memStat->dest.data[1] |= dst.l.hi & 0xff000000;
	    } else if (visual.src == VISUAL12HI) {
	      memStat->dest.data[0] = dst.l.lo & 0x00f0f0f0;
	      memStat->dest.data[0] |= memStat->dest.data[0] >> 4;
	      memStat->dest.data[0] |= dst.l.lo & 0xff000000;

	      memStat->dest.data[1] = dst.l.hi & 0x00f0f0f0;
	      memStat->dest.data[1] |= memStat->dest.data[1] >> 4;
	      memStat->dest.data[1] |= dst.l.hi & 0xff000000;
	    } else {
	      memStat->dest.data[0] = dst.l.lo;
	      memStat->dest.data[1] = dst.l.hi;
	    }
	    memStat->dest.mask = reqFIFO[tailIn].mask;
	    memStat->dataReady = -1;
	    reading = 1;
	    memStat->idle = 0;
#if 0
	    printf( "*** read of %-.8x, row %-.3x, col %-.3x, data %-.8x %-.8x\n",
		   reqFIFO[tailIn].addr, row, col, dst.l.hi, dst.l.lo);
#endif
	  }
	}
      }
      tailIn = (tailIn + 1) % FIFOSIZE;
    }
  }

  if (req) {
    /* add current request to the reqFIFO */
    memStat->idle = 0;

    /* convert pixel address to 32-bit address */
    if (memReq.cmd.packed8bit)  /* 8-bit packed */
      memReq.addr >>= 2;

    reqFIFO[headIn] = memReq;
    headIn = (headIn + 1) % FIFOSIZE;
    if (headIn == tailIn) {
      fprintf(stderr, "PANIC! request FIFO overflowed in MemIntfc\n");
      fprintf(stderr, "head = %d, tail = %d\n", headIn, tailIn);
      exit(1);
    }
#if 0
    if (memReq.cmd.planeMask)
      printf( "*** write to planeMask (%d), data %-.8x %-.8x\n",
	     memReq.cmd.planeMask, memReq.data[1], memReq.data[0]);
#endif
  }

  /* check for FIFO full */
  if (((headIn + 1) % FIFOSIZE) == tailIn)
    /* FIFO is full, set busy so we don't get more commands. */
    memStat->busy = -1;
  else
    /* FIFO is not full. */
    memStat->busy = 0;
  
  *headOut = headIn;
  *tailOut = tailIn;

  if (memStat->idle)
    /* idle - no need to get called next clock tick */
    *ticksOut = ticksIn;
  else
    /* not idle - make sure we get called next clock tick */
    *ticksOut = ticksIn + 1;
}

void BlockWriteVRAM (B24 addr,	/* N.B. this is a 64-bit address */
		     B32 dataHi,
		     B32 dataLo,
		     Bits8 mask,
		     B32 planeMask,
		     unsigned colorReg[],
		     Visual visual)
{
  PIXELS m;	/* mask bits  */
  unsigned w;	/* write mask */
  unsigned a;	/* computed address */
  int i, j;
	      
#if 0
  if (colorReg[0] != colorReg[1]) {
    extern int ticks;
    extern int cmdLine, vramLine;

    printf ("**** inconsistent colorreg: [0]=%-.8x, [1]=%-.8x\n",
	    colorReg[0], colorReg[1]);
    printf ("     clock tick %d\n", ticks);
    printf ("     cmdLine = %d, vramLine = %d\n", cmdLine, vramLine);
  }
#endif

  m.l.lo = dataLo;
  m.l.hi = dataHi;
#if 0
  printf( "**** block write: %-.8x %-.8x\n", dataHi, dataLo);
#endif
	      
  if (visual.depth32 &&  ! visual.unpacked8bit) {
    /*
     * 32bpp
     *
     * this code will work for rams with 4x8 block writes.
     * for rams with 4x4 block writes (TI rams), this code
     * works assuming that the low nibble and high nibble
     * are identical.
     *
     * this code DOES NOT work for rams with 8x8 block writes.
     */

    /* first do a consistency check on the mask */
    for (j=0; j<2; ++j) {
      w = m.p32[j] & 0xf;
      w |= w << 4;
      w |= w << 8;
      w |= w << 16;
      if (w != m.p32[j]) {
        fprintf (stderr, "BlockWriteVRAM: unexpected mask value\n");
        fprintf (stderr, "recvd = %-.8x, expected = %-.8x\n", m.p32[j], w);
      }
    }

    for (j=0; j<4; ++j) {
      w = 0;
      for (i=0; i<2; ++i) {
	if (m.p32[i] & (1<<j))
	  w |= (0xf << 4*i);
      }
      a = (addr&~0xc)+(j%4 << 2);
      WriteVRAM (a,
		 colorReg[((a&3)<<1)+1],
		 colorReg[(a&3)<<1],
		 GXcopy,
		 w,
		 planeMask,
		 visual);
    }
  } else {
    /*
     * 8bpp
     *
     * this code will work for rams with 4x8 block writes.
     * for rams with 4x4 block writes (TI rams), this code
     * works assuming that the low nibble and high nibble
     * are identical.
     *
     * this code DOES NOT work for rams with 8x8 block writes.
     */

    /* first do a consistency check on the mask */
    for (j=0; j<2; ++j) {
      w = m.p32[j] & 0x0f0f0f0f;
      w |= w << 4;
      if (w != m.p32[j]) {
        fprintf (stderr, "BlockWriteVRAM: unexpected mask value\n");
        fprintf (stderr, "recvd = %-.8x, expected = %-.8x\n", m.p32[j], w);
      }
    }

    for (j=0; j<4; ++j) {
      w = 0;
      for (i=0; i<8; ++i) {
	if (m.p8[i] & (1<<j))
	  w |= (1 << i);
      }
      if (visual.depth32)
	a = (addr&~0xc)+(j%4 << 2);
      else
	a = (addr&~3) + (j%4);
      if (w) {
	WriteVRAM (a,
		   colorReg[1],
		   colorReg[0],
		   GXcopy,
		   w,
		   planeMask,
		   visual);
#if 0
	printf( "**** a = %x, d = %-.8x %-.8x w = %-.2x, m = %-.8x\n",
	       a,
	       colorReg[1],
	       colorReg[0],
	       w,
	       planeMask);
#endif
      }
    }
  }
}

/*
 * read the Z-buffer.
 */
B9 ZBufferRead(B24 addr,	/* N.B. this is a 32-bit address */
	       Bits3 zTest,
	       B32 zFrag,
	       Bits8 fifoMask,
	       Signal depth32)
{
  extern int cmdLine, vramLine;
  PIXELS dst;
  unsigned zValue;
  unsigned zCompare;
  unsigned stencil;
  ULONG64 zero;

  zero.lo = zero.hi = 0;

  dst.l = do_rams (addr>>1, zero, 0, 0xff, 1, depth32);

  if (((addr & 1) != 0) != ((fifoMask & 8) == 0)) {
    fprintf (stderr, "**** ZRead: incorrect mask generated: ");
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
    fprintf (stderr, "received: addr=%x, %-.2x\n", addr<<2, fifoMask);
/*    exit (1);*/
  }

  zValue = dst.p32[addr & 1];
  stencil = zValue >> 24;

  /* mask z's to 24-bits */
  zValue &= 0xffffff;
  zFrag  &= 0xffffff;

  switch (zTest) {
  case /* 000 */ GL_GEQUAL:
    zCompare = zFrag >= zValue;
    break;
  case /* 001 */ GL_ALWAYS:
    zCompare = 1;
    break;
  case /* 010 */ GL_NEVER:
    zCompare = 0;
    break;
  case /* 011 */ GL_LESS:
    zCompare = zFrag < zValue;
    break;
  case /* 100 */ GL_EQUAL:
    zCompare = zFrag == zValue;
    break;
  case /* 101 */ GL_LEQUAL:
    zCompare = zFrag <= zValue;
    break;
  case /* 110 */ GL_GREATER:
    zCompare = zFrag > zValue;
    break;
  case /* 111 */ GL_NOTEQUAL:
    zCompare = zFrag != zValue;
    break;
  default:
    fprintf (stderr, "unrecognized zTest value (%d)\n", zTest);
    exit (1);
  }

#if 0
  fprintf(stderr,"**** ZRead (addr=%x, ztest=%d, zfrag=%x, zvalue=%x, result=%-.3x\n",
	  addr<<2, zTest, zFrag, zValue, (stencil & 0xff) | (zCompare << 8));
#endif
  return (stencil & 0xff) | (zCompare << 8);
}

/*
 * write the Z-buffer.
 */
void ZBufferWrite(B24 addr,	/* N.B. this is a 32-bit address */
		  B32 dataLo,

		  signal sWrite,
		  signal zWrite,
		  Bits8 fifoMask,
		  Signal depth32)
{
  extern int cmdLine, vramLine;
  PIXELS src;
  unsigned mask;

  src.l.lo = dataLo;
  src.l.hi = dataLo;
  
  mask = (zWrite ? 7 : 0);
  if (sWrite) mask |= 8;
  if (addr & 1) mask <<= 4;

  if ((mask&0xff) != (fifoMask&0xff)) {
    fprintf (stderr, "**** ZWrite: incorrect mask generated: ");
    fprintf (stderr, "cmdLine = %d, vramLine = %d\n",
	     cmdLine, vramLine);
    fprintf (stderr, "expected: %-.2x\n", mask);
    fprintf (stderr, "received: %-.2x\n", fifoMask);
    fprintf (stderr, "addr=%x, data=%x\n", addr<<2, dataLo);
/*    exit (1);*/
  }
#if 0
  fprintf(stderr,"**** ZWrite (addr=%x, zdata=%x, sWrite=%d, zWrite=%d\n",
	  addr<<2, dataLo, sWrite!=0, zWrite!=0);
#endif
  do_rams( addr >> 1, src.l, /*planeMask*/-1, mask, TRUE_L, depth32 );
}

/*
 * write the VRAM's.
 */
void WriteVRAM (B24 addr,	/* N.B. this is a 64-bit address */
		B32 dataHi,
		B32 dataLo,
		B4 rop,
		Bits8 mask,
		B32 planeMask,
		Visual visual)
{
  int i;
  PIXELS src;
  PIXELS dst;

  switch (rop) {
  case GXcopy:		/* src */
    src.l.lo = dataLo;
    src.l.hi = dataHi;
    break;

  case GXcopyInverted:	/* NOT src */
    src.l.lo = ~dataLo;
    src.l.hi = ~dataHi;
    break;

  case GXclear:		/* 0 */
    src.l.lo = 0;
    src.l.hi = 0;
    break;

  case GXset:		/* 1 */
    src.l.lo = 0xffffffff;
    src.l.hi = 0xffffffff;
    break;

  case GXnoop:		/* dst */
    /*
     * GXnoop can't be optimized due to old traces.
     * We need the write to occur so the comparison works.
     * Fall thru to the non-trivial rop cases.
     */
  case GXand:		/* src AND dst */
  case GXandReverse:	/* src AND NOT dst */
  case GXandInverted:	/* NOT src AND dst */
  case GXxor:		/* src XOR dst */
  case GXor:		/* src OR dst */
  case GXnor:		/* NOT src AND NOT dst */
  case GXequiv:		/* NOT src XOR dst */
  case GXinvert:	/* NOT dst */
  case GXorReverse:	/* src OR NOT dst */
  case GXorInverted:	/* NOT src OR dst */
  case GXnand:		/* NOT src OR NOT dst */
    src.l.lo = dataLo;
    src.l.hi = dataHi;
    dst.l = ReadVRAM( addr, visual );
  
    for (i=0; i<2; ++i)
      src.p32[i] = RopFunc( src.p32[i], dst.p32[i], rop );
    break;

  default:
    fprintf (stderr, "PANIC! unrecognized rop (%d)\n", rop);
    exit (1);
  }

#if 0
  printf( "*** write to %-.8x, mask %x ", addr, mask);
  printf( " planeMask %-.8x, rop %x\n", planeMask, rop);
  printf( "src data %-.8x.%-.8x\n", dataHi, dataLo);
  printf( "dst data %-.8x.%-.8x\n", dst.l.hi, dst.l.lo);
  printf( "rop data %-.8x.%-.8x\n", src.l.hi, src.l.lo);
#endif
  if (visual.unpacked8bit) {
    unsigned m, a;
#if 0
  fprintf( stderr, "*** unpacked 8 bit write to %-.8x, mask %x, data %-.8x %-.8x\n",
	 addr, mask, src.l.hi, src.l.lo);
#endif
    for (i=0; i<4; ++i) {
      PIXELS d;

      m = (mask >> 2*i) & 3;
      if (m) {
	unsigned b;
	/* do this write */
	b = src.p8 [2*i];
	b |= b << 8;
	b |= b << 16;
	d.p32 [0] = b;
	b = src.p8 [2*i+1];
	b |= b << 8;
	b |= b << 16;
	d.p32 [1] = b;
	a = (addr&~3) + i;
	switch (m) {
	case 0: /* can't happen */
	case 1: m = 0x01 << visual.bytePos; break;
	case 2: m = 0x10 << visual.bytePos; break;
	case 3: m = 0x11 << visual.bytePos; break;
	}

	do_rams( a, d.l, planeMask, m, TRUE_L, visual.depth32 );
      }
    }
  } else {
    do_rams( addr, src.l, planeMask, mask, TRUE_L, visual.depth32 );
  }
}


/*
 * read the VRAM's and return 64-bits of data.
 */
ULONG64 ReadVRAM (unsigned addr,	/* N.B. this is a 64-bit address */
		  Visual visual)
{
  ULONG64 zero;
  PIXELS data;

  zero.lo = zero.hi = 0;

  if (visual.unpacked8bit) {
    unsigned m, a, i;
    PIXELS rd;
    /* we have to do 4 separate reads since
       we use just two bytes out of every 8. */
    for (i=0; i<4; ++i) {
      rd.l = do_rams ((addr&~3)+i, zero, 0, 0xff, 1, visual.depth32);
      data.p8 [2*i + 0] = rd.p8 [0 + visual.bytePos];
      data.p8 [2*i + 1] = rd.p8 [4 + visual.bytePos];
    }
  } else {
    data.l = do_rams (addr, zero, 0, 0xff, 1, visual.depth32);
  }
#if 0
  printf ("**** read : %08x%08x, address = %08x\n",
	  data.p32[1], data.p32[0], addr<<3);
#endif
  return data.l;
}

/*
 * RopFunc implements the 16 standard GC functions.
 * see "X Window System - C Library and Protocol Reference",
 * Scheifler, Gettys, and Newman page 97.
 */
unsigned RopFunc (unsigned src, unsigned dst, unsigned rop)
{
  unsigned opbit3;
  unsigned opbit2;
  unsigned opbit1;
  unsigned opbit0;
  unsigned res;

  /*
   * separate out the raster operation bits
   */
  opbit3 = (rop & 0x08 ? 0xffffffff : 0);
  opbit2 = (rop & 0x04 ? 0xffffffff : 0);
  opbit1 = (rop & 0x02 ? 0xffffffff : 0);
  opbit0 = (rop & 0x01 ? 0xffffffff : 0);
  
  /*
   * perform the muxing action
   */
  res = (~src & ~dst & opbit3)
      | (~src &  dst & opbit2)
      | ( src & ~dst & opbit1)
      | ( src &  dst & opbit0);

  return res;
}
