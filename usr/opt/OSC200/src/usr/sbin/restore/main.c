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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/08 15:53:48 $";
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
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

/*
 * Modified to recursively extract all files within a subtree (supressed by the
 * h option) and recreate the heirarchical structure of that subtree and move
 * extracted files to their proper homes (supressed by the m option). Includes
 * the s (skip files) option for use with multiple dumps on a single tape.
 * 8/29/80 	by Mike Litzkow
 *
 * Modified to work on the new file system and to recover from tape read errors.
 * 1/19/82 	by Kirk McKusick
 *
 * Full incremental restore running entirely in user code and interactive tape
 * browser.
 * 1/19/83 	by Kirk McKusick
 */

#include	"restore.h"
#include	<signal.h>

static void	usage();

void
main(argc, argv)
	int		argc;
	char	       *argv[];
{
	ino_t		ino;
	char	       *inputdev = DEFAULT_TAPE;
	char	       *symtbl_filename = SYMBOL_TABLE;
	char		name[MAXPATHLEN];
	chfl		cfsbuf1[BUFSIZ];
	chfl		cfsbuf2[BUFSIZ];
	int		c;

	catd = catopen(MF_RESTORE, NL_CAT_LOCALE);
	setlocale(LC_ALL, "");

	if (argc < 2)
	{
		usage();

		/* NOTREACHED */
	}

	/* allow BSD style without "-" for keys */
	if ( argv[1][0] != '-' ) {
		int length = strlen( argv[1] );
		char *new  = calloc( length+2, sizeof(char) );
		
		new[0] = '-'; new[1] = '\0';
		strcat(new, argv[1]);
		argv[1] = new;
	}
		

	while ((c = getopt(argc, argv, "F:NYZb:cdf:hms:vyRirtx")) != EOF)
	{
		switch (c)
		{
		case '-':
			break;

		case 'N':
			Nflag = TRUE;
			break;

		case 'Y':
			if (overwrite_flag != OVERWRITE_DEFAULT)
			{
				msg(MSGSTR(YZMUX, "Y and Z options are mutually exclusive\n"));
				usage();

				/* NOTREACHED */
			}
			overwrite_flag = OVERWRITE_ALWAYS;
			break;

		case 'Z':
			if (overwrite_flag != OVERWRITE_DEFAULT)
			{
				msg(MSGSTR(YZMUX, "Y and Z options are mutually exclusive\n"));
				usage();

				/* NOTREACHED */
			}
			overwrite_flag = OVERWRITE_NEVER;
			break;

		case 'F':
			if ( command != 'i' ) {
				msg(MSGSTR(MISSINTR, "-i option is missing\n"));
				Exit(1);

				/* NOTREACHED */
			}
			if (argc < 1)
			{
				msg(MSGSTR(MISSCMD, "missing command file\n"));
				Exit(1);

				/* NOTREACHED */
			}
			if ((command_fp=fopen(optarg, "r")) == NULL) {
				msg(MSGSTR(CANTOPEN,"Cannot open %s\n"),optarg);
				Exit(1);
			
				/* NOTREACHED */
			}
			break;

		case 'b':

			/*
			 * change default tape blocksize
			 */

			block_size_flag = TRUE;
			if (argc < 1)
			{
				msg(MSGSTR(MISSBLK, "missing block size\n"));
				Exit(1);

				/* NOTREACHED */
			}
			ntrec = atoi(optarg);
			if (ntrec <= 0)
			{
				msg(MSGSTR(BLKPOS, "Block size must be a positive integer\n"));
				Exit(1);

				/* NOTREACHED */
			}
			break;

		case 'c':
			old_format_flag = TRUE;
			break;

		case 'd':
			debug_flag = TRUE;
			break;

		case 'f':
			if (argc < 1)
			{
				msg(MSGSTR(MISSDEV, "missing device specifier\n"));
				Exit(1);

				/* NOTREACHED */
			}
			inputdev = optarg;
			break;

		case 'h':
			children_flag = FALSE;
			break;

		case 'm':
			by_name_flag = FALSE;
			break;

		case 'v':
			verbose_flag = TRUE;
			break;

		case 's':

			/*
			 * dumpnum (skip to) for multifile dump tapes
			 */

			if (argc < 1)
			{
				msg(MSGSTR(MISSDUMP, "missing dump number\n"));
				Exit(1);

				/* NOTREACHED */
			}
			dumpnum = atoi(optarg);
			if (dumpnum <= 0)
			{
				msg(MSGSTR(DUMPPOS, "Dump number must be a positive integer\n"));
				Exit(1);

				/* NOTREACHED */
			}
			break;

		case 'y':
			auto_retry_flag = TRUE;
			break;

		case 'R':
		case 'i':
		case 'r':
		case 't':
		case 'x':
			if (command != '\0')
			{
				msg(MSGSTR(BADOPTM, "Options %c and %c are mutually exclusive\n"), c, command);
				usage();

				/* NOTREACHED */
			}
			command = c;
			break;

		default:
			msg(MSGSTR(BADKEY, "Bad option character %c\n"), c);
			usage();

			/* NOTREACHED */
		}
	}

	if (command == '\0')
	{
		msg(MSGSTR(MUSTITR, "Must specify one of R, i, r, t, or x options\n"));
		usage();

		/* NOTREACHED */
	}

	setinput(inputdev);

	if (argv[optind] == NULL)
	{
		--optind;
		argv[optind] = ".";
	}

	if (signal(SIGINT, sigintr) == SIG_IGN)
	{
		(void) signal(SIGINT, SIG_IGN);
	}
	if (signal(SIGTERM, sigintr) == SIG_IGN)
	{
		(void) signal(SIGTERM, SIG_IGN);
	}

	switch (command)
	{

	case 'i':

		/*
		 * Interactive mode.
		 */

		setup();
		extractdirs(TRUE);
		initsymtable(NULL);
		runcmdshell();
		Exit(0);

		/* NOTREACHED */

	case 'r':

		/*
		 * Incremental restoration of a file system.
		 */

		setup();
		if (dumptime > 0)
		{
			/*
			 * This is an incremental dump tape.
			 */

			vmsg(MSGSTR(BEGINIC, "Begin incremental restore\n"));
			initsymtable(symtbl_filename);
			extractdirs(TRUE);
			removeoldleaves();
			vmsg(MSGSTR(CALNODE, "Calculate node updates.\n"));
			treescan(".", ROOTINO, nodeupdates);
			findunreflinks();
			removeoldnodes();
		}
		else
		{
			/*
			 * This is a level zero dump tape.
			 */

			vmsg(MSGSTR(BEGINLE0, "Begin level 0 restore\n"));
			initsymtable(NULL);
			extractdirs(TRUE);
			vmsg(MSGSTR(CALEXTL, "Calculate extraction list.\n"));
			treescan(".", ROOTINO, nodeupdates);
		}
		createleaves(symtbl_filename);
		createlinks();
		setdirmodes();
		checkrestore();
		if (debug_flag == TRUE)
		{
			vmsg(MSGSTR(VERIFY, "Verify the directory structure\n"));
			treescan(".", ROOTINO, verifyfile);
		}
		dumpsymtable(symtbl_filename, (long) 1);
		Exit(0);

		/* NOTREACHED */

	case 'R':

		/*
		 * Resume an incremental file system restoration.
		 */

		initsymtable(symtbl_filename);
		skipmaps();
		skipdirs();
		createleaves(symtbl_filename);
		createlinks();
		setdirmodes();
		checkrestore();
		dumpsymtable(symtbl_filename, (long) 1);
		Exit(0);

		/* NOTREACHED */

	case 't':

		/*
		 * List contents of tape.
		 */

		setup();
		extractdirs(FALSE);
		initsymtable(NULL);
		for (; argv[optind] != NULL; ++optind)
		{
			/* convert to cfs first */
			strtocfs(cfsbuf1, argv[optind]);
			canon(cfsbuf2, cfsbuf1);
			/* convert back from cfs */
			cfstostr(name, cfsbuf2);
			ino = dirlookup(name);
			if (ino != (ino_t) 0)
			{
				treescan(name, ino, listfile);
			}
		}
		Exit(0);

		/* NOTREACHED */

	case 'x':

		/*
		 * Batch extraction of tape contents.
		 */

		setup();
		extractdirs(TRUE);
		initsymtable(NULL);
		for (; argv[optind] != NULL; ++optind)
		{
			/* convert to cfs first */
			strtocfs(cfsbuf1, argv[optind]);
			canon(cfsbuf2, cfsbuf1);
			/* convert back from cfs */
			cfstostr(name, cfsbuf2);
			ino = dirlookup(name);
			if (ino != (ino_t) 0)
			{
				if (by_name_flag == TRUE)
				{
					pathcheck(name);
				}
				treescan(name, ino, addfile);
			}
		}
		createfiles();
		createlinks();
		setdirmodes();
		if (debug_flag == TRUE)
		{
			checkrestore();
		}
		Exit(0);

		/* NOTREACHED */
	}
}

static void
usage()
{
	msg(MSGSTR(USAGE0, "Usage:\n%s%s%s%s%s"),
	    MSGSTR(USAGE1, "\trestore -i [-NYZhmvy] [-f device] [-F cmd_infile] [-s #] [-b #]\n"),
	    MSGSTR(USAGE2, "\trestore -t [-NYZhvy] [-f device] [-s #] [-b #] [file file ...]\n"),
	    MSGSTR(USAGE3, "\trestore -x [-NYZhmvy] [-f device] [-s #] [-b #] [file file ...]\n"),
	    MSGSTR(USAGE4, "\trestore -r [-NYZvy] [-f device] [-s #] [-b #]\n"),
	    MSGSTR(USAGE5, "\trestore -R [-NYZvy] [-f device] [-s #] [-b #]\n"));
	Exit(1);

	/* NOTREACHED */
}
