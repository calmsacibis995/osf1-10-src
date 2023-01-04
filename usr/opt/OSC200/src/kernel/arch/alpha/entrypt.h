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
/*
 *	"@(#)entrypt.h	9.1	(ULTRIX/OSF)	10/21/91"
 */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: alpha/entrypt.h
 *
 * 21-Sep-90 -- afd
 *	Created this file for Alpha support.  It is used for both the
 *	standalone system and the kernel to gain access to console callbacks.
 *
 *	Any .c file that uses call backs must include both
 *	machine/rpb.h and machine/entrypt.h.
 */


/*
 * Protect the file from multiple includes.
 */
#ifndef _ENTRYPT_HDR_
#define _ENTRYPT_HDR_

#define STARTUP_STACK	0x20030000

#ifdef KERNEL
extern long hwrpb_addr;			/* kernel virt addr of HWRPB */
#endif /* KERNEL */

/*
 * args to promexec -- monitor support for loading new programs
 *
 * bootfiles should be specified as: alpha boot syntax
 */
struct promexec_args {
	char	*pa_bootfile;		/* file to boot (only some devices) */
	int	pa_argc;		/* arg count */
	char	**pa_argv;		/* arg vector */
	char	**pa_environ;		/* environment vector */
	int	pa_flags;		/* flags, (see below) */
};

/*
 * promexec flags
 */

#define	EXEC_NOGO	1	/* just load, don't transfer control */
#endif /* _ENTRYPT_HDR_ */
