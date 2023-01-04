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
static char *rcsid = "@(#)$RCSfile: getprpwent.c,v $ $Revision: 4.2.4.5 $ (DEC) $Date: 1993/12/20 21:36:59 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	getprpwent.c,v $
 * Revision 1.1.1.4  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.10.4.2  1992/06/11  14:28:46  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  14:26:27  hosking]
 *
 * Revision 1.10.2.3  1992/03/29  00:26:30  hermi
 * 	Following code review, changes for PasswdEtc
 * 	Integration (i.e. code under #ifdef _OSF_DCE)
 * 	were removed.
 * 	Bug #4367.
 * 	[1992/03/28  22:01:50  hermi]
 * 
 * Revision 1.10.2.2  1992/02/10  15:15:14  coren
 * 	Fixed to not overload "last_uid_found", which caused incorrect behavior
 * 	if a legitimate uid < 0. (bug 4119)
 * 	[1992/02/03  19:28:23  coren]
 * 
 * Revision 1.10  1991/09/24  11:54:21  devrcs
 * 	Added conditional code (under #ifdef _OSF_DCE) so that the secure
 * 	OSF/1.1 source can be compiled to run on the top of the DCE
 * 	authentication mechanism.
 * 	[91/08/23  07:09:08  per]
 * 
 * Revision 1.9  91/03/23  17:57:28  devrcs
 * 	<<<replace with log message for ./usr/ccs/lib/libsecurity/getprpwent.c>>>
 * 	[91/03/12  11:30:42  devrcs]
 * 
 * 	Merge fixes up from 1.0.1
 * 	[91/03/11  15:25:33  seiden]
 * 
 * 	Added logic to audit missing fields in loadnamepair() properly. It now identifies the database, the entry, and the field that's bad.
 * 	[91/03/10  09:31:27  seiden]
 * 
 * Revision 1.6  90/10/07  20:06:51  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:14:21  gm]
 * 
 * Revision 1.5  90/08/09  14:24:22  devrcs
 * 	Changes for widened data types
 * 	[90/08/01  16:47:55  seiden]
 * 
 * Revision 1.4  90/07/27  10:31:42  devrcs
 * 	Any function using NLS messages will now open the catalog
 * 	[90/07/16  09:55:05  staffan]
 * 
 * Revision 1.3  90/07/17  12:19:40  devrcs
 * 	osc.14 SecureWare merge
 * 	[90/07/06  10:00:47  staffan]
 * 
 * 	Internationalized
 * 	[90/07/05  07:25:16  staffan]
 * 
 * Revision 1.2  90/06/22  21:46:56  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:20:53  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
 */

/* #ident "@(#)getprpwent.c	6.2 09:14:24 2/26/91 SecureWare" */
/*
 * Based on:
 *   "@(#)getprpwent.c	2.10.2.1 18:17:53 12/29/89 SecureWare"
 */

/*LINTLIBRARY*/


/*
 * This file contains a set of routines used to make programs
 * more secure.  Specifically, this particular file contains
 * routines to implement a protected password scheme.  The
 * routines parallel those of the getpwent(3) routines for
 * the Protected Password database.
 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#include <prot.h>
#include <pwd.h>

static uid_t last_uid_found;
static int no_last_uid = TRUE;
static struct pr_passwd *pr_passwd = (struct pr_passwd *) 0;


/*
 * This ugly macro is for token-pasting, whether __STDC__ or not.
 */
#ifdef __STDC__
#define	PX(t1,t2)	t1##t2
#else
#define	PX(t1,t2)	t1/**/t2
#endif

/*
 * Read the next entry of the Protected Password database.  If there is an
 * error or there are no more entries, return 0.
 */
struct pr_passwd *
getprpwent()
{
	register int found = 0;
	register struct passwd *p;
	register struct pr_passwd *status = (struct pr_passwd *) 0;

	check_auth_parameters();

	if (no_last_uid)
		setpwent();

	while (!found && ((p = getpwent()) != (struct passwd *) 0))  {
		status = getprpwnam(p->pw_name);
		found = (status != (struct pr_passwd *) 0);
		if (found) {
			last_uid_found = p->pw_uid;
			no_last_uid = FALSE;
		}
	}

	return status;
}


/*
 * Matches exact login name provided and returns the associated
 * entry from the Protected Password database.
 */
struct pr_passwd *
getprpwnam(nam)
	register char *nam;
{
	register struct pr_passwd *status;

	check_auth_parameters();

	status = (struct pr_passwd *) 0;

	setprpwent();

	if (agetuser(nam) == 1)  {
		status = pr_passwd;
		if (read_pw_fields(&status->ufld, &status->uflg) == -1)
			status = (struct pr_passwd *) 0;
		else {
			setprdfent();
			if (agetdefault() != 1 ||
			    read_pw_fields(&status->sfld, &status->sflg) == -1)
				status = (struct pr_passwd *) 0;
			else if (status->uflg.fg_template &&
			    status->ufld.fd_template[0]) {
				struct pr_field pfld;
				struct pr_flag pflg;
				if (agetuser(status->ufld.fd_template) == 1 &&
				    read_pw_fields(&pfld, &pflg) != -1) {
#define	CPY(_fld)	if (pflg.PX(fg_,_fld)) \
	status->sfld.PX(fd_,_fld) = pfld.PX(fd_,_fld), \
	status->sflg.PX(fg_,_fld) = 1;
#define	CPYS(_fld)	if (pflg.PX(fg_,_fld)) \
	(status->sflg.PX(fg_,_fld) = 1) && \
	memmove(status->sfld.PX(fd_,_fld), pfld.PX(fd_,_fld), \
		sizeof(pfld.PX(fd_,_fld)));
#if 0
					CPYS(encrypt)
#endif
					CPYS(owner)
					CPY(nice)
					CPYS(cprivs)
					CPYS(sprivs)
					CPYS(bprivs)
					CPYS(auditdisp)
					CPY(auditcntl)
					CPY(min)
					CPY(minlen)
					CPY(maxlen)
					CPY(expire)
					CPY(lifetime)
#if 0
					CPY(schange)
					CPY(uchange)
					CPY(pwchanger)
					CPYS(pwdict)
#endif
					CPY(pwdepth)
					CPY(oldcrypt)	/* questionable */
					CPY(newcrypt)
					CPY(pick_pwd)
					CPY(gen_pwd)
					CPY(gen_chars)
					CPY(gen_letters)
					CPY(restrict)
					CPY(policy)
					CPY(nullpw)
#ifdef TMAC
					CPY(pw_admin_num)
#endif
#if SEC_MAC
					CPY(clearance)
					if (pflg.fg_clearance)
						memmove(status->sfld.fd_clearance_filler,
							pfld.fd_clearance_filler,
							sizeof(pfld.fd_clearance_filler));
#endif
#if 0
					CPY(slogin)
					CPYS(suctty)
					CPYS(unsuctty)
					CPY(ulogin)
					CPY(nlogins)
#endif
					CPY(max_tries)
					CPY(retired)
					CPY(lock)
					CPY(unlockint)
					CPY(expdate)
					CPYS(tod)
#if SEC_NCAV
					CPY(nat_citizen)
					CPY(nat_caveats)
#endif
				}
				else status = (struct pr_passwd *) 0;
			}
		}
	}
	else
		status = (struct pr_passwd *) 0;

	return status;
}


/*
 * Matches exact login UID provided and returns the associated
 * entry from the Protected Password database.
 */
struct pr_passwd *
getprpwuid(uid)
	register int uid;
{
	register char *name;

	check_auth_parameters();

	name = pw_idtoname (uid);
	if (name == (char *) 0)
		return ((struct pr_passwd *) 0);
	else
		return (getprpwnam (name));
}


/*
 * Reset the position of the Protected Password database so that the
 * next time getprpwent() is invoked, it will return the first entry
 * in the database.  Since the database works off of the uid value,
 * this involves resetting the uid value associated with the previous
 * entry.
 */
void
setprpwent()
{
	check_auth_parameters();

	no_last_uid = TRUE;

	if (pr_passwd == (struct pr_passwd *) 0) {
		pr_passwd = (struct pr_passwd *)
		  malloc (sizeof (*pr_passwd));
		if (pr_passwd == (struct pr_passwd *) 0) {
			endprpwent();
		}
	}
}

/*
 * "Close" the Protected Password database.  Since the database works off
 * of the uid value, this involves resetting the uid value associated with
 * the previous entry.  Also note that there is no need to close a file since
 * in the Protected Password database, there is a file for each entry that
 * is closed immediately after the entry is retrieved.
 */
void
endprpwent()
{
	check_auth_parameters();

	no_last_uid = TRUE;
	end_authcap (OT_PRPWD);
}


/*
 * Place an entry into the Protected Password database under the given
 * name.  Lock the entire Authentication database for this operation.
 * When done, the Protected Password database entry is closed.
 */
int
putprpwnam(nam, p)
	register char *nam;
	register struct pr_passwd *p;
{
	register FILE *file;
	register char *pathname;
	register int status;
	register int cfs_status;
	char *temppathname;
	char *oldpathname;

	check_auth_parameters();

	status = 0;

	pathname = find_auth_file(nam, OT_PRPWD);

	if (!make_transition_files(pathname, &temppathname, &oldpathname))  {
		free(pathname);
		endprpwent();
		return (0);
	}

	cfs_status = create_file_securely(temppathname, AUTH_VERBOSE,
			     MSGSTR(GETPRPWENT_1, "make new Protected Password entry"));
	if (cfs_status != CFS_GOOD_RETURN) {
		free(pathname);
		free(temppathname);
		free(oldpathname);
		endprpwent();
		return(0);
	}

	file = fopen(temppathname, "w");
	if (file == (FILE *) 0)  {
		unlink(temppathname);
		free(temppathname);
		free(oldpathname);
	}
	else  {
		if (p->uflg.fg_name)  {
			status = store_pw_fields(file, nam, &p->ufld, &p->uflg);
			status = (fclose(file) == 0) && status;
			if (status)
				status = replace_file(temppathname, pathname,
					      oldpathname);
			else {
				(void) unlink (temppathname);
				free (temppathname);
				free (oldpathname);
			}
		}
		else  {
			status = (fclose(file) == 0);
			(void) unlink(temppathname);
			status = (unlink(pathname) == 0) && status;
			free(temppathname);
			free(oldpathname);
		}
	}

	free(pathname);

	endprpwent();

	return status;
}

/*
 * Read the fields for a Protected Password entry.  They are read
 * from the authcap entry currently loaded.  This routine must be
 * called twice for a full Protected Password entry, one for user
 * fields/flags and one for system default fields/flags.
 * (Three times if a template is specified.)
 * Returns 0 on success, -1 on failure.
 */
int
read_pw_fields(fld, flg)
	register struct pr_field *fld;
	register struct pr_flag *flg;
{
	long numres;
	int flgres;
	char *strres;
	char namelist[1000];

	check_auth_parameters();

	(void) strncpy ((char *) fld, "", sizeof (*fld));
	(void) strncpy ((char *) flg, "", sizeof (*flg));

	/* set up for command authorizations.
	 * The user must call this routine or authorized_user()
	 * before using the ST_MAX_CPRIV constant.
	 */
	if (build_cmd_priv() == -1)
		return(-1);

	strres = fld->fd_name;
	strres = agetstr(AUTH_U_NAME, &strres);
	if (strres != (char *) 0)
		flg->fg_name = 1;

	if(agtnum(AUTH_U_ID,&numres) == 0) {
		flg->fg_uid = 1;
		fld->fd_uid = numres;
	}

	strres = fld->fd_encrypt;
	strres = agetstr(AUTH_U_PWD, &strres);
	if (strres != (char *) 0)
		flg->fg_encrypt = 1;

	strres = fld->fd_owner;
	strres = agetstr(AUTH_U_OWNER, &strres);
	if (strres != (char *) 0)
		flg->fg_owner = 1;

	if(agtnum(AUTH_U_PRIORITY,&numres) == 0) {
		flg->fg_nice = 1;
		fld->fd_nice = numres;
	}

	strres = namelist;
	strres = agetstr(AUTH_U_CMDPRIV, &strres);
	if (strres != (char *) 0)  {
		flg->fg_cprivs = 1;
		loadnamepair(fld->fd_cprivs, ST_MAX_CPRIV, strres,
			     cmd_priv, AUTH_CMDPRIV, OT_PRPWD, fld->fd_name);
	}

	strres = namelist;
	strres = agetstr(AUTH_U_SYSPRIV, &strres);
	if (strres != (char *) 0)  {
		flg->fg_sprivs = 1;
		loadnamepair(fld->fd_sprivs, SEC_MAX_SPRIV, strres,
			     sys_priv, AUTH_SYSPRIV, OT_PRPWD, fld->fd_name);
	}


	strres = namelist;
	strres = agetstr(AUTH_U_BASEPRIV, &strres);
	if (strres != (char *) 0)  {
		flg->fg_bprivs = 1;
		loadnamepair(fld->fd_bprivs, SEC_MAX_SPRIV, strres,
			     sys_priv, AUTH_BASEPRIV, OT_PRPWD, fld->fd_name);
	}

	strres = fld->fd_auditdisp;
	strres = agetstr(AUTH_U_AUDITDISP, &strres);
	if (strres != (char *) 0)  {
		flg->fg_auditdisp = 1;
	}

	if (agtnum(AUTH_U_AUDITCNTL, &numres) == 0) {
		flg->fg_auditcntl = 1;
		fld->fd_auditcntl = (uchar_t) numres;
	}

	if(agtnum(AUTH_U_MINCHG,&numres) == 0) {
		flg->fg_min = 1;
		fld->fd_min = (time_t) numres;
	}

	if(agtnum(AUTH_U_MINLEN,&numres) == 0) {
		flg->fg_minlen = 1;
		fld->fd_minlen = numres;
	}

	if(agtnum(AUTH_U_MAXLEN,&numres) == 0) {
		flg->fg_maxlen = 1;
		fld->fd_maxlen = numres;
	}

	if(agtnum(AUTH_U_EXP,&numres) == 0) {
		flg->fg_expire = 1;
		fld->fd_expire = (time_t) numres;
	}

	if(agtnum(AUTH_U_LIFE,&numres) == 0) {
		flg->fg_lifetime = 1;
		fld->fd_lifetime = (time_t) numres;
	}

	if(agttime(AUTH_U_SUCCHG,&numres) == 0) {
		flg->fg_schange = 1;
		fld->fd_schange = (time_t) numres;
	}

	if(agtnum(AUTH_U_UNSUCCHG,&numres) == 0) {
		flg->fg_uchange = 1;
		fld->fd_uchange = (time_t) numres;
	}

	if (agtnum(AUTH_U_NEWCRYPT,&numres) == 0) {
		if (numres >=0 && numres <= AUTH_CRYPT__MAX) {
			flg->fg_newcrypt = 1;
			fld->fd_newcrypt = numres;
		}
	}

	if (agtnum(AUTH_U_OLDCRYPT,&numres) == 0) {
		if (numres >=0 && numres <= AUTH_CRYPT__MAX) {
			flg->fg_oldcrypt = 1;
			fld->fd_oldcrypt = numres;
		}
	}

	flgres = agetflag(AUTH_U_PICKPWD);
	if (flgres != -1)  {
		flg->fg_pick_pwd = 1;
		fld->fd_pick_pwd = flgres;
	}

	flgres = agetflag(AUTH_U_GENPWD);
	if (flgres != -1)  {
		flg->fg_gen_pwd = 1;
		fld->fd_gen_pwd = flgres;
	}

	flgres = agetflag(AUTH_U_RESTRICT);
	if (flgres != -1)  {
		flg->fg_restrict = 1;
		fld->fd_restrict = flgres;
	}

	flgres = agetflag(AUTH_U_POLICY);
	if (flgres != -1)  {
		flg->fg_policy = 1;
		fld->fd_policy = flgres;
	}

	flgres = agetflag(AUTH_U_NULLPW);
	if (flgres != -1) {
		flg->fg_nullpw = 1;
		fld->fd_nullpw = flgres;
	}

	numres = agetuid(AUTH_U_PWCHANGER);
	if (numres != -1) {
		flg->fg_pwchanger = 1;
		fld->fd_pwchanger = (uid_t) numres;
	}


	flgres = agetflag(AUTH_U_GENCHARS);
	if (flgres != -1)  {
		flg->fg_gen_chars = 1;
		fld->fd_gen_chars = flgres;
	}

	flgres = agetflag(AUTH_U_GENLETTERS);
	if (flgres != -1)  {
		flg->fg_gen_letters = 1;
		fld->fd_gen_letters = flgres;
	}

	strres = fld->fd_tod;
	strres = agetstr(AUTH_U_TOD, &strres);
	if (strres != (char *) 0)
		flg->fg_tod = 1;



#if SEC_MAC
	strres = namelist;
	strres = agetstr(AUTH_U_CLEARANCE, &strres);
	if (strres != (char *) 0)  {
		mand_ir_t *clearance;

		clearance = clearance_er_to_ir(strres);
		if (clearance != (mand_ir_t *) 0)  {
			mand_copy_ir(clearance, &fld->fd_clearance);
			flg->fg_clearance = 1;
			mand_free_ir(clearance);
		}
		else {
			audgenl(??);
			return -1;
		}
	}
#endif



	if(agttime(AUTH_U_SUCLOG,&numres) == 0) {
		flg->fg_slogin = 1;
		fld->fd_slogin = (time_t) numres;
	}

	strres = namelist;
	strres = agetstr(AUTH_U_SUCTTY, &strres);
	if (strres != (char *) 0) {
		flg->fg_suctty = 1;
		strncpy (fld->fd_suctty, strres, sizeof(fld->fd_suctty));
	}

	if(agtnum(AUTH_U_UNSUCLOG,&numres) == 0) {
		flg->fg_ulogin = 1;
		fld->fd_ulogin = (time_t) numres;
	}

	strres = namelist;
	strres = agetstr(AUTH_U_UNSUCTTY, &strres);
	if (strres != (char *) 0) {
		flg->fg_unsuctty = 1;
		strncpy (fld->fd_unsuctty, strres, sizeof(fld->fd_unsuctty));
	}

	if(agtnum(AUTH_U_NUMUNSUCLOG,&numres) == 0) {
		flg->fg_nlogins = 1;
		fld->fd_nlogins = (short) numres;
	}

	if(agtnum(AUTH_U_MAXTRIES,&numres) == 0) {
		flg->fg_max_tries = 1;
		fld->fd_max_tries = (short) numres;
	}

	if (agtnum(AUTH_U_UNLOCK, &numres) == 0) {
		flg->fg_unlockint = 1;
		fld->fd_unlockint = (time_t) numres;
	}

	flgres = agetflag(AUTH_U_RETIRED);
	if (flgres != -1)  {
		flg->fg_retired = 1;
		fld->fd_retired = flgres;
	}

	if (agttime(AUTH_U_EXPDATE, &numres) == 0) {
		flg->fg_expdate = 1;
		fld->fd_expdate = (time_t) numres;
	}

	if (agtnum(AUTH_U_PWDEPTH, &numres) == 0) {
		flg->fg_pwdepth = 1;
		fld->fd_pwdepth = (char) numres;
	}

	strres = fld->fd_pwdict;
	strres = agetstr(AUTH_U_PWDICT, &strres);
	if (strres != (char *) 0) {
		flg->fg_pwdict = 1;
	}

	flgres = agetflag(AUTH_U_ISTEMPLATE);
	if (flgres != -1) {
		flg->fg_istemplate = 1;
		fld->fd_istemplate = flgres;
	}

	strres = fld->fd_template;
	strres = agetstr(AUTH_U_TEMPLATE, &strres);
	if (strres != (char *) 0) {
		flg->fg_template = 1;
	}

	flgres = agetflag(AUTH_U_LOCK);
	if (flgres != -1)  {
		flg->fg_lock = 1;
		fld->fd_lock = flgres;
	}

	return(0);
}


/*
 * Store the user fields and flags associated with a Protected Password
 * entry.  This routine outputs to the actual file.  It returns 1 if there
 * is no error and 0 if an error occurred in writing.
 */
int
store_pw_fields(f, name, fd, fg)
	register FILE *f;
	register char *name;
	register struct pr_field *fd;
	register struct pr_flag *fg;
{
	register int fields = 1;
	int error;

	check_auth_parameters();

	/* In all cases, set up for command authorizations.
	 * The user will call this routine or
	 * authorized_user before using the ST_MAX_CPRIV variable.
	 */
	build_cmd_priv();

	fields = aptentname(f, &error, fields, name);

	if (!error && fg->fg_name)  {
		fields = aptentnstr(f, &error, fields, AUTH_U_NAME,
					sizeof(fd->fd_name), fd->fd_name);
	}
	if (!error && fg->fg_uid)  {
		fields = aptentuint(f, &error, fields, AUTH_U_ID,
					(uint) fd->fd_uid);
	}
	if (!error && fg->fg_oldcrypt)  {
		fields = aptentuint(f, &error, fields, AUTH_U_OLDCRYPT,
					(uint) fd->fd_oldcrypt);
	}
	if (!error && fg->fg_newcrypt)  {
		fields = aptentuint(f, &error, fields, AUTH_U_NEWCRYPT,
					(uint) fd->fd_newcrypt);
	}
	if (!error && fg->fg_encrypt)  {
		if (fields > 1)
			fields = pr_newline(f, 0, &error);
		fields = aptentnstr(f, &error, 0, AUTH_U_PWD,
					sizeof(fd->fd_encrypt), fd->fd_encrypt);
	}
	if (!error && fg->fg_owner)  {
		fields = aptentnstr(f, &error, fields, AUTH_U_OWNER,
					sizeof(fd->fd_owner), fd->fd_owner);
	}
	if (!error && fg->fg_nice)  {
		fields = aptentsint(f, &error, fields, AUTH_U_PRIORITY,
					(int) fd->fd_nice);
	}
	if (!error && fg->fg_cprivs)  {
		fields = aptentnmpair(f, &error, fields, AUTH_U_CMDPRIV,
					fd->fd_cprivs, ST_MAX_CPRIV, cmd_priv,
					AUTH_CMDPRIV);
	}
	if (!error && fg->fg_sprivs)  {
		if (fields > 1)
			fields = pr_newline(f, 0, &error);
		fields = aptentnmpair(f, &error, 0, AUTH_U_SYSPRIV,
					fd->fd_sprivs, SEC_MAX_SPRIV, sys_priv,
					AUTH_SYSPRIV);
	}


	if (!error && fg->fg_bprivs)  {
		if (fields > 1)
			fields = pr_newline(f, 0, &error);
		fields = aptentnmpair(f, &error, 0, AUTH_U_BASEPRIV,
					fd->fd_bprivs, SEC_MAX_SPRIV, sys_priv,
					AUTH_BASEPRIV);
	}

	if (!error && fg->fg_auditcntl)  {
		if (fields > 1 && fg->fg_auditdisp)
			fields = pr_newline(f, 0, &error);
		fields = aptentuint(f, &error, fields, AUTH_U_AUDITCNTL,
					(uint) fd->fd_auditcntl);
	}

	if (!error && fg->fg_auditdisp) {
		if (fields > 1 && !fg->fg_auditcntl)
			fields = pr_newline(f, 0, &error);
		fields = aptentnstr(f, &error, 0, AUTH_U_AUDITDISP,
					sizeof(fd->fd_auditdisp),
					fd->fd_auditdisp);
	}

	if (!error && fg->fg_min)  {
		fields = aptentunum(f, &error, fields, AUTH_U_MINCHG,
					(ulong) fd->fd_min);
	}
	if (!error && fg->fg_minlen)  {
		fields = aptentuint(f, &error, fields, AUTH_U_MINLEN,
					(uint) fd->fd_minlen);
	}
	if (!error && fg->fg_maxlen)  {
		fields = aptentuint(f, &error, fields, AUTH_U_MAXLEN,
					(uint) fd->fd_maxlen);
	}
	if (!error && fg->fg_expire)  {
		fields = aptentunum(f, &error, fields, AUTH_U_EXP,
					(ulong) fd->fd_expire);
	}
	if (!error && fg->fg_lifetime)  {
		fields = aptentunum(f, &error, fields, AUTH_U_LIFE,
					(ulong) fd->fd_lifetime);
	}
	if (!error && fg->fg_schange)  {
		fields = aptenttime(f, &error, fields, AUTH_U_SUCCHG,
					fd->fd_schange);
	}
	if (!error && fg->fg_uchange)  {
		fields = aptenttime(f, &error, fields, AUTH_U_UNSUCCHG,
					fd->fd_uchange);
	}
	if (!error && fg->fg_pick_pwd)  {
		fields = aptentbool(f, &error, fields, AUTH_U_PICKPWD,
					fd->fd_pick_pwd);
	}
	if (!error && fg->fg_gen_pwd)  {
		fields = aptentbool(f, &error, fields, AUTH_U_GENPWD,
					fd->fd_gen_pwd);
	}
	if (!error && fg->fg_restrict)  {
		fields = aptentbool(f, &error, fields, AUTH_U_RESTRICT,
					fd->fd_restrict);
	}
	if (!error && fg->fg_policy)  {
		fields = aptentbool(f, &error, fields, AUTH_U_POLICY,
					fd->fd_policy);
	}
	if (!error && fg->fg_pwdepth)  {
		if (fg->fg_pwdict && fields > 1)
			fields = pr_newline(f, 0, &error);
		fields = aptentuint(f, &error, fields, AUTH_U_PWDEPTH,
					(uint) fd->fd_pwdepth);
	}
	if (!error && fg->fg_pwdict)  {
		if (!fg->fg_pwdepth && fields > 1)
			fields = pr_newline(f, 0, &error);
		fields = aptentnstr(f, &error, 0, AUTH_U_PWDICT,
					sizeof(fd->fd_pwdict),
					fd->fd_pwdict);
	}
	if (!error && fg->fg_nullpw) {
		fields = aptentbool(f, &error, fields, AUTH_U_NULLPW,
					fd->fd_nullpw);
	}

	if (!error && fg->fg_pwchanger) {
		if (fg->fg_uid && fg->fg_name && fd->fd_pwchanger == fd->fd_uid)
			fields = aptentstr(f, &error, fields, AUTH_U_PWCHANGER,
						fd->fd_name);
		else
			fields = aptentuid(f, &error, fields, AUTH_U_PWCHANGER,
						fd->fd_pwchanger);
	}


	if (!error && fg->fg_gen_chars)  {
		fields = aptentbool(f, &error, fields, AUTH_U_GENCHARS,
					fd->fd_gen_chars);
	}
	if (!error && fg->fg_gen_letters)  {
		fields = aptentbool(f, &error, fields, AUTH_U_GENLETTERS,
					fd->fd_gen_letters);
	}
	if (!error && fg->fg_tod)  {
		fields = aptentstr(f, &error, fields, AUTH_U_TOD, fd->fd_tod);
	}

#if SEC_MAC
	if (!error && fg->fg_clearance)  {
		char *clearance_er;

		if (fields > 1)
			fields = pr_newline(f, 0, &error);
		clearance_er = clearance_ir_to_er(&fd->fd_clearance);
		if (clearance_er)
			fields = aptentstr(f, &error, 0, AUTH_U_CLEARANCE,
						clearance_er);
		else
			error = 1;
	}
#endif

	if (!error && fg->fg_slogin)  {
		fields = aptenttime(f, &error, fields, AUTH_U_SUCLOG,
					fd->fd_slogin);
	}
	if (!error && fg->fg_suctty) {
		fields = aptentstr(f, &error, fields, AUTH_U_SUCTTY,
					fd->fd_suctty);
	}
	if (!error && fg->fg_ulogin)  {
		fields = aptenttime(f, &error, fields, AUTH_U_UNSUCLOG,
					fd->fd_ulogin);
	}
	if (!error && fg->fg_unsuctty) {
		fields = aptentstr(f, &error, fields, AUTH_U_UNSUCTTY,
					fd->fd_unsuctty);
	}
	if (!error && fg->fg_nlogins)  {
		fields = aptentuint(f, &error, fields, AUTH_U_NUMUNSUCLOG,
					(uint) fd->fd_nlogins);
	}
	if (!error && fg->fg_max_tries)  {
		fields = aptentuint(f, &error, fields, AUTH_U_MAXTRIES,
					(uint) fd->fd_max_tries);
	}
	if (!error && fg->fg_unlockint)  {
		fields = aptentunum(f, &error, fields, AUTH_U_UNLOCK,
					(ulong) fd->fd_unlockint);
	}
	if (!error && fg->fg_expdate)  {
		fields = aptenttime(f, &error, fields, AUTH_U_EXPDATE,
					fd->fd_expdate);
	}
	if (!error && fg->fg_retired)  {
		fields = aptentbool(f, &error, fields, AUTH_U_RETIRED,
					fd->fd_retired);
	}
	if (!error && fg->fg_lock)  {
		fields = aptentbool(f, &error, fields, AUTH_U_LOCK,
					fd->fd_lock);
	}
	if (!error && fg->fg_istemplate)  {
		fields = aptentbool(f, &error, fields, AUTH_U_ISTEMPLATE,
					fd->fd_istemplate);
	}
	if (!error && fg->fg_template)  {
		fields = aptentnstr(f, &error, fields, AUTH_U_TEMPLATE,
					sizeof(fd->fd_template),
					fd->fd_template);
	}

	if (name)
		return aptentfin(f, &error);

	error = (fflush(f) != 0) || error;

	return !error;
}

/* #endif */ /*} SEC_BASE */
