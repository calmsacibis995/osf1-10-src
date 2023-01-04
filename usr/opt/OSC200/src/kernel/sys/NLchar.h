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
 *	@(#)$RCSfile: NLchar.h,v $ $Revision: 4.2.10.5 $ (DEC) $Date: 1993/11/09 23:55:44 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SYS_NLCHAR_H_
#define _SYS_NLCHAR_H_


#ifdef _KJI
#include <jctype.h>
#endif   /* _KJI */
#include <sys/types.h>

/*  BASIC DEFINITIONS FOR USING NLCHARS.
 */

/*  The big type itself.
 */
typedef wchar_t NLchar;

#ifdef _NO_PROTO

extern char *NLstrcpy();
#ifdef _KJI
extern int NCisshift();
#else
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef NCisshift
#undef NCisshift
#endif
#endif
#define NCisshift(c) (int) 0
#endif

extern unsigned char
	*NLstrncpy(),
	*NLstrchr();

#else /* _NO_PROTO */
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
extern char *NLstrcpy(char *, char *);

#ifdef _KJI
extern int NCisshift(int );
#else
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef NCisshift
#undef NCisshift
#endif
#endif
#define NCisshift(c) (int) 0
#endif

extern unsigned char
        *NLstrncpy(unsigned char *, unsigned char *, int),
        *NLstrchr(unsigned char *, NLchar);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* __STDC__ */
#endif /* _NO_PROTO */

/*  Number of distinct NLchars.
 */
#define NLCOLMAX        16384     /* maximum number of elements in collation table */
#define NLCHARMAX       257

#ifdef lint
extern NLchar NCdechr();
#else

/*  Single-shift character definition (both NLchar and char representation).
 */

#define _NCtop(nlc)     ((nlc) >> 8 & 0xff)
#define _NCbot(nlc)     ((nlc) & 0xff)

/*  SINGLE-CHARACTER CONVERSION MACROS.
 */

/*  Internal macro to test for multi-byte NLS code point.
 */
#define _NCis2(c0,c1)   NCisshift(c0)

/*  Internal macro to convert multi-byte NLS code point to NLchar.
 */
#define _NCd2(c0,c1)    (((unsigned char)(c0) << 8) | (unsigned char)(c1))

/*  Internal macro to convert NLchar to NLS code point.
 */

#define _NCe2(nlc, c0, c1)      (((nlc) > 0xff) ? \
					((c0) = _NCtop(nlc), \
						(c1) = _NCbot(nlc), 2) : \
					((c0) = (nlc), 1))

/*  Convert c0 (and c1, if need be) into NLchar nlc; return # chars used.
 *  This is the only conversion macro that accepts an NLchar, rather than
 *  a pointer to one, as an argument.
 */
#define _NCdec2(c0, c1, nlc) ((_NCis2((c0), (c1))) ? \
				((nlc) = _NCd2((c0), (c1)), 2) : \
				((nlc) = (unsigned)(c0), 1))

/*  Defs for accessing collating data.
 */
#ifndef _KJI
#define _NCmap(nlc)	( (nlc) )
#define _NCunmap(nlc)	( (nlc) )
#else
#define _NCmap(nlc)     ( ((nlc) <= 0xff) ? (nlc) : \
				( (nlc) < 0xdfff ? ( (nlc) - 0x8000 ) : \
						   ( (nlc) - 0xc000 ) ))
#define _NCunmap(nlc)   ( ((nlc) <= 0xff)? (nlc) : \
				( (nlc) < 0x2000 ? ( (nlc) + 0x8000 ) : \
						   ( (nlc) + 0xc000 ) ))
#endif  /* _KJI */

/* 
 * NCeqvmap is no longer supported because the definition of an equivalence 
 * class is changed to mean "all characters or collating elements with the 
 * same primary collation value". If used, the macro will always return 1 
 * ("no equivalence class").
 */

#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef NCeqvmap
#undef NCeqvmap
#endif
#endif
#define NCeqvmap(ucval) ( 1 )
#endif /* lint */

/*  MISCELLANY.
 */

/*  Internal defs for string(3) macros using charsets.
 */
#define NLCSETMAX	33	/* malloc charsets over this length - 1 */

#ifndef _KERNEL
#ifdef  _NO_PROTO
extern void free();
#else /* _NO_PROTO */
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
extern void free(void *);
#if defined(__cplusplus)
}
#endif	/* __cplusplus */
#endif	/* __STDC__ */
#endif  /* _NO_PROTO */
#endif	/* _KERNEL */

#endif /* _SYS_NLCHAR_H_ */ 
