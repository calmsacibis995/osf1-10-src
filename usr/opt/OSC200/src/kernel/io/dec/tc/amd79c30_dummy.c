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
static char *rcsid = "@(#)$RCSfile: amd79c30_dummy.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/03/15 17:39:53 $";
#endif

#ifndef STRISDN
/*
 * Dummy file for BBA when there is no ISDN (STRISDN is not defined)
 */
/*
 *Nothing to initialize here
 */
/* ARGSUSED */
dlld_unix_init (func)
	void *func;
{
	return 0;
}
/*
 * We don't know this ioctl
 */
/* ARGSUSED */
dlld_ioctl (mp, unit, q)
	struct msgblk *mp;
	int unit;
	struct queue *q;
{
	return -1;
}
/*
 * Dump any mproto messages
 */
/* ARGSUSED */
dlld_mproto (bp, unit)
	struct msgblk *bp;
	int unit;
{
	(void)freemsg (bp);
	return 1;
}
/* ARGSUSED */
void
ISDN_dscisr (unit, ireg)
	int unit, ireg;
{
}
#endif /* STRISDN */
