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
static char	*sccsid = "@(#)$RCSfile: tape.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 17:18:20 $";
#endif 
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
#include	<sys/ioctl.h>
#include	<sys/mtio.h>
#include	<sys/file.h>
#include	<setjmp.h>
#include	<sys/stat.h>
#include	<sys/time.h>
#include	<errno.h>

static int	is_volume();
static int	is_type();
static int	readhdr();
static int	gethead();
static int	ishead();
static int	checksum();
static void	setdumpnum();
static void	readtape();
static void	findtapeblksize();
static void	flsht();
static void	accthdr();
static void	findinode();
static unsigned long swabl();

/* NTREC starting default size for determining the block size */

#define NTREC_DEFAULT 64     /* kilobytes */
/* qar 11949: autosize problem; modified NTREC_DEFAULT from 128 to 64 */

static long	fssize = MAXBSIZE;
static int	mt = -1;
static int	pipe_in_flag = FALSE;
static int      regular_file_flag = FALSE;
static char	magtape[BUFSIZ];
static int	bct;
static char    *tbf;
static union u_spcl endoftapemark;
static long	blksread;
static long	tapesread;
static jmp_buf	restart;
static int	gettingfile = FALSE;/* restart has a valid frame */

#if	AIX

static struct devinfo	devinfo;

#endif	AIX

static int	ofile;
static char    *map;
static char	lnkbuf[MAXPATHLEN + 1];
static int	pathlen;

static char 		timbuf[BUFSIZ];
extern struct tm 	*localtime();

/*
 * Set up an input source
 */

void
setinput(source)
	char	       *source;
{
	char	       *host, *tape;
	struct stat    stbuf;

	flsht();
	if (block_size_flag == TRUE)
	{
		newtapebuf(ntrec);
	}
#if	AIX
	else if (isdiskette(source))
	{
		newtapebuf(2 * devinfo.un.dk.secptrk);
	}
#endif
	else
	{
		newtapebuf(NTREC_DEFAULT);
	}

	terminal_fp = stdin;

#if	REMOTE

	host = source;
	tape = index(host, ':');
	if (tape == 0)
	{
nohost:
		msg(MSGSTR(NEEDF, "need keyletter ``f'' and device ``host:tape''\n"));
		Exit(1);

		/* NOTREACHED */
	}
	*tape = '\0';
	++tape;
	(void) strcpy(magtape, tape);

	rmthost(&host);

	setuid(getuid());	/* no longer need or want root privileges */

#else	! REMOTE

	if (strcmp(source, "-") == 0)
	{
		/*
		 * Since input is coming from a pipe we must establish our own
		 * connection to the terminal.
		 */
		terminal_fp = fopen("/dev/tty", "r");
		if (terminal_fp == NULL)
		{
			perror(MSGSTR(CANTOPTTY, "Cannot open(\"/dev/tty\")"));
			terminal_fp = fopen("/dev/null", "r");
			if (terminal_fp == NULL)
			{
				perror(MSGSTR(CANTOPNUL, "Cannot open(\"/dev/null\")"));
				Exit(1);

				/* NOTREACHED */
			}
		}
		pipe_in_flag = TRUE;
	}
	else
	{
		if (lstat(source,&stbuf) < 0)
		{
			perror(MSGSTR(CANTSTATDUMPFILE, "cannot stat dump file"));
			Exit(1);
			/* NOTREACHED */
		}
		if (S_ISREG(stbuf.st_mode))
		{
			regular_file_flag = TRUE;
		}
	}

	(void) strcpy(magtape, source);

#endif	! REMOTE

	/* set command input filepointer, if it is not already set */
	if ( command_fp == NULL )
		command_fp = terminal_fp;

}

void
newtapebuf(size)
	long		size;
{
	static		tbfsize = -1;

	ntrec = size;
	if (size <= tbfsize)
	{
		return;
	}
	if (tbf != NULL)
	{
		free(tbf);
	}
	tbf = (char *) malloc(size * TP_BSIZE);
	if (tbf == NULL)
	{
		msg(MSGSTR(ALLOCF, "Cannot allocate space for tape buffer\n"));
		Exit(1);

		/* NOTREACHED */
	}
	tbfsize = size;
}

/*
 * Verify that the tape drive can be accessed and that it actually is a dump
 * tape.
 */

void
setup()
{
	int		i, j, *ip;
	struct stat	stbuf;

	vmsg(MSGSTR(VERIFYT, "Verify tape and initialize maps\n"));

#if	REMOTE

	if ((mt = rmtopen(magtape, O_RDONLY)) < 0)

#else

	if (pipe_in_flag == TRUE)
	{
		mt = 0;
	}
	else if ((mt = open(magtape, O_RDONLY)) < 0)

#endif

	{
		perror(magtape);
		Exit(1);

		/* NOTREACHED */
	}

	volno = 1;
	setdumpnum();
	flsht();

	if (pipe_in_flag == FALSE && block_size_flag == FALSE)
	{
		findtapeblksize();
	}

	if (gethead(&spcl) == FAIL)
	{
		--bct;		/* push back this block */
		old_format_flag = TRUE;
		if (gethead(&spcl) == FAIL)
		{
			msg(MSGSTR(NOTDUMT, "Tape is not a dump tape\n"));
			Exit(1);

			/* NOTREACHED */
		}
		msg(MSGSTR(CONVNEWF, "Converting to new file system format.\n"));
	}

	if (pipe_in_flag == TRUE)
	{
		endoftapemark.s_spcl.c_magic = (old_format_flag == TRUE)? OFS_MAGIC : NFS_MAGIC;
		endoftapemark.s_spcl.c_type = TS_END;
		ip = (int *) &endoftapemark;
		j = sizeof(union u_spcl) / sizeof(int);
		i = 0;
		do
		{
			i += *ip;
			++ip;
			--j;
		}
		while (j != 0);
		endoftapemark.s_spcl.c_checksum = CHECKSUM - i;
	}

	if (verbose_flag == TRUE || command == 't')
	{
		printdumpinfo();
	}

	dumptime = spcl.c_ddate;
	dumpdate = spcl.c_date;

	if (stat(".", &stbuf) < 0)
	{
		perror(MSGSTR(CANTSTAT, "cannot stat ."));
		Exit(1);

		/* NOTREACHED */
	}

	if (stbuf.st_blksize > 0 && stbuf.st_blksize <= MAXBSIZE)
	{
		fssize = stbuf.st_blksize;
	}
	if ((fssize - 1) & fssize)
	{
		msg(MSGSTR(BADBLKS, "bad block size %d\n"), fssize);
		Exit(1);

		/* NOTREACHED */
	}

	if (is_volume(&spcl, (long) 1) == FALSE)
	{
		msg(MSGSTR(NOTVOL1, "Tape is not volume 1 of the dump\n"));
		Exit(1);

		/* NOTREACHED */
	}

	if (readhdr(&spcl) == FAIL)
	{
		panic(MSGSTR(NOHEAD, "no header after volume mark!\n"));
	}

	findinode(&spcl);
	if (is_type(&spcl, TS_CLRI) == FALSE)
	{
		msg(MSGSTR(CANTFIND, "Cannot find file removal list\n"));
		Exit(1);

		/* NOTREACHED */
	}

	maxino = (spcl.c_count * TP_BSIZE * NBBY) + 1;
	dmsg("maxino = %d\n", maxino);
	map = clrimap = calloc((unsigned) 1, (unsigned) howmany(maxino, NBBY));
	curr_action = EXTRACTING;
	getfile(xtrmap, xtrmapskip);

	if (is_type(&spcl, TS_BITS) == FALSE)
	{
		msg(MSGSTR(CANTFINL, "Cannot find file dump list\n"));
		Exit(1);

		/* NOTREACHED */
	}

	map = dumpmap = calloc((unsigned) 1, (unsigned) howmany(maxino, NBBY));
	curr_action = EXTRACTING;
	getfile(xtrmap, xtrmapskip);
}

/*
 * Prompt user to load a new dump volume. "Nextvol" is the next suggested
 * volume to use. This suggested volume is enforced when doing full or
 * incremental restores, but can be overrridden by the user when only
 * extracting a subset of the files.
 */

#define tmpbuf	tmpspcl.s_spcl

void
getvol(nextvol)
	long		nextvol;
{
	long		newvol;
	long		savecnt, i;
	union u_spcl	tmpspcl;
	char		buf[TP_BSIZE];

	if (nextvol == 1)
	{
		tapesread = 0;
		gettingfile = FALSE;
	}

	if ((pipe_in_flag == TRUE) || (regular_file_flag == TRUE))
	{		
		if (nextvol != 1)
		{
			panic(MSGSTR(CANPIPE, "Changing volumes on pipe input?\n"));
		}
		if (volno == 1)
		{
			return;
		}
		goto gethdr;
	}
	savecnt = blksread;

again:
	if ((pipe_in_flag == TRUE) || (regular_file_flag == TRUE))
	{
		Exit(1);	/* pipes do not get a second chance */

		/* NOTREACHED */
	}
	if (command == 'R' || command == 'r' || curr_action != SKIP)
	{
		newvol = nextvol;
	}
	else
	{
		newvol = 0;
	}
	while (newvol <= 0)
	{
		if (tapesread == 0)
		{
			msg("%s%s%s%s%s",
				MSGSTR(TAPESMES1, "You have not read any tapes yet.\n"),
				MSGSTR(TAPESMES2, "Unless you know which volume your"),
				MSGSTR(TAPESMES3, " file(s) are on you should start\n"),
				MSGSTR(TAPESMES4, "with the last volume and work"),
				MSGSTR(TAPESMES5, " towards the first.\n"));
		}
		else
		{
			msg(MSGSTR(YOUHAVE, "You have read volumes"));
			strcpy(tbf, ": ");
			for (i = 1; i < 32; ++i)
			{
				if (tapesread & (1 << i))
				{
					msg("%s%d", tbf, i);
					strcpy(tbf, ", ");
				}
			}
			msg("\n");
		}
		do
		{
			msg(MSGSTR(NEXTVOL, "Specify next volume #: "));
			(void) fgets(tbf, BUFSIZ, command_fp);
		} while (!feof(command_fp) && tbf[0] == '\n');
		if (feof(command_fp))
		{
			Exit(1);

			/* NOTREACHED */
		}
		newvol = atoi(tbf);
		if (newvol <= 0)
		{
			msg(MSGSTR(VOLPOS, "Volume numbers are positive numerics\n"));
		}
	}
	if (newvol == volno)
	{
		tapesread |= 1 << volno;
		return;
	}
	closemt();
	msg(MSGSTR(MOUNTT, "Mount tape volume %d\n"), newvol);
	msg(MSGSTR(ENTERT, "then enter tape name (default: %s) "), magtape);
	(void) fgets(tbf, BUFSIZ, terminal_fp);
	if (feof(terminal_fp))
	{
		Exit(1);

		/* NOTREACHED */
	}
	if (tbf[0] != '\n')
	{
		(void) strcpy(magtape, tbf);
		magtape[strlen(magtape) - 1] = '\0';
	}

#if	REMOTE

	if ((mt = rmtopen(magtape, O_RDONLY)) == -1)

#else

	if ((mt = open(magtape, O_RDONLY)) == -1)

#endif

	{
		msg(MSGSTR(CANTOPEN, "Cannot open %s\n"), magtape);
		volno = -1;
		goto again;
	}

gethdr:
	volno = newvol;
	setdumpnum();
	flsht();
	if (readhdr(&tmpbuf) == FAIL)
	{
		msg(MSGSTR(NOTDUMT, "Tape is not a dump tape\n"));
		volno = 0;
		goto again;
	}
	if (is_volume(&tmpbuf, volno) == FALSE)
	{
		msg(MSGSTR(WRONGVOL, "Wrong volume (%d)\n"), tmpbuf.c_volume);
		volno = 0;
		goto again;
	}
	if (tmpbuf.c_date != dumpdate || tmpbuf.c_ddate != dumptime)
	{

		(void)strftime(timbuf, BUFSIZ, "%c %Z\0", localtime(&tmpbuf.c_date));
		msg(MSGSTR(WRONGDATE, "Wrong dump date\n\tgot: %s"), timbuf);
		(void)strftime(timbuf, BUFSIZ, "%c %Z\0", localtime(&dumpdate));
		msg(MSGSTR(WANTED, "\twanted: %s"), timbuf);

		volno = 0;
		goto again;
	}
	tapesread |= 1 << volno;
	blksread = savecnt;
	if (curr_action == EXTRACTING)
	{
		if (volno == 1)
		{
			panic(MSGSTR(ACTIVEF, "active file into volume 1\n"));
		}
		return;
	}

	/*
	 * Skip up to the beginning of the next record
	 */

	if (tmpbuf.c_type == TS_TAPE && (tmpbuf.c_flags & DR_NEWHEADER))
	{
		for (i = tmpbuf.c_count; i > 0; --i)
		{
			readtape(buf);
		}
	}

	(void) gethead(&spcl);
	findinode(&spcl);
	if (gettingfile == TRUE)
	{
		gettingfile = FALSE;
		longjmp(restart, 1);
	}
}

/*
 * handle multiple dumps per tape by skipping forward to the appropriate one.
 */

static void
setdumpnum()
{
	struct mtop	tcom;

	if (dumpnum == 1 || volno != 1)
	{
		return;
	}

	if (pipe_in_flag == TRUE)
	{
		msg(MSGSTR(MULTIDP, "Cannot have multiple dumps on pipe input\n"));
		Exit(1);

		/* NOTREACHED */
	}

	tcom.mt_op = MTFSF;
	tcom.mt_count = dumpnum - 1;

#if	REMOTE

	rmtioctl(MTFSF, dumpnum - 1);

#else	! REMOTE

	if (ioctl(mt, (int) MTIOCTOP, (char *) &tcom) < 0)
	{
		perror(MSGSTR(IOCTLE, "ioctl STFSF"));
	}

#endif	! REMOTE

}

void
printdumpinfo()
{

	(void)strftime(timbuf, BUFSIZ, "%c %Z\0", localtime(&spcl.c_date));
	msg(MSGSTR(DUMPDATE, "Dump   date: %s\n"), timbuf);
	(void)strftime(timbuf, BUFSIZ, "%c %Z\0", localtime(&spcl.c_ddate));
	msg(MSGSTR(DUMPFROM, "Dumped from: %s\n"), timbuf);

	if (spcl.c_host[0] == '\0')
	{
		return;
	}
	msg(MSGSTR(LVLDOO, "Level %d dump of %s on %s:%s\n"), spcl.c_level, spcl.c_filesys, spcl.c_host, spcl.c_dev);
	msg(MSGSTR(LABEL, "Label: %s\n"), spcl.c_label);
}

int
extractfile(name)
	char	       *name;
{
	int		mode;
	struct timeval	timev[3];
	struct entry   *ep;

	curr_file_name = name;
	curr_action = EXTRACTING;
	timev[0].tv_sec = curr_inode->di_atime;
	timev[1].tv_sec = curr_inode->di_mtime;
	timev[2].tv_sec = curr_inode->di_ctime;
	mode = curr_inode->di_mode;
	switch (mode & IFMT)
	{

	default:
		msg(MSGSTR(UNKFMODE, "%s: unknown file mode 0%o\n"), name, mode);
		skipfile();
		return(FAIL);

	case IFSOCK:
		vmsg(MSGSTR(SOCKET, "skipped socket %s\n"), name);
		skipfile();
		return(GOOD);

	case IFDIR:
		if (by_name_flag == TRUE)
		{
			ep = lookupname(name);
			if (ep == NULL || ep->e_flags & EXTRACT)
			{
				panic(MSGSTR(UNEXTDIR, "unextracted directory %s\n"), name);
			}
			skipfile();
			return(GOOD);
		}
		vmsg(MSGSTR(EXTRFILE, "extract file %s\n"), name);
		return(genliteraldir(name, curr_inumber));

	case IFLNK:
		lnkbuf[0] = '\0';
		pathlen = 0;
		getfile(xtrlnkfile, xtrlnkskip);
		if (pathlen == 0)
		{
			vmsg(MSGSTR(ZEROLEN, "%s: zero length symbolic link (ignored)\n"), name);
			return(GOOD);
		}

		return(linkit(lnkbuf, name, SYMLINK));

	case IFCHR:
	case IFBLK:
		vmsg(MSGSTR(EXTSFILE, "extract special file %s\n"), name);
		if (Nflag == TRUE)
		{
			skipfile();
			return(GOOD);
		}
		if (mknod(name, mode, (int) curr_inode->di_rdev) < 0)
		{
			msg("%s: ", name);
			perror(MSGSTR(CANTCRS, "cannot create special file"));
			skipfile();
			return(FAIL);
		}

		(void) chown(name, curr_inode->di_uid, curr_inode->di_gid);
		(void) chmod(name, mode);
		skipfile();
		xutimes(name, timev);
		return(GOOD);

	case IFIFO:
		vmsg(MSGSTR(EXTFFILE, "extract FIFO special file %s\n"), name);
		if (Nflag == TRUE)
		{
			skipfile();
			return(GOOD);
		}
		if (mkfifo(name, mode) < 0)
		{
			msg("%s: ", name);
			perror(MSGSTR(CANTCRFF,
					"cannot create FIFO special file"));
			skipfile();
			return(FAIL);
		}

		(void) chown(name, curr_inode->di_uid, curr_inode->di_gid);
		(void) chmod(name, mode);
		skipfile();
		xutimes(name, timev);
		return(GOOD);

	case IFREG:
		vmsg(MSGSTR(EXTRFILE, "extract file %s\n"), name);
		if (Nflag == TRUE)
		{
			skipfile();
			return(GOOD);
		}
		if ((ofile = open(name, O_CREAT | O_EXCL | O_WRONLY, 0666)) < 0)
		{
			if (errno == EEXIST)
			{
				switch (overwrite_flag)
				{

				case OVERWRITE_ALWAYS:
					vmsg(MSGSTR(OVRWRT, "%s: OVERWRITING existing file\n"), name);
					ofile = open(name, O_CREAT | O_TRUNC | O_WRONLY, 0666);
					break;

				case OVERWRITE_NEVER:
					break;

				default:
					msg(MSGSTR(FILAEX, "%s: file ALREADY EXISTS\n"), name);
					if (query(MSGSTR(DYWTOTF, "Do you want to overwrite this file")) == YES)
					{
						vmsg(MSGSTR(OVRWRT, "%s: OVERWRITING existing file\n"), name);
						ofile = open(name, O_CREAT | O_TRUNC | O_WRONLY, 0666);
					}
					break;
				}
			}
		}
		if (ofile < 0)
		{
			msg("%s: ", name);
			perror(MSGSTR(CANTCRF, "cannot create file"));
			skipfile();
			return(FAIL);
		}

		(void) fchown(ofile, curr_inode->di_uid, curr_inode->di_gid);
		(void) fchmod(ofile, mode);
		getfile(xtrfile, xtrskip);
		if (close(ofile) != 0)
		{
			msg("%s: ", name);
			msg(MSGSTR(FILECLE, "error closing file\n"));
			perror("extractfile(): close(ofile)");
			return(FAIL);
		}
		xutimes(name, timev);
		return(GOOD);
	}

	/* NOTREACHED */

}

/*
 * skip over bit maps on the tape
 */

void
skipmaps()
{
	while (is_type(&spcl, TS_CLRI) == TRUE ||
	       is_type(&spcl, TS_BITS) == TRUE)
	{
		skipfile();
	}
}

/*
 * skip over a file on the tape
 */

void
skipfile()
{
	curr_action = SKIP;
	getfile(NULLVOIDFP, NULLVOIDFP);
}

/*
 * Do the file extraction, calling the supplied functions with the blocks
 */

void
getfile(fn1, fn2)
	void		(*fn1)(), (*fn2)();
{
	register int	i;
	int		curblk = 0;
	off_t		size = spcl.c_dinode.di_size;
	static char	clearedbuf[MAXBSIZE];
	char		buf[MAXBSIZE / TP_BSIZE][TP_BSIZE];
	char		junk[TP_BSIZE];

	if (is_type(&spcl, TS_END) == TRUE)
	{
		panic(MSGSTR(ENDRUN, "ran off end of tape\n"));
	}
	if (ishead(&spcl) == FAIL)
	{
		panic(MSGSTR(NOTATB, "not at beginning of a file\n"));
	}
	if (gettingfile == FALSE && setjmp(restart) != 0)
	{
		return;
	}
	gettingfile = TRUE;

loop:
	for (i = 0; i < spcl.c_count; ++i)
	{
		if (spcl.c_addr[i])
		{
			readtape(&buf[curblk][0]);
			++curblk;
			if (curblk == fssize / TP_BSIZE)
			{
				if (fn1 != NULLVOIDFP)
				{
					(*fn1)(buf, (size > TP_BSIZE)? (long) (fssize): (curblk - 1) * TP_BSIZE + size);
				}
				curblk = 0;
			}
		}
		else
		{
			if (curblk > 0)
			{
				if (fn1 != NULLVOIDFP)
				{
					(*fn1)(buf, (size > TP_BSIZE)? (long) (curblk * TP_BSIZE): (curblk - 1) * TP_BSIZE + size);
				}
				curblk = 0;
			}
			if (fn2 != NULLVOIDFP)
			{
				(*fn2)(clearedbuf, (size > TP_BSIZE)? (long) TP_BSIZE: size);
			}
		}
		if ((size -= TP_BSIZE) <= 0)
		{
			for (++i; i < spcl.c_count; ++i)
			{
				if (spcl.c_addr[i])
				{
					readtape(junk);
				}
			}
			break;
		}
	}
	if (readhdr(&spcl) == GOOD && size > 0)
	{
		if (is_type(&spcl, TS_ADDR) == TRUE)
		{
			goto loop;
		}
		dmsg("Missing address (header) block for %s\n", curr_file_name);
	}
	if (curblk > 0)
	{
		if (fn1 != NULLVOIDFP)
		{
			(*fn1)(buf, (curblk * TP_BSIZE) + size);
		}
	}
	findinode(&spcl);
	gettingfile = FALSE;
}

/*
 * The next routines are called during file extraction to put the data into the
 * right form and place.
 */

void
xtrfile(buf, size)
	char	       *buf;
	long		size;
{
	if (Nflag == TRUE)
	{
		return;
	}
	if (write(ofile, buf, (int) size) == -1)
	{
		msg(MSGSTR(WRITEEE, "write error extracting inode %d, name %s\n"), curr_inumber, curr_file_name);
		perror("xtrfile(): write()");
		Exit(1);

		/* NOTREACHED */
	}
}

void
xtrskip(buf, size)
	char	       *buf;
	long		size;
{

#if	lint

	buf = buf;

#endif

	if (lseek(ofile, (off_t)size, 1) == (off_t) -1)
	{
		msg(MSGSTR(SEEKERR, "seek error extracting inode %d, name %s\n"), curr_inumber, curr_file_name);
		perror("xtrskip(): lseek()");
		Exit(1);

		/* NOTREACHED */
	}
}

void
xtrlnkfile(buf, size)
	char	       *buf;
	long		size;
{
	pathlen += size;
	if (pathlen > MAXPATHLEN)
	{
		msg(MSGSTR(SYMLONG, "symbolic link name: %s->%s%s; too long %d\n"), curr_file_name, lnkbuf, buf, pathlen);
		Exit(1);

		/* NOTREACHED */
	}
	(void) strcat(lnkbuf, buf);
}

void
xtrlnkskip(buf, size)
	char	       *buf;
	long		size;
{

#if	lint

	buf = buf, size = size;

#endif

	msg(MSGSTR(UNALLOCSY, "unallocated block in symbolic link %s\n"), curr_file_name);
	Exit(1);

	/* NOTREACHED */
}

void
xtrmap(buf, size)
	char	       *buf;
	long		size;
{
	memcpy(map, buf, (size_t) size);
	map += size;
}

void
xtrmapskip(buf, size)
	char	       *buf;
	long		size;
{

#if	lint

	buf = buf;

#endif

	panic(MSGSTR(HOLE, "hole in map\n"));
	map += size;
}

/*
 * Do the tape i/o, dealing with volume changes etc..
 */

static void
readtape(b)
	char	       *b;
{
	register long	i;
	long		rd, newvol;
	int		cnt;

	if (bct < ntrec)
	{
		memcpy(b, &tbf[bct * TP_BSIZE], (size_t) TP_BSIZE);
		++bct;
		++blksread;
		return;
	}
	for (i = 0; i < ntrec; ++i)
	{
		((struct s_spcl *) & tbf[i * TP_BSIZE])->c_magic = 0;
	}
	bct = 0;
	cnt = ntrec * TP_BSIZE;
	rd = 0;

getmore:

#if	REMOTE

	i = rmtread(&tbf[rd], cnt);

#else

	i = read(mt, &tbf[rd], cnt);

#endif

	if (i > 0 && i != ntrec * TP_BSIZE)
	{
		if (pipe_in_flag == TRUE)
		{
			rd += i;
			cnt -= i;
			if (cnt > 0)
			{
				goto getmore;
			}
			i = rd;
		}
		else
		{
			if (i % TP_BSIZE != 0)
			{
				panic(MSGSTR(PARTIALB, "partial block read: %d should be %d\n"), i, ntrec * TP_BSIZE);
			}
			memcpy(&tbf[i], (char *) &endoftapemark, (size_t) TP_BSIZE);
		}
	}
	if (i < 0)
	{
		msg(MSGSTR(TAPEREADW, "Tape read error while "));
		switch (curr_action)
		{
		case UNKNOWN:
			msg(MSGSTR(RESYNC, "trying to resynchronize\n"));
			break;

		case EXTRACTING:
			msg(MSGSTR(RESTOR, "restoring %s\n"), curr_file_name);
			break;

		case SKIP:
			msg(MSGSTR(SKIPI, "skipping over inode %d\n"), curr_inumber);
			break;

		default:
			msg(MSGSTR(SETUPT, "trying to set up tape\n"));
			break;
		}
		if (auto_retry_flag == FALSE && query(MSGSTR(CONTIN, "continue")) == NO)
		{
			Exit(1);

			/* NOTREACHED */
		}
		i = ntrec * TP_BSIZE;
		memset(tbf, (int) '\0', (size_t) i);

#if	REMOTE

		if (rmtseek(i, 1) < 0)

#else

		if (lseek(mt, (off_t)i, 1) == (off_t) -1)

#endif

		{
			perror(MSGSTR(CONTFAIL, "continuation failed"));
			Exit(1);

			/* NOTREACHED */
		}
	}
	if (i == 0)
	{
		if (pipe_in_flag == FALSE)
		{
			newvol = volno + 1;
			volno = 0;
			getvol(newvol);
			readtape(b);
			return;
		}
		if (rd % TP_BSIZE != 0)
		{
			panic(MSGSTR(PARTIALB, "partial block read: %d should be %d\n"), rd, ntrec * TP_BSIZE);
		}
		memcpy(&tbf[rd], (char *) &endoftapemark, (size_t) TP_BSIZE);
	}
	memcpy(b, &tbf[bct * TP_BSIZE], (size_t) TP_BSIZE);
	++bct;
	++blksread;
}

static void
findtapeblksize()
{
	register long	i;

	for (i = 0; i < ntrec; ++i)
	{
		((struct s_spcl *) & tbf[i * TP_BSIZE])->c_magic = 0;
	}
	bct = 0;

#if	REMOTE

	i = rmtread(tbf, ntrec * TP_BSIZE);

#else

	i = read(mt, tbf, ntrec * TP_BSIZE);

#endif

	if (i <= 0)
	{
		perror(MSGSTR(TAPEREADE, "Tape read error"));
		Exit(1);

		/* NOTREACHED */
	}
	if (i % TP_BSIZE != 0)
	{
		msg(MSGSTR(TAPEBNOT, "Tape block size (%d) is not a multiple of dump block size (%d)\n"), i, TP_BSIZE);
		Exit(1);

		/* NOTREACHED */
	}
	ntrec = i / TP_BSIZE;
	vmsg(MSGSTR(TAPEBI, "Tape block size is %d\n"), ntrec);
}

static void
flsht()
{
	bct = ntrec + 1;
}

void
closemt()
{
	if (mt < 0)
	{
		return;
	}

#if	REMOTE

	rmtclose();

#else

	if (close(mt) != 0)
	{
		msg(MSGSTR(FILECLE, "error closing file\n"));
		perror("closemt(): close(mt)");
	}

#endif

}

static int
is_volume(b, t)
	struct s_spcl  *b;
	long		t;
{
	if (b->c_volume != t)
	{
		return(FALSE);
	}
	return(TRUE);
}

static int
readhdr(b)
	struct s_spcl  *b;
{
	if (gethead(b) == FAIL)
	{
		dmsg("readhdr fails at %d blocks\n", blksread);
		return(FAIL);
	}
	return(GOOD);
}

/*
 * read the tape into buf, then return whether or or not it is a header block.
 */

static int
gethead(buf)
	struct s_spcl  *buf;
{
	long		i, *j;
	union u_ospcl
	{
		char		dummy[TP_BSIZE];
		struct s_ospcl
		{
			long		c_type;
			long		c_date;
			long		c_ddate;
			long		c_volume;
			long		c_tapea;
			u_short		c_inumber;
			long		c_magic;
			long		c_checksum;
			struct odinode
			{
				unsigned short	odi_mode;
				u_short		odi_nlink;
				u_short		odi_uid;
				u_short		odi_gid;
				long		odi_size;
				long		odi_rdev;
				char		odi_addr[36];
				long		odi_atime;
				long		odi_mtime;
				long		odi_ctime;
			}		c_dinode;
			long		c_count;
			char		c_addr[256];
		}		s_ospcl;
	}		u_ospcl;

	if (old_format_flag == FALSE)
	{
		readtape((char *) buf);
		if (buf->c_magic != NFS_MAGIC)
		{
			if (swabl(buf->c_magic) != NFS_MAGIC)
			{
				return(FAIL);
			}
			if (byte_swap_flag == FALSE)
			{
				vmsg(MSGSTR(SWAP, "Note: Doing Byte swapping\n"));
				byte_swap_flag = TRUE;
			}
		}
		if (checksum((int *) buf) == FAIL)
		{
			return(FAIL);
		}
		if (byte_swap_flag == TRUE)
		{
			swabst("8l4s31l", (char *) buf);
		}
		goto good;
	}
	readtape((char *) (&u_ospcl.s_ospcl));
	memset((char *) buf, (int) '\0', (size_t) TP_BSIZE);
	buf->c_type = u_ospcl.s_ospcl.c_type;
	buf->c_date = u_ospcl.s_ospcl.c_date;
	buf->c_ddate = u_ospcl.s_ospcl.c_ddate;
	buf->c_volume = u_ospcl.s_ospcl.c_volume;
	buf->c_tapea = u_ospcl.s_ospcl.c_tapea;
	buf->c_inumber = u_ospcl.s_ospcl.c_inumber;
	buf->c_checksum = u_ospcl.s_ospcl.c_checksum;
	buf->c_magic = u_ospcl.s_ospcl.c_magic;
	buf->c_dinode.di_mode = u_ospcl.s_ospcl.c_dinode.odi_mode;
	buf->c_dinode.di_nlink = u_ospcl.s_ospcl.c_dinode.odi_nlink;
	buf->c_dinode.di_uid = u_ospcl.s_ospcl.c_dinode.odi_uid;
	buf->c_dinode.di_gid = u_ospcl.s_ospcl.c_dinode.odi_gid;
	buf->c_dinode.di_size = u_ospcl.s_ospcl.c_dinode.odi_size;
	buf->c_dinode.di_rdev = u_ospcl.s_ospcl.c_dinode.odi_rdev;
	buf->c_dinode.di_atime = u_ospcl.s_ospcl.c_dinode.odi_atime;
	buf->c_dinode.di_mtime = u_ospcl.s_ospcl.c_dinode.odi_mtime;
	buf->c_dinode.di_ctime = u_ospcl.s_ospcl.c_dinode.odi_ctime;
	buf->c_count = u_ospcl.s_ospcl.c_count;
	memcpy(buf->c_addr, u_ospcl.s_ospcl.c_addr, (size_t) 256);
	if (u_ospcl.s_ospcl.c_magic != OFS_MAGIC ||
	    checksum((int *) (&u_ospcl.s_ospcl)) == FAIL)
	{
		return(FAIL);
	}
	buf->c_magic = NFS_MAGIC;

good:
#ifdef __alpha
	j = &(buf->c_dinode.di_qsize);
	i = *j;
#else
	j = (long *)(buf->c_dinode.di_qsize.val);
	i = j[1];
#endif

	if (buf->c_dinode.di_size == 0 &&
	    (buf->c_dinode.di_mode & IFMT) == IFDIR && quad_swap_flag == FALSE)
	{
		if (*j || i)
		{
			printf(MSGSTR(SWAPQ, "Note: Doing Quad swapping\n"));
			quad_swap_flag = TRUE;
		}
	}
	if (quad_swap_flag == TRUE)
	{
		j[1] = *j;
		*j = i;
	}

	switch (buf->c_type)
	{

		/*
		 * Have to patch up missing information in bit map headers
		 */

	case TS_CLRI:
	case TS_BITS:

		buf->c_inumber = 0;
		buf->c_dinode.di_size = buf->c_count * TP_BSIZE;
		for (i = 0; i < buf->c_count; ++i)
		{
			++(buf->c_addr[i]);
		}
		break;

	case TS_TAPE:
	case TS_END:
		buf->c_inumber = 0;
		break;

	case TS_INODE:
	case TS_ADDR:
		break;

	default:
		panic(MSGSTR(GETHEAD, "gethead: unknown inode type %d\n"), buf->c_type);
		break;
	}
	if (debug_flag == TRUE)
	{
		accthdr(buf);
	}
	return(GOOD);
}

/*
 * Check that a header is where it belongs and predict the next header
 */

static void
accthdr(header)
	struct s_spcl  *header;
{
	static ino_t	previno = 0x7fffffff;
	static int	prevtype;
	static long	predict;
	long		blks, i;

	if (header->c_type == TS_TAPE)
	{
		msg(MSGSTR(VOLHEAD, "Volume header\n"));
		previno = 0x7fffffff;
		return;
	}
	if (previno == 0x7fffffff)
	{
		goto newcalc;
	}

	switch (prevtype)
	{
	case TS_BITS:
		msg(MSGSTR(MASKH, "Dump mask header"));
		break;

	case TS_CLRI:
		msg(MSGSTR(REMMASK, "Remove mask header"));
		break;

	case TS_INODE:
		msg(MSGSTR(FILEHE, "File header, ino %d"), previno);
		break;

	case TS_ADDR:
		msg(MSGSTR(FILECON, "File continuation header, ino %d"), previno);
		break;

	case TS_END:
		msg(MSGSTR(EOTHE, "End of tape header"));
		break;
	}
	if (predict != blksread - 1)
	{
		msg(MSGSTR(PREDICT, "; predicted %d blocks, got %d blocks"), predict, blksread - 1);
	}
	msg("\n");
newcalc:
	blks = 0;
	if (header->c_type != TS_END)
	{
		for (i = 0; i < header->c_count; ++i)
		{
			if (header->c_addr[i] != 0)
			{
				++blks;
			}
		}
	}
	predict = blks;
	blksread = 0;
	prevtype = header->c_type;
	previno = header->c_inumber;
}

/*
 * Find an inode header.  Complain if had to skip.
 */

static void
findinode(header)
	struct s_spcl  *header;
{
	static long	skipcnt = 0;
	long		i;
	char		buf[TP_BSIZE];

	curr_file_name = "<name unknown>";
	curr_action = UNKNOWN;
	curr_inode = (struct dinode *)NULL;
	curr_inumber = 0;
	if (ishead(header) == FAIL)
	{
		++skipcnt;
		while (gethead(header) == FAIL || header->c_date != dumpdate)
		{
			++skipcnt;
		}
	}
	for (;;)
	{
		if (is_type(header, TS_ADDR) == TRUE)
		{
			/*
			 * Skip up to the beginning of the next record
			 */

			for (i = 0; i < header->c_count; ++i)
			{
				if (header->c_addr[i] != 0)
				{
					readtape(buf);
				}
			}
			(void) gethead(header);
			continue;
		}
		if (is_type(header, TS_INODE) == TRUE)
		{
			curr_inode = &header->c_dinode;
			curr_inumber = header->c_inumber;
			break;
		}
		if (is_type(header, TS_END) == TRUE)
		{
			curr_inumber = maxino;
			break;
		}
		if (is_type(header, TS_CLRI) == TRUE)
		{
			curr_file_name = "<file removal list>";
			break;
		}
		if (is_type(header, TS_BITS) == TRUE)
		{
			curr_file_name = "<file dump list>";
			break;
		}
		while (gethead(header) == FAIL)
		{
			++skipcnt;
		}
	}
	if (skipcnt > 0)
	{
		msg(MSGSTR(RESYNCR, "resync restore, skipped %d blocks\n"), skipcnt);
	}
	skipcnt = 0;
}

/*
 * return whether or not the buffer contains a header block
 */

static int
ishead(buf)
	struct s_spcl  *buf;
{
	if (buf->c_magic != NFS_MAGIC)
	{
		return(FAIL);
	}
	return(GOOD);
}

static int
is_type(spcl_rec, type)
	struct s_spcl  *spcl_rec;
	int		type;
{
	if (spcl_rec->c_type != type)
	{
		return(FALSE);
	}
	return(TRUE);
}

static int
checksum(b)
	register int   *b;
{
	register int	i, j;

	j = sizeof(union u_spcl) / sizeof(int);
	i = 0;
	if (byte_swap_flag == FALSE)
	{
		do
		{
			i += *b;
			++b;
			--j;
		}
		while (j != 0);
	}
	else
	{
		/*
		 * What happens if we want to read restore tapes for a 16bit
		 * int machine???
		 */

		do
		{
			i += swabl(*b);
			++b;
			--j;
		}
		while (j != 0);
	}

	if (i != CHECKSUM)
	{
		msg(MSGSTR(CHKSUM, "Checksum error %o, inode %d file %s\n"), i, curr_inumber, curr_file_name);
		return(FAIL);
	}
	return(GOOD);
}

/* VARARGS1 */

void
msg(va_alist)
	va_dcl
{
	va_list		ap;
	char	       *fmt;

	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void) fflush(stderr);
}

void
swabst(cp, sp)
	register char  *cp, *sp;
{
	int		repetitions = 0;
	char		c;

	while (*cp)
	{
		switch (*cp)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			repetitions = (repetitions * 10) + (*cp - '0');
			++cp;
			continue;

		case 's':
		case 'w':
		case 'h':
			c = sp[0];
			sp[0] = sp[1];
			sp[1] = c;
			sp += 2;
			break;

		case 'l':
			c = sp[0];
			sp[0] = sp[3];
			sp[3] = c;
			c = sp[2];
			sp[2] = sp[1];
			sp[1] = c;
			sp += 4;
			break;

		default:

			/* Any other character, like 'b' counts as
			 * byte. */

			++sp;
			break;
		}
		--repetitions;
		if (repetitions <= 0)
		{
			repetitions = 0;
			++cp;
		}
	}
}

static unsigned long
swabl(x)
{
	unsigned long	l = x;

	swabst("l", (char *) &l);
	return(l);
}

#if	AIX

int
isdiskette(media)
	char	       *media;
{
	int		fd;

	fd = open(media, O_RDONLY);
	if (fd < 0)
	{
		msg(MSGSTR(MEDIAOPEN, "Can't open %s\n"), media);
		Exit(1);

		/* NOTREACHED */
	}
	if (ioctl(fd, IOCINFO, &devinfo) < 0)
	{
		msg(MSGSTR(IOCFAIL, "Can't get info on %s\n"), media);
		close(fd);
		Exit(1);

		/* NOTREACHED */
	}
	if ((devinfo.devtype == DD_DISK) && ((devinfo.flags & DF_FIXED) == 0))
	{
		close(fd);
		return(1);
	}
	close(fd);
	return(0);
}

#endif	AIX


#if	REMOTE

void
abort_restore()
{
	msg(MSGSTR(REMABORT, "Remote restore is aborted.\n"));
	Exit(1);
}



void
restore_perror(where)
char *where;
{
#if     EDUMP

        msg("%s: %s\n", where, errmsg(errno));

#else   ! EDUMP

        perror(where);

#endif  ! EDUMP

}

int blocks_per_write = 2;

#endif	REMOTE
