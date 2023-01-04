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
static char *rcsid = "@(#)$RCSfile: fdetach.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/10/11 20:00:29 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/*
 * NAME: fdetach
 *
 * FUNCTION: Detaches a stream from a filesystem node
 *
 * NOTES:
 *
 * RETURNS: on error, returns 1 via exit
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/errno.h>

#include        <locale.h>
#include        <nl_types.h>
#include        "fdetach_msg.h"

#define MSGSTR(num,str) catgets(catd,MS_FDETACH,num,str)

nl_catd catd;

main (argc, argv)
	int argc;
	char **argv;
{
	int err;

        (void) setlocale (LC_ALL,"");
        catd = catopen(MF_FDETACH,NL_CAT_LOCALE);

	if (argc != 2) {
		printf(MSGSTR(USAGE, "Usage: fdetach pathname\n"));
		exit(1);
	}

	if ((err = fdetach (argv[1])) != 0) {
		perror(MSGSTR(FDETACH, "fdetach"));
		exit(1);
	}
}
