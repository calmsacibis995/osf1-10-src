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
 * @(#)$RCSfile: pfcntr.h,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/12/15 22:12:04 $
 */
#include <sys/types.h>
#include <sys/ioctl.h>

#ifndef _PFCNTR_H_
#define _PFCNTR_H_

/*
 * Sparse definition of the iccsr register.  Only the bits we can actually
 * change are set.  The rest are in `ignore' fields.
 */
struct iccsr {				/* ev4 iccsr register encoding */
	ulong	iccsr_pc1:1,
		iccsr_ign0:2,
		iccsr_pc0:1,
		iccsr_ign1:4,
		iccsr_mux0:4,
		iccsr_ign2:20,
		iccsr_mux1:3,
		iccsr_disable:2,
		iccsr_ign3:27;
};

/*
 * These are the driver's counts of interrupts not the chip's
 * count of events.
 */
struct pfcntrs {
	ulong	pf_cntr0;		/* driver's counter 1		*/
	ulong	pf_cntr1;		/*   ""		""  2		*/
	ulong	pr_cycle;		/* process cycle counter	*/
};

/*
 * ioctl's
 */
#define PCNTRDISABLE	_IO('P', 0)	/* disable the performance counters */
#define PCNTRENABLE	_IO('P', 1)	/* enable the performance counters */
#define PCNTSETMUX	_IOW('P', 2, struct iccsr)	/* set mux bits	*/
#define PCNTLOGALL	_IO('P', 3)	/* count all processes		*/
#define PCNTLOGSELECT	_IO('P', 4)	/* count only those with enable set */
#define PCNTCLEARCNT	_IO('P', 5)	/* clear driver counters	*/
#define PCNTGETCNT	_IOR('P', 6, struct pfcntrs)	/* read the drv cntrs */
#define PCNTSETSAMPLE	_IO('P', 7)	/* Collect PC samples		*/
#define PCNTCLRSAMPLE	_IO('P', 8)	/* Stop Collecting PC samples	*/
#define PCNTSETHISTO	_IO('P', 9)	/* Collect PC samples		*/
#define PCNTGETRSIZE	_IOR('P', 10, long)	/* get ring size	*/
#define PCNTGETIPLHIS	_IOR('P', 11, ulong[9])	/* collect IPL histograms */

/*
 * The following are EV4 specific multiplexor definations
 *
 * mux0 definitions
 */
#define PF_ISSUES	0x0	/* total issues/2			*/
#define PF_PIPEDRY	0x2	/* pipeline dry, ie lack of valid i-str	*/
#define PF_LOADI	0x4	/* load instructions			*/
#define PF_PIPEFROZEN	0x6	/* pipeline frozen, resource conflicts	*/
#define PF_BRANCHI	0x8	/* branch, and function calls		*/
#define PF_CYCLES	0xa	/* total cycles				*/
#define PF_PALMODE	0xb	/* cycles while executing palcode	*/
#define PF_NONISSUES	0xc	/* non_issues / 2			*/
#define PF_EXTPIN0	0xe	/* external pin, platform specific	*/

/*
 * mux1 definitions
 */
#define PF_DCACHE	0x0	/* data cache misses			*/
#define PF_ICACHE	0x1	/* instruction cache misses		*/
#define PF_DUAL		0x2	/* dual issue				*/
#define PF_BRANCHMISS	0x3	/* branch mispredictions		*/
#define PF_FPINST	0x4	/* floating point operate instructions	*/
#define PF_INTOPS	0x5	/* integer operations			*/
#define PF_STOREI	0x6	/* store instructions			*/
#define PF_EXTPIN1	0x7	/* external pin, platform specific	*/


#ifdef _KERNEL
/*
 * wrperfmon functions
 */
#define PFDISABLE	0
#define PFENABLE	1
#define PFSET		2
#define PFOPT		3
#endif /* _KERNEL */

#endif /* _PFCNTR_H_ */
