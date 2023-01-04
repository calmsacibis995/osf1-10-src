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
 * $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/x386/vga/vgaBank.h,v 1.2 91/12/15 12:42:16 devrcs Exp $
 */

#ifndef VGA_BANK_H
#define VGA_BANK_H

extern void *vgaReadBottom;
extern void *vgaReadTop;
extern void *vgaWriteBottom;
extern void *vgaWriteTop;
extern Bool vgaReadFlag, vgaWriteFlag;
extern void *writeseg;

extern void * vgaSetReadWrite();
extern void * vgaReadWriteNext();
extern void * vgaReadWritePrev();
extern void * vgaSetRead();
extern void * vgaReadNext();
extern void * vgaReadPrev();
extern void * vgaSetWrite();
extern void * vgaWriteNext();
extern void * vgaWritePrev();
extern void vgaSaveBank();
extern void vgaRestoreBank();
extern void vgaPushRead();
extern void vgaPopRead();

#define VGABASE 0xF0000000

#define BANK_FLAG(a) \
  vgaWriteFlag = (((unsigned long)a >= VGABASE) ? TRUE : FALSE); \
  vgaReadFlag = FALSE;

#define BANK_FLAG_BOTH(a,b) \
  vgaReadFlag = (((unsigned long)a >= VGABASE) ? TRUE : FALSE); \
  vgaWriteFlag  = (((unsigned long)b >= VGABASE) ? TRUE : FALSE);

#define SETR(x)  { if(vgaReadFlag) x = vgaSetRead(x); }
#define SETW(x)  { if(vgaWriteFlag) x = vgaSetWrite(x); }
#define SETRW(x) { if(vgaWriteFlag) x = vgaSetReadWrite(x); }
#define CHECKRO(x) { if(vgaReadFlag && ((void *)x >= vgaReadTop)) \
			 x = vgaReadNext(x); }
#define CHECKRU(x) { if(vgaReadFlag && ((void *)x < vgaReadBottom)) \
			 x = vgaReadPrev(x); }
#define CHECKWO(x) { if(vgaWriteFlag && ((void *)x >= vgaWriteTop)) \
			 x = vgaWriteNext(x); }
#define CHECKWU(x) { if(vgaWriteFlag && ((void *)x < vgaWriteBottom)) \
			 x = vgaWritePrev(x); }
#define CHECKRWO(x) { if(vgaWriteFlag && ((void *)x >= vgaWriteTop)) \
			  x = vgaReadWriteNext(x); }
#define CHECKRWU(x) { if(vgaWriteFlag && ((void *)x < vgaWriteBottom)) \
			  x = vgaReadWritePrev(x); }

#define NEXTR(x) { x = vgaReadNext(x);}
#define NEXTW(x) { x = vgaWriteNext(x); }
#define PREVR(x) { x = vgaReadPrev(x); }
#define PREVW(x) { x = vgaWritePrev(x); }

#define SAVE_BANK()     { if (vgaWriteFlag) vgaSaveBank(); }
#define RESTORE_BANK()  { if (vgaWriteFlag) vgaRestoreBank(); }

#define PUSHR()         { if(vgaWriteFlag) vgaPushRead(); }
#define POPR()          { if(vgaWriteFlag) vgaPopRead(); }


#endif /* VGA_BANK_H */
