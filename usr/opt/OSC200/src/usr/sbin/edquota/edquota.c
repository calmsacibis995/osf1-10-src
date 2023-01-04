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
static char	*sccsid = "@(#)$RCSfile: edquota.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1993/10/07 23:08:33 $";
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
 * Copyright (c) 1980, 1990 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Robert Elz at The University of Melbourne.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1990 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */


/*
 * Disk quota editor.
 */
#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <ufs/quota.h>
#include <errno.h>
#include <fstab.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "pathnames.h"
#include <locale.h>
#include <nl_types.h>
#include "edquota_msg.h"

nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_EDQUOTA,n,s)
#ifdef	SEC_BASE
#define	MSGSTR_SEC(n,s)	catgets(catd,MS_EDQUOTA_SEC,n,s)
#endif

/*
#if	defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif
*/

#if	SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
privvec_t saveprivs;
#endif

char tmpfil[] = _PATH_TMP;

struct quotause {
	struct	quotause *next;
	long	flags;
	struct	dqblk dqblk;
	char	fsname[MAXPATHLEN + 1];
	char	qfname[1];	/* actually longer */
} *getprivs();
#define	FOUND	0x01

main(argc, argv)
	register char **argv;
	int argc;
{
	register struct quotause *qup, *protoprivs, *curprivs;
	extern char *optarg;
	extern int optind;
	register long id, protoid;
	register int quotatype, tmpfd;
	char *protoname, ch;
	int tflag = 0, pflag = 0;

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_EDQUOTA,NL_CAT_LOCALE);

#if	SEC_BASE
        set_auth_parameters(argc, argv);
        initprivs();

        if (!authorized_user("sysadmin")) {
            fprintf(stderr, MSGSTR_SEC(AUTH,
                "%s: need sysadmin authorization\n"), command_name);
            exit(1);
        }
        if (forceprivs(privvec(SEC_LIMIT, SEC_ACCT, SEC_ALLOWDACACCESS,
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
                fprintf(stderr,
                        MSGSTR_SEC(PRIV, "%s: insufficient privileges\n"),
                        command_name);
                exit(1);
        }
#if SEC_MAC
        disablepriv(SEC_MULTILEVELDIR);
#endif	/* SEC_MAC */
#endif	/* SEC_BASE */
	if (argc < 2)
		usage();
#if	!SEC_BASE
	if (getuid()) {
		fprintf(stderr, MSGSTR(PERDEN, "edquota: permission denied\n"));
		exit(1);
	}
#endif	
	quotatype = USRQUOTA;
	while ((ch = getopt(argc, argv, "ugtp:")) != EOF) {
		switch(ch) {
		case 'p':
			protoname = optarg;
			pflag++;
			break;
		case 'g':
			quotatype = GRPQUOTA;
			break;
		case 'u':
			quotatype = USRQUOTA;
			break;
		case 't':
			tflag++;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (pflag) {
		if ((protoid = getentry(protoname, quotatype)) == -1)
			exit(1);
		protoprivs = getprivs(protoid, quotatype);
		for (qup = protoprivs; qup; qup = qup->next) {
			qup->dqblk.dqb_btime = 0;
			qup->dqblk.dqb_itime = 0;
		}
		while (argc-- > 0) {
			if ((id = getentry(*argv++, quotatype)) < 0)
				continue;
			putprivs(id, quotatype, protoprivs);
		}
		exit(0);
	}
	tmpfd = mkstemp(tmpfil);
	fchown(tmpfd, getuid(), getgid());
	if (tflag) {
		protoprivs = getprivs(0, quotatype);
		if (writetimes(protoprivs, tmpfd, quotatype) == 0)
			exit(1);
		if (editit(tmpfil) && readtimes(protoprivs, tmpfd))
			putprivs(0, quotatype, protoprivs);
		freeprivs(protoprivs);
		exit(0);
	}
	for ( ; argc > 0; argc--, argv++) {
		if ((id = getentry(*argv, quotatype)) == -1)
			continue;
		curprivs = getprivs(id, quotatype);
		if (writeprivs(curprivs, tmpfd, *argv, quotatype) == 0)
			continue;
		if (editit(tmpfil) && readprivs(curprivs, tmpfd))
			putprivs(id, quotatype, curprivs);
		freeprivs(curprivs);
	}
	close(tmpfd);
	unlink(tmpfil);
	exit(0);
}

usage()
{
	fprintf(stderr, "%s%s%s",
		MSGSTR(USE1, "Usage: edquota [-u] [-p username] username ...\n"),
		MSGSTR(USE2, "\tedquota -g [-p groupname] groupname ...\n"),
		MSGSTR(USE3, "\tedquota [-u] -t\n\tedquota -g -t\n"));
	exit(1);
}

/*
 * This routine converts a name for a particular quota type to
 * an identifier. This routine must agree with the kernel routine
 * getinoquota as to the interpretation of quota types.
 */
getentry(name, quotatype)
	char *name;
	int quotatype;
{
	struct passwd *pw;
	struct group *gr;

	if (alldigits(name))
		return (atoi(name));
	switch(quotatype) {
	case USRQUOTA:
		if (pw = getpwnam(name))
			return (pw->pw_uid);
		fprintf(stderr, MSGSTR(NOUSER, "%s: no such user\n"), name);
		break;
	case GRPQUOTA:
		if (gr = getgrnam(name))
			return (gr->gr_gid);
		fprintf(stderr, MSGSTR(NOGRP, "%s: no such group\n"), name);
		break;
	default:
		fprintf(stderr, MSGSTR(UNKTYPE, "%d: unknown quota type\n"),
			quotatype);
		break;
	}
	sleep(1);
	return (-1);
}

/*
 * Collect the requested quota information.
 */
struct quotause *
getprivs(id, quotatype)
	register long id;
	int quotatype;
{
	register struct fstab *fs;
	register struct quotause *qup, *quptail;
	struct quotause *quphead;
	int qcmd, qupsize, fd;
	char *qfpathname;
	static int warned = 0;
	extern int errno;

	setfsent();
	quphead = (struct quotause *)0;
	qcmd = QCMD(Q_GETQUOTA, quotatype);
	while (fs = getfsent()) {
		if (strcmp(fs->fs_vfstype, "ufs"))
			continue;
		if (!hasquota(fs, quotatype, &qfpathname))
			continue;
		qupsize = sizeof(*qup) + strlen(qfpathname);
		if ((qup = (struct quotause *)malloc(qupsize)) == NULL) {
			fprintf(stderr, MSGSTR(NOMEM, "edquota: out of memory\n"));
			exit(2);
		}
		if (quotactl(fs->fs_file, qcmd, id, &qup->dqblk) != 0) {
	    		if (errno == EOPNOTSUPP && !warned) {
				warned++;
				fprintf(stderr, MSGSTR(WARN, "Warning: %s\n"),
				    MSGSTR(NOQUOTAS, "Quotas are not compiled into this kernel"));
				sleep(3);
			}
			if ((fd = open(qfpathname, O_RDONLY)) < 0) {
				fd = open(qfpathname, O_RDWR|O_CREAT, 0640);
				if (fd < 0 && errno != ENOENT) {
					perror(qfpathname);
					free(qup);
					continue;
				}
				fprintf(stderr, MSGSTR(CREATEQ, "Creating quota file %s\n"),
				    qfpathname);
				sleep(3);
				(void) fchown(fd, getuid(),
				    getentry(quotagroup, GRPQUOTA));
				(void) fchmod(fd, 0640);
			}
			lseek(fd, ((off_t)id * sizeof(struct dqblk)), L_SET);
			switch (read(fd, &qup->dqblk, sizeof(struct dqblk))) {
			case 0:			/* EOF */
				/*
				 * Convert implicit 0 quota (EOF)
				 * into an explicit one (zero'ed dqblk)
				 */
				bzero((caddr_t)&qup->dqblk,
				    sizeof(struct dqblk));
				break;

			case sizeof(struct dqblk):	/* OK */
				break;

			default:		/* ERROR */
				fprintf(stderr, MSGSTR(RDERR, "edquota: read error in "));
				perror(qfpathname);
				close(fd);
				free(qup);
				continue;
			}
			close(fd);
		}
		strcpy(qup->qfname, qfpathname);
		strcpy(qup->fsname, fs->fs_file);
		if (quphead == NULL)
			quphead = qup;
		else
			quptail->next = qup;
		quptail = qup;
		qup->next = 0;
	}
	endfsent();
	return (quphead);
}

/*
 * Store the requested quota information.
 */
putprivs(id, quotatype, quplist)
	long id;
	int quotatype;
	struct quotause *quplist;
{
	register struct quotause *qup;
	int qcmd, fd;

	qcmd = QCMD(Q_SETQUOTA, quotatype);
	for (qup = quplist; qup; qup = qup->next) {
		if (quotactl(qup->fsname, qcmd, id, &qup->dqblk) == 0)
			continue;
		if ((fd = open(qup->qfname, O_WRONLY)) < 0) {
			perror(qup->qfname);
		} else {
			lseek(fd, (off_t)id * sizeof (struct dqblk), 0);
			if (write(fd, &qup->dqblk, sizeof (struct dqblk)) !=
			    sizeof (struct dqblk)) {
				fprintf(stderr, MSGSTR(EDQ, "edquota: "));
				perror(qup->qfname);
			}
			close(fd);
		}
	}
}

/*
 * Take a list of priviledges and get it edited.
 */
editit(tmpfile)
	char *tmpfile;
{
	long omask;
	int pid, stat;
	extern char *getenv();

	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
 top:
	if ((pid = fork()) < 0) {
		extern errno;

		if (errno == EPROCLIM) {
			fprintf(stderr, MSGSTR(PROC, "You have too many processes\n"));
			return(0);
		}
		if (errno == EAGAIN) {
			sleep(1);
			goto top;
		}
		perror(MSGSTR(FORK, "fork"));
		return (0);
	}
	if (pid == 0) {
		register char *ed;

		sigsetmask(omask);
#if	SEC_BASE
		seteffprivs(saveprivs, (priv_t *) 0);
#endif
		setgid(getgid());
		setuid(getuid());
		if ((ed = getenv("EDITOR")) == (char *)0)
			ed = _PATH_VI;
		execlp(ed, ed, tmpfile, 0);
		perror(ed);
		exit(1);
	}
	waitpid(pid, &stat, 0);
	sigsetmask(omask);
	if (!WIFEXITED(stat) || WEXITSTATUS(stat) != 0)
		return (0);
	return (1);
}

/*
 * Convert a quotause list to an ASCII file.
 */
writeprivs(quplist, outfd, name, quotatype)
	struct quotause *quplist;
	int outfd;
	char *name;
	int quotatype;
{
	register struct quotause *qup;
	FILE *fd;

	ftruncate(outfd, 0);
	lseek(outfd, 0, L_SET);
	if ((fd = fdopen(dup(outfd), "w")) == NULL) {
		fprintf(stderr, MSGSTR(EDQ, "edquota: "));
		perror(tmpfil);
		exit(1);
	}
	fprintf(fd, MSGSTR(QUOTAS, "Quotas for %s %s:\n"),
		qfextension[quotatype], name);
	for (qup = quplist; qup; qup = qup->next) {
		fprintf(fd, MSGSTR(QLIM1, "%s: %s %d, limits (soft = %d, hard = %d)\n"),
		    qup->fsname, MSGSTR(BUSE, "blocks in use:"),
		    dbtob(qup->dqblk.dqb_curblocks) / 1024,
		    dbtob(qup->dqblk.dqb_bsoftlimit) / 1024,
		    dbtob(qup->dqblk.dqb_bhardlimit) / 1024);
		fprintf(fd, MSGSTR(QLIM2, "%s %d, limits (soft = %d, hard = %d)\n"),
		    MSGSTR(IUSE, "\tinodes in use:"), qup->dqblk.dqb_curinodes,
		    qup->dqblk.dqb_isoftlimit, qup->dqblk.dqb_ihardlimit);
	}
	fclose(fd);
	return (1);
}

/*
 * Merge changes to an ASCII file into a quotause list.
 */
readprivs(quplist, infd)
	struct quotause *quplist;
	int infd;
{
	register struct quotause *qup;
	FILE *fd;
	int cnt;
	register char *cp;
	struct dqblk dqblk;
	char *fsp, line1[BUFSIZ], line2[BUFSIZ];

	lseek(infd, 0, L_SET);
	fd = fdopen(dup(infd), "r");
	if (fd == NULL) {
		fprintf(stderr, MSGSTR(TMP, "Can't re-read temp file!!\n"));
		return (0);
	}
	/*
	 * Discard title line, then read pairs of lines to process.
	 */
	(void) fgets(line1, sizeof (line1), fd);
	while (fgets(line1, sizeof (line1), fd) != NULL &&
	       fgets(line2, sizeof (line2), fd) != NULL) {
		if ((fsp = strtok(line1, " \t:")) == NULL) {
			fprintf(stderr, MSGSTR(FORM1, "%s: bad format\n"), line1);
			return (0);
		}
		if ((cp = strtok((char *)0, "\n")) == NULL) {
			fprintf(stderr, MSGSTR(FORM2, "%s: %s: bad format\n"), fsp,
			    &fsp[strlen(fsp) + 1]);
			return (0);
		}
		cnt = sscanf(cp,
		    MSGSTR(BLKUSE, " blocks in use: %d, limits (soft = %d, hard = %d)"),
		    &dqblk.dqb_curblocks, &dqblk.dqb_bsoftlimit,
		    &dqblk.dqb_bhardlimit);
		if (cnt != 3) {
			fprintf(stderr, MSGSTR(FORM2, "%s: %s: bad format\n"), fsp, cp);
			return (0);
		}
		dqblk.dqb_curblocks = btodb(dqblk.dqb_curblocks * 1024);
		dqblk.dqb_bsoftlimit = btodb(dqblk.dqb_bsoftlimit * 1024);
		dqblk.dqb_bhardlimit = btodb(dqblk.dqb_bhardlimit * 1024);
		if ((cp = strtok(line2, "\n")) == NULL) {
			fprintf(stderr, MSGSTR(FORM2, "%s: %s: bad format\n"), fsp, line2);
			return (0);
		}
		cnt = sscanf(cp,
		    MSGSTR(INOUSE, "\tinodes in use: %d, limits (soft = %d, hard = %d)"),
		    &dqblk.dqb_curinodes, &dqblk.dqb_isoftlimit,
		    &dqblk.dqb_ihardlimit);
		if (cnt != 3) {
			fprintf(stderr, MSGSTR(FORM2, "%s: %s: bad format\n"), fsp, line2);
			return (0);
		}
		for (qup = quplist; qup; qup = qup->next) {
			if (strcmp(fsp, qup->fsname))
				continue;
			/*
			 * Cause time limit to be reset when the quota
			 * is next used if previously had no soft limit
			 * or were under it, but now have a soft limit
			 * and are over it.
			 */
			if (dqblk.dqb_bsoftlimit &&
			    qup->dqblk.dqb_curblocks >= dqblk.dqb_bsoftlimit &&
			    (qup->dqblk.dqb_bsoftlimit == 0 ||
			     qup->dqblk.dqb_curblocks <
			     qup->dqblk.dqb_bsoftlimit))
				qup->dqblk.dqb_btime = 0;
			if (dqblk.dqb_isoftlimit &&
			    qup->dqblk.dqb_curinodes >= dqblk.dqb_isoftlimit &&
			    (qup->dqblk.dqb_isoftlimit == 0 ||
			     qup->dqblk.dqb_curinodes <
			     qup->dqblk.dqb_isoftlimit))
				qup->dqblk.dqb_itime = 0;
			qup->dqblk.dqb_bsoftlimit = dqblk.dqb_bsoftlimit;
			qup->dqblk.dqb_bhardlimit = dqblk.dqb_bhardlimit;
			qup->dqblk.dqb_isoftlimit = dqblk.dqb_isoftlimit;
			qup->dqblk.dqb_ihardlimit = dqblk.dqb_ihardlimit;
			qup->flags |= FOUND;
			if (dqblk.dqb_curblocks == qup->dqblk.dqb_curblocks &&
			    dqblk.dqb_curinodes == qup->dqblk.dqb_curinodes)
				break;
			fprintf(stderr,
			    MSGSTR(ALLOC, "%s: cannot change current allocation\n"), fsp);
			break;
		}
	}
	fclose(fd);
	/*
	 * Disable quotas for any filesystems that have not been found.
	 */
	for (qup = quplist; qup; qup = qup->next) {
		if (qup->flags & FOUND) {
			qup->flags &= ~FOUND;
			continue;
		}
		qup->dqblk.dqb_bsoftlimit = 0;
		qup->dqblk.dqb_bhardlimit = 0;
		qup->dqblk.dqb_isoftlimit = 0;
		qup->dqblk.dqb_ihardlimit = 0;
	}
	return (1);
}

/*
 * Convert a quotause list to an ASCII file of grace times.
 */
writetimes(quplist, outfd, quotatype)
	struct quotause *quplist;
	int outfd;
	int quotatype;
{
	register struct quotause *qup;
	char *cvtstoa();
	FILE *fd;

	ftruncate(outfd, 0);
	lseek(outfd, 0, L_SET);
	if ((fd = fdopen(dup(outfd), "w")) == NULL) {
		fprintf(stderr, MSGSTR(EDQ, "edquota: "));
		perror(tmpfil);
		exit(1);
	}
	fprintf(fd, MSGSTR(TIME, "Time units may be: days, hours, minutes, or seconds\n"));
	fprintf(fd, MSGSTR(GRACE, "Grace period before enforcing soft limits for %ss:\n"),
	    qfextension[quotatype]);
	for (qup = quplist; qup; qup = qup->next) {
		fprintf(fd, MSGSTR(BGRACE, "%s: block grace period: %s, "),
		    qup->fsname, cvtstoa(qup->dqblk.dqb_btime));
		fprintf(fd, MSGSTR(FGRACE, "file grace period: %s\n"),
		    cvtstoa(qup->dqblk.dqb_itime));
	}
	fclose(fd);
	return (1);
}

/*
 * Merge changes of grace times in an ASCII file into a quotause list.
 */
readtimes(quplist, infd)
	struct quotause *quplist;
	int infd;
{
	register struct quotause *qup;
	FILE *fd;
	int cnt;
	register char *cp;
	time_t itime, btime, iseconds, bseconds;
	char *fsp, bunits[10], iunits[10], line1[BUFSIZ];

	lseek(infd, 0, L_SET);
	fd = fdopen(dup(infd), "r");
	if (fd == NULL) {
		fprintf(stderr, MSGSTR(TMP, "Can't re-read temp file!!\n"));
		return (0);
	}
	/*
	 * Discard two title lines, then read lines to process.
	 */
	(void) fgets(line1, sizeof (line1), fd);
	(void) fgets(line1, sizeof (line1), fd);
	while (fgets(line1, sizeof (line1), fd) != NULL) {
		if ((fsp = strtok(line1, " \t:")) == NULL) {
			fprintf(stderr, MSGSTR(FORM1, "%s: bad format\n"), line1);
			return (0);
		}
		if ((cp = strtok((char *)0, "\n")) == NULL) {
			fprintf(stderr, MSGSTR(FORM2, "%s: %s: bad format\n"),
				fsp, &fsp[strlen(fsp) + 1]);
			return (0);
		}
		cnt = sscanf(cp,
		    MSGSTR(BFGRACE, " block grace period: %d %s file grace period: %d %s"),
		    &btime, bunits, &itime, iunits);
		if (cnt != 4) {
			fprintf(stderr, MSGSTR(FORM2, "%s: %s: bad format\n"), fsp, cp);
			return (0);
		}
		if (cvtatos(btime, bunits, &bseconds) == 0)
			return (0);
		if (cvtatos(itime, iunits, &iseconds) == 0)
			return (0);
		for (qup = quplist; qup; qup = qup->next) {
			if (strcmp(fsp, qup->fsname))
				continue;
			qup->dqblk.dqb_btime = bseconds;
			qup->dqblk.dqb_itime = iseconds;
			qup->flags |= FOUND;
			break;
		}
	}
	fclose(fd);
	/*
	 * reset default grace periods for any filesystems
	 * that have not been found.
	 */
	for (qup = quplist; qup; qup = qup->next) {
		if (qup->flags & FOUND) {
			qup->flags &= ~FOUND;
			continue;
		}
		qup->dqblk.dqb_btime = 0;
		qup->dqblk.dqb_itime = 0;
	}
	return (1);
}

/*
 * Convert seconds to ASCII times.
 */
char *
cvtstoa(time)
	time_t time;
{
	static char buf[20];

	if (time % (24 * 60 * 60) == 0) {
		time /= 24 * 60 * 60;
		sprintf(buf, "%d %s", time, time == 1 ? MSGSTR(DAY,"day") : MSGSTR(DAYS,"days"));
	} else if (time % (60 * 60) == 0) {
		time /= 60 * 60;
		sprintf(buf, "%d %s", time, time == 1 ? MSGSTR(HR,"hour") : MSGSTR(HOURS,"hours"));
	} else if (time % 60 == 0) {
		time /= 60;
		sprintf(buf, "%d %s", time, time == 1 ? MSGSTR(MINS,"minute") : MSGSTR(MINUTES,"minutes"));
	} else
		sprintf(buf, "%d %s", time, time == 1 ? MSGSTR(SEC,"second") : MSGSTR(SECONDS,"seconds"));
	return (buf);
}

/*
 * Convert ASCII input times to seconds.
 */
cvtatos(time, units, seconds)
	time_t time;
	char *units;
	time_t *seconds;
{

	if (bcmp(units, MSGSTR(SEC,"second"), 6) == 0)
		*seconds = time;
	else if (bcmp(units, MSGSTR(MINS,"minute"), 6) == 0)
		*seconds = time * 60;
	else if (bcmp(units, MSGSTR(HR,"hour"), 4) == 0)
		*seconds = time * 60 * 60;
	else if (bcmp(units, MSGSTR(DAY,"day"), 3) == 0)
		*seconds = time * 24 * 60 * 60;
	else {
		printf(MSGSTR(BADUNIT,"%s: bad units, specify %s\n"), units,
		    MSGSTR(DHMS, "days, hours, minutes, or seconds"));
		return (0);
	}
	return (1);
}

/*
 * Free a list of quotause structures.
 */
freeprivs(quplist)
	struct quotause *quplist;
{
	register struct quotause *qup, *nextqup;

	for (qup = quplist; qup; qup = nextqup) {
		nextqup = qup->next;
		free(qup);
	}
}

/*
 * Check whether a string is completely composed of digits.
 */
alldigits(s)
	register char *s;
{
	register c;

	c = *s++;
	do {
		if (!isdigit(c))
			return (0);
	} while (c = *s++);
	return (1);
}

/*
 * Check to see if a particular quota is to be enabled.
 */
hasquota(fs, type, qfnamep)
	register struct fstab *fs;
	int type;
	char **qfnamep;
{
	register char *opt;
	char *cp, *strtok();
	static char initname, usrname[100], grpname[100];
	static char buf[BUFSIZ];

	if (!initname) {
		sprintf(usrname, "%s%s", qfextension[USRQUOTA], qfname);
		sprintf(grpname, "%s%s", qfextension[GRPQUOTA], qfname);
		initname = 1;
	}
	strcpy(buf, fs->fs_mntops);
	for (opt = strtok(buf, ","); opt; opt = strtok(NULL, ",")) {
		if (cp = index(opt, '='))
			*cp++ = '\0';
		if (type == USRQUOTA && strcmp(opt, usrname) == 0)
			break;
		if (type == GRPQUOTA && strcmp(opt, grpname) == 0)
			break;
	}
	if (!opt)
		return (0);
	if (cp) {
		*qfnamep = cp;
		return (1);
	}
	(void) sprintf(buf, "%s/%s.%s", fs->fs_file, qfname, qfextension[type]);
	*qfnamep = buf;
	return (1);
}
