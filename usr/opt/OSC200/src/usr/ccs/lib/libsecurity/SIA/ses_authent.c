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
static char *rcsid = "@(#)$RCSfile: ses_authent.c,v $ $Revision: 1.1.8.4 $ (DEC) $Date: 1993/08/24 22:32:39 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_authent((*sia_collect),
*				SIAENTITY *entity,
*				int siastat,
*				int pkgind)
*
* Description: The purpose of this routine is to do session authentication.
* If the collect function pointer is NULL this is a non-interactive request.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
#include <sys/security.h>
#include <sys/audit.h>
#include <sia.h>
#include <siad.h>
#include <string.h>
#include "sia_mech.h"

#define	OVERRIDE_USER	"root"

#ifdef MSG
#include "libsec_msg.h" 
#define MSGSTR_SEC(n,s) GETMSG(MS_SIA,n,s) 
#else
#define MSGSTR_SEC(n,s) s
#endif

siad_ses_authent(collect,entity,siastat,pkgind)
int (*collect)();
SIAENTITY *entity;
int siastat;
int pkgind;
{
	prompt_t prompt[2];
	struct passwd *pwd;
	struct pr_passwd *prpwd;
	struct pr_term *prtc;
	int passwd_req, i;
	char *tty;
	extern int (*_c2_collect)();
	extern int _c2_collinput;
	int crypt_alg;

	_c2_collect = collect;
	if(!entity) {
		return SIADFAIL;
	}
	_c2_collinput = entity->colinput;

	entity->authtype = SIA_A_AUTH;
	if(!login_set_sys()) {
		return SIADFAIL;
	}
	passwd_req = entity->password?!entity->password[0]:0;
/*
 * Create link to mechanism specific data (C2 shadow records).
 */
	if(EN_MECH(entity, pkgind)) {
		free(EN_MECH(entity, pkgind));
		entity->mech[pkgind] = (int *) 0;
	}
	c2_make_mech(entity, pkgind, (PR_PASSWD *) 0, (PR_TERM *) 0);
/*
 * Get terminal access information.
 */
	if(entity->tty) {
/*
 * Derive tty name from tty pathname.
 */
		if(tty=strrchr(entity->tty, '/'))
			++tty;
		else
			tty = entity->tty;
/*
 * Get port access parameters.
 */
#if SEC_NET_TTY
		if(entity->hostname)
			prtc = login_net_term_params(entity->tty, entity->hostname);
		else
#endif
		prtc = login_term_params(entity->tty, tty);
		if(!prtc)
			return SIADFAIL|SIADSTOP;
		EN_PR_TRM(entity,pkgind) = prtc;
	} else
		prtc = (struct pr_term *) 0;
/*
 * Get user or application supplied authentication information.
 */
	if(!entity->acctname) {
/*
 * No local account name, try using global name.
 */
		if(entity->name)
/*
 * Make local name from global name.
 */
			entity->acctname = strdup(entity->name);
	}
	if((!entity->name) && (!entity->password)) {
/*
 * No local name, global name, or password, collect name and password.
 */
		if(collect && entity->colinput) {
			prompt[0].result = (u_char *) malloc(MAX_LOGIN_LENGTH+1);
			prompt[0].min_result_length = 0;
			prompt[0].max_result_length = MAX_LOGIN_LENGTH;
			prompt[0].control_flags = 0;
			prompt[0].prompt = (unsigned char *) MSGSTR(ENTRY_LOGIN, "login: ");
			prompt[1].result = (u_char *) malloc(MAX_PWD_LENGTH+1);
			prompt[1].min_result_length = 0;
			prompt[1].max_result_length = MAX_PWD_LENGTH;
			prompt[1].control_flags = SIARESINVIS;
			prompt[1].prompt = (unsigned char *) MSGSTR(LOGIN_SEC_52, "Password:");
			if(((*collect)(0, SIAFORM, "", 2, prompt)) == SIACOLSUCCESS) {
				entity->name = (char *) prompt[0].result;
				entity->acctname = strdup(entity->name);
				entity->password = (char *) prompt[1].result;
			} else
/*
 * Problem collecting name, fail.
 */
				return SIADFAIL|SIADSTOP;
		} else
/*
 * Collection of name not possible, fail.
 */
			return SIADFAIL;
	} else if((!entity->name) && entity->password) {
/*
 * No name, collect name only.  Strange case.
 */
		if(collect && entity->colinput) {
			prompt[0].result = (u_char *) malloc(MAX_PWD_LENGTH+1);
			prompt[0].min_result_length = 0;
			prompt[0].max_result_length = MAX_PWD_LENGTH;
			prompt[0].control_flags = SIAONELINER;
			prompt[0].prompt = (unsigned char *) MSGSTR(ENTRY_LOGIN, "login: ");
			if(((*collect)(0, SIAONELINER, "", 1, prompt)) == SIACOLSUCCESS) {
				entity->name = (char *) prompt[0].result;
				entity->acctname = strdup(entity->name);
			} else
				return SIADFAIL|SIADSTOP;
		} else
			return SIADFAIL;
	} else if(entity->name && (!entity->password)) {
/*
 * No password, collect password only.
 */
		if(collect && entity->colinput) {
			prompt[0].result = (u_char *) malloc(MAX_PWD_LENGTH+1);
			prompt[0].min_result_length = 0;
			prompt[0].max_result_length = MAX_PWD_LENGTH;
			prompt[0].control_flags = SIARESINVIS;
			prompt[0].prompt = (unsigned char *) MSGSTR(LOGIN_SEC_52, "Password:");
			if(((*collect)(0, SIAONELINER, "", 1, prompt)) == SIACOLSUCCESS)
				entity->password = (char *) prompt[0].result;
			else
				return SIADFAIL|SIADSTOP;
		} else
			return SIADFAIL;
	}
/*
 * Retrieve user's passwd and protected passwd records.
 */
	if(!login_fillin_user(entity->acctname, &prpwd, &pwd)) {
		return SIADFAIL;
	}
/*
 * Copy the passwd and protected passwd entries into the
 * entity structure.
 */
	EN_PR_PWD(entity,pkgind) = prpwd;
	sia_make_entity_pwd(pwd, entity);
/*
 * Authenticate user.
 */
	if(entity->tty) {
		i = login_validate(&prpwd, &prtc, &passwd_req, entity->password);
		EN_PR_PWD(entity,pkgind) = prpwd;
		EN_PR_TRM(entity,pkgind) = prtc;
		if(!i)
			return SIADFAIL;
	} else {
		if(!prpwd) {
			sia_entity_audit(entity, pkgind,
				MSGSTR_SEC(LOGIN_SEC_48, "no entry in protected password database"));
			return SIADFAIL;	/* failure_overide? */
		}
		if(entity->tty && prtc == (struct pr_term *) 0 &&
		    strcmp(prpwd->ufld.fd_name, OVERRIDE_USER)) {
			sia_entity_audit(entity, pkgind,
MSGSTR_SEC(LOGIN_SEC_49, "no entry for terminal in terminal control database"));
			return 0; /* Don't exit(1) - DAL */
		}
/* rsh doesn't like informational messages during authentication. -DAL */
		if(entity->tty && prpwd->uflg.fg_pwchanger) {
			char *pwchanger, buf[16];

			pwchanger = pw_idtoname(prpwd->ufld.fd_pwchanger);
			if (pwchanger == NULL) {
				sprintf(buf, MSGSTR_SEC(LOGIN_SEC_50,
				    "uid #%u"), prpwd->ufld.fd_pwchanger);
				pwchanger = buf;
			}
			show_mesg(MSGSTR_SEC(LOGIN_SEC_51,
			    "Your password was changed by %s on %s"), pwchanger,
			    ctime(&prpwd->ufld.fd_schange));
		}
		if (prpwd->uflg.fg_oldcrypt)
			crypt_alg = prpwd->ufld.fd_oldcrypt;
		else if (prpwd->sflg.fg_oldcrypt)
			crypt_alg = prpwd->sfld.fd_oldcrypt;
		else
			crypt_alg = AUTH_CRYPT_BIGCRYPT;

		if(prpwd->ufld.fd_encrypt[0] == '\0') {
			if(entity->tty)
				if(!check_valid_tty(prtc, prpwd))
					return SIADFAIL;
			if (entity->password && entity->password[0])
				return SIADFAIL;
		} else  {
			passwd_req = 0;
	
			if(entity->password &&
			    (strcmp(dispcrypt(entity->password,
			    prpwd->ufld.fd_encrypt,crypt_alg),
				    prpwd->ufld.fd_encrypt) == 0)) {
				if(entity->tty)
					if(!check_valid_tty(prtc, prpwd))
						return SIADFAIL;
			} else if(entity->colinput && !entity->password &&
			    strcmp(login_crypt(MSGSTR_SEC(LOGIN_SEC_52, "Password:"),
			    prpwd->ufld.fd_encrypt, crypt_alg),
			    prpwd->ufld.fd_encrypt) == 0) {
				if(entity->tty)
					if(!check_valid_tty(prtc, prpwd))
						return SIADFAIL;
			} else {
				if (prpwd && prtc) {
					prpwd = login_bad_user(prpwd, prtc);
					prtc = login_bad_tty(prtc, prpwd);
					EN_PR_PWD(entity,pkgind) = prpwd;
					EN_PR_TRM(entity,pkgind) = prtc;
				}
				show_mesg(MSGSTR_SEC(LOGIN_SEC_53, "Login incorrect\n"));
				sia_entity_audit(entity, pkgind, MSGSTR_SEC(LOGIN_SEC_54, "login incorrect"));
				if(entity->colinput)
					login_delay(MSGSTR_SEC(LOGIN_SEC_55, "retry"));
				return SIADFAIL;
			}
		}
	}

	return SIADSUCCESS;
}
