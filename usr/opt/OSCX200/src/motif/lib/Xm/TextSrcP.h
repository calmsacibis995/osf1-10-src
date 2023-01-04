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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/Xm/TextSrcP.h,v 1.1.2.2 92/04/01 15:17:59 Russ_Kuhn Exp $ */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)TextSrcP.h	3.8 91/01/10";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

#ifndef _XmTextSrcP_h
#define _XmTextSrcP_h

#include <ctype.h>

/****************************************************************
 *
 * Definitions for use by sources and source users.
 *
 ****************************************************************/

typedef enum {EditDone, EditError, EditReject} XmTextStatus;

typedef enum {XmsdLeft, XmsdRight} XmTextScanDirection;
    
typedef struct _XmSourceDataRec {
    XmTextSource source;	/* Backpointer to source record. */
    char *ptr;			/* Actual string data. */
    char *value;		/* Value of the string data. */
    char *gap_start;		/* Gapped buffer start pointer */
    char *gap_end;		/* Gapped buffer end pointer */
    XmTextWidget *widgets;	/* Array of widgets displaying this source. */
    int length;			/* Number of chars of data. */
    int maxlength;		/* Space allocated. */
    int old_length;		/* Space allocated for value pointer. */
    int numwidgets;		/* Number of entries in above. */
    int maxallowed;		/* The user is not allowed to grow source */
				/* to a size greater than this. */
    Boolean hasselection;	/* Whether we own the selection. */
    Boolean editable;		/* Whether we allow any edits. */
    XmTextPosition left, right; /* Left and right extents of selection. */
    Time prim_time;             /* time of primary selection */
#ifdef DEC_MOTIF_EXTENSION
    XmTextPosition opaque1, opaque2;
#endif
} XmSourceDataRec, *XmSourceData;

#ifdef _NO_PROTO
typedef void (*AddWidgetProc)(); /* source, data */
    /* XmTextSource source; */
    /* Text data; */

typedef int (*CountLinesProc)(); /* source, start, length */
    /* XmTextSource source; */
    /* XmTextPosition start; */
    /* int length; */

typedef void (*RemoveWidgetProc)(); /* source, data */
    /* XmTextSource source; */
    /* Text data; */

typedef XmTextPosition (*ReadProc)(); /* source, pos, lastPos, block */
    /* XmTextSource source; */
    /* XmTextPosition pos; */	/* starting position */
    /* XmTextPosition lastPos; */ /* The last position we're interested in.
       				     Don't return info about any later
				     positions. */
    /* XmTextBlock block; */	/* RETURN: text read in */

typedef XmTextStatus (*ReplaceProc)(); /* source, start, end, block */
    /* XmTextSource source; */
    /* XmTextPosition start, end; */
    /* XmTextBlock block; */

typedef XmTextPosition (*ScanProc)(); /* source, pos, sType, dir, n, include */
    /* XmTextSource source; */
    /* XmTextPosition pos; */
    /* XmTextScanType sType; */
    /* XmTextScanDirection dir; */  /* Either XmsdLeft or XmsdRight. */
    /* int n; */
    /* Boolean include; */

typedef Boolean (*GetSelectionProc)(); /* source, left, right */
    /* XmTextSource source; */
    /* XmTextPosition *left, *right; */ 

typedef void (*SetSelectionProc)(); /* source, left, right, time */
    /* XmTextSource source; */
    /* XmTextPosition left, right; */
    /* Time time; */

#else /* _NO_PROTO */

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef void (*AddWidgetProc)(
	XmTextSource source, XmTextWidget data
    );
typedef int (*CountLinesProc)(
	XmTextSource source, XmTextPosition start, int length
    );
typedef void (*RemoveWidgetProc)(
	XmTextSource source, XmTextWidget data
    );
typedef XmTextPosition (*ReadProc)(
	XmTextSource source, XmTextPosition pos, XmTextPosition lastPos,
	XmTextBlock block
    );
typedef XmTextStatus (*ReplaceProc)(
	XmTextSource source, XEvent *event,
	XmTextPosition start, XmTextPosition end,
	XmTextBlock block
    );
typedef XmTextPosition (*ScanProc)(
	XmTextSource source,
	XmTextPosition pos,
	XmTextScanType sType,
	XmTextScanDirection dir,
	int n,
	Boolean include
    );
typedef Boolean (*GetSelectionProc)(
	XmTextSource source,
	XmTextPosition *left,
	XmTextPosition *right
    );
typedef void (*SetSelectionProc)(
	XmTextSource source,
	XmTextPosition left,
	XmTextPosition right,
	Time time
    );

#if defined(__cplusplus) || defined(c_plusplus)
};
#endif
#endif /* else _NO_PROTO */
  
 
typedef struct _XmTextSourceRec {
    struct _XmSourceDataRec *data;   /* Source-defined data (opaque type). */
    AddWidgetProc	AddWidget;
    CountLinesProc	CountLines;
    RemoveWidgetProc	RemoveWidget;
    ReadProc		ReadSource;
    ReplaceProc		Replace;
    ScanProc		Scan;
    GetSelectionProc	GetSelection;
    SetSelectionProc	SetSelection;
} XmTextSourceRec;

#endif /* _XmTextSrcP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
