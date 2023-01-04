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
static char *rcsid = "@(#)$RCSfile: PrintFIFO_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:33:21 $";
#endif
printfifo_code( adr )
     Net_Entry **adr;
{
  extern void PrintFIFO();
  WrBuff4xType t_wrBuff;
  int new_z [1];
  register Net_Entry *z;

  t_wrBuff [0].bank = (*(adr+38))->value;
  t_wrBuff [0].addr = (*(adr+37))->value;
  t_wrBuff [0].data [0] = (*(adr+36))->value;
  t_wrBuff [0].data [1] = (*(adr+35))->value;
  t_wrBuff [0].mask = (*(adr+34))->value;
  t_wrBuff [0].barrier = (*(adr+33))->value;
  t_wrBuff [0].m4 = (*(adr+32))->value;
  t_wrBuff [0].cmd = (*(adr+31))->value;
  t_wrBuff [0].reRas = (*(adr+30))->value;
  t_wrBuff [1].bank = (*(adr+29))->value;
  t_wrBuff [1].addr = (*(adr+28))->value;
  t_wrBuff [1].data [0] = (*(adr+27))->value;
  t_wrBuff [1].data [1] = (*(adr+26))->value;
  t_wrBuff [1].mask = (*(adr+25))->value;
  t_wrBuff [1].barrier = (*(adr+24))->value;
  t_wrBuff [1].m4 = (*(adr+23))->value;
  t_wrBuff [1].cmd = (*(adr+22))->value;
  t_wrBuff [1].reRas = (*(adr+21))->value;
  t_wrBuff [2].bank = (*(adr+20))->value;
  t_wrBuff [2].addr = (*(adr+19))->value;
  t_wrBuff [2].data [0] = (*(adr+18))->value;
  t_wrBuff [2].data [1] = (*(adr+17))->value;
  t_wrBuff [2].mask = (*(adr+16))->value;
  t_wrBuff [2].barrier = (*(adr+15))->value;
  t_wrBuff [2].m4 = (*(adr+14))->value;
  t_wrBuff [2].cmd = (*(adr+13))->value;
  t_wrBuff [2].reRas = (*(adr+12))->value;
  t_wrBuff [3].bank = (*(adr+11))->value;
  t_wrBuff [3].addr = (*(adr+10))->value;
  t_wrBuff [3].data [0] = (*(adr+9))->value;
  t_wrBuff [3].data [1] = (*(adr+8))->value;
  t_wrBuff [3].mask = (*(adr+7))->value;
  t_wrBuff [3].barrier = (*(adr+6))->value;
  t_wrBuff [3].m4 = (*(adr+5))->value;
  t_wrBuff [3].cmd = (*(adr+4))->value;
  t_wrBuff [3].reRas = (*(adr+3))->value;

  PrintFIFO( t_wrBuff, (*(adr+2))->value, (*(adr+1))->value, &new_z[0] );
  z = (*(adr+0));
  assert_output( z, new_z[0] );
}
