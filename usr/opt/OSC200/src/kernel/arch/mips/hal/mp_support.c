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
static char *rcsid = "@(#)$RCSfile: mp_support.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:23:05 $";
#endif

/* 
 * Some oooooooooold ULTRIX comments about the following code
 *
 *
 * 06-Mar-90 gmm
 *      Added the routine alert_cpu() to update whichqs to ALLCPU under
 *      lk_rq.
 *
 * 14-Feb-90 -- gmm
 *      Avoid a race condition when starting more than one secondary
 *      processor by switching affinity of the idle_proc to boot cpu.
 *
 * 08-Dec-89 -- gmm
 *      Change the parent of the secondary's idle process to proc 0.
 *	When the idle process comes up on the secondary, it goes and 
 *	removes itself from the current parent's child queue and puts 
 *	itself on proc 0's child queue.
 *
 * 14-Nov-89 -- gmm
 *      Changes to secondary_startup(), new routine init_idleproc() 
 *	to start the idle process for the secondary, etc. 
 *	Stop_secondary_cpu() now dumps the tlbs in core for 
 *	debugging purposes.
 *
 * 09-Nov-89 -- jaw
 *      change references to maxcpu to smp.
 *
 * 13-Oct 89 -- gmm
 *      All MIPS specific smp changes. Changes to release_uarea_noreturn(),
 *      stop_secondary_cpu(), sig_parent_swtch(), new rotutines like
 *	intrcpu(), cpuident(), is_cpu(), secondary_startup() etc.
 *
 * 09-Jun-1989  gmm
 *      Call switch_to_idle_stack() in release_uarea_noreturn() before
 *      releasing the process's memory resources.
 *
 *
 */


/*
 * MP support module.  
 * The origin of this module's code is machdep.c.
 * Placed here for hw independence.
 */

#include <sys/proc.h>
#include <sys/user.h>

#include <machine/cpu.h>	/* to get tlb definitions below */
#include <hal/cpuconf.h>	/* for switch-based defines     */

extern int cpu;

/*
 * Satisfy the compiler?
 */
slave_config()
{
}


release_uarea_noreturn()
{

/* TODO: for now
        CURRENT_CPUDATA->cpu_exitproc = u.u_procp;
        (void)splclock();
        smp_lock(&lk_rq,LK_RETRY);
        swtch();
 *****/
}

 
struct second_tlb {                     /* dump tlb information */
        union tlb_hi tlb_high;		/* used in stop_secondary */
        union tlb_lo tlb_low;		/* for debug */
} second_tlb[NTLBENTRIES];


stop_secondary_cpu()
{
/*** TODO: stop_secondary_cpu is a null routine ***/
#ifdef notdef
        struct cpudata *pcpu;
        pcpu = CURRENT_CPUDATA;
        save_context();         /* save process's context */
/* What to do to flush write buffers */
        save_tlb(second_tlb); /* for debugging purpose */
        pcpu->cpu_status &= ~CPU_RUN;
        pcpu->cpu_status |= CPU_STOP;
        for(;;)  /* Something else ?? */
            ;
#endif /* notdef */
}



sig_parent_swtch()
{
/*** TODO: sig_parent_swtch is a null routine ***/
#ifdef notdef
        /* the caller to sig_parent_swtch() should have got lk_rq */
        CURRENT_CPUDATA->cpu_status |= CPU_SIGPARENT;
        swtch();
#endif /* notdef */
}


extern char *kn5800_ip[];
intrcpu(whichcpu)
int whichcpu;
{
	switch (cpu) {
              case DS_5800:
                *kn5800_ip[whichcpu] = 0; /* should be byte-type instr. */
                break;
              default:
                break;
        }
}

cpuident()
{
        switch (cpu) {
              case DS_5800:
                return(kn5800_cpuid());
             default:
                return(0);
       }
}


int start_stack_held = 0; /* indicates if any cpu using startup stack */
int current_secondary;    /* secondary cpu currently being started up */
struct proc *idle_proc;   /* idle proc of the currently starting cpu*/
extern struct user *boot_idle_up;
extern unsigned cputype_word, fptype_word;

secondary_startup()
{
/*** TODO: secondary_startup is a null routine ***/
#ifdef notdef
        int i;
        unsigned cputype,fptype;
        (void) splhigh();
        clear_bev();
        for (i=0; i < NTLBENTRIES; i++)
                invaltlb(i);

        tlbwired(TLBWIREDBASE, 0, UADDR,
        K0_TO_PHYS(boot_idle_up)>>(PGSHIFT-PTE_PFNSHIFT) | PG_M | PG_V | PG_G);
        u.u_pcb.pcb_cpuptr = cpudata[current_secondary];
        flush_cache();
        cputype = get_cpu_irr();
        if(cputype != cputype_word) {
                printf("WARNING: cpu %d version (0x%X) does not match with boot cpu version  (0x%X)\n",current_secondary,cputype,cputype_word);
        }
        fptype = get_fpc_irr();
        if( (fptype &= IRR_IMP_MASK) != fptype_word) {
                printf("WARNING: FPU version (0x%X) of cpu %d does not match with boot cpu's FPU version (0x%X)\n",fptype,current_secondary,fptype_w
ord);
        }
        fp_init();
        secondary_init();  /* cpu specific secondary initialization */
        splclock();  /* somebody lowers the ipl */
        smp_lock(&lk_rq,LK_RETRY);  /* synchs with the swtch of idle proc */
        CURRENT_CPUDATA->cpu_proc = idle_proc;
        init_tlbpid();
        remrq(idle_proc);
        get_tlbpid(idle_proc);
        smp_unlock(&lk_rq);
        idle_proc->p_cpumask = CURRENT_CPUDATA->cpu_mask;
        CURRENT_CPUDATA->cpu_status = CPU_RUN;
        startrtclock();
        resume(pcbb(idle_proc));    /* resumes the idle proc on the secondary.
                                       see init_idleproc() below */
#endif /* notdef */
}

secondary_init()
{
        /* should be through cpusw when more MP MIPS system become ready*/
        switch(cpu) {
              case DS_5800:
                kn5800_init_secondary();
                return;

              default:
                return;
        }
}


init_idleproc(cpunum)
{
/*** TODO: init_idleproc is a null routine ***/
#ifdef notdef
        int idlepid,found,s;
        register struct proc *cptr,*nq;

        while(start_stack_held)  /* if some other cpu
                            using the startup stack */
                sleep((caddr_t)&start_stack_held,PZERO+1);
        start_stack_held = 1;
        idlepid = get_proc_slot();
        if(idlepid == 0) {
                tablefull("proc");
                u.u_error = EAGAIN;
                start_stack_held = 0;
                wakeup((caddr_t)&start_stack_held);
                return(0);  /* get_proc_slot sets
                            u.u_error to EAGAIN */
        }
        current_secondary = cpunum;
        idle_proc = &proc[idlepid];
        if(newproc(idle_proc,0)) {
                switch_affinity(boot_cpu_mask); /* Avoid a race
                                   condition when more than 1 processor
                                   already running. Else the parent may
				   be put to sleep after the wakeup in the
				   child below */
                bcopy("idleproc",(caddr_t)&u.u_comm[0], MAXCOMLEN);
                idle_proc->p_affinity = 1<<current_secondary;
                idle_proc->p_type |= SSYS;
                idle_proc->p_sched |= SLOAD;
                splclock();
                smp_lock(&lk_rq,LK_RETRY);
                setrq(idle_proc);
                idle_proc->p_mips_flag |= SIDLEP;
                /* wake up the parent process, when
                   child proc is all set up */
                wakeup((caddr_t)idle_proc);
                if(save()) {
                        if(CURRENT_CPUDATA->cpu_num != current_secondary)
                                panic("idle proc not back on the correct secondary");
                        CURRENT_CPUDATA->cpu_idleproc = idle_proc;
                        s = splclock();
                        smp_lock(&lk_procqs,LK_RETRY);
                        cptr = idle_proc->p_pptr->p_cptr;
                        found = 0;
                        /* make swapper the parent */
                        while(cptr) {
                            if (cptr == idle_proc) {
                                   idle_proc->p_pptr->p_cptr = cptr->p_osptr;
                                   nq = idle_proc->p_osptr;
                                   if (nq != NULL)
                                        nq->p_ysptr = NULL;
                                   if(proc[0].p_cptr)
                                      proc[0].p_cptr->p_ysptr = idle_proc;
                                   idle_proc->p_osptr = proc[0].p_cptr;
                                   idle_proc->p_ysptr = NULL;
                                   proc[0].p_cptr = idle_proc;
                                   idle_proc->p_pptr = &proc[0];
                                   idle_proc->p_ppid = 0;
                                   found = 1;
                                   break;
                             }
                             cptr = cptr->p_osptr;
                        }
                        if(found == 0)
                                panic("init_idleproc: not found in child queue");
                        smp_unlock(&lk_procqs);
                        splx(s);
                        start_stack_held = 0;
                        wakeup((caddr_t)&start_stack_held);
                        idle();
                }
                start_idleproc(); /* start the primary's idle process */
       }
        /* make the parent sleep till child sets up the
           idle proc ready for the secondary cpu */
        sleep((caddr_t)idle_proc,PZERO-1);
        return(1); /* success */
#endif /* notdef */
}


alert_cpu()     /* inform every cpu that this process is available */
{
/*** TODO: alert_cpu is a null routine ***/
#ifdef notdef
        int s;
        s = splclock();
        smp_lock(&lk_rq,LK_RETRY);
        whichqs = ALLCPU;
        smp_unlock(&lk_rq);
        splx(s);
#endif /* notdef */
}
