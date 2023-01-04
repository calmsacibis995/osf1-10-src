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
#if !defined(lint) && defined(SECONDARY)
static char	*sccsid = "@(#)$RCSfile: alphaboot.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1992/07/31 14:31:04 $";
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
 * alphaboot.c
 */

/* 
 * derived from mipsboot.c	3.1 	(ULTRIX/OSF)	2/28/91";
 */

#include <sys/param.h>
#include <machine/cpu.h>
#include <machine/entrypt.h>

char	*oboot = "osf_boot";

/*
 * Functional Discription:
 *
 * Inputs:
 *	none
 *
 * Outputs:
 *	none
 *
 */

int ub_argc;
char **ub_argv;
char **ub_envp;

main (argc,argv,envp)
int argc;
char **argv, **envp;
{
	int     io;
	char	*boot;

	ub_argc = argc;	/* save prom's args for kernel */
	ub_argv = argv;	/* save prom's args for kernel */
	ub_envp = envp;	/* save prom's args for kernel */


	boot = (char *)prom_getenv("booted_dev");

	if(devopen(boot, 0) >= 0) {
		if ((io = open (oboot, 0)) >= 0) {
			load_image (io,1);	
		}
		prom_puts("can't open osf_boot\n");
	}
}
