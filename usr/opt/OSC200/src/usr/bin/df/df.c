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
static char rcsid[] = "@(#)$RCSfile: df.c,v $ $Revision: 4.3.10.8 $ (DEC) $Date: 1993/12/16 23:35:35 $";
#endif
/*
 * HISTORY
 */
/*
 * @(#)$RCSfile: df.c,v $ $Revision: 4.3.10.8 $ (DEC) $Date: 1993/12/16 23:35:35 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1980, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1990 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/* #ifndef lint
 * static char sccsid[] = "@(#)df.c	5.24 (Berkeley) 3/6/91";
 * #endif
 */

/*
 * df
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/file.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <locale.h>

#include "df_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_DF,n,s) 

#define DF_XPG4	"xpg4"
#define DF_SVR4	"svr4"

extern int optind;
extern char *optarg;
int	getvfsbyname();
char	*getmntpt();
char	*mktemp();
void	prtstat();
long	getmntinfo();
long	getmntinfo_type(struct statfs *, long, long, long);
int	iflag, kflag, nflag, typflag;
struct ufs_args mdev;

int     fstype;
char    *fstypename;

void        
main(int argc, char *argv[])
{
	int err, ch, i, pass;
	long width, maxwidth, mntsize, fssize;
	char *mntpt, **saved_argv;
	struct stat stbuf;
	struct statfs statfsbuf, *mntbuf = NULL;
	int exitret = 0;
	int prauto = 0;
	char *getptr=NULL;

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_DF,NL_CAT_LOCALE);

 /*GZ001 - made df booth sysV and BSD compatible. The behaviour depends on
  * DEC_XPG4 environment var, the same like in echo command */
 /*DS001 - Restructure the parse for the XPG/4 environment. The environment
  * variable is now named CMD_ENV. It currently takes the values XPG4 or
  * SVR4.
  */
	getptr = getenv("CMD_ENV");

	if ((getptr != NULL) && !strcmp(getptr, DF_XPG4))
	 { /* XPG/4 environment is set */
	  while ((ch = getopt(argc, argv, "PeikntF:")) != EOF)
		switch(ch) {
		case 'e':
			prauto = 1;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'k':
			kflag = 1;
			break;
		case 'n':
			nflag = 1;
			break;
                case 'F':
                        typflag = 1;
			fstypename = optarg;
                        break;
                case 't':
		case 'P':
			break;	/* they're the default, anyway */
		case '?':
		default:
			fprintf(stderr,
			    MSGSTR(USAGE_XPG, "usage: df [-Peiknt] [-F fstype] [file | file_system ...]\n"));
			exit(1);
		}
	 }
	else 
	 { /* no XPG/4 environment var, BSD behaviour */
	   while ((ch = getopt(argc, argv, "Peiknt:")) != EOF)
		switch(ch) {
		case 'e':
			prauto = 1;
			break;
		case 'i':
			iflag = 1;
			break;
		case 'k':
			kflag = 1;
			break;
		case 'n':
			nflag = 1;
			break;
                case 't':
                        typflag = 1;
			fstypename = optarg;
                        break;
		case 'P':
			break;	/* it's the default, anyway */
		case '?':
		default:
			fprintf(stderr,
			    MSGSTR(USAGE_BSD, "usage: df [-Peikn] [-t fstype] [file | file_system ...]\n"));
			exit(1);
		}
	 }
	/* GZ001 end */

	argc -= optind;
	argv += optind;
	if (typflag) {
		fstype = getvfsbyname(fstypename);
		if (fstype <= MOUNT_NONE) {
			fprintf(stderr,
				MSGSTR(FTYPE,
				       "df: %s: unknown file system type.\n"),
				fstypename);
			exit(1);
		}
	}

	if (!*argv) {
		/*
		 * scan through to calculate the the maximum
		 * width.  If type specified, only use widths of that fs type.
		 */
		mntsize = getmntinfo(&mntbuf, MNT_NOWAIT);
		if (mntbuf == NULL) { 
			fprintf(stderr, MSGSTR(NOMEM, "df: out of memory\n")); 
			exit(1); 
		} 
		maxwidth = 0;
		fssize = 0;
		for (i = 0; i < mntsize; i++) {
			if (!typflag || mntbuf[i].f_type == fstype) {
				if (typflag)
					fssize++;
				width = strlen(mntbuf[i].f_mntfromname);
				if (width > maxwidth)
					maxwidth = width;
			}
		}

		/*
		 * Print information for all mounted file systems, 
		 * discriminating by type if typflag specified.
		 * If -n specified, we already have the information; it
		 * is possibly stale, but at least we didn't hang.
		 */
		if (!nflag) {
			if (typflag)
				mntsize = getmntinfo_type(mntbuf, mntsize,
							  fssize, maxwidth);
			else
				mntsize = getmntinfo(&mntbuf, MNT_WAIT);
		}
		for (i = 0; i < mntsize; i++)
                        if (!typflag || mntbuf[i].f_type == fstype) {
                            /*
                             * Skip automount file systems unless
                             * prauto is set.
                             */
                            if (!prauto &&
                               (mntbuf[i].f_type == MOUNT_NFS) &&
                               (mntbuf[i].mount_info.nfs_args.flags & NFSMNT_AUTO))
                                    continue;
                            prtstat(&mntbuf[i], maxwidth);
			}
		exit(0);
	}

	pass = 0;
	maxwidth = 0;
	saved_argv = argv;
pass1:
	for (; *argv; argv++) {
		/*
		 * don't try to avoid hanging here, so don't even
		 * look at nflag.  We're doing the fs's one at a time
		 */
		if (stat(*argv, &stbuf) < 0) {
			err = errno;

			if ((mntpt = getmntpt(*argv)) == 0) {
				if (pass != 0) {
					errno = err;
					exitret = 1;
					perror(*argv);
				}
				continue;
			}
		} else if ((stbuf.st_mode & S_IFMT) == S_IFBLK) {
			if ((mntpt = getmntpt(*argv)) == 0) {
				fprintf(stderr,
					"%s: Filesystem not mounted\n",
					*argv);
				exitret = 1;
				continue;
			}
		} else
			mntpt = *argv;
		/*
		 * Statfs does not take a `wait' flag, so we cannot
		 * implement nflag here
		 */
		if (statfs(mntpt, &statfsbuf) < 0) {
			if (pass != 0) {
				exitret = 1;
				perror(mntpt);
			}
			continue;
		}
		if (pass == 0) {
			width = strlen(statfsbuf.f_mntfromname);
			if (width > maxwidth)
				maxwidth = width;
		} else 
                	prtstat(&statfsbuf, maxwidth);
	}
	if (pass == 0) {
		pass++;
		argv = saved_argv;
		goto pass1;
	}
	exit(exitret);
}

char *
getmntpt(name)
	char *name;
{
	long mntsize, i;
	struct statfs *mntbuf;

	mntsize = getmntinfo(&mntbuf, 0);
	for (i = 0; i < mntsize; i++) {
		if (!strcmp(mntbuf[i].f_mntfromname, name))
			return (mntbuf[i].f_mntonname);
	}
	return (0);
}

/*
 * only use statfs() when asked for particular type,
 * so that we don't hang on MNT_WAIT if something of a different
 * type is unresponsive.
 */

long
getmntinfo_type(struct statfs *mntbuf, long maxsize, long fsnum, long maxwidth)
{
	int i;
	/* use multiple statfs calls to get the information */
	for (i = 0; i < maxsize; i++) {
		if (mntbuf[i].f_type == fstype) {
			if (statfs(mntbuf[i].f_mntonname, &mntbuf[i]))
			    return(0);
			prtstat(&mntbuf[i], maxwidth);
		}
	}
	return 0;
}

/*
 * Print out status about a filesystem.
 */
void
prtstat(sfsp, maxwidth)
	register struct statfs *sfsp;
        long maxwidth;
{
	long used, availblks, inodes;
        static int timesthrough;
        int fsys_len = strlen(MSGSTR(FSYS, "Filesystem")) + 1;
       


	/*
	 * For testing purposes:
	 */
#ifdef VERY_BIG_DISKS
	sfsp->f_blocks=  0x7fffffff;
	sfsp->f_bavail = 0x3fffffff;
#endif
	if (maxwidth < fsys_len)
		maxwidth = fsys_len;

        if (++timesthrough == 1) {
                printf(MSGSTR(MSG1, "%-*.*s%s        Used       Avail Capacity"),
                       maxwidth, maxwidth, MSGSTR(FSYS, "Filesystem"),
                       kflag ? MSGSTR(KBYTES, " 1024-blocks") : MSGSTR(BLKS, "  512-blocks"));
                if (iflag)
                        printf(MSGSTR(IUSED, " Iused   Ifree  %%Iused"));
                printf(MSGSTR(MOUNTED, "  Mounted on\n"));
        }
	printf("%-*.*s", maxwidth, maxwidth, sfsp->f_mntfromname);
	used = sfsp->f_blocks - sfsp->f_bfree;
	availblks = sfsp->f_bavail + used;
	printf("%12ld%12ld%12ld",
	    (off_t) sfsp->f_blocks * sfsp->f_fsize / (kflag ? 1024 : 512),
	    used * sfsp->f_fsize / (kflag ? 1024 : 512),
	/*GZ001  (f_bavail < 0) means no more blocks for non su */
	    (sfsp->f_bavail < 0)? 0:
	    (off_t) sfsp->f_bavail * sfsp->f_fsize / (kflag ? 1024 : 512));
	printf("%6.0f%%",
	    availblks == 0 ? 100.0 : (double)used / (double)availblks * 100.0);
	if (iflag) {
		inodes = sfsp->f_files;
		used = inodes - sfsp->f_ffree;
		printf("%8ld%8ld%6.0f%% ", used, sfsp->f_ffree,
		   inodes == 0 ? 100.0 : (double)used / (double)abs(inodes) * 100.0);
	} else 
		printf("  ");
	printf("  %s\n", sfsp->f_mntonname);
}
