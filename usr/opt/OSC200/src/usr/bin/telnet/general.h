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
 *	@(#)$RCSfile: general.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:57:39 $
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
 * Copyright (c) 1988 Regents of the University of California.
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */

/*
 * Some general definitions.
 */


#define	numberof(x)	(sizeof x/sizeof x[0])
#define	highestof(x)	(numberof(x)-1)

#if	defined(unix)
#define	ClearElement(x)		bzero((char *)&x, sizeof x)
#define	ClearArray(x)		bzero((char *)x, sizeof x)
#else	/* defined(unix) */
#define	ClearElement(x)		memset((char *)&x, 0, sizeof x)
#define	ClearArray(x)		memset((char *)x, 0, sizeof x)
#endif	/* defined(unix) */

#if	defined(unix)		/* Define BSD equivalent mem* functions */
#define	memcpy(dest,src,n)	bcopy(src,dest,n)
#define	memmove(dest,src,n)	bcopy(src,dest,n)
#define	memset(s,c,n)		if (c == 0) { \
				    bzero(s,n); \
				} else { \
				    register char *src = s; \
				    register int count = n; \
					\
				    while (count--) { \
					*src++ = c; \
				    } \
				}
#define	memcmp(s1,s2,n)		bcmp(s1,s2,n)
#endif	/* defined(unix) */
