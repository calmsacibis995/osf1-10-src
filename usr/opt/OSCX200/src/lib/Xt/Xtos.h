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
* $XConsortium: Xtos.h,v 1.11 91/11/08 17:55:15 gildea Exp $
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

#ifndef _Xtos_h
#define _Xtos_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef INCLUDE_ALLOCA_H
#include <alloca.h>
#endif

#ifdef CRAY
#define WORD64
#endif

/* stolen from server/include/os.h */
#ifdef VMS
/*
* define allocate_local and deallocate_local as macros for small allocations
* using and reusing a single static per thread data area. If the toolkit
* is made multithread someday then either these statics should be in per
* thread space or the _XtAllocBusy flag use must be made thread safe.
*/
#define _XTALLOC_STACK_SIZE 512
#ifdef INTRINSIC_C
#if defined(VAXC) && !defined(__DECC)
_align(QUADWORD) globaldef {"_xtalloclocalarray"} noshare char _XtAllocLocalArray[_XTALLOC_STACK_SIZE];
globaldef {"_xtallocbusy"} noshare long _XtAllocBusy = 0;
#else
char _align(QUADWORD) _XtAllocLocalArray[_XTALLOC_STACK_SIZE];
long _XtAllocBusy = 0;
#endif
#else
#if defined(VAXC) && !defined(__DECC)
globalref char _XtAllocLocalArray[_XTALLOC_STACK_SIZE];
globalref long _XtAllocBusy;
#else
extern char _XtAllocLocalArray[_XTALLOC_STACK_SIZE];
extern long _XtAllocBusy;
#endif
#endif

#define ALLOCATE_LOCAL(size) \
    ((((size) <= _XTALLOC_STACK_SIZE) && (_XtAllocBusy == 0)) ? \
		(_XtAllocBusy = 1, (char *) &_XtAllocLocalArray) : \
		((char *) XtMalloc(size)))

#define DEALLOCATE_LOCAL(ptr) \
    if ((ptr) == &_XtAllocLocalArray) {_XtAllocBusy = 0;} else {XtFree(ptr);}
#define NO_ALLOCA
#endif

#ifndef NO_ALLOCA
/*
 * os-dependent definition of local allocation and deallocation
 * If you want something other than XtMalloc/XtFree for ALLOCATE/DEALLOCATE
 * LOCAL then you add that in here.
 */
#if defined(__HIGHC__)

extern char *alloca();

#if HCVERSION < 21003
#define ALLOCATE_LOCAL(size)	alloca((int)(size))
pragma on(alloca);
#else /* HCVERSION >= 21003 */
#define	ALLOCATE_LOCAL(size)	_Alloca((int)(size))
#endif /* HCVERSION < 21003 */

#define DEALLOCATE_LOCAL(ptr)  /* as nothing */

#endif /* defined(__HIGHC__) */


#ifdef __GNUC__
#define alloca __builtin_alloca
#define ALLOCATE_LOCAL(size) alloca((int)(size))
#define DEALLOCATE_LOCAL(ptr)  /* as nothing */
#else /* ! __GNUC__ */
/*
 * warning: mips alloca is unsuitable, do not use.
 */
#if defined(vax) || defined(sun) || defined(apollo) || defined(stellar)
/*
 * Some System V boxes extract alloca.o from /lib/libPW.a; if you
 * decide that you don't want to use alloca, you might want to fix it here.
 */
/* alloca might be a macro taking one arg (hi, Sun!), so give it one. */
#define __Xnullarg		/* as nothing */
char *alloca(__Xnullarg);
#define ALLOCATE_LOCAL(size) alloca((int)(size))
#define DEALLOCATE_LOCAL(ptr)  /* as nothing */
#endif /* who does alloca */
#endif /* __GNUC__ */

#endif /* NO_ALLOCA */

#ifndef ALLOCATE_LOCAL
#define ALLOCATE_LOCAL(size) XtMalloc((unsigned long)(size))
#define DEALLOCATE_LOCAL(ptr) XtFree((XtPointer)(ptr))
#endif /* ALLOCATE_LOCAL */

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _Xtos_h */
/* DON'T ADD STUFF AFTER THIS #endif */
