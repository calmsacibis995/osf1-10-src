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
static char *sccsid = "@(#)$RCSfile: capsar_get_header.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:03:35 $";
#endif lint

/*
 * capsar_get_header.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : char *capsar_get_header ( MM *m , char * name )
 *
 * For a MAIL_MESSAGE return a pointer to the first header line whose stem
 * is name.
 *
 */

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <ctype.h>
#include <syslog.h>
#include <capsar.h>
#include "libcapsar_msg.h" /*GAG*/

char		*dataptr;
int		msize;
char		*h_info[100];

char *capsar_get_header(m,name)
MM	*m;	/* pointer to message structure */
char	*name;	/* header name to be matched */
{
/*------*\
  Locals
\*------*/
	char	**cps,
		*cp;

	if(m==NULL || m->message_type != MAIL_MESSAGE)return(NULL);

	cps = capsar_get_header_list(m);

	if(cps == NULL){
		capsar_log(LOG_INFO,M_MSG_37,"no headers in mail message\n",NULL); /*GAG*/
		return(NULL);
	}	
	while(cp = *cps++){	
		if(getstem(cp,name) == OK)
			return(cp);
	}
	return(NULL);
}

getstem(buf,stem)	
char	*buf;
char	*stem;
{

	char	c;
	int	j;
	char	*cp;
	int	field_length;

	cp = buf;
	field_length =  j = strlen(buf);

	while((c = *buf++) != ':' && c != '\n' && --j > 0);
	
	if(j<=0 || c == '\n'){
		(void) capsar_log(LOG_INFO,M_MSG_38,"end of field encountered\n",NULL); /*GAG*/
		return(NOTOK);
	}

		
	/* get the header field and check that it is ok */

	if(compare(cp,stem,field_length - j) == 0)
		return(OK);

	else
		return(NOTOK);
}


