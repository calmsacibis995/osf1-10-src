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

#if !defined(lint) && defined(SECONDARY)
static char	*sccsid = "@(#)$RCSfile: mipsboot.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:38:35 $";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
#include "../../../sys/param.h"
#include "../../cpu.h"

#define	printf	_prom_printf
#define	getenv	prom_getenv
#define stop _prom_restart

#if LABELS
char	*oboot = "osf_boot";
#endif
char	*mboot = "mach_boot";
char    *uboot = "ultrixboot";

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

main (argc,argv,envp)
int argc;
char **argv, **envp;
{
	int     io;
	char	*boot;

	ub_argc = argc;	/* save prom's args for kernel */
	ub_argv = argv;	/* save prom's args for kernel */
	ub_envp = envp;	/* save prom's args for kernel */

	boot = (char *)getenv("boot");
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
#endif
	if (devopen(boot, 0) < 0) {
		printf("boot device open failed\n");
		stop();
	}
	if (
#if LABELS
	    ((io = open (oboot, 0)) < 0) &&	/* Open the image */
#endif
	    ((io = open (mboot, 0)) < 0) &&	/* Open the image */
	    ((io = open (uboot, 0)) < 0)) {
		printf("can't open %s or %s\n", mboot, uboot);
		stop();
	}
	load_image (io);	
	stop();
}
