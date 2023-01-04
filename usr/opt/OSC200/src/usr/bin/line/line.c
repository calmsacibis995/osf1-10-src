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
static char rcsid[] = "@(#)$RCSfile: line.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 21:48:45 $";
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
 * line.c	1.3  com/cmd/sh,3.1,9013 11/27/89 21:06:55
 */

#include <locale.h>
#include <unistd.h>

#define	LSIZE	512		/* line buffer size		*/

char nl = '\n';			/* newline character		*/
int EOF;			/* End Of File flag		*/
char readc();			/* character read function	*/

/*
 * NAME:	line
 *
 * SYNTAX:	line
 *
 * FUNCTION:	line - read one line from standard input
 *
 * NOTES:	This program reads a single line from the standard input
 *		and writes it on the standard output.  It is probably most
 *		useful in conjunction with the Bourne shell.
 *
 * RETURN VALUE DESCRIPTION:	1 on end-of-file, else 0
 */

int
main()
{
	register char c;		/* last character read	*/
	char line[LSIZE];		/* line buffer		*/
	register char *linep, *linend;	/* pointers within line	*/

	(void) setlocale (LC_ALL, "");

	EOF = 0;			/* initialize eof flag	*/
	linep = line;			/* init current pointer	*/
	linend = line + LSIZE;		/* point to end of line	*/

	/*
	 * loop reading 'till we hit eof or eol ...
	 */
	while ((c = readc()) != nl)
	{
		/*
		 * filled line up?
		 */
		if (linep == linend)
		{
			/*
			 * yep, write it & start over...
			 */
			(void) write (1, line, (unsigned)LSIZE);
			linep = line;
		}
		/*
		 * save the character we read
		 */
		*linep++ = c;
	}

	/*
	 * write last buffer (if anything there)
	 */
	if (linep > line)
		write (1, line, (unsigned)(linep-line));

	/*
	 * write newline
	 */
	write(1, &nl, (unsigned)1);

	/*
	 * exit 1 on eof, else 0
	 */
	exit(EOF);

	/* NOTREACHED */
}


/*
 * NAME:	readc
 *
 * FUNCTION:	readc - read a character and return it
 *
 * NOTES:	Readc reads a character from standard input (fd = 0)
 *		and returns it.  On end-of-file, a newline is returned
 *		and the end-of-file flag is turned on.
 *
 * DATA STRUCTURES:	EOF is set on end of file
 *
 * RETURN VALUE DESCRIPTION:	the character read is returned.  on
 *		end of file 'nl' is returned (newline character)
 */

char
readc()
{
	char c;

	if (read (0, &c, (unsigned)1) != 1) {
		EOF = 1;
		c = nl;
	}

	return (c);
}
