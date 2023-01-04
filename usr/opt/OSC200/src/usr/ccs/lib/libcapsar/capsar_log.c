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
static char *sccsid = "@(#)$RCSfile: capsar_log.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/09/23 18:30:23 $";
#endif lint

#include <stdio.h>
#include <locale.h> /*GAG*/
#include <syslog.h>
#include <capsar.h>

#include "libcapsar_msg.h" /*GAG*/
nl_catd scmc_catd;
#define MSGSTR(Num,Str) catgets(scmc_catd,MS_libcapsar,Num,Str)

static int logging_type = STDERR_LOGGING;

capsar_set_log_type (t)
int t;
{
	switch (t) {
	case NO_LOGGING:
	case STDERR_LOGGING:
	case SYSLOG_LOGGING:
		logging_type = t;
		break;
	default:
		logging_type = STDERR_LOGGING;
	}
}

int capsar_get_log_type () {
	return logging_type;
}


capsar_log(error_no,msg_num,s1,s2) /*GAG*/
int	error_no;
int	msg_num; /* specifies the catalog message ID */ /*GAG*/
char	*s1,*s2;
{

	switch(logging_type){
	case NO_LOGGING:
		break;
	case STDERR_LOGGING:
		(void) setlocale(LC_ALL,""); /*GAG*/
		scmc_catd = catopen(MF_LIBCAPSAR,NL_CAT_LOCALE);
		fprintf(stderr,MSGSTR(msg_num,s1),s2);
		catclose(scmc_catd);
		break;		
	case SYSLOG_LOGGING:
		syslog(error_no,s1,s2);
		break;		
	}
	return;
}
