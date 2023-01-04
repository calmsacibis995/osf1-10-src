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
 *	@(#)$RCSfile: atbus.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:07:31 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */


/*
 * per-controller & driver definitions
 */

#ifndef ASSEMBLER
/*
 * Per-controller structure.
 * (E.g. one for each disk and tape controller, and other things
 * which have slave-style devices).
 *
 */
struct isa_ctlr {
	struct	isa_driver *ctlr_driver;
	long	ctlr_ctlr;	/* controller index in driver */
	long	ctlr_alive;	/* controller exists */
	caddr_t	ctlr_addr;	/* csr address */
	long	ctlr_spl;	/* spl level set upon interrupt */
	long	ctlr_pic;	/* pic line for controller */
	int	(**ctlr_intr)();/* interrupt handler */
	caddr_t	ctlr_start;	/* start address in mem space */
	u_long	ctlr_len;	/* length of mem space used */
};

/*
 * Per ``device'' structure.
 * (Everything else is a ``device''.)
 *
 * If a controller has many drives attached, then there will
 * be several isa_dev structures associated
 *
 */
struct isa_dev {
	struct	isa_driver *dev_driver;
	long	dev_unit;	/* unit number on the system */
	long	dev_ctlr;	/* ctlr number; -1 if none */
	long	dev_slave;	/* slave on controller */
	long	dev_alive;	/* Was it found at config time? */
	caddr_t	dev_addr;	/* csr address */
	short	dev_spl;	/* spl level */
	long	dev_pic;	/* pic line for device */
	long	dev_dk;		/* if init 1 set to number for iostat */
	long	dev_flags;	/* parameter from system specification */
	int	(**dev_intr)();	/* interrupt handler(s) */
	caddr_t	dev_start;	/* start address in mem space */
	u_long	dev_len;	/* length of mem space used */
	long	dev_type;	/* driver specific type information */
/* this is the forward link in a list of devices on a controller */
	struct	isa_dev *dev_forw;
/* if the device is connected to a controller, this is the controller */
	struct	isa_ctlr *dev_mi;
};

/*
 * Per-driver structure.
 *
 * Each driver defines entries for a set of routines for use
 * at boot time by the autoconfig routines.
 */
struct isa_driver {
	int	(*driver_probe)();	/* see if a driver is really there */
	int	(*driver_slave)();	/* see if a slave is there */
	int	(*driver_attach)();	/* setup driver for a slave */
	char	*driver_dname;		/* name of a device */
	struct	isa_dev *driver_dinfo;/* backptrs to init structs */
	char	*driver_mname;		/* name of a controller */
	struct	isa_ctlr **driver_minfo;/* backpointers to init structs */
};
#endif
