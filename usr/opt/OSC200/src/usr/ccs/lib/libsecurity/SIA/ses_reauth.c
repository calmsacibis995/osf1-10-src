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
static char *rcsid = "@(#)$RCSfile: ses_reauth.c,v $ $Revision: 1.1.7.4 $ (DEC) $Date: 1993/08/24 22:32:52 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_reauthent((*sia_collect),
*				SIAENTITY *entityhdl,
*				int siastat,
*				int pkgind)
*
* Description: The purpose of this routine is to do session reauthentication.
* If the collect function pointer is NULL this is a non-interactive request.
* This call is used to perform a terminal lock or workstation pause function.
*
* Returns: 
*		SAIDSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
#include <sys/security.h>
#include <sys/audit.h>
#include <sia.h>
#include <siad.h>
#include "sia_mech.h"

#ifdef	MSG
#include "libsec_msg.h"
#define	MSGSTR_SEC(n,s)	GETMSG(MS_SIA,n,s)
#else
#define	MSGSTR_SEC(n,s)	s
#endif

siad_ses_reauthent(collect,entity,siastat,pkgind)
int (*collect)();
SIAENTITY *entity;
int siastat;
int pkgind;
{
	prompt_t prompt[2];
	struct passwd *pwd;
	struct pr_passwd *prpwd;
	extern int (*_c2_collect)();
	extern int _c2_collinput;
	int crypt_alg;

	_c2_collect = collect;
	if(!entity) {
		return SIADFAIL;
	}
	_c2_collinput = entity->colinput;

	entity->authtype = SIA_A_REAUTH;
/*
 * Create link to mechanism specific data (C2 shadow records).
 */
	if(EN_MECH(entity, pkgind)) {
		free(EN_MECH(entity, pkgind));
		entity->mech[pkgind] = (int *) 0;
	}
	c2_make_mech(entity, pkgind, (PR_PASSWD *) 0, (PR_TERM *) 0);
/*
 * Get user or application supplied authentication information.
 */
	if(!entity->acctname)
/*
 * No local account name, try using global name.
 */
		if(entity->name)
			entity->acctname = strdup(entity->name);
	if((!entity->name) && (!entity->password)) {
/*
 * No local name, global name, or password, collect name and password.
 */
		if(collect && entity->colinput) {
			prompt[0].prompt = (unsigned char *) MSGSTR(ENTRY_LOGIN, "login: ");
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
 * No name, collect name only.
 */
		if(collect && entity->colinput) {
			prompt[0].result = (u_char *) malloc(MAX_PWD_LENGTH+1);
			prompt[0].min_result_length = 0;
			prompt[0].max_result_length = MAX_PWD_LENGTH;
			prompt[0].control_flags = SIAONELINER;
			prompt[0].prompt = (unsigned char *) MSGSTR(LOGIN_SEC_52, "Password:");
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
	EN_PR_PWD(entity, pkgind) = prpwd;
	sia_make_entity_pwd(pwd, entity);
	if(!prpwd) {
		sia_entity_audit(entity, pkgind,
			MSGSTR_SEC(LOGIN_SEC_48, "no entry in protected password database"));
		return SIADFAIL;	/* failure_overide? */
	}
	if(prpwd->uflg.fg_pwchanger) {
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

	if(prpwd->ufld.fd_encrypt[0] == '\0') {
		;
	} else  {
		if (prpwd->uflg.fg_oldcrypt)
			crypt_alg = prpwd->ufld.fd_oldcrypt;
		else if (prpwd->sflg.fg_oldcrypt)
			crypt_alg = prpwd->sfld.fd_oldcrypt;
		else
			crypt_alg = AUTH_CRYPT_BIGCRYPT;

		if(entity->password) {
			if(strcmp(dispcrypt(entity->password,
			    prpwd->ufld.fd_encrypt,crypt_alg),
				  prpwd->ufld.fd_encrypt) != 0) {
				show_mesg(MSGSTR_SEC(SU_SEC_1, "Password incorrect\n"));
				sia_entity_audit(entity, pkgind,
				    MSGSTR_SEC(SU_SEC_2, "Password incorrect"));
			return SIADFAIL;
			}
		} else if(strcmp(login_crypt(MSGSTR_SEC(LOGIN_SEC_52, "Password:"),
		    prpwd->ufld.fd_encrypt, crypt_alg),
		    prpwd->ufld.fd_encrypt) != 0) {
			show_mesg(MSGSTR_SEC(SU_SEC_1, "Password incorrect\n"));
			sia_entity_audit(entity, pkgind,
			    MSGSTR_SEC(SU_SEC_2, "Password incorrect"));
			return SIADFAIL;
		}
	}
	return SIADSUCCESS;
}
