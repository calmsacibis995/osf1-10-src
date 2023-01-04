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
static char *sccsid = "@(#)$RCSfile: capsar_limitations.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:03:59 $";
#endif lint

/*
 * capsar_limitations.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : char *capsar_limitations ( MM *m ,char *message_type)
 *
 * This routine restricts a message to multiple bodied text or
 * single bodied DDIS documents 
 *
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <ctype.h>
#include <capsar.h>
#include <syslog.h>
#include "libcapsar_msg.h" /*GAG*/

int capsar_limitations(m,message_type)
MM	*m;	/* pointer to message structure */
char	*message_type;
{
/*------*\
  Locals
\*------*/

	MM	*m0;
	int	ddif=0,
		dots=0,
		text=0;
	
	if(!m )return(NOTOK);


	for(m0=m;m0;m0=m0->mm_next){
		if(strcmp(m0->body_type,DDIFTAG) == 0 )
			ddif++;
		else if(strcmp(m0->body_type,DOTSTAG) ==0)
			dots++;
		else if(strcmp(m0->body_type,BODY_TYPE_DEF) ==0)
			text++;
		else return(NOTOK);
	}
	if ((dots+ddif) > 1) {
		capsar_log(LOG_INFO,M_MSG_42,"? DOTS/DDIF document in mail message\n",NULL); /*GAG*/
		return(NOTOK);
	}
	
	if ((dots+ddif) > 0 && text > 0) {
		capsar_log(LOG_INFO,M_MSG_43,"? DOTS/DDIF documents in mail message and ? text documents\n", NULL); /*GAG*/
		return (NOTOK);
	}

	if(ddif)
		strcpy(message_type,DDIFTAG);
	else if(dots)
		strcpy(message_type,DOTSTAG);
	else 
		strcpy(message_type,BODY_TYPE_DEF);

	return(OK);
}
