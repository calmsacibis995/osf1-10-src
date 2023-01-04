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
 *	@(#)$RCSfile: ga_data.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:30:27 $
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
 * derived from ga_data.c	4.1	(ULTRIX)	7/2/90
 */

#define _GA_DATA_C_

#include <sys/param.h>
#include <io/dec/tc/gx.h>
#include <io/dec/tc/ga.h>

#include "ga.h"				/* for NGA */

#ifdef BINARY

#ifdef GA_ALLOC_TX
int ga_dummy();
#endif

#ifdef GA_ALLOC_DT
extern int Ring2da[];
#endif

/* XXX temp debugging garf ??? XXX */
int gaVintr_count[8] = {0, 0, 0, 0, 0 ,0 ,0, 0};
int gaSintr_count[8] = {0, 0, 0, 0, 0 ,0 ,0, 0};
int gaPintr_count[8] = {0, 0, 0, 0, 0 ,0 ,0, 0};
int gaEintr_count[8] = {0, 0, 0, 0, 0 ,0 ,0, 0};

ga_ComAreaPtr gaComArea;

#else binary

#ifdef GA_ALLOC_DT
/* size MUST match gx_priv_size in ga_cons_init, please... */
int	Ring2da[ (sizeof(gxPriv) + _128K + NBPG) / sizeof(int) ];
#endif

#ifdef GA_ALLOC_TX

#define _16W	i++;i++;i++;i++;i++;i++;i++;i++;i++;i++;i++;i++;i++;i++;i++;i++
#define _128W	_16W;_16W;_16W;_16W;_16W;_16W;_16W;_16W
#define _4KB	_128W;_128W;_128W;_128W;_128W;_128W;_128W;_128W
#define _20KB	_4KB;_4KB;_4KB;_4KB;_4KB
#define _32KB	_4KB;_4KB;_4KB;_4KB;_4KB;_4KB;_4KB;_4KB
#define _128KB	_32KB;_32KB;_32KB;_32KB

ga_dummy()
{
    register int i;

#   ifdef HWDEBUG
    _20KB;				/* sizeof(gxPriv) */
#   else
    _4KB;
#   endif

    _128KB;				/* + 128KB */
    _4KB;				/* + NBPG */
    _4KB;				/* + shm alignment */

#if NGA > 1
    _128KB;				/* + 128KB */
    _4KB;				/* + NBPG */
    _4KB;				/* + shm alignment */
#endif

#if NGA > 2
    _128KB;				/* + 128KB */
    _4KB;				/* + NBPG */
    _4KB;				/* + shm alignment */
#endif
}

#endif

#endif binary
