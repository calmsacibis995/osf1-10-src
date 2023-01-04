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
 * $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/x386/compiler.h,v 1.2 91/12/15 12:42:16 devrcs Exp $
 */


#ifndef _COMPILER_H
#define _COMPILER_H

#ifdef __GNUC__


static __inline__ void
outb(port, val)
     short port;
     char val;
{
  __asm__ volatile("out%B0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ void
outw(port, val)
     short port;
     short val;
{
  __asm__ volatile("out%W0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
inb(port)
     short port;
{
  unsigned int ret;
  __asm__ volatile("in%B0 (%1)" :
		   "=a" (ret) :
		   "d" (port));
  return ret;
}

static __inline__ unsigned int
inw(port)
     short port;
{
  unsigned int ret;
  __asm__ volatile("in%W0 (%1)" :
		   "=a" (ret) :
		   "d" (port));
  return ret;
}

static __inline__ void
intr_disable()
{
  __asm__ volatile("cli");
}

static __inline__ void
intr_enable()
{
  __asm__ volatile("sti");
}


#else /* __GNUC__ */
# if defined(__STDC__) && (__STDC__ == 1)
#  define asm __asm
# endif
#include <sys/inline.h>
#define intr_disable() asm("cli")
#define intr_enable()  asm("sti")
#endif

#ifndef __STDC__
#define signed /**/
#define const /**/
#endif

#endif /* _COMPILER_H */
