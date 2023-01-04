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
 *	@(#)$RCSfile: PrUtils.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:00:18 $
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
#ifdef SEC_BASE

#ifndef _PrUtils_h_
#define _PrUtils_h_
/*
 *	Copyright 1990, SecureWare, Inc. Atlanta, GA. All righse reserved
 */

/*
	filename:
		PrUtils.h
	
	copyright:
		Copyright (c) 1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		protected password utility declarations
*/


void
	GetAllLockedUsers(),
	GetAllLockedIssoUsers(),
	GetAllLockedNonIssoUsers(),
	GetAllUnlockedUsers(),
	GetAllUnlockedIssoUsers(),
	GetAllUnlockedNonIssoUsers(),
	GetAllNonretiredUsers(),
	GetAllNonretiredIssoUsers(),
	GetAllNonretiredNonIssoUsers();

#endif /* _PrUtils_h_ */
#endif /* SEC_BASE */
