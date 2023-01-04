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
static char *rcsid = "@(#)$RCSfile: alpha_init.c,v $ $Revision: 1.2.32.5 $ (DEC) $Date: 1994/01/05 19:39:57 $";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: ...msp/src/kernel/dec/machine/alpha/alpha_init.c
 *
 * 25-Oct-91 - jestabro
 *	Added some support for BL6
 *
 * 24-Apr-91 -- afd
 *	Created this file for Alpha init.
 */

#include <confdep.h>
#include <mach_kdb.h>

#include <sys/types.h>
#include <machine/reg.h>
#include <machine/cpu.h>
#include <machine/pcb.h>
#include <machine/rpb.h>
#include <machine/machparam.h>
#include <hal/cpuconf.h>

#include <sys/param.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/vm.h>
#include <sys/proc.h>
#include <sys/reboot.h>
#include <sys/conf.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <sys/file.h>
#include <sys/clist.h>
#include <sys/callout.h>
#include <mach/vm_param.h>
#include <kern/xpr.h>
#include <machine/prom.h>
#include <machine/entrypt.h>

vm_offset_t	avail_start, avail_end;
vm_offset_t	virtual_avail, virtual_end;
vm_size_t	mem_size;		/* 100 ways to cook an egg */
int		maxmem, physmem;
long		boothowto = RB_SINGLE|RB_ASKNAME|RB_KDB;
#ifdef MEMLIMIT
long memlimit = MEMLIMIT;    /* to artificially reduce memory */
#else  MEMLIMIT
long memlimit = 0;
#endif MEMLIMIT

extern long	do_virtual_tables;

extern int hz;
extern int tick;
extern int tickadj;
extern int printstate;

/*
 * Processor type info from Per-CPU slot in HWRPB
 */
long proc_type;				/* processor type */
long proc_var;				/* processor variation */
long proc_rev;				/* processor revision */

/*
 * System type info from common area in HWRPB
 */
long sys_type;				/* system type (family or platform) */
long sys_var;				/* system variation */
long sys_rev;				/* system revision */

long	askme;		 	 	/* set by getargs */

struct to_be_declared_elsewhere_later {	/* rpb software data buffer */
	vm_offset_t	physroot;	/* physical root of page table */
} rpb_software_data = {0};		/* must reside in initialized data */

long	hwrpb_addr = HWRPB_ADDR;	/* kernel virt addr of HWRPB */
struct	rpb *rpb;			/* ptr to HWRPB */
struct	rpb_percpu *percpu;		/* ptr to per-cpu portion of HWRPB */
struct	rpb_mdt *mdt;			/* ptr to mem desc table in HWRPB */
struct	rpb_cluster *cluster;		/* ptr to mem cluster desc in mdt */
struct	rpb_ctb *ctb;			/* ptr to console terminal block */
struct	rpb_crb *crb;			/* ptr to console routine block */
struct	rpb_map *map;			/* ptr to crb map */

int	console_magic;			/* Console id */

int	ub_argc;
#define	MAX_ARGS	10

/*
 * Initialize, to force this into the .data section.  Otherwise it
 * will be zeroed out in pmap_bootstrap() after we copy arguments
 * in copy_args().
 */
char	ub_argv[MAX_ARGS][MAX_ENVIRON_LENGTH+1] = {
	"", "", "", "", "", "", "", "", "", ""
};

/*
 * alpha_bootstrap
 *
 * This is the first kernel function to be invoked by the assembly
 * language startup code in entry.s.  It is called with a preliminary
 * vm mapping in effect (initialized by previous bootstrap software),
 * before the kernel's bss is zeroed, and before kernel breakpoints
 * will function.  It returns the physical address of the first HWPCB
 * set up by pmap_bootstrap.  The assembly language handling will then
 * put a new vm mapping into effect before calling alpha_init.
 */
vm_offset_t
alpha_bootstrap(ffpfn, ptbr, argc, argv)
long ffpfn, ptbr;
int argc;
char *argv[];
{
	extern vm_offset_t pmap_bootstrap();
	extern char end[], _fbss[];
	register char *src, *dest;
	register int i, j;
	extern int partial_dump;
	extern char init_args[];
	int kdebug_mode = 0;
	char *cp;
	extern char *prom_getenv();

	/*
	 * Zero the kernel's bss data.  Before this, uninitialized global
	 * kernel variables will probably not have a 0 value.
	 */
	bzero(_fbss, end - _fbss);	/* zero bss */
	mb();

	/*
	 * Perform initialization for console callbacks.
	 */
	prom_init();

	/*
	 * process the booted_osflags environment variable
	 */
	if (cp = prom_getenv("booted_osflags")) {
		while (*cp) {
			switch (*cp++) {
			case 'D':
			case 'd':
				/*
				 * use full dumps (default is partial)
				 */
				partial_dump = 0;
				break;
			case 'A':
			case 'a':
				/*
				 * tell init to go multi (default is single)
				 */
				strcpy(init_args, "-a");
				break;
			case 'K':
			case 'k':
				/*
				 * turn kdebug on now (default is off)
				 */
				kdebug_mode = 1;
				break;
			default:
				/* ignore other flags */
				break;
			}
		}
	}

	/*
	 * Perform initialization for kdebug entry.
	 */
	kdebug_bootstrap(argc, argv, kdebug_mode);

	/*
	 * Copy arguments passed in from secondary bootstrap address space
	 * to kernel.  Once pmap_bootstrap and context switch are done, we
	 * can't access them, since bootstrap address space is not mapped.
	 */
	if (argc > MAX_ARGS)	/* will warn later in alpha_init */
		argc = MAX_ARGS;
	for (i = 0; i < argc; i++) {
		src = argv[i];
		dest = ub_argv[i];
		for (j = 0; *src && j < MAX_ENVIRON_LENGTH; j++)
			*dest++ = *src++;
	}

	if (argc > MAX_ARGS) {
		ub_argc = MAX_ARGS;
		printf("alpha_bootstrap: too many boot arguments (max %d)\n",
								MAX_ARGS);
	} else
		ub_argc = argc;

	/*
	 * Process any kernel arguments.
	 */
	if (ub_argc > 0)
		getargs(ub_argc, ub_argv);

	/*
	 * Perform early initialization of pmap module.  This includes
	 * setting up a new virtual memory mapping which will become
	 * effective soon after alpha_bootstrap returns to entry.s.
	 * The reserved-for-software field in the rpb will be assigned
	 * to the physical address of rpb_software_data, and the first
	 * field of this structure will be assigned to pmap_physroot.
	 */
	return(pmap_bootstrap(ffpfn, ptbr, (vm_offset_t)&rpb_software_data,
						&rpb_software_data.physroot));
}

/*
 * Machine-dependent init code.
 * We now have an initial u_area and kernel stack that we are running on
 * set up by pmap_bootstrap.
 */
alpha_init(argc, argv, environ)
	int argc;
	char *argv[];
	char *environ[];
{
/*  These two declarations SHOULD NOT BE MERGED INTO GOLD!                 */
        char *test;
        extern char *prom_getenv();
/* ********************************************************                */
#if	MACH_KDB
	extern char *kdbesymtab;
	struct thread fake_th;
	struct task fake_tsk;
#endif /* MACH_KDB */
	register thread_t th;
	char *cp;
	extern main();
	extern thread_t setup_main();

	extern long _scb;
	extern struct timeval atime,*timepick;
	extern long reg_save;
	extern vm_offset_t hwrpb_offset;
	extern int steal_mem();
	extern int fixtick;
	extern int partial_dump;
	extern char *prom_getenv();
	extern char init_args[];

	console_magic = (int)environ;	/* save console id */

	/*
	 * Calculate ptrs to:
	 *	the Per-CPU portion of the HWRPB for this cpu.
	 * 	the Memory Descriptor Table portion of the HWRPB.
	 * 	    the first memory cluster descriptor in the mdt.
	 * 	the Console Terminal Block portion of the HWRPB.
	 * 	the Console Routine Block portion of the HWRPB.
	 * 	    the first virt/phys map entry in the CRB.
	 */

	rpb = (struct rpb *)hwrpb_addr;
	percpu = (struct rpb_percpu *) (hwrpb_addr + rpb->rpb_percpu_off +
		(rpb->rpb_cpuid * rpb->rpb_slotsize));
	mdt = (struct rpb_mdt *) (hwrpb_addr + rpb->rpb_mdt_off);
	cluster = mdt->rpb_cluster;
	ctb = (struct rpb_ctb *) (hwrpb_addr + rpb->rpb_ctb_off);
	crb = (struct rpb_crb *) (hwrpb_addr + rpb->rpb_crb_off);
	map = crb->rpb_map;

	/*
	 * Get the type, variation, revision & serial number info for both
	 * the processor and the system from the HWRPB.
	 */
	proc_type = percpu->rpb_proctype;
	proc_var = percpu->rpb_procvar;
	proc_rev = percpu->rpb_procrev;

	sys_type = rpb->rpb_systype;
	sys_var = rpb->rpb_sysvar;
	sys_rev = rpb->rpb_sysrev;

	/*
	 * Set clock parameters.  "hz" is the number of clock interrupts
	 * per second, and is stored in the HWRPB scaled up by 4096.
	 * "tick" is the number of micro-seconds between clock interrupts.
	 * "tickadj" is the clock skew, in micro-secs per tick.
 	 * "fixtick" is for clocks frequency doesn't divide evenly
	 * into a second.
	 */
	hz = rpb->rpb_clock / 4096;
	tick = 1000000 / hz;
	tickadj = 1;
	fixtick = 1000000 % hz;

	/*
	 * Set "cpu" (the ULTRIX system type) based on both
	 * processor type and system type.
	 *
	 * Initialize "cpup" to point to the cpusw table entry for the system
	 * that we are currently running on.
	 */
	set_cpuswitch();

	/* 
	 * In pmap_bootstrap we set up the kernel stack, and we just
     	 * initialized "cpu" and "cpup" so we can handle exceptions now.
	 * So its safe to set rpb->restart to doadump and clear BIP and RIP
	 */
	init_rpb();

	/*
	 * Do system specific initializations and
	 * initialize this cpu as the boot processor.
	 * Then do conventional mapinit.
	 */
	cpu_initialize();

	/*
	 * Grab memory for special devices
	 */
	steal_mem();

	/*
	 * Initialize the VM system.
	 */
	vm_mem_init();

	/*
	 * First available page of phys mem
	 */
#if	MACH_KDB
	bzero(&fake_th, sizeof fake_th);
	bzero(&fake_tsk, sizeof fake_tsk);
	current_thread() = &fake_th;
	current_thread()->task = &fake_tsk;
#endif

	panic_init();		/* even if not a multiprocessor */

        /* Initialize the head pointer to the kernel driver thread
         * structure
         */
        kd_thread_init();

	/*
	 * Now that pmap_init is done, map I/O space so cninit will work.
	 * Once cninit is done we can do kernel printfs to the console,
	 * thru the Ultrix driver (rather than thru console call backs).
	 */
	mapspace();
/*  This code fixes a console bug which caused the pixelvision hardware to  *
 *  corrupt its state if the first prom call occurred after printstate was  *
 *  set to CONSPRINT.  This line of code and the declarations which support *
 *  it SHOULD NOT BE MERGED INTO GOLD!                                      */

        test = prom_getenv("booted_osflags");
/*      *************************************                               */

	cninit();
	cnprobe(0);
	printstate &= ~PROMPRINT;
	printstate |= CONSPRINT;

#if	MACH_KDB
	if ((boothowto & (RB_HALT|RB_KDB)) == (RB_HALT|RB_KDB))
		gimmeabreak();
#endif	/* MACH_KDB */

	printf("Alpha boot: available memory from 0x%lx to 0x%lx\n",
					avail_start, avail_end);

	if (!do_virtual_tables) {
		allocate_unix_tables( &avail_start, FALSE );
		avail_start = round_page(avail_start);
	}

	/*
	 * Get the first thread and proceed to main
	 */
	th = setup_main();
	th->pcb->pcb_regs[PCB_PC] = (vm_offset_t)main;
	load_context(th);		/* Geronimo!! */
	/*NOTREACHED*/
}

extern void restart(); /* located in locore.s */
struct procval {
	long percpu_kseg_addr;	/* percpu kseg ptr and restore term flag */
	void (*func_addr)();	/* address of shared callback function */
};
struct procval restart_pv = {0L, restart};
struct procval rstrterm_pv = {0L, restart};

/*
 * Do software initialization on a few fields in the HWRPB then
 * re-calc the checksum.  Note that the global pointer "percpu"
 * has already been assigned by alpha_init() before this routine
 * gets called.
 */

init_rpb()
{
	register long sum, *lp1, *lp2;
	register struct bootpcb *bootpcb;
	register struct pcb *pcb;
	extern vm_offset_t bootpcb_va, pmap_physhwrpb;

	/* initialize restart context in boot pcb in case of an error halt */
	pcb = (struct pcb *)bootpcb_va;
	bootpcb = &percpu->rpb_pcb;
	bootpcb->rpb_ksp = pcb->pcb_ksp;
	bootpcb->rpb_usp = pcb->pcb_usp;
	bootpcb->rpb_ptbr = pcb->pcb_ptbr;
	bootpcb->rpb_cc = 0;
	bootpcb->rpb_asn = pcb->pcb_asn;
	bootpcb->rpb_proc_uniq = pcb->pcb_ptbr; /* for "broken" console code */
	bootpcb->rpb_fen = 0;

	/* initialize the virtual page table base and the callback vectors */
	rpb->rpb_vptb = (long)Selfmap;
	restart_pv.percpu_kseg_addr = PHYS_TO_KSEG(pmap_physhwrpb +
		((vm_offset_t)percpu - (vm_offset_t)hwrpb_addr));
	rpb->rpb_restart_pv = (long)&restart_pv;
	rpb->rpb_restart = restart;
	rstrterm_pv.percpu_kseg_addr = restart_pv.percpu_kseg_addr | 1;
	rpb->rpb_rstrterm_pv = (long)&rstrterm_pv;
	rpb->rpb_rstrterm = restart;

	/* recalculate and update the HWRPB checksum */
	sum = 0;
	lp1 = (long *)rpb;
	lp2 = &rpb->rpb_checksum;
	while (lp1 < lp2)
		sum += *lp1++;
	*lp2 = sum;

	/* clear the BIP flag and set the RC/CV flags in percpu rpb state */
	percpu->rpb_state = (percpu->rpb_state & ~STATE_BIP)|STATE_RC|STATE_CV;
}

/*
 * getargs(argc, argv)
 *
 *  1. loop through the arg list:
 *
 *  1a. if switch is "init="name, name is the file with the image of the
 *	  init process
 *
 *  1b. if switch is sw"="n, set kernel flag sw to number n; if "="n is
 *	  omitted, set to 1
 *
 */
getargs(argc, argv)
	int argc;
	register char argv[][MAX_ENVIRON_LENGTH+1];
{
	extern struct kernargs kernargs[];
	register struct kernargs *kp;
	register i, l;
	register char *cp;

#ifdef	notdef
	/*
	 * Note we assume no environment ptr, so start with argv
	 */
	if (argv) {
		argp = argv+1;	/* skip boot device */
	}
#endif

	for (i = 0; i < argc; i++) {
		if (i == 0) {
#define BOOTEDFILELEN 80
			extern char bootedfile[BOOTEDFILELEN];
			if (strlen(argv[0]) < BOOTEDFILELEN)
				strcpy(bootedfile, argv[0]);
		}
		if (strncmp("init=", argv[i], 5) == 0) {
			extern char init_program_name[];

			printf("Using %s for /sbin/init\n", &argv[i][5]);
			strcpy(init_program_name, &argv[i][5]);
			continue;
		}
		for (kp = kernargs; kp->name; kp++) {
			l = strlen(kp->name);
			if (strncmp(kp->name, argv[i], l) == 0) {
				extern char *atobl(), *symval();

				cp = &argv[i][l];
				if (*cp == 0)
					*kp->ptr = 1;
				else if (*cp == '=') {
					cp = atobl(++cp, kp->ptr);
					if (*cp)
		printf("WARNING: Badly formed kernel argument %s\n", argv[i]);
				} else if (*cp == ':') {
					cp = symval(++cp, kp->ptr);
					if (*cp)
		printf("WARNING: Badly formed kernel argument %s\n", argv[i]);
				} else
					continue;
				printf("Kernel argument %s = 0x%x\n",
					kp->name, *kp->ptr);
				break;
			}
		}
	}
}

/*
 * digit -- convert the ascii representation of a digit to its
 * binary representation
 */
unsigned int
digit(c)
	register char c;
{
	unsigned d;

	if (c >= '0' && c <= '9')
		d = c - '0';
	else if (c >= 'a' && c <= 'f')
		d = c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		d = c - 'A' + 10;
	else
		d = 999999; /* larger than any base to break callers loop */
	return(d);
}

/*
 * atob -- convert ascii to binary.  Accepts all C numeric formats.
 */
char *
atob(cp, iptr)
	register char *cp;
	int *iptr;
{
	int minus = 0;
	register int value = 0;
	unsigned base = 10;
	unsigned d;

	*iptr = 0;

	while (*cp == ' ' || *cp == '\t')
		cp++;

	while (*cp == '-') {
		cp++;
		minus = !minus;
	}

	/*
	 * Determine base by looking at first 2 characters
	 */
	if (*cp == '0') {
		switch (*++cp) {
		case 'X':
		case 'x':
			base = 16;
			cp++;
			break;

		case 'B':	/* a frill: allow binary base */
		case 'b':
			base = 2;
			cp++;
			break;
		
		default:
			base = 8;
			break;
		}
	}

	while ((d = digit(*cp)) < base) {
		value *= base;
		value += d;
		cp++;
	}

	if (minus)
		value = -value;

	*iptr = value;
	return(cp);
}

struct reg_values sym_values[] = {
	/* value			name */
#ifdef notdef
	{ XPR_CLOCK,			"clock" },
	{ XPR_TLB,			"tlb" },
	{ XPR_INIT,			"init" },
	{ XPR_SCHED,			"sched" },
	{ XPR_PROCESS,			"process" },
	{ XPR_EXEC,			"exec" },
	{ XPR_SYSCALL,			"syscall" },
	{ XPR_TRAP,			"trap" },
	{ XPR_VM,			"vm" },
	{ XPR_SWAP,			"swap" },
	{ XPR_SWTCH,			"swtch" },
	{ XPR_DISK,			"disk" },
	{ XPR_TTY,			"tty" },
	{ XPR_TAPE,			"tape" },
	{ XPR_BIO,			"bio" },
	{ XPR_INTR,			"intr" },
	{ XPR_RMAP,			"rmap" },
	{ XPR_TEXT,			"text" },
	{ XPR_CACHE,			"cache" },
	{ XPR_NFS,			"nfs" },
	{ XPR_RPC,			"rpc" },
	{ XPR_RPC|XPR_NFS,		"nfsrpc" },
	{ XPR_SIGNAL,			"signal" },
	{ XPR_SM,                       "shared memory" },
	{ XPR_TLB|XPR_SCHED|XPR_PROCESS
	   |XPR_EXEC|XPR_SYSCALL|XPR_TRAP
	   |XPR_NOFAULT|XPR_VM|XPR_SWAP
	   |XPR_SWTCH|XPR_RMAP|XPR_TEXT
	   |XPR_SIGNAL,
					"kernel" },
#endif notdef
	{ 0,				NULL }
};

char *
symval(cp, iptr)
register char *cp;
int *iptr;
{
	register struct reg_values *rv;
	register char *bp;
	char buf[32];

	*iptr = 0;

	while (cp && *cp) {
		bp = buf;
		while (*cp && *cp != '|' && bp < &buf[sizeof(buf)-1])
			*bp++ = *cp++;
		*bp = 0;
		for (rv = sym_values; rv->rv_name; rv++)
			if (strcmp(buf, rv->rv_name) == 0) {
				*iptr |= rv->rv_value;
				break;
			}
		if (rv->rv_name == NULL)
			printf("unknown symbol: %s\n", buf);
		while (*cp == '|')
			cp++;
	}
	return(cp);
}

/*
 * atobl -- convert ascii to binary (long).  Accepts all C numeric formats.
 */
char *
atobl(cp, iptr)
	register char *cp;
	long *iptr;
{
	int minus = 0;
	register long value = 0;
	unsigned base = 10;
	unsigned d;

	*iptr = 0;

	while (*cp == ' ' || *cp == '\t')
		cp++;

	while (*cp == '-') {
		cp++;
		minus = !minus;
	}

	/*
	 * Determine base by looking at first 2 characters
	 */
	if (*cp == '0') {
		switch (*++cp) {
		case 'X':
		case 'x':
			base = 16;
			cp++;
			break;

		case 'B':	/* a frill: allow binary base */
		case 'b':
			base = 2;
			cp++;
			break;
		
		default:
			base = 8;
			break;
		}
	}

	while ((d = digit(*cp)) < base) {
		value *= base;
		value += d;
		cp++;
	}

	if (minus)
		value = -value;

	*iptr = value;
	return(cp);
}
