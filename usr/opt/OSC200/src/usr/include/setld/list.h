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
 *      @(#)$RCSfile: list.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:12:11 $
 */
/*
 */
/*	list.h -
 *		header file for use with list routines in list.c
 *
 *	Modifications:
 *
 *	000	19-apr-1989	ccb
 *		added this comment
 *	001	14-jun-1989	ccb
 *		declare ListFree()
*/

/*	DataTypes
*/

typedef struct ListT {
	struct ListT	*l_next;	/* linkage pointer */
	char		*l_data;	/* trash data pointer */
} ListT;

/*	Function Types
*/
extern ListT	*ListAppend();
extern void	ListFree();


