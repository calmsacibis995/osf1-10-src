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
static char *rcsid = "@(#)$RCSfile: nfsportmon.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1992/10/13 14:46:08 $";
#endif

#include <stdio.h>
#include <sys/sysinfo.h>

main(argc, argv)
int	argc;
char	*argv[];
{

	/*
	 * must be superuser to run 
	 */

	if (geteuid() != 0){
		(void) fprintf(stderr, "nfsportmon:  must be super user\n");
		(void) fflush(stderr);
		exit(1);
	}

	**++argv;

	if ((argc != 2) || ((*argv)[0] != 'o')) {
		(void) fprintf(stderr,"usage: nfsportmon {on, off}\n");
		exit (1);
	}

	switch ((*argv)[1]) {

		case 'n':
			nfsportmonon ();
			break;

		case 'f':
			nfsportmonoff ();
			break;

		default:
			(void) fprintf(stderr,"usage: nfsportmon {on, off}\n");
			exit(1);
			break;
	}
}

int arg[2] = {SSIN_NFSPORTMON, 0};

nfsportmonon ()
{
	arg[1] = 1;
	if (setsysinfo(SSI_NVPAIRS, arg, 1, 0, 0))
		perror("nfsportmon - setsysinfo failed");
}

nfsportmonoff ()
{
	if (setsysinfo(SSI_NVPAIRS, arg, 1, 0, 0))
		perror("nfsportmon - setsysinfo failed");
}
