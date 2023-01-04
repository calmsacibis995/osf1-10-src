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
 * @(#)$RCSfile: io_struct.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:40:18 $
 */
struct io_struct
{
  Net_Entry Vdd;
  Net_Entry Vss;
  Net_Entry bdmaBase;
  Net_Entry baddrIn;
  Net_Entry bbg;
  Net_Entry bblkStyle;
  Net_Entry bblueinc;
  Net_Entry bblueval;
  Net_Entry bbresa1;
  Net_Entry bbresa2;
  Net_Entry bbrese;
  Net_Entry bbrese1;
  Net_Entry bbrese2;
  Net_Entry bcbdataIn;
  Net_Entry bcmdlast;
  Net_Entry bcol;
  Net_Entry bcolor;
  Net_Entry bcopy64;
  Net_Entry bcopyMode;
  Net_Entry bdataIn;
  Net_Entry bdataReg;
  Net_Entry bdepth;
  Net_Entry bdmaRdMode;
  Net_Entry bdmaWrMode;
  Net_Entry bdxdy;
  Net_Entry bfg;
  Net_Entry bgreeninc;
  Net_Entry bgreenval;
  Net_Entry bi_busy0;
  Net_Entry bidle;
  Net_Entry blineLength;
  Net_Entry blineMode;
  Net_Entry bloadDmaRdData;
  Net_Entry bLockReg;
  Net_Entry bmask;
  Net_Entry bmode;
  Net_Entry bmodeZ16;
  Net_Entry bpixelMask;
  Net_Entry bpixelShift;
  Net_Entry brdData0;
  Net_Entry brdData1;
  Net_Entry bredinc;
  Net_Entry bredval;
  Net_Entry breq0;
  Net_Entry brop;
  Net_Entry brotateDst;
  Net_Entry brotateSrc;
  Net_Entry brow;
  Net_Entry bsFail;
  Net_Entry bsimpleMode;
  Net_Entry bstencilRef;
  Net_Entry bsTest;
  Net_Entry bstippleMode;
  Net_Entry bszPass;
  Net_Entry btcMask;
  Net_Entry bvisualDst;
  Net_Entry bvisualSrc;
  Net_Entry bza1;
  Net_Entry bza2;
  Net_Entry bzBase;
  Net_Entry bzFail;
  Net_Entry bzIncHi;
  Net_Entry bzIncLo;
  Net_Entry bzOp;
  Net_Entry bzTest;
  Net_Entry bzValHi;
  Net_Entry bzValLo;
  Net_Entry flush;
  Net_Entry loadHiBuff;
  Net_Entry loadLoBuff;
  Net_Entry newAddr;
  Net_Entry newError;
  Net_Entry rdCopyBuff;
  Net_Entry readFlag0;
  Net_Entry selCpuData;
  Net_Entry sRdMask;
  Net_Entry sWrMask;
  Net_Entry _reset;
};
