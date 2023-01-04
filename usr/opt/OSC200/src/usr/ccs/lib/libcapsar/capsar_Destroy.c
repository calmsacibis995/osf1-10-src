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
static char *sccsid = "@(#)$RCSfile: capsar_Destroy.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:02:30 $";
#endif lint

/*
 * capsar_Destroy.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : int capsar_Destroy ( MM *m )
 *
 * This routine Destroy the message data structures.
 * ie unlinks temporary files , frees pointers
 *
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <ctype.h>
#include <capsar.h>
#include <sys/file.h>
#include <syslog.h>
#include "libcapsar_msg.h" /*GAG*/

capsar_Destroy(m)
MM	*m;	/* pointer to message structure */
{
/*------*\
  Locals
\*------*/
	MM	*m0;
	MM	**m2,*tmp;

	if(!m )return(NOTOK);

	for(m0=m;m0;m0=m0->mm_next){

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
	
	if((m2 = capsar_get_messages(m)) == NULL) {
		(void) capsar_log(LOG_INFO,M_MSG_0,"capsar_get_messages failed\n",NULL); /*GAG*/
		return(NOTOK);
	}
	
	while(tmp = *m2++){
		free(tmp);
	}
	return(OK);
}	
