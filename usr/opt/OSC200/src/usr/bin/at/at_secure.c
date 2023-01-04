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
static char rcsid[] = "@(#)$RCSfile: at_secure.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 14:34:07 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 */
#if SEC_BASE

/* #ident "@(#)at_secure.c	4.2 15:18:08 7/11/90 SecureWare" */
/*
 * Based on:	at_secure.c	2.9 14:36:07 11/1/89
 */

#include <sys/security.h>

#if SEC_MAC || SEC_NCAV
#include <stdio.h>
#include "cron_msg.h"
nl_catd catd;
#define MSGSTR_SEC(Num,Str) catgets(catd,MS_CRON_SEC,Num,Str)

extern char *malloc();

#if SEC_MAC
#include <mandatory.h>
#endif
#if SEC_NCAV
#include <ncav.h>
#endif
#endif /* SEC_MAC || SEC_NCAV */

/*
 * Check user's authorization to view and delete jobs belonging
 * to other users.
 */
at_authorized()
{
	static int	is_authorized = -1;

	if (is_authorized == -1)
		is_authorized = authorized_user("cron");
	return is_authorized;
}

#if SEC_MAC || SEC_NCAV
/*
 * Allocate a message buffer for communicating with the daemon.
 * Include space for a sensitivity label.
 */
char *
at_alloc_message(base_size)
	int	base_size;
{
	static char	*message = (char *) 0;
	int		msg_size = base_size;

	/*
	 * If message already allocated, just return it.
	 */
	if (message == (char *) 0) {
#if SEC_MAC
		if (mand_init()) {
			fprintf(stderr, 
				MSGSTR_SEC(AT_SEC_1, "Cannot initialize for sensitivity labels.\n"));
			exit(1);
		}
		msg_size += mand_bytes();
#endif
#if SEC_NCAV
		msg_size += sizeof(ncav_ir_t);
#endif

		message = malloc(msg_size);
		if (message == (char *) 0) {
			fprintf(stderr,
				MSGSTR_SEC(AT_SEC_2, "Cannot allocate space for daemon message.\n"));
			exit(1);
		}
		msg_size = base_size;

#if SEC_MAC
		if (getslabel((mand_ir_t *) &message[msg_size]) < 0) {
			fprintf(stderr,MSGSTR_SEC(AT_SEC_3, "Cannot get your sensitivity label.\n"));
			exit(1);
		}
		msg_size += mand_bytes();
#endif
#if SEC_NCAV
		if (getncav((ncav_ir_t *) &message[msg_size]) < 0) {
			fprintf(stderr,
				"Cannot get your nationality caveats.\n");
			exit(1);
		}
#endif
	}

	return message;
}


/*
 * Write a message to the cron daemon, including label information.
 */
at_send_message(fd, message, msg_size)
	int	fd;
	char	*message;
	int	msg_size;
{

#if SEC_MAC
	msg_size += mand_bytes();
#endif
#if SEC_NCAV
	msg_size += sizeof(ncav_ir_t);
#endif

	return write(fd, message, msg_size) == msg_size;
}
#endif /* SEC_MAC || SEC_NCAV */

#endif /* SEC_BASE */
