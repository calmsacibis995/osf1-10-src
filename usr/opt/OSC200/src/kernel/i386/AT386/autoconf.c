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
static char	*sccsid = "@(#)$RCSfile: autoconf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:07:36 $";
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

#include <i386/ipl.h>
#include <i386/AT386/atbus.h>

#include <hd.h>
#if NHD > 0
extern	struct	isa_driver	hddriver;
extern int			(*hdintrs[])();
#endif

#include <fd.h>
#if NFD > 0
extern	struct	isa_driver	fddriver;
extern int			(*fdintrs[])();
#endif

#include <pc586.h>
#if NPC586 > 0
extern	struct	isa_driver	pcdriver;
extern int			(*pc586intrs[])();
#endif

#include <wd8003.h>
#if NWD8003 > 0
extern	struct	isa_driver	wd8003driver;
extern int			(*wd8003intrs[])();
#endif

/*
#include <at3c501.h>
#if NAT3C501 > 0
extern	struct	isa_driver	at3c501driver;
extern int			(*at3c501intrs[])();
#endif
*/

#include <com.h>
#if NCOM > 0
extern	struct	isa_driver	comdriver;
extern int			(*comintrs[])();
#endif

#include <wt.h>
#if NWT > 0
extern	struct	isa_driver	wtdriver;
extern int			(*wtintrs[])();
#endif

#include <lv.h>
#if NLV > 0
extern int lvprobe();
#endif

#if	anyone_cares_anymore

#include <qd.h>
#if NQD > 0
extern	struct	isa_ctlr	qdctlr[];
#endif

#include <ln.h>
#if NLN > 0
extern	struct	isa_ctlr	lnctlr[];
extern	struct	isa_dev	lninfo[];
#endif

#endif	/* anyone_cares_anymore */


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
#endif
	machine_slot[master_cpu].is_cpu = TRUE;
	machine_slot[master_cpu].running = TRUE;
	machine_slot[master_cpu].cpu_type = CPU_TYPE_I386;
	cpuspeed = 6;
#ifdef	AT386
	machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_AT386;
	probeio();
#endif

#if     iPSC2
        machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_iPSC2;

        /* Call DCM driver to initialize itself, interrupts are already
           enabled at this point. We may want to change this later so that
           we disable DCM interrupts during bootld and turn them back on
           in dcminit().
         */
        dcminit();
#endif
#if NLV > 0
	lvprobe(0);
#endif
#if	GENERIC > 0
	setconf();
#endif
	return;
}

struct	isa_ctlr	Ctlrs[] = {
	/*
	    driver	idx   aliv 	 csr	        spl    pic  intrs
					start	    len type fwdp ctp
	 */
#if NHD > 0
	{&hddriver, 	 0,    0, (caddr_t)0x1f0,	 SPL5, 14, hdintrs,
				  (caddr_t)0x1f0,  8},
#else
	/* Holds place for Devs reference */
	{0,	 	 0,    0, (caddr_t)0x0,		 SPL0,  0, 0,
				  (caddr_t)0x0,    0},
#endif

#if NFD > 0
	{&fddriver,	 0,    0, (caddr_t)0x3f2,	 SPL5,  6, fdintrs,
				  (caddr_t)0x3f2,  6},
#else
	/* Holds place for Devs reference */
	{0,	 	 0,    0, (caddr_t)0x0,		 SPL0,  0, 0,
				  (caddr_t)0x0,    0},
#endif
};

struct	isa_dev	Devs[] = {
	/*
	   driver unit ct sl aliv 	 csr     spl    pic dk flags intrs
					start    len type fwdp ctp
	 */

#if NHD > 0
	{&hddriver,  0, 0, 0, 0, (caddr_t)0x104, SPL5, 14, 0, 0, hdintrs,
					(caddr_t)0x1f0, 8, 0, 0, &Ctlrs[0]},
	{&hddriver,  1, 0, 1, 0, (caddr_t)0x118, SPL5, 14, 1, 0, hdintrs,
					(caddr_t)0x1f0, 8, 0, 0, &Ctlrs[0]},
#endif

#if NFD > 0
	{&fddriver,  0, 0, 0, 0, (caddr_t)0x3f2, SPL5, 6, 0, 0, fdintrs,
					(caddr_t)0x3f2, 6, 0, 0, &Ctlrs[1]},
	{&fddriver,  1, 0, 1, 0, (caddr_t)0x3f2, SPL5, 6, 1, 0, fdintrs,
					(caddr_t)0x3f2, 6, 0, 0, &Ctlrs[1]},
#endif

#if NPC586 > 0
	/* For MACH Default - Warning: collides with wd0 */
	{&pcdriver, 0, -1, 0, 0, (caddr_t)0x0d0000, SPL6,  9, 0, 0, pc586intrs,
				 (caddr_t)0x0d0000, 0, 0, 0, 0},
	/* For Factory Default */
	{&pcdriver, 1, -1, 0, 0, (caddr_t)0x0c0000, SPL6,  5, 0, 0, pc586intrs,
				 (caddr_t)0x0c0000, 0, 0, 0, 0},
	/* For what Intel Ships */
	{&pcdriver, 2, -1, 0, 0, (caddr_t)0xf00000, SPL6, 12, 0, 0, pc586intrs,
				 (caddr_t)0x0f0000, 0, 0, 0, 0},
#endif

#if NWD8003 > 0
	/* Warning: collides with pc0 */
	{&wd8003driver, 0, -1, 0, 0, (caddr_t)0x280, SPL6, 9, 0, 0, wd8003intrs,
				      (caddr_t)0x0d0000, 0, 0,  0, 0},

	{&wd8003driver, 1, -1, 0, 0, (caddr_t)0x260, SPL6, 7, 0, 0, wd8003intrs,
				      (caddr_t)0x0d8000, 0, 0,  0, 0},

	{&wd8003driver, 2, -1, 0, 0, (caddr_t)0x240, SPL6, 5, 0, 0, wd8003intrs,
				      (caddr_t)0x0e0000, 0, 0,  0, 0},
#endif

#if NAT3C501 > 0
	/* Warning: collides with wt0 */
	{&at3c501driver,0,-1, 0, 0, (caddr_t)0x300, SPL6, 9, 0, 0, at3c501intrs,
				      (caddr_t)0x300, 0, 0,  0, 0},
#endif

#if NCOM > 0
	{&comdriver, 0, -1, 0, 0,(caddr_t)0x3f8, SPLTTY, 4, 0, 0, comintrs,
					 (caddr_t) 0x3f8, 8, 0, 0, 0},
	{&comdriver, 1, -1, 0, 0,(caddr_t)0x2f8, SPLTTY, 3, 0, 0, comintrs,
					 (caddr_t) 0x2f8, 8, 0, 0, 0},
	{&comdriver, 2, -1, 0, 0,(caddr_t)0x3e8, SPLTTY, 5, 0, 0, comintrs,
					 (caddr_t) 0x3e8, 8, 0, 0, 0},
	{&comdriver, 3, -1, 0, 0,(caddr_t)0x2e8, SPLTTY, 6, 0, 0, comintrs,
					 (caddr_t) 0x2e8, 8, 0, 0, 0},
#endif

#if NWT > 0
	/* Warning: collides with numerous addresses and pic's */
	{ &wtdriver, 0, 0, 0, 0, (caddr_t)0x300, SPL5, 5, 0, 0, wtintrs,
				      (caddr_t)0x300, 2, 0,  0, 0},
/*
	{ &wtdriver, 0, 0, 0, 0, (caddr_t)0x288, SPL5, 5, 0, 0, wtintrs,
				      (caddr_t)0x288, 2, 0,  0, 0},
	{ &wtdriver, 0, 0, 0, 0, (caddr_t)0x338, SPL5, 5, 0, 0, wtintrs,
				      (caddr_t)0x288, 2, 0,  0, 0},
*/
#endif

	{(struct isa_driver *) 0}
};

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
	struct	isa_dev	*dev_p;
	struct	isa_driver	*drv_p;

	for (dev_p = Devs; drv_p = dev_p->dev_driver; dev_p++) {
		struct isa_ctlr *ctl_p = dev_p->dev_mi;
#ifdef	DEBUG
		printf("%s%d\n", drv_p->driver_dname, dev_p->dev_unit);
#endif
		if ((int)ctl_p && !ctl_p->ctlr_alive) 
			if (drv_p->driver_probe(ctl_p))
				ctl_p->ctlr_alive = 1;
			else
				continue;

		if (((int)ctl_p ? drv_p->driver_slave :	drv_p->driver_probe)(dev_p)) {
			dev_p->dev_alive = 1;
			drv_p->driver_attach(dev_p);
		}
	}

}

	/*
	 * slave_config is a temporary artifact which will go away as soon
	 * as kern/slave.c is made conditional on NCPUS > 1
	 */
slave_config()
{
}
