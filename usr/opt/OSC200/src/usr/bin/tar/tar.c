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
static char	*sccsid = "@(#)$RCSfile: tar.c,v $ $Revision: 4.2.12.4 $ (DEC) $Date: 1993/10/11 19:12:29 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * tar.c
 *
 * Modification History:
 *
 * 12-Jan-91	Fred Canter
 *	Changed the default tape device from rmt8 to rmt0h.
 *
 * 06-Mar-1991	Dave Scoda
 *	Changed the initial read size, for character devices, from 1 to
 *	20 blocks to allow "auto-sizing" the acutal blocking factor.
 *
 * 23-Apr-91	Sam Lambert
 *	Added 'U' option for reading ULTRIX/BSD multivolume archives.
 *
 * 07-Jun-91	Sam Lambert
 *	Added 'R' option to support list of files to be processed.
 *	This function only supported on -c, -r, and -u operations.
 *
 * 16-Jul-91	Sam Lambert
 *	Include "-oflag" support from OSF/1 V1.0-1 kit.
 *	(That is, if -o specified on xtract, restore files to 
 *	current user's uid/gid, not to those in archive.)
 *
 * 30-Jul-91	Sam Lambert
 *	Changes dealing with blocking factors.  Allow > 20,
 *	allow other than 20, and get rid of message reporting
 *	blocking factor unless -v is specified.
 *
 * 02-Oct-91	Sam Lambert
 *	"Autosize" on read up to maximum blocksize.
 *
 * 28-Oct-91	Sam Lambert
 *	Fix problem which occurred when -Rflag was specified and stdout is 
 *	used for output.
 *
 * 05-Dec-91	Sam Lambert
 *	Remove requirement for input archives to be modulo 512 bytes in length.
 *	(Ugh.)  See additional comments in readtbuf().
 *
 * 05-Jan-92	Fred Canter
 *	Correct an error in the code for autosizing the blocking factor.
 *	The nblock variable was not set if vflag was not set and
 *	the block size was equal to the default (DBLOCK). This caused
 *	tar x or tar t of any tape archive with the default blocking
 *	factor to fail (only extract the first few files), but not
 *	print an error message.
 *
 * 24-Apr-92    Anna Salzberg
 * 	Fixed truncation of file names larger than 100 characters problem
 *      (function tomodes()).
 *
 * 05-May-92    Anna Salzberg
 *      Set user and group ids of the extracted file to the process' effective
 *      user and group ids when used by a non-privileged process to satisfy  
 *      posix and xopen standards.
 *
 * 11-May-92    Anna Salzberg
 *      Fixed over-backtracking of a file (via negative lseek) when using 
 *      flags r and f, and length of file greater than TBLOCK*DBLOCK. 
 *
 * 12-May-92    Anna Salzberg
 *      Now permissions of a directory are set after restoring its contents.
 *      This way if the directory has its permissions changed after it 
 *	already contained a non-empty substructure, extracting with p flag 
 *	works.
 *
 * 13-May-92    Anna Salzberg
 *      When stderr is set to line buffered, fprintf calls with partial lines
 *      cannot be freely mixed with calls to perror, which does not use
 *      stdio buffering.  Fixed problem by replacing vfile = stderr; with
 *      vfile = fdopen (dup(2), "a");
 *
 * 18-May-92    Anna Salzberg
 *	Fixed restoration of mtime so that now it works with rflag (disregard
 *	comments in dodirtimes() since that function is no longer used).
 *
 * 05-Aug-92    Anna Salzberg
 *      Fixed out of memory handling in putfile(). 
 */


#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: tar
 *
 * ORIGINS: 26, 27
 *
 * "tar.c	1.29  com/cmd/arch,3.1,9021 4/24/90 12:58:05";
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/* tar.c	5.1 15:44:05 8/16/90 SecureWare */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>
#endif

#include <stdio.h>
#include <sys/limits.h>
#include <tar.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <locale.h>
#include <nl_types.h>

#include "tar_msg.h"

#define MSGSTR(num,str)	catgets(catd,MS_TAR,num,str)
nl_catd	catd;		/*  message catalog descriptor  */

#define TBLOCK		512	/* tar block size  */
#define DBLOCK    	20	/* default number of TBLOCKs */
#define NBLOCK		128	/* maximum number of blocks */
#define	TAR_PATH_LEN	257	/* max size of tar archive path with \0  */
#define NAME_SIZE	100	/* file name size in tar header  */
#define	PREFIX_SIZE	155	/* path size in tar header  */
#define SUCCESS		0	/* success return value */
#define FAILURE		-1	/* failure return value */

#define	writetape(b)	writetbuf(b, 1)
#define	min(a,b)  ((a) < (b) ? (a) : (b))
#define	max(a,b)  ((a) > (b) ? (a) : (b))

			/*  hblock modified for POSIX compliance  */
#define UNAMELEN 32
#define GNAMELEN 32

union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAME_SIZE];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char typeflag;
		char linkname[NAME_SIZE];
		char magic[6];
		char version[2];
		char uname[UNAMELEN];
		char gname[GNAMELEN];
		char devmajor[8];
		char devminor[8];
		char prefix[PREFIX_SIZE];
	} dbuf;
};

struct linkbuf {
	ino_t	inum;
	dev_t	devnum;
	int	count;
	char	pathname[NAME_SIZE+1];		/*  +1 to allow for \0  */
struct	linkbuf *nextp;
};

struct elem {
	struct elem *next;
	char *data;
};

struct list {
	struct elem *first;
	struct elem *last;
};

struct dir_list{
	char	name[TAR_PATH_LEN+1];
	mode_t	mode;
	time_t  mtime;
	struct dir_list *next;
};

struct list except_list;
int	writingdir;
char	*stripstring;
char	*stripprefix();
char	*list_first();

union	hblock dblock;
union	hblock *tbuf;
struct	linkbuf *ihead;
struct	stat stbuf;
			/* tar functions:			*/
int	cflag;		/*	Write to archive - create	*/
int	rflag;		/*	Write to archive - append	*/
int	tflag;		/*	List contents of archive - table*/
int	xflag;		/*	Xtract from archive		*/
			/* tar options:				*/
int	bflag;		/*	blocking factor			*/
int	fflag;		/*	name of archive file		*/
int	hflag;		/*	follow symbolic links		*/
int	iflag;		/*	ignore checksum errors		*/
int	mflag;		/*	don't restore modification times*/
int	oflag;		/*	don't archive directory info	*/
			/*	on xtract, don't restore uid/gid */
int	pflag;		/*	restore orig file modes		*/
int	sflag;		/*	strip leading "/" if present - xtract */
int	uflag;		/*  add files to archive if not there */
int	vflag;		/*	verbose info			*/
int	wflag;		/*	wait for user confirmation 	*/
int	Bflag;		/*	force blocking to blocking factor */
int	Lflag;		/*	create symlink if hard link fails - xtract */
int	Fflag;		/*	filter out certain file names 	*/
int	Sflag;		/*	size of tape			*/
int	Pflag;		/*	strip specified prefix		*/
int	Rflag;		/*	accept list of files to process */
int	Uflag;		/*	process ULTRIX/BSD multivolume archive */

int	mt;
int	term;
long	trecs;		/*  number of records written to current volume  */
int	recno;
int	first;
int	prtlinkerr;
int	freemem = 1;
int	nblock = 0;
char	startcwd[PATH_MAX];
char	full_name_buf[TAR_PATH_LEN+1];	/* extra char for trailing '/' for directories */
char	*full_name = full_name_buf;
size_t	name_size = NAME_SIZE-1;  /* Actual size of names w/o null at end */
int	dev_min, dev_maj;
int	multivol;
int	tape_changed;
int	pipein = 0;		/* Set if reading from a pipe */

int	onintr(void);
int	onquit(void);
int	onhup(void);
int	onterm(void);

daddr_t	low;
daddr_t	high;
daddr_t	bsrch();

				/*  tape info defaults  */
#define GAP	7500          /* Interrecord gap, .0001 inch units */
#define	DENSITY	1200
#define	TAPE_FOOT	110000L		/*  11 inches  */
int	density = DENSITY ;
int	tapelen ;
int	bptape ;
int	rptape ;
int	lastread = 0;

FILE	*vfile = stdout;
FILE	*tfile;
FILE	*fp;			/* file pointer for Rflag */

char	tname[] = "/tmp/tarXXXXXX";
char	usefile [PATH_MAX+1] = "/dev/rmt0h";	/*  +1 for \0 char  */
char	listfile[PATH_MAX+1];			/* list file for -R flag */

extern	void	*malloc();
extern	char	*getwd();
extern	time_t	time();
extern	char	*mktemp();

size_t		strftime();
struct tm	*localtime();

char		*getpwd();
char		*getmem();


main(argc, argv)
int	argc;
char	*argv[];
{
	int status;
	char		*cp;


	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_TAR,NL_CAT_LOCALE);
	if (argc < 2)
		usage();

#if SEC_BASE
	ie_init(argc, argv, 0);
#endif
	list_empty(&except_list);
	tfile = NULL;
	argv[argc--] = NULL;
	argv++;

	for (cp = *argv++, argc--; *cp; cp++) 
		switch(*cp) {

		case 'o':
			oflag++;
			name_size = NAME_SIZE - 1;
			break;

		case 'n':
			/*
			 * older versions of tar expect null-terminated strings
			 * in the name and linkname fields.  POSIX specifies
			 * that names & linknames will not be terminated by a
			 * null if the string fits exactly in the header array.
			 * This option allows archives to be created with this
			 * new POSIX "feature".  The 'n' & 'o' options are
			 * mutually exclusive.
			 */
			oflag = 0;
			name_size = NAME_SIZE;
			break;

		case 's':
			sflag++;
			break;

		case 'f':
			if (! *argv) {
				fprintf(stderr, MSGSTR(EFFLAG,
			"tar: file must be specified with 'f' option\n"));
				usage();
			}
			if (strlen(*argv) > PATH_MAX) {
				fprintf(stderr, MSGSTR(ELONGFN,
					"tar: %s: file name too long\n"),*argv);
				exit(1);
			}
			strcpy(usefile, *argv++); argc--;
			if (strcmp(usefile, "-") == 0)
				pipein++;
			fflag++;
			break;

		case 'c':
			cflag++;
			rflag++;
			break;

		case 'p':
			pflag++;
			break;
		
		case 'u':
			mktemp(tname);
			if ((tfile = fopen(tname, "w")) == NULL) {
				fprintf(stderr, MSGSTR(ETCRTMP,
				 "tar: cannot create temporary file (%s)\n"),
				 tname);
				done(1);
			}
			uflag++;
			fprintf(tfile, "!!!!!/!/!/!/!/!/!/! 000\n");
			/*FALL THRU*/

		case 'r':
			rflag++;
			break;

		case 'v':
			vflag++;
			break;

		case 'w':
			wflag++;
			break;

		case 'x':
			xflag++;
			break;

		case 't':
			tflag++;
			break;

		case 'm':
			mflag++;
			break;

		case '-':
			break;

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
			usefile[strlen(usefile)-2] = *cp;
			break;

		case 'b':
			if (! *argv) {
				fprintf(stderr, MSGSTR(ETNOBSIZE,
			"tar: blocksize must be specified with 'b' option\n"));
				usage();
			}
			nblock = atoi(*argv);
			if (nblock <= 0 || nblock > NBLOCK ) {
				fprintf(stderr, MSGSTR ( EBSIZE,
				    "tar: invalid blocksize \"%s\"\n" ), *argv);
				done(1);
			}
		 	argv++; argc--;
			bflag++;
			break;

		case 'l':
			prtlinkerr++;
			break;

		case 'h':
			hflag++;
			break;

		case 'i':
			iflag++;
			break;

		case 'B':
			Bflag++;
			break;

		case 'F':
			Fflag++;
			break;

		case 'L':
			Lflag++;
			break;

		case 'P':	
			if (! *argv) {
				fprintf(stderr, MSGSTR(ETNOPREFIX,
			"tar: prefix must be specified with 'P' option\n"));
				usage();
			}
			stripstring = *argv++; argc--;
			Pflag++;
			break;

		case 'R':
			if (! *argv) {
				fprintf(stderr, MSGSTR(ERFLAG,
			"tar: a file name must be specified with the 'R' option\n"));
				usage();
			}
			if (strlen(*argv) > PATH_MAX) {
				fprintf(stderr, MSGSTR(ELONGFN,
					"tar: %s: file name too long\n"),*argv);
				exit(1);
			}
			strcpy(listfile, *argv++); argc--;
			Rflag++;
			break;

		case 'S':
			if (! *argv) {
				fprintf(stderr, MSGSTR(ETNOTAPESIZE,
			"tar: tape size must be specified with 'S' option\n"));
				usage();
			}
			{
			int	len;
			char	*mark;
			len = strlen(*argv);
			if ((*argv)[--len] == 'b') {
				(*argv)[len] = '\0';
				bptape = atoi(*argv);
			}
			else {
				tapelen = atoi(*argv);
				mark = strchr(*argv, '@');
				if (mark)
					density = atoi(++mark);
			}
			}
			argv++; argc--;
			Sflag++;
			break;

		case 'U':
			Uflag++;
			break;

		default:
			usage();
		}

	if (!rflag && !xflag && !tflag)
		usage();
	if (rflag + tflag + xflag > 1)
		usage();
	if ( mflag && tflag )
		usage();
	if ( (prtlinkerr && !cflag) && (prtlinkerr && !rflag)
			 && (prtlinkerr && !uflag) )
				usage();
	if (Uflag && rflag) {	/* Uflag valid only on -t and -x */
		usage();
	}

	if (Rflag) {
		if ((fp = fopen(listfile, "r")) == NULL) {
			fprintf(stderr, MSGSTR(ETOPEN, "tar: cannot open "));
			perror(listfile);
			done(1);
		}
		/*
		 * Create new argument list with additional file names.
		 */
		if (make_argv (&argv, &argc) != SUCCESS) {
			done(1);
		}
	}
	if (rflag) {
		if (cflag && tfile != NULL)
			usage();
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			(void) signal(SIGINT, (void (*)(int))onintr);
		if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
			(void) signal(SIGHUP, (void (*)(int))onhup);
		if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
			(void) signal(SIGQUIT, (void (*)(int))onquit);
		if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
			(void) signal(SIGTERM, (void (*)(int))onterm);
		mt = openmt(usefile, 1);
		dorep(argv);
		if (close(mt) == -1) {
			perror (MSGSTR(ETCLOSE, "tar: close error"));
			done(2);
		}
		done(0);
	}
	mt = openmt(usefile, 0);

	if ( xflag ) {
		doxtract(argv);
	}
	else
		dotable(argv);

	/* read second null block if standard input */
	if (pipein)
		readtape((char *)&dblock);

	if (close(mt) == -1) {
		perror (MSGSTR(ETCLOSE, "tar: close error"));
		done(2);
	}
	done(0);
}

usage()
{
	fprintf(stderr, MSGSTR(USAGE1,
	  "usage: tar [-]{crtux}[bfhilmnopsvwBLFPRSU0-9] [blocking] [tarfile]"));
	fprintf(stderr, MSGSTR(USAGE2,
	  " [prefix]\n\t\t\t\t   [listfile] [feet[@density]|<blocks>b]\n"));
	fprintf(stderr, MSGSTR(USAGE3,
	  "\t\t\t\t\t    [-C directory] [-e except] file ...\n"));
	done(1);
}

int
openmt(tape, writing)
	char *tape;
	int writing;
{
	struct stat sb;

	if (pipein) {
		/*
		 * Read from standard input or write to standard output.
		 */
#if SEC_BASE
		if (writing)
		    fprintf(stderr, MSGSTR(ESECSTDOUT,
			"tar: cannot write archives to standard output\n"));
		else
		    fprintf(stderr, MSGSTR(ESECSTDIN,
			"tar: cannot read archives from standard input\n"));
		fprintf(stderr, MSGSTR(ESECUSEF,
			"\tplease use -f tapefile option\n"));
		done(1);
#else
		if (writing) {
			if (cflag == 0) {
				fprintf(stderr, MSGSTR( ESTDOUT,
			 "tar: can only create standard output archives\n"));
				done(1);
			}
			vfile = fdopen (dup(2), "a");
			setlinebuf(vfile);
			mt = dup(1);
		} else {
			mt = dup(0);
			Bflag++;
		}
		multivol = 0;	/* Disable multi-vol processing */
#endif
	} else {
		/*
		 * Use file or tape on local machine.
		 */
#if SEC_BASE
		stopio(tape);
		ie_check_device(tape,
			writing ? AUTH_DEV_EXPORT : AUTH_DEV_IMPORT);
#endif
		if (writing) {
			if (cflag)
				mt = open(tape, O_RDWR|O_CREAT|O_TRUNC, 0666);
			else
				mt = open(tape, O_RDWR);
		} else
			mt = open(tape, O_RDONLY);
		if (mt < 0) {
			fprintf(stderr, MSGSTR(ETOPEN, "tar: cannot open "));
			perror(tape);
			done(1);
		}
		multivol = 1;
		if (*tape != '/')	/* Save cwd for multi-vol re-opens */
			(void) getpwd(startcwd);
	}
	if (multivol) {
		if (fstat(mt, &sb) != 0) {
			fprintf(stderr, MSGSTR(ESTAT,
				"tar: could not stat file = %s\n"), tape);
			done(1);
		}
		/* Only character devices are allowed multi-vol processing */
		multivol = S_ISCHR(sb.st_mode);

		/* If did not use fflag, expecting a tape device (man pgs) */
		if (vflag && !S_ISCHR(sb.st_mode)&&!S_ISBLK(sb.st_mode)&&!fflag)
			fprintf(vfile, MSGSTR(ENOTSPEC,
				"tar: warning: %s: not a character or block special device\n"), tape);
	}
	return(mt);
}


/*	dorep will create a new archive or append to an existing archive.
 */

dorep(argv)
	char *argv[];
{
	register char *cp2;
	char wdir[PATH_MAX], tempdir[PATH_MAX], *parent;
	int skip = 0;

	if (!cflag) {		/*  request is update or replace  */
		(void) getdir();
		do {
			passtape();
			if (term)
				done(0);
			(void) getdir();
		} while (!endtape());
		backtape();
		if (tfile != NULL) {
			char buf[200];

			sprintf(buf, "sort +0 -1 +1nr %s -o %s; awk '$1 != prev {print; prev=$1}' %s >%sX; mv %sX %s",
				tname, tname, tname, tname, tname, tname);
			fflush(tfile);
			system(buf);
			freopen(tname, "r", tfile);
			fstat((int)fileno(tfile), &stbuf);
			high = stbuf.st_size;
		}
	}

	(void) getpwd(wdir);
	while (*argv && ! term) {
		if (!strcmp(*argv, "-C") && argv[1]) {
			argv++;
			skip = 0;
			if (chdir(*argv) < 0) {
				fprintf(stderr, MSGSTR(ECHDIR,
					"tar: can't change directories to "));
				perror(*argv);
				skip++;
			} else
				(void) getpwd(wdir);
			argv++;
			continue;
		}
		if (!strcmp(*argv, "-e")) {
			argv++;
			if (*argv) {
				list_append(&except_list, *argv);
				argv++;
			}
			continue;
		}
		if (skip && **argv != '/') {
			/*
			 * This arg was meant to apply relative to a current
			 * directory set by "-C <dir>", but the chdir() failed.
			 */
			argv++;
			continue;
		}

		parent = wdir;
		cp2 = rindex(*argv, '/');
		if (!cp2)		/* pathname consists only of filename*/
			cp2 = *argv;
		else if (cp2 != *argv) {	/* there is a path prefix */
			*cp2 = '\0';
			if (chdir(*argv) < 0) {
				fprintf(stderr, MSGSTR(ECHDIR,
					"tar: can't change directories to "));
				perror(*argv);
				continue;
			}
			*cp2 = '/';
			cp2++;
			parent = getpwd(tempdir);
		}
		else {				/* path is / or /dir */
			if (chdir("/") < 0) {
				fprintf(stderr, MSGSTR(ECHDIR,
					"tar: can't change directories to "));
				perror("/");
				continue;
			}
			cp2++;
			parent = "/";
		}

		if (*cp2 == '\0')	/* Handle pathnames ending with '/' */
			cp2="." ;

		putfile(*argv++, cp2, parent);
		if (chdir(wdir) < 0) {
			fprintf(stderr, MSGSTR( ECHBACK,
				"tar: cannot change back?: "));
			perror(wdir);
		}
	}
	putempty();
	putempty();

	if (recno > 0) {		/* flush any remaining buffers */
		/* Clear unused portion of buffer to avoid garbage after  */
		/* the end of archive marker (2 512-byte blocks of zeros) */ 
		bzero ( (char *)&tbuf[recno], (nblock - recno) * TBLOCK); 
		bwrite((char *) tbuf);
	}

	if (prtlinkerr == 0)
		return;
	for (; ihead != NULL; ihead = ihead->nextp) {
		if (ihead->count == 0)
			continue;
		fprintf(stderr, MSGSTR( ELINKS,
			"tar: missing links to %s\n" ), ihead->pathname);
	}
}

endtape()
{
	return (dblock.dbuf.name[0] == '\0');
}


	/*	The function getdir will read a header from the archive
	 *	file. A POSIX header will contain all needed information
	 *	in the correct fields. Non POSIX headers need to be
	 *	checked and completed (e.g. major and minor device # in
	 *	the mtime field).
	 */
getdir()
{
	register struct stat *sp = &stbuf;
	int i, ftype;
	int	chksum;
#ifdef HDEBUG
	int	tmp;
#endif

top:
	readtape((char *)&dblock);
	if (dblock.dbuf.name[0] == '\0')
		return;

#ifdef HDEBUG
	sscanf(dblock.dbuf.size, "%o", &tmp);
	fprintf(stderr, "header\n______\n");
	fprintf(stderr, "name: %s;  size: %d\n", dblock.dbuf.name, tmp);
	fprintf(stderr, "(non-scanned size: %d)\n", dblock.dbuf.size);
	fflush(stderr);
#endif

	sscanf(dblock.dbuf.chksum, "%0o", &chksum);
	if (chksum != (i = checksum())) {
		fprintf(stderr, MSGSTR( ECSUMC,
			"tar: directory checksum error (%d != %d)\n" ),
		    chksum, i);
		if (iflag)		/*  ignore checksum errors */
			goto top;
		done(2);
	}

	sscanf(dblock.dbuf.mode, "%0o", &i);
	sp->st_mode = i;

	/* We need to do the following for POSIX which requires 'typeflag'
	   be used for setting the S_IFMT part of the mode bits */

	switch(dblock.dbuf.typeflag) {
		case DIRTYPE:
			sp->st_mode = ((sp->st_mode & 07777) | S_IFDIR);
			break;
		case FIFOTYPE:
			sp->st_mode = ((sp->st_mode & 07777) | S_IFIFO);
			break;
		case CHRTYPE:
			sp->st_mode = ((sp->st_mode & 07777) | S_IFCHR);
			break;
		case BLKTYPE:
			sp->st_mode = ((sp->st_mode & 07777) | S_IFBLK);
			break;
		case LNKTYPE:
			sp->st_mode = ((sp->st_mode & 07777) | S_IFREG);
			break;
		case SYMTYPE:
			sp->st_mode = ((sp->st_mode & 07777) | S_IFLNK);
			break;
		case REGTYPE:
			sp->st_mode = ((sp->st_mode & 07777) | S_IFREG);
			break;
		default:
			break;
	}

				/*  check for posix header  */
	bzero(full_name_buf, sizeof(full_name_buf));
	full_name = full_name_buf;
	if ( ! strncmp ( dblock.dbuf.magic , TMAGIC , (size_t)TMAGLEN )) {
		struct passwd	*pw_entry ;
		struct group	*gw_entry ;

		if (dblock.dbuf.prefix[0]) {	/*  use prefix + '/' + name  */
			strncpy(full_name, dblock.dbuf.prefix,
				(size_t)PREFIX_SIZE);
			i = strlen(full_name);
			full_name[i++] = '/';
		} else
			i = 0;
		strncpy(&full_name[i], dblock.dbuf.name, (size_t)NAME_SIZE);
		if (dblock.dbuf.typeflag == DIRTYPE) {
			i = strlen(full_name);
			if (full_name[i-1] != '/') {
				full_name[i++] = '/';
				full_name[i] = '\0';
			}
		}

			/*  set uid and gid based on uname and gname */
		if (pw_entry = getpwnam ( dblock.dbuf.uname )) {
			sp->st_uid = pw_entry->pw_uid ;
		}
		else {
			sscanf(dblock.dbuf.uid, "%0o", &i);
			sp->st_uid = i;
		}
		if (gw_entry = getgrnam ( dblock.dbuf.gname )) {
			sp->st_gid = gw_entry->gr_gid ;
		}
		else {
			sscanf(dblock.dbuf.gid, "%0o", &i);
			sp->st_gid = i;
		}
		sscanf(dblock.dbuf.mtime, "%0lo", &sp->st_mtime);
	}
	else {
				/*  older archive file  */
		full_name = dblock.dbuf.name;	/* already null-terminated */
		sscanf(dblock.dbuf.uid, "%0o", &i);
		sp->st_uid = i;
		sscanf(dblock.dbuf.gid, "%0o", &i);
		sp->st_gid = i;

				/*  if special file then decode major
				    and minor number in mtime  */
		sscanf(dblock.dbuf.mtime, "%0lo", &sp->st_mtime);
		ftype = sp->st_mode & S_IFMT ;
		switch ( ftype ) {
			case S_IFIFO :
			case S_IFBLK :
			case S_IFCHR : {
				int	val ;
					/*  place major and minor num
					 *  in dblock header
					 */
				val = ((sp->st_mtime >> 8) & 0xff00) |
					(sp->st_mtime & 0xff);
				sprintf(dblock.dbuf.devminor, "%06o " , val);
				val = (sp->st_mtime >> 8) & 0xff;
				sprintf(dblock.dbuf.devmajor, "%06o " , val);
				break ;
				}

			default :
				/* Required for POSIX */
				sprintf(dblock.dbuf.devminor, "%06o " , 0);
				sprintf(dblock.dbuf.devmajor, "%06o " , 0);
				break;
		}

	}

	sscanf(dblock.dbuf.size, "%0lo", &sp->st_size);

	if (Pflag) {
		full_name = stripprefix(full_name);
		if (!*full_name)
			goto top;
	}
	/* strip off leading "/" if present */
	if (sflag && full_name[0] == '/') {
		while (*full_name && *full_name == '/')
			++full_name;
		if (!*full_name)
			goto top;
	}

	if (tfile != NULL)
		fprintf(tfile, "%s %s\n", full_name, dblock.dbuf.mtime);

	return (TRUE) ;
}

passtape()
{
	long blocks;
	char *bufp;

	switch (dblock.dbuf.typeflag) {
		case LNKTYPE:
		case SYMTYPE:
		case DIRTYPE:
		case CHRTYPE:
		case BLKTYPE:
		case FIFOTYPE:
			/* These types have no data blocks on archive */
			return;
	}
	blocks = stbuf.st_size;
	blocks += TBLOCK-1;
	blocks /= TBLOCK;

	while (blocks-- > 0)
		(void) readtbuf(&bufp, TBLOCK);
}

putfile(longname, shortname, parent)
	char *longname;
	char *shortname;
	char *parent;
{
	int infile = 0;
	long blocks;
	char buf[TBLOCK];
	char *bigbuf;
	register char *cp;
	struct dirent *dp;
	DIR *dirp;
	register int i;
	long l;
	char newparent[PATH_MAX];
	extern int errno;
	int	maxread;
	int	hint;		/* amount to write to get "in sync" */
	int	update = 1;
	struct list list;
	char *elem;

	if (excepted(longname)) {
		if (vflag)
			fprintf(vfile, "except %s\n", longname);
		return;
	}
	if (Pflag && !writingdir)
		longname = stripprefix(longname);

	if (!hflag)
		i = lstat(shortname, &stbuf);
	else
		i = stat(shortname, &stbuf);
	if (i < 0) {
		fprintf(stderr, "tar: ");
		perror(longname);
		return;
	}
	if (tfile != NULL && checkupdate(longname) == 0)
		/* Even if longname is up-to-date, need to recurse if dir */
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
			update = 0;
		else
			return;
	if (update && checkw('r', longname) == 0)
		return;
	if (Fflag && checkf(shortname, stbuf.st_mode) == 0)
		return;
#if SEC_ARCH
	if (!ie_sl_export(longname, (stbuf.st_mode & S_IFMT) != S_IFLNK))
		return;
#endif

	switch (stbuf.st_mode & S_IFMT) {
	case S_IFDIR:
		for (i = 0, cp = buf; *cp++ = longname[i++];)
			;
		*--cp = '/';
		*++cp = '\0';
		if (!oflag && update) {
			stbuf.st_size = 0;
			if (!tomodes(&stbuf, buf, shortname))
				return;
			sprintf(dblock.dbuf.chksum, "%06o", checksum());
			(void) writetape((char *)&dblock);
			if ( vflag )
				fprintf(vfile, "a %s\n", longname);
		}

		sprintf(newparent, "%s/%s", parent, shortname);
		if (chdir(shortname) < 0) {
			perror(shortname);
			return;
		}
		if ((dirp = opendir(".")) == NULL) {
			fprintf(stderr, MSGSTR( EDIRREAD,
				"tar: %s: directory read error\n" ),
				longname);
			if (chdir(parent) < 0) {
				fprintf(stderr, MSGSTR( ECHBACK,
					"tar: cannot change back?: "));
				perror(parent);
			}
			return;
		}

		list_empty(&list);
		while ((dp = readdir(dirp)) != NULL && !term) {
			if (!strcmp(".", dp->d_name) ||
			    !strcmp("..", dp->d_name))
				continue;
			list_append(&list, dp->d_name);
		}
		closedir(dirp);

		while ((elem = list_first(&list)) != NULL && !term) {
			strcpy(cp, elem);
			free(elem);
			writingdir++;
			putfile(buf, cp, newparent);
		}

		if (chdir(parent) < 0) {
			fprintf(stderr, MSGSTR( ECHBACK,
				"tar: cannot change back?: "));
			perror(parent);
		}
		writingdir = 0;
		break;

	case S_IFLNK:
		if ( ! tomodes ( &stbuf , longname , shortname ))
			return;
		if (stbuf.st_size > name_size) {
			fprintf(stderr, MSGSTR( ELONGSL,
				"tar: %s: symbolic link too long\n" ),
				longname);
			if (stbuf.st_size == NAME_SIZE)
				fprintf(stderr, MSGSTR(ELONGFN2,
					"    (use 'n' option)\n"));
			return;
		}
		i = readlink(shortname, dblock.dbuf.linkname, name_size);
		if (i < 0) {
			fprintf(stderr, MSGSTR( EREADSL,
				"tar: can't read symbolic link "));
			perror(longname);
			return;
		}
		if (i < NAME_SIZE)
			dblock.dbuf.linkname[i] = '\0';
		if (vflag)
			fprintf(vfile, MSGSTR(SLINKTO,
				"a %s symbolic link to %.*s\n" ),
				longname, name_size, dblock.dbuf.linkname);
		sprintf(dblock.dbuf.size, "%011lo", 0L);
		sprintf(dblock.dbuf.chksum, "%06o", checksum());
		(void) writetape((char *)&dblock);
		break;

	case S_IFREG:
		if ( ! tomodes ( &stbuf , longname , shortname ))
			return;
		if ((infile = open(shortname, O_RDONLY)) < 0) {
			fprintf(stderr, "tar: ");
			perror(longname);
			return;
		}
		if ((i = link_file(longname)) < 0)
			return;
		else if (i) {
			sprintf(dblock.dbuf.chksum, "%06o", checksum());
			(void) writetape( (char *) &dblock);
			close(infile);
			return;
		}
		blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
		if (vflag)
			fprintf(vfile, MSGSTR( BLKS,
				"a %s %ld blocks\n" ), longname, blocks);
		sprintf(dblock.dbuf.chksum, "%06o", checksum());
		hint = writetape( (char *) &dblock);
		maxread = nblock * TBLOCK;
		if ((bigbuf = (char *) malloc((size_t)maxread)) == 0) {
			maxread = TBLOCK;
			if ((bigbuf = (char *) malloc((size_t)TBLOCK)) == 0) {
				fprintf(stderr, MSGSTR( ENOMEMORY, "tar: No memory.\n"));
				exit(1);
			}
		}

		while ((i = read(infile, bigbuf, (unsigned)(min((hint*TBLOCK), maxread)))) > 0
		  && blocks > 0) {
		  	register int nblks;

			nblks = ((i-1)/TBLOCK)+1;
		  	if (nblks > blocks)
		  		nblks = blocks;
			hint = writetbuf(bigbuf, nblks);
			blocks -= nblks;
		}
		close(infile);
		free((void *)bigbuf);
		if (i < 0) {
			fprintf(stderr, MSGSTR( EREAD, "tar: Read error on "));
			perror(longname);
		} else if (blocks != 0 || i != 0)
			fprintf(stderr, MSGSTR(EFSIZE,
				"tar: %s: file changed size\n"), longname);
		while (--blocks >=  0)
			putempty();
		break;

	case S_IFBLK:
	case S_IFCHR:
	case S_IFIFO:
				/*  special files - write header  */
		if (!oflag) {
			if (! tomodes(&stbuf, longname, shortname))
				return;
			if ((i = link_file(longname)) < 0)
				return;
			if (vflag && !i)
				if (dblock.dbuf.typeflag == FIFOTYPE)
					fprintf(vfile,"a %s fifo\n", longname);
				else {
					(void) decode();
					fprintf(vfile,"a %s %s device %d, %d\n",
						longname,
						dblock.dbuf.typeflag == BLKTYPE?
							"block" : "character",
						dev_maj, dev_min);
				}
			sprintf(dblock.dbuf.chksum, "%06o", checksum());
			(void) writetape( (char *) &dblock);
			break;
		}

	default:
		fprintf(stderr, MSGSTR( ENOARCH,
			"tar: %s could not be archived\n" ), longname);
		break;
	}
}


link_file (longname)
	char *longname;
{
	struct linkbuf *lp;
	int len, found = 0;

	if (stbuf.st_nlink > 1) {

		for (lp = ihead; lp != NULL; lp = lp->nextp)
			if (lp->inum == stbuf.st_ino &&
			    lp->devnum == stbuf.st_dev) {
				found++;
				break;
			}
		if (found) {
			if (lp->pathname[0] == '\0') {	/* link > NAME_SIZE */
				fprintf(stderr, MSGSTR(ELONGLN,
					"tar: %s skipped: linked to a pathname that is too long\n"),
					longname);
				if (lp->pathname[1] == 'n')
					fprintf(stderr, MSGSTR(ELONGFN2,
						"    (use 'n' option)\n"));
				return(-1);
			}
			strncpy(dblock.dbuf.linkname, lp->pathname, NAME_SIZE);
			dblock.dbuf.typeflag = LNKTYPE;
			sprintf(dblock.dbuf.size, "%011lo", 0L);
			if (vflag)
				fprintf(vfile, MSGSTR( LINKTO,
					"a %s linked to %s\n" ),
					longname, lp->pathname);
			lp->count--;
		}
		else {
			lp = (struct linkbuf *) getmem(sizeof(*lp));
			if (lp != NULL) {
				lp->nextp = ihead;
				ihead = lp;
				lp->inum = stbuf.st_ino;
				lp->devnum = stbuf.st_dev;
				lp->count = stbuf.st_nlink - 1;
				if ((len = strlen(longname)) <= name_size) 
					strcpy(lp->pathname, longname);
				else {	/* Can't link if name > NAME_SIZE */
					lp->pathname[0] = '\0';
					lp->pathname[1] = 
						len == NAME_SIZE ? 'n' : 'o';
				}
			}
		}
	}

	return(found);

}	/*  end link_file  */


/***
 ***  Searches linked list pointed to by "head" for "name."  If element 
 ***  already exists, its mode and mtime are updated.  Otherwise, new entry 
 ***  is added to linked list, maintaining postorder. 
 ***/
int
insert_postorder (head, name, mode, mtime)
	struct dir_list **head;
	char	*name;
	mode_t	mode;
	time_t  mtime;
{
	struct dir_list *current, *previous, *new_cell;
	int	prefix_res;

	current = *head;
	previous = *head;

	while (current) {
		prefix_res = gen_prefix(current->name, name);
		if (prefix_res == 2) {		/* entry already exists */
			current->mode = mode;
			current->mtime = mtime;
			return (0);
		}
		else if (prefix_res == 1)       /* entry does not exist */
			break;
		else {
			previous = current;
			current = current->next;
		}
	}

	new_cell = (struct dir_list *) malloc (sizeof (struct dir_list));
	if (new_cell == NULL)
		return (-1);
	bcopy (name, new_cell->name,strlen(name)+1);
	new_cell->mode = mode;
	new_cell->mtime = mtime;
	new_cell->next = current;
	if (previous && previous != current)
		previous->next = new_cell;
	else
		*head = new_cell;
	return (0);
}

doxtract(argv)
	char *argv[];
{
	long blocks, bytes;
	int  ofile, i, newdir, no_mem, wrt_bytes;
	char ftype;
	struct dir_list *head, *current;

	no_mem = 0;
	head = NULL;
	for (;;) {
		if ((i = wantit(argv)) == 0)
			continue;
		if (i == -1)
			break;		/* end of tape */
		if (checkw('x', full_name) == 0) {
			passtape();
			continue;
		}
		if (Fflag) {
			char *s;

			if ((s = rindex(full_name, '/')) == 0)
				s = full_name;
			else
				s++;
			if (checkf(s, stbuf.st_mode) == 0) {
				passtape();
				continue;
			}
		}

		if (checkdir(&newdir)) {	/* have a directory */
			if (vflag && newdir)
				fprintf(vfile, "x %s\n", full_name);
			if ((pflag || (mflag == 0)) && !no_mem)
				if (insert_postorder(&head, full_name, 
				    stbuf.st_mode, stbuf.st_mtime) == -1) {
					if (pflag)
						fprintf(stderr, MSGSTR( 
						   ENOMEMPFLAG,
						   "tar: no memory - not restoring modes of directories.\n" ));
					if (mflag == 0)
						fprintf(stderr, MSGSTR( 
						   ENOMEMTIME,
						   "tar: no memory - not restoring modification times of directories.\n" ));
					no_mem = 1;
				}
			continue;
		}

		if (dblock.dbuf.typeflag == SYMTYPE) {	/* symlink */
			char link_name[NAME_SIZE+1];
			/*
			 * only unlink non directories or empty
			 * directories
			 */
			if (rmdir(full_name) < 0) {
				if (errno == ENOTDIR)
					unlink(full_name);
			}
			strncpy(link_name, dblock.dbuf.linkname, NAME_SIZE);
			link_name[NAME_SIZE] = '\0';
			if (symlink(link_name, full_name)<0) {
				fprintf(stderr, MSGSTR( ESYMFAIL,
					"tar: %s: symbolic link failed: " ),
					full_name);
				perror("");
				continue;
			}
			if (vflag)
				fprintf(vfile, MSGSTR( XSYMLINK,
					"x %s symbolic link to %s\n" ),
					full_name, link_name);
#if SEC_ARCH
			ie_sl_set_attributes(full_name);
#endif
			/* ignore alien orders */
			/* Commented out (as is BSD 4.3) since modifications
			   are to linked to file, not link itself. */
			/*
			if (!oflag && !geteuid())
				chown(full_name, stbuf.st_uid, stbuf.st_gid);
			if (mflag == 0)
				setimes(full_name, stbuf.st_mtime);
			if (pflag)
				chmod(full_name, stbuf.st_mode & 07777);
			*/
			continue;
		}

		if (dblock.dbuf.typeflag == LNKTYPE) {	/* regular link */
			char	link_type;
			char	link_buf[NAME_SIZE+1];
			char	*link_name;
			/*
			 * only unlink non directories or empty
			 * directories
			 */
			if (rmdir(full_name) < 0) {
				if (errno == ENOTDIR)
					unlink(full_name);
			}
			strncpy(link_buf, dblock.dbuf.linkname, NAME_SIZE);
			link_buf[NAME_SIZE] = '\0';
			link_name = Pflag ? stripprefix(link_buf) : link_buf;
			if (sflag && link_name[0] == '/') {
				while (*link_name && *link_name == '/')
					++link_name;
			}
			if (!*link_name) {
				fprintf(stderr, MSGSTR(EFLINK,
					"tar: can't link %s to %s: "),
					full_name, link_buf);
				fprintf(stderr, MSGSTR(ELPREFIX,
			"link no longer exists since prefix was stripped.\n"));
				continue;
			}
			if (link(link_name, full_name) < 0) {
				if ( !Lflag ) {
					fprintf(stderr, MSGSTR( EFLINK,
						"tar: can't link %s to %s: " ),
						full_name, link_name);
					perror("");
					continue;
				}
				else if (symlink(link_name, full_name) < 0) {
					fprintf(stderr, MSGSTR(EFLINK,
						"tar: can't link %s to %s: "),
				    		full_name, link_name);
					perror("");
					continue;
				}
				else
					link_type = SYMTYPE;
			}
			else
				link_type = LNKTYPE ;

			if (vflag)
				if ( link_type == LNKTYPE )
					fprintf(vfile, MSGSTR( LINKEDTO,
					"x %s linked to %s\n" ),
					full_name, link_name);
				else
					fprintf(vfile, MSGSTR( XSYMLINK,
					"x %s symbolic link to %s\n" ),
					full_name, link_name);
			continue;
		}

				/*  check for special files  */
		if ((ftype = dblock.dbuf.typeflag) == FIFOTYPE ||
			ftype == BLKTYPE || ftype == CHRTYPE) {
#if SEC_BASE
			if (ftype != FIFOTYPE && !hassysauth(SEC_MKNOD)) {
			    fprintf(stderr, MSGSTR(ESECMKNODAUTH,
				"tar: %s: need mknod kernel authorization\n"),
				full_name);
			    continue;
			}
#endif
			if (rmdir(full_name) < 0 && errno == ENOTDIR)
				unlink(full_name);
			if (mknod(full_name, stbuf.st_mode, decode()) < 0) {
				fprintf(stderr, MSGSTR(EMKNOD,
					"tar: %s: mknod failed: "), full_name);
				perror("");
				continue;
			}
#if SEC_ARCH
			ie_sl_set_attributes(full_name);
#endif
#if SEC_BASE
			if (getluid() == stbuf.st_uid || hassysauth(SEC_OWNER))
#endif
			if (!oflag && !geteuid())
				chown(full_name, stbuf.st_uid, stbuf.st_gid);
			if (mflag == 0)
				setimes(full_name, stbuf.st_mtime);
			if (pflag)
#if SEC_BASE
				ie_chmod(full_name, stbuf.st_mode & 07777, 0);
#else
				chmod(full_name, stbuf.st_mode & 07777);
#endif
			if (vflag)
				if (ftype == FIFOTYPE)
					fprintf(vfile,"x %s fifo\n", full_name);
				else
					fprintf(vfile,"x %s %s device %d, %d\n",
						full_name,
						ftype == BLKTYPE ?
							"block" : "character",
						dev_maj, dev_min);
			continue;
		}

		if ((ofile = creat(full_name,stbuf.st_mode&0xfff)) < 0) {
			fprintf(stderr, MSGSTR( ETCREATE,
				"tar: can't create %s: " ),
				full_name);
			perror("");
			passtape();
			continue;
		}
#if SEC_ARCH
		if (!ie_sl_set_attributes(full_name)) {
			close(ofile);
			unlink(full_name);
			passtape();
			continue;
		}
#endif
#if SEC_BASE
		if (getluid() == stbuf.st_uid || hassysauth(SEC_OWNER))
#endif
		if (!oflag && !geteuid())
			chown(full_name, stbuf.st_uid, stbuf.st_gid);
		blocks = ((bytes = stbuf.st_size) + TBLOCK-1)/TBLOCK;
		if (vflag)
			fprintf(vfile, MSGSTR( XSTAT,
				"x %s, %ld bytes, %ld tape blocks\n" ),
				full_name, bytes, blocks);
		for (; blocks > 0;) {
			register int nread;
			char	*bufp;
			register int nwant;
			
			nwant = NBLOCK*TBLOCK;
			if (nwant > (blocks*TBLOCK))
				nwant = (blocks*TBLOCK);

			nread = readtbuf(&bufp, nwant);
			wrt_bytes = write(ofile, bufp, 
				(unsigned)min(nread, bytes));
			if (wrt_bytes != min(nread, bytes)) {
				if (wrt_bytes == -1) {
					fprintf(stderr, MSGSTR( EXWRITE,
					"tar: %s: extract write error" ),
					full_name);
					perror("");
				}
				else
					fprintf(stderr, MSGSTR( EXWREOF,
					"tar: %s: extract write error: unexpected EOF\n" ),
					full_name);
					
				done(2);
			}
			bytes -= nread;
			blocks -= (((nread-1)/TBLOCK)+1);
		}
		if (close(ofile) == -1) {
			fprintf (stderr, MSGSTR(EXCLOSE, 
				 "tar: %s: extract close error" ), full_name);
			perror("");
			done(2);
		}
		if (mflag == 0)
			setimes(full_name, stbuf.st_mtime);
		if (pflag)
#if SEC_BASE
			ie_chmod(full_name, stbuf.st_mode & 07777, 0);
#else
			chmod(full_name, stbuf.st_mode & 07777);
#endif
	}
	if (!no_mem) {
		current = head;
		while (current) {
			if (pflag)
#if SEC_BASE
				ie_chmod(current->name, current->mode & 07777, 1);
#else
				chmod(current->name, current->mode & 07777);
#endif
			if (mflag == 0)
				setimes(current->name, current->mtime);
			head = current;
			current = current->next;
			free (head);
		}
	}
}


	/*	The function decode will return a value which can be
	 *	used in the function call mknod. The value is extracted
	 *	from the dblock major and minor character strings.
	 */
decode ()
{
	sscanf(dblock.dbuf.devminor, "%0o", &dev_min);
	sscanf(dblock.dbuf.devmajor, "%0o", &dev_maj);
	return ( makedev ( dev_maj , dev_min )) ;
}	/*  end decode  */


dotable(argv)
	char *argv[];
{
	register int i;

	for (;;) {
		if ((i = wantit(argv)) == 0)
			continue;
		if (i == -1)
			break;		/* end of tape */
		if (vflag)
			longt(&stbuf);
		printf("%s ", full_name);
		switch (dblock.dbuf.typeflag) {
			case LNKTYPE:
				printf(MSGSTR(LINKED, "linked to %.*s"),
					NAME_SIZE, dblock.dbuf.linkname);
				break;
			case SYMTYPE:
				printf(MSGSTR(SLINKED, "symbolic link to %.*s"),
					NAME_SIZE, dblock.dbuf.linkname);
				break;
			case FIFOTYPE:
				printf("fifo");
				break;
			case CHRTYPE:
			case BLKTYPE:
				(void) decode();
				printf("%s device %d, %d",
					dblock.dbuf.typeflag == CHRTYPE ?
						"character" : "block",
					dev_maj, dev_min);
				break;
		}
		printf("\n");
		passtape();
	}
}

putempty()
{
	char buf[TBLOCK];

	bzero(buf, sizeof (buf));
	(void) writetape(buf);
}

longt(st)
	register struct stat *st;
{
	register char *cp;
	char *ctime(), timbuf[26];

	pmode(st);
	printf("%6d/%-2d", st->st_uid, st->st_gid);
	printf(" %7ld ", st->st_size);
	strftime(timbuf, (size_t)26,"%sD %T %Y", localtime(&st->st_mtime));
	printf("%s ",timbuf);
}

	/*	These constants are defined in tar.h
	 */
int	m1[] = { 1, TUREAD, 'r', '-' };
int	m2[] = { 1, TUWRITE, 'w', '-' };
int	m3[] = { 2, TSUID, 's', TUEXEC, 'x', '-' };
int	m4[] = { 1, TGREAD, 'r', '-' };
int	m5[] = { 1, TGWRITE, 'w', '-' };
int	m6[] = { 2, TSGID, 's', TGEXEC, 'x', '-' };
int	m7[] = { 1, TOREAD, 'r', '-' };
int	m8[] = { 1, TOWRITE, 'w', '-' };
int	m9[] = { 2, TSVTX, 't', TOEXEC, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

pmode(st)
	register struct stat *st;
{
	char	c ;
	register int **mp;

	switch ( st->st_mode & S_IFMT ) {
		case S_IFREG :
			c = '-' ; break ;
		case S_IFDIR :
			c = 'd' ; break ;
		case S_IFLNK :
			c = 'l' ; break ;
		case S_IFCHR :
			c = 'c' ; break ;
		case S_IFBLK :
			c = 'b' ; break ;
		case S_IFIFO :
			c = 'p' ; break ;
		default :
			c = '?' ; break ;
	}
	putchar (c) ;
	for (mp = &m[0]; mp < &m[9];)
		selectbits(*mp++, st);
}

selectbits(pairp, st)
	int *pairp;
	struct stat *st;
{
	register int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (st->st_mode&*ap++)==0)
		ap++;
	putchar(*ap);
}

/*
 * Make all directories needed by `name'.  If `name' is itself
 * a directory on the tar tape (indicated by a trailing '/'),
 * return 1; else 0.  The parameter allows the status of whether
 * the directory already existed to be returned.
 * The path name is created by concatenating dblock.dbuf.prefix
 * and dblock.dbuf.name together in the function getdir.
 */
checkdir(newdir)
int *newdir;
{
	register char *cp;

	*newdir = 0;
	/*
	 * Quick check for existence of directory.
	 */
	if ((cp = rindex(full_name, '/')) == 0)
		return (0);
	*cp = '\0';
	if (access(full_name, 0) == 0) {	/* already exists */
		*cp = '/';
		return (cp[1] == '\0');	/* return (lastchar == '/') */
	}
	*cp = '/';

	/*
	 * No luck, try to make all directories in path.
	 */
	cp = full_name;
	while (*cp == '/')
		cp++;
	for (; *cp; cp++) {
		if (*cp != '/')
			continue;
		*cp = '\0';
		if (access(full_name, 0) < 0) {
			if (mkdir(full_name, (mode_t)0777) < 0) {
				perror(full_name);
				*cp = '/';
				return (0);
			}
			*newdir = 1;
#if SEC_ARCH
			ie_sl_set_attributes(full_name);
#endif
#if SEC_BASE
			if (getluid() == stbuf.st_uid || hassysauth(SEC_OWNER))
#endif
			if (!oflag && !geteuid()) 
				chown(full_name, stbuf.st_uid, stbuf.st_gid);
		}
		*cp = '/';
	}
	return (cp[-1]=='/');
}

onintr(void)
{
	(void) signal(SIGINT, SIG_IGN);
	term++;
}

onquit(void)
{
	(void) signal(SIGQUIT, SIG_IGN);
	term++;
}

onhup(void)
{
	(void) signal(SIGHUP, SIG_IGN);
	term++;
}

onterm(void)
{
	(void) signal(SIGTERM, SIG_IGN);
	term++;
}


	/*	The function to modes will fill in the header
	 *	information for a file.
	*/
tomodes(sp, longname , shortname )
register struct stat *sp;
	char	*longname ;
	char	*shortname ;
{
	register char	*cp;
	char		*slash ;
	int		ftype ;
	struct passwd	*pw_entry ;
	struct group	*gr_entry ;
	int nameLen;    /* length of longname */
	register char *nameBkpt; /* Name break point */
	
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		*cp = '\0';

	nameLen = strlen(longname);
	if (nameLen <= name_size) {
		strncpy(dblock.dbuf.name, longname, nameLen);
		dblock.dbuf.prefix[0] = '\0';
	} else {
		nameBkpt = longname + nameLen;
		/*
		 * Put as much into name as possible and the rest into prefix,
		 * but only break on a slash.
		 */
		slash = NULL;
		do {
			nameBkpt--;
			if ('/' == *nameBkpt)
				slash = nameBkpt;
		} while (nameLen - (nameBkpt-longname) <= name_size);

		if (slash && ((slash-longname) <= PREFIX_SIZE)) {
			strncpy(dblock.dbuf.name, slash+1,
				(size_t)(nameLen - (slash+1-longname)));
			strncpy(dblock.dbuf.prefix, longname,
				(size_t)(slash-longname));
		} else {
			fprintf(stderr, MSGSTR(ELONGFN,
				"tar: %s: file name too long\n"), longname);
			if (nameLen == NAME_SIZE ||
			    *--nameBkpt == '/'
			    && nameLen - (nameBkpt+1-longname) <= NAME_SIZE
			    && nameBkpt-longname <= PREFIX_SIZE)
				fprintf(stderr, MSGSTR(ELONGFN2,
					"    (use 'n' option)\n"));
			return (FALSE) ;
		}
	}

	sprintf(dblock.dbuf.mode, "%06o ", sp->st_mode & 07777);
	sprintf(dblock.dbuf.uid, "%06o ", sp->st_uid);
	sprintf(dblock.dbuf.gid, "%06o ", sp->st_gid);
	sprintf(dblock.dbuf.size, "%011lo ", sp->st_size);
	sprintf(dblock.dbuf.mtime, "%011lo ", sp->st_mtime);

	strncpy (dblock.dbuf.magic, TMAGIC , (size_t)TMAGLEN ) ;
	strncpy (dblock.dbuf.version, TVERSION , (size_t)TVERSLEN ) ;

	ftype = sp -> st_mode & S_IFMT ;

				/*  set file type  */
	switch ( ftype ) {
		case S_IFIFO :
			dblock.dbuf.typeflag = FIFOTYPE ;
			break ;

		case S_IFCHR :
			dblock.dbuf.typeflag = CHRTYPE ;
			break ;

		case S_IFBLK :
			dblock.dbuf.typeflag = BLKTYPE ;
			break ;

		case S_IFDIR :
			dblock.dbuf.typeflag = DIRTYPE ;
			break ;

		case S_IFREG :
			dblock.dbuf.typeflag = REGTYPE ;
			break ;

		case S_IFLNK :
			dblock.dbuf.typeflag = SYMTYPE;
			break;
	}

				/*  set uname and gname (NULL terminated) */
	if (( pw_entry = getpwuid ( sp->st_uid )))
		strncpy ( dblock.dbuf.uname , pw_entry->pw_name, UNAMELEN ) ;
	dblock.dbuf.uname[UNAMELEN-1] = '\0';
	if (( gr_entry = getgrgid ( sp->st_gid )))
		strncpy ( dblock.dbuf.gname , gr_entry->gr_name, GNAMELEN ) ;
	dblock.dbuf.gname[GNAMELEN-1] = '\0';

				/*  set major and minor values  */
	if ( ftype == S_IFIFO || ftype == S_IFBLK || ftype == S_IFCHR ) {
		sprintf(dblock.dbuf.devminor, "%06o " , minor(sp->st_rdev));
		sprintf(dblock.dbuf.devmajor, "%06o " , major(sp->st_rdev));
	}
	else {		/* POSIX requires the 'right stuff' for all types */
		sprintf(dblock.dbuf.devminor, "%06o " , 0);
		sprintf(dblock.dbuf.devmajor, "%06o " , 0);
	}
		

	return (TRUE) ;
}


	/*	The checksum function will create a checksum for
	 *	the header of a file written to the output file.
	*/
checksum()
{
	register i;
	register char *cp;

	for (cp = dblock.dbuf.chksum;
	     cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
		*cp = ' ';
	i = 0;
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		i += *cp;
	return (i);
}

checkw(c, name)
	char *name;
{
	int	yesno;
	char	ans[2];
	if (!wflag)
		return (1);
	printf("%c ", c);
	if (vflag)
		longt(&stbuf);
	printf("%s: ", name);
	ans[0] = (char)response();
	ans[1] = '\0';
	if ((yesno = NLyesno(ans)) == -1)
		yesno = (*ans == 'y' || *ans == 'Y');
	return (yesno);
}

response()
{
	int c;

	c = getwchar();
	if (c != '\n')
		while (getwchar() != '\n')
			;
	return (c);
}

checkf(name, mode)
	char *name;
	int mode;
{
	int l;

	if ((mode & S_IFMT) == S_IFDIR){
		if ((strcmp(name, "SCCS")==0) || (strcmp(name, "RCS")==0)) 
			return(0); 
		return(1);
	}
	if ((l = strlen(name)) < 3)
		return (1);
	if (name[l-2] == '.' && name[l-1] == 'o')
		return (0);
	if (strcmp(name, "core") == 0 ||
	    strcmp(name, "errs") == 0 ||
	    (strcmp(name, "a.out") == 0))
		return (0);
	/* SHOULD CHECK IF IT IS EXECUTABLE */
	return (1);
}

/* Is the current file a new file, or the newest one of the same name? */
checkupdate(arg)
	char *arg;
{
	char name[TAR_PATH_LEN];
	long mtime;
	daddr_t seekp;
	daddr_t	lookup();

	rewind(tfile);
	for (;;) {
		if ((seekp = lookup(arg)) < 0)
			return (1);
		fseek(tfile, seekp, 0);
		fscanf(tfile, "%s %lo", name, &mtime);
		return (stbuf.st_mtime > mtime);
	}
}

done(n)
{
	unlink(tname);
	exit(n);
}

/* 
 * Do we want the next entry on the tape, i.e. is it selected?  If
 * not, skip over the entire entry.  Return -1 if reached end of tape.
 */
wantit(argv)
	char *argv[];
{
	register char **cp;

	getdir() ;
	if (endtape())
		return (-1);
	if (*argv == NULL)
		return (1);
	for (cp = argv; *cp; cp++)
		if (prefix(*cp, full_name))
			return (1);
	passtape();
	return (0);
}


/*
 * Does s2 begin with the string s1? Return 2 if s1 equal to s2, 1 if prefix
 * otherwise.
 */
int
gen_prefix(s1, s2)
	register char *s1, *s2;
{
	while (*s1)
		if (*s1++ != *s2++)
			return (0);
	if (*s2)
		return (1);
	return (2);
}

/*
 * Does s2 begin with the string s1, on a directory boundary?
 */
prefix(s1, s2)
	register char *s1, *s2;
{
	while (*s1)
		if (*s1++ != *s2++)
			return (0);
	if (*s2)
		return (*s2 == '/');
	return (1);
}

/*
 * The size of a buffer that when read from a random place in tfile is (mostly)
 * guaranteed to contain a complete record (tarpathlen + mtime size).
 */
#define	N	512

daddr_t
lookup(s)
	char *s;
{
	register i;
	daddr_t a;

	for(i=0; s[i]; i++)
		if (s[i] == ' ')
			break;
	a = bsrch(s, i, low, high);
	return (a);
}

daddr_t
bsrch(s, n, l, h)
	daddr_t l, h;
	char *s;
{
	register i, j;
	char b[N];
	daddr_t m, m1;

loop:
	if (l >= h)
		return ((daddr_t) -1);
	m = l + (h-l)/2 - N/2;
	if (m < l)
		m = l;
	fseek(tfile, m, 0);
	fread((void *)b, (size_t)1, (size_t)N, tfile);
	for(i=0; i<N; i++) {
		if (b[i] == '\n')
			break;
		m++;
	}
	if (m >= h)
		return ((daddr_t) -1);
	m1 = m;
	j = i;
	for(i++; i<N; i++) {
		m1++;
		if (b[i] == '\n')
			break;
	}
	i = cmp(b+j, s, n);
	if (i < 0) {
		h = m;
		goto loop;
	}
	if (i > 0) {
		l = m1;
		goto loop;
	}
	return (m+1);
}

cmp(b, s, n)
	char *b, *s;
{
	register i;

	if (b[0] != '\n')
		exit(2);
	for(i=0; i<n; i++) {
		if (b[i+1] > s[i])
			return (-1);
		if (b[i+1] < s[i])
			return (1);
	}
	return ((b[i+1] == ' ' || b[i+1] == '/' && b[i+2] == ' ') ? 0 : -1);
}

backtape()
{
	static int mtdev = 1;
	static struct mtop mtop = {MTBSR, 1};
	struct mtget mtget;
	
	if (mtdev == 1)
		mtdev = ioctl(mt, MTIOCGET, (char *)&mtget);
	if (mtdev == 0) {
		if (ioctl(mt, MTIOCTOP, (char *)&mtop) < 0) {
			fprintf(stderr, MSGSTR(EBACKSP,
				"tar: tape backspace error: "));
			perror("");
			done(4);
		}
	} else
		lseek(mt, (daddr_t) -lastread, 1);
	trecs--;
	recno--;
}


readtape(buffer)
	char *buffer;
{
	char *bufp;

	if (first == 0)
		getbuf();
	(void) readtbuf(&bufp, TBLOCK);

	/*
	 * If Uflag was specified and the tape was just changed then 
	 * skip over the first block on this tape, which contains the 
	 * old ULTRIX header.
	 */
	if (Uflag && tape_changed) {
		bufp += TBLOCK;
		tape_changed = 0;
	}
	bcopy(bufp, buffer, TBLOCK);
	return(TBLOCK);
}

readtbuf(bufpp, size)
	char **bufpp;
	int size;
{
	register int i;

	if (recno >= nblock || first == 0) {
		i = bread(mt, (char *)tbuf, TBLOCK*nblock);
		if (first == 0) {
			i /= TBLOCK;

			if (vflag)
				fprintf(stderr, MSGSTR( BLKSIZE,
				"tar: blocksize = %d\n" ), i);
			if (nblock != i)
				nblock = i;
			first = 1;
		}
		recno = 0;
	}
	if (size > ((nblock-recno)*TBLOCK))
		size = (nblock-recno)*TBLOCK;
	*bufpp = (char *)&tbuf[recno];
	recno += (size/TBLOCK);
	return (size);
}

writetbuf(buffer, n)
	register char *buffer;
	register int n;
{
	int i;

	if (first == 0) {
		getbuf();
		first = 1;
	}
	if (recno >= nblock) {
		bwrite ((char *) tbuf);
		recno = 0;
	}

	/*
	 *  Special case:  We have an empty tape buffer, and the
	 *  users data size is >= the tape block size:  Avoid
	 *  the bcopy and dma direct to tape.  BIG WIN.  Add the
	 *  residual to the tape buffer.
	 */
	while (recno == 0 && n >= nblock) {
		bwrite (buffer) ;
		n -= nblock;
		buffer += (nblock * TBLOCK);
	}
		
	while (n-- > 0) {
		bcopy(buffer, (char *)&tbuf[recno++], TBLOCK);
		buffer += TBLOCK;
		if (recno >= nblock) {
			bwrite ((char *) tbuf) ;
			recno = 0;
		}
	}

	/* Tell the user how much to write to get in sync */
	return (nblock - recno);
}


bwrite (buffer)
	char	*buffer ;
{
	int	wr ;

	if ( rptape && trecs >= rptape )
		nexttape (1) ;
tryagain:
	wr = write(mt, buffer, (unsigned)(TBLOCK*nblock)) ;
	if ( wr == -1 )
		if (multivol && (errno == ENXIO || errno == ENOSPC)) {
			nexttape (1) ;
			goto tryagain ;
		}
		else {
			perror ( MSGSTR( ETWRITE, "tar: tape write error" )) ;
			done(2);
		}
	else
		if (wr != TBLOCK*nblock) {
			fprintf(stderr, MSGSTR( ETWREOF,
				"tar: tape write error: unexpected EOF\n" ));
			done(2);
		}
	++trecs;
	return;
}


bread(fd, buf, size)
	int fd;
	char *buf;
	int size;
{
	int count;

	if (rptape && trecs >= rptape)
		nexttape(0);
	trecs++;
				/***	the for loop is for a communications
					link by specifing the 'B' option,
					all other will exit the for loop
					after the first read unless prompted
					to enter a new device	***/
	for (count = 0; count < size; count += lastread) {
		lastread = read(fd, buf, (unsigned)(size - count));
		switch ( lastread )
		{
			case -1 :
				perror(MSGSTR(ETREAD, "tar: tape read error"));
				done(3);

			case 0 :
				if (!multivol) {
					if (count)
						return(count);
					fprintf(stderr, MSGSTR(ETRDEOF,
						"tar: tape read error: unexpected EOF\n"));
					done(2);
				}
				nexttape (0) ;	/*  prompt for new device  */
				break ;

			default :
				if ( !Bflag )
					return (lastread);
				break ;
		}
		buf += lastread;
	}
	return (count);
}


char *
getpwd(buf)
	char *buf;
{
	if (getwd(buf) == NULL) {
		fprintf(stderr, "tar: %s\n", buf);
		exit(1);
	}
	return (buf);
}

getbuf()
{

	if (nblock == 0) {
#ifdef	OSF_REFERENCE_SOURCE
		/* This is the way OSF/1 worked, but it adds 
		 * unnecessary complication.
		 */
		fstat(mt, &stbuf);
		if ((stbuf.st_mode & S_IFMT) == S_IFCHR)
			nblock = DBLOCK;
		else {
			nblock = stbuf.st_blksize / TBLOCK;
			if (nblock == 0)
				nblock = DBLOCK;
		}

#else	/* ! OSF_REFERENCE_SOURCE */

		/* Default to DBLOCK blocks for creation, otherwise
		 * try NBLOCK so autosizing works up to maximum blocksize.
		 */
		if (cflag) {
			nblock = DBLOCK;
		}
		else {
			nblock = NBLOCK;
		}

#endif	/* OSF_REFERENCE_SOURCE */
	}

	if (Sflag && multivol)		/*  set number of records per tape  */
		if ( bptape )
			rptape = bptape / nblock ;
		else {
   			unsigned long	len1rec; /* rcd len (100s of inches) */
			len1rec = ((unsigned long) nblock * (TBLOCK * 10000L))
				/ density + GAP;
			rptape = (tapelen * TAPE_FOOT) / len1rec;
		}

	/* Allocate space for # of blocks requested (nblock) */
	tbuf = (union hblock *)malloc((size_t)nblock*TBLOCK);
	if (tbuf == NULL) {
		fprintf(stderr, MSGSTR(EBLKBIG,
			"tar: blocksize %d too big, can't get memory\n"),
			nblock);
		done(1);
	}
}

/*
 * Save this directory and its mtime on the stack, popping and setting
 * the mtimes of any stacked dirs which aren't parents of this one.
 * A null directory causes the entire stack to be unwound and set.
 *
 * Since all the elements of the directory "stack" share a common
 * prefix, we can make do with one string.  We keep only the current
 * directory path, with an associated array of mtime's, one for each
 * '/' in the path.  A negative mtime means no mtime.  The mtime's are
 * offset by one (first index 1, not 0) because calling this with a null
 * directory causes mtime[0] to be set.
 * 
 * This stack algorithm is not guaranteed to work for tapes created
 * with the 'r' option, but the vast majority of tapes with
 * directories are not.  This avoids saving every directory record on
 * the tape and setting all the times at the end.
 */
char dirstack[NAME_SIZE];
#define NTIM (NAME_SIZE/2+1)		/* a/b/c/d/... */
time_t mtime[NTIM];

dodirtimes()
{
	register char *p = dirstack;
	register char *q = full_name;
	register int ndir = 0;
	char *savp;
	int savndir;

	/* Find common prefix */
	while (*p == *q) {
		if (*p++ == '/')
			++ndir;
		q++;
	}

	savp = p;
	savndir = ndir;
	while (*p) {
		/*
		 * Not a child: unwind the stack, setting the times.
		 * The order we do this doesn't matter, so we go "forward."
		 */
		if (*p++ == '/')
			if (mtime[++ndir] >= 0) {
				*--p = '\0';	/* zap the slash */
				setimes(dirstack, mtime[ndir]);
				*p++ = '/';
			}
	}
	p = savp;
	ndir = savndir;

	/* Push this one on the "stack" */
	while (*p = *q++)	/* append the rest of the new dir */
		if (*p++ == '/')
			mtime[++ndir] = -1;
	mtime[ndir] = stbuf.st_mtime;	/* overwrite the last one */
}

setimes(path, mt)
	char *path;
	time_t mt;
{
	struct timeval tv[2];

	tv[0].tv_sec = time((time_t *) 0);
	tv[1].tv_sec = mt;
	tv[0].tv_usec = tv[1].tv_usec = 0;
	if (utimes(path, tv) < 0) {
		fprintf(stderr, MSGSTR( ESETTIME,
			"tar: can't set time on %s: " ), path);
		perror("");
	}
}

char *
getmem(size)
{
	char *p = (char *) malloc((size_t) size);

	if (p == NULL && freemem) {
		fprintf(stderr, MSGSTR(EMEM,
		    "tar: out of memory, link and directory modtime info lost\n"));
		freemem = 0;
	}
	return (p);
}

list_empty(listp)
struct list *listp;
{
	listp->first = listp->last = NULL;
}

list_append(listp, data)
struct list *listp;
char *data;
{
	struct elem *ep;
	int len;

	ep = (struct elem *) calloc(1, sizeof(struct elem));
	if (ep == NULL) {
		fprintf(stderr, MSGSTR(EMEM, "tar: out of memory\n"));
		done(5);
	}
	ep->data = malloc(len = (strlen(data) + 1));
	if (ep->data == NULL) {
		fprintf(stderr, MSGSTR(EMEM, "tar: out of memory\n"));
		done(5);
	}
	bcopy(data, ep->data, len);
	if (listp->first == NULL)
		listp->first = ep;
	else
		listp->last->next = ep;
	listp->last = ep;
}

char *
list_first(listp)
struct list *listp;
{
	struct elem *ep;
	char *data;

	if ((ep = listp->first) == NULL)
		return(NULL);
	listp->first = ep->next;
	data = ep->data;
	free((char *)ep);
	return(data);
}

int
excepted(data)
char *data;
{
	struct elem *ep;

	for (ep = except_list.first; ep != NULL; ep = ep->next)
		if (strcmp(ep->data, data) == 0)
			break;
	return(ep != NULL);
}

char *
stripprefix(filename)
register char *filename;
{
	register char *ptr;
	static int len = 0;
	
	if (!len)
		len = strlen(stripstring);
	if (strncmp(filename, stripstring, len))
		return(filename);
	else
		return(filename+len);
}

				/* move on to the next tape */
				/* mode is 0 for read, 1 for write */
nexttape(writing)
	int writing;
{
	int	c;
	char	tempdir[NAME_MAX];
	FILE	*devtty = stdin;
	int	tries = 0;

	if (!multivol) {
		fprintf(stderr, MSGSTR(EBADMVOL,
			"tar: %s is not a device capable of %s multi-volume archives.\n"),
			usefile, writing ? MSGSTR(TARCREATE, "creating")
					 : MSGSTR(TARXTRACT, "extracting"));
		done(4);
	}

	if (close(mt) == -1) {
		perror(MSGSTR (EVCLOSE, "tar: volume close error"));
		done(2);
	}
	fprintf(stderr, MSGSTR(BLKSON, "tar: %ld blocks on %s\n"),
		trecs * nblock, usefile);

	if (!isatty(fileno(devtty)) &&
	    (devtty = fopen("/dev/tty", "r")) == NULL) {
		fprintf(stderr, MSGSTR(EEOT, "tar: ran off end of tape\n"));
		done(4);
	}
loop:
	fprintf(stderr, MSGSTR(EOT,
	   "\007tar: End of device.  Mount next volume on %s and hit return."),
		usefile);
	do {
		if ((c = fgetc(devtty)) == EOF || term)
			done(4);
	} while (c != '\n');

	if (*startcwd && tries == 0) {
		(void) getpwd(tempdir);
		if (chdir(startcwd) < 0) {
			fprintf(stderr, MSGSTR(ECHDIR,
				"tar: can't change directories to "));
			perror(startcwd);
		}
	}
	if ((mt = open(usefile, writing ? O_RDWR : O_RDONLY)) < 0) {
		fprintf(stderr, MSGSTR(ETOPEN, "tar: cannot open "));
		perror(usefile);
		if (++tries >= 3) {
			fprintf(stderr, MSGSTR(EMAXRETRIES,
				"tar: maximum retries limit exceeded.\n"));
			done(3);
		}
		goto loop;
	}
	if (*startcwd && chdir(tempdir) < 0) {
			fprintf(stderr, MSGSTR( ECHBACK,
				"tar: cannot change back?: "));
			perror(tempdir);
			done(3);
	}

	fprintf(stderr, MSGSTR(PROCEED, "proceeding with device %s\n"),usefile);
	trecs = 0;
	tape_changed++;

	return;
}

/*
 * make_argv() - Make Argument List from file list in a file.
 *
 * Description:
 *	This function creates an argument list from files listed
 * in a specified file.  The input file is expected to have each
 * file one a single line, terminated by a newline.
 *
 * Inputs:
 *	char ***argvp = Pointer to destination argv.
 *	int *argcp = Pointer to destination argc.
 *
 * Outputs:
 *	char ***argvp = Contains new argv array.
 *	int *argcp = Contains new argc value.
 *
 * Return Value:
 *	Returns 0 / -1 = SUCCESS / FAILURE.
 */

extern char **_exec_argv(int *argvct);	/* Standard libc.a function.	*/

int
make_argv (char ***argvp,		/* Pointer to destination argv.	*/
	   int *argcp)			/* Pointer to destination argc.	*/
{
	int argc;			/* Arguments processed so far.	*/
	int alloced;			/* Allocated pointers so far.	*/
	char **argp;			/* Used to 'walk thru' *argvp.	*/
	char **oargp;			/* Old argument array pointer.	*/
	char fname[PATH_MAX + 1];	/* Buffer for file name read.	*/
	char *fnp;			/* Pointer to file name buffer.	*/
	int fns;			/* The file name size.		*/
	char *bp;			/* malloc()'ed filename buffer.	*/

	oargp = *argvp;			/* Save pointer to old argv.	*/
	argc = alloced = 0;

	do {
	    if (argc >= alloced) {	/* Used up our allocated space?	*/
		if ((*argvp = _exec_argv(&alloced)) == NULL) {
		    perror("_exec_argv()");
		    return (FAILURE);
		}
		argp = *argvp + argc;
	    }

	    /*
	     * Copy remaining arguments into new argument array first.
	     */
	    if (*argcp > 0) {
		bp = *oargp++;
		(*argcp)--;
		argc++;
		continue;
	    }

	    if ((bp = fnp = fgets (fname, sizeof(fname), fp)) == NULL) {
		if (!feof(fp)) {
		    perror("fgets()");
		    return (FAILURE);
		}
	    } else {
		if (fnp[(fns=strlen(fname))-1] == '\n') {
		    fnp[--fns] = '\0';
		}
		/*
		 * Ensure path name is not too long for 'tar'.
		 */
		if (fns > TAR_PATH_LEN) {
		    errno = ENAMETOOLONG;
		    perror(fnp);
		    return (FAILURE);
		}
		argc++;
		if ( (bp = (char *) malloc (fns + 1)) == NULL) {
		    perror("malloc()");
		    return (FAILURE);
		}
		(void) strcpy (bp, fnp);
	    }

	} while ((*argp++ = bp) != NULL);

	*argcp = argc;
	return (SUCCESS);
}
