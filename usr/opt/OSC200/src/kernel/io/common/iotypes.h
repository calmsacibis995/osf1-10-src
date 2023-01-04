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
 * @(#)$RCSfile: iotypes.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/12/20 21:36:33 $
 */
#if !defined(_IOTYPES_INCLUDE_)
#define _IOTYPES_INCLUDE_

/*
 * Defines for 64bit conversions.
 */
#define I8	char
#define U8	unsigned char
#define I16	short
#define U16	unsigned short
#define I32	int
#define U32	unsigned int
#define I64	long
#define U64	unsigned long
#define I_WORD	long
#define U_WORD	unsigned long

typedef union pad_u32 {
    U32 reg;
    unsigned long padding;
} PAD_U32;

#endif /* !defined(_IOTYPES_INCLUDE_) */
