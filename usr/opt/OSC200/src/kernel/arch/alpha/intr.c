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
static char *rcsid = "@(#)$ $ (DEC) $";
#endif

#include <sys/types.h>
#include <vm/vm_kern.h>
#include <machine/scb.h>
#include <sys/table.h>

#include <sys/time.h>

#include <dec/binlog/errlog.h>
#include <sys/syslog.h>

int atintr_level = 0;

#if NCPUS > 1
volatile unsigned long  system_intr_cnts_level[NCPUS][INTR_MAX_LEVEL];
volatile unsigned long  system_intr_cnts_type[CPUS]INTR_TYPE_SIZE];
#else	/* NCPUS */
volatile unsigned long  system_intr_cnts_level[INTR_MAX_LEVEL];
volatile unsigned long  system_intr_cnts_type[INTR_TYPE_SIZE];
#endif	/* NCPUS */
volatile int *system_intr_cnts_type_transl;

/*
 * I/O interrrupt handler...
 */

u_long
ev4_intr(parameter)
	struct scbentry *parameter; /* address of int handler in Xscb */
{
	u_long (*handler)();	/* user handler	*/
	u_long param;		/* user's parameter */
	
	/* is this to expensive to justify it's purpose		*/
	incr_interrupt_counter_level(mfpr_ipl());
	/* could call directly but this allows debugging	*/
	handler = parameter->scb_vector;
	param = parameter->scb_param;
	(*handler)(param);
}

/*
 * Stray interrupt handler
 */
stray(ef,vector)
	u_long *ef;			/* pointer to the exception frame */
	u_long vector;			/* scb vector of stray interrupt  */
{
	long present_ipl;

	incr_interrupt_counter_type(INTR_TYPE_STRAY);
	present_ipl = getspl();
        log(LOG_WARNING, "intr: stray interrupt:\tpresent_ipl:0x%x\tvector:0x%x\n\r", 
	    present_ipl, vector);
}

extern _scb, _scbend;

/*
 * allocate a vector from the scb for an I/O device 
 */
kern_return_t
allocvec(nvec, vecaddr)
	int nvec;
	vm_offset_t *vecaddr;
{
	static u_short nextfree = 0x800;    /* arch defined start I/O ints */
	static u_short lastfree;

	lastfree = (u_long)&_scbend - (u_long)&_scb;

	if((nextfree + sizeof(struct scbentry)*nvec) < lastfree) {
		*vecaddr = (vm_offset_t)(((u_long)&_scb) + nextfree);
		nextfree += sizeof(struct scbentry) * nvec;
		return(KERN_SUCCESS);
	} else
		return(KERN_RESOURCE_SHORTAGE);
}

/* return the 16 bit vector offset into the scb */
u_short
vecoffset(vecaddr)
	vm_offset_t *vecaddr;
{
	return((u_short)((u_long)vecaddr - (u_long)&_scb));
}

/*
 * This routine sets up the interrupt dispatching... Using an
 * interrupt vector, a routine, and a parameter. For the
 * current implementation, this stuffs the data into the SCB. for
 * future ECOs the scb may become a data structure used by operating
 * system code to dispatch interrupts...
 */
void
intrsetvec(vector, handler, parameter)
	u_short vector;         /* scb vector to put handler address in. */
	u_long (*handler)();    /* address to place in scb */
	u_long parameter;       /* parameter to place in scb */
{
	struct scbentry *scbp;

	scbp = (struct scbentry *)((u_long)&_scb + vector);
	scbp->scb_vector = handler;
	scbp->scb_param = parameter;
}

void
dumpvec(offset)
	u_short offset;
{
	struct scbentry *scbp;

	scbp = (struct scbentry *)((u_long)&_scb + offset);
	printf("dumpvec: offset 0x%x scb.vector 0x%l016x scb.param 0x%l016x ",
	       offset, scbp->scb_vector, scbp->scb_param);
}

