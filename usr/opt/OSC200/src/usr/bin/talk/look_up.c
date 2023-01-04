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
static char rcsid[] = "@(#)$RCSfile: look_up.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 17:29:41 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* 
 * COMPONENT_NAME: TCPIP look_up.c
 * 
 * FUNCTIONS: MSGSTR, check_local, look_for_invite 
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
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/* look_up.c	1.4  com/sockcmd/talk,3.1,9021 10/8/89 17:28:55 */

#include "talk_ctl.h"

#include "talk_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 

/*
 * See if the local daemon has an invitation for us.
 */
check_local()
{
	CTL_RESPONSE response;
	register CTL_RESPONSE *rp = &response;

	/* the rest of msg was set up in get_names */
	msg.ctl_addr = *(struct sockaddr *)&ctl_addr;
	msg.ctl_addr.sa_family = htons(msg.ctl_addr.sa_family);
	/* must be initiating a talk */
	if (!look_for_invite(rp))
		return (0);
	/*
	 * There was an invitation waiting for us, 
	 * so connect with the other (hopefully waiting) party 
	 */
	current_state = MSGSTR(WAIT_TO_CONN, "Waiting to connect with caller");
	do {
		if (rp->addr.sa_family != AF_INET)
			p_error(MSGSTR(ERR_INV_RESP, "Response uses invalid network address")); /*MSG*/
		errno = 0;
		if (connect(sockt, &rp->addr, sizeof (rp->addr)) != -1)
			return (1);
	} while (errno == EINTR);
	if (errno == ECONNREFUSED) {
		/*
		 * The caller gave up, but his invitation somehow
		 * was not cleared. Clear it and initiate an 
		 * invitation. (We know there are no newer invitations,
		 * the talkd works LIFO.)
		 */
		ctl_transact(his_machine_addr, msg, DELETE, rp);
		close(sockt);
		open_sockt();
		return (0);
	}
	p_error(MSGSTR(ERR_CONNECT, "Unable to connect with initiator")); /*MSG*/
	/*NOTREACHED*/
}

/*
 * Look for an invitation on 'machine'
 */
look_for_invite(rp)
	CTL_RESPONSE *rp;
{
	struct in_addr machine_addr;

	current_state = MSGSTR(CHK_INVITE, "Checking for invitation on caller's machine");
	ctl_transact(his_machine_addr, msg, LOOK_UP, rp);
	/* the switch is for later options, such as multiple invitations */
	switch (rp->answer) {

	case SUCCESS:
		msg.id_num = htonl(rp->id_num);
		return (1);

	default:
		/* there wasn't an invitation waiting for us */
		return (0);
	}
}
