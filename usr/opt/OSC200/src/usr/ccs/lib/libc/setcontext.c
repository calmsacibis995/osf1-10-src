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
static char *rcsid = "@(#)$RCSfile: setcontext.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/30 21:39:07 $";
#endif

#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak setcontext = __setcontext 
#endif

#include <signal.h>
#include <sys/siginfo.h>
#include <sys/ucontext.h>

/*
 * Copy changes from the ucontext into the sigcontext, then call
 * sigreturn(). 
 */
setcontext(ucontext_t *ucp)
{
	struct sigcontext *scp;

	scp = &ucp->uc_mcontext;
	fix_sigcontext(scp, ucp, FALSE);
	return(sigreturn(scp));
}

