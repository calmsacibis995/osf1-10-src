/************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**/

/************************************************************************
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	Header file containg bitmasks for help in C written bit-twidling 
**	routines.
**
**  ENVIRONMENT:
**
**	Ultrix V3.0
**
**  AUTHOR(S):
**
**	Written by Brian M. Stevens  27-Sep-1988
**
**	Clean-up and put in library by Michael D. O'Connor  5-Dec-1988
**
**  CREATION DATE:
**
**	5-Dec-1988
**
************************************************************************/

/*
 * Bit masks used by IPS$$FFS_LONG, IPS$$FFC_LONG, IPS$$BUILD_CHANGELIST,
 * IPS$$PUT_RUNS, IPS$$MOVV5
 *
 */
#ifndef BYTE_SIZE
#define BYTE_SIZE	8	/* number of bits per byte		      */
#define LONG_SIZE	32	/* number of bits per longword		      */
#endif

/*
 * these masks protect regions of the destination buffer (used in IpsMovv5)
 * from being destroyed when copying
 */

static
int lo_mask[] = {	0xffffffff,
			0xfffffffe,
			0xfffffffc,
			0xfffffff8,
			0xfffffff0,
			0xffffffe0,
			0xffffffc0,
			0xffffff80,
			0xffffff00,
			0xfffffe00,
			0xfffffc00,
			0xfffff800,
			0xfffff000,
			0xffffe000,
			0xffffc000,
			0xffff8000,
			0xffff0000,
			0xfffe0000,
			0xfffc0000,
			0xfff80000,
			0xfff00000,
			0xffe00000,
			0xffc00000,
			0xff800000,
			0xff000000,
			0xfe000000,
			0xfc000000,
			0xf8000000,
			0xf0000000,
			0xe0000000,
			0xc0000000,
			0x80000000,
			0x00000000};

static
int hi_mask[] = {	0x00000000,
			0x00000001,
			0x00000003,
			0x00000007,
			0x0000000f,
			0x0000001f,
			0x0000003f,
			0x0000007f,
			0x000000ff,
			0x000001ff,
			0x000003ff,
			0x000007ff,
			0x00000fff,
			0x00001fff,
			0x00003fff,
			0x00007fff,
			0x0000ffff,
			0x0001ffff,
			0x0003ffff,
			0x0007ffff,
			0x000fffff,
			0x001fffff,
			0x003fffff,
			0x007fffff,
			0x00ffffff,
			0x01ffffff,
			0x03ffffff,
			0x07ffffff,
			0x0fffffff,
			0x1fffffff,
			0x3fffffff,
			0x7fffffff,
			0xffffffff};
