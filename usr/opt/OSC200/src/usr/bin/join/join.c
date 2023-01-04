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
static char rcsid[] = "@(#)$RCSfile: join.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/11/19 18:10:43 $";
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
 * FUNCTIONS: join
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.14  com/cmd/files/join.c, cmdfiles, bos320, 9128320 6/25/91 09:02:21"
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<locale.h>
#include "join_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_JOIN, Num, Str)

extern char *optarg;
extern int optind;

#define F1 0
#define F2 1
#define	NFLD  1024	/* max field per line */
#define comp() strcoll(ppi[F1][j1],ppi[F2][j2])

FILE	*f[2];		/* file descriptors */
char buf[2][LINE_MAX];	/* input lines */
char	*ppi[2][NFLD];	/* pointers to fields in lines */
int	j1	= 1;	/* join of this field of file 1 */
int	j2	= 1;	/* join of this field of file 2 */
int	mb_cur_max;	/* local copy of MB_CUR_MAX */
int	multibyte;	/* multibyte path flag */
wctype_t blankhandle;	/* locale specific <blank> */
int	olist[2*NFLD];	/* output these fields */
int	olistf[2*NFLD];	/* from these files */
int	no;		/* number of entries in olist */
int	sep;		/* -t field separator */
char	*null   = "";	/* -e default string */
int	aflg;		/* common and non-matching flag */
int	vflg;		/* non-matching only flag */
int	swapped;	/* remember arg reordering for output */

/*
 * NAME: join [-afilenumber] [-v filenumber] [-e string] [-[j]filenumner field] [-o n.m[,n.m ...]] 
 *            [-tseparatorchar] file1 file2
 * FUNCTION: Joins data fields of two fields.
 */
main(argc, argv)
char *argv[];
{
	register int i;
	register int n1, n2;
	long top2, bot2;
	int c;
	int ofile,ofield;
	char *optr;

	setlocale(LC_ALL, "");
	catd = catopen(MF_JOIN, NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;
	multibyte = mb_cur_max != 1;
	blankhandle = wctype("blank");

	/* Process  join  options including POSIX "obsolescent" forms of
	 * -j and -o that do not follow POSIX command syntax guidelines.
	 */
	vflg = 0;
	no = 0;
	while ((c=getopt(argc,argv,"a:v:e:t:j:1:2:o:")) != EOF) {
	  switch(c){
	  case 'a':
	  case 'v':
		if ((strlen(optarg)>1)
		    || ((optarg[0] != '1') && (optarg[0] != '2')))
			Usage();
		if (optarg[0] == '1') 
		  if (c=='a') aflg |= 1;
		  else vflg |= 1;
		else 
		  if (c=='a') aflg |= 2;
		  else vflg |= 2;
		break;
	  case 'e':
		null = optarg;
		break;
	  case 't':
		if (multibyte) {
			wchar_t wc;
			if (mbtowc(&wc, optarg, mb_cur_max) < 1)
				error(MSGSTR(BADSEP, "join: Invalid -t separator character\n"), NULL);
			sep = wc;
		} else
			sep = optarg[0];
		break;
	  case 'j':
		/* IF optarg is "1\0" or "2\0" immediately preceded by "-j"
		 * THEN process "-jn m"  where optarg is n
		 * ELSE process "-j m"  where optarg is m.
		 */
		if ( ((optr=optarg-2)[0]=='-') && (optr[1]=='j')
		     && ((optarg[0]=='1') || (optarg[0]=='2'))
		     && (optarg[1]=='\0')) 
		{ /* Process "-jn m" with optarg=n  as if it was "-n m"
		   * with optarg=m .
		   */
			/* Use character  n  as option-char c in case c=='1'
			 * and  case c=='2' following.
			 */
		  c = optarg[0];
                        /* If no argument after n, display usage
                         * error message.
                         */
                  if ((argc - optind) < 1)
                    Usage();
			/* If NEXT argument after n begins with '-' it
			 * is syntax error: omitted  m  or negative  m.
			 * It would look like an option to getopt() .
			 */
		  if (argv[optind][0] == '-') Usage();
			/* Get  m  as optarg, and move getopt() pointer
			 * up so getopt() resumes parsing at the argument
			 * following  -jn m .
			 */
		  optarg=argv[optind++];
		}
	  case '1': /* Equivalent to pre-POSIX  -j1 m */
	  case '2': /* Equivalent to pre-POSIX  -j2 m */
		optr = optarg;
		i = strtol(optarg,&optr,10);
			/* If strtol didn't find a nonempty string ending
			 * with '\0', the  m  argument was not valid.
			 */
		if ((i==0) && ((optr==optarg) || (optr[0]!='\0'))) Usage();
		if (i < 1 || i > NFLD)
			error(MSGSTR(BADFLDNUM,"join: Invalid field number %s\n"), optarg);
		if (c != '1') j2 = i; /* if c=2 or c=j */
		if (c != '2') j1 = i; /* if c=1 or c=j */
		break;
	  case 'o':
		/* Process zero or more -o lists, each of which is a string
		 * of one or more specifiers of the form "1.m" or "2.m",
		 * separated by commas, blanks or tabs within a string, where
		 * each m is a nonnegative decimal integer.
		 */
			/* While (next argument begins with "1.m" or "2.m"
			 *        and the field table has not overflowed)
			 * {process next argument as a  -o list parameter}.
			 */
		while( (optarg != NULL)
			&& (strlen(optarg)>2) && (optarg[1]=='.')
			&& ((optarg[0]=='1') || (optarg[0]=='2'))
			&& (no<2*NFLD) ) 
		{
		  olistf[no] = ((optarg[0]=='1')? F1 : F2);
		  olist[no++] = strtol((optr=&optarg[2]),&optarg,10) -1;
			/* If m was not a nonempty string terminated by 
			 * either ',', ' ', '\t' or '\0', syntax error in -o list.
			 */
		  if( (optr==optarg) 
			|| ((optarg[0]!='\0') && (optarg[0]!=',') && (optarg[0]!=' ')
			   && (optarg[0]!='\t')) ) Usage();
			/* If m did not end at a NUL continue next argument in
			 * this -o list; otherwise consider the next
			 * argument as (possible) next -o list.
			 */
		  if((optarg++)[0]=='\0')
		    optarg = argv[optind++];
		}
			/* Leave getopt() pointer on first argument not used
			 * as a  -o list .
			 */
		if (no == 0) /* no entries found for olist arg */
			Usage();
		optind--;
		break;
	  default:
		Usage();
		break;
	  } /* switch c */
	} /* while c is next valid option character */
	j1--;
	j2--;
	/* Process file parameters */
	/* Either but not both can be "-" meaning stdin */
	if (optind+2 != argc) 
		Usage();
	if (strcmp(argv[optind],"-") == 0)
		f[F1] = stdin;
	else if ((f[F1] = fopen(argv[optind],"r")) == NULL)
		error(MSGSTR(CANTOPEN,
			"join: Cannot open file %s\n"), argv[optind]);
	if (strcmp(argv[++optind],"-") == 0) {
		if (f[F1] == stdin)
			error(MSGSTR(BOTHSTDIN,
				"join: Both files cannot be \"-\"\n"), NULL);
		/* switch file1 and file2 - remember swap for later */
		else {
			f[F2] = f[F1];
			f[F1] = stdin;
			i = j1;
			j1 = j2;
			j2 = i;
			if (aflg == 1 || aflg == 2)
				aflg ^= 3;
			if (vflg == 1 || vflg == 2)
				vflg ^= 3;
			for (i=0; i<no; i++)
				olistf[i] ^= 1;
			swapped = 1;
		}
	} else if ((f[F2] = fopen(argv[optind],"r")) == NULL)
		error(MSGSTR(CANTOPEN,
			"join: Cannot open file %s\n"), argv[optind]);

#define get1() n1=input(F1)
#define get2() n2=input(F2)

	get1();
	bot2 = ftell(f[F2]);
	get2();
	while(n1>0 && n2>0 || ((aflg!=0 || vflg!=0) && n1+n2>0)) {
		if(n1==0 || n2>0 && comp()>0) {
			if((aflg&2)|(vflg&2)) output(0, n2);
			bot2 = ftell(f[F2]);
			get2();
		} else if(n2==0 || n1>0 && comp()<0) {
			if((aflg&1)|(vflg&1)) output(n1, 0);
			get1();
		} else /*(n1>0 && n2>0 && comp()==0)*/ {
			if(vflg==0) {
				while(n2>0 && comp()==0) {
					output(n1, n2);
			       		top2 = ftell(f[F2]);
					get2();
				}
				fseek(f[F2], bot2, 0);
				get2();
				get1();
				for(;;) {
					if(n1>0 && n2>0 && comp()==0) {
						output(n1, n2);
						get2();
					} else if(n2==0 || n1>0 && comp()<0) {
						fseek(f[F2], bot2, 0);
						get2();
						get1();
					} else /*(n1==0 || n2>0 && comp()>0)*/{
						fseek(f[F2], top2, 0);
						bot2 = top2;
						get2();
						break;
					}
				}
			} else {
				optr = (char *)strdup(ppi[F1][j1]);
				do
					get1();
				while (n1 > 0 && strcoll(optr,ppi[F1][j1]) == 0);
				do {
					bot2 = ftell(f[F2]);
					get2();
				} while (n2 > 0 && strcoll(optr,ppi[F2][j1]) == 0);
				free (optr);
			}
		}
	}
	return(0);
}

/*
 * get input line and split into fields
 * returns zero on EOF, otherwise returns number of fields
 */
input(n)
{
	register int i;
	register int c;
	register char *bp;
	register char **pp;
	int wclen;
	wchar_t wc;

	bp = buf[n];
	pp = ppi[n];
	if (fgets(bp, LINE_MAX, f[n]) == NULL)
		return(0);
	i = 0;
	if (sep == 0) {
		do {
			if (++i > NFLD)
				error(MSGSTR(FLDOVFLO, "join: Too many fields in line %s\n"), buf[n]);
			/* Skip <blank>s in locale as per sort -b
			 */
			if (multibyte) {
				while ((wclen = mbtowc(&wc, bp, mb_cur_max)) > 0
				       && iswctype(wc, blankhandle))
					bp += wclen;
				*pp++ = bp;	/* record beginning of field */
				while (*bp != '\n'
				       && (wclen=mbtowc(&wc,bp,mb_cur_max)) > 0
				       && !iswctype(wc, blankhandle))
					bp += wclen;
			} else {
				while (iswctype((wchar_t)*bp, blankhandle))
					bp++;
				*pp++ = bp;	/* record beginning of field */
				while (!iswctype((wchar_t)*bp, blankhandle)
				       && *bp != '\n' && *bp != '\0')
					bp++;
			}
			/* we are at the end of field (may be end of line)
			 */
			c = *bp;
			*bp++ = '\0';	/* mark end by overwriting separator */
		} while (c != '\n' && c != '\0');
	} else {
		do {
			if (++i > NFLD)
				error(MSGSTR(FLDOVFLO, "join: Too many fields in line %s\n"), buf[n]);
			*pp++ = bp;	/* record beginning of field */
			if (multibyte && sep >= 0x40) {
				while (1) {
					wclen = mbtowc(&wc, bp, mb_cur_max);
					if (wclen > 0) {
						if (wc == sep || wc == '\n')
							break;
						else
							bp += wclen;
					} else if (wclen == 0) {
						wclen = 1;
						wc = '\0';
						break;
					} else
						bp++;
				}
				*bp = '\0';
				bp += wclen;
				c = wc;
			} else {
				while ((c = *bp) != sep && c != '\n' && c != '\0')
					bp++;
				*bp++ = '\0';	/* mark end by overwriting separator */
			}
		} while (c != '\n' && c != '\0');
	}
	c = i;
	while (++c <= NFLD)
		*pp++ = null;
	return(i);
}

default_output(F, j, on)	/* print default olist */
register int F, j, on;
{
	int 	i;
	char	*temp;

	for (i = 0; i < on; i++)
		if (i != j) {
			temp = *ppi[F][i] ? ppi[F][i] : null;
			if (sep)
				/* avoid SCCS problem with 2 printf() */
				if (multibyte && sep > 0xff) {
					printf("%C", sep);
					printf("%s", temp);
				} else
					printf("%c%s", sep, temp);
			else
				printf(" %s", temp);
		}
}

output(on1, on2)	/* print items from olist */
register int on1, on2;
{
	register int i;
	register char *temp;

	if (no <= 0) {	/* default case */
		if (!*(temp = on1 ? ppi[F1][j1]: ppi[F2][j2]))
			temp = null;
		printf ("%s", temp);

		if (!swapped) {
			default_output(F1, j1, on1);
			default_output(F2, j2, on2);
		} else {
			default_output(F2, j2, on2);
			default_output(F1, j1, on1);
		}
		printf("\n");
	} else {
		for (i = 0; i < no; i++) {
			temp = ppi[olistf[i]][olist[i]];
			if(olistf[i]==F1 && on1<=olist[i] ||
			   olistf[i]==F2 && on2<=olist[i] ||
			   *temp==0)
				temp = null;
			printf("%s", temp);
			if (i == no - 1)
				printf("\n");
			else if (sep)
				if (multibyte && sep > 0xff)
					printf("%C", sep);
				else
					printf("%c", sep);
			else
				printf(" ");
		}
	}
}

error(s1, s2)
char *s1;
char *s2;
{
	fprintf(stderr, s1, s2);
	exit(1);
}

Usage()
{
	fprintf(stderr,MSGSTR(USAGE,
"Usage: join [-aNum] [-vNum] [-e Str] [-[j]Num Field] [-tChr] [-o Fieldlist] File1 File2\n"));
	exit(1);
}
