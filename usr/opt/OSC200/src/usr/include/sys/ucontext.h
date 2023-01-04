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
 * @(#)$RCSfile: ucontext.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/01 21:07:33 $
 */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/user.h>

/*
 * The mcontext structure in the DEC OSF/1 implementation of ucontext
 * is really a copy of the underlying sigcontext structure from which
 * the ucontext structure is constructed.
 */

typedef struct sigcontext mcontext_t;

typedef struct ucontext {
	u_long		uc_flags;
	struct ucontext	*uc_link;
	sigset_t	uc_sigmask;
	stack_t		uc_stack;
	mcontext_t	uc_mcontext;
	long		uc_filler[5];	/* pad structure to 512 bytes */
} ucontext_t;

#endif /* _SYS_UCONTEXT_H */
