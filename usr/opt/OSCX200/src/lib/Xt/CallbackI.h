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
/* $XConsortium: CallbackI.h,v 1.13 90/12/29 12:12:48 rws Exp $ */
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

#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

/****************************************************************
 *
 * Callbacks
 *
 ****************************************************************/

typedef XrmResource **CallbackTable;

#define _XtCBCalling 1
#define _XtCBFreeAfterCalling 2

typedef struct internalCallbackRec {
    unsigned short count;
    char	   is_padded;	/* contains NULL padding for external form */
    char	   call_state;  /* combination of _XtCB{FreeAfter}Calling */
#ifdef DEC_BUG_FIX	/* 64 bit change */
    char *align;	/* assure alignment of the composite when CallbackRec is appended */
#endif
    /* XtCallbackList */
} InternalCallbackRec, *InternalCallbackList;

extern void _XtAddCallback(
#if NeedFunctionPrototypes
    InternalCallbackList*	/* callbacks */,
    XtCallbackProc		/* callback */,
    XtPointer 			/* closure */
#endif
);

extern void _XtAddCallbackOnce(
#if NeedFunctionPrototypes
    InternalCallbackList*	/* callbacks */,
    XtCallbackProc		/* callback */,
    XtPointer 			/* closure */
#endif
);

extern InternalCallbackList _XtCompileCallbackList(
#if NeedFunctionPrototypes
    XtCallbackList		/* xtcallbacks */
#endif
);

extern XtCallbackList _XtGetCallbackList(
#if NeedFunctionPrototypes
    InternalCallbackList*	/* callbacks */
#endif
);

extern void _XtRemoveAllCallbacks(
#if NeedFunctionPrototypes
    InternalCallbackList*	/* callbacks */
#endif
);

extern void _XtRemoveCallback(
#if NeedFunctionPrototypes
    InternalCallbackList*	/* callbacks */,
    XtCallbackProc		/* callback */,
    XtPointer			/* closure */
#endif
);

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
