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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: autoconf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:16:07 $";
#endif 
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
 
#include <cpus.h>
#include <cputypes.h>
#include <generic.h>

#include <sys/param.h>
#include <mach/machine.h>
#include <machine/cpu.h>


#if	AT386 || PS2
#include <i386/AT386/atbus.h>

#if PS2
#include <un.h>
#endif /* PS2 */
#if NUN > 0
extern	struct	i386_ctlr	unctlr[];
#endif /* NUN */


#include <pc586.h>
#if NPC586 > 0
extern	struct	i386_dev	pcinfo[];
#endif /* NPC586 */

#if 	JDXXX
#include <at3c501.h>
#if NAT3C501 > 0
extern	struct	i386_dev	at3c501info[];
#endif /* NAT3C501 */
#endif 	/* JDXXX */

#include <qd.h>
#if NQD > 0
extern	struct	i386_ctlr	qdctlr[];
#endif /* NQD */

#include <com.h>
#if NCOM > 0
extern	struct	i386_ctlr	comctlr[];
extern	struct	i386_dev	cominfo[];
#endif /* NCOM */

#include <ln.h>

#if NLN > 0
extern	struct	i386_ctlr	lnctlr[];
extern	struct	i386_dev	lninfo[];
#endif /* NLN */

#endif	/* AT386 | PS2 */


/*
 * Determine mass storage and memory configuration for a machine.
 * Get cpu type, and then switch out to machine specific procedures
 * which will probe adaptors to see what is out there.
 */
configure()
{

	master_cpu = 0;
	set_cpu_number();
#if	NCPUS > 1
	printf("Master cpu at slot %d.\n", master_cpu);
#endif	/* NCPUS > 1*/
	machine_slot[master_cpu].is_cpu = TRUE;
	machine_slot[master_cpu].running = TRUE;
	machine_slot[master_cpu].cpu_type = CPU_TYPE_I386;
	cpuspeed = 6;
#if	EXL
	machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_EXL;
#endif	/* EXL */
#if	AT386
	machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_AT386;
#endif	/* AT386 */

#if	AT386 || PS2
	probeio();
#endif

#if	GENERIC > 0
	setconf();
#endif	/* GENERIC > 0 */
	return;
}


#if	AT386 || PS2
/*
 * probeio:
 *
 *	Probe and subsequently attach devices out on the AT bus.
 *
 *
 */

probeio()

{
	int	i;
	struct	i386_dev	*dev_p;
	struct	i386_driver	*drv_p;

#if NPC586 > 0
	for (i = 0; i < NPC586; i++) {
		dev_p = &pcinfo[i];
		drv_p = dev_p->dev_driver;

		if (drv_p->driver_probe(dev_p->dev_addr, i)) {
			dev_p->dev_alive = 1;
			drv_p->driver_attach(dev_p);
		}
	}
#endif /* NPC586 */

#if NUN > 0
        for (i = 0; i < NUN; i++) {
                drv_p = unctlr[i].ctlr_driver;
                dev_p = drv_p->driver_dinfo;
                if (drv_p->driver_probe(dev_p->dev_addr, i))
                        drv_p->driver_attach(dev_p);
        }
#endif /* NUN > 0 */

#if NAT3C501 > 0
	for (i = 0; i < NAT3C501; i++) {
		dev_p = &at3c501info[i];
		drv_p = dev_p->dev_driver;

		if (drv_p->driver_probe(dev_p->dev_addr, i)) {
			dev_p->dev_alive = 1;
			drv_p->driver_attach(dev_p);
		}
	}
#endif /* NAT3C501 */

#if NQD > 0
	for (i = 0; i < NQD; i++) {
		drv_p = qdctlr[i].ctlr_driver;
		dev_p = (struct i386_dev *)(drv_p->driver_dinfo + i);
		if (drv_p->driver_probe(dev_p->dev_addr, i)) {
			drv_p->driver_attach(dev_p);
		}
	}
#endif /*  NQD */

#if NCOM > 0
	for (i = 0; i <NCOM; i++) {
		drv_p = comctlr[i].ctlr_driver;
		dev_p = (struct i386_dev *)(drv_p->driver_dinfo + i);
		if (drv_p->driver_probe(dev_p->dev_addr, i)) {
			drv_p->driver_attach(dev_p);
		}
	}
#endif /* NCOM */

#if NLN > 0
	for (i = 0; i < NLN; i++) {
		drv_p = lnctlr[i].ctlr_driver;
		dev_p = (struct i386_dev *)(drv_p->driver_dinfo + i);
		if (drv_p->driver_probe(dev_p->dev_addr, i)) {
			drv_p->driver_attach(dev_p);
		}
	}
#endif /* NLN */


}
#endif /* AT386 || PS2 */
