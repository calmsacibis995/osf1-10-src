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
static char	*sccsid = "@(#)$RCSfile: XDevModLog.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:16 $";
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

#if SEC_BASE


/*
	filename:
		XDevModLog.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for explicitly locking a user account
		
	entry points:
		DevModLogStart()
		DevModLogOpen()
		DevModLogClose()
		DevModLogStop()
*/

/* Common C include files */
#include <sys/types.h>

/* ISSO include files */
#include "XMain.h"
#include "XDevices.h"

/* Definitions */
static struct dev_if dv;

/* External routines */
extern int
	XGetDeviceInfo();

extern void
#ifdef V2
	XDevModHos(),
#endif
	XDevModPri(),
	XDevModRem(),
	XDevModTer();

void 
DevModLogStart() 
{
}

/* Main entry point for XDevModLog.c - Decide which device it is and then
 * call the appropiate screen */
 
void 
DevModLogOpen()
{
	struct dev_asg *dev = &dv.dev;
	int ret;

	ret = XGetDeviceInfo(chosen_device_name, &dv);
	if (ret)
		return;

	/* Bring up appropiate screen */
	if (ISBITSET(dev->ufld.fd_type, AUTH_DEV_TERMINAL)) {
		DevModTerOpen();
		}
	else if (ISBITSET(dev->ufld.fd_type, AUTH_DEV_TAPE))
		DevModRemOpen();
	else if (ISBITSET(dev->ufld.fd_type, AUTH_DEV_PRINTER))
		DevModPriOpen();
	/* This is 2.0 XXXXX
#ifdef V2.0
	else if (ISBITSET(dev->ufld.fd_type, AUTH_DEV_REMOTE))
		DevModHosOpen();
#endif
		*/

}

void 
DevModLogClose() 
{
}

void 
DevModLogStop() 
{
}

#endif /* SEC_BASE **/
