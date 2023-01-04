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
static char	*sccsid = "@(#)$RCSfile: tty_subr.c,v $ $Revision: 4.3.2.3 $ (DEC) $Date: 1992/01/30 23:51:58 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/clist.h>


static struct cblock zblock;

#define setquote(cp) \
	setbit(((char *)((vm_offset_t)(cp)&~CROUND)+sizeof(struct cblock *)), \
		(vm_offset_t)(cp)&CROUND)
#define isquote(cp) \
	isset(((char *)((vm_offset_t)(cp)&~CROUND)+sizeof(struct cblock *)), \
		(vm_offset_t)(cp)&CROUND)
#define cbptr(x) ((struct cblock *)(x))

udecl_simple_lock_data(,cblock_freelist_lock)

#define	CFREELIST_LOCK()	usimple_lock(&cblock_freelist_lock)
#define	CFREELIST_UNLOCK()	usimple_unlock(&cblock_freelist_lock)

#define	CFREELIST_RETURN(cblock)				\
			CFREELIST_LOCK();			\
			*(cblock) = zblock;			\
			(cblock)->c_next = cfreelist;		\
			cfreelist = (cblock);			\
			cfreecount += CBSIZE;			\
			CFREELIST_UNLOCK();

#define	CFREELIST_FETCH(cblockp)				\
			CFREELIST_LOCK();			\
			if ((*(cblockp) = cfreelist) != NULL) {	\
				cfreelist = (*(cblockp))->c_next;\
				cfreecount -= CBSIZE;		\
				(*(cblockp))->c_next = NULL;	\
			}					\
			CFREELIST_UNLOCK();

/*
 * Character list get/put
 */
getc(p)
	register struct clist *p;
{
	register struct cblock *bp;
	register int c;
	TSPLVAR(s)

	TSPLTTY(s);
	if (p->c_cc <= 0) {
		c = -1;
		p->c_cc = 0;
		p->c_cf = p->c_cl = NULL;
	} else {
		c = *p->c_cf & 0377;
		if (isquote(p->c_cf))
			c |= TTY_QUOTE;
		p->c_cf++;
		if (--p->c_cc<=0) {
 			bp = cbptr(p->c_cf-1);
 			bp = cbptr((vm_offset_t)bp & ~CROUND);
			p->c_cf = NULL;
			p->c_cl = NULL;
			CFREELIST_RETURN (bp);
		} else if (((vm_offset_t)p->c_cf & CROUND) == 0){
 			bp = cbptr(p->c_cf);
			bp--;
			p->c_cf = bp->c_next->c_info;
			CFREELIST_RETURN(bp);
		}
	}
	TSPLX(s);
	return (c);
}

/*
 * copy clist to buffer.
 * return number of bytes moved.
 */
q_to_b(q, cp, cc)
	register struct clist *q;
	register char *cp;
	long cc;
{
	register struct cblock *bp;
	register nc;
	char *acp;
	TSPLVAR(s)

	if (cc <= 0)
		return (0);
	TSPLTTY(s);
	if (q->c_cc <= 0) {
		q->c_cc = 0;
		q->c_cf = q->c_cl = NULL;
		TSPLX(s);
		return (0);
	}
	acp = cp;

	while (cc) {
		nc = sizeof (struct cblock) - ((vm_offset_t)q->c_cf & CROUND);
		nc = MIN(nc, cc);
		nc = MIN(nc, q->c_cc);
		(void) bcopy(q->c_cf, cp, (unsigned)nc);
		q->c_cf += nc;
		q->c_cc -= nc;
		cc -= nc;
		cp += nc;
		if (q->c_cc <= 0) {
 			bp = cbptr(q->c_cf - 1);
 			bp = cbptr((vm_offset_t)bp & ~CROUND);
			q->c_cf = q->c_cl = NULL;
			CFREELIST_RETURN(bp);
			break;
		}
		if (((vm_offset_t)q->c_cf & CROUND) == 0) {
 			bp = cbptr(q->c_cf);
			bp--;
			q->c_cf = bp->c_next->c_info;
			CFREELIST_RETURN(bp);
		}
	}
	TSPLX(s);
	return (cp-acp);
}

/*
 * Return count of contiguous characters
 * in clist starting at q->c_cf.
 * Stop counting if flag&character is non-null.
 */
ndqb(q, flag)
	register struct clist *q;
	long flag;
{
	register cc;
	TSPLVAR(s)

	TSPLTTY(s);
	if (q->c_cc <= 0) {
		cc = -q->c_cc;
		goto out;
	}
	cc = ((vm_offset_t)q->c_cf + CBSIZE) & ~CROUND;
	cc -= (vm_offset_t)q->c_cf;
	if (q->c_cc < cc)
		cc = q->c_cc;
	if (flag) {
		register char *p, *end;

		p = q->c_cf;
		end = p;
		end += cc;
		while (p < end) {
			if ( isquote(p) ) {
				cc = (vm_offset_t)p;
				cc -= (vm_offset_t)q->c_cf;
				break;
			}
			p++;
		}
	}
out:
	TSPLX(s);
	return (cc);
}

/*
 * Flush cc bytes from q.
 */
ndflush(q, cc)
	register struct clist *q;
	register cc;
{
	register struct cblock *bp;
	char *end;
	int rem;
	TSPLVAR(s)

	TSPLTTY(s);
	if (q->c_cc <= 0)
		goto out;
	while (cc>0 && q->c_cc) {
 		bp = cbptr((vm_offset_t)q->c_cf & ~CROUND);
		if ((vm_offset_t)bp == (((vm_offset_t)q->c_cl-1) & ~CROUND)) {
			end = q->c_cl;
		} else {
			end = (char *)((vm_offset_t)bp + sizeof (struct cblock));
		}
		rem = end - q->c_cf;
		if (cc >= rem) {
			cc -= rem;
			q->c_cc -= rem;
			q->c_cf = bp->c_next->c_info;
			CFREELIST_RETURN(bp);
		} else {
			q->c_cc -= cc;
			q->c_cf += cc;
			if (q->c_cc <= 0) {
				CFREELIST_RETURN(bp);
			}
			break;
		}
	}
	if (q->c_cc <= 0) {
		q->c_cf = q->c_cl = NULL;
		q->c_cc = 0;
	}
out:
	TSPLX(s);
}


putc(c, p)
	long c;
	register struct clist *p;
{
	struct cblock *bp;
	register char *cp;
	TSPLVAR(s)

	TSPLTTY(s);
 	if ((cp = p->c_cl) == NULL || p->c_cc < 0 ) {	/* no cblocks yet */
		CFREELIST_FETCH(&bp);
		if (bp == NULL)
			return -1;
		p->c_cf = cp = bp->c_info;
	} else if (((vm_offset_t)cp & CROUND) == 0) {
 		bp = cbptr(cp) - 1;	/* pointer arith */
		CFREELIST_FETCH(&bp->c_next);
		if (bp->c_next == NULL)
			return -1;
		cp = bp->c_next->c_info;
	}
 	if (c&TTY_QUOTE)
 		setquote(cp);
	*cp++ = c;
	p->c_cc++;
	p->c_cl = cp;
	TSPLX(s);
	return (0);
}

/*
 * copy buffer to clist.
 * return number of bytes not transfered.
 */
b_to_q(cp, cc, q)
	register char *cp;
	struct clist *q;
	register long cc;
{
	register char *cq;
	struct cblock *bp;
	register nc;
	int acc;
	TSPLVAR(s)

	if (cc <= 0)
		return (0);
	acc = cc;
	TSPLTTY(s);
	if ((cq = q->c_cl) == NULL || q->c_cc < 0) {
		CFREELIST_FETCH(&bp);
		if (bp == NULL)
			goto out;
		q->c_cf = cq = bp->c_info;
	}

	while (cc) {
		if (((vm_offset_t)cq & CROUND) == 0) {
 			bp = cbptr(cq) - 1;
			CFREELIST_FETCH(&bp->c_next);
			if (bp->c_next == NULL)
				goto out;
			cq = bp->c_next->c_info;
		}
		nc = MIN(cc, sizeof (struct cblock) - ((vm_offset_t)cq & CROUND));
		(void) bcopy(cp, cq, (unsigned)nc);
		cp += nc;
		cq += nc;
		cc -= nc;
	}
out:
	q->c_cl = cq;
	q->c_cc += acc - cc;
	TSPLX(s);
	return (cc);
}

/*
 * Given a non-NULL pointter into the list (like c_cf which
 * always points to a real character if non-NULL) return the pointer
 * to the next character in the list or return NULL if no more chars.
 *
 * Callers must not allow getc's to happen between nextc's so that the
 * pointer becomes invalid.  Note that interrupts are NOT masked.
 */
char *
nextc(p, cp, c)
	register struct clist *p;
	register char *cp;
 	register int *c;
{

	if (p->c_cc && ++cp != p->c_cl) {
 		if (((vm_offset_t)cp & CROUND) == 0) {
 			cp = (cbptr(cp))[-1].c_next->c_info;
 		}
 		*c = *cp;
 		if (isquote(cp))
 			*c |= TTY_QUOTE;
		return (cp);
	}
	return (0);
}

/*
 * Remove the last character in the list and return it.
 */
unputc(p)
	register struct clist *p;
{
	struct cblock *bp;
	register int c;
	struct cblock *obp;
	register int first = 1;
	TSPLVAR(s)

	TSPLTTY(s);
	if (p->c_cc <= 0)
		c = -1;
	else {
		c = *--p->c_cl;
		if (isquote(p->c_cl))
			c |= TTY_QUOTE;
		if (--p->c_cc <= 0) {
 			bp = cbptr(p->c_cl);
 			bp = cbptr((vm_offset_t)bp & ~CROUND);
			p->c_cl = p->c_cf = NULL;
			/*
			 * Worry:  why aren't we checking cwaiting
			 * in the original code?  The new code will
			 * wake up cblock waiters.
			 */
			CFREELIST_RETURN(bp);
 		} else if (p->c_cl == (cbptr((vm_offset_t)p->c_cl & ~CROUND))->c_info) {
			p->c_cl = (char *)((vm_offset_t)p->c_cl & ~CROUND);
 			bp = cbptr(p->c_cf);
 			bp = cbptr((vm_offset_t)bp & ~CROUND);
 			while (bp->c_next != cbptr(p->c_cl))
				bp = bp->c_next;
			p->c_cl = (char *)(bp + 1);
			CFREELIST_RETURN(bp->c_next);
			bp->c_next = NULL;
		}
	}
	TSPLX(s);
	return (c);
}

/*
 * Put the chars in the from que
 * on the end of the to que.
 */
catq(from, to)
	struct clist *from, *to;
{
#ifdef notdef
	char bbuf[CBSIZE*4];
#endif
	register c;
	TSPLVAR(s)

	TSPLTTY(s);
	if (to->c_cc == 0) {
		*to = *from;
		from->c_cc = 0;
		from->c_cf = NULL;
		from->c_cl = NULL;
		TSPLX(s);
		return;
	}
	TSPLX(s);
#ifdef notdef
	while (from->c_cc > 0) {
		c = q_to_b(from, bbuf, sizeof bbuf);
		(void) b_to_q(bbuf, c, to);
	}
#endif
 	/* XXX - FIX */
 	while ((c = getc(from)) >= 0)
 		putc(c, to);
}

#ifdef	unneeded
/*
 * Integer (short) get/put using clists.
 */
typedef	u_short word_t;

getw(p)
	register struct clist *p;
{
	register int c;
	register struct cblock *bp;
	TSPLVAR(s)

	if (p->c_cc <= 1)
		return(-1);
	if (p->c_cc & 01) {
		c = getc(p);
#if	BYTE_MSF
		return (getc(p) | (c<<8));
#else
		return (c | (getc(p)<<8));
#endif
	}
	TSPLTTY(s);
#if	BYTE_MSF
	c = (((u_char *)p->c_cf)[1] << 8) | ((u_char *)p->c_cf)[0];
#else
 	c = (((u_char *)p->c_cf)[0] << 8) | ((u_char *)p->c_cf)[1];
#endif
	p->c_cf += sizeof (word_t);
	p->c_cc -= sizeof (word_t);
	if (p->c_cc <= 0) {
 		bp = cbptr(p->c_cf-1);
 		bp = cbptr((vm_offset_t)bp & ~CROUND);
		p->c_cf = NULL;
		p->c_cl = NULL;
		bp->c_next = cfreelist;
		cfreelist = bp;
		cfreecount += CBSIZE;
		if (cwaiting) {
			wakeup(&cwaiting);
			cwaiting = 0;
		}
	} else if (((vm_offset_t)p->c_cf & CROUND) == 0) {
 		bp = cbptr(p->c_cf);
		bp--;
		p->c_cf = bp->c_next->c_info;
		bp->c_next = cfreelist;
		cfreelist = bp;
		cfreecount += CBSIZE;
		if (cwaiting) {
			wakeup(&cwaiting);
			cwaiting = 0;
		}
	}
	TSPLX(s);
	return (c);
}

putw(c, p)
	register struct clist *p;
	word_t c;
{
	register struct cblock *bp;
	register char *cp;
	TSPLVAR(s)

	TSPLTTY(s);
	if (cfreelist==NULL) {
		TSPLX(s);
		return(-1);
	}
	if (p->c_cc & 01) {
#if	BYTE_MSF
		(void) putc(c>>8, p);
		(void) putc(c, p);
#else
		(void) putc(c, p);
		(void) putc(c>>8, p);
#endif
	} else {
		if ((cp = p->c_cl) == NULL || p->c_cc < 0 ) {
			if ((bp = cfreelist) == NULL) {
				TSPLX(s);
				return (-1);
			}
			cfreelist = bp->c_next;
			cfreecount -= CBSIZE;
			bp->c_next = NULL;
			p->c_cf = cp = bp->c_info;
		} else if (((vm_offset_t)cp & CROUND) == 0) {
 			bp = cbptr(cp) - 1;
			if ((bp->c_next = cfreelist) == NULL) {
				TSPLX(s);
				return (-1);
			}
			bp = bp->c_next;
			cfreelist = bp->c_next;
			cfreecount -= CBSIZE;
			bp->c_next = NULL;
			cp = bp->c_info;
		}
#if	BYTE_MSF
		((u_char *)cp)[0] = c>>8;
		((u_char *)cp)[1] = c;
#else
		*(word_t *)cp = c;
#endif
		p->c_cl = cp + sizeof (word_t);
		p->c_cc += sizeof (word_t);
	}
	TSPLX(s);
	return (0);
}
#endif	/* unneeded */
