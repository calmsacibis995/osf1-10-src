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
static char *rcsid = "@(#)$RCSfile: ptsname.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/06/23 21:23:34 $";
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
#ifdef _THREAD_SAFE
#define ptsname_r __ptsname_r
#pragma weak ptsname_r = __ptsname_r
#else
#define ptsname __ptsname
#pragma weak ptsname = __ptsname
#endif
#endif

/*
 * ptsname
 */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <pty.h>
#include "ts_supp.h"

/* 
 * ptsname: return the pathname of the slave pty device associated with
 *          the passed-in master pty.
 */
#ifdef _THREAD_SAFE
int
ptsname_r(int masterfd, char *slave_name, int len)
#else
char *
ptsname(int masterfd)
#endif	/* _THREAD_SAFE */
{
	dev_t devno;
#ifndef _THREAD_SAFE
	static char slave_name[MAX_PTYNAME_LEN];
#endif	/* _THREAD_SAFE */

	TS_EINVAL((slave_name == 0) || (len < MAX_PTYNAME_LEN));
	
	if ((devno = ioctl(masterfd, ISPTM, 0)) < 0)
		return (TS_FAILURE);

	sprintf(slave_name, "%s%d", _PATH_PTY, minor(devno));
	return (TS_FOUND(slave_name));
}
 

