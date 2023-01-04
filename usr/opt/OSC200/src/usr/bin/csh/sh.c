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
static char rcsid[] = "@(#)$RCSfile: sh.c,v $ $Revision: 4.2.12.8 $ (DEC) $Date: 1993/12/17 19:58:25 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: main untty importpath srccat srcunit rechist goodbye exitstat 
 *            phup pintr pintr1 process dosource mailchk gethdir initdesc exit
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 * Bill Joy, UC Berkeley, California, USA
 * October 1978, May 1980
 *
 * Jim Kulp, IIASA, Laxenburg, Austria
 * April 1980
 *
 *	1.27  com/cmd/csh/sh.c, cmdcsh, bos320, 9125320 5/22/91 13:51:37	
 */ 

#include <pwd.h>
#include <fcntl.h>
#include <locale.h>
#include "sh.h"
#include "pathnames.h"

int	ooSHIN;		
char	*pathlist[] =	{ ".", "/usr/bin", "/usr/bin", 0 };
char	*dumphist[] =	{ "history", "-h", 0, 0 };
char	*loadhist[] =	{ "source", "-h", "~/.history", 0 };
#ifndef _SBCS
wchar_t	HIST = '!';
wchar_t	HISTSUB = '^';
#else
uchar_t	HIST = '!';
uchar_t	HISTSUB = '^';
#endif
bool	nofile = 0;
volatile bool	reenter1 = 0; /* CMR001  Initializing vars for reenter test */
bool	nverbose;
bool	nexececho;
bool	quitit;
bool	fast = 0; 	/* CMR001  Initializing vars for reenter test */
bool    batch;
bool	prompt = 1;
bool	enterhist = 0;

nl_catd catd;       /* message catalog descriptor */

int open_max = {0};     /* highest file descriptor opened; we have to track
                           this, because OPEN_MAX is too big.
                         */


main(int c, uchar_t **av)
{
	register uchar_t **v, *cp;
	register int f;
	struct sigvec osv;

        settimes();                     /* Immed. estab. timing base */

	setlocale(LC_ALL,"");
#ifndef _SBCS
	mb_cur_max = MB_CUR_MAX;
	iswblank_handle = wctype("blank");
#endif
	/* Note that we cannot do a deferred open of the message
	 * catalog because the shell frees the storage of error values 
	 * before its error routines print out those values, counting 
	 * on no intervening alloc'ing -- which could happen on the 
	 * first error message if a deferred open is used.  Get any
	 * message from the catalog to insure a file descriptor is
	 * acquired.
	 */

	catd = catopen(MF_CSH, NL_CAT_LOCALE);
	(void)MSGSTR(M_DIRS, "");

	v = av;
	if (EQ(v[0], "a.out"))		/* A.out's are quittable */
		quitit = 1;
	uid = getuid();
	loginsh = **v == '-';
	if (loginsh)
		time(&chktim);

	/*
	 * Move the descriptors to safe places.
	 * The variable didfds is 0 while we have only FSH* to work with.
	 * When didfds is true, we have 0,1,2 and prefer to use these.
	 */
#ifdef DEBUG
printf("main: calling initdesc\n");
#endif
	initdesc();

	/*
	 * Initialize the shell variables.
	 * ARGV and PROMPT are initialized later.
	 * STATUS is also munged in several places.
	 * CHILD is munged when forking/waiting
	 */

	set((uchar_t *)"status", (uchar_t *)"0");
	dinit(cp = (uchar_t *)getenv("HOME"));	
					/* dinit thinks that HOME == cwd in a
					 * login shell */
	if (cp == NOSTR || !*cp)
		fast++;			/* No home -> can't read scripts */
	else
		set((uchar_t *)"home", savestr(cp));
	/*
	 * Grab other useful things from the environment.
	 * Should we grab everything??
	 */
	if ((cp = (uchar_t *)getenv("TERM")) != NOSTR && *cp) {
		set((uchar_t *)"term", savestr(cp));
	}

        /*
         * Initialization of $USER, $user:
         *    if $USER exists, copy it into $user
         *    else if $LOGNAME exists, copy it into $USER and $user
         */
        if((cp = (uchar_t *)getenv("USER")) != NOSTR&&*cp) {
                set((uchar_t *)"user", savestr(cp));
	}
        else if((cp = (uchar_t *)getenv("LOGNAME")) != NOSTR && *cp) {
                setcenv("USER", cp);
                set((uchar_t *)"user", savestr(cp));
        }

	/*
	 * Re-initialize path if set in environment
	 */
	if((cp = (uchar_t *)getenv("PATH")) == NOSTR || !*cp) {
		set1((uchar_t *)"path", saveblk((uchar_t **)pathlist), &shvhed);
	}
	else {
		register unsigned i = 0;
		register uchar_t *dp;
		register uchar_t **pv;

		for (dp = cp; *dp; dp++)
			if (*dp == ':')
				i++;
		pv = (uchar_t **)calloc(i+2, sizeof (uchar_t **));
		for (dp = cp, i = 0; ;)
			if (*dp == ':') {
				*dp = 0;
				pv[i++] = savestr(*cp ? cp : (uchar_t *)".");
				*dp++ = ':';
				cp = dp;
			} else if (*dp++ == 0) {
				if (*cp)
					pv[i++] = savestr(cp);
				break;
			}
		pv[i] = 0;
		set1((uchar_t * )"path", pv, &shvhed);
	}

	set((uchar_t *)"shell", (uchar_t *)_PATH_CSHELL);
#ifdef CMDEDIT
        if ((cp = (uchar_t *) getenv("EDITMODE")) != NOSTR)
                set((uchar_t *)"editmode", savestr(cp));
#endif

	doldol = putn(getpid());			/* For $$ */
	shtemp = strspl((uchar_t *)"/tmp/sh", doldol);	/* For << */

	/*
	 * Record the interrupt states from the parent process.
	 * If the parent is non-interruptible our hand must be forced
	 * or we (and our children) won't be either.
	 * Our children inherit termination from our parent.
	 * We catch it only if we are the login shell.
	 */
		/* parents interruptibility */
	(void) sigvec(SIGINT, (struct sigvec *)0, &osv);
	parintr = osv.sv_handler;
		/* parents terminability */
	(void) sigvec(SIGTERM, (struct sigvec *)0, &osv);
	parterm = osv.sv_handler;
	if (loginsh) {
		struct sigvec nsv;

		nsv.sv_handler = (void (*)(int))phup;
		nsv.sv_mask = 0;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGHUP, &nsv,
			(struct sigvec *)0);   /* exit processing on HUP */
		(void)sigvec(SIGXCPU, &nsv,
			(struct sigvec *)0);  /* ...and on XCPU */
		(void)sigvec(SIGXFSZ, &nsv,
			(struct sigvec *)0);  /* ...and on XFSZ */
	}

#ifdef DEBUG
	printf("main: processing arguments\n");
#endif
	/*
	 * Process the arguments.
	 *
	 * Note that processing of -v/-x is actually delayed till after
	 * script processing.
	 *
	 * We set the first character of our name to be '-' if we are
	 * a shell running interruptible commands.  Many programs which
	 * examine ps'es use this to filter such shells out.
	 */
	c--, v++;
	while (c > 0 && (cp = v[0])[0] == '-' && !batch) {
		do switch (*cp++) {

		case 0:			/* -	Interruptible, no prompt */
			prompt = 0;
			setintr++;
			nofile++;
			break;

		case 'b':               /* -b   Next arg is input file */
			batch++;
			break;

		case 'c':		/* -c	Command input from arg */
			if (c == 1)
				exitcsh(0);
			c--, v++;
			arginp = v[0];
			prompt = 0;
			nofile++;
			break;

		case 'e':		/* -e	Exit on any error */
			exiterr++;
			break;

		case 'f':		/* -f	Fast start */
			fast++;
			break;

		case 'i':		/* -i	Interactive, even if !intty */
			intact++;
			nofile++;
			break;

		case 'n':		/* -n	Don't execute */
			noexec++;
			break;

		case 'q':		/* -q	(Undoc'd) ... die on quit */
			quitit = 1;
			break;

		case 's':		/* -s	Read from std input */
			nofile++;
			break;

		case 't':		/* -t	Read one line from input */
			onelflg = 2;
			prompt = 0;
			nofile++;
			break;

		case 'v':		/* -v	Echo hist expanded input */
			nverbose = 1;			/* ... later */
			break;

		case 'x':		/* -x	Echo just before execution */
			nexececho = 1;			/* ... later */
			break;

		case 'V':		/* -V	Echo hist expanded input */
			set((uchar_t *)"verbose",(uchar_t *)"");/*NOW!*/
			break;

		case 'X':		/* -X	Echo just before execution */
			set((uchar_t *)"echo",(uchar_t *)"");/*NOW!*/
			break;

		} while (*cp);
		v++, c--;
	}

	if (quitit)  {                  /* With all due haste, for debugging */
		struct sigvec nsv;

		nsv.sv_handler = SIG_DFL;
		nsv.sv_mask = 0;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGQUIT, &nsv,
			(struct sigvec *)0);   /* exit processing on HUP */
	}

	/*
	 * Unless prevented by -, -c, -i, -s, or -t, if there
	 * are remaining arguments the first of them is the name
	 * of a shell file from which to read commands.
	 */
	if (nofile == 0 && c > 0) {
		nofile = open((char *)v[0], 0);
#ifdef DEBUG
	printf("main: open(%s) yields %d\n",v[0],nofile);
#endif
		if (nofile < 0) {
			child++;		/* So this ... */
			Perror((char *)v[0]);	/* ... doesn't return */
		}
                track_open(nofile);
		file = v[0];
		SHIN = dmove(nofile, FSHIN);	/* Replace FSHIN */
		prompt = 0;
		c--, v++;
	}
	if (!batch && (uid != geteuid() || getgid() != getegid())) {
		errno = EACCES;
		child++;                        /* So this ... */
		Perror("csh");		       /* ... doesn't return */
	}
	/*
	 * Consider input a tty if it really is or we are interactive.
	 */
	intty = intact || isatty(SHIN);
	/*
	 * Decide whether we should play with signals or not.
	 * If we are explicitly told (via -i, or -) or we are a login
	 * shell (arg0 starts with -) or the input and output are both
	 * the ttys("csh", or "csh</dev/ttyx>/dev/ttyx")
	 * Note that in only the login shell is it likely that parent
	 * may have set signals to be ignored
	 */
	if (loginsh || intact || intty && isatty(SHOUT))
		setintr = 1;
#ifdef CMDEDIT
        if (setintr) {
                register struct varent *vp;

                vp = adrof((uchar_t *)"editmode");
                if (vp && vp->vec[0] && !EQ(vp->vec[0], "none"))
                        cmdedit = 1;
        }
#endif
	/*
	 * Save the remaining arguments in argv.
	 */
	setq((uchar_t *)"argv", v, &shvhed);

	/*
	 * Set up the prompt.
	 */
	if (prompt)
                set((uchar_t *)"prompt",
		    (uid == 0) ? (uchar_t *)"# " : (uchar_t *)"% ");

	/*
	 * If we are an interactive shell, then start fiddling
	 * with the signals; this is a tricky game.
	 */
	shpgrp = getpgrp();
	opgrp = tpgrp = -1;
	oldisc = -1;
#ifdef DEBUG
	printf("main: setintr = %d\n", setintr);
#endif
	if (setintr) {
		struct sigvec nsv;

		**av = '-';
		nsv.sv_handler = SIG_IGN;
		nsv.sv_mask = SA_RESTART;
		nsv.sv_onstack = 0;
		if (!quitit)		/* Wary! */
			(void)sigvec(SIGQUIT, &nsv, (struct sigvec *)0);
		nsv.sv_handler = (void (*)(int))pintr;
		(void)sigvec(SIGINT, &nsv, (struct sigvec *)0);
		(void) sigblock(sigmask(SIGINT));
		nsv.sv_handler = SIG_IGN;
		(void)sigvec(SIGTERM, &nsv, (struct sigvec *)0);
		if (quitit == 0 && arginp == 0) {
			(void)sigvec(SIGTSTP, &nsv, (struct sigvec *)0);
			(void)sigvec(SIGTTIN, &nsv, (struct sigvec *)0);
			(void)sigvec(SIGTTOU, &nsv, (struct sigvec *)0);
			/*
			 * Wait till in foreground, in case someone
			 * stupidly runs
			 *	csh &
			 * dont want to try to grab away the tty.
			 */
			if (isatty(FSHDIAG))
				f = FSHDIAG;
			else if (isatty(FSHOUT))
				f = FSHOUT;
			else if (isatty(OLDSTD))
				f = OLDSTD;
			else
				f = -1;
retry:
			if (ioctl(f, TIOCGPGRP, &tpgrp) == 0 && tpgrp != -1) {
				int ldisc;
				struct sigvec osv;
#ifdef DEBUG
	printf("main: tpgrp = %d  shpgrp = %d\n", tpgrp, shpgrp);
#endif
				if (tpgrp != shpgrp) {
					nsv.sv_handler = SIG_DFL;
					nsv.sv_mask = SA_RESTART;
					nsv.sv_onstack = 0;
					(void)sigvec(SIGTTIN, &nsv, &osv);
					kill(getpid(), SIGTTIN);
					(void)sigvec(SIGTTIN, &osv,
						(struct sigvec *)0);
					goto retry;
				}
				oldisc = -1;
				opgrp = shpgrp;
				shpgrp = getpid();
				tpgrp = shpgrp;
#ifdef DEBUG
printf("main: setting pgrp tpgrp = %d  shpgrp = %d\n", tpgrp, shpgrp);
#endif
				{
				struct sigvec nsv, osv;
		
				nsv.sv_handler = SIG_IGN;
				nsv.sv_mask = 0;
				nsv.sv_onstack = 0;
				(void)sigvec(SIGTTOU, &nsv, &osv);
				IOCTL(f, TIOCSPGRP, &shpgrp, "20");
				(void)sigvec(SIGTTOU, &osv, (struct sigvec *)0);
				}
  				setpgid(0, shpgrp);
				dcopy(f, FSHTTY);
				/* Replaced with fcntl to set close on exec */
				/*
				ioctl(FSHTTY, FIOCLEX, 0);
				*/
				fcntl(FSHTTY, F_SETFD, 0);
			} else {

notty:
 csh_printf(MSGSTR(M_NOTTY,
 "Warning: no access to tty; thus no job control in this shell...\n"));
				tpgrp = -1;
#ifdef CMDEDIT
                                cmdedit = 0;
#endif

			}
		}
	}
#ifdef DEBUG
	printf("main: after setting pgrp tpgrp = %d  shpgrp = %d\n",
		tpgrp, shpgrp);
#endif
	if (setintr == 0 && (void(*)(int))parintr == SIG_DFL)
		setintr++;
	{       /* while signals not ready */
		struct sigvec nsv;
		nsv.sv_handler = (void (*)(int))pchild;
		nsv.sv_mask = 0;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGCHLD, &nsv, (struct sigvec *)0);
	}

	/*
	 * Set an exit here in case of an interrupt or error reading
	 * the shell start-up scripts.
	 */
	setexit();
	haderr = 0;		/* In case second time through */
	if (!fast && reenter1 == 0) {	/* Name modified to avoid conflict */
		reenter1++;		 /* CMR001       */
	/* Will have value((uchar_t *)"home") here because set fast if don't */
        /* Add code for csh.login BJB 002 */
                if (loginsh ) {
                        srccat((uchar_t *)"/etc", (uchar_t *)"/csh.login", 0);
                }
		srccat(value((uchar_t *)"home"),(uchar_t *)"/.cshrc", 1);
		if (!fast && !arginp && !onelflg)
			dohash();
		if (loginsh) {
			srccat(value((uchar_t *)"home"), (uchar_t *)"/.login", 1);
		}
		dosource((uchar_t **)loadhist);
	}

	/*
	 * Now are ready for the -v and -x flags
	 */
	if (nverbose)
		set((uchar_t *)"verbose", (uchar_t *)"");
	if (nexececho)
		set((uchar_t *)"echo", (uchar_t *)"");

	/*
	 * All the rest of the world is inside this call.
	 * The argument to process indicates whether it should
	 * catch "error unwinds".  Thus if we are a interactive shell
	 * our call here will never return by being blown past on an error.
	 */
	process(setintr);
	/*
	 * Mop-up.
	 */
	if (loginsh) {
		csh_printf(MSGSTR(M_BYEBYE, "logout\n"));
		close(SHIN);
		child++;
		goodbye();
	}
	rechist();
	exitstat();
}

void
untty(void)
{
	struct sigvec nsv, osv;
		
	nsv.sv_handler = SIG_IGN;
	nsv.sv_mask = SA_RESTART;
	nsv.sv_onstack = 0;
	(void)sigvec(SIGTTOU, &nsv, &osv);
	if (tpgrp > 0) {
		setpgid(0, opgrp);
		IOCTL(FSHTTY, TIOCSPGRP, &opgrp, "21");
	}
	(void)sigvec(SIGTTOU, &osv, (struct sigvec *)NULL);
}

void
importpath(uchar_t *cp)
{
	register int i = 0;
	register uchar_t *dp;
	register uchar_t *sp;
	register uchar_t **pv;
	static uchar_t dot[2] = {'.', 0};

	for (dp = cp; *dp; dp++)
		if (*dp == ':')
			i++;
	/*
	 * i+2 where i is the number of colons in the path.
	 * There are i+1 directories in the path plus we need
	 * room for a zero terminator.
	 */
	pv = (uchar_t **)calloc(i+2, sizeof(char **));
	dp = cp;
	sp = dp;		/* save off beginning of dp */
	i = 0;

	for (dp = cp; *dp; ++dp) {

		if (*dp == ':') {
			*dp = 0;
			pv[i++] = savestr(*cp ? cp : dot);
			cp = dp + 1;
			*dp = ':';
		}
	}
	/*
	* handle trailing ':'.
	* /usr/bin:  ==>  /usr/bin  .
	*/
	if (*cp || (dp > sp && *(dp - 1) == ':'))
		pv[i++] = savestr(*cp ? cp : dot);

	pv[i] = 0;
	set1((uchar_t *)"path", pv, &shvhed);
}

/*
 * Source to the file which is the catenation of the argument names.
 */
/* Add parameter for csh.login execution BJB 002 */
void
srccat(uchar_t *cp, uchar_t *dp, int perm)
{
	register uchar_t *ep = strspl(cp, dp);
	register int unit = dmove(open((char *)ep, O_RDONLY), -1);

	/* ioctl(unit, FIOCLEX, NULL); */
	xfree(ep);
	srcunit(unit, perm, 0);
}

/*
 * Source to a unit.  If onlyown it must be our file or our group or
 * we don't chance it.	This occurs on ".cshrc"s and the like.
 */
void
srcunit(register int unit, bool onlyown, bool hflg)
{
	/* We have to push down a lot of state here */
	/* All this could go into a structure */
	int oSHIN = -1, oldintty = intty;
	int *tmp = &oSHIN;    
	struct whyle *oldwhyl = whyles;
	uchar_t *ogointr = gointr, *oarginp = arginp;
	uchar_t *oevalp = evalp, **oevalvec = evalvec;
	int oonelflg = onelflg;
	bool oenterhist = enterhist;
#ifndef _SBCS
	int OHIST = HIST;
#else
	uchar_t OHIST = HIST;
#endif
	struct Bin saveB;

	/* The (few) real local variables */
	jmp_buf oldexit;
	static int reenter;
	int omask;

	if (unit < 0)
		return;
	if (didfds)
		donefds();
	if (onlyown) {
		struct stat stb;

		if (fstat(unit, &stb) < 0 ||
		    (stb.st_uid != uid && stb.st_gid != getgid())) {
			close(unit);
			return;
		}
	}

	/*
	 * There is a critical section here while we are pushing down the
	 * input stream since we have stuff in different structures.
	 * If we weren't careful an interrupt could corrupt SHIN's Bin
	 * structure and kill the shell.
	 *
	 * We could avoid the critical region by grouping all the stuff
	 * in a single structure and pointing at it to move it all at
	 * once.  This is less efficient globally on many variable references
	 * however.
	 */
	getexit(oldexit);
	reenter = 0;
	if (setintr)
		omask = sigblock(sigmask(SIGINT));
	setexit();
	reenter++;
	if (reenter == 1) {
		/* Setup the new values of the state stuff saved above */
		copy((uchar_t *)&saveB, (uchar_t *)&B, sizeof(saveB));
		fbuf = (uchar_t **) 0;
		fseekp = feobp = fblocks = 0;
		ooSHIN = SHIN;  
		oSHIN = SHIN, SHIN = unit, arginp = 0, onelflg = 0;
		intty = isatty(SHIN), whyles = 0, gointr = 0;
		evalvec = 0; evalp = 0;
		enterhist = hflg;
		if (enterhist)
			HIST = '\0';
		/*
		 * Now if we are allowing commands to be interrupted,
		 * we let ourselves be interrupted.
		 */
		if (setintr)
			(void)sigsetmask(omask);
		process(0);		/* 0 -> blow away on errors */
	}
	if (setintr)
		(void)sigsetmask(omask);
	if (oSHIN >= 0 || reenter == 2) {   
		register int i;

		/* We made it to the new state... free up its storage */
		/* This code could get run twice but free doesn't care */
		for (i = 0; i < fblocks; i++)
			xfree((uchar_t *)fbuf[i]);
		xfree((uchar_t *)fbuf);

		/* Reset input arena */
		copy((uchar_t *)&B, (uchar_t *)&saveB, sizeof(B));

		close(SHIN), SHIN = oSHIN;
		arginp = oarginp, onelflg = oonelflg;
		evalp = oevalp, evalvec = oevalvec;
		intty = oldintty, whyles = oldwhyl, gointr = ogointr;
		if (enterhist)
			HIST = OHIST;
		enterhist = oenterhist;
	}

	resexit(oldexit);
	/*
	 * If process reset() (effectively an unwind) then
	 * we must also unwind.
	 */
	if (reenter >= 2)
		error((char *)NOSTR);
}

void
rechist()
{
	static uchar_t savehist[]="savehist";
	uchar_t buf[PATH_MAX + 1];
	int fp, ftmp, oldidfds;

	if (!fast) {
		if (value((uchar_t *)savehist)[0] == '\0')
			return;
		strcpy( (char *)buf, (char *) value((uchar_t *)"savehist"));
		strcat(buf, (uchar_t *)"/.history");
		fp = creat((char *)buf, S_IRWXU);
		if (fp == -1)
			return;
		oldidfds = didfds;
		didfds = 0;
		ftmp = SHOUT;
		SHOUT = fp;
		strcpy( (char *)buf, (char *) value((uchar_t *)"savehist"));
		dumphist[2] = (char *)buf;
		dohist((uchar_t **)dumphist);
		close(fp);
		SHOUT = ftmp;
		didfds = oldidfds;
	}
}

void
goodbye()
{
	struct sigvec nsv;

	nsv.sv_handler = SIG_IGN;
	nsv.sv_mask = 0;
	nsv.sv_onstack = 0;
	if (loginsh) {
		(void)sigvec(SIGQUIT, &nsv, (struct sigvec *)NULL);
		(void)sigvec(SIGINT, &nsv, (struct sigvec *)NULL);
		(void)sigvec(SIGTERM, &nsv, (struct sigvec *)NULL);
		/*
		 * Allow background processes forked from "~/.logout" to
		 * continue running after logout is completed.
		 *
		 * This allows cleanup routines, etc. to be run automatically
		 * when the user logs out. I can't see any reason not to
		 * allow this since processes can be automatically run when the
		 * user isn't logged on anyway, using at(1).
		 *
		 * Although the 4.1 and 4.2 version of csh don't explicitly
		 * have this code (i.e. catching SIGHUP during the sourcing of
		 * .logout), I believe they do allow the continued execution of
		 * processes forked from the .logout sourcing; perhaps this is
		 * due to a different definition of "process group"?
		 */
		(void)sigvec(SIGHUP, &nsv, (struct sigvec *)NULL);
		setintr = 0;		/* No interrupts after "logout" */
		if (adrof((uchar_t *)"home"))
			(void)srccat(value((uchar_t *)"home"), (uchar_t *)"/.logout", 1);
	}
#ifdef CMDEDIT
        if (!fast && cmdedit)
                SaveMacros();
#endif
	rechist();
	exitstat();
}

void
exitstat()
{
	int stat;
	/*
	 * Note that if STATUS is corrupted (i.e. getn bombs)
	 * then error will exit directly because we poke child here.
	 * Otherwise we might continue unwarrantedly (sic).
	 */
	child++;
	stat = (int)getn(value((uchar_t *)"status"));              /* 002 */
	exitcsh(stat);
}

/*
 * in the event of a HUP we want to save the history
 */
void
phup()
{
	rechist();
	exitcsh(1);
}

char	*jobargv[2] = { "jobs", 0 };
/*
 * Catch an interrupt, e.g. during lexical input.
 * If we are an interactive shell, we reset the interrupt catch
 * immediately.  In any case we drain the shell output,
 * and finally go through the normal error mechanism, which
 * gets a chance to make the shell go away.
 */
void
pintr()
{
	pintr1(1);
}

void
pintr1(bool wantnl)
{
	register uchar_t **v;
	int omask;

	omask = sigblock(0);
	if (setintr) {
		(void)sigsetmask(omask & ~sigmask(SIGINT));
		if (pjobs) {
			pjobs = 0;
			csh_printf("\n");
			dojobs((uchar_t **)jobargv);
			bferr(MSGSTR(M_INTER, "Interrupted"));
		}
	}
	(void)sigsetmask(omask & ~sigmask(SIGCHLD));
	draino();

	/*
	 * If we have an active "onintr" then we search for the label.
	 * Note that if one does "onintr -" then we shan't be interruptible
	 * so we needn't worry about that here.
	 */
	if (gointr) {
		search(ZGOTO, 0, gointr);
		timflg = 0;
		if(pargv) {
			v = pargv;
			pargv = 0;
			blkfree(v);
		}
		if(gargv) {
			v = gargv;
			gargv = 0;
			blkfree(v);
		}
		reset();
	} else if (intty && wantnl)
		csh_printf("\n");		/* Some like this, others don't */
	error((char *)NOSTR);
}

/*
 * Process is the main driving routine for the shell.
 * It runs all command processing, except for those within { ... }
 * in expressions (which is run by a routine evalav in sh.exp.c which
 * is a stripped down process), and `...` evaluation which is run
 * also by a subset of this code in sh.glob.c in the routine backeval.
 *
 * The code here is a little strange because part of it is interruptible
 * and hence freeing of structures appears to occur when none is necessary
 * if this is ignored.
 *
 * Note that if catch is not set then we will unwind on any error.
 * If an end-of-file occurs, we return.
 */
void
process(bool catch)
{
	register uchar_t *cp;
	jmp_buf osetexit;
	struct command *t;

	getexit(osetexit);
	for (;;) {
#ifdef CMDEDIT
                int sawhist;
                int edithist = (adrof((uchar_t *)"edithist") != 0);
#endif
		pendjob();
		paraml.next = paraml.prev = &paraml;
		paraml.word = (uchar_t *)"";
		t = 0;
		setexit();
		justpr = enterhist;	/* execute if not entering history */

		/*
		 * Interruptible during interactive reads
		 */
		if (setintr)
			(void)sigsetmask(sigblock(0) & ~sigmask(SIGINT));

		/*
		 * For the sake of reset()
		 */
		freelex(&paraml);
		freesyn(t);
		t = 0;

		if (haderr) {
			if (!catch) {
				/* unwind */
				doneinp = 0;
				resexit(osetexit);
				reset();
			}
			haderr = 0;
			/*
			 * Every error is eventually caught here or
			 * the shell dies.  It is at this
			 * point that we clean up any left-over open
			 * files, by closing all but a fixed number
			 * of pre-defined files.  Thus routines don't
			 * have to worry about leaving files open due
			 * to deeper errors... they will get closed here.
			 */
			closem();
			continue;
		}
		if (doneinp) {
			doneinp = 0;
			break;
		}
		if (chkstop)
			chkstop--;
		if (neednote)
			pnote();
		if (intty && evalvec == 0 && !arginp) {
			mailchk();
			/*
			 * If we are at the end of the input buffer
			 * then we are going to read fresh stuff.
			 * Otherwise, we are rereading input and don't
			 * need or want to prompt.
			 */
#ifdef DEBUG
printf("debug: process\n");
#endif
			if (fseekp == feobp)
				printprompt();
		}
		err = 0;

		/*
		 * Echo not only on VERBOSE, but also with history expansion.
		 * If there is a lexical error then we forego history echo.
		 */
#ifdef CMDEDIT
                /* Also push back input if history went off in
                 * interactive mode.
                 */
                if ((sawhist=lex(&paraml)) && !err && intty ||
                    adrof((uchar_t *)"verbose")) {
                        haderr = 1;
                        if ((justpr || edithist) && sawhist && cmdedit)
                            prpushlex(&paraml);
                        else
                            prlex(&paraml);
                        haderr = 0;
                }
#else   /* CMDEDIT */
		if (lex(&paraml) && !err && intty ||
		    adrof((uchar_t *)"verbose")) {
			haderr = 1;
			prlex(&paraml);
			haderr = 0;
		}
#endif /* CMDEDIT */

		/*
		 * The parser may lose space if interrupted.
		 */
		if (setintr)
			(void)sigblock(sigmask(SIGINT));

		/*
		 * Save input text on the history list if 
		 * reading in old history, or it
		 * is from the terminal at the top level and not
		 * in a loop.
		 */
#ifdef CMDEDIT
                /* ... unless we have just pushed the command back.
                 */
                if (!((justpr || edithist) && sawhist && cmdedit))
#endif
		if (enterhist || catch && intty && !whyles)
			savehist(&paraml);

		/*
		 * Print lexical error messages, except when sourcing
		 * history lists.
		 */
		if (!enterhist && err)
			error((char *)err);

		/*
		 * If had a history command :p modifier then
		 * this is as far as we should go
		 */
#ifdef CMDEDIT
                /* ... or if we had an interactive history expand
                 */
                if (justpr || (edithist && sawhist && cmdedit))
                        reset();
#else

		if (justpr)
			reset();
#endif

		alias(&paraml);

		/*
		 * Parse the words of the input into a parse tree.
		 */
		t = syntax(paraml.next, &paraml, 0);
		if (err)
			error((char *)err);

		/*
		 * Execute the parse tree
		 */
#ifdef DEBUG
printf("process: calling execute\n");
#endif
		execute(t, tpgrp, (int *)0, (int *)0);
#ifdef DEBUG
printf("process:  back from execute\n");
#endif

		freelex(&paraml);
		freesyn(t);
	}
	resexit(osetexit);
}

void
dosource(register uchar_t **t)
{
	register uchar_t *f;
	register int u;
	bool hflg = 0;
	uchar_t buf[BUFSIZ];

	t++;
	if (*t && *t[0] == '-') 
	{
		if ( EQ(*t, "-h") ) 
		{
			t++;
			hflg++;
			if (*t == NULL)
				bferr(MSGSTR(M_TOOFEW, "Too few arguments"));
		} else 
		{
			flush ();
			haderr = 1;
			csh_printf (MSGSTR(M_OPTION,"Unknown option : %s \n"), *t);
			error(NULL);
		}
        }
	strcpy( (char *)buf, (char *) *t);
	f = globone(buf);
	u = dmove(open((char *)f, O_RDONLY), -1);
	if (u < 0 && !hflg)
		Perror_free((char *)f);
	xfree(f);
	srcunit(u, 0, hflg);
}

/*
 * Check for mail.
 * If we are a login shell, then we don't want to tell
 * about any mail file unless its been modified
 * after the time we started.
 * This prevents us from telling the user things he already
 * knows, since the login program insists on saying
 * "You have mail."
 */
void
mailchk(void)
{
	register struct varent *v;
	register uchar_t **vp;
	time_t t;
	int intvl, cnt;
	struct stat stb;
	bool new;

	v = adrof((uchar_t *)"mail");
	if (v == 0)
		return;
	time(&t);
	vp = v->vec;
	cnt = blklen(vp);
	intvl = (cnt && number(*vp)) ? (--cnt, (int)getn(*vp++)) : MAILINTVL;     /* 002 */
	if (intvl < 1)
		intvl = 1;
	if (chktim + intvl > t)
		return;
	for (; *vp; vp++) {
		if (stat((char *)*vp, &stb) < 0)
			continue;
		new = stb.st_mtime > time0.tv_sec;
		if (stb.st_size == 0 || stb.st_atime > stb.st_mtime ||
		    (stb.st_atime < chktim && stb.st_mtime < chktim) ||
		    loginsh && !new)
			continue;
		if (cnt == 1)
			csh_printf (new ? MSGSTR(M_NEWMAIL, "You have new mail.\n")
				    : MSGSTR(M_MAIL, "You have mail.\n"));
		else
			csh_printf (new ? MSGSTR(M_NEWMAILIN, "New mail in %s.\n")
				    : MSGSTR(M_MAILIN, "Mail in %s.\n"), *vp);
	}
	chktim = t;
}

/*
 * Extract a home directory from the password file
 * The argument points to a buffer where the name of the
 * user whose home directory is sought is currently.
 * We write the home directory of the user back there.
 */
gethdir(uchar_t *home)
{
	struct passwd *pp = (struct passwd *) NULL;

        endpwent();	/* 001 BJB */

	pp = getpwnam((char *)home);

	if (pp == 0)
		return (1);
	strcpy( (char *)home, pp->pw_dir);
	return (0);
}

/*
 * Move the initial descriptors to their eventual
 * resting places, closin all other units.
 */
void
initdesc(void)
{

	didcch = 0;			/* Havent closed for child */
	didfds = 0;			/* 0, 1, 2 aren't set up */
	SHIN = dcopy(0, FSHIN);
	SHOUT = dcopy(1, FSHOUT);
	SHDIAG = dcopy(2, FSHDIAG);
	OLDSTD = dcopy(SHIN, FOLDSTD);
#ifdef DEBUG
printf("initdesc:closing file desc.\n");
#endif
	closem();
}

void
exitcsh(int i)
{
	untty();
	_exit(i);
}


void
printprompt()
{
	extern uchar_t *linp, linbuf[];
	register uchar_t *cp;
	extern bool	print_control_codes;

#ifndef _SBCS
	register int	 n;
	wchar_t		 nl;
#endif

	print_control_codes = 1;/* Don't interpret prompt.
				   Note: If you have ctrl chars in your
				   prompt, the shell will get confused as
				   to where the cursor is.*/

	if (!whyles) {
		linp = linbuf;
#ifndef _SBCS 
		for (cp = value((uchar_t *)"prompt"); *cp;) {
			n = mbtowc(&nl, (char *)cp, MB_CUR_MAX);
			if (n < 0) {
				n = 1;
				nl = *cp & 0xff;
			}
			if (nl == HIST) {
				csh_printf("%d", eventno + 1);
				cp += n;
			} else {
				if (*cp == '\\') {
					n = mbtowc(&nl, (char *)cp+1, 
						MB_CUR_MAX);
					if (n < 1) {
						n = 1;
						nl = cp[1] & 0xff;
					}
					if (nl == HIST)
						cp++;
				}
				do {
					display_char(*cp++);
				} while (--n);
			}
		}
#else
		for (cp = value((uchar_t *)"prompt"); *cp; cp++) {
			if (*cp == HIST)
				csh_printf("%d", eventno + 1);
			else {
				if (*cp == '\\' && cp[1] == HIST)
					cp++;
				if ((*cp&QUOTE)==0)
					display_char(NLQUOTE);
				display_char(*cp);
			}
		}
#endif
	}
	else 
		/*
		 * Prompt for forward reading loop
		 * body content.
		 */
		csh_printf("? ");
	flush();
	print_control_codes = 0;
}
