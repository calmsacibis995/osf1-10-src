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
 *	@(#)$RCSfile: vm_tuning.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:19:54 $
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
 * derived from vm_tuning.h	2.1	(ULTRIX/OSF)	12/3/90
 */

#ifndef	_MIPS_VM_TUNING_H_
#define	_MIPS_VM_TUNING_H_

/*
 *	File:	mips/vm_tuning.h
 *
 *	VM tuning parameters for the mips.
 */

#define	VM_PAGE_INACTIVE_TARGET(free)	(5 + (free) / 50)

#define	VM_PAGEOUT_BURST_WAIT	100

#endif	/* _MIPS_VM_TUNING_H_ */
