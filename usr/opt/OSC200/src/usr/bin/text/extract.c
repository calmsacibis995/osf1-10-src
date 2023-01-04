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
static	char	*sccsid = "@(#)$RCSfile: extract.c,v $ $Revision: 4.3.4.2 $ (DEC) $Date: 1993/10/11 19:19:46 $";
#endif lint
/*
 */
/*
 *
 *   File name: extract.c
 *
 *   Source file description:
 * 	This program is used to interactively extract message strings 
 *	from files and to allow the user to produce a new file with
 *	them rewritten as function calls. 
 *
 *   Usage:	extract [-i ignfile] [-m prefix] [-p pattern] 
 *		      [-s string ] [-u] [-n] filelist
 *
 *   Compile:   cc -o extract -O extract.c gen.c -lcurses -ltermlib
 *
 */

/*
 * Modification history
 * ~~~~~~~~~~~~~~~~~~~~
 * 004  Tom Woodburn, 21 Apr 1991
 *	- Included curses.h if OSF, cursesX.h if not.
 *	- Replaced strings with calls to catgets(3).
 *	- Changed type of windup() from int to void.
 *	- Added call to setlocale().
 *	- Changed copyright notice to DIGITAL_COPYRIGHT macro.
 *	- Changed version in BANNER from V1.0 to V1.1.
 *	- Fixed showhelp to not display comments in the help file.
 *
 * 003	David Lindner Wed Feb 28 09:20:30 EST 1990
 *	- Fixed mneumonic option so it would do set mneumonics correctly.
 *
 * 002	David Lindner Wed Jan 31 12:26:50 EST 1990
 *	- Modified init_str code so that it would increment set for
 *        each source code module processed.
 *      - Also added %s option to patterns so set would increment in
 *        new code as well.
 *	- Fixed reversal of src1head and src2head
 *
 * 001	Rainer Schaaf
 *	- Changed the behaviour of the init_str
 *	- Added cathead src1head and src2head
 *	- Changed the behaviour when working on a filelist instead 
 *	  of a file.
 *	- Added the -o option.
 *
 * 000	Andy Gadsby, 12-Jan-1987.
 *	- Created.
 *
 */

#ifdef OSF		/* TJW 004 */
#include <curses.h>
#else
#include <cursesX.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include "defs.h"
#include "extract_msg.h"

nl_catd catd;

#define BANNER	  "DEC OSF/1 String Extraction Tool V1.1" /* TJW 004 */
#define DIVIDER   '-'		/* line character			*/

#define SRC_LINES	12	/* number of lines in source window	*/
#define MSG_LINES	5	/* number of lines in message window	*/
#define CMD_LINES	3	/* number of lines in command window	*/

#define SRC_START	2	/* start positions of windows		*/
#define MSG_START	(SRC_START + SRC_LINES + 1)
#define CMD_START	(MSG_START + MSG_LINES + 1)

/*
 * This structure contains all that we need to call catgets for a message.
 * A list of these structures, opt_msgs, is used in loadopts() to get the
 * options which the user may select.  These options are stored in the
 * list opts.
 */

typedef struct _opt_msg_t {
	int msg_num;		/* message number  */
	char *def_msg;		/* default message */
} opt_msg_t;

				/* list of options which user may select */
				/* order is important here		 */

static opt_msg_t opt_msgs[] = {
	{M_EXTRACT_25, "EXTRACT"},
	{M_EXTRACT_26, "DUPLICATE"},
	{M_EXTRACT_27, "IGNORE"},
	{M_EXTRACT_28, "PASS"},
	{M_EXTRACT_29, "ADD"},
	{M_EXTRACT_30, "COMMENT"},
	{M_EXTRACT_31, "QUIT"},
	{M_EXTRACT_32, "HELP"}
};

#define NUM_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))
static char *opts[NUM_ELEMENTS(opt_msgs) + 1];	/* null-terminated	*/
static int optsize = 0;		/* size on screen of each option 	*/

#define EXTRACT	    0		/* order here matches order in opts	*/
#define DUPLICATE   1
#define IGNORE      2
#define PASS        3
#define ADD         4
#define COMMENT	    5
#define QUIT        6
#define HELP        7

static void loadopts();		/* uses opt_msgs to load opts		*/

char   *curfile;		/* current file name 			*/
char   *progname;		/* this programs invocation name	*/
int 	ac;			/* global argument count and array	*/
char  **av;			/* setup in setopts			*/
char   *ignfile = (char *)0;    /* name of ignore file, if any		*/
char   *msgprefix = "";		/* prefix for message numbers		*/

int 	errors;			/* count / file				*/
int 	usemessage = 0;		/* TRUE if we are to use old msg file   */
int     texty, textx;		/* start coordinate of text 		*/
int	iscflag = FALSE;	/* TRUE if C source			*/
int  	firsttime = TRUE;	/* stops us clearing strings		*/
int	new_msg_cat = FALSE;	/* flag wether to make one or more msgcats */

extern char   *init_str;	/* from re.c				*/
extern char   src1head[];	/* from re.c				*/
extern char   src2head[];	/* from re.c				*/

main(argc, argv)
int 	argc;
char   *argv[];
{ 	void	windup();		/* interrupt handler		*/

	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_EXTRACT, NL_CAT_LOCALE);

	setopts(argc, argv);		/* initialise			*/
	loadopts();			/* load selectable options	*/
	loadre();
	loadignore(ignfile);
	signal(SIGINT, windup);
	startterm();

	while (ac-- > 0) {		/* for each file name		*/
		curfile = *av++;
		errors = 0;
		if ( ! firsttime) { 	/* clear out duplicate string	*/
			clearstr();
		}
		dofile(curfile);
		firsttime = FALSE;
	}
	windup();			/* tidy up and exit		*/
	/* NOTREACHED */
}


/*
 * setopts()
 *	set the option flags based on the command line arguments, also
 *	sets ac and av to point to the start of the file list.
 */

setopts(argc, argv)
int    argc;
char **argv;
{	int  num;
	char c;
	extern char *refile;		/* in re.c			*/
	
	progname = argv[0];

	for (num = 1; num < argc; num++) {
		if (*argv[num] == '-')
			switch(c = argv[num][1]) {
			default:
				fprintf(stderr, catgets(catd, MS_EXTRACT, M_EXTRACT_1, "%s: bad option %c\n"), progname, c);
				usage();
				/* NOTREACHED */
			case 'n':
				new_msg_cat = TRUE;
				break;
			case 'c':
				iscflag = TRUE;
				break;
			case 'i':
				ignfile = argv[++num];
				break;
			case 'p':
				refile = argv[++num];
				break;
			case 'u':
				usemessage++;
				break;
			case 'm':
				msgprefix = argv[++num];
				break;
			case 's':
				init_str = argv[++num];
				break;
			}
		else {
			ac = argc - num;
			av = &argv[num];
			return;
		}
	}
	usage();
	/* NOTREACHED */
}

usage()
{
	fprintf(stderr, catgets(catd, MS_EXTRACT, M_EXTRACT_2, "usage: %s\n[-p patterns] [-i ignore] [-u] [-n] [-m prefix] [-s string] filelist\n"), progname);
	exit(1);
}

/*
 * loadopts()
 *	load the selectable options from the message catalog
 */

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

static void
loadopts()
{
	opt_msg_t *m;
	opt_msg_t *last_m;
	char **o;
	char *s;
	int size;

	/*
	 * lookup each option and make a copy of it
	 */
	last_m = opt_msgs + NUM_ELEMENTS(opt_msgs);
	for (m = opt_msgs, o = opts; m < last_m; m++, o++) {
		s = catgets(catd, MS_EXTRACT, m->msg_num, m->def_msg);

		/*
		 * need to know the longest option to space them evenly
		 */
		size = strlen(s) + 1; /* "+ 1" is for EOS */
		optsize = MAX(optsize, size);

		if ((*o = (char *) malloc(size)) == (char *) NULL) {
			fprintf(stderr,
				catgets(catd, MS_EXTRACT, M_EXTRACT_33,
					"%s: malloc failed in loadopts"),
				progname);
			exit(1);
		}

		strcpy(*o, s);
	}

	/*
	 * the list of options needs to be null-terminated
	 */
	*o = (char *) NULL;
}

/*
 * startterm()
 *	setup the terminal with the appropriate	windows using curses.
 */

WINDOW *source;				/* for the source code		*/
WINDOW *message;			/* for the extracted messages   */
WINDOW *command;			/* for communication with user  */

startterm()
{
	char *banner;

	initscr();
	crmode();
	clear();
	banner = catgets(catd, MS_EXTRACT, M_EXTRACT_21, BANNER);
	move(0, COLS/2 - strlen(banner)/2);
	addstr(banner);

	source  = subwin(stdscr, SRC_LINES, 0, SRC_START, 0);
	message = subwin(stdscr, MSG_LINES, 0, MSG_START, 0);
	command = subwin(stdscr, CMD_LINES, 0, CMD_START, 0);
	scrollok(message, TRUE);
	scrollok(command, TRUE);

	refresh();
}

/* 
 * windup()
 *	reset the terminal
 */

void windup()
{
	signal(SIGINT, SIG_IGN);
	mvcur(0, COLS - 1, LINES -1, 0);
	endwin();
	putchar('\n');
	fflush(stdout);

	exit(errors ? 1 : 0);
}

/* 
 * error()
 *	Print the two string passed as arguments to command window
 */

error(str1, str2)
char *str1, *str2; 
{
	wprintw(command, catgets(catd, MS_EXTRACT, M_EXTRACT_3, "Error: %s %s\n"), str1, str2);
	errors++;
}
	

/*
 * line()
 *	Draw pretty line across screen with appropriate fields displayed.
 */


line(line, str1, str2)
int   line;				/* line to display on		*/
char *str1, *str2;			/* optional strings to display	*/
{	int ind;

	move(line, 0);
	for (ind = 1 ; ind < COLS ; ind++)
		addch(DIVIDER);

	if (str1) {
		move(line, 5);
		addstr(str1);
	}
	if (str2)
	{
		move(line, 5 + (str1 ?  strlen(str1) : 0));
		addstr(str2);
	}
}

/*
 * dofile()
 *	For the message file given attempt to open it, the corresponding
 *	catalogue file, the original source and the new source!
 */

dofile(name)
char *name;
{	FILE *mp, *op, *np;
	char *tmpname;			/* temporary file name		*/
	static FILE *cp;
	static int setnum=0;		/* set number DJL 002		*/

	/*
	 * tidy up the screen ready for new file
	 */
	line(SRC_START - 1, catgets(catd, MS_EXTRACT, M_EXTRACT_4, "Source: "), curfile);
	line(MSG_START - 1, catgets(catd, MS_EXTRACT, M_EXTRACT_5, "Catalogue: "), fixsuffix(curfile, CATSUFFIX));
	line(CMD_START - 1, (char *)NULL, (char *)NULL);

	werase(source);
	werase(message);
	werase(command);
	wrefresh(source);
	wrefresh(message);
	wrefresh(command);
	refresh();

	if ( ! usemessage)
		if (getmsg(name) == ERROR)
			return;

	curfile = name;
	tmpname = fixsuffix(name, MSGSUFFIX);
	if ((mp = fopen(tmpname, "r")) == (FILE *)NULL) {
		error(catgets(catd, MS_EXTRACT, M_EXTRACT_6, "cannot open"), tmpname);
		return;
	}
	if ((op = fopen(name, "r")) == (FILE *)NULL) {
		error(catgets(catd, MS_EXTRACT, M_EXTRACT_6, "cannot open"), name);
		fclose(mp);
		return;
	}
	tmpname = fixprefix(INTPREFIX, name);
	if ((np = fopen(tmpname, "w+")) == (FILE *)NULL) {
		error(catgets(catd, MS_EXTRACT, M_EXTRACT_6, "cannot open"), tmpname);
		fclose(mp);
		fclose(op);
		/* fclose(cp); */
		return;
	}
	werase(message);

	/*
	 * message catalogue is opened only once
	 * the header of the message catalogue is written to the file
	 * (the default is the init_str)
	 */
	if (firsttime || new_msg_cat) {
	    tmpname = fixsuffix(name, CATSUFFIX);
	    if ((cp = fopen(tmpname, "w")) == (FILE *)NULL) {
		error(catgets(catd, MS_EXTRACT, M_EXTRACT_6, "cannot open"), tmpname);
		fclose(mp);
		fclose(op);
		return;
	    }
	    if (*init_str) {			/* DJL 002 */
		    fputs(init_str, cp);
		    wprintw(message, "%s", init_str);
		    if (*msgprefix) {		/* DJL 003 */
			fprintf(cp, "\n$set S_%s%d\n", msgprefix, ++setnum);
			wprintw(message, "\n$set S_%s%d\n", msgprefix, setnum);
		    }
		    else {
		    	fprintf(cp, "\n$set %d\n", ++setnum);
		    	wprintw(message, "\n$set %d\n", setnum);
		    }
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

	wrefresh(message);

	processfile(mp, cp, op, np, setnum);

	fclose(op);
	fclose(np);
	/* fclose(cp); */
	fclose(mp);
	if ( ! usemessage)
		unlink(fixsuffix(name, MSGSUFFIX));
}

/*
 * getmsg()
 *	Run the string extract file to produce a new message catalogue
 *	for the file. We popen() and collect the stderr so that we
 *	can let the user see the prescan. On exit the msg file will
 *	have been produced.
 */

getmsg(name)
char *name;
{	FILE *open();
	FILE *errp;
	char ebuf[LINESIZE];
	char cmd[LINESIZE];
	extern char *refile, *ignfile;
	char msgbuf[NL_TEXTMAX];
	int ans;
	char *no;

	strcpy(cmd, STREXTRACT);
	strcat(cmd, " -d ");
	if (refile) {
		strcat(cmd, "-p ");
		strcat(cmd, refile);
		strcat(cmd, " ");
	}
	if (ignfile) {
		strcat(cmd, "-i ");
		strcat(cmd, ignfile);
		strcat(cmd, " ");
	}
	strcat(cmd, name);
	strcat(cmd, " 2>&1");
	
	if ((errp = popen(cmd, "r")) == (FILE *)NULL) {	
		error(catgets(catd, MS_EXTRACT, M_EXTRACT_7, "cannot popen "), STREXTRACT);
		return ERROR;
	}
	while (fgets(ebuf, LINESIZE, errp) != (char *)NULL) {
		waddstr(source, ebuf);
		wrefresh(source);
		errors++;
	}
	fclose(errp);

	if (errors) {

	        /*
		 * Want to pass two separate messages to answer(), but
		 * catgets() stores the message it returns in a static
		 * buffer.  A second call to catgets() then overwrites
		 * the message returned by the first call.
		 *
		 * The solution to this problem is to copy the message
		 * returned by the first call into a buffer of our own,
		 * msgbuf.
		 */

	        strcpy(msgbuf, catgets(catd, MS_EXTRACT, M_EXTRACT_9, "yn"));
		ans = answer(command, catgets(catd, MS_EXTRACT, M_EXTRACT_8, "Errors were found. Continue (y/n)? "), msgbuf);
		no  = catgets(catd, MS_EXTRACT, M_EXTRACT_22, "n");
		if (ans == *((unsigned char *) no))
			return ERROR;
		waddch(command, '\n');
	}
	werase(source);
	return OK;
}


/* 
 * processfile()
 *	Scan through the message file and for each line modify the old
 *	program using the REWRITE rule given in the patterns file.
 */

long  line3, line2, line1;		/* offsets to lines seen	*/

processfile(msg, cat, old, new, setnum)
FILE *msg, *cat, *old, *new;		/* file pointers		*/
int setnum;				/* set number DJL 002		*/
{	
	static int  msgnum = 1;		/* current message number	*/
	int  linenum;			/* line field from msgline	*/
	long offset;			/* offset field from msgline	*/
	int  len;			/* length of text		*/
	char text[LINESIZE];		/* the actual text		*/
	int  num;			/* message number to use	*/
	int  cmd;			/* actual command user gave	*/
	struct element *match;		/* pointer to a matching string */
	struct element elem;		/* used to save a string	*/
	char comment[LINESIZE];		/* temporary used for comments	*/
	char msgbuf[NL_TEXTMAX];
	int ans;
	char *yes;
	char *quit;
	
	line3 = line2 = line1 = 0L;
	while (fscanf(msg, "%d %ld %d %[^\n]", &linenum, &offset, &len, text) != EOF) {
					/* see if we have seen this	*/
		match = lookupstr(text, len);	
					/* skip if we are ignoring text	*/
		if (match && (match->flags & STR_IGNORE))
			continue;

		/* display the text */
		copy(old, new, offset);
		display(line3, new, old, len);
		wrefresh(source);

again:
		switch(cmd = getcommand(match ? DUPLICATE : EXTRACT )) {
		case DUPLICATE:
			if ( ! match) {
				answer(command, catgets(catd, MS_EXTRACT, M_EXTRACT_10, "No string to duplicate. Press <RETURN> "), "\n");
				goto again;
			}
			num = match->msgnum;
			/* FALL THROUGH */
		case EXTRACT:
			if (cmd != DUPLICATE)
				num = msgnum++;
			wmove(source, texty, textx);
			wclrtobot(source);
			wmove(source, texty, textx);

			/* wstandout(source); */
			rewrite(text, len, setnum, num, new);
			/* wstandend(source); */

			fseek(old, (long)len, 1);
			redisplay(old);
			wrefresh(source);

			if (cmd != DUPLICATE) {
				wprintw(message, "%s%d\t%s\n", msgprefix, num, text);
				wrefresh(message);
				fprintf(cat, "%s%d\t%s\n", msgprefix, num, text);
				elem.len = len;
				elem.flags = 0;
				elem.linenum = linenum;
				elem.msgnum = num;
				savestr(text, &elem);
			}
			break;
		case IGNORE:			/* ignore text now	*/
		case ADD:			/* add to ignore file	*/
			elem.len = len;
			elem.flags = STR_IGNORE;
			elem.linenum = linenum;
			savestr(text, &elem);
			if (cmd == IGNORE)
				break;
			if (addignore(ignfile, text, len) == ERROR)
				answer(command, catgets(catd, MS_EXTRACT, M_EXTRACT_11, "Cannot write to ignore file. Press <RETURN> "), "\n");
			break;
		case PASS:			/* pass by this string	*/
			continue;
		case COMMENT:			/* put text in message file */
			for (;;) {
				int y,x;

				waddstr(command, catgets(catd, MS_EXTRACT, M_EXTRACT_12, "Input text for message file and press <RETURN>\n"));
				wrefresh(command);
				wgetstr(message, comment);
				strcpy(msgbuf, catgets(catd, MS_EXTRACT, M_EXTRACT_14, "ynq"));
				ans = answer(command, catgets(catd, MS_EXTRACT, M_EXTRACT_13, "Text OK (y/n/q) ? "), msgbuf);
				yes = catgets(catd, MS_EXTRACT, M_EXTRACT_23, "y");
				if (ans == *((unsigned char *) yes))
					break;

				getyx(message, y, x);
				wmove(message, y - 1, x);
				wdeleteln(message);
				wrefresh(message);
				quit = catgets(catd, MS_EXTRACT, M_EXTRACT_24, "q");
				if (ans == *((unsigned char *) quit))
					goto again;
			}
			wrefresh(message);
			fprintf(cat, "%s\n", comment);
			goto again;
		case QUIT:
			strcpy(msgbuf, catgets(catd, MS_EXTRACT, M_EXTRACT_9, "yn"));
			ans = answer(command, catgets(catd, MS_EXTRACT, M_EXTRACT_15, "Really quit (y/n) ? "), msgbuf);
			yes = catgets(catd, MS_EXTRACT, M_EXTRACT_23, "y");
			if (ans == *((unsigned char *) yes)) {
				errors++;	/* to set exit status	*/
				windup();	/* exit() will cleanup	*/
				/* NOTREACHED */
			}
			goto again;
		case HELP:
			showhelp();
			goto again;
		}
	}
	copy(old, new, (long)EOF);		/* copy to EOF		*/
}

/*
 * getcommand()
 *	Display the options available for the user and get the users
 *	choice. The default will be highlighted.
 */

getcommand(def)
int def;				/* the default selection	*/
{	int inp;			/* users input character	*/
	int opt;			/* selected option		*/
	char **cp;			/* pointer to list of options	*/

	opt = def;
	werase(command);
	for (;;) {
		dispopt(opt);
		wmove(command, 1, 0);
		waddstr(command, catgets(catd, MS_EXTRACT, M_EXTRACT_16, "Select option and press <RETURN> "));
		wrefresh(command);
		inp = wgetch(command);
		if (inp == '\n')
			return opt;
		inp = toupper(inp);
		for (cp = opts; *cp; cp++)
			if (**((unsigned char **) cp) == inp) {
				opt = cp - opts;
				break;
			}
	}
}

/*
 * dispopt()
 *	Display the options available for the user and highlight the
 *	appropriate one.
 */

dispopt(high)
int high;				/* option number to highlight   */
{	int col = 0;			/* current column number	*/
	char **cp;			/* pointer to list of options	*/
	
	for (cp = opts; *cp; cp++) {
		wmove(command, 0, col);
		col += optsize;
		if (high == 0)
			wstandout(command);
		waddstr(command, *cp);
		if (high-- == 0)
			wstandend(command);
	}
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
			error(catgets(catd, MS_EXTRACT, M_EXTRACT_17, "Bad offset in"), curfile);
			return ERROR;
		}
		while (current++ < offset) {
			if ((c = fgetc(old)) == EOF) {
				error(catgets(catd, MS_EXTRACT, M_EXTRACT_18, "Unexpected EOF in "), curfile);
				return ERROR;
			}
			fputc(c, new);
			if (c == '\n') {
				line3 = line2;
				line2 = line1;
				line1 = ftell(new);
			}
		}
	return OK;
}

/*
 * display()
 */


display(newoffset, new, old, highlen)
long  newoffset;
int   highlen;
FILE *new, *old;
{	int c;
	long saveoffset = ftell(old);

	werase(source);
	fseek(new, newoffset, 0);
	
	while ((c = fgetc(new)) != EOF)
		if(waddch(source, c) == ERR)	/* would scroll		*/
			break;
	wstandout(source);
	getyx(source, texty, textx);
	while (highlen-- > 0 && (c = fgetc(old)) != EOF)
		if(waddch(source, c) == ERR)	/* would scroll		*/
			break;
	wstandend(source);
	while ((c = fgetc(old)) != EOF)
		if(waddch(source, c) == ERR)	/* would scroll		*/
			break;
	fseek(new, 0L, 2);
	fseek(old, saveoffset, 0);
}


/* 
 * redisplay()
 *	Redo the display from the current position onto the source 
 *	window, fills up until the screen would scroll.
 */

redisplay(fp)
FILE *fp;
{	int c;
	long saveoffset;

	saveoffset = ftell(fp);

	while ((c = fgetc(fp)) != EOF) 
		if (waddch(source, c) == ERR)
			break;
	fseek(fp, saveoffset, 0);
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
int setnum;	/* DJL 002 */
int  msgnum;
FILE *new;
{	extern char rewstring[];
	char *rp, *tp, *cp;
	char c, save;

	for (rp = rewstring; c = *rp; rp++) {
		if (c != '%')  {
			waddch(source, c);
			fputc(c, new);
			if (c == '\n') {
				line3 = line2;
				line2 = line1;
				line1 = ftell(new);
			}
		}
		else
			switch(c = *++rp) {
			default:
				error(catgets(catd, MS_EXTRACT, M_EXTRACT_19, "Bad rewrite string in pattern file"), rewstring);
				return ERROR;
			case '%':
				waddch(source, '%');
				fputc('%', new);
				break;
			case 'n':
				wprintw(source, "%s%d", msgprefix, msgnum);
				fprintf(new, "%s%d", msgprefix, msgnum);
				break;
			case 's':	/* DJL 002 */
				if (*msgprefix) {	/* DJL 003 */
					wprintw(source, "S_%s%d", msgprefix, setnum);
					fprintf(new, "S_%s%d", msgprefix, setnum);
				}
				else {
					wprintw(source, "%d", setnum);
					fprintf(new, "%d", setnum);
				}
				break;
			case 'l':
				wprintw(source, "%d", len);
				fprintf(new, "%d", len);
				break;
			case 't':
				waddstr(source, text);
				for (cp = text; c = *cp; cp++) {
					if (c == '\n') {
						line3 = line2;
						line2 = line1;
						line1 = ftell(new);
					}
					fputc(c, new);
				}
				break;
			case 'r':
				tp = text + strlen(text) - 1;
				save = *tp;
				*tp = '\0';
				waddstr(source, text + 1);
				for (cp = text + 1; c = *cp; cp++) {
					if (c == '\n') {
						line3 = line2;
						line2 = line1;
						line1 = ftell(new);
					}
					fputc(c, new);
				}
				*tp = save;
				break;
			case 'N':
				line3 = line2;
				line2 = line1;
				line1 = ftell(new);
				fputc('\n', new);
				waddch(source, '\n');
				break;
			case 'T':
				fputc('\t', new);
				waddch(source, '\t');
				break;
			}
	}
	return OK;
}
 

/* 
 * answer()
 *	Get a one letter answer to the question through window
 */

answer(win, question, valid)
WINDOW *win;
char   *question, *valid;
{	char *cp;
	int   c;

	for (;;) {
		waddstr(win, question);
		wrefresh(win);
		c = wgetch(win);
		waddch(win, '\n');
		c = tolower(c);
		for (cp = valid; *cp; cp++)
			if (c == *cp)
				return c;
	}

	return 0;
}


/*
 * showhelp()
 * 	Display some useful help text on the screen. 
 */

showhelp()
{	static WINDOW *helpwin = (WINDOW *)NULL;
	static FILE *hp = (FILE *)NULL;
	char  line[LINESIZE];

	if (helpwin == (WINDOW *)NULL) {
		hp = fopen(HELPFILE, "r");
		helpwin = newwin(LINES - 1, COLS, 1, 0);
	}
	if (hp == (FILE *)NULL)
		return ERROR;
	fseek(hp, 0L, 0);		/* rewind			*/
	werase(helpwin);		/* blast previous screen	*/
	touchwin(helpwin);
	
					/* read and display text	*/
	while (fgets(line, LINESIZE, hp))
		if (*line != '#')
			waddstr(helpwin, line);
	wrefresh(helpwin);

	answer(helpwin, catgets(catd, MS_EXTRACT, M_EXTRACT_20, "Press <RETURN> to continue "), "\n");
	touchwin(stdscr);		/* now restore original window	*/
	refresh();
	return OK;
}
