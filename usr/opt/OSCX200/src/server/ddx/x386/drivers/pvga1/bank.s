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
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 *
 * $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/x386/drivers/pvga1/bank.s,v 1.2 91/12/15 12:42:16 devrcs Exp $
 */

/*
 * These are here the very lowlevel VGA bankswitching routines.
 * The segment to switch to is passed via %eax. Only %eax and %edx my be used
 * without saving the original contents.
 *
 * WHY ASSEMBLY LANGUAGE ???
 *
 * These routines must be callable by other assembly routines. But I don't
 * want to have the overhead of pushing and poping the normal stack-frame.
 */

/*
 * what happens really here ?
 *
 * PRA and PRB are segmentpointers to out two segments. They have a granularity
 * of 4096. That means we have to multiply the segmentnumber by 8, if we are
 * working with 32k segments. But since PRA and PRB are 'indexed' registers,
 * the index must be emitted first. This is accomplished by loading %al with
 * the index and %ah with the value. Therefor we must shift the logical
 * segmentnumber by 11.
 *
 * Another quirk is PRA. It's physical VGA mapping starts at 0xA0000, but it is
 * only visible starting form 0xA8000 to 0xAFFFF. That means PRA has to be
 * loaded with a value that points to the previous logical segment.
 *
 * The latter FEATURE was mentioned correctly (but somewhat not understandable)
 * in the registerdoc of the PVGA1A. But it was COMPLETELY WRONG shown in their
 * programming examples....
 */

	.text

/* 
 * for ReadWrite operations, we are using only PR0B as pointer to a 32k
 * window.
 */
	.align 4
	.globl PVGA1SetReadWrite
PVGA1SetReadWrite:
	shll	$11,%eax            /* combined %al*8 & movb %al,%ah */
	movb	$10,%al
	movl	$0x3CE,%edx
	outw	(%dx)
	ret

/* 
 * for Write operations, we are using PR0B as write pointer to a 32k
 * window.
 */
	.align 4
	.globl PVGA1SetWrite
PVGA1SetWrite:
	shll	$11,%eax
	movb	$10,%al
	movl	$0x3CE,%edx
	outw	(%dx)
	ret

/* 
 * for Read operations, we are using PR0A as read pointer to a 32k
 * window.
 */
	.align 4
	.globl PVGA1SetRead
PVGA1SetRead:
	decl	%eax             /* segment wrap ... */
	shll	$11,%eax        
	movb	$9,%al
	movl	$0x3CE,%edx
	outw	(%dx)
	ret



