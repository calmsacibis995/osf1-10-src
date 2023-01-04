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
 * @(#)$RCSfile: alphaosf.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:01:12 $
 */

/*
 * OSF/1 1.2
 */

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)alphaosf.h	5.3 (Berkeley) 6/1/90
 */

/*
 * alphaosf.h --
 *
 * Common constants and type declarations for Alphaosf.
 */

#ifndef _ALPHAOSF
#define _ALPHAOSF

#include <sys/types.h>

/*
 * A boolean type is defined as an integer, not an enum. This allows a
 * boolean argument to be an expression that isn't strictly 0 or 1 valued.
 */

typedef int Boolean;
#ifndef TRUE
#define TRUE	1
#endif /* TRUE */
#ifndef FALSE
#define FALSE	0
#endif /* FALSE */

/*
 * Functions that must return a status can return a ReturnStatus to
 * indicate success or type of failure.
 */

typedef int  ReturnStatus;

/*
 * The following statuses overlap with the first 2 generic statuses 
 * defined in status.h:
 *
 * SUCCESS			There was no error.
 * FAILURE			There was a general error.
 */

#define	SUCCESS			0x00000000
#define	FAILURE			0x00000001


/*
 * A nil pointer must be something that will cause an exception if 
 * referenced.  There are two nils: the kernels nil and the nil used
 * by user processes.
 */

#define NIL 		0xFFFFFFFFFFFFFFFF	/*John Derzinski 64 bit port*/ 
#define USER_NIL 	0

/*
 * An address is just a pointer in C.  It is defined as a character pointer
 * so that address arithmetic will work properly, a byte at a time.
 */

typedef char *Address;

/*
 * ClientData is an uninterpreted word.  It is defined as an int so that
 * kdbx will not interpret client data as a string.  Unlike an "Address",
 * client data will generally not be used in arithmetic.
 */

typedef int *ClientData;

#ifdef notdef
#include "status.h"
#endif

#endif /* _ALPHAOSF */



