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
static char	*sccsid = "@(#)$RCSfile: ask_print.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:26 $";
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
/* Copyright (c) 1988-1990 SecureWare, Inc.
 *   All rights reserved
 */



/* routine to ask the user whether he wants output to go to file, printer
 * or terminal.  If not interrupted, the command with the arguments specified
 * is executed.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include "userif.h"
#include <stdio.h>
#include <setjmp.h>
#include <sys/signal.h>
#include <ctype.h>
#include <term.h>
#include <string.h>
#ifdef DEBUG
extern	FILE *logfp;
#endif

#define FPTWIDTH 45
struct	print_stat {
	char	fpt[2];		/* file/printer/terminal */
	char	dest[FPTWIDTH+1]; /* destination (file, printer, or pager) */
	long	pagewidth;	/* in characters */
	long	pagelength;	/* in lines */
};

#define LPSTAT_COMMAND	"/usr/bin/lpstat"
#define PRINTDESTCMD	"/usr/bin/lpstat -d"
#define PRINTCLASSCMD	"/usr/bin/lpstat -c"
#define PRINTDEVCMD	"/usr/bin/lpstat -v"
#define PRINTFILECMD	"/usr/bin/lp"
#define DEFAULT_PWIDTH	80
#define DEFAULT_PLENGTH 64
#define DEFAULT_PAGER	"/usr/bin/pg"
#define TEMPFILETEMPLATE	"/tmp/lpXXXXXX"
#define FILECHAR	'F'
#define PRINTCHAR	'P'
#define TERMCHAR	'T'
#define DEFAULT_SHELL	"SHELL=/sbin/sh"

extern char *getenv();

static void set_default_dest();
static int check_printstat(), matchclass(), matchdev(), ps_run_program(),
		ps_child(), executable();


/*
---------------------------------------------------
|                                                 |
|                  Output Options                 |
|                                                 |
|       (F)ile, (P)rinter, (T)erminal:  _         |
|                                                 |
|             File/Printer/Pager Name:            |
|  _____________________________________________  |
|                                                 |
|              Page Length:    ___                |
|              Page Width:     ___                |
|                                                 |
---------------------------------------------------
*/

static
struct	scrn_desc	fpt_desc[] = {
{  2, 19, FLD_PROMPT, 0, FLD_OUTPUT, NO, "Output Options" },		/* 0 */
{  4,  8, FLD_PROMPT, 0, FLD_OUTPUT, NO,
	"(F)ile, (P)rinter, (T)erminal:" },				/* 1 */
{  4, 40, FLD_ALPHA,  1, FLD_BOTH,   NO, NULL, "print/1" },		/* 2 */
{  6, 14, FLD_PROMPT, 0, FLD_OUTPUT, NO,
	"File/Printer/Pager Name:" },					/* 3 */
{  7,  3, FLD_ALPHA, FPTWIDTH, FLD_BOTH, NO, NULL, "print/2" },		/* 4 */
{  9, 15, FLD_PROMPT, 0, FLD_OUTPUT, NO,
	"Page Length:" },						/* 5 */
{  9, 31, FLD_NUMBER, 3, FLD_BOTH,   NO, NULL, "print/3" },		/* 6 */
{ 10, 15, FLD_PROMPT, 0, FLD_OUTPUT, NO,
	"Page Width:" },						/* 7 */
{ 10, 31, FLD_NUMBER, 3, FLD_BOTH,   NO, NULL, "print/4" }		/* 8 */
};

static
struct	scrn_parms	fpt_scrn = {
	SCR_FILLIN,	/* scrn_type  */
	4,		/* top row */
	(80 - FPTWIDTH - 6) / 2, /* left column */
	14,		/* number of rows */
	FPTWIDTH + 6,		/* number of columns */
	NUMDESCS(fpt_desc),/* number of scrn_descs */
	fpt_desc,	/* table of descs */
};

static
struct	scrn_struct	fpt_struct[] = {
{ NULL, 0, 1, 2 },
{ NULL, 0, 1, 4 },
{ NULL, 0, 1, 6 },
{ NULL, 0, 1, 8 }
};

#define	FPTNAME	0
#define FPTDEST 1
#define PLENGTH 2
#define PWIDTH  3

print_output (program, argv)
char	*program;		/* program to execute */
char	*argv[];		/* argument list of program */
{
	struct	print_stat	ps;
	int	i;
	int	ret;
	WINDOW	*window;
	struct	scrn_ret	scrn_ret;
	extern	jmp_buf	env;

#ifdef DEBUG
fprintf (logfp, "print_output: program \'%s\'.\nargs:");
for (i = 1; argv[i]; i++)
	fprintf (logfp, " %s", argv[i]);
fprintf (logfp, "\n");
#endif

	/* set up structure to point to screen items */

	fpt_struct[FPTNAME].pointer = ps.fpt;
	fpt_struct[FPTNAME].changed = 0;
	fpt_struct[FPTDEST].pointer = ps.dest;
	fpt_struct[FPTDEST].changed = 0;
	fpt_struct[PWIDTH].pointer = (char *) &ps.pagewidth;
	fpt_struct[PWIDTH].changed = 0;
	fpt_struct[PLENGTH].pointer = (char *) &ps.pagelength;
	fpt_struct[PLENGTH].changed = 0;

	ps.fpt[0] = TERMCHAR;
	ps.fpt[1] = '\0';
	set_default_dest (&ps);
	ps.pagelength = LINES - 1;
	ps.pagewidth  = COLS;
loop:
	if (ret = setjmp (env))
		return (ret);
	set_default_dest (&ps);
	window = putscreen (&fpt_scrn, fpt_struct, POP);
	scrn_ret = getscreen (window, &fpt_scrn, fpt_struct, 0);
	if ((scrn_ret.flags & R_ABORTED) ||
	    (scrn_ret.flags & R_QUIT))
		return (1);
		/* check validity of responses */
	if (check_printstat (&ps))
		goto loop;
	/* execute program with specified args, piping either to printer,
	 * file, or terminal pager.
	 */
	
	ps_run_program (program, argv, &ps);
	return (1);
}

/* set the default destination depending on the type of output required */

static void
set_default_dest (ps)
struct	print_stat	*ps;
{
	int	i;
	char	*pgdest;
	FILE	*fp;
	char	buf[80];
	char	*targv[3];

	if (islower (ps->fpt[0]))
		ps->fpt[0] = toupper (ps->fpt[0]);
	/* zero out the destination */
	for (i = 0; i < sizeof (ps->dest); i++)
		ps->dest[i] = '\0';
	switch (ps->fpt[0]) {
	case	TERMCHAR:
		if ((pgdest = getenv ("PAGER")) != (char *) 0)
			strcpy (ps->dest, pgdest);
		else	strcpy (ps->dest, DEFAULT_PAGER);
		break;
	case	PRINTCHAR:
		targv[0] = strrchr (LPSTAT_COMMAND, '/') + 1;
		targv[1] = "-d";
		targv[2] = (char *) 0;
		fp = popen_all_output (LPSTAT_COMMAND, targv);
		if (fp != (FILE *) 0)
			if (fgets (buf, sizeof (buf), fp) != (char *) 0) {
				buf[strlen(buf)-1] = '\0'; /* remove \n */
				strcpy (ps->dest, strrchr (buf, ' ') + 1);
				pclose_all_output (fp);
				break;
			} else
				pclose_all_output (fp);
		/* no default available */
		ps->dest[0] = '\0';
		break;
	case	FILECHAR:
		/* no defaults for files */
		ps->dest[0] = '\0';
	}
}

/* check the values of a print_stat structure for validity */

static int
check_printstat (ps)
struct	print_stat	*ps;
{
	char	buf[80];
	struct	stat	sb;
	FILE	*fp;
	char	*cp;
	char	*lpdest;
	int	found;
	char	*targv[3];

	if (islower (ps->fpt[0]))
		ps->fpt[0] = toupper (ps->fpt[0]);

	switch (ps->fpt[0])  {
	case	FILECHAR:
		/* if file doesn't exist, check that parent dir is
		 * writeable.
		 */
		if (ps->dest[0] == '\0') {
			pop_msg ("You must specify a file for file output.",
			"Please enter one now.");
			return (1);
		}
		if (stat (ps->dest, &sb) < 0)  {
			cp = strrchr (ps->dest, '/');
			if (cp == (char *) 0) {
				pop_msg (
		"File requested is not writeable. Please name a file",
		"for which you have write permission.");
				return (1);
			}
			else {
				*cp = '\0';
				if (stat (ps->dest, &sb) <  0) {
					pop_msg (
		"File must exist in parent directory which is writeable.",
		"Please choose a file for which you have permission.");
					*cp = '/';
					return (1);
				} else if ((sb.st_mode & S_IFMT) != S_IFDIR) {
					pop_msg (
		"File must have a parent which is a directory.",
		"Please choose a file for which you have permission.");
					*cp = '/';
					return (1);
				} else if (eaccess (ps->dest, 7) < 0) {
					pop_msg (
		"You must have write permission in the file's directory.",
		"Please choose a file for which you have permission.");
					*cp = '/';
					return (1);
				}
				*cp = '/';
			}
		} else
		if (warn ("The file that you specified exists.  To overwrite",
		"this file, enter (Y)es now at the prompt.") != YES)
			return (1);
		else
		/* file exists; check for writeable file */
		if (eaccess (ps->dest, 6) < 0) {
			pop_msg (
		"You must have write permission for the file.",
		"Please choose a file for which you have permission.");
			return (1);
		}
		break;
	case	TERMCHAR:
		/* if no pager specified, check his environment variable */
		if (ps->dest[0] == '\0') {
			lpdest = getenv ("PAGER");
			if (lpdest == (char *) 0)
				strcpy (ps->dest, DEFAULT_PAGER);
			else	strcpy (ps->dest, lpdest);
			return (0);
		}
		/* make sure that pager specified is executable */
		if (cp = strrchr (ps->dest, ' '))
			*cp = '\0';
		if (!executable (ps->dest)) {
			sprintf (buf,
		"Pager specified \'%s\' must be executable.  Please use a",
			ps->dest);
			pop_msg (buf,
		"program that you can run or use the default (blank field).");
			if (cp)
				*cp = ' ';
			return (1);
		}
		if (cp)
			*cp = ' ';
		break;
	case	PRINTCHAR:
		/* check that dest is a valid printer or class */
		targv[0] = strrchr (LPSTAT_COMMAND, '/') + 1;
		targv[1] = "-c";
		targv[2] = (char *) 0;
		fp = popen_all_output (LPSTAT_COMMAND, targv);
		if (fp != (FILE *) 0) {
			found = 0;
			while (fgets (buf, sizeof(buf), fp) != (char *) 0)
				if (!found && matchclass (buf, ps->dest))
					found = 1;
			pclose_all_output (fp);
			if (found)
				return (0);
		}
		targv[0] = strrchr (LPSTAT_COMMAND, '/') + 1;
		targv[1] = "-v";
		targv[2] = (char *) 0;
		fp = popen_all_output (LPSTAT_COMMAND, targv);
		if (fp != (FILE *) 0) {
			found = 0;
			while (fgets (buf, sizeof(buf), fp) != (char *) 0)
				if (!found && matchdev (buf, ps->dest))
					found = 1;
			pclose_all_output (fp);
			if (found)
				return (0);
		}
		sprintf (buf, 
		"use \'%s\' or \'%s\'.",
		  PRINTCLASSCMD, PRINTDEVCMD);
		pop_msg (
   "Print destination is not a known printer or class.  To see valid ones,",
		  buf);
		return (1);
	default:
		sprintf (buf,
		"Destination must be one of \'%c\', \'%c\', or \'%c\'.",  
		FILECHAR, PRINTCHAR, TERMCHAR);
		pop_msg (buf,
		"Lower case is OK.  Please choose a valid destination.");
		return (1);
	}
	return (0);
}

/* match a class name line as output from "lpstat -c" */
/* returns 1 if class matches */

static int
matchclass (line, printer)
char	*line;
char	*printer;
{
	char	*cp;

	if (line[0] == '\t')
		return (0);
	cp = strrchr (line, ':');
	if (cp == (char *) 0)
		return (0);
	*cp = '\0';
	cp = strrchr (line, ' ') + 1;
	if (strcmp (printer, cp) == 0)
		return (1);
	return (0);
}

/* match a printer name line as output from "lpstat -v" */
/* returns 1 if printer matches */

static int
matchdev (line, printer)
char	*line;
char	*printer;
{
	char	*cp;

	if (line[0] == '\t')
		return (0);
	cp = strrchr (line, ':');
	if (cp == (char *) 0)
		return (0);
	*cp = '\0';
	cp = strrchr (line, ' ') + 1;
	if (strcmp (printer, cp) == 0)
		return (1);
	return (0);
}

/* run a program according to the output destination specified by the user. */

static int
ps_run_program (program, argv, ps)
char	*program;
char	*argv[];
struct	print_stat	*ps;
{
	int	pid;
	int	wait_stat;
	char	buf[80];

	/* anytime hitting the screen, need to allow hangup to stop us.
	 */
	(void) signal (SIGHUP, SIG_DFL);
	if (hup_caught == 0) {
		clear();
		move (0, 0);
		refresh();
		reset_shell_mode();
	}
	signal (SIGHUP, hup_catch);
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
	switch (pid = fork()) {
	default:	/* see parent code below */
		break;
	case	0:	/* child */
		/* allow all the usual signals to kill the children
		 * forked underneath the parent (screen) process.
		 */
		(void) signal (SIGHUP, SIG_DFL);
		(void) signal (SIGINT, SIG_DFL);
		(void) signal (SIGQUIT, SIG_DFL);
		ps_child (program, argv, ps);
		/* never returns */
	case 	-1:
		reset_prog_mode();
		pop_msg ("Cannot fork process to run program.",
		"Please check conditions and re-run.");
		return (1);
	}
	/* parent waits for child and restores shell mode */
#ifdef DEBUG
fprintf (logfp, "Parent forked child %d\n", pid);
#endif
	pid = wait (&wait_stat);
#ifdef DEBUG
fprintf (logfp, "Parent wait returned pid %d status 0x%x\n", pid, wait_stat);
#endif
	(void) signal (SIGHUP, SIG_DFL);
	if (hup_caught == 0) {
		printf ("\nPress <RETURN> to continue: ");
		fflush (stdout);
		gets (buf);
		reset_prog_mode();
		clearok(stdscr, TRUE);
	}
	(void) signal (SIGHUP, hup_catch);
	return (1);
}

static int
ps_child (program, argv, ps)
char	*program;
char	*argv[];
struct	print_stat	*ps;
{
	char	command[80];
	char	tempfile[80];
	char	*cp;
	int	i;
	FILE	*fp;
	int	count;
	char	**pgargv;
	int	pipefd[2];
	int	pid, wait_stat;
	char	**envp;
	extern	char	**environ;
	int	nenviron;
	char	*targv[6];
	char	*shellenv = (char *) 0;

	switch (ps->fpt[0]) {
	case	FILECHAR:
		if (freopen (ps->dest, "w", stdout) == (FILE *) 0)
			exit (1);
		fclose (stderr);
		dup (1);
		if (execvp (program, argv) < 0)
			exit (1);
	case	TERMCHAR:
		if (pipe (pipefd) < 0) {
			printf ("Cannot create pipe.\n");
			exit (1);
		}
		(void) signal (SIGINT, SIG_IGN);
		(void) signal (SIGQUIT, SIG_IGN);
		switch (pid = fork()) {
		case	0:  /* child needs to run the program */
			(void) signal (SIGINT, SIG_DFL);
			(void) signal (SIGQUIT, SIG_DFL);
			close (pipefd[0]);
			close (1);
			close (2);
			dup (pipefd[1]);
			dup (1);
			close (pipefd[1]);
			/* add PRINTROWS and PRINTCOLS to environment */
			nenviron = 0;
			for (envp = environ; *envp != (char *) 0; envp++) {
				if (strncmp (*envp, "SHELL=",
				  sizeof("SHELL=")-1) == 0)
					shellenv = *envp;
				nenviron++;
			}
			if (shellenv == (char *) 0)
				nenviron++;
			envp = (char **) calloc (nenviron + 3, sizeof (char *));
			for (i = 0; i < nenviron; i++)
				envp[i] = environ[i];
			if (shellenv == (char *) 0)
				envp[i++] = "SHELL=/sbin/sh";
			envp[i] = malloc (sizeof ("PRINTROWS") + 5);
			sprintf(envp[i], "PRINTROWS=%d", ps->pagelength);
			i++;
			envp[i] = malloc (sizeof ("PRINTCOLS") + 5);
			sprintf(envp[i], "PRINTCOLS=%d", ps->pagewidth);
			envp[i+1] = (char *) 0;
			if (execve (program, argv, envp) < 0) {
				printf ("Error: could not execute \'%s\'.\n",
				  program);
				exit (1);
			}
		case	-1:
			printf (
		"Error: could not fork.  Check conditions and try again.\n");
			exit (1);
		default:	/* parent should exec the pager */
			close (pipefd[1]);
			close (0);
			dup (pipefd[0]);
			close (pipefd[0]);
			for (count = 0, cp = ps->dest; *cp; cp++)
				if (*cp == ' ')
					count++;
			pgargv = (char **) calloc (count + 2, sizeof (char *));
			i = 0;
			for (cp = ps->dest; *cp; cp++)
				if (*cp == ' ')  {
					*cp = '\0';
					i++;
					pgargv[i] = cp + 1;
				}
			pgargv[count + 1] = NULL;
			pgargv[0] = strrchr (ps->dest, '/');
			if (pgargv[0])
				pgargv[0]++;
			else	pgargv[0] = ps->dest;
			if (execvp (ps->dest, pgargv) < 0)  {
				perror ("Cannot exec pager");
				kill (pid, 9);
				wait (&count);
				exit (1);
			}
		}
	case	PRINTCHAR:
		cp = getenv ("TMPDIR");
		if (cp != (char *) 0) {
			strcpy (tempfile, cp);
			strcat (tempfile, strrchr (TEMPFILETEMPLATE, '/'));
		} else
			strcpy (tempfile, TEMPFILETEMPLATE);
		(void) mktemp (tempfile);
		pid = fork();
		if (pid == 0) {
			freopen (tempfile, "w", stdout);
			/* add PRINTROWS and PRINTCOLS to environment */
			nenviron = 0;
			for (envp = environ; *envp != (char *) 0; envp++)
				nenviron++;
			envp = (char **) calloc (nenviron + 3, sizeof (char *));
			for (i = 0; i < nenviron; i++)
				envp[i] = environ[i];
			envp[i] = malloc (sizeof ("PRINTROWS") + 5);
			sprintf(envp[i], "PRINTROWS=%d", ps->pagelength);
			i++;
			envp[i] = malloc (sizeof ("PRINTCOLS") + 5);
			sprintf(envp[i], "PRINTCOLS=%d", ps->pagewidth);
			envp[i+1] = (char *) 0;

			if (execve (program, argv, envp) < 0) {
				printf ("Error: could not execute \'%s\'.\n",
				  program);
				exit (1);
			}
		} else if (pid > 0) {
			do { /* need to pick up child's status */
				pid = wait (&wait_stat);
			} while (pid < 0);
		} else {
			printf (
			"Error: could not fork to run print program.\n");
			exit (1);
		}
		if (wait_stat == 0)  {
			targv[0] = strrchr (PRINTFILECMD, '/') + 1;
			targv[1] = "-c";
			sprintf(command, "-d%s", ps->dest);
			targv[2] = command;
			targv[3] = tempfile;
			targv[4] = (char *) 0;
			if ((fp = popen_all_output (PRINTFILECMD, targv)) !=
			  (FILE *) 0) {
				while (fgets (command, sizeof (command), fp) !=
				 (char *) 0)
					fputs (command, stdout);
				pclose_all_output (fp);
			}
		}
		(void) unlink (tempfile);
		exit (0);
	default:
		/* shouldn't happen */
		printf ("Child: got strange character for option.\n");
		exit (0);
	}
}

/* returns true if the program is executable according to the PATH environment
 * variable.
 */

static int
executable (path)
char	*path;
{
	char	*cp;
	char	*pathenv;
	char	filename[80];
	int	i;

	if (strchr (path, '/') != (char *) 0)
		if (eaccess (path, 1) == 0)
			return (1);
		else	return (0);
	cp = getenv ("PATH");
	pathenv = malloc (strlen (cp) + 1);
	if (pathenv == (char *) 0)
		return (0);
	strcpy (pathenv, cp);
	/* current directory cases */
	if (pathenv[0] == ':' || pathenv [strlen(pathenv) - 1] == ':')
		if (eaccess (path, 1) == 0) {
			free (pathenv);
			return (1);
		}
	for (i = 0; i < strlen (pathenv); i++)
		if (pathenv[i] == ':' && pathenv[i + 1] == ':')
			if (eaccess (path, 1) == 0) {
				free (pathenv);
				return (1);
			}
	cp = strtok (pathenv, ":");
	do {
		if (cp[0] == '\0')
			strcpy  (filename, path);
		else	sprintf (filename, "%s/%s", cp, path);
		if (eaccess (filename, 1) == 0) {
			free (pathenv);
			return (1);
		}
	} while (cp = strtok (NULL, ":"));
	free (pathenv);
	return (0);
}

/* Subroutine that asks the user whether he wants to continue an action,
 * then returns 1 or 0 to tell the result (yes or no, respectively).
 */

#define AnswerMsg "To continue the action, enter 'Y':"
static char	yesno;

static
struct	scrn_desc	warn_desc[] = {
{  2, 0, FLD_PROMPT, 0, FLD_OUTPUT, NO,
	"Caution Message Screen" },
{  4, 0, FLD_PROMPT, 0, FLD_OUTPUT, NO, NULL },	/* filled with string1 */
{  5, 0, FLD_PROMPT, 0, FLD_OUTPUT, NO, NULL },	/* filled with string2 */
{  7, 0, FLD_PROMPT, 0, FLD_OUTPUT, NO, AnswerMsg },
{  7, 0, FLD_YN,     1, FLD_INPUT,  YES, NULL }
};

static
struct	scrn_parms	warn_scrn = {
	SCR_FILLIN,	/* scrn_type  */
	6,		/* top row */
	0,		/* left column */
	11,		/* number of rows */
	0,		/* number of columns */
	NUMDESCS(warn_desc),/* number of scrn_descs */
	warn_desc	/* table of descs */
};

static
struct	scrn_struct	warn_struct[] = {
{ &yesno, 0, 0, 4 }
};

warn (string1, string2)
char	*string1;
char	*string2;
{
	int	widest, thiswide;
	WINDOW	*window;
	extern	int	LINES, COLS;
	int	i;
	struct	scrn_ret	scrn_ret;
	int	ret;

	warn_desc[1].prompt = string1;
	warn_desc[2].prompt = string2;
	/* find widest string to see how wide to make the window. */
	widest = strlen (string1);
	if ((thiswide = strlen (string2)) > widest)
		widest = thiswide;
	if ((thiswide = strlen (AnswerMsg) + 2) > widest)
		widest = thiswide;
	/* need a space and the vertical box character on both sides of win */
	warn_scrn.leftcol = (COLS - widest - 4 + 1) / 2;
	warn_scrn.nbrcols = widest + 4;
	for (i = 0; i < 4; i++)
		warn_desc[i].col = (warn_scrn.nbrcols -
		  (strlen (warn_desc[i].prompt) + warn_desc[i].len + 1)) / 2;
	warn_desc[4].col = warn_desc[3].col + strlen (warn_desc[3].prompt) + 1;
	warn_struct[0].changed = warn_struct[0].filled = 0;
	/* This is a lower level routine, so should return back to higher
	 * level only if interrupt is hit in a warn screen.
	 */
	LowerLevel = 1;
	window = putscreen (&warn_scrn, warn_struct, POP);
	scrn_ret = getscreen (window, &warn_scrn, warn_struct, 0);
	/* reset signal handling so that signals aren't allowed.
	 * at this point, user has committed action.
	 */
	LowerLevel = 0;
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);
	if (yesno == 1)
		return (YES);
	else	return (NO);
}

#ifdef DEBUG
print_ask_print_scrn() {
	printscreen (&fpt_scrn);
}
#endif

print_ask_print_help (prefix)
char *prefix;
{
	printhelp (prefix, &fpt_scrn);
}
