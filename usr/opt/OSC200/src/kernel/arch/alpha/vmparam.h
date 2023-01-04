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
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*-----------------------------------------------------------------------
 *
 * Modification History: machine/alpha/vmparam.h
 *
 *-----------------------------------------------------------------------
 */

#ifndef	_MACHINE_VMPARAM_H_
#define	_MACHINE_VMPARAM_H_ 1

/*
 * Machine dependent constants for Alpha
 */

#define USRTEXT		no_USRTEXT_constant_on_Alpha /* gag the compiler */
#define USRDATA		no_USRDATA_constant_on_Alpha /* gag the compiler */
/*
 * The user stack grows down from USRSTACK.
 */
#define	USRSTACK	((vm_offset_t)u.u_stack_end)	/* base of user stack */
#define	EA_SIZE		32		/* EMULATE_AREA size */

/*
 * Virtual memory related constants, all in bytes
 */
#ifndef MAXDSIZ
#define	MAXDSIZ		(1<<30)		/* max data size */
#endif

#ifndef	MAXSSIZ
#define	MAXSSIZ		(1<<25)		/* max stack size */
#endif

#ifndef MAXRSS
#define MAXRSS		(100)		/* maximum rss limit */
#endif

#ifndef DFLDSIZ
#define	DFLDSIZ		(1<<27)		/* initial data size limit */
#endif

#ifndef	DFLSSIZ
#define	DFLSSIZ		(1<<21)		/* initial stack size limit */
#endif

#ifndef DFLRSS
#define DFLRSS		(100)		/* initial rss limit */
#endif

#endif	/* _MACHINE_VMPARAM_H_ */
