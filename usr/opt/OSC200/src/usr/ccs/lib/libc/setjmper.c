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
static char	*sccsid = "@(#)$RCSfile: setjmper.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:38:01 $";
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
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: longjmperror
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * setjmper.c	1.6  com/lib/c/gen,3.1,8943 10/11/89 11:06:08
 */

/*
 * NAME: longjmperror()		(BSD)
 *
 * FUNCTION: writes a message to file descriptor 2 (nominally stderr)
 *	     that longjmp() failed.
 *
 * NOTES: 
 * 	This routine is called from longjmp() when an error occurs.
 * 	Programs that wish to exit gracefully from this error may
 * 	write their own versions.
 * 	If this routine returns, the program is aborted.
 *
 * RETURN VALUES: 
 *	NONE
 */

	/* message for longjmp error */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak longjmperror = __longjmperror
#endif
#endif
#define ERRMSG	"longjmp or siglongjmp function used outside of saved context\n"
#ifdef MSG
#include "libc_msg.h"
#define E_ERRMSG NLgetamsg(MF_LIBC, MS_LIBC, M_SETJMPER, ERRMSG)
#else
#define E_ERRMSG ERRMSG
#endif /* MSG */

/*
 * This routine is called from longjmp() when an error occurs.
 * Programs that wish to exit gracefully from this error may
 * write their own versions.
 * If this routine returns, the program is aborted.
 */
void longjmperror()
{
	extern	int	write();	/* system call to write data */
	char	*errmsg = E_ERRMSG;	/* pointer to message text */

	/* write longjmp error message to file descriptor 2, hoping that
 	   file descriptor 2 is stderr */
	write(2, errmsg, NLstrlen(errmsg));
}
