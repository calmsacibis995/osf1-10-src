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
static char *rcsid = "@(#)$RCSfile: ReadFIFO_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:35:30 $";
#endif
readfifo_code( adr )
     Net_Entry **adr;
{
  extern void ReadFIFO();
  WrBuff4xType t_wrBuff;
  DestType t_dest;
  int new_z [6];
  register Net_Entry *z;

  t_wrBuff [0].bank = (*(adr+48))->value;
  t_wrBuff [0].addr = (*(adr+47))->value;
  t_wrBuff [0].data [0] = (*(adr+46))->value;
  t_wrBuff [0].data [1] = (*(adr+45))->value;
  t_wrBuff [0].mask = (*(adr+44))->value;
  t_wrBuff [0].barrier = (*(adr+43))->value;
  t_wrBuff [0].m4 = (*(adr+42))->value;
  t_wrBuff [0].cmd = (*(adr+41))->value;
  t_wrBuff [0].reRas = (*(adr+40))->value;
  t_wrBuff [1].bank = (*(adr+39))->value;
  t_wrBuff [1].addr = (*(adr+38))->value;
  t_wrBuff [1].data [0] = (*(adr+37))->value;
  t_wrBuff [1].data [1] = (*(adr+36))->value;
  t_wrBuff [1].mask = (*(adr+35))->value;
  t_wrBuff [1].barrier = (*(adr+34))->value;
  t_wrBuff [1].m4 = (*(adr+33))->value;
  t_wrBuff [1].cmd = (*(adr+32))->value;
  t_wrBuff [1].reRas = (*(adr+31))->value;
  t_wrBuff [2].bank = (*(adr+30))->value;
  t_wrBuff [2].addr = (*(adr+29))->value;
  t_wrBuff [2].data [0] = (*(adr+28))->value;
  t_wrBuff [2].data [1] = (*(adr+27))->value;
  t_wrBuff [2].mask = (*(adr+26))->value;
  t_wrBuff [2].barrier = (*(adr+25))->value;
  t_wrBuff [2].m4 = (*(adr+24))->value;
  t_wrBuff [2].cmd = (*(adr+23))->value;
  t_wrBuff [2].reRas = (*(adr+22))->value;
  t_wrBuff [3].bank = (*(adr+21))->value;
  t_wrBuff [3].addr = (*(adr+20))->value;
  t_wrBuff [3].data [0] = (*(adr+19))->value;
  t_wrBuff [3].data [1] = (*(adr+18))->value;
  t_wrBuff [3].mask = (*(adr+17))->value;
  t_wrBuff [3].barrier = (*(adr+16))->value;
  t_wrBuff [3].m4 = (*(adr+15))->value;
  t_wrBuff [3].cmd = (*(adr+14))->value;
  t_wrBuff [3].reRas = (*(adr+13))->value;
  t_dest.data [0] = (*(adr+5))->value;
  t_dest.data [1] = (*(adr+4))->value;
  t_dest.mask = (*(adr+3))->value;

  ReadFIFO( (*(adr+49))->value, t_wrBuff, (*(adr+12))->value, (*(adr+11))->value, (*(adr+10))->value, (*(adr+9))->value, &new_z[0], &new_z[1], &new_z[2], &t_dest, &new_z[3], &new_z[4], &new_z[5] );
  z = (*(adr+8));
  assert_output( z, new_z[0] );
  z = (*(adr+7));
  assert_output( z, new_z[1] );
  z = (*(adr+6));
  assert_output( z, new_z[2] );
  z = (*(adr+5));
  assert_output( z, t_dest.data [0] );
  z = (*(adr+4));
  assert_output( z, t_dest.data [1] );
  z = (*(adr+3));
  assert_output( z, t_dest.mask );
  z = (*(adr+2));
  assert_output( z, new_z[3] );
  z = (*(adr+1));
  assert_output( z, new_z[4] );
  z = (*(adr+0));
  assert_output( z, new_z[5] );
}
