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
static char	*sccsid = "@(#)$RCSfile: fcatgetmsg.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 22:48:36 $";
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: LIBCMSG
 *
 * FUNCTIONS: fcatgetmsg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sccsid[] = "fcatgetmsg.c      1.9  com/lib/c/msg,3.1,9021 3/29/90 17:33:19";
 */

/*
 * NAME: fcatgetmsg
 *                                                                    
 * FUNCTION: Gets a message from a catalog and copies it into a buffer.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	Executes under a process.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the buffer on success or
 *	a pointer to a NULL string on failure.
 *
 */  

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak fcatgetmsg = __fcatgetmsg
#endif
#endif
#include <nl_types.h>

char *fcatgetmsg(nl_catd catd, int setno,int msgno,char *buf,int buflen)

	/*---- catd: the catd to be read from ----*/
 	/*---- setno: the set number of the message ----*/
	/*---- msgno: the message number to be read ----*/
	/*---- buf: the buffer to store the message ----*/
	/*---- buflen: the maximum length of the message ----*/


{
	return catgetmsg(catd, setno, msgno, buf, buflen);
}
