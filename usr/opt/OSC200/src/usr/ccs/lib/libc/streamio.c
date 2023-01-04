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
static char	*sccsid = "@(#)$RCSfile: streamio.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 22:53:38 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif

/** Copyright (c) 1989  Mentat Inc. **/

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak getmsg = __getmsg
#pragma weak putmsg = __putmsg
#endif
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <stropts.h>
#include "ts_supp.h"

int
getmsg (fd, ctlptr, dataptr, flags)
	int	fd;
	struct strbuf	* ctlptr;
	struct strbuf	* dataptr;
	int	* flags;
{
	struct strpeek	strp;
	int	i1;

	memset(&strp, '\0', sizeof(struct strpeek));
	if (ctlptr)
		strp.ctlbuf = *ctlptr;
	else
		strp.ctlbuf.maxlen = -1;
	if (dataptr)
		strp.databuf = *dataptr;
	else
		strp.databuf.maxlen = -1;
	strp.flags = *flags;
	if ((i1 = ioctl(fd, I_GETMSG, &strp)) >= 0) {
		if (ctlptr)
			*ctlptr = strp.ctlbuf;
		if (dataptr)
			*dataptr = strp.databuf;
		*flags = strp.flags;
	} else if (TS_GETERR() == ENOTTY)
		TS_SETERR(ENOSTR);
	return i1;
}

int
putmsg (fd, ctlptr, dataptr, flags)
	int	fd;
	struct strbuf	* ctlptr;
	struct strbuf	* dataptr;
	int	flags;
{
	struct strpeek	strp;
	int i1;

	memset(&strp, '\0', sizeof(struct strpeek));
	if (ctlptr)
		strp.ctlbuf = *ctlptr;
	else
		strp.ctlbuf.len = -1;
	if (dataptr)
		strp.databuf = *dataptr;
	else
		strp.databuf.len = -1;
	strp.flags = flags;
	if ((i1 = ioctl(fd, I_PUTMSG, &strp)) < 0 && TS_GETERR() == ENOTTY)
		TS_SETERR(ENOSTR);
	return i1;
}
