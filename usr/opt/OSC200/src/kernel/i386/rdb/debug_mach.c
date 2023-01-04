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
static char	*sccsid = "@(#)$RCSfile: debug_mach.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:55 $";
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
int db_16bit = 0;		/* set if in 16 bit mode in unasm */
int cs_size = 0;
int db_min_args = 4;		/* min args to show */

#if defined(_KERNEL) && !defined(MACH)
#include "../ios/i386.h"
#endif

#include <i386/reg.h>

/* note: printregs requires that the name be < 7 bytes 
 * entries following " " are not displayed by the register
 * display code so act as invisible synonyms
 */
struct symtab internal[] = {
SYM("eax", R(t_eax))
SYM("ecx", R(t_ecx))
SYM("edx", R(t_edx))
SYM("ebx", R(t_ebx))
SYM("esp", R(t_esp))
SYM("ebp", R(t_ebp))
SYM("esi", R(t_esi))
SYM("edi", R(t_edi))
SYM("es", R(t_es))
SYM("cs", R(t_cs))
#ifndef MACH
SYM("ss", R(t_ss))
#endif MACH
SYM("ds", R(t_ds))
SYM("fs", R(t_fs))
SYM("gs", R(t_gs))
SYM("eip", R(t_eip))
SYM("eflags", R(t_eflags))
#ifndef MACH
SYM("link", R(t_back))
SYM("ldt", R(t_ldt))
SYM("pdbr", R(t_pdbr))
#endif /* MACH */
SYM(" ",	0)		/* stop display here; synonyms follow */
SYM("ax", R(t_eax))
SYM("cx", R(t_ecx))
SYM("dx", R(t_edx))
SYM("bx", R(t_ebx))
SYM("sp", R(t_esp))
SYM("bp", R(t_ebp))
SYM("si", R(t_esi))
SYM("di", R(t_edi))
SYM("ip", R(t_eip))
SYM("flags", R(t_eflags))
#ifndef MACH
SYM("esp0", R(t_esp0))
SYM("ss0", R(t_ss0))
SYM("esp1", R(t_esp1))
SYM("ss1", R(t_ss1))
SYM("esp2", R(t_esp2))
SYM("ss2", R(t_ss2))
#endif /* MACH */
};

int reg_mask = -1;		/* display all registers */

struct symtab sys_internal[] = {
SYM("cr0",	SYS_CR0)
SYM("cr1",	SYS_CR1)
SYM("cr2",	SYS_CR2)
SYM("cr3",	SYS_CR3)
SYM("dr0",  SYS_DR0)
SYM("dr1",  SYS_DR1)
SYM("dr2",  SYS_DR2)
SYM("dr3",  SYS_DR3)
SYM("dr4",  SYS_DR4)
SYM("dr5",  SYS_DR5)
SYM("dr6",  SYS_DR6)
SYM("dr7",  SYS_DR7)
SYM("gdt",	SYS_GDT)
SYM("gdtl",	SYS_GDTL)
SYM("idt",	SYS_IDT)
SYM("idtl",	SYS_IDTL)
SYM("task", SYS_TASK)
0, 0 };

int sys_reg_mask = -1;		/* display all registers */
int deb_reg_mask = -1;		/* display all registers */
int V[10];			/* debugger variables */

/*
 * following is for internal debugger flags and variables
 */

struct symtab deb_internal[] = {
SYM("regmask", (int) &reg_mask)
SYM("sysmask", (int) &sys_reg_mask)
SYM("debmask", (int) &deb_reg_mask)
SYM("option",  (int) &debug_option)
SYM("optmask", (int) &option_mask)
SYM("dsize",   (int) &dispsize)
SYM("offset",  (int) &db_offset)
SYM("scan",    (int) &debug_lastscan)
SYM("scanlen", (int) &debug_scan)
SYM("scanp0", (int) &debug_pattern.words[0])
SYM("scanp1", (int) &debug_pattern.words[1])
SYM("scanp2", (int) &debug_pattern.words[2])
SYM("scanp3", (int) &debug_pattern.words[3])
SYM("radix1", (int) &db_radix1)
SYM("radix2", (int) &db_radix2)
SYM("symtab1", (int) &symtab)
SYM("symtab2", (int) &symtab2)
SYM("V0",	   (int) (V+0))
SYM("V1",	   (int) (V+1))
SYM("V2",	   (int) (V+2))
SYM("V3",	   (int) (V+3))
SYM("V4",	   (int) (V+4))
SYM("V5",	   (int) (V+5))
SYM("V6",	   (int) (V+6))
SYM("V7",	   (int) (V+7))
SYM("V8",	   (int) (V+8))
SYM("V9",	   (int) (V+9))
SYM("wtype",   (int) &wtype)
SYM("lines",   (int) &screen_lines)
SYM("cs_size", (int) &cs_size)
SYM("min_args", (int) &db_min_args)
0, 0 };

struct symtab *scansym(addr, offset)
int addr, offset;
{
	return(0);
}


tlbprint()
{
printf("tlbprint called\n");
}


call(fn,arg1,arg2,arg3)
FN fn;
int arg1, arg2, arg3;
{
	(*fn)(arg1,arg2,arg3);
}

#define PUSH(value) esp -= sizeof (int); \
	* (int *) esp = (int) value

#define POP(value) \
	value = * (int *) esp; \
	esp += sizeof (int) \


/*
 * program call: do the call from the current location
 * in the program. we save the current state so that nothing
 * is destroyed while we do the call.
 * stack contents:
 *
 * struct tss386	32
 * debug_state		28
 * .byte 0,0,0		25
 * int $1		23
 * add $12,%esp		20
 * PCALL_MAGIC		16
 * arg 3		12
 * arg 2		8
 * arg 1		4
 * return eip		0
 */

#define PCALL_MAGIC 0xBADCBAD4
#define PCALL_DONE_LENGTH 5	/* add $12,%esp; int $1 = 3 + 2 = 5 */

static int pstack;

pcall(fn,arg1,arg2,arg3)
FN fn;
int arg1, arg2, arg3;
{
	register int esp = REG(t_esp);
	extern int call_done[2];	/* allows for 8 bytes of call done */
	int return_eip;

	esp -= sizeof (struct tss386);
	bcopy((char *) regsave, (char *) esp, sizeof (struct tss386));
	PUSH(pstack);			/* previous stack state */
	PUSH(debug_state);		/* remember the state */
	PUSH(call_done[1]);
	PUSH(call_done[0]);
	return_eip = esp;
	PUSH(PCALL_MAGIC);		/* magic marker */
	pstack = esp;			/* for preturn */
	PUSH(arg3);
	PUSH(arg2);
	PUSH(arg1);
	PUSH(return_eip);		/* return address */
	REG(t_eip) = (int) fn;			/* the function address */
	REG(t_esp) = esp;
	go_step = 0;
	step_count = 0;	       		/* don't step it */
	set_watch();			/* set watch & break points */
	set_break();
	restart(debug_state, 0);
}

/* 
 * we've reached the end of a pcall, so we arrange to get back
 * to the original state
 */
pcall_done(type)
int type;
{
	register int esp = REG(t_esp);

	if (pstack == 0 || 
	    esp+PCALL_DONE_LENGTH+sizeof (int) != REG(t_eip) &&
	    wfetch(esp) != PCALL_MAGIC)
		return(type);			/* wasn't a pcall */
	esp += 3 * sizeof (int);		/* pop magic & code */
	POP(type);				/* debug debug_state */
	POP(pstack);
	bcopy((char *) esp, (char *) regsave, sizeof (struct tss386));
	_debugger(type,"status");			/* return to cmd mode */
	/* NOT REACHED */
}

preturn()
{
	register int esp = pstack;

	if (esp == 0 || wfetch(esp) != PCALL_MAGIC) {
		printf("no pending pcall\n");
		return;
	}
	esp += 3 * sizeof (int);		/* pop magic & code */
	POP(debug_state);			/* debug debug_state */
	POP(pstack);				/* previous pstack */
	bcopy((char *) esp, (char *) regsave, sizeof (struct tss386));
}


getreg(reg)
int reg;
{
	return(GET_REG(reg));
}

printregs(state)
int state;
{
	print_registers(internal, reg_mask, getreg);
}

#define SPACES "        "

/*
 * print a single register
 * fn is the function to obtain the register value
 */
print_reg(name,sym,fn)
	char *name;
	struct symtab *sym;
	int (*fn)();
{
	int dot;
	dot = lookup(name, &err_flag, sym);
	if (err_flag)
		badreg();
	else
		printf("%x\n", fn(dot));
}

print_registers(sym,reg_mask,fn)
	struct symtab *sym;
	int reg_mask;
	int (*fn)();
{
	int i, j = 1;
	int len;

	for (i = 0; sym->symbol[0] && sym->symbol[0] != ' ' ; ++sym, j <<= 1) {
		if ((reg_mask&j) == 0)
			continue;		/* skip this value */
		len =  strlen(sym->symbol);
		printf("%s", sym->symbol);
		if (len >= (sizeof SPACES)-1)
			printf(" ");	/* use only 1 */
		else
			printf(SPACES + len); /* make up space */
		printf("%08x", (*fn)(sym->value));
		printf((++i & 03) ? "   " : "%s\n",CE);
	}
	if (i & 03)
		printf("%s\n",CE);
}

set_reg(regname, new)
char *regname;
int new;
{
	int dot;

	dot = lookup(regname, &err_flag, internal);
	if (err_flag == 0)
		SET_REG(dot,new);       /* set the register */
	else
		badreg();
}

sys_regs()
{
	int MFSR();

	print_registers(sys_internal, sys_reg_mask, MFSR);
}

sys_reg(regname, new)
char *regname;
int new;
{
	int dot = lookup(regname, &err_flag, sys_internal);

	if (err_flag == 0)
		MTSR(dot,new);       /* set the register */
	else
		badreg();
}

/*
 * get the value of a debugger register
 * "reg"	is the value stored in the symbol table
 */
debug_reg(reg)
int reg;
{
	return(* (int *) reg);
}

/*
 * print out the contents of the debug registers
 */
debug_regs()
{
	print_registers(deb_internal, deb_reg_mask, debug_reg);
}

/*
 * set a debug register
 */
set_debug_reg(regname, value)
char *regname;
int value;
{
	int dot = lookup(regname, &err_flag, deb_internal);

	if (err_flag)
		badreg();
	else
		* (int *) dot = value;
}

#define EFLAGS_FMT "\20\1CF\3PF\5AF\7ZF\10SF\11TF\12IF\13DF\14OF\15IOPL1\16IOPL2\17NT\21RF\21VM"

static unsigned long error_code;

debug_status(state)
	register int state;
{
	register int i;		       /* for watch point check */
	register int eip = REG(t_eip);
	register int eflags = REG(t_eflags);
	int cs = REG(t_cs);
	int dr6 = 0;

	switch (state) {
	case NORMAL_STATE:
		printf("in debugger\n");
		return;
	case BREAK_STATE:
		printf("break point");
		break;
	case CALL_STATE:
		printf("debugger call");
		break;
	case PM_STATE:
		printf("post mortem");
		break;
	case DIVERR_STATE:
		printf("divide error");
		break;
	case OVERFLOW_STATE:
		printf("overflow");
		break;
	case BADOP_STATE:
		printf("bad opcode");
		break;
	case BOUNDS_STATE:
		printf("bounds check");
		break;
	case NODEV_STATE:
		printf("no device");
		break;
	case DOUBLE_STATE:
		printf("double exception");
		break;
	case TSS_STATE:
		printf("tss exception");
		print_seg_except(error_code);
		break;
	case SEGMENT_STATE:
		printf("segment exception");
		print_seg_except(error_code);
		break;
	case STACK_STATE:
		printf("stack exception");
		print_seg_except(error_code);
		break;
	case GP_STATE:
		printf("general protection");
		print_seg_except(error_code);
		break;
	case PAGE_STATE:
		printf("page-fault");
		print_page_except(error_code);
		break;
	case STEP_STATE:		       /* stepping */
		{
		dr6 = mfdr(6)&(DR_BD|DR_BS|DR_BT|DR_B0|DR_B1|DR_B2|DR_B3);
		if (dr6 == DR_BS)
			printf("instruction-step");
		else if (dr6 == DR_BD)
			printf("debug-exception");
		else if (dr6 == DR_BT)
			printf("debug task");
		else if ((dr6|(DR_B0|DR_B1|DR_B2|DR_B3)) == (DR_B0|DR_B1|DR_B2|DR_B3))
			printf("watch-point");
		else if (dr6)
			/* multiple debug exceptions */
			printf("%b",dr6&(DR_BD|DR_BS|DR_BT|DR_B0|DR_B1|DR_B2|DR_B3), "\20\1watch-1\2watch-2\3watch-3\4watch-4\15debug-exception\16instruction-step\17debug-task");
		else
			printf("unknown debug exception");
		break;
		}
/* fall thru in case not instruction step */
	default:
		printf("state=%d", state);
		break;
	}
	printf(": IP = %x ", eip);
	prsym(eip, db_offset, (char *)0); /* symbolicly */
	printf(" flags=%b", eflags, EFLAGS_FMT);

	/* printf out information from cs */
	printf(" IOPL=%d %s", cs&3, (cs&4) ? "ldt" : "gdt");
	if (cs_size == 0 && get_cs_size() == 16)
		printf("16-bit");

	if (step_count > 1)
		printf(" %d remaining steps%s\n", step_count,CE);
	printf("%s\n",CE);
	for (i = 0; i < watch_count; ++i)
		if ((dr6&(1<<i)) || *watch_point[i].addr != watch_point[i].value) {
			prsym((int) watch_point[i].addr, db_offset, "watch point tripped: %x ");
			printf(" %x ==> %x\n", watch_point[i].value, *watch_point[i].addr);
		}
}

show_instn(debug_state)
	register int debug_state;
{
	register int eip;

	if (debug_state != NORMAL_STATE) {
		if (hfetch(eip = REG(t_eip) & iar_mask) != -1)
			{
			printf("%s",CE);
			do_unasm(eip, 1, (char *) 0); /* display 1 inst. */
			}
	}
}

#define IDT_BIT	2
#define EXT_BIT	1
#define U_BIT	4
#define W_BIT	2
#define P_BIT	1

print_seg_except(error_code)
	unsigned long error_code;
{
	printf(" seg=0x%04x%s%s",error_code & (~(IDT_BIT|EXT_BIT) & 0xffff),
		error_code & IDT_BIT ? " IDT" : "",
		error_code & EXT_BIT ? " EXT" : "");
}

print_page_except(error_code)
	unsigned long error_code;
{
	printf(" page=0x%08x%s%s%s",mfcr(2),
		error_code & U_BIT ? " user" : " kernel",
		error_code & W_BIT ? "-write" : "-read",
		error_code & P_BIT ? "-protect" : "-not_present");
}

#define TF 0x100	/* trace flag */

step(count)
	register int count;
{

	DEBUGX(printf("stepping %d\n", count));	/* DEBUG */
	if (debug_state == NORMAL_STATE || debug_state == PM_STATE) {
		not_stopped();
		return;
	}
	step_count = count;
	/* special case for BPT */
	REG(t_eflags) |= TF;		/* single step */
	DEBUGX(printf("trace interrupt requested\n")); /* DEBUG */
	set_watch();			/* set the watch points */
	restart(debug_state, 0);       /* execute at least one (usually) */
}

/*
 * resume execution after an interrupt.
 * v = address of interrupt vector to do a lps on
 * t = 0 if we don't execute at least one instruction
 *     0x10 if we will execution at least one instruction
 */
restart(v, t)
	register int t;
	register int v;
{
#ifdef MACH
	extern int kdb_singlestep;
	static struct tss386 temptss;
#endif
	printf("jdxxx restart v %x, t %x\n",v,t);
	DEBUGX(printf(" ==> eip=%8x flags=%b (my flags=%b) break=%d\n",
		REG(t_eip),
		REG(t_eflags),
		EFLAGS_FMT,
		mff(),
		EFLAGS_FMT,
		break_set));
	/* DEBUG */
	debug_state = NORMAL_STATE;    /* til something happens */
	mtdr(6,0);			/* clear debug status register */
	EXIT_DEBUGGER();		/* leave debugger */
#ifdef MACH
	kdb_singlestep = step_count;		/* tell mach kernel about it */
	if (REG(t_esp) != (int) &regsave[UESP])
		{
		/* stack pointer has moved - we moved the context into a temp
		 * area and then copy it back into the new region. 
		 * TODO: check to see if we're trashing our own stack!
		 */
		int diff  =  REG(t_esp) - ((int) &regsave[UESP]);
		printf("stack pointer moved %x bytes\n", diff);
		bcopy((char *)regsave, (char *) &temptss, UESP * 4);
		bcopy((char *) &temptss, diff + (char *)regsave, UESP * 4);
		db_return(diff + (char *) regsave);
		}
	printf("jdxxx calling db_return\n");
	db_return((char *) regsave);
#else
	iret(0,0);			/* dummy values */
#endif
/* NOT REACHED */
}

/*
 * we have just had an interrupt and have done a task switch to save
 * the registers at 'REGSAVE' and have set up a debugger stack
 * and called 'trap' with the trap type (an integer between 0 and
 * 256).
 * trap will check if we are single stepping and if so it
 * will check for watch and break points and then continue
 * stepping. otherwise we pass control to the debugger command
 * processor.
 */

#define IS_BREAKPOINT 1 /* additional condition on being breakpoint */
trap(type,error)
	register int type;		/* must be register */
	unsigned long error;
{
	register int i;
	register debug_flag = 0;	/* XXX look at dr6 */
	register break_t *eip;
#ifdef _KERNEL
	extern void trap_int3();	/* kernel's int $3 flih */
#endif


	ENTER_DEBUGGER();		/* flip state */
	rm_watch();			/* prevent tripping over ourselves */
	set_regsave();			/* set regsave properly */
#ifndef MACH
	if ((debug_option&OPTION_CR3) == 0)
		mtcr(3, REG(t_pdbr));	/* copy program cr3 */
#endif
	rm_break();			/* get rid of break points
					 * must do it AFTER setting CR3! */
	DEBUGX(printf("trap %x ...", type)); /* DEBUG */
#if defined(_KERNEL) && !defined(MACH)
	/* give int3 back to the kernel */
	setgate(saidt, 3, 0xee, 0, DEF_CS, (int)trap_int3);
#endif
	error_code = error;
	if (type == PCALL_STATE)
		pcall_done(type);	/* process pcall done */
	if (type == BREAK_STATE)
		REG(t_eip) -= 1;	/* point to actual instruction */
	if (type == STEP_STATE && step_count > 1 && debug_flag == 0) {
		--step_count;	       /* count down */
		for (i = watch_count; --i >= 0;)
			if (*watch_point[i].addr != watch_point[i].value &&
			    (watch_point[i].cmd[0] == 0 ||
			    trip_cmd(watch_point[i].cmd, type))) {
				debug_flag = 1;
				goto watch_trip;
			}
		eip = (break_t *) (REG(t_eip) & iar_mask);
		/*
		 * check to see if we've hit a break-point and stop
		 * if we have.
		 */
		for (i = break_count; --i >= 0;)
			if (break_point[i].addr == eip && (break_point[i].counter==0 ||
				--break_point[i].counter == 0)) {
				if (break_point[i].cmd[0] == 0 ||
					trip_cmd(break_point[i].cmd, type)) {
				debug_flag = 2;	/* kill later break check */
				goto watch_trip;
				}
			}
		if ((debug_option & SHOW_REGS) && (option_mask & step_count) == 0) {
			register int old = debug_state;

			debug_state = (int)type;
			printf(HO);	/* home (ibm3101) */
			debug_print(debug_state);
			debug_state = old;
		}
		restart((int)type, 1);
		/* NOT REACHED */
	} else if (type == STEP_STATE && watch_count) {	/* must be watch point etc. */
		int dr6 = mfdr(6);
		for (i = watch_count; --i >= 0;) {
			if (((dr6&(1<<i)) ||
			    *watch_point[i].addr != watch_point[i].value) &&
			    (watch_point[i].cmd[0] == 0 ||
			    trip_cmd(watch_point[i].cmd, type))) {
				debug_flag = 1;
				goto watch_trip;
			}
		}
		if ((dr6 & (DR_BD|DR_BS|DR_BT)) == 0) {
			set_watch();
			set_break();	       /* set break points */
			restart((int)type, 1);	/* watch points executed ok */
		}
	}
	/* continue counting */
watch_trip:			       /* watch point encountered */
/*
 * if single step request is set then clear it so that we do not get
 * more interrupts later.
 */
	i = REG(t_eflags);
	if (i & TF)
		REG(t_eflags) &= ~TF; /* kill trace request */
	if (type == STEP_STATE && debug_flag == 0 && go_step) {
		go_step = 0;	       /* no longer go stepping */
		set_watch();
		set_break();	       /* set break points */
		restart(type, 0);    /* and continue go execution */
	}
	/*
	 * test to see if we have encountered a break-point.
	 * if so test to see if the break-point count has expired.
	 * if not we continue execution by stepping past the break-point.
	 */
	if (type == BREAK_STATE && debug_flag == 0 && IS_BREAKPOINT) {
		eip = (break_t *) (REG(t_eip) & iar_mask);
		for (i = break_count; --i >= 0;)
			if (break_point[i].addr == eip) {
				if ((break_point[i].counter && --break_point[i].counter != 0)) {
					DEBUGX(printf("step past breakpoint\n")); /* DEBUG */
					go_step = 1;	/* step past break point */
					debug_state = type;
					step(1);	/* and go! */
					}
				else if (break_point[i].temp) {
					break_delete(i);  /* remove temp */
				} else if (break_point[i].cmd[0])
					if (trip_cmd(break_point[i].cmd, type) == 0) {
						go_step = 1;	/* step past break point */
						debug_state = type;
						step(1);	/* and go! */
					}
			}
	}
	if (screen_buffer && screen_save) {
		screen_restore(screen_buffer, screen_save, screen_size);
		screen_saved = 1;
	}
	if (debug_option & SHOW_REGS) {
		printf(HO);		/* home (ibm3101) */
		_debugger(type,"$");
	} else
		_debugger(type,"status");
	restart(debug_state, 0);       /* restart execution again */
}

/*
 * execute a watch point or breakpoint command
 */
static trip_cmd(cmd, type)
char *cmd;
int type;
{
	int state = debug_state;
	int result;

	debug_state = type;
	result = debug_cmd(cmd, 1);
	debug_state = state;
	return(result);
}

/*
 * continue execution
 * flag = 1	start at given (dot) address
 */

debug_go(flag, dot)
int flag, dot;
{
	printf("jdxxx debug_go flag %x, dot %x\n",flag,dot);
	if (flag)
		REG(t_eip) =  dot; /* set IAR */
	if (get_break(REG(t_eip) & iar_mask) >= 0) {
		DEBUGX(printf("step past breakpoint\n")); /* DEBUG */
		go_step = 1;
		step(1);
	}
	go_step = 0;
	step_count = 0;			/* don't step it */
	set_watch();			/* set the watch points */
	set_break();			/* set the break points */
	restart(debug_state, 0);	/* doesn't return here */
}

#define MIN_ADDR 0x100	/* minimal address we print symbolicly */
int xprsym(start, offset, fmt)
	register unsigned start, offset;
	register char *fmt;
{
	register unsigned nstart = start & iar_mask; /* get masked version */
	register struct symtab *s = closest(nstart, offset);

	if (fmt)
		xprintf(fmt, start);
	if (nstart >= MIN_ADDR && s) {
		xprintf("%s", s->symbol);
		if (s->value != nstart)
			xprintf("+0x%x ", nstart - s->value);
		else
			xprintf(" ");
	}
	return (s != 0);
}

set_watch()
{
	int i;
	int dr7 = 0;

	if (watch_count == 0)
		mtdr(7,0);	/* no debug flags enabled */
	else
		{
		for (i=0; i<watch_count; ++i)
			{
			DEBUGX(printf("set_watch: %d %x dr7=%x\n", i, watch_point[i].addr,dr7));
			watch_point[i].value = *watch_point[i].addr;
			if (i>=4)
				continue;
			DEBUGX(printf("mtdr(%d,%x)\n", i, watch_point[i].addr)); /* DEBUG */
			mtdr(i, watch_point[i].addr);	/* set the address */
			dr7 |= ((watch_point[i].type&0x0f) << (16 + (i<<2))) |
				(DR_G0 << (i<<1));
			}
		DEBUGX(printf("mtdr(7,%x)", dr7|DR_GE)); /* DEBUG */
		mtdr(7,dr7|DR_GE);	/* debug flags enabled */
		DEBUGX(printf(".\n")); /* DEBUG */
		}
	if (screen_saved) {
		screen_restore(screen_save, screen_buffer, screen_size);
		screen_saved = 0;
	}
}

cvt_watch_type(how)
char *how;
{
	int rwe = 0;
	int len = 0;
	for (; *how; ++how)
		switch(*how)
			{
		case 'E':
			rwe = 1;
			break;
		case 'W':
			rwe = 2;
			break;
		case 'R':
			rwe = 4;
			break;
		case 'w':
			len = 4;
			break;
		case 'h':
			len = 2;
			break;
		case 'b':
			len = 1;
			break;
		default:
			goto bad;
			}
		if (rwe == 1)
			len = 1;
		if (rwe == 0 || len == 0)
bad:			printf("watch type not [RWE][bhw]\n");
	return (((len-1) << 2) | (rwe-1));
}

char *pr_wtype(type)
int type;
{
	static char wtype[3];

	wtype[0] = "EWXR"[(type)&3];
	wtype[1] = "bhXw"[(type>>2)&3];
	wtype[2] = 0;
	return(wtype);
}

/*
 * locate the next symbol (in case the current symbol spans two
 * symbol table slots).
 */
struct symtab *next_sym(s)
register struct symtab *s;
{
	while (s->symbol[MAXSYMLEN-1])
		++s;
	return(s+1);
}
