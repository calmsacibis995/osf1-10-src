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
 *	@(#)$RCSfile: machlimits.h,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/04/14 18:13:31 $
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
 * derived from machlimits.h	2.1	(ULTRIX/OSF)	12/3/90
 */
/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */
#ifndef _MACH_MACHLIMITS_H_
#define _MACH_MACHLIMITS_H_

#define	CHAR_BIT	8		/* number of bits in a char */

#define	SCHAR_MIN	-128		/* min value for a signed char */
#define	SCHAR_MAX	0x7f		/* max value for a signed char */

#define	CHAR_MAX	0x7f		/* max value for a char */
#define	CHAR_MIN	-128		/* min value for a char */

#define	SHRT_MAX	0x7fff		/* max value for a short */
#define	SHRT_MIN	-32768		/* min value for a short */

#define	INT_MAX		0x7fffffff	/* max value for an int */
#define	INT_MIN		0x80000000	/* min value for an int */

#define	LONG_MAX	0x7fffffff	/* max value for a long */
#define	LONG_MIN	0x80000000	/* min value for a long */

#if defined(__STDC__)
#define UCHAR_MAX       255U		/* max value for an unsigned char */
#define USHRT_MAX       65535U		/* max value for an unsigned short */
#define UINT_MAX        4294967295U	/* max value for an unsigned int */
#define ULONG_MAX       4294967295U	/* max value for an unsigned long */
#else /*  defined(__STDC__)  */
#define UCHAR_MAX       255		/* max value for an unsigned char */
#define USHRT_MAX       65535		/* max value for an unsigned short */
#define UINT_MAX        4294967295	/* max value for an unsigned int */
#define ULONG_MAX       4294967295	/* max value for an unsigned long */
#endif /*  defined(__STDC__)  */

/* Must be at least two, for internationalization (NLS/KJI) */
#define MB_LEN_MAX	2		/* multibyte characters */

#endif /* _MACH_MACHLIMITS_H_ */

