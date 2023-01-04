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
static char *sccsid = "@(#)$RCSfile: capsar_report.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/03 15:26:30 $";
#endif lint

/*
 *   capsar_report 1/18/88
 *   Lynn C Wood
 *
 *   This routine is provided as part of the capsar library for
 *   handling of Multiple Body part messages in Ultrix mail
 *
 *  syntax : capsar_report( MM *m )
 *
 *  This routinereports on the contents of the message fields
 *
 */

#include <stdio.h>
#include <capsar.h>

capsar_report(m)
MM	*m;	/* message strucure */
{

	if(m == NULL){
		(void) printf("empty message \n");
		return;
	}

	(void) printf("\n MAIL STRUCTURE INFORMATION \n");

	(void) printf("Message name = %s \n",m->name);
	if(m->start !=NULL)(void) printf("Message start tag = %s ",m->start);
	if(m->stop !=NULL)(void) printf("Message Stop tag = %s ",m->stop);
	if(m->body_type !=NULL)(void)printf("Body Type = %s \n",m->body_type);
	(void) printf("Message size = %d \n",m->size);
	(void) printf("Message offset = %d \n",m->offset);
	switch(m->message_type){
		case EMPTY : 
			(void) printf("Message Type : empty \n");
			break;
		case SIMPLE_MESSAGE : 
			(void) printf("Message Type : simple_message \n");
			break;
		case COMPOUND_MESSAGE : 
			(void) printf("Message Type : compound message \n");
			break;
		case MAIL_MESSAGE : 
			(void) printf("Message Type : mail message \n");
			break;
		case MULTIPLE_FORMAT : 
			(void) printf("Message Type : multiple format \n");
			break;
	}
	if(m->dataptr)
		printf(" Message data in core \n");
	else if(m->swapfile)
		printf(" Message stored in %s \n",m->swapfile);

} 
