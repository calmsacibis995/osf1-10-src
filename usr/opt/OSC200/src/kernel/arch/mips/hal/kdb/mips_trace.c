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
static char	*sccsid = "@(#)$RCSfile: mips_trace.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:12:18 $";
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
 * derived from mips_trace.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

#include <hal/kdb/defs.h>
#include <machine/inst.h>
#include <machine/reg.h>
#include <hal/kdb/mips_trace.h>
#include <machine/xpr.h>

#define	DEBUG 0

extern short adrflg;
extern unsigned long adrval, cntval;
extern long userpc;
extern char *symtab;
extern REGLIST reglist[];
extern int start();	/* lowest kernel code address */

#define IS_JAL(x)	(x.j_format.opcode == jal_op)
#define IS_JALR(x)	((x.j_format.opcode == spec_op) && \
			 (x.r_format.func == jalr_op))
#define IS_RETURN(x)	((x.j_format.opcode == spec_op) && \
			 (x.r_format.func == jr_op) && \
			 (x.r_format.rs == 31))
#define IS_RFE(x)	((x.j_format.opcode == cop0_op) && \
			 (x.f_format.func == 16))
#define STORES_REG(x,n)	((x.j_format.opcode == sw_op) && \
			 (x.i_format.rt == n))
#define SAVES_RA(x)	STORES_REG((x),31)

#define REG_ARG(i)	(4+i)

int kdbtracedebug;

mips_printtrace(modif)
{
	unsigned stackp, framep, argp, newfp;
	long int curpc, oldra, newra;
	int narg, nloc;
	unsigned minpc;
	int top_frame = 1;
	struct frame_info f;
	char *save_symtab = symtab;
	int *esp;
	extern int kdbtrapflag;

	minpc = VM_MIN_ADDRESS;
	if (modif == 'k' || modif == 'n')
		minpc = (unsigned)start;

	if (adrflg) {
		/* figure out a stack from where user says */
		stackp = adrval;
		curpc = userpc;
		newra = curpcb->pcb_regs[PCB_RA];
	} else {
		/* figure it out from what we know */
		if (1) {
			if (kdbtrapflag && curpcb == &kdbpcb) {
				esp = (int *)var[varchk('t')];
				printf("<Exception frame> ");
				printf("Cause= 0x%X Badvaddr= 0x%X SR= 0x%X\n",
				   esp[EF_CAUSE], esp[EF_BADVADDR], esp[EF_SR]);
			}
			/* kernel: use pcb */
			stackp = curpcb->pcb_regs[PCB_SP];
			curpc  = curpcb->pcb_regs[PCB_PC];
			newra  = curpcb->pcb_regs[PCB_RA];
			framep = curpcb->pcb_regs[PCB_FP];
		} else {
			/* user: use registers */
			stackp = *(long *) (((long) & u) + R29);
			curpc = *(long *) (((long) & u) + R_PC);
			newra = *(long *) (((long) & u) + R31);
		}
	}
	if (curpc == 0)
		goto out;/* sanity */
	if(newra) newra -= 8;
	oldra = newra;
	newfp = framep;

	while (cntval--) {
		extern short ld_screwed_up;	/* No params info */

		chkerr();

		bzero(&f, sizeof f);

		/* 1: find out where we are */
		findsym(curpc, ISYM);
		if (cursym)
			printf ("%s(", cursym->n_name);
		else
			printf("%R(", guess_procedure(curpc));

		/* 2: find out more about it */
		guess_frame(curpc, oldra, &f);
if (kdbtracedebug)
  printf("\n\tframereg= %D framep = %R\n", f.framereg, framep);
		nloc = f.nloc;
		if (!symtab || ld_screwed_up)
			narg = 4;
		else
			narg = f.narg;

		/*
		 * Where are the arguments ?
		 * Where is the next frame ?
		 */
if (kdbtracedebug) printf("\tcurpc = %R newra = %R\n", curpc, newra);
		if (top_frame) {
			if (f.at_entry)
if (kdbtracedebug) printf("\tat_entry\n");
				argp = framep + 4 * 4;
			if (f.mod_sp) {
if (kdbtracedebug) printf("\tmod_sp\n");
				/* New frame allocated */
                                if (f.framereg == SP_REGISTER)
					framep = stackp + f.framesize;
				argp = framep;
			}
			/* Top procedure made any calls ? */
			if (!f.at_entry && f.mod_sp)
				goto saved_ra;
		} else {
			if (f.isvector) {
				/* Mach traps are "varargs": */
				stackp = stackp + f.framesize - EF_SIZE;	
				curpc = chkget(stackp + 4*EF_EPC, DSP);
				prints(")\n");
				if (modif == 'n')
					print_exception_frame(stackp, curpc);
				oldra = newra = chkget(stackp + 4*EF_RA, DSP) - 8;
				stackp = chkget(stackp + 4*EF_SP, DSP);
#if	DEBUG
				printf("{Vector %R %R %R %R}\n",
					curpc, oldra, newra, stackp);
#endif	DEBUG
				if (((unsigned)curpc) < K0BASE)
					if (modif == 'k' || modif == 'n')
						goto out;
					else {
						/* crossing to user state */
printf("User stack:\n");
						symtab = 0;
					}
				top_frame++;
				continue;
			}
			/* Frame is allocated, or else */
			if (f.framereg == SP_REGISTER)
				framep = stackp + f.framesize;
			argp = framep;
			if (f.at_entry) {
				/* Not the top frame and RA not saved ?
				 * Must be start, or else we dont know
				 */
				newra = 0;
			} else {
saved_ra:
				newra = get(framep + f.saved_pc_off, DSP);
				newfp = get(framep + f.saved_fp_off, DSP);
if (kdbtracedebug) printf("\tnewra= %R newfp= %R\n", newra, newfp);
#if	DEBUG
				printf("newra = {%R}", newra);
#endif	DEBUG
				if (newra < minpc)
					newra = 0;
				else
					newra -= 8;
			}
		}
if (kdbtracedebug) printf("\targp= %R narg= %D\n", argp, narg);

		/* Print arguments */
		if (top_frame) {
			register int i;
			for (i = 0; narg && i < 4; i++, argp += 4) {
				if (saves_arg(curpc, REG_ARG(i))) {
					printf("%R", get (argp, DSP));
				} else
					printf("%R", reg_val(REG_ARG(i)));
				if (--narg)
					printc(',');
			}
		}
		for (; narg--; argp += 4) {
			printf("%R", get (argp, DSP));
			if (narg)
				printc(',');
		}


		/* 3: find out how we got here */
		if (newra) {
		    prints (") from ");
		    psymoff (newra, PSYM, "\n");
		}
		else
		    prints (")\n");


#ifdef notyet
		/* 4: possibly print local vars */
		if (modif == 'C') {
			unsigned w;
			extern STRING errflg;
			findsym(curpc,ISYM);
			while (localsym(stackp,argp)) {
				w = chkget(localval,DSP);
				printf("%8t%s:%10t",
					    cursym->n_name);
				if (errflg) {
					prints("?\n");
					errflg=0;
				} else
					printf("%R\n", w);
			}
		}
#endif	notyet

		/* 5: find out where we go after this */
		if (f.mod_sp)
			stackp = framep;
		if ((curpc = newra) == 0 || (framep = newfp) == 0)
			break;

if (kdbtracedebug) printf("\tBottom: framep = %R\n", framep);
		oldra = newra;
		top_frame = 0;
	}
out:
#if	DEBUG
	printf("{fp=%R, esp=%R}\n", framep, curpcb->pcb_regs[PCB_SP]);
#endif	DEBUG
	symtab = save_symtab;
}

guess_procedure(pc)
unsigned pc;
{
    union mips_instruction w;
    int             bw_pc, fw_pc;

    if (pc < (unsigned)start)
    	fw_pc = VM_MIN_ADDRESS;
    else
        fw_pc = (unsigned)start;

    for (bw_pc = pc; bw_pc >= fw_pc; bw_pc -= 4) {
	w.word = get(bw_pc, ISP);
	if (IS_RETURN(w))
	    break;
    }
    return (bw_pc < fw_pc) ?
	fw_pc : bw_pc + 8;
}

int curproc;

guess_frame(pc, fpc, fr)
frame_info_t fr;
long int fpc, pc;
{
	int inc;
	union mips_instruction w;
	int bw_pc, fw_pc, binc, finc;

	curproc = 0;

	/* If symtab, believe it */
	if (symtab) {
		findsym(pc, ISYM);
		curproc = cursym->n_value;
	}
	else
		curproc = guess_procedure(pc);

	if (curproc == 0)
		curproc = pc;

	/*
	 * Given the pc, figure out how we might have
	 * modified the sp.
	 */
	if (symtab) {
		findproc(curproc, fr);
		fr->at_entry = 1;
		/* maybe the sp has not yet been modified */
		inc = 0;
	       	for (fw_pc = curproc; fw_pc < pc; fw_pc += 4) {
	       		w.word = chkget ( fw_pc, ISP);
	       		inc -= mods_sp(w,fr);
	       		if (SAVES_RA(w))
	       			fr->at_entry = 0;
		      }
		 fr->mod_sp = (inc != 0);
                 if (fr->isvector) {
		   if (inc && inc != fr->framesize) {
#if	DEBUG
				printf("[?frame %R != %R]", inc, fr->framesize);
#endif	DEBUG
				/* For Mach traps VEC_syscall is varargs */
				findsym(curproc, ISYM);
				if (eqsym(cursym->n_name,"VEC_syscall",'~'))
				  fr->framesize += inc;
			      }
		 }
		return;
	      }

	/* No symtab, play guessing games */

	/* Guess 1: did we save the RA and where, or not */
	fr->at_entry = 1;
	inc = 0;
	for (fw_pc = curproc; fw_pc < pc; fw_pc += 4) {
		w.word = get ( fw_pc, ISP);
		inc -= mods_sp(w,fr);
		if (SAVES_RA(w))
			fr->at_entry = 0;
	}

	/* Guess 2: did we alter the SP */
	fr->mod_sp = (inc > 0);
	fr->framesize = inc;
	fr->nloc = inc / 4;

	/* Defaults for the unknowns */
	fr->narg = 0;
	fr->isleaf = 0;
	fr->isvector = 0;
	fr->saved_pc_off = -4;
}

static reg_val(r)
{
	/* XXX this is not quite what I need */
	if (r == 0) return 0;
	return 	*(long *) (((long) & u) + reglist[r].roffs);
}

static int mods_sp(x,fr)
register union mips_instruction x;
frame_info_t fr;
{
	short simmed;

	/* frame is mods by add and sub instructions. else you lose */
	switch (x.j_format.opcode) {
	case spec_op:
		switch (x.r_format.func) {
		case add_op:
		case addu_op:
			if (x.r_format.rd != fr->framereg) return 0;
			return reg_val(x.r_format.rs)+reg_val(x.r_format.rt)-reg_val(fr->framereg);
			break;
		case sub_op:
		case subu_op:
			if (x.r_format.rd != fr->framereg) return 0;
			return reg_val(x.r_format.rs)-reg_val(x.r_format.rt)-reg_val(fr->framereg);
			break;
		default: return 0;
		}
		break;
	case addi_op:
	case addiu_op:
		if (x.i_format.rt != fr->framereg) return 0;
		simmed = x.i_format.simmediate;	/* work around gcc bug */
		return  simmed;
	default:return 0;
	}
}

static int saves_arg(pc,n)
long pc;
{
	long fw_pc;
	union mips_instruction w;

	for (fw_pc = curproc; fw_pc < pc; fw_pc += 4) {
		w.word = chkget(fw_pc , ISP);
		if (STORES_REG(w,n))
			return TRUE;
	}
	return FALSE;
}

static print_exception_frame(fp, epc)
unsigned fp, epc;
{
	printf("%8t Exception: taken at pc=%R, frame (at %R) :\n", epc, fp);

	printf("%16t arg0=%-14R arg1=%-14R arg2=%-14R arg3=%-14R\n",
	    chkget(fp+0, DSP), chkget(fp+4, DSP), chkget(fp+8, DSP), chkget(fp+12, DSP));
	printf("%16t at=%-16R v0=%-16R v1=%-16R a0=%-16R\n",
	    chkget(fp+16, DSP), chkget(fp+20, DSP), chkget(fp+24, DSP), chkget(fp+28, DSP));
	printf("%16t a1=%-16R a2=%-16R a3=%-16R t0=%-16R\n",
	    chkget(fp+32, DSP), chkget(fp+36, DSP), chkget(fp+40, DSP), chkget(fp+44, DSP));
	printf("%16t t1=%-16R t2=%-16R t3=%-16R t4=%-16R\n",
	    chkget(fp+48, DSP), chkget(fp+52, DSP), chkget(fp+56, DSP), chkget(fp+60, DSP));
	printf("%16t t5=%-16R t6=%-16R t7=%-16R s0=%-16R\n",
	    chkget(fp+64, DSP), chkget(fp+68, DSP), chkget(fp+72, DSP), chkget(fp+76, DSP));
	printf("%16t s1=%-16R s2=%-16R s3=%-16R s4=%-16R\n",
	    chkget(fp+80, DSP), chkget(fp+84, DSP), chkget(fp+88, DSP), chkget(fp+92, DSP));
	printf("%16t s5=%-16R s6=%-16R s7=%-16R t8=%-16R\n",
	    chkget(fp+96, DSP), chkget(fp+100, DSP), chkget(fp+104, DSP), chkget(fp+108, DSP));
	printf("%16t t9=%-16R k0=%-16R k1=%-16R gp=%-16R\n",
	    chkget(fp+112, DSP), chkget(fp+116, DSP), chkget(fp+120, DSP), chkget(fp+124, DSP));
	printf("%16t sp=%-16R fp=%-16R ra=%-16R sr=%-16R\n",
	    chkget(fp+128, DSP), chkget(fp+132, DSP), chkget(fp+136, DSP), chkget(fp+140, DSP));
	printf("%16t lo=%-16R hi=%-16R bad=%-15R cs=%-16R\n",
	    chkget(fp+144, DSP), chkget(fp+148, DSP), chkget(fp+152, DSP), chkget(fp+156, DSP));
	printf("%16t epc=%-16R\n",
	    chkget(fp+160, DSP));
}


isa_rei(ins)
	register union mips_instruction ins;
{
	return IS_RFE(ins);
}

isa_ret(ins)
	register union mips_instruction ins;
{
	return IS_RETURN(ins);
}

isa_call(ins)
	register union mips_instruction ins;
{
	return ( IS_JAL(ins) || IS_JALR(ins) );
}
