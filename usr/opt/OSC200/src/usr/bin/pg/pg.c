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
static char rcsid[] = "@(#)$RCSfile: pg.c,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/10/11 17:42:15 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.23  com/cmd/scan/pg.c, cmdscan, bos320, 9146320b 11/13/91 09:29:52
 * 
 *
 *      pg -- paginator for crt terminals
 *
 *   The pg  command reads files  and writes them  to standard
 *   output one  screen at a time.   If you specify file  as -
 *   (minus) or  run pg  without arguments, pg  reads standard
 *   input.   Each screen  is followed  by a  prompt.  If  you
 *   press the Enter  key, another page is  displayed.  The pg
 *   command lets  you back  up to  review something  that has
 *   already passed.
 *
 *   Includes the ability to display pages that have
 *   already passed by. Also gives the user the ability
 *   to search forward and backwards for regular expressions.
 *   This works for piped input by copying to a temporary file,
 *   and resolving back references from there.
 *
 *	Note:	The reason that there are so many commands to do
 *		the same types of things is to try to accommodate
 *		users of other paginators.
 */                                                                   

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <nl_types.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <stdio.h>
#include <curses.h>
#include <errno.h>
#include <term.h>
#include <termios.h>
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>
#include "pathnames.h"
#include "pg_msg.h"

nl_catd	catd;
#define MSGS(n,s)	catgets(catd,MS_PG,n,s)

#define LINSIZ	1024
#define QUIT	'\034'
#define BOF	(EOF - 1)	/* Beginning of File */
#define STOP    (EOF - 2)
#define PROMPTSIZE	256
#undef	TTY_PAGE 


struct line {			/* how line addresses are stored */
	long	l_addr;		/* file offset */
	int	l_no;		/* line number in file */
};

typedef	struct line	LINE;
LINE		*zero = NULL,		/* first line */
		*dot,		/* current line */
		*dol,		/* last line */
		*contig;	/* where contiguous (non-aged) lines start */
int	        nlall;          /* room for how many LINEs in memory */
 
FILE		*in_file = NULL;	/* current input stream */
FILE		*tmp_fin = NULL;	/* pipe temporary file in */
FILE		*tmp_fou = NULL;	/* pipe temporary file out */

char		*tmp_name;

short		sign;		/* sign of command input */

int             fnum,           /* which file argument we're in */
		pipe_in,	/* set when stdin is a pipe */
		out_is_tty;	/* set if stdout is a tty */

static fpos_t	last_tmp_pos;	/* remember last tmp file position, to reset
				   position when ungetwc */


void	help(),
	doclear(void),
	sopr(void *, int),
	pr( void * ),
	prompt(char *),
	error(char *),
	terminit(void),
	newdol(FILE *),
	compact(void),
	save_pipe(void),
	help(void),
	copy_file(FILE *, FILE *),
	save_input(FILE *),
	lineset();

void	erase_line(int),
	lineset(int),
	kill_line(void);

void	catchtstp(int);
void	re_error(int);

int	getline(FILE*);
int	ttyin(void);
int	number(void);
wint_t	readwch(void);
int	skipf(int);
int	command(char *);
int	screen(char *);
char	*setprompt(char *);

FILE *	checkf(char *);

static int search( char *, int);
int find(int,int);
static wint_t fgetputc(FILE *);

int	set_state(int *, int ,char*);

extern	void saveterm(void),
	     fixterm(void),
	     resetterm(void),
	     setupterm(char *, int, int *),
	     putp(char *);

void		on_brk(int),
		chgwinsz(int);

void		end_it(void);
short		brk_hit;	/* interrupt handling is pending flag */

int             window = 0;	/* window size in lines */
short		win_sz_set = 0; /* window size set by the user */  
short		eof_pause = 1;	/* pause w/ prompt at end of files */
short		soflag = 0;	/* output all messages in standout mode */
short           promptlen;      /* length of the current prompt */
short           firstf = 1;	/* set before first file has been processed */
short           inwait,		/* set while waiting for user input */
		errors;         /* set if error message has been printed.
				 * if so, need to erase it and prompt */

char		**fnames;
short		fflag = 0;	/* set if the f option is used */
short		nflag = 0;	/* set for "no newline" input option */
short		clropt = 0;	/* set if the clear option is used */
int		initopt = 0;	/* set if the line option is used */
int		srchopt = 0;	/* set if the search option is used */
int		initline;
unsigned char	initbuf[BUFSIZ];
char		lastpattern[BUFSIZ];
char            leave_search = 't';     /* where on the page to leave a found string */
short           nfiles;
char		*shell;
char		*promptstr = ":";
char		*setprompt();
short		lenprompt;		/* length of prompt string */
int		nchars;			/* return from getline in find() */
jmp_buf		restore;
char            msgbuf[BUFSIZ];
unsigned char   Line[LINSIZ+2];
int             catch_susp = 0;                 /* has SIGTSTP been caught? */

struct screen_stat {
	int	first_line;
	int	last_line;
	short	is_eof;
	};
struct screen_stat old_ss = { 0, 0, 0 };
struct screen_stat new_ss;

short		eoflag;		/* set whenever at end of current file */
short		doliseof;	/* set when last line of file is known */
int		eofl_no;	/* what the last line of the file is */

#ifdef TTY_PAGE
struct tty_page gpage;
struct tty_page spage= {0,0};
#endif

#ifndef TTY_PAGE
#	define USAGE() { fputs(MSGS(USGE,"Usage: pg [-number] [-p string] [-cefns] [+line] [+/pattern/] files\n"), stderr); exit(1); }	/*MSG*/
#else /* TTY_PAGE */
#	define USAGE() { fprintf(stderr,MSGS(USGE,"Usage: pg [-number] [-p string] [-cefns] [+line] [+/pattern/] files\n")); ioctl(fileno(stdout),TCSLEN, &gpage); exit(1); } /*MSG*/
#endif /* TTY_PAGE */

void
main(int argc, char *argv[])
{
	register char	*s;
	register char	*p;
	register char	ch;
	int		prnames = 0; 

	extern nl_catd catd;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_PG,NL_CAT_LOCALE);

	out_is_tty = isatty(1);  	/* P34397 */ /*A2019*/

#ifdef TTY_PAGE
	if ( out_is_tty ){		/* P34397 */ /*A2019*/
		if(ioctl(fileno(stdout),TCGLEN, &gpage) == -1){
			perror("pg: ");
			exit(2);
		}
		spage.tp_flags &= ~PAGE_MSK;
		spage.tp_flags |= PAGE_OFF;
		if(ioctl(fileno(stdout),TCSLEN, &spage) == -1){
			perror("pg: ");
			exit(2);
		}
	}
#endif
	fnum = 0;
	nfiles = argc;
	fnames = argv;
	while (--nfiles > 0) {
		if ((ch = (*++fnames)[0]) == '-') {
			if (fnames[0][1] == '-' && fnames[0][2] == '\0') {
				nfiles--;
				*fnames++;
				break;
			}
			if (fnames[0][1] == '\0' )
				break;
			for (s = fnames[0]+1; *s != '\0'; s++)
				if (isdigit((int)*s)) {
					window = 0;
					do {
						window = window*10+*s-'0';
					} while (isdigit((int)*++s));
					if (*s != '\0')
						USAGE();
					break;
				}
				else if (*s == 'c')
					clropt = 1;
				else if (*s == 'e')
					eof_pause = 0;
				else if (*s == 'f')
					fflag = 1;
				else if (*s == 'n')
					nflag = 1;
				else if (*s == 's')
					soflag = 1;	/* standout mode */
				else if (*s == 'p') {
					if (*++s != '\0')
						promptstr = setprompt(s);
					else if (nfiles > 1) {
						promptstr = setprompt(*++fnames);
						nfiles--;
					}
					else
						USAGE();
					break;
				}
				else
					USAGE();

		}
		else if (ch == '+') {
			s = *fnames;
			if (*++s == '/') {
				srchopt++;
				initopt = 0;
				for (++s, p=(char *)initbuf; *s!='\0' && *s!='/';)
					if (p < (char *)initbuf + sizeof(initbuf))
						*p++ = *s++;
					else {
						fputs(MSGS(PATLONG,"pg: pattern too long\n"), stderr);	/*MSG*/
#ifdef TTY_PAGE
					     ioctl(fileno(stdout),TCSLEN,&gpage);
#endif
						exit(1);
					}
				*p = '\0';
			}
			else {
				initopt++;
				srchopt = 0;
				for (; isdigit((int)*s); s++)
					initline = initline*10 + *s -'0';
				if (*s != '\0')
					USAGE();
			}
		}
		else
			break;
	}
	signal(SIGQUIT,(void (*)(int))end_it);
	signal(SIGINT,(void (*)(int))end_it);
	/*out_is_tty = isatty(1);  Move to the top! P34397 */
	if (out_is_tty) {
		terminit();
		signal(SIGQUIT,(void (*)(int))on_brk);

		signal(SIGINT,(void (*)(int))on_brk);
	}
	if (window == 0)
		window = lines - 1;
	lenprompt = strlen(promptstr);
	if (window <= 1)
		window = 2;
	if (initline <= 0)
		initline = 1;
	if (nfiles > 1)
		prnames++;

	if (nfiles == 0) {
		fnames[0] = "-";
		nfiles++;
	}
	while (fnum < nfiles) {
		signal(SIGWINCH,(void (*)(int))chgwinsz);
		if (strcmp(fnames[fnum],"") == 0)
			fnames[fnum] = "-";
		if ((in_file = checkf(fnames[fnum])) == NULL)
			fnum++;
		else {
			if (out_is_tty)
				fnum += screen(fnames[fnum]);
			else {
				if (prnames) {
					pr("::::::::::::::\n");
					pr(fnames[fnum]);
					pr("\n::::::::::::::\n");
				}
				copy_file(in_file,stdout);
				fnum++;
			}
			fflush(stdout);
			if (pipe_in)
				save_pipe();
			else
			if (in_file != tmp_fin)
				fclose(in_file);
		}
	}
	end_it();
}

/*
 * NAME: setprompt
 *                                                                    
 * FUNCTION: 	Set the prompt corresponding to the -p option.  %d is
 *		the current page number.
 *                                                                    
 * RETURN VALUE DESCRIPTION:  A pointer to the string prompt is returned.
 *			    
 */  

char *
setprompt(char *s)
{
	register int i = 0;
	register int pct_d = 0;
	static char pstr[PROMPTSIZE];

	while (i < PROMPTSIZE - 2)
		switch(pstr[i++] = *s++) {
		case '\0':
			return(pstr);
		case '%':
			if (*s == 'd' && !pct_d) {
				pct_d++;
			}
			else if (*s != '%')
				pstr[i++] = '%';
			if ((pstr[i++] = *s++) == '\0')
				return(pstr);
			break;
		default:
			break;
		}
	fputs(MSGS(PRMPTLNG,"pg: prompt too long\n"), stderr);
#ifdef TTY_PAGE
     	ioctl(fileno(stdout),TCSLEN,&gpage);
#endif
	exit(1);
	/* NOTREACHED */
	return (NULL);
}


/*
 * NAME: screen
 *                                                                    
 * FUNCTION: Print out the contents of the file f, one screenful at a time.
 */  

int
screen(char *file_name)
{
	int cmd_ret = 0;
	int start;
	short hadchance = 0;

	old_ss.is_eof = 0;
	old_ss.first_line = 0;
	old_ss.last_line = 0;
	new_ss = old_ss;

	signal(SIGWINCH,(void (*)(int))chgwinsz);
	if (!firstf)
		cmd_ret = command(file_name);
	else {
		firstf = 0;
		if (initopt) {
			initopt = 0;
			new_ss.first_line = initline;
			new_ss.last_line = initline + window - 1;
		}
		else
		if (srchopt) {
			srchopt = 0;
			if (!search((char *)initbuf,1))
				cmd_ret = command(file_name);
		}
		else {
			new_ss.first_line = 1;
			new_ss.last_line = window;
		}
	}

	for (;;) {
	 	signal(SIGWINCH,(void (*)(int))chgwinsz);
		if (cmd_ret)
			return(cmd_ret);
		if (hadchance && new_ss.first_line >= eofl_no - 1)
			return(1);
		hadchance = 0;

		if (new_ss.last_line < window)
			new_ss.last_line = window;
		if (find(0,new_ss.last_line + 1) != EOF)
			new_ss.is_eof = 0;
		else {
			new_ss.is_eof = 1;
			new_ss.last_line = eofl_no - 1;
			new_ss.first_line = new_ss.last_line - window + 1;
		}

		if (new_ss.first_line < 1)
			new_ss.first_line = 1;
		if (clropt) {
			doclear();
			start = new_ss.first_line;
		}
		else {
			if (new_ss.first_line == old_ss.last_line)
				start = new_ss.first_line + 1;
			else
			if (new_ss.first_line > old_ss.last_line)
				start = new_ss.first_line;
			else
			if (old_ss.first_line < new_ss.first_line)
				start = old_ss.last_line + 1;
			else
				start = new_ss.first_line;

			if (start < old_ss.first_line)
				sopr(MSGS(SKIPBW,"...skipping backward\n"),0);
			else
			if (start > old_ss.last_line + 1)
				sopr(MSGS(SKIPFW,"...skipping forward\n"),0);
		}

		for(; start <= new_ss.last_line; start++) {
			find(0,start);
			pr(Line);
			if (brk_hit) {
				new_ss.last_line = find(1,0);
				new_ss.is_eof = 0;
				break;
			}
		}

		brk_hit = 0;
		fflush(stdout);
		if (new_ss.is_eof) {
			if (!eof_pause || eofl_no == 1)
				return(1);
			hadchance++;
			error(MSGS(SAYEOF, "(EOF)"));
		}
		old_ss = new_ss;
		cmd_ret = command((char *)NULL);
	}
}

char	cmdbuf[LINSIZ], *cmdptr;
#define BEEP()		if (bell) { putp(bell); fflush(stdout); }
#define	BLANKS(p)	while (*p == ' ' || *p == '\t') p++
#define CHECKEND()	BLANKS(cmdptr); if (*cmdptr) { BEEP(); break; }

/*
 * NAME: command
 *                                                                    
 * FUNCTION: 
 * 	Read a command and do it. A command consists of an optional integer
 * 	argument followed by the command character.
 
 *                                                                    
 * RETURN VALUE: 
 *		Return the number of files to skip, 0 if
 *		we're still talking about the same file.
 */  
int
command (char *filename)
{
	register int nlines;
	register char c;
	char *cmdend;
	int id;
	int skip;

	for (;;) {
		/* Wait for output to drain before going on.     */
		/* This is done so that the user will not hit    */
		/* break and quit before he has seen the prompt. */
		ioctl(1,TCSBRK,1); 
		if (setjmp(restore) != 0)
			end_it();
		inwait = 1;
		if (brk_hit) {
			brk_hit = 0;
			errors  = 0;
		}
		if (errors)
			errors = 0;
		else {
			promptlen++;
			kill_line();
			prompt(filename);
		}
	 	signal(SIGWINCH,(void (*)(int))chgwinsz);
		fflush(stdout);
		if (ttyin())
			continue;
		cmdptr = cmdbuf;
	 	signal(SIGWINCH,(void (*)(int))chgwinsz);
		nlines = number();
		BLANKS(cmdptr);
		switch (*cmdptr++) {
		case 'h':
			CHECKEND();
			help();
			break;
		case '\014': /* ^L */
		case '.':       /* redisplay current window */
			CHECKEND();
			new_ss.first_line = old_ss.first_line;
			new_ss.last_line = new_ss.first_line + window - 1;
			inwait = 0;
			return(0);
		case 'w':       /* set window size */
		case 'z':
			if (sign == -1) {
				BEEP();
				break;
			}
			if (!win_sz_set)
				win_sz_set++;
			CHECKEND();
			if (nlines == 0)
				nlines = window;
			else 
			if (nlines > 1)
				window = nlines;
			else {
				BEEP();
				break;
			}
			new_ss.first_line = old_ss.last_line;
			new_ss.last_line = new_ss.first_line + window - 1;
			inwait = 0;
			return(0);
		case '\004': /* ^D */
		case 'd':
			CHECKEND();
			if (sign == 0)
				sign = 1;
			new_ss.last_line = old_ss.last_line + sign*window/2;
			new_ss.first_line = new_ss.last_line - window + 1;
			inwait = 0;
			return(0);
		case 's':
			/*
			* save input in filename.
			* Check for filename, access, etc.
			*/
			{
			FILE * volatile sf = NULL;

			BLANKS(cmdptr);
			if (!*cmdptr) {
				BEEP();
				break;
			}
			if (setjmp(restore) != 0) {
				BEEP();
			}
			else {

				if ((sf=fopen(cmdptr,"w")) == NULL) {
					error(MSGS(OPSAVERR,"cannot open save file"));	/*MSG*/
					break;
				}
				kill_line();
				sprintf(msgbuf, MSGS(SAVFIL,"saving file %s"),
					cmdptr);
				sopr(msgbuf,1);
				fflush(stdout);
				save_input(sf);
				error(MSGS(SAVED,"saved"));		/*MSG*/
				fclose(sf);
				break;
			}
			if (sf) fclose(sf);	/* Close it if it got opened */
			break;
		        }
		case 'q':
		case 'Q':
			CHECKEND();
			inwait = 0;
			end_it();
		case 'f':       /* skip forward screenfuls */
			CHECKEND();
			if (sign == 0)
				sign++;	/* skips are always relative */
			if (nlines == 0)
				nlines++;
			nlines = nlines * (window - 1);
			if (sign == 1)
				new_ss.first_line = old_ss.last_line + nlines;
			else
				new_ss.first_line = old_ss.first_line - nlines;
			new_ss.last_line = new_ss.first_line + window - 1;
			inwait = 0;
			return(0);
		case 'l':       /* get a line */
			CHECKEND();
			if (nlines == 0) {
				nlines++;
				if (sign == 0)
					sign = 1;
			}
			switch(sign){
			case 1:
				new_ss.last_line = old_ss.last_line + nlines;
				new_ss.first_line = new_ss.last_line - window + 1;
				break;
			case 0:  /* leave addressed line at top */
				new_ss.first_line = nlines;
				new_ss.last_line = nlines + window - 1;
				break;
			case -1:
				new_ss.first_line = old_ss.first_line - nlines;
				new_ss.last_line = new_ss.first_line + window - 1;
				break;
			}
			inwait = 0;
			return(0);
		case '\0': /* \n or blank */
			if (nlines == 0){
				nlines++;
				if (sign == 0)
					sign = 1;
			}
			nlines = (nlines - 1) * (window - 1);
			switch(sign) {
			case 1:
				new_ss.first_line = old_ss.last_line + nlines;
				new_ss.last_line = new_ss.first_line + window - 1;
				break;
			case 0:
				new_ss.first_line = nlines + 1;
				new_ss.last_line = nlines + window;
				break;
			case -1:
				new_ss.last_line = old_ss.first_line - nlines;
				new_ss.first_line = new_ss.last_line - window + 1;
				break;
			}
			inwait = 0;
			return(0);
		case 'n':       /* switch to next file in arglist */
			CHECKEND();
			if (sign == 0)
				sign = 1;
			if (nlines == 0)
				nlines++;
			if ((skip = skipf(sign *nlines)) == 0) {
				BEEP();
				break;
			}
			inwait = 0;
			return(skip);
		case 'p':       /* switch to previous file in arglist */
			CHECKEND();
			if (sign == 0)
				sign = 1;
			if (nlines == 0)
				nlines++;
			if ((skip = skipf(-sign * nlines)) == 0) {
				BEEP();
				break;
			}
			inwait = 0;
			return(skip);
		case '$':       /* go to end of file */
			CHECKEND();
			sign = 1;
			while(find(1,10000) != EOF)
				/* any large number will do */;
			new_ss.last_line = eofl_no - 1;
			new_ss.first_line = eofl_no - window;
			inwait = 0;
			return(0);
		case '/':       /* search forward for r.e. */
		case '?':       /*   "  backwards */
		case '^':	/* this ones a ? for regent100s */
			if(sign < 0) {
				BEEP();
				break;
			}
			if (nlines == 0)
				nlines++;
			cmdptr--;
			cmdend = cmdptr + (strlen(cmdptr) - 1);
			if ( (cmdend > cmdptr + 1)
				&& (*cmdptr ==  *(cmdend - 1))
				&& ( ((c = *cmdend) == 't')
					|| (c == 'm')
					|| (c == 'b') ) ) {
				leave_search = c;
				cmdend--;
			}
			if ((cmdptr < cmdend) && (*cmdptr == *cmdend))
				*cmdend = '\0';
			if (*cmdptr != '/')  /* signify back search by - */
				nlines = -nlines;
			if (!search(++cmdptr, nlines))
				break;
			else {
				inwait = 0;
				return(0);
			}
		case '!':       /* shell escape */
			if (!hard_copy) { /* redisplay the command */
				pr(cmdbuf);
				pr("\n");
			}
			if ((id = fork ()) < 0) {
				error(MSGS(CANTFORK,"cannot fork, try again later"));
				break;
			}
			if (id == 0) {
				/*
				* if stdin is a pipe, need to close it so
				* that the terminal is really stdin for
				* the command
				*/
                                if (catch_susp)
                                        signal(SIGTSTP, SIG_DFL);
				fclose(stdin);
				dup(fileno(stdout));
				execl(shell, shell, "-c", cmdptr, 0);
				fputs(MSGS(EXECFAIL,"exec failed\n"), stderr);	/*MSG*/
#ifdef TTY_PAGE
     				ioctl(fileno(stdout),TCSLEN,&gpage);
#endif
				exit(1);
			}
			signal (SIGINT, SIG_IGN);
			signal (SIGQUIT, SIG_IGN);
			wait ((int *) 0);
			pr("!\n");
			fflush(stdout);
			signal(SIGINT,(void (*)(int))on_brk);
			signal(SIGQUIT,(void (*)(int))on_brk);
			break;
		default:
			BEEP();
			break;
		}
	}
}

/*
 * NAME: number
 *                                                                    
 * FUNCTION:	return the number in the command line and the option plus
 *		or minus value.
 *                                                                    
 * RETURN VALUE:  number of lines.
 */  
int
number(void)
{
	register int i;
	register char *p;

	i = 0;
	sign = 0;
	p = cmdptr;
	BLANKS(p);
	if (*p == '+') {
		p++;
		sign = 1;
	}
	else
	if (*p == '-') {
		p++;
		sign = -1;
	}
	while (isdigit((int)*p))
		i = i * 10 + *p++ - '0';
	cmdptr = p;
	return(i);
}

/*
 * NAME: ttyin
 *                                                                    
 * FUNCTION:  read a line of input.
 */  
int
ttyin (void)
{
	register char *sptr;
	register char *ptr;
	register wint_t wch;
	register int slash = 0;
	wchar_t	wptr;
	int state = 0, cnt;
	int i, k, l;

	fixterm();
	sptr = cmdbuf;
	set_state(&state,' ', sptr);	/* initialize state processing */

	while(state != 10) {
		wch = readwch();
		if (wch == '\n' && !slash)
			break;
		if (wch == erasechar() && !slash) {
			if (sptr > cmdbuf) {
				ptr = sptr;
				for (i=0;i<MB_CUR_MAX;i++) {
					ptr--;
					if ((k=mbtowc(&wptr,ptr,MB_CUR_MAX))>0)
						break;
				}
				/* if mbtowc() somehow failed, degrade to
				   single-byte mode */
				if (k <= 0) { 	
					k = 1;
					l = 1;
				} 
				else {
					if (iswcntrl(wptr) == 0)
						l = wcwidth(wptr);
					else
						/* assuming cntrl code occupies
						   two screen columns */
						l = 2;
				}
				for (i=0;i<l;i++) {
					promptlen--;
					pr("\b \b");
				}
				sptr = sptr-k;
			}
			set_state(&state,wch,sptr);
			fflush(stdout);
			continue;
		}
		else
		if (wch == killchar() && !slash) {
			if (hard_copy)
				putchar(wch);
			resetterm();
			return(1);
		}
		if (slash) {
			slash = 0;
			pr("\b \b");
			sptr--;
			promptlen--;
		}
		else /* is there room to keep this character? */
		if (sptr>=cmdbuf + sizeof(cmdbuf)) {
			BEEP();
			continue;
		}
		else
		if (wch == '\\')
			slash++;
		if (set_state(&state,wch,sptr) == 0) {
			BEEP();
			continue;
		}
		cnt = wctomb(sptr, wch);
		sptr += cnt;
		if (wch < ' ') {
			wch += 0100;
			putchar('^');
			putchar(wch);
			promptlen += 2;
		}
		else {
			putwchar(wch);
			promptlen += wcwidth(wch);
		}
		fflush(stdout);
	}

	*sptr = '\0';
	kill_line();
	fflush(stdout);
	resetterm();
	return(0);
}

/*
 * NAME: set_state
 *                                                                    
 * FUNCTION:  	Set the state of the command line.  Whether incoming command
 *		is a positive number, continuation....
 *                                                                    
 * RETURN VALUE:  return 1 if no errors
 */  

int
set_state(int *pstate, int c, char *pc)
{
	static char *psign;
	static char *pnumber;
	static char *pcommand;
	static int slash;

	if (*pstate == 0) {
		psign = NULL;
		pnumber = NULL;
		pcommand = NULL;
		*pstate = 1;
		slash = 0;
		return(1);
	}
	if (c == '\\' && !slash) {
		slash++;
		return(1);
	}
	if (c == erasechar() && !slash)
		switch(*pstate) {
		case 4:
			if (pc > pcommand)
				return(1);
			pcommand = NULL;
		case 3:
			if (pnumber && pc > pnumber) {
				*pstate = 3;
				return(1);
			}
			pnumber = NULL;
		case 2:
			if (psign && pc > psign) {
				*pstate = 2;
				return(1);
			}
			psign = NULL;
		case 1:
			*pstate = 1;
			return(1);
		}

	slash = 0;
	switch(*pstate) {
	case 1: /* before receiving anything interesting */
		if (c == '\t' || (!nflag && c == ' '))
			return(1);
		if (c == '+' || c == '-') {
			psign = pc;
			*pstate = 2;
			return(1);
		}
	case 2: /* received sign, waiting for digit */
		if (isdigit(c)) {
			pnumber = pc;
			*pstate = 3;
			return(1);
		}
	case 3: /* received digit, waiting for the rest of the number */
		if (isdigit(c))
			return(1);
		if (strchr("h\014.wz\004dqQfl np$",c)) {
			pcommand = pc;
			if (nflag)
				*pstate = 10;
			else
				*pstate = 4;
			return(1);
		}
		if (strchr("s/^?!",c)) {
			pcommand = pc;
			*pstate = 4;
			return(1);
		}
		return(0);
	case 4:
		return(1);
	}
	/*
	 * Don't expect to ever reach this, but just in case....
	 */
	return (0);
}

/*
 * NAME: readwch
 *                                                                    
 * FUNCTION:  Input a wide character.
 *
 * RETURN VALUE:  wide character
 */  

wint_t
readwch(void)
{
	wchar_t wch;
        static int wasintrd = 0;
	int i, cnt;
	char mbbuf[MB_LEN_MAX];

        /*
         * if we were interrupted before, force a newline...
         */
        if (wasintrd) {
                wasintrd = 0;
                return ('\n');
                }

        /*
         * if we're returning from an interrupt (either SIGTSTP or
         * SIGWINCH), force a redraw...
         * this becomes tough in the !nflag case, 'cause we also
         * have to force a newline (see above).
         * yeah, I know it's strange...
         */
trynext:
	for(i = 0; i < MB_CUR_MAX; i++)
		mbbuf[i] = 0;
	for(i = 0; i < MB_CUR_MAX; i++){
		if(read(fileno(stdout), mbbuf+i, 1) == EOF){
			wch = WEOF;
			break;
		}
		if(cnt=(mbtowc(&wch, mbbuf, MB_CUR_MAX)) > 0)
			break;
	}
	if(cnt == -1 && wch != WEOF)
		goto trynext; 		/* invalid mb found */
        if (wch == WEOF && errno == EINTR) {
                if (!nflag)
                        wasintrd = 1;
                wch = '\014';
	}
	return (wch);
}

/*
 * NAME: help
 *                                                                    
 * FUNCTION: Print out a help screen.
 *                                                                    
 * RETURN VALUE: void
 */  

void
help(void)
{
	if (clropt)
		doclear();

     pr(MSGS(H01,"-------------------------------------------------------\n"));
     pr(MSGS(H02,"   h               help\n"));
     pr(MSGS(H03,"   q or Q          quit\n"));
     pr(MSGS(H04,"   <blank> or \\n   next page\n"));
     pr(MSGS(H05,"   l               next line\n"));
     pr(MSGS(H06,"   d or ^D         display half a page more\n"));
     pr(MSGS(H07,"   . or ^L         redisplay current page\n"));
     pr(MSGS(H08,"   f               skip the next page forward\n"));
     pr(MSGS(H09,"   n               next file\n"));
     pr(MSGS(H11,"   p               previous file\n"));
     pr(MSGS(H12,"   $               last page\n"));
     pr(MSGS(H13,"   w or z          set window size and display next page\n"));
     pr(MSGS(H14,"   s savefile      save current file in savefile\n"));
     pr(MSGS(H15,"   /pattern/       search forward for pattern\n"));
     pr(MSGS(H16,"   ?pattern? or\n"));
     pr(MSGS(H17,"   ^pattern^       search backward for pattern\n"));
     pr(MSGS(H18,"   !command         execute command\n"));
     pr(MSGS(H19,"\n"));
     pr(MSGS(H20,"Most commands can be preceeded by a number, as in:\n"));
     pr(MSGS(H21,"+1\\n (next page); -1\\n (previous page); 1\\n (page 1).\n"));
     pr(MSGS(H22,"\n"));
     pr(MSGS(H23,"See the manual page for more detail.\n"));
     pr(MSGS(H24,"-------------------------------------------------------\n"));
}

/*
 * NAME: nskip
 *                                                                    
 * FUNCTION: 
 * 		Skip nskip files in the file list (from the command line).
 *		Nskip may be negative.
 *
 * RETURN VALUE:  The number of files skipped.
 */  
int
skipf (int nskip)
{
	if (fnum + nskip < 0) {
		nskip = -fnum;
		if (nskip == 0)
			error(MSGS(NOPREV,"No previous file"));		/*MSG*/
	}
	else
	if (fnum + nskip > nfiles - 1) {
		nskip = (nfiles - 1) - fnum;
		if (nskip == 0)
			error(MSGS(NONEXT,"No next file"));		/*MSG*/
	}
	return(nskip);
}

/*
 * NAME: checkf
 *                                                                    
 * FUNCTION: 
 * 	Check whether the file named by fs is a file which the user may
 * 	access.  
 *
 * RETURN VALUE: 	If it is, return the opened file if it is.
 *			Otherwise return NULL. 
 */  


FILE *
checkf (char *fs)
{
	struct stat stbuf;
	register FILE *f;
	int fd;

	pipe_in = 0;
	if (strcmp(fs,"-") == 0) {
		if (tmp_fin == NULL)
			f = stdin;
		else {
			rewind(tmp_fin);
			f = tmp_fin;
		}
	}
	else {
		if ((f=fopen(fs, "r")) == NULL) {
			fflush(stdout);
			perror(fs);
			return (NULL);
		}
	}
	if (fstat((int)fileno(f), &stbuf) == -1) {
		fflush(stdout);
		perror(fs);
		return (NULL);
	}
	if (stbuf.st_mode & S_IFDIR) {
		fprintf(stderr, MSGS(DIRECT,"pg: %s is a directory\n"),fs);
		return (NULL);
	}
	if (stbuf.st_mode & S_IFREG) {
		if (f == stdin)		/* It may have been read from */
			rewind(f);	/* already, and not reopened  */
	}
	else {
		if (f != stdin) {
			fputs(MSGS(SPECFIL,"pg: special files only handled as standard input\n"), stderr);
			return(NULL);
		}
		else {
		        tmp_name = tempnam("/usr/tmp","pg");
			if ((fd=creat(tmp_name,0600)) < 0) {
			    fputs(MSGS(NOTEMP,"pg: Can't create temp file\n"), stderr);	/*MSG*/
			    return(NULL);
			}			
			close(fd);
			if ((tmp_fou = fopen(tmp_name, "w")) == NULL) {
				fputs(MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"), stderr);	/*MSG*/
				return(NULL);
			}
			if ((tmp_fin = fopen(tmp_name, "r")) == NULL) {
			;	fputs(MSGS(TEMPERRR,"pg: Can't get temp file for reading\n"), stderr);	/*MSG*/
				return(NULL);
			}
			pipe_in = 1;
		}
	}
	lineset(BOF);
	return(f);
}

/*
 * NAME: copy_file
 *                                                                    
 * FUNCTION:  Copy the file to the output file so we can scan back and forth.
 *                                                                    
 * RETURN VALUE:  void
 */  

void
copy_file(FILE *f, FILE *out)
{
	register int c;

	while ((c = getc(f)) != EOF)
		putc(c,out);
}

#include <regex.h>
void
re_error(int i)
{
	int j;
	static struct messages {
		char *message;
		int number;
		} re_errmsg[] = {
		"Pattern not found",				1,
		"Range endpoint too large",			11,
		"Bad number",					16,
		"`\\digit' out of range",			25,
		"Illegal or missing delimeter",			36,
		"No remembered search string",  		41,
		"\\( \\) imbalance",				42,
		"Too many \\(",					43,
		"More than two numbers given in \\{ \\}",	44,
		"} expected after \\",				45,
		"First number exceeds second in \\{ \\}",	46,
		"Invalid endpoint in range",			48,
		"[] imbalance",					49,
		"Regular expression overflow",			50,
		"Bad regular expression",		 	0
		};

	for (j = 0; re_errmsg[j].number != 0; j++ )
		if (re_errmsg[j].number == i )
			break;
	if (re_errmsg[j].number == 0)
		error(MSGS(BADREG,"Bad Regular Expression"));
	else
		error(catgets(catd,MS_PG,i,re_errmsg[j].message));
	longjmp(restore,1);  /* restore to search() */
}

/*
 * NAME: search
 *                                                                    
 * FUNCTION: 
 * Search for nth ocurrence of regular expression contained in buf in the file
 *	negative n implies backward search
 *	n 'guaranteed' non-zero
 *                                                                    
 * RETURN VALUE: 1 if pattern found else 0
 */  

regex_t re;

static int
search (char *buf, int n)
{
	register int direction;
	unsigned char *endbuf;
	int END_COND;
        int stat;
	int save_newline;

	if (buf == NULL || *buf == '\0') {
		buf = lastpattern;
	} else {
		strncpy(lastpattern, buf, BUFSIZ);
	}
	endbuf = (unsigned char *)buf + strlen(buf) - 1;
	save_newline = (*endbuf == '$');
	if (setjmp(restore) == 0) {
                if ( ( stat = regcomp( &re, buf, 0) ) != 0) {
                        perror("pg: regcomp");
                        }

		if (n < 0) {	/* search back */
			direction = -1;
			find(0,old_ss.first_line);
			END_COND = BOF;
		}
		else {
			direction = 1;
			find(0,old_ss.last_line);
			END_COND = EOF;
		}

		endbuf = NULL;	/* Reset endbuf pointer */
		while (find(1,direction) != END_COND){
			if (brk_hit)
				break;
			/*
			 * Clear newline before passing it to regexec() and
			 * restore it afterward.
			 */
			if (save_newline) {
				endbuf = Line + strlen((char *)Line) - 1 ;
				if (*endbuf == '\n')
					*endbuf = '\0';
				else
					endbuf = NULL;
			}
                        stat = regexec(&re, (const char *)Line, (size_t) 0, (regmatch_t *) NULL, 0);
			if (endbuf != NULL)
				*endbuf = '\n' ;	/* Restore newline */
                        if (stat == 0)
				if ((n -= direction) == 0) {
					switch(leave_search) {
					case 't':
						new_ss.first_line = find(1,0);
						new_ss.last_line = new_ss.first_line + window - 1;
						break;
					case 'b':
						new_ss.last_line = find(1,0);
						new_ss.first_line = new_ss.last_line - window + 1;
						break;
					case 'm':
						new_ss.first_line =
find(1,0) - (window - 1)/2;
						new_ss.last_line = new_ss.first_line + window - 1;
						break;
					}
					return(1);
				}
		}
		re_error(1); /* Pattern not found */
	}
	BEEP();
	return(0);
}

/*
 *	find -- find line in file f, subject to certain constraints.
 *
 *	This is the reason for all the funny stuff with sign and nlines.
 *	We need to be able to differentiate between relative and abosolute
 *	address specifications. 
 *
 *	So...there are basically three cases that this routine
 *	handles. Either line is zero, which  means there is to be
 *	no motion (because line numbers start at one), or
 *	'how' and 'line' specify a number, or line itself is negative,
 *	which is the same as having how == -1 and line == abs(line).
 *
 *	Then, figure where exactly it is that we are going (an absolute
 *	line number). Find out if it is within what we have read,
 *	if so, go there without further ado. Otherwise, do some
 *	magic to get there, saving all the intervening lines,
 *	in case the user wants to see them some time later.
 *
 *	In any case, return the line number that we end up at. 
 *	(This is used by search() and screen()). If we go past EOF,
 *	return EOF.
 *	This EOF will go away eventually, as pg is expanded to
 *	handle multiple files as one huge one. Then EOF will
 *	mean we have run off the file list.
 *	If the requested line number is too far back, return BOF.
 */
int
find(int how, int line)	/* find the line and seek there */
{
	/* no compacted memory yet */
	register FILE *f = in_file;
 	register int where;

	if (how == 0)
		where = line;
	else
		if (dot == zero - 1)
			where = how * line;
		else
			where = how * line + dot->l_no;

	/* now, where is either at, before, or after dol */
	/* most likely case is after, so do it first */

	eoflag = 0;
	if (where >= dol->l_no) {
		if (doliseof) {
			dot = dol;
			eoflag++;
			return(EOF);
		}
		if (pipe_in)
			in_file = f = stdin;
		else
			fseek(f, dol->l_addr, 0);
		dot = dol - 1;
		while ((nchars = getline(f)) != EOF) {
			dot++;
			newdol(f);
			if ( where == dot->l_no || brk_hit)
				break;
		}
		if (nchars != EOF)
			return(dot->l_no);
		else { /* EOF */
			dot = dol;
			eoflag++;
			doliseof++;
			eofl_no = dol->l_no;
			return(EOF);
		}
	}
	else { /* where < dol->l_no */
		if (pipe_in) {
			if (fflush(tmp_fou) == EOF) {
				fputs(MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"),stderr);
				end_it();
			}
			in_file = f = tmp_fin;
		}
		if (where < zero->l_no){
			fseek(f, zero->l_addr, 0);
			dot = zero - 1;
			return(BOF);
		}
		else {
			dot = zero + where - 1;
			fseek(f, dot->l_addr, 0);
			nchars = getline(f);
			return(dot->l_no);
		}
	}
}

/*
 * NAME: getline
 *                                                                    
 * FUNCTION: 
 * 		Get a logical line
 *                                                                    
 * RETURN VALUE:  return the column number in which this line is read.
 */  

int
getline(FILE *f)
{
	wint_t	c;
	unsigned char	*p;
	int	column;
	int	i, wrapped = 0;
	int	cnt;
	wint_t (*rdchar)(FILE *);
	
	if (pipe_in && f == stdin)
		rdchar = fgetputc;
	else
		rdchar = fgetwc;

	for (i = 1, column=0, p=Line; i < LINSIZ-1; i++, p+=cnt) {
		c = (*rdchar)(f);
		if((c == '\t' && 1 + (column | 7) > columns ||
		   column + wcwidth(c) > columns) && !fflag){
			ungetwc(c,f);
			if(rdchar == fgetputc) {
				/* avoid duplicated put to tmp file */
				fsetpos(tmp_fou, &last_tmp_pos);
			}
			wrapped++;
			break;
		}
		cnt = wctomb((char *)p, c);
		switch(c) {
		case WEOF:

			/* Detect and report read errors.
			 */
			if (ferror(f)) {
				fflush(stdout);
				perror(0);
			}
			clearerr(f);
			if (p > Line) {	/* last line doesn't have '\n', */
				*p++ = '\n';
				*p = '\0';	/* print it any way */
				return(column);
			}
			return(EOF);
		case '\n':
			break;
		case '\t': /* just a guess */
			column = 1 + (column | 7);
			break;
		case '\b':
			if (column > 0)
				column--;
			break;
		case '\r':
			column = 0;
			break;
		default:
			if (c >= ' ')
				column += wcwidth(c);
			break;
		}
		if (c == '\n') {
			p++;
			break;
		}
	}
	if (c != '\n') { /* We're stopping in the middle of the line */
		if (wrapped || !auto_right_margin)
			*p++ = '\n';	/* for the display */
		/* peek at the next character */
		c = fgetwc(f);
		if (c == '\n') {
			ungetwc(c,f);
			c = (*rdchar)(f); /* gobble and copy it */
		}
		else
		if (c == WEOF) /* get it next time */
			clearerr(f);
		else
			ungetwc(c,f);
	}
	*p = 0;
	wrapped = 0;
	return(column);
}

/*
 * NAME: save_input
 *                                                                    
 * FUNCTION:  	Copy a file, if it is a real file lseek to the begining.
 * 		if output is from a pipe, then start reading from there.
 *
 * RETURN VALUE:  void
 */  

void
save_input(FILE *f)
{
	if (pipe_in) {
		save_pipe();
		in_file = tmp_fin;
		pipe_in = 0;
	}
	fseek(in_file,0L,0);
	copy_file(in_file,f);
}

/*
 * NAME: save_pipe
 *                                                                    
 * FUNCTION: try to save the output from a pipe.
 *                                                                    
 * RETURN VALUE: void
 */  

void
save_pipe(void)
{
	if (!doliseof)
		while (fgetputc(stdin) != WEOF)
			if (brk_hit) {
				brk_hit = 0;
				error(MSGS(PIPSAV,"Piped input only partially saved"));	/*MSG*/
				break;
			}
	if (fclose(tmp_fou) == EOF) {
		fputs(MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"), stderr);	/*MSG*/
		end_it();
	}
}

/*
 * NAME: fgetputc
 *                                                                    
 * FUNCTION:	copy anything read from a pipe to tmp_fou 
 *                                                                    
 * RETURN VALUE: The character read in is returned.
 */  

static wint_t
fgetputc(FILE *f)
{
	register wint_t c;
	fgetpos(tmp_fou, &last_tmp_pos);
	if ((c = getwc(f)) != WEOF)
		if (putwc(c,tmp_fou) == WEOF) {
			fputs(MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"),stderr);
			end_it();
		}
	return(c);
}

/*
 * NAME: lineset
 *                                                                    
 * FUNCTION: initialize line memory
 *
 * RETURN VALUE: void
 */  

void
lineset(int how)	
{
	if (zero == NULL) {
		nlall = 128;
		zero = (LINE *) malloc((size_t)(nlall*sizeof(LINE)));
		if (zero == NULL)
		{
			fputs( "malloc failed\n", stderr);
			exit (-1);
		}
			
	}
	dol = contig = zero;
	zero->l_no = 1;
	zero->l_addr = 0l;
	if (how == BOF) {
		dot = zero - 1;
		eoflag = 0;
		doliseof = 0;
		eofl_no = -1;
	}
	else {
		dot = dol;
		eoflag = 1;
		doliseof = 1;
		eofl_no = 1;
	}
}

/*
 * NAME: newdol
 *                                                                    
 * FUNCTION: 	Add address of new 'dol'
 *		assumes that f is currently at beginning of said line
 *		updates dol
 *
 * RETURN VALUE: 
 */  

void
newdol(FILE *f)
{
	register int diff;

	if ((dol - zero) + 1 >= nlall){
		LINE *ozero = zero;

		nlall += 512;
		if ((zero = (LINE *) realloc ((void *) zero,
		     (size_t)(nlall * sizeof(LINE)))) == NULL){
			zero = ozero;
			compact();
		}
		diff = (char *)zero - (char *)ozero;
		dot = (LINE *)((char *)dot + diff);
		dol = (LINE *)((char *)dol + diff);
		contig = (LINE *)((char *)contig + diff);
	}
	dol++;
	if (!pipe_in)
		dol->l_addr = ftell(f);
	else {
		if (fflush(tmp_fou) == EOF) {
			fputs(MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"), stderr);	/*MSG*/
			end_it();
		}
		dol->l_addr = ftell(tmp_fou);
	}
	dol->l_no = (dol-1)->l_no + 1;
}

void
compact(void)
{
	fprintf(stderr, MSGS(MEMOUT,"pg: no more memory - line %d\n"),dol->l_no);	/*MSG*/
	end_it();
}

/*
 * NAME: terminit
 *                                                                    
 * FUNCTION: Set up terminal dependencies from termlib 
 */  

void
terminit(void)
{
	int err_ret;
        struct termios ntty;
        FILE *fp;


        if ((fp = fopen("/dev/tty","r+")) != NULL) {
                fclose(fp);
                if ((freopen("/dev/tty","r+",stdout)) == NULL) {
                        fputs(MSGS(NOREOPN,"pg: cannot reopen stdout\n"),stderr);
#ifdef TTY_PAGE
     		ioctl((int)fileno(stdout),TCSLEN,&gpage);
#endif
		exit(1);
                        }
                }
        setupterm(0,fileno(stdout),&err_ret);
	if (err_ret != 1)
                setupterm("dumb",fileno(stdout),&err_ret);
	if (err_ret != 1) {
		fputs(MSGS(TERMTYP,"pg: cannot find terminal type\n"), stderr);/*MSG*/
#ifdef TTY_PAGE
		ioctl(fileno(stdout),TCSLEN,&gpage);
#endif
		exit(1);
	}

	/* there must be a better way using "curses" */
	tcgetattr(fileno(stdout),&ntty);
	ntty.c_iflag |= ICRNL;
	ntty.c_lflag &= ~(ECHONL | ECHO | ICANON);
	ntty.c_cc[VMIN] = 1;
	ntty.c_cc[VTIME] = 1;
	tcsetattr(fileno(stdout),TCSANOW,&ntty);
        /*
         * catch SIGTSTP
         */
        if (signal(SIGTSTP, SIG_IGN) == SIG_DFL) {
                signal(SIGTSTP, catchtstp);
                catch_susp++;
        }
	saveterm();
	resetterm();
	if (lines <= 0 || hard_copy) {
		hard_copy = 1;
		lines = 24;
	}
	if (columns <= 0)
		columns = 80;
	if (clropt && !clear_screen)
		clropt = 0;
	if ((shell = getenv("SHELL")) == NULL)
			shell = _PATH_SH;
}
void
error(char *mess)
{
	kill_line();
	sopr(mess,1);
	prompt((char *) NULL);
	errors++;
}

/*
 * NAME: prompt
 *                                                                    
 * FUNCTION: 
 */  
void
prompt(char *filename)
{
	char outstr[PROMPTSIZE+6];
	int pagenum;

	if (filename != NULL) {
		sprintf(msgbuf, MSGS(NXTFIL, "(Next file: %s)"), filename);
		sopr(msgbuf,1);   /*MSG*/
	}
	else {
		if ((pagenum=(int)((new_ss.last_line-2)/(window-1)+1))
						> 999999)
			pagenum = 999999;
		sprintf(outstr,promptstr,pagenum);
		sopr(outstr,1);
	}
	fflush(stdout);
}

/*
 * NAME: sopr
 *                                                                    
 * FUNCTION:
 *  sopr puts out the message (please no \n's) surrounded by standout
 *  begins and ends
 *                                                                    
 * RETURN VALUE: none
 */  
void
sopr(void *m, int count)
{
	if (count)
		promptlen += strlen(m);
	if (soflag && enter_standout_mode && exit_standout_mode) {
		putp(enter_standout_mode);
		pr(m);
		putp(exit_standout_mode);
	}
	else
		pr(m);
}

void
pr(void *s)
{
	fputs((char *)s,stdout);
}

void
doclear(void)
{
	if (clear_screen)
		putp(clear_screen);
	putchar('\r');  /* this resets the terminal drivers character */
			/* count in case it is trying to expand tabs  */
}
void
kill_line(void)
{
	erase_line(0);
	if (!clr_eol) putchar ('\r');
}

/*
 * NAME: erase_line
 *                                                                    
 * FUNCTION: 	
 * 		Erase from after col to end of prompt
 */  
void
erase_line(register int col)
{

	if (promptlen == 0)
		return;
	if (hard_copy)
		putchar('\n');
	else {
		if (col == 0)
			putchar('\r');
		if (clr_eol) {
			putp(clr_eol);
			putchar('\r');  /* for the terminal driver again */
		}
		else
			for (col = promptlen - col; col > 0; col--)
				putchar (' ');
	}
	promptlen = 0;
}

/*
 * NAME: on_brk
 *                                                                    
 * FUNCTION: 	
 * 		Come here if a quit or interrupt signal is received
 *                                                                    
 */  
void
on_brk(int sno)
{
	if (!inwait) {
		BEEP();
		brk_hit = 1;
	}
	else {
		brk_hit = 0;
		longjmp(restore,1);
	}
}

/*
 * NAME: chgwinsz
 *                                                                    
 * FUNCTION:
 * 		Update window size data.
 */  
void
chgwinsz (int sno)
{
	struct winsize win;
	
	if ((!win_sz_set) && (out_is_tty)) {
    		if (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1) {
	    		window = win.ws_row-1;
			if (window <= 1)
				window = 2;
			columns = win.ws_col;
			if (columns <= 0)
				columns = 80;
    		}
	}
}

/*
 * NAME: end_it
 *                                                                    
 * FUNCTION:
 * 		Clean up terminal state and exit.
 */  
void
end_it (void)
{

	if (out_is_tty) {
		kill_line();
		resetterm();
	}
	if (tmp_fin)
		fclose(tmp_fin);
	if (tmp_fou)
		fclose(tmp_fou);
	if (tmp_fou || tmp_fin)
		unlink(tmp_name);
#ifdef TTY_PAGE
     	ioctl(fileno(stdout),TCSLEN,&gpage);
#endif
	exit(0);
}

/*
 *      catch SIGTSTP
 */
void
catchtstp(int sig)
{
        signal(sig, SIG_IGN);           /* temporarily... */

        /* ignore SIGTTOU so we don't get stopped if csh grabs the tty */
        signal(SIGTTOU, SIG_IGN);
        resetterm();
        fflush (stdout);
        signal(SIGTTOU, SIG_DFL);

        /* Send the TSTP signal to suspend our process group */
        signal(sig, SIG_DFL);
        sigsetmask(0);
        kill (0, sig);

        /* Pause for station break */

        /* We're back */
        signal (sig, catchtstp);
        fixterm();
}
