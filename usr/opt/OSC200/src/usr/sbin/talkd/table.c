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
static char	*sccsid = "@(#)$RCSfile: table.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/10/13 14:54:51 $";
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
 * COMPONENT_NAME: TCPIP table.c
 * 
 * FUNCTIONS: MSGSTR, delete, delete_invite, find_match, find_request, 
 *            insert_table, new_id 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
#ifndef lint
static char sccsid[] = "table.c	5.5 (Berkeley) 6/18/88";
#endif  not lint */

/*
 * Routines to handle insertion, deletion, etc on the table
 * of requests kept by the daemon. Nothing fancy here, linear
 * search on a double-linked list. A time is kept with each 
 * entry so that overly old invitations can be eliminated.
 *
 * Consider this a mis-guided attempt at modularity
 */
#include <stdio.h>
#include <sys/time.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <protocols/talkd.h>


#include "talkd_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALKD,n,s) 

#define MAX_ID 16000	/* << 2^15 so I don't have sign troubles */

#define NIL ((TABLE_ENTRY *)0)

extern	int debug;
struct	timeval tp;
struct	timezone *txp=NULL;

typedef struct table_entry TABLE_ENTRY;

struct table_entry {
	CTL_MSG request;
	int	time;
	TABLE_ENTRY *next;
	TABLE_ENTRY *last;
};

TABLE_ENTRY *table = NIL;
CTL_MSG *find_request();
CTL_MSG *find_match();
char	*malloc();

char fl_l_name1[8];
char fl_l_name2[8];
char fl_r_name1[8];
char fl_r_name2[8];

/*
 * Look in the table for an invitation that matches the current
 * request looking for an invitation
 */
CTL_MSG *
find_match(request)
	register CTL_MSG *request;
{
	register TABLE_ENTRY *ptr;
	int current_time;

	gettimeofday(&tp, txp);
	current_time = tp.tv_sec;
	if (debug)
		print_request(MSGSTR(FIND_MATCH, "find_match"), request); /*MSG*/
	for (ptr = table; ptr != NIL; ptr = ptr->next) {
		if ((ptr->time - current_time) > MAX_LIFE) {
			/* the entry is too old */
			if (debug)
				print_request(MSGSTR(DEL_EXP, "deleting expired entry"), /*MSG*/
				    &ptr->request);
			delete(ptr);
			continue;
		}
		if (debug)
			print_request("", &ptr->request);
		NLflatstr(request->l_name, fl_l_name1, 8);
		NLflatstr(ptr->request.l_name, fl_l_name2, 8);
		NLflatstr(request->r_name, fl_r_name1, 8);
		NLflatstr(ptr->request.r_name, fl_r_name2, 8);
		if (strcmp(fl_l_name1, fl_r_name2) == 0 &&
		    strcmp(fl_r_name1, fl_l_name2) == 0 &&
		     ptr->request.type == LEAVE_INVITE)
			return (&ptr->request);
	}
	return ((CTL_MSG *)0);
}

/*
 * Look for an identical request, as opposed to a complimentary
 * one as find_match does 
 */
CTL_MSG *
find_request(request)
	register CTL_MSG *request;
{
	register TABLE_ENTRY *ptr;
	int current_time;

	gettimeofday(&tp, txp);
	current_time = tp.tv_sec;
	/*
	 * See if this is a repeated message, and check for
	 * out of date entries in the table while we are it.
	 */
	if (debug)
		print_request(MSGSTR(FIND_REQ, "find_request"), request); /*MSG*/
	for (ptr = table; ptr != NIL; ptr = ptr->next) {
		if ((ptr->time - current_time) > MAX_LIFE) {
			/* the entry is too old */
			if (debug)
				print_request(MSGSTR(DEL_EXP, "deleting expired entry"), /*MSG*/
				    &ptr->request);
			delete(ptr);
			continue;
		}
		if (debug)
			print_request("", &ptr->request);
		NLflatstr(request->l_name, fl_l_name1, 8);
		NLflatstr(ptr->request.l_name, fl_l_name2, 8);
		NLflatstr(request->r_name, fl_r_name1, 8);
		NLflatstr(ptr->request.r_name, fl_r_name2, 8);
		if (strcmp(fl_r_name1, fl_r_name2) == 0 &&
		    strcmp(fl_l_name1, fl_l_name2) == 0 &&
		    request->type == ptr->request.type &&
		    request->pid == ptr->request.pid) {
			/* update the time if we 'touch' it */
			ptr->time = current_time;
			return (&ptr->request);
		}
	}
	return ((CTL_MSG *)0);
}

insert_table(request, response)
	CTL_MSG *request;
	CTL_RESPONSE *response;
{
	register TABLE_ENTRY *ptr;
	int current_time;

	gettimeofday(&tp, txp);
	current_time = tp.tv_sec;
	request->id_num = new_id();
	response->id_num = htonl(request->id_num);
	/* insert a new entry into the top of the list */
	ptr = (TABLE_ENTRY *)malloc(sizeof(TABLE_ENTRY));
	if (ptr == NIL) {
		syslog(LOG_ERR, MSGSTR(OUT_OF_MEM, "insert_table: Out of memory")); /*MSG*/
		_exit(1);
	}
	ptr->time = current_time;
	ptr->request = *request;
	ptr->next = table;
	if (ptr->next != NIL)
		ptr->next->last = ptr;
	ptr->last = NIL;
	table = ptr;
}

/*
 * Generate a unique non-zero sequence number
 */
new_id()
{
	static int current_id = 0;

	current_id = (current_id + 1) % MAX_ID;
	/* 0 is reserved, helps to pick up bugs */
	if (current_id == 0)
		current_id = 1;
	return (current_id);
}

/*
 * Delete the invitation with id 'id_num'
 */
delete_invite(id_num)
	int id_num;
{
	register TABLE_ENTRY *ptr;

	ptr = table;
	if (debug)
		syslog(LOG_DEBUG, MSGSTR(DEL_INVITE, "delete_invite(%d)"), id_num); /*MSG*/
	for (ptr = table; ptr != NIL; ptr = ptr->next) {
		if (ptr->request.id_num == id_num)
			break;
		if (debug)
			print_request("", &ptr->request);
	}
	if (ptr != NIL) {
		delete(ptr);
		return (SUCCESS);
	}
	return (NOT_HERE);
}

/*
 * Classic delete from a double-linked list
 */
delete(ptr)
	register TABLE_ENTRY *ptr;
{

	if (debug)
		print_request(MSGSTR(DELETE_MSG, "delete"), &ptr->request); /*MSG*/
	if (table == ptr)
		table = ptr->next;
	else if (ptr->last != NIL)
		ptr->last->next = ptr->next;
	if (ptr->next != NIL)
		ptr->next->last = ptr->last;
	free((char *)ptr);
}
