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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: local.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 16:54:23 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
*/
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 *	1.9  com/cmd/csh/local.h, cmdcsh, bos320, 9132320b 7/24/91 17:42:24
 * 
 *
 * This file defines certain local parameters
 * A symbol should be defined in Makefile for local conditional
 * compilation, e.g. IIASA or ERNIE, to be tested here and elsewhere.
 *
 * Fundamental definitions which may vary from system to system.
 *
 *	BUFR_SIZ	The i/o buffering size; also limits word size
 *	MAILINTVL	How often to mailcheck; more often is more expensive
 */

#include <sys/limits.h>

#define BUFR_SIZ 2*BUFSIZ    /* 2 bytes per character keeps limits compatable */
#define FORKSLEEP	10	/* delay loop on non-interactive fork failure */
#define	MAILINTVL	600	/* 10 minutes */


/*
 * The shell moves std in/out/diag and the old std input away from units
 * 0, 1, and 2 so that it is easy to set up these standards for invoked
 * commands.  If possible they should go into descriptors closed by exec.
 */
#define	FSHTTY	15		/* /dev/tty when manip pgrps */
#define	FSHIN	16		/* Preferred desc for shell input */
#define	FSHOUT	17		/* ... shell output */
#define	FSHDIAG	18		/* ... shell diagnostics */
#define	FOLDSTD	19		/* ... old std input */

#define track_open(num)  { if ((num) > open_max) open_max = num; }

extern int open_max;            /* Highest numbered file descriptor opened;
                                   we track this rather than using OPEN_MAX
                                   because OPEN_MAX is too big.
                                 */

#define INF	1000		/* minargs in the bfunc array in init.c */

/*
 * The keywords for the parser
 */
#define	ZBREAK		0
#define	ZBRKSW		1
#define	ZCASE		2
#define	ZDEFAULT 	3
#define	ZELSE		4
#define	ZEND		5
#define	ZENDIF		6
#define	ZENDSW		7
#define	ZEXIT		8
#define	ZFOREACH	9
#define	ZGOTO		10
#define	ZIF		11
#define	ZLABEL		12
#define	ZLET		13
#define	ZSET		14
#define	ZSWITCH		15
#define	ZTEST		16
#define	ZTHEN		17
#define	ZWHILE		18
