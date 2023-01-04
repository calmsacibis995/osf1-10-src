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
/*
 * @(#)$RCSfile: parts_c.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:43:21 $
 */
/*
 * this typedef is for the benefit of the behavioral 
 * code modules which include this header.
 */
typedef void BEHAVIORAL;
#define OUTPUT

#undef DEBUG

#ifndef PRESTO

#include "flv_structs.h"

extern int *net_fanout[];

#define sched_fanout(n) \
{ \
  register char **i; \
  if((n)->fanout_list) \
    for(i = (n)->fanout_list;*i;i++) \
	**i = 1; \
}

#define assert_output(a,new_a) \
{ \
  if(a->value != (new_a)) \
    { \
	sched_fanout(a); \
	a->value = new_a; \
    } \
}

#define uncond_assert_output(a,new_a) \
{ \
    sched_fanout(a); \
    a->value = new_a; \
}

#define assert_comp_outputs(a,new_a,b,new_b) \
{ \
  if(a->value != (new_a)) \
    { \
	sched_fanout(a); \
	a->value = new_a; \
    } \
  if(b->value != (new_b)) \
    { \
	sched_fanout(b); \
	b->value = new_b; \
    } \
}

#define uncond_assert_comp_outputs(a,new_a,b,new_b) \
{ \
  sched_fanout(a); \
  a->value = new_a; \
  sched_fanout(b); \
  b->value = new_b; \
}

#endif /* PRESTO */

