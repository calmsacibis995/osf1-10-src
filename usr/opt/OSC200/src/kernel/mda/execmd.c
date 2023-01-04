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
static char	*sccsid = "@(#)$RCSfile: execmd.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:05 $";
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
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#define	RDR	0
#define	WTR	1

static	int p[2];
static	int q[2];

execmd(ifid, ofid, cmd, arg1, arg2, arg3)
char	*cmd;
int	*ifid, *ofid;
{
	int pid;

	if(pipe(p) < 0)
		return(-1);
	if(pipe(q) < 0)
		return(-1);

	if((pid = vfork()) == 0) {
		close(0);
		dup(p[RDR]);
		close(1);
		dup(q[WTR]);
		close(2);
		dup(q[WTR]);

		execl("/bin/sh", "sh", "-c", cmd,
			arg1 ? arg1 : 0,
			arg2 ? arg2 : 0,
			arg3 ? arg3 : 0,
			(char *)0);
		_exit(127);
	} else if(pid == -1)
		return(-1);

	*ifid = q[RDR];
	*ofid = p[WTR];
	return(0);
}

exedone()
{
	int	ret, status;

	close(p[0]);
	close(p[1]);
	close(q[0]);
	close(q[1]);

	while((ret = wait(&status)) != -1)
		;
	return(ret);
}
