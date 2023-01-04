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
static char	*sccsid = "@(#)$RCSfile: xpr.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1993/01/19 17:48:36 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * xpr silent tracing circular buffer.
 */

#include <kern/xpr.h>
#include <kern/lock.h>
#include <machine/cpu.h>
#include <machine/machparam.h>		/* for splhigh */
#include <vm/vm_kern.h>

decl_simple_lock_data(,xprlock)

int nxprbufs = 0;	/* Number of contiguous xprbufs allocated */
unsigned long xprflags = 0;	/* Bit mask of xpr flags enabled */
struct xprbuf *xprbase;	/* Pointer to circular buffer nxprbufs*sizeof(xprbuf)*/
struct xprbuf *xprptr;	/* Currently allocated xprbuf */
struct xprbuf *xprlast;	/* Pointer to end of circular buffer */

/*VARARGS1*/
xpr(msg, arg1, arg2, arg3, arg4, arg5, arg6)
	char *msg;
	long arg1, arg2, arg3, arg4, arg5, arg6;
{
	register s;
	register struct xprbuf *x;

#ifdef	lint
	arg6++;
#endif	lint

	/* If we aren't initialized, ignore trace request */
	if (!nxprbufs)
		return;
	/* Gaurd against all interrupts and allocate next buffer */
	s = splhigh();
	simple_lock(&xprlock);
	x = xprptr++;
	if (xprptr >= xprlast) {
		/* wrap around */
		xprptr = xprbase;
	}
	simple_unlock(&xprlock);
	splx(s);
	x->msg = msg;
	x->arg1 = arg1;
	x->arg2 = arg2;
	x->arg3 = arg3;
	x->arg4 = arg4;
	x->arg5 = arg5;
	x->timestamp = XPR_TIMESTAMP;
	x->cpuinfo = cpu_number();
}

xprbootstrap()
{
	simple_lock_init(&xprlock);
	if (nxprbufs == 0)
		return;	/* assume XPR support not desired */
	xprbase = (struct xprbuf *)kmem_alloc(kernel_map, 
			(vm_size_t)(nxprbufs * sizeof(struct xprbuf)));
	xprlast = &xprbase[nxprbufs];
	xprptr = xprbase;
}

int		xprinitial = 0;

xprinit()
{
	xprflags |= (long)xprinitial;
}

/*
 *	Save 'number' of xpr buffers to the area provided by buffer.
 */

xpr_save(buffer,number)
struct xprbuf *buffer;
int number;
{
    int i,s;
    struct xprbuf *x;
    s = splhigh();
    simple_lock(&xprlock);
    if (number > nxprbufs) number = nxprbufs;
    x = xprptr;
    
    for (i = number - 1; i >= 0 ; i-- ) {
	if (--x < xprbase) {
	    /* wrap around */
	    x = xprlast - 1;
	}
	bcopy(x,&buffer[i],sizeof(struct xprbuf));
    }
    simple_unlock(&xprlock);
    splx(s);
}

