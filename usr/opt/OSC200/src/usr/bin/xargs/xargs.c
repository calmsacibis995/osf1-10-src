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
static char rcsid[] = "@(#)$RCSfile: xargs.c,v $ $Revision: 4.2.4.4 $ (DEC) $Date: 1993/10/11 19:54:51 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * 1.10  com/cmd/sh/xargs.c, cmdsh, bos320, bosarea 9125 6/7/91 19:15:05
 */

#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <nl_types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "pathnames.h"
#include "xargs_msg.h"

#define	MSGSTR(Num, Str)	catgets(catd, MS_XARGS, Num, Str)

/*
 * 'boolean' constants ...
 */
#define FALSE		0
#define TRUE		1

#define MAXSBUF		255		/* Tmp replacement buffer size */
#define MAXIBUF		512		/* Total replacement buffer size */
#define MAXINSERTS	5		/* Max replacements per line */
					/* Size for generic character arrays */
#define BUFSIZE		LINE_MAX + 100	/* buffer size of entire command+xtra */
#define MAXBUFLIM	LINE_MAX	/* Max command line size */
#define MAXARGS		LINE_MAX / 2	/* Max # of command line args */
					/* worst case: single char plus null */
#define MAXRESP		64		/* Maximum length of prompt response */



/*
 * usage message
 */

#define USAGE	\
"Usage:  xargs [-e[EndOfFileString]] [-E EndOfFileString] \n\
\t[-i[ReplacementString]] [-I ReplacementString] [-l[Number]] [-L Number] \n\
\t[-n Number] [-ptrx] [-s Length] [CommandString] [argument ...]\n" /* WW-01 */

/*
 * globals ...
 */
nl_catd	catd;			/* message catalog descriptor */
char Errstr[LINE_MAX];		/* generic error message buffer	*/
char *arglist[MAXARGS+1];	/* ptrs to args for the command to execute */
char argbuf[BUFSIZE+1];		/* destination for copied in arguments */
char *next = argbuf;		/* pointer to next empty spot in 'argbuf' */
char *lastarg = "";		/* last arg we parsed, but didn't use yet */
char **ARGV = arglist;		/* next empty slot in 'arglist' */
char *LEOF = "_"; 		/* logical end-of-file string */
char *INSPAT = "{}";		/* replace string pattern */
struct inserts {
	char **p_ARGV;		/* where to put newarg ptr in arg list */
	char *p_skel;		/* ptr to arg template */
	} saveargv[MAXINSERTS];
char ins_buf[MAXIBUF];		/* insert buffer */
char *p_ibuf;			/* pointer within ins_buf */
int PROMPT = -1;		/* prompt /dev/tty file descriptor */
int BUFLIM = MAXBUFLIM;		/* max command line size */
int N_ARGS = MAXARGS-1;		/* # of standard input args to use per cmd */
int N_args = 0;			/* # of arguments we've read so far */
int N_lines = 0;		/* # of input lines used so far for this cmd */
int DASHX = FALSE;		/* -x arg given? */
int MORE = TRUE;		/* process more input? */
int PER_LINE = FALSE;		/* # of input lines to use per cmd */
int Whole_line = FALSE;         /* Whole line will be taken as argument
                                   for option -I */
int ERR = FALSE;		/* should we stop from errors yet? */
int OK = TRUE;			/* had any errors yet? */
int LEGAL = FALSE;		/* stop if argument list size > BUFLIM */
int TRACE = FALSE;		/* echo cmd and args each time? */
int INSERT = FALSE;		/* do command replacements? */
int linesize = 0;		/* current size of cmd & args */
int ibufsize = 0;		/* current length of ins_buf */
int mb_cur_max;			/* Max bytes in multibyte character */

/*
 * function prototypes
 */

void	ermsg(char *);
void	usage(void);
void	addibuf(struct inserts *);
int	xindex(char *, char *);
int	echoargs();
int	lcall(char *, char **);
char *	addarg(char *);
char *	getarg();
char *	checklen(char *);
char *	insert(char *, char *);
wchar_t	getchr();

/*
 * NAME:	xargs
 *
 * SYNTAX:	xargs [flags] command
 *
 * FUNCTION:	Xargs constructs argument lists and runs commands
 *
 * RETURN VALUE DESCRIPTION:	0   if all utilities exited with 0
 *				1   if any error occurs
 *				126 if utility found but cannot exec
 *				127 if utility not found
 */

int
main(int argc, char **argv)
{
	char *cmdname, *initbuf, **initlist;
	int  ch;
	int  initsize;
	register int j, n_inserts;
	register struct inserts *psave;
	int  utility_err = 0;

	(void) setlocale (LC_ALL, "");
	mb_cur_max = MB_CUR_MAX;

	catd = catopen(MF_XARGS, NL_CAT_LOCALE);

	n_inserts = 0;
	psave = saveargv;

	/* look for flag arguments */
	/*
	 * Because of ugly syntax, a ';' means option has an optional
	 * argument (ie -i[replstr]).  See libc getopt code.
	 */
	while  ((ch = getopt(argc, argv, "E:e;I:i;L:l;n:prs:tx")) != -1) {
		switch (ch) {

		case 'x':	/* quit if any arg list size > BUFLIM */
			DASHX = LEGAL = TRUE;
			break;

		case 'l':
			if (!optarg) 		/* option omitted */
				optarg = "1";	/* default */
			LEGAL = TRUE;		/* only for 'l' */
			/* Fall Thru */
		case 'L':	/* specify number of input arg lines per cmd */
			PER_LINE = TRUE;
                        Whole_line = TRUE;
			N_ARGS = 0;
			if((PER_LINE=atoi(optarg)) <= 0 ) {
				sprintf(Errstr, MSGSTR(LINECNT,
				"#lines must be positive int: %s\n"), optarg);
				ermsg(Errstr);
				exit(1);
			}
			break;

		case 'i':			/* both same as -I {} */
		case 'r':
			if (!optarg)		/* option omitted for -i */
				optarg = "{}";	/* default */
			/* Fall Thru */
		case 'I':	/* process replace strings "{}" in arglist */
			INSERT = LEGAL = TRUE;
                        Whole_line = TRUE;
			N_ARGS = 0;
			INSPAT = optarg; /* change replace string */
			break;

		case 't':	/* echo each argument list to stderr */
			TRACE = TRUE;
			break;

                case 'E':       /* set logical end-of-file string */
			/* WW-01 option -E requires End-Of-File string */
   			/* If End-Of-File string missing, getopt() will */
			/* complain and exit */
		case 'e':	/* set logical end-of-file string */
			if (!optarg)
				strcpy(LEOF,"");
                        else LEOF = optarg;	
			break;

		case 's':	/* set max size of arg list (max: MAXBUFLIM) */
			BUFLIM = atoi(optarg);
			if( BUFLIM > MAXBUFLIM)		/* POSIX says */
				BUFLIM = MAXBUFLIM;
			if (BUFLIM <= 0 ) {
				sprintf(Errstr, MSGSTR(LINESIZ,
				 "size must be a positive int: %s.\n"), optarg);
				ermsg(Errstr);
				exit(1);
			}
			break;

		case 'n':	/* number of arguments to use per cmd */
			if( (N_ARGS = atoi(optarg)) <= 0 ) {
				sprintf(Errstr, MSGSTR(ARGCNT,
				 "#args must be positive int: %s\n"), optarg);
				ermsg(Errstr);
				exit(1);
			} else {
				LEGAL = DASHX || N_ARGS==1;
				PER_LINE = FALSE;
				}
			break;

		case 'p':	/* prompt for each cmd before running */
			if( (PROMPT = open("/dev/tty",0)) == -1) {
				ermsg(MSGSTR(TTYREAD,
					"can't read from tty for -p\n"));
				exit(1);
			} else
				TRACE = TRUE;
			break;

		case ':':
			fprintf(stderr, MSGSTR(NEEDARG, 
		                "%s: option requires an argument -- %c\n"), 
				argv[0], optopt);
			usage();
			break;
		default:
			fprintf(stderr, MSGSTR(UNKWNOPT, 
				"The %s flag is not valid.\n"), &optopt);
			usage();
			break;
		}
	}
	argv += optind;
	argc -= optind;

	/* pick up command name */

	if ( argc == 0 ) {
		cmdname = ECHO;
		*ARGV++ = addarg(cmdname);	/* add echo into our argv */
		}
	else
		cmdname = *argv;	/* will be added to argv below */

	/* pick up args on command line */

	while ( OK == TRUE && argc-- > 0) {
		if ( INSERT == TRUE && ! ERR ) {
			/* does this argument have an insert pattern? */
			if ( xindex(*argv, INSPAT) != -1 ) {
				/* yes, keep track of which arg has it */
				if ( ++n_inserts > MAXINSERTS ) {
					sprintf(Errstr, MSGSTR(ARGSIZ,
						"too many args with %s\n"),
						INSPAT);
					ermsg(Errstr);
					ERR = TRUE;
				}	
				psave->p_ARGV = ARGV;
				(psave++)->p_skel = *argv;
			}
		}
		/* add arg to our new argv */
		*ARGV++ = addarg( *argv++ );
	}

	/* pick up args from standard input */

	initbuf = next;			/* save first spot past cmd & argv */
	initlist = ARGV;		/* save first argv past cmd & argv */
	initsize = linesize;		/* save current total cmd size */

	/* loop once for each time we need to call cmd... */
	while ( OK == TRUE && MORE ) {
		/*
		 * reset our pointers and line size
		 */
		next = initbuf;
		ARGV = initlist;
		linesize = initsize;

		/*
		 * get any previous arguments we didn't process yet
		 */
		if ( *lastarg )
			*ARGV++ = addarg( lastarg );

		/*
		 * get the new args
		 */
		while ( (*ARGV++ = getarg()) && OK == TRUE )
			;

		/* insert arg if requested */

		if ( !ERR && INSERT == TRUE ) {
			p_ibuf = ins_buf;
			ARGV--;
			j = ibufsize = 0;
			for ( psave=saveargv;  ++j<=n_inserts;  ++psave ) {
				addibuf(psave);
				if ( ERR )
					break;
			}
		}
		*ARGV = 0;

		/* exec command */

		if ( ! ERR ) {
			if ( ! MORE &&
			    (PER_LINE && N_lines==0 || N_ARGS && N_args==0) )
				exit (0);
			OK = TRUE;
			j = TRACE ? echoargs() : TRUE;
			if( j ) {
				int retcode;
				if ((retcode=lcall(cmdname, arglist)) == 0 )
					continue;
				if ( retcode == 127 )
					sprintf(Errstr, MSGSTR(NOTFOUND_POSIX,
						"%s not found\n"), cmdname);
				else if ( retcode == 126 )
					sprintf(Errstr, MSGSTR(NOEXEC_POSIX,
						"can't execute %s\n"), cmdname);
				else if ( retcode == -1 ) {
					sprintf(Errstr, MSGSTR(NOEXEC,
					"%s not executed or was terminated.\n"),
						cmdname);
					retcode = 1;
				} else if ( retcode == 255 ) {
					sprintf(Errstr, MSGSTR(POSIX_STOP,
				    "%s returned 255 exit code.\n"), cmdname);
					retcode = 1;
				} else {
					utility_err = 1;
					continue;
				}
				ermsg(Errstr);
				exit (retcode);
			}
		}
		ERR = FALSE;
	}

	exit ((OK == TRUE && !utility_err) ? 0 : 1);

	/* NOTREACHED */
}

/*
 * NAME:	checklen
 *
 * FUNCTION:	checklen - check length of current arguments plus a new one
 *
 * NOTES:	Checklen checks the length of the current arguments plus
 *		the new one.  If we've gone last BUFLIM, we possibly print
 *		an error and set some error flags.
 *
 * RETURN VALUE DESCRIPTION:	NULL if we've gone past BUFLIM, else
 *		we return the new argument
 */

char *
checklen(char *arg)
{
	register int oklen;

	oklen = TRUE;
	if ( (linesize += strlen(arg)+1) > BUFLIM ) {
		lastarg = arg;
		oklen = FALSE;
		if ( LEGAL ) {
			ERR = TRUE;
			ermsg(MSGSTR(ARGLIST, "arg list too long\n"));
			}
		else if( N_args > 1 )
			N_args = 1;
		else {
			ermsg(MSGSTR(SNGLARG,
		"a single arg was greater than the max arglist size\n"));
			ERR = TRUE;
			OK = TRUE;
			arg="";
			lastarg="";
			}
		}

	return ( oklen == TRUE  ? arg : 0 );
}

/*
 * NAME:	addarg
 *
 * FUNCTION:	addarg - copy in our new arg
 *
 * NOTES:	Addarg copies in our new arg, then calls checklen().
 *
 * RETURN VALUE DESCRIPTION:	return value from checklen()
 */

char *
addarg(char *arg)
{
	(void) strcpy(next, arg);
	arg = next;
	next += strlen(arg) + 1;

	return ( checklen(arg) );
}

/*
 * NAME:	getarg
 *
 * FUNCTION:	getarg - read our next argument from stdin
 *
 * NOTES:	Getarg reads/parses our next argument from stdin.
 *
 * RETURN VALUE DESCRIPTION:	0 if there are no more args or if
 *		the arg doesn't fit, else a pointer to the arg
 */

char *
getarg()
{
	register wchar_t c, c1;
	register char *arg;
	char *retarg;

	/*
	 * skip white space
	 */
	while ( (c=getchr()) == '\n' || iswctype((wint_t)c, wctype("blank")) )
		;

	/*
	 * all done?
	 */
	if ( c == '\0' ) {
		MORE = FALSE;
		return ((char *)NULL);
	}

	arg = next;
	/*
	 * read chars and process them ...
	 */
	for ( ; ; c = getchr() )
		switch ( c ) {

		case '\t':		/* blank space */
		case ' ' :
		space:
			/*
			 * only save blank characters if we have to do replace
			 * patterns...
			 */
			if (INSERT == TRUE) {
				if ( mb_cur_max == 1 )
					*next++ = c;
				else
					next += wctomb(next, c);
				break;
			}

		case '\0':		/* end of line/file */
		case '\n':
			*next++ = '\0';	/* null terminate current arg */
			/*
			 * Process logical EOF. Stop trying to read
			 * if a newline or null terminator typed at
			 */
			if( strcmp(arg, LEOF) == 0 || c == '\0' ) {
				MORE = FALSE;
				while ( c != '\n' && c != '\0' )
					c = getchr();
				return 0;
			}
			else {
				/*
				 * add arg to our arguments
				 */
				++N_args;
				if( retarg = checklen(arg) ) {
					if( (Whole_line && c=='\n' &&
					     ++N_lines>=PER_LINE)
					||   (N_ARGS && N_args>=N_ARGS) ) {
						N_lines = N_args = 0;
						lastarg = "";
						OK = FALSE;
						}
				}
				return (retarg);
			}

		case '\\':		/* backslash */
			c = getchr();
			if ( mb_cur_max == 1 )
				*next++ = c;
			else
				next += wctomb(next, c);
			break;

		case '"':		/* quotes and double quotes */
		case '\'':
			while( (c1=getchr()) != c) {
				/* copy chars in till hit the next one */
				if( c1 == '\0' || c1 == '\n' ) {
					/* missing a quote... */
					*next++ = '\0';
					sprintf(Errstr, MSGSTR(MSNGQUOT,
						"missing quote?: %s\n"), arg);
					ermsg(Errstr);
					ERR = TRUE;
					return ((char *)NULL);
				}
				if ( mb_cur_max == 1 )
					*next++ = c1;
				else
					next += wctomb(next, c1);
			}
			break;

		default:
			if (iswspace(c))
				goto space;
			if ( mb_cur_max == 1 )
				*next++ = c;
			else
				next += wctomb(next, c);
			break;
		}
}

/*
 * NAME:	ermsg
 *
 * FUNCTION:	ermsg - print an error message to standard error
 *
 * NOTES:	Errmsg prints the error messages 'messages' to
 *		standard error and sets the OK flag to false.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
ermsg(char *messages)
{
	write(2, "xargs: ", 7);
	write(2, messages, strlen(messages));

	OK = FALSE;
}

/*
 * NAME:	echoargs
 *
 * FUNCTION:	echoargs - print out arguments and prompt for yes or
 *		no
 *
 * NOTES:	Echoargs prints out the arguments and prompts the user
 *		for a yes or no answer.
 *
 * RETURN VALUE DESCRIPTION:	TRUE if the user responds with yes expression,
 *		else FALSE
 */

int
echoargs()
{
	register char **anarg;
	char yesorno[MAXRESP+1], *presp;
	char *prompt;
	int j;

	anarg = arglist-1;
	while ( *++anarg ) {
		write(2, *anarg, strlen(*anarg) );
		write(2," ",1);
	}

	if( PROMPT == -1 ) {
		write(2,"\n",1);
		return (TRUE);
	}

	prompt = MSGSTR(QUESTION, "?...");
	write(2, prompt, strlen(prompt));

	presp = yesorno;
	while ( ((j = read(PROMPT,presp,1))==1) && (*presp!='\n') )
		if ( presp < &yesorno[MAXRESP] )
			presp++;
	if ( j == 0 )
		exit(0);
	if ( yesorno[0] == '\n')
		return (FALSE);
	*presp = '\0';
	return (rpmatch(yesorno) == 1 ? TRUE : FALSE);
}

/*
 * NAME:	insert
 *
 * FUNCTION:	insert - handle a replacement
 *
 * NOTES:	Insert takes a pattern and a replacement string and
 *		does any replacements necessary in the pattern.  The new
 *		string is returned.
 *
 * RETURN VALUE DESCRIPTION:	The new string after replacements.
 */

char *
insert(char *pattern, char *subst)
{
	static char buffer[MAXSBUF+1];
	int len, ipatlen;
	register char *pat;
	register char *bufend;
	register char *pbuf;

	len = strlen(subst);
	ipatlen = strlen(INSPAT)-1;
	pat = pattern-1;
	pbuf = buffer;
	bufend = &buffer[MAXSBUF];

	while ( *++pat ) {
		if( xindex(pat,INSPAT) == 0 ) {
			if ( pbuf+len >= bufend )
				break;
			else {
				strcpy(pbuf, subst);
				pat += ipatlen;
				pbuf += len;
			}
		}
		else {
			*pbuf++ = *pat;
			if (pbuf >= bufend )
				break;
			}
		}

	if ( ! *pat ) {
		*pbuf = '\0';
		return (buffer);
	}

	sprintf(Errstr, MSGSTR(MAXSARGSIZ,
		"max arg size with insertion via %s's exceeded\n"), INSPAT);
	ermsg(Errstr);
	ERR = TRUE;

	return ((char *)NULL);
}

/*
 * NAME:	addibuf
 *
 * FUNCTION:	addibuf - perform a command replacement
 *
 * NOTES:	Addibuf looks at a struct insert structure, which
 *		contains the information for one replacement in
 *		the command string, and does the replacement.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
addibuf(struct inserts *p)
{
	register char *newarg, *skel, *sub;
	int l;

	skel = p->p_skel;	/* place in argv the replacement was */
	sub = *ARGV;		/* destination ARGV for the replacement */

	if ( sub ) {
		linesize -= strlen(skel)+1;
		newarg = insert(skel, sub);	/* do the replacement */
		/*
	 	 * make sure the replacement fits
	 	 */
		if ( checklen(newarg) ) {
			if( (ibufsize += (l=strlen(newarg)+1)) > MAXIBUF) {
				ermsg(MSGSTR(OVRFLOW, 
						"insert-buffer overflow\n"));
				ERR = TRUE;
			}
		strcpy(p_ibuf, newarg);	/* save replacement string */
		*(p->p_ARGV) = p_ibuf;	/* point to it */
		p_ibuf += l;		/* increment replacement pointer */
		}
	}
	else
		return;
}

/*
 * NAME:	getchr
 *
 * FUNCTION:	getchr - read the next character from standard input
 *
 * NOTES:	Getchr reads the next character from standard input
 *		and returns it.  0 is returned on end of file or error.
 *
 * RETURN VALUE DESCRIPTION:	0 if end of file is encountered or
 *		we get an I/O error,  else the character process code
 *		is returned
 */

wchar_t
getchr()
{
	int c;

	if ( mb_cur_max == 1) {
		c = getchar();
		if ( c == EOF ) c = 0;
	}
	else {
		c = getwchar();
		if ( c == WEOF ) c = 0;
	}

	return (c);
}

/*
 * NAME:	lcall
 *
 * FUNCTION:	lcall - exec program with arguments
 *
 * NOTES:	Lcall forks a new process and executes the program 'sub'
 *		using 'subargs' as it's arguments.  It also waits for the
 *		program to finish and returns a status code indicating
 *		the success of the program.
 *
 * RETURN VALUE DESCRIPTION:	-1 if the fork failed or child terminated,
 * 		 else the exit status of the program.
 */

int
lcall(char *sub, char **subargs)
{

	int retcode;
	register pid_t iwait; 
	pid_t child;

	switch( child=fork() ) {

	default:
		iwait = waitpid(child, &retcode, 0);

		/* exit code macros are in sys/wait.h ... */
		if( iwait == -1  ||  !WIFEXITED(retcode))
			return (-1);
		return ( WEXITSTATUS(retcode) );

	case 0:    /* child */
		(void) close(0);
		(void) open("/dev/null", O_RDONLY);
		execvp(sub, subargs);
		if ( errno == ENOENT )
			exit(127);
		else
			exit(126);

	case -1:
		return (-1);
	}

	/* NOTREACHED */
}
/************************************************************************/
/*	WARNING: RETURNED STATUS OF OFFSET WITHIN STRING IS		*/
/*	IGNORED - WHICH MAKES THE USE OF THIS FUNCTION SUSPECT		*/
/*	WHEN ARGUMENT REPLACEMENT IS PERFORMED				*/
/************************************************************************/

/*
 * NAME:	xindex
 *
 * FUNCTION:	xindex - search for substring
 *
 * NOTES:	Xindex searches for a substring 'as2' in the string 'as1'.
 *		If the substring exists, it returns the offset of the
 *		first occurrence.  If it doesn't exist, it returns -1.
 *
 * RETURN VALUE DESCRIPTION:	-1 if the substring doesn't exist, else
 *		the index of the first occurrence of the substring
 */

int
xindex(char *as1, char *as2)
{
	register char *s1,*s2,c;
	int offset;

	s1 = as1;
	s2 = as2;
	c = *s2;

	while (*s1)
		if (*s1++ == c) {
			offset = s1 - as1 - 1;
			s2++;
			while ((c = *s2++) == *s1++ && c)
				;
			if (c == 0)
				return (offset);
			s1 = offset + as1 + 1;
			s2 = as2;
			c = *s2;
		}
	 return (-1);
}

/*
 * print usage and exit
 */
void
usage(void)
{
	fprintf(stderr, MSGSTR(USEMSG, USAGE));
	exit(1);
}
