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
static char *rcsid = "@(#)$RCSfile: zbeh.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:48:25 $";
#endif
#include <stdio.h>
#include "lyreTypes.h"
#include "parts_c.h"
#include "types.h"
#include "vars.h"

#include "RasterOperator_parts.c"
BEHAVIORAL RasterOperator(B2x8 source,
			  B16 dest,
			  B8 asyncData,
			  B4 ropFunct,
			  Signal enbRop,
			  Signal selAsync,
			  B16 *dataOut)
{
  int src;

  if (selAsync) {
    src = asyncData & 0xff;
    src |= src << 8;
  }
  else
    src = (source [1] & 0xff) << 8 | source [0] & 0xff;

  if (enbRop)
    *dataOut = RopFunc (src, dest, ropFunct);
  else
    *dataOut = src;
}


#include "sComparitor_parts.c"
BEHAVIORAL sComparitor(B8 a, B8 b,
		       signal *aGEb,
		       signal *aLb,
		       signal *aEb,
		       signal *aLEb, 
		       signal *aGb,
		       signal *aNEb)
{
  *aLb=0; *aLEb=0; *aEb=0;
  *aGb=0; *aNEb=0; *aGEb=0;

  if(a<b)   *aLb = -1;
  if(a<=b) *aLEb = -1;
  if(a==b)  *aEb = -1;
  if(a>b)   *aGb = -1;
  if(a!=b) *aNEb = -1;
  if(a>=b) *aGEb = -1;
}

#include "ZCheck_parts.c"
BEHAVIORAL ZCheck(B3x8 a,
		  Bits24 b,
		  Signal z16Sel,
		  Bits3 zTest,
		  Signal *zcomp)
{
  unsigned aval, bval;

  aval = (a[0] & 0xff)
       | (a[1] & 0xff) << 8
       | (a[2] & 0xff) << 16;

  bval = b & 0xffffff;

  if (z16Sel) {
    aval &= 0xffff00;
    bval &= 0xffff00;
  }

  switch (zTest & 7) {
  case /* 000 */ GL_GEQUAL:
    *zcomp = (aval >= bval ? -1 : 0);
    break;

  case /* 001 */ GL_ALWAYS:
    *zcomp = -1;
    break;

  case /* 010 */ GL_NEVER:
    *zcomp = 0;
    break;

  case /* 011 */ GL_LESS:
    *zcomp = (aval <  bval ? -1 : 0);
    break;

  case /* 100 */ GL_EQUAL:
    *zcomp = (aval == bval ? -1 : 0);
    break;

  case /* 101 */ GL_LEQUAL:
    *zcomp = (aval <= bval ? -1 : 0);
    break;

  case /* 110 */ GL_GREATER:
    *zcomp = (aval >  bval ? -1 : 0);
    break;

  case /* 111 */ GL_NOTEQUAL:
    *zcomp = (aval != bval ? -1 : 0);
    break;

  default:
    fprintf (stderr, "PANIC! unrecognized zTest value in ZCheck (%d)", zTest);
    exit (1);
  }
}

static int panic = 0;

#include "Match_parts.c"
BEHAVIORAL Match(Signal a)
{
  extern int  ticks;
  extern int  cmdLine;
  extern int  vramLine;

  if (panic)
    exit (1);

  if (a) {
    fprintf (stderr, "PANIC! behavioral/structural mismatch\n");
    fprintf (stderr, "cmdLine = %d, near vramLine = %d\n",
	     cmdLine, vramLine);
    fprintf (stderr, "**** clock tick %d\n", ticks);
    ++panic;
  }
}
