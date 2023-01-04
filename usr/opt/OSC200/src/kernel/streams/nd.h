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
 * @(#)$RCSfile: nd.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/12 20:08:31 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 */
/** Copyright (c) 1988  Mentat Inc.
 ** nd.c 1.5, last change 1/2/90
 **/

#ifndef _ND_H
#define _ND_H

#define	ND_BASE		('N' << 8)	/** base */
#define	ND_GET		(ND_BASE + 0)	/** Get a value */
#define	ND_SET		(ND_BASE + 1)	/** Set a value */

extern	void	nd_free(caddr_t * nd_pparam);
extern	int	nd_getset(queue_t * q, caddr_t nd_param, mblk_t * mp);
extern	int	nd_get_default(queue_t * q, mblk_t * mp, caddr_t data);
extern	int	nd_get_long(queue_t * q, mblk_t * mp, ulong * lp);
extern	int	nd_load(caddr_t * nd_pparam, char * name, int (*get_pfi)(), int (*set_pfi)(), caddr_t data);
extern	int	nd_set_long(queue_t * q, mblk_t * mp, char * value, ulong * lp);
#endif
