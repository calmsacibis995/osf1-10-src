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
static char	*sccsid = "@(#)$RCSfile: getty_sec.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:08:31 $";
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

/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc.
 * and should be treated as Confidential.
 */

#include <sys/secdefines.h>

#if SEC_BASE


/*

 */

#include <stdio.h>
#include <protcmd.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "getty_msg.h"
nl_catd catd;
#define MSGSTR_SEC(n,s) catgets(catd,MS_GETTY_SEC,n,s)
#else
#define MSGSTR_SEC(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif


#ifdef BSD_GETTY
#include <syslog.h>
#endif

static void execute();

extern char *strrchr();

/*
 * Just prior to opening the terminal, set up the modes properly and
 * use stopio() to revoke all rights to the line by processes that already
 * have the line open.  Future opens (like the one about to be done in getty)
 * are not affected.
 */
void
getty_condition_line(line)
	register char *line;
{
	execute(line, -1);
}

#ifndef BSD_GETTY
/*
 * Return the full pathname of the login program.
 */
char *
getty_login_program()
{
	return LOGIN_PROGRAM;
}
#endif

/*
 * Call an auxiliary program to do the security actions.
 */
static void
execute(name, fd)
	register char *name;
	register int fd;
{
	register char *cmd_end;
	register int pid;
	register int wait_ret;
	int wait_stat;
	char fd_num[20];

	cmd_end = strrchr(INITCOND_PROGRAM, '/');
	if (cmd_end == (char *) 0)
		cmd_end = INITCOND_PROGRAM;
	else
		cmd_end++;

	pid = fork();
	switch (pid)  {
	    case -1:
#ifdef BSD_GETTY
		syslog(LOG_ERR, MSGSTR_SEC(S_CANT_FORK1, "getty: can not fork a subprocess"));
		closelog();
#else
		error(MSGSTR_SEC(S_CANT_FORK2, "getty: cannot fork a subprocess\r\n"));
#endif
		break;

	    case 0:
		if (fd >= 0)  {
			(void) sprintf(fd_num, "%d", fd);
			(void) execl(INITCOND_PROGRAM, cmd_end, "getty", name,
				fd_num, (char *) 0);
		}
		else
			(void) execl(INITCOND_PROGRAM, cmd_end, "getty", name,
				(char *) 0);
#ifdef BSD_GETTY
		syslog(LOG_ERR, MSGSTR_SEC(S_CANT_EXEC1, "getty: cannot execute %s: %m"),
				INITCOND_PROGRAM);
		closelog();
#else
		error(MSGSTR_SEC(S_CANT_EXEC2, "getty: cannot execute %s\r\n"), INITCOND_PROGRAM);
#endif
		exit(1);
		break;

	    default:
		do  {
			wait_ret = wait(&wait_stat);
		}
		while ((wait_ret != -1) && (wait_ret != pid));

		if (wait_stat != 0)
#ifdef BSD_GETTY
			syslog(LOG_ERR, MSGSTR_SEC(S_EXIT_CODE1, "getty: %s returned exit code 0x%x"),
				INITCOND_PROGRAM, wait_stat);
#else
			error(MSGSTR_SEC(S_EXIT_CODE2, "getty: %s returned exit code 0x%x\r\n"),
				INITCOND_PROGRAM, wait_stat);
#endif
		break;
	}
}
#endif /* SEC_BASE */
