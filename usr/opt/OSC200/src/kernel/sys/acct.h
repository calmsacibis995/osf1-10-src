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
 *	@(#)$RCSfile: acct.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/04/15 14:06:03 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/* acct.h	5.2 - 87/01/09 - 18:20:16 */
/*
 * OSF/1 Release 1.0.3
 */

#ifndef _SYS_ACCT_H_
#define _SYS_ACCT_H_

#include "sys/types.h"
#include "sys/param.h"
/*
 * Accounting structures
 */

typedef ushort comp_t;		/* "floating point" */
		/* 13-bit fraction, 3-bit exponent  */

struct	acct
{
	char	ac_comm[8];		/* Accounting command name */
	comp_t	ac_io;			/* number of chars read/written */
	comp_t	ac_utime;		/* Accounting user time */
	comp_t	ac_stime;		/* Accounting system time */
	comp_t	ac_etime;		/* Accounting elapsed time */
	time_t	ac_btime;		/* Beginning time */
	uid_t	ac_uid;			/* Accounting user ID */
	gid_t	ac_gid;			/* Accounting group ID */
	short	ac_mem;			/* average memory usage */
	comp_t	ac_rw;			/* blocks read or written */
	dev_t	ac_tty;			/* control typewriter */
	char	ac_flag;		/* Accounting flag */
	char	ac_stat;		/* Exit status */
};

#define AFORK	0001		/* has executed fork, but no exec */
#define ASU	0002		/* used super-user privileges */
#define	ACCTF	0300		/* record type: 00 = acct */
#define ACOMPAT 0004		/* used compatibility mode */
#define ACORE	0010		/* dumped core */
#define AXSIG	0020		/* killed by a signal */

/*
 * 1/AHZ is the granularity of the data encoded in the various
 * comp_t fields.  This is not necessarily equal to hz.
 */
#define AHZ 64

#ifdef	_KERNEL
extern struct	acct	acctbuf;
extern struct	vnode	*acctp;
#endif	/* _KERNEL */


#endif	/* _SYS_ACCT_H_ */
