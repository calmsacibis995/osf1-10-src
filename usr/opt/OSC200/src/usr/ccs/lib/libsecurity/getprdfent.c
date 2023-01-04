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
static char *rcsid = "@(#)$RCSfile: getprdfent.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 13:24:44 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	getprdfent.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.8.2.2  1992/06/11  14:28:29  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  14:26:13  hosking]
 *
 * Revision 1.8  1991/03/23  17:57:22  devrcs
 * 	<<<replace with log message for ./usr/ccs/lib/libsecurity/getprdfent.c>>>
 * 	[91/03/12  11:30:32  devrcs]
 * 
 * 	Merge fixes up from 1.0.1
 * 	[91/03/11  15:25:20  seiden]
 * 
 * 	Added logic to audit missing fields in loadnamepair() properly. It now identifies the database, the entry, and the field that's bad.
 * 	[91/03/10  09:31:15  seiden]
 * 
 * Revision 1.5  90/10/07  20:06:37  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:13:59  gm]
 * 
 * Revision 1.4  90/07/27  10:31:29  devrcs
 * 	Any function using NLS messages will now open the catalog
 * 	[90/07/16  09:54:20  staffan]
 * 
 * Revision 1.3  90/07/17  12:19:27  devrcs
 * 	Internationalized
 * 	[90/07/05  07:24:36  staffan]
 * 
 * Revision 1.2  90/06/22  21:46:47  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:18:26  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All Rights Reserved.
 */

/* #ident "@(#)getprdfent.c	6.2 09:14:17 2/26/91 SecureWare" */
/*
 * Based on:
 *   "@(#)getprdfent.c	2.14.2.3 17:22:25 12/27/89 SecureWare"
 */

/*LINTLIBRARY*/


/*
 * This file contains a set of routines used to make programs
 * more secure.  Specifically, this particular file contains
 * routines to implement a scheme for default parameters.  The
 * routines parallel those of the getpwent(3) routines for
 * the System Default database.
 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdio.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#include <prot.h>

#define min(a,b) (a)<(b)?(a):(b)

static int store_df_fields();

/* This pointer is kept in authcap.c for packaging: */
extern struct pr_default *pr_default;

/*
 * Read the next entry of the System Default database.  If there is an
 * error or there are no more entries, return 0.
 */
struct pr_default *
getprdfent()
{
	register struct pr_default *status;
	long	number;
	int	flgres;
	char	*strres;
	char	namelist[300];
	char	c = '\0';
#if SEC_MAC
	mand_ir_t	*sensitivity_level;
#endif

	if (agetdefault() != 1)
		return (struct pr_default *) 0;

	status = pr_default;
	(void) strncpy ((char *) status, &c, sizeof (*status));

	strres = status->dd_name;
	strres = agetstr(AUTH_D_NAME, &strres);
	if (strres != (char *) 0)
		status->dg_name = 1;

/*
 *
 *	Get the global system values
 *
 */

	strres = namelist;
	strres = agetstr(AUTH_D_SECCLASS, &strres);
	if (strres != (char *) 0)  {
		status->sflg.fg_secclass = 1;
		loadnamepair(status->sfld.fd_secclass, AUTH_MAX_SECCLASS,
				strres, secclass,
				AUTH_CLASSES, OT_DFLT_CNTL, "default");
	}

	flgres = agetflag(AUTH_D_BOOT_AUTHENTICATE);
	if (flgres != -1)  {
		status->sflg.fg_boot_authenticate = 1;
		status->sfld.fd_boot_authenticate = flgres;
	}

	if (agtnum(AUTH_D_INACTIVITY_TIMEOUT, &number) == 0) {
		status->sflg.fg_inactivity_timeout = 1;
		status->sfld.fd_inactivity_timeout = (time_t) number;
	}

	if (agtnum(AUTH_D_PW_EXPIRE_WARNING, &number) == 0) {
		status->sflg.fg_pw_expire_warning = 1;
		status->sfld.fd_pw_expire_warning = (time_t) number;
	}

	strres = status->sfld.fd_pw_site_callout;
	strres = agetstr(AUTH_D_PW_SITE_CALLOUT, &strres);
	if (strres != (char *) 0)  {
		status->sflg.fg_pw_site_callout = 1;
	}

#if SEC_MAC
	strres = namelist;
	strres = agetstr(AUTH_D_SINGLE_USER_SL, &strres);
	if (strres != (char *) 0)  {
		status-> sfld.fd_single_user_sl = mand_er_to_ir(strres);
		if (status-> sfld.fd_single_user_sl != (mand_ir_t *) 0)  {
			status-> sflg.fg_single_user_sl = 1;
		}
	}
#endif

#ifdef TPATH
	if (agtnum(AUTH_D_LOGIN_SESSION_TIMEOUT, &number) == 0) {
		status-> sflg.fg_session_timeout = 1;
		status-> sfld.fd_session_timeout = (ushort) number;
	}

	if (agtnum(AUTH_D_LOGIN_SESSION_WARNING, &number) == 0) {
		status-> sflg.fg_session_warning = 1;
		status-> sfld.fd_session_warning = (ushort) number;
	}

	if (agtnum(AUTH_D_MULTIPLE_LOGIN, &number) == 0) {
		status-> sflg.fg_multiple_login_rule = 1;
		status-> sfld.fd_multiple_login_rule = (ushort)number;
	}
	strres = status-> sfld.fd_trusted_path_seq;
	strres = agetstr(AUTH_D_TRUSTED_PATH_SEQ, &strres);
	if (strres != (char *) 0)
		status-> sflg.fg_trusted_path_seq = 1;
#endif

/*
 *
 *	Get the values from the other "data bases"
 *
 */

	if (read_pw_fields(&status->prd, &status->prg) == -1)
		return (struct pr_default *) 0;

	read_tc_fields(&status->tcd, &status->tcg);

#if SEC_MAC

	strres = namelist;
	strres = agetstr(AUTH_V_MAX_SL, &strres);
	if (strres != (char *) 0)  {
		status-> devd.fd_max_sl = mand_er_to_ir(strres);
		if (status-> devd.fd_max_sl != (mand_ir_t *) 0)  {
			status-> devg.fg_max_sl = 1;
		}
	}

	strres = namelist;
	strres = agetstr(AUTH_V_MIN_SL, &strres);
	if (strres != (char *) 0)  {
		status-> devd.fd_min_sl = mand_er_to_ir(strres);
		if (status-> devd.fd_min_sl != (mand_ir_t *) 0)  {
			status-> devg.fg_min_sl = 1;
		}
	}

	strres = namelist;
	strres = agetstr(AUTH_V_CUR_SL, &strres);
	if (strres != (char *) 0)  {
		status-> devd.fd_cur_sl = mand_er_to_ir(strres);
		if (status-> devd.fd_cur_sl != (mand_ir_t *) 0)  {
			status-> devg.fg_cur_sl = 1;
		}
	}
#endif

#if SEC_ILB
	strres = namelist;
	strres = agetstr(AUTH_V_CUR_IL, &strres);
	if (strres != (char *) 0)  {
		status-> devd.fd_cur_il = ilb_er_to_ir(strres);
		if (status-> devd.fd_cur_il != (ilb_ir_t *) 0)  {
			status-> devg.fg_cur_il = 1;
		}
	}

#endif

	return status;
}


/*
 * Matches exact default name provided and returns the associated
 * entry from the System Default database.
 */
struct pr_default *
getprdfnam(nam)
	register char *nam;
{
	register struct pr_default *status;

	setprdfent();

	do
		status = getprdfent();
	while ((status != (struct pr_default *) 0) &&
	       (!status->dg_name ||
	        (strncmp(nam, status->dd_name, sizeof(status->dd_name)) != 0)));

	return status;
}


/*
 * Place an entry into the System Default database under the given
 * default name.  Replace an existing entry if the names compare or add
 * this entry at the end.  (The entry is deleted if the fg_devname
 * is 0.)  Lock the entire Authentication database for this operation.
 * When done, the System Default database is closed.
 */
int
putprdfnam(nam, p)
	register char *nam;
	register struct pr_default *p;
{
	register struct pr_default *others;
	register FILE *tempfile;
	register int replaced;
	register int status;
	register char *pathname;
	register int cfs_status;
	char *temppathname;
	char *oldpathname;

	status = 0;
	replaced = 0;

	pathname = find_auth_file(nam, OT_DFLT_CNTL);

	if (!make_transition_files(pathname, &temppathname, &oldpathname))  {
		endprdfent();
		free(pathname);
		return (0);
	}

	cfs_status = create_file_securely(temppathname, AUTH_VERBOSE,
			     MSGSTR(GETPRDFENT_1, "make new System Default database"));
	if (cfs_status != CFS_GOOD_RETURN) {
		endprdfent();
		free(pathname);
		free(temppathname);
		free(oldpathname);
		return(0);
	}

	/* reset the default database, in case it has changed. */
	reset_default(pathname);

	tempfile = fopen(temppathname, "w");
	if (tempfile == (FILE *) 0)  {
		unlink(temppathname);
		free(temppathname);
		free(oldpathname);
	}
	else  {
		status = 1;
		while (status &&
		       ((others = getprdfent()) != (struct pr_default *) 0))  {
			if (!others->dg_name)
				status = store_df_fields(tempfile, "", others);
			else if (strncmp(nam, others->dd_name,
				    sizeof(others->dd_name)) == 0)  {
				if (p->dg_name)
					status =
					    store_df_fields(tempfile, nam, p);
				replaced = 1;
			}
			else
				status = store_df_fields(tempfile,
						others->dd_name, others);
		}

		if (status && !replaced && p->dg_name)
			status = store_df_fields(tempfile, nam, p);

		status = (fclose(tempfile) == 0) && status;


		if (status)
			status = replace_file(temppathname, pathname,
				      oldpathname);
		else {
			(void) unlink (temppathname);
			free (temppathname);
			free (oldpathname);
		}

	}

	free(pathname);

	if (!status)
		audit_auth_entry(nam, OT_DFLT_CNTL, MSGSTR(GETPRDFENT_2, "cannot update entry"));

	endprdfent();

	return status;
}


/*
 * Store the fields for a System Default entry.
 * This routine is only called once for a System Default entry,
 * with the Protected Password, Terminal Control
 * default fields/flags.  It returns 1 if there
 * is no error and 0 if an error occurred in writing.
 */
static int
store_df_fields(f, name, pr)
	register FILE *f;
	register char *name;
	register struct pr_default *pr;
{
	int error;
	register char *namelist;

#if SEC_MAC || SEC_ILB
	char	*level_er;
#endif

	check_auth_parameters();

	(void) aptentname(f, &error, 0, name);

	if (!error && pr->dg_name)
		(void) aptentstr(f, &error, 0, AUTH_D_NAME, pr->dd_name);

/*
 *
 *	Store the fields that are global to the system.
 *
 */

	if (!error && pr->sflg.fg_secclass)
		(void) aptentnmpair(f, &error, 0, AUTH_D_SECCLASS,
				pr->sfld.fd_secclass,
				AUTH_MAX_SECCLASS, secclass, AUTH_CLASSES);

	if (!error && pr->sflg.fg_boot_authenticate)
		(void) aptentbool(f, &error, 0, AUTH_D_BOOT_AUTHENTICATE,
				pr->sfld.fd_boot_authenticate);

	if (!error && pr->sflg.fg_inactivity_timeout)
		(void) aptentunum(f, &error, 0, AUTH_D_INACTIVITY_TIMEOUT,
				(ulong) pr->sfld.fd_inactivity_timeout);

	if (!error && pr->sflg.fg_pw_expire_warning)
		(void) aptentunum(f, &error, 0, AUTH_D_PW_EXPIRE_WARNING,
				(ulong) pr->sfld.fd_pw_expire_warning);

	if (!error && pr->sflg.fg_pw_site_callout)
		(void) aptentnstr(f, &error, 0, AUTH_D_PW_SITE_CALLOUT,
				sizeof(pr->sfld.fd_pw_site_callout),
				pr->sfld.fd_pw_site_callout);

#if SEC_MAC
	if (!error && pr->sflg.fg_single_user_sl) {
		level_er = mand_ir_to_er(pr->sfld.fd_single_user_sl);
		if (level_er)
			(void) aptentstr(f, &error, 0, AUTH_D_SINGLE_USER_SL,
					level_er);
		else
			error = 1;
	}
#endif

#ifdef TPATH
	if (!error && pr-> sflg.fg_session_timeout)
		(void) aptentuint(f, &error, 0, AUTH_D_LOGIN_SESSION_TIMEOUT,
				(uint) pr-> sfld.fd_session_timeout);
	
	if (!error && pr-> sflg.fg_session_warning)
		(void) aptentuint(f, &error, 0, AUTH_D_LOGIN_SESSION_WARNING, 
				(uint) pr-> sfld.fd_session_warning);

	if (!error && pr-> sflg.fg_trusted_path_seq)
		(void) aptentnstr(f, &error, 0, AUTH_D_TRUSTED_PATH_SEQ,
				sizeof(pr-> sfld.fd_trusted_path_seq),
				pr-> sfld.fd_trusted_path_seq);
#endif

/*
 *	Store the fields that belong to the other "data bases"
 */

	if (!error)
		error = !store_pw_fields(f, (char *) 0, &pr->prd, &pr->prg);
	if (!error)
		error = fprintf(f, "\\\n\t:") < 0;

	if (!error)
		error = !store_tc_fields(f, (char *) 0, &pr->tcd, &pr->tcg);
	if (!error)
		error = fprintf(f, "\\\n\t:") < 0;

#if SEC_MAC
	if (!error && pr-> devg.fg_max_sl)  {
		level_er = mand_ir_to_er(pr-> devd.fd_max_sl);
		if (level_er)
			(void) aptentstr(f, &error, 0, AUTH_V_MAX_SL, level_er);
		else
			error = 1;
	}

	if (!error && pr-> devg.fg_min_sl)  {
		level_er = mand_ir_to_er(pr-> devd.fd_min_sl);
		if (level_er)
			(void) aptentstr(f, &error, 0, AUTH_V_MIN_SL, level_er);
		else
			error = 1;
	}

	if (!error && pr-> devg.fg_cur_sl)  {
		level_er = mand_ir_to_er(pr-> devd.fd_cur_sl);
		if (level_er)
			(void) aptentstr(f, &error, 0, AUTH_V_CUR_SL, level_er);
		else
			error = 1;
	}
#endif

#if SEC_ILB
	if (!error && pr-> devg.fg_cur_il)  {
		level_er = ilb_ir_to_er(pr-> devd.fd_cur_il);
		if (level_er)
			(void) aptentstr(f, &error, 0, AUTH_V_CUR_IL, level_er);
		else
			error = 1;
	}
#endif

	return aptentfin(f, &error);
}
/* #endif */ /*} SEC_BASE */
