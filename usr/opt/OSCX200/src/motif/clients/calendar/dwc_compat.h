#ifndef _dwc_compat_h_
#define _dwc_compat_h_
/* $Id$ */
/* #module dwc_compat.h */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows general purpose
**
**  AUTHOR:
**
**	Marios Cleovoulou, January-1988
**
**  ABSTRACT:
**
**	Sets up things for cross os compiling.
**	
**--
*/

#if defined (__DECC) && defined (VAXCSHR)
#include <vaxcshr.h>
#endif

/*
** Handle certain VAXCisms to make them supported when the compiler doesn't.
*/
#if defined (__VAX) && !defined (VAX)
#define VAX 1
#define vax 1
#endif
#if defined (__VAXC) && !defined (VAXC)
#define VAXC 1
#define vaxc 1
#endif
#if defined (__VMS) && !defined (VMS)
#define VMS 1
#define vms 1
#endif
/*
** Handle MIPS cc inability to do const
*/
#if !defined(vaxc) && !defined(__STDC__) && !defined(__osf__)
#define const
#endif
/*
** Definition for dwcaddr_t
*/
#if defined(VAXC) || defined(__DECC) || defined(__STDC__) || defined(__osf__)

typedef void *dwcaddr_t;

#else

typedef char *dwcaddr_t;

#endif

/*
** Definition for readonly needed when there is no VAXC
*/
#ifndef VAXC
#define	readonly
#endif

/*
** Definition for noshare needed for ANSI compilers.  This is a language
** extension.
*/
#if defined (__STDC__) || defined (__osf__)
#define noshare
#ifdef DWC$NOGLOB
#define globaldef
#define globalref extern
#endif
#endif

/*
** On VMS these get defined in Intrinsic.h
*/
#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))	
#endif /* MAX */



/*	  
**  Do we support function prototypes?
*/
#if defined (FUNCPROTO)
#if FUNCPROTO
#define CAUGHT_FUNCPROTO
#endif
#endif

#if defined (VAXC) || defined (__STDC__) || defined (CAUGHT_FUNCPROTO)
#define _DWC_PROTO_

#define PROTOTYPE(args) args

#else

#define PROTOTYPE(args) ()

#endif

/*
** Temporary work around for DECC problem with XtOffset
*/
#if defined (XtOffset) && defined (__DECC)

#undef XtOffset
#define XtOffset(p_type,field) \
    (((char *)(&(((p_type)NULL)->field))) - (char *)NULL)

#endif

/*
** Make sure malloc, etc. are properly defined.
*/
#include <stdlib.h>

/*
** Make sure CHAR_BIT is defined
*/
#include <limits.h>

#ifdef sparc
/*
** Sparc is byteswapped and still uses the help widget.
*/
#define HELPWIDGET 1
#define BYTESWAP 1
#define UL_ONE ((unsigned long)1)
#else

/*
** Non-Sparc is non-byteswapped and has (or will shortly) HyperHelp.
*/
#define UL_ONE 1ul

#define BYTESWAP 0

#define OLD_HYPERHELP 1

#endif

/*
** Alpha has a real fsync.  So will BLADE.  However, I don't know how to
** detect BLADE at compile time.
*/
#if 0
#if defined(VMS) && !defined(__DECC)
#define DWC$NOFSYNC 1
#endif
#endif

#if defined(LONG_BIT)
#if (LONG_BIT == 64)
#define DWC_LONG_BIT 64
#else
#define DWC_LONG_BIT 32
#endif
#else
#define DWC_LONG_BIT 32
#endif

#endif /* end of _dwc_compat_h_ */

/*
** Notice that this is outside the redo check.  I want to make sure that this
** gets set!!!
*/
#if defined (XtIsRealized)
#undef XtIsRealized
#define XtIsRealized(object) (XtWindowOfObject(object) != 0)
#endif

