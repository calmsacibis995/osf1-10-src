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
static char *rcsid = "@(#)$RCSfile: parity.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:23:23 $";
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
 * derived from trap.c	4.10	(ULTRIX)	12/19/90";
 */

			     /* HAL -- change to boottime global */
#include <machine/cpu.h>     /* CPEINTVL -- should be a global! */
#include <sys/syslog.h>      /* LOG_WARNING */
#include <sys/types.h>

extern	int	hz;

int     cpeintvl = CPEINTVL; /* logging freq. of cache parity err count */
int     cpecount = 0;        /* current cache parity error count */
int     cpelogcount = 0;     /* count at last log */
/*
 * This routine is initially called by startup() (after config),
 * thereafter it is called by timeout.  Its purpose is to log 
 * cache parity errors.
 * The count of cache parity errors is kept by per-platform
 * hardclock interfaces.
 */
chk_cpe()
{
        if (cpecount > cpelogcount) {
                log(LOG_WARNING, "Cache parity error count is %d\n",
			cpecount);
                cpelogcount = cpecount;
        }
        if (cpeintvl > 0)
                timeout (chk_cpe, (caddr_t) 0, cpeintvl * hz);
}
