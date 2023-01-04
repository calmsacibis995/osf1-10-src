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
static char rcsid[] = "@(#)$RCSfile: grep.c,v $ $Revision: 4.2.8.6 $ (DEC) $Date: 1993/10/11 17:08:25 $";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 *  1.32  com/cmd/scan/grep.c, cmdscan, bos320, 9144320j 10/31/91 10:41:45
 */

/*
 * grep -- print lines matching (or not matching) a pattern
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <wchar.h>
#include <sys/types.h>
#include <macros.h>
#include <locale.h>
#include <unistd.h>
#include "grep_msg.h"
#include <regex.h>
nl_catd	catd;
#define MSGSTR(n,s)	catgets(catd,MS_GREP,n,s)

int mb_cur_max;		/* const. value after start of program. */

int regstat;
int regflags = 0; 	/* regular expression flags */

/*
 * linked list needed for the fflag 
 * because it's possible to have more than
 * 1 pattern in a file
 */
struct regexpr {
	struct regexpr *next;
	regex_t regpreg;  	/* regular expression structure */
};

struct regexpr *s1;
struct regexpr *head;
struct regexpr *tail;

int wordcnt = 0;	/* current number of words in linked list 	  */
#define MAXWORDS 400    /* maximum number of words allowed in linked list */

#define ESIZE	(256*5)
#define BSIZE	512
#define BSPACE  10000                                           /* FPM001 */
#define NUMEXPS 20
#define LINE_MAX 4096	/* for ultrix compatibility. See OSF_QAR #5271,
			   this override the value in sys/limits.h
			   which is now only 2048 */

#ifdef _SBCS
int     flagset;                /* fast grep flag            */
static void fastexecute(char *);/* fast grep execution	     */
#endif 

long	lnum;			/* current line number       */ 
int	nflag;			/* Number lines              */
int	iflag;			/* ignore case.              */
int	bflag;			/* display block numbers     */
int	lflag;			/* list file names           */
int	cflag;			/* count line matches        */
int	hflag = 0;		/* suppress prnt. filenames  */
int	vflag;			/* the NOT case              */
int	sflag;			/* silent flag               */
int	qflag;			/* quiet flag                */
int	wflag;			/* match word only           */
int	eflag = 0;		/* use next expression       */
int 	xflag;			/* match an entire string    */
int 	Fflag = 0;		/* match fixed expression    */
int 	Eflag = 0;		/* extended regular express  */
int 	fflag = 0;		/* take pattern from file    */
int	nfile;			/* how many files to look at */
long	tln;			/* total lines that matched  */
int	nsucc=0;		/* Return code.  success?    */
long    blkno1;			/* block number              */
int	wgrep=0;		/* flag: which grep--grep, egrep, or fgrep   */
				/* grep=0, egrep=1, fgrep=2		     */
				/* if nonzero, use egrep's and fgrep's	     */
				/*  -s flag of displaying only error essages */
				/*  as opposed to grep's suppressing error   */
				/*  messages of nonexistant or unreadable    */
				/*  files. 	 		   	     */

#define GREP    0
#define EGREP   1
#define FGREP   2

#define	COMMONOPTS	"clqbhinsvwxye:f:" /* options common to grep family */

struct {int id; char msg[400]; }    /* Message id and default message text */
	usages[] = {
		{USAGE, "usage: %s [-E | -F] [-c|-l|-q] [-insvxbhwy] [-p parasep] -e pattern_list...\n\t[-f pattern_file...] [file...]\nusage: %s [-E | -F] [-c|-l|-q] [-insvxbhwy] [-p parasep] [-e pattern_list...]\n\t-f pattern_file... [file...]\nusage: %s [-E | -F] [-c|-l|-q] [-insvxbhwy] [-p parasep] pattern [file...]\n"},

		{EUSAGE, "usage: %s [-hisvwxy] [[-bn]|[c|l|q]] -e pattern_list...\n\t[-f pattern_file...] [file...]\nusage: %s [-hisvwxy] [[-bn]|[c|l|q]] [-e pattern_list...]\n\t-f pattern_file... [file...]\nusage: %s [-hisvwxy] [[-bn]|[c|l|q]] pattern [file...]\n"},

		{FUSAGE, "usage: %s [-hisvwxy] [[-bn]|[c|l|q]] -e pattern_list...\n\t[-f pattern_file...] [file...]\nusage: %s [-hisvwxy] [[-bn]|[c|l|q]] [-e pattern_list...]\n\t-f pattern_file... [file...]\nusage: %s [-hisvwxy] [[-bn]|[c|l|q]] pattern [file...]\n"},
		};

				/* Is it grep, egrep, or fgrep?  	     */
				/* For error displaying purposes.	     */
				/*  (egrep = "grep -E", fgrep = "grep -F")   */
char		*callname;
char		perrormsgstr[] = "egrep malloc ";    /* error message for perror() */
char		*perrormsg = perrormsgstr;
char    	*endspace;
char 		*escape();
long    	lnum1;
unsigned        lspace;
int     	pflag;
char    	*space;
int i;
regex_t 	*terms[NUMEXPS + 1];   /* paragraph delimiters */

size_t nmatch = 1;			/* used when checking for words */
regmatch_t  match_offsets[1];		/* used when checking for words */
regmatch_t *match_ptr;			/* used when checking for words */

static void execute(char *);
static int check_list(char *, char *);
static void succeed(char *, FILE *);
static void prntregerr(int, regex_t *);
       void errexit(char *, char *);
	int wordmatch(regmatch_t *offsets, char *);
       void insertpatterns(char *);
       void getpreg(regex_t *, char *);

FILE *wordf;

void
main(int argc, char *argv[])
{ 
	register int c;
	int errflg = 0;
	extern int optind;
	extern char *optarg;
	char *epatterns[MAXWORDS];
	char **ffiles;
	int ffiles_cnt = 10;
	char buf[LINE_MAX];
	char *p;
	char *opstring;		/* option string for getopt */

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_GREP,NL_CAT_LOCALE);

	mb_cur_max = MB_CUR_MAX;

	if (!(ffiles = (char **)malloc(ffiles_cnt * (sizeof(char *))))) {
		perror("grep:no memory");
		exit(2);
	}

	if (p = strrchr(argv[0],'/')) argv[0] = p + 1;

	callname = argv[0];

	switch(*callname) {
	case 'e' :
		wgrep = EGREP;
		Eflag++;
		opstring = COMMONOPTS;
		break;

	case 'f' :
		*perrormsg= 'f';
		wgrep = FGREP;
		Fflag++;
		opstring = COMMONOPTS;
		break;

	default  :  		/* assume it is grep */
		opstring = "EFp:" COMMONOPTS;
		perrormsg++;
	}

	while((c=getopt(argc, argv, opstring)) != -1) switch(c) {
		case 'v':
			vflag++;
#ifdef _SBCS
			flagset++;
#endif
			break;


		case 'i':
		case 'y':
			iflag++;
#ifdef _SBCS
			flagset++;
#endif
			break;

		case 'c':
			cflag++;
			if (lflag) errflg++;
#ifdef _SBCS
			flagset++;
#endif
			break;

		case 'n':
			nflag++;
#ifdef _SBCS
			flagset++;
#endif
			break;

		case 'b':
			bflag++;
#ifdef _SBCS
			flagset++;
#endif
			break;


		case 'h':
			hflag++;
			break;

		case 's':
			sflag++;
#ifdef _SBCS
			flagset++;
#endif
			break;

		case 'q':
			qflag++;
			break;

		case 'l':
			lflag++;
			if (cflag) errflg++;
#ifdef _SBCS
			flagset++;
#endif
			break;

		case 'w':
			wflag++;
#ifdef _SBCS
			flagset++;
#endif
			continue;

		case 'e':
			epatterns[eflag++] = optarg;
			continue;

		case 'p':
#ifdef _SBCS
			flagset++;
#endif
			if (pflag >= NUMEXPS)
				errexit(MSGSTR(PARG,			/*MSG*/
				"too many `p' arguments\n"), (char *) NULL);

			if ((terms[pflag] = (regex_t *)malloc(sizeof(regex_t))) == NULL)
			{
				perror (perrormsg);
				exit (2);
			}
		/* getopt allows no arguments after a minus argument or a
		   mandatory argument after a minus argument, but no optional
			   argument, so work around this problem */

			if (optarg != argv[optind-1]) {
				if ( (regstat = regcomp(terms[pflag], optarg, 0) ) != 0 )
					prntregerr(regstat, terms[pflag]);
			} else {
				if ( (regstat = regcomp(terms[pflag], "^$", 0) ) != 0 ) 
					prntregerr(regstat,terms[pflag]);
				else
					--optind;
			}
			++pflag;
			if (space == 0)
				if ((space = malloc((size_t)(lspace = BSPACE))) == NULL)
				{
					perror (perrormsg);
					exit (2);
				}
			break;

		case 'x': 
			xflag++;
			break;
		case 'F':
			if (wgrep != EGREP) {
				Fflag++;
				Eflag = 0;
				}
			break;
		case 'f':
			if (ffiles_cnt == fflag) {
				ffiles_cnt += 10;
				if (!(ffiles = (char **)realloc(ffiles,
					ffiles_cnt * (sizeof(char *))))) {
					perror("grep:no memory");
					exit(2);
				}
			}
			ffiles[fflag++] = optarg;
			continue;
		case 'E':
			if (wgrep != FGREP) {
				Eflag++;
				Fflag = 0;
				}
			break;
		case '?':
#ifdef _SBCS
			flagset++;
#endif
			errflg++;
		}

	argc -= optind;

	if(errflg || (argc <= 0 && !eflag && !fflag)) {
		fprintf(stderr, MSGSTR(usages[wgrep].id, usages[wgrep].msg), callname,
				callname, callname);
		exit(2);
	}
	if ( !eflag  && !fflag ) {
		epatterns[eflag++] = argv[optind++];
		nfile = --argc;
	}
	else
		nfile = argc;

	argv = &argv[optind];

	if (Eflag) {
		regflags |= REG_EXTENDED;
	}

	if (iflag) {
		regflags |= REG_ICASE;
	}
	/* Insert -f patterns from ffiles
	 */
	if (fflag)
		for (i=0; i<fflag; i++) {
			/* Always print msg for fatal conditions */
			if (!(wordf = fopen(ffiles[i], "r"))) {
				fprintf(stderr,MSGSTR(OPERR,
				"%s: can't open %s\n"), callname, ffiles[i]);
				exit(2);                             /*FPM001*/
			}
			while (fgets(buf, LINE_MAX, wordf) != NULL)
				insertpatterns(buf);
			fclose(wordf);
		}
	/* Insert -e patterns from epatterns
	 */
	for (i=0; i<eflag; i++)
		insertpatterns(epatterns[i]);

	if (space == 0) {
		if ((space = malloc((size_t)(lspace = LINE_MAX + 1))) == NULL)
		{
			perror (perrormsg);
			exit (2);
		}
        }
	endspace = space + lspace;                                  /*FPM001*/

	if (argc<=0) {
#ifdef _SBCS
		if (flagset == 0)
			fastexecute((char *)NULL);
		else
#endif
			execute((char *)NULL);
        }
        else while (--argc >= 0) {
#ifdef _SBCS
		if (flagset == 0)
			fastexecute(*argv++);
		else
#endif
			execute(*argv++);
	}
	exit(nsucc == 2 ? 2 : (nsucc == 0));  /* two means it's an error */
}

#ifdef _SBCS
/*
 * NAME: fastexecute
 *
 * FUNCTION:    Fast grep - print each line which matches user pattern.
 *		No options other than -q are allowed.
 *
 * RETURN VALUE: void
 *
 */

static void
fastexecute(char *file)
{
        FILE	*fp;		/* input stream */
	int	i;		/* fgets line length */
	int	match;

        if (file) {
                if ((fp = fopen(file, "r")) == NULL) {
                        if (!sflag || wgrep)   /* if wgrep==0, use grep's -s */
                                fprintf(stderr,(MSGSTR(OPERR, "%s: can't open %s\n")), callname, file);
			nsucc = (nsucc==1 && qflag)? 1:2;
                        return;
	        }
        } else
                fp = (FILE *)stdin;

        while (fgets(space, LINE_MAX, fp) != NULL)
		if (vflag ^ check_list(file, space))
			succeed(file, fp);


	if (fp != stdin)
	    fclose(fp);
}
#endif

/*
 * NAME: execute
 *                                                                    
 * FUNCTION:	For each file in the agument list, search each line
 *		for characters matching the user's pattern.  Multibyte
 *		charcters are converted if the iflag is set.
 *                                                                    
 * RETURN VALUE: void
 *
 */  

static void
execute(char *file)
{
	register char *p1;
	register int c;
	int match;
	int i;
	char *cp;
	int rbflag = bflag;
	FILE *fp;
	int chklline = 0;
	int nlflag = 0;
	char *linebuf,
	     *lbuffer,
	     linebuffer[LINE_MAX * 2  +  1];

	lbuffer = linebuffer;

	if (file) {
		if ((fp = fopen(file, "r")) == NULL) {
                        if (!sflag || wgrep)   /* if wgrep==0, use grep's -s */
				fprintf(stderr,(MSGSTR(OPERR, "%s: can't open %s\n")), callname, file);
			nsucc = (nsucc==1 && qflag)? 1:2;
			return;
		}
	} else
		fp = (FILE *)stdin;

	lnum = 0;
	tln = 0;
	match = 0;
	cp = space;
	lnum1 = 0;
	if ((c = getc(fp)) == EOF) {
	  	if (!cflag) {
		    if (fp != stdin)
			fclose(fp);
		    return;
		}
		goto countout;	/* must avoid 'success()' which bumps tln */
	} else
		ungetc(c, fp);

	for (;;) {
next:           lnum++;

                /* get more memory to store paragraph in if rqd */   /*FPM001*/
                if ((cp != space)  && ((endspace - cp) < (LINE_MAX + 3))) {
                    char *tmpspace = (char *) NULL;
                    int  offset    = cp - space;

                    lspace+=(BSPACE * 2);
                    if (!(tmpspace = realloc (space, (size_t) lspace))) {
                        perror("Realloc failed\n"); 
			exit (2);
                    }
             
                    space    = tmpspace;
	            endspace = space + lspace;
                    cp       = space + offset;
                }

		p1 = cp;
		linebuf = lbuffer;

		while (((c = getc(fp)) != '\n') && (c != '\0')) {
			if (c == EOF) {
				if (pflag && cp != space && (match ^ vflag)
				    && !chklline) 
					succeed(file, fp);

				/* Force last line with no newline to be checked
				 * for match . If last line has a newline, then
				 * check has already been made.
				 */
				if (chklline++ == 0 && !nlflag)
					goto checklast;
countout:
				if (cflag && !qflag) {
					if (nfile>1 && !hflag)
						printf("%s:", file? file:MSGSTR(STDIN,"(standard input)"));
					printf("%ld\n", tln);
				}
				if (fp != stdin)
				    fclose(fp);
				return;
			}
			nlflag = 0;


			*p1++ = c;
			*linebuf++ = c;

		  	if (p1 - cp >= LINE_MAX )  /* Test max char limit*/
			  	break;
		}

checklast:	nlflag++;
		*p1 = '\0';
		*linebuf = '\0';
		if (lnum1 == 0) {
			lnum1 = lnum;
			if (rbflag)
				blkno1 = (ftell(fp) - 1) / BSIZE;
		}

		if (pflag) {
			for (i = 0; terms[i]; ++i) {
				if ((regstat = regexec( terms[i], lbuffer, (size_t) 0, (regmatch_t *) NULL, 0) ) == 0 ) {
                                       *cp = 0; *lbuffer = 0;
					if (match ^ vflag && cp != space)
						succeed(file, fp);
					cp = space;
					lnum1 = 0;
					match = 0;
					goto next;
				}
			}
		}

		if (!match)
			match = check_list(file, lbuffer);

		if (pflag) {
			*p1++ = '\n';
			*linebuf = '\n';
		}
		*p1 = 0;
		*linebuf = 0;

		if (!pflag) {
			if (vflag ^ match)
				succeed(file, fp);
			lnum1 = 0;
			match = 0;
		} else {
			cp = p1;
			linebuf = lbuffer;
			if (cp >= endspace) {
				fprintf(stderr,(MSGSTR(OVFLO,		/*MSG*/
					"\n\nparagraph space overflow\n\n")), file);
                           exit(1);
			}
		}
	}
}

/*
 * NAME: succeed
 *                                                                    
 * FUNCTION:	A line was selected (note that this is *not* the same
 *		thing as saying that a match was found).  Now print
 *		out all the information	the user requested such as
 *		block number, file name, inc line count (printed at
 *		end only), or do nothing.
 *
 * Note: If a selection was made and the q flag is on, then the exit status should
 *  	 be zero even if an access or read error was detected on an earlier
 *	 file, so mark nsucc as having matched (i.e. set it to 1).  Also the -q
 *	 option specifies that nothing be printed to the standard output.
 *                                                                   
 * (Globals:) 
 *	nsucc is set to one if there hasn't been any previous errors.
 *	tln  is incremented.
 *
 */  

static void
succeed(char *file, FILE *fp)
{
	nsucc = (qflag || nsucc != 2) ? 1 : 2;

	if (cflag) {
		tln++;
		return;
	}
	if (lflag) {
		if (!qflag)
			printf("%s\n", file? file:MSGSTR(STDIN,"(standard input)"));
		fseek(fp, 0l, 2);
		return;
	}
	if (qflag || sflag && wgrep)	/* f/egrep, -s displays ONLY error */
		return;			/*   messages.			   */

	if (nfile > 1 && !hflag)
		printf("%s:", file? file:MSGSTR(STDIN,"(standard input)"));
	if (bflag)
		printf("%ld:", blkno1);
	if (nflag)
		printf("%ld:", lnum1);
	if (pflag && (nfile > 1 || nflag || bflag))
		putchar('\n');
	printf("%s\n", space);
}

/* NAME: prntregerr
 *
 * FUNCTION:    Print the error message produced by regerror().
 *
 */
void
prntregerr(int regstat, regex_t *preg)
{
	char *err_msg_buff;
	size_t sobuff;     /* size of buffer needed */

	sobuff = regerror(regstat, preg, 0, 0);
	err_msg_buff = (char *)malloc(sizeof(char)*sobuff);
	sobuff = regerror(regstat, preg, err_msg_buff, sobuff);

	fprintf(stderr, "%s\n", err_msg_buff);
	exit(2);
}

/*
 * NAME: errexit
 *                                                                    
 * FUNCTION: Error printed and program terminates.
 *                                                                    
 * RETURN VALUE:  Terminates with exit code 2
 *		
 */  
void
errexit(char *s, char *f)
{
	fprintf(stderr, s, callname, f);
	exit(2);
}

/* 
 * NAME: check_list
 *
 * FUNCTION:  This function will check to see if ANY of the patterns matches
 *		the input string (instring).
 *
 * RETURN VALUE: 1 = Matched.
 *		 0 = Did not match.
 *
 * Note: If a match is found and the q flag is on, then the exit status should
 *  	 be zero even if an access or read error was detected on an earlier
 *	 file, so mark nsucc as having matched (i.e. set it to 1).  Also the -q
 *	 option specifies that nothing be printed to the standard output.
 */
static
int
check_list(char *file, char *instring)
{
	char *tmpptr;

	match_ptr = match_offsets;
	nmatch = 1;

	s1 = head;
	if (!wflag) {
		match_ptr = (regmatch_t *) NULL;
		nmatch    = 0;
	}
	if ((tmpptr = strchr(instring,'\n')) != NULL) 
		*tmpptr++ = '\0';
	while (s1 != NULL) {
		if (  !(regstat = regexec(&s1->regpreg, instring, nmatch, match_offsets, 0))    &&     (!wflag || wordmatch(match_offsets, instring))  )  {
			/* nsucc = (qflag || nsucc!=2)? (!vflag):2; */
			return (1);
		}
		tmpptr=instring;
		while (tmpptr && *tmpptr != '\0') {
			match_offsets[0].rm_so=match_offsets[0].rm_eo=-1;
			regstat=regexec(&s1->regpreg,tmpptr,nmatch,match_offsets,0);
			if (!regstat && 
			   (!wflag || wordmatch(match_offsets, tmpptr)))
				return(1);	/* found a match */
			tmpptr = (match_offsets[0].rm_eo != -1) ?
				    &tmpptr[match_offsets[0].rm_eo] : NULL;
		}
		s1 = s1->next;
	}
	return (0);
}

/*
 * NAME: wordmatch
 *                                                                    
 * FUNCTION:  Tests to see if the substring that was matched is a word.
 *		A word is a substring that is delimited on the left by
 *		either the beginning of line or a non-alphanumeric char-
 *		acter and on the right by either the end of line or
 *		a non-alphanumeric character.
 *                                                                    
 * RETURN VALUE:  0 - No match.  Is not a "word".
 *		  1 - Matched.  The substring found matches the pattern
 *			being sought and is therefor a word.
 *		
 * NOTES:  This function relies on offsets[] which gets its values from
 *	   	regexec().  regexec() is only required to do this for
 *		regular expressions but the current implementation will
 *		do it regardless.  In case of problems with word matching
 *		when using extended regular expressions, check to see if
 *		offsets[] is getting its values correctly.
 */  

int
wordmatch(regmatch_t offsets[], char *string)
{ 
	int s1 = offsets[0].rm_so;  	/* start of substr. matched	*/
	int s2 = offsets[0].rm_eo;	/* start of remainder of string	*/
	char *p1, *p2;
	int  tmplen, bol=0, eol=0;
	wchar_t pmbc, nmbc;

	if (mb_cur_max == 1) {  	/* for single-byte locales FPM002 */
            int lok = 0, rok = 0;

            if ((string[s2] == 0) ||
                (!isalnum(string[s2]) && (!wflag || string[s2] != '_')))
                rok = 1;

            if (!s1 ||
                (!isalnum(string[s1-1]) && (!wflag || string[s1-1] != '_' )))
                lok = 1;
           
            return (lok && rok);
        }							/*FPM002*/

					/* for multi-byte locales	*/
	pmbc = nmbc = (wchar_t) 0;
	p1 = p2 = &string[s1];

	if (s1) {			/* find previous mb character	*/
		p1 = (p1 - mb_cur_max >= string)? p1 - mb_cur_max : string;
		for(; p1 + (tmplen = mblen(p1, mb_cur_max)) < p2; p1 += (tmplen>0)? tmplen : 1);
		mbtowc(&pmbc, p1, mb_cur_max); 
	} else				/* at beginning of line		*/
		bol++;

	p2 = &string[s2];
	if (*p2 == '\0')		/* at end of line 		*/
		eol++;
	else
		mbtowc(&nmbc, p2, mb_cur_max); 
		
	return ((bol || (!iswalnum(pmbc) && pmbc != (wchar_t)'_')) && /*FPM002*/
                (eol || (!iswalnum(nmbc) && nmbc != (wchar_t)'_')));  /*FPM002*/
}

/*
 * NAME: insertpatterns
 *                                                                    
 * FUNCTION:  Inserts an entry into the linked list which contains the patterns
 *		that may be matched by the input.  This list is used when more
 *		than one pattern will be supplied by the user.  This will
 *		happen when either the -f is used specifying a file of patterns,
 *		or when a list of patterns is quoted at the command line and
 *		separated by newlines.
 *		e.g.:
 *			$ grep 'hello
 *			world
 *			3rd pattern' inputfile
 *		or
 *			$ grep -e 'hello
 *			world
 *			3rd pattern' inputfile
 *		or
 *			$ grep -f patternfile inputfile
 *                                                                    
 *		Note:  insertpatterns() takes care of taking each of the 
 *			<newline> separated patterns and inserting them as 
 *			individual entries in the linked list.
 *
 * RETURN VALUE:  none.  Any errors that may occur are fatal.
 *		
 */
void
insertpatterns(char *buffer)
{
	char *ptr;

	if (!buffer) return;

	do {
		if (++wordcnt > MAXWORDS) {
			fprintf(stderr,MSGSTR(OVERFLO,"list too large\n"), callname);
			exit(2);
		}
		if ( (s1 = (struct regexpr*) malloc ( sizeof( struct regexpr))) == NULL) {
			perror(perrormsg);
			exit(2);
		}
		if (head == NULL) {
			head = s1;
			tail = s1;
		} else {
			tail->next = s1;
			tail = s1;
		}
		s1->next = NULL;
				
		if ((ptr = strchr(buffer,'\n')) != NULL) {
			*ptr++ = '\0';
		}

		getpreg(&s1->regpreg, buffer);
		buffer = ptr;
	} while (buffer && *buffer);
}

/*
 * NAME: getpreg
 *                                                                    
 * FUNCTION:  This function is an interface to regcomp().  It takes a null
 *		terminated pattern and compiles it.  Note that if there are
 *		any <newline>'s in the pattern, the pattern will be taken to be
 *		the string up to the character just before the first <newline>.
 *	      If the -F option (fixed string) is specified, all special
 *		characters are escaped before sending the string to regcomp().
 *	      If the -x option was specified, a circumflex (^) is added at the
 *		begining of the pattern and a dollar sign ($) is added at the
 *		end so that it will only match if the entire line is used up in
 *		the match.  If compiling ERE's, then | would mess up the precedence
 *		so, the whole regular expression is parenthesized, ala
 *			^(ERE)$
 *
 * RETURN VALUE:  none.  Any errors that may occur are fatal.
 *		
 */
void
getpreg(regex_t *localpreg, char *localpattern)
{
	char	protectedbuf[LINE_MAX * 2], 	/* Allow for backquoting each char */
		*protbufptr = &protectedbuf[0];
	unsigned char *ptr;
	int	clen;

	if ((ptr = (unsigned char *)strchr(localpattern, '\n')) != NULL)
		*ptr = '\0';

	if (xflag || Fflag) {
		if (xflag) {
			*protbufptr++ = '^';
			if (Eflag) *protbufptr++ = '(';
		}
		ptr = (unsigned char *)localpattern;
		while(*ptr) {
		    /*
		     * INVARIANT: ptr always points to the first byte of a
		     * (possibly multi-byte) character.
		     */
			if (Fflag) 
				switch(*ptr) {
					case '\\':
					case '[' :
					case '*' :
					case '?' :
					case '+' :
					case '.' :
					case '^' :
					case '$' :
						*protbufptr++ = '\\';
						*protbufptr++ = *ptr++;
						break;
					default:
						clen = mblen((char *)ptr,mb_cur_max);
						if (clen <= 0) clen = 1;
						for(;clen;clen--)
						    *protbufptr++ = *ptr++;
				}
			else
			    *protbufptr++ = *ptr++;
		}

		if (xflag) {
			if (Eflag) *protbufptr++ = ')';
			*protbufptr++ = '$';
		}

		*protbufptr = '\0';
		localpattern = protectedbuf; 
	}

	if ((regstat = regcomp(localpreg, localpattern, regflags) ) != 0)
		prntregerr(regstat, localpreg);
}

