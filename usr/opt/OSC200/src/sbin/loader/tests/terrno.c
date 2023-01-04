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
static char	*sccsid = "@(#)$RCSfile: terrno.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:11 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* terrno.c
 * Test loader errno printing
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <loader.h>

#include "ldr_types.h"
#include "ldr_malloc.h"
#include "ldr_sys_int.h"
#include "ldr_errno.h"


main(argc, argv)
int argc;
char **argv;
{
	char	ch = 'Z';

	ldr_heap_init();

	ldr_log("This is the log message, %s, %d, 0x%x, %c %c\n",
		"a string", 69, 0x69, (char)'Q', ch);

	ldr_msg("This should appear on %s\nldr_msg status %E\n%B\n",
		  "/dev/tty", LDR_EEXIST);

	ldr_msg("Test of bad errnos\nZero: %E\nPositive: %E\nToo small: %E\n",
		  0, 10, -200);

	ldr_log("Some funny numbers: 0x%0x%20d %020d\n",
		69, 696969, 69696969);

	ldr_msg("Printing funny numbers from the log\n%B\n");

	exit(0);
}
