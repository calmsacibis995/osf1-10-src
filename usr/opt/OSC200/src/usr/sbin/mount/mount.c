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
static char	sccsid[] = "@(#)$RCSfile: mount.c,v $ $Revision: 4.3.19.11 $ (DEC) $Date: 1994/01/10 18:37:07 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1980, 1989 The Regents of the University of California.
 * All rights reserved.
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
/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

/*
 *	Revision History
 *
 *  07-Nov-91	condylis
 *	noconn mount option made default.
 *	
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#include <sys/secdefines.h>

#include "pathnames.h"
#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fstab.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/fs_types.h>
#define _MACH_VM_PARAM_H_  /* so that vnode.h will compile */
#include <sys/vnode.h>
#undef _MACH_VM_PARAM_H_
#ifdef NFS
#define _SOCKADDR_LEN
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_prot.h>
#include <rpcsvc/mount.h>
#include <nfs/nfs_clnt.h>
#include <nfs/rnode.h>
#endif

#ifdef	OSF
/*
 * One day we'll change to use the MNT_* syntax
 */
#define MNT_RDONLY	M_RDONLY
#define MNT_EXRDONLY	M_EXRDONLY
#define MNT_EXPORTED	M_EXPORTED
#define MNT_UPDATE	M_UPDATE
#define MNT_NOEXEC	M_NOEXEC
#define MNT_NOSUID	M_NOSUID
#define MNT_NODEV	M_NODEV
#define MNT_SYNCHRONOUS	M_SYNCHRONOUS
#define MNT_QUOTA	M_QUOTA
#define MNT_LOCAL	M_LOCAL
#define MNT_VISFLAGMASK	M_VISFLAGMASK
#define MNT_FMOUNT	M_FMOUNT
#if	SEC_ARCH
#define MNT_SECURE	M_SECURE
#endif
#endif	/* OSF */

#define DEFAULT_ROOTUID -2

#if     UNIX_DOMAIN
#include <sys/un.h>
#endif 

#include <cdfs/cdfsmount.h>

#define	BADTYPE(type) \
	(strcmp(type, FSTAB_RO) && strcmp(type, FSTAB_RW) && \
	    strcmp(type, FSTAB_RQ))
#define	SETTYPE(type) \
	(!strcmp(type, FSTAB_RW) || !strcmp(type, FSTAB_RQ))

extern int getfsstat();
int fake, verbose, mntflg, mnttype, fullopts;
char *mntname, **envp;
char **vfslist, **makevfslist();

#ifdef NFS
AUTH *authunix_default();
int xdr_dir(), xdr_fh();
char *getnfsargs();
static void prmount();
struct nfs_args nfsdefargs = {
	NULL,
	NULL,
	NFSMNT_INT,
	8192,
	8192,
	NFS_TIMEO,
	5,
	NULL,
	ACMINMAX,
        ACMAXMAX,
	ACMINMAX,
        ACMAXMAX,
	NULL,
	NULL,
};

struct nfhret {
	u_int	stat;
	nfsv2fh_t nfh;
};
int retrycnt;
#define	DEF_RETRY	10000
#define	BGRND	1
#define	ISBGRND	2
int opflags = 0;
int nfs_port;
#endif

extern char *rawname();

int devdone;

main(argc, argv, arge)
	int argc;
	char **argv;
	char **arge;
{
	extern char *optarg;
	extern int optind;
	register struct fstab *fs;
	register int cnt;
	int all, ch, rval, flags, ret, pid, i, prauto;
	int mntsize;
	struct statfs *mntbuf, *getmntpt();
	char *type, *options = NULL;
	FILE *pidfile;

#if SEC_BASE 
	mount_init(argc, argv);
#endif
	envp = arge;
	all = 0;
	prauto = 0;
	type = NULL;
	mnttype = MOUNT_UFS;
	mntname = "ufs";
#if SEC_ARCH
	/*
	 * To minimize #ifdef clutter, we add the flag letters
	 * for all possible label options to the option string
	 * and just protect the individual cases with appropriate
	 * policy conditionals. If a flag for a non-configured
	 * policy is given, we still get the usage message by
	 * falling into the default case.
	 */
	while ((ch = getopt(argc, argv, "adefrwuvlt:o:A:S:I:C:")) != EOF)
#else
	while ((ch = getopt(argc, argv, "adefrwuvlt:o:")) != EOF)
#endif
		switch((char)ch) {
		case 'a':
			all = 1;
			break;
		case 'd':
			mntflg |= MNT_FMOUNT;
			break;
		case 'e':
			prauto = 1;
			break;
		case 'f':
			fake = 1;
			break;
		case 'r':
			if (type && SETTYPE(type)) {
				fprintf(stderr, "Conflicting ro/rw options.\n");
				usage();
				/* NOTREACHED */
			}
			type = FSTAB_RO;
			mntflg |= MNT_RDONLY;
			break;
		case 'u':
			mntflg |= MNT_UPDATE;
			break;
		case 'v':
			verbose++;
			break;
		case 'w':
			if (type && !SETTYPE(type)) {
				fprintf(stderr, "Conflicting ro/rw options.\n");
				usage();
				/* NOTREACHED */
			}
			type = FSTAB_RW;
			break;
		case 'o':
			options = optarg;
			break;
		case 't':
			vfslist = makevfslist(optarg);
			mnttype = getmnttype(optarg);
			break;
#if SEC_ARCH
#if SEC_ACL
		case 'A':
#endif
#if SEC_MAC
		case 'S':
#endif
#if SEC_ILB
		case 'I':
#endif
#if SEC_NCAV
		case 'C':
#endif
			mount_optflag(ch, optarg);
			break;
#endif
		case 'l':
			fullopts = 1;
			break;

		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	/* NOSTRICT */

	if (all) {
		rval = 0;
		while (fs = getfsent()) {
			if (BADTYPE(fs->fs_type))
				continue;
			if (badvfsname(fs->fs_vfstype, vfslist))
				continue;
			/* `/' is special, it's always mounted */
			if (!strcmp(fs->fs_file, "/"))
				flags = MNT_UPDATE;
			else
				flags = mntflg;
			mnttype = getmnttype(fs->fs_vfstype);
#if SEC_ARCH
			/*
			 * Clear out any security labels that were
			 * specified in the fstab entry for the previous
			 * filesystem.
			 */
			mount_resetopts();
#endif /* SEC_ARCH */
			rval |= mountfs(fs->fs_spec, fs->fs_file, flags,
			    type, options, fs->fs_mntops);
		}
		if ((pidfile = fopen(_PATH_MOUNTDPID, "r")) != NULL) {
			pid = 0;
			fscanf(pidfile, "%d", &pid);
			fclose(pidfile);
			if (pid > 0)
				kill(pid, SIGHUP);
		}
		exit(rval);
	}

	if (argc == 0) {
		if (verbose || fake || type)
			usage();
#ifdef DEBUG
		printf("Trying getmntinfo at 257\n");
#endif
		if ((mntsize = getmntinfo(&mntbuf, MNT_NOWAIT)) == 0) {
			fprintf(stderr,
				"mount: cannot get mount information\n");
			exit(1);
		}
#ifdef DEBUG
		printf("Finished getmntinfo at 265\n");
#endif
		for (i = 0; i < mntsize; i++) {
			if (badvfstype(mntbuf[i].f_type, vfslist))
				continue;
			if ((!prauto) && (mntbuf[i].f_type == MOUNT_NFS) && 
			(mntbuf[i].mount_info.nfs_args.flags & NFSMNT_AUTO))
				continue;
			prmount(mntbuf[i].f_type, mntbuf[i].f_mntfromname, 
				mntbuf[i].f_mntonname, mntbuf[i].f_flags, 
				&mntbuf[i].mount_info);
		}
#ifdef DEBUG
		printf("Returning #1\n");
#endif
		exit(0);
	}

	if (argc == 1 && (mntflg & MNT_UPDATE)) {
		if ((mntbuf = getmntpt(*argv)) == NULL) {
			fprintf(stderr,
			    "mount: unknown special file or file system %s.\n",
			    *argv);
			exit(1);
		}
		mnttype = mntbuf->f_type;
		if (!strcmp(mntbuf->f_mntfromname, "root_device")) {
			fs = getfsfile("/");
			strcpy(mntbuf->f_mntfromname, fs->fs_spec);
		}
		ret = mountfs(mntbuf->f_mntfromname, mntbuf->f_mntonname,
		    mntflg, type, options, (char *)NULL);
	} else if (argc == 1) {
		if (!(fs = getfsfile(*argv)) && !(fs = getfsspec(*argv))) {
			fprintf(stderr,
			    "mount: unknown special file or file system %s.\n",
			    *argv);
			exit(1);
		}
		if (BADTYPE(fs->fs_type)) {
			fprintf(stderr,
			    "mount: %s has unknown file system type.\n", *argv);
			exit(1);
		}
		mnttype = getmnttype(fs->fs_vfstype);
		ret = mountfs(fs->fs_spec, fs->fs_file, mntflg,
		    type, options, fs->fs_mntops);
	} else if (argc != 2) {
		usage();
		exit(1);
	} else {
		/*
		 * If -t flag has not been specified, and spec
		 * contains either a ':' or a '@' then assume that
		 * an NFS filesystem is being specified ala Sun.
		 */
		if (vfslist == (char **)0 &&
		    (index(argv[0], ':') || index(argv[0], '@')))
			mnttype = MOUNT_NFS;
		ret = mountfs(argv[0], argv[1], mntflg, type, options, 
			      (char *)NULL);
	}
	if ((pidfile = fopen(_PATH_MOUNTDPID, "r")) != NULL) {
		pid = 0;
		fscanf(pidfile, "%d", &pid);
		fclose(pidfile);
		if (pid > 0)
			kill(pid, SIGHUP);
	}
	exit (ret);
}

mountfs(spec, name, flags, type, options, mntopts)
	char *spec, *name, *type, *options, *mntopts;
	int flags;
{
	extern int errno;
	int status;
	pid_t pid;
	int argc, i;
	struct ufs_args args;
	struct nfs_args nfsargs;
	struct cdfs_args cdfsargs;
	struct ffm_args ffmargs;
	char *argv[50];
	caddr_t argp;
	char execname[MAXPATHLEN + 1], flagval[12];
	char canonb[MAXPATHLEN + 1];
	char *canon;
	extern char *realpath();

#ifdef DEBUG
	printf("Entering mountfs\n");
#endif
	devdone = 0;			/* M_NODEV hasn't been seen yet */
	nfsargs = nfsdefargs;
	if (mntopts)
		getstdopts(mntopts, &flags);
	if (options)
		getstdopts(options, &flags);
	if (type) {
		if (SETTYPE(type)) { /* FSTAB_RW || FSTAB_RQ */
			if (flags & MNT_RDONLY) {
				fprintf(stderr, "Conflicting ro/rw options.\n");
				errno = EINVAL;	/* error message already printed */
				return(1);
			}
		} else { /* FSTAB_RO */
			if ((flags & MNT_RDONLY) == 0) {
				fprintf(stderr, "Conflicting ro/rw options.\n");
				errno = EINVAL;	/* error message already printed */
				return(1);
			}
		}
	}
	if (type)
		getstdopts(type, &flags);
#if SEC_ARCH
	/*
	 * Look for label options specified with -o on the command line.
	 * These override corresponding ones specified in fstab.
	 */
	if (options)
		mount_getopts(options);
	/*
	 * Look for label options specified in the fstab entry.
	 */
	if (mntopts)
		mount_getopts(mntopts);
#endif
#if SEC_BASE
	mount_checkauth();
#endif

	/*
	 * Massage out symbolic links from name
	 */
	canon = realpath(name, canonb);
	if (canon)
		name = canon;

	switch (mnttype) {
	case MOUNT_UFS:
		if (mntopts)
			getufsopts(mntopts, &flags);
		if (options)
			getufsopts(options, &flags);
		args.fspec = spec;
		args.exroot = DEFAULT_ROOTUID;
		if (flags & MNT_RDONLY)
			args.exflags = MNT_EXRDONLY;
		else
			args.exflags = 0;
		argp = (caddr_t)&args;
		/* if !update we know it can't be overlap since it's
		 * already mounted;  otherwise, we need to check.
		 */
		if ((flags & MNT_UPDATE) == 0 && check_for_overlap(spec)) {
			errno = EINVAL;	/* error message already printed */
			if (fake)
				goto out;
			else
				return(1);
		}
		break;

	case MOUNT_CDFS:
		memset(&cdfsargs, 0, sizeof(cdfsargs));
		if (mntopts)
			getcdfsopts(mntopts, &cdfsargs);
		if (options)
			getcdfsopts(options, &cdfsargs);
		if (!devdone)
		    /* default CDs to M_NODEV;
		       device mappings are almost certainly required. */
		    flags |= M_NODEV;
		cdfsargs.fspec = spec;
		cdfsargs.exroot = DEFAULT_ROOTUID;
		cdfsargs.exflags = MNT_EXRDONLY;
		argp = (caddr_t)&cdfsargs;
		break;

	case MOUNT_S5FS:
		if (mntopts)
			gets5fsopts(mntopts, &flags);
		if (options)
			gets5fsopts(options, &flags);
		args.fspec = spec;
		argp = (caddr_t)&args;
		break;

        case MOUNT_FFM:
                ffmargs.ffm_flags = 0;
                if (options)
                        getffmopts(options, &ffmargs);
                ffmargs.ffm_pathname = spec;
                argp = (caddr_t)&ffmargs;
                break;

#ifdef NFS
	case MOUNT_NFS:
		retrycnt = DEF_RETRY;
		nfs_port = 0; /* init port number of server */
		if (mntopts)
			getnfsopts(mntopts, &nfsargs, &opflags, &retrycnt);
		if (options)
			getnfsopts(options, &nfsargs, &opflags, &retrycnt);
		if (argp = getnfsargs(spec, &nfsargs)) {
#ifdef	DEBUG
			dumpnfsargs(&nfsargs);
#endif
			break;
		}
		return (1);
#endif /* NFS */

	case MOUNT_PROCFS:
		if(mntopts)
			getprocopts(mntopts, &flags);
		if(options)
			getprocopts(options, &flags);
		args.fspec = spec;
		if (flags & MNT_RDONLY)
			args.exflags = MNT_EXRDONLY;
		else
			args.exflags = 0;
		argp = (caddr_t)&args;
		break;

	case MOUNT_MFS:
	default:
		argv[0] = mntname;
		argc = 1;
		if (flags) {
			argv[argc++] = "-F";
			sprintf(flagval, "%d", flags);
			argv[argc++] = flagval;
		}
		if (mntopts)
			argc += getexecopts(mntopts, &argv[argc]);
		if (options)
			argc += getexecopts(options, &argv[argc]);
		argv[argc++] = spec;
		argv[argc++] = name;
		argv[argc++] = NULL;
		sprintf(execname, "%s/mount_%s", _PATH_EXECDIR, mntname);
		if (verbose) {
			(void)printf("exec: %s", execname);
			for (i = 1; i < argc; i++)
				(void)printf(" %s", argv[i]);
			(void)printf("\n");
		}
		if (fake)
			break;
		if (pid = vfork()) {
			if (pid == -1) {
				perror("mount: vfork starting file system");
				return (1);
			}
			if (waitpid(pid, &status, 0) != -1 &&
			    WIFEXITED(status) &&
			    WEXITSTATUS(status) != 0)
				return (WEXITSTATUS(status));
			spec = mntname;
			goto out;
		}
		execve(execname, argv, envp);
		fprintf(stderr, "mount: cannot exec %s for %s: ",
			execname, name);
		perror((char *)NULL);
		exit (1);
		/* NOTREACHED */

	}
#if SEC_ARCH
	/*
	 * Convert any label options that were specified into their
	 * internal representations, and mount the filesystem.
	 */
	mount_setattrs();
	if (!fake && mount_domount((long)mnttype, name, (long)flags, argp))
#else
#ifdef DEBUG
	printf("Trying to mount...\n");
	printf("type %o name %s\n", mnttype, name);
	printf("flags %o data %s\n", flags, args.fspec);
	if (((caddr_t)&args) != argp) printf("argp has bogus addr\n");
#endif
	if (!fake && mount(mnttype, name, flags, argp))
#endif
	{
#ifdef DEBUG
		printf("Problems with mount...\n");
		printf("Errno %d\n", errno); 
		printf("test\n");
#endif
		if (opflags & ISBGRND)
			exit(1);
#ifdef DEBUG
		printf("Made it to printf...\n");
#endif
		fprintf(stderr, "%s on %s: ", spec, name);
#ifdef DEBUG
		printf("Did printf...\n");
#endif
#if SEC_ARCH
		{
		    extern int sec_errno;
		    extern char *sys_secerrlist[];
		    if (sec_errno) {
			fprintf(stderr, "%s\n", sys_secerrlist[sec_errno]);
			return(1);
		    }
		}
#endif
		switch (errno) {
		case EDIRTY:
			fprintf(stderr, "Dirty file system\n");
			break;
		case EMFILE:
			fprintf(stderr, "Mount table full\n");
			break;
		case EINVAL:
			if (flags & MNT_UPDATE)
				fprintf(stderr, "Specified device does %s\n",
					"not match mounted device");
			else
				switch (mnttype) {
				case MOUNT_NFS:
					fprintf(stderr, "Range check for NFS option value failed\n");
					break;
				default:
					fprintf(stderr, "No valid filesystem exists on this partition\n");
					break;
				}
			break;
		case EOPNOTSUPP:
			switch (mnttype) {
			      case MOUNT_UFS:
				fprintf(stderr, "Block size must equal 8192\n");
				break;
			      default:
			fprintf(stderr, "Operation not supported\n");
				break;
			}
			break;
		default:
			perror((char *)NULL);
			break;
		}
#ifdef DEBUG
		printf("Exiting mountfs with error\n");
#endif
		return(1);
	}

out:
#ifdef DEBUG
	printf("Going out...\n");
#endif
	if (verbose)
		prmount(mnttype, spec, name, flags, argp);

	if (opflags & ISBGRND)
		exit(1);
#ifdef DEBUG
	printf("Exiting mountfs\n");
#endif
	return(0);
}

static void
prmount(mnttype, spec, name, flags, mount_info)
	char *spec, *name;
	register int mnttype;
	register short flags;
	register union mount_info *mount_info;
{
        extern char *bavtooptstr();
	register int first;
	register int sflags, printoptions = 0;
	char optstr[1024];

#ifdef DEBUG
	printf("Entering prmount...\n");
#endif
	if (opflags & ISBGRND)
		return;
#ifdef DEBUG
	printf("Printing in prmount...\n");
#endif
	(void)printf("%s on %s", spec, name);
	(void)printf(" type");

	/* Look up FS name in mnt_names array defined in sys/fs_types.h> */
        if (mnttype > 0 && mnttype <= MOUNT_MAXTYPE)
               printf( " %s", mnt_names[mnttype] );
#ifdef DEBUG
	printf("Going to bavtoopststr...\n");
#endif
	bavtooptstr(optstr, mnttype, flags, mount_info, fullopts);
#ifdef DEBUG
	printf("Back from bavtoopststr...\n");
#endif
	(void)printf("%s", optstr);
#ifdef DEBUG
	printf("Prmount done...\n");
#endif
}

char *
bavtooptstr(s, mnttype, flags, mount_info, fullopts)
	char *s;
	register int mnttype;
	register int flags;
	register union mount_info *mount_info;
	int fullopts;
{
	register int first, sflags;

#ifdef DEBUG
	printf("In bavtoopststr...\n");
#endif
	first = 0;
#define	PR(s, msg) s += sprintf(s, "%s%s", !first++ ? " (" : ", ", msg)

#define	PRV(s, val) s += sprintf(s, "%d", val) 

	if (flags & MNT_RDONLY)
		PR(s, "ro");
	else 
		PR(s, "rw");

	if (flags & MNT_NOEXEC)
		PR(s, "noexec");
	else if (fullopts)
		PR(s, "exec");

	if (flags & MNT_NOSUID)
		PR(s, "nosuid");
	else if (fullopts)
		PR(s, "suid");

	if (flags & MNT_NODEV)
		PR(s, "nodev");
	else if (fullopts)
		PR(s, "dev");

	if (flags & MNT_SYNCHRONOUS)
		PR(s, "sync");
	else if (fullopts)
		PR(s, "nosync");

	if (flags & MNT_QUOTA)
		PR(s, "quota");
	else if (fullopts)
		PR(s, "noquota");

/*	if (flags & MNT_LOCAL)
		PR(s, "local");
*/
#if SEC_ARCH
        if ((flags & MNT_SECURE) == 0)
                PR(s, "unextended");
#endif
	if (flags & MNT_EXPORTED)
		if (flags & MNT_EXRDONLY)
			PR(s, "NFS exported read-only");
		else
			PR(s, "NFS exported");

	switch (mnttype) {
	case MOUNT_CDFS:
		sflags = mount_info->cdfs_args.flags;

		if (sflags & M_RRIP)
			PR(s, "rrip");

		if (sflags & M_NODEFPERM)
			PR(s, "nodefperm");
		else 
			PR(s, "defperm");

		if (sflags & M_NOVERSION)
			PR(s, "noversion");

		if (sflags & M_PRIMARY)
			PR(s, "primary");

		break;
	case MOUNT_NFS:
		sflags = mount_info->nfs_args.flags;
		if (sflags & NFSMNT_SOFT)
			PR(s, "soft");
		else
			PR(s, "hard");

		if (sflags & NFSMNT_INT)
			PR(s, "intr");
		else
			PR(s, "nintr");

		if (sflags & NFSMNT_NOAC)
			PR(s, "noac");
		else if (fullopts)
			PR(s, "ac");

		if (sflags & NFSMNT_NOCTO)
			PR(s, "nocto");
		else if (fullopts)
			PR(s, "cto");

		if ((sflags & NFSMNT_WSIZE) || (fullopts)) {
			PR(s, "wsize="); PRV(s, mount_info->nfs_args.wsize);
		}

		if ((sflags & NFSMNT_RSIZE) || (fullopts)) {
			PR(s, "rsize="); PRV(s, mount_info->nfs_args.rsize);
		}

		if ((sflags & NFSMNT_TIMEO) || (fullopts)) {
			PR(s, "timeo="); PRV(s, mount_info->nfs_args.timeo);
		}

		if ((sflags & NFSMNT_RETRANS) || (fullopts)) {
			PR(s, "retrans="); PRV(s, mount_info->nfs_args.retrans);
		}

#define AC_MASK (NFSMNT_ACREGMAX | NFSMNT_ACDIRMAX | NFSMNT_ACREGMIN | NFSMNT_ACDIRMIN)
		if (((sflags & AC_MASK) == AC_MASK) 
		&& ((mount_info->nfs_args.acregmin == 
		mount_info->nfs_args.acregmax) &&
		(mount_info->nfs_args.acregmin == 
		mount_info->nfs_args.acdirmin) &&
		(mount_info->nfs_args.acregmin ==
		mount_info->nfs_args.acdirmax))) {
			PR(s, "actimeo="); PRV(s, mount_info->nfs_args.acregmax);
		} else {
			if ((sflags & NFSMNT_ACREGMIN) || (fullopts)) {
				PR(s, "acregmin=");
				PRV(s, mount_info->nfs_args.acregmin);
			}
			if ((sflags & NFSMNT_ACREGMAX) || (fullopts)) {
				PR(s, "acregmax=");
				PRV(s, mount_info->nfs_args.acregmax);
			}
			if ((sflags & NFSMNT_ACDIRMIN) || (fullopts)) {
				PR(s, "acdirmin=");
				PRV(s, mount_info->nfs_args.acdirmin);
			}
			if ((sflags & NFSMNT_ACDIRMAX) || (fullopts)) {
				PR(s, "acdirmax=");
				PRV(s, mount_info->nfs_args.acdirmax);
			}
		}
		break;

	default:
		break;
	}
		
#ifdef DEBUG
	printf("Finished bavtoopststr\n");
#endif
	(void)sprintf(s, ")\n");
#ifdef DEBUG
	printf("Really finished bavtoopststr\n");
#endif
}

getmnttype(fstype)
	char *fstype;
{
	int mount_type;

	mntname = fstype;
        for (mount_type = MOUNT_UFS; mount_type <= MOUNT_MAXTYPE; mount_type++) 
                if (!strcmp(fstype, mnt_names[mount_type]))
                        return(mount_type);
	return (MOUNT_NONE);
}

usage()
{
#if SEC_ARCH
        char labelargs[48];
	
	sprintf(labelargs, "%s%s%s%s",
#if SEC_ACL
		" [-A acl]",
#endif
#if SEC_MAC
		" [-S slevel]",
#endif
#if SEC_ILB
		" [-I ilevel]",
#endif
#if SEC_NCAV
		" [-C ncav]",
#endif
		"", "", "", "");
	fprintf(stderr, "usage: mount [-adefurw]\n");
	fprintf(stderr, "   or  mount [-dfurw]%s special | node\n", labelargs);
	fprintf(stderr, "   or  mount [-dfurw]%s special node\n", labelargs);
#else
	fprintf(stderr, "usage:\n  mount %s %s\n  mount %s\n  mount %s\n",
		"[ -dfrwu ] [ -t nfs | ufs | cdfs | external_type ]",
		"[ -o options ] special node",
		"[ -adefrwu ] [ -t nfs | ufs | cdfs | external_type ]",
		"[ -dfrwu ] special | node");
#endif
	exit(1);
}

getstdopts(options, flagp)
	char *options;
	int *flagp;
{
	register char *opt, *start;
	register char *leftopts = options;
	int negative;
	char optbuf[BUFSIZ];
	char *type = NULL;

	(void)strcpy(optbuf, options);
	for (opt = strtok(optbuf, ","); opt; opt = strtok((char *)NULL, ",")) {
		start = opt;
		if (opt[0] == 'n' && opt[1] == 'o') {
			negative++;
			opt += 2;
		} else {
			negative = 0;
		}
		if (!negative && !strcasecmp(opt, FSTAB_RO)) {
			if (type && SETTYPE(type)) {
				fprintf(stderr, "Conflicting ro/rw options.\n");
				usage();
				/* NOTREACHED */
			} else
				type = FSTAB_RO;
			*flagp |= MNT_RDONLY;
			continue;
		}
		else if (!negative && !strcasecmp(opt, FSTAB_RW)) {
			if (type && !SETTYPE(type)) {
				fprintf(stderr, "Conflicting ro/rw options.\n");
				usage();
				/* NOTREACHED */
			} else
				type = FSTAB_RW;
			*flagp &= ~MNT_RDONLY;
			continue;
		}
		else if (!strcasecmp(opt, "exec")) {
			if (negative)
				*flagp |= MNT_NOEXEC;
			else
				*flagp &= ~MNT_NOEXEC;
			continue;
		}
		else if (!strcasecmp(opt, "suid")) {
			if (negative)
				*flagp |= MNT_NOSUID;
			else
				*flagp &= ~MNT_NOSUID;
			continue;
		}
		else if (!strcasecmp(opt, "dev")) {
			if (negative)
				*flagp |= MNT_NODEV;
			else
				*flagp &= ~MNT_NODEV;
			devdone = 1;
			continue;
		}
		else if ((!strcasecmp(opt, "sync")) ||
		(!strcasecmp(opt, "synchronous"))) {
			if (!negative)
				*flagp |= MNT_SYNCHRONOUS;
			else
				*flagp &= ~MNT_SYNCHRONOUS;
			continue;
		}
		if (leftopts != options)
			*leftopts++ = ',';
		strcpy(leftopts, start);
		leftopts += strlen(start);
	}
	*leftopts = '\0';
}

gets5fsopts(options, flagp)
	char *options;
	int *flagp;
{

	return;
}

getufsopts(options, flagp)
	char *options;
	int *flagp;
{

	return;
}

getcdfsopts(optarg, cdfsargsp)
	char *optarg;
	struct cdfs_args *cdfsargsp;
{
	char *modestr;
	int *flagp = &cdfsargsp->flags;
	int valflt;
	register char *cp, *nextcp;
	int num;
	char *nump;

	cp = optarg;
	while (cp != NULL && *cp != '\0') {
		if ((nextcp = index(cp, ',')) != NULL)
			*nextcp++ = '\0';
		if ((nump = index(cp, '=')) != NULL) {
			*nump++ = '\0';
			num = atoi(nump);
		} else
			num = -1;
		/*
		 * Just test for a string match and do it
		 */
		if (!strcasecmp(cp, "defperm")) {
			*flagp &= ~M_NODEFPERM;
		} else if (!strcasecmp(cp, "nodefperm")) {
			*flagp |= M_NODEFPERM;
		} else if (!strcasecmp(cp, "noversion")) {
			*flagp |= M_NOVERSION;
		} else if (!strcasecmp(cp, "primary")) {
			*flagp |= M_PRIMARY;
		} else if (!strcasecmp(cp, "rrip")) {
			*flagp |= M_RRIP;
		}  
		cp = nextcp;
	}
	return;
}

/* ARGSUSED */
getprocopts(mntopts, flags)
	char *mntopts;
	int *flags;
{
	return;
}

getexecopts(options, argv)
	char *options;
	char **argv;
{
	register int argc = 0;
	register char *opt;

	for (opt = strtok(options, ","); opt; opt = strtok(NULL, ",")) {
		if (opt[0] != '-')
			continue;
		argv[argc++] = opt;
		if (opt[2] == '\0' || opt[2] != '=')
			continue;
		opt[2] = '\0';
		argv[argc++] = &opt[3];
	}
	return (argc);
}

struct statfs *
getmntpt(name)
	char *name;
{
	int mntsize;
	register int i;
	struct statfs *mntbuf;

#ifdef DEBUG
	printf("Trying getmntinfo at 904\n");
#endif
	mntsize = getmntinfo(&mntbuf, MNT_NOWAIT);
#ifdef DEBUG
	printf("Finished getmntinfo at 908\n");
#endif
	for (i = 0; i < mntsize; i++) {
		if (!strcmp(mntbuf[i].f_mntfromname, name) ||
		    !strcmp(mntbuf[i].f_mntonname, name))
			return (&mntbuf[i]);
	}
	return ((struct statfs *)0);
}

static int skipvfs;

badvfstype(vfstype, vfslist)
	short vfstype;
	char **vfslist;
{

	if (vfslist == 0)
		return(0);
	while (*vfslist) {
		if ((vfstype == getmnttype(*vfslist)) != MOUNT_NONE)
			return(skipvfs);
		vfslist++;
	}
	return (!skipvfs);
}

badvfsname(vfsname, vfslist)
	char *vfsname;
	char **vfslist;
{

	if (vfslist == 0)
		return(0);
	while (*vfslist) {
		if (strcmp(vfsname, *vfslist) == 0)
			return(skipvfs);
		vfslist++;
	}
	return (!skipvfs);
}

char **
makevfslist(fslist)
	char *fslist;
{
	register char **av, *nextcp;
	register int i;

	if (fslist == NULL)
		return (NULL);
	if (fslist[0] == 'n' && fslist[1] == 'o') {
		fslist += 2;
		skipvfs = 1;
	}
	for (i = 0, nextcp = fslist; *nextcp; nextcp++)
		if (*nextcp == ',')
			i++;
	av = (char **)malloc((size_t)(i+2) * sizeof(char *));
	if (av == NULL)
		return (NULL);
	nextcp = fslist;
	i = 0;
	av[i++] = nextcp;
	while (nextcp = index(nextcp, ',')) {
		*nextcp++ = '\0';
		av[i++] = nextcp;
	}
	av[i++] = 0;
	return (av);
}


#ifdef NFS
/*
 * Handle the getoption arg.
 * Essentially update "opflags", "retrycnt" and "nfsargs"
 */
getnfsopts(optarg, nfsargsp, opflagsp, retrycntp)
	char *optarg;
	struct nfs_args *nfsargsp;
	int *opflagsp;
	int *retrycntp;
{
	register char *cp, *nextcp;
	int num, valflt;
	char *nump;

	cp = optarg;
	while (cp != NULL && *cp != '\0') {
		/* Set by badval macro when invalid option value specified */
		valflt = 0;
		if ((nextcp = index(cp, ',')) != NULL)
			*nextcp++ = '\0';
		if ((nump = index(cp, '=')) != NULL) {
			*nump++ = '\0';
			num = atoi(nump);
		} else
			/* -1 for ac*min or ac*max means set to max */
			num = -2;

/* This is used to validate an option that specifies a numeric value	*/
#define	badval(cmp, val) ((val) cmp ? valflt=1 : (0)) 
		/*
		 * Just test for a string match and do it
		 */
		nfsargsp->flags |= NFSMNT_HOSTNAME;
		if (!strcasecmp(cp, "bg")) {
			*opflagsp |= BGRND;
		} else if (!strcasecmp(cp, "fg")) {
			*opflagsp &= ~BGRND;
		} else if (!strcasecmp(cp, "soft")) {
			nfsargsp->flags |= NFSMNT_SOFT;
		} else if (!strcasecmp(cp, "hard")) {
			nfsargsp->flags &= ~NFSMNT_SOFT;
		} else if (!strcasecmp(cp, "intr")) {
			nfsargsp->flags |= NFSMNT_INT;
		} else if (!strcasecmp(cp, "nintr")) {
			nfsargsp->flags &= ~NFSMNT_INT;
		} else if ((!strcasecmp(cp, "retry")) && !badval(<=0, num)) {
			*retrycntp = num;
		} else if ((!strcasecmp(cp, "rsize")) && !badval(<=0, num)) {
			nfsargsp->rsize = num;
			nfsargsp->flags |= NFSMNT_RSIZE;
		} else if (!strcasecmp(cp, "wsize") && !badval(<=0, num)) {
			nfsargsp->wsize = num;
			nfsargsp->flags |= NFSMNT_WSIZE;
		} else if (!strcasecmp(cp, "timeo")) {
			nfsargsp->timeo = num;
			nfsargsp->flags |= NFSMNT_TIMEO;
		} else if (!strcasecmp(cp, "retrans") && !badval(<=0, num)) {
			nfsargsp->retrans = num;
			nfsargsp->flags |= NFSMNT_RETRANS;
		} else if (!strcasecmp(cp, "actimeo") && !badval(<=0, num)) {
			nfsargsp->acregmin = num;
			nfsargsp->acregmax = num;
			nfsargsp->acdirmin = num;
			nfsargsp->acdirmax = num;
			nfsargsp->flags |= AC_MASK;
		} else if (!strcasecmp(cp, "ac")) {
			nfsargsp->flags &= ~NFSMNT_NOAC;
		} else if (!strcasecmp(cp, "noac")) {
			nfsargsp->flags |= NFSMNT_NOAC;
		} else if (!strcasecmp(cp, "cto")) {
			nfsargsp->flags &= ~NFSMNT_NOCTO;
		} else if (!strcasecmp(cp, "nocto")) {
			nfsargsp->flags |= NFSMNT_NOCTO;
		} else if (!strcasecmp(cp, "acregmin") && !badval(<-1, num)) {
			nfsargsp->acregmin = num;
			nfsargsp->flags |= NFSMNT_ACREGMIN;
		} else if (!strcasecmp(cp, "acregmax") && !badval(<-1, num)) {
			nfsargsp->acregmax = num;
			nfsargsp->flags |= NFSMNT_ACREGMAX;
		} else if (!strcasecmp(cp, "acdirmin") && !badval(<-1, num)) {
			nfsargsp->acdirmin = num;
			nfsargsp->flags |= NFSMNT_ACDIRMIN;
		} else if (!strcasecmp(cp, "acdirmax") && !badval(<-1, num)) {
			nfsargsp->acdirmax = num;
			nfsargsp->flags |= NFSMNT_ACDIRMAX;
		} else if (!strcasecmp(cp, "port") && !badval(<-1, num)) {
			nfs_port = num;
		} else if (valflt)
			fprintf(stderr, "WARNING: bad value for %s option\n", cp);
		else
			fprintf(stderr, "WARNING: unknown option %s\n", cp);
		cp = nextcp;
	}
	/* If no attribute cache selected and any option selected to	*/
	/* adjust attribute cache operation 				*/
	if ((nfsargsp->flags & NFSMNT_NOAC) && (nfsargsp->flags & AC_MASK)) {
		(void) fprintf(stderr,
		"mount: contradicting attribute cache options\n");
		exit(1);
	}


}

char *
getnfsargs(spec, nfsargsp)
	char *spec;
	struct nfs_args *nfsargsp;
{
	extern int errno;
	register CLIENT *clp;
	struct hostent *hp;
#if     UNIX_DOMAIN
	static struct sockaddr_un saddr_un;
	int     using_local = 0;
#endif 
	static struct sockaddr_in saddr;
	struct timeval pertry, try;
	enum clnt_stat clnt_stat;
	int so = RPC_ANYSOCK;
	char *hostp, *delimp;
	u_short tport;
	static struct nfhret nfhret;
	static char nam[MNAMELEN + 1];

	strncpy(nam, spec, MNAMELEN);
	nam[MNAMELEN] = '\0';
	if ((delimp = index(spec, '@')) != NULL) {
		hostp = delimp + 1;
	} else if ((delimp = index(spec, ':')) != NULL) {
		hostp = spec;
		spec = delimp + 1;
	} else {
		fprintf(stderr,
		    "No <host>:<dirpath> or <dirpath>@<host> spec\n");
		return (0);
	}
	*delimp = '\0';
#if     UNIX_DOMAIN
#define NFSD_PATH_NAME  "/tmp/.nfsd"
#define LOCAL   "localhost"
	/*
	 * recognize the following as indications of UNIX domain mounts:
	 * 	no hostname (e.g. "path@" or ":path")
 	 * 	hostname == "unix" (e.g. "path@unix" or "unix:path"
	 * The former will be indicated at this point by a hostp
	 * pointing at a null.  The latter is a simple comparision.
	 */
	if ( (*hostp == '\0') || (strcmp(hostp, "unix") == 0) ) {
		hostp=LOCAL;
		using_local = 1;
	}
#endif  /* UNIX_DOMAIN */
	if ((hp = gethostbyname(hostp)) == NULL) {
		fprintf(stderr, "Can't get net id for host\n");
		return (0);
	}
	bcopy(hp->h_addr, (caddr_t)&saddr.sin_addr, hp->h_length);
	nfhret.stat = EACCES;	/* Mark not yet successful */
	while (retrycnt > 0) {
		saddr.sin_family = AF_INET;
#if     UNIX_DOMAIN
		if (!using_local)
#endif  /* UNIX_DOMAIN */
			if (!(tport = nfs_port)) {
				saddr.sin_port = htons(PMAPPORT);
				tport = pmap_getport(&saddr, NFS_PROGRAM,
						     NFS_VERSION, IPPROTO_UDP);
				if (tport == 0) {
					if ((opflags & ISBGRND) == 0)
						clnt_pcreateerror("NFS Portmap");
					goto skip_mount;
				}
			}
			saddr.sin_port = 0;
			pertry.tv_sec = 10;
			pertry.tv_usec = 0;
			if ((clp = clntudp_create(&saddr, MOUNTPROG,
			    MOUNTVERS, pertry, &so)) == NULL) {
				if ((opflags & ISBGRND) == 0)
					clnt_pcreateerror("Cannot MNT PRC");
			} else {
#ifdef notyet
				clp->cl_auth = authunix_create_default();
#else
				clp->cl_auth = authunix_default();
#endif
				try.tv_sec = 10;
				try.tv_usec = 0;
				clnt_stat = clnt_call(clp, MOUNTPROC_MNT,
				    xdr_dir, spec, xdr_fh, &nfhret, try);
				if (clnt_stat != RPC_SUCCESS) {
					if ((opflags & ISBGRND) == 0) {
						char str_tmp[MNAMELEN + 14];

						/* print offending FS name */
						strcpy(str_tmp, "Bad MNT RPC: ");
						strcat(str_tmp, nam);
						clnt_perror(clp, str_tmp);
					}
				} else
					retrycnt = 0;

				auth_destroy(clp->cl_auth);
				clnt_destroy(clp);
			}
skip_mount:
		if (--retrycnt > 0) {
			if (opflags & BGRND) {
				opflags &= ~BGRND;
				if (fork())
					return (0);
				opflags |= ISBGRND;
				(void)setsid();
			} 
			sleep(10);
		}
	}
	if (nfhret.stat) {
		if (opflags & ISBGRND)
			exit(1);
		fprintf(stderr, "Can't access %s: ", spec);
		errno = nfhret.stat;
		perror(NULL);
		return (0);
	}
#if     UNIX_DOMAIN
        if (using_local) {
                saddr_un.sun_family = AF_UNIX;
                strcpy(saddr_un.sun_path, NFSD_PATH_NAME);
                /* saddr_un.sun_len = 0; */
                /*
                 * should change def. of nfsargs->addr to be
                 * a struct sockaddr *, but until then ...
                 */
                nfsargsp->addr = (struct sockaddr_in *) &saddr_un;
                nfsargsp->hostname = "unix";
        } else {
                saddr.sin_port = htons(tport);
                nfsargsp->addr = &saddr;
                nfsargsp->hostname = nam;
        }

#else
	nfsargsp->flags |= NFSMNT_HOSTNAME;
	saddr.sin_port = htons(tport);
	nfsargsp->addr = &saddr;
	nfsargsp->hostname = nam;
#endif  /* UNIX_DOMAIN */
        nfsargsp->fh = &nfhret.nfh; /* junk if UNIX_DOMAIN */
	return ((caddr_t)nfsargsp);
}

/*
 * xdr routines for mount rpc's
 */
xdr_dir(xdrsp, dirp)
	XDR *xdrsp;
	char *dirp;
{
	return (xdr_string(xdrsp, &dirp, MAXPATHLEN));
}

xdr_fh(xdrsp, np)
	XDR *xdrsp;
	struct nfhret *np;
{
	if (!xdr_u_int(xdrsp, &(np->stat)))
		return (0);
	if (np->stat)
		return (1);
	return (xdr_opaque(xdrsp, (caddr_t)&(np->nfh), NFS_FHSIZE));
}

AUTH *
authunix_default()
{
	register int glen;
	char hostname[MAX_MACHINE_NAME+1];
	int	uid;
	int	gid;
	int	grps[NGROUPS];

	if (gethostname(hostname, MAX_MACHINE_NAME) < 0)
		abort();		
	hostname[MAX_MACHINE_NAME] = 0;
	uid = geteuid();
	gid = getegid();
	if ((glen = getgroups(NGROUPS, grps)) < 0)
		abort();
	return (authunix_create(hostname, uid, gid, glen, grps));
}
#endif /* NFS */

#ifdef DEBUG
dumpfsent(fs)
	register struct fstab *fs;
{
	fprintf(stderr, "fs_spec (block dev.): %s\n", fs->fs_spec);
	fprintf(stderr, "fs_file (path): %s\n", fs->fs_file);
	fprintf(stderr, "fs_type (e.g. ro, rw): %s\n", fs->fs_type);
	fprintf(stderr, "fs_freq (dump frequency): %d\n", fs->fs_freq);
	fprintf(stderr, "fs_passno (fsck pass no.): %d\n", fs->fs_passno);
	fprintf(stderr, "fs_vfstype (fs type): %s\n", fs->fs_vfstype);
	if (fs->fs_mntops)
		fprintf(stderr, "fs_mntops (mount options): %s\n", fs->fs_mntops);
}
dumpnfsargs(nfsargs)
struct nfs_args *nfsargs;
{
	fprintf(stderr, "nfsargs: \n");
	fprintf(stderr, "	flags:	 %x\n", nfsargs->flags);
	fprintf(stderr, "	wsize:	 %x\n", nfsargs->wsize);
	fprintf(stderr, "	rsize:	 %x\n", nfsargs->rsize);
	fprintf(stderr, "	timeo:	 %x\n", nfsargs->timeo);
	fprintf(stderr, "	retrans: %x\n", nfsargs->retrans);
	fprintf(stderr, "	host:    %s\n", nfsargs->hostname);
}
#endif

/*
 * Input name in raw, canonicalized pathname output to canon.  If dosymlinks
 * is nonzero, resolves all symbolic links encountered during canonicalization
 * into an equivalent symlink-free form.  Returns 0 on success, -1 on failure.
 * The routine fails if the current working directory can't be obtained or if
 * either of the arguments is NULL.
 *
 * Sets errno on failure.
 */
int
pathcanon(raw, canon, dosymlinks)
	char        *raw, *canon;
	int         dosymlinks;
{
	register char       *s, *d;
	register char       *limit = canon + MAXPATHLEN;
	char                *modcanon;
	int                 nlink = 0;

	/*
	 * Do a bit of sanity checking.
	 */
	if (raw == NULL || canon == NULL) {
		errno = EINVAL;
		return (-1);
	}

	/*
	 * If the path in raw is not already absolute, convert it to that form.
	 * In any case, initialize canon with the absolute form of raw.  Make
	 * sure that none of the operations overflow the corresponding buffers.
	 * The code below does the copy operations by hand so that it can easily
	 * keep track of whether overflow is about to occur.
	 */
	s = raw;
	d = canon;
	if (*s != '/') {
		/* Relative; prepend the working directory. */
		if (getwd(d) == NULL) {
			/* Use whatever errno value getwd may have left around. */
			return (-1);
		}
		d += strlen(d);
		/* Add slash to separate working directory from relative part. */
		if (d < limit)
			*d++ = '/';
		modcanon = d;
	} else
		modcanon = canon;
	while (d < limit && *s)
		*d++ = *s++;

	/* Add a trailing slash to simplify the code below. */
	s = "/";
	while (d < limit && (*d++ = *s++))
		continue;


	/*
	 * Canonicalize the path.  The strategy is to update in place, with
	 * d pointing to the end of the canonicalized portion and s to the
	 * current spot from which we're copying.  This works because
	 * canonicalization doesn't increase path length, except as discussed
	 * below.  Note also that the path has had a slash added at its end.
	 * This greatly simplifies the treatment of boundary conditions.
	 */
	d = s = modcanon;
	while (d < limit && *s) {
		if ((*d++ = *s++) == '/' && d > canon + 1) {
			register char  *t = d - 2;

			switch (*t) {
			      case '/':
				/* Found // in the name. */
				d--;
				continue;
			      case '.':
				switch (*--t) {
				      case '/':
					/* Found /./ in the name. */
					d -= 2;
					continue;
				      case '.':
					if (*--t == '/') {
						/* Found /../ in the name. */
						while (t > canon && *--t != '/')
							continue;
						d = t + 1;
					}
					continue;
				      default:
					break;
				}
				break;
			      default:
				break;
			}
			/*
			 * We're at the end of a component.  If dosymlinks is set
			 * see whether the component is a symbolic link.  If so,
			 * replace it by its contents.
			 */
			if (dosymlinks) {
				char            link[MAXPATHLEN + 1];
				register int    llen;

				/*
				 * See whether it's a symlink by trying to read
				 * it.
				 *
				 * Start by isolating it.
				 */
				*(d - 1) = '\0';
				if ((llen = readlink(canon, link, sizeof link)) >= 0) {
					/* Make sure that there are no circular
					 * links.
					 */
					nlink++;
					if (nlink > MAXSYMLINKS) {
						errno = ELOOP;
						return (-1);
					}
					/*
					 * The component is a symlink.  Since
					 * its value can be of arbitrary size,
					 * we can't continue copying in place.
					 * Instead, form the new path suffix in
					 * the link buffer and then copy it back
					 * to its proper spot in canon.
					 */
					t = link + llen;
					*t++ = '/';
					/*
					 * Copy the remaining unresolved portion
					 * to the end of the symlink. If the sum
					 * of the unresolved part and the
					 * readlink exceeds MAXPATHLEN, the
					 * extra bytes will be dropped off. Too
					 * bad!
					 */
					(void) strncpy(t, s, sizeof link - llen - 1);
					link[sizeof link - 1] = '\0';
					/*
					 * If the link's contents are absolute,
					 * copy it back to the start of canon,
					 * otherwise to the beginning of the
					 * link's position in the path.
					 */
					if (link[0] == '/') {
						/* Absolute. */
						(void) strcpy(canon, link);
						d = s = canon;
					}
					else {
						/*
						 * Relative: find beginning of
						 * component and copy.
						 */
						--d;
						while (d > canon && *--d != '/')
							continue;
						s = ++d;
						/*
						 * If the sum of the resolved
						 * part, the readlink and the
						 * remaining unresolved part
						 * exceeds MAXPATHLEN, the extra
						 * bytes will be dropped off.
						 */
						if (strlen(link) >= (limit - s)) {
							(void) strncpy(s, link, limit - s);
							*(limit - 1) = '\0';
						} else {
							(void) strcpy(s, link);
						}
					}
					continue;
				} else {
					/*
					 * readlink call failed. It can be
					 * because it was not a link (i.e. a
					 * file, dir etc.) or because the
					 * the call actually failed.
					 */
					if (errno != EINVAL)
						return (-1);
					*(d - 1) = '/';     /* Restore it */
				}
			} /* if (dosymlinks) */
		}
	} /* while */

	/* Remove the trailing slash that was added above. */
	if (*(d - 1) == '/' && d > canon + 1)
		d--;
	*d = '\0';
	return (0);
}

/*
 * Canonicalize the path given in raw, resolving away all symbolic link
 * components.  Store the result into the buffer named by canon, which
 * must be long enough (MAXPATHLEN bytes will suffice).  Returns NULL
 * on failure and canon on success.
 *
 * The routine indirectly invokes the readlink() system call and getwd()
 * so it inherits the possibility of hanging due to inaccessible file
 * system resources.
 */
char *
realpath(raw, canon)
	char        *raw;
	char        *canon;
{
	return (pathcanon(raw, canon, 1) < 0 ? NULL : canon);
}

/*
 * routines used to see if partitions overlap, i.e. we don't want to
 * simultaneously mount overlapping partitions.  this code is 'borrowed'
 * from newfs/newfs.c:
 *	ultrix_style()
 *	init_partid()
 *	check_for_overlap()
 *	verify_ok()
 *	swap_partition_name()
 * Detect overlapping mounted partitions.
 *
 * This code is also used almost verbatim in usr/sbin/newfs/newfs.c,
 * usr/sbin/swapon/swapon.c, and usr/sbin/ufs_fsck/setup.c
 * Change all places!
 * (XXX maybe we should pull this out into a shared source file?)
 *
 * XXX - Only 8 partitions are used in overlap detection for ultrix style,
 *       otherwise MAXPARTITIONS are used.
 */
#undef	fs_bsize	/* overlaps <nfs/nfs.h> */
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <ufs/fs.h>
#include <sys/table.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

struct ufs_partid {
	dev_t lvm_or_lsm_dev;		/* lvm/lsm device		*/
	short bus;			/* Bus				*/
	short adpt_num;			/* Adapter number		*/
	short nexus_num;		/* Nexus or node on adapter no.	*/
	short bus_num;			/* Bus number			*/
	short ctlr_num;			/* Controller number		*/
	short rctlr_num;		/* Remote controller number	*/
	short slave_num;		/* Plug or line number		*/
	short unit_num;			/* Ultrix device unit number	*/
	long  category_stat;		/* Category specific mask	*/
	struct pt {
		long part_blkoff;	/* Beginning partition sector	*/
		long part_nblocks;	/* Number of sectors in partition */
	} pt_part[MAXPARTITIONS];
} ufs_partid;

#define DEV_NAME_LSM	"LSM"
#define DEV_NAME_LVM	"LVM"

devget_to_partid(fd, devget, partid)
	int fd;
	struct devget *devget;
	struct ufs_partid *partid;
{
#ifdef DEBUG
    printf("devget_to_partid\n");
#endif
    bzero(partid, sizeof(*partid));
    if (strncmp(devget->dev_name, DEV_NAME_LVM, strlen(DEV_NAME_LVM)) == 0 ||
	strncmp(devget->dev_name, DEV_NAME_LSM, strlen(DEV_NAME_LSM)) == 0)
    {
	struct stat info;

	if (fstat(fd, &info) < 0)
		return(-1);
	partid->lvm_or_lsm_dev = info.st_rdev;
#ifdef DEBUG
	printf("lvm/lsm partition %d, %d\n",
		major(partid->lvm_or_lsm_dev),
		minor(partid->lvm_or_lsm_dev));
#endif
    } else
    {
        partid->bus = devget->bus;
        partid->adpt_num = devget->adpt_num;
        partid->nexus_num = devget->nexus_num;
        partid->bus_num = devget->bus_num;
        partid->ctlr_num = devget->ctlr_num;
        partid->rctlr_num = devget->rctlr_num;
        partid->slave_num = devget->slave_num;
        partid->unit_num = devget->unit_num;
        partid->category_stat = devget->category_stat & (MAXPARTITIONS - 1);
    }
    return (0);
}

print_partid(device, partid)
	char *device;
	struct ufs_partid *partid;
{
	int ind;

	printf("device %s\n", device);
	if (partid->lvm_or_lsm_dev)
		printf("\tLVM/LSM %d, %d\n",
			major(partid->lvm_or_lsm_dev),
			minor(partid->lvm_or_lsm_dev));
	else
	{
		printf("\tbus %d\n", partid->bus);
		printf("\tadpt_num %d\n", partid->adpt_num);
		printf("\tnexus_num %d\n", partid->nexus_num);
		printf("\tbus_num %d\n", partid->bus_num);
		printf("\tctlr_num %d\n", partid->ctlr_num);
		printf("\trctlr_num %d\n", partid->rctlr_num);
		printf("\tslave_num %d\n", partid->slave_num);
		printf("\tunit_num %d\n", partid->unit_num);
		printf("\tcategory_stat %d\n", partid->category_stat);
		for (ind = 0; ind < MAXPARTITIONS; ind++)
			printf("\tpt_offset %d length %d\n",
		       		partid->pt_part[ind].part_blkoff,
		       		partid->pt_part[ind].part_nblocks);
	}
	printf("\n");
}

#define PT_MAGIC        0x032957        /* Partition magic number */
#define PT_VALID        1               /* Indicates if struct is valid */

/*
 * Structure that is used to determine the partitioning of the disk.
 * It's location is at the end of the superblock area.
 * The reason for both the cylinder offset and block offset
 * is that some of the disk drivers (most notably the uda
 * driver) require the block offset rather than the cyl.
 * offset.
 */
struct ult_pt {
	int	pt_magic;       /* magic no. indicating part. info exits */
	int     pt_valid;       /* set by driver if pt is current */
	struct  pt_info {
		int     pi_nblocks;     /* no. of sectors for the partition */
		daddr_t pi_blkoff;      /* block offset for start of part. */
	} pt_part[8];
};

ultrix_style(device, partid, dl)
	char *device;
	struct ufs_partid *partid;
	struct disklabel *dl;
{
	struct fs *fs;
	struct ult_pt *pt;
	char buf[32];
	char sb[SBSIZE];
	int len, ind, fd;

	len = strlen(device);
	strcpy(buf, device);
	buf[len - 1] = 'c';

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return(-1);
	lseek(fd, SBOFF, 0);
	if (read(fd, sb, SBSIZE) != SBSIZE) {	
		close(fd);
		return(-1);
	}
	close(fd);

	fs = (struct fs *)sb;
	if (fs->fs_magic != FS_MAGIC)
		return(-1);

	pt = (struct ult_pt *)&sb[8192 - sizeof(struct ult_pt)];
	if (pt->pt_magic != PT_MAGIC || pt->pt_valid != PT_VALID)
		return(-1);

	/*
	 * Valid ULTRIX Super block and partition table
	 */
	for (ind = 0; ind < 8; ind++) {
		dl->d_partitions[ind].p_offset = pt->pt_part[ind].pi_blkoff;
		dl->d_partitions[ind].p_size = pt->pt_part[ind].pi_nblocks;
	}
	return(0);
}

/*
 * Return -1 if partid could not be initialized.
 * Rerurn 0 if partid was initialized.
 */
init_partid(file, partid)
	char *file;
	struct ufs_partid *partid;
{
	int fd, ind;
	struct devget devget;
	struct disklabel disklabel, *dl;
	

	fd = open(file, O_RDONLY);
	if (fd < 0)
		return(-1);
	/*
	 * Call drive to fill in devget struct
	 */
	if (ioctl(fd, DEVIOCGET, (char *)&devget) < 0) {
		close(fd);
		return(-1);
	}
	if (devget_to_partid(fd, &devget, partid) < 0)
	{
		close(fd);
		return(-1);
	}

	/*
	 * If logical volume then we are finished.
	 */
	if (partid->lvm_or_lsm_dev)
	{
		close(fd);
		return(0);
	}	

	/*
	 * Get partition table info for drive:
	 *
	 *	Check for a disklabel
	 * 	Check for ULTRIX style partition table
	 *	Use disktab
	 */
	dl = &disklabel;
	if (ioctl(fd, DIOCGDINFO, dl) < 0 && ultrix_style(file, partid, dl) < 0)
		dl = (struct disklabel *)getdiskbyname(devget.device);

	close(fd);

	/*
	 * If no partition table found, no testing possible.
	 */
	if (!dl)
		return(-1);

	for (ind = 0; ind < MAXPARTITIONS; ind++) {
		partid->pt_part[ind].part_blkoff = dl->d_partitions[ind].p_offset;
		partid->pt_part[ind].part_nblocks = dl->d_partitions[ind].p_size;
	}
	return(0);
}

/*
 * Code to form the swap partition name lifted from usr/sbin/swapon/swapon.c.
 * Code to call table() inspired by swapon.c
 *
 * verify_ok() is common code to both swap checking and mounted file checking
 * code; it formerly was inline in check_for_overlap().
 */

static  char   *swap_partition_name(dev_t);
static	int    verify_ok(struct ufs_partid *,
			 const char *,
			 const char *,
			 const char *,
			 const char *);

/*
 * Return non zero if overlap.
 */
check_for_overlap(device)
	char *device;
{
	struct ufs_partid *partid = &ufs_partid;
	struct statfs *mntbuf;
	char *raw;
	int i, mntsize, ret;

	ret = 0;
	/* convert block device to char device for init_partid(),
	 * just in case the block device is somehow in-use(ebusy).
	 */
	raw = rawname(device);
	if (init_partid(raw, partid))
		return(0);

	/* Get swap info & verify non-overlapping; code lifted from swapon.c */
	for (i = 0;; i++) {
	    struct tbl_swapinfo swapinfo;
	    char *name;

	    /*
	     ** Attempt to read the swap information.
	     */
	    if (table(TBL_SWAPINFO, i, &swapinfo, 1, sizeof(swapinfo)) < 0)
		break;

	    /*
	     ** check this swap partition.
	     */
	   
	    if ((name = swap_partition_name(swapinfo.dev)) == NULL) {
		/* Name is not in a standard place. */	
		continue; 
	    }
	    ret += verify_ok(partid, device, name, "swap", "in use as");
	}

	if ((mntsize = getmntinfo(&mntbuf, MNT_NOWAIT)) == 0)
		return(ret);
	for (i = 0; i < mntsize; i++) {
		if (mntbuf[i].f_type != MOUNT_UFS)
			continue;
		ret += verify_ok(partid, device, mntbuf[i].f_mntfromname,
				 mntbuf[i].f_mntonname, "mounted on");
	}
	return(ret);		
}

getffmopts(options, argsp)
        char *options;
        struct ffm_args *argsp;
{
        register char *cp, *nextcp;
        int num;

        argsp->ffm_flags = 0;
        cp = options;
        while (cp != NULL && *cp != '\0') {
                if ((nextcp = index(cp, ',')) != NULL)
                        *nextcp++ = '\0';
                /*
                 * Just test for a string match and do it
                 */
                if (!strcasecmp(cp, "clone")) {
                        argsp->ffm_flags |= FFM_CLONE;
                } else {
                        fprintf(stderr,"illegal option: %s\n", cp);
                        exit(1);
                }
                cp = nextcp;
        }
}

/*
 * Return 0 if no overlap or if we could not tell
 *	if overlapped or not.
 * Return 1 if overlap
 *
 * WARNING:
 *	THIS CODE DOES NOT DETECT AN ATTEMPT TO USE
 *	A PARTITION WHICH IS USE BY THE LVM.
 *
 *	THIS CODE DOES NOT DETECT AN ATTEMPT TO USE
 *	A PARTITION WHICH WOULD OVERLAP A PARTITION
 *	WHICH IS IN USE BY THE LVM.
 *
 *	AT PRESENT NO EASY WAY EXISTS TO PERFORM
 *	THESE TESTS
 *
 */  
static int
verify_ok(struct ufs_partid *partid,
	  const char *device,
	  const char *blockdev,
	  const char *usename,
	  const char *msg)
{
    struct devget mnt_devget;
    struct ufs_partid mnt_partid;
    int fd, ret = 0;
    long bot, top, mbot, mtop;
    char *raw = rawname(blockdev);

    fd = open(raw, O_RDONLY);
    if (fd < 0)
	return (0);
    /*
     * Call drive to fill in devget struct
     */
    if (ioctl(fd, DEVIOCGET, (char *)&mnt_devget) < 0) {
	close(fd);
	return (0);
    }

    /*
     * Extract what we need
     */
    if (devget_to_partid(fd, &mnt_devget, &mnt_partid) < 0)
    {
	close(fd);
	return(0);
    }
    close(fd);

    /*
     * Either one a logical volume ?
     */
    if (mnt_partid.lvm_or_lsm_dev || partid->lvm_or_lsm_dev)
    {
	/*
	 * Same logical volume ?
	 */
    	if (mnt_partid.lvm_or_lsm_dev == partid->lvm_or_lsm_dev)
	{
		/*
		 * if exactly the same device, don't complain,
		 * let mount(2) die.
		 */
		return (0);
	}
	/*
	 * Assume no overlap if different logical volume or
	 * only one logical volume.
	 */
	return(0);	
    }
    
    /*
     * Check for drive match
     */
    if (mnt_partid.bus != partid->bus ||
	mnt_partid.adpt_num != partid->adpt_num ||
	mnt_partid.nexus_num != partid->nexus_num ||
	mnt_partid.bus_num != partid->bus_num ||
	mnt_partid.ctlr_num != partid->ctlr_num ||
	mnt_partid.rctlr_num != partid->rctlr_num ||
	mnt_partid.slave_num != partid->slave_num ||
	mnt_partid.unit_num != partid->unit_num)
	return (0);

    bot = partid->pt_part[partid->category_stat].part_blkoff;
    top = bot + partid->pt_part[partid->category_stat].part_nblocks - 1;

    mbot = partid->pt_part[mnt_partid.category_stat].part_blkoff;
    mtop = mbot + partid->pt_part[mnt_partid.category_stat].part_nblocks - 1;
    if (bot == mbot && top == mtop) {
	/* if exactly the same device, don't complain, let mount(2) die. */
	struct stat stb1, stb2;
	if (stat(blockdev, &stb1) == 0 && stat(device, &stb2) == 0) {
	    if (stb1.st_rdev == stb2.st_rdev)
		return (0);
	}
    }
    /*
     * If same partition or partitions overlap,
     * return true
     */
    if (top > 0 && mtop > 0 &&
	(mnt_partid.category_stat == partid->category_stat ||
	 bot >= mbot && bot <= mtop ||
	 top >= mbot && top <= mtop ||
	 mbot >= bot && mbot <= top)) {
	if (ret++ == 0) {
	    fprintf(stderr, "New filesystem would overlap mounted filesystem(s) or active swap area\n");
	    fprintf(stderr, "Unmount required before mounting %s (start %d end %d)\n", device, bot, top);
	}
	fprintf(stderr, "\t%s %s %s (start %d end %d)\n",
	       blockdev, msg, usename, mbot, mtop);
    }
    return (ret);
}

static char *
swap_partition_name(dev_t dev)
{
  static char device_name[256+4+1];     /* `/dev/file-name-up-to-255-characters'<NUL> */
  DIR *dir;
  struct dirent *dirent;

  /*
  ** Find the device in the /tmp directory.
  */
  if (!(dir = opendir("/dev")))
    return (NULL);

  /*
  ** Directory is accessed, read through each name in `/dev', and
  ** stat the file to obtain the (possible) major/minor numbers.
  ** When we have a block device which matches, return its name.
  */
  while (dirent = readdir(dir)) {
    /*
    ** Got some directory information.  Check for a block special
    ** with matching major and minor numbers.  If (when) found,
    ** return a pointer to the generated name.
    */
    struct stat local_stat;

    sprintf(device_name, "/dev/%s", dirent->d_name);
    if (!stat(device_name, &local_stat)) {
      if (S_ISBLK(local_stat.st_mode) &&
          major(local_stat.st_rdev) == major(dev) &&
          minor(local_stat.st_rdev) == minor(dev)) {
        closedir(dir);
        return (device_name);
        }
      }
    }
  closedir(dir);
  return (NULL);
}

#define	LSM_CDEV	"rvol"
#define	LSM_CDEV_LEN	4
#define	LSM_BDEV	"vol"
#define	LSM_BDEV_LEN	3

char *
unrawname(name)
	char *name;
{
	char *dp;
	char *ddp;
	char *p;
	struct stat stb;

	if (stat(name, &stb) < 0)
		return (name);
	if ((stb.st_mode & S_IFMT) != S_IFCHR)
		return (name);

	for (;;)	/* just so we can use break */
	{
		/* see if any '/' */
		if ((dp = rindex(name, '/')) == 0)
		{
			dp = name-1;	/* simple name */
			break;
		}

		/* look for a second slash */
		p = dp-1;
		ddp = 0;
		while (p >= name)
		{
			if (*p == '/')
			{
				ddp = p;
				break;
			}
			p--;
		}
		if (ddp)
		{
			/* look for "rvol" */
			p = ddp - LSM_CDEV_LEN;
			/* is place we are looking at valid ? */
			if (p == name || (p > name && p[-1] == '/'))
			{
				/* actually look for string */
				if (strncmp(p, LSM_CDEV, LSM_CDEV_LEN) == 0)
				{
					dp = p-1; /* name preceeded by rvol/xxxx/ */
					break;
				}
			}
		}

		/* look for "rvol" */
		p = dp - LSM_CDEV_LEN;
		/* is place we are looking at valid ? */
		if (p == name || (p > name && p[-1] == '/'))
		{
			/* actually look for string */
			if (strncmp(p, LSM_CDEV, LSM_CDEV_LEN) == 0)
			{

				dp = p-1; /* name preceeded by rvol/ */
				break;
			}
		}

		break;
	}
	
	if (*(dp + 1) != 'r')
		return (name);
	(void)strcpy(dp + 1, dp + 2);
	return (name);
}

char *
rawname(name)
	char *name;
{
	static char rawbuf[MAXPATHLEN];
	char *dp;
	char *ddp;
	char *p;

	for (;;)	/* just so we can use break */
	{
		/* see if any '/' */
		if ((dp = (char *)rindex(name, '/')) == 0)
		{
			dp = name-1;	/* a simple name */
			break;
		}

		/* look for a second '/' */
		p = dp-1;
		ddp = 0;
		while (p >= name)
		{
			if (*p == '/')
			{
				ddp = p;
				break;
			}
			p--;
		}
		if (ddp)
		{
			/* look for "vol" */
			p = ddp - LSM_BDEV_LEN;
			/* is place we are looking at valid ? */
			if (p == name || (p > name && p[-1] == '/'))
			{
				/* actually look for string */
				if (strncmp(p, LSM_BDEV, LSM_BDEV_LEN) == 0)
				{
					dp = p-1; /* name preceeded by vol/xxxx/ */
					break;
				}
			}
		}

		/* look for "vol" */
		p = dp - LSM_BDEV_LEN;
		/* is place we are looking at valid ? */
		if (p == name || (p > name && p[-1] == '/'))
		{
			/* actually look for string */
			if (strncmp(p, LSM_BDEV, LSM_BDEV_LEN) == 0)
			{
				dp = p-1; /* a name preceeded by vol/ */
				break;
			}
		}

		break;
	}
	
	dp++;
	memcpy(rawbuf, name, dp - name);
	strcpy(rawbuf + (dp-name), "r");
	strcat(rawbuf, dp);

	return (rawbuf);
}

