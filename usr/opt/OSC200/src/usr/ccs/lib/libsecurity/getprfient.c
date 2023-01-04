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
static char	*sccsid = "@(#)$RCSfile: getprfient.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/04/01 20:23:15 $";
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
 * routines to implement a file integrity scheme.  The
 * routines parallel those of the getpwent(3) routines for
 * the File Control database.
 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#include <prot.h>

#if SEC_MAC
#include <mandatory.h>
static mand_ir_t	*mand_ir = (mand_ir_t *) 0;
#endif

#if SEC_ACL
#include <acl.h>
#endif

static struct pr_file *pr_file = (struct pr_file *) 0;
static char *old_name = (char *) 0;
static long filepos = 0L;
static FILE *fp = (FILE *) 0;

static int read_fi_fields();
static int store_fi_fields();

/*
 * Read the next entry of the File Control database.  If there is an
 * error or there are no more entries, return 0.
 */
struct pr_file *
getprfient()
{
	return getprfinam((char *)0);
}


/*
 * Matches exact command name provided or a wild-card and returns the associated
 * entry from the Command Control database.
 */
struct pr_file *
getprfinam(nam)
	char *nam;
{
	char *bp;

	if (!fp || !!nam)
		setprfient();

	bp = agetfile(&filepos, fp, nam);
	if (bp && read_fi_fields(&pr_file->ufld, &pr_file->uflg, bp) == 0)
		return pr_file;

	return (struct pr_file *) 0;
}

/*
 * Reset the position of the File Control database so that the
 * next time getprfient() is invoked, it will return the first entry
 * in the database.
 */
void
setprfient()
{
	static time_t modify_time;
	struct stat sb;
	char *filename;
	int ret;

	if (old_name)
		free(old_name);

	if (fp == (FILE *) 0) {
		open_auth_file((char *) 0, OT_FILE_CNTL, &fp);
		if (fp != (FILE *) 0) {
			fstat (fileno(fp), &sb);
			modify_time = sb.st_mtime;
		} else {
	       char *msg = MSGSTR(GETPRFIENT_2,"cannot open File Control database\n");
	       audgenl(AUTH_EVENT, T_CHARP, msg, NULL);
               fprintf(stderr, "%s", msg);
               exit(0);
               }
	} else {
		filename = find_auth_file ((char *) 0, OT_FILE_CNTL);
		ret = stat (filename, &sb);
		if (ret != 0 || sb.st_mtime > modify_time) {
			(void) fclose (fp);
			open_auth_file((char *) 0, OT_FILE_CNTL, &fp);
			if (fp != (FILE *) 0) {
				fstat (fileno(fp), &sb);
				modify_time = sb.st_mtime;
			}
		}
		free (filename);
	}
	filepos = 0L;
	if (pr_file == (struct pr_file *) 0) {
		pr_file = (struct pr_file *) malloc (sizeof (*pr_file));
		if (pr_file == (struct pr_file *) 0) {
			endprfient();
		}
	}

#if SEC_MAC
	mand_init();

	if (mand_ir == (mand_ir_t *) 0) {
		mand_ir = mand_alloc_ir();
		if (mand_ir == (mand_ir_t *) 0) {
			endprfient();
		}
	}
#endif

}

/*
 * Close the file(s) related the to the File Control database.
 */
void
endprfient()
{
	if (fp != (FILE *) 0)  {
		(void) fclose(fp);
		fp = (FILE *) 0;
	}
	filepos = 0L;
#if SEC_MAC
	if (mand_ir != (mand_ir_t *) 0)
		mand_free_ir(mand_ir);
#endif
	if (old_name)
		free(old_name);
	end_authcap (OT_FILE_CNTL);
}


/*
 * Place an entry into the File Control database under the given
 * file name.  Replace an existing entry if the names compare or add
 * this entry at the end.  (The entry is deleted if the fg_devname
 * is 0.)  Lock the entire Authentication database for this operation.
 * When done, the File Control database is closed.
 */
int
putprfinam(nam, p)
	register char *nam;
	register struct pr_file *p;
{
	register FILE *tempfile;
	register int replaced;
	register int status;
	register int cfs_status;
	register char *pathname;
	char *temppathname;
	char *oldpathname;
	char *bp;
	char filebuf[BUFSIZ];

	status = 0;
	replaced = 0;

	setprfient();

	pathname = find_auth_file(nam, OT_FILE_CNTL);

	if (!make_transition_files(pathname, &temppathname, &oldpathname))  {
		endprfient();
		free(pathname);
		return (0);
	}

	cfs_status = create_file_securely(temppathname, AUTH_VERBOSE,
			     MSGSTR(GETPRFIENT_1, "make new File Control database"));
	if (cfs_status != CFS_GOOD_RETURN) {
	       char *msg = MSGSTR(GETPRFIENT_2,"cannot open File Control database\n");
	       audgenl(AUTH_EVENT, T_CHARP, msg, NULL);
               fprintf(stderr, "%s", msg);
		endprfient();
		free(temppathname);
		free(oldpathname);
		free(pathname);
		return(0);
	}

	/* now file is locked.  Reference the current database */

	tempfile = fopen(temppathname, "w");
	if (tempfile == (FILE *) 0)  {
		unlink(temppathname);
		free(temppathname);
		free(oldpathname);
	}
	else  {
		status = 1;
		rewind(fp);
		while (status && fgets(filebuf, sizeof filebuf, fp)) {
			if (ackentfile(nam, filebuf)) {
				(void) agetfile(&filepos, fp, (char *) 0);
				status = store_fi_fields(tempfile,
						nam, &p->ufld, &p->uflg);
				replaced = 1;
			} else {
				do status = fputs(filebuf, tempfile) >= 0;
				while (status &&
					((filebuf[strlen(filebuf)-1] != '\n') ||
					(filebuf[strlen(filebuf)-2] == '\\')) &&
					fgets(filebuf, sizeof filebuf, fp));
				filepos = ftell(fp);
			}
		}

		if (status && !replaced)
			status = store_fi_fields(tempfile, nam, &p->ufld,
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

	endprfient();

	return status;
}


static char *file_table[] = {
	AUTH_F_OWNER,
	AUTH_F_GROUP,
	AUTH_F_MODE,
	AUTH_F_TYPE
#if SEC_MAC
	, AUTH_F_SLEVEL
#endif
#if SEC_ACL
	, AUTH_F_ACL
#endif
#if SEC_PRIV
	, AUTH_F_PPRIVS,
	AUTH_F_GPRIVS
#endif
};

enum {
	OwnerOffset = 0,
	GroupOffset,
	ModeOffset,
	TypeOffset
#if SEC_MAC
      , SLevelOffset
#endif
#if SEC_ACL
      , ACLOffset
#endif
#if SEC_PRIV
      , PPrivOffset,
	GPrivOffset
#endif
};

#define	F_OWNER		(int) OwnerOffset
#define	F_GROUP		(int) GroupOffset
#define	F_MODE		(int) ModeOffset
#define	F_TYPE		(int) TypeOffset
#if SEC_MAC
#define	F_SLEVEL	(int) SLevelOffset
#endif
#if SEC_ACL
#define	F_ACL		(int) ACLOffset
#endif
#if SEC_PRIV
#define	F_PPRIVS	(int) PPrivOffset
#define	F_GPRIVS	(int) GPrivOffset
#endif

#define	FILE_TABLE_SIZE	(sizeof file_table / sizeof file_table[0])


/*
 * Read the fields for a File Control entry.  They are read
 * from the authcap entry currently loaded.  This routine must be
 * called twice for a full Protected Password entry, one for specific
 * file fields/flags and one for system default fields/flags.
 */
static int
read_fi_fields(fld, flg, cp)
	register struct f_field *fld;
	register struct f_flag *flg;
	register char *cp;
{
	register int	i, label_len;
	long val;
	char	*end_field, *end_entry, savec;

	(void) strncpy ((char *) fld, "", sizeof (*fld));
	(void) strncpy ((char *) flg, "", sizeof (*flg));

	/* Get the name field */
	if (*cp == ':') {
		fld->fd_name = (char *) 0;
		flg->fg_name = 0;
	} else {
		if (old_name)
			free(old_name);
		old_name = adecodestr(cp);
		if (!old_name)
			return 1;
		fld->fd_name = old_name;
		flg->fg_name = 1;
	}

	/* Get the remaining fields */
	i = agetuid(AUTH_F_OWNER);
	if (i != -1) {
		fld->fd_uid = i;
		flg->fg_uid = 1;
	}
	i = agetgid(AUTH_F_GROUP);
	if (i != -1) {
		fld->fd_gid = i;
		flg->fg_gid = 1;
	}

	if (agtnum(AUTH_F_MODE, &val) != -1) {
		fld->fd_mode = val;
		flg->fg_mode = 1;
	}

	cp = agetstr(AUTH_F_TYPE, (char **)0);
	if (cp) {
		fld->fd_type[0] = *cp;
		flg->fg_type = 1;
		free(cp);
	}
#if SEC_MAC
	cp = agetstr(AUTH_F_SLEVEL, (char **)0);
	if (cp) {
		flg->fg_slevel = 1;
		if (strcmp(cp, AUTH_F_WILD) == 0)
			fld->fd_slevel = (mand_ir_t *) 0;
		else if (cp, AUTH_F_SYSLO) == 0) {
			memcpy(mand_ir, mand_syslo, mand_bytes());
			fld->fd_slevel = mand_ir;
		}
		else if (strcmp(cp, AUTH_F_SYSHI) == 0) {
			memcpy(mand_ir, mand_syshi, mand_bytes());
			fld->fd_slevel = mand_ir;
		}
		else {
			flg->fg_slevel = 0;
			audgenl(AUTH_EVENT,
				T_CHARP,
				MSGSTR(GETPRFIENT_4,
				"unrecognized clearance in File Control database"),
				T_CHARP, cp, NULL);
			free(cp);
			return 1;
		}
		free(cp);
	}
#endif /* SEC_MAC */
#if SEC_ACL
	cp = agetstr(AUTH_F_ACL, (char **)0);
	if (cp) {
		flg->fg_acl = 1;
		if (strcmp(cp, AUTH_F_WILD) == 0) {
			fld->fd_acl = ACL_DELETE;
			fld->fd_acllen = 0;
		} else {
			fld->fd_acl = acl_er_to_ir(cp, &fld->fd_acllen);
			if (fld->fd_acl == (acle_t *) 0) {
				flg->fg_acl = 0;
				/* audgenl(??); */
				free(cp);
				return 1;
			}
		}
		free(cp);
	}
#endif /* SEC_ACL */
#if SEC_PRIV
	cp = agetstr(AUTH_F_PPRIVS, (char **)0);
	if (cp) {
		loadnamepair(fld->fd_pprivs, SEC_MAX_SPRIV, cp,
				sys_priv, AUTH_PPRIVS, OT_FILE_CNTL,
				fld->fd_name);
		flg->fg_pprivs = 1;
		free(cp);
	}

	cp = agetstr(AUTH_F_GPRIVS, (char **)0);
	if (cp) {
		loadnamepair(fld->fd_gprivs, SEC_MAX_SPRIV, cp,
				sys_priv, AUTH_GPRIVS, OT_FILE_CNTL,
				fld->fd_name);
		flg->fg_gprivs = 1;
		free(cp);
	}
#endif
	return 0;
}

/*
 * Store the file specific fields and flags associated with a File Control
 * entry.  This routine outputs to the actual file.  It returns 1 if there
 * is no error and 0 if an error occurred in writing.
 */
static int
store_fi_fields(f, name, fd, fg)
	register FILE *f;
	register char *name;
	register struct f_field *fd;
	register struct f_flag *fg;
{
	register int fields = 1;
	int error;

	fields = aptentfileq(f, &error, fields, name);

	if (!error && fg->fg_uid)  {
		fields = aptentuid(f, &error, fields, AUTH_F_OWNER, fd->fd_uid);
	}
	if (!error && fg->fg_gid)  {
		fields = aptentgid(f, &error, fields, AUTH_F_GROUP, fd->fd_gid);
	}
	if (!error && fg->fg_mode)  {
		fields = aptentonum(f, &error, fields, AUTH_F_MODE,
					(u_long) fd->fd_mode);
	}
	if (!error && fg->fg_type)  {
		fields = aptentstr(f, &error, fields, AUTH_F_TYPE, fd->fd_type);
	}
#if SEC_MAC
	if (!error && fg->fg_slevel)  {
		char *mander;

		if (fd->fd_slevel == (mand_ir_t *) 0)
			mander = AUTH_F_WILD;
		else if (memcmp(fd->fd_slevel, mand_syslo, mand_bytes()) == 0)
			mander = AUTH_F_SYSLO;
		else if (memcmp(fd->fd_slevel, mand_syshi, mand_bytes()) == 0)
			mander = AUTH_F_SYSHI;
		else
			mander = (char *) 0;
		if (mander) {
			fields = aptentstr(f, &error, fields, AUTH_F_SLEVEL,
						mander);
		}
		else {
			error = 1;
			audgenl(AUTH_EVENT,
				T_CHARP,
				MSGSTR(GETPRFIENT_4,
				"unknown clearance in File Control database"),
				NULL);
		}
	}
#endif
#if SEC_ACL
	if (!error && fg->fg_acl)  {
		char *acler;

		if (fd->fd_acl == ACL_DELETE)
			acler = AUTH_F_WILD;
		else
			acler = acl_ir_to_er(fd->fd_acl, fd->fd_acllen);
		if (acler) {
			fields = aptentstr(f, &error, fields, AUTH_F_ACL,
						acler);
		}
		else {
			error = 1;
			/* audgenl(??); */
		}
	}
#endif
#if SEC_PRIV
	if (!error && fg->fg_pprivs)  {
		fields = aptentnmpair(f, &error, fields, AUTH_F_PPRIVS,
				fld->fd_pprivs, SEC_MAX_SPRIV, sys_priv,
				AUTH_PPRIVS);
	}
	if (!error && fg->fg_gprivs)  {
		fields = aptentnmpair(f, &error, fields, AUTH_F_GPRIVS,
				fld->fd_gprivs, SEC_MAX_SPRIV, sys_priv,
				AUTH_GPRIVS);
	}
#endif

	if (name)
		return aptentfin(f, &error);

	error = (fflush(f) != 0) || error;

	return !error;
}
/* #endif */ /*} SEC_BASE */
