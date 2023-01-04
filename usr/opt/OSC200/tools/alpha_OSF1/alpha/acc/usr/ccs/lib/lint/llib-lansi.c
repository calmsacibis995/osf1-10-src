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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: llib-lansi.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:08:30 $";
#endif 
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
 * COMPONENT_NAME: (CMDPROG) Programming Utilities
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27; 32
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 	llib-lansi	1.1  com/cmd/prog/lint,3.1,9013 10/10/89 17:36:18 
 */
/*
** Standard ANSI C library include files.
*/

#ifdef _POSIX_SOURCE
#define _POSIX_REMEMBER
#undef _POSIX_SOURCE
#endif

#ifdef _XOPEN_SOURCE
#define _XOPEN_REMEMBER
#undef _XOPEN_SOURCE
#endif

#ifdef _AES_SOURCE
#define _AES_REMEMBER
#undef _AES_SOURCE
#endif

#ifdef _OSF_SOURCE
#define _OSF_REMEMBER
#undef _OSF_SOURCE
#endif

#ifdef _ANSI_C_SOURCE
#define _ANSI_C_REMEMBER
#else
#define _ANSI_C_SOURCE
#endif


/*LINTSTDLIB*/
#include	<assert.h>
#include	<ctype.h>
#include	<errno.h>
#include	<float.h>
#include	<limits.h>
#include	<locale.h>
#include	<math.h>
#include	<setjmp.h>
#include	<signal.h>
#include	<stdarg.h>
#include	<stddef.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>


#ifdef _POSIX_REMEMBER
#define _POSIX_SOURCE
#undef _POSIX_REMEMBER
#endif

#ifdef _XOPEN_REMEMBER
#define _XOPEN_SOURCE
#undef _XOPEN_REMEMBER
#endif

#ifdef _AES_REMEMBER
#define _AES_SOURCE
#undef _AES_REMEMBER
#endif

#ifdef _OSF_REMEMBER
#define _OSF_SOURCE
#undef _OSF_REMEMBER
#endif

#ifdef _ANSI_C_REMEMBER
#undef _ANSI_C_REMEMBER
#else
#undef _ANSI_C_SOURCE
#endif

#line	1

