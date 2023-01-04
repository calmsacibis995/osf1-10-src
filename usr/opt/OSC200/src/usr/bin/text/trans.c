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
static	char	*sccsid = "@(#)$RCSfile: trans.c,v $ $Revision: 4.3.4.2 $ (DEC) $Date: 1993/10/11 19:20:15 $";
#endif lint
/*
 */
/*
 *
 *   File name: trans.c
 *
 *   Source file description:
 * 	This program is used to interactively translate message source
 *	files.
 *
 *   Usage:	trans [-o name] [-c] filelist
 *
 */

/*
 * Modification history
 * ~~~~~~~~~~~~~~~~~~~~
 * 002  Tom Woodburn, 21 Apr 1991
 *	- Included curses.h if OSF, cursesX.h if not.
 *	- Replaced strings with calls to catgets(3).
 *	- Assigned return value of getopt(3) to an int instead of a char
 *	  to avoid endless loop on OSF.
 *	- Changed type of windup() from int to void.
 *	- Added call to setlocale().
 *	- Changed copyright notice to DIGITAL_COPYRIGHT macro.
 *	- Changed variable "inline", which is now a reserved word,
 *	  to "inputline".
 *
 * 001	David Lindner Thu Apr 19 14:20:12 EDT 1990
 *	- Added mutiple file processing capabilties.
 *	- Implemented getopts().
 *	- Implemented intermediate file saving.
 *	- Generally made code more robust.
 *
 * 000	Andy Gadsby, 21-Apr-1987.
 *	- Created.
 *
 */

#ifdef OSF		/* TJW 002 */
#include <curses.h>
#else
#include <cursesX.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include "trans_msg.h"

nl_catd catd;


/* 
 * Curses and basic defintions
 */
#define BANNER	  "DEC OSF/1 Translation Tool"
#define DIVIDER   '-'
#define ORIG_LINES	10
#define TRANS_LINES	5
#define	CMD_LINES	4
#define ORIG_START	2
#define TRANS_START	(ORIG_START + ORIG_LINES + 1)
#define CMD_START	(TRANS_START + TRANS_LINES + 1)
#define LINEMAX		1024


/*
 * Declarations of functions
 */
char	getquote();			/* determine quote character */
void	windup();			/* interrupt handler */
void	startterm();			/* terminal initialization */
extern char	*index();		/* string function */
extern char	*rindex();		/* string function */
extern int	getopt();		/* get options function */


/*
 * Global variable defintions
 */
char   *outname;		/* default output file */
char   *inname;			/* input file name */
char   *progname;		/* this programs invocation name */
int 	errors = 0;		/* error flag */
int	oflag = FALSE;		/* toggle output name flag */
int	trans_comments = FALSE;	/* toggle translate comment flag */


/*
 * main()
 * 	process args and drive dofile with argument list
 */
main(argc, argv)
int 	argc;
char   *argv[];
{
	extern int optind, opterr;	/* getopt externals */
	extern char *optarg;		/* getopt externals */
	int c;

	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_TRANS, NL_CAT_LOCALE);
	
	progname = argv[0];
	opterr = 0;		/* supress getopt error message */

	while ((c = getopt(argc, argv, "co:")) != EOF)
		switch(c) {
		case 'c':
			trans_comments = TRUE;
			break;
		case 'o':
			outname = optarg;
			oflag = TRUE;
			break;
		case '?':
			fprintf(stderr, catgets(catd, MS_TRANS, M_TRANS_1, "\nusage: %s [-c] [-o name] file.msg\n"), progname);
			exit(1);
			
		}

	/*
	 * check for multiple files
	 */
	if (((argc - optind) > 1) && (oflag)) {
		oflag = FALSE;
		fprintf(stderr, catgets(catd, MS_TRANS, M_TRANS_2, "\nwarning: output name ignored, multiple files\n"));
		printf(catgets(catd, MS_TRANS, M_TRANS_3, "\nAcknowledged? (return)"));
		getc(stdin);
	}

	signal(SIGINT, windup);
	startterm();
	/*
	 * must loop through the rest of the arguments which are assumed
	 * to be .msg files (we check of course...)
	 */
	for ( ; optind < argc; optind++) {
		inname = argv[optind];
		dofile(argv[optind]);
		if (errors)
			windup(0);
	}	

	windup(0);			/* tidy up and exit */
	/* NOTREACHED */
}


/*
 * startterm()
 *	setup the terminal with the appropriate	windows using curses.
 */

WINDOW *original;			/* for the original text	*/
WINDOW *translate;			/* to do the translation in	*/
WINDOW *command;			/* for communication with user  */

void
startterm()
{
	char *banner;
    
	initscr();
	crmode();
	cbreak();
	noecho();
	clear();
	banner = catgets(catd, MS_TRANS, M_TRANS_22, BANNER);
	move(0, COLS/2 - strlen(banner)/2);
	addstr(banner);

	original  = subwin(stdscr, ORIG_LINES, 0, ORIG_START, 0);
	translate = subwin(stdscr, TRANS_LINES, 0, TRANS_START, 0);
	command = subwin(stdscr, CMD_LINES, 0, CMD_START, 0);
	scrollok(command, TRUE);

	refresh();
}

/* 
 * windup()
 *	reset the terminal, delete output file if interrupt
 */

void windup(sig)
int sig;
{
	char	rs[LINEMAX];	/* response string */
	char	*y;		/* yes string */
	char	*n;		/* no  string */

	signal(SIGINT, SIG_IGN);
	mvcur(0, COLS - 1, LINES -1, 0);
	endwin();
	putchar('\n');
	fflush(stdout);
	if (sig != 0) {
		while (TRUE) {
			printf(catgets(catd, MS_TRANS, M_TRANS_4, "Save? (y/n) "));
			fflush(stdout);
			fgets(rs, LINEMAX, stdin);
			y = catgets(catd, MS_TRANS, M_TRANS_6, "y");
			if (*rs == *y)
				exit(0);
			n = catgets(catd, MS_TRANS, M_TRANS_8, "n");
			if (*rs == *n) {
				unlink(outname);
				exit(0);
			}
		}
	}
	exit(errors ? 1 : 0);
}

/* 
 * error()
 *	Print the two strings passed as arguments to command window,
 *	and set the error flag.
 */

error(str1, str2)
char *str1, *str2; 
{
	if (*str2)
		wprintw(command, catgets(catd, MS_TRANS, M_TRANS_9, "error: %s: %s\n"), str2, str1);
	else
		wprintw(command, catgets(catd, MS_TRANS, M_TRANS_10, "error: %s\n"), str1);
	wrefresh(command);
	errors++;
}

/*
 * warning()
 *	Print the warning string to the command window
 */

warning(s)
char *s;
{
	wprintw(command, catgets(catd, MS_TRANS, M_TRANS_11, "warning: %s\n"), s);
	wrefresh(command);
}

/*
 * getquote()
 *	determine quote character of file being processed
 */

char
getquote(line)
char *line;
{
	int eos=0;	/* end of string flag */
	char ch;

	line += 7;	/* seven is from $quote */
	while (!eos) {
		ch = line[0];
		if ((ch == '\0') || (ch == '\n')) {
			eos++;
			break;
		}
		if (!((ch == ' ') || (ch == '\t')))
			break;	/* found  quotechar */
		line++;
	}
	if (eos) {
		error(catgets(catd, MS_TRANS, M_TRANS_12, "no quote char set"), "");
		return 0;
	}
	else
		return ch;
}


/*
 * line()
 *	Draw pretty line across screen with appropriate fields displayed.
 */

line(line, str1)
int   line;				/* line to display on		*/
char *str1;				/* optional strings to display	*/
{	int ind;

	move(line, 0);
	for (ind = 1 ; ind < COLS ; ind++)
		addch(DIVIDER);

	if (str1) {
		move(line, 5);
		addstr(str1);
	}
}

/*
 * dofile()
 *	For the message source file given attempt to open it, then open
 *	the translation output file and do the translation.
 */

dofile(name)
char *name;
{	FILE *orig, *trans;			/* file pointer */
	char *basename, *baseptr;		/* base file name */
	char *suffix;				/* file suffix */
	char ostr[LINEMAX], tstr[LINEMAX];	/* temp strings */
	char oline[LINEMAX], tline[LINEMAX];	/* input lines from files */
	char quotechar;				/* quote character */
	long line1, line2, line3;		/* input pointers */
	unsigned int n;

	/*
	 * tidy up the screen ready for new file
	 */
	quotechar = '\0';
	line1 = line2 = line3 = 0L;
	line(ORIG_START - 1, catgets(catd, MS_TRANS, M_TRANS_13, "Original"));
	line(TRANS_START - 1, catgets(catd, MS_TRANS, M_TRANS_14, "Translated"));
	line(CMD_START - 1, catgets(catd, MS_TRANS, M_TRANS_15, "Type '<ctrl>k' for help"));
	werase(original);
	werase(translate);
	werase(command);
	wrefresh(original);
	wrefresh(translate);
	wrefresh(command);
	refresh();

	/*
	 * try and open original file
	 */
	if ((orig = fopen(name, "r")) == (FILE *)NULL) {
		error(catgets(catd, MS_TRANS, M_TRANS_16, "cannot open"), name);
		return;
	}

	/*
	 * determine name of translation file
	 */
	if (oflag == FALSE) {
		if (baseptr = index(name, '.')) {
			n = baseptr - name;
			suffix = baseptr + 1;
		}
		else {
			suffix = 0;
			n = strlen(name);
		}
		basename = (char *)(malloc(n * sizeof(char)));
		strncpy(basename, name, n);
		baseptr = basename + n;
		*baseptr = '\0';
		if (suffix) {
			outname = (char *)(malloc((strlen(suffix)+n+8) * sizeof(char)));
			sprintf(outname, "%s_trans.%s", basename, suffix);
		}
		else {
			outname = (char *)(malloc((n+7) * sizeof(char)));
			sprintf(outname, "%s_trans", basename);
		}
		free(basename);
	}
		
	/* 
	 * try and open translation for reading and writing
	 * if it doesn't exist, create, and open for writing
	 */
	if ((trans = fopen(outname, "r+")) == (FILE *)NULL) {
		if ((trans = fopen(outname, "w")) == (FILE *)NULL) {
			error(catgets(catd, MS_TRANS, M_TRANS_16, "cannot open"), outname);
			fclose(orig);
			return;
		}
	}
	else {
		/*
		 * since an old trans file exists, must try and 
		 * position the file pointers where session ended
		 */
		fseek(orig, 0L, SEEK_SET);
		fseek(trans, 0L, SEEK_SET);
		while (readline(tline, LINEMAX, trans) && readline(oline, LINEMAX, orig)) {
			line3 = line2;
			line2 = line1;
			line1 = ftell(orig);
			sscanf(oline, "%s", ostr);
			sscanf(tline, "%s", tstr);
			if (strcmp(ostr, tstr)) {
				error(catgets(catd, MS_TRANS, M_TRANS_17, "file has been modified"), outname);
				return;
			}
			/*
			 * must also determine quote char
			 */
			if (!strncmp(ostr, "$quote", 6))
				quotechar = getquote(oline);
		}
	}
	dotrans(orig, trans, quotechar, line1, line2, line3);
	fclose(trans);
	fclose(orig);
}

/* 
 * dotrans()
 *	Perform the translation a line at a time.
 */

dotrans(inp, outp, quotechar, line1, line2, line3)
FILE *inp, *outp;
char quotechar;
long line1, line2, line3;
{	char inputline[LINEMAX];
	char c;
	char *start;			/* points to first char to translate */
	char *end;			/* to just after last char 	     */
	int  len;
	
	for(;;) {
		werase(command);
		line3 = line2;
		line2 = line1;
		line1 = ftell(inp);
		if ((len = readline(inputline, LINEMAX, inp)) == 0)
			break;

		/*
		 * preserve blank lines
		 */
		if (*inputline == '\n') {
			fputs(inputline, outp);
			continue;
		}

		/* 
		 * check for control or comment
		 */
		if (*inputline == '$') {

			/*
			 * if a quote directive set our quote character
			 */
			if (!strncmp(inputline, "$quote", 6))
				quotechar = getquote(inputline);

			/*
			 * simply copy across control directive
			 */
			if (inputline[1] != ' ') {
				fputs(inputline, outp);
				continue;
			}

			/* 
			 * must be a comment
			 */
			if ( ! trans_comments) {
				fputs(inputline, outp);
				continue;
			}
		}

		/*
		 * have a line to translate, isolate the text and then
		 * display in standout mode
		 */
		if (inputline[0] != '$' && quotechar) {
			start = index(inputline, quotechar);
			if (start) {
				end = rindex(inputline, quotechar);
				if (start == end) {
					warning(catgets(catd, MS_TRANS, M_TRANS_18, "unmatched quote, reinput line"));
					start = inputline;
					end = inputline + len - 1;
				} else
					/*
					 * don't translate the quote!!
					 */
					start++;
			} else {
				warning(catgets(catd, MS_TRANS, M_TRANS_19, "missing quote, reinput line"));
				start = inputline;
				end = inputline + len - 1;
			}
		} else {
			start = inputline;
			end = inputline + len - 1;
			while ((c = *start) && c != ' ' && c != '\t')
				start++;
			if (start == inputline)
				warning(catgets(catd, MS_TRANS, M_TRANS_20, "line starts with a blank, reinput line"));
			else
				/*
				 * don't translate the space!!
				 */
				start++;
		}
		display(line3, inp, line1 + (start - inputline), end - start);
		gettext(inputline, start, (*inputline == '$') ? 0 : (start == inputline ? 0 : quotechar), outp);
	}
}

/*
 * readline()
 *	Read a complete line from the input file doing any <backslash newline>
 *	processing which is required. We return the complete line which
 *	the translator wishes to work on.
 */

readline(buf, len, fp)
char *buf;
int len;
FILE *fp;
{	int   escaped;		/* TRUE if escaped newline		*/
	char *cp = buf;		/* current point in buffer		*/
	int   tlen;		/* temporary length of read		*/
	int   total = 0;	/* total length of read			*/

	do {
		if (fgets(cp, len, fp) == (char *)NULL)
			break;
		tlen = strlen(cp);
		/*
		 * true if line has an escaped newline
		 */
		escaped = (cp[tlen - 2] == '\\');
		/*
		 * update pointers and totals
		 */
		cp    += tlen;
		total += tlen;
		len   -= tlen;
	} while (escaped && len > 0);

	return total;
}	

/*
 * display()
 *	Display the context of the text to be translated. Highlight the 
 *	text which the translator is replacing.
 */

display(offset, fp, highoff, highlen)
long  offset;
FILE *fp;
long  highoff;			/* to start highlighting at 		*/
int   highlen;			/* number of characters to be highlight */
{	int c;
	long saveoffset = ftell(fp);

	werase(original);
	fseek(fp, offset, 0);
	
	while (offset++ < highoff && (c = fgetc(fp)) != EOF) {
		if(waddch(original, c) == ERR)	/* would scroll		*/
			break;
	}
	wstandout(original);
	while (highlen-- > 0 && (c = fgetc(fp)) != EOF)
		if(waddch(original, c) == ERR)	/* would scroll		*/
			break;
	wstandend(original);
	while ((c = fgetc(fp)) != EOF)
		if(waddch(original, c) == ERR)	/* would scroll		*/
			break;
	fseek(fp, saveoffset, 0);
	wrefresh(original);
}

/*
 * gettext()
 * 	Ask the user to input the translation for the text highlighted.
 *	And then ask for verification.
 */

gettext(line, start, quote, outp)
char *line;			/* line which we will be translating */
char *start;			/* start of text to be translated    */
int quote;			/* the quote character if required   */
FILE *outp;	
{	
	char	outline[LINEMAX], 	/* output line buffer */
		bufline[LINEMAX];	/* temporary line buffer */
	char	*cp, *lp;		/* character pointers */

	/* 
	 * Copy across the message number/mnemonic
	 */
	cp = outline;
	lp = line;
	while (lp < start) 
		*cp++ = *lp++;
	*cp = '\0';

	sprintf(bufline, catgets(catd, MS_TRANS, M_TRANS_21, "Translating file: %s\n"), inname);
	waddstr(command, bufline);
	wrefresh(command);

	werase(translate);
	waddstr(translate, outline);
	/*
	 * now get the line doing lots of editing
	 */
	getline(translate, command, cp, LINEMAX - (cp - line), quote);

	/*
	 * now output the translated string quotes and all
	 */
	fputs(outline, outp);
	fputc('\n', outp);
}

/*
 * inputmode()
 *	Display the current input mode on the line above the translation
 *	window. 
 */

inputmode(type)
char *type;
{	int len;

	move(TRANS_START - 1, COLS - 10);
	standout();
	addstr(type);
	standend();
	for (len = 10 - strlen(type); len > 0; len--)
		addch(DIVIDER);
	refresh();
}

#ifdef DEBUG
/*
 * Debug print routine
 * 	Set tty to another window on same machine
 */

printdbg(mesg)
char *mesg;
{
	FILE *fp;

	fp = fopen("/dev/ttys9", "w");
	fprintf(fp, "%s\n", mesg);
	fclose(fp);
}
#endif /* DEBUG */
