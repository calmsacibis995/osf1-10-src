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
static char	*sccsid = "@(#)$RCSfile: c765.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:07:55 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
 
/*
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 *    Copyright 1988  Intel Corporation.
 */

/*	Copyright (c) 1987, 1988 TOSHIBA Corp.		*/
/*		All Rights Reserved			*/



/*******************************************************************
 *
 *	 Toshiba Floppy Driver for UNIX System V R3
 *
 *					June 21, 1988 
 *
 *	Intended Drive Units:
 *		Worldwide - Model No. ND-356  3.5" unformatted 2MB/1MB
 *		UNIX Media Type Name: 2HD512/2DD512/2D512/1D512.
 *
 *		In Japan Only - Model No. ND-355  3.5" unformatted 1.6MB/1MB
 *		UNIX Media Type Name: 2HC1024/2HC512/2HC256/2DD512/2D512/1D512.
 *
 *		Worldwide - Model No. ND-04DT-A  5.25" unformatted 500 KB
 *		UNIX Media Type Name: 2D512/1D512.
 *
 *		In Japan Only - Model No. ND-08DE  5.25" unformatted 1.6MB/1MB
 *		UNIX Media Type Name: 2HC1024/2HC512/2HC256/2DD512/2D512/1D512.
 *
 *		Use with other devices may require modification.
 *
 *	Notes:
 *		For further detail regarding drive units contact 
 *		Toshiba America,Inc. Disk Products Division,
 *		Irvine, CA (714) 583-3000.
 *
 *******************************************************************/

#include <sys/types.h>
#include <sys/buf.h>
#include <i386/AT386/m765.h>

int	dmause = 0;
int	m765verify[MAXUNIT] = {1,1};	/* 0 != verify mode	*/ 
					/* 0 == not verify mode */
char	fddtype = 0;		/* cmos ram data 10h */
char	fdtype[MAXUNIT];

unsigned char	kbd_FDsts = 0;	/* fd/prt switch data */

int	first_fdopen_ever = 1;	/* flag for first open */
/*
 * Floppy Partitions.
 *
 *  start sector number , max sector number 
 *
 *                              use cyl 0 / not use cyl 0 
*/
struct	fdpart F2h18[] =  { 0, 2880, 36, 2844 };
struct	fdpart F2q26[] =  { 0, 4160, 52, 4108 };
struct	fdpart F2q15[] =  { 0, 2400, 30, 2370 };
struct	fdpart F2q8[]  =  { 0, 1232, 16, 1216 };
struct	fdpart F2w9[]  =  { 0, 1440, 18, 1422 };
struct	fdpart F2w8[]  =  { 0, 1280, 16, 1264 };
struct	fdpart F2d9[]  =  { 0, 720,  18, 702  };
struct	fdpart F2d8[]  =  { 0, 640,  16, 624  };
struct	fdpart F1d9[]  =  { 0, 360,   9, 351  };
struct	fdpart F1d8[]  =  { 0, 320,   8, 312  };

/*
 * Floppy Device-Table Definitions (drtabs)
 *
 *      Cyls,Head,Sec,SecSiz,spc,part,Mtype,RWFpl,FGpl
 */
struct	fddrtab m765f0[FORMMAX] = {
	80, 2,18, 512, 36, F2h18,0x08,0x1b,0x6c, /* [1] */
	 0, 0, 0,   0,  0,     0,   0,   0,   0, /* [2] */
	 0, 0, 0,   0,  0,     0,   0,   0,   0, /* [3] */
	 0, 0, 0,   0,  0,     0,   0,   0,   0, /* [4] */
	80, 2, 9, 512, 18,  F2w9,0x88,0x2a,0x50, /* [5] */
	80, 2, 8, 512, 16,  F2w8,0x88,0x2a,0x50, /* [6] */
	40, 2, 9, 512, 18,  F2d9,0xa8,0x2a,0x50, /* [7] */
	40, 2, 8, 512, 16,  F2d8,0xa8,0x2a,0x50, /* [8] */
	40, 1, 9, 512,  9,  F1d9,0xa8,0x2a,0x50, /* [9] */
	40, 1, 8, 512,  8,  F1d8,0xa8,0x2a,0x50, /*[10] */

	 0, 0, 0,   0,  0,     0,   0,   0,   0, /*[11] */
	 0, 0, 0,   0,  0,     0,   0,   0,   0, /*[12] */
	 0, 0, 0,   0,  0,     0,   0,   0,   0, /*[13] */
	 0, 0, 0,   0,  0,     0,   0,   0,   0  /*[14] */
};
struct	fddrtab m765f1[FORMMAX] = {
	 0, 0, 0,   0,  0,     0,   0,   0,   0, /* [1] */
	80, 2,26, 256, 52, F2q26,0x08,0x0e,0x36, /* [2] */
	80, 2,15, 512, 30, F2q15,0x08,0x1b,0x54, /* [3] */
	77, 2, 8,1024, 16,  F2q8,0x08,0x35,0x74, /* [4] */
	80, 2, 9, 512, 18,  F2w9,0x48,0x2a,0x50, /* [5] */
	80, 2, 8, 512, 16,  F2w8,0x48,0x2a,0x50, /* [6] */
	40, 2, 9, 512, 18,  F2d9,0x68,0x2a,0x50, /* [7] */
	40, 2, 8, 512, 16,  F2d8,0x68,0x2a,0x50, /* [8] */
	40, 1, 9, 512,  9,  F1d9,0x68,0x2a,0x50, /* [9] */
	40, 1, 8, 512,  8,  F1d8,0x68,0x2a,0x50, /*[10] */

	40, 2, 9, 512, 18,  F2d9,0x80,0x2a,0x50, /*[11] */
	40, 2, 8, 512, 16,  F2d8,0x80,0x2a,0x50, /*[12] */
	40, 1, 9, 512,  9,  F1d9,0x80,0x2a,0x50, /*[13] */
	40, 1, 8, 512,  8,  F1d8,0x80,0x2a,0x50  /*[14] */
};

struct	fdcmn m765fdata = { 0xdf,0x02,0x12,0xff };

struct	fdtree fd0tree[]   = { 
   &m765f0[0], 2,0,18,  &fd0tree[1],  &fd0tree[2], /* [ 0] 2hd ?	  */
   &m765f0[0], 0,0, 0,            0,            0, /* [ 1] 2hd media      */

   &m765f0[4], 2,0, 1,  &fd0tree[3],  &fd0tree[7], /* [ 2] 2dd ?	  */
   &m765f0[4], 2,0, 9,  &fd0tree[4],  &fd0tree[5], /* [ 3] 2dd sec9 ?	  */
   &m765f0[4], 0,0, 0,            0,            0, /* [ 4] 2dd sec9 media */
   &m765f0[5], 2,0, 8,  &fd0tree[6], &fd0tree[17], /* [ 5] 2dd sec8 ?	  */
   &m765f0[5], 0,0, 0,            0,            0, /* [ 6] 2dd sec8 media */

   &m765f0[6], 2,1, 1,  &fd0tree[8], &fd0tree[12], /* [ 7] 2d ?	          */
   &m765f0[6], 2,1, 9,  &fd0tree[9], &fd0tree[10], /* [ 8] 2d sec9 ?	  */
   &m765f0[6], 0,0, 0,            0,            0, /* [ 9] 2d sec9 media  */
   &m765f0[7], 2,1, 8, &fd0tree[11], &fd0tree[17], /* [10] 2d sec8 media  */
   &m765f0[7], 0,0, 0,            0,	        0, /* [11] 2d sec8 media  */

   &m765f0[8], 2,0, 1, &fd0tree[13], &fd0tree[17], /* [12] 1d ?	          */
   &m765f0[8], 2,0, 9, &fd0tree[14], &fd0tree[15], /* [13] 1d sec9 ?	  */
   &m765f0[8], 0,0, 0,            0,            0, /* [14] 1d sec9 media  */
   &m765f0[9], 2,0, 8, &fd0tree[16], &fd0tree[17], /* [15] 1d sec8 media  */
   &m765f0[9], 0,0, 0,            0,	        0, /* [16] 1d sec8 media  */
  	    0, 0,0, 0,            0,            0  /* [17] illegal media  */
};
struct	fdtree fd1tree[]   = { 
   &m765f1[1], 2,0,26,  &fd1tree[1],  &fd1tree[2], /* [ 0] 2hc 256 ?	  */
   &m765f1[1], 0,0, 0,		  0,	        0, /* [ 1] 2hc 256 media  */
   &m765f1[2], 2,0,15,  &fd1tree[3],  &fd1tree[4], /* [ 2] 2hc 512 ?      */
   &m765f1[2], 0,0, 0,		  0,	        0, /* [ 3] 2hc 512 media  */
   &m765f1[3], 2,0, 8,  &fd1tree[5],  &fd1tree[6], /* [ 4] 2hc 1024 ?     */
   &m765f1[3], 0,0, 0,  	  0, 	        0, /* [ 5] 2hc 1024 media */

   &m765f1[4], 2,0, 1,  &fd1tree[7], &fd1tree[11], /* [ 6] 2dd ?	  */
   &m765f1[4], 2,0, 9,  &fd1tree[8],  &fd1tree[9], /* [ 7] 2dd sec9 ?	  */
   &m765f1[4], 0,0, 0,		  0,	        0, /* [ 8] 2dd sec9 media */
   &m765f1[5], 2,0, 8, &fd1tree[10], &fd1tree[21], /* [ 9] 2dd sec8 ?     */
   &m765f1[5], 0,0, 0, 		  0,	        0, /* [10] 2dd sec8 media */

   &m765f1[6], 2,1, 1, &fd1tree[12], &fd1tree[16], /* [11] 2d ?	          */
   &m765f1[6], 2,1, 9, &fd1tree[13], &fd1tree[14], /* [12] 2d sec9 ?	  */
   &m765f1[6], 0,0, 0,		  0,	        0, /* [13] 2d sec9 media  */
   &m765f1[7], 2,1, 8, &fd1tree[15], &fd1tree[21], /* [14] 2d sec8 ?	  */
   &m765f1[7], 0,0, 0, 		  0,	        0, /* [15] 2d sec8 media  */

   &m765f1[8], 2,0, 1, &fd1tree[17], &fd1tree[21], /* [16] 1d ?	          */
   &m765f1[8], 2,0, 9, &fd1tree[18], &fd1tree[19], /* [17] 1d sec9 ?	  */
   &m765f1[8], 0,0, 0,		  0,	        0, /* [18] 1d sec9 media  */
   &m765f1[9], 2,0, 8, &fd1tree[20], &fd1tree[21], /* [19] 1d sec8 ?	  */
   &m765f1[9], 0,0, 0, 		  0,	        0, /* [20] 1d sec8 media  */
	    0, 0,0, 0,		  0,	        0  /* [21] illegal media  */
};
struct	fdtree fd2tree[]   = { 
  &m765f1[10], 2,1, 1,  &fd2tree[1],  &fd2tree[5], /* [ 0] 2d ?	          */
  &m765f1[10], 2,1, 9,  &fd2tree[2],  &fd2tree[3], /* [ 1] 2d sec9 ?	  */
  &m765f1[10], 0,0, 0,		  0,	        0, /* [ 2] 2d sec9 media  */
  &m765f1[11], 2,1, 8,  &fd2tree[4], &fd2tree[10], /* [ 3] 2d sec8 ?	  */
  &m765f1[11], 0,0, 0, 		  0,	        0, /* [ 4] 2d sec8 media  */

  &m765f1[12], 2,0, 1,  &fd2tree[6], &fd2tree[10], /* [ 5] 1d ?	          */
  &m765f1[12], 2,0, 9,  &fd2tree[7],  &fd2tree[8], /* [ 6] 1d sec9 ?	  */
  &m765f1[12], 0,0, 0,		  0,	        0, /* [ 7] 1d sec9 media  */
  &m765f1[13], 2,0, 8,  &fd2tree[9], &fd2tree[10], /* [ 8] 1d sec8 ?	  */
  &m765f1[13], 0,0, 0, 		  0,	        0, /* [ 9] 1d sec8 media  */
	    0, 0,0, 0,		  0,	        0  /* [10] illegal media  */
};

struct fdpart m765part[MAXUNIT*2] = {		/* Floppy partitions data */
	0, 2880, 36, 2844,
	0, 2400, 30, 2370
};
/*
 * The following are static initialization variables
 * which are based on the configuration. These variables
 * MUST NOT CHANGE because the m765 device driver makes
 * most of the calculations based on these variables.
*/
struct fddrtab fd3_5 = {
	80, 2,18, 512, 0,&m765part[0],0x08,0x1b,0x6c,
};
struct fddrtab fd5_25 = {
	80, 2,15, 512, 0,&m765part[2],0x08,0x1b,0x54,
};

struct fddev  m765dev;

struct fdubuf unitbuf[MAXUNIT];	/* unit buffer headers	*/

char *fderr = "FD Error on unit";
char *fdmsg[] = {
	"?",
	"Missing data address mark",
	"Write protected",
	"Sector not found",
	"Data Overrun",				/* Over run error */
	"Uncorrectable data read error",	/* CRC Error */
	"FDC Error",
	"Illegal format type",
	"Drive not ready",
	"diskette not present - please insert",
	"Illegal interrupt type"
};

struct buf	fdrbuf[MAXUNIT];	/* data transfer buffer structures */

