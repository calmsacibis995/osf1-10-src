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
static char	*sccsid = "@(#)$RCSfile: quota.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/08 15:53:04 $";
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
 * Disk quota reporting program.
 */
#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#include <prot.h>
#endif

#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <ufs/quota.h>
#include <stdio.h>
#include <fstab.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "quota_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_QUOTA,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

/* remote quota rpc related includes */

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <sys/socket.h>
#include <netdb.h>
#include <rpcsvc/rquota.h>

struct quotause {
	struct	quotause *next;
	long	flags;
	struct	dqblk dqblk;
	char	fsname[MAXPATHLEN + 1];
} *getprivs();
#define	FOUND	0x01

int	qflag;
int	vflag;

#if	SEC_BASE
extern priv_t *privvec();
int has_auth;
#endif

main(argc, argv)
	char *argv[];
{
	int ngroups, gidset[NGROUPS];
	int i, gflag = 0, uflag = 0;
	char ch;
	extern char *optarg;
	extern int optind, errno;

#ifdef NLS
	(void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
	catd = catopen(MF_QUOTA,NL_CAT_LOCALE);
#endif

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (forceprivs(privvec(SEC_ACCT, SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}
	has_auth = authorized_user("sysadmin");
#endif
	if (quotactl("/", 0, 0, (caddr_t)0) < 0 && errno == EOPNOTSUPP) {
		fprintf(stderr,MSGSTR(NOQUOTAONSYS,
				      "There are no quotas on this system\n"));
		exit(0);
	}
	while ((ch = getopt(argc, argv, "ugvq")) != EOF) {
		switch(ch) {
		case 'g':
			gflag++;
			break;
		case 'u':
			uflag++;
			break;
		case 'v':
			vflag++;
			break;
		case 'q':
			qflag++;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (!uflag && !gflag)
		uflag++;
	if (argc == 0) {
		if (uflag)
			showuid(getuid());
		if (gflag) {
			ngroups = getgroups(NGROUPS, gidset);
			if (ngroups < 0) {
				perror("quota: getgroups");
				exit(1);
			}
			for (i = 0; i < ngroups; i++)
				showgid(gidset[i]);
		}
		exit(0);
	}
	if (uflag && gflag)
		usage();
	if (uflag) {
		for (; argc > 0; argc--, argv++) {
			if (alldigits(*argv))
				showuid(atoi(*argv));
			else
				showusrname(*argv);
		}
		exit(0);
	}
	if (gflag) {
		for (; argc > 0; argc--, argv++) {
			if (alldigits(*argv))
				showgid(atoi(*argv));
			else
				showgrpname(*argv);
		}
		exit(0);
	}
}

usage()
{

	fprintf(stderr, "%s\n%s\n%s\n",
		MSGSTR(USE1, "Usage: quota [-guqv]"),
		MSGSTR(USE2, "\tquota [-qv] -u username ..."),
		MSGSTR(USE3, "\tquota [-qv] -g groupname ..."));
	exit(1);
}

/*
 * Print out quotas for a specified user identifier.
 */
showuid(uid)
	u_long uid;
{
	struct passwd *pwd = getpwuid(uid);
	u_long myuid;
	char *name;

	if (pwd == NULL)
		name = MSGSTR(NOACC, "(no account)");
	else
		name = pwd->pw_name;
	myuid = getuid();
#if	SEC_BASE
	if (uid != myuid && uid != getluid() && !has_auth)
#else
	if (uid != myuid && myuid != 0) 
#endif
	{
		printf(MSGSTR(PERDENU,
			      "quota: %s (uid %d): permission denied\n"), name, uid);
		return;
	}
	showquotas(USRQUOTA, uid, name);
}

/*
 * Print out quotas for a specifed user name.
 */
showusrname(name)
	char *name;
{
	struct passwd *pwd = getpwnam(name);
	u_long myuid;

	if (pwd == NULL) {
		fprintf(stderr, MSGSTR(UNKUSR,
				       "quota: %s: unknown user\n"), name);
		return;
	}
	myuid = getuid();
	if (pwd->pw_uid != myuid && myuid != 0) {
		fprintf(stderr, MSGSTR(PERDENU,
				"quota: %s (uid %d): permission denied\n"),
		    	name, pwd->pw_uid);
		return;
	}
	showquotas(USRQUOTA, pwd->pw_uid, name);
}

/*
 * Print out quotas for a specified group identifier.
 */
showgid(gid)
	u_long gid;
{
	struct group *grp = getgrgid(gid);
	int ngroups, gidset[NGROUPS];
	register int i;
	char *name;

	if (grp == NULL)
		name = MSGSTR(NOENT, "(no entry)");
	else
		name = grp->gr_name;
	ngroups = getgroups(NGROUPS, gidset);
	if (ngroups < 0) {
		perror(MSGSTR(GETGR, "quota: getgroups"));
		return;
	}
	for (i = 0; i < ngroups; i++)
		if (gid == gidset[i])
			break;
	if (i >= ngroups && getuid() != 0) {
		fprintf(stderr, MSGSTR(PERDENG,
				"quota: %s (gid %d): permission denied\n"),
		    	name, gid);
		return;
	}
	showquotas(GRPQUOTA, gid, name);
}

/*
 * Print out quotas for a specifed group name.
 */
showgrpname(name)
	char *name;
{
	struct group *grp = getgrnam(name);
	int ngroups, gidset[NGROUPS];
	register int i;

	if (grp == NULL) {
		fprintf(stderr, MSGSTR(UNKGRP,
					"quota: %s: unknown group\n"),
			name);
		return;
	}
	ngroups = getgroups(NGROUPS, gidset);
	if (ngroups < 0) {
		perror(MSGSTR(GETGR, "quota: getgroups"));
		return;
	}
	for (i = 0; i < ngroups; i++)
		if (grp->gr_gid == gidset[i])
			break;
	if (i >= ngroups && getuid() != 0) {
		fprintf(stderr, MSGSTR(PERDENG,
				"quota: %s (gid %d): permission denied\n"),
		    name, grp->gr_gid);
		return;
	}
	showquotas(GRPQUOTA, grp->gr_gid, name);
}

showquotas(type, id, name)
	int type;
	u_long id;
	char *name;
{
	register struct quotause *qup;
	struct quotause *quplist, *getprivs();
	char *msgi, *msgb, *timeprt();
	int myuid, fd, lines = 0;
	static int first;
	static time_t now;

	if (now == 0)
		time(&now);
	quplist = getprivs(id, type);
	for (qup = quplist; qup; qup = qup->next) {
		if (!vflag &&
		    qup->dqblk.dqb_isoftlimit == 0 &&
		    qup->dqblk.dqb_ihardlimit == 0 &&
		    qup->dqblk.dqb_bsoftlimit == 0 &&
		    qup->dqblk.dqb_bhardlimit == 0)
			continue;
		msgi = (char *)0;
		if (qup->dqblk.dqb_ihardlimit &&
		    qup->dqblk.dqb_curinodes >= qup->dqblk.dqb_ihardlimit)
			msgi = MSGSTR(FLIMIT, "File limit reached on");
		else if (qup->dqblk.dqb_isoftlimit &&
		    qup->dqblk.dqb_curinodes >= qup->dqblk.dqb_isoftlimit)
			if (qup->dqblk.dqb_itime > now)
				msgi = MSGSTR(FGRACE, "In file grace period on");
			else
				msgi = MSGSTR(FQUOTA, "Over file quota on");
		msgb = (char *)0;
		if (qup->dqblk.dqb_bhardlimit &&
		    qup->dqblk.dqb_curblocks >= qup->dqblk.dqb_bhardlimit)
			msgb = MSGSTR(BLIMIT, "Block limit reached on");
		else if (qup->dqblk.dqb_bsoftlimit &&
		    qup->dqblk.dqb_curblocks >= qup->dqblk.dqb_bsoftlimit)
			if (qup->dqblk.dqb_btime > now)
				msgb = MSGSTR(BGRACE, "In block grace period on");
			else
				msgb = MSGSTR(BQUOTA, "Over block quota on");
		if (qflag) {
			if ((msgi != (char *)0 || msgb != (char *)0) &&
			    lines++ == 0)
				heading(type, id, name, "");
			if (msgi != (char *)0)
				printf("\t%s %s\n", msgi, qup->fsname);
			if (msgb != (char *)0)
				printf("\t%s %s\n", msgb, qup->fsname);
			continue;
		}
		if (vflag ||
		    qup->dqblk.dqb_curblocks ||
		    qup->dqblk.dqb_curinodes) {
			if (lines++ == 0)
				heading(type, id, name, "");
			printf("%15s%8d%c%7d%8d%8s"
				, qup->fsname
				, dbtob(qup->dqblk.dqb_curblocks) / 1024
				, (msgb == (char *)0) ? ' ' : '*'
				, dbtob(qup->dqblk.dqb_bsoftlimit) / 1024
				, dbtob(qup->dqblk.dqb_bhardlimit) / 1024
				, (msgb == (char *)0) ? ""
				    : timeprt(qup->dqblk.dqb_btime));
			printf("%8d%c%7d%8d%8s\n"
				, qup->dqblk.dqb_curinodes
				, (msgi == (char *)0) ? ' ' : '*'
				, qup->dqblk.dqb_isoftlimit
				, qup->dqblk.dqb_ihardlimit
				, (msgi == (char *)0) ? ""
				    : timeprt(qup->dqblk.dqb_itime)
			);
			continue;
		}
	}
	if (!qflag && lines == 0)
		heading(type, id, name, MSGSTR(NONE, "none"));
}

heading(type, id, name, tag)
	int type;
	u_long id;
	char *name, *tag;
{

	printf(MSGSTR(HEAD, "Disk quotas for %s %s (%cid %d): %s\n"),
		qfextension[type], name, *qfextension[type], id, tag);
	if (!qflag && tag[0] == '\0') {
		printf("%15s%8s %7s%8s%8s%8s %7s%8s%8s\n"
			, MSGSTR(MSGFS, "Filesystem")
			, MSGSTR(MSGB, "blocks")
			, MSGSTR(MSGQ, "quota")
			, MSGSTR(MSGL, "limit")
			, MSGSTR(MSGG, "grace")
			, MSGSTR(MSGF, "files")
			, MSGSTR(MSGQ, "quota")
			, MSGSTR(MSGL, "limit")
			, MSGSTR(MSGG, "grace")
		);
	}
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
		return (MSGSTR(NONE, "none"));
	seconds -= now;
	minutes = (seconds + 30) / 60;
	hours = (minutes + 30) / 60;
	if (hours >= 36) {
		sprintf(buf, "%d%s", (hours + 12) / 24, MSGSTR(DAYS,"days"));
		return (buf);
	}
	if (minutes >= 60) {
		sprintf(buf, "%2d:%d", minutes / 60, minutes % 60);
		return (buf);
	}
	sprintf(buf, "%2d", minutes);
	return (buf);
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
	char *qfpathname;
	int qcmd, fd;
	struct dqblk local_dqblk;


	setfsent();
	quphead = (struct quotause *)0;
	qcmd = QCMD(Q_GETQUOTA, quotatype);
	while (fs = getfsent()) {
	  if (!strcmp(fs->fs_vfstype, "nfs")){ /* matches "nfs" */
#ifdef DEBUG
	    printf(" ===============NFS=============================\n");
	    printf(" block special device name: %s\n", fs->fs_spec  );
	    printf(" file system path prefix: %s\n", fs->fs_file  );
	    printf(" Mount tyoe (FSTAB_*): %s\n", fs->fs_type  );
	    printf(" File system type: %s\n", fs->fs_vfstype  );
	    printf(" Mount options: %s\n", fs->fs_mntops  );
	    printf(" ===============================================\n");
#endif DEBUG
	    /* call getnfsquota() that does an rpc to the nfs host */

	      if ((qup = (struct quotause *)malloc((size_t)sizeof *qup)) == NULL) {
	          fprintf(stderr, MSGSTR(NOMEM, "quota: out of memory\n"));
	          exit(2);
	      }
	      if (!getnfsquota( fs, id, &local_dqblk)) 
	         continue;
	      else {
		qup->dqblk = local_dqblk;
	      }
		 
		
	  }
	  else  /* not nfs mounted FS */ {

		if (strcmp(fs->fs_vfstype, "ufs"))
			continue;
		if (!hasquota(fs, quotatype, &qfpathname))
			continue;
		if ((qup = (struct quotause *)malloc((size_t)sizeof *qup)) == NULL) {
			fprintf(stderr, MSGSTR(NOMEM, "quota: out of memory\n"));
			exit(2);
		}
		if (quotactl(fs->fs_file, qcmd, id, &qup->dqblk) != 0) {
			if ((fd = open(qfpathname, O_RDONLY)) < 0) {
				perror(qfpathname);
				free(qup);
				continue;
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
				fprintf(stderr, MSGSTR(RDERR, "quota: read error"));
				perror(qfpathname);
				close(fd);
				free(qup);
				continue;
			      }
			close(fd);
		}
	      }  /* matches else of not nfs mounted FS */

		strcpy(qup->fsname, fs->fs_file);
		if (quphead == NULL)
			quphead = qup;
		else
			quptail->next = qup;
		quptail = qup;
		qup->next = 0;

	} /* matches the while loop for getfsent() */
	endfsent();
	return (quphead);
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
 * nfs quota checking routines ...
 */



int
getnfsquota(mntp, uid, dqp)
	struct fstab *mntp;
	int uid;
	struct dqblk *dqp;
{
	char *hostp;
	char *cp;
	struct getquota_args gq_args;
	struct getquota_rslt gq_rslt;
	extern char *index();

	cp = index(mntp->fs_spec, '@');
	if (cp == 0) {
		fprintf(stderr, "cannot find hostname for %s\n", mntp->fs_spec);
		return (0);
	}
	*cp = '\0';
	hostp = cp + 1;
	gq_args.gqa_pathp = mntp->fs_spec;
	gq_args.gqa_uid = uid;


	if (callaurpc(hostp, RQUOTAPROG, RQUOTAVERS,
	    (vflag? RQUOTAPROC_GETQUOTA: RQUOTAPROC_GETACTIVEQUOTA),
	    xdr_getquota_args, &gq_args, xdr_getquota_rslt, &gq_rslt) != 0) {
		*cp = '@';
		return (0);
	}
	switch (gq_rslt.gqr_status) {
	case Q_OK:
		{
		struct timeval tv;
#ifdef DEBUG
		printf ("RQUOTA successful\n");
#endif 
		if (!vflag && gq_rslt.gqr_rquota.rq_active == FALSE)
			return (0);
		gettimeofday(&tv, NULL);
/*
 * dqblk in Alpha pool is now:  The new structure is:
 *

   struct  dqblk {
        u_int   dqb_bhardlimit; absolute limit on disk blks alloc 
        u_int   dqb_bsoftlimit; preferred limit on disk blks 
        u_int   dqb_curblocks;  current block count 
        u_int   dqb_ihardlimit; maximum # allocated inodes + 1 
        u_int   dqb_isoftlimit; preferred inode limit
        u_int   dqb_curinodes;  current # allocated inodes 
        time_t  dqb_btime;      time limit for excessive disk use 
        time_t  dqb_itime;      time limit for excessive files
   };
 *
 *
 */
		dqp->dqb_bhardlimit =
		    gq_rslt.gqr_rquota.rq_bhardlimit *
		    gq_rslt.gqr_rquota.rq_bsize / DEV_BSIZE;
		dqp->dqb_bsoftlimit =
		    gq_rslt.gqr_rquota.rq_bsoftlimit *
		    gq_rslt.gqr_rquota.rq_bsize / DEV_BSIZE;
		dqp->dqb_curblocks =
		    gq_rslt.gqr_rquota.rq_curblocks *
		    gq_rslt.gqr_rquota.rq_bsize / DEV_BSIZE;
		dqp->dqb_ihardlimit = gq_rslt.gqr_rquota.rq_fhardlimit;
		dqp->dqb_isoftlimit = gq_rslt.gqr_rquota.rq_fsoftlimit;
		dqp->dqb_curinodes = gq_rslt.gqr_rquota.rq_curfiles;
		dqp->dqb_btime =
		    tv.tv_sec + gq_rslt.gqr_rquota.rq_btimeleft;
		dqp->dqb_itime =
		    tv.tv_sec + gq_rslt.gqr_rquota.rq_ftimeleft;

		*cp = '@';
		return (1);
		}

	case Q_NOQUOTA:
#ifdef DEBUG
		printf("Contacted server, but NOQUOTA \n");
#endif 
		break;

	case Q_EPERM:
		fprintf(stderr, "quota permission error, host: %s\n", hostp);
		break;

	default:
		fprintf(stderr, "bad rpc result, host: %s\n",  hostp);
		break;
	}
	*cp = '@';
	return (0);
}

callaurpc(host, prognum, versnum, procnum, inproc, in, outproc, out)
	char *host;
	xdrproc_t inproc, outproc;
	char *in, *out;
{
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat;
	struct hostent *hp;
	struct timeval timeout, tottimeout;

	static CLIENT *client = NULL;
	static int socket = RPC_ANYSOCK;
	static int valid = 0;
	static int oldprognum, oldversnum;
	static char oldhost[256];

	if (valid && oldprognum == prognum && oldversnum == versnum
		&& strcmp(oldhost, host) == 0) {
		/* reuse old client */		
	}
	else {
		valid = 0;
		close(socket);
		socket = RPC_ANYSOCK;
		if (client) {
			clnt_destroy(client);
			client = NULL;
		}
		if ((hp = gethostbyname(host)) == NULL)
			return ((int) RPC_UNKNOWNHOST);
		timeout.tv_usec = 0;
		timeout.tv_sec = 6;
		bcopy(hp->h_addr, &server_addr.sin_addr, hp->h_length);
		server_addr.sin_family = AF_INET;
		/* ping the remote end via tcp to see if it is up */
		server_addr.sin_port =  htons(PMAPPORT);
		if ((client = clnttcp_create(&server_addr, PMAPPROG,
		    PMAPVERS, &socket, 0, 0)) == NULL) {
			return ((int) rpc_createerr.cf_stat);
		} else {
			/* the fact we succeeded means the machine is up */
			close(socket);
			socket = RPC_ANYSOCK;
			clnt_destroy(client);
			client = NULL;
		}
		/* now really create a udp client handle */
		server_addr.sin_port =  0;
		if ((client = clntudp_create(&server_addr, prognum,
		    versnum, timeout, &socket)) == NULL)
			return ((int) rpc_createerr.cf_stat);
		client->cl_auth = authunix_create_default();
		valid = 1;
		oldprognum = prognum;
		oldversnum = versnum;
		strcpy(oldhost, host);
	}
	tottimeout.tv_sec = 25;
	tottimeout.tv_usec = 0;
	clnt_stat = clnt_call(client, procnum, inproc, in,
	    outproc, out, tottimeout);
	/* 
	 * if call failed, empty cache
	 */
	if (clnt_stat != RPC_SUCCESS)
		valid = 0;
	return ((int) clnt_stat);
}
