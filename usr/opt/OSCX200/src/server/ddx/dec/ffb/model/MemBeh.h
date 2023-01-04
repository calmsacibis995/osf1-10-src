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
 * @(#)$RCSfile: MemBeh.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:21:14 $
 */
/*
 * function prototypes.
 */

typedef struct {
  int depth32;		/* 32-plane system */
  int unpacked8bit;	/* unpacked 8-bit visual on 32-plane system */
  int bytePos;		/* byte position for unpacked 8-bit visuals */
} Visual;

ULONG64 ReadVRAM (unsigned addr, Visual v);

void WriteVRAM (B24 addr, B32 dataHi, B32 dataLo, B4 rop, Bits8 mask,
		B32 planeMask, Visual v);

void BlockWriteVRAM (B24 addr, B32 dataHi, B32 dataLo, Bits8 mask,
		     B32 planeMask, unsigned colorReg[], Visual v);

void ZBufferWrite (B24 addr, B32 dataLo, signal sWrite, signal zWrite,
		   Bits8 fifoMask, Signal depth32);

B9 ZBufferRead(B24 addr, Bits3 zTest, B32 zFrag, Bits8 fifoMask, Signal depth32);

unsigned RopFunc (unsigned src, unsigned dst, unsigned rop);
