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
static char	*sccsid = "@(#)$RCSfile: display-lists.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:36:47 $";
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
#include "mda.h"
#include "mmax_apc.h"
#include "pty.h"
#include "kern/queue.h"
#include "kern/thread.h"
#include "vm/vm_map.h"


vm_map_entry_t ve0 = (vm_map_entry_t) 0;
struct thread *th0 = (struct thread *) 0;

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


display_vm_map_entry_brief(ve, pe)
     vm_map_entry_t ve, pe;
{
  vm_prot_t curprot, maxprot;

  pe = (vm_map_entry_t)(MAPPED(pe));
  printf("%#x: %#10x %#10x %#10x %#10x ",
	 ve, pe->start, pe->end, pe->object, pe->offset);
  display_prot(pe->protection);
  printf("  ");
  display_prot(pe->max_protection);
  printf("\n");
}


display_thread_brief(vth,pth)
     thread_t vth, pth;
{
  task_t task;
  int state, event, result;
  struct pcb * pcb;
  boolean_t needcomma;
  char *symbol;
  int size;
  int value, diff;

  printf("%x	", vth);
  pth = (thread_t)MAPPED(pth);
  task = (task_t)(pth->task);
  state = (int) (pth->state);
  pcb = (struct pcb *)((pth->pcb));
  event = pth->wait_event;

  printf("%x	", task);
  needcomma = FALSE;
  if (state & TH_WAIT)
    {
      needcomma = TRUE;
      printf("W");
    }
  if (state & TH_SUSP)
    {
      if (needcomma)
	printf(",");
      needcomma = TRUE;
      printf("S");
    }
  if (state & TH_RUN)
    {
      if (needcomma)
	printf(",");
      needcomma = TRUE;
      printf("R");
    }
  if (state & TH_SWAPPED)
    {
      if (needcomma)
	printf(",");
      needcomma = TRUE;
      printf("SW");
    }
  if (state & TH_IDLE)
    {
      if (needcomma)
	printf(",");
      needcomma = TRUE;
      printf("I");
    }

  printf("	%x", pcb);

  result = get_symbol(&symbol, &size, &value, event);
  if (result != SUCCESS)
    {
      result = get_data_symbol(&symbol, &size, &value, event);
    }

  if ( (result == SUCCESS) && (event == value) )
    printf(" %.*s\n", size, symbol);
  else
    if (result == SUCCESS)
      {
	diff = event - value;
	printf(" %.*s+%#x\n", size, symbol, diff);
      }
  else
    if ( (event > (int)vth) && (event < (int)(vth+sizeof(struct thread))) )
      printf(" (thread)+%#x\n", (event-(int)vth));
  else
    if ( (event > (int)pt_ttyadr)
	&& (event < (int)(pt_ttyadr + NPTY*sizeof(struct tty))) )
      {
	printf(" pt_tty[%d]\n", (event-(int)pt_ttyadr)/sizeof(struct tty));
      }
  else
    printf("	%x\n", event);

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
      va = (char *)MAP(pa + offset);
    }
}

