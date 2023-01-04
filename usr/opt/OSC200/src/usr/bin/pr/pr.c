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
static char rcsid[] = "@(#)$RCSfile: pr.c,v $ $Revision: 4.3.7.7 $ (DEC) $Date: 1993/10/19 11:19:10 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: pr
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.19  com/cmd/files/pr.c, cmdfiles, bos320, 9138320 9/11/91 13:34:13
 */

/* This code is compliant with POSIX 1003.2 Draft 11. */

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <sys/types.h>
#include <errno.h>
#include <nl_types.h>
#include <langinfo.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include "pr_msg.h"


nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_PR,Num,Str)

#define ESC     '\033'
#define LENGTH  66
#define LINEW   72
#define NUMW    5
#define MARGIN  10
#define DEFTAB  8

#define USAGE_MSG "Usage: pr [-adfFmprt] [-e[Character][Gap]]\n\
\t\t   [-h \"Header\"] [-i[Character][Gap]] [-l Lines]\n\
\t\t   [-n[Character][Width]] [-o Offset] [-s[Character]] [-w Width]\n\
\t\t   [-Column] [+Page] [File...]\n"

#define wcNEWLINE       ('\n')
#define wcTAB           ('\t')
#define wcSPACE         (' ')
#define wcFF            ('\f')
#define wcNO_ACTION   	('\000')        /* GA001 */

#define _WEOF  ((wchar_t) WEOF)
#define FOREVER 1
#define NO_ACTION

/*
 *      PR command (print files in pages and columns, with headings)
 *      2+head+2+page[56]+5
 */

char	nulls[] = "";
typedef struct {
	FILE	*f_f;
	char	*f_name;
	wchar_t f_nextc;
} FILS;
wchar_t Etabc, Itabc, Nsepc;
FILS *Files;
int	Multi = 0, Nfiles = 0, Error = 0, onintr(void);
int	mbcm;	/* MB_CUR_MAX */

typedef int		ANY;
typedef unsigned	UNS;

#define NFILES  10
char	obuf[BUFSIZ];
int	Ttyout;

#define istty(F)        isatty((int)fileno(F))
#define done()         	/* no cleanup */
#define INTREXIT        _exit
#define CADDID()
#define HEAD    "%s %s %s %d\n\n\n", date, head, MSGSTR(PAGE, "Page"), Page
#define cerror(S)       fprintf(stderr, "pr: %s", S)
#define STDINNAME()     nulls
#define TTY     	"/dev/tty", "r"
#define PROMPT()        putc('\7', stderr)	/* BEL */
#define NOSFILE		nulls
#define ETABS		(Inpos % Etabn)
#define NSEPC		wcTAB

/* Module Protototypes
 */
FILE	*mustopen(char *, FILS *);
char	*GETDATE(void);
ANY	*getspace(UNS);
wchar_t get(int);
int	atoix(char *), balance(int), die(char *),
	errprint(void), findopt(int, char **), fixtty(void),
	intopt(char *, wchar_t *), nexbuf(void), onintr(void),
	print(char *), put(wchar_t), putpage(void), putspace(void);

long	Lnumb = 0;
FILE	*Ttyin = stdin;
int	Dblspace = 1, Fpage = 1, Formfeed = 0, Pausefirst = 0,
	Length = LENGTH, Linew = 0, Offset = 0, Ncols = 1, Pause = 0,
	Colw, Plength, Margin = MARGIN, Numw, Report = 1,
	Etabn = 0, Itabn = 0;
int	numw;
wchar_t Sepc = 0;
char	*Head = NULL;
wchar_t *Buffer = NULL, *Bufend;

typedef struct {
	wchar_t	*c_ptr, *c_ptr0;
	long	c_lno;
} *COLP;
COLP Colpts;

int	Page, Nspace, Inpos;
wchar_t	__WC = '\0';

int	Outpos, Lcolpos, Pcolpos, Line;

#define EMPTY   14     	/* length of " -- empty file" */
typedef struct err {
	struct err	*e_nextp;
	char		*e_mess;
} ERR;
ERR	*Err = NULL, *Lasterr = (ERR *)&Err;


/*
 * NAME: fixtty
 *
 * FUNCTION: set up a buffer for the tty
 */
fixtty(void)
{
	setbuf(stdout, obuf);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, (void (*)(int))onintr);
	Ttyout = istty(stdout);
	return;
}


/*
 * NAME: GETDATE
 *
 * FUNCTION:  return date file was last modified
 */
char *
GETDATE(void)
{
	static char	now[64] = "";
	static struct stat sbuf, nbuf;
	char	*s_ptr, fmt[64];

	/*
	 * Ideally we would like to use nl_langinfo() to get this
	 * date format, but it doesn't match the POSIX required
	 * format in the POSIX locale.
	 */
	s_ptr = nl_langinfo(_M_D_RECENT);
	if (s_ptr == NULL || *s_ptr == '\0')
		s_ptr = MSGSTR(DATE_FMT, "%b %e %H:%M %Y");
	else {
		strcpy(fmt, s_ptr);
		strcat(fmt, " %Y");
		s_ptr = fmt;
	}

	if (Nfiles > 1 || Files->f_name == nulls) {
		if (now[0] == '\0') {
			time(&nbuf.st_mtime);
			strftime(now, 63, s_ptr, localtime(&nbuf.st_mtime));
		}
	} else {
		stat(Files->f_name, &sbuf);
		strftime(now, 63, s_ptr, localtime(&sbuf.st_mtime));
	}
	return (now);
}


/*
 * NAME: pr [options] [files]
 *
 * FUNCTION: Writes a file to standard output.  If a file is not specified
 *      or a '-' is for the file name then standard input is read.
 *      OPTIONS:
 *      -a        Displays multi-column output
 *      -d        Double-spaces the output
 *      -e[c][n]  Expands tabs to n+1, 2*n+1, 3*n+1 etc. Default is 8
 *                If a character c is specified then that character becomes
 *                the input tab character.
 *      -f        Uses a form-feed character to advance to a new page.
 *                (Historical; see -F below.)
 *      -F        Uses a form-feed character to advance to a new page.
 *                (POSIX)
 *      -h "string"   Displays string as the page header instead of the file
 *      -i[c][n]  replaces white spaces with tabs at n+1, 2*n+1, 3*n+1 etc.
 *                Default is 8.  If a character c is specified then that
 *                character becomes the output tab  character.
 *      -lnum     Sets the length of a page to num
 *      -m        combines and writes all files at the same time, with each
 *                in a separate column (overrides -num and -a flags).
 *      -n[c][n]  number of digits used for numbering lines c is added to
 *                the line.
 *      -onum     Indents each line by num spaces.
 *      -p        pauses before beginning each page.
 *                (Historical; not POSIX.)
 *      -r        suppresses diagnostic messages if the system can't open files
 *      -schar    separates columns by the single character char instead
 *                of spaces
 *      -t        does not display header and footer
 *      -wnum     sets the width of a line
 *      -num      produce num-column  output
 *      +num      begin the display with page num (default is 1)
 */
main(int argc, char **argv)
{
	FILS fstr[NFILES];
	int	nfdone = 0;
	int	ind;
	char	*p;

	p = setlocale(LC_ALL, "");
	catd = catopen(MF_PR, NL_CAT_LOCALE);

	mbcm = MB_CUR_MAX;

	Nsepc = Itabc = Etabc = wcTAB;
	Files = fstr;
	ind = findopt(argc, argv);
	argv += ind;
	argc -= ind;
	for (; argc > 0; --argc, ++argv)
		if (Multi == 'm') {
			if (Nfiles >= NFILES - 1)
				die(MSGSTR(TOOMANYF, "too many files"));
			if (mustopen(*argv, &Files[Nfiles++]) == NULL)
				++nfdone;	/* suppress printing */
		} else {
			if (print(*argv))
				fclose(Files->f_f);
			++nfdone;
		}
	if (!nfdone)	/* no files named, use stdin */
		print(NOSFILE);	/* on GCOS, use current file, if any */
	errprint();	/* print accumulated error reports */
	exit(Error);
	/* NOTREACHED */
}


/*
 * NAME: findopt
 *
 * FUNCTION: get options and set flag
 * 
 * RETURNS: the index in to argv where the argument stop (optind)
 */
findopt(int argc, char **argv)
{
	int	c;

	fixtty();
	/*
	 * Do to ugly syntax, we have our own getopt
	 * a semi-colon (;) means 'optional argument'.
	 * a dash (-) return means optarg points to a number (-123)
	 * a plus-sign (+) means optarg point to a number (+123)
	 */
	while ((c = pr_getopt(argc,argv,"ade;fFh:i;l:n;mo:prs;tw:x;")) != -1) {
	switch (c) {
		case '-':
			Ncols = atoix(optarg);
			break;
		case '+':
			if ((Fpage = atoix(optarg)) < 1)
				Fpage = 1;
			break;
		case 'a':
			if (Multi != 'm')
				Multi = 'a';	/* 3.1 */
			break;;
		case 'd':
			Dblspace = 2;
			break;;
		case 'e':
			if ((Etabn = intopt(optarg, &Etabc)) < 0)
				Etabn = DEFTAB;
			break;;
		case 'f':
			++Pausefirst;
		case 'F':
			++Formfeed;
			break;;
		case 'h':
			Head = optarg;
			break;;
		case 'i':
			if ((Itabn = intopt(optarg, &Itabc)) < 0)
				Itabn = DEFTAB;
			break;;
		case 'l':
			Length = atoix(optarg);
			break;;
		case 'm':
			Multi = 'm';
			break;;
		case 'n':
		case 'x': /* retained for historical reasons */
			++Lnumb;
			if ((Numw = intopt(optarg, &Nsepc)) <= 0)
				Numw = NUMW;
			break;
		case 'o':
			Offset = atoix(optarg);
			break;;
		case 'p':
			++Pause;
			break;;
		case 'r':
			Report = 0;
			break;;
		case 's':
			if (optarg != NULL)
				mbtowc(&Sepc, optarg, mbcm);
			else 
				Sepc = wcTAB;
			break;;
		case 't':
			Margin = 0;
			break;;
		case 'w':
			Linew = atoix(optarg);
			break;;
		case 'b': /* retained for historical reasons */
		case 'q': /* retained for historical reasons */
		case 'j': /* ignore GCOS jprint option */
			break;;
		default :
			die(MSGSTR(USAGE, USAGE_MSG));
			break;
		}
	}

	if (Length == 0)
		Length = LENGTH;
	if (Length <= Margin)
		Margin = 0;
	Plength = Length - Margin / 2;
	if (Multi == 'm')
		Ncols = argc - optind;
	switch (Ncols) {
	case 0:
		Ncols = 1;
	case 1:
		break;
	default:
		if (Etabn == 0)	/* respect explicit tab specification */
			Etabn = DEFTAB;
		if (Itabn == 0)
			Itabn = DEFTAB;
	}
	if (Linew == 0)
		Linew = Ncols != 1 && Sepc == 0 ? LINEW : 512;
	if (Lnumb) {

		if (Nsepc == wcTAB) {
			if (Itabn == 0)
				numw = Numw + DEFTAB - (Numw % DEFTAB);
			else
				numw = Numw + Itabn - (Numw % Itabn);
		} else {
			numw = Numw + ((iswprint(Nsepc)) ? 1 : 0);
		}
		Linew -= (Multi == 'm') ? numw : numw * Ncols;
	}
	if ((Colw = (Linew - Ncols + 1) / Ncols) < 1)
		die(MSGSTR(WDTHSMALL, "width too small"));
	if (Ncols != 1 && Multi == 0) {
		UNS buflen = ((UNS)(Plength / Dblspace + 1))
			     * 2 * (Linew + 1) * sizeof(wchar_t);

		Buffer = (wchar_t *)getspace(buflen);
		Bufend = &Buffer[buflen];
		Colpts = (COLP)getspace((UNS)((Ncols + 1) * sizeof(*Colpts)));
	}
	if (Ttyout && (Pause || Pausefirst) && !istty(stdin))
		Ttyin = fopen(TTY);
	return (optind);
}


char	*optarg;	/* argument for current option	*/
extern int	optind;		/* index in to argv */

/*
 * NAME: pr_getopt
 *
 * FUNCTION: get flag letters from the argument vector
 *
 * NOTES:	The optstring can have ';' to indicate an optional
 *		argument (ie -n[char][num]).
 *
 */  

int 
pr_getopt(  int	 argc,			/* number of command line arguments */
	    char * const *argv,		/* pointer to command line arguments */
	    char const *optstring)	/* string describing valid flags */
{
	int	c;
	char	*cp;
	static int stringind = 1;	/* index of next option */

	if (stringind == 1) {
		if (optind >= argc || argv[optind] == NULL)
			return (-1);
		if ((argv[optind][0] != '-' && argv[optind][0] != '+') || 
		     argv[optind][1] == '\0') {
			return (-1);
		} else if (strcmp(argv[optind], "--") == 0) {
			optind++;
			return (-1);
		}
		if (argv[optind][0] == '+') {
			optarg = argv[optind] + 1;
			optind++;
			return('+');
		}
	}
	c = argv[optind][stringind];

	if (isdigit(c)) {		/* digit string as option */
		optarg = (char *) &argv[optind][stringind];
		while (isdigit(argv[optind][++stringind])) ;
		if (argv[optind][stringind] == '\0') {
			stringind = 1;
			optind++;
		}
		return ('-');
	}

	if (c == ':' || (cp = strchr(optstring, c)) == NULL) {
		fprintf(stderr,MSGSTR(BAD_OPT,"pr: illegal option -- %c\n"), c);
		if (argv[optind][++stringind] == '\0') {
			optind++;
			stringind = 1;
		}
		return ('?');
	}
	if (*++cp == ':') {		/* parameter is needed */
		/* no blanks to separate option and parameter */
		if (argv[optind][stringind+1] != '\0')
			optarg = (char *) &argv[optind++][stringind+1];
		else if (++optind >= argc) {
			stringind = 1;
			if (optstring[0] != ':') {
				fprintf(stderr,MSGSTR(ARG_REQ,
				 "pr: option requires an argument -- %c\n"), c);
				return ('?');
			}
			return (':');
		} else
			optarg = (char *) argv[optind++];
		stringind = 1;
	} else if (*cp == ';') {		/* optional parameter */
		if (argv[optind][stringind+1] != '\0')
			optarg = (char *) &argv[optind][stringind+1];
		else 
			optarg = NULL;
		stringind = 1;
		optind++;
	} else {			/* parameter not needed */
		/* if c is the last option update optind */
		if (argv[optind][++stringind] == '\0') {
			stringind = 1;
			optind++;
		}
		optarg = NULL;
	}
	return (c);
}

/*
 * NAME: intopt
 *
 * FUNCTION: get num and char and get the ascii values
 */
intopt(char *arg, wchar_t *optp)
{
	char	c;
	int	num;

	if (arg == NULL)
		return (-1);

	if (!isdigit(*arg))		/* digits must be 1 byte */
		arg += mbtowc(optp, arg, mbcm);
	num = atoix(arg);
	if (num == 0)
		num = -1;
	return (num);
}


/*
 * NAME: print
 *
 * FUNCTION: print header for next page
 */
print(char *name)
{
	static int	notfirst = 0;
	char	*date = NULL, *head = NULL;
	short	len;
	wchar_t wc;

	if (Multi != 'm' || Nfiles == 0)
		if (mustopen(name, &Files[Nfiles]) == NULL)
			return(0);
	if (Buffer)
		ungetwc(Files->f_nextc, Files->f_f);
	if (Lnumb)
		Lnumb = 1;

	/* This is the main loop, putpage is where the buffer information
	   is printed out  */

	for (Page = 0; ; putpage()) {
		if (__WC == _WEOF)
			break;
		/* Fill up the buffer */
		if (Buffer)
			nexbuf();
		Inpos = 0;
		if (get(0) == _WEOF)
			break;
		fflush(stdout);
		if (++Page >= Fpage) {
			if (Ttyout && (Pause || (Pausefirst && !notfirst++))) {
				PROMPT();	/* prompt with bell and pause */
				while ((wc = getwc(Ttyin)) != _WEOF
				       && wc != '\n')
					;
			}
			if (Margin == 0)
				continue;
			CADDID();
			if (date == NULL)
				date = GETDATE();
			if (head == NULL)
				head = Head != NULL ? Head :
				    Nfiles < 2 ? Files->f_name : nulls;
			printf("\n\n");
			Nspace = Offset;
			putspace();
			printf(HEAD);
		}
	}
	__WC = '\0';
	return (1);
}


/*
 * NAME: putpage
 *
 * FUNCTION: print out the current page
 */
putpage(void)
{
	int	colno;

	for (Line = Margin / 2; FOREVER; get(0)) {
		for (Nspace = Offset, colno = Outpos = 0; __WC != wcFF; ) {
			if (Lnumb && __WC != _WEOF
			    && ((colno == 0 && Multi == 'm') || Multi != 'm')) {
				if (Page >= Fpage) {
					putspace();
					printf("%*ld%c", Numw,
					       Buffer ? Colpts[colno].c_lno++
						      : Lnumb, Nsepc);
					Outpos += numw;
				}
				++Lnumb;
			}
			/* This loop will print out the characters to stdout.
			 * put puts a character from the buffer to stdout and
			 * get gets a character from the buffer */
			for (Lcolpos = 0, Pcolpos = 0;
			     __WC != wcNEWLINE && __WC != wcFF
			     && __WC != _WEOF; ) {
				put(__WC);
				get(colno);
			}
			if (__WC == _WEOF)
				break;
			if (++colno == Ncols)
				break;
			if (__WC == wcNEWLINE && get(colno) == _WEOF)
				break;
			if (Sepc)
				put(Sepc);
			else if ((Nspace += Colw - Lcolpos + 1) < 1)
				Nspace = 1;
		}
		if (__WC == _WEOF) {
			if (Margin != 0)
				break;
			if (colno != 0)
				put(wcNEWLINE);
			return;
		}
		if (__WC == wcFF)
			break;
		put(wcNEWLINE);
		if (Dblspace == 2 && Line < Plength)
			put(wcNEWLINE);
		if (Line >= Plength)
			break;
	}
	if (Formfeed)
		put(wcFF);
	else
		while (Line < Length)
			put(wcNEWLINE);
}


/*
 * NAME: nexbuf
 *
 * FUNCTION: build next line
 */
nexbuf(void)
{
	wchar_t	*s = Buffer;
	COLP	p = Colpts;
	int	j, bline = 0;
	wchar_t wc;

	for ( ; ; ) {
		p->c_ptr0 = p->c_ptr = s;
		if (p == &Colpts[Ncols])
			return;

		(p++)->c_lno = Lnumb + bline;

		/* This is the loop where the characters are read from
		 * the file */

		for (j = (Length - Margin) / Dblspace; --j >= 0; ++bline)
			for (Inpos = 0; ; ) {
				errno = 0;
				wc = fgetwc(Files->f_f);

				/* If the character is an EOF, is it the real
				 * EOF or was an illegal character found (for
				 * the current locale). */

				if (wc == _WEOF) {

					/* If there is an illegal character,
					 * get it out of the file and put a #?
					 * in the buffer (if there is space left
					 * in the column). */

					if (errno == EILSEQ) {
						fgetc(Files->f_f);
						if (Inpos < Colw - 1) {
							*s++ = '#';
							if (s >= Bufend)
				die(MSGSTR(PBOVRFL, "page-buffer overflow"));
							*s++ = '?';
							if (s >= Bufend)
				die(MSGSTR(PBOVRFL, "page-buffer overflow"));
						}
						Inpos += 2;
						Error++;
					} else {
						/* Real EOF or error condition
						 * from fgetwc that can't be
						 * fixed.  Setup everything for
						 * the end of a file */
						for (*s = _WEOF;
						     p <= &Colpts[Ncols]; ++p)
							p->c_ptr0 =
							p->c_ptr = s;
						balance(bline);
						return;
					}
				} else {

					/* The character was "gotten" ok and
					 * processing should proceed as normal.
					 */

			/* According to POSIX.2 Draft 11, if a character is not
			 * printable, it is still put in the buffer, but it is
			 * not used in the computation of the column width or
			 * or page length. This will cause pr to look busted
			 * when there are characters that are marked unprintable
			 * in the locale, but are really printable because of
			 * font set, etc. IE In the C locale, the characters
			 * above 127 are marked as unprintable, but they will
			 * show up when sent to stdout.
			 */

					if (iswprint(wc))
						Inpos = Inpos + wcwidth(wc);
					if (Inpos <= Colw || wc == wcNEWLINE) {
						*s = wc;
						++s;
						if (s >= Bufend)
				die(MSGSTR(PBOVRFL, "page-buffer overflow"));
					}
					if (wc == wcNEWLINE)
						break;

					switch (wc) {
					case '\b':
						if (Inpos == 0)
							--s;
					case ESC:
						if (Inpos > 0)
							--Inpos;
					}
				}
			}
	}
}


/*
 * NAME: balance
 *
 * FUNCTION: line balancing for last page
 */
balance(int bline)
{
	wchar_t	*s = Buffer;
	COLP	p = Colpts;
	int	colno = 0, j, c, l, flag;

	c = bline % Ncols;
	l = (bline + Ncols - 1) / Ncols;
	bline = 0;
	do {
		for (j = 0; j < l; ++j)
			while (*s++ != '\n')
				;
		(++p)->c_lno = Lnumb + (bline += l);
		p->c_ptr0 = p->c_ptr = s;
		if (++colno == c)
			--l;
	} while (colno < Ncols - 1);
}


/*
 * NAME: get
 *
 * FUNCTION: build next column
 */
wchar_t get(int colno)
{
	static int	peekc = 0;
	COLP	p;
	FILS	*q;
	wchar_t wc;
	short	len;

	if (peekc) {
		peekc = 0;
		wc = Etabc;
	} else if (Buffer) {
		p = &Colpts[colno];
		if (p->c_ptr >= (p + 1)->c_ptr0)
			wc = _WEOF;
		else
			wc = *p->c_ptr++;
	} else
	{
		wc = (q = &Files[Multi == 'a' ? 0 : colno])->f_nextc;
		if (wc == _WEOF) {
			for (q = &Files[Nfiles];
			    --q >= Files && q->f_nextc == _WEOF; )
				;
			if (q >= Files)
				wc = wcNEWLINE;
		} else
			q->f_nextc = getwc(q->f_f);
	}

	if (Etabn != 0 && wc == Etabc) {
		++Inpos;
		peekc = ETABS;
		wc = wcSPACE;
	} else {
		if (wc != _WEOF && iswprint(wc))
			Inpos = Inpos + wcwidth(wc);
		else {
			if (wc != _WEOF) {
				switch (wc) {
				case '\b':
				case ESC:
					if (Inpos > 0)
						--Inpos;
					break;
				case '\f':
					if (Ncols == 1)
						break;
					wc = wcNO_ACTION;       /* GA001 */

				case '\n':
				case '\r':
					Inpos = 0;
				}
			}
		}
	}
	__WC = wc;
	return (__WC);
}


/*
 * NAME: put
 *
 * FUNCTION: put out char c into the buffer, but check for special characters
 *           first.
 */
put(wchar_t wc)
{
	int	move;
	char	temp[16];
	int	save_n;

	switch (wc) {
	case ' ':
		if (Ncols < 2 || Lcolpos < Colw) {
			++Nspace;
			++Lcolpos;
		}
		return;
	case '\t':
		if (Itabn == 0) {
			move = DEFTAB - (Lcolpos % DEFTAB);
			break;
		}
		if (Lcolpos < Colw) {
			move = Itabn - ((Lcolpos + Itabn) % Itabn);
			move = (move < Colw - Lcolpos) ? move : Colw - Lcolpos;
			Nspace += move;
			Lcolpos += move;
		}
		return;
	case '\b':
		if (Lcolpos == 0)
			return;
		if (Nspace > 0) {
			--Nspace;
			--Lcolpos;
			return;
		}
		if (Lcolpos > Pcolpos) {
			--Lcolpos;
			return;
		}
	case ESC:
		move = -1;
		break;
	case '\n':
		++Line;
	case '\r':
	case '\f':
		Pcolpos = Lcolpos = Nspace = Outpos = 0;
	default:
		move = iswprint(wc) ? wcwidth(wc) : 0;
	}
	if (Page < Fpage)
		return;
	if (Lcolpos > 0 || move > 0) {
		save_n = Lcolpos;
		Lcolpos += move;
		if ((Lcolpos > Colw) && iswprint(wc)) {
			for (; save_n < Colw; save_n++) {
				putchar(' ');
				Outpos++;
			}
		}
	}
	putspace();

	if ((Ncols < 2 && Outpos < Linew) || Lcolpos <= Colw) {
		putwchar(wc);
		Outpos += move;
		Pcolpos = Lcolpos;
	}
}


/*
 * NAME: putspace
 *
 * FUNCTION: put out white space (tab or space or specified char)
 */
putspace(void)
{
	int	nc, flag;
	for ( ; Nspace > 0; Outpos += nc, Nspace -= nc) {
		if (Itabn > 0 && Nspace >= (nc = Itabn - Outpos % Itabn) && 
		    nc > 1)
			putwchar(Itabc);
		else {
			nc = 1;
			putchar(' ');
		}
	}
/*
        for ( ; Nspace > 0; Outpos += nc, Nspace -= nc) {
           nc = Itabn - Outpos % Itabn;
           flag = (Itabn > 0 && Nspace >= nc);
           if (flag)
              putwchar(Itabc);
           else {
              nc = 1;
              putchar(' ');
           }
        }
*/
}


/*
 * NAME: atoix
 *
 * FUNCTION: convert character string of digits to an integer
 */
atoix(char *p)
{
	int	n = 0, c;

	while (isdigit(c = *p++))
		n = 10 * n + c - '0';
	return (n);
}


/*
 * NAME: mustopen
 *
 * FUNCTION: open a file
 */
/* Defer message about failure to open file to prevent messing up
   alignment of page with tear perforations or form markers.
   Treat empty file as special case and report as diagnostic.
*/
FILE *
mustopen(char *s, FILS *f)
{
	char	*empty, *cantopen;

	if (s[0] == '\0' || (s[0] == '-' && s[1] == '\0')) {
		f->f_name = STDINNAME();
		f->f_f = stdin;
	} else if ((f->f_f = fopen(f->f_name = s, "r")) == NULL) {
		cantopen = MSGSTR(CANTOPEN, "Cannot open file %s\n");
		s = (char *)getspace((UNS)(strlen(cantopen)
					   + strlen(f->f_name) + 1));
		sprintf(s, cantopen, f->f_name);
	}
	if (f->f_f != NULL) {
		if ((f->f_nextc = getwc(f->f_f)) != _WEOF || Multi == 'm')
			return (f->f_f);
		empty = MSGSTR(EMPTYF, "%s -- empty file");
		sprintf(s = (char *)getspace((UNS)(strlen(f->f_name)
						   + 1 + strlen(empty))), empty,
					     f->f_name);
		fclose(f->f_f);
	}
	Error = 1;
	if (Report)
		if (Ttyout) {	/* accumulate error reports */
			Lasterr =
			Lasterr->e_nextp = (ERR *)getspace((UNS)sizeof(ERR));
			Lasterr->e_nextp = NULL;
			Lasterr->e_mess = s;
		} else {	/* ok to print error report now */
			cerror(s);
			putc('\n', stderr);
		}
	return ((FILE *)NULL);
}


/*
 * NAME: getspace
 *
 * FUNCTION: get more space from memory
 */
ANY *
getspace(UNS n)
{
	ANY	*t;

	if ((t = (ANY *)malloc((size_t)n)) == NULL)
		die(MSGSTR(NOCORE, "out of space"));
	return (t);
}


/*
 * NAME: die
 *
 * FUNCTION: print error messages and exit
 */
die(char *s)
{
	++Error;
	errprint();
	cerror(s);
	putwc('\n', stderr);
	exit(1);
}


/*
 * NAME: onintr
 *
 * FUNCTION: on interrupt print error messages and exit
 */
onintr(void)
{
	++Error;
	errprint();
	INTREXIT(1);
}


/*
 * NAME: errprint
 *
 * FUNCTION:  print accumulated error reports
 */
errprint(void)
{
	fflush(stdout);
	for ( ; Err != NULL; Err = Err->e_nextp) {
		cerror(Err->e_mess);
		putc('\n', stderr);
	}
	done();
}


