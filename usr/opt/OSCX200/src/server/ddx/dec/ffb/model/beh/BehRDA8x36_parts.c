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
static char *rcsid = "@(#)$RCSfile: BehRDA8x36_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:24:33 $";
#endif
behrda8x36_code( adr )
     Net_Entry **adr;
{
  extern void BehRDA8x36();
  B36 t_lastDin;
  B36 t_dout;
  B36 t_din;
  int new_z [1];
  register Net_Entry *z;

  t_lastDin.hi = (*(adr+13))->value;
  t_lastDin.lo = (*(adr+12))->value;
  t_dout.hi = (*(adr+11))->value;
  t_dout.lo = (*(adr+10))->value;
  t_din.hi = (*(adr+8))->value;
  t_din.lo = (*(adr+7))->value;

  BehRDA8x36( t_lastDin, &t_dout, (*(adr+9))->value, t_din, (*(adr+6))->value, (*(adr+5))->value, (*(adr+4))->value, (*(adr+3))->value, (*(adr+2))->value, (*(adr+1))->value, &new_z[0] );
  z = (*(adr+11));
  assert_output( z, t_dout.hi );
  z = (*(adr+10));
  assert_output( z, t_dout.lo );
  z = (*(adr+0));
  assert_output( z, new_z[0] );
}
