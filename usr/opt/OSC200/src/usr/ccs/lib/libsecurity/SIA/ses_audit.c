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
#include <sia.h>
#include <siad.h>
#include <sia_mech.h>
#include <sys/audit.h>

int sia_entity_audit(entity, pkgind, message)
SIAENTITY *entity;
char *message;
int pkgind;
{
	struct pr_passwd *prpwd;
	char *progname;

	if(!entity)
		return 0;
	if(entity->argc > 0 && entity->argv)
		progname = entity->argv[0];
	prpwd = EN_PR_PWD(entity, pkgind);
	if(entity->hostname) {
		if (audgenl(LOGIN,
				T_LOGIN,   entity->pwd->pw_name,
				T_SHELL,   entity->pwd->pw_shell,
				T_UID,     entity->pwd->pw_uid,
				T_HOMEDIR, entity->pwd->pw_dir,
				T_DEVNAME, entity->tty,
				T_HOSTADDR, entity->hostname,
				T_CHARP,   entity->argv[0],
				T_CHARP,   message, NULL) == -1)
			perror("audgenl");
	} else {
		switch(entity->authtype) {
		case SIA_A_NONE:
		case SIA_A_AUTH:
			if (audgenl(LOGIN,
					T_LOGIN,   entity->pwd->pw_name,
					T_SHELL,   entity->pwd->pw_shell,
					T_UID,     entity->pwd->pw_uid,
					T_HOMEDIR, entity->pwd->pw_dir,
					T_DEVNAME, entity->tty,
					T_CHARP,   message, NULL) == -1)
				perror("audgenl");
			break;
		default:
			if (audgenl(AUTH_EVENT,
					T_CHARP,   progname,
					T_CHARP,   message, NULL) == -1)
				perror("audgenl");
		}
	}
	return 1;
}
