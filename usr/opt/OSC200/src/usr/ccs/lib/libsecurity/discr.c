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
static char *rcsid = "@(#)$RCSfile: discr.c,v $ $Revision: 4.2.10.5 $ (DEC) $Date: 1993/12/16 23:56:01 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
 */


/*
 * Based on:

 */

/*LINTLIBRARY*/

/*
 * This module includes routines that access the File Control database
 * to create files with proper attributes.
 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include <errno.h>
#include <unistd.h>


#if SEC_MAC
#include <mandatory.h>
#endif
#if SEC_ACL
#include <acl.h>
#endif

extern char *strrchr();
extern char *malloc();

static int create_dir_securely();
static int set_file_securely();

static int
set_file_securely(file, reason)
	char *file;
	char *reason;
{
	int labeledfs;	/* check for f/s attributes */
	struct pr_file *pr;
	struct stat sb;
	int (*remfile)();

	/*
	 * Figure out how to remove the file if we have to punt.
	 */

	if (stat(file, &sb) == 0 && S_ISDIR(sb.st_mode))
		remfile = rmdir;
	else
		remfile = unlink;

	/*
	 * Get entry for thie file from the file control database.
	 */

	pr = getprfinam(file);
	if (pr == (struct pr_file *) 0) {
		(void) audgenl(AUTH_EVENT, T_CHARP, MSGSTR(DISCR_2, "could not obtain entry of file to create securely"), T_CHARP, file, NULL);
		(void) (*remfile)(file);
		return(CFS_NO_FILE_CONTROL_ENTRY);
	}

	/*
	 * Figure out whether we have the right kind of file.
	 */

	 if (pr->uflg.fg_type) {
		int ok;
		switch (*pr->ufld.fd_type) {
		case 'f':
			ok = S_ISFIFO(sb.st_mode);
			break;
		case 'r':
			ok = S_ISREG(sb.st_mode);
			break;
#if SEC_MAC
		case 'd':
			forcepriv(SEC_MULTILEVELDIR);
			ok = stat(file, &sb) == 0 && S_ISDIR(sb.st_mode) && !ismultdir(file);
			disablepriv(SEC_MULTILEVELDIR);
			break;
		case 'm':
			forcepriv(SEC_MULTILEVELDIR);
			ok = stat(file, &sb) == 0 && S_ISDIR(sb.st_mode) && ismultdir(file);
			disablepriv(SEC_MULTILEVELDIR);
			break;
#else /* SEC_MAC */
		case 'd':
			ok = S_ISDIR(sb.st_mode);
			break;
#endif /* ! SEC_MAC */
		case 'b':
			ok = S_ISBLK(sb.st_mode);
			break;
		case 'c':
			ok = S_ISCHR(sb.st_mode);
			break;
		case 's':
			ok = S_ISSOCK(sb.st_mode);
			break;
		case 'l':
			ok = (lstat(file, &sb) == 0 && S_ISLNK(sb.st_mode));
			break;
		default:
			ok = 0;
			break;
		}
		if (! ok) {
			(void) audgenl(AUTH_EVENT, T_CHARP, MSGSTR(DISCR_1, "could not create file securely"), T_CHARP, file, NULL);
			(void) (*remfile)(file);
			return(CFS_CAN_NOT_CHG_MODE);
		}
	}

	/*
	 * An unspecified owner (group) in the authentication database
	 * means that the current owner (group) of this process will
	 * own the file.  Otherwise, it is set as specified.  Also, if
	 * the mode is unspecified, set it to 0.
	 */
	if (!pr->uflg.fg_uid)
		pr->ufld.fd_uid = starting_ruid();
	if (!pr->uflg.fg_gid)
		pr->ufld.fd_gid = starting_rgid();
	if (!pr->uflg.fg_mode)
		pr->ufld.fd_mode = 0;
#if SEC_ACL
	/*
	 * Retrieve indication of whether filesystem has security
	 * attributes.
	 */

	labeledfs = islabeledfs(file);

	/*
	 * If the database specifies an ACL, set it before changing
	 * the mode.
	 */
	if (labeledfs && pr->uflg.fg_acl &&
	    chacl(file, pr->ufld.fd_acl, pr->ufld.fd_acllen) != 0) {
		(void) (*remfile)(file);
		(void) audgenl(AUTH_EVENT, 
			T_CHARP, MSGSTR(DISCR_3, "could not assign ACL specified in File Control database"), T_CHARP, file, NULL);
		return(CFS_CAN_NOT_CHG_ACL);
	}
#endif

	/*
	 * Change the mode before the owner/group so that we can
	 * correct the mode before we (possibly) give away the file.
	 */
	if (chmod(file, pr->ufld.fd_mode) != 0)  {
		(void) (*remfile)(file);
		(void) audgenl(AUTH_EVENT, T_CHARP, MSGSTR(DISCR_4, "could not set mode specified in File Control database"), T_CHARP, file, NULL);
		return(CFS_CAN_NOT_CHG_MODE);
	}

#if SEC_MAC
	/*
	 * If the database specifies a sensitivity level, set it.
	 */
	if (labeledfs && pr->uflg.fg_slevel &&
	    chslabel(file, pr->ufld.fd_slevel) != 0) {
		(void) (*remfile)(file);
		(void) audgenl(AUTH_EVENT, T_CHARP, MSGSTR(DISCR_5, "could not set sensitivity level specified in File Control database"), T_CHARP, file, NULL);
		return(CFS_CAN_NOT_CHG_SL);
	}
#endif


	/*
	 * Finally, set the owner and group as specified in the
	 * database.
	 */
	if (chown(file, pr->ufld.fd_uid, pr->ufld.fd_gid) != 0) {
		(void) (*remfile)(file);
		(void) audgenl(AUTH_EVENT, T_CHARP, MSGSTR(DISCR_7, "could not set owner and group specified in File Control database"), T_CHARP, file, NULL);
		return(CFS_CAN_NOT_CHG_OWNER_GROUP);
	}

	return(CFS_GOOD_RETURN);
}

static int
create_dir_securely(path, decibels, reason)
	char *path;
	int decibels;
	char *reason;
{
	char *cp;
	struct pr_file *pr;
	int i;

	pr = getprfinam(path);
	if (! pr)
		return(CFS_NO_FILE_CONTROL_ENTRY);
	if (!pr->uflg.fg_type || *pr->ufld.fd_type != 'd')
		return(CFS_CAN_NOT_OPEN_FILE);
	if (decibels == AUTH_SILENT)
		enter_quiet_zone();
	i = mkdir(path, 0);
	if (i < 0 && errno == ENOENT) {
		cp = strrchr(path, '/');
		if (!cp) {
			if (decibels == AUTH_SILENT)
				exit_quiet_zone();
			return(CFS_CAN_NOT_OPEN_FILE);
		}
		*cp = 0;
		i = create_dir_securely(path, AUTH_LIMITED, reason);
		*cp = '/';
		i = mkdir(path, 0);
	}
	if (i < 0) {
		if (decibels == AUTH_SILENT)
			exit_quiet_zone();
		return(CFS_CAN_NOT_OPEN_FILE);
	}
	i = set_file_securely(path, reason);
	if (decibels == AUTH_SILENT)
		exit_quiet_zone();
	return(i);
}

/*
 * This procedure will create a file as specified in the authentication
 * database.  It is more flexible than hard-coding the discretionary
 * attributes of the file, and a further assurance that the file being
 * created is in accordance with security concerns.
 */
int
create_file_securely(file, decibels, purpose)
	char *file;
	int decibels;
	char *purpose;
{
	register int fd;
	register struct pr_file *pr;
	register int i;
	int changed_i;
	time_t orig_mtime;
	struct stat sb;
	char is_temp_file;

	if (decibels == AUTH_SILENT)
		enter_quiet_zone();


	/* Try to exclusively create the file.
	 * The AUTH_VERBOSE argument to this routine specifies that
	 * this call should wait until the file does not exist.
	 * If creating a temporary file and the file hasn't been changed
	 * after AUTH_LOCK_ATTEMPTS tries, unlink the file.
	 * Otherwise, the existence of the file allows the routine to
	 * succeed.
	 */

	orig_mtime = 0;
	changed_i = 0;
	is_temp_file = strcmp(&file[strlen(file)-2], AUTH_TEMP_EXT) == 0;
	for (i = 0; i < AUTH_LOCK_ATTEMPTS; i++) {
		fd = open(file, O_RDONLY | O_CREAT | O_EXCL, 0640);
		if (fd < 0 && errno == ENOENT) {
			char *cp = strrchr(file, '/');
			if (cp) {
				*cp = 0;
				if (stat(file, &sb) < 0 && errno == ENOENT) {
					(void) create_dir_securely(file, AUTH_LIMITED, purpose);
					*cp = '/';
					fd = open(file, O_RDONLY|O_CREAT|O_EXCL, 0640);
				}
				else
					*cp = '/';
			}
		}
		if (fd >= 0 || decibels != AUTH_VERBOSE)
			break;
		/*
		 * If file exists, check conditions for forced removal
		 */

		if (is_temp_file && stat(file, &sb) == 0) {

			if (i - changed_i == AUTH_LOCK_ATTEMPTS / 2 &&
			    sb.st_mtime == orig_mtime)
				unlink(file);

			/*
			 * If original time not set or changed, reset it
			 */

			if (!orig_mtime || sb.st_mtime != orig_mtime) {
				orig_mtime = sb.st_mtime;
				changed_i = i;
			}
		}
		(void) sleep (AUTH_RETRY_DELAY);
	}

	if (fd < 0) {
		if (decibels == AUTH_VERBOSE) {
			(void) audgenl(AUTH_EVENT, T_CHARP, MSGSTR(DISCR_1,
				"could not create file securely"), T_CHARP,
				file, NULL);
			return(CFS_CAN_NOT_OPEN_FILE);
		}
	}

	/*
	 * Close the file now or an attempt to regrade the file
	 * will fail because of the reference count check.
	 */

	(void) close(fd);

	/*
	 * Set file attributes appropriately
	 */

	i = set_file_securely(file, purpose);

	if (decibels == AUTH_SILENT)
		exit_quiet_zone();

	return(i);
}


/*
 * This routine sets the effective user and group IDs to the real user
 * and group IDs.  This allows the program to have the access protection
 * of the invoking user, and is for use by protected subsystems.
 */
void
setuid_least_privilege()
{
	check_auth_parameters();

	/*
	 * Reset effective user and group IDs to real user and group IDs
	 */
	if (starting_ruid() != starting_euid())
		if (setuid(starting_ruid()) != 0)  {
			(void) fprintf(stderr, MSGSTR(DISCR_8, "%s: cannot reset user\n"),
				       command_name);
			exit(1);
		}
	if (starting_rgid() != starting_egid())
		if (setgid(starting_rgid()) != 0)  {
			(void) fprintf(stderr, MSGSTR(DISCR_9, "%s: cannot reset group\n"),
				       command_name);
			exit(1);
		}
}


/*
 * Create companion files for pathname for later saving (the oldpathname
 * file) and replacing (the temppathname file).  The space obtained here
 * is freed internally if this fails or is returned by replace_file() if
 * this succeeds.
 */
int
make_transition_files(pathname, ptemppathname, poldpathname)
	register char *pathname;
	register char **ptemppathname;
	register char **poldpathname;
{
	register int pathlen;

	check_auth_parameters();

	pathlen = strlen(pathname);

	*ptemppathname = malloc(pathlen + strlen(AUTH_TEMP_EXT) + 1);
	if (*ptemppathname == (char *) 0) 
		return 0;

	*poldpathname = malloc(pathlen + strlen(AUTH_OLD_EXT) + 1);
	if (*poldpathname == (char *) 0)  {
		free(*ptemppathname);
		return 0;
	}

	(void) strcpy(*ptemppathname, pathname);
	(void) strcat(*ptemppathname, AUTH_TEMP_EXT);

	(void) strcpy(*poldpathname, pathname);
	(void) strcat(*poldpathname, AUTH_OLD_EXT);

	return 1;
}


/*
 * Before moving the temporary file to the real path,
 * save the old file with a .o extension in case
 * we can't update the real file.  In all cases, we
 * try to have either the old or new file in the real
 * path s.t. the database integrity is kept.
 * 
 * We chose to link rather than write directly s.t. anyone reading
 * the file will not get a partially built file.  (Readers do not
 * need to lock;  they use the AUTH_CHKENT entry to validate
 * authentication database entries.)
 *
 * This routine assumes temppathname and oldpathname were created from
 * make_transition_files(), because the space is deallocated here.
 */
int
replace_file(temppathname, pathname, oldpathname)
	register char *temppathname;
	register char *pathname;
	register char *oldpathname;
{
	register int good_update = 0;

	check_auth_parameters();

	(void) unlink(oldpathname);
	if ((access(pathname, 0) != 0) || (link(pathname, oldpathname) == 0))  {

		/*
		 * When making a new entry, this file may not exist
		 * so we don't care about the status.  If it turns
		 * out that it did exist and we couldn't remove it,
		 * the next link below will fail and the purpose is served.
		 */

		if (rename(temppathname, pathname) == 0)  {
			good_update = 1;
			(void) unlink(oldpathname);
		}
		else {
			sigset_t newset, oldset;
			sigemptyset(&newset);
			sigaddset(&newset, SIGINT);
			sigaddset(&newset, SIGHUP);
			sigaddset(&newset, SIGALRM);
			sigaddset(&newset, SIGQUIT);
			sigaddset(&newset, SIGTSTP);
			(void) sigprocmask(SIG_BLOCK, &newset, &oldset);
			(void) unlink(pathname);
			if (link(temppathname, pathname) == 0)  {
				good_update = 1;
				(void) unlink(oldpathname);
			}
			else
				(void) link(oldpathname, pathname);
			(void) sigprocmask(SIG_SETMASK, &oldset, NULL);
		}
	}

	(void) unlink(temppathname);

	/* flush files to disk to avoid the possibility of lost files */
	sync();

	free(oldpathname);
	free(temppathname);

	return good_update;
}
/* #endif */ /*} SEC_BASE */
