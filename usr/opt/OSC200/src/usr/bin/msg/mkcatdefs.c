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
static char rcsid[] = "@(#)$RCSfile: mkcatdefs.c,v $ $Revision: 4.2.12.5 $ (DEC) $Date: 1993/12/21 20:37:05 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDMSG) Message Catalogue Facilities
 *
 * FUNCTIONS: main, mkcatdefs, incl, chkcontin 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.19  com/cmd/msg/mkcatdefs.c, cmdmsg, bos320, 9125320 5/29/91 10:23:42";
 */

/*
 * If _BLD is defined, this command will only correctly process
 * ASCII/8-bit message files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <sys/dir.h>
#include <limits.h>
#include <string.h>
#ifdef _BLD
#define	 MSGSTR(N,S)	S
#else
#include <nl_types.h>
#include "msgfac_msg.h"
#define	 MSGSTR(N,S)	catgets(errcatd,MS_MKCATDEFS,N,S)
#endif /* _BLD */


#ifndef NL_TEXTMAX
#define NL_TEXTMAX 4096
#endif

/*
 * Increased MAXLINELEN from NL_TEXTMAX to NL_TEXTMAX+128 to allow
 *  message text to be NL_TEXTMAX long 
 */
#define MAXLINELEN NL_TEXTMAX+128
#define KEY_START '$'
#define MAXIDLEN 64

/*
 * Defined returned values of getident()
 */
#define IDGOOD		0
#define IDTOOLONG	1
#define IDINVALID	2
#define IDBADCHAR	3	

#ifdef _BLD
#define wchar_t		unsigned char
#define iswblank(wc)	isspace(wc)
#define iswlower(wc)	islower(wc)
#define iswupper(wc)	isupper(wc)
#define iswdigit(wc)	isdigit(wc)
#define iswspace(wc)	isspace(wc)
#define iswalnum(wc)	isalnum(wc)
#define iswalpha(wc)	isalpha(wc)
#define towupper(wc)	toupper(wc)
#define mbtowc(wc,char,size)	(*(wc) = *(char),1)
#define wctomb(char,wc)		(*(char) = (wc),1)
#else	/* _BLD */
#ifndef iswblank
#define iswblank(wc)      iswctype(wc, wctype("blank")) /* not defined by X/Open */
#endif
#endif	/* _BLD */

/*
 * EXTERNAL PROCEDURES CALLED: descopen, descclose, descset, descgets,
 *                             descerrck, insert, nsearch
 */

char *descgets();
#ifndef _BLD
nl_catd errcatd;
#endif
static int errflg = 0;
static int setno = 1;
static int msgno = 1;
static int symbflg = 0;
static int inclfile = 1;
FILE *outfp;
FILE *msgfp;
static char inname [PATH_MAX+1];
static char outname [PATH_MAX+1];
static char catname [PATH_MAX+1];
char *mname;
static void mkcatdefs(char *);
static void incl(char *, char *, char *);
static int chkcontin(char *);
static int getident(char *, char *);
wchar_t quote_char;
extern int optind;
/*
 * NAME: main
 *
 * FUNCTION: Make message catalog defines.
 *
 * EXECUTION ENVIRONMENT:
 *  	User mode.
 *
 * NOTES:  Invoked by:
 *         mkcatdefs <name> <msg_file>
 *
 *  	Results are 1) Creates header file <name>.h.
 *                  2) Displays message file to stdout. The message file is 
 *                     ready to be used as input to gencat.
 *
 *   	mkcatdefs takes a message definition file and produces
 *  	a header file containing #defines for the message catalog,
 * 	the message sets and the messages themselves.  It also
 *  	produces a new message file which has the symbolic message set and
 *  	message identifiers replaced by their numeric values (in the form
 *  	required by gencat).
 *
 * DATA STRUCTURES: Effects on global data structures -- none.
 *
 * RETURNS: 1 - error condition
 */

main (int argc, char *argv[]) 

{
    register int i;
    register char *cp;
    int c;
    char *t;
    wchar_t wc;		/* filename process code */

#ifndef _BLD
    setlocale (LC_ALL,"");
    errcatd = catopen(MF_MSGFAC,NL_CAT_LOCALE);
#endif /* _BLD */

    /* usage: handle multiple files;  */
    if (argc < 3) {
	fprintf (stderr, MSGSTR(MKCATUSAGE,"Usage: mkcatdefs [-h] catname msg_file [msg_file...]\n"));	
	exit (1);
    }
    while((c = getopt(argc,argv,"h")) != EOF)
         switch(c) {
	 case 'h':
	      inclfile = 0;
	      argc--;   /* skip over options */
	      argv = &argv[1];
	      break;
	 case '?':
	      fprintf (stderr, MSGSTR(MKCATUSAGE,"Usage: mkcatdefs [-h] catname msg_file [msg_file...]\n"));	
	      exit (1);
	 }

    /* open header output file */
    if (inclfile) {
	mname = argv [1];
	if ((strlen((t = strrchr(mname,'/')) ? t + 1 : mname) +
             sizeof("_msg.h") - 1) > NAME_MAX) {
		fprintf (stderr, MSGSTR(MNAMTOOLONG, "mkcatdefs: catname too long\n"));
		exit (1);
	}
    	sprintf (outname, "%s_msg.h", mname);
	if (strrchr(mname,'/'))
	    mname = strrchr(mname,'/') + 1;
        sprintf (catname, "%s.cat", mname);
    	if ((outfp = fopen (outname, "w")) == NULL) {
		fprintf (stderr, MSGSTR(MKCATOPN, "mkcatdefs: Cannot open %s\n"), outname);								/*MSG*/
		exit (1);
	} else  {
    		/* convert name to upper case */
    		for (cp=mname; *cp; cp+=i) {
			i = mbtowc(&wc, cp, MB_CUR_MAX);
			if (i < 0) {
				fprintf (stderr, MSGSTR(IMBCHD, "mkcatdefs: catname contains invalid character\n"));
				exit (1);
			}
			if (iswlower(wc) != 0)
				wc = towupper(wc);
			else if (!iswupper(wc) && !iswdigit(wc))
				wc = '_';
			wctomb(cp, wc);
		}
                incl ("#ifndef _H_%s_MSG \n", mname, "");
                incl ("#define _H_%s_MSG \n", mname, "");
    		incl ("#include <limits.h>\n", "", "");
    		incl ("#include <nl_types.h>\n", "", "");
    		incl ("#define MF_%s \"%s\"\n\n", mname, catname);
	}
    } else sprintf (outname, "msg.h");


    /* open new msg output file */
    msgfp = stdout;

    /* Symbol name (catname) is required */
    if (argc == 2) {
        fprintf (stderr, MSGSTR(MKCATUSAGE,"Usage: mkcatdefs [-h] SymbolName SourceFile[...SourceFile] \n"));
        exit (1);
    }

    for (i = 2; i < argc; i++) {
    /* open input file */
    	sprintf (inname, "%s", argv[i]);
	if (strcmp(inname,"-") == 0) {
		strcpy(inname,"stdin");
		descset(stdin);       /* input from stdin if no source files */
		mkcatdefs(inname);
	} else	{
		if (descopen(inname) < 0) {
			fprintf (stderr, MSGSTR(MKCATOPN,"mkcatdefs: Cannot open %s\n"), inname);							/*MSG*/
			errflg = 1;
		} else  {
			mkcatdefs (inname);
			descclose();
		}
	}
    }
    incl ("#endif \n", "", "");

    if (inclfile) {
    	fflush (outfp);
    	if (ferror (outfp)) {
		fprintf (stderr, MSGSTR(WRITERRS,"mkcatdefs: There were write errors on file %s\n"), outname);						/*MSG*/
		errflg = 1;
	}
    	fclose (outfp);
    }

    if (errflg) {
	fprintf (stderr, MSGSTR(ERRFND,"mkcatdefs: Errors found: no %s created\n"), outname);								/*MSG*/
	if (inclfile)  unlink(outname);
    } else {
	   if (inclfile) {
		if (symbflg)
			fprintf (stderr, MSGSTR(HCREAT,"mkcatdefs: %s created\n"), outname);
	   	else {
			fprintf (stderr, MSGSTR(NOSYMB,"mkcatdefs: No symbolic identifiers; no %s created\n"), outname);				/*MSG*/
			unlink (outname);
		}
   	   } 
	   else 
                fprintf(stderr,MSGSTR(NOHDR,"mkcatdefs: no %s created\n"), outname);                                      				/*MSG*/
    }
    exit (errflg);
}

/*
 * NAME: mkcatdefs
 *
 * FUNCTION: Make message catalog definitions.
 *
 * EXECUTION ENVIRONMENT:
 *  	User mode.
 *
 * RETURNS: None
 */

void
mkcatdefs (char *fname)

	/*---- fname: message descriptor file name ----*/

{
    char msgname[MAXIDLEN+1];
    char line[MAXLINELEN];
    char *str;			/* The return of descgets() */
    register char *cp;
    register char *cpt;
    register int m;
    register int n;
    int contin = 0;
    int len;		/* # bytes in a character */
    int ident;		/* the return of getident() */
    wchar_t wc;		/* process code */


    /* put out header for include file */
    incl ("\n\n/* The following was generated from %s. */\n\n",fname, "");

    /* process the message file */
    while ((str = descgets(line, MAXLINELEN)) != NULL ) {
	/* check if the message source is over long so to be truncated */
	if(strchr(str, '\n') == NULL) {
		fprintf (stderr, MSGSTR(MSGTOOLONG, "mkcatdefs: The message text is too long [%d]:\n\t%s\n"), NL_TEXTMAX, line);
		errflg = 1;
		return;
	}
	/* find first nonblank character */
	for (cp=line; *cp; cp+=len) {
		len = mbtowc(&wc, cp, MB_CUR_MAX);
		if (len < 0) {
			fprintf (stderr, MSGSTR(IMBCTX, "mkcatdefs: sourcefile contains invalid character:\n\t%s"),line);
			errflg = 1;
			return;
		}
		if (iswblank(wc) == 0)
			break;
	}
	if (*cp == KEY_START && !contin) {
		cp++;
		for (cpt = cp; *cp; cp += len) {
			len = mbtowc(&wc, cp, MB_CUR_MAX);
			if (len < 0) {
				fprintf (stderr, MSGSTR(IMBCTX, "mkcatdefs: sourcefile contains invalid character:\n\t%s"),line);
				errflg = 1;
				return;
			}
			if (iswspace(wc) == 0)
				break;
		}
		if (cp != cpt) {
		    msgname[0] = '\0';
		    if (((ident = getident(cp, msgname)) == IDGOOD) &&
			(msgname[0] != '\0') && (m = nsearch(msgname)) > 0) {
			fprintf (msgfp, "$ %d", m);
			cp += strlen(msgname);
			fprintf (msgfp, "%s", cp);
		    } else
		    	fputs (line, msgfp);
		    continue; /* line is a comment */
		}
		if ((strncmp (cp, "set", 3) == 0) && ((len = mbtowc(&wc, cp+3, MB_CUR_MAX)) > 0) && (iswspace(wc) != 0)) {
    		    char setname [MAXIDLEN+1];

		    setname[0] = '\0';
		    cpt = cp+3+len;
		    if((ident = getident(cpt, setname)) != IDGOOD) {
			switch (ident) {
				case IDTOOLONG:
					fprintf (stderr, MSGSTR(SETTOOLONG, "mkcatdefs: The set identifier is too long [%d]:\n\t%s\n"), MAXIDLEN, line);
					break;
				case IDINVALID:
					fprintf (stderr, MSGSTR(INVTAG, "mkcatdefs: The symbolic set or message identifier is not valid:\n\t%s\n"), line);
					break;
				default:	/* include case IDBADCHAR */
					fprintf (stderr, MSGSTR(IMBCTX, "mkcatdefs: sourcefile contains invalid character:\n\t%s"), line);
			}
			errflg = 1;
			return;	
		     }
		    incl ("\n/* definitions for set %s */\n", setname, "");
		    if(!setname[0]) {
			fprintf(stderr,MSGSTR(MISSSET, "mkcatdefs: A set number or identifier is missing:\n\t%s\n"), line);
			errflg = 1;
			return;
		    }
		    if (isdigit((unsigned char) setname[0])) {
			n = atoi (setname);
			if (n >= setno)
			    	setno = n;
		        else {
				if (n == 0)
				   fprintf(stderr,MSGSTR(ZEROINV, "mkcatdefs: %s is an invalid identifier\n"), setname);	
				else
				   fprintf(stderr,MSGSTR(INVLDSET, "mkcatdefs: set # %d already assigned or sets not in ascending sequence\n"), n);
				errflg = 1;
				return;
			}
		    } else  {
			incl ("#define %s %d\n\n", setname, (char *)setno);
		        symbflg = 1;

                        if(insert(setname,setno) < 0) {
                           fprintf(stderr,MSGSTR(MULTOPN, "mkcatdefs: name %s used more than once\n"), setname);
                           errflg = 1;
                           return;
                        }
		    }
		    fprintf (msgfp,"$delset");
		    fprintf (msgfp," %d\n", setno);
		    fprintf (msgfp,"%.4s", line);
		    fprintf (msgfp," %d\n", setno++);
		    msgno = 1;
		    /* Someone may use '\' to continue one's comment */
		    if(chkcontin(line)) {
			fprintf(stderr,MSGSTR(BADCONTINUE, "mkcatdefs: Only message text can use '\' to continue: %s\n"), cpt+strlen(setname));
			errflg = 1;
			return;
		    }
		    continue;
		} else {
		     /* !!!other command */
		}

                /* Search for the quote character */
                if ((strncmp (cp, "quote", 5) == 0) && ((len = mbtowc(&wc, cp+5, MB_CUR_MAX)) > 0) && (iswspace(wc) != 0)) {
                        char *cptt = cp+5+len;
                        while (((len = mbtowc(&wc, cptt, MB_CUR_MAX)) > 0) && (iswspace(wc) != 0))
                                cptt += len;
                        quote_char = wc;
                }
	} else
		if (contin) {
		    if (!chkcontin(line))
			contin = 0;
		} else {

		    msgname [0] = '\0';
		    if((ident = getident(cp, msgname)) != IDGOOD) {
			switch (ident) {
				case IDTOOLONG:
					fprintf (stderr, MSGSTR(TAGTOOLONG, "mkcatdefs: The symbolic identifier is too long [%d]:\n\t%s\n"), MAXIDLEN, line);
					break;
				case IDINVALID:
					fprintf (stderr, MSGSTR(INVTAG, "mkcatdefs: The symbolic set or message identifier is not valid:\n\t%s\n"), line);
					break;
				default:	/* include case IDBADCHAR */
					fprintf (stderr, MSGSTR(IMBCTX, "mkcatdefs: sourcefile contains invalid character:\n\t%s\n"),line);	
			}
			errflg = 1;
			return;
		    }
		    if(msgname[0]) {
			if (isdigit((unsigned char) msgname[0])) {
				n = atoi (msgname);
				if ((n >= msgno) || (n == 0 && msgno == 1))
					msgno = n + 1;
				else {
				     if (n == 0)
					fprintf(stderr,MSGSTR(ZEROINV, "mkcatdefs: %s is an invalid identifier\n"), msgname);			
				     else if (n == msgno)
					fprintf(stderr,MSGSTR(MULTNR, "mkcatdefs: message id %s already assigned to identifier\n"), msgname);		/*MSG*/
					  else
					    fprintf(stderr,MSGSTR(NOTASC, "mkcatdefs: source messages not in ascending sequence\n"));                   /*MSG*/
				     errflg = 1;
				     return;
				}
			} else { /* Leading char should be alpha char */
			    cp += strlen(msgname);
			    fprintf (msgfp,"%d%s", msgno,cp);
			    incl ("#define %s %d\n", msgname, (char *)msgno);
			    symbflg = 1;
			    if (chkcontin(line))
				contin = 1;
			    if(insert(msgname,msgno++) < 0) {
				fprintf(stderr,MSGSTR(MULTOPN, "mkcatdefs: name %s used more than once\n"), msgname); 
				errflg = 1;
				return;
			    }
			    continue;
			}
		    }
		    if (chkcontin(line))
			contin = 1;
		}
	fputs (line, msgfp);
    }

    /* make sure the operations read/write operations were successful */
    if (descerrck() == -1) {
	fprintf (stderr, MSGSTR(READERRS, "mkcatdefs: There were read errors on file %s\n"), inname);							/*MSG*/
	errflg = 1;
    }
    return;
}

/*
 * NAME: incl
 *
 * FUNCTION: Output strings to file.
 *
 * EXECUTION ENVIRONMENT:
 *  	User mode.
 *
 * RETURNS: None
 */

void
incl(char *a, char *b, char *c) 

	/*
	  a - pointer to "printf" format
	  b - pointer to optional "printf" arg
	  c - pointer to optional "printf" arg
	*/

{
	if (inclfile) fprintf (outfp, a, b, c);
}


/*
 * NAME: chkcontin
 *
 * FUNCTION: Check for continuation line.
 *
 * EXECUTION ENVIRONMENT:
 *  	User mode.
 *
 * RETURNS: 0 - not a continuation line.
 *          1 - continuation line.
 */

#define GETWC()		len = mbtowc(&wc, line, MB_CUR_MAX); \
			if (len < 0) { \
				fprintf (stderr, MSGSTR(IMBCTX, "mkcatdefs: sourcefile contains invalid character:\n\t%s"),line); \
				errflg = 1; \
				return (0); \
                	} \
			if (wc == '\0') \
				return(0); \
			line += len;

int
chkcontin(char *line)

{
        int     len;            /* # bytes in character */
        wchar_t wc;             /* process code of current character in line */
	int quotes = 0;

	for(;;) {
		GETWC();
		if (wc == '\\') {
			GETWC();
			if (wc == '\n')
				return(quotes < 2);
			continue;
		}
		if (wc == '\n')
			return(0);
		if (wc == quote_char)
			quotes++;
	}
}

/*
 * NAME: getident 
 *
 * FUNCTION: Check and get a valid symbolic identifier. 
 *
 * EXECUTION ENVIRONMENT:
 *      User mode.
 *
 * RETURNS: IDGOOD - valid (not > MAXIDLEN, but also including empty case).
 *          IDTOOLONG - invalid because > MAXIDLEN.
 *	    IDINVALID - invalid beasuce contains invalid character.
 *	    IDBADCHAR - Error because len < 0 
 *			(or len != 1 in identifier, since only 1-b char is 
 *			 allowed in token).
 */

int
getident(char *cp, char *identname)
{
	char	*cpt;
	int	len, identlen;
	int	isdig = 0;
	wchar_t wc;

	if ((*cp == '\0') || (*cp == '\n')) 
		return(IDGOOD);

	/* Get rid of multi spaces */
	do {
		len = mbtowc(&wc, cp, MB_CUR_MAX);
		if (len < 0) 
			return(IDBADCHAR);
	} while ((iswspace(wc)) && (*(cp += len)));

	if ((*cp == '\0') || (*cp == '\n')) 
		return(IDGOOD);

	cpt = cp;
	/* Only Number and alphanumeric identifier are allowed */
	if (iswalpha(wc) != 0) 
		isdig = 0;
	else if (iswdigit(wc) != 0) 
		isdig = 1;
	else	/* The invalid indentifier */
		return(IDINVALID);

	identlen = 0;
	identname[identlen] = *cpt;
	identlen += len;
	cpt += len;

	/*
	 * Walk thr the string to check whether it is 1-b char, nonspace,
	 * digit if the identifier is supposed to be Number, alphanumeric 
	 * or '_' if the identifier is supposed to be alphanumeric identifier; 
	 * and the total length of the identifier is not larger than 
	 * MAXIDLEN. At the same time, copy the identifier. 
	 */
	while ((*cpt) && ((len = mbtowc(&wc, cpt, MB_CUR_MAX)) == 1)
	     && (!iswspace(wc)) && (identlen <= MAXIDLEN)
	     && (*cpt != '\n')) {

		if (((iswalnum(wc) == 0) && (wc != '_'))
		     || ((isdig == 1) && (iswdigit(wc) == 0))) {
			return(IDINVALID);
		}
		identname[identlen] = *cpt;
		identlen += len;
		cpt += len;
	}

	if (identlen > MAXIDLEN)
		return(IDTOOLONG);
	if (len != 1)
		return(IDBADCHAR);

	identname[identlen] = '\0';
	return(IDGOOD);
}
