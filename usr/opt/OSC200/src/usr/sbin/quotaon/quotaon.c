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
static char	*sccsid = "@(#)$RCSfile: quotaon.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/12/09 20:42:59 $";
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
 * Turn quota on/off for a filesystem.
 */
#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

#include <sys/param.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <ufs/quota.h>
#include <stdio.h>
#include <fstab.h>

#ifdef	NLS
#include <locale.h>
#endif

#ifdef MSG
#include "quotaon_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_QUOTAON,n,s)
#ifdef SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_QUOTAON_SEC,n,s)
#endif
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

int	aflag;		/* all file systems */
int	gflag;		/* operate on group quotas */
int	uflag;		/* operate on user quotas */
int	vflag;		/* verbose */

main(argc, argv)
	int argc;
	char **argv;
{
	register struct fstab *fs;
	char ch, *qfnp, *whoami, *rindex();
	long argnum, done = 0;
	int i, offmode = 0, errs = 0;
	extern char *optarg;
	extern int optind;
#if	SEC_BASE
	privvec_t saveprivs;
#endif

#ifdef	NLS
	setlocale( LC_ALL, "" );
#endif

#ifdef	MSG
	catd = catopen(MF_QUOTAON,NL_CAT_LOCALE);
#endif

#if SEC_BASE
        set_auth_parameters(argc, argv);
        initprivs();

        if (!authorized_user("sysadmin")) {
                fprintf(stderr,
                        MSGSTR_SEC(AUTH, "%s: need sysadmin authorization\n"),
                        command_name);
                exit(1);
        }
#else
	if (getuid()) {
		fprintf(stderr,
			MSGSTR(SUSER, "\Must be root to execute quotaon or quotaoff\n"));
		exit(1);
	}
#endif

	whoami = rindex(*argv, '/') + 1;
	if (whoami == (char *)1)
		whoami = *argv;
	if (strcmp(whoami, "quotaoff") == 0)
		offmode++;
	else if (strcmp(whoami, "quotaon") != 0) {
		fprintf(stderr, MSGSTR(NAME, "Name must be quotaon or quotaoff not %s\n"),
			whoami);
		exit(1);
	}
	while ((ch = getopt(argc, argv, "avug")) != EOF) {
		switch(ch) {
		case 'a':
			aflag++;
			break;
		case 'g':
			gflag++;
			break;
		case 'u':
			uflag++;
			break;
		case 'v':
			vflag++;
			break;
		default:
			usage(whoami);
		}
	}
	argc -= optind;
	argv += optind;
	if (argc <= 0 && !aflag)
		usage(whoami);
	if (argc > 0 && aflag)
		usage(whoami);
	if (!gflag && !uflag) {
		gflag++;
		uflag++;
	}
	setfsent();
	while ((fs = getfsent()) != NULL) {
		if (strcmp(fs->fs_vfstype, "ufs") ||
		    (strcmp(fs->fs_type, FSTAB_RW) &&
		     strcmp(fs->fs_type, FSTAB_RQ)))
			continue;
		if (aflag) {
#if	SEC_BASE
			if (forceprivs(privvec(SEC_ACCT, -1), saveprivs)) {
				fprintf(stderr,
					MSGSTR_SEC(PRIV, "%s: insufficient privileges\n"),
					command_name);
				exit(1);
			}
#endif
			if (gflag && hasquota(fs, GRPQUOTA, &qfnp))
				errs += quotaonoff(fs, offmode, GRPQUOTA, qfnp);
			if (uflag && hasquota(fs, USRQUOTA, &qfnp))
				errs += quotaonoff(fs, offmode, USRQUOTA, qfnp);
#if	SEC_BASE
			seteffprivs(saveprivs, (priv_t *) 0);
#endif

			continue;
		}
		if ((argnum = oneof(fs->fs_file, argv, argc)) >= 0 ||
		    (argnum = oneof(fs->fs_spec, argv, argc)) >= 0) {
			done |= 1 << argnum;
#if	SEC_BASE
			if (forceprivs(privvec(SEC_ACCT, -1), saveprivs)) {
				fprintf(stderr,
					MSGSTR_SEC(PRIV, "%s: insufficient privileges\n"),
					command_name);
				exit(1);
			}
#endif
			if (gflag && hasquota(fs, GRPQUOTA, &qfnp))
				errs += quotaonoff(fs, offmode, GRPQUOTA, qfnp);
			if (uflag && hasquota(fs, USRQUOTA, &qfnp))
				errs += quotaonoff(fs, offmode, USRQUOTA, qfnp);
#if	SEC_BASE
			seteffprivs(saveprivs, (priv_t *) 0);
#endif
		}
	}
	endfsent();
	for (i = 0; i < argc; i++)
		if ((done & (1 << i)) == 0)
			fprintf(stderr, MSGSTR(NOFSTAB, "%s not found in fstab\n"),
				argv[i]);
	exit(errs);
}

usage(whoami)
	char *whoami;
{

	fprintf(stderr, MSGSTR(USE1, "Usage:\n\t%s [-g] [-u] [-v] -a\n"), whoami);
	fprintf(stderr, MSGSTR(USE2, "\t%s [-g] [-u] [-v] filesys ...\n"), whoami);
	exit(1);
}

quotaonoff(fs, offmode, type, qfpathname)
	register struct fstab *fs;
	int offmode, type;
	char *qfpathname;
{

	if (strcmp(fs->fs_file, "/") && readonly(fs))
		return (1);
	if (offmode) {
		if (quotactl(fs->fs_file, QCMD(Q_QUOTAOFF, type), 0, 0) < 0) {
			fprintf(stderr, MSGSTR(QOFF, "quotaoff: "));
			perror(fs->fs_file);
			return (1);
		}
		if (vflag)
			printf(MSGSTR(QTOFF, "%s: quotas turned off\n"), fs->fs_file);
		return (0);
	}
	if (quotactl(fs->fs_file, QCMD(Q_QUOTAON, type), 0, qfpathname) < 0) {
		fprintf(stderr, MSGSTR(QON, "quotaon: using %s on"), qfpathname);
		perror(fs->fs_file);
		return (1);
	}
	if (vflag)
		printf(MSGSTR(QTON, "%s: %s quotas turned on\n"), fs->fs_file,
		    qfextension[type]);
	return (0);
}

/*
 * Check to see if target appears in list of size cnt.
 */
oneof(target, list, cnt)
	register char *target, *list[];
	int cnt;
{
	register int i;

	for (i = 0; i < cnt; i++)
		if (strcmp(target, list[i]) == 0)
			return (i);
	return (-1);
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
	char *cp, *index(), *strtok();
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

/*
 * Verify file system is mounted and not readonly.
 */
readonly(fs)
	register struct fstab *fs;
{
	struct statfs fsbuf;

	if (statfs(fs->fs_file, &fsbuf) < 0 ||
	    strcmp(fsbuf.f_mntonname, fs->fs_file) ||
	    strcmp(fsbuf.f_mntfromname, fs->fs_spec)) {
		printf(MSGSTR(NOMNT, "%s: not mounted\n"), fs->fs_file);
		return (1);
	}
	if (fsbuf.f_flags & M_RDONLY) {
		printf(MSGSTR(RDONLY, "%s: mounted read-only\n"), fs->fs_file);
		return (1);
	}
	return (0);
}
