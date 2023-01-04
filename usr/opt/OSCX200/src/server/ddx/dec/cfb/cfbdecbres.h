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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbdecbres.h,v 1.1.2.2 92/01/07 12:48:08 Jim_Ludwig Exp $ */

#ifndef __cfbdecbres__

/* Solid horizontal line painters */
extern void cfbHorzSCopy();
extern void cfbHorzSXor();
extern void cfbHorzSGeneral();

/* Solid vertical line painters */
extern void cfbVertSCopy();
extern void cfbVertSXor();
extern void cfbVertSGeneral();

/* Solid sloped line painters */
extern void cfbBresSCopy();
extern void cfbBresSXor();
extern void cfbBresSGeneral();

/* OnOff dashed line painters */
extern void cfbOODashCopy();
extern void cfbOODashXor();
extern void cfbOODashGeneral();

/* Double dashed line painters */
extern void cfbDDashCopy();
extern void cfbDDashXor();
extern void cfbDDashGeneral();

#endif

/* Macros for painting sloped lines */

#define CPH()					\
{						\
    e += e1;					\
    if (e >= 0) {				\
	addr += snlwidth;			\
	e += e2;				\
    }						\
} /* CPH */

#define CPHW(offset)				\
{						\
    CFBFILL(addr+offset, fgandbits, fgxorbits); \
    CPH();					\
} /* CPHW */

#define CNH()					\
{						\
    e += e1;					\
    if (e > 0) {				\
	addr += snlwidth;			\
	e += e2;				\
    }						\
} /* CNH */

#define CNHW(offset)				\
{						\
    CFBFILL(addr+offset, fgandbits, fgxorbits); \
    CNH();					\
} /* CNHW */

#define CV()					\
{						\
    addr += snlwidth;				\
    e += e1;					\
    if (e > 0) {				\
	addr += signdx;				\
	e += e2;				\
    }						\
}

#define CVW()					\
{						\
    CFBFILL(addr, fgandbits, fgxorbits);	\
    CV();					\
} /* CVW */

