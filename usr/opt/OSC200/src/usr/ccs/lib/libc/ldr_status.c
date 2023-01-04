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
static char	*sccsid = "@(#)$RCSfile: ldr_status.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:24:07 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_errno.c
 * error message handling for loader
 *
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak ldr_status_to_errno = __ldr_status_to_errno
#endif
#include <sys/types.h>
#include <errno.h>
#include <loader.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>


/* All loader internal errno's have magnitudes > SYSTEM_ERRNO_MAX */

#define	SYSTEM_ERRNO_MAX	0x3fffffff


/* This table translates internal loader status codes to errno's */

const static int stat2errno[] = {
	0,				/* 0 */
	EINVAL,				/* LDR_ENOMODULE */
	EINVAL,				/* LDR_ENOMAIN */
	ENOMEM				/* LDR_EALLOC */
	};

#define	LDR_MAXSTATUS	(sizeof(stat2errno) / sizeof(int))


int
ldr_status_to_errno(int rc)

/* Translate a loader error status to a system errno.  If loader error status
 * is non-negative, returns 0 (success); if negative and magnitude is less
 * than SYSTEM_ERRNO_MAX, returns negative of loader error status; otherwise
 * uses table lookup to translate.
 */
{
	if (rc >= 0)
		return(rc);
	else if (rc >= -SYSTEM_ERRNO_MAX)
		return(-rc);
	else {
		rc = -rc - SYSTEM_ERRNO_MAX;
		if (rc >= LDR_MAXSTATUS)
			return(EINVAL);
		return(stat2errno[rc]);
	}
}
