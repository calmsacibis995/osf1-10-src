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
static char	*sccsid = "@(#)$RCSfile: fcs.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:38:58 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBICONV)
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.6  com/lib/iconv/fcs.c, libiconv, bos320, 9142320c 10/8/91 14:17:09
 */

#include <stdlib.h>
#include "iconv_local.h"
#include "iconvP.h"
#include "iconv932.h"
#include "fcs.h"

static char	iconv_ASCII[] = { 0x1b, 0x28, 0x42 };
static char	iconv_ASCII_GR[] = { 0x1b, 0x29, 0x42 };
static char	iconv_ISO8859_1_GL[] = { 0x1b, 0x2e, 0x41 };
static char	iconv_ISO8859_1_GR[] = { 0x1b, 0x2d, 0x41 };
static char	iconv_ISO8859_2_GL[] = { 0x1b, 0x2e, 0x42 };
static char	iconv_ISO8859_2_GR[] = { 0x1b, 0x2d, 0x42 };
static char	iconv_ISO8859_3_GL[] = { 0x1b, 0x2e, 0x43 };
static char	iconv_ISO8859_3_GR[] = { 0x1b, 0x2d, 0x43 };
static char	iconv_ISO8859_4_GL[] = { 0x1b, 0x2e, 0x44 };
static char	iconv_ISO8859_4_GR[] = { 0x1b, 0x2d, 0x44 };
static char	iconv_ISO8859_5_GL[] = { 0x1b, 0x2e, 0x4c };
static char	iconv_ISO8859_5_GR[] = { 0x1b, 0x2d, 0x4c };
static char	iconv_ISO8859_6_GL[] = { 0x1b, 0x2e, 0x47 };
static char	iconv_ISO8859_6_GR[] = { 0x1b, 0x2d, 0x47 };
static char	iconv_ISO8859_7_GL[] = { 0x1b, 0x2e, 0x46 };
static char	iconv_ISO8859_7_GR[] = { 0x1b, 0x2d, 0x46 };
static char	iconv_ISO8859_8_GL[] = { 0x1b, 0x2e, 0x48 };
static char	iconv_ISO8859_8_GR[] = { 0x1b, 0x2d, 0x48 };
static char	iconv_ISO8859_9_GL[] = { 0x1b, 0x2e, 0x4d };
static char	iconv_ISO8859_9_GR[] = { 0x1b, 0x2d, 0x4d };
static char	iconv_JISX0201_1976[] = { 0x1b, 0x28, 0x4a };
static char	iconv_JISX0201_1976_GL[] = { 0x1b, 0x28, 0x49 };
static char	iconv_JISX0201_1976_GR[] = { 0x1b, 0x29, 0x49 };
static char	iconv_JISX0208_1978_GL[] = { 0x1b, 0x24, 0x28, 0x40 };
static char	iconv_JISX0208_1978_GL_MAIL[] = { 0x1b, 0x24, 0x40 };
static char	iconv_JISX0208_1978_GR[] = { 0x1b, 0x24, 0x29, 0x40 };
static char	iconv_JISX0208_1983_GL[] = { 0x1b, 0x24, 0x28, 0x42 };
static char	iconv_JISX0208_1983_GL_MAIL[] = { 0x1b, 0x24, 0x42 };
static char	iconv_JISX0208_1983_GR[] = { 0x1b, 0x24, 0x29, 0x42 };

/*
 *	Macro Definitions for "Escape Sequence Vector"
 */

#define	ICONV_ASCII		{ NULL, &iconv_ASCII[0], sizeof (iconv_ASCII), \
					NULL, 0, True }
#define	ICONV_ASCII_GR		{ "ASCII-GR", \
					&iconv_ASCII_GR[0], \
					sizeof (iconv_ASCII_GR), \
					NULL, 0, False }
#define	ICONV_ISO8859_1_GL	{ "ISO8859-1-GL", \
					&iconv_ISO8859_1_GL[0], \
					sizeof (iconv_ISO8859_1_GL), \
					NULL, 0, True }
#define	ICONV_ISO8859_1_GR	{ "ISO8859-1-GR", \
					&iconv_ISO8859_1_GR[0], \
					sizeof (iconv_ISO8859_1_GR), \
					NULL, 0, False }
#define	ICONV_ISO8859_2_GL	{ "ISO8859-2-GL", \
					&iconv_ISO8859_2_GL[0], \
					sizeof (iconv_ISO8859_2_GL), \
					NULL, 0, True }
#define	ICONV_ISO8859_2_GR	{ "ISO8859-2-GR", \
					&iconv_ISO8859_2_GR[0], \
					sizeof (iconv_ISO8859_2_GR), \
					NULL, 0, False }
#define	ICONV_ISO8859_3_GL	{ "ISO8859-3-GL", \
					&iconv_ISO8859_3_GL[0], \
					sizeof (iconv_ISO8859_3_GL), \
					NULL, 0, True }
#define	ICONV_ISO8859_3_GR	{ "ISO8859-3-GR", \
					&iconv_ISO8859_3_GR[0], \
					sizeof (iconv_ISO8859_3_GR), \
					NULL, 0, False }
#define	ICONV_ISO8859_4_GL	{ "ISO8859-4-GL", \
					&iconv_ISO8859_4_GL[0], \
					sizeof (iconv_ISO8859_4_GL), \
					NULL, 0, True }
#define	ICONV_ISO8859_4_GR	{ "ISO8859-4-GR", \
					&iconv_ISO8859_4_GR[0], \
					sizeof (iconv_ISO8859_4_GR), \
					NULL, 0, False }
#define	ICONV_ISO8859_5_GL	{ "ISO8859-5-GL", \
					&iconv_ISO8859_5_GL[0], \
					sizeof (iconv_ISO8859_5_GL), \
					NULL, 0, True }
#define	ICONV_ISO8859_5_GR	{ "ISO8859-5-GR", \
					&iconv_ISO8859_5_GR[0], \
					sizeof (iconv_ISO8859_5_GR), \
					NULL, 0, False }
#define	ICONV_ISO8859_6_GL	{ "ISO8859-6-GL", \
					&iconv_ISO8859_6_GL[0], \
					sizeof (iconv_ISO8859_6_GL), \
					NULL, 0, True }
#define	ICONV_ISO8859_6_GR	{ "ISO8859-6-GR", \
					&iconv_ISO8859_6_GR[0], \
					sizeof (iconv_ISO8859_6_GR), \
					NULL, 0, False }
#define	ICONV_ISO8859_7_GL	{ "ISO8859-7-GL", \
					&iconv_ISO8859_7_GL[0], \
					sizeof (iconv_ISO8859_7_GL), \
					NULL, 0, True }
#define	ICONV_ISO8859_7_GR	{ "ISO8859-7-GR", \
					&iconv_ISO8859_7_GR[0], \
					sizeof (iconv_ISO8859_7_GR), \
					NULL, 0, False }
#define	ICONV_ISO8859_8_GL	{ "ISO8859-8-GL", \
					&iconv_ISO8859_8_GL[0], \
					sizeof (iconv_ISO8859_8_GL), \
					NULL, 0, True }
#define	ICONV_ISO8859_8_GR	{ "ISO8859-8-GR", \
					&iconv_ISO8859_8_GR[0], \
					sizeof (iconv_ISO8859_8_GR), \
					NULL, 0, False }
#define	ICONV_ISO8859_9_GL	{ "ISO8859-9-GL", \
					&iconv_ISO8859_9_GL[0], \
					sizeof (iconv_ISO8859_9_GL), \
					NULL, 0, True }
#define	ICONV_ISO8859_9_GR	{ "ISO8859-9-GR", \
					&iconv_ISO8859_9_GR[0], \
					sizeof (iconv_ISO8859_9_GR), \
					NULL, 0, False }
#define	ICONV_JISX0201_1976	{ NULL, &iconv_JISX0201_1976[0], \
					sizeof (iconv_JISX0201_1976), \
					NULL, 0, True }
#define	ICONV_JISX0201_1976_GL	{ "JISX0201.1976-GL", \
					&iconv_JISX0201_1976_GL[0], \
					sizeof (iconv_JISX0201_1976_GL), \
					NULL, 0, True }
#define	ICONV_JISX0201_1976_GR	{ "JISX0201.1976-GR", \
					&iconv_JISX0201_1976_GR[0], \
					sizeof (iconv_JISX0201_1976_GR), \
					NULL, 0, False }
#define	ICONV_JISX0208_1978_GL	{ "JISX0208.1978-GL", \
					&iconv_JISX0208_1978_GL[0], \
					sizeof (iconv_JISX0208_1978_GL), \
					NULL, 0, True }
#define	ICONV_JISX0208_1978_GL_MAIL	\
				{ "JISX0208.1978-GL", \
					&iconv_JISX0208_1978_GL_MAIL[0], \
					sizeof (iconv_JISX0208_1978_GL_MAIL), \
					NULL, 0, True }
#define	ICONV_JISX0208_1978_GR	{ "JISX0208.1978-GR", \
					&iconv_JISX0208_1978_GR[0], \
					sizeof (iconv_JISX0208_1978_GR), \
					NULL, 0, False }
#define	ICONV_JISX0208_1983_GL	{ "JISX0208.1983-GL", \
					&iconv_JISX0208_1983_GL[0], \
					sizeof (iconv_JISX0208_1983_GL), \
					NULL, 0, True }
#define	ICONV_JISX0208_1983_GL_MAIL	\
				{ "JISX0208.1983-GL", \
					&iconv_JISX0208_1983_GL_MAIL[0], \
					sizeof (iconv_JISX0208_1983_GL_MAIL), \
					NULL, 0, True }
#define	ICONV_JISX0208_1983_GR	{ "JISX0208.1983-GR", \
					&iconv_JISX0208_1983_GR[0], \
					sizeof (iconv_JISX0208_1983_GR), \
					NULL, 0, False }

/*
 *	definitions of EscTbl, which list possible ISO2022 escape sequences.
 */

static EscTbl	etbl_ct_IBM_932[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GR,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GR,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GR,
	ICONV_JISX0208_1983_GL,
};

static unsigned char	csidx_ct_IBM_932(unsigned char *p, size_t l)
{
	unsigned char	c;
	unsigned short	dbcs;
	int	low, mid, high;

	c = *p;
	if (c <= 0x1f) {
		if (c == '\t' || c == '\n')
			return CONTROLCSID;
		return INVALIDCSID;
	}
	if (c <= 0x7f)
		return 0;
	if (c == 0x80)
		return INVALIDCSID;
	if (c == 0xa0)
		return INVALIDCSID;
	if (0xa1 <= c && c <= 0xdf)
		return 2;
	if (c <= 0xfc) {
		if (l < 2)
			return NEEDMORE;
		dbcs = (c << 8 & 0xff00) + (p[1] & 0xff);
		low = 0;
		high = sizeof (CP932toSJIS) / sizeof (CP932toSJIS[0]);
		while (low <= high) {
			mid = low + high >> 1;
			if (dbcs < CP932toSJIS[mid][FROM])
				high = mid - 1;
			else if (dbcs > CP932toSJIS[mid][FROM])
				low = mid + 1;
			else {
				dbcs = CP932toSJIS[mid][TO];
                                break;
			}
		}
		c = dbcs >> 8 & 0xff;
		if (c <= 0xef)
			return 8;
		else
			return 10;
	}
	return INVALIDCSID;
}

static EscTbl	etbl_ct_IBM_eucJP[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GR,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GR,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GR,
	ICONV_JISX0208_1983_GL,
};

static unsigned char	csidx_ct_IBM_eucJP(unsigned char *p, size_t l)
{
	unsigned char	c;

	c = *p;
	if (c <= 0x1f) {
		if (c == '\t' || c == '\n')
			return CONTROLCSID;
		return INVALIDCSID;
	}
	if (c <= 0x7f)
		return 0;
	if (c <= 0x8d)
		return INVALIDCSID;
	if (c == 0x8e)
		return 2;
	if (c == 0x8f) {
		if (l < 3)
			return NEEDMORE;
		if (0xa1 <= p[1] && p[1] <= 0xfe)
			return 10;
		return INVALIDCSID;
	}
	if (c <= 0x9a)
		return INVALIDCSID;
	if (c == 0x9b)
		return CONTROLCSID;
	if (c <= 0xa0)
		return INVALIDCSID;
	if (c <= 0xf4)
		return 8;
	if (c <= 0xfe)
		return 10;
	return INVALIDCSID;
}

static EscTbl	etbl_ct_ISO8859_1[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_1_GR,
	ICONV_ISO8859_1_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_ct_ISO8859_1(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X, X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static EscTbl	etbl_ct_ISO8859_2[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_2_GR,
	ICONV_ISO8859_2_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_ct_ISO8859_2(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X, X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static EscTbl	etbl_ct_ISO8859_3[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_3_GR,
	ICONV_ISO8859_3_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_ct_ISO8859_3(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X, X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_ct_ISO8859_4[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_4_GR,
	ICONV_ISO8859_4_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_ct_ISO8859_4(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X, X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_ct_ISO8859_5[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_5_GR,
	ICONV_ISO8859_5_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_ct_ISO8859_5(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X, X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_ct_ISO8859_6[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_6_GR,
	ICONV_ISO8859_6_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_ct_ISO8859_6(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X, X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_ct_ISO8859_7[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_7_GR,
	ICONV_ISO8859_7_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_ct_ISO8859_7(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X, X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static EscTbl	etbl_ct_ISO8859_8[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_8_GR,
	ICONV_ISO8859_8_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_ct_ISO8859_8(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X, X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static EscTbl	etbl_ct_ISO8859_9[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_9_GR,
	ICONV_ISO8859_9_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_ct_ISO8859_9(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	X, X, X, X, X, X, X, X, X, C, C, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, C, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static EscTbl	etbl_fold7_IBM_932[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GL,
};

static unsigned char	csidx_fold7_IBM_932(unsigned char *p, size_t l)
{
	unsigned char	c;
	unsigned short	dbcs;
	int	low, mid, high;

	c = *p;
	if (c <= 0x1f)
		return CONTROLCSID;
	if (c <= 0x7f)
		return 0;
	if (c == 0x80)
		return INVALIDCSID;
	if (c == 0xa0)
		return INVALIDCSID;
	if (0xa1 <= c && c <= 0xdf)
		return 2;
	if (c <= 0xfc) {
		if (l < 2)
			return NEEDMORE;
		dbcs = (c << 8 & 0xff00) + (p[1] & 0xff);
                low = 0;
		high = sizeof (CP932toSJIS) / sizeof (CP932toSJIS[0]);
		while (low <= high) {
			mid = low + high >> 1;
			if (dbcs < CP932toSJIS[mid][FROM])
				high = mid - 1;
			else if (dbcs > CP932toSJIS[mid][FROM])
				low = mid + 1;
			else {
				dbcs = CP932toSJIS[mid][TO];
                                break;
			}
		}
		c = dbcs >> 8 & 0xff;
		if (c <= 0xef)
			return 4;
		else
			return 7;
	}
	return INVALIDCSID;
}

static EscTbl	etbl_fold7_IBM_eucJP[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GL,
};

static unsigned char	csidx_fold7_IBM_eucJP(unsigned char *p, size_t l)
{
	unsigned char	c;

	c = *p;
	if (c <= 0x1f)
		return CONTROLCSID;
	if (c <= 0x7f)
		return 0;
	if (c <= 0x8d)
		return INVALIDCSID;
	if (c == 0x8e)
		return 2;
	if (c == 0x8f) {
		if (l < 3)
			return NEEDMORE;
		if (0xa1 <= p[1] && p[1] <= 0xfe)
			return 7;
		return INVALIDCSID;
	}
	if (c <= 0x9a)
		return INVALIDCSID;
	if (c == 0x9b)
		return CONTROLCSID;
	if (c <= 0xa0)
		return INVALIDCSID;
	if (c <= 0xf4)
		return 4;
	if (c <= 0xfe)
		return 7;
	return INVALIDCSID;
}

static EscTbl	etbl_fold7_ISO8859_1[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_1_GL,
};

static unsigned char	csidx_fold7_ISO8859_1(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static EscTbl	etbl_fold7_ISO8859_2[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_2_GL,
};

static unsigned char	csidx_fold7_ISO8859_2(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold7_ISO8859_3[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_3_GL,
};

static unsigned char	csidx_fold7_ISO8859_3(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold7_ISO8859_4[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_4_GL,
};

static unsigned char	csidx_fold7_ISO8859_4(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold7_ISO8859_5[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_5_GL,
};

static unsigned char	csidx_fold7_ISO8859_5(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold7_ISO8859_6[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_6_GL,
};

static unsigned char	csidx_fold7_ISO8859_6(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold7_ISO8859_7[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_7_GL,
};

static unsigned char	csidx_fold7_ISO8859_7(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold7_ISO8859_8[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_8_GL,
};

static unsigned char	csidx_fold7_ISO8859_8(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold7_ISO8859_9[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_9_GL,
};

static unsigned char	csidx_fold7_ISO8859_9(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static EscTbl	etbl_fold8_IBM_932[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GR,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GR,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GR,
	ICONV_JISX0208_1983_GL,
};

static unsigned char	csidx_fold8_IBM_932(unsigned char *p, size_t l)
{
	unsigned char	c;
	unsigned short	dbcs;
	int	low, mid, high;

	c = *p;
	if (c <= 0x1f)
		return CONTROLCSID;
	if (c <= 0x7f)
		return 0;
	if (c == 0x80)
		return INVALIDCSID;
	if (c == 0xa0)
		return INVALIDCSID;
	if (0xa1 <= c && c <= 0xdf)
		return 2;
	if (c <= 0xfc) {
		if (l < 2)
			return NEEDMORE;
		dbcs = (c << 8 & 0xff00) + (p[1] & 0xff);
                low = 0;
		high = sizeof (CP932toSJIS) / sizeof (CP932toSJIS[0]);
		while (low <= high) {
			mid = low + high >> 1;
			if (dbcs < CP932toSJIS[mid][FROM])
				high = mid - 1;
			else if (dbcs > CP932toSJIS[mid][FROM])
				low = mid + 1;
			else {
				dbcs = CP932toSJIS[mid][TO];
                                break;
			}
		}
		c = dbcs >> 8 & 0xff;
		if (c <= 0xef)
			return 8;
		else
			return 10;
	}
	return INVALIDCSID;
}

static EscTbl	etbl_fold8_IBM_eucJP[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_JISX0201_1976_GR,
	ICONV_JISX0201_1976_GL,
	ICONV_JISX0208_1978_GL_MAIL,
	ICONV_JISX0208_1983_GL_MAIL,
	ICONV_JISX0208_1978_GR,
	ICONV_JISX0208_1978_GL,
	ICONV_JISX0208_1983_GR,
	ICONV_JISX0208_1983_GL,
};

static unsigned char	csidx_fold8_IBM_eucJP(unsigned char *p, size_t l)
{
	unsigned char	c;

	c = *p;
	if (c <= 0x1f)
		return CONTROLCSID;
	if (c <= 0x7f)
		return 0;
	if (c <= 0x8d)
		return INVALIDCSID;
	if (c == 0x8e)
		return 2;
	if (c == 0x8f) {
		if (l < 3)
			return NEEDMORE;
		if (0xa1 <= p[1] && p[1] <= 0xfe)
			return 10;
		return INVALIDCSID;
	}
	if (c <= 0x9a)
		return INVALIDCSID;
	if (c == 0x9b)
		return CONTROLCSID;
	if (c <= 0xa0)
		return INVALIDCSID;
	if (c <= 0xf4)
		return 8;
	if (c <= 0xfe)
		return 10;
	return INVALIDCSID;
}

static EscTbl	etbl_fold8_ISO8859_1[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_1_GR,
	ICONV_ISO8859_1_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_fold8_ISO8859_1(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static EscTbl	etbl_fold8_ISO8859_2[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_2_GR,
	ICONV_ISO8859_2_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_fold8_ISO8859_2(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}


static EscTbl	etbl_fold8_ISO8859_3[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_3_GR,
	ICONV_ISO8859_3_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_fold8_ISO8859_3(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold8_ISO8859_4[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_4_GR,
	ICONV_ISO8859_4_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_fold8_ISO8859_4(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold8_ISO8859_5[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_5_GR,
	ICONV_ISO8859_5_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_fold8_ISO8859_5(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold8_ISO8859_6[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_6_GR,
	ICONV_ISO8859_6_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_fold8_ISO8859_6(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold8_ISO8859_7[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_7_GR,
	ICONV_ISO8859_7_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_fold8_ISO8859_7(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}
static EscTbl	etbl_fold8_ISO8859_8[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_8_GR,
	ICONV_ISO8859_8_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_fold8_ISO8859_8(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static EscTbl	etbl_fold8_ISO8859_9[] = {
	ICONV_ASCII,
	ICONV_JISX0201_1976,
	ICONV_ISO8859_9_GR,
	ICONV_ISO8859_9_GL,
	ICONV_ASCII_GR,
};

static unsigned char	csidx_fold8_ISO8859_9(unsigned char *p, size_t l)
{
static unsigned char	csidx[256] = {
#define	X	INVALIDCSID
#define	C	CONTROLCSID
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
#undef	C
#undef	X
};
	return csidx[*p];
}

static unsigned char	ct_isctl[256] = {
	2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static unsigned char	fold7_isctl[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static unsigned char	fold8_isctl[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};



#define	SIZEOF(name)	(sizeof (name) / sizeof (name[0]))

EscTblTbl	_iconv_ct_ett[] = {
	{
		"SJIS",
		SIZEOF(etbl_ct_IBM_932),
		0, 0,
		etbl_ct_IBM_932,
		csidx_ct_IBM_932,
		ct_isctl,
	},
	{
		"EUJIS",
		SIZEOF(etbl_ct_IBM_eucJP),
		0, 0,
		etbl_ct_IBM_eucJP,
		csidx_ct_IBM_eucJP,
		ct_isctl,
	},
	{
		"ISO8859-1",
		SIZEOF(etbl_ct_ISO8859_1),
		0, 2,
		etbl_ct_ISO8859_1,
		csidx_ct_ISO8859_1,
		ct_isctl,
	},
	{
		"ISO8859-2",
		SIZEOF(etbl_ct_ISO8859_2),
		0, 2,
		etbl_ct_ISO8859_2,
		csidx_ct_ISO8859_2,
		ct_isctl,
	},
	{
		"ISO8859-3",
		SIZEOF(etbl_ct_ISO8859_3),
		0, 2,
		etbl_ct_ISO8859_3,
		csidx_ct_ISO8859_3,
		ct_isctl,
	},
	{
		"ISO8859-4",
		SIZEOF(etbl_ct_ISO8859_4),
		0, 2,
		etbl_ct_ISO8859_4,
		csidx_ct_ISO8859_4,
		ct_isctl,
	},
	{
		"ISO8859-5",
		SIZEOF(etbl_ct_ISO8859_5),
		0, 2,
		etbl_ct_ISO8859_5,
		csidx_ct_ISO8859_5,
		ct_isctl,
	},
	{
		"ISO8859-6",
		SIZEOF(etbl_ct_ISO8859_6),
		0, 2,
		etbl_ct_ISO8859_6,
		csidx_ct_ISO8859_6,
		ct_isctl,
	},
	{
		"ISO8859-7",
		SIZEOF(etbl_ct_ISO8859_7),
		0, 2,
		etbl_ct_ISO8859_7,
		csidx_ct_ISO8859_7,
		ct_isctl,
	},
	{
		"ISO8859-8",
		SIZEOF(etbl_ct_ISO8859_8),
		0, 2,
		etbl_ct_ISO8859_8,
		csidx_ct_ISO8859_8,
		ct_isctl,
	},
	{
		"ISO8859-9",
		SIZEOF(etbl_ct_ISO8859_9),
		0, 2,
		etbl_ct_ISO8859_9,
		csidx_ct_ISO8859_9,
		ct_isctl,
	},
	{ NULL, 0, 0, 0, NULL, (unsigned char (*)())0, NULL }
};

EscTblTbl	_iconv_fold7_ett[] = {
	{
		"SJIS",
		SIZEOF(etbl_fold7_IBM_932),
		0, 0,
		etbl_fold7_IBM_932,
		csidx_fold7_IBM_932,
		fold7_isctl,
	},
	{
		"EUJIS",
		SIZEOF(etbl_fold7_IBM_eucJP),
		0, 0,
		etbl_fold7_IBM_eucJP,
		csidx_fold7_IBM_eucJP,
		fold7_isctl,
	},
	{
		"ISO8859-1",
		SIZEOF(etbl_fold7_ISO8859_1),
		0, 0,
		etbl_fold7_ISO8859_1,
		csidx_fold7_ISO8859_1,
		fold7_isctl,
	},
	{
		"ISO8859-2",
		SIZEOF(etbl_fold7_ISO8859_2),
		0, 0,
		etbl_fold7_ISO8859_2,
		csidx_fold7_ISO8859_2,
		fold7_isctl,
	},
	{
		"ISO8859-3",
		SIZEOF(etbl_fold7_ISO8859_3),
		0, 0,
		etbl_fold7_ISO8859_3,
		csidx_fold7_ISO8859_3,
		fold7_isctl,
	},
	{
		"ISO8859-4",
		SIZEOF(etbl_fold7_ISO8859_4),
		0, 0,
		etbl_fold7_ISO8859_4,
		csidx_fold7_ISO8859_4,
		fold7_isctl,
	},
	{
		"ISO8859-5",
		SIZEOF(etbl_fold7_ISO8859_5),
		0, 0,
		etbl_fold7_ISO8859_5,
		csidx_fold7_ISO8859_5,
		fold7_isctl,
	},
	{
		"ISO8859-6",
		SIZEOF(etbl_fold7_ISO8859_6),
		0, 0,
		etbl_fold7_ISO8859_6,
		csidx_fold7_ISO8859_6,
		fold7_isctl,
	},
	{
		"ISO8859-7",
		SIZEOF(etbl_fold7_ISO8859_7),
		0, 0,
		etbl_fold7_ISO8859_7,
		csidx_fold7_ISO8859_7,
		fold7_isctl,
	},
	{
		"ISO8859-8",
		SIZEOF(etbl_fold7_ISO8859_8),
		0, 0,
		etbl_fold7_ISO8859_8,
		csidx_fold7_ISO8859_8,
		fold7_isctl,
	},
	{
		"ISO8859-9",
		SIZEOF(etbl_fold7_ISO8859_9),
		0, 0,
		etbl_fold7_ISO8859_9,
		csidx_fold7_ISO8859_9,
		fold7_isctl,
	},
	{ NULL, 0, 0, 0, NULL, (unsigned char (*)())0, NULL }
};

EscTblTbl	_iconv_fold8_ett[] = {
	{
		"SJIS",
		SIZEOF(etbl_fold8_IBM_932),
		0, 0,
		etbl_fold8_IBM_932,
		csidx_fold8_IBM_932,
		fold8_isctl,
	},
	{
		"EUJIS",
		SIZEOF(etbl_fold8_IBM_eucJP),
		0, 0,
		etbl_fold8_IBM_eucJP,
		csidx_fold8_IBM_eucJP,
		fold8_isctl,
	},
	{
		"ISO8859-1",
		SIZEOF(etbl_fold8_ISO8859_1),
		0, 2,
		etbl_fold8_ISO8859_1,
		csidx_fold8_ISO8859_1,
		fold8_isctl,
	},
	{
		"ISO8859-2",
		SIZEOF(etbl_fold8_ISO8859_2),
		0, 2,
		etbl_fold8_ISO8859_2,
		csidx_fold8_ISO8859_2,
		fold8_isctl,
	},
	{
		"ISO8859-3",
		SIZEOF(etbl_fold8_ISO8859_3),
		0, 2,
		etbl_fold8_ISO8859_3,
		csidx_fold8_ISO8859_3,
		fold8_isctl,
	},
	{
		"ISO8859-4",
		SIZEOF(etbl_fold8_ISO8859_4),
		0, 2,
		etbl_fold8_ISO8859_4,
		csidx_fold8_ISO8859_4,
		fold8_isctl,
	},
	{
		"ISO8859-5",
		SIZEOF(etbl_fold8_ISO8859_5),
		0, 2,
		etbl_fold8_ISO8859_5,
		csidx_fold8_ISO8859_5,
		fold8_isctl,
	},
	{
		"ISO8859-6",
		SIZEOF(etbl_fold8_ISO8859_6),
		0, 2,
		etbl_fold8_ISO8859_6,
		csidx_fold8_ISO8859_6,
		fold8_isctl,
	},
	{
		"ISO8859-7",
		SIZEOF(etbl_fold8_ISO8859_7),
		0, 2,
		etbl_fold8_ISO8859_7,
		csidx_fold8_ISO8859_7,
		fold8_isctl,
	},
	{
		"ISO8859-8",
		SIZEOF(etbl_fold8_ISO8859_8),
		0, 2,
		etbl_fold8_ISO8859_8,
		csidx_fold8_ISO8859_8,
		fold8_isctl,
	},
	{
		"ISO8859-9",
		SIZEOF(etbl_fold8_ISO8859_9),
		0, 2,
		etbl_fold8_ISO8859_9,
		csidx_fold8_ISO8859_9,
		fold8_isctl,
	},
	{ NULL, 0, 0, 0, NULL, (unsigned char (*)())0, NULL }
};
