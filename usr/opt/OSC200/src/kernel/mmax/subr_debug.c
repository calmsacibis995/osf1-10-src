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
static char	*sccsid = "@(#)$RCSfile: subr_debug.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:43:11 $";
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
/* 
 * Mach Operating System
 * Copyright (c) 1989 Encore Computer Corporation
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

#include <sys/secdefines.h>
#include <mmax_debug.h>
#include <mmax_idebug.h>
#include <mach_ldebug.h>

#include <kern/assert.h>
#include <mmax/machparam.h>

#include <sys/param.h>
#include <sys/user.h>
#if     SEC_BASE
#include <sys/security.h>
#endif

#if	MMAX_DEBUG
sys_panic()
{
#if     SEC_BASE
        if (privileged(SEC_DEBUG, 0))
#else   /* !SEC_BASE */
	if (!suser(u.u_cred, &u.u_acflag))
#endif  SEC_BASE 
		panic("sys_panic");
	else
		printf("hello, world\n");
}
#endif

#if	MMAX_DEBUG || MMAX_IDEBUG || MACH_LDEBUG
/*
 * panic on failure of ASSERT macro.
 */
assfail(a, f, l)
register char *a;			/* ASSERT's parameter expression */
register char *f;			/* file containing failing ASSERT */
int l;					/* line number of failing ASSERT */
{
	printf("assertion failed: %s, file: %s, line: %d\n", a, f, l);
	panic("assertion error");
}
#endif

#if	DEB_DEBUG
stupid_printf(s)
char	*s;
{
	register char c;

	while (c = *s++)
		slputc(c);
}

static void
dbtc(message)
char	*message;
{
	if (!message)
		return;
	while (*message) {
		while (*SCC_CONSOLE_XMTREQ)	{}
		*SCC_CONSOLE_XMTBUF = *message++;
		*SCC_CONSOLE_XMTREQ = TRUE;
	}
}
#endif


#if	MMAX_IDEBUG
ckints()
{
	ASSERT(icu_ints_on());
	ASSERT(ints_on());
}
#endif
