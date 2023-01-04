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
static char rcsid[] = "@(#)$RCSfile: test.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/10/11 19:19:38 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * "com/cmd/sh,3.1,9021 9/13/89 21:04:44";
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <locale.h>

# include  "test_msg.h"
# define   MSGSTR(errno, str)	catgets(catd, MS_TEST, errno, str)

/*
 * string comparison
 */
#define	eq(s1, s2)		(strcmp(s1, s2) == 0)

/*
 * check file access
 */
#define	tio(file, perm)		(access(file, perm) == 0)

/*
 * check file mode
 */
#define	filmod(file, mode) \
	(stat(file, &statb) == 0 && (statb.st_mode & mode) == mode)

/*
 * check file type
 */
#define	filtyp(file, type) \
	(stat(file, &statb) == 0 && (statb.st_mode & S_IFMT) == type)

/*
 * check file size > 0
 */
#define	fsizep(file)	(stat(file, &statb) == 0 && statb.st_size > 0)

/*
 * macro to 'unget' an arg (push it back so getarg will return
 * the same arg next time...)
 */
#define	ungetarg()	(ap--)

/*
 * is 'arg' a valid expression separator? (see exp())
 */
#define	separator(arg)	(eq(arg, "-a") || eq(arg, "-o") || eq(arg, ")"))

/*
 * flags to getarg() to indicate whether next argument required or not
 */
#define	REQUIRED	(1)
#define	NOT_REQUIRED	(0)

/*
 * local functions
 */
void failed();
char *getarg();
int exp(), e1(), e2(), e3();

/*
 * globals
 */
int	ap = 1;			/* index to current argument	*/
int	ac;			/* global for 'argc'		*/
char	**av;			/* global for 'argv'		*/
struct stat statb;		/* for filmod, filtype & fsizep	*/
nl_catd catd;

/*
 * NAME:	test
 *
 * FUNCTION:	test
 *
 * SYNOPSIS:	test expression
 *		[ expression ]
 *
 * NOTES:	Test evaluates 'expression' and returns a true value (0)
 *		if 'expression' evaluates true.  In the second form of
 *		the command: '[', the ']' is required and the brackets
 *		must be surrounded by blanks.
 *
 * RETURN VALUE DESCRIPTION:	255 if a syntax error is encountered within
 *		'expression', 0 if 'expression' evaluates true, else 1
 */

int
main(argc, argv)
int argc;
char *argv[];
{
	int status;

	(void) setlocale (LC_ALL, "");

	/*
	 * initialize globals...
	 */
	ac = argc;
	av = argv;

	catd = catopen(MF_TEST, NL_CAT_LOCALE);
	/*
	 * check for matching brackets if called as '[' ...
	 */
	if (eq(av[0], "[") && !eq(av[--ac], "]"))
		failed(MSGSTR(MISSING_BRACKET, "] missing"));

	if (ac <= 1)			/* fail on no expression	*/
		return(1);

	status = exp() ? 0 : 1;		/* evaluate expression		*/

	if (getarg(NOT_REQUIRED) != 0)	/* check for extra args		*/
		failed(MSGSTR(TOO_MANY_ARGS, "too many arguments"));

	catclose(catd);
	return (status);
}

/*
 * NAME:	exp
 *
 * FUNCTION:	exp - top level of expression evaluation
 *
 * NOTES:	Exp is the top level of evaluation.  It calls e1 to
 *		do the actual evaluation, then handles the -o (binary
 *		or) operator, which has the lowest precedence of the
 *		combining operators.
 *
 * RETURN VALUE DESCRIPTION:	1 if the expression evaluated is true,
 *		else 0
 */

int
exp()
{
	register int value;
	register char *arg2;

	/*
	 * evaluate current function
	 */
	value = e1();

	/*
	 * see if there are any args left
	 */
	if ((arg2 = getarg(NOT_REQUIRED)) != 0)
	{
		/*
		 * if -o, get the next expression and or it's value in
		 * with this one...
		 */
		if (eq(arg2, "-o"))
			return(value | exp());

		/*
		 * if not -o, then it should be a ')' (since e3 could
		 * have called exp() because of a '(')
		 */
		if (!eq(arg2, ")"))
			failed(MSGSTR(SYNTAX, "syntax error"));

		/*
		 * push the ) back on
		 */
		ungetarg();
	}

	return(value);
}

/*
 * NAME:	e1
 *
 * FUNCTION:	e1 - second level of expression evaluation
 *
 * NOTES:	E1 is the second level of evaluation.  It calls
 *		e2() and handles the -a (binary and) operator, which
 *		has higher priority than -o but lower than !.
 *
 * RETURN VALUE DESCRIPTION:	1 if the expression evaluated is true,
 *		else 0
 */

int
e1()
{
	register int value;
	register char *arg2;

	/*
	 * evaluate current function
	 */
	value = e2();

	/*
	 * check for -a
	 */
	if ((arg2 = getarg(NOT_REQUIRED)) != 0)
	{
		/*
		 * process -a
		 */
		if (eq(arg2, "-a"))
			return(value & e1());

		/*
		 * not -a.  stick the arg back
		 */
		ungetarg();
	}

	return(value);
}

/*
 * NAME:	e2
 *
 * FUNCTION:	e2 - third level of expression evaluation
 *
 * NOTES:	E2 is the third level of evaluation.  It processes
 *		the ! (negation) operator.
 *
 * RETURN VALUE DESCRIPTION:	1 if the expression evaluated is true,
 *		else 0
 */

int
e2()
{
	/*
	 * negate value if ! found
	 */
	if (eq(getarg(REQUIRED), "!"))
		return(!e3());

	ungetarg();

	return(e3());
}

/*
 * NAME:	e3
 *
 * FUNCTION:	e3 - last level of evaluation
 *
 * NOTES:	E3 does the actual evaluation of a function.  It also
 *		handles expressions within parentheses.
 *
 * RETURN VALUE DESCRIPTION:	1 if the expression evaluated is true,
 *		else 0
 */

int
e3()
{
	int value;
	long int1, int2;
	register char *arg1, *arg2;

	arg1 = getarg(REQUIRED);		/* get first token */

	if (eq(arg1, "("))
	{
		/*
		 * found paren.  call exp() to evaluate the
		 * nested expression.
		 */
		value = exp();

		/*
		 * eat right paren
		 */
		if ((arg1 = getarg(NOT_REQUIRED)) == 0 || !eq(arg1, ")"))
			failed(MSGSTR(PAREN_EXPECTED, ") expected"));

		return(value);
	}

	arg2 = getarg(NOT_REQUIRED);	/* get next token ... */
	ungetarg();		/* put it back in case we need it again */

	/*
	 * if no more tokens exist, or if the next isn't "=" or "!=",
	 * then try to evaluate the operator (note that if no more
	 * tokens exist, the getarg(REQUIRED)'s will error and exit)
	 */
	if ((arg2 == 0) || (!eq(arg2, "=") && !eq(arg2, "!=")))
	{
		if (eq(arg1, "-r"))	/* check file for read perm */
			return(tio(getarg(REQUIRED), R_OK));
		if (eq(arg1, "-w"))	/* check file for write perm */
			return(tio(getarg(REQUIRED), W_OK));
		if (eq(arg1, "-x"))	/* check file for execute perm */
			return(tio(getarg(REQUIRED), X_OK));
		if (eq(arg1, "-d"))	/* directory? */
			return(filtyp(getarg(REQUIRED), S_IFDIR));
		if (eq(arg1, "-c"))	/* char special device? */
			return(filtyp(getarg(REQUIRED), S_IFCHR));
		if (eq(arg1, "-b"))	/* block special device? */
			return(filtyp(getarg(REQUIRED), S_IFBLK));
		if (eq(arg1, "-h"))	/* soft link? */ {
		    /*
		     * While filtyp() macro uses stat(), must have lstat here
		     */
		    	if( lstat(getarg(REQUIRED), &statb) ) return (0);

			return ( (statb.st_mode & S_IFMT) == S_IFLNK);
/*		  	return(filtyp(getarg(REQUIRED), S_IFLNK)); */
		    }
		if (eq(arg1, "-e"))     /* file exists? */
			return(lstat(getarg(REQUIRED), &statb) == 0 ? 1 : 0);
		if (eq(arg1, "-f"))	/* regular file? */
			return(filtyp(getarg(REQUIRED), S_IFREG));
		if (eq(arg1, "-u"))	/* setuid on? */
			return(filmod(getarg(REQUIRED), S_ISUID));
		if (eq(arg1, "-g"))	/* setgid on? */
			return(filmod(getarg(REQUIRED), S_ISGID));
		if (eq(arg1, "-k"))	/* sticky bit on? */
			return(filmod(getarg(REQUIRED), S_ISVTX));
		if (eq(arg1, "-p"))	/* named pipe? (FIFO) */
			return(filtyp(getarg(REQUIRED),S_IFIFO));
   		if (eq(arg1, "-s"))	/* size > 0? */
			return(fsizep(getarg(REQUIRED)));
		if (eq(arg1, "-t"))	/* isatty? */
		{
			/*
			 * get file descriptor to check
			 */
			if ((arg1 = getarg(NOT_REQUIRED)) == 0 ||
			    separator(arg1))
			{
				/*
				 * no file descriptor, check 1...
				 * stick the arg back if it's a separator
				 */
				if (arg1 != 0)
					ungetarg();
				return(isatty(1));
			}

			return(isatty(atoi(arg1)));
		}
		if (eq(arg1, "-n"))	/* check string length != 0 */
			return(!eq(getarg(REQUIRED), ""));
		if (eq(arg1, "-z"))	/* check string length == 0 */
			return(eq(getarg(REQUIRED), ""));
	}

	/*
	 * if no more tokens in this expression, check the string length != 0
	 * (like -n)
	 */
	if ((arg2 = getarg(NOT_REQUIRED)) == 0 || separator(arg2))
	{
		/*
		 * stick the arg back if it's a separator
		 */
		if (arg2 != 0)
			ungetarg();
		return(!eq(arg1, ""));
	}

	if (eq(arg2, "="))	/* string comparison */
		return(eq(getarg(REQUIRED), arg1));

	if (eq(arg2, "!="))	/* ! string comparison */
		return(!eq(getarg(REQUIRED), arg1));

	int1 = atol(arg1);
	int2 = atol(getarg(REQUIRED));

	/*
	 * do algebraic comparisons ...
	 */
	if (eq(arg2, "-eq"))
		return(int1 == int2);
	if (eq(arg2, "-ne"))
		return(int1 != int2);
	if (eq(arg2, "-gt"))
		return(int1 > int2);
	if (eq(arg2, "-lt"))
		return(int1 < int2);
	if (eq(arg2, "-ge"))
		return(int1 >= int2);
	if (eq(arg2, "-le"))
		return(int1 <= int2);

	/*
	 * there's a token in place where there should be an operator,
	 * but we don't recognize it ...
	 */
	failed(MSGSTR(UNKNOWN_OP, "unknown operator"));
	/* NOTREACHED */
}

/*
 * NAME:	getarg
 *
 * FUNCTION:	getarg - return next argument
 *
 * NOTES:	Getarg returns the next argument to look at.  If 'required'
 *		is non-zero, getarg will print an error message and exit
 *		if no more arguments exist.
 *
 * DATA STRUCTURES:	ap is incremented
 *
 * RETURN VALUE DESCRIPTION:	0 if there are no more args to look
 *		at, else a pointer to the next arg
 */

char *
getarg(required)
int required;
{
	/*
	 * return the next argument if one exists
	 */
	if (ap < ac)
		return(av[ap++]);

	/*
	 * print an error & exit if this argument was required
	 */
	if (required)
		failed(MSGSTR(ARG_EXPECTED, "argument expected"));

	/*
	 * increment ap anyway (in case of an ungetarg()) and
	 * return 0 since there are not arguments left
	 */
	ap++;

	return(0);
}

/*
 * NAME:	failed
 *
 * FUNCTION:	failed - print error message and exit
 *
 * NOTES:	Failed prints an error message and exits with
 *		the value of 255.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

#define	WRITE(msg)	(void) write(2, msg, (unsigned) strlen(msg))

void
failed(msg)
char *msg;
{
	char *test = MSGSTR(TEST, "test: ");

	WRITE(test);
	WRITE(msg);
	WRITE("\n");

	exit(255);
}
