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
static char *rcsid = "@(#)$RCSfile: BehRAM.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:20:11 $";
#endif
#include <stdio.h>
#include "lyreTypes.h"
#include "parts_c.h"

extern FILE *cmdFile;
extern int cmdLine, lastMatchLine;

#include "BehRAM8x36_parts.c"
BEHAVIORAL BehRAM8x36(WrBuffType din,
		      B3 wradr,
		      signal we,
		      B3 rdadr,
		      Pointer p,
		      WrBuffType *dout,
		      Pointer *pout)
{
  WrBuffType *pmem;

  if (p == NULL) {
    p = (Pointer) malloc(8 * sizeof(WrBuffType));
    if (p == NULL) {
      fprintf(stderr, "PANIC! BehRAM8x36: malloc returned NULL.\n");
      if (cmdFile)
	fprintf (stderr, "**** malloc NULL: cmdLine = %d, near vramLine = %d\n",
		 cmdLine, lastMatchLine);
      exit (1);
    }
  }
  pmem = (WrBuffType *) p;

  wradr &= 7;
  rdadr &= 7;

  if (we)
    pmem [wradr] = din;

  *dout = pmem [rdadr];
  *pout = p;

#if 0
  {
    static int written = 0;

    if (we) ++written;

    if (written) {
      fprintf(stderr, "**** rda8x36 (waddr=%d, wdata=%-.2x%-.8x%-.8x, wenb=%d, raddr=%d, rdata=%-.2x%-.8x%-.8x\n",
	      wradr,
	      din.top8,
	      din.low64[1],
	      din.low64[0],
	      we,
	      rdadr,
	      memContents [rdadr].top8,
	      memContents [rdadr].low64[1],
	      memContents [rdadr].low64[0]);
    }
  }
#endif
}

#include "BehRAM8x72_parts.c"
BEHAVIORAL BehRAM8x72(B72 lastDin,
		      B3 lastWradr,
		      B72 din,
		      B3 wradr,
		      B3 rdadr,
		      signal we,
		      signal lastWe,
		      B72 *dout)
{
  static B72 memContents[8];
  extern int initSfb, ticks;
  
  lastWradr &= 7;
  wradr &= 7;
  rdadr &= 7;

  if (lastWe) {
    if (wradr != lastWradr) {
      if (!initSfb) {
	fprintf (stderr, "BehRAM8x72: write address hold violation (%d -> %d)\n",
		 lastWradr, wradr);
	if (cmdFile)
	  fprintf (stderr, "**** address hold: cmdLine = %d, near vramLine = %d\n",
		   cmdLine, lastMatchLine);
	ExitSfb (0);
      }
    }
    if (din.top8 != lastDin.top8
	|| din.low64[1] != lastDin.low64[1]
	|| din.low64[0] != lastDin.low64[0]) {
      if (!initSfb) {
	fprintf (stderr, "BehRAM8x72: data hold violation (%-.2x.%-.8x.%-.8x -> %-.2x.%-.8x.%-.8x)\n",
		 lastDin.top8, lastDin.low64[1], lastDin.low64[0], 
		 din.top8, din.low64[1], din.low64[0]);
	if (cmdFile)
	  fprintf (stderr, "**** data hold: cmdLine = %d, near vramLine = %d\n",
		   cmdLine, lastMatchLine);
	ExitSfb (0);
      }
    }
    if (!initSfb && we && ticks > 0) {
      fprintf (stderr, "BehRAM8x72: we asserted for more than 1 tick at time %d\n",
	       ticks);
      if (cmdFile)
	fprintf (stderr, "**** we assertion: cmdLine = %d, near vramLine = %d\n",
		 cmdLine, lastMatchLine);
      ExitSfb (0);
    }
  }
#if 0
  {
    static int written = 0;

    if (we) ++written;

    if (written) {
      printf ("**** rda8x72 (waddr=%d, wdata=%-.2x%-.8x%-.8x, wenb=%d, raddr=%d, rdata=%-.2x%-.8x%-.8x\n",
	      wradr,
	      din.top8,
	      din.low64[1],
	      din.low64[0],
	      we,
	      rdadr,
	      memContents [rdadr].top8,
	      memContents [rdadr].low64[1],
	      memContents [rdadr].low64[0]);
    }
  }
#endif

  if (we) 
    memContents [wradr] = din;

  *dout = memContents [rdadr];
}

#include "BehRDA8x36_parts.c"
BEHAVIORAL BehRDA8x36(B36 lastDin,     B36 *dout,
		      B3 lastWradr,
		      B36 din,
		      B3 wradr,
		      B3 rdadr,
		      Signal we,
		      Signal lastWe,
		      Signal init,
		      Pointer p,        Pointer *pout)
{
  B36 *pmem;
/*
 * Do not init the ram until the init pulse arrives
 */
  if ((p == NULL) && init) {
    p = (Pointer) malloc(8 * sizeof(B36));
    if (p == NULL) {
      fprintf(stderr, "PANIC! BehRDA8x36: malloc returned NULL.\n");
      if (cmdFile)
	fprintf (stderr, "**** malloc NULL: cmdLine = %d, near vramLine = %d\n",
		 cmdLine, lastMatchLine);
      exit (1);
    }
  }
/*
 * If the pointer is null return (assume it is uninitialized)
 */
  if (p == NULL) return;

  pmem = (B36 *) p;
  
  lastWradr &= 7;
  wradr &= 7;
  rdadr &= 7;

  if (lastWe) {
    if (wradr != lastWradr) {
      fprintf (stderr, "BehRDA8x63: write address hold violation (%d -> %d)\n",
	       lastWradr, wradr);
      if (cmdFile)
	fprintf (stderr, "**** address hold: cmdLine = %d, near vramLine = %d\n",
		 cmdLine, lastMatchLine);
      ExitSfb (1);
    }
    if (!init && ((din.hi != lastDin.hi) || (din.lo != lastDin.lo))) {
      fprintf (stderr, "BehRDA8x36: data hold violation (%-.2x.%-.8x -> %-.2x.%-.8x)\n",
	       lastDin.hi, lastDin.lo, din.hi, din.lo);
      if (cmdFile)
	fprintf (stderr, "**** data hold: cmdLine = %d, near vramLine = %d\n",
		 cmdLine, lastMatchLine);
      ExitSfb (1);
    }
/*
 * Only check for multiple we cycles after init
 */
    if (we && !init) {
      fprintf (stderr, "BehRDA8x36: we asserted for more than 1 tick\n");
      if (cmdFile)
	fprintf (stderr, "**** we assertion: cmdLine = %d, near vramLine = %d\n",
		 cmdLine, lastMatchLine);
      ExitSfb (1);
    }
  }

  if (we)
    pmem [wradr] = din;

  *dout = pmem [rdadr];
  *pout = p;
}

#include "BehRAM16x9_parts.c"
BEHAVIORAL BehRAM16x9(B9 lastDin,
		      B4 lastWradr,
		      B9 din,
		      B4 wradr,
		      B4 rdadr,
		      signal we,
		      signal lastWe,
		      B9 *dout)
{
  static int memContents[16];
  extern int initSfb, ticks;
  
  if (initSfb) return;

  if (we && (wradr < 0 || wradr > 15 || rdadr < 0 || rdadr > 15)) {
    fprintf(stderr, "PANIC! Stencil FIFO address out of range tick %d: wradr = %d, rdadr = %d\n",
	    ticks, wradr, rdadr);
    if (cmdFile)
      fprintf (stderr, "**** bad address: cmdLine = %d, near vramLine = %d\n",
	       cmdLine, lastMatchLine);
    ExitSfb(0);
    return;
  }

  if (lastWe) {
    if (wradr != lastWradr) {
      fprintf (stderr, "BehRAM16x9: write address hold violation (%d -> %d)\n",
	       lastWradr, wradr);
      if (cmdFile)
	fprintf (stderr, "**** address hold: cmdLine = %d, near vramLine = %d\n",
		 cmdLine, lastMatchLine);
      ExitSfb (1);
    }
    if (din != lastDin) {
      fprintf (stderr, "BehRAM16x9: data hold violation (%-.3x -> %-.3x)\n",
	       lastDin, din);
      if (cmdFile)
	fprintf (stderr, "**** data hold: cmdLine = %d, near vramLine = %d\n",
		 cmdLine, lastMatchLine);
      ExitSfb (1);
    }
    if (we) {
      fprintf (stderr, "BehRAM16x9: we asserted for more than 1 tick\n");
      if (cmdFile)
	fprintf (stderr, "**** we assertion: cmdLine = %d, near vramLine = %d\n",
		 cmdLine, lastMatchLine);
      ExitSfb (1);
    }
  }
#if 0
  printf ("**** BehRAM16x9 (waddr=%d, wdata=%-.3x, wenb=%d, raddr=%d, rdata=%-.3x\n",
	  wradr, din, we, rdadr, memContents[rdadr]);
#endif
  if (we) 
    memContents[wradr] = din;

  *dout = memContents[rdadr];
}


