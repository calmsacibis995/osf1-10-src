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
static char *rcsid = "@(#)$RCSfile: tlb_trap.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/07/08 08:50:39 $";
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
/* 
 * derived from trap.c	4.10	(ULTRIX)	12/19/90";
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/***********************************************************************
 *
 *	(Old,relevant) Modification History from trap.c
 *
 * 10-Oct-91    Marian Macartney
 *   Cleanup definitions from BL6.  Fix compiler warnings.
 *
 * 22-Jun-91    Marian Macartney
 *   Merged in OSF-1.0.1 changes.  Added reference bit implementation 
 *   which is set via TLBMISS instead of uTLBMISS.  Note that this merge 
 *   includes some definitions normally put in pmap.h - which will have 
 *   to go back there in the next BL.
 *
 * 21-Apr-91	Ron Widyono
 *	Change kast_preemption, hack_preemption, original_preemption, 
 *      pp_count, ast_was_on to rt_preempt_async, rt_preempt_syscall,
 *      rt_preempt_user, rt_preempt_pp.  Remove some syscall preemption 
 *      debugging.
 *	Remove kernel_ast.
 *
 * 19-Apr-91	Ron Widyono
 *	Always include parallel.h.
 *
 * 19-Oct-89 -- jmartin
 *	Add TLBMISS_STUCK code to panic if retrying page fault endlessly.
 *
 * 17-Nov-1988  depp
 *	Added addition memory page protection to tlbmiss(), and tlbmod().
 *
 ***********************************************************************/

int tlb_trap_debug = 0;
#define	lock_pmap(pmap)		simple_lock(&(pmap)->lock)
#define	unlock_pmap(pmap)	simple_unlock(&(pmap)->lock)

#define TRACE(x)

#define CNT_TLBMISS_HACK 0


#include <pmap_pcache.h>
#include <ref_bits.h>
#include <rt_preempt.h>
#include <rt_preempt_debug.h>

#include <sys/secdefines.h>
#include <sys/uswitch.h>

#if SEC_BASE
#include <sys/security.h>
#include <kern/lock.h>
#include <sys/audit.h>
#endif /* SEC_BASE */

#include <machine/cpu.h>
#include <mach/exception.h>
#include <builtin/ux_exception.h>
#include <mach/machine/thread_status.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>
#include <kern/parallel.h>
#if	RT_PREEMPT
#include <kern/ast.h>
#include <sys/preempt.h>
#if	RT_PREEMPT_DEBUG
#include <mach/time_value.h>
#include <mach/boolean.h>
#include <kern/processor.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/queue.h>
#endif	/* RT_PREEMPT */
#endif	/* RT_PREEMPT_DEBUG */

#include <machine/pmap.h>
#include <machine/reg.h>
#include <machine/inst.h>


#if	RT_PREEMPT && RT_PREEMPT_DEBUG
extern int	rt_preempt_syscall;
extern int	rt_preempt_user;
extern int	rt_preempt_async;
extern int	rt_preempt_ast_set;
extern int	rt_preempt_pp;
#endif

extern int runrun;
extern int k_puac;

#if	REF_BITS
#define clear_refclu_bit(pte)					\
      {                                                         \
       lock_pmap(pmap);						\
       ((pte_template *)(pte))->raw = (((pte_template *)(pte))->raw & ~PG_REFCLU) | PG_V; \
       unlock_pmap(pmap);                                        \
       }

#endif	/* REF_BITS */

#ifdef TLBMISS_STUCK
	/* must be power of 2 */
#define VADDR_RING_SIZE (1<<3)
u_int vaddr_ring[VADDR_RING_SIZE];
int vaddr_ring_index = 0;
#endif /* TLBMISS_STUCK */

tlbmiss(ep, code, vaddr, user_fault)
u_int *ep;
vm_offset_t vaddr;
u_int code, user_fault;
{
	register pt_entry_t *pte;
	vm_offset_t user_address;
	pmap_t pmap;
	kern_return_t ret;
	extern void pmap_pte_fault();

	/*
	 * Workaround for 3.0/4.0 chip bug.  If badvaddr has been trashed
	 * by chip, epc is correct vaddr
	 */
	if (vaddr == E_VEC) {
		vaddr = ep[EF_EPC];
	}

#ifdef TLBMISS_STUCK
		{
		   int i;

		   vaddr_ring[vaddr_ring_index++ & (VADDR_RING_SIZE-1)] = vaddr;
		   for (i=0; i < VADDR_RING_SIZE-1; i++)
		       if (vaddr_ring[i] != vaddr_ring[i+1])
			   break;
		   if (i == VADDR_RING_SIZE-1) {
                       printf("vaddr==%8x\n", vaddr);
		       panic("tlbmiss: stuck on same vaddr");
		   }
		 }
#endif TLBMISS_STUCK

	if (vaddr >= KPTEADDR) {

		/*
		 * Miss on the kernel's page tables.
		 */

		TRACE({printf("(%cKP %x)", (code==EXC_WMISS) ? 'W' : 'R', vaddr);})

		pmap = kernel_pmap;

		pte = pmap_root_l1pte(pmap,vaddr);	/* get l1pte */
	
		if (!pte->pg_v)				/* is it valid ? */
			pmap_pte_fault(pmap, pte); 	/* no, fault it in */
	
		tlb_map_random(0, vaddr, *(int*)pte);	/* fill tlb */

		return 0;				/* return */

	} else if (vaddr >= VM_MAX_KERNEL_ADDRESS) {

		/*
		 * Miss on a user page table
		 */

		TRACE({printf("(%cUP %x)", (code == EXC_WMISS) ? 'W' : 'R', vaddr);})

		pmap = active_pmap;

		if (ep[EF_EPC] >= E_VEC) {

			/*
			 * We get here when the UPTE was referenced 
			 * outside of the UTLB_MISS handler. 
			 */

			TRACE({printf("epc= 0x%x, vaddr= 0x%x\n", ep[EF_EPC], vaddr);})

			pte = pmap_root_l1pte(pmap,vaddr); /* get l1pte */ 
	
			if (!pte->pg_v)		   	   /* signal upper */
		  		return SEXC_SEGV;	   /* vm if invalid */	
#if PMAP_PCACHE
			{ register int i;
		  	lock_pmap(pmap); 
		  	i = pmap->pcache.index;
		  	pmap->pcache.data[i].vaddr = vaddr;
		  	pmap->pcache.data[i].pte = *pte;
			tlb_map(2 + i, pmap->pid, vaddr, *(int*)pte);  
		  	if (++i >= NPCACHE) i = 0;
		  		pmap->pcache.index = i;
		  	unlock_pmap(pmap);
			}
#else  /* PMAP_PCACHE */	
			tlb_map_random(pmap->pid, vaddr, *(int*)pte);
#endif /* PMAP_PCACHE */
			return 0;			   /* return */
		}

		/*
		 * The utlmiss handler missed.
		 * An unpleasant chip bug makes it uncertain whether
		 * a reported miss at address zero is really such.
		 * How exciting.
		 * And if you want to get really thrilled, think that on the
		 * pmax the utlbmiss handler is invoked even from kernel mode!
		 */

		code = user_fault & CAUSE_EXCMASK;
		user_address = (vaddr & TLBCTXT_VPNMASK) << (MIPS_PGSHIFT-2);
		if ((user_address == 0) && (code == EXC_RMISS)) {
					/* Q: why not WMISS for kernel */
			/*
			 * Now, if the (user) code was mapped allright it is
			 * definitely a bogus pointer, else it might
			 * be one of those chips that just screw up.
			 */
			vm_offset_t pc = trunc_page(ep[EF_K1]);

			TRACE({ printf("ctxtbug: %x %x %x %x\n", ep[EF_K1],
				ep[EF_EPC], vaddr, user_fault);})

			if (IS_KUSEG(pc) && (tlb_probe(pmap->pid, pc) == 0))
				user_address = pc;
		}

		/*
		 * Set the EPC and SR to return to the place that
		 * caused the original utlbmiss
		 */
		ep[EF_EPC] = ep[EF_K1];
		ep[EF_SR] &= ~(SR_KUP|SR_IEP);
		ep[EF_SR] |= (ep[EF_SR] & (SR_KUO|SR_IEO)) >> 2;
		ep[EF_BADVADDR] = user_address;

		/* begin optional System V null ptr behavior */

		if (user_address < VM_MIN_ADDRESS)
		{
			if (user_address >= 0 && user_address <= (PAGE_SIZE-1)
			 	&& emulate_null())
			{
				ret = emulate_sysv_null(ep, user_fault);
				if (ret == KERN_SUCCESS)
					return(KERN_SUCCESS);
				else
					return(SEXC_SEGV);
			}
			else
				return(SEXC_SEGV);
		}
		/* end   optional System V null ptr behavior */

		pte = pmap_root_l1pte(pmap, vaddr);	/* get l1pte */

		if (!pte->pg_v)				/* signal upper */
		  	return SEXC_SEGV;		/* vm if invalid */
#if	PMAP_PCACHE
		{ register int i;
		  lock_pmap(pmap); /* uhmmm */
		  i = pmap->pcache.index;
		  pmap->pcache.data[i].vaddr = vaddr;
		  pmap->pcache.data[i].pte = *pte;
		  tlb_map(2 + i, pmap->pid, vaddr, *(int*)pte);   
		  if (++i >= NPCACHE) i = 0;
		  pmap->pcache.index = i;
		  unlock_pmap(pmap);
		}		
#else	/* PMAP_PCACHE */
		tlb_map_random(pmap->pid, vaddr, *(int*)pte);
#endif	/* PMAP_PCACHE */
		pte = pmap_l2pte(pte, user_address);

		if (!pte->pg_v) {			/* is it invalid ? */
#if REF_BITS
			if (pte->pg_refclu) {
				pmap_set_reference(PTETOPHYS(pte)); 
				clear_refclu_bit(pte);
		}
		else
#endif REF_BITS
		  		return SEXC_SEGV;	
		}

		if (code == EXC_WMISS && *(int *) pte & VM_PROT_WRITE) {
			pmap_set_modify(PTETOPHYS(pte));
			* (int *) pte |= PG_M;
		}

		tlb_map_random(pmap->pid, user_address, *(int*)pte);

		return 0;				/* return */
	
	} else if (vaddr >= VM_MIN_KERNEL_ADDRESS) {

		/*
		 * Miss on a kernel virtual address proper
		 */

		TRACE({printf("(%cKV %x)", (code == EXC_WMISS) ? 'W' : 'R', vaddr);})

		pmap = kernel_pmap;
	
		pte = pmap_l1pte(pmap, vaddr);		/* get l1pte */

		if (!pte->pg_v)				/* is it valid ? */
			return SEXC_SEGV;		/* no, signal upper VM */

		pte = pmap_l2pte(pte, vaddr);		/* get l2pte */

		if (!pte->pg_v) {			/* is it invalid ? */
#if REF_BITS
                        if (pte->pg_refclu) {
				pmap_set_reference(PTETOPHYS(pte)); 
				clear_refclu_bit(pte);
			    }
			else 				
#endif /* REF_BITS */		
			return SEXC_SEGV;
		      }

		tlb_map_random(0, vaddr, *(int*)pte);  	/* fill tlb */ 

		return 0;				/* return */

	} else {
	
		/*
		 * If we get here it's because the utlbmiss handler did
		 * find the pte for the page, but it was invalid.
		 * Therefore we go straight to the VM system.
		 */

#if REF_BITS
		pmap = active_pmap;
		pte = pmap_l1pte(pmap, vaddr);
		pte = pmap_l2pte(pte, vaddr);
		if (pte->pg_refclu) {
			pmap_set_reference(PTETOPHYS(pte)); 
			clear_refclu_bit(pte);
			tlb_map_random(pmap->pid, vaddr, *(int*)pte);
	return 0;
		}
#endif /* REF_BITS */		

		return SEXC_SEGV;		/*  signal upper VM */

	}
}

tlbmod(ep, code, vaddr, cause)
u_int *ep;
register vm_offset_t vaddr;
{
	register pmap_t map;
	register pt_entry_t *pte;

	TRACE({printf("tlbmod x%x x%x x%x x%x\n", ep, code, vaddr, cause);})

	if (vaddr >= K2BASE) map = kernel_pmap;
	else map = active_pmap;

	pte = pmap_l1pte(map, vaddr);
	pte = pmap_l2pte(pte, vaddr);

	if ((pte->pg_prot & VM_PROT_WRITE) == 0)
		return EXC_WMISS;

	pte->pg_m = 1;
	pmap_set_modify( PTETOPHYS(pte));
	tlb_modify(map->pid, vaddr, 1/*writeable*/);
	return 0;
}
/*---------------------------------------------------------------*/
/*
 * Name:
 *	emulate_sysv_null()
 *
 * Description:
 *	In System V, a load instruction executed from user space with a NULL 
 *	base pointer will not cause a SIGSEGV.  Instead, 0 will be returned
 *	to the user.   
 * Calls:
 *	emulate_branch()
 *
 * Called by:
 *	trap()
 *
 * Inputs:
 *	ep	- pointer to exception frame
 *
 * Outputs:
 *	Returns KERN_FAILURE if no emulation is required or if something
 *	goes wrong. A successful emulation returns KERN_SUCCESS.
 *
 * Side effects:
 *	Store 0 in the exception frame for the appropriate register. If
 *	not in the branch delay slot, store epc += 4 in the exception frame
 *	for return address.  If in the branch delay slot, invoke emulate_branch
 *	to calculate next instruction.
 */
kern_return_t
emulate_sysv_null(ep, cause)

register u_int *ep;
u_int		cause;

{

	union mips_instruction 	inst, branch_inst;
	u_int 			new_epc;


	/*
	 * Find instruction that caused the trap. If we're in the
	 * branch delay slot, get the branch instruction too.
	 */
	if(cause & CAUSE_BD)
	{
	    branch_inst.word = fuiword((caddr_t)ep[EF_EPC]);
	    inst.word = fuiword((caddr_t)(ep[EF_EPC] + 4));
	}
	else
	{
	    inst.word = fuiword((caddr_t)ep[EF_EPC]);
	}

	/*
	 * See if a load instruction was responsible for the NULL ptr
	 * reference. If yes, calculate the new epc, else return
	 * KERN_FAILURE to let trap() know that it should deliver a
	 * SIGSEGV.
	 */
	if (inst.i_format.opcode >= lb_op && inst.i_format.opcode <= lwr_op)
	{
	    if (cause & CAUSE_BD)
	        new_epc = emulate_branch(ep, branch_inst.word, NULL);
	    else
    	        new_epc = ep[EF_EPC] + 4;
	}	
	else
	    return(KERN_FAILURE);

	/*
	 * Figure out which register we need to stuff with 0, find
	 * it in the exception frame and then assign it.
	 */
	ep[inst.i_format.rt+3] = 0;
	
	/* Set up new program counter*/
	ep[EF_EPC] = new_epc;	
	return(KERN_SUCCESS);
}
/*---------------------------------------------------------------*/
