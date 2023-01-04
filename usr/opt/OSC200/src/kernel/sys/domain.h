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
 *	@(#)$RCSfile: domain.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/12/15 22:11:44 $
 */ 
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base:	domain.h	7.3 (Berkeley) 6/27/88
 *	Merged: domain.h	7.4 (Berkeley) 6/28/90
 */

#ifndef	_SYS_DOMAIN_H_
#define _SYS_DOMAIN_H_

/* forward declaration required for C++ */
#ifdef __cplusplus
struct protosw;
#endif

/*
 * Structure per communications domain.
 */
struct	domain {
	int	dom_family;		/* AF_xxx */
	char	*dom_name;
	void	(*dom_init)();		/* initialize domain data structures */
	int	(*dom_externalize)();	/* externalize access rights */
	void	(*dom_dispose)();	/* dispose of internalized rights */
	struct	protosw *dom_protosw, *dom_protoswNPROTOSW;
	struct	domain *dom_next;
	int	dom_refcnt;		/* # sockets in this domain */
	void	(*dom_funnel)();	/* uniprocessor compat */
	void	(*dom_funfrc)();	/* uniprocessor compat */
#if	defined(_KERNEL) && NETSYNC_LOCK
	simple_lock_data_t	dom_rc_lock;
#endif
};

#ifdef	_KERNEL
extern	struct domain *domains;
#if	NETSYNC_LOCK
extern	lock_data_t domain_lock;
#define DOMAIN_LOCKINIT()	lock_init2(&domain_lock, TRUE, LTYPE_DOMAIN)
#define DOMAIN_LOCK_DECL()	NETSPL_DECL(_ds)
#define DOMAIN_READ_LOCK()	{ NETSPL(_ds,net); lock_read(&domain_lock); }
#define DOMAIN_WRITE_LOCK()	{ NETSPL(_ds,net); lock_write(&domain_lock); }
#define DOMAIN_READ_UNLOCK()	{ lock_done(&domain_lock); NETSPLX(_ds); }
#define DOMAIN_WRITE_UNLOCK()	{ lock_done(&domain_lock); NETSPLX(_ds); }
#define	DOMAINRC_LOCKINIT(dp)	simple_lock_init(&((dp)->dom_rc_lock))
#define	DOMAINRC_LOCK(dp)	simple_lock(&((dp)->dom_rc_lock))
#define	DOMAINRC_UNLOCK(dp)	simple_unlock(&((dp)->dom_rc_lock))
#else	/* !NETSYNC_LOCK */
#define DOMAIN_LOCKINIT()
#define DOMAIN_LOCK_DECL()	NETSPL_DECL(_ds)
#define DOMAIN_READ_LOCK()	NETSPL(_ds,net)
#define DOMAIN_WRITE_LOCK()	NETSPL(_ds,net)
#define DOMAIN_READ_UNLOCK()	NETSPLX(_ds)
#define DOMAIN_WRITE_UNLOCK()	NETSPLX(_ds)
#define	DOMAINRC_LOCKINIT(dp)
#define	DOMAINRC_LOCK(dp)
#define	DOMAINRC_UNLOCK(dp)
#endif	/* !NETSYNC_LOCK */

#define	DOMAINRC_REF(dp) \
	{ DOMAINRC_LOCK(dp); (dp)->dom_refcnt++; DOMAINRC_UNLOCK(dp); }

#define	DOMAINRC_UNREF(dp) \
	{ DOMAINRC_LOCK(dp); (dp)->dom_refcnt--; DOMAINRC_UNLOCK(dp); }

/*
 * Uniprocessor compatibility: allows per-domain funnel operation.
 * Replaces all socket-level spl's, for instance.
 */
struct domain_funnel {
	void (*unfunnel)();		/* unfunnel operation or NULL */
	union {				/* local storage for operation */
		int	spl;			/* previous spl */
		caddr_t	other;			/* recursive lock_t, e.g. */
	} object;
};

#define DOMAIN_FUNNEL_DECL(f) \
	struct domain_funnel f;

#define DOMAIN_FUNNEL(dp, f) \
	{ (f).unfunnel = 0; if ((dp)->dom_funnel) (*(dp)->dom_funnel)(&(f)); }

#define DOMAIN_UNFUNNEL(f) \
	{ if ((f).unfunnel) (*(f).unfunnel)(&(f)); }

/* Forced unfunnel is used before sleeping in sosleep() */
#define DOMAIN_UNFUNNEL_FORCE(dp, f) \
	{ (f).unfunnel = 0; \
	  if ((dp)->dom_funfrc) (*(dp)->dom_funfrc)(&(f)); }

#endif
#endif
