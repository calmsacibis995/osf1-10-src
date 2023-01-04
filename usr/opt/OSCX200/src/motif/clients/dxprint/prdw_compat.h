#ifndef _prdw_compat_h_
#define _prdw_compat_h_
/* $Id$ */
/* #module prdw_compat.h */
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
** Definition for readonly needed when there is no VAXC
*/
#ifndef VAXC
#define	readonly
#endif

/*
** Definition for noshare needed for ANSI compilers.  This is a language
** extension.
*/
#if !defined(VMS) || defined(__STDC__)
#define noshare
#ifdef DWC$NOGLOB
#define globaldef
#define globalref extern
#endif
#endif

/*	  
**  Do we support function prototypes?
*/
#if defined (FUNCPROTO)
#if FUNCPROTO
#define CAUGHT_FUNCPROTO
#endif
#endif

#if defined (VAXC) || defined (__STDC__) || defined (CAUGHT_FUNCPROTO)

#define _PRDW_PROTO_ 1
#define PROTOTYPE(args) args

#else

#define _PRDW_PROTO_ 0
#define PROTOTYPE(args) ()

#ifndef _NO_PROTO
#define _NO_PROTO 1
#endif

#endif

#ifdef sparc
#define BYTESWAP 1
#else
#define BYTESWAP 0
#endif

#endif /* end of _prdw_compat_h_ */
