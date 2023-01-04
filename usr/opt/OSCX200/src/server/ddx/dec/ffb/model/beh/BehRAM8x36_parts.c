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
static char *rcsid = "@(#)$RCSfile: BehRAM8x36_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:24:04 $";
#endif
behram8x36_code( adr )
     Net_Entry **adr;
{
  extern void BehRAM8x36();
  WrBuffType t_din;
  WrBuffType t_dout;
  int new_z [1];
  register Net_Entry *z;

  t_din.bank = (*(adr+22))->value;
  t_din.addr = (*(adr+21))->value;
  t_din.data [0] = (*(adr+20))->value;
  t_din.data [1] = (*(adr+19))->value;
  t_din.mask = (*(adr+18))->value;
  t_din.barrier = (*(adr+17))->value;
  t_din.m4 = (*(adr+16))->value;
  t_din.cmd = (*(adr+15))->value;
  t_din.reRas = (*(adr+14))->value;
  t_dout.bank = (*(adr+9))->value;
  t_dout.addr = (*(adr+8))->value;
  t_dout.data [0] = (*(adr+7))->value;
  t_dout.data [1] = (*(adr+6))->value;
  t_dout.mask = (*(adr+5))->value;
  t_dout.barrier = (*(adr+4))->value;
  t_dout.m4 = (*(adr+3))->value;
  t_dout.cmd = (*(adr+2))->value;
  t_dout.reRas = (*(adr+1))->value;

  BehRAM8x36( t_din, (*(adr+13))->value, (*(adr+12))->value, (*(adr+11))->value, (*(adr+10))->value, &t_dout, &new_z[0] );
  z = (*(adr+9));
  assert_output( z, t_dout.bank );
  z = (*(adr+8));
  assert_output( z, t_dout.addr );
  z = (*(adr+7));
  assert_output( z, t_dout.data [0] );
  z = (*(adr+6));
  assert_output( z, t_dout.data [1] );
  z = (*(adr+5));
  assert_output( z, t_dout.mask );
  z = (*(adr+4));
  assert_output( z, t_dout.barrier );
  z = (*(adr+3));
  assert_output( z, t_dout.m4 );
  z = (*(adr+2));
  assert_output( z, t_dout.cmd );
  z = (*(adr+1));
  assert_output( z, t_dout.reRas );
  z = (*(adr+0));
  assert_output( z, new_z[0] );
}
