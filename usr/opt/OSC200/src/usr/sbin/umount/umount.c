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
static char	*sccsid = "@(#)$RCSfile: umount.c,v $ $Revision: 4.2.9.4 $ (DEC) $Date: 1993/12/09 20:49:26 $";
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
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#endif
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/fs_types.h>

#ifdef NFS
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_prot.h>
#include <nfs/rpcv2.h>
#endif

#include <errno.h>
#include <rpcsvc/mount.h>
#include <fstab.h>
#include <stdio.h>
#include <string.h>


#define MNTFROMSIZE	sizeof(((struct statfs *) 0) -> f_mntfromname)
#define MNTONSIZE	sizeof(((struct statfs *) 0) -> f_mntonname)

#ifdef NFS
/* 
 * need our own version of authunix_create_default() to limit # groups 
 * RPC can't handle more than 8
 */
AUTH *authunix_default(); /* need our own version to limit # groups */
int xdr_dir();
char *nfshost;
#endif

int	vflag, all, errs, fake, All, broadcast_all = 0;
int	fflag = MNT_NOFORCE;
char	*getmntname();

int *typelist, *maketypelist();
void send_umntall();

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	int ch;

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif
	sync();
	while ((ch = getopt(argc, argv, "AabfFh:t:v")) != EOF)
		switch((char)ch) {
		case 'v':
			vflag++;
			break;
		case 'f':
			fflag = MNT_FORCE;
			break;
		case 'F':
			fake++;
			break;
		case 'A':
			All++;
			break;
		case 'a':
			all++;
			break;
		case 'b':
			/* broadcast "unmount all" message */
			broadcast_all++;
			break;
		case 't':
			typelist = maketypelist(optarg);
			break;
#ifdef	NFS
		case 'h':
			/* -h flag implies -a, and "-t nfs" if no -t flag */
			nfshost = optarg;
			all++;
			if (typelist == NULL)
				typelist = maketypelist("nfs");
			break;
#endif /* NFS */
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	if (broadcast_all) {
	  if (super_user())
	    send_umntall();
	  else
	    fprintf(stderr, "Must be super-user to broadcast\n");
	  if (!(vflag || all ||  errs || fake || All ))
	    if (fflag == MNT_NOFORCE)
	      exit(0);
        }

	if (argc == 0 && !(all || All))
		usage();
	if (All) {
		if (argc > 0)
			usage();
		Umountall(typelist);
		exit(0);
	}
	if (all) {
		if (argc > 0)
			usage();
		if (setfsent() == 0)
			perror(FSTAB), exit(1);
		umountall(typelist);
		exit(0);
	} else
		setfsent();
	while (argc > 0) {
		if (umountfs(*argv++, 0, 0, 0, 0) == 0)
			errs++;
		argc--;
	}
	exit(errs);
}

usage()
{
	fprintf(stderr,
		"%s\n%s\n%s\n",
		"Usage: umount [-fv] special | node",
#ifndef	NFS
		"    or umount -a[fv] [-t fstypelist]",
		"    or umount -b"
#else
		"    or umount -a[fv] [-h host] [-t fstypelist]",
		"    or umount -b"
#endif
	);
	exit(1);
}

Umountall(typelist)
	int *typelist;
{
	struct statfs *mntbuf;
	int i, mntsize;

#if SEC_BASE
	umount_checkauth();
#endif
	if ((mntsize = getmntinfo(&mntbuf, MNT_NOWAIT)) == 0) {
		fprintf(stderr, "umount: cannot get mount information\n");
		exit(1);
	}
	for (i = mntsize - 1; i; i--) {
#ifdef NFS
		if (mntbuf[i].f_type == MOUNT_NFS &&
		    mntbuf[i].mount_info.nfs_args.flags & NFSMNT_AUTO)
			continue; /* Mount point owned by automount */
#endif
		(void) umountfs(mntbuf[i].f_mntonname, mntbuf[i].f_mntfromname,
				(int)mntbuf[i].f_type, typelist, 1);
	}
}

umountall(typelist)
	int *typelist;
{
	register struct fstab *fs;
	struct fstab *allocfsent();
	int type;

#if SEC_BASE
	umount_checkauth();
#endif
	if ((fs = getfsent()) == (struct fstab *)0)
		return;
	fs = allocfsent(fs);
	umountall(typelist);
	if (strcmp(fs->fs_file, "/") == 0) {
		freefsent(fs);
		return;
	}
	if (strcmp(fs->fs_type, FSTAB_RW) &&
	    strcmp(fs->fs_type, FSTAB_RO) &&
	    strcmp(fs->fs_type, FSTAB_RQ)) {
		freefsent(fs);
		return;
	}
	type = getvfstype(fs->fs_vfstype);
	/*
	 * cause umountfs to skip some work if NFS is involved, to
	 * avoid hung servers.
	 */
	(void) umountfs(fs->fs_file, fs->fs_spec,
			type, typelist, type == MOUNT_NFS);
	freefsent(fs);
}

struct fstab *
allocfsent(fs)
	register struct fstab *fs;
{
	register struct fstab *new;
	register char *cp;
	void *malloc();

	new = (struct fstab *)malloc((unsigned)sizeof (*fs));
	cp = malloc((unsigned)strlen(fs->fs_file) + 1);
	strcpy(cp, fs->fs_file);
	new->fs_file = cp;
	cp = malloc((unsigned)strlen(fs->fs_type) + 1);
	strcpy(cp, fs->fs_type);
	new->fs_type = cp;
	cp = malloc((unsigned)strlen(fs->fs_vfstype) + 1);
	strcpy(cp, fs->fs_vfstype);
	new->fs_vfstype = cp;
	cp = malloc((unsigned)strlen(fs->fs_spec) + 1);
	strcpy(cp, fs->fs_spec);
	new->fs_spec = cp;
	new->fs_passno = fs->fs_passno;
	new->fs_freq = fs->fs_freq;
	return (new);
}

freefsent(fs)
	register struct fstab *fs;
{

	if (fs->fs_file)
		free(fs->fs_file);
	if (fs->fs_spec)
		free(fs->fs_spec);
	if (fs->fs_type)
		free(fs->fs_type);
	free((char *)fs);
}

char canonb[MAXPATHLEN + 1];

umountfs(name, from, ftype, typelist, trustme)
	char *name;
	char *from;
	int ftype;
	int *typelist;
	int trustme;
{
	char *mntpt;
	struct stat stbuf;
	int type;
	int doagain;
	extern char *realpath();
	char *canon;
#ifdef NFS
	register CLIENT *clp;
	struct hostent *hp = 0;
	struct sockaddr_in saddr;
	struct timeval pertry, try;
	enum clnt_stat clnt_stat;
	int so = RPC_ANYSOCK;
	char *hostp, *delimp;
#endif /* NFS */

	if (fflag & MNT_FORCE)
		doagain = 0;
	else
		doagain = 1;
	/*
	 * {U,u}mountall call with trustme set, since it has definitive
	 * information, which can be trusted.
	 */
	if (trustme) {
		mntpt = name;
		name = from;
		type = ftype;
	} else {
		if ((mntpt = getmntname(&name, &type, doagain)) == (char *)0)
		{
			if ((fflag & MNT_FORCE) == 0)
				return(0);
			/*
			 * Assume the name the user supplied is a the mount point.
			 * We really do not know what kind of mount it is.
			 */
			mntpt = name;
			name = "";
			type = MOUNT_NONE;
		}
	}

	if (badtype(type, typelist))
		return(1);
#ifdef NFS
	/*
	 * Parse hostname and file system.  For those cases where we'll
	 * need the host address, get it.  If we don't need it, don't
	 * call gethostbyname() so we don't risk accessing a hung network.
         */
        if (type == MOUNT_NFS) {
		if ((delimp = index(name, '@')) != NULL) {
			hostp = delimp + 1;
			if (nfshost || !(fflag & MNT_FORCE))
				hp = gethostbyname(hostp);
		} else if ((delimp = index(name, ':')) != NULL) {
			hostp = name;
			if (nfshost || !(fflag & MNT_FORCE)) {
				*delimp = '\0';
				hp = gethostbyname(hostp);
				*delimp = ':';
			}
			name = delimp+1;
		}
	
		/* only check for a name match if a host name was
		 * supplied on the command line and we're not forcing
		 * unmounts.  Otherwise, go ahead and umount without
		 * checking.
		 */
		if (nfshost && hp)
		    if (!namematch(hp, nfshost))
			return(1);
	}
#endif	/* NFS */
#if SEC_BASE
	if (!fake && umount_do_umount(mntpt, fflag) < 0)
#else
	if (!fake && umount(mntpt, fflag) < 0)
#endif
	{
		perror(mntpt);
		return (0);
	}
	if (vflag) {
#ifdef  NFS
                if (type == MOUNT_NFS)
                        fprintf(stderr, "%s:", hostp);
#endif
		if (*name)
			fprintf(stderr, "%s: Unmounted from %s\n", name, mntpt);
		else
			fprintf(stderr, "Unmounted from %s\n", mntpt);
	}

#ifdef	NFS
	if (!fake && hp != NULL && (fflag & MNT_FORCE) == 0) {
		*delimp = '\0';
		bcopy(hp->h_addr,(caddr_t)&saddr.sin_addr,hp->h_length);
		saddr.sin_family = AF_INET;
		saddr.sin_port = 0;
		pertry.tv_sec = 3;
		pertry.tv_usec = 0;
		if ((clp = clntudp_create(&saddr, RPCPROG_MNT, RPCMNT_VER1,
		    pertry, &so)) == NULL) {
			clnt_pcreateerror("Cannot contact server");
			return (1);
		}
#ifdef notyet
/*
 * should be able to use the REAL one when we get up to rpc 4.0
 */
		clp->cl_auth = authunix_create_default();
#else
		clp->cl_auth = authunix_default();
#endif
		/*
                 * Since this rpc is advisory only, let's not
                 * try too hard.  It can cause unmounts to hang
                 * unnecessarily.  So we set timeout to 0, which
                 * means send only and quit; don't worry about
                 * replies (we get RPC_TIMEDOUT error in this case,
                 * which are ignored).
                 */
		try.tv_sec = 0; /* no timeout == send only */
		try.tv_usec = 0;
		clnt_stat = clnt_call(clp, RPCMNT_UMOUNT, xdr_dir, name,
			xdr_void, (caddr_t)0, try);
		if ((clnt_stat != RPC_SUCCESS) &&
		    (clnt_stat != RPC_TIMEDOUT)) {
			clnt_perror(clp, "Bad UMOUNT RPC");
			return (1);
		}
		auth_destroy(clp->cl_auth);
		clnt_destroy(clp);
	}
#endif /* NFS */
	return (1);
}

char *
getmntname(name, type, doagain)
	char **name;
	int *type;
	int doagain;
{
	int mntsize, i;
	struct statfs *mntbuf;
	extern char *realpath();
	char *canon;
	int len = strlen(*name);

	static char mntfrom[MNTFROMSIZE];
	static char mnton[MNTONSIZE];

	if (len > MNTFROMSIZE && len > MNTONSIZE)
	{
		if (fflag != MNT_FORCE)
			fprintf(stderr,
				"Name too long for mount table, use -f flag:\n%s\n",
				*name);
		return (0);
	}

	if ((mntsize = getmntinfo(&mntbuf, MNT_NOWAIT)) == 0) {
		perror("umount");
		return (0);
	}
again:
	for (i = 0; i < mntsize; i++) {
		/* arg == local mntpt */
		if (len <= MNTONSIZE &&
			strncmp(mntbuf[i].f_mntonname, *name, MNTONSIZE)==0) {
			*type = mntbuf[i].f_type;
			mntfrom[0] = 0;
			strncat(mntfrom, mntbuf[i].f_mntfromname, MNTFROMSIZE);
			if (vflag && strlen(mntfrom) == MNTFROMSIZE)
				fprintf(stderr,
					"%s: may be truncated\n", mntfrom);
			*name = mntfrom;
			mnton[0] = 0;
			strncat(mnton, mntbuf[i].f_mntonname, MNTONSIZE);
			if (vflag && strlen(mnton) == MNTONSIZE)
				fprintf(stderr,
					"%s: may truncated\n", mnton);
			return (mnton);
		}
		/* arg == blk spec or server:path */
		if (len <= MNTFROMSIZE &&
			strncmp(mntbuf[i].f_mntfromname, *name, MNTFROMSIZE)==0) {
			*type = mntbuf[i].f_type;
			mnton[0] = 0;
			strncat(mnton, mntbuf[i].f_mntonname, MNTONSIZE);
			if (vflag && strlen(mnton) == MNTONSIZE)
				fprintf(stderr,
					"%s: may truncated\n", mnton);
			return (mnton);
		}
	}
	if (doagain) {
		doagain = 0;
		canon = realpath(*name, canonb);
		if (canon) {
			*name = canon;
			goto again;
		}
	}
	fprintf(stderr, "%s: not currently mounted\n", *name);
	return (0);
}

static int skipvfs;

badtype(type, typelist)
	int type;
	int *typelist;
{
	if (typelist == 0)
		return(0);
	while (*typelist) {
		if (type == *typelist)
			return(skipvfs);
		typelist++;
	}
	return(!skipvfs);
}

int
getvfstype(type)
	char *type;
{
	int rtype;

	for(rtype = MOUNT_UFS; rtype <= MOUNT_MAXTYPE; rtype++)
		if(!strcmp(type,mnt_names[rtype]))
			return(rtype);
	return MOUNT_NONE;
}

int *
maketypelist(fslist)
	char *fslist;
{
	register char *nextcp;
	void *malloc();
	register int *av, i, type;

	if (fslist == NULL)
		return(NULL);
	if (fslist[0] == 'n' && fslist[1] == 'o') {
		fslist += 2;
		skipvfs = 1;
	} else
		skipvfs = 0;
	for (i = 0, nextcp = fslist; *nextcp; nextcp++)
		if (*nextcp == ',')
			i++;
	av = (int *)malloc((i+2) * sizeof(int));
	if (av == NULL)
		return(NULL);
	for (i = 0; fslist; fslist = nextcp) {
		if (nextcp = index(fslist, ','))
			*nextcp++ = '\0';
		if ((type = getvfstype(fslist)) != MOUNT_NONE)
			av[i++] = type;
	}
	av[i++] = 0;
	return(av);
}

#ifdef	NFS
namematch(hp, nfshost)
	struct hostent *hp;
	char *nfshost;
{
	register char *cp;
	register char **np;

	if (hp == NULL || nfshost == NULL)
		return(1);
	if (strcasecmp(nfshost, hp->h_name) == 0)
		return(1);
	if (cp = index(hp->h_name, '.')) {
		*cp = '\0';
		if (strcasecmp(nfshost, hp->h_name) == 0)
			return(1);
	}
	for (np = hp->h_aliases; *np; np++) {
		if (strcasecmp(nfshost, *np) == 0)
			return(1);
		if (cp = index(*np, '.')) {
			*cp = '\0';
			if (strcasecmp(nfshost, *np) == 0)
				return(1);
		}
	}
	return(0);
}

/*
 * xdr routines for mount rpc's
 */
xdr_dir(xdrsp, dirp)
	XDR *xdrsp;
	char *dirp;
{
	return (xdr_string(xdrsp, &dirp, RPCMNT_PATHLEN));
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

void
send_umntall()
{
        (void) clnt_broadcast(MOUNTPROG, MOUNTVERS, MOUNTPROC_UMNTALL,
                xdr_void, (caddr_t)0, xdr_void, (caddr_t)0, (caddr_t)0);
}


int
super_user()
{
	if ((getuid() == 0) && ( geteuid() == 0) ){
		return 1;  /* super user */
	}
	return 0; /* not su */
}
