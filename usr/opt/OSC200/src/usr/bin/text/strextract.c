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
static	char	*sccsid = "@(#)$RCSfile: strextract.c,v $ $Revision: 4.3.2.2 $ (DEC) $Date: 1993/10/11 19:19:53 $";
#endif lint
/*
 */
/*
 *
 *   File name: strextract.c
 *
 *   Source file description:
 *	Driver for batch string extraction. Used to produce message
 *	catalogues which can be updated by the user and then merged 
 *	back into the original text using strmerge.c
 *
 *   Functions:
 *	main()
 *	    dofile()
 *	    processfile()
 *		fixsuffix()
 *
 *   Usage:	strextract [-c] [-p file] [-i ignfile] filelist
 *
 *   Modification history:
 * 001	Tom Woodburn, 22 March 1991
 *	- Replaced strings with calls to catgets(3).
 *	- Added call to setlocale().
 *	- Changed copyright notice to DIGITAL_COPYRIGHT macro.
 *
 *	Andy Gadsby, 18-Dec-1986.
 *		Created.
 *
 */

#include <stdio.h>
#include <locale.h>
#include "defs.h"
#include "extract_msg.h"

nl_catd catd;

int	iscflag = FALSE;	/* TRUE if source files are 'C'		*/
				/* if this is set ignore comments	*/
int	dflag = TRUE;		/* TRUE if we report duplicates		*/
char	*progname;		/* name program was invoked as		*/
char 	*ignfile = (char *)0;	/* name of ignore file, if any 		*/
int	errors = 0;

/*
 * main()
 *	Parse the command line arguments and then for each file
 *	call dofile to handle the actual extraction.
 */

main(argc, argv)
int    argc;
char **argv;
{	extern char *refile;		/* in re.c			*/

	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_EXTRACT, NL_CAT_LOCALE);

	progname = argv[0];

	while (--argc > 0) {
		argv++;
		if (**argv == '-')
			switch (*(*argv + 1)) {
			default:
				fprintf(stderr, catgets(catd, MS_STREXTRACT, M_STREXTRACT_1, "%s: bad argument %c\n"), progname, *(*argv + 1));
				exit(1);
			case 'c':
				iscflag = TRUE;
				break;
			case 'd':
				dflag = 0;
				break;
			case 'i':
				ignfile = *++argv;
				argc--;
				break;
			case 'p':
				refile = *++argv;
				argc--;
				break;
			}
		else
			break;
	}
	loadre();
	loadignore(ignfile);

	if (argc > 0)
		while ( argc-- > 0)
			dofile(*argv++);
	else
		processfile(stdin, stdout, catgets(catd, MS_STREXTRACT, M_STREXTRACT_2, "STDIN"));

	exit(errors > 0 ? 1 : 0);
}

/*
 * dofile()
 *	For the file given attempt to open it and the corresponding
 *	message file.
 */

dofile(name)
char *name;
{	FILE *fp, *mp;
	char *msgname;			/* name of catalogue		*/
	char *fixsuffix();

	if ((fp = fopen(name, "r")) == (FILE *)NULL) {
		fprintf(stderr, catgets(catd, MS_STREXTRACT, M_STREXTRACT_3, "%s: cannot open %s\n"), progname, name);
		errors++;
		return;
	}
	msgname = fixsuffix(name, MSGSUFFIX);
	if ((mp = fopen(msgname, "w")) == (FILE *)NULL) {
		fprintf(stderr, catgets(catd, MS_STREXTRACT, M_STREXTRACT_3, "%s: cannot open %s\n"), progname, msgname);
		errors++;
		return;
	}
	clearstr();			/* clear out old strings	*/
	processfile(fp, mp, name);
	fclose(mp);
	fclose(fp);
}

/* 
 * processfile()
 *	Scan through the file looking for strings matching the 
 *    	patterns specified in the pattern file. If any are found 
 *	these are written to the message (temporary) file, 
 *	together with linenumber, count and length information.
 */

processfile(in, msgp, name)
FILE *in, *msgp;
char *name;
{	char line[LINESIZE];		/* the line to scan		*/
	char *cp;			/* pointer in line		*/
	long offset = 0;		/* of string in file		*/
	int  linenum = 0;		/* in file			*/
	int  len;			/* of matched string		*/
	char *mesg;			/* pointer to error message	*/
	char *matchre();
	struct element *match;

	while (fgets(line, LINESIZE, in)) {
		linenum++;
		cp = line;
		do {
			cp = matchre(cp, cp == line, &len, &mesg);
			if (cp) {	/* got a match			*/
				if (mesg) {
					/* matched on an error		*/
					fprintf(stderr, catgets(catd, MS_STREXTRACT, M_STREXTRACT_4, "%s: %s, line %d: %s\n"), progname, name, linenum, mesg);
					errors++;
				} else {
					match = lookupstr(cp, len);

					/*
					 * if ignoring string skip it,
					 * if a duplicate warn as appropriate
					 * otherwise is valid match
					 */

					if (match && (match->flags & STR_IGNORE))
						;
					else {
					    if (dflag && match)
						fprintf(stderr, catgets(catd, MS_STREXTRACT, M_STREXTRACT_5, "%s: %s. Duplicate string line %d %.*s\n"), progname, name, linenum, len, cp);
					    else {
						struct element elem;
	
						elem.len = len;
						elem.flags = 0;
						elem.linenum = linenum;
						savestr(cp, &elem);
					    }
					    fprintf(msgp, "%d %d %d %.*s\n", linenum, offset + (cp - line), len, len, cp);
					}
				}
				cp += len;
			}
		} while (cp && *cp != '\0');
		offset += strlen(line);
	}
}
