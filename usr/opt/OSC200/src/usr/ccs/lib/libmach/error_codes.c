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
static char	*sccsid = "@(#)$RCSfile: error_codes.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:10:54 $";
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
 * File:	errr_codes.c
 * Purpose:
 *	Generic error code interface
 */

#include <mach/error.h>
#include <servers/errorlib.h>
#include "err_kern.sub"
#include "err_us.sub"
#include "err_server.sub"
#include "err_ipc.sub"


struct error_system errors[err_max_system+1] = {
	/* 0; err_kern */
	{
		errlib_count(err_os_sub),
		"(operating system/?) unknown subsystem error",
		err_os_sub,
	},
	/* 1; err_us */
	{
		errlib_count(err_us_sub),
		"(user space/?) unknown subsystem error",
		err_us_sub,
	},
	/* 2; err_server */
	{
		errlib_count(err_server_sub),
		"(server/?) unknown subsystem error",
		err_server_sub,
	},
	/* 3 (& 3f); err_ipc */
	{
		errlib_count(err_ipc_sub),
		"(ipc/?) unknown subsystem error",
		err_ipc_sub,
	},
};


int error_system_count = errlib_count(errors);


