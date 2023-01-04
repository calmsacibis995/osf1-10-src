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
static char rcsid[] = "@(#)$RCSfile: main.c,v $ $Revision: 4.3.9.12 $ (DEC) $Date: 1993/12/21 21:08:45 $";
#endif
/*
 * HISTORY
 */
/*
 * $OSF_Log:	main.c,v $
 * HISTORY
 * Revision 1.1.1.6  93/01/07  08:44:00  devrcs
 *  *** OSF1_1_2B07 version ***
 * 
 * Revision 1.8.3.4  1992/11/03  23:52:47  tom
 * 	Bug 8134: Increase size of message buffer.
 * 	[1992/11/03  23:51:30  tom]
 *
 * Revision 1.8.3.3  1992/10/26  21:38:28  tom
 * 	The MORE environment variable should be treated just like an argv.
 * 	[1992/10/26  21:37:32  tom]
 * 
 * Revision 1.8.3.2  1992/08/24  18:17:40  tom
 * 	New more for POSIX.2/XPG4.
 * 	[1992/08/24  17:30:42  tom]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988 Mark Nudleman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 Mark Nudleman.\n\
@(#) Copyright (c) 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)main.c	5.13 (Berkeley) 6/1/90";
#endif /* not lint */

/*
 * Entry point, initialization, miscellaneous routines.
 */

#include <sys/types.h>
#include <sys/file.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include "less.h"

int	ispipe;
int	new_file;
int	is_tty;
int	tty;			/* keyboard input */
char	*current_file, *previous_file, *current_name, *next_name;
char	*first_cmd = NULL;
char	*every_first_cmd = NULL;
off_t	prev_pos;
int	any_display;
int	scroll;
int	ac;
char	**av;
int	curr_ac;
int	quitting;
int	mb_cur_max;
int	mbcodeset;
nl_catd	catd;
extern int help_mode;
char	*myself;
int	read_failure = 0;

static void	cat_file(void);

extern int	file;
extern int	cbufs;
extern int	errmsgs;

extern char	*tagfile;
extern int	tagoption;
extern int	top_scroll;

/*
 * Edit a new file.
 * Filename "-" means standard input.
 * No filename means the "current" file, from the command line.
 */
int
edit(register char *filename)
{
	register int f;
	register char *m;
	off_t initial_pos;
	static int didpipe;
	char message[PATH_MAX*2], *p;

	initial_pos = NULL_POSITION;
	if (filename == NULL || *filename == '\0') {
		if (curr_ac >= ac) {
			error(MSGSTR(NOCURR, "No current file"));
			return(0);
		}
		filename = save(av[curr_ac]);
	}
	else if (strcmp(filename, "#") == 0) {
		if (*previous_file == '\0') {
			error(MSGSTR(NOPREV, "No previous file"));
			return(0);
		}
		filename = save(previous_file);
		initial_pos = prev_pos;
	} else
		filename = save(filename);

	/* use standard input. */
	if (!strcmp(filename, "-")) {
		if (didpipe) {
			error(MSGSTR(NOVIEW, "Can view standard input only once"));
			return(0);
		}
		f = 0;
	}
	else if ((m = bad_file(filename, message, sizeof(message))) != NULL) {
		error(m);
		free(filename);
		read_failure++;
		return(-1);
	}
	else if ((f = open(filename, O_RDONLY, 0)) < 0) {
		(void)sprintf(message, "%s: %s", filename, strerror(errno));
		error(message);
		free(filename);
		return(0);
	}

	if (isatty(f)) {
		/*
		 * Not really necessary to call this an error,
		 * but if the control terminal (for commands)
		 * and the input file (for data) are the same,
		 * we get weird results at best.
		 */
		error(MSGSTR(NOTERM, "Can't take input from a terminal"));
		if (f > 0)
			(void)close(f);
		(void)free(filename);
		return(0);
	}

	/*
	 * We are now committed to using the new file.
	 * Close the current input file and set up to use the new one.
	 */
	if (file > 0)
		(void)close(file);
	new_file = 1;
	if (previous_file != NULL)
		free(previous_file);
	previous_file = current_file;
	current_file = filename;
	pos_clear();
	prev_pos = position(TOP);
	ispipe = (f == 0);
	if (ispipe) {
		didpipe = 1;
		current_name = MSGSTR(STDIN, "stdin");
	} else
	/* If in the help mode, set current_name to 'Help'. */
	if(help_mode == 1){
		current_name = "Help";
	}
	else{
		current_name = (p = rindex(filename, '/')) ? p + 1 : filename;
	}
	if (curr_ac >= ac)
		next_name = NULL;
	else
		next_name = av[curr_ac + 1];
	file = f;
	ch_init(cbufs, 0);
	init_mark();

	if (every_first_cmd != NULL)
		first_cmd = every_first_cmd;

	if (is_tty) {
		int no_display = !any_display;
		any_display = 1;
		if (no_display && errmsgs > 0) {
			/*
			 * We displayed some messages on error output
			 * (file descriptor 2; see error() function).
			 * Before erasing the screen contents,
			 * display the file name and wait for a keystroke.
			 */
			error(filename);
		}
		/*
		 * Indicate there is nothing displayed yet.
		 */
		if (initial_pos != NULL_POSITION)
			jump_loc(initial_pos);
		clr_linenum();
	}
	return(1);
}

/*
 * Edit the next file in the command line list.
 */
void
next_file(int n)
{
int curr_ac_save = curr_ac;
extern int quit_at_eof;

	off_t position();

	if (curr_ac + n >= ac) {
		if (!quit_at_eof || position(TOP) == NULL_POSITION)
			quit();
		error(MSGSTR(NONTHF, "No (N-th) next file"));
	}
	else {
try_next:
		if(edit(av[curr_ac += n]) == -1){
			if(curr_ac + 1 >= ac){
				curr_ac = curr_ac_save;
				n = 0;
				goto try_next;
			}
			n = 1;
			goto try_next;
		}
	}
}

/*
 * Edit the previous file in the command line list.
 */
void
prev_file(int n)
{
int curr_ac_save = curr_ac;

	if (curr_ac - n < 0)
		error(MSGSTR(NONTHF2, "No (N-th) previous file"));
	else {
try_prev:
		if(edit(av[curr_ac -= n]) == -1){
			if(curr_ac - 1 < 0){
				curr_ac = curr_ac_save;
				n = 0;
				goto try_prev;
			}
			n = 1;
			goto try_prev;
		}
	}

}

/*
 * copy a file directly to standard output; used if stdout is not a tty.
 * the only processing is to squeeze multiple blank input lines.
 */
static void
cat_file(void)
{
	extern int squeeze;
	register wint_t c;
	register int empty;

	if (squeeze) {
		empty = 1;
		while ((c = ch_forw_get()) != EOI)
			if (c != L'\n') {
				putwchr(c);
				empty = 0;
			}
			else if (empty < 2) {
				putwchr(c);
				++empty;
			}
	}
	else while ((c = ch_forw_get()) != EOI)
		putwchr(c);
	flush();
}

int
main(int argc, char **argv)
{
	int envargc, argcnt;
#define	MAXARGS		50			/* should be enough */
	char *envargv[MAXARGS+1];
	char *p;

	setlocale( LC_ALL, "");
	catd = catopen(MF_MORE,NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;
#ifdef MBCODESET
	mbcodeset = 1;
#else	
	mbcodeset = mb_cur_max > 1;
#endif

	/* myself is used instead of more, when "/bin/csh -c "more -h /tmp/aaa..." */
	myself = argv[0];
	
	if ((p = strrchr(argv[0], '/')) == NULL)
		p = argv[0];
	else
		p++;
	if (strcmp(p, "page") == 0)
		top_scroll = 1;

	/*
	 * Process command line arguments and MORE environment arguments.
	 * Command line arguments override environment arguments.
	 */
	if ((p = getenv("MORE")) && *p) {
		int i = 2;

		p = strdup(p);

		envargv[0] = "more";
		envargv[1] = strtok(p, "\t ");
		while ((p = strtok(NULL, "\t ")) != NULL && i < MAXARGS)
			envargv[i++] = p;
		envargv[i] = NULL;
		envargc = i;
		(void)option(envargc, envargv);
	}
	argcnt = option(argc, argv);
	argv += argcnt;
	argc -= argcnt;

	/*
	 * Set up list of files to be examined.
	 */
	ac = argc;
	av = argv;
	curr_ac = 0;

	/*
	 * Set up terminal, etc.
	 */

	get_term();
	is_tty = isatty(1);
	if (!is_tty) {
		/*
		 * Output is not a tty.
		 * Just copy the input file(s) to output.
		 */
		if (ac < 1) {
			(void)edit("-");
			cat_file();
		} else {
			do {
				(void)edit((char *)NULL);
				if (file >= 0)
					cat_file();
			} while (++curr_ac < ac);
		}
		exit(0);
	}

	open_getchr();
	raw_mode(1);
	init();
	init_signals(1);

	/* select the first file to examine. */
	if (tagoption) {
		/*
		 * A -t option was given; edit the file selected by the
		 * "tags" search, and search for the proper line in the file.
		 */
		if (!tagfile || !edit(tagfile) || tagsearch())
			quit();
	}
	else if (ac < 1)
		(void)edit("-");	/* Standard input */
	else {
		/*
		 * Try all the files named as command arguments.
		 * We are simply looking for one which can be
		 * opened without error.
		 */
		do {
			(void)edit((char *)NULL);
		} while (file < 0 && ++curr_ac < ac);
	}

	if (file >= 0)
		commands();
	quit();
	/*NOTREACHED*/
}

/*
 * Copy a string to a "safe" place
 * (that is, to a buffer allocated by malloc).
 */
char *
save(char *s)
{
	char *p;

	p = malloc(strlen(s)+1);
	if (p == NULL)
	{
		error(MSGSTR(NOMEM, "cannot allocate memory"));
		quit();
	}
	return(strcpy(p, s));
}

/*
 * Exit the program.
 */
void
quit(void)
{
	extern char *help_file;
	/*
	 * Clear the line,
	 * reset the terminal modes, and exit.
	 */
	quitting = 1;
	remove(help_file);
	(void) move_bol();
	clear_eol();
	/* Turn off the bold mode */
	bo_exit();
	deinit();
	flush();
	raw_mode(0);
	if(read_failure)
		exit(1);
	else
		exit(0);
}
