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
static char *rcsid = "@(#)$RCSfile: MemReqCvt_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:30:48 $";
#endif
memreqcvt_code( adr )
     Net_Entry **adr;
{
  extern void MemReqCvt();
  FromPixelGenType t_lastReq;
  RotateType t_rotate;
  MemRequestType t_memReq;
  RowColType t_addr_L;
  B8x8 t_data;
  int new_z [4];
  register Net_Entry *z;

  t_lastReq._addr.row.bank = (*(adr+43))->value;
  t_lastReq._addr.row.addr = (*(adr+42))->value;
  t_lastReq._addr.col.bank = (*(adr+41))->value;
  t_lastReq._addr.col.addr = (*(adr+40))->value;
  t_lastReq.cmd = (*(adr+39))->value;
  t_lastReq.m4 = (*(adr+38))->value;
  t_lastReq.barrier = (*(adr+37))->value;
  t_rotate.src = (*(adr+35))->value;
  t_rotate.dst = (*(adr+34))->value;
  t_memReq.addr = (*(adr+32))->value;
  t_memReq.data [0] = (*(adr+31))->value;
  t_memReq.data [1] = (*(adr+30))->value;
  t_memReq.mask = (*(adr+29))->value;
  t_memReq.zWrite = (*(adr+28))->value;
  t_memReq.sWrite = (*(adr+27))->value;
  t_memReq.zTest = (*(adr+26))->value;
  t_memReq.cmd.readFlag = (*(adr+25))->value;
  t_memReq.cmd.selectZ = (*(adr+24))->value;
  t_memReq.cmd.readZ = (*(adr+23))->value;
  t_memReq.cmd.planeMask = (*(adr+22))->value;
  t_memReq.cmd.color = (*(adr+21))->value;
  t_memReq.cmd.block = (*(adr+20))->value;
  t_memReq.cmd.fastFill = (*(adr+19))->value;
  t_memReq.cmd.packed8bit = (*(adr+18))->value;
  t_memReq.cmd.unpacked8bit = (*(adr+17))->value;
  t_memReq.cmd.line = (*(adr+16))->value;
  t_addr_L.row.bank = (*(adr+15))->value;
  t_addr_L.row.addr = (*(adr+14))->value;
  t_addr_L.col.bank = (*(adr+13))->value;
  t_addr_L.col.addr = (*(adr+12))->value;
  t_data [0] = (*(adr+7))->value;
  t_data [1] = (*(adr+6))->value;
  t_data [2] = (*(adr+5))->value;
  t_data [3] = (*(adr+4))->value;
  t_data [4] = (*(adr+3))->value;
  t_data [5] = (*(adr+2))->value;
  t_data [6] = (*(adr+1))->value;
  t_data [7] = (*(adr+0))->value;

  MemReqCvt( t_lastReq, (*(adr+36))->value, t_rotate, (*(adr+33))->value, t_memReq, &t_addr_L, &new_z[0], &new_z[1], &new_z[2], &new_z[3], t_data );
  z = (*(adr+15));
  assert_output( z, t_addr_L.row.bank );
  z = (*(adr+14));
  assert_output( z, t_addr_L.row.addr );
  z = (*(adr+13));
  assert_output( z, t_addr_L.col.bank );
  z = (*(adr+12));
  assert_output( z, t_addr_L.col.addr );
  z = (*(adr+11));
  assert_output( z, new_z[0] );
  z = (*(adr+10));
  assert_output( z, new_z[1] );
  z = (*(adr+9));
  assert_output( z, new_z[2] );
  z = (*(adr+8));
  assert_output( z, new_z[3] );
  z = (*(adr+7));
  assert_output( z, t_data [0] );
  z = (*(adr+6));
  assert_output( z, t_data [1] );
  z = (*(adr+5));
  assert_output( z, t_data [2] );
  z = (*(adr+4));
  assert_output( z, t_data [3] );
  z = (*(adr+3));
  assert_output( z, t_data [4] );
  z = (*(adr+2));
  assert_output( z, t_data [5] );
  z = (*(adr+1));
  assert_output( z, t_data [6] );
  z = (*(adr+0));
  assert_output( z, t_data [7] );
}
