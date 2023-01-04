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
/*LINTLIBRARY*/

#ifndef lint
static char *sccsid = "@(#)$RCSfile: capsar_delete_message.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:03:11 $";
#endif lint

/*
 * capsar_delete_message.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : capsar_delete_message ( MM *m , MM *n )
 *
 * This routine deletes the message n from the set of bodyparts on m
 *
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <ctype.h>
#include <sys/file.h>
#include <syslog.h>
#include <capsar.h>
#include "libcapsar_msg.h" /*GAG*/

capsar_delete_message(m,n)
MM	*m;	/* pointer to message structure */
MM	*n;	/* pointer to message structure */
{
/*------*\
  Locals
\*------*/

	MM	*mprev,*m0,
		*mmpp,*m1;

	if(!m || !n){
		capsar_log(LOG_INFO,M_MSG_25,"NULL message\n",NULL); /*GAG*/
		return(NOTOK);
	}
	if(m == n){
		capsar_clearup(m);
		m=NULL;
	}
	
	/* try to locate message structure n from m */

	mprev = m;
	for(m0 = m; m0 != NULL; m0->mm_next){
		if(m0 == n)
			break;
		mprev = m0;
	}

	if(m0 == NULL)
		return(NOTOK);

	/* mark all  children etc of n */

	m0 = m0->mm_next;
	for(m1 = m0;m1;m1 = m1->mm_next){
		mmpp = m1;
		while(mmpp->parent != m && !mmpp)
			mmpp = mmpp->parent;

		if(!mmpp)break;
		clean(m1);
	}

	/* remove n from linked list */

	if(!m1)mprev->mm_next = NULL;
	else 
		mprev->mm_next = m1;

	return(OK);
}	

clean(m0)
MM	*m0;	/* pointer to message structure */
{


	if(m0->dataptr)free(m0->dataptr);
	if(m0->name)free(m0->name);
	if(m0->start)free(m0->start);
	if(m0->stop)free(m0->stop);
	if(m0->separator)free(m0->separator);
		
	if(m0->swapfile){
		if(access(m0->swapfile,F_OK)==0)
			(void) unlink(m0->swapfile);

		free(m0->swapfile);
	}
}	
