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
static char rcsid[] = "@(#)$RCSfile: nl.c,v $ $Revision: 4.2.7.6 $ (DEC) $Date: 1993/12/13 13:43:41 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: nl
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.12  com/cmd/files/nl.c, cmdfiles, bos320, 9132320b 7/26/91 09:54:23"
 */

#include <stdio.h>	/* Include Standard Header File */
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <locale.h>

#include <ctype.h>
#include <regex.h>

#include "nl_msg.h"
nl_catd catd;
#define MSGSTR(N,S)	catgets(catd,MS_NL,N,S)

#define CONVSIZ		128  /* size of arg array buffer */
#define PATSIZ		128  /* size of buff used for regexp pattern */
#define SEPSIZ		128  /* buf used for separator and/or argv's */

char nbuf[100];         /* Declare buf size used in convert/pad/cnt routines */

char header = 'n';	/* 'n' - default line numbering */
char body   = 't';  	/* 't' - default doesn't number blank lines */
char footer = 'n';	/* 'n' - default line numbering */

int hbf_flags;
char *hbf_types[3]={&header, &body, &footer};
regex_t hbf_pats[3];

char s1[CONVSIZ];       /* Declare the conversion array */
char format = 'n';      /* Declare the format of numbers to be rt just */
char delim1[MB_LEN_MAX+1];
char delim2[MB_LEN_MAX+1];
char pad = ' ';         /* Declare the default pad for numbers */
char *s;                /* Declare the temp array for args */
int width = 6;          /* Declare default width of number */
int k;                  /* Declare var for return of convert */
int r;                  /* Declare the arg array ptr for string args */
int q = 2;              /* Initialize arg pointer to drop 1st 2 chars */

int mb_cur_max, 
    regstat;
/* GZ001. The wide string to keep converted input line when -b t option
 * is processed */
#define WCS_SIZE        (BUFSIZ * sizeof(wchar_t))
wchar_t *wcs;

/*
 * NAME: nl [-htype] [-btype] [-ftype] [-vstart#] [-iincr] [-p]
 *          [-l num] [-s sep] [-w width] [-n format] [-d delim] file
 *                                                                    
 * FUNCTION:  Numbers lines in a file.  Input must be written in logical
 *     pages.  Each logic page has a header, body and footer section
 *     ( you can have empty sections).  To mark the different sections
 *     the first line of the section must contain delimiters only. Default:
 *     Line contents    Start of
 *     \:\:\:           Header
 *     \:\:             Body
 *     \:               Footer
 *
 *     FLAGS:
 *      -btype      chooses which body section lines to number
 *                  a   number all lines
 *                  t   do not number blank lines (default)
 *                  n   do not number any lines
 *                  ppattern   number only lines containing pattern
 *      -dxx        Usess xx as the delimiter for the start of logical
 *                  page sections.
 *      -ftype      Chooses which logical page footer lines to number
 *                  same types as in -b.
 *      -htype      Chooses which logical page header line to number
 *                  same types as in -b.
 *      -inum       Increments logical page line numbers by num.
 *                  default value of num is 1.
 *      -lnum       num is the number of blank lines to count as one.
 *                  can only be used in documnets where the -ba flag
 *                  is used.
 *      -nformat    Uses format as the line numbering format.
 *                  ln  left justified, leading zeroes suppressed.
 *                  rn  right justified, leading zeroes suppressed. (default)
 *                  rz  right justified, leading zeroes kept.
 *      -p          does not restart numbering a logicial page delimiters.
 *      -s[sep]     separates the text from its line number by the sep
 *                  character. default sep = \t.
 *      -vnum       sets the initial logical page line number to num.
 *      -wnum       uses num as the number of characters in the line number.
 *                  default num=6.
 */  
main(argc,argv)
int argc;
char *argv[];
{
	int j;
	int i = 0;
	char *p;
	char line[BUFSIZ];
	char tempchr;	/* Temporary holding variable. */
	int temp1;	/* another temp holding variable */
	char swtch = 'n';
	char cont = 'n';
	char prevsect;
	char type;
	int cnt;	/* line counter */
	char sep[SEPSIZ];
	char pat[PATSIZ];
	char *ptr ;
	int startcnt=1;
	int increment=1;
	int blank=1;
	int blankctr = 0;
	int c;
	int lcv1, lcv2;
	FILE *iptr=stdin;
	FILE *optr=stdout;
	int len1,len2,seccnt;		/* temp. vars. */
	char *ct;

	sep[0] = '\t';
	sep[1] = '\0';
	opterr = 0;			/* we print our own errors */

	(void) setlocale(LC_ALL,"");     /* set tables up for current lang */
	catd = catopen(MF_NL,NL_CAT_LOCALE);

	mb_cur_max = MB_CUR_MAX;

	/* GZ001 If current locale is multibyte we need a buffer
	 * to convert input lines in search of visual characters */
	if (mb_cur_max > 1) {
	 wcs = (wchar_t *)malloc(WCS_SIZE);
	 if ( wcs == NULL ) {
	   fprintf(stderr,MSGSTR(NOMEM, "nl: OUT OF MEMORY - PROCESSING TERMINATED\n"));
	   exit(1);
	 }
	}

	delim1[0] = '\\'; delim1[1] = '\0';	/* Default delimiters. */
	delim2[0] = ':';  delim2[1] = '\0'; 	/* Default delimiters. */

/*		DO WHILE THERE IS AN ARGUMENT
		CHECK ARG, SET IF GOOD, ERR IF BAD	*/

	/*
	 * trick getopt so -s[sep] always has an argument
	 */
	{
		char **av;

		for (av = argv; *av != NULL; av++) {
			if (!strcmp(*av, "-s"))
				*av = "-s\177";
			if (!strcmp(*av, "-ps"))
				*av = "-ps\177";
		}
	}
				
	while((c=getopt(argc, argv, "h:b:f:v:i:pl:s:w:n:d:"))!=EOF) {
		hbf_flags = 2;
		switch(c) {
		case 'h':      /* header lines */
			hbf_flags--;
		case 'b':      /* body lines */
			hbf_flags--;
		case 'f':     /* footer lines */
			settypes(c, argv);
			break;
		case 'p':     /* do not restart numbering */
			cont = 'y';
			break;
		case 'v':      /* set initial logical page number */
		case 'i':      /* set lin incrementor value */
		case 'w': /* set num of spaces used for line number */
		case 'l': /* set num of blank lines to count as one */
			for (ptr=optarg; (*ptr>='0')&&(*ptr<='9'); ptr++);
			if (*ptr != '\0')
				optmsg(optarg);
			switch(c) {
				case 'v' : startcnt = atoi(optarg);  break;
				case 'i' : increment = atoi(optarg); break;
				case 'w' : width = atoi(optarg);     break;
				case 'l' : blank = atoi(optarg);     break;
				}
			break;
		case 'n':   /* numbering format */
			if (strcmp(optarg, "ln")==0) {
				format = 'l';
				continue;
				}
			if (strcmp(optarg, "rn")==0) {
				format = 'r';
				continue;
				}
			if (strcmp(optarg, "rz")==0) {
				format = 'z';
				continue;
				}
			if (argv[optind-1][0]!='-') {
				*(optarg-1) = ' ';
				optmsg(argv[optind-2]);	/* fatal error */
				}
			optmsg(argv[optind-1]);		/* fatal error */
			break;
		case 's':     /* set separator character */
			if (*optarg == 0177)
				sep[0] = '\0';
			else {
			    if (strlen(optarg)>127) {
				fprintf(stderr,MSGSTR(SEPTOOLONG, "SEPARATOR TOO LONG - PROCESSING TERMINATED\n"));
				exit(1);
			    }
			    strcpy(sep, optarg);
			}
			break;
		case 'd': /* set delimiter */
			if((lcv1=mblen(optarg, mb_cur_max))>0)
				strncpy(delim1, optarg, lcv1);
			else {
				fprintf(stderr,MSGSTR(INVDELIM1, "Invalid 1st delimiter.\n"));
				exit(1);
				}
			optarg += lcv1;
                        if (*optarg != '\0') { /* more than one delimiter */
			    if((lcv2=mblen(optarg, mb_cur_max))>0)
				strncpy(delim2, optarg, lcv2);
			    else {
				fprintf(stderr,MSGSTR(INVDELIM2, "Invalid 2nd delimiter.\n"));
				exit(1);
		 	    }
			    if (*(optarg+lcv2) != '\0') {
				*(optarg-lcv1-1) = ' ';
				optmsg(argv[optind-2]);
			    }
                        }
			break;
		default:
			optmsg(&optopt);
			break;
		}
	}

	if (argv[optind]!=NULL && (iptr = fopen(argv[optind],"r")) == NULL)  {
		fprintf(stderr,MSGSTR( CANTOPEN, "CANNOT OPEN FILE %s\n"), argv[optind]); /*MSG*/
		exit(1);
		}

	/* On first pass only, set line counter (cnt) = startcnt and set
	   the initial type to body (not header, body, nor footer) */

	cnt = startcnt; 
	type = body;
	prevsect = '\0';

/*		DO WHILE THERE IS INPUT
		CHECK TO SEE IF LINE IS NUMBERED,
		IF SO, CALCULATE NUM, PRINT NUM,
		THEN OUTPUT SEPERATOR CHAR AND LINE	*/

	while (( p = fgets(line,(int)sizeof(line),iptr)) != NULL) {
		ct = p;
		for(seccnt = 0; seccnt < 3; seccnt++) {
			len1=mblen(ct, mb_cur_max);
			len2=mblen(ct+len1, mb_cur_max);
			if (delim2[0]=='\0') {
				if (strncmp(ct, delim1, len1)==0)
					ct += len1;
				else
					break;
			}
			else {
				if ((strncmp(ct, delim1, len1)==0)  &&
			  	  (strncmp(ct+len1, delim2, len2)==0))
					ct += len1 + len2;
				else
					break;
			}
		}
		if (seccnt>0 && *ct == '\n') {
			swtch = 'y';
			switch (seccnt) {
			case 1:
				if ((prevsect != 'h') && (prevsect != 'b') && (cont != 'y'))
					cnt = startcnt;
				prevsect = 'f';
				type = footer;
				break;
			case 2:
				if ((prevsect != 'h') && (cont != 'y'))
					cnt = startcnt;
				prevsect = 'b';
				type = body;
				break;
			case 3:
				if ( cont != 'y' )
					cnt = startcnt;
				prevsect = 'h';
				type = header;
				break;
			default:
				break;
			}
		}
		if (swtch == 'y') {
			swtch = 'n';
			fprintf(optr,"\n");
			}
		else {
			hbf_flags = 2;
			switch(type) {    /* check if line is numbered */
				case 'n':
					npad(width,sep);
					break;
				case 't':
                        /* GZ001. if there is at least one character with
                         * visible represantation we count this line */
                                	if ( isgraph_in_line(p) ) {
						pnum(cnt,sep);
						cnt+=increment;
					}
					else 
						npad(width,sep);
					break;
				case 'a':
					if (p[0] == '\n') {
						blankctr++;
						if (blank == blankctr) {
							blankctr = 0;
							pnum(cnt,sep);
							cnt+=increment;
							}
						else npad(width,sep);
						}
					else {
						blankctr = 0;
						pnum(cnt,sep);
						cnt+=increment;
						}
					break;
				case 'h':
					hbf_flags--;
				case 'b': 
					hbf_flags--;
				case 'f':
					if (ct=strchr(p, '\n'))
						*ct = '\0';
					if((regexec(&hbf_pats[hbf_flags], p, (size_t) 0, (regmatch_t *) NULL, 0))==0) {
						pnum(cnt,sep);
						cnt+=increment;
						}
					else  {
						npad(width,sep);
						}
					*ct = '\n';
					break;
				}
			fprintf(optr,"%s",line);

			}	/* Closing brace of "else" (~ line 307). */
	}	/* Closing brace of "while". */
	fclose(iptr);
}

/*
 * NAME: regerr
 *                                                                    
 * FUNCTION:	REGEXP ERR ROUTINE
 */  
regerr(c)
int c;
{
fprintf(stderr,MSGSTR( REGERR1, "%d This is the error code\n"),c); /*MSG*/
fprintf(stderr,MSGSTR( REGERR2, "Illegal Regular Expression\n")); /*MSG*/
exit(1);
}

/*
 * NAME: pnum
 *                                                                    
 * FUNCTION: convert integer to string.
 */  
pnum(n,sep)
int	n;
char *	sep;
{
	int	i;

		if (format == 'z') {
			pad = '0';
		}
	for ( i = 0; i < width; i++)
		nbuf[i] = pad;
		num(n,width - 1);
	if (format == 'l') {
		while (nbuf[0]==' ') {
			for ( i = 0; i < width; i++)
				nbuf[i] = nbuf[i+1];
			nbuf[width-1] = ' ';
		}
	}
		printf("%s%s",nbuf,sep);
}

/*
 * NAME: procname1
 *                                                                    
 * FUNCTION: 	Convert integer to character.
 *              IF NUM > 10, THEN USE THIS CALCULATE ROUTINE
 */  
num(v,p)
int v,p;
{
	if (v < 10)
		nbuf[p] = v + '0' ;
	else {
		nbuf[p] = (v % 10) + '0' ;
		if (p>0) num(v / 10,p - 1);
	}
}

/*
 * NAME: npad
 *                                                                    
 * FUNCTION: 	CALCULATE LENGTH OF NUM/TEXT SEPRATOR
 */  
npad(width,sep)
	int	width;
	char *	sep;
{
	int i;

	pad = ' ';
	for ( i = 0; i < width; i++)
		nbuf[i] = pad;
	printf("%s",nbuf);


	if(sep[0] == '\t')
		printf("\t");
	else
	for(i=0; i < strlen(sep); i++)
		printf(" ");
}

/*
 * NAME: optmsg
 *                                                                    
 * FUNCTION: print option error massage.
 */  
optmsg(option)
char *option;
{
	fprintf(stderr,MSGSTR( OPTINVAL, "INVALID OPTION (%s) - PROCESSING TERMINATED\n\tUsage: nl [-b Type] [-d Delimiter1Delimiter2] [-f Type] [-h Type]\n\t[-i Number] [-l Number] [-n Format] [-p] [-s Separator]\n\t[-v Number] [-w Number] [File]\n"),option); /*MSG*/
	exit(1);
}

settypes(class, argv)
char class;		/* 'h', 'b', or 'f' */
char **argv;
{
	switch(*optarg) {
	case 'n':
	case 't':
	case 'a':
		*hbf_types[hbf_flags] = *optarg;
		break;
	case 'p':
		*hbf_types[hbf_flags] = class;
		if ((regstat=regcomp(&hbf_pats[hbf_flags], optarg+1, 0))!=0)
			prntregerr(regstat, &hbf_pats[hbf_flags]);
		break;
	default:
		if (argv[optind-1][0]!='-') {
			*(optarg-1) = ' ';
			optmsg(argv[optind-2]);		/* fatal error */
			}
		optmsg(argv[optind-1]);			/* fatal error */
	}
	if (*optarg != 'p' && *(optarg+1) != '\0') {	/* -[h|b|f] <file> */
		*(optarg-1) = ' ';
		optmsg(argv[optind-2]);
	}
}

prntregerr(regstat, preg)
int regstat;
regex_t *preg;
{
	char *err_msg_buff;
	size_t sobuff;     /* size of buffer needed */

	sobuff = regerror(regstat, preg, 0, 0);
	err_msg_buff = (char *)malloc(sizeof(char)*sobuff);
	sobuff = regerror(regstat, preg, err_msg_buff, sobuff);

	fprintf(stderr, "%s\n", err_msg_buff);
	exit(2);
}

/* GZ001. isgraph_in_line returns 1 if there is a visible character in
 *      the string, returns 0 otherwise.
 */
int isgraph_in_line(p)
char *p;
{
 wchar_t *pwcs;
 char *pcs;
 size_t n;

 if (mb_cur_max == 1) {
  /* single byte locale */
  for ( pcs = p; *pcs; pcs++ )
    if ( isgraph( (int)(*pcs) ) )  return 1;
  }
  else {
   /* multybyte locale */
   n = mbstowcs( wcs, p, (size_t)WCS_SIZE );
   /* if invalid byte sequence was detected, let skip this line*/
   if (n == (size_t)-1) return 0;
   for ( pwcs = wcs; *pwcs != (wchar_t)0; pwcs++ )
     if ( iswgraph( (wint_t)(*pwcs) ) ) return 1;
  }

 return 0;
}
/* GZ001. end */
