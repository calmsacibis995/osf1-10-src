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
static char	*sccsid = "@(#)$RCSfile: kdb.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:10:58 $";
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
 * derived from kdb.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

#include <sys/param.h>
#include <sys/reboot.h>
#include <sys/systm.h>
#include <sys/vmmac.h>

#include <machine/pcb.h>
#include <machine/pmap.h>
#include <machine/reg.h>
#include <hal/entrypt.h>

#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <sys/user.h>		/* for u.u_procp  */
#include <sys/proc.h>
#include <kern/task.h>
#include <kern/thread.h>


/*
 *  kdb_init - initialize kernel debugger
 */

kdb_init()
{
	extern char end[];

	read_mips_symtab(end);
	kdb_enable();
}

/*
 *  kdb_enable - switch between kdb and dbgmon
 */
int use_kdb = 1;

kdb_enable()
{
	register struct restart_blk *rb = (struct restart_blk *)RESTART_ADDR;

	extern debugger_present, dbgmon;
	extern int kdb_breakpoint(), halt_cpu();

	if (use_kdb) {
		rb-> rb_bpaddr = kdb_breakpoint;
		debugger_present = 1;
	} else if (dbgmon != -1)
		/*
		 * For lack of better ideas, stick the
		 * address of breakpoint() in dbgmon by hand.
		 * This is such a GROSS HACK, and I am ashamed of it.
		 */
		rb-> rb_bpaddr = (int (*)()) 0xa00100e0;
	else
		rb-> rb_bpaddr = halt_cpu;
	init_restartblk();
}


/*
 *  kdb_kintr - received keyboard interrupt sequence
 *
 *  Queue software interrupt to transfer into the debugger at the
 *  first oppurtunity.  This is not done if we are already in the
 *  debugger or if the system was booted without the KDB boot flag.
 */

int kdbactive = 0;
extern short kdbsstep;

kdb_kintr()
{
    if (kdbactive == 0)
    {
	kdbsstep = 0;
	gimmeabreak();
    }
}


#define TRAP_TYPES 20
char	*kdb_trap_names[TRAP_TYPES] = {
/* Hardware trap codes */
	"Interrupt",
	"TLB mod",
	"TLB miss (read)",
	"TLB miss (write)",
	"Read Address Error",
	"Write Address Error",
	"Instruction Bus Error",
	"Data Bus Error",
	"Syscall",
	"Breakpoint",
	"Illegal Instruction",
	"Coprocessor Unusable",
	"Overflow",
	"13", "14", "15",
/* Software trap codes */
	"Segmentation Violation",
	"AST",
	"Illegal Instruction (SW)",
	"Coprocessor Unusable (SW)"
};

/*
 * kdb_nofault - dismiss an addressing fault to allow kdb to continue.
 */

kdb_nofault()
{
	kdberror("faulted within kdb -- continuing");
}

/*
 *  kdb_trap - field a BPT trap
 */

short kdbtrapflag;

kdb_trap(esp, flag)
register int *esp;
{
    extern struct pcb kdbpcb;
    register struct pcb *pcb = &kdbpcb;

    if (!kdbactive)
	    pm_prepare();	/* In case pm is the console... */

    if (flag) {
	    register int type = (esp[EF_CAUSE] & CAUSE_EXCMASK) >> CAUSE_EXCSHIFT;

	    dprintf("kernel: ");
	    if ((unsigned)type >= TRAP_TYPES)
		dprintf("type %d", type);
	    else
		dprintf("%s", kdb_trap_names[type]);
	    dprintf(" trap\n");
    }

    /* Don't invoke kdb recursively */
    if (kdbactive) {
	switch (esp[EF_CAUSE] & CAUSE_EXCMASK) {
	case EXC_RMISS:
	case EXC_WMISS:
	case EXC_RADE:
	case EXC_WADE:
	case EXC_IBE:
	case EXC_DBE:
	case SEXC_SEGV:
		esp[EF_EPC] = (int)kdb_nofault;
		return(flag);
	}
    }

    kdbtrapflag = flag;
    /*
     * If invoked from trap(), copy registers from
     * the exception frame to pcb so that user will
     * see the context that existed at the time
     * the exception occurred.
     */
    if (kdbtrapflag)
	save_exception_frame(&kdbpcb, esp);
    kdbactive++;
    cnpollc(TRUE);
    kdb( kdbsstep, esp, current_thread());
    cnpollc(FALSE);
    kdbactive--;
    if (kdbtrapflag)
	restore_exception_frame(esp, &kdbpcb);

out:
    {
	long pc = pcb->pcb_regs[PCB_PC];
	long ins = kdbget(pc, 1);

	if (isa_break(ins)) {	/* No loops! */
		pcb->pcb_regs[PCB_PC] += 4;
#if	DEBUG
		dprintf("[bp!%x]\n", pc);
#endif
	}
    }
    pm_restore();
    return(flag);
}


extern	short	kdbecho;
short	kdbpagelength = 57;	/* 57 rows on pmconsole */
short	kdbpagewidth = 128;	/* 128 columns on pmconsole */
static	short	colcount, linecount;
static	short	kdbflusho;
/*
 *  kdbread - read character from input queue
 */

kdbread(x, cp, len)
register char *cp;
{
	register int c;

	kdbflusho = 0;
	do {
		c = cngetc();
	} while (c == -1);

	if (c == '\r')
		c = '\n';
	*cp = c;
	colcount = linecount = 1;
	if (kdbecho)
		kdbwrite(x, cp, 1);
	return(1);
}

/*
 *  kdbwrite - send characters to terminal
 */

/* ARGSUSED */
kdbwrite(x, cp, len)
register char *cp;
register int len;
{
#ifdef	__STDC__
    const
#endif
    static char pagemodemsg[] =
	"\r[PAGE MODE: Press any key to continue, 'q' to flush output] "; 
    
    while (len--) {
	if (!kdbflusho) {
		if (*cp == '\n' ||	/* count lines, including wrap */
		    (kdbpagewidth && ++colcount > kdbpagewidth)) {
			colcount = 1;
			linecount++;
		}
		cnputc(*cp++);
	}
	else
		cp++;
	if (kdbpagelength && (linecount >= kdbpagelength)) {
		int c;
		int count = 0;
		while (count < sizeof(pagemodemsg))
			cnputc(pagemodemsg[count++]);
		do c = cngetc(); while (c == 0); /* swallow 1 character. */
		if ( c == '\f') {
			cnputc(c);
		} else {
			count = 0;
			cnputc('\r');
			while (count++ < sizeof(pagemodemsg))
				cnputc(' ');
			cnputc('\r');
		}
		colcount = 0;
		switch (c) {
		case 'q': case 'Q':
			kdbflusho = 1;
			/* fall through */
		case ' ': case 'f': case 'F': case ('F' & 0x1f):
			linecount = 1;
			break;
		case '\r': case 'j': case 'J': case '\n':
			linecount--;
			break;
		default:
			linecount /= 2;
			break;
		}
	}
    }
}

/*
 *   kdbrlong - read long word from kernel address space
 */

kdbrlong(addr, p)
long *addr;
long *p;
{
	long temp = 0;
	switch ((unsigned)addr & 0x3) {
	case 0:
		temp = *addr;
		break;
	case 2:
		{
			unsigned short *s = (unsigned short*)addr;
#if	BYTE_MSF
			temp = (s[0] << 16) | s[1];
#else
			temp = (s[1] << 16) | s[0];
#endif
		}
		break;
	default:
		{
			int i;
			unsigned char *b = (unsigned char*)addr;
#if	BYTE_MSF
			for (i = 0; i <= 3; i++) {
#else
			for (i = 3; i >= 0; i--) {
#endif
				temp <<= 8;
				temp |= b[i];
			}
		}
		break;
	}
	*p = temp;
}

/*
 *  kdbwlong - write long word to kernel address space
 */

kdbwlong(addr, p)
long *addr;
long *p;
{
    register long va = (long)addr;
    unsigned long	temp;

    if ((unsigned)addr > K2BASE) {
		vm_offset_t	pa;	/* physical address */
		int		i, ret;
		extern void putmemc();

		do {
			pa = pmap_resident_extract(kernel_pmap, addr);
			if (pa == 0) {
				ret = vm_fault(kernel_map, trunc_page(addr),
					VM_PROT_READ|VM_PROT_WRITE,
					FALSE);
				if (ret != KERN_SUCCESS)
					return (-1);
			}
		} while (pa == 0);

		temp = *p;
#if	BYTE_MSF
		for (i = 3; i <= 0; i--) {
#else
		for (i = 0; i <= 3; i++) {
#endif
			putmemc(pa + i, temp & 0xFF);
			temp >>= 8;
		}
		return(0);
    }

	temp = *p;
	switch ((unsigned)addr & 0x3) {
	case 0:
		*addr = temp;
		break;
	case 2:
		{
			unsigned short *s = (unsigned short*)addr;
#if	BYTE_MSF
			s[0] = (temp >> 16) & 0xffff;
			s[1] = temp & 0xffff;
#else
			s[1] = (temp >> 16) & 0xffff;
			s[0] = temp & 0xffff;
#endif
		}
		break;
	default:
		{
			int i;
			unsigned char *b = (unsigned char*)addr;
#if	BYTE_MSF
			for (i = 0; i <= 3; i++) {
#else
			for (i = 3; i >= 0; i--) {
#endif
				b[i] = temp & 0xff;
				temp >>= 8;
			}
		}
		break;
	}
	if ((unsigned)addr < K1BASE)
		/* Might be text, flush I-cache */
		clean_icache((unsigned)addr & ~0x3, 2 * sizeof(int));
}

/*
 *	Reads or writes a longword to/from the specified address
 *	in the specified map.  The map is forced to be the kernel
 *	map if the address is in the kernel address space.
 *
 *	Returns 0 if read/write OK, -1 if not.
 */

int kdbreadwrite(map, addr, value, rd)
	vm_map_t	map;
	vm_offset_t	addr;
	long		*value;	/* IN/OUT */
	boolean_t	rd;
{
	if (addr >= K0BASE) {
		/*
		 *	in kernel
		 */
		if (rd)
			kdbrlong(addr, value);
		else
			kdbwlong(addr, value);
		return (0);
	}
	else {
		vm_offset_t	pa;	/* physical address */
		unsigned long	temp;
		int		i, ret;

		do {
			pa = pmap_resident_extract(vm_map_pmap(map), addr);
			if (pa == 0) {
				ret = vm_fault(map, trunc_page(addr),
					rd ? VM_PROT_READ : VM_PROT_READ|VM_PROT_WRITE,
					FALSE);
				if (ret != KERN_SUCCESS)
					return (-1);
			}
		} while (pa == 0);

		if (rd) {
			extern unsigned getmemc();

			temp = 0;
#if	BYTE_MSF
			for (i = 0; i <= 3; i++) {
#else
			for (i = 3; i >= 0; i--) {
#endif
				temp <<= 8;
				temp |= (getmemc(pa + i)) & 0xFF;
			}
			*value = (int) temp;
		}
		else {
			extern void putmemc();
			temp = *value;
#if	BYTE_MSF
			for (i = 3; i >= 0; i--) {
#else
			for (i = 0; i < 4; i++) {
#endif
				putmemc(pa + i, temp & 0xFF);
				temp >>= 8;
			}
		}
		return (0);
	}
}

/*
 *  kdbsbrk - extended debugger dynamic memory
 */

static char kdbbuf[1024];
char *kdbend = kdbbuf; 

char *
kdbsbrk(n)
unsigned n;
{
    char *old = kdbend;

    if ((kdbend+n) >= &kdbbuf[sizeof(kdbbuf)])
    {
	return((char *)-1);
    }
    kdbend += n;
    return(old);
}

/*
 *	Return the map and pcb for a process.
 */
void kdbgetprocess(p, map, pcb)
	struct proc	*p;
	vm_map_t	*map;	/* OUT */
	struct pcb	**pcb;	/* OUT */

{
	/*
	 *	special case for current process
	 */
	if (p == u.u_procp) {
		*map = current_task()->map;
		*pcb = current_thread()->pcb;
	}
	else {
		if (p->task)
			*map = p->task->map;
		else
			*map = VM_MAP_NULL;
		if (p->thread)
			*pcb = p->thread->pcb;
		else
			*pcb = (struct pcb *)0;
	}
}

/*
 * Copy the contents of an exception frame into the
 * pcb, so that the user can see the context that existed
 * at the time the exception occurred.  Any changes that
 * the user makes in the contents of the registers will
 * be copied back into the exception frame before kdb returns
 * to trap(), and these new values will therefore be used
 * when the execution continues.
 */
save_exception_frame(pcb, esp)
register struct pcb *pcb;
register int *esp;
{
	pcb->pcb_regs[PCB_AT] = esp[EF_AT];
	pcb->pcb_regs[PCB_V0] = esp[EF_V0];
	pcb->pcb_regs[PCB_V1] = esp[EF_V1];
	pcb->pcb_regs[PCB_A0] = esp[EF_A0];
	pcb->pcb_regs[PCB_A1] = esp[EF_A1];
	pcb->pcb_regs[PCB_A2] = esp[EF_A2];
	pcb->pcb_regs[PCB_A3] = esp[EF_A3];
	pcb->pcb_regs[PCB_T0] = esp[EF_T0];
	pcb->pcb_regs[PCB_T1] = esp[EF_T1];
	pcb->pcb_regs[PCB_T2] = esp[EF_T2];
	pcb->pcb_regs[PCB_T3] = esp[EF_T3];
	pcb->pcb_regs[PCB_T4] = esp[EF_T4];
	pcb->pcb_regs[PCB_T5] = esp[EF_T5];
	pcb->pcb_regs[PCB_T6] = esp[EF_T6];
	pcb->pcb_regs[PCB_T7] = esp[EF_T7];
	pcb->pcb_regs[PCB_S0] = esp[EF_S0];
	pcb->pcb_regs[PCB_S1] = esp[EF_S1];
	pcb->pcb_regs[PCB_S2] = esp[EF_S2];
	pcb->pcb_regs[PCB_S3] = esp[EF_S3];
	pcb->pcb_regs[PCB_S4] = esp[EF_S4];
	pcb->pcb_regs[PCB_S5] = esp[EF_S5];
	pcb->pcb_regs[PCB_S6] = esp[EF_S6];
	pcb->pcb_regs[PCB_S7] = esp[EF_S7];
	pcb->pcb_regs[PCB_T8] = esp[EF_T8];
	pcb->pcb_regs[PCB_T9] = esp[EF_T9];
	pcb->pcb_regs[PCB_K0] = esp[EF_K0];
	pcb->pcb_regs[PCB_K1] = esp[EF_K1];
	pcb->pcb_regs[PCB_GP] = esp[EF_GP];
	pcb->pcb_regs[PCB_SP] = esp[EF_SP];
	pcb->pcb_regs[PCB_FP] = esp[EF_FP];
	pcb->pcb_regs[PCB_RA] = esp[EF_RA];
	pcb->pcb_regs[PCB_LO] = esp[EF_MDLO];
	pcb->pcb_regs[PCB_HI] = esp[EF_MDHI];
	pcb->pcb_regs[PCB_PC] = esp[EF_EPC];
	pcb->pcb_regs[PCB_SR] = esp[EF_SR];
	pcb->pcb_regs[PCB_BAD] = esp[EF_BADVADDR];
	pcb->pcb_regs[PCB_CS] = esp[EF_CAUSE];
}

/*
 * Copy the contents of the pcb back into the exception frame.
 * Any changes that the user mades in the contents of the
 * registers while in kdb WILL be reflected in the values
 * used when execution continues.
 */
restore_exception_frame(esp, pcb)
register int *esp;
register struct pcb *pcb;
{
	esp[EF_AT] = pcb->pcb_regs[PCB_AT];
	esp[EF_V0] = pcb->pcb_regs[PCB_V0];
	esp[EF_V1] = pcb->pcb_regs[PCB_V1];
	esp[EF_A0] = pcb->pcb_regs[PCB_A0];
	esp[EF_A1] = pcb->pcb_regs[PCB_A1];
	esp[EF_A2] = pcb->pcb_regs[PCB_A2];
	esp[EF_A3] = pcb->pcb_regs[PCB_A3];
	esp[EF_T0] = pcb->pcb_regs[PCB_T0];
	esp[EF_T1] = pcb->pcb_regs[PCB_T1];
	esp[EF_T2] = pcb->pcb_regs[PCB_T2];
	esp[EF_T3] = pcb->pcb_regs[PCB_T3];
	esp[EF_T4] = pcb->pcb_regs[PCB_T4];
	esp[EF_T5] = pcb->pcb_regs[PCB_T5];
	esp[EF_T6] = pcb->pcb_regs[PCB_T6];
	esp[EF_T7] = pcb->pcb_regs[PCB_T7];
	esp[EF_S0] = pcb->pcb_regs[PCB_S0];
	esp[EF_S1] = pcb->pcb_regs[PCB_S1];
	esp[EF_S2] = pcb->pcb_regs[PCB_S2];
	esp[EF_S3] = pcb->pcb_regs[PCB_S3];
	esp[EF_S4] = pcb->pcb_regs[PCB_S4];
	esp[EF_S5] = pcb->pcb_regs[PCB_S5];
	esp[EF_S6] = pcb->pcb_regs[PCB_S6];
	esp[EF_S7] = pcb->pcb_regs[PCB_S7];
	esp[EF_T8] = pcb->pcb_regs[PCB_T8];
	esp[EF_T9] = pcb->pcb_regs[PCB_T9];
	esp[EF_K0] = pcb->pcb_regs[PCB_K0];
	esp[EF_K1] = pcb->pcb_regs[PCB_K1];
	esp[EF_GP] = pcb->pcb_regs[PCB_GP];
	esp[EF_SP] = pcb->pcb_regs[PCB_SP];
	esp[EF_FP] = pcb->pcb_regs[PCB_FP];
	esp[EF_RA] = pcb->pcb_regs[PCB_RA];
	esp[EF_MDLO] = pcb->pcb_regs[PCB_LO];
	esp[EF_MDHI] = pcb->pcb_regs[PCB_HI];
	esp[EF_EPC] = pcb->pcb_regs[PCB_PC];
}
