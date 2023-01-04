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
static char rcsid[] = "@(#)$RCSfile: expre_sec.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 21:28:43 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc.
 * and should be treated as Confidential.
 */

#include <sys/secdefines.h>

#if SEC_MAC

/* #ident "@(#)expre_sec.c	5.2 16:33:52 8/16/90 SecureWare" */
/*
 * Based on:	@(#)expre_sec.c	10.1 15:11:21 2/16/90
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <sys/security.h>
#include <mandatory.h>
#include <prot.h>
#include "ex_msg.h"
extern nl_catd ex_catd;
#define MSGSTR(id,ds)      catgets(ex_catd, MS_EX, id, ds)

extern int	errno;

/*
 * Check user's authorization to run expreserve.  If authorized,
 * make sure we have the necessary  privileges turned on.
 */

expreserve_check_auth()
{
	if (!authorized_user("sysadmin"))
		return 0;
	
	if (mand_init()) {
		fprintf(stderr, MSGSTR(SECMSG_1, "%s: can't initialize for sensitivity labels\n"),
			command_name);
		catclose(ex_catd);
		exit(1);
	}

	if (setclrnce(mand_syshi) < 0) {
		fprintf(stderr, MSGSTR(SECMSG_2, "%s: can't set clearance\n"), command_name);
		catclose(ex_catd);
		exit(1);
	}

	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_CHOWN, SEC_LIMIT,
			       SEC_OWNER, SEC_MULTILEVELDIR, SEC_ALLOWMACACCESS,
#if SEC_ILB
			       SEC_ALLOWILBACCESS,
#endif
#if SEC_NCAV
			       SEC_ALLOWNCAVACCESS,
#endif
			       -1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(SECMSG_3, "%s: insufficient privileges\n"), command_name);
		catclose(ex_catd);
		exit(1);
	}

#if SEC_ILB
	/*
	 * Allow the process IL to float to the level of the files
	 * being saved.  We set it to syslo before copying each file.
	 */
	disablepriv(SEC_ILNOFLOAT);
#endif
	return 1;
}


/*
 * Scan the specified spool directory for lost editor sessions.
 */

expreserve_full_copy(spooldir, prefix)
	char	*spooldir;
	char	*prefix;
{
	int		prelen;
	MDIR		*mdp;
	struct dirent	*ent;
	char		fullpath[NAME_MAX];
	char		subdir[NAME_MAX];
	struct stat	stat_buf;

	mdp = openmultdir(spooldir, MAND_MLD_ALLDIRS, (mand_ir_t *) 0);
	if (mdp == (MDIR *) 0)  {
		perror(spooldir);
		catclose(ex_catd);
		exit(1);
	}

	prelen = strlen(prefix);

	for (;;) {
		readmultdir(mdp, subdir, &ent);

		if (ent == (struct dirent *) 0)
			break;

		if (ent->d_fileno == 0)
			continue;
		if (strncmp(ent->d_name, prefix, prelen) != 0)
			continue;

		sprintf(fullpath, "%s/%s/%s", spooldir, subdir, ent->d_name);
		if (stat(fullpath, &stat_buf) == 0 &&
		    (stat_buf.st_mode & S_IFMT) == S_IFREG)
			copyout(fullpath);
	}

	return 1;
}

/*
 * Create a file in the preserve directory at the appropriate
 * sensitivity level.  If saving standard input, just use the
 * current process level.  Otherwise, adjust our process level
 * to match the level of the file being saved.
 */

int
expreserve_create_file(name, pattern, uid)
	char *name;
	char *pattern;
	int uid;
{
	int			ret;
	static mand_ir_t	*sl;

	/*
	 * If preserving standard input, create the file as usual.
	 */
	if (name == (char *) 0) {
		disablepriv(SEC_MULTILEVELDIR);		/* paranoia */
		ret = creat(pattern, 0600);
		if (ret >= 0)
			chown(pattern, uid, 0);
		return(ret);
	}

	if (sl == (mand_ir_t *) 0) {
		sl = mand_alloc_ir();
		if (sl == (mand_ir_t *) 0) {
			fprintf(stderr, MSGSTR(SECMSG_4, "%s: can't allocate SL buffer\n"),
				command_name);
			catclose(ex_catd);
			exit(1);
		}
	}

	/*
	 * Set our process level to match the file we are preserving,
	 * then drop multileveldir privilege and create the file.
	 */

	if (statslabel(name, sl) < 0) {
		fprintf(stderr, MSGSTR(SECMSG_5, "%s: can't get SL of %s\n"), command_name, name);
		return -1;
	}
	if (setslabel(sl) < 0) {
		fprintf(stderr, MSGSTR(SECMSG_6, "%s: can't change SL to process %s\n"),
			command_name, name);
		return -1;
	}

#if SEC_ILB
	/*
	 * Reset our information label to syslo to allow it to float
	 * up to the level of the file being saved.
	 */
	setilabel(mand_syslo);
#endif

	disablepriv(SEC_MULTILEVELDIR);
	ret = creat(pattern, 0600);
	if (ret >= 0)
		chown(pattern, uid, 0);
	forcepriv(SEC_MULTILEVELDIR);

	return ret;
}

/*
 * Unlink the saved file from the preserve directory in case of failure
 */

expreserve_unlink(pattern)
	char	*pattern;
{
	disablepriv(SEC_MULTILEVELDIR);
	unlink(pattern);
	forcepriv(SEC_MULTILEVELDIR);
}

/*
 * Create a pipe and spawn a child process whose standard input is
 * the read side of the pipe.  Give the child sufficient attributes
 * to invoke the mail command.  Return the file descriptor of the
 * write side of the pipe.
 */

FILE *
expreserve_popen(cmd)
	char	*cmd;
{
	FILE		*fp;
	int		pfd[2];
	pid_t		pid;
	privvec_t	minpriv;

	if (pipe(pfd) < 0)
		return NULL;

	pid = fork();
	if (pid < 0)
		return NULL;
	if (pid) {	/* parent */
		close(pfd[0]);
		fp = fdopen(pfd[1], "w");
		if (fp == NULL)	
			close(pfd[1]);
		return fp;
	}

	/*
	 * Child process.  Set up the security environment:
	 * luid, clearance, sensitivity level, base privs
	 */
	setluid(0);
	setclrnce(mand_syshi);
	setslabel(mand_syslo);
	setprivvec(minpriv, SEC_EXECSUID, -1);
	setpriv(SEC_BASE_PRIV, minpriv);

	/* Connect the pipe to our stdin */
	close(pfd[1]);
	close(0);
	dup(pfd[0]);
	close(pfd[0]);

	(void) execlp("sh", "sh", "-c", cmd, (char *)0);
	catclose(ex_catd);
	exit(1);
}

/*
 * Close the pipe to the child process and wait for it to exit
 */

expreserve_pclose(fp)
	FILE	*fp;
{
	fclose(fp);
	while (wait((int *) 0) != -1 || errno != ECHILD)
		;
}

#endif
