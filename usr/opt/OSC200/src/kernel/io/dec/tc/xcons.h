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
 *	@(#)$RCSfile: xcons.h,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1993/07/14 18:18:16 $
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
 * derived from xcons.h	4.1	(ULTRIX)	8/9/90
 */
/*
 * xcons.h
 *
 * xcons alternate console driver
 *
 * Modification history
 *
 *   4-Jul-90	Randall Brown
 *		Created file.
 */

#ifndef _TC_XCONS_H_
#define _TC_XCONS_H_

extern int xcons_kern_loop;

#define XCONSDEV	0

#define XCONS_CLOSED	1
#define XCONS_BLOCKED	2
#define XCONS_OK	3

#endif
