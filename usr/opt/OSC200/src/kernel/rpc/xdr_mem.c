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
static char *rcsid = "@(#)$RCSfile: xdr_mem.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/03/31 15:09:42 $";
#endif
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)xdr_mem.c	1.4 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.20 88/02/08 
 */


/*
 * xdr_mem.h, XDR implementation using memory buffers.
 *
 * If you have some data to be interpreted as external data representation
 * or to be converted to external data representation in a memory buffer,
 * then this is the package for you.
 *
 */

#ifdef KERNEL
#include <rpc/rpc.h>
#else
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <netinet/in.h>
#endif


static bool_t	xdrmem_getlong();
static bool_t	xdrmem_putlong();
static bool_t	xdrmem_getbytes();
static bool_t	xdrmem_putbytes();
static u_int	xdrmem_getpos();
static bool_t	xdrmem_setpos();
static int *	xdrmem_inline();
static void	xdrmem_destroy();

static struct	xdr_ops xdrmem_ops = {
	xdrmem_getlong,
	xdrmem_putlong,
	xdrmem_getbytes,
	xdrmem_putbytes,
	xdrmem_getpos,
	xdrmem_setpos,
	xdrmem_inline,
	xdrmem_destroy
};

/*
 * The procedure xdrmem_create initializes a stream descriptor for a
 * memory buffer.  
 */
void
xdrmem_create(xdrs, addr, size, op)
	register XDR *xdrs;
	caddr_t addr;
	u_int size;
	enum xdr_op op;
{

	xdrs->x_op = op;
	xdrs->x_ops = &xdrmem_ops;
	xdrs->x_private = xdrs->x_base = addr;
	xdrs->x_handy = size;
}

static void
xdrmem_destroy(/*xdrs*/)
	/*XDR *xdrs;*/
{
}

static bool_t
xdrmem_getlong(xdrs, lp)
	register XDR *xdrs;
	int *lp;
{

	if ((xdrs->x_handy -= sizeof(int)) < 0)
		return (FALSE);
	*lp = (int)ntohl((u_int)(*((int *)(xdrs->x_private))));
	xdrs->x_private += sizeof(int);
	return (TRUE);
}

static bool_t
xdrmem_putlong(xdrs, lp)
	register XDR *xdrs;
	int *lp;
{

	if ((xdrs->x_handy -= sizeof(int)) < 0)
		return (FALSE);
	*(int *)xdrs->x_private = (int)htonl((u_int)(*lp));
	xdrs->x_private += sizeof(int);
	return (TRUE);
}

static bool_t
xdrmem_getbytes(xdrs, addr, len)
	register XDR *xdrs;
	caddr_t addr;
	register u_int len;
{

	if ((xdrs->x_handy -= len) < 0)
		return (FALSE);
	bcopy(xdrs->x_private, addr, len);
	xdrs->x_private += len;
	return (TRUE);
}

static bool_t
xdrmem_putbytes(xdrs, addr, len)
	register XDR *xdrs;
	caddr_t addr;
	register u_int len;
{

	if ((xdrs->x_handy -= len) < 0)
		return (FALSE);
	bcopy(addr, xdrs->x_private, len);
	xdrs->x_private += len;
	return (TRUE);
}

static u_int
xdrmem_getpos(xdrs)
	register XDR *xdrs;
{

	return ((u_int)xdrs->x_private - (u_int)xdrs->x_base);
}

static bool_t
xdrmem_setpos(xdrs, pos)
	register XDR *xdrs;
	u_int pos;
{
	register caddr_t newaddr = xdrs->x_base + pos;
	register caddr_t lastaddr = xdrs->x_private + xdrs->x_handy;

	if ((int)newaddr > (int)lastaddr)
		return (FALSE);
	xdrs->x_private = newaddr;
	xdrs->x_handy = (int)lastaddr - (int)newaddr;
	return (TRUE);
}

static int *
xdrmem_inline(xdrs, len)
	register XDR *xdrs;
	int len;
{
	int *buf = 0;

	if (xdrs->x_handy >= len) {
		xdrs->x_handy -= len;
		buf = (int *) xdrs->x_private;
		xdrs->x_private += len;
	}
	return (buf);
}
