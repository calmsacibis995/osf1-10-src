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
static	char	*sccsid = "@(#)$RCSfile: strmerge.c,v $ $Revision: 4.3.2.2 $ (DEC) $Date: 1993/10/11 19:20:01 $";
#endif lint
/*
 */
/*
 *
 *   File name: strmerge.c
 *
 *   Source file description:
 *	Driver for batch string merge/replacment. Given a message file
 *	merge the text strings contained using the REWRITE rule from
 *	the pattern file into a new version of the source and produce
 *	a message catalogue for input to gencat(1).
 *
 *   Usage:	strmerge [-p file] [-m prefix] [-s string] [-n] filelist
 *
 */

/*
 * Modification history:
 * ~~~~~~~~~~~~~~~~~~~~~
 * 005	Tom Woodburn, 22 March 1991
 *	- Replaced strings with calls to catgets(3).
 *	- Added call to setlocale().
 *	- Changed copyright notice to DIGITAL_COPYRIGHT macro.
 *
 * 004	David Lindner Wed Feb 28 09:28:05 EST 1990
 *	- Fixed mneumonic option so it does set mneumonics correctly.
 *
 * 003	David Lindner Fri Oct 13 11:22:34 EDT 1989
 *	- Modified init_str code so that it would increment set for
 *        each source code module processed.
 *	- Also added %s option to patterns so set would increment in
 *	  new code as well.
 *
 * 002	David Lindner Mon Oct  9 16:58:32 EDT 1989
 *	- Fixed reversal of src1head and src2head
 *	- Modified comment header
 *
 * 001	Rainer Schaaf
 *		Changed the behaviour of the init_str
 *		added cathead src1head and src2head
 *		changed the behaviour when working on
 *		a filelist instead of a file.
 *		added the -o option.
 *
 * 000	Andy Gadsby,  5-Jan-1986.
 *		Created.
 *
 */

#include <stdio.h>
#include <locale.h>
#include "defs.h"
#include "extract_msg.h"

nl_catd catd;

char	*progname;			/* name program was invoked as	  */
char    *curfile;			/* current file open, for errors  */
int	errors = 0;
char 	*msgprefix = "";		/* prefix for message numbers	  */
int firsttime;
int new_msg_cat = 1;

/*
 * main()
 *	Parse the command line arguments and then for each file
 *	call dofile to handle the actual merge.
 */

extern char *refile;		/* in re.c			*/
extern char *init_str;		/* in re.c			*/
extern char src1head[];		/* in re.c			*/
extern char src2head[];		/* in re.c			*/
main(argc, argv)
int    argc;
char **argv;
{
	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_EXTRACT, NL_CAT_LOCALE);

	progname = argv[0];

	while (--argc > 0) {
		argv++;
		if (**argv == '-')
			switch (*(*argv + 1)) {
			default:
				fprintf(stderr, catgets(catd, MS_STRMERGE, M_STRMERGE_1, "%s: bad argument %c\n"), progname, *(*argv + 1));
				exit(1);
			case 'p':
				refile = *++argv;
				argc--;
				break;
			case 'm':
				msgprefix = *++argv;
				argc--;
				break;
			case 's':
				init_str = *++argv;
				argc--;
				break;
			case 'n':
				new_msg_cat = 0;
				argc--;
				break;
			}
		else
			break;
	}
	loadre();

	while ( argc-- > 0) {
		dofile(*argv++);
		firsttime++;
	}

	exit(errors > 0 ? 1 : 0);
}

/*
 * dofile()
 *	For the message file given attempt to open it, the corresponding
 *	catalogue file, the original source and the new source!
 */

dofile(name)
char *name;
{	FILE *mp, *op, *np;
	static FILE *cp;
	char *tmpname;			/* temporary file name		*/
	char *fixsuffix();
	char *fixprefix();
	static int setnum=0;		/* DJL 003 */

	curfile = name;
	tmpname = fixsuffix(name, MSGSUFFIX);
	if ((mp = fopen(tmpname, "r")) == (FILE *)NULL) {
		fprintf(stderr, catgets(catd, MS_STRMERGE, M_STRMERGE_2, "%s: cannot open %s\n"), progname, tmpname);
		errors++;
		return;
	}
	if ((op = fopen(name, "r")) == (FILE *)NULL) {
		fprintf(stderr, catgets(catd, MS_STRMERGE, M_STRMERGE_2, "%s: cannot open %s\n"), progname, name);
		fclose(mp);
		/* fclose(cp); */
		errors++;
		return;
	}
	tmpname = fixprefix(INTPREFIX, name);
	if ((np = fopen(tmpname, "w")) == (FILE *)NULL) {
		fprintf(stderr, catgets(catd, MS_STRMERGE, M_STRMERGE_2, "%s: cannot open %s\n"), progname, tmpname);
		fclose(mp);
		/* fclose(cp); */
		fclose(op);
		errors++;
		return;
	}
	/*
	 * message catalogue is opened only once
	 * the header of the message catalogue is written to the file
	 * (the default is the init_str)
	 */
	if (firsttime || new_msg_cat) {
	    tmpname = fixsuffix(name, CATSUFFIX);
	    if ((cp = fopen(tmpname, "w")) == (FILE *)NULL) {
		errors++;
		fprintf(stderr, catgets(catd, MS_STRMERGE, M_STRMERGE_2, "%s: cannot open %s\n"), progname, tmpname);
		fclose(mp);
		fclose(op);
		return;
	    }
	    if (*init_str) {			/* DJL 003 */
		    fputs(init_str, cp);
		    if (*msgprefix)		/* DJL 004 */
			fprintf(cp, "\n$set S_%s%d\n", msgprefix, ++setnum);
		    else
			fprintf(cp, "\n$set %d\n", ++setnum);
	    }
	}
	/*
	 * print the header lines to the new source file
	 */
	if (!firsttime && *src1head) {		/* DJL 002 */
		fputs(src1head, np);
	}
	else if (*src2head) {
		fputs(src2head, np);
	}

	processfile(mp, cp, op, np, setnum);
	fclose(op);
	fclose(np);
	/* fclose(cp); */
	fclose(mp);
}

/* 
 * processfile()
 *	Scan through the message file and for each line modify the old
 *	program using the REWRITE rule given in the patterns file.
 */

processfile(msg, cat, old, new, setnum)
FILE *msg, *cat, *old, *new;		/* file pointers		*/
int setnum;				/* DJL 003 */
{	
	int  line;			/* line field from msgline	*/
	long offset;			/* offset field from msgline	*/
	int  len;			/* length of text		*/
	char text[LINESIZE];		/* the actual text		*/
	int  msgnum;			/* current message number	*/
	
	msgnum = 1;
	while (fscanf(msg, "%d %ld %d %[^\n]", &line, &offset, &len, text) != EOF) {
		copy(old, new, offset);
		rewrite(text, len, setnum, msgnum, new);
		fseek(old, (long)len, 1);
		fprintf(cat, "%s%d\t%s\n", msgprefix, msgnum, text);
		msgnum++;
	}
	copy(old, new, (long)EOF);		/* copy to EOF		*/
}

/*
 * copy()
 *	Copy from current position in old to offset to the file new.
 * 	If the offset if EOF copy to EOF
 */

copy(old, new, offset)
FILE *old, *new;
long offset;
{ 	long current;
	int  c;

	current = ftell(old);
	if (offset == (long)EOF)
		while ((c = fgetc(old)) != EOF)
			fputc(c, new);
	else
		if (current > offset) {
			fprintf(stderr, catgets(catd, MS_STRMERGE, M_STRMERGE_3, "%s: Bad offset %d in %s\n"), progname, offset, curfile);
			errors++;
			return ERROR;
		}
		while (current++ < offset) {
			if ((c = fgetc(old)) == EOF) {
				fprintf(stderr, catgets(catd, MS_STRMERGE, M_STRMERGE_4, "%s: Unexpected EOF in %s\n"), progname, curfile);
				errors++;
				return ERROR;
			}
			fputc(c, new);
		}
	return OK;
}

/*
 * rewrite()
 *	Rewrite the message text in the form given by the REWRITE string
 *	from the patterns file. The result is written to new.
 *	The following replacement occurs:
 *		%n => message number
 *		%t => the message text
 *		%l => the length of the text
 *		%r => the raw text i.e. no quotes
 *		%N => newline
 *		%T => tab
 */

rewrite(text, len, setnum, msgnum, new)
char *text;
int  len;
int  setnum;		/* DJL 003 */
int  msgnum;
FILE *new;
{	extern char rewstring[];
	char *rp, *tp;
	char c, save;

	for (rp = rewstring; c = *rp; rp++) {
		if (c != '%') 
			fputc((int)c, new);
		else
			switch(c = *++rp) {
			default:
				fprintf(stderr, catgets(catd, MS_STRMERGE, M_STRMERGE_5, "%s: Bad rewrite string in pattern file at %%%c\n"), progname, c);
				errors++;
				return ERROR;
			case '%':
				fputc('%', new);
				break;
			case 'n':
				fprintf(new, "%s%d", msgprefix, msgnum);
				break;
			case 's':	/* DJL 003 */
				if (*msgprefix)		/* DJL 004 */
					fprintf(new, "S_%s%d", msgprefix, setnum);
				else
					fprintf(new, "%d", setnum);
					
				break;
			case 'l':
				fprintf(new, "%d", len);
				break;
			case 't':
				fputs(text, new);
				break;
			case 'r':
				tp = text + strlen(text) - 1;
				save = *tp;
				*tp = '\0';
				fputs(text + 1, new);
				*tp = save;
				break;
			case 'N':
				fputc('\n', new);
				break;
			case 'T':
				fputc('\t', new);
				break;
			}
	}
	return OK;
}
