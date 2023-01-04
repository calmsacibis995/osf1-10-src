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
/* $XConsortium: IntrinsicI.h,v 1.48 91/06/27 13:24:18 converse Exp $ */

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

#ifndef _XtintrinsicI_h
#define _XtintrinsicI_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include "Xtos.h"
#include "IntrinsicP.h"
#include <X11/Xos.h>

#include "Object.h"
#include "RectObj.h"
#include "ObjectP.h"
#include "RectObjP.h"

#include "TranslateI.h"
#include "CallbackI.h"
#include "ConvertI.h"
#include "EventI.h"
#include "PassivGraI.h"
#include "InitialI.h"
#include "ResourceI.h"

#define RectObjClassFlag	0x02
#define WidgetClassFlag		0x04
#define CompositeClassFlag	0x08
#define ConstraintClassFlag	0x10
#define ShellClassFlag		0x20
#define WMShellClassFlag	0x40
#define TopLevelClassFlag	0x80

/*
 * The following macros, though very handy, are not suitable for
 * IntrinsicP.h as they violate the rule that arguments are to
 * be evaluated exactly once.
 */

#define XtDisplayOfObject(object) \
    ((XtIsWidget(object) ? (object) : _XtWindowedAncestor(object)) \
     ->core.screen->display)

#define XtScreenOfObject(object) \
    ((XtIsWidget(object) ? (object) : _XtWindowedAncestor(object)) \
     ->core.screen)

#define XtWindowOfObject(object) \
    ((XtIsWidget(object) ? (object) : _XtWindowedAncestor(object)) \
     ->core.window)

#define XtIsManaged(object) \
    (XtIsRectObj(object) ? (object)->core.managed : False)

#define XtIsSensitive(object) \
    (XtIsRectObj(object) ? ((object)->core.sensitive && \
			    (object)->core.ancestor_sensitive) : False)


/****************************************************************
 *
 * Byte utilities
 *
 ****************************************************************/

#define _XBCOPYFUNC _XtBCopy
#include <X11/Xfuncs.h>

/* If the alignment characteristics of your machine are right, these may be
   faster */

#ifdef UNALIGNED

#define XtBCopy(src, dst, size)				    \
    if (size == sizeof(int))				    \
	*((int *) (dst)) = *((int *) (src));		    \
    else if (size == sizeof(char))			    \
	*((char *) (dst)) = *((char *) (src));		    \
    else if (size == sizeof(short))			    \
	*((short *) (dst)) = *((short *) (src));	    \
    else						    \
	bcopy((char *) (src), (char *) (dst), (int) (size));

#define XtBZero(dst, size)				    \
    if (size == sizeof(int))				    \
	*((int *) (dst)) = 0;				    \
    else						    \
	bzero((char *) (dst), (int) (size));

#define XtBCmp(b1, b2, size)				    \
    (size == sizeof(int) ?				    \
	*((int *) (b1)) != *((int *) (b2))		    \
    :   bcmp((char *) (b1), (char *) (b2), (int) (size))    \
    )

#else

#define XtBCopy(src, dst, size)		\
	bcopy((char *) (src), (char *) (dst), (int) (size));

#define XtBZero(dst, size) bzero((char *) (dst), (int) (size));

#define XtBCmp(b1, b2, size) bcmp((char *) (b1), (char *) (b2), (int) (size))

#endif


/****************************************************************
 *
 * Stack cache allocation/free
 *
 ****************************************************************/

#define XtStackAlloc(size, stack_cache_array)     \
    ((size) <= sizeof(stack_cache_array)	  \
    ?  (XtPointer)(stack_cache_array)		  \
    :  XtMalloc((unsigned)(size)))

#define XtStackFree(pointer, stack_cache_array) \
    if ((pointer) != ((XtPointer)(stack_cache_array))) XtFree(pointer); else

/***************************************************************
 *
 * Filename defines
 *
 **************************************************************/

/* used by XtResolvePathname */
#ifndef XFILESEARCHPATHDEFAULT
#ifdef VMS
#define XFILESEARCHPATHDEFAULT "%T%N%S"
#else
#ifdef I18N_BUG_FIX
#define XFILESEARCHPATHDEFAULT "/usr/lib/X11/%L/%T/%N%S:/usr/lib/X11/%l_%t/%T/%N%S:/usr/lib/X11/%l/%T/%N%S:/usr/lib/X11/%T/%N%S"
#else
#define XFILESEARCHPATHDEFAULT "/usr/lib/X11/%L/%T/%N%S:/usr/lib/X11/%l/%T/%N%S:/usr/lib/X11/%T/%N%S"
#endif
#endif
#endif

/* the following two were both "X Toolkit " prior to R4 */

#ifdef DEC_EXTENSION
#define XTERROR_PREFIX "X Toolkit "
#define XTWARNING_PREFIX "X Toolkit "
#endif

#ifndef XTERROR_PREFIX
#define XTERROR_PREFIX ""
#endif

#ifndef XTWARNING_PREFIX
#define XTWARNING_PREFIX ""
#endif

#ifndef ERRORDB
#ifdef VMS
#define ERRORDB "sys$message:decw$dwterrdb.dat"
#else
#define ERRORDB "/usr/lib/X11/XtErrorDB"
#endif
#endif

extern String XtCXtToolkitError;

extern void _XtAllocError(
#if NeedFunctionPrototypes
    String	/* alloc_type */
#endif
);

extern void _XtCompileResourceList(
#if NeedFunctionPrototypes
    XtResourceList 	/* resources */,
    Cardinal 		/* num_resources */
#endif
);

extern XtGeometryResult _XtMakeGeometryRequest(
#if NeedFunctionPrototypes
    Widget 		/* widget */,
    XtWidgetGeometry*	/* request */,
    XtWidgetGeometry*	/* reply_return */,
    Boolean*		/* clear_rect_obj */
#endif
);

#if !defined(XtFree) && defined(DEC_EXTENSION) && !defined(XTTRACEMEMORY) && defined(VMS)
/*
 *  Define an XtFree macro for performance reasons.  This must
 *  be the same as the XtFree routine in Alloc.c
 */
#define XtFree(ptr) \
 if (1) { char* _p_; if ((_p_ = (ptr)) != NULL) lib$vm_free((_p_)); } else
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XtintrinsicI_h */
/* DON'T ADD STUFF AFTER THIS #endif */
