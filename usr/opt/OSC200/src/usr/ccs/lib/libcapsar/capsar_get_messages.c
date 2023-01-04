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
static char *sccsid = "@(#)$RCSfile: capsar_get_messages.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:03:50 $";
#endif lint

/*
 * capsar_get_messages.c   01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part Messages in Ultrix Mail
 *
 * syntax : MM **capsar_get_messages ( MM *m )
 *
 * For anything other than a SIMPLE MESSAGE return a null-terminated
 * list of the body parts.
 */
 
#include <stdio.h>
#include <locale.h> /*GAG*/
#include <syslog.h>
#include <capsar.h>
#include "libcapsar_msg.h" /*GAG*/
#define MAX_BODY_PARTS 200
MM *m1[MAX_BODY_PARTS+1];

MM **capsar_get_messages(m)
MM	*m;	/* message structure */
{
/*------*\
  Locals
\*------*/
	int	i=0;
	
	if(m == NULL) 
		return(NULL);

	m1[i] = NULL;

	while(m != NULL) {
		if(i >= MAX_BODY_PARTS){
			(void) capsar_log(LOG_INFO,M_MSG_41,"to many body parts\n"); /*GAG*/
			return(NULL);
		}

		m1[i++] = m;
		m = m->mm_next;
	}
	m1[i++] = NULL;
	return(m1);
}

	

