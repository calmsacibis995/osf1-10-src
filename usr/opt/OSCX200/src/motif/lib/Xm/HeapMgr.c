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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/Xm/HeapMgr.c,v 1.1.2.2 92/04/01 14:58:53 Russ_Kuhn Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)HeapMgr.c	3.7 91/01/10";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1990, HEWLETT-PACKARD COMPANY
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
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
#ifdef DEC_MOTIF_BUG_FIX
#include "XmI.h"
#else
#include <Xm/XmI.h>
#endif

XmHeap _XmHeapCreate(segment_size)
    int	segment_size;
{
    XmHeap	heap;

    heap = XtNew(XmHeapRec);
    heap->start = NULL;
    heap->segment_size = segment_size;
    heap->bytes_remaining = 0;
    return heap;
}

char* _XmHeapAlloc(heap, bytes)
    XmHeap	heap;
    Cardinal	bytes;
{
    register char* heap_loc;
    if (heap == NULL) return XtMalloc(bytes);
    if (heap->bytes_remaining < bytes) {
	if ((bytes + sizeof(char*)) >= (heap->segment_size>>1)) {
	    /* preserve current segment; insert this one in front */
#ifdef _TRACE_HEAP
	    printf( "allocating large segment (%d bytes) on heap %#x\n",
		    bytes, heap );
#endif
	    heap_loc = XtMalloc(bytes + sizeof(char*));
	    if (heap->start) {
		*(char**)heap_loc = *(char**)heap->start;
		*(char**)heap->start = heap_loc;
	    }
	    else {
		*(char**)heap_loc = NULL;
		heap->start = heap_loc;
	    }
	    return heap_loc;
	}
	/* else discard remainder of this segment */
#ifdef _TRACE_HEAP
	printf( "allocating new segment on heap %#x\n", heap );
#endif
	heap_loc = XtMalloc((unsigned)heap->segment_size);
	*(char**)heap_loc = heap->start;
	heap->start = heap_loc;
	heap->current = heap_loc + sizeof(char*);
	heap->bytes_remaining = heap->segment_size - sizeof(char*);
    }
#ifdef WORD64
    /* round to nearest 8-byte boundary */
    bytes = (bytes + 7) & (~7);
#else
    /* round to nearest 4-byte boundary */
    bytes = (bytes + 3) & (~3);
#endif /* WORD64 */
    heap_loc = heap->current;
    heap->current += bytes;
    heap->bytes_remaining -= bytes; /* can be negative, if rounded */
    return heap_loc;
}

void _XmHeapFree(heap)
    XmHeap	heap;
{
    char* segment = heap->start;
    while (segment != NULL) {
	char* next_segment = *(char**)segment;
	XtFree(segment);
	segment = next_segment;
    }
    heap->start = NULL;
    heap->bytes_remaining = 0;
}

