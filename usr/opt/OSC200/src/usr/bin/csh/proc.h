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
static char rcsid[] = "@(#)$RCSfile: proc.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 17:09:36 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
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
 */

/* flag values for p_flags */
#define	PRUNNING	0x0001		/* running */
#define	PSTOPPED	0x0002		/* stopped */
#define	PNEXITED	0x0004		/* normally exited */
#define	PAEXITED	0x0008		/* abnormally exited */
#define	PSIGNALED	0x0010		/* terminated by a signal != SIGINT */
#define	PNOTIFY		0x0020		/* notify async when done */
#define	PTIME		0x0040		/* job times should be printed */
#define	PAWAITED	0x0080		/* top level is waiting for it */
#define	PFOREGND	0x0100		/* started in shells pgrp */
#define	PDUMPED		0x0200		/* process dumped core */
#define	PDIAG		0x0400		/* diagnostic output also piped out */
#define	PPOU		0x0800		/* piped output */
#define	PREPORTED	0x1000		/* status has been reported */
#define	PINTERRUPTED	0x2000		/* job stopped via interrupt signal */
#define	PPTIME		0x4000		/* time individual process */
#define	PNEEDNOTE	0x8000		/* notify as soon as practical */

#define	PNULL		(struct process *)0
#define	PMAXLEN		80

#define	PALLSTATES	\
		(PRUNNING|PSTOPPED|PNEXITED|PAEXITED|PSIGNALED|PINTERRUPTED)

/* defines for arguments to pprint */
#define	NUMBER		01
#define	NAME		02
#define	REASON		04
#define	AMPERSAND	010
#define	FANCY		020
#define	SHELLDIR	040		/* print shell's dir if not the same */
#define	JOBDIR		0100		/* print job's dir if not the same */
#define	AREASON		0200

struct	process	proclist;		/* list head of all processes */
bool	pnoprocesses;			/* pchild found nothing to wait for */

struct	process *pholdjob;		/* one level stack of current jobs */

struct	process *pcurrjob;		/* current job */
struct	process	*pcurrent;		/* current job in table */
struct	process *pprevious;		/* previous job in table */

short	pmaxindex;			/* current maximum job index */

bool	timesdone;			/* shtimes buffer full ? */
