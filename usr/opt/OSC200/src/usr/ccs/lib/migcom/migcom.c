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
static char	*sccsid = "@(#)$RCSfile: migcom.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:22:32 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *  ABSRACT:
 *	Main control program for mig.
 *	Mig parses an interface definitions module for a mach server.
 *	It generates three c modules: subsystem.h, 
 *	subsystemUser.c and subsystemServer.c for the user
 *	and server sides of the ipc-message passing interface.
 *
 *	Switches are;
 *		-[v,Q]  verbose or not quiet:  prints out type
 *			and routine information as mig runs.
 *		-[V,q]  not verbose or quiet : don't print 
 *			information during compilation
 *			(this is the default)
 *		-[r,R]  do or don't use msg_rpc calls instead of 
 *			msg_send, msg_receive pairs. Default is -r
 *		-[s,S]	generate symbol table or not:  generate a
 *			table of rpc-name, number, routine triplets
 *			as an external data structure -- main use is
 *			for protection system's specification of rights
 *			and for protection dispatch code.  Default is -s.
 *		-i	Put each user routine in its own file.  The
 *			file is named <routine-name>.c.
 *		-user <name>
 *			Name the user-side file <name>
 *		-server <name>
 *			Name the server-side file <name>
 *		-header <name>
 *			Name the user-side header file <name>
 *
 *  DESIGN:
 *	Mig uses a lexxer module created by lex from lexxer.l and
 *	a parser module created by yacc from parser.y to parse an
 *	interface definitions module for a mach server.
 *	The parser module calls routines in statement.c
 *	and routines.c to build a list of statement structures.
 *	The most interesting statements are the routine definitions
 *	which contain information about the name, type, characteristics
 *	of the routine, an argument list containing information for
 *	each argument type, and a list of special arguments. The
 *	argument type structures are build by routines in type.c
 *	Once parsing is completed, the three code generation modules:
 *	header.c user.c and server.c are called sequentially. These
 *	do some code generation directly and also call the routines
 *	in utils.c for common (parameterized) code generation.
 *	
 */

#include "error.h"
#include "lexxer.h"
#include "global.h"
#include "write.h"

extern int yyparse();

boolean_t	GenIndividualUser = FALSE;

static void
parseArgs(argc, argv)
    int argc;
    char *argv[];
{
    while (--argc > 0)
	if ((++argv)[0][0] == '-') 
	{
	    switch (argv[0][1]) 
	    {
	      case 'q':
		BeQuiet = TRUE;
		break;
	      case 'Q':
		BeQuiet = FALSE;
		break;
	      case 'v':
		BeVerbose = TRUE;
		break;
	      case 'V':
		BeVerbose = FALSE;
		break;
	      case 'r':
		UseMsgRPC = TRUE;
		break;
	      case 'R':
		UseMsgRPC = FALSE;
		break;
	      case 's':
		if (!strcmp(argv[0], "-server"))
		{
		    --argc; ++argv;
		    if (argc == 0)
			fatal("missing name for -server option");
		    ServerFileName = strmake(argv[0]);
		}
		else
		    GenSymTab = TRUE;
		break;
	      case 'S':
	        GenSymTab = FALSE;
		break;
	      case 'i':
		GenIndividualUser = TRUE;
		break;
	      case 'u':
		if (!strcmp(argv[0], "-user"))
		{
		    --argc; ++argv;
		    if (argc == 0)
			fatal("missing name for -user option");
		    UserFileName = strmake(argv[0]);
		}
		else
		    fatal("unknown flag: '%s'", argv[0]);
		break;
	      case 'h':
		if (!strcmp(argv[0], "-header"))
		{
		    --argc; ++argv;
		    if (argc == 0)
			fatal("missing name for -header option");
		    HeaderFileName = strmake(argv[0]);
		}
		else
		    fatal("unknown flag: '%s'", argv[0]);
		break;
	      default:
		fatal("unknown flag: '%s'", argv[0]);
		/*NOTREACHED*/
	    }
	}
	else
	    fatal("bad argument: '%s'", *argv);
}

void
main(argc, argv)
    int argc;
    char *argv[];
{
    FILE *h, *server, *user;

    set_program_name("mig");
    parseArgs(argc, argv);
    init_global();
    init_type();

    LookNormal();
    (void) yyparse();

    if (errors > 0)
	exit(1);

    more_global();

    if ((h = fopen(HeaderFileName, "w")) == NULL)
	fatal("fopen(%s): %s", HeaderFileName, unix_error_string(errno));
    if (!GenIndividualUser)
    {
	if ((user = fopen(UserFileName, "w")) == NULL)
	    fatal("fopen(%s): %s", UserFileName, unix_error_string(errno));
    }
    if ((server = fopen(ServerFileName, "w")) == NULL)
	fatal("fopen(%s): %s", ServerFileName, unix_error_string(errno));

    if (BeVerbose)
    {
	printf("Writing %s ... ", HeaderFileName);
	fflush(stdout);
    }
    WriteHeader(h, stats);
    fclose(h);
    if (GenIndividualUser)
    {
	if (BeVerbose)
	{
	    printf("done.\nWriting individual user files ... ");
	    fflush(stdout);
	}
	WriteUserIndividual(stats);
    }
    else
    {
	if (BeVerbose)
	{
	    printf("done.\nWriting %s ... ", UserFileName);
	    fflush(stdout);
	}
	WriteUser(user, stats);
	fclose(user);
    }
    if (BeVerbose)
    {
	printf("done.\nWriting %s ... ", ServerFileName);
	fflush(stdout);
    }
    WriteServer(server, stats);
    fclose(server);
    if (BeVerbose)
	printf("done.\n");

    exit(0);
}
