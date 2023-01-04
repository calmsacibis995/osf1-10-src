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

#ifndef _DLI_DLI_TYPES_H
#define	_DLI_DLI_TYPES_H
#include <machine/endian.h>

#define	W_T	unsigned short
#define	LW_T	unsigned int
#if defined(__alpha)
#define QW_T	unsigned long
#endif
#ifdef	QW_T
#define	BS_T	QW_T
#else
#define	BS_T	LW_T
#endif

#define	DLI_DECL_BITSET(a, n)	BS_T a[((n) + (sizeof(BS_T)*8)-1)/(sizeof(BS_T)*8)]
#define	DLI_SETBIT(bit, set)	((set)[(bit)/(sizeof(BS_T)*8)] |=  (1L << ((bit) & (sizeof(BS_T)*8-1))))
#define	DLI_TSTBIT(bit, set)	((set)[(bit)/(sizeof(BS_T)*8)] &   (1L << ((bit) & (sizeof(BS_T)*8-1))))
#define	DLI_CLRBIT(bit, set)	((set)[(bit)/(sizeof(BS_T)*8)] &= ~(1L << ((bit) & (sizeof(BS_T)*8-1))))

#define LE_PUT8(p,v)	(*(p)++ = (v))
#define LE_EXT8(p)	((unsigned) (*((u_char *) p)))
#define LE_GET8(p)	((unsigned) (*((u_char *) p)++))

#define LE_PUT16(p,v)	(*(p)++ = (v) & 0xFF, *(p)++ = (v) >> 8)
#define LE_EXT16(p)	((unsigned) (*((u_char *) p)) | (*((u_char *) p + 1) << 8))
#define LE_INS16(p,v)	(*(p) = (v) & 0xFF, *((p)+1) = (v) >> 8)

#define LE_PUT32(p,v)	(*(p)++ = (v) & 0xFF, *(p)++ = (v) >> 8, \
			 *(p)++ = (v) >> 16, *(p)++ = (v) >> 24)

#if BYTE_ORDER == LITTLE_ENDIAN

#define	LE_PUT16A(p, v)	(*(u_short *)(p) = (v), (p) += 2)
#define	LE_PUT32A(p, v)	(*(u_int   *)(p) = (v), (p) += 4)

#define	W_B0(x)		(((W_T) x) <<  0)
#define	W_B1(x)		(((W_T) x) <<  8)

#define	W_X_B0(x)	((((W_T) x) >>  0) & 0xFF)
#define	W_X_B1(x)	((((W_T) x) >>  8) & 0xFF)

#define	LW_B0(x)	(((LW_T) x) <<  0)
#define	LW_B1(x)	(((LW_T) x) <<  8)
#define	LW_B2(x)	(((LW_T) x) << 16)
#define	LW_B3(x)	(((LW_T) x) << 24)

#define	LW_W0(x)	(((LW_T) x) <<  0)
#define	LW_W1(x)	(((LW_T) x) << 16)

#define	LW_X_B0(x)	((((LW_T) x) >>  0) & 0xFF)
#define	LW_X_B1(x)	((((LW_T) x) >>  8) & 0xFF)
#define	LW_X_B2(x)	((((LW_T) x) >> 16) & 0xFF)
#define	LW_X_B3(x)	((((LW_T) x) >> 24) & 0xFF)

#define	LW_X_W0(x)	((((LW_T) x) >>  0) & 0xFFFF)
#define	LW_X_W1(x)	((((LW_T) x) >> 16) & 0xFFFF)

#define	LW_X_B123(x)	(((LW_T) x) >> 8)

#ifdef QW_T
#define	QW_B0(x)	(((QW_T) x) <<  0)
#define	QW_B1(x)	(((QW_T) x) <<  8)
#define	QW_B2(x)	(((QW_T) x) << 16)
#define	QW_B3(x)	(((QW_T) x) << 24)
#define	QW_B4(x)	(((QW_T) x) << 32)
#define	QW_B5(x)	(((QW_T) x) << 40)
#define	QW_B6(x)	(((QW_T) x) << 48)
#define	QW_B7(x)	(((QW_T) x) << 56)

#define	QW_W0(x)	(((QW_T) x) <<  0)
#define	QW_W1(x)	(((QW_T) x) << 16)
#define	QW_W2(x)	(((QW_T) x) << 32)
#define	QW_W3(x)	(((QW_T) x) << 48)

#define	QW_LW0(x)	(((QW_T) x) <<  0)
#define	QW_LW1(x)	(((QW_T) x) << 32)

#define	QW_X_B0(x)	(((QW_T) x) >>  0) & 0xFFL)
#define	QW_X_B1(x)	(((QW_T) x) >>  8) & 0xFFL)
#define	QW_X_B2(x)	(((QW_T) x) >> 16) & 0xFFL)
#define	QW_X_B3(x)	(((QW_T) x) >> 24) & 0xFFL)
#define	QW_X_B4(x)	(((QW_T) x) >> 32) & 0xFFL)
#define	QW_X_B5(x)	(((QW_T) x) >> 40) & 0xFFL)
#define	QW_X_B6(x)	(((QW_T) x) >> 48) & 0xFFL)
#define	QW_X_B7(x)	(((QW_T) x) >> 56) & 0xFFL)

#define	QW_X_W0(x)	(((QW_T) x) >>  0) & 0xFFFFL)
#define	QW_X_W1(x)	(((QW_T) x) >> 16) & 0xFFFFL)
#define	QW_X_W2(x)	(((QW_T) x) >> 32) & 0xFFFFL)
#define	QW_X_W3(x)	(((QW_T) x) >> 48) & 0xFFFFL)

#define	QW_X_LW0(x)	(((QW_T) x) >>  0) & 0xFFFFFFFFL)
#define	QW_X_LW1(x)	(((QW_T) x) >> 32) & 0xFFFFFFFFL)
#endif

#elif BYTE_ORDER == BIT_ENDIAN

#define	LE_PUT16A	LE_PUT16
#define	LE_PUT32A	LE_PUT32

#define	W_B1(x)		(((W_T) x) <<  0)
#define	W_B0(x)		(((W_T) x) <<  8)

#define	W_X_B1(x)	((((W_T) x) >>  0) & 0xFF)
#define	W_X_B0(x)	((((W_T) x) >>  8) & 0xFF)

#define	LW_B3(x)	(((LW_T) x) <<  0)
#define	LW_B2(x)	(((LW_T) x) <<  8)
#define	LW_B1(x)	(((LW_T) x) << 16)
#define	LW_B0(x)	(((LW_T) x) << 24)

#define	LW_W1(x)	(((LW_T) x) <<  0)
#define	LW_W0(x)	(((LW_T) x) << 16)

#define	LW_X_B3(x)	((((LW_T) x) >>  0) & 0xFF)
#define	LW_X_B2(x)	((((LW_T) x) >>  8) & 0xFF)
#define	LW_X_B1(x)	((((LW_T) x) >> 16) & 0xFF)
#define	LW_X_B0(x)	((((LW_T) x) >> 24) & 0xFF)

#define	LW_X_W1(x)	((((LW_T) x) >>  0) & 0xFFFF)
#define	LW_X_W0(x)	((((LW_T) x) >> 16) & 0xFFFF)

#define	LW_X_B123(x)	(((LW_T) x) << 8)

#endif /* BYTE_ORDER */
#endif /* _DLI_DLI_TYPES_H */
