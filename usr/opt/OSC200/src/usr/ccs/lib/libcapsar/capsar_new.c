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
static char *sccsid = "@(#)$RCSfile: capsar_new.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/03/23 14:04:13 $";
#endif lint

/*
 * capsar_new.c   01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part Messages in Ultrix Mail
 *
 * syntax : MM * capsar_new ()
 *
 * This routine retruns a pointer to a new, empty mail message.
 *
 */
 
#include <stdio.h>
#include <locale.h> /*GAG*/
#include <syslog.h>
#include <capsar.h>
#include "libcapsar_msg.h" /*GAG*/

MM *capsar_new()
{
/*------*\
  Locals
\*------*/

	MM	*mnew;
	
	mnew = (MM *)malloc(sizeof *mnew);

	if(mnew == NULL){
		(void) capsar_log(LOG_INFO,M_MSG_44,"cannot allocate space for new mail message\n"); /*GAG*/
		return(NULL);
	}	
	capsar_setup(mnew);
	return(mnew);
}

	

