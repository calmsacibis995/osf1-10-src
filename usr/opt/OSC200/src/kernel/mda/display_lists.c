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
static char	*sccsid = "@(#)$RCSfile: display_lists.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:08 $";
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
#include "lock_stats.h"
#include "slock_stats.h"
#include "mach_ltracks.h"
#include "uni_compat.h"
#include "unix_uni.h"
#include "mda.h"
#include <mmax_apc.h>
#include <pty.h>
#include <sys/param.h>
#include <kern/thread.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <vm/vm_map.h>
#include <strings.h>
#include <sys/user.h>
#include <sys/callout.h>
#include <mmaxio/crqdefs.h>


vm_map_entry_t ve0 = (vm_map_entry_t) 0;
struct thread *th0 = (struct thread *) 0;
struct callout *callout0 = (struct callout *) 0;

char *pt_ttyadr = (char *)-1;

char	Canttranslate[] = "mda: Unable to translate virtual address %#x\n";

char	*truefalse();




display_prot(prot)
     vm_prot_t prot;
{
  int cnt;
  char *space = "   ";

  cnt = 0;			/* # of prot letters printed */
  if ((prot & VM_PROT_READ) == VM_PROT_READ)
    {
      printf("R");
      cnt++;
    }
  if ((prot & VM_PROT_WRITE) == VM_PROT_WRITE)
    {
      printf("W");
      cnt++;
    }
  if ((prot & VM_PROT_EXECUTE) == VM_PROT_EXECUTE)
    {
      printf("E");
      cnt++;
    }
  printf("%.*s", (3-cnt), space);

  return(SUCCESS);
}


char * status_text(code)
     long code;
{
	if (code == 0)
	  return("Successful");
	else if (code == 1)
	  return("Successful with warning");
	else if (code == 2)
	  return("Operation Pending");
	else if (code == 3)
	  return("Command Queued");
	else if (code == 4)
	  return("Block Free");
	else if (code == 5)
	  return("Operation Aborted");
	else if (code == 6)
	  return("Failed due to Invalid Argument");
	else if (code == 7)
	  return("Failed due to bad Opcode");
	else if (code == 8)
	  return("Operation Failed");
	else if (code == 9)
	  return("Operation timed out");
	else return("Unrecognized Status Code");
}


display_crq_msg_brief(vmsg, pmsg)
     crq_msg_t *vmsg, *pmsg;
{
	int result;

	result = phys(vmsg, &pmsg, ptb0);
	if (result != SUCCESS)
	  return(FAILED);
	pmsg = (crq_msg_t *)MAPPED(pmsg);
	printf("%#10x: %4d %#4x %#8x %2d - %s\n",
	       vmsg, pmsg->crq_msg_code, pmsg->crq_msg_unitid,
	       pmsg->crq_msg_refnum, pmsg->crq_msg_status,
	       status_text(pmsg->crq_msg_status));
}



display_callout_brief(vc, pc)
     struct callout *vc, *pc;
{
  int result;
  char *symbol;
  int size;
  int value, diff;

  result = phys(vc, &pc, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  pc = (struct callout *)(MAPPED(pc));
  printf("%#x: %#10x  %#10x  ",
	 vc, pc->c_time, pc->c_arg);
  result = get_symbol(&symbol, &size, &value, pc->c_func);
  if (result != SUCCESS)
    {
      result = get_data_symbol(&symbol, &size, &value, pc->c_func);
    }

  if ( (result == SUCCESS) && (pc->c_func == (int *)value) )
    printf(" %.*s", size, symbol);
  else
    if (result == SUCCESS)
      {
	diff = pc->c_func - (int *)value;
	printf(" %.*s+%#x", size, symbol, diff);
      }
    else
      printf("%#x", pc->c_func);
  printf("\n");
}


display_vm_map_entry_brief(ve, pe)
     vm_map_entry_t ve, pe;
{
  vm_prot_t curprot, maxprot;

  pe = (vm_map_entry_t)(MAPPED(pe));
  printf("%#x: %#10x %#10x %#10x %#10x ",
	 ve, pe->vme_start, pe->vme_end, pe->object, pe->offset);
  display_prot(pe->protection);
  printf("  ");
  display_prot(pe->max_protection);
  printf("\n");
}


char state_str[20];


sdisplay_state(state)
     int state;
{
  boolean_t needcomma;

  needcomma = FALSE;
  state_str[0] = '\0';
  if (state & TH_WAIT)
    {
      needcomma = TRUE;
      strcat(state_str,"W");
    }

  if (state & TH_SUSP)
    {
      if (needcomma)
	strcat(state_str,",");
      needcomma = TRUE;
      strcat(state_str,"S");
    }

  if (state & TH_RUN)
    {
      if (needcomma)
	strcat(state_str,",");
      needcomma = TRUE;
      strcat(state_str,"R");
    }

  if (state & TH_SWAPPED)
    {
      if (needcomma)
	strcat(state_str,",");
      needcomma = TRUE;
      strcat(state_str,"SW");
    }

  if (state & TH_IDLE)
    {
      if (needcomma)
	strcat(state_str,",");
      needcomma = TRUE;
      strcat(state_str,"I");
    }

  return(SUCCESS);
}



extern char *master_runq_adr;
extern boolean_t thread_on_master;

display_thread_brief(vth,pth)
     thread_t vth, pth;
{
  task_t task;
  int state, event, result;
  struct pcb * pcb;
  char *symbol;
  int size, len;
  int value, diff;
  char *spaces = "           ";
  struct utask *utp;
  char *cmd;

  pth = (thread_t)MAPPED(pth);
  PHYS(task,&vth->task);
  PHYS(state,&vth->state);
  PHYS(pcb,&vth->pcb);
  PHYS(event,&vth->wait_event);
  PHYS(utp,&vth->u_address.utask);
  task = (task_t)MAP(task);
  state = (int)MAP(state);
  pcb = (struct pcb *)MAP(pcb);
  event = (int)MAP(event);
  utp = (struct utask *)MAP(utp);
  PHYS(cmd,&utp->uu_comm[0]);
  cmd = (char *)MAPPED(cmd);

  printf("%#10x", vth);
  if (pth->bound_processor == (processor_t) master_processor)
    {
      thread_on_master = TRUE;
      printf("*");
    }
    else
    printf(" ");
  printf(" %#10x   ", task);
  sdisplay_state(state);
  printf("%s",state_str);
  len = strlen(state_str);
  printf("%.*s", (10-len), spaces);
  printf(" %#10x", pcb);
  if (*cmd == '\0177')
    printf("           ");
  else
    printf(" %11.11s", cmd);

  if (event != 0)
    {
      result = get_symbol(&symbol, &size, &value, event);
      if (result != SUCCESS)
	{
	  result = get_data_symbol(&symbol, &size, &value, event);
	}

      if ( (result == SUCCESS) && (event == value) )
	printf(" %.*s", size, symbol);
      else
	if (result == SUCCESS)
	  {
	    diff = event - value;
	    printf(" %.*s+%#x", size, symbol, diff);
	  }
	else
	  if ((event >(int)vth) && (event < (int)(vth+sizeof(struct thread))) )
	    printf(" (thread)+%#x", (event-(int)vth));
	  else
	    if ( (event > (int)pt_ttyadr)
		&& (event < (int)(pt_ttyadr + NPTY*sizeof(struct tty))) )
	      {
		printf(" pt_tty[%d]",
		       (event-(int)pt_ttyadr)/sizeof(struct tty));
	      }
	    else
	      printf("	%x", event);
    }
  else
    printf("	%x", event);

  printf("\n");
}



display_links(hdr, hdroffset, offset, display_entry)
     queue_head_t *hdr;
     int	hdroffset;
     int	offset;
     void 	(*display_entry)();
{
  char	*va, *pa;
  int	result;

  if ( queue_empty(hdr) )
    {
      printf("mda: Queue at 0x%.8x is empty\n");
      return(SUCCESS);
    }
  va = hdr;
  result = phys(va,&pa,ptb0);
  if (result != SUCCESS)
    {
      printf(Canttranslate, va);
      return(FAILED);
    }

  va = (char *)MAP(pa + hdroffset);
  while (TRUE)
    {
      result = phys(va,&pa,ptb0);
      if (result != SUCCESS)
	{
	  printf(Canttranslate, va);
	  return(FAILED);
	}
      if (hdr == (char *)va)
	return(SUCCESS);
      (*display_entry)(va,pa);
      va = va + offset;
      result = phys(va,&pa,ptb0);
      if (result != SUCCESS)
	{
	  printf(Canttranslate, va);
	  return(FAILED);
	}
      va = (char *)MAP(pa);
    }
}



display_list(hdr, hdroffset, offset, display_entry)
     char	*hdr;
     int	hdroffset;
     int	offset;
     void 	(*display_entry)();
{
  char	*va, *pa;
  int	result;

  va = hdr;
  result = phys(va,&pa,ptb0);
  if (result != SUCCESS)
    {
      printf(Canttranslate, va);
      return(FAILED);
    }

  va = (char *)MAP(pa + hdroffset);
  while (va)
    {
      result = phys(va,&pa,ptb0);
      if (result != SUCCESS)
	{
	  printf(Canttranslate, va);
	  return(FAILED);
	}
      if ((va ==0) || (hdr == (char *)va))
	return(SUCCESS);
      (*display_entry)(va,pa);
      va = va + offset;
      result = phys(va,&pa,ptb0);
      if (result != SUCCESS)
	{
	  printf(Canttranslate, va);
	  return(FAILED);
	}
      va = (char *)MAP(pa);
    }
}

display_pset_threads(vpset, ppset)
     processor_set_t	vpset;
     processor_set_t	ppset;
{
  int *offset;

  offset = (int *) &th0->pset_threads;
  display_list(&vpset->threads, 0, offset, display_thread_brief);
}


