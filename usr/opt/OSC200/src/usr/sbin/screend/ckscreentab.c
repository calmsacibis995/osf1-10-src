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
static char *rcsid = "@(#)$RCSfile: ckscreentab.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 12:54:03 $";
#endif

/*
 * ckscreentab.c
 * Parses screen table to check for errors
 *
 * Usage:
 *	ckscreentab filename
 *
 * 21 December 1988	Jeffrey Mogul/DECWRL
 *	Created.
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

#include <stdio.h>

char *sourcefile;

int debug = 0;
int semantic_errors = 0;

main(argc, argv)
int argc;
char **argv;
{
	if (argc != 2) {
	    fprintf(stderr, "Usage: ckscreentab filename\n");
	    exit(1);
	}

	sourcefile = argv[1];
	
	if (freopen(sourcefile, "r", stdin) == NULL) {
	    perror(sourcefile);
	    exit(1);
	}

	InitTables();
	if (yyparse() || semantic_errors) {
	    fprintf(stderr, "%s: not correct\n", sourcefile);
	    exit(1);
	}

	DumpNetMaskTable();
	DumpActionTable();
}
