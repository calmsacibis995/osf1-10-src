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
static char	*sccsid = "@(#)$RCSfile: trace.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:40 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include <sys/unix_defs.h>
#include "mda.h"
#include <stdio.h>
#include <sys/param.h>
#include <kern/thread.h>
#include <mmax/boot.h>
#include <mmax/panic.h>
#include <mmax/psl.h>


/* Max times to bump sp looking for good return adr */
#define MAXFIXES 200
/* Max stack levels to follow before giving up */
#define MAXLEVELS 20
/* Instruction mask for movd */
#define MOVD_MASK 0x3f
/* Op-code value for movd instruction */
#define MOVD_OPCODE 0x17
/* Destination address mask for movd */
#define DEST_MASK 0x7c0
/* TOS Address mode for movd */
#define DEST_TOS 0x5c0
/* Source address mask for movd */
#define SRC_MASK 0xf800
/* Shift to right-justify source in movd */
#define SRC_SHIFT 11
/* Op code for Enter instruction */
#define ENTER_OPCODE (0x82)
/* Op code for jsr instruction */
#define JSR_OPCODE (0x67f)
/* Mask for jsr Op Code */
#define JSR_OPCODE_MASK (0x7ff)
/* Op code for bsr instruction */
#define BSR_OPCODE (0x2)
/* Mask for bsr Op Code */
#define BSR_OPCODE_MASK (0xff)

#define WORDSIZE 4

char *syscallnames = (char*)-1;

extern char *start_text, *text_end;	/* Used to validate pc's */
extern char *master_rett;
extern char *intstkbeg, *intstkend;
extern int nsysent;

trace(stkp, pc)
     char *stkp, *pc;
{
  int result, offset, item_selected, i, disp, last_routine_size;
  int symbol_size, levels;
  char *pc_adr, *psa, *symbol, *instr, *instr_adr, *psr, *psr_adr;
  char *stkp_adr, *pstkp, *last_routine, *call_adr;
  char *svc_adr, symbol_name[256], *interrupt_type, *svcname;
  int value, diff, num_regs, svcnum;
  boolean_t is_kernel_thread;
  int narg, next_inst;


  if (syscallnames == (char *)-1)
    {
      result = get_address("_syscallnames", &syscallnames);
      if (result != SUCCESS)
	{
	  printf("mda: Could not get address of 'syscallnames'\n");
	  return(FAILED);
	}
    }
  call_adr = (char *)0;
  result = get_symbol(&symbol,&symbol_size,&value,pc);
  if (result != SUCCESS)
    return(SUCCESS);
  diff = (int)pc - value;
  for (levels=0; levels<MAXLEVELS; levels++)
    {
	    /*
      if ((strncmp("_thread_block",
		  symbol,
		  (13 < symbol_size ? 13 : symbol_size)) == 0) &&
	  (stkp > intstkbeg) && (stkp < intstkend))
	{
	  printf("mda: Running on interrupt stack at _thread_block+%#x\n",
		 diff);
	  return(SUCCESS);
	}
	*/
      if (strncmp("_idle_thread",
		  symbol,
		  (12 < symbol_size ? 12 : symbol_size)) == 0)
	{
	  printf("mda: Running in _idle_thread\n");
	  return(SUCCESS);
	}

      if (call_adr != (char *)0)
	{
	  next_inst = MAP(call_adr);
	  if ((next_inst & 0x7ff) == 0x57c) 		/* adjspb */
	    narg = (-((next_inst<<8)>>24)) & 0xff;
	  else if ((next_inst & 0x7ff) == 0x57d) 	/* adjspw */
	    narg = -((next_inst<<8)>>24);
	  else if ((next_inst & 0xffff) == 0xbdc7)   /* cmpd tos,tos */
	    narg = 2*WORDSIZE;			/* 2  */
	  else if ((next_inst & 0xffff) == 0xb81f)   /* cmpqd $0,tos */
	    narg = 1*WORDSIZE;			/* 1  */
	  else
	    narg = 0;

/*	  narg /= WORDSIZE;	*/
	  stkp += narg;		/* Account for args passed on stack */
	}

      instr_adr = (char *)MAPPED(value);
      instr = (char *)(MAP(value));
      if ((unsigned char)*instr_adr == (unsigned char)ENTER_OPCODE)
	{
	  num_regs = bits_set_in_byte(*(instr_adr+1));
	  disp = calc_disp(instr_adr);
	  stkp += disp;			/* account for local params    */
	  stkp += (4 * num_regs);	/* account for saved registers */
	  stkp += (4 * 1);		/* account for frame pointer   */
	}
      else
	while (((*instr_adr & MOVD_MASK) == MOVD_OPCODE)
	       && ((*(unsigned short *)(instr_adr) & DEST_MASK) == DEST_TOS)
	       && (strncmp(symbol,"_assfail",7)!=0))
	  {
	    stkp += 4;		/* account for a saved register */
	    value += 2;		/* bump pc to next instruction  */
	    instr_adr = (char *)MAPPED(value);
	    instr = (char *)(MAP(value));
	  }
      
      result = phys(stkp,&pstkp,ptb0);
      if (result != SUCCESS)
	{
	  printf ("mda: could not translate address 0x%x\n",stkp);
	  return(FAILED);
	}
      call_adr = (char *)(MAP(pstkp));

      result = check_in_text(&call_adr,&stkp);
      if (result != SUCCESS)
	{
	  printf("mda: Unable to unravel stack further (sorry!)\n");
	  return(FAILED);
	}

      result = check_branch(&call_adr,&stkp);
      if (result != SUCCESS)
	{
	  printf("mda: previous instruction not jsr\n");
	  return(FAILED);
	}

      last_routine = symbol;
      last_routine_size = symbol_size;
      result = get_symbol(&symbol,&symbol_size,&value,call_adr);
      if (result != SUCCESS)
	return(SUCCESS);
      diff = (int)call_adr - value;
      printf("%x: %.*s called from %.*s+0x%x\n",
	     stkp, last_routine_size, last_routine, symbol_size,
	     symbol, diff);

      if (is_os_thread(symbol)) {
	      printf("\t%s is a kernel thread.\n", symbol);
	      return(SUCCESS);
      }

      if (is_os_thread(last_routine)) {
	      printf("\t%s is a kernel thread.\n", last_routine);
	      return(SUCCESS);
      }

      if ((strcmp(symbol,"svc") == 0)
	  || (strcmp(symbol,"acall.cmplxout") == 0)
	  || (strcmp(symbol,"get_on_with_it") == 0)
	  || (strcmp(symbol,"_syscallret") == 0))
	{
	  skip_frame(&call_adr,&stkp,&psr);
	  check_psr(&call_adr, &psr, &stkp);
	  result = phys(stkp, &pstkp, ptb0);
	  if (result != SUCCESS)
	    return(FAILED);
	  result = get_symbol(&symbol,&symbol_size,&value,call_adr);
	  if (result != SUCCESS)
	    {
	      symbol = "";
	    }
	  diff = (int)call_adr - value;
	  if (((int)psr & PSL_U) == 0)
	      printf("\tsvc trap from pc %x (%s+0x%x), psr %x\n",
		     call_adr, symbol, diff, psr);
	  else
	    {
	      svcnum = MAP(pstkp-12);
	      if (svcnum < nsysent && svcnum >= 0)
		{
		  svcname = (char *)(MAP(syscallnames+(svcnum*4)));
		  svcname = (char *)(MAPPED(svcname));
		}
	      else
		      if (svcnum < 0 && svcnum > -10)
			      svcname = "CMU system call";
		      else if (svcnum < 0 && svcnum < -10)
			      svcname = "Mach system call";
		      else
			      svcname = "illegal svc";
	      printf("\tsvc trap %d (%s) from user pc %x, psr %x\n",
		     svcnum, svcname, call_adr, psr);
	    }
	  return(SUCCESS);
	}
      if (strcmp(symbol,"abt_2") == 0)
	{
	  skip_frame(&call_adr,&stkp,&psr);
	  check_psr(&call_adr, &psr, &stkp);

	  result = get_symbol(&symbol,&symbol_size,&value,call_adr);
	  if (result != SUCCESS)
	    {
	      symbol = "";
	    }
	  diff = (int)call_adr - value;
	  if (((int)psr & PSL_U) == 0)
	      printf("\tabt trap from kernel pc %x (%s+0x%x), psr %x\n",
		     call_adr, symbol, diff, psr);
	  else
	    {
	      printf("\tabt trap from user pc %x, psr %x\n",
		     call_adr, psr);
	    }
	}

      if (strncmp(symbol,"flg_time",symbol_size) == 0)
	{
	  skip_frame(&call_adr,&stkp,&psr);
	  check_psr(&call_adr, &psr, &stkp);
	  result = phys(stkp, &pstkp, ptb0);
	  if (result != SUCCESS)
	    return(FAILED);

	  result = get_symbol(&symbol,&symbol_size,&value,call_adr);
	  if (result != SUCCESS)
	    {
	      symbol = "";
	    }
	  diff = (int)call_adr - value;
	  if (((int)psr & PSL_U) == 0)
	      printf("\tflg trap from kernel pc %x (%s+0x%x), psr %x\n",
		     call_adr, symbol, diff, psr);
	  else
	    {
	      printf("\tflg trap from user pc %x, psr %x\n",
		     call_adr, psr);
	      return(SUCCESS);
	    }
	}

      if (strncmp(symbol,"_master_rett",symbol_size) == 0)
	{
	  stkp += 8;	/* skip over pc, psr for rett */
	  result = phys(stkp,&pstkp,ptb0);
	  if (result != SUCCESS)
	    {
	      printf ("mda: could not translate address 0x%x\n",stkp);
	      return(FAILED);
	    }
	  pc = (char *)(MAP(pstkp));
	  psr = (char *)(MAP(pstkp+4));
	  printf("\tTrap returning to pc %x, psr %x\n",
		 pc, psr);

	  if (((int)psr & PSL_U) == 0)
	      trace(stkp,call_adr);
	  return(SUCCESS);
	}

      if (strncmp(symbol,"_master_reti",symbol_size) == 0) {
	      stkp += 4;	/* skip over pc, psr for rett */
	      result = phys(stkp,&pstkp,ptb0);
	      if (result != SUCCESS) {
		      printf ("mda: could not translate address 0x%x\n",stkp);
		      return(FAILED);
	      }
	      pc = (char *)(MAP(pstkp));
	      psr = (char *)(MAP(pstkp+4));
	      printf("\tTrap returning to pc %x, psr %x\n",
		     pc, psr);
	      
	      if (((int)psr & PSL_U) == 0)
		      trace(stkp,call_adr);
	      return(SUCCESS);
      }

      if ( (strcmp(symbol,"intexit") == 0)
	  || (strcmp(symbol,"int_handled") == 0)
	  || (strcmp(symbol, "int.1") == 0)
	  || (strcmp(symbol,"tse.1") == 0)
	  || (strcmp(symbol,"tseout1") == 0) )
	{
	  if (strcmp(symbol,"intexit") == 0
	      || strcmp(symbol,"int_handled") == 0
	      || strcmp(symbol,"int.1") == 0)
	    interrupt_type = "vecbus interrupt";
	  if (strcmp(symbol,"tse.1") == 0
	      || strcmp(symbol,"tseout1") == 0 )
	    interrupt_type = "tse interrupt";

	  stkp +=4;		/* Skip over value pushed by time_int_entry */
	  skip_frame(&call_adr,&stkp,&psr);
	  check_psr(&call_adr, &psr, &stkp);

	  result = get_symbol(&symbol,&symbol_size,&value,call_adr);
	  if (result != SUCCESS)
	    {
	      symbol = "";
	    }
	  diff = (int)call_adr - value;
	  if (((int)psr & PSL_U) == 0) {
		  printf("* %s from pc %x (%.*s+0x%x), psr %x *\n",
			 interrupt_type, call_adr, symbol_size, symbol,
			 diff, psr);
		  if (is_os_thread(symbol)) {
			  printf("\t%s is a kernel thread.\n", symbol);
			  return(SUCCESS);
		  }
		  trace(stkp,call_adr);
	   } else
		  printf("\t%s from user-mode pc %x with psr %x\n",
			 interrupt_type, call_adr, psr);
	  return(SUCCESS);
	}

      if ((strcmp(symbol,"nmi") == 0) ||
	  (strncmp(symbol,"nmi.chk3",8) == 0))
	{
	  printf("%s trap from pc %#x\n", symbol, call_adr);
	  return(SUCCESS);
	}

      if (strcmp(symbol, "unexpected") == 0)
	{
	  skip_frame(&call_adr,&stkp,&psr);
	  check_psr(&call_adr, &psr, &stkp);

	  result = get_symbol(&symbol,&symbol_size,&value,call_adr);
	  if (result != SUCCESS)
	    {
	      symbol = "";
	    }
	  diff = (int)call_adr - value;
	  if (((int)psr & PSL_U) == 0) {
		 printf("\t'unexpected' trap from pc %x (%.*s+0x%x), psr %x\n",
			 call_adr, symbol_size, symbol, diff, psr);
		  if (is_os_thread(symbol)) {
			  printf("\t%s is a kernel thread.\n", symbol);
			  return(SUCCESS);
		  }
		  trace(stkp,call_adr);
	   } else
		  printf("\t'unexpected' trap from user-mode pc %x with psr %x\n",
			 call_adr, psr);
	  return(SUCCESS);
	}

      stkp += 4;			/* Account for return address */
    }
  printf("mda: Exceeded maximum stack trace levels\n");
  return(SUCCESS);
}



check_branch(call_adr,stkp)
     char **call_adr, **stkp;
{
  char *instr_adr, *instr, *jsr_adr, *pstkp;
  char *bsr_adr;
  int i, result;

  for (i=0; i<MAXFIXES; i++)
    {
      jsr_adr = *call_adr - 6;	/* jsr @adr (most common case) */
      instr = (char *)(MAP(jsr_adr));
      if (((int)instr & JSR_OPCODE_MASK) == JSR_OPCODE)
	{
	  return(SUCCESS);
	}
      jsr_adr = *call_adr - 3;	/* jsr (Rx) */
      instr = (char *)(MAP(jsr_adr));
      if (((int)instr & JSR_OPCODE_MASK) == JSR_OPCODE)
	{
	  return(SUCCESS);
	}
      jsr_adr = *call_adr - 2;	/* jsr Rx */
      instr = (char *)(MAP(jsr_adr));
      if (((int)instr & JSR_OPCODE_MASK) == JSR_OPCODE)
	{
	  return(SUCCESS);
	}


      bsr_adr = *call_adr - 2;	/* one-byte displacement */
      instr = (char *)(MAP(bsr_adr));
      if (((int)instr & BSR_OPCODE_MASK) == BSR_OPCODE)
	{
	  return(SUCCESS);
	}
      bsr_adr = *call_adr - 3;	/* one-byte displacement */
      instr = (char *)(MAP(bsr_adr));
      if (((int)instr & BSR_OPCODE_MASK) == BSR_OPCODE)
	{
	  return(SUCCESS);
	}
      bsr_adr = *call_adr - 5;	/* one-byte displacement */
      instr = (char *)(MAP(bsr_adr));
      if (((int)instr & BSR_OPCODE_MASK) == BSR_OPCODE)
	{
	  return(SUCCESS);
	}

      *stkp +=4;		/* branch not found, look at next stack loc */
      result = phys(*stkp,&pstkp,ptb0);
      if (result != SUCCESS)
	{
	  printf ("mda: could not translate address 0x%x\n",stkp);
	  return(FAILED);
	}
      *call_adr = (char *)(MAP(pstkp));
    }
  return(FAILED);
}




is_os_thread(symbol)
     char *symbol;
{
  boolean_t os_thread;

  os_thread = FALSE;
  if (strcmp(symbol,"_vm_pageout") == 0)
    os_thread = TRUE;
  else if (strcmp(symbol,"_reaper_thread") == 0)
    os_thread = TRUE;
  else if (strcmp(symbol,"_swapin_thread") == 0)
    os_thread = TRUE;
  else if (strcmp(symbol,"_swapout_thread") == 0)
    os_thread = TRUE;
  else if (strcmp(symbol,"_netisr_thread") == 0)
    os_thread = TRUE;
  else if (strcmp(symbol,"_ux_handler") == 0)
    os_thread = TRUE;
  else if (strcmp(symbol,"_tcp_slowthread") == 0)
    os_thread = TRUE;
  else if (strcmp(symbol,"_tcp_fastthread") == 0)
    os_thread = TRUE;
  else if (strcmp(symbol,"_biodone_thread") == 0)
    os_thread = TRUE;

  /*   Should really check THREAD_SYSTEMMODE or some such here */

  return(os_thread);
}


calc_disp(instr_adr)
     char *instr_adr;
{
  int disp;

  disp = (unsigned char)(*(instr_adr+2));
  if ((disp & 0x80) == 0)
    {
      if ((disp & 0x40) == 0x40)
	{
	  disp |= 0x80;	/* It's negative */
	}
    }
  else
    { if ((disp & 0xc0) == 0x80)
	{
	  disp = (disp & 0x3f) << 8;
	  disp |= (unsigned char)(*(instr_adr+3));
	}
    else
      {
	disp = (disp & 0x3f) << 8;
	disp |= (unsigned char)(*(instr_adr+3));
	disp = disp << 8;
	disp |= (unsigned char)(*(instr_adr+4));
	disp = disp << 8;
	disp |= (unsigned char)(*(instr_adr+5));
      }
    }

return(disp);
}


check_in_text(call_adr,stkp)
     char **call_adr, **stkp;
{
  int i, result;
  char *pstkp;

  i = 0;
  while ((*call_adr < start_text) || (*call_adr > text_end))
    {
      if (i++ > MAXFIXES)
	{
	  return(FAILED);
	}
      *stkp += 4;            /* next stack location */
      result = phys(*stkp,&pstkp,ptb0);
      if (result != SUCCESS)
	{
	  printf ("mda: could not translate address 0x%x\n",stkp);
	  return(FAILED);
	}
      *call_adr = (char *)(MAP(pstkp));
    }
  return(SUCCESS);
}


check_psr(call_adr, psr, stkp)
     char **call_adr, **psr, **stkp;
{
  int i, result;
  char *pstkp;
  extern int OSmodpsr;

  i = 0;
  while ((((int)*psr & 0xffff) != 0x20)		/* Normal user mode psr */
	 && (((int)*psr & 0xffff) != 0x50) 	/* mul-t uses this ! */
	 && (((int)*psr & 0xffff) != (OSmodpsr&0xffff))) /* kernel mod */
    {
      if (i++ > MAXFIXES)
	{
	  printf("mda: Cannot unravel stack further (sorry!)\n");
	  return(FAILED);
	}
      *call_adr = *psr;
      *stkp += 4;
      result = phys(*stkp,&pstkp,ptb0);
      if (result != SUCCESS)
	{
	  printf ("mda: could not translate address 0x%x\n",stkp);
	  return(FAILED);
	}
      *psr = (char *)(MAP(pstkp));
    }
}
  
skip_frame(call_adr, stkp, psr)
     char **call_adr, **stkp, **psr;
{
  int i, result;
  char *pstkp;
  

  *stkp += 44;
  result = phys(*stkp,&pstkp,ptb0);
  if (result != SUCCESS)
    {
      printf ("mda: could not translate address 0x%x\n",stkp);
      return(FAILED);
    }
  *call_adr = (char *)(MAP(pstkp));
  *stkp += 4;
  result = phys(*stkp,&pstkp,ptb0);
  if (result != SUCCESS)
    {
      printf ("mda: could not translate address 0x%x\n",stkp);
      return(FAILED);
    }
  *psr = (char *)(MAP(pstkp));
  return(SUCCESS);
}


cpu_data(psa, pc, psr, stkp)
     psa_t *psa;
     char **pc, **psr, **stkp;
{
  psa += current_cpu;
  *pc = (char *)MAP(&psa->psa_pc);
  *psr = (char *)MAP(&psa->psa_psr);
  *stkp = (char *)MAP(&psa->psa_ssp);
  return(SUCCESS);
}


extern caddr_t     panicstr_adr;
extern vm_offset_t panicstr;

check_console_panic(stkp)
     char **stkp;
{
  int result;
  char *pmsg_adr;		/* panic message address */

/*  the following code can't be used until nmipanic_msg is .globl */
/*  result = get_address("nmipanic_msg", pmsg_adr);
    if (result == FAILED)
      return(FAILED);
    pmsg_adr = (char *)(MAPPED(pmsg_adr));
    if (strcmp(panicstr,pmsg_adr) == 0)
*/

  if (strcmp(panicstr,"Fatal NMI Received from Console\n") == 0)
    *stkp += 4 * 12;		/* If panic was induced from the console */
				/* 	restore the stack pointer to its */
				/* 	value when the nmi occurred	 */
  return(SUCCESS);
}
