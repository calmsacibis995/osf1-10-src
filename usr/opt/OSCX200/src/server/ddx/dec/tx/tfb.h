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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#ifndef TFB_H
#define TFB_H

#define tfbDrawSetFb24 xfbDrawSetFb24

#define tfbPrivGCPtr cfbPrivGCPtr
#define tfbWindowPrivateIndex cfbWindowPrivateIndex
#define tfbGCPrivateIndex cfbGCPrivateIndex

#define tfbScreenPrivateIndex cfb32ScreenPrivateIndex

#define tfbDoBitbltCopy cfb32DoBitbltCopy
#define tfbDoBitblt cfb32DoBitblt
#define tfbSaveAreas cfb32SaveAreas
#define tfbRestoreAreas cfb32RestoreAreas
#define tfbGetImage cfb32GetImage
#define tfbGetSpans cfb32GetSpans
#define tfbCreateGC cfb32CreateGC
#define tfbDrawClose cfb32DrawClose
#define tfbDrawInit cfb32DrawInit
#define tfbCreateWindow cfb32CreateWindow
#define tfbDestroyWindow cfb32DestroyWindow
#define tfbMapWindow cfb32MapWindow
#define tfbPositionWindow cfb32PositionWindow
#define tfbUnmapWindow cfb32UnmapWindow
#define tfbCopyWindow cfb32CopyWindow
#define tfbChangeWindowAttributes cfb32ChangeWindowAttributes
#define tfbPaintWindow cfb32PaintWindow

#endif TFB_H
