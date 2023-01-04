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
 *	@(#)$RCSfile: fcs.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:44:09 $
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
 * 1.2  com/inc/fcs.h, libiconv, bos320, 9125320 6/11/91 00:19:14
 */

#define	False	0
#define	True	0

typedef struct _EscTbl	EscTbl;
struct _EscTbl	{
	char	*name;
	char	*str;
	int	len;
	char	*seg;
	int	seglen;
	int	gl;
};

typedef struct _EscTblTbl	EscTblTbl;
struct _EscTblTbl	{
	char	*name;
	int	netbl;
	int	defgl;
	int	defgr;
	EscTbl	*etbl;
	unsigned char	(*csidx)();
	unsigned char	*isctl;
};

#define	INVALIDCSID	0xff
#define	CONTROLCSID	0xfe
#define	NEEDMORE	0xfd

extern EscTblTbl	_iconv_ct_ett[];
extern EscTblTbl	_iconv_fold7_ett[];
extern EscTblTbl	_iconv_fold8_ett[];
