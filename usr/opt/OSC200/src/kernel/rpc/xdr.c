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
static char *rcsid = "@(#)$RCSfile: xdr.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/18 21:07:25 $";
#endif
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)xdr.c	1.5 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.36 88/02/08
 */


/*
 * xdr.c, Generic XDR routines implementation.
 *
 * These are the "generic" xdr routines used to serialize and de-serialize
 * most common data items.  See xdr.h for more info on the interface to
 * xdr.
 */

#ifdef KERNEL
#include <rpc/rpc.h>
#else
#include <sys/syslog.h>
#include <stdio.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#endif


/*
 * constants specific to the xdr "protocol"
 */
#define XDR_FALSE	((int) 0)
#define XDR_TRUE	((int) 1)
#define LASTUNSIGNED	((u_int) 0-1)

/*
 * for unit alignment
 */
static char xdr_zero[BYTES_PER_XDR_UNIT] = { 0, 0, 0, 0 };

#ifndef KERNEL
/*
 * Free a data structure using XDR
 * Not a filter, but a convenient utility nonetheless
 */
void
xdr_free(proc, objp)
	xdrproc_t proc;
	char *objp;
{
	XDR x;
	
	x.x_op = XDR_FREE;
	(*proc)(&x, objp);
}
#endif

/*
 * XDR nothing
 */
bool_t
xdr_void(/* xdrs, addr */)
	/* XDR *xdrs; */
	/* caddr_t addr; */
{

	return (TRUE);
}

/*
 * XDR integers
 */
bool_t
xdr_int(xdrs, ip)
	XDR *xdrs;
	int *ip;
{
        if (xdrs->x_op == XDR_ENCODE)
                return (XDR_PUTLONG(xdrs, ip));

        if (xdrs->x_op == XDR_DECODE)
                return (XDR_GETLONG(xdrs, ip));

        if (xdrs->x_op == XDR_FREE)
                return (TRUE);

#ifdef KERNEL
        printf("xdr_int: FAILED\n");
#endif
        return (FALSE);
}

/*
 * XDR unsigned integers
 */
bool_t
xdr_u_int(xdrs, up)
	XDR *xdrs;
	u_int *up;
{
        if (xdrs->x_op == XDR_DECODE)
                return (XDR_GETLONG(xdrs, (u_int *)up));
        if (xdrs->x_op == XDR_ENCODE)
                return (XDR_PUTLONG(xdrs, (u_int *)up));
        if (xdrs->x_op == XDR_FREE)
                return (TRUE);
#ifdef KERNEL
        printf("xdr_u_int: FAILED\n");
#endif
        return (FALSE);
}

/*
 * XDR 64 bit longs
 * same as xdr_u_long - open coded to save a proc call!
 */
bool_t
xdr_long(xdrs, lp)
	register XDR *xdrs;
	long *lp;
{
        if (xdrs->x_op == XDR_ENCODE) {
		/* check that we are encoding a 32 bit value */
		int *iptr = (int *)lp;

		++iptr;
		if (*iptr == 0 || *iptr == 0xffffffff)
			return (XDR_PUTLONG(xdrs, (int *)lp));
		else
			return (FALSE);
	}

        if (xdrs->x_op == XDR_DECODE)
                if (XDR_GETLONG(xdrs, (int *)lp)) {
			/* need to sign extend the 32 bit xdr_int */
			int *iptr = (int *)lp;

			if (*iptr >= 0)
				*++iptr = 0;
			else
				*++iptr = 0xffffffff;
			return (TRUE);
		} else
			return (FALSE);

        if (xdrs->x_op == XDR_FREE)
                return (TRUE);

#ifdef KERNEL
        printf("xdr_long: FAILED\n");
#endif
        return (FALSE);
}

/*
 * XDR 64 bit longs
 * same as xdr_long - open coded to save a proc call!
 */
bool_t
xdr_u_long(xdrs, ulp)
	register XDR *xdrs;
	u_long *ulp;
{
        if (xdrs->x_op == XDR_ENCODE) {
               /* check that we are encoding a 32 bit value */
                u_int *iptr = (u_int *)ulp;

                if (*++iptr == 0)
			return (XDR_PUTLONG(xdrs, (u_int *)ulp));
		else
			return (FALSE);
	}

        if (xdrs->x_op == XDR_DECODE)
                if (XDR_GETLONG(xdrs, (u_int *)ulp)) {
			u_int *iptr = (u_int *)ulp;

			*++iptr = 0;
			return (TRUE);
		} else
			return (FALSE);

        if (xdrs->x_op == XDR_FREE)
                return (TRUE);

#ifdef KERNEL
        printf("xdr_u_long: FAILED\n");
#endif
        return (FALSE);
}

/*
 * XDR short integers
 */
bool_t
xdr_short(xdrs, sp)
	register XDR *xdrs;
	short *sp;
{
	int l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = (int) *sp;
		return (XDR_PUTLONG(xdrs, &l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
			return (FALSE);
		}
		*sp = (short) l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
	return (FALSE);
}

/*
 * XDR unsigned short integers
 */
bool_t
xdr_u_short(xdrs, usp)
	register XDR *xdrs;
	u_short *usp;
{
	u_int l;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		l = (u_int) *usp;
		return (XDR_PUTLONG(xdrs, &l));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &l)) {
#ifdef KERNEL
			printf("xdr_u_short: decode FAILED\n");
#endif
			return (FALSE);
		}
		*usp = (u_short) l;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
#ifdef KERNEL
	printf("xdr_u_short: bad op FAILED\n");
#endif
	return (FALSE);
}


/*
 * XDR a char
 */
bool_t
xdr_char(xdrs, cp)
	XDR *xdrs;
	char *cp;
{
	int i;

	i = (*cp);
	if (!xdr_int(xdrs, &i)) {
		return (FALSE);
	}
	*cp = i;
	return (TRUE);
}

#ifndef KERNEL
/*
 * XDR an unsigned char
 */
bool_t
xdr_u_char(xdrs, cp)
	XDR *xdrs;
	char *cp;
{
	u_int u;

	u = (*cp);
	if (!xdr_u_int(xdrs, &u)) {
		return (FALSE);
	}
	*cp = u;
	return (TRUE);
}
#endif /* !KERNEL */

/*
 * XDR booleans
 */
bool_t
xdr_bool(xdrs, bp)
	register XDR *xdrs;
	bool_t *bp;
{
	int lb;

	switch (xdrs->x_op) {

	case XDR_ENCODE:
		lb = *bp ? XDR_TRUE : XDR_FALSE;
		return (XDR_PUTLONG(xdrs, &lb));

	case XDR_DECODE:
		if (!XDR_GETLONG(xdrs, &lb)) {
#ifdef KERNEL
			printf("xdr_bool: decode FAILED\n");
#endif
			return (FALSE);
		}
		*bp = (lb == XDR_FALSE) ? FALSE : TRUE;
		return (TRUE);

	case XDR_FREE:
		return (TRUE);
	}
#ifdef KERNEL
	printf("xdr_bool: bad op FAILED\n");
#endif
	return (FALSE);
}

/*
 * XDR enumerations
 */
bool_t
xdr_enum(xdrs, ep)
	XDR *xdrs;
	enum_t *ep;
{
#ifndef lint
	enum sizecheck { SIZEVAL };	/* used to find the size of an enum */

	/*
	 * enums are treated as ints
	 */
	if (sizeof (enum sizecheck) == sizeof (int)) {
		return (xdr_int(xdrs, (int *)ep));
	} else if (sizeof (enum sizecheck) == sizeof (short)) {
		return (xdr_short(xdrs, (short *)ep));
	} else {
		return (FALSE);
	}
#else
	(void) (xdr_short(xdrs, (short *)ep));
	return (xdr_int(xdrs, (int *)ep));
#endif
}

/*
 * XDR opaque data
 * Allows the specification of a fixed size sequence of opaque bytes.
 * cp points to the opaque object and cnt gives the byte length.
 */
bool_t
xdr_opaque(xdrs, cp, cnt)
	register XDR *xdrs;
	caddr_t cp;
	register u_int cnt;
{
	register u_int rndup;
	static crud[BYTES_PER_XDR_UNIT];

	/*
	 * if no data we are done
	 */
	if (cnt == 0)
		return (TRUE);

	/*
	 * round byte count to full xdr units
	 */
	rndup = cnt % BYTES_PER_XDR_UNIT;
	if (rndup != 0)
		rndup = BYTES_PER_XDR_UNIT - rndup;

	if (xdrs->x_op == XDR_DECODE) {
		if (!XDR_GETBYTES(xdrs, cp, cnt)) {
#ifdef KERNEL
			printf("xdr_opaque: decode FAILED\n");
#endif
			return (FALSE);
		}
		if (rndup == 0)
			return (TRUE);
		return (XDR_GETBYTES(xdrs, crud, rndup));
	}

	if (xdrs->x_op == XDR_ENCODE) {
		if (!XDR_PUTBYTES(xdrs, cp, cnt)) {
#ifdef KERNEL
			printf("xdr_opaque: encode FAILED\n");
#endif
			return (FALSE);
		}
		if (rndup == 0)
			return (TRUE);
		return (XDR_PUTBYTES(xdrs, xdr_zero, rndup));
	}

	if (xdrs->x_op == XDR_FREE) {
		return (TRUE);
	}

#ifdef KERNEL
	printf("xdr_opaque: bad op FAILED\n");
#endif
	return (FALSE);
}

/*
 * XDR counted bytes
 * *cpp is a pointer to the bytes, *sizep is the count.
 * If *cpp is NULL maxsize bytes are allocated
 */
bool_t
xdr_bytes(xdrs, cpp, sizep, maxsize)
	register XDR *xdrs;
	char **cpp;
	register u_int *sizep;
	u_int maxsize;
{
	register char *sp = *cpp;  /* sp is the actual string pointer */
	register u_int nodesize;

	/*
	 * first deal with the length since xdr bytes are counted
	 */
	if (! xdr_u_int(xdrs, sizep)) {
#ifdef KERNEL
		printf("xdr_bytes: size FAILED\n");
#endif
		return (FALSE);
	}
	nodesize = *sizep;
	if ((nodesize > maxsize) && (xdrs->x_op != XDR_FREE)) {
#ifdef KERNEL
		printf("xdr_bytes: bad size FAILED\n");
#endif
		return (FALSE);
	}

	/*
	 * now deal with the actual bytes
	 */
	switch (xdrs->x_op) {

	case XDR_DECODE:
		if (nodesize == 0) {
			return (TRUE);
		}
		if (sp == NULL) {
			*cpp = sp = (char *)mem_alloc(nodesize);
		}
#ifndef KERNEL
		if (sp == NULL) {
			(void) syslog(LOG_ERR, "xdr_bytes: out of memory");
			return (FALSE);
		}
#endif
		/* fall into ... */

	case XDR_ENCODE:
		return (xdr_opaque(xdrs, sp, nodesize));

	case XDR_FREE:
		if (sp != NULL) {
			mem_free(sp, nodesize);
			*cpp = NULL;
		}
		return (TRUE);
	}
#ifdef KERNEL
	printf("xdr_bytes: bad op FAILED\n");
#endif
	return (FALSE);
}

/*
 * Implemented here due to commonality of the object.
 */
bool_t
xdr_netobj(xdrs, np)
	XDR *xdrs;
	struct netobj *np;
{

	return (xdr_bytes(xdrs, &np->n_bytes, &np->n_len, MAX_NETOBJ_SZ));
}

/*
 * XDR a descriminated union
 * Support routine for discriminated unions.
 * You create an array of xdrdiscrim structures, terminated with
 * an entry with a null procedure pointer.  The routine gets
 * the discriminant value and then searches the array of xdrdiscrims
 * looking for that value.  It calls the procedure given in the xdrdiscrim
 * to handle the discriminant.  If there is no specific routine a default
 * routine may be called.
 * If there is no specific or default routine an error is returned.
 */
bool_t
xdr_union(xdrs, dscmp, unp, choices, dfault)
	register XDR *xdrs;
	enum_t *dscmp;		/* enum to decide which arm to work on */
	char *unp;		/* the union itself */
	struct xdr_discrim *choices;	/* [value, xdr proc] for each arm */
	xdrproc_t dfault;	/* default xdr routine */
{
	register enum_t dscm;

	/*
	 * we deal with the discriminator;  it's an enum
	 */
	if (! xdr_enum(xdrs, dscmp)) {
#ifdef KERNEL
		printf("xdr_enum: dscmp FAILED\n");
#endif
		return (FALSE);
	}
	dscm = *dscmp;

	/*
	 * search choices for a value that matches the discriminator.
	 * if we find one, execute the xdr routine for that value.
	 */
	for (; choices->proc != NULL_xdrproc_t; choices++) {
		if (choices->value == dscm)
			return ((*(choices->proc))(xdrs, unp, LASTUNSIGNED));
	}

	/*
	 * no match - execute the default xdr routine if there is one
	 */
	return ((dfault == NULL_xdrproc_t) ? FALSE :
	    (*dfault)(xdrs, unp, LASTUNSIGNED));
}


/*
 * Non-portable xdr primitives.
 * Care should be taken when moving these routines to new architectures.
 */


/*
 * XDR null terminated ASCII strings
 * xdr_string deals with "C strings" - arrays of bytes that are
 * terminated by a NULL character.  The parameter cpp references a
 * pointer to storage; If the pointer is null, then the necessary
 * storage is allocated.  The last parameter is the max allowed length
 * of the string as specified by a protocol.
 */
bool_t
xdr_string(xdrs, cpp, maxsize)
	register XDR *xdrs;
	char **cpp;
	u_int maxsize;
{
	register char *sp = *cpp;  /* sp is the actual string pointer */
	u_int size;
	u_int nodesize;

	/*
	 * first deal with the length since xdr strings are counted-strings
	 */
	switch (xdrs->x_op) {
	case XDR_FREE:
		if (sp == NULL) {
			return(TRUE);	/* already free */
		}
		/* fall through... */
	case XDR_ENCODE:
		size = strlen(sp);
		break;
	}
	if (! xdr_u_int(xdrs, &size)) {
#ifdef KERNEL
		printf("xdr_string: size FAILED\n");
#endif
		return (FALSE);
	}
	if (size > maxsize) {
#ifdef KERNEL
		printf("xdr_string: bad size FAILED\n");
#endif
		return (FALSE);
	}
	nodesize = size + 1;

	/*
	 * now deal with the actual bytes
	 */
	switch (xdrs->x_op) {

	case XDR_DECODE:
		if (nodesize == 0) {
			return (TRUE);
		}
		if (sp == NULL)
			*cpp = sp = (char *)mem_alloc(nodesize);
#ifndef KERNEL
		if (sp == NULL) {
			(void) syslog(LOG_ERR, "xdr_string: out of memory");
			return (FALSE);
		}
#endif
		sp[size] = 0;
		/* fall into ... */

	case XDR_ENCODE:
		return (xdr_opaque(xdrs, sp, size));

	case XDR_FREE:
		mem_free(sp, nodesize);
		*cpp = NULL;
		return (TRUE);
	}
#ifdef KERNEL
	printf("xdr_string: bad op FAILED\n");
#endif
	return (FALSE);
}

/*
 * XDR hyper (8-bytes, 64-bit)
 */
bool_t
xdr_hyper(xdrs, hp)
	register XDR *xdrs;
	longlong_t *hp;
{
	/*
	 * This handles 64 bit items as 2 32-bit items by calling GET/PUTLONG
	 * twice.  A potential problem could result if there were only 32-bits
	 * of space remaining in the stream buffer.  In that case the first
	 * PUT could succeed and the second one could fail; leaving 32-bits
	 * of data stuck in the stream.
	 */
	if (xdrs->x_op == XDR_ENCODE) {
#ifdef sun
		if (XDR_PUTLONG(xdrs, (long *) hp) == TRUE)
			return (XDR_PUTLONG(xdrs,
					    (long *)((char *) hp +
						     BYTES_PER_XDR_UNIT)));

#else
		if (XDR_PUTLONG(xdrs, (int *)((char *) hp +
					       BYTES_PER_XDR_UNIT)) == TRUE)
			return (XDR_PUTLONG(xdrs, (int *) hp));

#endif
		return (FALSE);

	} else if (xdrs->x_op == XDR_DECODE) {
#ifdef sun
		if (XDR_GETLONG(xdrs, (long *)hp) == TRUE)
			return (XDR_GETLONG(xdrs,
					    (long *)((char *) hp +
						     BYTES_PER_XDR_UNIT)));
#else
		if (XDR_GETLONG(xdrs, (int *)((char *) hp +
					       BYTES_PER_XDR_UNIT)) == TRUE)
			return (XDR_GETLONG(xdrs, (int *) hp));
#endif
		return (FALSE);
	}
	return (TRUE);
}

/*
 * XDR unsigned hyper (8-bytes, 64-bit)
 * - Rather than calling xdr_hyper here it may be beneficial to duplicate the
 *   code here to avoid the procedure call as is done for xdr_u_long.
 * - Since it is unclear which will be more commonly used (xdr_hyper vs
 *   xdr_longlong, they are both included here with xdr_hyper avoiding the
 *   additional procedure call of a wrapper routine.
 */
bool_t
xdr_u_hyper(xdrs, uhp)
	register XDR *xdrs;
	u_longlong_t *uhp;
{
	return (xdr_hyper(xdrs, (longlong_t *)uhp));
}

/*
 * The routines xdr_longlong_t and xdr_u_longlong_t are called 
 * from routines built by rpcgen to xdr the type "hyper".  This is 
 * a necessary evil of the translation problem because you can't convert
 * an xdr call for a hyper to xdr_long because that only deals with 32 bits.
 * These 2 routines just end up calling xdr_hyper to do the work.
 */
bool_t
xdr_longlong_t(xdrs, hp)
	register XDR *xdrs;
	longlong_t *hp;
{
	return(xdr_hyper(xdrs, hp));
}

bool_t
xdr_u_longlong_t(xdrs, uhp)
	register XDR *xdrs;
	u_longlong_t *uhp;
{
	return(xdr_u_hyper(xdrs, (u_longlong_t *)uhp));
}

#ifndef KERNEL
/* 
 * Wrapper for xdr_string that can be called directly from 
 * routines like clnt_call
 */
bool_t
xdr_wrapstring(xdrs, cpp)
	XDR *xdrs;
	char **cpp;
{
	if (xdr_string(xdrs, cpp, LASTUNSIGNED)) {
		return (TRUE);
	}
	return (FALSE);
}
#endif /* !KERNEL */
