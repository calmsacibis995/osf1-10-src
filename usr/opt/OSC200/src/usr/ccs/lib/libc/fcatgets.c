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
static char	*sccsid = "@(#)$RCSfile: fcatgets.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 22:49:01 $";
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
 *
 * sccsid[] = "fcatgets.c        1.10  com/lib/c/msg,3.1,9021 3/29/90 17:32:56";
 */ 

/*
 * COMPONENT_NAME: (opats name) descriptive name
 *
 * FUNCTIONS: LIBCMSG
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*
 * NAME: fcatgets
 *                                                                    
 * FUNCTION: Gets a pointer to a message from a message catalog.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Fcatgets executes under a process.
 *
 * RETURNS: Returns a pointer to the message on success.
 *	If the catd is invalid, the default string is returned.
 *	If the message or set number is invalid, a null string is returned.
 */  

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak fcatgets = __fcatgets
#endif
#endif
#include <nl_types.h>

char *fcatgets(nl_catd catd,int setno,int msgno,char *def) 

	/*---- catd: the catd to get the message from ----*/
	/*---- setno: the set number of the message ----*/
	/*---- msgno: the message number of the message ----*/
	/*---- def: the default string to be returned ----*/

{
	return catgets(catd, setno, msgno, def);    
}
