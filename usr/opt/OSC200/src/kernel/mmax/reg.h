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
 *	@(#)$RCSfile: reg.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:45 $
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
 *     Copyright (C) 1984 Hydra Computer Systems, Inc.
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Hydra 
 * Computer Systems, Inc. This software is made available solely 
 * pursuant to the terms of a software license agreement which governs 
 * its use. Unauthorized duplication, distribution or sale are strictly 
 * prohibited.
 *
 * Include file description:
 *	This include defines the register offsets on the system stack relative
 *	to register 0 (r0), pointed to by u.u_ar0.
 *
 * Original Author: Tony Anzelmo	Created on: 85/02/07
 */

#define PS	(3)
#define PSR	(3)
#define PC	(2)
#define FP	(1)
#define R0	(0)
#define R1	(-1)
#define R2	(-2)
#define R3	(-3)
#define R4	(-4)
#define R5	(-5)
#define R6	(-6)
#define R7	(-7)
#define SP	(-8)
