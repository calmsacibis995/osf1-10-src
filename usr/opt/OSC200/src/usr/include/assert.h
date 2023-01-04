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
 *	@(#)$RCSfile: assert.h,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/06/09 14:04:20 $
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
 * assert.h
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:
 *
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * assert.h	1.10  com/inc,3.1,9021 5/11/90 10:03:55 
 */

#ifndef _ASSERT_H_
#define _ASSERT_H_
#include <standards.h>

/*
 *
 *      The ANSI standard requires that certain values be in assert.h.
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present.  This header includes all ANSI required entries.  
 *
 */
#ifdef _ANSI_C_SOURCE

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C" {
#endif
extern void __assert(char *, char *, int);
#if defined(__cplusplus)
}
#endif
#else  /* __STDC__ */
extern void __assert();
#endif /* _NO_PROTO */

#if defined(__STDC__) || defined(__cplusplus)
#define assert(EX) (((int) (EX)) ? (void)0 : __assert(#EX, __FILE__, __LINE__))
#else
#define assert(EX) if ((int) (EX)) ; else __assert("EX", __FILE__, __LINE__)
#endif /* __STDC__*/

#endif /* NDEBUG */
#endif /* _ANSI_C_SOURCE */
#endif /* _ASSERT_H_ */
