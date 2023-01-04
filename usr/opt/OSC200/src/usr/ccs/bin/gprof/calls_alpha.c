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
    static    char *sccsid = "@(#)calls.c    1.4 (Berkeley) 3/30/83";
#endif not lint

#include    "gprof.h"

    /*
     *    a namelist entry to be the child of indirect calls
     */
nltype    indirectchild = {
    "(*)" ,               /* the name */
    (unsigned long) 0 ,   /* the pc entry point */
    (unsigned long) 0 ,   /* entry point aligned to histogram */
    (double) 0.0 ,        /* ticks in this routine */
    (double) 0.0 ,        /* cumulative ticks in children */
    (long) 0 ,            /* how many times called */
    (long) 0 ,            /* how many calls to self */
    (double) 1.0 ,        /* propagation fraction */
    (double) 0.0 ,        /* self propagation time */
    (double) 0.0 ,        /* child propagation time */
    (bool) 0 ,            /* print flag */
    (int) 0 ,             /* index in the graph list */
    (int) 0 ,             /* graph call chain top-sort order */
    (int) 0 ,             /* internal number of cycle on */
    (struct nl *) &indirectchild ,    /* pointer to head of cycle */
    (struct nl *) 0 ,     /* pointer to next member of cycle */
    (arctype *) 0 ,       /* list of caller arcs */
    (arctype *) 0         /* list of callee arcs */
    };

findcall(nltype *parentp, unsigned long p_lowpc, unsigned long p_highpc)
{
    long length;
    nltype *childp;
    unsigned long destpc;
    unsigned int *ip;

    if (textspace == 0) { return; }
    if (p_lowpc < s_lowpc) { p_lowpc = s_lowpc; }
    if (p_highpc > s_highpc) { p_highpc = s_highpc; }

    /*
     * Look through the instructions for *jump*.
     * Functions called by name will use bsr, functions called
     * through pointers use jmp or jsr.
     */
    for( ip=(unsigned int *)(textspace+(p_lowpc-s_lowpc)) ;
        ip < (unsigned int *)(textspace+(p_highpc-s_lowpc)) ; ip++)
    {   /* If the high 6 bits are a 0x34, we have a BSR */
      if ((*ip >> 26) == 0x34)
      {   destpc = ((unsigned long)textspace - (unsigned long)ip + s_lowpc);
          destpc += (*ip & 0xFFFFFL) << 2L;
            if( destpc >= s_lowpc && destpc <= s_highpc )
            {   if ((childp = nllookup(destpc))->value == destpc)
                    addarc(parentp, childp, 0L);
#ifdef DEBUG
              if( debug & CALLDEBUG )
                  printf("[findcalls] (bsr) instr=%0x in %s destpc=%0lx in name=%s\n",
                      *ip,parentp->name,destpc,childp->name);
#endif DEBUG
            }
        } else
      /* If the top 6 bits are 0x1A, and the next bit is 0, we have JMP|JSR */
      if ((*ip >> 25) == (0x1A << 1))
        {   addarc(parentp, &indirectchild, 0L);
#ifdef DEBUG
            if( debug & CALLDEBUG )
                printf("[findcalls] (jmp|jsr) instr=%0x in %s in %s\n",
                    *ip,parentp->name,indirectchild.name);
#endif DEBUG
        }
    }
}
