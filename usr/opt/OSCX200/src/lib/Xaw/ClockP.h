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
* $XConsortium: ClockP.h,v 1.21 90/10/22 14:43:22 converse Exp $
*/


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#ifndef _XawClockP_h
#define _XawClockP_h

#include <X11/Xos.h>		/* Needed for struct tm. */
#include <X11/Xaw/Clock.h>
#include <X11/Xaw/SimpleP.h>

#define SEG_BUFF_SIZE		128
#define ASCII_TIME_BUFLEN	32	/* big enough for 26 plus slop */

/* New fields for the clock widget instance record */
typedef struct {
	 Pixel	fgpixel;	/* color index for text */
	 Pixel	Hipixel;	/* color index for Highlighting */
	 Pixel	Hdpixel;	/* color index for hands */
	 XFontStruct	*font;	/* font for text */
	 GC	myGC;		/* pointer to GraphicsContext */
	 GC	EraseGC;	/* eraser GC */
	 GC	HandGC;		/* Hand GC */
	 GC	HighGC;		/* Highlighting GC */
/* start of graph stuff */
	 int	update;		/* update frequence */
	 Dimension radius;		/* radius factor */
	 int	backing_store;	/* backing store type */
	 Boolean chime;
	 Boolean beeped;
	 Boolean analog;
	 Boolean show_second_hand;
	 Dimension second_hand_length;
	 Dimension minute_hand_length;
	 Dimension hour_hand_length;
	 Dimension hand_width;
	 Dimension second_hand_width;
	 Position centerX;
	 Position centerY;
	 int	numseg;
	 int	padding;
	 XPoint	segbuff[SEG_BUFF_SIZE];
	 XPoint	*segbuffptr;
	 XPoint	*hour, *sec;
	 struct tm  otm ;
	 XtIntervalId interval_id;
	 char prev_time_string[ASCII_TIME_BUFLEN];
   } ClockPart;

/* Full instance record declaration */
typedef struct _ClockRec {
   CorePart core;
   SimplePart simple;
   ClockPart clock;
   } ClockRec;

/* New fields for the Clock widget class record */
typedef struct {int dummy;} ClockClassPart;

/* Full class record declaration. */
typedef struct _ClockClassRec {
   CoreClassPart core_class;
   SimpleClassPart simple_class;
   ClockClassPart clock_class;
   } ClockClassRec;

/* Class pointer. */
extern ClockClassRec clockClassRec;

#endif /* _XawClockP_h */
