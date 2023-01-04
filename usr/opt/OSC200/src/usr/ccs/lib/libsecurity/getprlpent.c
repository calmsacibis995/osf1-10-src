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
static char	*sccsid = "@(#)$RCSfile: getprlpent.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:23:20 $";
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
 * routines to implement a printer control scheme.  The
 * routines parallel those of the getpwent(3) routines for
 * the Printer Control database.
 */

#include <sys/secdefines.h>
#include "libsecurity.h"

#if SEC_MAC || SEC_ILB /*{*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

static struct pr_lp *pr_lp = (struct pr_lp *) 0;
static long filepos = 0L;
static FILE *fp = (FILE *) 0;


static void read_lp_fields();
static int store_lp_fields();

/*
 * Read the next entry of the Printer Control database.  If there is an
 * error or there are no more entries, return 0.
 */
struct pr_lp *
getprlpent()
{
	return getprlpnam((char *)0);
}


/*
 * Matches exact printer name provided (the relative name from /dev) and
 * returns the associated entry from the Printer Control database.
 */
struct pr_lp *
getprlpnam(nam)
	register char *nam;
{
	register struct pr_lp *status;

	check_auth_parameters();

	if (!fp || !!nam)
		setprlpent();

	if (agetlp (&filepos, fp, nam) == 1) {
		status = pr_lp;
		read_lp_fields (&status->ufld, &status->uflg);
	}
	else
		status = (struct pr_lp *) 0;
	
	return status;
}


/*
 * Reset the position of the Printer Control database so that the
 * next time getprlpent() is invoked, it will return the first entry
 * in the database.
 */
void
setprlpent()
{
	static time_t modify_time;
	struct stat sb;
	char *filename;
	int ret;

	check_auth_parameters();

	if (fp == (FILE *) 0) {
		open_auth_file((char *) 0, OT_LP_CNTL, &fp);
		if (fp != (FILE *) 0) {
			fstat (fileno(fp), &sb);
			modify_time = sb.st_mtime;
		}
	} else {
		filename = find_auth_file ((char *) 0, OT_LP_CNTL);
		ret = stat (filename, &sb);
		if (ret != 0 || sb.st_mtime > modify_time) {
			(void) fclose (fp);
			open_auth_file((char *) 0, OT_LP_CNTL, &fp);
			if (fp != (FILE *) 0) {
				fstat (fileno(fp), &sb);
				modify_time = sb.st_mtime;
			}
		}
		free (filename);
	}
	filepos = 0L;
	if (pr_lp == (struct pr_lp *) 0) {
		pr_lp = (struct pr_lp *)
		  malloc (sizeof (*pr_lp));
		if (pr_lp == (struct pr_lp *) 0) {
			endprlpent();
		}
	}
}


/*
 * Close the file(s) related the to the Printer Control database.
 */
void
endprlpent()
{
	check_auth_parameters();

	if (fp != (FILE *) 0)  {
		(void) fclose(fp);
		fp = (FILE *) 0;
	}
	filepos = 0L;
	end_authcap (OT_LP_CNTL);
}


/*
 * Place an entry into the Printer Control database under the given
 * tty name.  Replace an existing entry if the names compare or add
 * this entry at the end.  (The entry is deleted if the fg_name
 * is 0.)  Lock the entire Authentication database for this operation.
 * When done, the Printer Control database is closed.
 */
int
putprlpnam(nam, p)
	register char *nam;
	register struct pr_lp *p;
{
	register struct pr_lp *others;
	register FILE *tempfile;
	register int replaced;
	register int status;
	register char *pathname;
	register int cfs_status ;
	char *temppathname;
	char *oldpathname;

	check_auth_parameters();

	status = 0;
	replaced = 0;

	pathname = find_auth_file(nam, OT_LP_CNTL);

	if (!make_transition_files(pathname, &temppathname, &oldpathname))  {
		free(pathname);
		endprlpent();
		return (0);
	}

	cfs_status = create_file_securely(temppathname, AUTH_VERBOSE,
			     MSGSTR(GETPRLPENT_1, "make new Printer Control database"));
	if (cfs_status != CFS_GOOD_RETURN) {
		endprlpent() ;
		free(pathname);
		free(temppathname);
		free(oldpathname);
		return(0) ;
	}

	/* now file is locked.  Reference the current database */

	if (fp != (FILE *) 0)
		(void) fclose (fp);
	fp = fopen (pathname, "r");
	filepos = 0L;
	
	tempfile = fopen(temppathname, "w");

	if (tempfile == (FILE *) 0)  {
		(void) unlink(temppathname);
		free(temppathname);
		free(oldpathname);
	}
	else  {
		status = 1;
		while (status &&
		   agetlp(&filepos, fp, (char *) 0) == 1)  {
			others = pr_lp;
			read_lp_fields(&others->ufld, &others->uflg);
			if (!others->uflg.fg_name)
				status = store_lp_fields(tempfile, "",
						&others->ufld, &others->uflg);
			else if (ackentname(nam, (char *)0)) {
				if (p->uflg.fg_name)
					status = store_lp_fields(tempfile, nam,
							&p->ufld, &p->uflg);
				replaced = 1;
			}
			else
				status = store_lp_fields(tempfile,
						others->ufld.fd_name,
						&others->ufld, &others->uflg);
		}

		if (status && !replaced && p->uflg.fg_name)
			status = store_lp_fields(tempfile, nam, &p->ufld,
						 &p->uflg);

		status = (fclose(tempfile) == 0) && status;


		if (status)
			status = replace_file(temppathname, pathname,
				oldpathname);
		else {
			(void) unlink(temppathname);
			free(temppathname);
			free(oldpathname);
		}

	}

	free(pathname);

	endprlpent();

	return status;
}


/*
 * Read the fields for a Printer Control entry.  They are read
 * from the authcap entry currently loaded.
 */
static void
read_lp_fields(fld, flg)
	register struct l_field *fld;
	register struct l_flag *flg;
{
	long numres;
	register int flgres;
	char *strres;

	check_auth_parameters();

	(void) strncpy ((char *) fld, "", sizeof (*fld));
	(void) strncpy ((char *) flg, "", sizeof (*flg));


	strres = fld->fd_name;
	strres = agetstr(AUTH_L_LPNAME, &strres);
	if (strres != (char *) 0)
		flg->fg_name = 1;

	strres = fld->fd_initseq;
	strres = agetstr(AUTH_L_INITSEQ, &strres);
	if (strres != (char *) 0)
		flg->fg_initseq = 1;

	strres = fld->fd_termseq;
	strres = agetstr(AUTH_L_TERMSEQ, &strres);
	if (strres != (char *) 0)
		flg->fg_termseq = 1;

	strres = fld->fd_emph;
	strres = agetstr(AUTH_L_EMPH, &strres);
	if (strres != (char *) 0)
		flg->fg_emph = 1;

	strres = fld->fd_deemph;
	strres = agetstr(AUTH_L_DEEMPH, &strres);
	if (strres != (char *) 0)
		flg->fg_deemph = 1;

	strres = fld->fd_chrs;
	strres = agetstr(AUTH_L_CHRS, &strres);
	if (strres != (char *) 0)
		flg->fg_chrs = 1;

	if (agtnum(AUTH_L_CHRSLEN, &numres) == 0) {
		flg->fg_chrslen = 1;
		fld->fd_chrslen = (ushort) numres;
	}

	strres = fld->fd_escs;
	strres = agetstr(AUTH_L_ESCS, &strres);
	if (strres != (char *) 0)
		flg->fg_escs = 1;

	if (agtnum(AUTH_L_ESCSLEN, &numres) == 0) {
		flg->fg_escslen = 1;
		fld->fd_escslen = (ushort) numres;
	}

	if (agtnum(AUTH_L_LINELEN, &numres) == 0) {
		flg->fg_linelen = 1;
		fld->fd_linelen = numres;
	}

	if (agtnum(AUTH_L_PAGELEN, &numres) == 0) {
		flg->fg_pagelen = 1;
		fld->fd_pagelen = numres;
	}

	flgres = agetflag(AUTH_L_TRUNCLINE);
	if (flgres != -1)  {
		flg->fg_truncline = 1;
		fld->fd_truncline = flgres;
	}
}


/*
 * Store the user fields and flags associated with a Printer Control
 * entry.  This routine outputs to the actual file.  It returns 1 if there
 * is no error and 0 if an error occurred in writing.
 */
static int
store_lp_fields(f, name, fd, fg)
	register FILE *f;
	register char *name;
	register struct l_field *fd;
	register struct l_flag *fg;
{
	register int fields = 1;
	int error;

	check_auth_parameters();

	fields = aptentnameq(f, &error, fields, name);

	if (!error && fg->fg_name)  {
		fields = aptentstr(f, &error, fields, AUTH_L_LPNAME,
					fd->fd_name);
	}
	if (!error && fg->fg_initseq)  {
		if (fields > 1)
			fields = pr_newline(f, 0, &error);
		fields = aptentstr(f, &error, 0, AUTH_L_INITSEQ,
					fd->fd_initseq);
	}
	if (!error && fg->fg_termseq)  {
		if (fields > 1)
			fields = pr_newline(f, 0, &error);
		fields = aptentstr(f, &error, 0, AUTH_L_TERMSEQ,
					fd->fd_termseq);
	}
	if (!error && fg->fg_chrs)  {
		if (fields > 1)
			fields = pr_newline(f, 0, &error);
		fields = aptentstr(f, &error, 0, AUTH_L_CHRS, fd->fd_chrs);
	}
	if (!error && fg->fg_escs)  {
		if (fields > 1)
			fields = pr_newline(f, 0, &error);
		fields = aptentstr(f, &error, 0, AUTH_L_ESCS, fd->fd_escs);
	}
	if (!error && fg->fg_emph)  {
		fields = aptentstr(f, &error, fields, AUTH_L_EMPH, fd->fd_emph);
	}
	if (!error && fg->fg_deemph)  {
		fields = aptentstr(f, &error, fields, AUTH_L_DEEMPH,
					fd->fd_deemph);
	}
	if (!error && fg->fg_chrslen)  {
		fields = aptentuint(f, &error, fields, AUTH_L_CHRSLEN,
				(uint) fd->fd_chrslen);
	}
	if (!error && fg->fg_escslen)  {
		fields = aptentuint(f, &error, fields, AUTH_L_ESCSLEN,
				(uint) fd->fd_escslen);
	}
	if (!error && fg->fg_linelen)  {
		fields = aptentuint(f, &error, fields, AUTH_L_LINELEN,
				(uint) fd->fd_linelen);
	}
	if (!error && fg->fg_pagelen)  {
		fields = aptentuint(f, &error, fields, AUTH_L_PAGELEN,
				(uint) fd->fd_pagelen);
	}
	if (!error && fg->fg_truncline)  {
		fields = aptentbool(f, &error, fields, AUTH_L_TRUNCLINE,
					fd->fd_truncline);
	}

	if (name)
		return aptentfin(f, &error);

	error = (fflush(f) != 0) || error;

	return !error;
}
#endif /*} SEC_MAC || SEC_ILB */
