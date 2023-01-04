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
static char *rcsid = "@(#)$RCSfile: gettxt.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/09/27 19:44:49 $";
#endif

#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak gettxt = __gettxt
#endif
#include <string.h>
#include <nl_types.h>
#include <locale.h>

#define NULL_STRING(s) (((s) == NULL) || (*(s) == '\0'))

static char msg_not_fnd[] = "Message not found!!\n";
/*
 *
 *	Function: gettxt
 *		retrieve a text string from a message catalog. This
 *		provides a SVID 3 interface to the existing OSF message
 *		catalog facility.  
 *
 *	Input:
 *		msgid_ptr: message identification string of the form..
 *				<msgfilename>:<msgnumber>
 *		dflt_str:  message to be used if the retrieval fails
 *
 *	Note: OSF also specifies a message set number, SVID 3 does not,
 *	      therefore, this function uses a default set number of 1.
 */

char *
gettxt(char *msgid_ptr, char *dflt_str)
{
#ifdef _THREAD_SAFE
char *nxt_token;
#endif
char *msgid, *cat, *msgno_str, *ret_msg;
int msgno = 0;
int setno = NL_SETD;
nl_catd	catd;

	if (NULL_STRING(msgid_ptr)) {
		return(msg_not_fnd);
	}
	if((msgid = (char *) malloc(strlen(msgid_ptr) + 1)) == NULL) {
		return(msg_not_fnd);
	}
	(void)strcpy(msgid, msgid_ptr);

#ifdef _THREAD_SAFE
	cat = strtok_r(msgid, ":", &nxt_token);
#else
	cat = strtok(msgid, ":");
#endif
	if(cat != NULL) {
#ifdef _THREAD_SAFE
		msgno_str = strtok_r(nxt_token, "", &nxt_token);
#else
		msgno_str = strtok(NULL, "");
#endif
		if(msgno_str != NULL) msgno = atoi(msgno_str);
	}

	catd = catopen(cat, NL_CAT_LOCALE);
	ret_msg = catgets(catd, setno, msgno, dflt_str);
	if((ret_msg == dflt_str) && NULL_STRING(dflt_str)) {
		ret_msg = msg_not_fnd;
	}
	/*
	 * Don't do a catclose() for two reasons.  First, it would
	 * free the memory containing the message.  More importantly,
	 * it would cause the catalog to be re-read on the next call.
	 * That'd be pretty slow.
	 */
	free(msgid);
	return(ret_msg);
}
