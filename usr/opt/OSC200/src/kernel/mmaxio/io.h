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
 *	@(#)$RCSfile: io.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:16 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*

 *
 */


/*
 *	Various structures used for configuration data
 */

#define NEMCDEV		32	/* number of subunits on an EMC */
#define NUMSYSITBL	256	/* number of entries in itbl structure */

struct	subdevaddr {		/* subdevice address */
	char	s_slotunit;	/* index into appropriate devaddr[] */
	char	s_subunit;	/* subunit within channel/slot */
	char	s_valid;	/* initialization routine must set to 1 */
} ;


struct	devaddr	{		/* channel device address */
	char	v_chan;		/* 3B channel number */
	char	v_dev;		/* device location on channel */
	char	v_valid;	/* initialization routine must set to 1 */
} ;



struct	itbl	{		/* interrupt table - map device to unit */
	int	(*i_func)();	/* interrupt handler */
	int	i_param;	/* unit number passed to handler */
} ;

/* 
 * For s_valid and v_valid fields above.  Initially zero.
 */
#define DEV_INVALID	0
#define DEV_VALID	1
#define DEV_INITIALIZED	2

