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
 *	@(#)$RCSfile: limits.h,v $ $Revision: 4.3.10.4 $ (DEC) $Date: 1993/07/20 17:17:14 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * limits.h
 *
 * Modification History:
 *
 * 27-Sep-91     Paula Long
 *      Moved P1003.4 defines to rt_limits.h.  rt_limits.h will be 
 *      included in limits.h if _KERNEL || _POSIX_4SOURCE is defined.
 *
 * 3-jun-91	Rob Haydt
 *	add more aio limits
 *
 * 4-Apr-91     Paula Long
 *      Add P1003.4 required extensions.  
 *
 */

/*
 * COMPONENT_NAME: limits.h
 *                                                                    
 *
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * 
 * 1.49  com/inc/sys/limits.h, incstd, bos320, 9125320 6/6/91 08:49:28
 */                                                                   

#ifndef _SYS_LIMITS_H_
#define _SYS_LIMITS_H_ 
#include <standards.h>

/*
 *
 *      The ANSI and POSIX standards require that certain values be in limits.h.
 *      They also require that if _ANSI_C_SOURCE or _POSIX_SOURCE is 
 *      defined then ONLY those standard specific values are present.
 *      This header includes all the ANSI and POSIX required entries.
 *      In addition other entries for the XIX system are included.
 *
 */
#ifdef _ANSI_C_SOURCE

#include <machine/machlimits.h>


#endif /* _ANSI_C_SOURCE */

/*
 *   The following are values that have historically been in limits.h.
 *
 *   They are a part of the POSIX defined limits.h and therefore are
 *   included when _POSIX_SOURCE is defined.
 *
 */


#ifdef _POSIX_SOURCE


/* limits on a system conforming to the POSIX standard */

#define _POSIX_ARG_MAX		4096
#define _POSIX_CHILD_MAX	6
#define _POSIX_LINK_MAX		8
#define _POSIX_MAX_CANON	255
#define _POSIX_NAME_MAX		14
#define _POSIX_NGROUPS_MAX	0
#define _POSIX_MAX_INPUT	255
#define _POSIX_OPEN_MAX		16
#define _POSIX_PATH_MAX		255
#define _POSIX_PIPE_BUF		512
#define _POSIX_SSIZE_MAX	32767
#define _POSIX_STREAM_MAX	8
#define _POSIX_TZNAME_MAX	3

/* The system supports the following utility limit minimum values */

#define _POSIX2_BC_BASE_MAX      99   /* max ibase and obase values for bc */
#define _POSIX2_BC_DIM_MAX       2048 /* max num elements in array for bc */
#define _POSIX2_BC_SCALE_MAX     99   /* max scale value allowed by bc */
#define _POSIX2_BC_STRING_MAX    1000 /* max len of string constant by bc */
#define _POSIX2_COLL_WEIGHTS_MAX 2    /* max num weight for LC_COLLATE order */
#define _POSIX2_EXPR_NEST_MAX    32   /* max num expression nested for expr */
#define _POSIX2_LINE_MAX         2048 /* max len of utility input line */
#define _POSIX2_RE_DUP_MAX       255  /* max num repeated reg for interval */

/* The system supports the following symbolic utility limits */

#define BC_BASE_MAX      _POSIX2_BC_BASE_MAX
#define BC_DIM_MAX       _POSIX2_BC_DIM_MAX
#define BC_SCALE_MAX     _POSIX2_BC_SCALE_MAX
#define BC_STRING_MAX    _POSIX2_BC_STRING_MAX
#define COLL_WEIGHTS_MAX 	6		/* same as in sys/localedef.h */
#define EXPR_NEST_MAX    _POSIX2_EXPR_NEST_MAX
#define LINE_MAX         _POSIX2_LINE_MAX
#ifndef RE_DUP_MAX	 /* NLregexp.h */
#define RE_DUP_MAX       _POSIX2_RE_DUP_MAX
#endif /* RE_DUP_MAX */


/* RUN-TIME INVARIANT VALUES
   The system supports the following fixed magnitude limitations.
*/

#include <sys/syslimits.h>

#endif /* _POSIX_SOURCE */

#ifdef _XOPEN_SOURCE

#define	CHARCLASS_NAME_MAX	255 /* Max. number of bytes in character class name */

/* Some of the values in <float.h> are required in limits.h by XPG4, but
 * are not allowed by Posix.1.  The include must remain in _XOPEN_SOURCE.
 * These are flagged in XPG4 "TO BE WITHDRAWN". 
 */
#define FLT_DIG            6
#define FLT_MAX            3.40282347e+38f
#define DBL_DIG            15
#define DBL_MAX            1.79769313486231570e+308


/* ANSI C requires TMP_MAX to be in stdio.h, however, XOPEN requires it to be
 * in limits.h.  
 */

#ifndef TMP_MAX
#define TMP_MAX		16384		/* The number of unique names 
					   generated by the tmpnam() function */
#endif
					/* max decimal value of a double */
/* defines for message catalog usage */

#define NL_ARGMAX       9               /* max number of printf args */

#define NL_MSGMAX       65535           /* max number of messages per set */

#define NL_SETMAX       65535           /* max number of set per catalog */

#define NL_TEXTMAX      8192            /* max message length in bytes */

#define PASS_MAX	16		/* max number of bytes in a password
					   (not a string length; doesn't include
					   terminating null), >= 8 */
#define NL_LANGMAX	PATH_MAX   /* max number of of bytes in a LANG name */

#define NL_NMAX 	10		/* max number of bytes in N-to-1 mapping
					   characters */

#if defined( __alpha) || defined(__mips64)
#define LONG_BIT	64		/* number of bits in a long     */
#else
#define LONG_BIT	32		/* number of bits in a long     */
#endif

#define WORD_BIT	32		/* number of bits in a "word" or int */


#endif /* _XOPEN_SOURCE */

#ifdef _BSD
#define NZERO		0		/* default proc priority (BSD) */
#else	/* _BSD */
#ifdef _XOPEN_SOURCE
#define NZERO		20		/* default proc priority (SYSV/XOPEN) */
#endif	/* _XOPEN_SOURCE */
#endif	/* _BSD */


#ifdef _AES_SOURCE

#if defined(__alpha) || defined(__mips64)
#define ATEXIT_MAX	2048
#else
#define	ATEXIT_MAX	32
#endif

#endif /* _AES_SOURCE */

#ifdef _OSF_SOURCE

#include <machine/machtime.h>		/* to get CLOCKS_PER_SEC */

#define PID_MAX		SHRT_MAX	/* max value for a process ID,
					   >=30000	*/ 
#define UID_MAX		USHRT_MAX	/* max value for a user or group ID */
#define USI_MAX		UINT_MAX	/* max unsigned int - for OSF1.1 comp.*/

#endif /*_OSF_SOURCE */   
                       
/* 
 * P1003.4 Required Constants
 */

#if defined(_KERNEL) || defined(_POSIX_4SOURCE) 
#include <sys/rt_limits.h>
#endif /* _KERNEL || _POSIX_4SOURCE */

#endif /* _SYS_LIMITS_H_ */
