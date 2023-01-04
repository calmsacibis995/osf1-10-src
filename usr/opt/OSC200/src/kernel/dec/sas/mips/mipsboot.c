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
static char	*sccsid = "@(#)$RCSfile: mipsboot.c,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 01:38:27 $";
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
 * mipsboot.c
 */

/* 
 * derived from mipsboot.c	3.1 	(ULTRIX/OSF)	2/28/91";
 */

/*
 * Revision History
 *
 * 06-Mar-91 -- Don Dutile
 *      Removed LABELS conditional compilation sections, hardwiring
 *      use of osf_boot (no optional use of mach_boot or ultrixboot).
 *      inlined exit() functionality of v4.2 in order to reduce size
 *      of mipsboot.o further.
 *
 * 21-Feb-91 -- Don Dutile
 *      Merged v4.2 mipsboot.c to sandbox mipsboot.c.
 *      Basically added rex console support in.
 *
 * jas - Added entrypt.h.  It contains rex callbacks.  Modified code to
 *       use rex callbacks if envp indicates new console present.
 */

#include "../../../sys/param.h"
#include <machine/cpu.h>
#include <machine/entrypt.h>

#define	printf	_prom_printf
#define	getenv	prom_getenv
#define stop _prom_restart

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

extern int prom_io;

int ub_argc;
char **ub_argv;
char **ub_envp;
char **ub_vector;

extern int rex_base;  	/* used by REX console */
extern int rex_magicid; /* used by REX console */

main (argc,argv,envp,vector)
int argc;
char **argv, **envp, **vector;
{
	int     io;
	char	*boot;

	ub_argc = argc;	/* save prom's args for kernel */
	ub_argv = argv;	/* save prom's args for kernel */
	ub_envp = envp;	/* save prom's args for kernel */
	ub_vector = vector; /* save prom's args for kernel */

	rex_magicid = (int)envp;

	if((int)envp == REX_MAGIC) {
		rex_base = (int)vector;
		if(rex_bootinit() < 0) {
			printf("binit fld\n");
			stop();
		}
	}
	else {
		rex_base = 0;
		boot = (char *)getenv("boot");
/*
 * see sys.c file for changes with respect to this comment
 */
#ifdef notdef
#if LABELS
		/*
	 	* The boot string should be of the form:
	 	* rz(ctrlr,unit)(partition)program
	 	* where '(partition)' is optional. This differs slightly
	 	* from the normal PMAX boot command, which is
	 	* rz(ctrlr,unit,partition)program
	 	* By omitting the partition from the first ()'s, we use
	 	* the ROMs to access the raw device (aka 'c' partition).
	 	*/
#endif /* LABELS */
#endif /* notdef */

		if(devopen(boot, 0) < 0 )	{
			printf("devopen(boot) fld\n");
        		stop();
		}
	}

	if (
	    ((io = open (oboot, 0)) < 0)) { 	/* Open the image */
		printf("can't open %s\n", oboot);
        	stop();
	}

	load_image (io,1);	

       	stop();
}

