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
 *	@(#)$RCSfile: cmn_err.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:56:59 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 * 
 */

#ifndef	_CMN_ERR_H
#define	_CMN_ERR_H

/*
 * STREAMS common error codes.
 */

#ifdef	_NO_PROTO

extern	void	cmn_err();

#else

extern	void	cmn_err(int, char *, ...);

#endif

/*
 *	level definitions for cmn_err()
 */
#define	CE_CONT		0x0001		/* simple printf */
#define CE_NOTE		0x0002		/* "NOTICE:" */
#define	CE_WARN		0x0003		/* "WARNING:" */
#define	CE_PANIC	0x0004		/* "PANIC:" */
#endif
