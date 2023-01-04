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
static char	*sccsid = "@(#)$RCSfile: NLvprintf.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:19:07 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions 
 *
 * FUNCTIONS: NLvprintf
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * NLvprintf.c	1.3  com/lib/c/prnt,3.1,8943 9/7/89 10:05:12
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NLvprintf = __NLvprintf
#endif
#endif
#include <stdio.h>
#include <varargs.h>

/*                                                                    
 * FUNCTION: Writes output to to the stream pointed to by stream, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: format - format used to print arguments
 *	       args   -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */

int
NLvprintf (char *format, va_list args)
{       
	register int rc;	/*---	return code ---*/

	rc = _doprnt (format, args, stdout);
	return (ferror(stdout)? EOF: rc);
}
