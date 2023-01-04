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
static char rcsid[] = "@(#)$RCSfile: crontab_sec.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 14:18:59 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1988-1990, SecureWare, Inc.  All rights reserved.
 */
/* #ident "@(#)crontab_sec.c	4.1 10:11:07 7/12/90 SecureWare" */
/*
 * Based on:	@(#)crontab_sc.c	2.8 16:01:05 10/19/89
 */

#include <sys/secdefines.h>

#if SEC_BASE

#include "cron_msg.h"
nl_catd catd;
#define MSGSTR_SEC(Num,Str) catgets(catd,MS_CRON_SEC,Num,Str)

#include <sys/security.h>
#include <prot.h>
#include <fcntl.h>

#if SEC_MAC || SEC_NCAV

#include <stdio.h>

extern char *malloc();

#endif /* SEC_MAC || SEC_NCAV */

/*
 * Create the crontab so that it cannot be tampered by other than the
 * cron utilities.
 */
int
crontab_secure_create(file_name)
	char	*file_name;
{
	int	cfs_status;

	cfs_status = create_file_securely(file_name, AUTH_SILENT,
					  MSGSTR_SEC(CRONTAB_SEC_1, "create crontab file"));
	if (cfs_status != CFS_GOOD_RETURN)
		return -1;
	return open(file_name, O_WRONLY);
}


#if SEC_MAC || SEC_NCAV
/*
 * Set up the message buffer to send to the cron daemon.  Include
 * the security level of the sender, obtaining that information from
 * getslabel().
 */
char *
crontab_set_message(base_size)
	int base_size;
{
	char *message;
	int msg_size = base_size;

#if SEC_MAC
	if (mand_init()) {
		fprintf(stderr, MSGSTR_SEC(CRONTAB_SEC_2, "Cannot initialize for sensitivity labels.\n"));
		exit(1);
	}
	msg_size += mand_bytes();
#endif
#if SEC_NCAV
	if (ncav_init()) {
		fprintf(stderr, "Cannot initialize for nationality caveats.\n");
		exit(1);
	}
	msg_size += sizeof(ncav_ir_t);
#endif

	message = malloc(msg_size);
	if (message == (char *) 0) {
		fprintf(stderr, MSGSTR_SEC(CRONTAB_SEC_4, "Cannot allocate cron message\n"));
		exit(1);
	}
	msg_size = base_size;

#if SEC_MAC
	if (getslabel((mand_ir_t *) (message + msg_size)) < 0) {
		fprintf(stderr, MSGSTR_SEC(CRONTAB_SEC_5, "Cannot get your sensitivity level\n"));
		exit(1);
	}
	msg_size += mand_bytes();
#endif
#if SEC_NCAV
	if (getncav((ncav_ir_t *) (message + msg_size)) < 0) {
		fprintf(stderr, MSGSTR_SEC(CRONTAB_SEC_6, "Cannot get your nationality caveats\n"));
		exit(1);
	}
#endif

	return message;
}


/*
 * Write the message to the cron daemon, including the security level.
 * Also, free the space used for the message.
 */
int
crontab_write_message(fd, message, base_size)
	int fd;
	char *message;
	int base_size;
{
	int status;

#if SEC_MAC
	base_size += mand_bytes();
#endif
#if SEC_NCAV
	base_size += sizeof(ncav_ir_t);
#endif
	status = (write(fd, message, base_size) == base_size);

	free(message);

	return status;
}
#endif /* SEC_MAC || SEC_NCAV */

#endif /* SEC_BASE */
