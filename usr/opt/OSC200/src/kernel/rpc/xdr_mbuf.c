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
static char *rcsid = "@(#)$RCSfile: xdr_mbuf.c,v $ $Revision: 1.1.3.7 $ (DEC) $Date: 1992/12/13 15:01:02 $";
#endif
#ifdef	KERNEL
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)xdr_mbuf.c	1.4 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.22 88/02/08 
 */


/*
 * xdr_mbuf.c, XDR implementation on kernel mbufs.
 */

#include <rpc/rpc.h>
#include <sys/mbuf.h>
#include <sys/uio.h>

bool_t	xdrmbuf_getlong(), xdrmbuf_putlong();
bool_t	xdrmbuf_getbytes(), xdrmbuf_putbytes();
u_int	xdrmbuf_getpos();
bool_t	xdrmbuf_setpos();
int *	xdrmbuf_inline();
void	xdrmbuf_destroy();

/*
 * Xdr on mbufs operations vector.
 */
struct	xdr_ops xdrmbuf_ops = {
	xdrmbuf_getlong,
	xdrmbuf_putlong,
	xdrmbuf_getbytes,
	xdrmbuf_putbytes,
	xdrmbuf_getpos,
	xdrmbuf_setpos,
	xdrmbuf_inline,
	xdrmbuf_destroy
};

/*
 * Initailize xdr stream.
 */
void
xdrmbuf_init(xdrs, m, op)
	register XDR		*xdrs;
	register struct mbuf	*m;
	enum xdr_op		 op;
{

	xdrs->x_op = op;
	xdrs->x_ops = &xdrmbuf_ops;
	xdrs->x_base = (caddr_t)m;
	xdrs->x_private = mtod(m, caddr_t);
	xdrs->x_public = (caddr_t)0;
	xdrs->x_handy = m->m_len;
}

void
/* ARGSUSED */
xdrmbuf_destroy(xdrs)
	XDR	*xdrs;
{
	/* do nothing */
}

bool_t
xdrmbuf_getlong(xdrs, lp)
	register XDR	*xdrs;
	int		*lp;
{

	if ((xdrs->x_handy -= sizeof(int)) < 0) {
		if (xdrs->x_handy != -sizeof(int))
			printf("xdr_mbuf: int crosses mbufs!\n");
		if (xdrs->x_base) {
			register struct mbuf *m =
			    ((struct mbuf *)xdrs->x_base)->m_next;

			while (m && m->m_len == 0)
				m = m->m_next;
			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = mtod(m, caddr_t);
			xdrs->x_handy = m->m_len - sizeof(int);
		} else {
			return (FALSE);
		}
	}
	*lp = ntohl(*((int *)(xdrs->x_private)));
	xdrs->x_private += sizeof(int);
	return (TRUE);
}

bool_t
xdrmbuf_putlong(xdrs, lp)
	register XDR	*xdrs;
	int		*lp;
{

	if ((xdrs->x_handy -= sizeof(int)) < 0) {
		if (xdrs->x_handy != -sizeof(int))
			printf("xdr_mbuf: putlong, int crosses mbufs!\n");
		if (xdrs->x_base) {
			register struct mbuf *m =
			    ((struct mbuf *)xdrs->x_base)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = mtod(m, caddr_t);
			xdrs->x_handy = m->m_len - sizeof(int);
		} else {
			return (FALSE);
		}
	}
	*(int *)xdrs->x_private = htonl(*lp);
	xdrs->x_private += sizeof(int);
	return (TRUE);
}

bool_t
xdrmbuf_getbytes(xdrs, addr, len)
	register XDR	*xdrs;
	caddr_t		 addr;
	register u_int	 len;
{

	while ((xdrs->x_handy -= len) < 0) {
		if ((xdrs->x_handy += len) > 0) {
			bcopy(xdrs->x_private, addr, (u_int)xdrs->x_handy);
			addr += xdrs->x_handy;
			len -= xdrs->x_handy;
		}
		if (xdrs->x_base) {
			register struct mbuf *m =
			    ((struct mbuf *)xdrs->x_base)->m_next;

			while (m && m->m_len == 0)
				m = m->m_next;
			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = mtod(m, caddr_t);
			xdrs->x_handy = m->m_len;
		} else {
			return (FALSE);
		}
	}
	bcopy(xdrs->x_private, addr, (u_int)len);
	xdrs->x_private += len;
	return (TRUE);
}

/*
 * Sort of like getbytes except that instead of getting
 * bytes we return the mbuf that contains all the rest
 * of the bytes.
 */
bool_t
xdrmbuf_getmbuf(xdrs, mm, lenp)
	register XDR	*xdrs;
	register struct mbuf **mm;
	register u_int *lenp;
{
	register struct mbuf *m;
	register int len, used;

	if (! xdr_u_int(xdrs, lenp)) {
		return (FALSE);
	}
	m = (struct mbuf *)xdrs->x_base;
	used = m->m_len - xdrs->x_handy;
	m->m_data += used;
	m->m_len -= used;
	*mm = m;
	/*
	 * Consistency check.
	 */
	len = 0;
	while (m) {
		len += m->m_len;
		m = m->m_next;
	}
	if (len < *lenp) {
#ifdef KERNEL
		printf("xdrmbuf_getmbuf failed\n");
#endif
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdrmbuf_putbytes(xdrs, addr, len)
	register XDR	*xdrs;
	caddr_t		 addr;
	register u_int		 len;
{

	while ((xdrs->x_handy -= len) < 0) {
		if ((xdrs->x_handy += len) > 0) {
			bcopy(addr, xdrs->x_private, (u_int)xdrs->x_handy);
			addr += xdrs->x_handy;
			len -= xdrs->x_handy;
		}
		if (xdrs->x_base) {
			register struct mbuf *m =
			    ((struct mbuf *)xdrs->x_base)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_private = mtod(m, caddr_t);
			xdrs->x_handy = m->m_len;
		} else {
			return (FALSE);
		}
	}
	bcopy(addr, xdrs->x_private, len);
	xdrs->x_private += len;
	return (TRUE);
}

/*
 * Like putbytes, only we avoid the copy by pointing a type 2
 * mbuf at the buffer.  Not safe if the buffer goes away before
 * the mbuf chain is deallocated.
 */
#ifdef	DEBUG
extern int rrokdebug;
#define DPRINTF(c, args)	if(c) printf args
#else
#define DPRINTF(c, args)
#endif

bool_t
xdrmbuf_putbuf(xdrs, addr, len, func, arg)
	register XDR	*xdrs;
	caddr_t		 addr;
	u_int		 len;
	int		 (*func)();
	int		 arg;
{
	register struct mbuf *m;
	struct mbuf *mclgetx();
	int llen = len;

	DPRINTF(rrokdebug,
		("putbuf0: len %d base 0x%x %d m_next 0x%x handy %d\n",
		len, (struct mbuf *)xdrs->x_base,
		((struct mbuf *)xdrs->x_base)->m_len,
		((struct mbuf *)xdrs->x_base)->m_next,
		xdrs->x_handy));

	if (len & 3) {  /* can't handle roundup problems */
		return (FALSE);
	}
	if (! xdrmbuf_putlong(xdrs, &llen)) {
		return(FALSE);
	}
	((struct mbuf *)xdrs->x_base)->m_len -= xdrs->x_handy;

	DPRINTF(rrokdebug,
		("putbuf1: len %d base 0x%x %d m_next 0x%x\n",
		len, (struct mbuf *)xdrs->x_base,
		((struct mbuf *)xdrs->x_base)->m_len,
		((struct mbuf *)xdrs->x_base)->m_next));

	DPRINTF(rrokdebug,
		("putbuf: func 0x%x arg 0x%x\n",
		func, arg));

	m = mclgetx(func, arg, addr, (int)len, M_WAIT);
	if (m == NULL) {
		printf("xdrmbuf_putbuf: mclgetx failed\n");
		return (FALSE);
	}
	((struct mbuf *)xdrs->x_base)->m_next = m;
	DPRINTF(rrokdebug,
		("putbuf2: base 0x%x %d m_next 0x%x %d\n",
		(struct mbuf *)xdrs->x_base,
		((struct mbuf *)xdrs->x_base)->m_len, m,
		m->m_len));
	xdrs->x_handy = 0;
	return (TRUE);
}

u_int
xdrmbuf_getpos(xdrs)
	register XDR	*xdrs;
{

	return (
	    (u_int)xdrs->x_private - mtod(((struct mbuf *)xdrs->x_base), u_int));
}

bool_t
xdrmbuf_setpos(xdrs, pos)
	register XDR	*xdrs;
	u_int		 pos;
{
	register caddr_t	newaddr =
	    mtod(((struct mbuf *)xdrs->x_base), caddr_t) + pos;
	register caddr_t	lastaddr =
	    xdrs->x_private + xdrs->x_handy;

	if ((int)newaddr > (int)lastaddr)
		return (FALSE);
	xdrs->x_private = newaddr;
	xdrs->x_handy = (int)lastaddr - (int)newaddr;
	return (TRUE);
}

int *
xdrmbuf_inline(xdrs, len)
	register XDR	*xdrs;
	int		 len;
{
	int *buf = 0;

	if (xdrs->x_handy >= len) {
		xdrs->x_handy -= len;
		buf = (int *) xdrs->x_private;
		xdrs->x_private += len;
	}
	return (buf);
}
#endif /* KERNEL */
