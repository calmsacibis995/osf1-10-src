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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: invite.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 17:26:59 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* 
 * COMPONENT_NAME: TCPIP invite.c
 * 
 * FUNCTIONS: MSGSTR, announce_invite, invite_remote, re_invite, 
 *            send_delete 
 *
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
/* invite.c	1.4  com/sockcmd/talk,3.1,9021 10/8/89 17:28:28 */
/*
 "invite.c	5.5 (Berkeley) 6/29/88";
*/

#include "talk_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 

#include "talk_ctl.h"
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>

/*
 * There wasn't an invitation waiting, so send a request containing
 * our sockt address to the remote talk daemon so it can invite
 * him 
 */

/*
 * The msg.id's for the invitations
 * on the local and remote machines.
 * These are used to delete the 
 * invitations.
 */
int	local_id, remote_id;
void	re_invite();
jmp_buf invitebuf;

invite_remote()
{
	int new_sockt;
	struct itimerval itimer;
	CTL_RESPONSE response;

	itimer.it_value.tv_sec = RING_WAIT;
	itimer.it_value.tv_usec = 0;
	itimer.it_interval = itimer.it_value;
	if (listen(sockt, 5) != 0)
		p_error(MSGSTR(ERROR_LISTEN, "Error on attempt to listen for caller")); /*MSG*/
	msg.addr = *(struct sockaddr *)&my_addr;
	msg.addr.sa_family = htons(msg.addr.sa_family);
	msg.id_num = htonl(-1);		/* an impossible id_num */
	invitation_waiting = 1;
	announce_invite();
	/*
	 * Shut off the automatic messages for a while,
	 * so we can use the interupt timer to resend the invitation
	 */
	end_msgs();
	setitimer(ITIMER_REAL, &itimer, (struct itimerval *)0);
	message(MSGSTR(WAIT_TO_RESP, "Waiting for your party to respond")); /*MSG*/
	signal(SIGALRM, re_invite);
	(void) setjmp(invitebuf);
	while ((new_sockt = accept(sockt, 0, 0)) < 0) {
		if (errno == EINTR)
			continue;
		p_error(MSGSTR(ERROR_CONN, "Unable to connect with your party")); /*MSG*/
	}
	close(sockt);
	sockt = new_sockt;

	/*
	 * Have the daemons delete the invitations now that we
	 * have connected.
	 */
	current_state = MSGSTR(WAIT_RESP, "Waiting for your party to respond");
	start_msgs();

	msg.id_num = htonl(local_id);
	ctl_transact(my_machine_addr, msg, DELETE, &response);
	msg.id_num = htonl(remote_id);
	ctl_transact(his_machine_addr, msg, DELETE, &response);
	invitation_waiting = 0;
}

/*
 * Routine called on interupt to re-invite the callee
 */
void
re_invite()
{

	message(MSGSTR(RINGING_AGAIN, "Ringing your party again")); /*MSG*/
	current_line++;
	/* force a re-announce */
	msg.id_num = htonl(remote_id + 1);
	announce_invite();
	longjmp(invitebuf, 1);
}

static	char *answers[] = {
	"answer #0",					/* SUCCESS */
	"Your party is not logged on",			/* NOT_HERE */
	"Target machine is too confused to talk to us",	/* FAILED */
	"Target machine does not recognize us",		/* MACHINE_UNKNOWN */
	"Your party is refusing messages",		/* PERMISSION_REFUSED */
	"Target machine can not handle remote talk",	/* UNKNOWN_REQUEST */
	"Target machine indicates protocol mismatch",	/* BADVERSION */
	"Target machine indicates protocol botch (addr)",/* BADADDR */
	"Target machine indicates protocol botch (ctl_addr)",/* BADCTLADDR */
};
#define	NANSWERS	(sizeof (answers) / sizeof (answers[0]))

/*
 * Transmit the invitation and process the response
 */
announce_invite()
{
	CTL_RESPONSE response;

	current_state = MSGSTR(CONN_TALK_DAEM, "Trying to connect to your party's talk daemon");
	ctl_transact(his_machine_addr, msg, ANNOUNCE, &response);
	remote_id = response.id_num;
	if (response.answer != SUCCESS) {
		if (response.answer < NANSWERS)
			message(MSGSTR(MSG_SUCCESS + response.answer, answers[response.answer]));
		quit();
	}
	/* leave the actual invitation on my talk daemon */
	ctl_transact(my_machine_addr, msg, LEAVE_INVITE, &response);
	local_id = response.id_num;
}

/*
 * Tell the daemon to remove your invitation
 */
send_delete()
{

	msg.type = DELETE;
	/*
	 * This is just a extra clean up, so just send it
	 * and don't wait for an answer
	 */
	msg.id_num = htonl(remote_id);
	daemon_addr.sin_addr = his_machine_addr;
	if (sendto(ctl_sockt, &msg, sizeof (msg), 0, (struct sockaddr *)&daemon_addr,
	    sizeof (daemon_addr)) != sizeof(msg))
		perror(MSGSTR(ERR_SND_DEL_R, "send_delete (remote)")); /*MSG*/
	msg.id_num = htonl(local_id);
	daemon_addr.sin_addr = my_machine_addr;
	if (sendto(ctl_sockt, &msg, sizeof (msg), 0, (struct sockaddr *)&daemon_addr,
	    sizeof (daemon_addr)) != sizeof (msg))
		perror(MSGSTR(ERR_SND_DEL, "send_delete (local)")); /*MSG*/
}
