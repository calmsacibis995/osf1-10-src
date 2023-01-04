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
static char *rcsid = "@(#)$RCSfile: ses_estab.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/04 21:23:23 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_estab((*sia_collect),
*				SIAENTITY *entity,
*				int pkgind)
*
* Description: The purpose of this routine is to do session establishment.
* The assumption is that all necessary authentication has been accomplished.
* The work done by this routine consists of collecting any additional state
* required to actually launch a session. If the collect function pointer is
* NULL this is a non_interactive request.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
#include <varargs.h>
#include <sia.h>
#include <siad.h>
#include "sia_mech.h"

siad_ses_estab(collect,entity,pkgind)
int (*collect)();
SIAENTITY *entity;
int pkgind;
{
	extern int errno;
	struct passwd *pwd;
        prompt_t warn_prompt, info_prompt;
	struct pr_passwd *prpwd=NULL;
	struct pr_term *prtc=NULL;
	int passwd_req=1;
	char *tty=NULL;
	extern int (*_c2_collect)();

	_c2_collect = collect;
	if(!entity)
		return SIADFAIL;
/*
 * If necessary, resolve local account name.
 */
	if(!entity->pwd || !EN_MECH(entity,pkgind) || !EN_PR_PWD(entity,pkgind)) {
		if(!entity->acctname) {
			if(!entity->name) {
				return SIADFAIL;
			}
			entity->acctname = strdup(entity->name);
		}
/*
 * Retrieve passwd and protected passwd entries for the account.
 */
		if(!login_fillin_user(entity->acctname, &prpwd, &pwd)) {
			return SIADFAIL;
		}
/*
 * copy passwd and protected passwd entries into the entity structure.
 */
		c2_make_mech(entity, pkgind, prpwd, (PR_TERM *) 0);
		sia_make_entity_pwd(pwd, entity);
	} else {
/*
 * Local name and passwd information alread exists.  Get pointers to it.
 */
		pwd = entity->pwd;
		prpwd = EN_PR_PWD(entity, pkgind);
	}
/*
 * Last check for presence of protected password information.
 */
	if(!prpwd)
		return SIADFAIL;
/*
 * If necessary, look up port access information.
 */
	if(entity->tty && !(prtc=EN_PR_TRM(entity, pkgind))) {
#if	SEC_NET_TTY
		if(entity->hostname)
			prtc = login_net_term_params(entity->tty, entity->hostname);
		else
#endif
/*
 * Get terminal name from terminal device pathname.
 */
		if(entity->tty) {
			if(tty=rindex(entity->tty, '/'))
				tty++;
			else
				tty = entity->tty;
			prtc = login_term_params(entity->tty, tty);
		}
		if(!prtc) {
			return SIADFAIL;
		}
		EN_PR_TRM(entity,pkgind) = prtc;
	}
/*
 * Create session based on type of authentication performed.  If no
 * authentication was done assume full blown session creation as in
 * the sia_ses_authent case.
 */
	if(entity->authtype == SIA_A_NONE || entity->authtype == SIA_A_AUTH) {
/*
 * Check if logins disabled for everyone (except root).
 */
		if(entity->tty && checknologin())
			return SIADFAIL;
	if(!login_set_user(prpwd, prtc, pwd)) {
		return SIADFAIL;
	}
	alarm((unsigned) 0);
/*
 * Force setting of password if required.
 */
	if(!login_need_passwd(prpwd, prtc, &passwd_req))
		return SIADFAIL;
/*
 * Force resetting of expired password if required.
 */
	prpwd = login_check_expired(prpwd, prtc);
	(void) setluid(prpwd->ufld.fd_uid);	/* make sure of audit id */
#ifdef Q_SETUID
/*
 * Verify resources available to user.
 */
	if (quota(Q_SETUID, pwd->pw_uid, 0, (char *)0) < 0 && errno != EINVAL) {
		switch(errno) {
		case EUSERS:
			if(entity->tty) {
				(void)show_error(
				    MSGSTR(TOO_MANY_USR, "Too many users logged on already.\nTry again later."));
				return SIADFAIL;
			}
			break;
		case EPROCLIM:
			(void)show_error(
			    MSGSTR(TOO_MANY_PROC, "You have too many processes running."));
			return SIADFAIL;
		default:
			(void)show_perror("quota"); /* Not thread safe */
			return SIADFAIL;
		}
	}
#endif

        /* Capacity limit exceeded? */                              /* BVT */

	if (entity->tty && (pwd == NULL || pwd->pw_uid))
		if(setsysinfo(SSI_ULIMIT,0,0,0,0) != 0) {
			show_error(
			    MSGSTR(TOO_MANY_USR, "Too many users logged on already.\nTry again later."));
			return SIADFAIL;
		}
	} else if(entity->authtype == SIA_A_SUAUTH) {
/*
 * Create sub-session as for the su utility.
 */
/* Inelegant way of telling if this is a full su. -DAL */
		if(entity->argc >= 2 && !strcmp(entity->argv[1], "-"))
			su_ensure_admission(1, (char *) 0);
		else
			su_ensure_admission(0, (char *) 0);
	} else
/*
 * Impossible auth type.
 */
		return SIADFAIL;
	return SIADSUCCESS;
}
