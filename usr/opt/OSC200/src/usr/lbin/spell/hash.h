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
 *	@(#)$RCSfile: hash.h,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/12/07 10:05:18 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */ 
/*  hash.h	1.2  11/18/89 17:09:07"   */
/*
 * COMPONENT_NAME: (CMDTEXT) Text Formatting Services
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3
 *
 * OBJECT CODE ONLY SOURCE MATERIALS
 *
 */
#define HASHWIDTH 27
#define HASHSIZE 134217689	/*prime under 2^HASHWIDTH*/
#define INDEXWIDTH 9
#define INDEXSIZE (1<<INDEXWIDTH)
#define NI (INDEXSIZE+1)
#define ND ((25750/2)*sizeof(*table))
#define BYTE 8

extern unsigned *table;
extern int spell_index[];	/*into dif table based on hi hash bits*/
				/* 001-kak */

extern int hash();
