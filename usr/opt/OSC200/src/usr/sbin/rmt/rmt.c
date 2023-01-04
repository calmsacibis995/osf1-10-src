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
static char	*sccsid = "@(#)$RCSfile: rmt.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/08 15:54:09 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE ONLY
 * SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved.
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 *	rmt
 *
 * This command is used primarily as the remote host tape device interface
 * slave by the rdump and rrestore master commands, but it could be used
 * by other master programs as an interface to any file.
 *
 * It receives commands and data from standard input and sends acknowledgements
 * and data to standard output.  If given a debug file name argument, it will
 * write a trace of all transactions to the debug file.  Errors in command
 * execution are reported over standard output as well.
 *
 * This program distinguishes between two classes of error: 1) errors resulting
 * from execution of valid commands from the master, and 2) errors not master-
 * command-related.
 *
 * The first class of error could be from open(), close(), read(), write(), and
 * ioctl() calls on the file being manipulated by the program.  These errors
 * are reported back to the master program over standard output.
 *
 * The second class of error are "internal" errors which should not or could
 * not be reported back to the master.  The prime example is an error in the
 * communications with the master.  These cause the program to write to the
 * debug file (if one is open), and non-gracefully exit.
 */

#include	<stdio.h>
#include	<sgtty.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<errno.h>
#include	<sys/mtio.h>
#include	<varargs.h>
#include 	<locale.h>

#include	"rmt_msg.h"
static nl_catd		catd;
#define	MSGSTR(Num, Str)	catgets(catd, MS_RMT, Num, Str)

#if	SSIZE
#undef	SSIZE
#endif

#define	SSIZE	64

#define	EXIT_OK		0
#define	EXIT_ERROR	1

extern off_t		lseek();
extern char	       *malloc();
extern void		free();
extern void		exit();

extern int		errno;
extern char	       *sys_errlist[];

static char	       *checkbuf();
static void		acknowledge();
static void		getstring();
static void		msg();
static void		tape_error();

static FILE	       *debug_fp = NULL;

void
main(argc, argv)
	int		argc;
	char	       *argv[];
{
	char		device[SSIZE];
	char		flags[SSIZE];
	char		count[SSIZE];
	char		pos[SSIZE];
	char		op[SSIZE];
	char	       *buffer = NULL;
	int		tape_fd = -1;
	int		rval;
	char		c;
	int		num_read;
	int		n, i;
	struct mtget	mtget;
	struct mtop	mtop;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_RMT, NL_CAT_LOCALE);

	/* rmt can have a maximum of one argument, the debug output file name */

	if (argc > 2)
	{
		fprintf(stderr, MSGSTR(USAGE, "Usage: rmt [<debug-output-file>]\n"));
		exit(EXIT_ERROR);

		/* NOTREACHED */
	}

	if (argc == 2)
	{
		if ((debug_fp = fopen(argv[1], "w")) == NULL)
		{
			fprintf(stderr, MSGSTR(CODBG, "cannot open debug output file %s\n"), argv[1]);
			fprintf(stderr, MSGSTR(CODBG2, "main(): fopen(): %s\n"),
				 sys_errlist[errno]);
			exit(EXIT_ERROR);

			/* NOTREACHED */
		}
		(void) setbuf(debug_fp, NULL);
	}

	/* loop forever until EOF on standard input or unrecoverable error */
	/* causes exit() */

	while (1)
	{
		/* read next command character */

		num_read = read(0, &c, 1);
		if (num_read < 0)
		{
			msg(MSGSTR(REOI, "read error on standard input"));
			msg(MSGSTR(REOI2,"main(): read(): %s"), sys_errlist[errno]);
			exit(EXIT_ERROR);

			/* NOTREACHED */
		}
		else if (num_read == 0)
		{
			msg(MSGSTR(REOF, "end-of-file on standard input"));
			exit(EXIT_OK);

			/* NOTREACHED */
		}

		/* switch depending on command */

		switch (c)
		{

		case 'O':

			/* open file with given flags */

			getstring(device);
			getstring(flags);

			msg("O %s %s", device, flags);

			/* if a file was already open, close it first */

			if (tape_fd >= 0)
			{
				msg("C");

				if (close(tape_fd) < 0)
				{
					tape_error();
					break;
				}
			}

			if ((tape_fd = open(device, atoi(flags), 0644)) < 0)
			{
				tape_error();
				break;
			}

			acknowledge(0);
			break;

		case 'C':

			/* close the file */

			getstring(device);	/* discard */

			msg("C %s", device);

			if (close(tape_fd) < 0)
			{
				tape_error();
				break;
			}
			tape_fd = -1;

			acknowledge(0);
			break;

		case 'L':

			/* lseek to a position within the open file */

			getstring(count);
			getstring(pos);

			msg("L %s %s", count, pos);

			rval = (int)lseek(tape_fd, (off_t) atoi(count), atoi(pos));
			if (rval < 0)
			{
				tape_error();
				break;
			}

			acknowledge(rval);
			break;

		case 'W':

			/* get data from standard input and write it */
			/* to the open file */

			/* count is the number of bytes to write */

			getstring(count);

			msg("W %s", count);

			n = atoi(count);

			/* make sure we have a buffer large enough to */
			/* read from standard input */

			buffer = checkbuf(buffer, n);

			/* loop to keep reading additional pieces from */
			/* standard input until the full amount has been */
			/* read */

			for (i = 0; i < n; i += num_read)
			{
				num_read = read(0, &buffer[i], n - i);
				if (num_read < 0)
				{
					msg(MSGSTR(REOI, 
					       "read error on standard input"));
					msg(MSGSTR(REOI2, "main(): read(): %s"),
							sys_errlist[errno]);
					exit(EXIT_ERROR);

					/* NOTREACHED */
				}
				else if (num_read == 0)
				{
					msg(MSGSTR(RPEOF, "premature end-of-file on standard input"));
					exit(EXIT_ERROR);

					/* NOTREACHED */
				}
			}

			/* now write the buffer out to the open file */

			rval = write(tape_fd, buffer, n);
			if (rval < 0)
			{
				tape_error();
				break;
			}

			acknowledge(rval);
			break;

		case 'R':

			/* read data from the open file and write it to */
			/* standard output */

			/* count is the number of bytes to read */

			getstring(count);

			msg("R %s", count);

			n = atoi(count);

			/* make sure we have a buffer large enough to */
			/* read from the file */

			buffer = checkbuf(buffer, n);

			/* read the data from the file */

			rval = read(tape_fd, buffer, n);
			if (rval < 0)
			{
				tape_error();
				break;
			}

			acknowledge(rval);

			/* write the data to standard output */

			if (write(1, buffer, rval) != rval)
			{
				msg(MSGSTR(WEOSO, "write error on standard output"));
				msg(MSGSTR(WEOSO2,"main(): write(): %s"), 
					sys_errlist[errno]);
				exit(EXIT_ERROR);

				/* NOTREACHED */
			}
			break;

		case 'I':
			
			/* do an ioctl() call on the open file to set some */
			/* parameter or do some other operation */

			/* op is the operation to do, count is the argument */
			/* for the operation */

			getstring(op);
			getstring(count);

			msg("I %s %s", op, count);

			mtop.mt_op = atoi(op);
			mtop.mt_count = atoi(count);

			if (ioctl(tape_fd, MTIOCTOP, (char *) &mtop) < 0)
			{
				tape_error();
				break;
			}

			rval = mtop.mt_count;

			acknowledge(rval);
			break;

		case 'S':
			
			/* get the status of the open file by doing an */
			/* ioctl() call, and sending the status buffer */
			/* to standard output */

			msg("S");

			/* get the status of the file */

			if (ioctl(tape_fd, MTIOCGET, (char *)&mtget) < 0)
			{
				tape_error();
				break;
			}

			rval = sizeof(mtget);

			acknowledge(rval);

			/* send the status buffer to standard output */

			if (write(1, (char *) &mtget, rval) != rval)
			{
				msg(MSGSTR(WEOSO, "write error on standard output"));
				msg(MSGSTR(WEOSO2,"main(): write(): %s"), 
					sys_errlist[errno]);
				exit(EXIT_ERROR);

				/* NOTREACHED */
			}
			break;

		default:
			
			/* if something screws up, chances are it will be */
			/* caught here as an unrecognized command character */

			msg(MSGSTR(ILLCOM, "unrecognized command %c"), c);
			exit(EXIT_ERROR);

			/* NOTREACHED */
		}
	}
}

/* acknowledge() simply acknowledges the completion of the previously */
/* requested operation to standard output.  The return code of the */
/* operation will be converted to ascii and sent following an 'A'. */

static void
acknowledge(rval)
	int		rval;
{
	char		resp[BUFSIZ];

	msg("A %d", rval);

	(void) sprintf(resp, "A%d\n", rval);
	if (write(1, resp, strlen(resp)) != strlen(resp))
	{
		msg(MSGSTR(WEOSO, "write error on standard output"));
		msg(MSGSTR(WEOSO3,"acknowledge(): write(): %s"), 
			sys_errlist[errno]);
		exit(EXIT_ERROR);

		/* NOTREACHED */
	}
}

/* tape_error() is called whenever an operation on the device file fails. */
/* The return code and string of the failed operation are converted to */
/* ascii and sent to standard output. */

static void
tape_error()
{
	char		resp[BUFSIZ];

	msg("E %d (%s)", errno, sys_errlist[errno]);

	(void) sprintf(resp, "E%d\n%s\n", errno, sys_errlist[errno]);
	if (write(1, resp, strlen(resp)) != strlen(resp))
	{
		msg(MSGSTR(WEOSO, "write error on standard output"));
		msg(MSGSTR(WEOSO4,"tape_error(): write(): %s"), 
			sys_errlist[errno]);
		exit(EXIT_ERROR);

		/* NOTREACHED */
	}
}

/* getstring() reads the next '\n'-terminated string from standard input, */
/* puts it into the buffer pointed to by bp, and replaces the '\n' with */
/* a proper '\0' string terminator. */

static void
getstring(bp)
	char	       *bp;
{
	int		i;
	int		num_read;

	for (i = 0; i < SSIZE; ++i)
	{
		num_read = read(0, &bp[i], 1);
		if (num_read < 0)
		{
			msg(MSGSTR(REOI, "read error on standard input"));
			msg(MSGSTR(REOI3,"getstring(): read(): %s"), 
				sys_errlist[errno]);
			exit(EXIT_ERROR);

			/* NOTREACHED */
		}
		else if (num_read == 0)
		{
			msg(MSGSTR(RPEOF, "premature end-of-file on standard input"));
			exit(EXIT_ERROR);

			/* NOTREACHED */
		}
		if (bp[i] == '\n')
		{
			break;
		}
	}
	bp[i] = '\0';
}

/* checkbuf() is used to make sure the currently-allocated local buffer */
/* is large enough for the read or write which has been requested.  When */
/* it is not large enough, the old buffer is freed and a new, larger one */
/* allocated.  checkbuf() also changes the socket buffer size to the new */
/* larger size. */

static char	       *
checkbuf(buffer, requested_size)
	char	       *buffer;
	int		requested_size;
{
	static int	old_buffer_size = -1;
	int		size;

	/* if the old buffer is big enough, return it */

	if (buffer != NULL && requested_size <= old_buffer_size)
	{
		return(buffer);
	}

	/* if not, free the old buffer */

	if (buffer != NULL)
	{
		free(buffer);
	}

	/* get a new buffer with the larger size */

	buffer = malloc((unsigned) requested_size);
	if (buffer == NULL)
	{
		msg(MSGSTR(CNAL, "cannot allocate buffer space"));
		exit(EXIT_ERROR);

		/* NOTREACHED */
	}

	old_buffer_size = requested_size;

	/* change the socket buffer size to the new size */

	for (size = requested_size; size > 1024; size -= 1024)
	{
		if (setsockopt(0, SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(size)) >= 0)
		{
			return(buffer);
		}
	}

	msg(MSGSTR(SOCSZ, "cannot set socket receive-buffer size"));
	msg(MSGSTR(SOCSZ2,"checkbuf(): setsockopt(): %s"), 
		sys_errlist[errno]);
	exit(EXIT_ERROR);

	/* NOTREACHED */
}

/* VARARGS1 */

static void
msg(va_alist)
	va_dcl
{
	va_list		ap;
	char	       *fmt;

	if (debug_fp == NULL)
	{
		return;
	}

	(void) fprintf(debug_fp, "rmt: ");

	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vfprintf(debug_fp, fmt, ap);
	va_end(ap);

	(void) fprintf(debug_fp, "\n");
	(void) fflush(debug_fp);
}
