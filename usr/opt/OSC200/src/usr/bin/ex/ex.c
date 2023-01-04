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
static char rcsid[] = "@(#)$RCSfile: ex.c,v $ $Revision: 4.3.8.5 $ (DEC) $Date: 1993/11/05 20:06:40 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
#ifndef lint
char *copyright =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex.c
 *
 * FUNCTION: main, init, tailpath
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 
 * Copyright (c) 1981 Regents of the University of California 
 * 
 * 1.11  com/cmd/edit/vi/ex.c, bos320, 9134320 8/11/91 12:27:13
 */

#include <langinfo.h>
#include <locale.h>

nl_catd ex_catd;

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"

/*
 * The code for ex is divided as follows:
 *
 * ex.c 		Entry point and routines handling interrupt, hangup
 *			signals; initialization code.
 *
 * ex_addr.c		Address parsing routines for command mode decoding.
 *			Routines to set and check address ranges on commands.
 *
 * ex_cmds.c		Command mode command decoding.
 *
 * ex_cmds2.c		Subroutines for command decoding and processing of
 *			file names in the argument list.  Routines to print
 *			messages and reset state when errors occur.
 *
 * ex_cmdsub.c		Subroutines which implement command mode functions
 *			such as append, delete, join.
 *
 * ex_data.c		Initialization of options.
 *
 * ex_get.c		Command mode input routines.
 *
 * ex_io.c		General input/output processing: file i/o, unix
 *			escapes, filtering, source commands, preserving
 *			and recovering.
 *
 * ex_put.c		Terminal driving and optimizing routines for low-level
 *			output (cursor-positioning); output line formatting
 *			routines.
 *
 * ex_re.c		Global commands, substitute, regular expression
 *			compilation and execution.
 *
 * ex_set.c		The set command.
 *
 * ex_subr.c		Loads of miscellaneous subroutines.
 *
 * ex_temp.c		Editor buffer routines for main buffer and also
 *			for named buffers (Q registers if you will.)
 *
 * ex_tty.c		Terminal dependent initializations from termcap
 *			data base, grabbing of tty modes (at beginning
 *			and after escapes).
 *
 * ex_unix.c		Routines for the ! command and its variations.
 *
 * ex_v*.c		Visual/open mode routines... see ex_v.c for a
 *			guide to the overall organization.
 */

short ivis;
/*
 * Main procedure.  Process arguments and then
 * transfer control to the main command processing loop
 * in the routine commands.  We are entered as either "ex", "edit", "vi"
 * or "view" and the distinction is made here.	Actually, we are "vi" if
 * there is a 'v' in our name, "view" if there is a 'w', and "edit" if
 * there is a 'd' in our name.	For edit we just diddle options;
 * for vi we actually force an early visual command.
 */
extern char closepunct[ONMSZ];

#define VEDIT_USAGE	"Usage: vedit [-c subcommand] [-l] [-R] | [-r] [-t tag] [-wnumber] [-] [file ...]\n"

#define VI_USAGE	"Usage: vi [-c subcommand] [-ls] [-R] | [-r] [-t tag] [-wnumber] [+subcommand] [-] [file ...]\n"

#define VIEW_USAGE	"Usage: view [-c subcommand] [-l] [-t tag] [-wnumber] [+subcommand] [-] [file ...]\n"

#define EX_USAGE	"Usage: ex [-c subcommand] [-lRsv] [-wnumber] [+subcommand] [-] [file ...]\n   or: ex [-c subcommand] [-lRsv] [-t tag] [file ...]\n   or: ex [-c subcommand] -r[file] [-lRsv] [file]\n"

#define EDIT_USAGE	"Usage: edit [-c subcommand] [-lRv] [-wnumber] [+subcommand] [-] [file ...]\n   or: edit [-c subcommand] [-lRv] [-t tag] [file ...]\n   or: edit [-c subcommand] -r[file] [-lRv] [file]\n"


int
main(register int ac, register char *av[])
{
	register char *cp;
	register int c;
	char *ptr;
	char *test;
	static char buft[SBUFSIZ]; /* temporary buffer to form char strings  */
	int maxlines = MAXLINES;
	int maxlines_used = 0;
	short recov = 0;
 	short itag = 0;
	short fast = 0;
	char *charstr;
	int usage_msg;
	char *usage;
#ifdef TRACE
	register char *tracef;
#endif


	/*
	 * Immediately grab the tty modes so that we wont
	 * get messed up if an interrupt comes in quickly.
	 */
	gTTY(2);
#if !defined(USG) && !defined(_POSIX_SOURCE)
	normf = tty.sg_flags;
#else
	normf = tty;
#endif
	setlocale(LC_ALL,"");
	ex_catd = catopen(MF_EX,NL_CAT_LOCALE);
	(void) MSGSTR(M_000, "foo"); 	/* force catalog system to mmap */
	ppid = getpid();

	/*
	 * Defend against d's, v's, w's, and a's in directories of
	 * path leading to our true name.
	 */
	av[0] = tailpath(av[0]);

	/*
	 * Figure out how we were invoked: ex, edit, vi, view, vedit..
	 */
	if ((ivis = any('v', av[0])) == 1) {
		if (any('w', av[0])) {		/* view */
			value(READONLY) = 1;
			usage_msg = M_307;
			usage = VIEW_USAGE;
		} else if (any('d', av[0])) {	/* vedit */
			value(NOVICE) = 1;
			value(REPORT) = 1;
			value(MAGIC) = 0;
			value(SHOWMODE) = 1;
			usage_msg = M_305;
			usage = VEDIT_USAGE;
		} else {			/* vi */
			usage_msg = M_306;
			usage = VI_USAGE;
		}
	} else if (any('d', av[0])) {	/* edit */
		value(NOVICE) = 1;
		value(REPORT) = 1;
		value(MAGIC) = 0;
		value(SHOWMODE) = 1;
		usage_msg = M_309;
		usage = EDIT_USAGE;
	} else {
		usage_msg = M_308;	/* ex */
		usage = EX_USAGE;
	}

	/*
	 * Open the error message file.
	 */
	draino();
	pstop();

	/*
	 * Initialize interrupt handling.
	 */
	oldhup = signal(SIGHUP, SIG_IGN);
	if (oldhup == SIG_DFL)
		signal(SIGHUP, onhup);
	oldquit = signal(SIGQUIT, SIG_IGN);
	ruptible = (signal(SIGINT, SIG_IGN) == SIG_DFL);
	if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
	signal(SIGILL, oncore);
	signal(SIGTRAP, oncore);
	signal(SIGIOT, oncore);
	signal(SIGFPE, oncore);
	signal(SIGBUS, oncore);
	signal(SIGSEGV, oncore);
	signal(SIGPIPE, oncore);

	/*
	 * Process flag arguments.
	 */
	ac--, av++;
	while (ac && av[0][0] == '-') {
		c = av[0][1];
		if (c == 0) {
			hush = 1;
			value(AUTOPRINT) = 0;
			fast++;
		} else switch (c) {

		case 'R':
			value(READONLY) = 1;
			break;

                case 's':
                        hush = 1;
                        value(AUTOPRINT) = 0;
                        fast++;
                        break;

		case 'T':
#ifdef TRACE
			if (av[0][2] == 0)
				tracef = "trace";
			else {
				static char tttrace[] = { '/','d','e','v','/','t','t','y','x','x',0 };
				tracef = tttrace;
				tracef[8] = av[0][2];
				if (tracef[8])
					tracef[9] = av[0][3];
				else
					tracef[9] = 0;
			}
			trace = fopen(tracef, "w");
#define tracbuf NULL
			if (trace == NULL)
				ex_printf("Trace create error\n");
			else
				setbuf(trace, tracbuf);
#endif
			break;

		case 'l':
			value(LISP) = 1;
			value(SHOWMATCH) = 1;
			break;

		case 'r':
			recov++;
			break;

		case 't':
			if (ac > 1 && av[1][0] != '-') {
				ac--, av++;
				itag = 1;
				/* truncates too long tag. */
				if (mbstowcs(lasttag, av[0], WCSIZE(lasttag)) == -1) {
					fprintf(stderr, MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed.\n"));
					exit(1);
				}
			}
			break;

		case 'v':
			ivis = 1;
			break;

		case 'w':
			defwind = 0;
			if (av[0][2] == 0) defwind = 3;
			else for (cp = &av[0][2];
				  isascii(*cp) && isdigit(*cp);
				  cp++)
				defwind = 10*defwind + *cp - '0';
			break;

                case 'y':
                        maxlines = av[0][2] ? atoi(&av[0][2]) : ~(unsigned)0 >> 1;
                        maxlines_used = 1;
                        break;

                case 'c':
			/* Make sure there is a command following the -c. */
			if (ac < 2) {
				fprintf(stderr,MSGSTR(M_304, "No command specified to -c.\n"));
				fprintf(stderr,MSGSTR(usage_msg, usage));
				exit(1);
			}

                        firstpat = firstpatbuf;
                        if (mbstowcs(firstpat, &av[1][0], WCSIZE(firstpatbuf)) == -1) {
				fprintf(stderr, MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."));
				exit(1);
			}
                        ac--; av++;
                        break;
#ifdef CRYPT
                case 'x':
                        /* -x: encrypted mode */
                        xflag = 1;
			columns = 80;
                        break;
#endif

		default:
			fprintf(stderr, MSGSTR(M_000, "Unknown option %s\n"), av[0]);
			fprintf(stderr,MSGSTR(usage_msg, usage));
			exit(1);
			break;
		}
		ac--, av++;
	}

#ifdef SIGTSTP
# ifdef		TIOCLGET	/* Berkeley 4BSD */
	if (!hush && olttyc.t_suspc != '\377'
	    && signal(SIGTSTP, SIG_IGN) == SIG_DFL){
 		dosusp++;
		signal(SIGTSTP, onsusp);
	}
#endif
#ifdef         _POSIX_JOB_CONTROL
        if (!hush && tty.c_cc[VSUSP] != _POSIX_VDISABLE
            && signal(SIGTSTP, SIG_IGN) == SIG_DFL){
                dosusp++;
                signal(SIGTSTP, onsusp);
        }
#endif
#endif

	if (ac && av[0][0] == '+') {
	    firstpat = firstpatbuf;
	    if (mbstowcs(firstpat, &av[0][1], WCSIZE(firstpatbuf)) == -1)
		error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);
	    ac--, av++;
	}

#ifdef CRYPT
        if(xflag){
                key = getpass(KEYPROMPT);
                kflag = crinit(key, perm);
        }
#endif

	/*
	 * If we are doing a recover and no filename
	 * was given, then execute an exrecover command with
	 * the -r option to type out the list of saved file names.
	 * Otherwise set the remembered file name to the first argument
	 * file name so the "recover" initial command will find it.
	 */
	if (recov) {
		if (ac == 0) {
			ppid = 0;
			setrupt();
			execl(EXRECOVER, "exrecover", "-r", 0);
			filioerr(EXRECOVER);
			catclose(ex_catd);
			exit(1);
		}
		strcpy(savedfile, *av++), ac--;
	}

	/*
	 * Initialize the argument list.
	 */
	argv0 = av;
	argc0 = ac;
	args0 = av[0];

        /* Test for Japanese language to provide compatable closing
        punctuation with previous release. This is only valid for ibm932
        codeset  

        ptr = nl_langinfo(CODESET);
    test = "IBM-932";
    if (strcmp(test, ptr) == 0 )
    {
        int count;

        for(count = 0; closepunct[count] != 0; count++);
        sprintf(&closepunct[count], "%c%c%c%c%c%c%c%c",
            0xa1, 0xa4, 0x81, 0x41, 0x81, 0x42, 0x81, 0x76);
    }
*/
	/*
	 * Initialize a temporary file (buffer) and
	 * set up terminal environment.  Read user startup commands.
	 */
	if (setexit() == 0) {
		setrupt();
		intty = isatty(0);
		value(PROMPT) = intty;
		if (cp = getenv("SHELL"))
			strcpy(shell, cp);
		if (fast)
			setterm("dumb");
		else {
			gettmode();
			cp = getenv("TERM");
			if (cp == NULL || *cp == '\0')
				cp = "unknown";
			setterm(cp);
		}
	}

	erewind();

	if (setexit() == 0 && !fast) {
		char *p = getenv ("EXINIT");

		if (p && *p) {
			/* need to count the number of code points */
			char *tmpp;
			int len = 0;
			int charlen;

			tmpp = p;
                        while ((charlen = mblen(tmpp, MB_CUR_MAX)) > 0){
                                tmpp += charlen;
                                len++;
                        }
                        len += 1;
                        if ((globp = (wchar_t *) malloc(sizeof(wchar_t) * len))== NULL)
                        {
                                lprintf(MSGSTR(M_700, 
                       "Not enough memory space available.\n"), DUMMY_CHARP);
                                flush();
                                catclose(ex_catd);
                                exit(1);
                        }
                        if (mbstowcs(globp, p, (size_t)len) == -1)
                                error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);
                        commands((short)1,(short)1);
		}
		else {
			globp = 0;
			if ((cp = getenv("HOME")) != NULL && *cp) {
				strcat(strcpy(buft, cp), "/.exrc");
				if (iownit(buft) && nonwriteable(buft)) {
					source(buft, (short)1);
				}
			}
		}
		/*
                 * If exrc option is set, allow local .exrc too.
                 * This loses if . is $HOME, but nobody should notice unless
                 * they do stupid things like putting a version command in
                 * .exrc.  Besides, they should be using EXINIT, not .exrc,
                 * right?
		 */
		if (value(EXRC)) {
			if (iownit(".exrc") && nonwriteable(".exrc")) {
				source(".exrc", (short)1);
			}
		}
	}
#ifdef OSF_MMAP
	/*
	 * We can ask for a large initial size because with
	 * mmap in OSF/1 we pay no penalty if we don't use it.
	 *
	 * This also insures we get our memory after any loaded
	 * objects (like a locale) or other mmap'd things (like
	 * a message catalog).
	 * The odds of having to ask for more contiguous memory
	 * are decreased with a larger initial size as well.
	 */
        pagesize = sysconf(_SC_PAGE_SIZE);   
        fendcore = (line *)mmap(NULL, pagesize, PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE | MAP_VARIABLE,
                                -1, (off_t)0);
        endcore = fendcore + (pagesize/sizeof(line)) - 1;
#else
#ifdef  UNIX_SBRK
	/*
	 * Initialize end of core pointers.
	 * Normally we avoid breaking back to fendcore after each
	 * file since this can be expensive (much core-core copying).
	 * If your system can scatter load processes you could do
	 * this as ed does, saving a little core, but it will probably
	 * not often make much difference.
	 */
	fendcore = (line *) sbrk(0);
	endcore = fendcore - 2;
#else
	/*
	 * Allocate all the memory we will ever use in one chunk.
	 * This is for system such as VMS where sbrk() does not
	 * guarantee that the memory allocated beyond the end is
	 * consecutive.  VMS's RMS does all sorts of memory allocation
	 * and screwed up ex royally because ex assumes that all
	 * memory up to "endcore" belongs to it and RMS has different
	 * ideas.
	 */
        if (maxlines_used) {
                /*
                * Initialize end of core pointers.
                */
                if (maxlines < 1024) maxlines = 1024;
                while ((fendcore = (line *)malloc(maxlines * sizeof(line))) == NULL)
                        maxlines -= 1024;
                value(LINELIMIT) = maxlines;
        } else {
                fendcore = (line *) malloc((unsigned)
                        value(LINELIMIT) * sizeof (line *));
                if (fendcore == NULL) {
                        lprintf(MSGSTR(M_701, 
			  "cannot handle %d lines\n"),(char *)value(LINELIMIT));
                        lprintf(MSGSTR(M_702, 
				"set \"linelimit\" lower\n"), DUMMY_CHARP);
                        flush();
                        catclose(ex_catd);
                        exit(1);
                }
	}
	endcore = fendcore + (value(LINELIMIT) - 1);
#endif
#endif /* OSF_MMAP */
	init(); /* moved after prev 2 chunks to fix directory option */

	/*
	 * Initial processing.	Handle tag, recover, and file argument
	 * implied next commands.  If going in as 'vi', then don't do
	 * anything, just set initev so we will do it later (from within
	 * visual).
	 */
	if (setexit() == 0) {
		static wchar_t globbuf[8];	/* fit strlen("recover")+1 */

		if (recov)
                {
                    charstr = "recover";
                    globp = globbuf;
                    if (mbstowcs(globp, charstr, WCSIZE(globbuf)) == -1)
                        error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);
                }
		else if (itag)
                    {
                    charstr = ivis ? "tag" : "tag|p";
                    globp = globbuf;
                    if (mbstowcs(globp, charstr, WCSIZE(globbuf)) == -1)
                        error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);
                    }
		else if (argc)
                    {
                    charstr = "next";
                    globp = globbuf;
                    if (mbstowcs(globp, charstr, WCSIZE(globbuf)) == -1)
                        error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);
                    }
		if (ivis)
			initev = globp;
		else if (globp) {
			inglobal = 1;
			commands((short)1, (short)1);
			inglobal = 0;
		}
	}

	/*
	 * Vi command... go into visual.
	 * Strange... everything in vi usually happens
	 * before we ever "start".
	 */
	if (ivis) {
		static wchar_t globbuf[7];	/* fit strlen("visual")+1 */

		/*
		 * Don't have to be upward compatible with stupidity
		 * of starting editing at line $.
		 */
		if (dol > zero)
			dot = one;
                charstr = "visual";
                globp = globbuf;
                if (mbstowcs(globp, charstr, WCSIZE(globbuf)) == -1)
                        error(MSGSTR(M_650, "Invalid multibyte character string encountered, conversion failed."), DUMMY_INT);
		if (setexit() == 0)
			commands((short)1, (short)1);
	}

	/*
	 * Clear out trash in state accumulated by startup,
	 * and then do the main command loop for a normal edit.
	 * If you quit out of a 'vi' command by doing Q or ^\,
	 * you also fall through to here.
	 */
	seenprompt = 1;
	ungetchar(0);
	globp = 0;
	initev = 0;
	setlastchar('\n');
	setexit();
	commands((short)0, (short)0);
	cleanup((short)1);
	catclose(ex_catd);
	exit(0);
/* NOTREACHED */
}

/*
 * Initialization, before editing a new file.
 * Main thing here is to get a new buffer (in fileinit),
 * rest is peripheral state resetting.
 */
void
init(void)
{
	register int i;

	fileinit();
	dot = zero = truedol = unddol = dol = fendcore;
	one = zero+1;
	undkind = UNDNONE;
	chng = 0;
	edited = 0;
	for (i = 0; i <= 'z'-'a'+1; i++)
		names[i] = 1;
	anymarks = 0;
#ifdef CRYPT
        if(xflag) {
                xtflag = 1;
                makekey(key, tperm);
        }
#endif
}

/*
 * Return last component of unix path name p.
 */
char *
tailpath(register char *p)
{
	register char *r;

	for (r=p; *p; p++)
		if (*p == '/')
			r = p+1;
	return(r);
}

/*
 * Check ownership of file.  Return nonzero if it exists and is owned by the
 * user or the option sourceany is used
 */
int
iownit(char *file)
{
	struct stat sb;

	if (stat(file, &sb) == 0 && (value(SOURCEANY) || sb.st_uid == getuid()))
		return(1);
	else
		return(0);
}

/*
 * Check to see whether file has write permission permission bit set for
 * group or other (public). Return nonzero if neither write permission
 * bit is set.
 */

int
nonwriteable(char *file)
{
        struct stat sb;
        int cc = 0;

        cc = stat(file, &sb);

        if (((sb.st_mode & S_IWGRP) == S_IWGRP) ||
              ((sb.st_mode & S_IWOTH) == S_IWOTH))
                return(0);
        else
		return(1);
}
