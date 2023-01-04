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
static char *rcsid = "@(#)$RCSfile: BehRAM8x72_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:24:22 $";
#endif
behram8x72_code( adr )
     Net_Entry **adr;
{
  extern void BehRAM8x72();
  B72 t_lastDin;
  B72 t_din;
  B72 t_dout;
  register Net_Entry *z;

  t_lastDin.top8 = (*(adr+13))->value;
  t_lastDin.low64 [0] = (*(adr+12))->value;
  t_lastDin.low64 [1] = (*(adr+11))->value;
  t_din.top8 = (*(adr+9))->value;
  t_din.low64 [0] = (*(adr+8))->value;
  t_din.low64 [1] = (*(adr+7))->value;
  t_dout.top8 = (*(adr+2))->value;
  t_dout.low64 [0] = (*(adr+1))->value;
  t_dout.low64 [1] = (*(adr+0))->value;

  BehRAM8x72( t_lastDin, (*(adr+10))->value, t_din, (*(adr+6))->value, (*(adr+5))->value, (*(adr+4))->value, (*(adr+3))->value, &t_dout );
  z = (*(adr+2));
  assert_output( z, t_dout.top8 );
  z = (*(adr+1));
  assert_output( z, t_dout.low64 [0] );
  z = (*(adr+0));
  assert_output( z, t_dout.low64 [1] );
}
