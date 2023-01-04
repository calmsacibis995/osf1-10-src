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
/*******************************************************************************
**  Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**  
**                          All Rights Reserved
**  
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**  
**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
**  SOFTWARE.
**  
*******************************************************************************/

/************************************************************************
**
**  FACILITY:
**
**      X Image Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**	Header file containg bitmasks for help in C written bit-twidling 
**	routines.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**	Bernardo Tagariello
**
**  CREATION DATE:
**
**	24-May-1989
**
************************************************************************/

/*
** Bit masks used by  MiPutRuns
**
*/

#define BYTE_SIZE	8	/* number of bits per byte		      */
#define LONG_SIZE	32	/* number of bits per longword		      */

/*
 * these masks protect regions of the destination buffer (used in ImgMovv5)
 * from being destroyed when copying
 */

static
long int lo_mask[] = {	0xffffffff,
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
long int hi_mask[] = {	0x00000000,
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
