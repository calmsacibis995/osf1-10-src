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
 * @(#)$RCSfile: termiox.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/02 16:43:31 $
 */

#ifndef _SYS_TERMIOX_H_
#define _SYS_TERMIOX_H_

#define NFF 	8

struct termiox {
	unsigned short x_hflag ;
	unsigned short x_cflag ;
	unsigned short x_rflag[NFF] ;
	unsigned short x_sflag ;
};

#define	RTSXOFF	0000001
#define	CTSXON	0000002
#define	DTRXOFF	0000004
#define	CDXON	0000010
#define	ISXOFF	0000020


#define	XMTCLK	0000007
#define	XCIBRG	0000000
#define	XCTSET	0000001
#define	XCRSET	0000002
#define	RCVCLK	0000070
#define	RCIBRG	0000000
#define	RCTSET	0000010
#define	RCRSET	0000020
#define	TSETCLK	0000700
#define	TSETCOFF	0000000
#define	TSETCRBRG	0000100
#define	TSETCTBRG	0000200
#define	TSETCTSET	0000300
#define	TSETCRSET	0000400
#define	RSETCLK	0007000
#define	RSETCOFF	0000000
#define	RSETCRBRG	0001000
#define	RSETCTBRG	0002000
#define	RSETCTSET	0003000
#define	RSETCRSET	0004000

#endif /* _SYS_TERMIOX_H_ */
