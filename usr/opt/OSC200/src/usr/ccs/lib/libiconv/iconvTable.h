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
 *	@(#)$RCSfile: iconvTable.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:44:58 $
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
 * 1.5  com/inc/iconvTable.h, , bos320, 9134320c 8/19/91 21:45:16
 */

#ifndef	_ICONVTABLE_H
#define	_ICONVTABLE_H

/*
 * This header file defines the convertion table structures.   
 * The structure of the translation table is also the table format used
 * by the command genxlt.
 */


#define	ICONV_REL1_MAGIC	's'
#define	ICONV_REL2_MAGIC	'2'

#define	INVAL_STRINGHEAD	'i'

typedef struct _IconvTable	{
	unsigned char	magic;
	unsigned char	inval_handle;
	unsigned char	inval_char;
	unsigned char	dummy[29];
	unsigned char	data[256];
} IconvTable;

typedef struct _iconvTable_rec	{
	_LC_core_iconv_t	core;
	IconvTable	table;
} iconvTable_t;

#endif	/* _ICONVTABLE_H */
