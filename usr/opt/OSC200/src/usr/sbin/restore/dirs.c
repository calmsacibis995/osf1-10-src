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
static char	*sccsid = "@(#)$RCSfile: dirs.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/23 14:30:57 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE ONLY
 * SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

#include	"restore.h"
#include	<sys/file.h>
#include	<sys/time.h>

static void putdir();
static void putent();
static void flushent();
static void dcvt();

/*
 * Symbol table of directories read from tape.
 */

#define	HASHSIZE	1000
#define	INOHASH(val)	(val % HASHSIZE)

/*
 * Format of old style directories.
 */

#define	ODIRSIZ	14

struct odirect
{
	u_short		d_fileno;
	char		d_name[ODIRSIZ];
};

struct inotab
{
	struct inotab  *t_next;
	ino_t		t_ino;
	daddr_t		t_seekpt;
	long		t_size;
};

/*
 * Information retained	about directories.
 */

struct modeinfo
{
	ino_t		ino;
	struct timeval	timev[3];
	short		mode;
	short		uid;
	short		gid;
};

static ino_t		search();

/*
 * Global variables for this file.
 */

static struct inotab   *inotab[HASHSIZE];
static daddr_t		seekpt;
static FILE	       *df, *mf;
static RST_DIR	       *dirp;
static char		dirfile[32] = "#";	/* No file */
static char		modefile[32] = "#";	/* No file */

/*
 * Extract directory contents, building up a directory structure on disk for
 * extraction by name. If genmode is requested, save mode, owner, and times for
 * all directories on the tape.
 */

void
extractdirs(genmode)
	int		genmode;
{
	register int	i;
	register struct	dinode *ip;
	struct inotab  *itp;
	struct dirent	nulldir;

	vmsg(MSGSTR(EXTRDIR, "Extract directories from tape\n"));
	(void) sprintf(dirfile, "/tmp/rstdir%d", getpid());
	df = fopen(dirfile, "w");
	if (df == 0)
	{
		msg(MSGSTR(CNCRDIR, "restore: %s - cannot create directory temporary\n"), dirfile);
		perror("extractdirs(): fopen(df)");
		Exit(1);

		/* NOTREACHED */
	}
	if (genmode == TRUE)
	{
		(void) sprintf(modefile, "/tmp/rstmode%d", getpid());
		mf = fopen(modefile, "w");
		if (mf == 0)
		{
			msg(MSGSTR(CNCRMO, "restore: %s - cannot create modefile \n"), modefile);
			perror("extractdirs(): fopen(mf)");
			Exit(1);

			/* NOTREACHED */
		}
	}
	nulldir.d_fileno = 0;
	nulldir.d_namlen = 1;
	(void) strcpy(nulldir.d_name, "/");
	nulldir.d_reclen = DIRSIZ(&nulldir);
	for (;;)
	{
		curr_file_name = "<directory file - name unknown>";
		curr_action = EXTRACTING;
		ip = curr_inode;

		if (ip == NULL || (ip->di_mode & IFMT) != IFDIR)
		{
			if (fclose(df) != 0)
			{
				msg(MSGSTR(STREAMCLE, "error closing stream\n"));
				perror("setdirmodes(): fclose(df)");
				return;
			}
			dirp = rst_setupdir(dirfile);
			if (dirp == NULL)
			{
				perror("extractdirs(): rst_setupdir()");
			}
			if (mf != NULL)
			{
				if (fclose(mf) != 0)
				{
					msg(MSGSTR(STREAMCLE, "error closing stream\n"));
					perror("setdirmodes(): fclose(mf)");
					return;
				}
			}
                        if (!MAPBITTEST (dumpmap, ROOTINO)) {
                                msg(MSGSTR(TAPEEMPTY, "No files found on the tape.\n"));
                                Exit(0);
                        }
			i = dirlookup(".");
			if (i == 0)
			{
				panic(MSGSTR(NOROOT, "Root directory is not on tape\n"));
			}
			return;
		}
		itp = allocinotab(curr_inumber, ip, seekpt);
		getfile(putdir, NULLVOIDFP);
		putent(&nulldir);
		flushent();
		itp->t_size = seekpt - itp->t_seekpt;
	}
}

/*
 * skip over all the directories on the tape
 */

void
skipdirs()
{
	while ((curr_inode->di_mode & IFMT) == IFDIR)
	{
		skipfile();
	}
}

/*
 * Recursively find names and inumbers of all files in subtree pname and pass
 * them off to be processed.
 */

void
treescan(pname, ino, todo)
	char	       *pname;
	ino_t		ino;
	long	      (*todo)();
{
	register struct inotab *itp;
	register struct dirent *dp;
	register struct entry *np;
	int		namelen;
	daddr_t		bpt;
	char		locname[MAXPATHLEN + 1];

	itp = inotablookup(ino);
	if (itp == NULL)
	{
		/*
		 * Pname is name of a simple file or an unchanged directory.
		 */

		(void) (*todo) (pname, ino, LEAF);
		return;
	}

	/*
	 * Pname is a dumped directory name.
	 */

	if ((*todo)(pname, ino, NODE) == FAIL)
	{
		return;
	}

	/*
	 * begin search through the directory skipping over "." and ".."
	 */

	(void) strncpy(locname, pname, MAXPATHLEN);
	(void) strncat(locname, "/", MAXPATHLEN);
	namelen = strlen(locname);
	rst_seekdir(dirp, itp->t_seekpt, itp->t_seekpt);
	dp = rst_readdir(dirp);				/* "." */
	if (dp != NULL && strcmp(dp->d_name, ".") == 0)
	{
		dp = rst_readdir(dirp);			/* ".." */
	}
	else
	{
		msg(MSGSTR(MISSDOT, "Warning: `.' missing from directory %s\n"), pname);
	}
	if (dp != NULL && strcmp(dp->d_name, "..") == 0)
	{
		dp = rst_readdir(dirp);			/* first real entry */
	}
	else
	{
		msg(MSGSTR(MDOTDOT, "Warning: `..' missing from directory %s\n"), pname);
	}
	bpt = rst_telldir(dirp);

	/*
	 * a zero inode signals end of directory
	 */

	while (dp != NULL && dp->d_fileno != 0)
	{
		locname[namelen] = '\0';
		if (namelen + dp->d_namlen >= MAXPATHLEN)
		{
			msg(MSGSTR(NAMLONG, "%s%s: name exceeds %d char\n"), locname, dp->d_name, MAXPATHLEN);
		}
		else
		{
			(void) strncat(locname, dp->d_name, (int) dp->d_namlen);
			treescan(locname, dp->d_fileno, todo);
			rst_seekdir(dirp, bpt, itp->t_seekpt);
		}
		dp = rst_readdir(dirp);
		bpt = rst_telldir(dirp);
	}
	if (dp == NULL)
	{
		msg(MSGSTR(CORRDIR, "corrupted directory: %s.\n"), locname);
	}
}

/*
 * Search the directory tree rooted at inode ROOTINO for the path pointed at by
 * n
 */

ino_t
psearch(n)
	char	       *n;
{
	register char  *cp, *cp1;
	ino_t		ino;
	char		c;

	ino = ROOTINO;
	if (*(cp = n) == '/')
	{
		++cp;
	}
next:
	cp1 = cp + 1;
	while (*cp1 != '/' && *cp1)
	{
		++cp1;
	}
	c = *cp1;
	*cp1 = 0;
	ino = search(ino, cp);
	if (ino == 0)
	{
		*cp1 = c;
		return(0);
	}
	*cp1 = c;
	if (c == '/')
	{
		cp = cp1 + 1;
		goto next;
	}
	return(ino);
}

/*
 * search the directory inode ino looking for entry cp
 */

static ino_t
search(inum, cp)
	ino_t		inum;
	char	       *cp;
{
	register struct dirent *dp;
	register struct inotab *itp;
	int		len;

	itp = inotablookup(inum);
	if (itp == NULL)
	{
		return(0);
	}
	rst_seekdir(dirp, itp->t_seekpt, itp->t_seekpt);
	len = strlen(cp);
	do
	{
		dp = rst_readdir(dirp);
		if (dp == NULL || dp->d_fileno == 0)
		{
			return(0);
		}
	} while (dp->d_namlen != len || strncmp(dp->d_name, cp, len) != 0);
	return(dp->d_fileno);
}

/*
 * Put the directory entries in the directory file
 */

static void
putdir(buf, size)
	char	       *buf;
	int		size;
{
	struct dirent	cvtbuf;
	register struct odirect	*odp;
	struct odirect *eodp;
	register struct dirent *dp;
	long		loc, i;

	if (old_format_flag == TRUE)
	{
		eodp = (struct odirect *) & buf[size];
		for (odp = (struct odirect *) buf; odp < eodp; ++odp)
			if (odp->d_fileno != 0)
			{
				dcvt(odp, &cvtbuf);
				putent(&cvtbuf);
			}
	}
	else
	{
		for (loc = 0; loc < size;)
		{
			dp = (struct dirent *) (buf + loc);
			if (byte_swap_flag == TRUE)
			{
				swabst("l2s", (char *) dp);
			}
			i = DIRBLKSIZ - (loc & (DIRBLKSIZ - 1));
			if (dp->d_reclen == 0 || dp->d_reclen > i)
			{
				loc += i;
				continue;
			}
			loc += dp->d_reclen;
			if (dp->d_fileno != 0)
			{
				putent(dp);
			}
		}
	}
}

/*
 * These variables are "local" to the following two functions.
 */

char		dirbuf[DIRBLKSIZ];
long		dirloc = 0;
long		prev = 0;

/*
 * add a new directory entry to a file.
 */

static void
putent(dp)
	struct dirent  *dp;
{
	dp->d_reclen = DIRSIZ(dp);
	if (dirloc + dp->d_reclen > DIRBLKSIZ)
	{
		((struct dirent *) (dirbuf + prev))->d_reclen =
			DIRBLKSIZ - prev;
		if (fwrite(dirbuf, 1, DIRBLKSIZ, df) != DIRBLKSIZ)
		{
			msg(MSGSTR(STREAMWRE, "error writing stream\n"));
			perror("putent(): fwrite(df)");
		}
		dirloc = 0;
	}
	memcpy(dirbuf + dirloc, (char *) dp, (size_t) dp->d_reclen);
	prev = dirloc;
	dirloc += dp->d_reclen;
}

/*
 * flush out a directory that is finished.
 */

static void
flushent()
{

	((struct dirent *) (dirbuf + prev))->d_reclen = DIRBLKSIZ - prev;
	if (fwrite(dirbuf, (int) dirloc, 1, df) != 1)
	{
		msg(MSGSTR(STREAMWRE, "error writing stream\n"));
		perror("flushent(): fwrite(df)");
	}
	seekpt = ftell(df);
	dirloc = 0;
}

static void
dcvt(odp, ndp)
	register struct odirect	*odp;
	register struct dirent *ndp;
{

	memset((char *) ndp, (int) '\0', sizeof(struct dirent));

	ndp->d_fileno = odp->d_fileno;
	(void) strncpy(ndp->d_name, odp->d_name, ODIRSIZ);
	ndp->d_namlen = strlen(ndp->d_name);
	ndp->d_reclen = DIRSIZ(ndp);
}

/*
 * Alas under MACH using files that are really on another system, telldir()
 * and seekdir for non-local directories can cause looping and confusion.
 * Therefore for restore's purpose we have to also have a rst_telldir()
 * routine and to use it.
 */

daddr_t
rst_telldir(dirp)
	register RST_DIR   *dirp;
{
        return((daddr_t)lseek(dirp->dd_fd, (off_t)0L, 1) - dirp->dd_size + dirp->dd_loc);
}

/*
 * At CMU opendir does some additional checks on directories being 
 * directories and also optimize the buffer size to use instead of simply
 * using DIRBLKSIZ. Sigh. Therefore we can't use opendir() to setup calls
 * for rst_XXXXX() series routines...therefore we import the old opendir()
 * code as rst_setup() and to make debugging easier use type RST_DIR
 * (which also avoids the CMU dynamic buffer issue).
 */

RST_DIR		*
rst_setupdir(name)
	char	       *name;
{
	register RST_DIR   *dirp;
	register int	fd;

	if ((fd = open(name, O_RDONLY)) < 0)
	{
		return(NULL);
	}
	if ((dirp = (RST_DIR *) malloc(sizeof(RST_DIR))) == NULL)
	{
		if (close(fd) != 0)
		{
			msg(MSGSTR(FILECLE, "error closing file\n"));
			perror("rst_setupdir(): close(df)");
		}

		return(NULL);
	}
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return(dirp);
}

/*
 * Seek to an entry in a directory. Only values returned by ``telldir'' should
 * be passed to rst_seekdir. This routine handles many directories in a single
 * file. It takes the base of the directory in the file, plus the desired seek
 * offset into it.
 */

void
rst_seekdir(dirp, loc, base)
	register RST_DIR   *dirp;
	daddr_t		loc, base;
{

	if (loc == rst_telldir(dirp))
	{
		return;
	}
	loc -= base;
	if (loc < 0)
	{
		msg(MSGSTR(BADSEEKP, "bad seek pointer to rst_seekdir %d\n"), loc);
	}
	(void) lseek(dirp->dd_fd, (off_t)(base + (loc & ~(DIRBLKSIZ - 1))), 0);
	dirp->dd_loc = loc & (DIRBLKSIZ - 1);
	if (dirp->dd_loc != 0)
	{
		dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, DIRBLKSIZ);
	}
}

/*
 * get next entry in a directory.
 */

struct dirent  *
rst_readdir(dirp)
	register RST_DIR   *dirp;
{
	register struct dirent *dp;

	for (;;)
	{
		if (dirp->dd_loc == 0)
		{
			dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, DIRBLKSIZ);
			if (dirp->dd_size <= 0)
			{
				dmsg("error reading directory\n");
				return(NULL);
			}
		}
		if (dirp->dd_loc >= dirp->dd_size)
		{
			dirp->dd_loc = 0;
			continue;
		}
		dp = (struct dirent *) (dirp->dd_buf + dirp->dd_loc);
		if (dp->d_reclen == 0 ||
		    dp->d_reclen > DIRBLKSIZ + 1 - dirp->dd_loc)
		{
			dmsg("corrupted directory: bad reclen %d\n",
				dp->d_reclen);
			return(NULL);
		}
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_fileno == 0 && strcmp(dp->d_name, "/") != 0)
		{
			continue;
		}
		if (dp->d_fileno >= maxino)
		{
			dmsg("corrupted directory: bad inum %d\n",
				dp->d_fileno);
			continue;
		}
		return(dp);
	}
}

/*
 * Simulate the opening of a directory
 */

RST_DIR	       *
rst_opendir(name)
	char	       *name;
{
	struct inotab  *itp;
	ino_t		ino;

	if ((ino = dirlookup(name)) > 0 &&
	    (itp = inotablookup(ino)) != NULL)
	{
		rst_seekdir(dirp, itp->t_seekpt, itp->t_seekpt);
		return(dirp);
	}
	return(0);
}

/*
 * Set the mode, owner, and times for all new or changed directories
 */

void
setdirmodes()
{
	FILE	       *mf;
	struct modeinfo	node;
	struct entry   *ep;
	char	       *cp;

	vmsg(MSGSTR(SETMOT, "Set directory mode, owner, and times.\n"));
	(void) sprintf(modefile, "/tmp/rstmode%d", getpid());
	mf = fopen(modefile, "r");
	if (mf == NULL)
	{
		msg(MSGSTR(NOMODEFI, "cannot open mode file %s\n"), modefile);
		msg(MSGSTR(MODENOTS, "directory mode, owner, and times not set\n"));
		perror("setdirmodes(): fopen()");
		return;
	}
	clearerr(mf);
	for (;;)
	{
		(void) fread((char *) &node, 1, sizeof(struct modeinfo), mf);
		if (feof(mf))
		{
			break;
		}
		ep = lookupino(node.ino);
		if (command == 'i' || command == 'x')
		{
			if (ep == NULL)
			{
				continue;
			}
			if (ep->e_flags & EXISTED)
			{
				ep->e_flags &= ~NEW;
				continue;
			}
			if ((command == 'i') && 
			    (node.ino == ROOTINO) &&
			    (query(MSGSTR(SETDOT, "set owner/mode for '.'")) == NO))
			{
				continue;
			}
		}
		if (ep == NULL)
		{
			panic(MSGSTR(NODIRI, "cannot find directory inode %d\n"), node.ino);
		}
		cp = myname(ep);
		(void) chown(cp, node.uid, node.gid);
		(void) chmod(cp, node.mode);
		xutimes(cp, node.timev);
		ep->e_flags &= ~NEW;
	}
	if (ferror(mf))
	{
		panic(MSGSTR(ESETDIR, "error setting directory modes\n"));
	}
	if (fclose(mf) != 0)
	{
		perror("setdirmodes(): fclose()");
		panic(MSGSTR(ESETDIR, "error setting directory modes\n"));
	}
}

/*
 * Generate a literal copy of a directory.
 */

int
genliteraldir(name, ino)
	char	       *name;
	ino_t		ino;
{
	register struct inotab *itp;
	int		ofile, dp, i, size, rsize;
	char		buf[BUFSIZ];

	itp = inotablookup(ino);
	if (itp == NULL)
	{
		panic(MSGSTR(NODIRINO, "Cannot find directory inode %d named %s\n"), ino, name);
	}
	if ((ofile = creat(name, 0666)) < 0)
	{
		msg(MSGSTR(FILECRE, "error creating file %s\n"),name);
		perror("genliteraldir(): creat()");
		return(FAIL);
	}
	rst_seekdir(dirp, itp->t_seekpt, itp->t_seekpt);
	dp = dup(dirp->dd_fd);
	for (i = itp->t_size; i > 0; i -= BUFSIZ)
	{
		size = i < BUFSIZ ? i : BUFSIZ;
		if (read(dp, buf, (int) size) == -1)
		{
			msg(MSGSTR(EREADINO, "read error extracting inode %d, name %s\n"), curr_inumber, curr_file_name);
			perror("genliteraldir(): read()");
			Exit(1);

			/* NOTREACHED */
		}
		if ((Nflag == FALSE) && ((rsize = write(ofile, buf, (int)size)) != (int)size))
		{		      
			msg(MSGSTR(EWRITINO1, "write error extracting inode %d, name %s\n"), curr_inumber, curr_file_name);
			msg(MSGSTR(EWRITINO2, "wanted to write %d, but actual wrote %d\n"),size,rsize);
			if (rsize == -1)
			{
				perror("genliteraldir(): write()");
			}
			else
			{
				msg("genliteraldir(): write():\n");
			}
			Exit(1);
			
			/* NOTREACHED */
		}
	}

	if (close(dp) != 0)
	{
		msg(MSGSTR(FILECLE, "error closing file\n"));
		perror("genliteraldir(): close(dp)");
		Exit(1);
		
		/* NOTREACHED */
	}

	if (close(ofile) != 0)
	{
		msg(MSGSTR(FILECLE, "error closing file\n"));
		perror("genliteraldir(): close(ofile)");
		Exit(1);
		
		/* NOTREACHED */
	}

	return(GOOD);
}

/*
 * Determine the type of an inode
 */

int
inodetype(ino)
	ino_t		ino;
{
	struct inotab  *itp;

	itp = inotablookup(ino);
	if (itp == NULL)
	{
		return(LEAF);
	}
	return(NODE);
}

/*
 * Allocate and initialize a directory inode entry. If requested, save its
 * pertinent mode, owner, and time info.
 */

struct inotab  *
allocinotab(ino, dip, seekpt)
	ino_t		ino;
	struct dinode  *dip;
	daddr_t		seekpt;
{
	register struct inotab *itp;
	struct modeinfo	node;

	itp = (struct inotab *) calloc(1, sizeof(struct inotab));
	itp->t_next = inotab[INOHASH(ino)];
	inotab[INOHASH(ino)] = itp;
	itp->t_ino = ino;
	itp->t_seekpt = seekpt;
	if (mf == NULL)
	{
		return(itp);
	}
	node.ino = ino;
	node.timev[0].tv_sec = dip->di_atime;
	node.timev[1].tv_sec = dip->di_mtime;
	node.timev[2].tv_sec = dip->di_ctime;
	node.mode = dip->di_mode;
	node.uid = dip->di_uid;
	node.gid = dip->di_gid;

	if (fwrite((char *) &node, 1, sizeof(struct modeinfo), mf) != sizeof(struct modeinfo))
	{
		msg(MSGSTR(STREAMWRE, "error writing stream\n"));
		perror("putent(): fwrite(mf)");
	}
	return(itp);
}

/*
 * Look up an inode in the table of directories
 */

struct inotab  *
inotablookup(ino)
	ino_t		ino;
{
	register struct inotab *itp;

	for (itp = inotab[INOHASH(ino)]; itp != NULL; itp = itp->t_next)
	{
		if (itp->t_ino == ino)
		{
			return(itp);
		}
	}
	return((struct inotab *) 0);
}

/*
 * Clean up and exit
 */

void
Exit(exitcode)
	int		exitcode;
{

	closemt();
	if (modefile[0] != '#')
	{
		(void) unlink(modefile);
	}
	if (dirfile[0] != '#')
	{
		(void) unlink(dirfile);
	}
	exit(exitcode);
}
