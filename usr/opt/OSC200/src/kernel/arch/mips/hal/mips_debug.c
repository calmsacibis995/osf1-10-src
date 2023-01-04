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
static char	*sccsid = "@(#)$RCSfile: mips_debug.c,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/04/16 14:02:50 $";
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
 * derived from mips_debug.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <machine/reg.h>
#include <machine/cpu.h>
#ifdef mips
#include <hal/entrypt.h>
#endif

#include <sys/user.h>
#include <kern/xpr.h>


/*
 * options for mipskopt system call
 */
#define	KOPT_GET	1		/* get kernel option */
#define	KOPT_SET	2		/* set kernel option */
#define	KOPT_BIS	3		/* or in new option value */
#define	KOPT_BIC	4		/* clear indicated bits */


struct reg_values sig_values[] = {
	{ SIGHUP,		"SIGHUP" },
	{ SIGINT,		"SIGINT" },
	{ SIGQUIT,		"SIGQUIT" },
	{ SIGILL,		"SIGILL" },
	{ SIGTRAP,		"SIGTRAP" },
	{ SIGIOT,		"SIGIOT" },
	{ SIGEMT,		"SIGEMT" },
	{ SIGFPE,		"SIGFPE" },
	{ SIGKILL,		"SIGKILL" },
	{ SIGBUS,		"SIGBUS" },
	{ SIGSEGV,		"SIGSEGV" },
	{ SIGSYS,		"SIGSYS" },
	{ SIGPIPE,		"SIGPIPE" },
	{ SIGALRM,		"SIGALRM" },
	{ SIGTERM,		"SIGTERM" },
	{ SIGURG,		"SIGURG" },
	{ SIGSTOP,		"SIGSTOP" },
	{ SIGTSTP,		"SIGTSTP" },
	{ SIGCONT,		"SIGCONT" },
	{ SIGCHLD,		"SIGCHLD" },
	{ SIGTTIN,		"SIGTTIN" },
	{ SIGTTOU,		"SIGTTOU" },
	{ SIGIO,		"SIGIO" },
	{ SIGXCPU,		"SIGXCPU" },
	{ SIGXFSZ,		"SIGXFSZ" },
	{ SIGVTALRM,		"SIGVTALRM" },
	{ SIGPROF,		"SIGPROF" },
	{ 0,			0 },
};

struct reg_values imask_values[] = {
	{ SR_IMASK8,	"8" },
	{ SR_IMASK7,	"7" },
	{ SR_IMASK6,	"6" },
	{ SR_IMASK5,	"5" },
	{ SR_IMASK4,	"4" },
	{ SR_IMASK3,	"3" },
	{ SR_IMASK2,	"2" },
	{ SR_IMASK1,	"1" },
	{ SR_IMASK0,	"0" },
	{ 0,		NULL },
};

struct reg_desc sr_desc[] = {
	/* mask	     shift      name   format  values */
	{ SR_CU3,	0,	"CU3",	NULL,	NULL },
	{ SR_CU2,	0,	"CU2",	NULL,	NULL },
	{ SR_CU1,	0,	"CU1",	NULL,	NULL },
	{ SR_CU0,	0,	"CU0",	NULL,	NULL },
	{ SR_BEV,	0,	"BEV",	NULL,	NULL },
	{ SR_TS,	0,	"TS",	NULL,	NULL },
	{ SR_PE,	0,	"PE",	NULL,	NULL },
	{ SR_CM,	0,	"CM",	NULL,	NULL },
	{ SR_PZ,	0,	"PZ",	NULL,	NULL },
	{ SR_SWC,	0,	"SwC",	NULL,	NULL },
	{ SR_ISC,	0,	"IsC",	NULL,	NULL },
	{ SR_IBIT8,	0,	"IM8",	NULL,	NULL },
	{ SR_IBIT7,	0,	"IM7",	NULL,	NULL },
	{ SR_IBIT6,	0,	"IM6",	NULL,	NULL },
	{ SR_IBIT5,	0,	"IM5",	NULL,	NULL },
	{ SR_IBIT4,	0,	"IM4",	NULL,	NULL },
	{ SR_IBIT3,	0,	"IM3",	NULL,	NULL },
	{ SR_IBIT2,	0,	"IM2",	NULL,	NULL },
	{ SR_IBIT1,	0,	"IM1",	NULL,	NULL },
	{ SR_IMASK,	0,	"IPL",	NULL,	imask_values },
	{ SR_KUO,	0,	"KUo",	NULL,	NULL },
	{ SR_IEO,	0,	"IEo",	NULL,	NULL },
	{ SR_KUP,	0,	"KUp",	NULL,	NULL },
	{ SR_IEP,	0,	"IEp",	NULL,	NULL },
	{ SR_KUC,	0,	"KUc",	NULL,	NULL },
	{ SR_IEC,	0,	"IEc",	NULL,	NULL },
	{ 0,		0,	NULL,	NULL,	NULL },
};

struct reg_values exc_values[] = {
	{ EXC_INT,	"INT" },
	{ EXC_MOD,	"MOD" },
	{ EXC_RMISS,	"RMISS" },
	{ EXC_WMISS,	"WMISS" },
	{ EXC_RADE,	"RADE" },
	{ EXC_WADE,	"WADE" },
	{ EXC_IBE,	"IBE" },
	{ EXC_DBE,	"DBE" },
	{ EXC_SYSCALL,	"SYSCALL" },
	{ EXC_BREAK,	"BREAK" },
	{ EXC_II,	"II" },
	{ EXC_CPU,	"CPU" },
	{ EXC_OV,	"OV" },
	{ SEXC_SEGV,	"SW_SEGV" },
	{ SEXC_AST,	"SW_AST" },
	{ SEXC_ILL,	"SW_ILL" },
	{ SEXC_CPU,	"SW_CP_UNUSABLE" },
	{ 0,		NULL },
};

struct reg_desc exccode_desc[] = {
	/* mask	     shift      name   format  values */
	{ 1,		0,	"USER",	NULL,	NULL },
	{ CAUSE_EXCMASK,0,	"EXC",	NULL,	exc_values },
	{ 0,		0,	NULL,	NULL,	NULL }
};

struct reg_desc cause_desc[] = {
	/* mask	     shift      name   format  values */
	{ CAUSE_BD,	0,	"BD",	NULL,	NULL },
	{ CAUSE_CEMASK,	-CAUSE_CESHIFT,	"CE",	"%d",	NULL },
	{ CAUSE_IP8,	0,	"IP8",	NULL,	NULL },
	{ CAUSE_IP7,	0,	"IP7",	NULL,	NULL },
	{ CAUSE_IP6,	0,	"IP6",	NULL,	NULL },
	{ CAUSE_IP5,	0,	"IP5",	NULL,	NULL },
	{ CAUSE_IP4,	0,	"IP4",	NULL,	NULL },
	{ CAUSE_IP3,	0,	"IP3",	NULL,	NULL },
	{ CAUSE_SW2,	0,	"SW2",	NULL,	NULL },
	{ CAUSE_SW1,	0,	"SW1",	NULL,	NULL },
	{ CAUSE_EXCMASK,0,	"EXC",	NULL,	exc_values },
	{ 0,		0,	NULL,	NULL,	NULL },
};

struct reg_desc tlbhi_desc[] = {
	/* mask	     shift      name   format  values */
	{ TLBHI_VPNMASK,0,	"VA",	"0x%x",	NULL },
	{ TLBHI_PIDMASK,-TLBHI_PIDSHIFT,"PID",	"%d",	NULL },
	{ 0,		0,	NULL,	NULL,	NULL },
};

struct reg_desc tlblo_desc[] = {
	/* mask	     shift      name   format  values */
	{ TLBLO_PFNMASK,0,	"PA",	"0x%x",	NULL },
	{ TLBLO_N,	0,	"N",	NULL,	NULL },
	{ TLBLO_D,	0,	"D",	NULL,	NULL },
	{ TLBLO_V,	0,	"V",	NULL,	NULL },
	{ TLBLO_G,	0,	"G",	NULL,	NULL },
	{ 0,		0,	NULL,	NULL,	NULL },
};

struct reg_desc tlbinx_desc[] = {
	/* mask	     shift      name   format  values */
	{ TLBINX_PROBE,	0,	"PFAIL",NULL,	NULL },
	{ TLBINX_INXMASK, -TLBINX_INXSHIFT, "INDEX", "%d", NULL },
	{ 0,		0,	NULL,	NULL,	NULL },
};

struct reg_desc tlbrand_desc[] = {
	/* mask	     shift      name   format  values */
	{ TLBRAND_RANDMASK, -TLBRAND_RANDSHIFT, "RANDOM", "%d", NULL },
	{ 0,		0,	NULL,	NULL,	NULL },
};

struct reg_desc tlbctxt_desc[] = {
	/* mask	     shift      name   format  values */
	{ TLBCTXT_BASEMASK, 0,	"PTEBASE", "0x%x", NULL },
	{ TLBCTXT_VPNMASK, 11,	"BADVAP", "0x%x", NULL},
	{ 0,		0,	NULL,	NULL,	NULL },
};

struct reg_values prot_values[] = {
	/* value			name */
	{ VM_PROT_NONE,			"None"},
	{ VM_PROT_READ,			"R" },
	{ VM_PROT_WRITE,		"W" },
	{ VM_PROT_READ|VM_PROT_WRITE,	"RW" },
	{ VM_PROT_EXECUTE,		"E" },
	{ VM_PROT_EXECUTE|VM_PROT_READ,	"RE" },
	{ VM_PROT_EXECUTE|VM_PROT_WRITE,"WE" },
	{ VM_PROT_DEFAULT,		"WRE" },
	{ 0,				NULL }
};
struct reg_desc pte_desc[] = {
	/* mask	     shift      name   format  values */
	{ VA_PAGEMASK,	0,	"PADDR", "0x%x", NULL },
	{ PG_N,		0,	"N",	NULL,	NULL },
	{ PG_M,		0,	"M",	NULL,	NULL },
	{ PG_V,		0,	"V",	NULL,	NULL },
	{ PG_G,		0,	"G",	NULL,	NULL },
	{ PG_PROT,	0,	"PROT",	NULL,	prot_values },
	{ 0,		0,	NULL,	NULL,	NULL }
};


struct reg_values sym_values[] = {
	/* value			name */
#if	XPR_DEBUG
	{ XPR_SCHED,			"sched" },
	{ XPR_SYSCALLS,			"syscall" },
	{ XPR_TRAPS,			"trap" },
	{ XPR_TTY,			"tty" },
	{ XPR_BIO,			"bio" },
	{ XPR_INTR,			"intr" },
	{ XPR_CACHE,			"cache" },
	{ XPR_NFS,			"nfs" },
	{ XPR_SIGNAL,			"signal" },
	{ XPR_SYSCALLS|XPR_TRAPS
	   |XPR_SCHED
	   |XPR_SIGNAL,
					"kernel" },
#endif	/* XPR_DEBUG */
	{ 0,				NULL }
};


assfail(ass, file, line)
char *ass, *file;
int line;
{
	printf("ASSERTION %s FAILED in file %s at line %d\n",
	    ass, file, line);
	panic("ASSERTION");
}

/*
 * getargs(argc, argv)
 *
 *  1. loop through the arg list:
 *
 *  1a. if switch is "init="name, name is the file with the image of the
 *	  init process
 *
 *  1b. if switch is "-"x, copy the switch from the boot stack to kernel
 *	  space so it can be passed to init process
 *
 *  1c. if switch is sw"="n, set kernel flag sw to number n; if "="n is
 *	  omitted, set to 1
 *
 */
getargs(argc, argv, environ)
char *argv[];
char *environ[];
{
	extern struct kernargs kernargs[];
	register struct kernargs *kp;
	register int i;
	register char *cp;
	register char **argp;

	argp = 0;	/* environ is junk on ultrix */
	if ((u_int)argv <= (u_int)K1BASE ||
	    (u_int)argv >= (u_int)(K1BASE + 0x20000)) {
		dprintf("getargs: bad argv from prom 0x%x\n",argv);
		return;
	}

#ifdef	mips
	{
	/*
	 * set bootdevice string
	 *
	 * Note:  check_dbg() copies argv[0] to bootdevice.
	 *	  Also, netbootchk() in machdep.c has code almost
	 *	  the same as this, but it's never called.
	 */
		extern char bootdevice[];
		extern int console_magic;

		if(console_magic != REX_MAGIC)
			strcpy(bootdevice, (char *)prom_getenv("boot") );
		else {
			/*
			 * Find boot param of form 3/rz?/vmunix
			 */
			for(i = 1; i < argc; ++i) {

				if(argv[i][0] == '-')
					continue;
				if(argv[i][0] == 0)
					continue;
				if(argv[i][1] != '/')
					continue;

				strcpy(bootdevice, argv[i]);
				break;
			}
		}
	}
#endif	/* mips */


loop:
	for (; argp && *argp; argp++) {
		if (strncmp("init=", *argp, 5) == 0) {
			extern char init_program_name[];

			dprintf("Using %s for /etc/init\n", &(*argp)[5]);
			strcpy(init_program_name, &(*argp)[5]);
			continue;
		}
		if (**argp == '-') {
			register char *cp,*ca;
			extern char init_args[];
			dprintf("/etc/init arg: %s\n", *argp);
			for (cp = *argp, ca = init_args; *cp;)
				*ca++ = *cp++;
			*ca = 0;
			continue;
		}
		for(kp = kernargs; kp->name; kp++) {
			i = strlen(kp->name);
			if (strncmp(kp->name, *argp, i) == 0) {
				extern char *atob();

				cp = &((*argp)[i]);
				if (*cp == 0)
					*kp->ptr = 1;
				else if (*cp == '=') {
					cp = atob(++cp, kp->ptr);
					if (*cp)
		dprintf("WARNING: Badly formed kernel argument %s\n", *argp);
				} else if (*cp == ':') {
					extern char *symval();
					cp = symval(++cp, kp->ptr);
					if (*cp)
		dprintf("WARNING: Badly formed kernel argument %s\n", *argp);
				} else
					continue;
				dprintf("Kernel argument %s = 0x%x\n",
				    kp->name, *kp->ptr);
				break;
			}
		}
	}
	if (argv) {
		argp = argv+1;	/* skip boot device */
		argv = 0;
		goto loop;
	}
}

/*
 * System call which allows a user to read/write kernel variables specified
 * in the above table by symbolic name.  Only the super-user is allowed to
 * write non-readonly variables.
 */

int
mipskopt(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	struct args {
		char *argname;
		int value;
		int op;
	} *uap = (struct args *) args;
#define MAXKVARNAME 20
	char nbuf[MAXKVARNAME];
	register char *nbp;
	register struct kernargs *kp;
	int error = 0;

	nbp = nbuf;

	if ((error = copyinstr(uap->argname, nbuf, MAXKVARNAME, 0)) != 0) {
		if (error == ENOENT)
			error = EINVAL; /* name too long */
		return(error);
	}

	for (kp = kernargs; kp->name; kp++)
		if (strcmp(kp->name, nbuf) == 0) {
			error = do_opt(kp, uap->value, uap->op, retval);
			return(error);
		}
	
	return (EINVAL);
}

do_opt(kp, val, op, retval)
struct kernargs *kp;
int *retval;
{
	*retval = *(kp->ptr);

#if	SEC_BASE
	if (op != KOPT_GET && (!privileged(SEC_DEBUG, 0) || kp->readonly))
		return EACCES;
#else
	if (op != KOPT_GET && (suser(u.u_cred, &u.u_acflag) || kp->readonly))
		return EACCES;
#endif

	switch (op) {

	case KOPT_GET:
		break;

	case KOPT_SET:
		*(kp->ptr) = val;
		break;

	case KOPT_BIS:
		*(kp->ptr) |= val;
		break;

	case KOPT_BIC:
		*(kp->ptr) &= ~val;
		break;

	default:
		return(EINVAL);
	}

	if (kp->func && op != KOPT_GET)
		return((*kp->func)(*(kp->ptr)));
	return(0);
}

/*
 * digit -- convert the ascii representation of a digit to its
 * binary representation
 */
unsigned
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
