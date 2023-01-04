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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: cron_sec.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/06/10 14:14:54 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 */
/* #ident "@(#)cron_sec.c	4.1 10:16:35 7/12/90 SecureWare" */
/*
 * Based on:	@(#)cron_sec.c	2.16.1.1 18:08:21 12/29/89
 */

#include <sys/secdefines.h>

#if SEC_BASE
#include "cron_msg.h"
nl_catd catd;
#define MSGSTR_SEC(Num,Str) catgets(catd,MS_CRON_SEC,Num,Str)

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include <protcmd.h>

#if SEC_MAC
#include <mandatory.h>
#endif

#define	SECURE_MODE	0060	/* R/W by cron group only */

static int cron_user;
static struct pr_field *dummy;	/* used for sizeof below only */
static char this_user[sizeof(dummy->fd_name)];
static privvec_t our_sprivs;
static privvec_t our_bprivs;
static privvec_t nosprivs = { 0 };

#if SEC_MAC
static char *message;
static int msg_size;
void cron_setlevel();
static mand_ir_t *subproclevel = (mand_ir_t *) 0;
static tmp_audit = 0;
#endif

static void close_auth_files();
#if SEC_MAC
static void mldscan();
#endif

extern priv_t *privvec();


/*
 * Startup initialization code.  Make sure that our luid is not set.
 */
cron_init(argc, argv)
	int	argc;
	char	*argv[];
{
	set_auth_parameters(argc, argv);
	initprivs();

	errno = 0;
	getluid();
	if (errno == 0)		/* no error, luid is already set */
		crabort(MSGSTR_SEC(CRON_SEC_1, "must be started by init."));

#if SEC_MAC
	if (mand_init() || (subproclevel = mand_alloc_ir()) == (mand_ir_t *) 0)
		crabort(MSGSTR_SEC(CRON_SEC_2,"cannot initialize for sensitivity labels."));
#endif
}


/*
 * Set the process file mask to a value that totally prevents unauthorized
 * access.
 */
int
cron_secure_mask()
{
	return umask(~SEC_DEFAULT_MODE);
}


/*
 * First, on the first pass through here, get the gid for the cron group.
 * Make the FIFO file so that at and crontab can communicate new jobs to the
 * daemon.  Make sure the file is only accessible through the cron group.
 *
 * By the time we leave this routine, the fifo file is good or it is removed.
 */
int
cron_set_communications(file, nofork)
	register char *file;
	int nofork;
{
	register int good_fifo = 0, save_umask;
	privvec_t saveprivs;
	struct stat stb;
	static been_here_before = 0;
	static int cron_group;

	if (!been_here_before)  {
		been_here_before = 1;
		cron_user = pw_nametoid("cron");
		if (cron_user == -1)
			crabort(MSGSTR_SEC(CRON_SEC_3, "cannot find cron user"));
		cron_group = gr_nametoid("cron");
		if (cron_group == -1)
			crabort(MSGSTR_SEC(CRON_SEC_4, "cannot find cron group"));
	}

	/*
	 * If there is a file already there, make sure it's a fifo that
	 * is only readable by the cron group.  If not, remove it so that
	 * we can replace it with a file that meets our needs.
	 */
	if (stat(file, &stb) == 0)  {
		if (stb.st_mode == (S_IFIFO | SECURE_MODE) &&
		    stb.st_uid == cron_user && stb.st_gid == cron_group)
			good_fifo = 1;
		else
			unlink(file);
	}

	/*
	 * If the FIFO is not there or had the wrong attributes,
	 * make a new one with correct attributes.
	 */
	if (!good_fifo) {
		good_fifo = 1;
		save_umask = umask(0);
		disableprivs(privvec(SEC_SUSPEND_AUDIT, -1), saveprivs);
		if (mknod(file, S_IFIFO | SECURE_MODE, 0) == -1 ||
		    chown(file, cron_user, cron_group) == -1)
			good_fifo = 0;
		umask(save_umask);

#if SEC_MAC
		if (good_fifo && islabeledfs(file)) {
			if (chslabel(file, mand_syshi) == -1)
				good_fifo = 0;
		}
#endif
		seteffprivs(saveprivs, (priv_t *) 0);
		if (!good_fifo)
			unlink(file);
	}

	return good_fifo;
}

#if SEC_MAC
/*
 * Find an available file in the temp dir.  On systems configured with
 * SEC_MAC, the pathname must include the name of the MLD diversion
 * directory.
 */
char *
cron_tempnam(dir, prefix, level)
	char *dir;
	char *prefix;
	char *level;
{
	char *fullpath;

	{
		register char *subdirpath;
		register char *subdir;
		register char *levelstr;

		subdir = ir_to_subdir(dir, (mand_ir_t *) level);

		/*
		 * if the above fails, either (1) the temp dir is not a
		 * multilevel dir, in which case we'll just put the file
		 * in it directly, or (2) the MLD child for this level is
		 * missing, in which case just return null and let the caller
		 * fail.
		 */

		if (subdir == (char *) 0) {
			if (!ismultdir (dir)) {
				if (!tmp_audit) {
					audit_subsystem(MSGSTR_SEC(CRON_SEC_18, "make temporary file for cron output"),
							MSGSTR_SEC(CRON_SEC_19, "temp dir is not multilevel directory"), ET_SUBSYSTEM);
					tmp_audit = 1;
				}
				msg(MSGSTR_SEC(MS_SEC_NOTMLD, "%s is not a multilevel dir."), dir);
				fullpath = (char *) 0; /* fail, to prevent */
							/* writedown path */
			}
			else {
				levelstr = mand_ir_to_er((mand_ir_t *) level);
				msg(MSGSTR_SEC(MS_SEC_NOMLDCHILD, "No child directory for %s in %s."),
					   (levelstr == (char *)0 ? "sensitivity label" : levelstr), dir);
				fullpath = (char *) 0;
			}
		}
		else {		/* subdir is OK */
			subdirpath = malloc(strlen(dir) + strlen(subdir) + 2);
			if (subdirpath == (char *) 0)
				fullpath = (char *) 0;
			else  {
				sprintf(subdirpath, "%s/%s", dir, subdir);
				fullpath = tempnam(subdirpath, prefix);
				free(subdirpath);
			}
		}
	}

	cron_setlevel(level);
		
	return fullpath;
}


/*
 * Set the notion of the `current' security level.  This level is in
 * force while processing a new (file or fifo) request, executing a
 * task, or cleaning up a finished task.
 */
void
cron_setlevel(level)
	char *level;
{
	mand_copy_ir((mand_ir_t *) level, subproclevel);
	level += mand_bytes();
}

/*
 * Form the line for mailing the results of the job.  We form the longname
 * minus the name minus the subdirectory name, so the path appears as
 * it would to a process without the MULTILEVELDIR privilege.  This
 * assumes the subdirectory is the second to last component of longname.
 */
void
cron_mail_line(line, mailpgm, name, longname)
	char *line;
	char *mailpgm;
	char *name;
	char *longname;
{
	register char *last_comp;
	register char *seclast_comp;

	/*
	 * We have to stick a blank line at the top
	 * of the outfile so mail doesn't get
	 * error msgs confused with mail headers.
	 */
	(void) strcpy(line, "(echo; /usr/bin/cat \"");

	last_comp = (char *) strrchr(longname, '/');
	if (last_comp == (char *) 0)
		strcat(line, longname);
	else  {
		*last_comp = '\0';
		seclast_comp = (char *) strrchr(longname, '/');
		*last_comp = '/';

		if (seclast_comp == (char *) 0)
			strcat(line, longname);
		else  {
			strncat(line, longname, seclast_comp - longname);
			strcat(line, last_comp);
		}
	}

	sprintf(strchr(line, '\0'), "\") | %s %s\n", mailpgm, name);
}
#endif /* SEC_MAC */


/*
 * Set the LUID for the user (it should not be set until now).
 * Also, set the privileges to what the user normally gets upon
 * login.  If we can't 1) set the LUID, 2) get the user from the
 * protected passwd database, or 3) set the privileges, don't
 * run the at or crontab file since it may have an insecure
 * environment.
 *
 * There is no need to do a chroot() if we separate all cron
 * daemons and run one for each restricted environment.  Because
 * cron (and at and crontab) have hard-wired locations for the
 * files it uses, it is not too hard to separate cron's -- one
 * for each environment.  Administratively, we just don't link the
 * FIFOs or work files.  Since this is not in the cron code, and
 * general users will not have access to these files, there is no
 * problem.
 */
void
#if SEC_MAC
cron_set_user_environment(level, user_name, luid)
	char *level;
#else
cron_set_user_environment(user_name, luid)
#endif
	register char *user_name;
	register int luid;
{
	register struct pr_passwd *pr;
	register priv_t *ptr;
#if SEC_MAC
	register mand_ir_t *clearance;
#endif

	pr = getprpwnam(user_name);
	if (pr == (struct pr_passwd *) 0)  {
		audit_auth_entry(user_name, OT_PRPWD,
		   MSGSTR_SEC(CRON_SEC_5, "cannot find Protected Password entry for cron verification"),
				ET_SUBSYSTEM);
		mail(user_name, MSGSTR_SEC(CANTGETAUTH,"can't get authentication information for you.\nYour commands will not be executed."), 2);
		exit(1);
	}

	/*
	 * If user's account is frozen, don't bother mailing to him because
	 * his account may be locked for a long time and the mail will
	 * pile up.
	 */
	if (locked_out(pr))
		exit(1);

	/*
	 * Check consistency between /etc/passwd and
	 * protected password entry.
	 */
	if (!pr->uflg.fg_uid || (pr->ufld.fd_uid != luid))  {
		audit_auth_entry(user_name, OT_PRPWD,
		  MSGSTR_SEC(CRON_SEC_6, "mismatch in /etc/passwd and Protected Password UIDs"),
		  ET_SUBSYSTEM);
		mail(user_name, MSGSTR_SEC(AUTHMISMATCH, "authentication mismatch on your account.\nYour commands will not be executed."), 2);
		exit(1);
	}

	/* check for retired account */

	if (pr->uflg.fg_retired && pr->ufld.fd_retired) {
		audit_auth_entry(user_name, OT_PRPWD, MSGSTR_SEC(CRON_SEC_7,
		  "user cannot submit cron jobs because account is retired"),
		  ET_SUBSYSTEM);
		exit(1);
	}

	/*
	 * Just like login, set a secure umask.  Let the user explicitly
	 * relax the mode.
	 */
	(void) umask (~SEC_DEFAULT_MODE);

#if SEC_MAC
	if (pr->uflg.fg_clearance)
		clearance = &pr->ufld.fd_clearance;
	else if (pr->sflg.fg_clearance)
		clearance = &pr->sfld.fd_clearance;
	else
		clearance = (mand_ir_t *) 0;

	if (clearance == (mand_ir_t *) 0)  {
		audit_subsystem(MSGSTR_SEC(CRON_SEC_8, "get clearance for at/cron session"),
			MSGSTR_SEC(CRON_SEC_9,"no clearance in protected password entry, session is aborted"), ET_SUBSYSTEM);
		mail(user_name, MSGSTR_SEC(CANTGETCLEAR, "can't get clearance.\nYour commands will not be executed."), 2);
		exit(1);
	}

	if (setclrnce(clearance) != 0)  {
		audit_subsystem(MSGSTR_SEC(CRON_SEC_10,"set clearance for at/cron session"),MSGSTR_SEC(CRON_SEC_11,
			"setclrnce failed, session is aborted"), ET_SEC_LEVEL);
		mail(user_name, MSGSTR_SEC(CANTSETCLEAR, "can't set clearance.\nYour commands will not be executed."), 2);
		exit(1);
	}

	if (setslabel((mand_ir_t *) level) != 0)  {
		audit_subsystem(MSGSTR_SEC(CRON_SEC_12,"set level for at/cron session"),
			MSGSTR_SEC(CRON_SEC_13,"setslabel failed, session is aborted"), ET_SEC_LEVEL);
		mail(user_name, MSGSTR_SEC(CANTSETLEVEL, "can't set security level.\nYour commands will not be executed."), 2);
		exit(1);
	}
	level += mand_bytes();
#endif

	if (setluid(luid) != 0)  {
		audit_subsystem(MSGSTR_SEC(CRON_SEC_14,"set login UID for at/cron session"),
			MSGSTR_SEC(CRON_SEC_15,"setluid failed, session is aborted"), ET_SUBSYSTEM);
		mail(user_name, MSGSTR_SEC(CANTSETLUID, "can't set login UID.\nYour commands will not be executed."), 2);
		exit(1);
	}

	if (pr->uflg.fg_sprivs)
		ptr = pr->ufld.fd_sprivs;
	else if (pr->sflg.fg_sprivs)
		ptr = pr->sfld.fd_sprivs;
	else
		ptr = nosprivs;
	memcpy(our_sprivs, ptr, sizeof our_sprivs);

	if (pr->uflg.fg_bprivs)
		ptr = pr->ufld.fd_bprivs;
	else if (pr->sflg.fg_bprivs)
		ptr = pr->sfld.fd_bprivs;
	else
		ptr = nosprivs;
	memcpy(our_bprivs, ptr, sizeof our_bprivs);

	/*
	 * Set the user's special audit parameters.
	 */
	audit_adjust_mask(pr);

	(void) strcpy(this_user, pr->ufld.fd_name);

	/*
	 * Make sure all the database files are closed so that the
	 * user does not get leftover file descriptors in his batch
	 * session.
	 */
	close_auth_files();
}


/*
 * We turn off self-audit after the UID and GID values are fixed so that 
 * when auditing starts up, the process will immediately reflect the user's
 * environment.
 *
 * Note:  This can only be used after cron_set_user_environment() because it
 *	  defines both user_name and our_sprivs;
 */
void
cron_adjust_privileges()
{
	if (setsysauths(our_sprivs) || setbaseprivs(our_bprivs)) {
		audit_subsystem(MSGSTR_SEC(CRON_SEC_16,"set privileges for at/cron session"),
				MSGSTR_SEC(CRON_SEC_17,"setpriv failed and session is aborted"),
				ET_SUBSYSTEM);
		if (*this_user)
			mail(this_user, MSGSTR_SEC(CANTSETPRIV,"system privileges cannot be set for you.\nYour commands will not be executed."), 2);
	}
#if SEC_MAC
	/*
	 * This is done so the mktemp file will be created in the
	 * appropriate subdirectory.
	 */
	disablepriv(SEC_MULTILEVELDIR);
#endif

	/*
	 * This is done so the nice() will give a high priority, if the
	 * job class calls for it.
	 */
	forcepriv(SEC_LIMIT);
}


/*
 * Close all open files.
 */
void
cron_close_files()
{
	register int scan_files;
	int nfile = sysconf(_SC_OPEN_MAX);

	/*
	 * We must do this first so that the static information in the
	 * database files will be in sync with the true state of all the
	 * files being closed.
	 */
	close_auth_files();

	/*
	 * For those files where we can get access via file
	 * pointers, use fclose so that the buffers will drain.
	 */

	(void) fclose(stdin);
	(void) fclose(stdout);
	(void) fclose(stderr);

	for (scan_files = 0; scan_files < nfile; scan_files++)
		(void) close(scan_files);
}


/*
 * Sending mail must be done by a subprocess because the mail command
 * is SGID and we must set identity before the exec.  However, doing
 * a setluid() in the parent process spoils it for all future jobs.
 * This returns the PID of the child, 0 for the child itself, and -1 for
 * a failed fork.
 */
int
cron_mail_setup(same_proc)
	int same_proc;
{
	int pid;
	int status;
	privvec_t privs;

	if (same_proc)
		pid = 0;
	else
		pid = fork();

	if (pid == 0)  {
		(void) forcepriv(SEC_SETPROCIDENT);
		(void) setluid(cron_user);
		if (setuid(cron_user) != 0) /* MUST check setuid rval */
			exit(1);
		(void) disablepriv(SEC_SETPROCIDENT);

#if SEC_MAC
		(void) disablesysauth(SEC_MULTILEVELDIR);
		if ((setclrnce(subproclevel) != 0) ||
		    (setslabel(subproclevel) != 0))
			exit(1);
#endif
	}

	return pid;
}


/*
 * The only thing the subprocess needs to do is mail the message.
 * After that, it is done.
 */
void
cron_mail_finish()
{
	exit(0);
}


#if SEC_MAC
void
cron_existing_jobs(read_error, fn, iscrontab)
	char *read_error;
	int (*fn)();
	int iscrontab;
{
	MDIR *dir;

	dir = openmultdir(".", MAND_MLD_ALLDIRS, (mand_ir_t *) 0);
	if (dir == (MDIR *) 0)  {
		if (iscrontab)
			crabort(read_error);
		else
			msg(read_error);
	}
	else  {
		mldscan(dir, fn);
		closemultdir(dir);
	}
}


char *
cron_jobname(curr_dir_code, req_dir_code, dir_name, user_name, buf)
	int curr_dir_code;
	int req_dir_code;
	char *dir_name;
	char *user_name;
	register char *buf;
{
	register char *subdir;

	if(curr_dir_code != req_dir_code)
		strcpy(buf, dir_name);
	else
		strcpy(buf, ".");

	/*
	 * Get the name of the subdirectory with the given IR.
	 * If we can't get the name, fill in the buffer with a path that
	 * will make the path check fail.
	 */
	subdir = ir_to_subdir(buf, subproclevel);
	if (subdir == (char *) 0)  {
		buf[0] = (char) ~0;
		buf[1] = 0;
	}
	else  {
		strcat(buf, "/");
		strcat(buf, subdir);
		strcat(buf, "/");
		strcat(buf, user_name);
	}

	return buf;
}
#endif /* SEC_MAC */

#if SEC_MAC
char *
cron_getlevel()
{
	register char *cp, *buffer;
	int buflen = 0;

	buflen += mand_bytes();
	buffer = malloc(buflen);
	cp = buffer;
	if (cp != (char *) 0) {
		mand_copy_ir(subproclevel, (mand_ir_t *) cp);
		cp += mand_bytes();
	}
	return buffer;
}


char *
cron_set_message(base_size)
	int base_size;
{
	if (message == (char *) 0)  {
		msg_size = base_size;
		msg_size += mand_bytes();
		message = malloc(msg_size);
	}

	return message;
}


/*
 * Read the message from at or crontab and save the security level
 * sent as part of the message to be used later in job identification.
 */
int
cron_read_message(fd, buffer, orig_size, amt_read)
	int fd;
	char *buffer;
	int orig_size;
	int *amt_read;
{
	int correct_size;

	*amt_read = read(fd, buffer, msg_size);
	correct_size = (*amt_read == msg_size);

	if (correct_size)
		cron_setlevel(buffer + orig_size);
	return correct_size;
}


/*
 * Release the space for the security level when the job is deleted.
 */
void
cron_release_ir(level)
	char *level;
{
	if (level != (char *) 0)
		free(level);
}
#endif /* SEC_MAC */


#if SEC_MAC
/*
 * Job matching involves a match of both the user AND the security
 * level.
 */
int
cron_id_match(req_name, name, level)
	char *req_name;
	char *name;
	char *level;
{
	int equal;

	equal = ((strcmp(name, req_name) == 0) &&
		 (memcmp(level, subproclevel, mand_bytes()) == 0));

	return equal;
}
#endif


/*
 * Close all Authentication database-related files.
 */
static void
close_auth_files()
{
	endpwent();
	endgrent();
	endprpwent();
	endprfient();
	endprdfent();
}


#if SEC_MAC
/*
 * Scan all the files within a multilevel directory and call fn for
 * each of the files.
 */
static void
mldscan(df, fn)
	MDIR *df;
	int (*fn)();
{

	register	i;
	struct		dirent	*dp;
	char		fullname[256];
	char            compname[NAME_MAX + 1];
	int		ret;

	readmultdir(df, fullname, &dp);

	while (dp != (struct dirent *) 0)  {
		strcat(fullname, "/");
		strcat(fullname, dp->d_name);
		if (statslabel(fullname, subproclevel) == 0) {
			/* some systems use a static buffer for readdir */
			strcpy(compname, dp->d_name);
			(*fn) (compname);
		}
		readmultdir(df, fullname, &dp);
	}
}
#endif /* SEC_MAC */

#endif /* SEC_BASE */
