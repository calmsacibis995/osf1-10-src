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
static char	*sccsid = "@(#)$RCSfile: sadebug.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:14:01 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#ifndef _KERNEL
#define MAXGDT 100			/* our GDT */
#endif _KERNEL

struct gdt
{
unsigned int limit:16,
	base:16,
	base2:8,
	attr:16,
	base3:8;
#ifndef _KERNEL
} sagdt[MAXGDT];
#else _KERNEL
} *sagdt;
#endif _KERNEL

#ifndef _KERNEL
#define MAXIDT 17		/* enough to handle int 3 */
#endif _KERNEL

#define PAGING		0x80000000
#define PAGE_MASK	((4*1024)-1)
#define PAGEDIR		22
#define PAGENO		12
#define	PGN_MASK	(1024-1)
#define PRESENT		1

struct idt 
{
	unsigned int offset:16,
		selector:16,
		attr:16,
		offset2:16;
}
#ifndef _KERNEL
	saidt[MAXIDT];
#else _KERNEL
	*saidt;
#endif _KERNEL

#include "i386/rdb/debug.h"

#ifdef i386
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#endif

#ifdef MACH
#include <i386/seg.h>
#ifdef STANDALONE
char **environ;
#endif
#endif /* MACH */

#ifdef MACH
#include <mach_kdb.h>
#else
struct tss386 tss;	/* a TSS */
struct tss386 dtss;	/* the debugger TSS */
struct tss386 dtss2;	/* a secondary debugger TSS */
#endif


int sysregs[16];
int dflg = 1;
int nflg;
int debug_state = NORMAL_STATE;


char *HO;		/* home screen */
char *CL;		/* clear screen */
char *CE;		/* clear to eol */

struct symtab symbols[];
struct symtab *symtab = symbols;

struct dtinfo { short fill, limit; int addr; } gdtp, sagdtp, saidtp;
struct dtinfo debugidtp;

#define DEBUG_OUT_SCR	1
#define DEBUG_OUT_COM	2

int debug_output = DEBUG_OUT_SCR;

#define DEBUG_IN_COM	0
#define DEBUG_IN_KBD	1

int debug_input = DEBUG_IN_KBD;

int debug_option;		       /* the debugger debug flag etc */
#define DEBUG_DEBUG	0x01
#define SHOW_REGS	0x02


int tss_s;			/* task selector for current task */
int dtss_s;			/* task selector for debugger task */
int dtss2_s;			/* task selector for debugger task */
#define MAXDEBUGSTK 2048
int debugstk[MAXDEBUGSTK];
int *debug_stack = debugstk+MAXDEBUGSTK;	/* top of debugger stack */

#define G 0x8000
#define D 0x4000
#define AVL 0x2000
#define DPL_MASK 0x3
#define DPL_SHIFT 5
#define DT 0x10
#define GATE 4
#define P 0x80
#define DPL_0   (0<<DPL_SHIFT)
#define DPL_1   (1<<DPL_SHIFT)
#define DPL_2   (2<<DPL_SHIFT)
#define DPL_3   (3<<DPL_SHIFT)
#define DWORD_COUNT_MASK 0x1f	/* count of parameters for call gate */

#define INT_386		0x0e	/* 386 interrupt gate */
#define TSS_386		0x09	/* 386 available task gate */
#define TSS_BUSY_386	0x0b	/* busy 386 task gate */
#define TASK_GATE	0x05	/* task gate */
#define TSS_LDT		0x02	/* LDT */

#ifndef _KERNEL
#define MAX_INT		(MAXIDT+1)	/* MAX int we're concerned with */
#else _KERNEL
#define MAX_INT		17 /*(MAXIDT+1)	/* MAX int we're concerned with */
#endif _KERNEL

struct idt debugidt[MAX_INT];	/* debugger temp idt */
struct dtinfo debugidtp;

void prdtentry();
void setgate();

#ifndef _KERNEL
main()
{
	debugger_init();
}
#endif /* _KERNEL */

#ifndef MACH
pre_debug_init()
{

	puts("pre_debug_init!\n");
	debug_input = DEBUG_IN_COM;
	puts("build_gdt\n");
	build_gdt(0);		/* make the GDT now! */
	puts("debugger_init\n");
	debugger_init();
}
#endif

debugger_init()
{
    int cr0, cr2, cr3;
    int breakint();
    int cs;
    int i;
    int debug_s;
    int debugexception();

#ifdef MACH
    extern struct gdt gdt[];
    cs = _mfsr(1);
    make_debug_idt(cs);
    sagdt = gdt;			/* point to kernel's gdt */
    HO = "\33[H";			/* assume xterm or ansi for now */
    CL = "\33[H\33[2J";
    CE = "\33[K";
#else
    db_delay();
    puts("             SATEST (standalone 386 protected mode test)\n");
    db_delay();
    regsave = (int *) &tss;		/* point to saved registers */
    tss.t_es = _mfsr(0);
    tss.t_cs = cs = _mfsr(1);
    tss.t_ss = _mfsr(2);
    tss.t_ds = _mfsr(3);
    tss.t_fs = _mfsr(4);
    tss.t_gs = _mfsr(5);
    tss.t_eax = mfgr(0);
    tss.t_ecx = mfgr(1);
    tss.t_edx = mfgr(2);
    tss.t_ebx = mfgr(3);
    tss.t_esp = mfgr(4);
    tss.t_ebp = mfgr(5);
    tss.t_esi = mfgr(6);
    tss.t_edi = mfgr(7);
    tss.t_eip = mfip();
    tss.t_pdbr = mfcr(3);
    tss.t_eflags = mff();
    HO = "\33[H";	/* assume xterm or ansi for now */
    CL = "\33[H\33[2J";
    CE = "\33[K";
#ifndef _KERNEL
#define pr(value) printf("value = "); prhex(value); printf("\n");
    pr(tss.t_cs);
    pr(tss.t_es);
    pr(tss.t_ds);
    pr(tss.t_ss);
    pr(tss.t_fs);
    pr(tss.t_gs);
    pr(tss.t_eax);
    pr(tss.t_ebx);
    pr(tss.t_ecx);
    pr(tss.t_edx);
    pr(tss.t_esp);
    pr(tss.t_ebp);
    pr(tss.t_esi);
    pr(tss.t_edi);
    pr(tss.t_eip);
    pr(tss.t_eflags);
#endif _KERNEL
    dtss = tss;		/* make copy of tss */
#ifndef _KERNEL
    dtss.t_eip = (int) breakint;	/* routine to handle break points */
    dtss.t_esp = (int) debug_stack;	/* set initial stack */
    pr(dtss.t_eip);
    pr(dtss.t_esp);
    sgdt(&gdtp.limit);
    pr(gdtp.limit);
    pr(gdtp.addr);
    prgdt((struct gdt *) gdtp.addr,(gdtp.limit+1)/8);
    printf("copying gdt from %x to %x (%d bytes)\n",
	    (char *) gdtp.addr, (char *) sagdt, gdtp.limit+1);
    bcopy((char *) gdtp.addr, (char *) sagdt, gdtp.limit+1);
    sagdtp.limit = gdtp.limit;		/* preserve the size */
    sagdtp.addr = (int) sagdt;
    printf("loading our own gdt...");
    makedes(sagdt, (sagdtp.limit+1)/8, P + TSS_386, (int) &tss, sizeof (struct tss386) - 1);
    tss_s = (sagdtp.limit+1);
    sagdtp.limit += 8;		/* one more selector */
    debug_s = (sagdtp.limit+1);
    sagdtp.limit += 8;		/* one more selector */
    dtss_s = (sagdtp.limit+1);
    sagdtp.limit += 8;		/* one more selector */
    dtss2_s = (sagdtp.limit+1);
#else _KERNEL
    dtss.t_eip = (int) breakint; /* routine to handle break points */
    dtss.t_esp = (int) debug_stack; /* set initial stack */
    dtss2 = dtss;			/* a new debugger tss */
    tss_s = allocate_gdt();
    makedes(sagdt, (tss_s)/8, P + TSS_386, (int) &tss, 
						sizeof (struct tss386) - 1);
    debug_s = tss_s + 8;
    dtss_s = debug_s + 8;
    dtss2_s = dtss_s + 8 * DOUBLE_STATE;
#endif _KERNEL
    /* make TSS entries in GDT for each interrupt */
    for (i=0; i<MAX_INT; ++i)
	{
	makedes(sagdt, dtss_s/8+i, P + TSS_386, (int) &dtss, 
				sizeof (struct tss386) - 1);
#ifndef _KERNEL
	sagdtp.limit += 8;		/* one more selector */
#endif _KERNEL
	}
    /* make a TSS for double faults */
    makedes(sagdt, dtss2_s/8, P + TSS_386, (int) &dtss2, 
			    sizeof (struct tss386) - 1);
    /* build task gate for 'call's to the debugger */
    setgate((struct idt *)sagdt, debug_s/8, P + TASK_GATE, 0, dtss_s + CALL_STATE*8, 0);
#ifndef _KERNEL
    lgdt(&sagdtp.limit);
    printf("\n");
    prgdt((struct gdt *)sagdtp.addr,(sagdtp.limit+1)/8);
    printf("reloading ds with %04x\n",_mfsr(3));
#endif _KERNEL
    _mtsr(3,_mfsr(3));
#ifndef DRIVER0
    printf("loading task register with %04x\n",tss_s);
#endif
    ltr(tss_s);
#ifndef _KERNEL
    sidt(&saidtp.limit);
    pr(saidtp.limit);
    pr(saidtp.addr);
    saidtp.limit = MAXIDT * sizeof (struct idt) - 1;
    saidtp.addr = (int) saidt;
    for (i=0; i<MAXIDT; ++i)
#else _KERNEL
    for (i=0; i< MAX_INT; ++i)
#endif _KERNEL
    /* build task gate (at DPL 3 so that we can debug user code) */
    setgate(saidt, i, DPL_3 + P + TASK_GATE,
	0, dtss_s+i*sizeof (struct gdt), 0);
#ifndef _KERNEL
    lidt(&saidtp.limit);	/* set idt */
    printf("Debugger idt\n");
    prgdt((struct gdt *)saidtp.addr,(saidtp.limit+1)/8);
    pr(saidtp.limit);
    pr(saidtp.addr);
    make_debug_idt(cs);
#endif _KERNEL
    cr0 = mfcr(0);
    cr2 = mfcr(2);
    cr3 = mfcr(3);
#ifndef _KERNEL
    pr(cr0);
    pr(cr2);
    pr(cr3);
#else _KERNEL
    /*_mtcr(3,cr3);	/* flush the tlb */
#endif _KERNEL
    debug_option = 0;
    make_debug_idt(cs);
#ifndef _KERNEL
    for (;;)
	    {
	    printf("generating a breakpoint\n");
	    int3();
	    printf("calling foo\n");
	    foo();
	    printf("calling debugger\n");
	    db_delay();
	    _debugger(NORMAL_STATE, (char *) 0);
	    printf("calling thru task gate\n");
	    debugger(debug_s,0);	/* 386 call gate to task gate */
	    db_delay();
	    }
#else _KERNEL
    int3();
#endif _KERNEL
#endif /* MACH */
}

#ifdef notdef
make_debug_idt(cs)
{
    int i;

    /* build a temporary debugger idt that is used to handle exceptions */
    for (i=0; i<MAXIDT; ++i)
	    setgate(debugidt, i, P + INT_386, 0, cs, (int) debugexception);
    debugidtp.limit = MAXIDT * sizeof (struct idt) - 1;
    debugidtp.addr = (int) debugidt;
}
#endif
int foovar;

foo()
{
	foovar = 1;
	printf("in foo\n");
	foovar = 2;
}

char *mem_names[16] = {
"RO", "RO, acc",
"RW", "RW, acc",
"RO, stack", "RO, stack, acc", 
"RW, stack", "RW, stack, acc", 
"EX", "EX, acc",
"REX", "REX, acc",
"EX, conform", "EX, conform, acc",
"REX, conform", "REX, conform, acc" };

char *seg_names[16] = {
"undefined", "avail 286 TSS",		/* 0 1 */
"ldt", "busy 286 TSS", 			/* 2 3 */
"286 call gate", "task gate",		/* 4 5 */
"286 int gate", "286 trap gate",	/* 6 7 */
"undefined", "avail 386 TSS",		/* 8 9 */
"undefined", "busy 386 TSS",		/* a b */
"386 call gate", "undefined",		/* c d */
"386 int gate", "386 trap gate" };	/* e f */

/*
 * print out a gdt, ldt, or idt table given the table and 
 * a count of the number of entries 
 */
void prgdt(gdt,count)
struct gdt *gdt;
int count;
{
	int i;

	if (gdt)
		for (i=0; i<count; ++i)
			prdtentry(gdt, i<<3);
}

/*
 * print out a gdt, ldt, or idt entry given the table and 
 * the selector.
 */
void prdtentry(dt, selector)
struct gdt *dt;
int selector;
{
	int i = selector>>3;
	struct gdt *gdt = dt + i;
	int base = (((gdt->base3 << 8) + gdt->base2) << 16) + gdt->base;
	int attr = gdt->attr;
	int limit = gdt->limit + ((attr&0xf00) << 8);
	int type = attr & 0xf;

	printf("%04x (%2d): limit=%x base=%x base2=%x attr=%x base3=%x\n",
		selector, i, gdt->limit, gdt->base, gdt->base2, attr, gdt->base3);
	if ((attr&DT) || (type&GATE) == 0)
		printf("     G=%s %s AVL=%d P=%d DPL=%d base=%08x limit=%05x type=%x (%s)\n",
			(attr&G) ? "Page" : "Byte",
			(attr&DT) ? ((attr&D) ? "386" : "286") :
				((attr&D) ? "X=1" : "X=0"),
			(attr&D) ? 1 : 0,
			(attr&P) ? 1 : 0,
			(attr>>DPL_SHIFT) & DPL_MASK,
			base, limit, type,
			(attr&DT) ? mem_names[type] : seg_names[type]);
	else		/* its a gate */
		{
		struct idt *idt = (struct idt *) gdt;
		printf("      DPL=%d P=%d count=%03d selector=%04x offset=%08x type=%s\n",
			(attr>>DPL_SHIFT) & DPL_MASK,
			(attr&P) ? 1 : 0,
			idt->attr&DWORD_COUNT_MASK,
			idt->selector,
			idt->offset | (idt->offset2<<16),
			seg_names[type]);
		}
}

#if !defined(MACH)
/*
 * build descriptor of given type (except for gates)
 * with the given base and limit.
 * type includes everything in attributes except limit.
 */
makedes(gdt, vector, type, base, limit)
struct gdt *gdt;
unsigned int vector, type, base, limit;
{
	gdt += vector;		/* get to right vector */
	gdt->attr = type | ((limit & 0xf0000) >> 8);
	gdt->limit = limit;
	gdt->base = base;
	gdt->base2 = base>>16;
	gdt->base3 = base>>24;
}
#endif

/* set an entry in idt */
void setgate(idt, vector, type, count, selector, offset)
struct idt *idt;
int vector, type, count, selector, offset;
{
	struct idt * v = idt + vector;

	v->offset = offset & 0xffff;
	v->selector = selector;
	v->attr = (type << 8) + count;
	v->offset2 = offset >> 16;
}

lps()
{
	if (dflg)
		printf("can't lps");
}


int wfetch(addr)
int addr;
{
	int value;
	struct dtinfo idtp;

	sidt(&idtp.limit);
	lidt(&debugidtp.limit);
	value = _wfetch(addr);
	lidt(&idtp.limit);
	return(value);
}

int hfetch(addr)
int addr;
{
	int value;
	struct dtinfo idtp;

	sidt(&idtp.limit);
	lidt(&debugidtp.limit);
	value = _hfetch(addr);
	lidt(&idtp.limit);
	return(value);
}


int bfetch(addr)
int addr;
{
	int value;
	struct dtinfo idtp;

	sidt(&idtp.limit);
	lidt(&debugidtp.limit);
	value = _bfetch(addr);
	lidt(&idtp.limit);
	return(value);
}


int wstore(addr,value)
	int addr;
	unsigned int value;
{
	* (int *) addr = value;
	return(value);
}

int hstore(addr,hvalue)
int addr;
int hvalue;
{
	* (short *) addr = hvalue;
	return(hvalue);
}


int bstore(addr,bvalue)
int addr;
int bvalue;
{
	* (char *) addr = bvalue;
	return(bvalue);
}

#ifdef __STDC__
void err/* KILL CTAGS */(char *fmt, ...)
#else
void err/* KILL CTAGS */(fmt,d1,d2,d3)
char *fmt;
#endif __STDC__
{
	printf(fmt,((int *) &fmt)[1],((int *) &fmt)[2],((int *) &fmt)[3]);
	printf("\n");
	exit(1);
}

int mfsr(reg)
register int reg;
{
	if (dflg>1)
		printf("mfsr s%d\n",reg);
	if (reg >= 0 && reg < 16)
		return(sysregs[reg]);
	else
		err("mfsr s%d\n",reg);
	return(0);
}

int mtsr(reg,value)
register int reg;
register int value;
{
	if (dflg>1)
		printf("mtsr s%d %d\n",reg,value);
	if (reg >= 0 && reg < 16)
		sysregs[reg] = value;
	else
		err("mtsr s%d %d\n",reg,value);
	return(0);
}

callabs()
{
	printf("callabs\n");
}


_init_vectors(n)
register int n;
{
	debug_state = n;	/* what a kludge! */
}

init_kbd()
{
	printf("init_kbd\n");
}

put_status(pos,str)
int pos;
register char *str;
{
	if (!nflg)
		printf("%s\r",str);
}

struct symtab internal[];

fix_internal()
{
#ifdef notdef
	register struct symtab *sym;
	register adjust = ((int) regsave) - REGSAVE;

	internal[0].value = (int) &sysregs[SCR_IAR];
	for (sym = internal; sym->symbol[0]; ++sym)
		if (sym->value >= REGSAVE && sym->value < REGSAVE+64)
			sym->value += adjust;
#endif
}



/*
 * do virtual to real translation using the page directory table
 * and the page tables pointed to by it.
 */
int vtop(where)
	register unsigned int where;
{
	unsigned long	cr3 = mfcr(3) | KVBASE;	/* get virtual alias */
	unsigned long	page_table;
	unsigned long	page_index;
	unsigned long	physpage;

	/*
	 * so cr3 points to the directory page table
	 */
	page_index = where >> PAGEDIR;
	cr3 &= ~PAGE_MASK;
	page_table = wfetch(cr3 + (page_index<<2));
	if (debug_option&DEBUG_DEBUG)
		printf("vtop(%x) cr3=%x index=%x pdr=%x ", where, cr3,
			page_index, page_table);
	if (page_table == -1) {
		printf("bad virtual address (bad pdr)\n");
		++err_flag;
		return (-1);
	}
	if ((page_table & PRESENT) == 0) {
		printf("bad virtual address (pdr)\n");
		++err_flag;
		return (-1);
	}
	page_table &= ~PAGE_MASK;
	page_index = (where >> PAGENO) & PGN_MASK;
	physpage = wfetch((page_table + (page_index<<2)) | KVBASE);
	if (debug_option&DEBUG_DEBUG)
		printf("pte=%x index=%x",page_table,page_index);
	if (physpage == -1 || (physpage & PRESENT) == 0) {
		printf("bad virtual address (pte)\n");
		++err_flag;
		return (-1);
	}
	physpage &= ~PAGE_MASK;
	if (debug_option&DEBUG_DEBUG)
		printf("==>%x\n",physpage | (where & PAGE_MASK));
	return(physpage | (where & PAGE_MASK));
}


/*
 * NOTE: we put the type on the same line to defeat ctags which otherwise
 * would pick these definitions instead of the real ones.
 */

void save_screen(buffer,save,size)
char *buffer, *save;
int size;
{
}

void screen_restore(buffer,save,size)
char *buffer, *save;
int size;
{
}


INW ( port )  { printf("stub INW ( port )  called "); 
 printf("port = %x ",port);
 printf("\n"); 
}


/*
 * mapping of register numbers is defined by sys_internal ordering
 */

MFSR(reg)
{
	int value=0;

	switch(reg)
		{
	case SYS_CR0:
	case SYS_CR1:
	case SYS_CR2:
	case SYS_CR3:
		value = mfcr(reg-SYS_CR0);
		break;
	case SYS_DR0:
	case SYS_DR1:
	case SYS_DR2:
	case SYS_DR3:
	case SYS_DR4:
	case SYS_DR5:
	case SYS_DR6:
	case SYS_DR7:
		value = mfdr(reg-SYS_DR0);
		break;
	case SYS_GDT:
		{
		struct dtinfo gdtp;
			sgdt(&gdtp.limit);
			value = gdtp.addr;
			break;
		}
	case SYS_GDTL:
		{
		struct dtinfo gdtp;
			sgdt(&gdtp.limit);
			value = gdtp.limit;
			break;
		}
	case SYS_IDT:
		{
		struct dtinfo idtp;
			sidt(&idtp.limit);
			value = idtp.addr;
			break;
		}
	case SYS_IDTL:
		{
		struct dtinfo idtp;
			sidt(&idtp.limit);
			value = idtp.limit;
			break;
		}
	case SYS_TASK:
#ifdef MACH
		value = str();		/* get the task register */
#else
		value = dtss.t_back;	/* the task we are debugging */
#endif
		break;
	default:
		printf("MFSR(%d) not defined\n",reg);
		break;
		}
	return(value);
}

MTSR(reg,value)
{
	switch(reg)
		{
	case SYS_CR0:
	case SYS_CR1:
	case SYS_CR2:
	case SYS_CR3:
		mtcr(reg-SYS_CR0,value);	/* */
		break;
	case SYS_DR0:
	case SYS_DR1:
	case SYS_DR2:
	case SYS_DR3:
	case SYS_DR4:
	case SYS_DR5:
	case SYS_DR6:
	case SYS_DR7:
		mtdr(reg-SYS_DR0,value);
		break;
	case SYS_IDT:
		{
		struct dtinfo idtp;
			sidt(&idtp.limit);
			idtp.addr = value;
			lidt(&idtp.limit);
			break;
		}
	case SYS_IDTL:
		{
		struct dtinfo idtp;
			sidt(&idtp.limit);
			idtp.limit = value;
			lidt(&idtp.limit);
			break;
		}
	case SYS_GDT:
		{
		struct dtinfo idtp;
			sgdt(&idtp.limit);
			idtp.addr = value;
			lgdt(&idtp.limit);
			break;
		}
	case SYS_GDTL:
		{
		struct dtinfo idtp;
			sgdt(&idtp.limit);
			idtp.limit = value;
			lgdt(&idtp.limit);
			break;
		}
	case SYS_TASK:
#ifdef MACH
		ltr(value);
#else
		dtss.t_back = value;	/* the task we are debugging */
		set_regsave();		/* update our register pointer */
#endif
		break;
	default:
		printf("MTSR(%d,%d) not defined\n",reg,value);
		break;
		}
	return(value);
}


OUTW ( port , value )  { printf("stub OUTW ( port , value )  called "); 
 printf("port = %x ",port);
 printf("value = %x ",value);
 printf("\n"); 
}

#ifndef _KERNEL
#define forever() for (;;)
#endif

#ifndef MACH
exit()
{
	forever ();
}
#endif

___Fp_Init(void)
{}

db_delay(void)
{
	int i;
	for (i=0; i<1000000; ++i)
		;
}

prhex(value)
unsigned value;
{
	int i, j;
	for (i=32; (i -= 4) >= 0; )
		{
		j = (value>>i)&0x0f;
		if (j >= 10)
			putchar(j - 10 + 'A');
		else
			putchar(j + '0');
		}
}
#ifdef AT386
#define com_putchar cnputc
#endif AT386
putchar(c)
int c;
{
#ifndef _KERNEL
	com_putchar(c);
	screen_putchar(c);
#else _KERNEL
	if (debug_output&DEBUG_OUT_COM)
		com_putchar(c);
	if (debug_output&DEBUG_OUT_SCR)
		screen_putchar(c);
#endif _KERNEL
}

/*
 * swap debugger from console to com port and back again
 */
debugswap(void)
{
	debug_input = 1-debug_input;
	debug_output = 3-debug_output; 
	if (debug_output == 0)
		debug_output = 3;
}

db_reboot(void)
{
	struct dtinfo idtp;
	idtp.limit = 0;
	idtp.addr = 0;
	lidt(&idtp.limit);
	db_delay();
	int3();
}

#ifdef _KERNEL
int getchar(void)
{
	int c = _getchar();
	putchar(c);
	return(c);
}

#ifdef AT386
#define kdd_getchar cngetc
#define com_getchar cngetc
#endif

int _getchar(void)
{
	register int c;
	char kdd_getchar();

	
	if (debug_input != DEBUG_IN_COM)
		c = kdd_getchar();
	else
		c = com_getchar();
	if (c == '\r')
		c = '\n';
	return (c);
}
#endif _KERNEL

/*
 * build debugger idt for use when accessing memory or doing other 
 * dangerous things that might result in exceptions.
 * we allow stepping and break points for debugging of the debugger.
 */
make_debug_idt(cs)
int cs;
{
	int i = 0;
	int debugexception();

#ifndef MACH
	for (i; i<=3; ++i)
		debugidt[i] = saidt[i];	/* int 0...3 are copied */
#endif

	/* set the rest of the interrupts */
	for (; i<MAX_INT; ++i)
	    setgate(debugidt, i, P + INT_386, 0, cs, (int) debugexception);
	debugidtp.limit = MAX_INT * sizeof (struct idt) - 1;
	debugidtp.addr = (int) debugidt;
}

/*
 * called on entry to the trap routine to point "regsave" at the
 * saved registers (e.g. the tss of that task that was executing).
 * we assume that the debugger's tss is "dtss".
 */
set_regsave(void)
{
#ifndef MACH
	int selector = (str()!=dtss2_s) ? dtss.t_back : dtss2.t_back;
	int i = selector>>3;	/* the task we're debugging's selector */
	struct gdt *gdt = sagdt + i;
	int base = (((gdt->base3 << 8) + gdt->base2) << 16) + gdt->base;
	int type = gdt->attr & 0xf;

	if (type == TSS_BUSY_386 || type == TSS_386)
		regsave = (int *) base;
	else
		printf("%x not TSS selector\n", selector);
	if (debug_option&DEBUG_DEBUG)
		prdtentry(sagdt, selector);
#endif /* MACH */
}

/*
 * print out an idt entry
 */
pridtentry(selector)
int selector;
{
	struct dtinfo idtp;

	sidt(&idtp.limit);

	if (selector > idtp.limit)
		printf("selector outside IDT limit (%x)\n", idtp.limit);
	else
		prdtentry((struct gdt *) idtp.addr, selector);
}


static struct gdt *getldt();

static struct gdt *getgdt(selector,err_ptr)
int selector;
int *err_ptr;
{
	struct dtinfo gdtp;

	selector &= 0xffff;		/* mask down to 16 bits */
	if (err_ptr)
		*err_ptr = 0;
	if (selector&4)
		return(getldt(selector,err_ptr));
	sgdt(&gdtp.limit);
	if ((unsigned) selector > (unsigned) gdtp.limit)
		{
		if (err_ptr)
			++*err_ptr;
		else
			printf("selector outside GDT limit (%x)\n", gdtp.limit);
		return(0);
		}
	return((struct gdt *) gdtp.addr);
}

/*
 * get the ldt pointer for given selector.
 * we use the processes's context LDT instead of our own
 */
static struct gdt *getldt(selector,err_ptr)
int selector;
int *err_ptr;
{
#ifdef MACH
	int i = sldt();		/* the task we're debugging's LDT selector */
#else
	int i = REG(t_ldt);	/* the task we're debugging's LDT selector */
#endif
	struct gdt *gdt = getgdt(i,err_ptr);
	int limit;
	int type;
	struct gdt * ldt;

	i &= 0xffff;		/* mask down to 16 bits */
	i >>= 3;
	if (i == 0 || gdt == 0) {
		if (err_ptr)
			++*err_ptr;
		else
			printf("no LDT defined\n");
		return(0);
	}
	gdt += i;
	ldt = (struct gdt *) ((((gdt->base3 << 8) + gdt->base2) << 16) + gdt->base);
	limit = gdt->limit + ((gdt->attr&0xf00) << 8);
	type = gdt->attr & 0xf;
	if (type == TSS_LDT && selector < limit)
		return(ldt);
	else
		{
		if (err_ptr)
			++*err_ptr;
		else
			printf("%x not valid LDT selector\n", selector);
		return(0);
		}
}

/*
 * print out a GDT (or ldt if that's appropriate) entry
 */
prgdtentry(selector)
int selector;
{
	struct gdt *gdt;

	if ((gdt = getgdt(selector,(int *) 0)) != 0)
		prdtentry((struct gdt *) gdt, selector);
}

/*
 * return the size (32 or 16 bits) of the cs segment
 */
get_cs_size(void)
{
	int i = REG(t_cs) & 0xffff;	/* the task we're debugging's cs selector */
	struct gdt *gdt;

	if (i == 0)
		return(32);		/* something is very wrong here */
	gdt = getgdt(i,(int *) 0);
	if (gdt == 0)
		return(32);	/* assume 32 bit */
	gdt += i>>3;		/* get proper entry */

	return((gdt->attr & D) ? 32 : 16);
}

/*
 * fix an old symbol table format symbol 
 */
fixsym(sym)
struct symtab *sym;
{
	struct oldsym {
	char symbol[20];
	int value;
	} *osym = (struct oldsym *) sym, oldsym;

	for (;osym->symbol[0]; ++osym, sym = next_sym(sym))
		{
		oldsym = *osym;
		bzero((char *) osym, sizeof *osym);
		strcpy(sym->symbol, oldsym.symbol);
		sym->value = oldsym.value;
		}
}

#if defined(MACH) && !defined(_KERNEL)
_getchar()
{
	return(getch());
}

int screen_lines = 24;

#endif /* MACH */

#if defined(MACH) && defined(MACH)

static kdb_trap(apsl)
int *apsl;
{
	printf("entered kdb_trap\n");

	_debugger(0,(char *) 0);
}

#include "i386/rdb/reg.h"

struct tss386 debug_tss;	/* a TSS */

int debug_init = 0;

int db_stack = 5 * 4;

db_kdb(x,y,z)
{
	if (debug_option&DEBUG_DEBUG)
		printf("entered db_kdb(%x,%x,%x)\n", x, y, z);
	if (!debug_init)
		{
		++debug_init;
		debugger_init();
		printf("debugger_init done\n");
		}
	if (x == -1 || y == 0)
		{
		extern int boothowto;

		printf("calling set_tss\n");
		set_tss(&debug_tss);
		regsave =  (int *) &debug_tss;
		printf("calling _debugger\n");
/*		_debugger(x,(char *) 0);	*/
		set_tf();		/* cause the debugger to be entered */
		if ((boothowto&RB_NOSYNC))
		    int3();			/* if that doesn't do it this will! */
		}
	else
		{
		int s = sploff();

		regsave =  (int *) y;
		if (x == 8)
			db_double(y);			/* fix up from tss */
		else
			regsave[ESP] += db_stack;	/* fudge the stack pointer */
		db_trap(x, regsave[ERR]);
		splon(s);		/* doesn't get here normally */
		}
}

#if MACH_KDB == 0
kdb(x, y, z)
{
	return(db_kdb(x, y, z));
}
#endif

int kdb_singlestep;

/*
 * function (called from keyboard or other interrupt) to request a 
 * single step interrupt to pass control into the debugger at the point
 * that the interrupt actually happened.
 */
kdb_set_tf(locr0)
int		*locr0;		/* pointer to interrupt stack frame */
{
	kdb_singlestep = 1;	/* tell mach kernel about it */
	locr0[EFL] |= 0x100;	/* cause single step interrupt */
}

#include "i386/rdb/atox.c"
#define atol atoi
#include "i386/rdb/atol.c"
#include "i386/rdb/gets.c"
#include "i386/rdb/puts.c"
#include "i386/rdb/strncmp.c"
#endif
