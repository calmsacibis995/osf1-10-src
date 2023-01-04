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
static char	*sccsid = "@(#)$RCSfile: binmail_sec.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:20:38 $";
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
 * Copyright (c) 1990, All rights reserved.
 *
 * Unix mail(1) handling hook routines
 *
 * Mail is supported at multiple security levels using a mail spool file
 * that is created at the user's clearance. Only mail written from levels
 * dominated by that clearance will be delivered. Currently, only local
 * mail is supported. When mail is written, the appropriate privilege is
 * raised to allow a write up to the target user's mail file.
 */



#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/types.h>
#include <sys/security.h>
#ifdef _OSF_SOURCE
#include <sys/fcntl.h>
#endif
#include <stdio.h>
#include <pwd.h>
#include <prot.h>

#if SEC_MAC
#include <mandatory.h>

static mand_ir_t	*invoker_clearance,
			*process_level,
			*mailbox_level;

static privvec_t	starting_privs;

extern priv_t		*privvec(),
			*forceprivs(),
			*checkprivs();
#endif

extern char		*strdup();

/*
 * mail_init()-initialization for the hook routines
 */

void
mail_init(argc, argv)
int argc;
char *argv[];
{
	set_auth_parameters(argc, argv);
	initprivs();

#if SEC_MAC
	if (forceprivs(privvec(SEC_CHOWN, SEC_ALLOWMACACCESS, -1),
			starting_privs) ||
	    checkprivs(privvec(SEC_ALLOWDACACCESS, -1)))  /* needed later */
#else
	if (!forcepriv(SEC_CHOWN))
#endif
	{
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}
}

/*
 * Retrieve the user's login name and home directory.
 * Also get the user's clearance from his authentication profile
 * and retrieve the current process sensitivity level.
 */

mail_identity(user, home)
char **user, **home;
{
	uid_t			uid = (uid_t) getluid();
	struct passwd		*pw;
#if SEC_MAC
	privvec_t		saveprivs;
	struct pr_passwd	*ppw;
#endif

	if ((int) uid == -1) {
		fprintf(stderr, "%s: login uid has not been set\n",
			command_name);
		exit(1);
	}
	pw = getpwuid(uid);
	if (pw == NULL) {
		fprintf(stderr, "%s: no passwd file entry for uid %d\n",
			command_name, getluid());
		exit(1);
	}
	*user = strdup(pw->pw_name);
	*home = strdup(pw->pw_dir);
	if (*user == NULL || *home == NULL) {
		fprintf(stderr, "%s: memory allocation failure\n",
			command_name);
		exit(1);
	}

#if SEC_MAC
	invoker_clearance = mand_alloc_ir();
	process_level = mand_alloc_ir();
	mailbox_level = mand_alloc_ir();
	if (invoker_clearance == NULL || process_level == NULL ||
	    mailbox_level == NULL) {
		fprintf(stderr, "%s: can't initialize for sensitivity labels\n",
			command_name);
		exit(1);
	}
	if (getslabel(process_level) < 0) {
		fprintf(stderr, "%s: sensitivity level has not been set\n",
			command_name);
		exit(1);
	}
	forceprivs(privvec(SEC_ALLOWDACACCESS, -1), saveprivs);
	ppw = getprpwnam(*user);
	seteffprivs(saveprivs, (priv_t *) 0);
	if (ppw == NULL) {
		fprintf(stderr, "%s: no protected password file for %s\n",
			command_name, *user);
		exit(1);
	}
	if (ppw->uflg.fg_clearance)
		mand_copy_ir(&ppw->ufld.fd_clearance, invoker_clearance);
	else if (ppw->sflg.fg_clearance)
		mand_copy_ir(&ppw->sfld.fd_clearance, invoker_clearance);
	else
		mand_copy_ir(mand_syslo, invoker_clearance);
#endif
}

#if SEC_MAC
/*
 * Ensure that the process sensitivity level equals the invoking
 * user's clearance in order to permit the mail file to be read.
 */

int
mail_check_proc_level()
{
	return memcmp(process_level, invoker_clearance, mand_bytes()) == 0;
}

/*
 * Check the level of the mailbox file and make sure it matches
 * the user's clearance.
 */

int
mail_check_file(mailfd)
	int	mailfd;
{
	return fstatslabel(mailfd, mailbox_level) == 0 &&
		memcmp(invoker_clearance, mailbox_level, mand_bytes()) == 0;
}

/*
 * Exchange the effective privilege set with the contents of the
 * starting_privs vector.  Calls to this function should come in
 * pairs.
 */

void
mail_revert_privs()
{
	seteffprivs(starting_privs, starting_privs);
}

#endif

/*
 * Check the mailbox file for the specified user to verify that its level
 * matches the user's clearance, and that our current process level is
 * dominated by the file's level.  Check upgrade authorization if the
 * levels differ.  Create the mailbox file with appropriate attributes
 * if it does not exist.
 */

int
mail_check_mailbox(user, uid, mailfile)
	char	*user;
	uid_t	uid;
	char	*mailfile;
{
#if SEC_MAC
	privvec_t		saveprivs;
	struct pr_passwd	*ppw;
	int			decision = 0;
	mand_ir_t		*clearance;

	/*
	 * Get the target user's protected password database entry
	 * and find his clearance.
	 */

	forceprivs(privvec(SEC_ALLOWDACACCESS, -1), saveprivs);
	ppw = getprpwnam(user);
	seteffprivs(saveprivs, (priv_t *) 0);
	if (ppw == NULL) {
		fprintf(stderr, "%s: no protected password entry for %s\n",
			command_name, user);
		return 0;
	}
	if (ppw->uflg.fg_clearance)
		clearance = &ppw->ufld.fd_clearance;
	else if (ppw->sflg.fg_clearance)
		clearance = &ppw->sfld.fd_clearance;
	else
		clearance = mand_syslo;

	/*
	 * Check that the target user's clearance dominates the current
	 * process level to avoid downgrading.
	 */

	decision = mand_ir_relationship(clearance, process_level);
	if ((decision & (MAND_SDOM | MAND_EQUAL)) == 0) {
		fprintf(stderr, "%s: %s: downgrade prohibited\n",
			command_name, user);
		return 0;
	}

	/*
	 * If the target mailbox sensitivity level dominates the process,
	 * then sending mail to this user is an upgrade.  If the user does
	 * not have the upgrade authorization, suppress delivery.
	 */

	if ((decision & MAND_EQUAL) == 0 && !authorized_user("upgrade")) {
		fprintf(stderr, "%s: %s: need upgrade authorization\n",
			command_name, user);
		return 0;
	}
#endif /* SEC_MAC */

	/* Create the mail file if necessary */

	if (access(mailfile, 0) == -1) {
		mode_t	save_umask = umask(7);
		int	mf;

		/*
		 * The mailbox is created with mode 0620 to allow delivery
		 * using the setgid mechanism.
		 */

		if ((mf = creat(mailfile, 0620)) == -1) {
			umask(save_umask);
			return 0;
		}
		close(mf);
		umask(save_umask);

#if SEC_MAC
		/*
		 * Set the file's sensitivity level (while we still own it)
		 * to the target user's clearance.
		 */

		if (chslabel(mailfile, clearance) == -1) {
			fprintf(stderr, "%s: ", command_name);
			psecerror("can't set file level");
			unlink(mailfile);
			return 0;
		}
#endif

		/*
		 * Make the target user the file's owner and set the
		 * file's group id to "mail".
		 */

		if (chown(mailfile, uid, starting_egid()) == -1) {
			fprintf(stderr, "%s: ", command_name);
			perror("can't set file owner/group");
			unlink(mailfile);
			return 0;
		}
	}
#if SEC_MAC
	else {
		/*
		 * The mailbox file exists already.  Check that its level
		 * matches the target user's clearance.
		 */
		
		if (statslabel(mailfile, mailbox_level) == -1) {
			fprintf(stderr, "%s: ", command_name);
			psecerror("can't check file level");
			return 0;
		}
		if (memcmp(mailbox_level, clearance, mand_bytes())) {
			fprintf(stderr,
				"%s: %s: mailbox not at user's clearance\n",
				command_name, user);
			return 0;
		}
	}
#endif

	return 1;
}

#endif
