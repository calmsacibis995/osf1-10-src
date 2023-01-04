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
static char	*sccsid = "@(#)$RCSfile: repquota.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/12/09 20:43:02 $";
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
 * Quota report
 */
#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

#include <sys/param.h>
#include <sys/stat.h>
#include <ufs/quota.h>
#include <fstab.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <errno.h>

#ifdef	NLS
#include <locale.h>
#endif

#ifdef MSG
#include "repquota_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_REPQUOTA,n,s)
#ifdef SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_REPQUOTA_SEC,n,s)
#endif
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

struct fileusage {
	struct	fileusage *fu_next;
	struct	dqblk fu_dqblk;
	long	fu_id;
	char	fu_name[1];
	/* actually bigger */
};
#define FUHASH 1024	/* must be power of two */
struct fileusage *fuhead[MAXQUOTAS][FUHASH];
struct fileusage *lookup();
struct fileusage *addid();
long highid[MAXQUOTAS];	/* highest addid()'ed identifier per type */

int	vflag;			/* verbose */
int	aflag;			/* all file systems */

main(argc, argv)
	int argc;
	char **argv;
{
	register struct fstab *fs;
	register struct passwd *pw;
	register struct group *gr;
	int gflag = 0, uflag = 0, errs = 0;
	long i, argnum, done = 0;
	extern char *optarg;
	extern int optind;
	char ch, *qfnp;

#if	NLS
	setlocale( LC_ALL, "" );
#endif

#ifdef	MSG
	catd = catopen(MF_REPQUOTA,NL_CAT_LOCALE);
#endif
#if	SEC_BASE
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
		fprintf(stderr, MSGSTR(SUSER, "Must be root to execute repquota\n"));
		exit(1);
	}
#endif

	while ((ch = getopt(argc, argv, "aguv")) != EOF) {
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
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc == 0 && !aflag)
		usage();
	if (argc != 0 && aflag)
		usage();
	if (!gflag && !uflag) {
		if (aflag)
			gflag++;
		uflag++;
	}
	if (gflag) {
		setgrent();
		while ((gr = getgrent()) != 0)
			(void) addid(gr->gr_gid, GRPQUOTA, gr->gr_name);
		endgrent();
	}
	if (uflag) {
		setpwent();
		while ((pw = getpwent()) != 0)
			(void) addid(pw->pw_uid, USRQUOTA, pw->pw_name);
		endpwent();
	}
	setfsent();
	while ((fs = getfsent()) != NULL) {
		if (strcmp(fs->fs_vfstype, "ufs"))
			continue;
		if (aflag) {
			if (gflag && hasquota(fs, GRPQUOTA, &qfnp))
				errs += repquota(fs, GRPQUOTA, qfnp);
			if (uflag && hasquota(fs, USRQUOTA, &qfnp))
				errs += repquota(fs, USRQUOTA, qfnp);
			continue;
		}
		if ((argnum = oneof(fs->fs_file, argv, argc)) >= 0 ||
		    (argnum = oneof(fs->fs_spec, argv, argc)) >= 0) {
			done |= 1 << argnum;
			if (gflag && hasquota(fs, GRPQUOTA, &qfnp))
				errs += repquota(fs, GRPQUOTA, qfnp);
			if (uflag && hasquota(fs, USRQUOTA, &qfnp))
				errs += repquota(fs, USRQUOTA, qfnp);
		}
	}
	endfsent();
	for (i = 0; i < argc; i++)
		if ((done & (1 << i)) == 0) {
			fprintf(stderr, MSGSTR(NOFSTAB, "%s not found or not properly enabled for quotas in fstab\n"), argv[i]);
			errs++;
		}
	exit(errs);
}

usage()
{
	fprintf(stderr, "Usage:\n\t%s\n\t%s\n",
		MSGSTR(USE1, "repquota [-v] [-g] [-u] -a"),
		MSGSTR(USE2, "repquota [-v] [-g] [-u] filesys ..."));
	exit(1);
}

repquota(fs, type, qfpathname)
	register struct fstab *fs;
	int type;
	char *qfpathname;
{
	register struct fileusage *fup;
	FILE *qf;
	long id;
	struct dqblk dqbuf;
	char *timeprt();
	static struct dqblk zerodqblk;
	static int warned = 0;
	static int multiple = 0;
	extern int errno;
#if	SEC_BASE
	privvec_t saveprivs;
#endif

#if SEC_BASE
        if (forcepriv(privvec(SEC_ACCT, -1), saveprivs)) {
                fprintf(stderr,
                        MSGSTR_SEC(PRIV, "%s: insufficient privileges\n"),
                        command_name);
                exit(1);
        }
#endif /* SEC_BASE */
	if (quotactl(fs->fs_file, QCMD(Q_SYNC, type), 0, 0) < 0 &&
	    errno == EOPNOTSUPP && !warned && vflag) {
		warned++;
		fprintf(stdout,
		    MSGSTR(NOQUOTA, "*** Warning: Quotas are not compiled into this kernel\n"));
	}
#if SEC_BASE
        seteffprivs(saveprivs, (priv_t *) 0);
#endif
	if (multiple++)
		printf("\n");
	if (vflag)
		fprintf(stdout, MSGSTR(REPORT, "*** Report for %s quotas on %s (%s)\n"),
		    qfextension[type], fs->fs_file, fs->fs_spec);
#if SEC_BASE
        if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
                                SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
                                SEC_ALLOWNCAVACCESS,
#endif
                                -1), saveprivs)) {
                fprintf(stderr,
                        MSGSTR(PRIV, "%s: insufficient privileges\n"),
                        command_name);
                exit(1);
        }
        qf = fopen(qfpathname, "r");
        seteffprivs(saveprivs, (priv_t *) 0);
#else /* !SEC_BASE */
	qf = fopen(qfpathname, "r");
#endif
	if (qf == NULL) {
		perror(qfpathname);
		return (1);
	}
	for (id = 0; ; id++) {
		fread(&dqbuf, sizeof(struct dqblk), 1, qf);
		if (feof(qf))
			break;
		if (dqbuf.dqb_curinodes == 0 && dqbuf.dqb_curblocks == 0)
			continue;
		if ((fup = lookup(id, type)) == 0)
			fup = addid(id, type, (char *)0);
		fup->fu_dqblk = dqbuf;
	}
	fclose(qf);
	printf(MSGSTR(LIMITS, "                        Block limits               File limits\n"));
	if (type == USRQUOTA)
		printf(MSGSTR(MSGHDU, "User            used    soft    hard  grace    used  soft  hard  grace\n"));
	else if (type == GRPQUOTA)
		printf(MSGSTR(MSGHDG, "Group           used    soft    hard  grace    used  soft  hard  grace\n"));
	for (id = 0; id <= highid[type]; id++) {
		fup = lookup(id, type);
		if (fup == 0)
			continue;
		if (fup->fu_dqblk.dqb_curinodes == 0 &&
		    fup->fu_dqblk.dqb_curblocks == 0)
			continue;
		printf("%-10s", fup->fu_name);
		printf("%c%c%8d%8d%8d%7s",
			fup->fu_dqblk.dqb_bsoftlimit && 
			    fup->fu_dqblk.dqb_curblocks >= 
			    fup->fu_dqblk.dqb_bsoftlimit ? '+' : '-',
			fup->fu_dqblk.dqb_isoftlimit &&
			    fup->fu_dqblk.dqb_curinodes >=
			    fup->fu_dqblk.dqb_isoftlimit ? '+' : '-',
			dbtob(fup->fu_dqblk.dqb_curblocks) / 1024,
			dbtob(fup->fu_dqblk.dqb_bsoftlimit) / 1024,
			dbtob(fup->fu_dqblk.dqb_bhardlimit) / 1024,
			fup->fu_dqblk.dqb_bsoftlimit && 
			    fup->fu_dqblk.dqb_curblocks >= 
			    fup->fu_dqblk.dqb_bsoftlimit ?
			    timeprt(fup->fu_dqblk.dqb_btime) : "");
		printf("  %6d%6d%6d%7s\n",
			fup->fu_dqblk.dqb_curinodes,
			fup->fu_dqblk.dqb_isoftlimit,
			fup->fu_dqblk.dqb_ihardlimit,
			fup->fu_dqblk.dqb_isoftlimit &&
			    fup->fu_dqblk.dqb_curinodes >=
			    fup->fu_dqblk.dqb_isoftlimit ?
			    timeprt(fup->fu_dqblk.dqb_itime) : "");
		fup->fu_dqblk = zerodqblk;
	}
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
 * Routines to manage the file usage table.
 *
 * Lookup an id of a specific type.
 */
struct fileusage *
lookup(id, type)
	long id;
	int type;
{
	register struct fileusage *fup;

	for (fup = fuhead[type][id & (FUHASH-1)]; fup != 0; fup = fup->fu_next)
		if (fup->fu_id == id)
			return (fup);
	return ((struct fileusage *)0);
}

/*
 * Add a new file usage id if it does not already exist.
 */
struct fileusage *
addid(id, type, name)
	long id;
	int type;
	char *name;
{
	struct fileusage *fup, **fhp;
	int len;
	extern char *calloc();

	if (fup = lookup(id, type))
		return (fup);
	if (name)
		len = strlen(name);
	else
		len = 10;
	if ((fup = (struct fileusage *)calloc(1, sizeof(*fup) + len)) == NULL) {
		fprintf(stderr, "out of memory for fileusage structures\n");
		exit(1);
	}
	fhp = &fuhead[type][id & (FUHASH - 1)];
	fup->fu_next = *fhp;
	*fhp = fup;
	fup->fu_id = id;
	if (id > highid[type])
		highid[type] = id;
	if (name) {
		bcopy(name, fup->fu_name, len + 1);
	} else {
		sprintf(fup->fu_name, "%u", id);
	}
	return (fup);
}

/*
 * Calculate the grace period and return a printable string for it.
 */
char *
timeprt(seconds)
	time_t seconds;
{
	time_t hours, minutes;
	static char buf[20];
	static time_t now;

	if (now == 0)
		time(&now);
	if (now > seconds)
		return ("none");
	seconds -= now;
	minutes = (seconds + 30) / 60;
	hours = (minutes + 30) / 60;
	if (hours >= 36) {
		sprintf(buf, MSGSTR(DAYS, "%ddays"), (hours + 12) / 24);
		return (buf);
	}
	if (minutes >= 60) {
		sprintf(buf, "%2d:%d", minutes / 60, minutes % 60);
		return (buf);
	}
	sprintf(buf, "%2d", minutes);
	return (buf);
}
