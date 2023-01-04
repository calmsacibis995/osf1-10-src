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
static char	*sccsid = "@(#)$RCSfile: getprtcent.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:23:39 $";
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
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 */


/*
 * Based on:

 */

/*LINTLIBRARY*/


/*
 * This file contains a set of routines used to make programs
 * more secure.  Specifically, this particular file contains
 * routines to implement a terminal control scheme.  The
 * routines parallel those of the getpwent(3) routines for
 * the Terminal Control database.
 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#include <prot.h>

static struct pr_term *pr_term = (struct pr_term *) 0;
static long filepos = 0L;
static FILE *fp = (FILE *) 0;

struct pr_term *getprtcent(), *getprtcnam();

/*
 * Read the next entry of the Terminal Control database.  If there is an
 * error or there are no more entries, return 0.
 */
struct pr_term *
getprtcent()
{
	return getprtcnam((char *)0);
}


/*
 * Matches exact terminal name provided (the relative name from /dev) and
 * returns the associated entry from the Terminal Control database.
 */
struct pr_term *
getprtcnam(nam)
	register char *nam;
{
	register struct pr_term *status;

	check_auth_parameters();

	if (!!nam || !fp)
		setprtcent();

	if (agettty (&filepos, fp, nam) == 1) {
		status = pr_term;
		read_tc_fields (&status->ufld, &status->uflg);

		setprdfent();
		if (agetdefault() == 1)
			read_tc_fields (&status->sfld, &status->sflg);
		else
			status = (struct pr_term *) 0;
	}
	else if (nam && strcmp(nam, "*") && strlen(nam) < 6) {
		struct stat tsb;
		char tnm[3+2+3+2+1];	/* /dev/tty??\0 */
		if (0 > stat(strcat(strcpy(tnm,"/dev/"),nam),&tsb) ||
		    !S_ISCHR(tsb.st_mode))
			status = (struct pr_term *) 0;
		else
			status = getprtcnam("*");
		if (status) {
			(void) strncpy(status->ufld.fd_devname, nam,
					sizeof(status->ufld.fd_devname)-1);
			status->uflg.fg_devname = 1;
		}
	}
	else
		status = (struct pr_term *) 0;
	
	return status;
}


/*
 * Reset the position of the Terminal Control database so that the
 * next time getprtcent() is invoked, it will return the first entry
 * in the database.
 */
void
setprtcent()
{
	static time_t modify_time;
	struct stat sb;
	char *filename;
	int ret;

	check_auth_parameters();

	if (fp == (FILE *) 0) {
		open_auth_file((char *) 0, OT_TERM_CNTL, &fp);
		if (fp != (FILE *) 0) {
			fstat (fileno(fp), &sb);
			modify_time = sb.st_mtime;
		}
	} else {
		filename = find_auth_file ((char *) 0, OT_TERM_CNTL);
		ret = stat (filename, &sb);
		if (ret != 0 || sb.st_mtime > modify_time) {
			(void) fclose (fp);
			open_auth_file((char *) 0, OT_TERM_CNTL, &fp);
			if (fp != (FILE *) 0) {
				fstat (fileno(fp), &sb);
				modify_time = sb.st_mtime;
			}
		}
		free (filename);
	}
	filepos = 0L;
	if (pr_term == (struct pr_term *) 0) {
		pr_term = (struct pr_term *)
		  malloc (sizeof (*pr_term));
		if (pr_term == (struct pr_term *) 0) {
			endprtcent();
		}
	}
}


/*
 * Close the file(s) related the to the Terminal Control database.
 */
void
endprtcent()
{
	check_auth_parameters();

	if (fp != (FILE *) 0)  {
		(void) fclose(fp);
		fp = (FILE *) 0;
	}
	filepos = 0L;
	end_authcap (OT_TERM_CNTL);
}


/*
 * Place an entry into the Terminal Control database under the given
 * tty name.  Replace an existing entry if the names compare or add
 * this entry at the end.  (The entry is deleted if the fg_name
 * is 0.)  Lock the entire Authentication database for this operation.
 * When done, the Terminal Control database is closed.
 */
int
putprtcnam(nam, p)
	register char *nam;
	register struct pr_term *p;
{
	register struct pr_term *others;
	register FILE *tempfile;
	register int replaced;
	register int status;
	register int cfs_status;
	register char *pathname;
	char *temppathname;
	char *oldpathname;

	check_auth_parameters();

	status = 0;
	replaced = 0;

	pathname = find_auth_file(nam, OT_TERM_CNTL);

	if (!make_transition_files(pathname, &temppathname, &oldpathname))  {
		free(pathname);
		endprtcent();
		return (0);
	}

	cfs_status = create_file_securely(temppathname, AUTH_VERBOSE,
			     MSGSTR(GETPRTCENT_1, "make new Terminal Control database"));

	if(cfs_status != CFS_GOOD_RETURN) {
		free(pathname);
		free(temppathname);
		free(oldpathname);
		endprtcent();
		return(0);
	}

	tempfile = fopen(temppathname, "w");

	/* now file is locked.  Reference the current database */

	if (fp != (FILE *) 0)
		(void) fclose (fp);
	fp = fopen (pathname, "r");
	filepos = 0L;
	

	if (tempfile == (FILE *) 0)  {
		unlink(temppathname);
		free(temppathname);
		free(oldpathname);
	}
	else  {
		status = 1;
		while (status &&
		   agettty(&filepos, fp, (char *) 0) == 1)  {
			others = pr_term;
			read_tc_fields(&others->ufld, &others->uflg);
			if (!others->uflg.fg_devname)
				status = store_tc_fields(tempfile, "",
						&others->ufld, &others->uflg);
			else if (ackentname(nam, (char *)0))  {
				if (p->uflg.fg_devname)
					status = store_tc_fields(tempfile, nam,
							&p->ufld, &p->uflg);
				replaced = 1;
			}
			else
				status = store_tc_fields(tempfile,
						others->ufld.fd_devname,
						&others->ufld, &others->uflg);
		}

		if (status && !replaced && p->uflg.fg_devname)
			status = store_tc_fields(tempfile, nam, &p->ufld,
						 &p->uflg);

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

	endprtcent();

	return status;
}

/*
 * Read the fields for a Terminal Control entry.  They are read
 * from the authcap entry currently loaded.  This routine must be
 * called twice for a full Terminal Control entry, one for terminal specific
 * fields/flags and one for system default fields/flags.
 */
void
read_tc_fields(fld, flg)
	register struct t_field *fld;
	register struct t_flag *flg;
{
	long numres;
	register int flgres;
	char *strres;

	check_auth_parameters();

	(void) strncpy ((char *) fld, "", sizeof (*fld));
	(void) strncpy ((char *) flg, "", sizeof (*flg));


	strres = fld->fd_devname;
	strres = agetstr(AUTH_T_DEVNAME, &strres);
	if (strres != (char *) 0)
		flg->fg_devname = 1;

	numres = agetuid(AUTH_T_UID);
	if (numres != -1)  {
		flg->fg_uid = 1;
		fld->fd_uid = (uid_t) numres;
	}

	if(agtnum(AUTH_T_LOGTIME,&numres) == 0) {
		flg->fg_slogin = 1;
		fld->fd_slogin = (time_t) numres;
	}

	numres = agetuid(AUTH_T_UNSUCUID);
	if (numres != -1)  {
		flg->fg_uuid = 1;
		fld->fd_uuid = (uid_t) numres;
	}

	if(agtnum(AUTH_T_UNSUCTIME,&numres) == 0) {
		flg->fg_ulogin = 1;
		fld->fd_ulogin = (time_t) numres;
	}

	numres = agetuid(AUTH_T_PREVUID);
	if (numres != -1)  {
		flg->fg_loutuid = 1;
		fld->fd_loutuid = (uid_t) numres;
	}

	if(agtnum(AUTH_T_PREVTIME,&numres) == 0) {
		flg->fg_louttime = 1;
		fld->fd_louttime = (time_t) numres;
	}

	if(agtnum(AUTH_T_FAILURES,&numres) == 0) {
		flg->fg_nlogins = 1;
		fld->fd_nlogins = (ushort) numres;
	}

	if(agtnum(AUTH_T_UNLOCK,&numres) == 0) {
		flg->fg_unlockint = 1;
		fld->fd_unlockint = (time_t) numres;
	}

	if(agtnum(AUTH_T_LOGDELAY,&numres) == 0) {
		flg->fg_logdelay = 1;
		fld->fd_logdelay = (time_t) numres;
	}

	if(agtnum(AUTH_T_MAXTRIES,&numres) == 0) {
		flg->fg_max_tries = 1;
		fld->fd_max_tries = (ushort) numres;
	}

	flgres = agetflag(AUTH_T_XDISPLAY);
	if (flgres != -1)  {
		flg->fg_xdisp = 1;
		fld->fd_xdisp = flgres;
	}

	flgres = agetflag(AUTH_T_LOCK);
	if (flgres != -1)  {
		flg->fg_lock = 1;
		fld->fd_lock = flgres;
	}

	if(agtnum(AUTH_T_LOGIN_TIMEOUT,&numres) == 0) {
		flg-> fg_login_timeout = 1 ;
		fld-> fd_login_timeout = (ushort) numres ;
	}

}


/*
 * Store the terminal specific fields and flags associated with a
 * Terminal Control entry.  This routine outputs to the actual file.
 * It returns 1 if there is no error and 0 if an error occurred in writing.
 */
int
store_tc_fields(f, name, fd, fg)
	register FILE *f;
	register char *name;
	register struct t_field *fd;
	register struct t_flag *fg;
{
	register int fields = 1;
	int error;

	check_auth_parameters();

	fields = aptentnameq(f, &error, fields, name);

	if (!error && fg->fg_devname)  {
		fields = aptentstr(f, &error, fields, AUTH_T_DEVNAME,
					fd->fd_devname);
	}
	if (!error && fg->fg_uid)  {
		fields = aptentuid(f, &error, fields, AUTH_T_UID, fd->fd_uid);
	}
	if (!error && fg->fg_slogin)  {
		fields = aptentunum(f, &error, fields, AUTH_T_LOGTIME,
					(ulong) fd->fd_slogin);
	}
	if (!error && fg->fg_uuid)  {
		fields = aptentuid(f, &error, fields, AUTH_T_UNSUCUID,
					fd->fd_uuid);
	}
	if (!error && fg->fg_ulogin)  {
		fields = aptentunum(f, &error, fields, AUTH_T_UNSUCTIME,
					(ulong) fd->fd_ulogin);
	}
	if (!error && fg->fg_loutuid)  {
		fields = aptentuid(f, &error, fields, AUTH_T_PREVUID,
					fd->fd_loutuid);
	}
	if (!error && fg->fg_louttime)  {
		fields = aptentunum(f, &error, fields, AUTH_T_PREVTIME,
					(ulong) fd->fd_louttime);
	}
	if (!error && fg->fg_nlogins)  {
		fields = aptentuint(f, &error, fields, AUTH_T_FAILURES,
					(uint) fd->fd_nlogins);
	}
	if (!error && fg->fg_logdelay)  {
			fields = aptentunum(f, &error, fields, AUTH_T_LOGDELAY,
						(ulong) fd->fd_logdelay);
	}
	if (!error && fg->fg_max_tries)  {
		fields = aptentuint(f, &error, fields, AUTH_T_MAXTRIES,
					(uint) fd->fd_max_tries);
	}
	if (!error && fg->fg_unlockint)  {
		fields = aptentunum(f, &error, fields, AUTH_T_UNLOCK,
					(ulong) fd->fd_unlockint);
	}
	if (!error && fg->fg_lock)  {
		fields = aptentbool(f, &error, fields, AUTH_T_LOCK,
					fd->fd_lock);
	}

	if (!error && fg->fg_xdisp)  {
		fields = aptentbool(f, &error, fields, AUTH_T_XDISPLAY,
					fd->fd_xdisp);
	}

	if (!error && fg-> fg_login_timeout)  {
		fields = aptentuint(f, &error, fields, AUTH_T_LOGIN_TIMEOUT,
					(uint) fd-> fd_login_timeout);
	}

	if (name)
		return aptentfin(f, &error);

	error = (fflush(f) != 0) || error;

	return !error;
}

/* #endif */ /*} SEC_BASE */
