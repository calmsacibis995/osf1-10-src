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
static char *rcsid = "@(#)$RCSfile: wrapper.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:47:50 $";
#endif
#include <stdio.h>

#include "flv_structs.h"

extern struct gate_call *level_tables[];
extern char *level_flags[];

/*
 * simulate one cycle.
 */
OneCycle( initFlag )
{
  register int *flagWord;
  register char *flag;
  register int i;
  register struct gate_call *gcp;
  register struct gate_call **gc;
  register char **flag_table;

  if( initFlag ) {
    /*
     * this loop ignores all the event flags
     * and just evaluates the entire network.
     */
    for( gc=level_tables; *gc; gc++ )
      {
	gcp = *gc+1;
	for(i = (int)(*gc)->data; i > 0; i--)
	  {
	    (*gcp->func)(gcp->data);
	    gcp++;
	  }
      }
	
  } else {
    /*
     * this loop evaluates only those primitives
     * whose event flag has been set (this includes
     * flops, too).
     *
     * note that the event flag must be cleared
     * *before* the primitive is evaluated.
     */
    for(gc=level_tables, flag_table=level_flags;
	*gc;
	gc++,flag_table++)
      {
	int mod8, j;

	gcp = *gc+1;
	flag = *flag_table+1;
	i = (int)(*gc)->data;

	/*
	 * do (up to) the first 3 entries to get
	 * flag word-aligned.
	 */
	for (j=0; j<3; ++j) {
	  if (i > 0) {
	    if(*flag) {
	      *flag = 0;
	      (*gcp->func)(gcp->data);
	    }
	    gcp++;
	    flag++;
	    --i;
	  }
	}

	mod8 = i & 7;
	i -= mod8;

	flagWord = (int *) flag;

	/*
	 * this loop evaluates 8 devices per pass.
	 */
	while (i > 0) {
	  if (flagWord[0]) {
	    if(*flag) {
	      *flag = 0;
	      (*gcp->func)(gcp->data);
	    }
	    if(*(flag+1)) {
	      *(flag+1) = 0;
	      (*(gcp+1)->func)((gcp+1)->data);
	    }
	    if(*(flag+2)) {
	      *(flag+2) = 0;
	      (*(gcp+2)->func)((gcp+2)->data);
	    }
	    if(*(flag+3)) {
	      *(flag+3) = 0;
	      (*(gcp+3)->func)((gcp+3)->data);
	    }
	  }
	  if (flagWord[1]) {
	    if(*(flag+4)) {
	      *(flag+4) = 0;
	      (*(gcp+4)->func)((gcp+4)->data);
	    }
	    if(*(flag+5)) {
	      *(flag+5) = 0;
	      (*(gcp+5)->func)((gcp+5)->data);
	    }
	    if(*(flag+6)) {
	      *(flag+6) = 0;
	      (*(gcp+6)->func)((gcp+6)->data);
	    }
	    if(*(flag+7)) {
	      *(flag+7) = 0;
	      (*(gcp+7)->func)((gcp+7)->data);
	    }
	  }
	  i -= 8;
	  gcp += 8;
	  flag += 8;
	  flagWord += 2;
	}
	/*
	 * the device evaluation loop is unrolled 8 times.
	 * first do device count MOD 8 devices.
	 */
	while (mod8) {
	  if(*flag) {
	    *flag = 0;
	    (*gcp->func)(gcp->data);
	  }
	  gcp++;
	  flag++;
	  --mod8;
	}
      }
  }
/*	
    for(gc=level_tables;*gc;gc++)
      for(gcp = *gc;gcp->func;gcp++)
	if(gcp->eval_flag)
	  {
	    gcp->eval_flag = 0;
	    (*gcp->func)(gcp->data);
	  }
*/    
}

/*
 * evaluate the gates in the circuit (but not the flops).
 */
EvaluateGates( initFlag )
{
  register struct gate_call **gc,*gcp;
  register int i;
  register char **flag_table;
  register char *flag;

  if( initFlag ) {
    /*
     * this loop ignores all the event flags
     * and just evaluates the entire network.
     */
    for( gc=level_tables+1; *gc; gc++ )
      {
	gcp = *gc+1;
	for(i = (int)(*gc)->data; i > 0; i--)
	  {
	    (*gcp->func)(gcp->data);
	    gcp++;
	  }
      }
	
  } else {
    /*
     * this loop evaluates only those primitives
     * whose event flag has been set (this includes
     * flops, too).
     *
     * note that the event flag must be cleared
     * *before* the primitive is evaluated.
     */
    for(gc=level_tables+1, flag_table=level_flags;
	*gc;
	gc++,flag_table++)
      {
	gcp = *gc+1;
	flag = *flag_table+1;
	i = (int)(*gc)->data;
	/*
	 * the device evaluation loop is unrolled 8 times.
	 * first do device count MOD 8 devices.
	 */
	while (i & 7)
	  {
	    if(*flag)
	      {
		*flag = 0;
		(*gcp->func)(gcp->data);
	      }
	    gcp++;
	    flag++;
	    --i;
	  }
	
	/*
	 * this loop evaluates 8 devices per pass.
	 */
	while (i > 0)
	  {
	    if(*flag)
	      {
		*flag = 0;
		(*gcp->func)(gcp->data);
	      }
	    if(*(flag+1))
	      {
		*(flag+1) = 0;
		(*(gcp+1)->func)((gcp+1)->data);
	      }
	    if(*(flag+2))
	      {
		*(flag+2) = 0;
		(*(gcp+2)->func)((gcp+2)->data);
	      }
	    if(*(flag+3))
	      {
		*(flag+3) = 0;
		(*(gcp+3)->func)((gcp+3)->data);
	      }
	    if(*(flag+4))
	      {
		*(flag+4) = 0;
		(*(gcp+4)->func)((gcp+4)->data);
	      }
	    if(*(flag+5))
	      {
		*(flag+5) = 0;
		(*(gcp+5)->func)((gcp+5)->data);
	      }
	    if(*(flag+6))
	      {
		*(flag+6) = 0;
		(*(gcp+6)->func)((gcp+6)->data);
	      }
	    if(*(flag+7))
	      {
		*(flag+7) = 0;
		(*(gcp+7)->func)((gcp+7)->data);
	      }
	    i -= 8;
	    gcp += 8;
	    flag += 8;
	  }
      }
  }
}

Deposit (pnet, value)
Net_Entry *pnet;
{
  if (pnet->value != value)
    {
      pnet->value = value;
      sched_fanout (pnet);
    }
}


extern int *net_fanout[];

sched_fanout(n)
     Net_Entry *n;
{
  char **i;
  if(n->fanout_list)
    for(i = n->fanout_list;*i;i++)
      {
	**i = 1;
      }
}
