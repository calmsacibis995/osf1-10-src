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
* $XConsortium: StripCharP.h,v 1.4 90/10/22 14:38:15 converse Exp $
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

#ifndef _XawStripChartP_h
#define _XawStripChartP_h

#include <X11/Xaw/StripChart.h>
#include <X11/Xaw/SimpleP.h>

#define NO_GCS 0
#define FOREGROUND 1 << 0
#define HIGHLIGHT  1 << 1
#define ALL_GCS (FOREGROUND | HIGHLIGHT)

/* New fields for the stripChart widget instance record */

typedef struct {
    Pixel	fgpixel;	/* color index for graph */
    Pixel	hipixel;	/* color index for lines */
    GC	fgGC;		/* graphics context for fgpixel */
    GC	hiGC;		/* graphics context for hipixel */
    
    /* start of graph stuff */
    
    int	update;		/* update frequence */
    int	scale;		/* scale factor */
    int	min_scale;	/* smallest scale factor */
    int	interval;	/* data point interval */
    XPoint * points ;	/* Poly point for repairing graph lines. */
    double max_value;	/* Max Value in window */
    double valuedata[2048];/* record of data points */
    XtIntervalId interval_id;
    XtCallbackList get_value; /* proc to call to fetch load pt */
    int jump_val;		/* Amount to jump on each scroll. */
} StripChartPart;

/* Full instance record declaration */
typedef struct _StripChartRec {
   CorePart core;
   SimplePart simple;
   StripChartPart strip_chart;
} StripChartRec;

/* New fields for the StripChart widget class record */
typedef struct {int dummy;} StripChartClassPart;

/* Full class record declaration. */
typedef struct _StripChartClassRec {
   CoreClassPart core_class;
   SimpleClassPart simple_class;
   StripChartClassPart strip_chart_class;
} StripChartClassRec;

/* Class pointer. */
extern StripChartClassRec stripChartClassRec;

#endif /* _XawStripChartP_h */
