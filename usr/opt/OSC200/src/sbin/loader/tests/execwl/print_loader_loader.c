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
static char	*sccsid = "@(#)$RCSfile: print_loader_loader.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:42:52 $";
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

#include <stdio.h>

extern int _ldr_crt0_request;
static int *_lcrp = &_ldr_crt0_request;
int _ldr_present = 1;

void auxv_init();
char *auxv_get_exec_loader_filename();

main(argc, argv)
	char *argv[];
{
	char *exec_loader_filename;

	auxv_init();
	if ((exec_loader_filename = auxv_get_exec_loader_filename()) == (char *)-1) {
		(void)fprintf(stderr, "%s: auxv_get_exec_loader_filename() failed\n",
			argv[0]);
		exit(1);
	}
	(void)printf("%s\n", exec_loader_filename);
	exit(0);
}
