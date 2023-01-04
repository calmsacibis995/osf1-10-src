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
static char	*sccsid = "@(#)$RCSfile: vipw.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/10/08 16:13:34 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.2
*/
/* @(#)vipw.c	5.2 17:33:32 8/20/90 SecureWare
static char sccsid[] = "@(#)vipw.c	1.2  com/cmd/oper,3.1, 9/12/89 13:42:29"; */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: vipw
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 * Modification History
 *
 * 91/10/8 dlong - Use /etc/shells for list of valid root shells.
 * 91/11/5 dlong 001 - If hashed database doesn't exist, ask first.
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>
#endif

#include <stdio.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <strings.h>
#include <errno.h>
#include <signal.h>

#ifdef MSG
#include "vipw_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) NLcatgets(catd,MS_VIPW,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#if SEC_BASE
extern priv_t *privvec();
#endif

/*
 * NAME: vipw
 * FUNCTION:  Password file editor with locking.
 */
char	temp[] = "/etc/ptmp";        /* temp files used by vipw and mkpasswd */
char	temp_pag[] = "/etc/ptmp.pag";
char	temp_dir[] = "/etc/ptmp.dir";
char	passwd[] = "/etc/passwd";
char	passwd_pag[] = "/etc/passwd.pag";
char	passwd_dir[] = "/etc/passwd.dir";
char	vi[] = "/usr/bin/vi";
char	ed[] = "/usr/bin/ed";
char	buf[BUFSIZ];
char	*getenv();
extern	int errno;

main(argc, argv)
	char *argv[];
{
	int fd;
	FILE *ft, *fp;
	char *editor;
#if SEC_BASE
	privvec_t saveprivs;
#endif

	(void ) setlocale(LC_ALL,"");
#ifdef MSG
	catd = catopen(MF_VIPW,NL_CAT_LOCALE);
#endif
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("auth")) {
		fprintf(stderr, "vipw: need auth authorization\n");
		exit(1);
	}

	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_CHOWN, SEC_LIMIT,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, "vipw: insufficient privileges\n");
		exit(1);
	}
#endif
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	setbuf(stderr, NULL);
#if SEC_BASE
	if (create_file_securely(temp, AUTH_VERBOSE, 0) != CFS_GOOD_RETURN) {
		fprintf(stderr,MSGSTR(BUSY,"vipw: password file busy\n"));
		exit(1);
	}
	if ((fd = open(temp, O_WRONLY)) < 0) {
		fprintf(stderr, "vipw: ");
		perror(temp);
		exit(1);
	}
#else /* !SEC_BASE */
	umask(0);
	fd = open(temp, O_WRONLY|O_CREAT|O_EXCL, 0644);
	if (fd < 0) {
		if (errno == EEXIST) {
			fprintf(stderr,MSGSTR(BUSY,"vipw: password file busy\n"));
			exit(1);
		}
		fprintf(stderr,MSGSTR(VIPW, "vipw: ")); perror(temp);
		exit(1);
	}
#endif /* !SEC_BASE */
	ft = fdopen(fd, "w");
	if (ft == NULL) {
		fprintf(stderr,MSGSTR(VIPW,"vipw: ")); perror(temp);
		bad();
	}
	fp = fopen(passwd, "r");
	if (fp == NULL) {
		fprintf(stderr,MSGSTR(VIPW,"vipw: ")); perror(passwd);
		bad();
	}
	while (fgets(buf, sizeof (buf) - 1, fp) != NULL)
		fputs(buf, ft);
	fclose(ft); fclose(fp);
	editor = getenv("EDITOR");
	if (editor == 0) {
		editor = vi;
		if (access(vi,X_OK) == -1) {
			editor = ed;
			fprintf(stdout,MSGSTR(EDMSG,"Unable to find vi editor using ed.\n"));
			fprintf(stdout,MSGSTR(EDMSG2,"Enter q to exit ed\n"));
		}
	}
	sprintf(buf, "%s %s", editor, temp);
	if (system(buf) == 0) {
		struct stat sbuf;
		int ok;

		/* sanity checks */
		if (stat(temp, &sbuf) < 0) {
			fprintf(stderr,MSGSTR(STAT,
			    "vipw: can't stat temp file, %s unchanged\n"),
			    passwd);
			bad();
		}
		if (sbuf.st_size == 0) {
			fprintf(stderr, MSGSTR(BADTEMP,
				"vipw: bad temp file, %s unchanged\n"), passwd);
			bad();
		}
		ft = fopen(temp, "r");
		if (ft == NULL) {
			fprintf(stderr,MSGSTR(REOPEN,
			    "vipw: can't reopen temp file, %s unchanged\n"),
			    passwd);
			bad();
		}
		ok = 0;
		while (fgets(buf, sizeof (buf) - 1, ft) != NULL) {
			register char *cp;

			cp = index(buf, '\n');
			if (cp == 0)
				continue;
			*cp = '\0';
			cp = index(buf, ':');
			if (cp == 0)
				continue;
			*cp = '\0';
			if (strcmp(buf, "root"))
				continue;
			/* password */
			cp = index(cp + 1, ':');
			if (cp == 0)
				break;
			/* uid */
			if (atoi(cp + 1) != 0)
				break;
			cp = index(cp + 1, ':');
			if (cp == 0)
				break;
			/* gid */
			cp = index(cp + 1, ':');
			if (cp == 0)
				break;
			/* gecos */
			cp = index(cp + 1, ':');
			if (cp == 0)
				break;
			/* login directory */
			if (strncmp(++cp, "/:", 2))
				break;
			cp += 2;
			if (*cp && strcmp(cp, "/sbin/sh") ) {
			FILE *shells;
			char shell[PATH_MAX], *s, *sp;

			if(shells=fopen("/etc/shells", "r")) {
				while(s=fgets(shell, sizeof shell, shells)) {
					if((sp=index(s, '\n')))
						*sp = '\0';
					if(strcmp(shell, cp)) {
						if(sp=rindex(shell, '/'))
							if(!strcmp(++sp, cp))
								break;
					} else
						break;
				}
				fclose(shells);
				if(!s) {
					fprintf(stderr,
					    MSGSTR(BADSHELL, "vipw: %s entry has an invalid shell\n"),
					     buf);
					bad();
					exit(1);
				}
			} else {
				fputs(MSGSTR(NOSHELLS, "vipw: Warning, unable to open /etc/shells\n"), stderr);
				if(strcmp(cp, "/bin/sh")  &&
				    strcmp(cp, "/bin/csh") &&
				    strcmp(cp, "/bin/bsh")) {
					fprintf(stderr,
					    MSGSTR(BADSHELL, "vipw: %s entry has an invalid shell\n"),
					     buf);
					bad();
					exit(1);
				}
			} }
			ok++;
		}
		fclose(ft);
		if (ok) {
			struct stat sbuf;	/* 001 begin */

			if(stat(passwd_pag, &sbuf) < 0 && stat(passwd_dir, &sbuf) < 0) {
				char answer[20], *s;

				fputs(MSGSTR(ASKMKPW, "Hashed passwd database does not exist, create? [yes]> "), stdout);
				if(fgets(answer, sizeof answer, stdin)) {
					if(s=strrchr(answer, '\n'))
						*s = '\0';
					if(*answer && strncmp(answer, MSGSTR(YES, "yes"), strlen(answer)))
						if (rename(temp, passwd) < 0) {
							fputs(MSGSTR(VIPW, "vipw: "), stderr);
							perror("rename");
							bad();
							exit(1);
						} else
							exit(0);
				}
			} /* 001 end */
			if (makedb(temp) < 0)
				fprintf(stderr, MSGSTR(MKPASS,"vipw: mkpasswd failed\n"));
			else if (rename(temp_pag, passwd_pag) < 0)
				fprintf(stderr, MSGSTR(VIPW,"vipw: ")), perror(temp_pag);
			else if (rename(temp_dir, passwd_dir) < 0)
				fprintf(stderr,MSGSTR(VIPW, "vipw: ")), perror(temp_dir);
			else if (rename(temp, passwd) < 0)
				fprintf(stderr,MSGSTR(VIPW, "vipw: ")), perror("rename");
			else
				exit(0);
		} else
			fprintf(stderr,MSGSTR(MANGLED,
			    "vipw: you mangled the temp file, %s unchanged\n"),
			    passwd);
	}
	bad();
}

/*
 * NAME: bad
 * FUNCTION: cleanup and exit
 */
bad()
{
	unlink(temp_pag);
	unlink(temp_dir);
	unlink(temp);
	exit(1);
}

/*
 * NAME: makedb
 * FUNCTION: make database
 */
makedb(file)
	char *file;
{
	int status, pid, w;

	if ((pid = vfork()) == 0) {
		execl("/usr/sbin/mkpasswd", "mkpasswd", file, 0);
		exit(1);
	}
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1 || status != 0)
		status = -1;
	return(status);
}
