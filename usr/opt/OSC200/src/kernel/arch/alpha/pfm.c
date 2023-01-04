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
static char *rcsid = "@(#)$RCSfile: pfm.c,v $ $Revision: 1.1.7.6 $ (DEC) $Date: 1993/08/24 13:03:10 $";
#endif
/*
 * Alpha performance monitor psuedo-device
 *
 * This file contains support for the alpha performance counters
 */


#include "pfm.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

#include <vm/vm_kern.h>
#include <sys/pfcntr.h>
#include <machine/scb.h>
#include <machine/psl.h>
#include <machine/pcb.h>

#define PFCNTROFSET 0x650		/* scb offset for performace cntrs */

/*
 * Current state
 */
struct pfmstate {
	ulong	pfm_counter[2];		/* counters		*/
	ulong	pfm_cycles;		/* proc cycle counter	*/
	ulong	pfm_iccsr;		/* last state we set	*/
	ulong	*pfm_samplering;	/* sample ring		*/
	ulong	pfm_ringsize;		/* size of the ring	*/
	ulong	pfm_head;		/* head index		*/
	ulong	pfm_tail;		/* tail index		*/
	ulong	pfm_ipl[9];		/* ipl histogram	*/
	int	pfm_state;		/* state flag		*/
	int	pfm_disabled;		/* 0, 1, 2, or 3	*/
} pfmstate = { 0, 0, 0, NULL, 0, 0 };

#define PFM_RINGENTS    (8192*8)
#define PFM_RINGSIZE    (PFM_RINGENTS*sizeof(long)) /* size in bytes */

extern unsigned long _etext[], eprol[];
extern long rpcc();
unsigned long pfmetext = (unsigned long)_etext;
unsigned long pfmftext = (unsigned long)eprol;
/*
 * Driver state can be closed or open for either counters or
 * pc samples, counters, and histogram (pc profiling)
 */
#define PFM_COUNTERS	1	
#define PFM_SAMPLES	2
#define PFM_HISTOGRAM	3

/*
 * Called through the cdevsw open table
 */
/* ARGSUSED */
int
pfmopen()
{
	extern struct scbentry _scb[];		/* the scb	*/
	extern void pfmintr();			/* the interrupt handler */

	if( pfmstate.pfm_state )
		return EBUSY;
	else {
		/*
		 * put things is a known state
		 */
		wrperfmon( PFDISABLE );		/* off	*/
		wrperfmon( PFOPT, 0 );		/* log none */
		/*
		 * Point the SCB to our code.
		 */
		_scb[ PFCNTROFSET/16 ].scb_vector = (ulong (*)())pfmintr;
		pfmstate.pfm_state = PFM_COUNTERS;
		pfmstate.pfm_disabled = 3;	/* disable both counters */
		return (0);
	}
}

/*
 * Shutdown the counters
 */
int
pfmclose()
{
	extern struct scbentry _scb[];	/* the scb      */
	extern void stray();		/* the interrupt handler */

	
	wrperfmon( PFDISABLE );
	wrperfmon( PFOPT, 0 );		/* log none */
	/*
	 * Point the SCB to stray()
	 */
	_scb[ PFCNTROFSET/16 ].scb_vector = (ulong (*)())stray;
	pfmstate.pfm_state = 0;
	pfmstate.pfm_disabled = 3;	/* disable both counters */
	/*
	 * return any memory we've used
	 */
	if( pfmstate.pfm_samplering != NULL ){
		kmem_free(kernel_map, pfmstate.pfm_samplering, pfmstate.pfm_ringsize);
		pfmstate.pfm_samplering = NULL;
	}
	return (0);
}

/*
 * Give the user the current set of samples
 */
int
pfmread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	int error = 0;
	register u_int c;		/* number of bytes to be read	*/
	register u_int actual;		/* actual number to be sent	*/
	register u_int avail;		/* available ring entries	*/

	register u_long start=pfmstate.pfm_tail, end=pfmstate.pfm_head;
	register struct iovec *iov;

	uio->uio_rw = UIO_READ;
	if( pfmstate.pfm_state ==  PFM_SAMPLES ) {
		/*
		 * Figure out how many entries are available
		 */
		if( start <= end )
			avail = end - start;
		else 	
			avail = PFM_RINGENTS - (start - end);

		while (uio->uio_resid > 0 && error == 0 && avail > 0) {
			iov = uio->uio_iov;
			if (iov->iov_len == 0) {
				uio->uio_iov++;
				uio->uio_iovcnt--;
				if (uio->uio_iovcnt < 0)
					panic("pfmread");
				continue;
			}

			c = iov->iov_len;
			/*
			 * actual # entries is either here to end of ring
			 * or start of ring to here.
			 */
			if( start <= end )
				actual = end - start;
			else
				actual = PFM_RINGENTS - start;

			if( c/sizeof(long) > actual )
				c = actual * sizeof(long);
			else
				actual = c/sizeof(long);

			error = copyout(&pfmstate.pfm_samplering[start],
					iov->iov_base, c);

			iov->iov_base += c;
			iov->iov_len -= c;
			uio->uio_offset += c;
			uio->uio_resid -= c;

			avail -= actual;
			start += actual;
			if( start >= PFM_RINGENTS )
				start = 0;
		}
		pfmstate.pfm_tail = start;
	} else if(  pfmstate.pfm_state == PFM_HISTOGRAM ){
		start = 0;
		avail = pfmstate.pfm_ringsize;
		while (uio->uio_resid > 0 && error == 0 && avail > 0) {
			iov = uio->uio_iov;
			if (iov->iov_len == 0) {
				uio->uio_iov++;
				uio->uio_iovcnt--;
				if (uio->uio_iovcnt < 0)
					panic("pfmread");
				continue;
			}

			c = iov->iov_len;
			if( c > avail )
				c = avail;
			error = copyout(&pfmstate.pfm_samplering[start],
					iov->iov_base, c);

			iov->iov_base += c;
			iov->iov_len -= c;
			uio->uio_offset += c;
			uio->uio_resid -= c;
			avail -= c;
			start += c/sizeof(long);
		}
	}
	return (error);
}
/*
 * This routine is called through the cdevsw[]
 * ioctl interface
 */
/* ARGSUSED */
int
pfmioctl(dev, cmd, data, flag)
	dev_t dev;
	unsigned int cmd;
	caddr_t data;
	int flag;
{
	register struct iccsr *iccsr = (struct iccsr *)data;

	/*
	 * Process commands
	 */
	switch( cmd ){

	case PCNTRDISABLE:			/* turn off interrupts	*/
		wrperfmon( PFDISABLE );
		break;

	case PCNTRENABLE:			/* turn on interrupts	*/
		wrperfmon( PFENABLE );
		break;

	case PCNTSETMUX:			/* select counters	*/
		if (iccsr->iccsr_disable == 3)
			return (EINVAL);	/* Can't have both turned off */
		pfmstate.pfm_disabled = iccsr->iccsr_disable;
		iccsr->iccsr_disable = 0;	/* Don't pass these bits to call_pal */
		pfmstate.pfm_iccsr = *(ulong *)iccsr;
		wrperfmon( PFSET, pfmstate.pfm_iccsr );
		break;

	case PCNTLOGALL:			/* all processes	*/
		wrperfmon( PFOPT, 1 );
		break;

	case PCNTLOGSELECT:			/* processes with pme set */
		 wrperfmon( PFOPT, 2 );
		 /* Set our PME bit in PCB */
		 current_pcb->pcb_fen |= PCB_PME_BIT;
		 break;

	case PCNTCLEARCNT:			/* clear the interrupt cntrs */
		pfmstate.pfm_counter[0] = 0;
		pfmstate.pfm_counter[1] = 0;
		bzero (pfmstate.pfm_ipl, sizeof(pfmstate.pfm_ipl));
		pfmstate.pfm_cycles = rpcc();
		break;

	case PCNTGETCNT:			/* read the interrupt cntrs */
		((ulong *)data)[0] = pfmstate.pfm_counter[0];
		((ulong *)data)[1] = pfmstate.pfm_counter[1];
		((ulong *)data)[2] = pfmstate.pfm_cycles;
		break;

	case PCNTGETIPLHIS:			/* read the IPL histogram */
		bcopy (pfmstate.pfm_ipl, data, sizeof(pfmstate.pfm_ipl));
		break;

	case PCNTSETSAMPLE:			/* setup pc samples	*/

		/*
		 * No samples or pc histograms while we free memory
		 */
		pfmstate.pfm_state = PFM_COUNTERS;
		/*
		 * Deallocate the old memory and allocate new
		 * ring
		 */
		if( pfmstate.pfm_samplering != NULL ){
			kmem_free(kernel_map, pfmstate.pfm_samplering,
				pfmstate.pfm_ringsize);
			pfmstate.pfm_samplering = NULL;
		}
		pfmstate.pfm_ringsize = PFM_RINGSIZE;
		pfmstate.pfm_samplering = (ulong*)kmem_alloc(
			kernel_map, pfmstate.pfm_ringsize);
		if( pfmstate.pfm_samplering == NULL )
			return (ENOMEM);
		pfmstate.pfm_head = pfmstate.pfm_tail = 0;
		/*
		 * Okay, now we can take pc samples
		 */
		pfmstate.pfm_state = PFM_SAMPLES;
		break;
		
	case PCNTCLRSAMPLE:
		pfmstate.pfm_state = PFM_COUNTERS;
		break;

	case PCNTSETHISTO:
		/*
		 * No samples or pc histograms while we free memory
		 */
		pfmstate.pfm_state = PFM_COUNTERS;
		/*
		 * Deallocate the old memory and allocate new
		 * ring
		 */
		if( pfmstate.pfm_samplering != NULL ){
			kmem_free(kernel_map, pfmstate.pfm_samplering,
				pfmstate.pfm_ringsize);
			pfmstate.pfm_samplering = NULL;
		}
		pfmstate.pfm_ringsize = (long)(pfmetext - pfmftext);
		pfmstate.pfm_samplering = (ulong*)kmem_alloc(
			kernel_map, pfmstate.pfm_ringsize);
		if( pfmstate.pfm_samplering == NULL )
			return (ENOMEM);
		pfmstate.pfm_head = 0;
		pfmstate.pfm_tail = pfmstate.pfm_ringsize/sizeof(long);
		/*
		 * Okay, now we can take samples
		 */
		pfmstate.pfm_state = PFM_HISTOGRAM;
		break;

	case PCNTGETRSIZE:
		*(ulong *)data = pfmstate.pfm_ringsize;
		break;

	default:
		return (EINVAL);
	}
	return 0;
}

/*
 * Process counter interrupts
 */
void
pfmintr( cntr, pc, ps )
ulong cntr, pc, ps;
{
	/*
	 * If we've "disabled" this counter, ignore it
	 */
	if (pfmstate.pfm_disabled & (1<<cntr))
		return;

	/*
	 * update the process cycle counter stuff
	 */
	pfmstate.pfm_counter[cntr]++;
	pfmstate.pfm_cycles = rpcc();

	/* Bump the appropriate IPL histogram entry */
	/* Note: IPL6 and IPL7 are always empty (this intr=IPL6) */
	/* We use IPL7 to tally "idle" ticks, a subset of IPL0 */
	pfmstate.pfm_ipl[ps & 0xf]++;
	if (current_thread()->state & TH_IDLE && BASEPRI(ps))
		pfmstate.pfm_ipl[7]++;

	/*
	 * Collect user mode samples or kernel mode pc histograms
	 */
	if( pfmstate.pfm_state == PFM_SAMPLES && ps == 8 ){
		pfmstate.pfm_samplering[pfmstate.pfm_head++] = pc | cntr;
		if(pfmstate.pfm_head >= PFM_RINGENTS)
			pfmstate.pfm_head = 0;
		/* If the ring has wrapped, bump tail pointer */
		if (pfmstate.pfm_head == pfmstate.pfm_tail) {
			pfmstate.pfm_tail++;
			if(pfmstate.pfm_tail >= PFM_RINGENTS)
				pfmstate.pfm_tail = 0;
		}
	} else if( pfmstate.pfm_state == PFM_HISTOGRAM && ps != 8){
		/*
		 * If it's kernel mode
		 */
		register uint *ptr = (uint *)pfmstate.pfm_samplering;

		ptr[ (pc-pfmftext)/sizeof(int) ]++;
	}
}
