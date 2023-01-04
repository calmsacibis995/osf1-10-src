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
 *	@(#)$RCSfile: iconvP.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:44:44 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 *
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
 * 1.4  com/inc/iconvP.h, libiconv, bos320, 9142320c 10/8/91 00:00:38
 */

#ifndef __iconvP_h
#define __iconvP_h

#define	 TRUE			1
#define	 FALSE			0
#define	HOST_SUBCHAR		0x3f
#define	ISO88591_SUBCHAR	0x1a
#define	IBM850_SUBCHAR		0x1a
#define	IBM932_SUBCHAR		0x1a
#define	IBM932_D_SUBCHAR_1	0xfc
#define	IBM932_D_SUBCHAR_2	0xfc
#define	EUCJP_SUBCHAR		0x1a
#define	EUCJP_D_SUBCHAR_1	0xf4
#define	EUCJP_D_SUBCHAR_2	0xfe
#define	SUB			0x1a
#define	HOST_D_SUBCHAR_1	0xfe
#define	HOST_D_SUBCHAR_2	0xfe
#define EUCSS2 0x8E             /* EUC Single Shift 2 */
#define EUCSS3 0x8F             /* EUC Single Shift 3 */
#define SO (unsigned char) 0x0E      /* Shift Out, host code to turn DBCS on */
#define SI (unsigned char) 0x0F      /* Shift In, host code to turn DBCS off */
#define	FROM			0
#define	TO			1

extern int	_ascii_exec(
		unsigned char **, size_t *,
		unsigned char **, size_t *);


#endif
