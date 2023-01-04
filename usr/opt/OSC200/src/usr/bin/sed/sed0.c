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
static char rcsid[] = "@(#)$RCSfile: sed0.c,v $ $Revision: 4.3.8.3 $ (DEC) $Date: 1993/10/11 19:01:08 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDEDIT) sed0.c
 *
 * FUNCTIONS: main, fcomp, comploop, compsub, rline, address, cmp, 
 * text, search, dechain, ycomp, comple, getre and growspace.
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1984, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.17  com/cmd/edit/sed0.c, cmdedit, bos320, 9132320b 8/1/91 13:51:36
 *
 */

#include "sed.h"
#include <unistd.h>
#include <string.h>

FILE    *fin;
FILE    *fcode[12];
static struct addr *lastre;
static wchar_t    sseof;
static char    *reend;

static char  *rhsend;
static char  *rhsp;

union reptr     *ptrend;
int     eflag;
int     nbra;
int	errno;

/* linebuf start, end and size */
char    *linebuf;
char	*lbend;
int	lsize;

/* holdsp start, end and size */
char	*holdsp;
char	*hend;
int	hsize;
char	*hspend;	/* end of active hold */

/* genbuf start, end and size */
char	*genbuf;
char	*gend;
int	gsize;

/* pattern start, end and size */
/* Used to hold regular expression pattern string before call to regcomp */
char    *pattern;
char	*pend;
int	psize;

static int     gflag;
int     nlno;
char    fname[12][40];
int     nfiles;
union reptr ptrspace[PTRSIZE];
union reptr *rep;
char    *cp;
char	*sp;
struct label ltab[LABSIZE];
struct label    *lab;
struct label    *labend;
int     depth;
int	eargc;
union reptr     **cmpend[DEPTH];
char    *badp;
char    bad;

nl_catd catd;

#define CCEOF   22

struct label    *labtab = ltab;

int	exit_value = 0;
static int last_bad;

/*
 *	Define default messages.
 */
char    CGMES[]    = "sed: Function %s cannot be parsed.\n";
char    TMMES[]    = "sed: The %s function did not end the command line.\n";
char    LTL[]   = "sed: The label %s is greater than eight characters.\n";
char    AD0MES[]   = "sed: Cannot specify an address with the %s function.\n";
char    AD1MES[]   = "sed: Function %s allows only one address.\n";
char    MALLOCMES[]    = "sed: Memory allocation failed.\n";

/*
 *	Function prototypes.
 */
static void fcomp(char *);
static void dechain(void);
static int rline(char *, char *);
static struct addr *address(void);
static struct label *search(struct label *);
static char *text(char *, char *);
static struct addr *comple(wchar_t);
static int compsub(char *);
static int cmp(char *, char *);
static wchar_t	*ycomp(void);
static int getre(wchar_t);
static char *growspace(char *, char **);

int main(int argc, char **argv)
{
	int	compflag = 0;
	int 	ch;

	setlocale(LC_ALL,"");		/* required by NLS environment tests */

	catd = catopen(MF_SED,NL_CAT_LOCALE);

	nlno = 0;
	badp = &bad;
	aptr = abuf;
	lab = labtab + 1;       /* 0 reserved for end-pointer */
	rep = ptrspace;
	/* Dynamic memory allocation for buffer storage */
	sp = growspace((char *)0, &reend);
	growbuff(&lsize, &linebuf, &lbend, (char **)0);
	growbuff(&hsize, &holdsp, &hend, (char **)0);
	growbuff(&gsize, &genbuf, &gend, (char **)0);
	growbuff(&psize, &pattern, &pend, (char **)0);

	ptrend = &ptrspace[PTRSIZE];
	labend = &labtab[LABSIZE];

	rhsp = growspace((char *)0, &rhsend);

	lastre = 0;
	lnum = 0;
	pending = 0;
	depth = 0;
	spend = linebuf;
	hspend = holdsp;
	fcode[0] = stdout;
	nfiles = 1;

	if(argc == 1) 
		exiting(0);
	
	while ((ch=getopt(argc, argv, "e:f:gn?")) != -1) {
		switch (ch) {

		case 'n':
			nflag++;
			continue;

		case 'f':
			if((fin = fopen(optarg, "r")) == NULL) {
				fprintf(stderr, MSGSTR(PATTFIL, "sed: Cannot open pattern file %s.\n"), optarg);
				exiting(2);
			}

			fcomp(NULL);
			compflag++;
			fclose(fin);
			continue;

		case 'e':
			eflag++;
			fcomp(optarg);
			compflag++;
			eflag = 0;
			continue;

		case 'g':
			gflag++;
			continue;

		case '?':
		default:
			fprintf(stdout, MSGSTR(USAGE, "Usage: sed [-n] [-e script] [-f source_file] [file...]\n"));
			exiting(2);
		}
	}

	argc -= optind;
	argv += optind;

	if( argc > 0 && !compflag &&  rep == ptrspace) {
		eflag++;
		fcomp(*argv);
		argv++; argc--;
		eflag = 0;
	}

	if(depth) {
		fprintf(stderr, MSGSTR(LEFTBRC, "sed: There are too many '{'.\n"));
		exiting(2);
	}

	labtab->address = rep;

	dechain();

	eargc = argc;
	if(eargc <= 0)
		execute((char *)NULL);
	else while(--eargc >= 0) {
		execute(*argv++);
	}
	fclose(stdout);
	catclose(catd);
	return(exit_value);
}

/*
 *	Read sed commands into reptr structure ptrspace.
 *	Ptrspace stores address data, type of command,
 *	regular expressions and any new/inserted text, as well
 *	as any flags that are necessary for the processing
 *	of these commands.
 * 
 * 	The source argument points to either the expression
 * 	we want to compile, or NULL if we read it from a file.
 * 	Used by rline.
 */
static void fcomp(char *source)
{
	char   *tp;
	struct addr 	*op;

	union reptr     *pt, *pt1;
	int     i;
	struct label    *lpt;

	op = lastre;

	if (rline(linebuf, source) < 0)  
		return;
	if(last_bad) {
		fprintf( stderr, MSGSTR(FNDESC,"sed: Found escape character at end of editing script.\n"));  /* MSG */
		exiting(2);
	}
	if (*linebuf == '#') {
			/* if "#n" on first line, same effect as 
			   using -n flag from command line */
		if(linebuf[1] == 'n')
			nflag = 1;
	} else {
		cp = linebuf;
		goto comploop;
	}

	for(;;) {
		if(rline(linebuf, source) < 0)  
			break;
		if(last_bad) {
			fprintf( stderr, MSGSTR(FNDESC,"sed: Found escape character at end of editing script.\n"));  /* MSG */
			exiting(2);
		}
		if (*linebuf == '#')	/* skip comments anywhere! */
			continue;

		cp = linebuf;
comploop:
		while(*cp == ' ' || *cp == '\t')	/* skip white space */
			cp++;
		if(*cp == '\0')
			continue;
		if(*cp == ';') {
			cp++;
			goto comploop;
		}

		for (;;) {
			rep->r1.ad1 = address();
			if (errno != MORESPACE)
				break;
			sp = growspace(sp, &reend);
		}
		if(errno == BADCMD) {
			fprintf(stderr,MSGSTR(CGMSG, CGMES), linebuf);
			exiting(2);
		}

		if(errno == REEMPTY) {
			if(op) 
				rep->r1.ad1 = op;
			else {
				fprintf(stderr, MSGSTR(FRSTRE, "sed: The first regular expression cannot be null.\n"));  /* MSG */
				exiting(2);
			}
		} else if(errno == NOADDR) {
			rep->r1.ad1 = 0;
		} else {
			op = rep->r1.ad1;
			if(*cp == ',' || *cp == ';') {
				cp++;
				for (;;) {
					rep->r1.ad2 = address();
					if (errno != MORESPACE)
						break;
					sp = growspace(sp, &reend);
				}
				if(errno == BADCMD || errno == NOADDR) {
					fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exiting(2);
				}
				if(errno == REEMPTY) 
					rep->r1.ad2 = op;
				else
					op = rep->r1.ad2;

			} else
				rep->r1.ad2 = 0;
		}

		while (sp >= reend) 
			sp = growspace(sp, &reend);

		while(*cp == ' ' || *cp == '\t')	/* skip wkite space */
			cp++;
swit:
		switch(*cp++) {

			default:
				fprintf(stderr, MSGSTR( BADCMND, "sed: %s is not a recognized function.\n"), linebuf);  /* MSG */
				exiting(2);

			case '!':
				rep->r1.negfl = 1;
				goto swit;

			case '{':
				rep->r1.command = BCOM;
				rep->r1.negfl = !(rep->r1.negfl);
				cmpend[depth++] = &rep->r2.lb1;
				if(++rep >= ptrend) {
					fprintf(stderr, MSGSTR( TOOMANYCMDNS, "sed: There are more than 99 commands in pattern file.\n"), linebuf);  /* MSG */
					exiting(2);
				}
				if(*cp == '\0') continue;

				goto comploop;

			case '}':
				if(rep->r1.ad1) {
					fprintf(stderr, MSGSTR(AD0MSG, AD0MES), linebuf);  /* MSG */
					exiting(2);
				}

				if(--depth < 0) {
					fprintf(stderr, MSGSTR( RGHTBRC, "sed: There are too many '}'.\n"));  /* MSG */
					exiting(2);
				}
				*cmpend[depth] = rep;

				continue;

			case '=':
				rep->r1.command = EQCOM;
				if(rep->r1.ad2) {
					fprintf(stderr, MSGSTR(AD1MSG, AD1MES), linebuf);  /* MSG */
					exiting(2);
				}
				break;

			case ':':
				if(rep->r1.ad1) {
					fprintf(stderr, MSGSTR( AD0MSG, AD0MES), linebuf);  /* MSG */
					exiting(2);
				}

				while(*cp++ == ' ');
				cp--;


				tp = lab->asc;
				while((*tp++ = *cp++))
					if(tp >= &(lab->asc[8])) {
						fprintf(stderr, MSGSTR(LTLMSG, LTL), linebuf);  /* MSG */
						exiting(2);
					}

				if(lpt = search(lab)) {
					if(lpt->address) {
						fprintf(stderr, MSGSTR(DUPLBL, "sed: There are more than one %s labels.\n"), linebuf);  /* MSG */
						exiting(2);
					}
				} else {
					lab->chain = 0;
					lpt = lab;
					if(++lab >= labend) {
						fprintf(stderr, MSGSTR( LABELCNT, "sed: There are too many labels in file %s.\n"), linebuf);  /* MSG */
						exiting(2);
					}
				}
				lpt->address = rep;

				continue;

			case 'a':
				rep->r1.command = ACOM;
				if(rep->r1.ad2) {
					fprintf(stderr, MSGSTR( AD1MSG, AD1MES), linebuf);  /* MSG */
					exiting(2);
				}
				if(*cp == '\\') cp++;
				if(*cp++ != '\n') {
					fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exiting(2);
				}
				rep->r1.rhs = sp;
				while ((sp = text(rep->r1.rhs, reend)) == 0) {
					sp = growspace(rep->r1.rhs, &reend);
					rep->r1.rhs = sp;
				}
				break;
			case 'c':
				rep->r1.command = CCOM;
				if(*cp == '\\') cp++;
				if(*cp++ != ('\n')) {
					fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exiting(2);
				}
				rep->r1.rhs = sp;
				while ((sp = text(rep->r1.rhs, reend)) == 0) {
					sp = growspace(rep->r1.rhs, &reend);
					rep->r1.rhs = sp;
				}
				break;
			case 'i':
				rep->r1.command = ICOM;
				if(rep->r1.ad2) {
					fprintf(stderr, MSGSTR( AD1MSG, AD1MES), linebuf);  /* MSG */
					exiting(2);
				}
				if(*cp == '\\') cp++;
				if(*cp++ != ('\n')) {
					fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exiting(2);
				}
				rep->r1.rhs = sp;
				while ((sp = text(rep->r1.rhs, reend)) == 0) {
					sp = growspace(rep->r1.rhs, &reend);
					rep->r1.rhs = sp;
				}
				break;

			case 'g':
				rep->r1.command = GCOM;
				break;

			case 'G':
				rep->r1.command = CGCOM;
				break;

			case 'h':
				rep->r1.command = HCOM;
				break;

			case 'H':
				rep->r1.command = CHCOM;
				break;

			case 't':
				rep->r1.command = TCOM;
				goto jtcommon;

			case 'b':
				rep->r1.command = BCOM;
jtcommon:
				while(*cp == ' ')
					cp++;

				if(*cp == '\0') {
					if(pt = labtab->chain) {
						while(pt1 = pt->r2.lb1)
							pt = pt1;
						pt->r2.lb1 = rep;
					} else
						labtab->chain = rep;
					break;
				}
				tp = lab->asc;
				while((*tp++ = *cp++))
					if(tp >= &(lab->asc[8])) {
						fprintf(stderr, MSGSTR( LTLMSG, LTL), linebuf);  /* MSG */
						exiting(2);
					}
				cp--;

				if(lpt = search(lab)) {
					if(lpt->address) {
						rep->r2.lb1 = lpt->address;
					} else {
						pt = lpt->chain;
						while(pt1 = pt->r2.lb1)
							pt = pt1;
						pt->r2.lb1 = rep;
					}
				} else {
					lab->chain = rep;
					lab->address = 0;
					if(++lab >= labend) {
						fprintf(stderr, MSGSTR( LABELCNT, "sed: There are too many labels in file %s.\n"), linebuf);  /* MSG */
						exiting(2);
					}
				}
				break;

			case 'n':
				rep->r1.command = NCOM;
				break;

			case 'N':
				rep->r1.command = CNCOM;
				break;

			case 'p':
				rep->r1.command = PCOM;
				break;

			case 'P':
				rep->r1.command = CPCOM;
				break;

			case 'r':
				rep->r1.command = RCOM;
				if(rep->r1.ad2) {
					fprintf(stderr, MSGSTR( AD1MSG, AD1MES), linebuf);  /* MSG */
					exiting(2);
				}
				if(*cp++ != ' ') {
					fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exiting(2);
				}
				rep->r1.rhs = sp;
				while ((sp = text(rep->r1.rhs, reend)) == 0) {
					sp = growspace(rep->r1.rhs, &reend);
					rep->r1.rhs = sp;
				}
				break;

			case 'd':
				rep->r1.command = DCOM;
				break;

			case 'D':
				rep->r1.command = CDCOM;
				rep->r2.lb1 = ptrspace;
				break;

			case 'q':
				rep->r1.command = QCOM;
				if(rep->r1.ad2) {
					fprintf(stderr, MSGSTR( AD1MSG, AD1MES), linebuf);  /* MSG */
					exiting(2);
				}
				break;

			case 'l':
				rep->r1.command = LCOM;
				break;

			case 's':
				rep->r1.command = SCOM;
				cp += mbtowc(&sseof, cp, MB_CUR_MAX); 
				rep->r1.re1 = comple(sseof);
				if(errno == BADCMD) {
					fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exiting(2);
				}
				if(errno == REEMPTY) 
					rep->r1.re1 = op;
				else 
					op = rep->r1.re1;

				rep->r1.rhs = rhsp;
				while (errno = compsub(rep->r1.rhs)) {
					if (errno == MORESPACE) {
						rhsp = growspace(rhsp, &rhsend);
						rep->r1.rhs = rhsp;
						continue;
					}
					fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exiting(2);
				}
				if(gflag)
					rep->r1.gfl = GLOBAL_SUB;
				else
					rep->r1.gfl = 1;
				while (strspn(cp, "gpPw0123456789")) {
					if(*cp == 'g')
						rep->r1.gfl = GLOBAL_SUB;
					else if(*cp == 'p')
						rep->r1.pfl = 1;
					else if(*cp == 'P')
						rep->r1.pfl = 2;
					else if(*cp == 'w') {
						cp++;
						if(*cp++ !=  ' ') {
							fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
							exiting(2);
						}
						if(nfiles >= 10) {
							fprintf(stderr, MSGSTR( FILECNT, "sed: The w function allows a maximum of ten files.\n"));  /* MSG */
							exiting(2);
						}

						(void)text(fname[nfiles], (char *)0);
						for(i = nfiles - 1; i >= 0; i--)
							if(cmp(fname[nfiles],fname[i]) == 0) {
								rep->r1.fcode = fcode[i];
								goto done;
							}
						if((rep->r1.fcode = fopen(fname[nfiles], "w")) == NULL) {
							fprintf(stderr, MSGSTR( FILEOPEN, "cannot open %s\n"), fname[nfiles]);  /* MSG */
							exiting(2);
						}
						fcode[nfiles++] = rep->r1.fcode;
						break;
					} else {
						rep->r1.gfl = (short) strtol(cp,&tp,10);
						if (rep->r1.gfl == 0) {
							fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
							exiting(2);
						}
						cp = --tp;
					}
					cp++;
				}
				break;

			case 'w':
				rep->r1.command = WCOM;
				if(*cp++ != ' ') {
					fprintf(stderr, MSGSTR( CGMSG, CGMES), linebuf);  /* MSG */
					exiting(2);
				}
				if(nfiles > 10){
					fprintf(stderr, MSGSTR( FILECNT, "sed: The w function allows a maximum of ten files.\n"));  /* MSG */
					exiting(2);
				}

				(void) text(fname[nfiles], (char *)0);
				for(i = nfiles - 1; i >= 0; i--)
					if(cmp(fname[nfiles], fname[i]) == 0) {
						rep->r1.fcode = fcode[i];
						goto done;
					}

				if((rep->r1.fcode = fopen(fname[nfiles], "w")) == NULL) {
					fprintf(stderr, MSGSTR(CREATERR, "sed: Cannot create file %s.\n"), fname[nfiles]);  /* MSG */
					exiting(2);
				}
				fcode[nfiles++] = rep->r1.fcode;
				break;

			case 'x':
				rep->r1.command = XCOM;
				break;

			case 'y':
				rep->r1.command = YCOM;
				cp += mbtowc(&sseof, cp, MB_CUR_MAX);

				rep->r1.ytxt = ycomp();
				if(rep->r1.ytxt == 0) {
					fprintf(stderr, 
						MSGSTR(CGMSG, CGMES), linebuf);
					exiting(2);
				}
				break;

		}
done:
		if (rep->r1.command == SCOM && !rep->r1.re1)
			if (lastre)
				rep->r1.re1 = lastre;
			else {
				fprintf(stderr, MSGSTR(FRSTRE, "sed: The first regular expression cannot be null.\n"));  /* MSG */
				exiting(2);
			}
		if(++rep >= ptrend) {
			fprintf(stderr, MSGSTR(CMNDCNT, "sed: There are too many commands for the %s function.\n"), linebuf);  /* MSG */
			exiting(2);
		}

		if(*cp++ != '\0') {
			if(cp[-1] == ';')
				goto comploop;
			fprintf(stderr, MSGSTR(CGMSG, CGMES), linebuf);  /* MSG */
			exiting(2);
		}

	}
	rep->r1.command = 0;
	lastre = op;
}

/*
 *	Write replacement string for substitution command
 *	into rhsbufparm. Any '\\' characters are left in the
 *	string and parsed for in the replacement phase.
 */
static int    compsub(char *rhsbufparm)
{
	register char   *q;
	register char   *p;
	wchar_t	wc;
	int	len;

	p = rhsbufparm;
	q = cp;
	while (*q) {
		if (p > rhsend)
			return(MORESPACE);
		if(*q == '\\') {
			*p++ = *q++;
				/* check for illegal subexpression */
			if (*q > nbra + '0' && *q <= '9')
				return(0);
			if ((len = mblen(q, MB_CUR_MAX)) < 1)
				break;
		} else {
			if ((len = mbtowc(&wc, q, MB_CUR_MAX)) < 1)
				break;
			if(wc == sseof) {
				*p++ = '\0';
				cp = q + len;
				if (p > rhsend)
					return(MORESPACE);
				rhsp = p;	/* update the rhsbuf pointer */
				return(0);
			}
		}
		while (len--)
			*p++ = *q++;
	}
	return(BADCMD);
}

/*
 *	Read in a single command line.
 * 
 * 	lbuf 	- buffer to put line in
 * 	source 	- points to command line expr if any.
 */
static int	rline(char *lbuf, char *source)
{
	register char   *p, *q;
	register        t;
	static char	*saveq;
	int	i, len;
	char 	str[MB_LEN_MAX];

	last_bad = 0;
	p = lbuf - 1;

	if(source != NULL) {
		if (eflag > 0) {
			eflag = -1;
			q = source;
		} else {
			if ((q = saveq) == 0)
				return(-1);
		}

		saveq = NULL;
		while (*q) {
			/* Don't test for '\n' after '\\' */
			if(*q == '\\') {
				*++p = *q++; 	/* Copy the '\\' */
				if(*q == '\0') {
					last_bad = 1;
					*++p = '\0';
					return(1);
				}
			} else if (*q == '\n') {
				*++p = '\0';
				saveq = ++q;
				return(1);
			}
			if ((len = mblen(q, MB_CUR_MAX)) < 1)
				return(-1);
			while (len--)
				*++p = *q++;
		}
		*++p = '\0';
		return(1);
	}

	/*
	 * If source is NULL, read from file (fin) which is
	 * is already opened for us
	 */
	while((t = getc(fin)) != EOF) {
		if (t == '\\') {
			*++p = t;
			t = getc(fin);
			if(t == EOF) {
				last_bad = 1;
				*++p = '\0';
				return(1);
			}
			if(t == '\n') {
				if((t = getc(fin)) == EOF) {
					last_bad = 1;
					*++p = '\0';
					return(1);
				} else {
					ungetc(t, fin);
					t = '\n';
				}
			}
		} else if (t == '\n') {
			*++p = '\0';
			return(1);
		}
		len = 1;
		str[0] = t;
		while (t != EOF && mblen(str, MB_CUR_MAX) != len) {
			if (++len > MB_CUR_MAX)
				return(-1);
			str[len-1] = t = getc(fin);
		}
		for (i=0; t != EOF && i<len; i++)
			*++p = str[i];
	}
	if(p != lbuf - 1) {
		*++p = '\0';
		return(1);
	}
	return(-1);
}

/*
 *	Store an address into addr structure if one is present.
 *	If not, set errno flag :
 *		- BADCMD, error in command line.
 *		- REEMPTY, a regular expr. indentified but empty.
 *			i.e. substitute previous RE.
 *		- NOADDR, no address given.
 *		- MORESPACE, need a bigger buffer
 */
static struct addr *address(void)
{
	struct addr	*abuf;
	register char   *rcp, *rsp;
	long    lno;

	errno = 0;
	rsp = sp;
	
	if(*cp == '$') {
		if ((abuf = (struct addr *)malloc(sizeof(struct addr))) == 0) {
			fprintf(stderr, 
				MSGSTR(MALLOCMSG, MALLOCMES));
		}
		abuf->afl = STRA;
		abuf->ad.str = rsp;
		*rsp++ = CEND;
		*rsp++ = CCEOF;
		if (rsp >= reend) {
			errno = MORESPACE;
			free((void *)abuf);
			return(0);
		}
		cp++;
		sp = rsp;
		return(abuf);
	}

	if (*cp == '/' || *cp == '\\' ) {	/* address is RE */
		if ( *cp == '\\' )
			cp++;
		cp += mbtowc(&sseof, cp, MB_CUR_MAX); 
		return(comple(sseof));
	}

	rcp = cp;
	lno = 0;

	while(*rcp >= '0' && *rcp <= '9')	/* address is line number */
		lno = lno*10 + *rcp++ - '0';

	if(rcp > cp) {
		if ((abuf = (struct addr *)malloc(sizeof(struct addr))) == 0) {
			fprintf(stderr, 
				MSGSTR(MALLOCMSG, MALLOCMES));
		}
		abuf->afl = STRA;
		abuf->ad.str = rsp;
		*rsp++ = CLNUM;
		*rsp++ = nlno;
		tlno[nlno++] = lno;
		if(nlno >= NLINES) {
			fprintf(stderr, MSGSTR( LINECNT, "sed: There are too many line numbers specified.\n"));  /* MSG */
			exiting(2);
		}
		*rsp++ = CCEOF;
		if (rsp >= reend) {
			errno = MORESPACE;
			free((void *)abuf);
			return (0);
		}
		cp = rcp;
		sp = rsp;
		return(abuf);
	}
	errno = NOADDR;
	return(0);
}

static int cmp(char *a, char *b)
{
	register char   *ra, *rb;

	ra = a - 1;
	rb = b - 1;

	while(*++ra == *++rb)
		if(*ra == '\0') return(0);
	return(1);
}

/*
 *	Read text from linebuf(cp) into textbuf.
 *	Return null if textbuf exceeds endbuf.
 */
static char    *text(char *textbuf, char *endbuf)
{
	register char   *p, *q;
	int	len;

	p = textbuf;
	q = cp;
	for(;;) {
		if (endbuf && (p >= endbuf))
			return(0);
		if(*q == '\\') 
			q++;	/* Discard '\\' and read next character */
		if(*q == '\0') {
			*p = *q;
			cp = q;
			return(++p);
		}
		/* Copy multi-byte character to p */
		if ((len = mblen(q, MB_CUR_MAX)) < 1)
			fprintf(stderr, MSGSTR(CGMSG, CGMES), linebuf);
		while (len--)
			*p++ = *q++;
	}
}

static struct label    *search(struct label *ptr)
{
	struct label    *rp;

	rp = labtab;
	while(rp < ptr) {
		if(cmp(rp->asc, ptr->asc) == 0)
			return(rp);
		rp++;
	}

	return(0);
}

static void dechain(void)
{
	struct label    *lptr;
	union reptr     *rptr, *trptr;

	for(lptr = labtab; lptr < lab; lptr++) {

		if(lptr->address == 0) {
			fprintf(stderr, MSGSTR( UNDFNLBL, "sed: %s is not a defined label.\n"), lptr->asc);  /* MSG */
			exiting(2);
		}

		if(lptr->chain) {
			rptr = lptr->chain;
			while(trptr = rptr->r2.lb1) {
				rptr->r2.lb1 = lptr->address;
				rptr = trptr;
			}
			rptr->r2.lb1 = lptr->address;
		}
	}
}

/*
 *	Parse a 'y' command i.e y/xyz/abc/
 *	where xyz are the characters to be matched and
 *	abc are their replacement characters. 
 *	N.B. these characters can be multi-byte or the string "\\n"
 *	Return a pointer to the buffer storing these characters.
 */
static wchar_t	*ycomp(void)
{
	wchar_t c1, c2;
	wchar_t *ybuf, *yp;
	char	*tsp, *ssp;
	int	count = 0, len1, len2;

	ssp = cp;
	tsp = cp;
	/* Determine no of characters to be matched */
	/* and then allocate space for their storage */
	/* Set tsp to point to the replacement characters */
	for (;;) {
		if (*tsp == '\0') 
			return (0);
		if ((len1 = mbtowc(&c1, tsp, MB_CUR_MAX)) > 0) {
			tsp += len1;
			if (c1 == sseof)
				break;
			count++;
		} else
			return (0);
	}

	/* Allocate space for the characters to be replaced and */
	/* their replacements. The buffer will be built up by storing */
	/* the characters to be replaced and their replacement one after */
	/* the other i.e. in pairs in the buffer. For the search and replace */
	/* stage every second char will be tested and when a match is found */
	/* it will be replaced by the next character in the search buffer */

	ybuf = (wchar_t *)malloc((count * 2 + 1)*sizeof(wchar_t));
	if (!ybuf) {
		fprintf(stderr,
			MSGSTR(MALLOCMSG, MALLOCMES));	/* MSG */
	}

	len2 = 0;
	yp = ybuf;
	while ((len1 = mbtowc(&c1,ssp,MB_CUR_MAX)) > 0) {
		len2 = mbtowc(&c2, tsp, MB_CUR_MAX);
		if (c1 == sseof)
			break;
		if (len2 < 1 || *tsp == '\0' || c2 == sseof)
			return (0);
		if (len1 == 1 && *ssp == '\\' && ssp[1] == 'n') {
			ssp++;
			mbtowc(&c1,"\n", MB_CUR_MAX);
		}
		ssp += len1;
		*yp++ = c1;

		if(len2 == 1 && *tsp == '\\' && tsp[1] == 'n') {
			tsp++;
			mbtowc(&c2,"\n", MB_CUR_MAX);
		}
		tsp += len2;
		*yp++ = c2;
	}
	if(c2 != sseof)
		return (0);
	cp = tsp + len2;
	yp = '\0';

	return (ybuf);
}

/*
 *	Compile the regular expression, returning the results
 *	in a struct addr structure and setting the appropriate
 *	error number if required.
 */
static struct addr	*comple(wchar_t reeof)
{
	struct addr	*res;
	regex_t	*reg;
	int	cflag;

	errno = 0;
	/* Read reg. expr. string into pattern */
	cflag = getre(reeof);
	if (!cflag) {
		if ((reg = (regex_t *)malloc(sizeof(regex_t))) == 0) {
			fprintf(stderr, 
				MSGSTR(MALLOCMSG, MALLOCMES));
		}
		if (regcomp(reg, pattern, 0) == 0) {
			if ((res = (struct addr *)malloc(sizeof(struct addr))) == 0) {
				fprintf(stderr, 
					MSGSTR(MALLOCMSG, MALLOCMES));
			}
			res->afl = REGA;
			res->ad.re = reg;
			nbra = reg->re_nsub;
			return(res);
		} else {
			free((void *)reg);
			errno = BADCMD;
		}
	} else
		errno = cflag;
	return(0);
}

/*
 *	Read regular expression into pattern, replacing reeof
 *	with a null terminator. Maintains cp, the pointer
 *	into the linebuf string. 
 */
static int	getre(wchar_t reeof)
{
	register char	*p1;
	char *p2;
	wchar_t	wc;
	int	empty = 1, len;
	int	brackcnt = 0;

	p1 = pattern;
	for (;;) {
		if (*cp == '\0' || *cp == '\n')
			break;
		if ((len = mbtowc(&wc, cp, MB_CUR_MAX)) < 1)
			break;
		if (!brackcnt && wc == reeof) {
			cp += len;
			*p1 = '\0';
			return (empty ? REEMPTY : 0);
		}
		empty = 0;
		if (*cp == '\\') {
			*p1 = *cp++;
			if (*cp == 'n') {
				while (p1 >= pend) {
					p2 = p1;
					growbuff(&psize, &pattern, &pend, &p2);
					p1 = p2;
				}
				*p1++ = '\n';
				cp++;
				continue;
			}
			while (p1 >= pend) {
				p2 = p1;    /* need p2 because p1 is register */
				growbuff(&psize, &pattern, &pend, &p2);
				p1 = p2;
			}
			p1++;
			if ((len = mblen(cp, MB_CUR_MAX)) < 1)
				break;
		/* Special code to allow delimiter to occur within bracket
		   expression without a preceding backslash */
		} else if (*cp == '[') {
			if (!brackcnt) {
				if (((*(cp+1) == '^') && (*(cp+2) == ']')) || (*(cp+1) == ']'))
					brackcnt++;
				brackcnt++;
			} else {
				if ((*(cp+1) == '.') || (*(cp+1) == ':') || (*(cp+1) == '='))
					brackcnt++;
			}
		} else if (*cp == ']' && brackcnt)
			brackcnt--;
		while ((p1 + len) >= pend) {
			p2 = p1;	/* need p2 because p1 is register */
			growbuff(&psize, &pattern, &pend, &p2);
			p1 = p2;
		}
		while (len--)
			*p1++ = *cp++;
	}
	return (BADCMD);
}

static char *last = 0;

/*
 *	Dynamic memory allocation for buffer storage.
 */
static char *growspace(char *buf, char **endp)
{
	int amount;

	if (last && buf == last) { /* can do realloc */
		amount = (*endp - last) << 1;
		last = realloc(last, amount);
	} else {
		if (!buf || (amount = *endp - buf) < LBSIZE)
			amount = LBSIZE;
		last = malloc(amount);
	}
	if (!last) {
		fprintf(stderr, MSGSTR(MALLOCMSG, MALLOCMES));
		exiting(2);
	}
	*endp = last + amount;
	return last;
}
