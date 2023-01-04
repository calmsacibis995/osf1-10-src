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
static char *rcsid = "@(#)$RCSfile: BehAdder36_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:22:31 $";
#endif
behadder36_code( adr )
     Net_Entry **adr;
{
  extern void BehAdder36();
  B36 t_in1;
  B36 t_in2;
  B36 t_out;
  register Net_Entry *z;

  t_in1.hi = (*(adr+5))->value;
  t_in1.lo = (*(adr+4))->value;
  t_in2.hi = (*(adr+3))->value;
  t_in2.lo = (*(adr+2))->value;
  t_out.hi = (*(adr+1))->value;
  t_out.lo = (*(adr+0))->value;

  BehAdder36( t_in1, t_in2, &t_out );
  z = (*(adr+1));
  assert_output( z, t_out.hi );
  z = (*(adr+0));
  assert_output( z, t_out.lo );
}
