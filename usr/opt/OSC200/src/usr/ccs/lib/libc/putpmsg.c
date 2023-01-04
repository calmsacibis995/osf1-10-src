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
static char *rcsid = "@(#)$RCSfile: putpmsg.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/07 18:06:18 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#define putpmsg __putpmsg
#pragma weak putpmsg = __putpmsg
#endif

/** Copyright (c) 1989-1991  Mentat Inc. **/

#include <sys/errno.h>
#include <stropts.h>
#include "ts_supp.h"

int
putpmsg (fd, ctlptr, dataptr, band, flags)
	int	fd;
	struct strbuf	* ctlptr;
	struct strbuf	* dataptr;
	int	band;
	int	flags;
{
	struct strpmsg	strp;
	int	i1;

	memset(&strp, '\0', sizeof(struct strpmsg));
	if (ctlptr)
		strp.ctlbuf = *ctlptr;
	else
		strp.ctlbuf.len = -1;
	if (dataptr)
		strp.databuf = *dataptr;
	else
		strp.databuf.len = -1;
	strp.band = band;
	strp.flags = flags;
	if ((i1 = ioctl(fd, I_PUTPMSG, (caddr_t)&strp)) < 0 &&
	    TS_GETERR() == ENOTTY)
		TS_SETERR(ENOSTR);
	return i1;
}
