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
static char *rcsid = "@(#)$RCSfile: ses_suauth.c,v $ $Revision: 1.1.7.5 $ (DEC) $Date: 1993/12/16 23:55:59 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_suauthent((*sia_collect),
*				SIAENTITY *entity,
*				int siastat,
*				int pkgind)
*
* Description: The purpose of this routine is to do session authentication
* for the su command.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
#include <sys/security.h>
#include <stdio.h>
#include <stdlib.h>
#include <sia.h>
#include <siad.h>
#include "sia_mech.h"

#ifdef MSG
#include "libsec_msg.h" 
#define MSGSTR_SEC(n,s) GETMSG(MS_SIA,n,s) 
#else
#define MSGSTR_SEC(n,s) s
#endif

siad_ses_suauthent(collect,entity,siastat,pkgind)
int (*collect)();
SIAENTITY *entity;
int siastat;
int pkgind;
{
	prompt_t prompt[2];
	char *p, auditbuf[80], *name;
	struct passwd *pwd;
	extern int (*_c2_collect)();
	extern int _c2_collinput;
	extern int su_crypt_alg;

	_c2_collect = collect;
	if(!entity)
		return SIADFAIL;
	_c2_collinput = entity->colinput;
	entity->authtype = SIA_A_SUAUTH;
/*
 * Identify invoker.
 */
	if ((pwd = getpwuid(getuid())) == NULL) {
		sprintf(auditbuf, MSGSTR_SEC(S_GET_PWD1, "get password entry for id %d"), getuid());
		audit_subsystem(auditbuf, MSGSTR_SEC(S_NOT_FOUND, "entry not found; abort su"),
			18 /* ET_SUBSYSTEM */);
		show_error(MSGSTR(WHO_ARE_YOU, "Who are you?\n"));
		exit(1);
	}
/*
 * Create link to mechanism specific data (C2 shadow records).
 */
	if(!EN_MECH(entity,pkgind))
		c2_make_mech(entity, pkgind, (PR_PASSWD *) 0, (PR_TERM *) 0);
/*
 * Get local account name, try global name if local not available.
 */
	if(!entity->acctname)
		if(entity->name)
			entity->acctname = strdup(entity->name);
	if (!entity->name)
		return SIADFAIL;	/* must pass in the name */
/*
 * Identify target user.
 */
	if ((pwd = getpwnam(entity->acctname)) == NULL) {
		sprintf(auditbuf, MSGSTR_SEC(S_GET_PWD2, "get password entry for %s"), entity->name);
		audit_subsystem(auditbuf, MSGSTR_SEC(S_NOT_FOUND, "entry not found; abort su"),
			18 /* ET_SUBSYSTEM */);
		show_error(MSGSTR_SEC(S_UNKNOWN1, "su: Unknown id: %s\n"),
			entity->name);

		return SIADFAIL;
	}
/*
 * Copy passwd entry into entity structure.
 */
	sia_make_entity_pwd(pwd, entity);
/*
 * Authenticate target user.
 */
	su_ensure_secure(pwd, &EN_PR_PWD(entity, pkgind));
/*
 * Only ask for the password now, and if we need it.
 */
	if (!entity->password && pwd->pw_passwd && *pwd->pw_passwd) {
/*
 * No password, collect password only.
 */
		if(collect && entity->colinput) {
			prompt[0].result = malloc(MAX_PWD_LENGTH+1);
			prompt[0].min_result_length = 0;
			prompt[0].max_result_length = MAX_PWD_LENGTH;
			prompt[0].control_flags = SIARESINVIS;
			prompt[0].prompt = (unsigned char *) MSGSTR(LOGIN_SEC_52, "Password:");
			if(((*collect)(0, SIAONELINER, "", 1, prompt)) == SIACOLSUCCESS)
				entity->password = (char *)prompt[0].result;
			else
				return SIADFAIL|SIADSTOP;
		} else
			return SIADFAIL;
	}
	else if (!entity->password)
		entity->password = "";
/* su_ensure_secure made pwd->pw_passwd point to the C2 password. */
	if (pwd->pw_passwd && strcmp(pwd->pw_passwd,
	    *pwd->pw_passwd ? dispcrypt(entity->password, pwd->pw_passwd, su_crypt_alg)
			    : entity->password)) {
		show_error(MSGSTR_SEC(S_SORRY, "Sorry\n"));
		return SIADFAIL;
	}

	return SIADSUCCESS;
}
