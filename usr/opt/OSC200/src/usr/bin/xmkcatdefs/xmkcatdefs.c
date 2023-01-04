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
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: xmkcatdefs.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/11 20:00:27 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 


/*
 * COMPONENT_NAME: CMDMSG
 *
 * FUNCTIONS: main, mkcatdefs, incl, chkcontin
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#if defined(NLS) || defined(KJI)
#include <NLctype.h>
#else
#include <ctype.h>
#endif
#include <sys/types.h>
#include <sys/dir.h>
#include <string.h>
#ifdef MSG
#include "msgfac_msg.h"
#define	 MSGSTR(N,S)	NLcatgets(errcatd,MS_MKCATDEFS,N,S)
#else
#ifdef NLS
#include <nl_types.h>
#endif
#ifdef _BLD
#ifndef	PATH_MAX
#define	PATH_MAX	1024
#endif
#ifndef	NL_TEXTMAX
#define	NL_TEXTMAX	4096
#endif
#else
#include <limits.h>
#endif
#define  MSGSTR(N,S)	S
#endif

#define MAXLINELEN NL_TEXTMAX
#define KEY_START '$'
#define MAXIDLEN 64
#ifdef _D_NAME_MAX
#define MDIRSIZ _D_NAME_MAX
#else
#define MDIRSIZ 14
#endif

#ifdef _BLD
#undef	_toupper
#define	_toupper	toupper
#undef	NLfprintf
#define	NLfprintf	fprintf
#endif

/*
 * EXTERNAL PROCEDURES CALLED: descopen, descclose, descset, descgets,
 *                             descerrck, insert, nsearch
 */

char *descgets();
#ifdef MSG
    nl_catd errcatd;
#endif
    static int errflg = 0;
    static int setno = 1;
    static int msgno = 1;
    static int symbflg = 0;
    static int inclfile = 1;
    FILE *outfp;
    FILE *msgfp;
    static char inname [PATH_MAX];
    static char outname [PATH_MAX];
    static char catname [PATH_MAX];
    char *mname;

/*
 * NAME: main
 *
 * FUNCTION: Makes message catalog definitions.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * NOTES: Invoked by
 *        mkcatdefs <name> <msg_file>
 *
 *        Results are 1) Creates header file <name>.h.
 *                    2) Displays message file to stdout ready to be used as
 *                       input for gencat.
 *
 *        mkcatdefs takes a message definition file and produces
 *        a header file containing #defines for the message catalog,
 *  	  the message sets and the messages themselves.  It also
 * 	  produces a new message file which has the symbolic message set and
 * 	  message identifiers replaced by their numeric values (in the form
 * 	  required by gencat).
 *
 * RETURNS: 1 - error condition
 */

#ifdef _ANSI
main (int argc, char *argv[])
#else
main (argc, argv)
int argc;
char *argv [];
#endif
{
    register int i;
    register char *cp;

#ifdef MSG
	errcatd = NLcatopen(MF_MSGFAC,NL_CAT_LOCALE);
#endif
    if (argc < 3) {
	fprintf (stderr, MSGSTR(MKCATUSAGE, 				/*MSG*/
		"mkcatdefs: Usage: %s <catname> <msg_file>\n"), argv [0]);	/*MSG*/
	exit (0);
    }

    /* check if  include file should be created */
    if ((argv[1][0] == '-' && argv[1][1] == 'h') || (argv[1][0] == '\0'))
		inclfile = 0;

    /* open header output file */
    if (inclfile) {
	char *t;
	mname = argv [1];
    	sprintf (outname, "%s_msg.h", mname);
	if (strrchr(mname,'/'))
	    mname = strrchr(mname,'/') + 1;
        sprintf (catname, "%s.cat", mname);
    	if ((outfp = fopen (outname, "w")) == NULL) {
		fprintf (stderr, MSGSTR(MKCATOPN, 			/*MSG*/
			"mkcatdefs: Cannot open %s\n"), outname);	/*MSG*/
		exit (1);
	} else  {
    		incl ("#include <limits.h>\n");
    		incl ("#include <nl_types.h>\n");
    		/* convert name to upper case */
    		for (cp=mname; *cp; cp++)
			if (islower (*cp))
	    			*cp = _toupper (*cp);

    		incl ("#define MF_%s \"%s\"\n\n", mname, catname);

	}
    } else sprintf (outname, "msg.h");


    /* open new msg output file */
    msgfp = stdout;

/* if message descriptor files were specified then process each one in turn */
    for (i = 2; i < argc; i++) {
    /* open input file */
    	sprintf (inname, "%s", argv[i]);
	if (strcmp(inname,"-") == NULL) {
		strcpy(inname,"stdin");
		descset(stdin);       /* input from stdin if no source files */
		mkcatdefs(inname);
	} else	{
		if (descopen(inname) < 0) {
			fprintf (stderr, MSGSTR(MKCATOPN, 		/*MSG*/
			      "mkcatdefs: Cannot open %s\n"), inname);	/*MSG*/
			errflg = 1;
		} else  {
			mkcatdefs (inname);
			descclose();
		}
	}
    }

    if (inclfile) {
    	fflush (outfp);
    	if (ferror (outfp)) {
		fprintf (stderr, MSGSTR(WRITERRS, 			/*MSG*/
		   "mkcatdefs: There were write errors on file %s\n"), 	/*MSG*/
		   outname);						/*MSG*/
		errflg = 1;
	}
    	fclose (outfp);
    }

    if (errflg) {
	fprintf (stderr, MSGSTR(ERRFND,					/*MSG*/
		"mkcatdefs: Errors found: no %s created\n"), outname);	/*MSG*/
	if (inclfile)  unlink(outname);
    } else {
	   if (inclfile) {
		if (symbflg)
			fprintf (stderr, MSGSTR(HCREAT, 		/*MSG*/
				"mkcatdefs: %s created\n"), outname);	/*MSG*/
	   	else {
			fprintf (stderr, MSGSTR(NOSYMB, 		/*MSG*/
			"mkcatdefs: No symbolic identifiers; no %s created\n"), 
					outname);			/*MSG*/
			unlink (outname);
		}
   	   }
    }
    exit (errflg);
}

 /*
 * NAME: mkcatdefs
 *
 * FUNCTION: Make message catalog definitions.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: None
 */

#ifdef _ANSI
mkcatdefs (char *fname)
#else
mkcatdefs (fname)
				/*
				  fname - message descriptor file name
				*/
char *fname;
#endif
{
    char msgname [PATH_MAX];
    char line [MAXLINELEN];
    register char *cp;
    register char *cpt;
    register int m;
    register int n;
    int contin = 0;


    /* put out header for include file */
    incl ("\n\n/* The following was generated from %s. */\n\n",fname);

    /* process the message file */
    while (descgets (line, MAXLINELEN) ) {
#if defined(NLS) || defined(KJI)
	for (cp=line; isspace (*cp); cp += NLchrlen(cp)); /* find first nonblank character */
#else
	for (cp=line; isspace (*cp); cp++); /* find first nonblank character */
#endif
	    if (*cp == KEY_START) {
		cp++;
		if (isspace (*cp)) {
#if defined(NLS) || defined(KJI)
			for (; isspace (*cp); cp += NLchrlen(cp)); 
#else
			for (; isspace (*cp); cp++);
#endif
		    sscanf (cp, "%s", msgname);
		    if ((m = nsearch(msgname)) > 0) {
			fprintf (msgfp, "$ %d", m);
			cp += strlen(msgname);
			fprintf (msgfp, "%s", cp);
		    } else
		    	fputs (line, msgfp);
		    continue; /* line is a comment */
		}
		if (strncmp (cp, "set", 3) == 0 && isspace (cp[3])) {
    		    char setname [MAXIDLEN];

		    sscanf (cp+4, "%s", setname);
		    incl ("\n/* definitions for set %s */\n", setname);
		    if (isdigit(setname[0])) {
			    cpt = setname;
			    do  {
				if (!isdigit(*cpt)) {
					fprintf(stderr,MSGSTR(ZEROINV,
					"mkcatdefs: %s is an invalid identifier\n"), setname);	/*MSG*/
					errflg = 1;
					break;
				}
			    }   while (*++cpt);
			n = atoi (setname);
			if (n >= setno)
			    	setno = n;
		        else {
				if (n = 0)
					fprintf(stderr,MSGSTR(ZEROINV,
					"mkcatdefs: %s is an invalid identifier\n"), setname);	/*MSG*/
				else	fprintf(stderr,MSGSTR(INVLDSET,
					"mkcatdefs: set # %d already assigned or sets not in ascending sequence\n"), n);
				errflg = 1;
			}
		    } else  {
			    cpt = setname;
			    do  {
				if ((!isalpha(*cpt)) && 
				    (!isdigit(*cpt)) && 
				    (*cpt != '_'))    {
					fprintf(stderr,MSGSTR(ZEROINV,
					"mkcatdefs: %s is an invalid identifier\n"), setname);	/*MSG*/
					errflg = 1;
					break;
				}
			    }   while (*++cpt);
			incl ("#define %s %d\n\n", setname, setno);
		        symbflg = 1;
		    }
		    fprintf (msgfp,"$delset");
		    fprintf (msgfp," %d\n", setno);
		    fprintf (msgfp,"%.4s", line);
		    fprintf (msgfp," %d\n", setno++);
		    msgno = 1;
		    continue;
		} else {
		     /* !!!other command */
		}
	    } else
		if (contin) {
#if defined(NLS) || defined(KJI)
		    if (!chkcontin(line))
#else
		    if (line[strlen(line) - 2] != '\\')
#endif
			contin = 0;
		} else if (setno > 1) { /* set must have been seen first */
    		    char msgname [MAXIDLEN];

		    msgname [0] = '\0';
		    if (sscanf (cp, "%s", msgname) && msgname [0] )
			if (isalpha (msgname[0])) {
			    cpt = msgname;
			    do  {
				if ((!isalpha(*cpt)) && 
				    (!isdigit(*cpt)) && 
				    (*cpt != '_'))    {
					fprintf(stderr,MSGSTR(ZEROINV,
					"mkcatdefs: %s is an invalid identifier\n"), msgname);	/*MSG*/
					errflg = 1;
					break;
				}
			    }   while (*++cpt);
			    cp += strlen(msgname);
			    fprintf (msgfp,"%d %s", msgno,cp);
			    incl ("#define %s %d\n", msgname, msgno);
			    symbflg = 1;
#if defined(NLS) || defined(KJI)
			    if (chkcontin(line))
#else
		    	    if (line[strlen(line) - 2] == '\\')
#endif
				contin = 1;
			    if(insert(msgname,msgno++) < 0) {
				fprintf(stderr,MSGSTR(MULTOPN, "mkcatdefs: name %s used more than once\n"),msgname); /*MSG*/
				errflg = 1;
			    }
			    continue;
			} else if (isdigit (msgname[0])){
			    cpt = msgname;
			    do  {
				if (!isdigit(*cpt)) {
					fprintf(stderr,MSGSTR(INVTAG, 
					"mkcatdefs: invalid syntax in %s\n"),
					line);
					errflg = 1;
					break;
				}
			    }   while (*++cpt);
			    n = atoi (msgname);
			    if ((n >= msgno) || (n == 0 && msgno == 1))
				msgno = n + 1;
			    else {
				 errflg = 1;
				 if (n == 0)
					fprintf(stderr,MSGSTR(ZEROINV,
					"mkcatdefs: %s is an invalid identifier\n"), msgno);	/*MSG*/
				 else if (n == msgno) 
						NLfprintf(stderr,MSGSTR(MULTNR,
						"mkcatdefs: message id %s already assigned to identifier\n"), msgname);	/*MSG*/
				      else	fprintf(stderr,MSGSTR(NOTASC, 
						"mkcatdefs: source messages not in ascending sequence\n"));	/*MSG*/
			    }
			}
#if defined(NLS) || defined(KJI)
		    if (chkcontin(line))
#else
		    if (line[strlen(line) - 2] == '\\')
#endif
			contin = 1;
		}
	fputs (line, msgfp);
    }

    /* make sure the operations read/write operations were successful */
    if (descerrck() == -1) {
	fprintf (stderr, MSGSTR(READERRS, 				/*MSG*/
	    "mkcatdefs: There were read errors on file %s\n"), inname);	/*MSG*/
	errflg = 1;
    }
}

 /*
 * NAME: incl
 *
 * FUNCTION: Outputs strings to file.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: None
 */

#ifdef _ANSI
incl(char *a, char *b, char *c)
#else
incl(a, b, c)
				/*
				  a - pointer to "printf" format
				  b - pointer to optional "printf" arg
				  c - pointer to optional "printf" arg
				*/
char *a, *b, *c;
#endif
{
	if (inclfile) fprintf (outfp, a, b, c);
}


#if defined(NLS) || defined(KJI)


 /*
 * NAME: chkcontin
 *
 * FUNCTION: Check for a continuation line.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: 0 - not a continuation line.
 *          1 - continuation line.
 */


#ifdef _ANSI
chkcontin(char *line)
#else

chkcontin(line)
				/*
				  line - pointer to line to be checked
				*/
  char *line;
#endif
{
	register char *ptr, *eptr;

	ptr = line;
	eptr = &line[strlen(line) - 2];
	while (*ptr && ptr < eptr)
		ptr += NLchrlen(ptr);
	if (ptr == eptr && *ptr == '\\')
		return (1);
	return (0);
}
#endif
