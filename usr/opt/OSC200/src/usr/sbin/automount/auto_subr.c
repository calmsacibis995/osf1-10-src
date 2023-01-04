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
#ifdef ultrix
static char     *sccsid = "%W%  ULTRIX  %G%";
#else /* ultrix */
static char     *sccsid = "@(#)$RCSfile: auto_subr.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/11 17:32:14 $";
#endif /* ultrix */
#endif /* lint */

/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

/*
 *	Revision History
 *
 *  06 Nov 91 -- condylis
 *	noconn is default mount option now.
 *
 */

#include <sys/param.h>
#include <sys/mount.h>
#include <rpc/rpc.h>
#include <sys/errno.h>
#include <sys/time.h>
#ifdef ultrix				/* JFS */
typedef quad fsid_t;                    /* file system id type */
#include "auto_ultrix.h"  
#include <nfs/nfs_gfs.h>
#endif /* ultrix */
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include "mntent.h"
#include "nfs_prot.h"
#include "automount.h"
#include <rpcsvc/mount.h>

extern int trace;
extern int verbose;
char tmpopts[1024];

char	*index();


static char *
mntopt(p)
char **p;
{
	char *cp = *p;
	char *retstr;
	while (*cp && isspace(*cp))
		cp++;
	retstr = cp;
	while (*cp && *cp != ',')
		cp++;
	if (*cp) {
		*cp = '\0';
		cp++;
	}
	*p = cp;
	return (retstr);
}

char *
hasmntopt(mnt, opt)
register struct mntent *mnt;
register char *opt;
{
	char *f, *o;

	strcpy(tmpopts, mnt->mnt_opts);
	o = tmpopts;
	f = mntopt(&o);
	for (; *f; f = mntopt(&o)) {
		if (strncmp(opt, f, strlen(opt)) == 0)
			return (f - tmpopts + mnt->mnt_opts);
	} 
	return (NULL);
}

removemntopt(mnt,opt)
register struct mntent *mnt;
register char *opt;
{
	char *optp, *optend;

	if ((optp = hasmntopt(mnt, opt)) != NULL) {
		optend = index(optp, ',');
		if (optp != mnt->mnt_opts) {
			optp--;
			if (optend == NULL)
				*optp = '\0';
		}
		else {
			if (optend == NULL)
				*optp = '\0';
			else
				optend++;
		}
		if (optend != NULL)			
			while (*optp++ = *optend++)
				;
		return (1);
	}
	else
		return (0);
}

/*
 * Returns true if s1 is a pathname substring of s2.
 */
substr(s1, s2)
char *s1;
char *s2;
{
	while (*s1 == *s2) {
		s1++;
		s2++;
	}
	if (*s1 == '\0' && *s2 == '/') {
		return (1);
	}
	return (0);
}

getstdopts(options, flagp)
        char *options;
        int *flagp;
{
        register char *opt, *start;
        register char *leftopts = options;
        int negative = 0;
        char optbuf[BUFSIZ];

        (void)strcpy(optbuf, options);
        for (opt = strtok(optbuf, ","); opt; opt = strtok((char *)NULL, ",")) {
                start = opt;
                if (opt[0] == 'n' && opt[1] == 'o') {
                        negative++;
                        opt += 2;
                } else {
                        negative = 0;
                }
                if (!negative && !strcasecmp(opt, "ro")) {
                        *flagp |= M_RDONLY;
                        continue;
                }
                else if (!negative && !strcasecmp(opt, "rw")) {
#ifdef ultrix
                        *flagp &= ~M_RONLY;
#else /* ultrix */
                        *flagp &= ~M_RDONLY;
#endif /* ultrix */
                        continue;
                }
                else if (!strcasecmp(opt, "exec")) {
                        if (negative)
                                *flagp |= M_NOEXEC;
                        else
                                *flagp &= ~M_NOEXEC;
                        continue;
                }
                else if (!strcasecmp(opt, "suid")) {
                        if (negative)
                                *flagp |= M_NOSUID;
                        else
                                *flagp &= ~M_NOSUID;
                        continue;
                }
                else if (!strcasecmp(opt, "dev")) {
                        if (negative)
                                *flagp |= M_NODEV;
                        else
                                *flagp &= ~M_NODEV;
                        continue;
                }
                else if ((!strcasecmp(opt, "sync")) ||
                (!strcasecmp(opt, "synchronous"))) {
                        if (!negative)
                                *flagp |= M_SYNCHRONOUS;
                        else
                                *flagp &= ~M_SYNCHRONOUS;
                        continue;
                }
#ifdef ultrix
		/* JFS - Added support for grpid */
		else if (!strcasecmp(opt, "grpid")) {
                        if (!negative)
				*flagp |= M_GRPID;
                        else
                                *flagp &= ~M_GRPID;
			continue;
		}
#endif /* ultrix */
                if (leftopts != options)
                        *leftopts++ = ',';
                strcpy(leftopts, start);
                leftopts += strlen(start);
        }
        *leftopts = '\0';
}


getnfsopts(optarg, nfsargsp)
        char *optarg;
        struct nfs_args *nfsargsp;
{
        register char *cp, *nextcp;
        int num, valflt;
        char *nump;
        nfsargsp->flags |= (NFSMNT_INT | NFSMNT_NOCONN);

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

/* This is used to validate an option that specifies a numeric value    */
#define badval(cmp, val) ((val) cmp ? valflt=1 : (0))
                /*
                 * Just test for a string match and do it
                 */
                if (!strcasecmp(cp, "soft")) {
                        nfsargsp->flags |= NFSMNT_SOFT;
                } else if (!strcasecmp(cp, "hard")) {
                        nfsargsp->flags &= ~NFSMNT_SOFT;
                } else if (!strcasecmp(cp, "intr")) {
                        nfsargsp->flags |= NFSMNT_INT;
                } else if (!strcasecmp(cp, "nintr")) {
                        nfsargsp->flags &= ~NFSMNT_INT;
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
#define AC_MASK (NFSMNT_ACREGMAX | NFSMNT_ACDIRMAX | NFSMNT_ACREGMIN | NFSMNT_ACDIRMIN)
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
                } else if (valflt) {
			if (verbose)
				syslog(LOG_ERR, 
					"WARNING: bad value for %s option\n", 
					cp);
			if (trace > 1)
                        	fprintf(stderr, 
					"WARNING: bad value for %s option\n",
					cp);
                } else {
			if (verbose)
				syslog(LOG_ERR,
					"WARNING: unknown option %s\n", cp);
			if (trace > 1)
                        	fprintf(stderr, "WARNING: unknown option %s\n",
					cp);
		}
                cp = nextcp;
        }
        /* If no attribute cache selected and any option selected to    */
        /* adjust attribute cache operation                             */
        if ((nfsargsp->flags & NFSMNT_NOAC) && (nfsargsp->flags & AC_MASK)) {
		if (verbose)
			syslog(LOG_ERR,
				"Contradicting attr cache options\n");
		if (trace > 1)
                	(void) fprintf(stderr,
                		"Contradicting attr cache options\n");
		return(0);
        }
	return(1);
}


same_opts(opts, fs)
	char *opts;
	struct filsys *fs;
{
	int	flags = 0;
	struct nfs_args na;

	bzero(&na, sizeof(na));
	strcpy(tmpopts, opts);
	getstdopts(tmpopts, &flags);
	getnfsopts(tmpopts, &na);
	return(same_bopts(flags, fs->fs_mflags, &na, &fs->fs_nfsargs)); 
}


same_bopts(flag1, flag2, na1, na2)
	int	flag1, flag2;
	struct nfs_args	*na1, *na2;
{
	if (flag1 != flag2)
		return(0);

	if ((na1->flags & ~NFSMNT_HOSTNAME) != (na2->flags & ~NFSMNT_HOSTNAME))
		return(0);

	if ((na1->flags & NFSMNT_WSIZE) && (na1->wsize != na2->wsize))
		return(0);
	if ((na1->flags & NFSMNT_RSIZE) && (na1->rsize != na2->rsize))
		return(0);
	if ((na1->flags & NFSMNT_TIMEO) && (na1->timeo != na2->timeo))
		return(0);
	if ((na1->flags & NFSMNT_RETRANS) && (na1->retrans != na2->retrans))
		return(0);
	if ((na1->flags & NFSMNT_ACREGMIN) && (na1->acregmin != na2->acregmin))
		return(0);
	if ((na1->flags & NFSMNT_ACREGMAX) && (na1->acregmax != na2->acregmax))
		return(0);
	if ((na1->flags & NFSMNT_ACDIRMIN) && (na1->acdirmin != na2->acdirmin))
		return(0);
	if ((na1->flags & NFSMNT_ACDIRMAX) && (na1->acdirmax != na2->acdirmax))
		return(0);
	return(1);
}
