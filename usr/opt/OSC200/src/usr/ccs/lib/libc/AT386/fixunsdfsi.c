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
static char	*sccsid = "@(#)$RCSfile: fixunsdfsi.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:47:54 $";
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

/* **********************************************************************
 File:         fixunsdfsi.c
 Description:  convert double to unsigned int (runtime support for gcc)
 Author:       Mike Kupfer, Olivetti Research Center
********************************************************************** */

/* 
 * This code has a wart, because the '387 does conversions to 
 * integers, but not conversions to unsigned integers.  So, for values 
 * between max(int) and max(unsigned int), we have use a trick.
 */

double ldexp();
int __fixdfsi();
union double_di { double d; int i[2]; };
#define HIGH 1
#define LOW 0

unsigned int
__fixunsdfsi(arg)
     double arg;
{
  union double_di maxint, maxunsigned;
  unsigned int result;

  maxint.i[HIGH] = 0x41dfffff;		/* ldexp(1.0, 31) - 1.0 */
  maxint.i[LOW] =  0xffc00000;
  maxunsigned.i[HIGH] = 0x41efffff;	/* ldexp(1.0, 32) - 1.0 */
  maxunsigned.i[LOW] =  0xffe00000;

  if (arg <= maxint.d)			/* includes negative values */
    result = (unsigned int)__fixdfsi(arg);
  else if (arg > maxunsigned.d)
    result = __fixdfsi(arg);		/* will cause exception */
  else 
    {
      int onesbit;
      union double_di my_dbl;

      my_dbl.d = arg;
      onesbit = my_dbl.i[LOW] & 0x200000;
      result = __fixdfsi(arg/2) * 2 + (onesbit ? 1 : 0);
    }

  return result;
}
