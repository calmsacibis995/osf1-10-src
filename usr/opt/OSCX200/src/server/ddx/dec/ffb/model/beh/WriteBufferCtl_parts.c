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
static char *rcsid = "@(#)$RCSfile: WriteBufferCtl_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:36:57 $";
#endif
writebufferctl_code( adr )
     Net_Entry **adr;
{
  extern void WriteBufferCtl();
  Bits4x2 t_ldByte;
  int new_z [4];
  register Net_Entry *z;

  t_ldByte [0] = (*(adr+4))->value;
  t_ldByte [1] = (*(adr+3))->value;
  t_ldByte [2] = (*(adr+2))->value;
  t_ldByte [3] = (*(adr+1))->value;

  WriteBufferCtl( (*(adr+16))->value, (*(adr+15))->value, (*(adr+14))->value, (*(adr+13))->value, (*(adr+12))->value, (*(adr+11))->value, (*(adr+10))->value, (*(adr+9))->value, (*(adr+8))->value, &new_z[0], &new_z[1], &new_z[2], t_ldByte, &new_z[3] );
  z = (*(adr+7));
  assert_output( z, new_z[0] );
  z = (*(adr+6));
  assert_output( z, new_z[1] );
  z = (*(adr+5));
  assert_output( z, new_z[2] );
  z = (*(adr+4));
  assert_output( z, t_ldByte [0] );
  z = (*(adr+3));
  assert_output( z, t_ldByte [1] );
  z = (*(adr+2));
  assert_output( z, t_ldByte [2] );
  z = (*(adr+1));
  assert_output( z, t_ldByte [3] );
  z = (*(adr+0));
  assert_output( z, new_z[3] );
}
