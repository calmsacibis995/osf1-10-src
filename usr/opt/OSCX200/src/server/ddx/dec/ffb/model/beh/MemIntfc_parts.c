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
static char *rcsid = "@(#)$RCSfile: MemIntfc_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:30:36 $";
#endif
memintfc_code( adr )
     Net_Entry **adr;
{
  extern void MemIntfc();
  VisualType t_visual;
  RotateType t_rotate;
  MemRequestType t_memReq;
  MemStatusType t_memStat;
  int new_z [3];
  register Net_Entry *z;

  t_visual.src = (*(adr+38))->value;
  t_visual.dst = (*(adr+37))->value;
  t_rotate.src = (*(adr+36))->value;
  t_rotate.dst = (*(adr+35))->value;
  t_memReq.addr = (*(adr+34))->value;
  t_memReq.data [0] = (*(adr+33))->value;
  t_memReq.data [1] = (*(adr+32))->value;
  t_memReq.mask = (*(adr+31))->value;
  t_memReq.zWrite = (*(adr+30))->value;
  t_memReq.sWrite = (*(adr+29))->value;
  t_memReq.zTest = (*(adr+28))->value;
  t_memReq.cmd.readFlag = (*(adr+27))->value;
  t_memReq.cmd.selectZ = (*(adr+26))->value;
  t_memReq.cmd.readZ = (*(adr+25))->value;
  t_memReq.cmd.planeMask = (*(adr+24))->value;
  t_memReq.cmd.color = (*(adr+23))->value;
  t_memReq.cmd.block = (*(adr+22))->value;
  t_memReq.cmd.fastFill = (*(adr+21))->value;
  t_memReq.cmd.packed8bit = (*(adr+20))->value;
  t_memReq.cmd.unpacked8bit = (*(adr+19))->value;
  t_memReq.cmd.line = (*(adr+18))->value;
  t_memStat.dest.data [0] = (*(adr+10))->value;
  t_memStat.dest.data [1] = (*(adr+9))->value;
  t_memStat.dest.mask = (*(adr+8))->value;
  t_memStat.dataReady = (*(adr+7))->value;
  t_memStat.busy = (*(adr+6))->value;
  t_memStat.idle = (*(adr+5))->value;
  t_memStat.stencil = (*(adr+4))->value;
  t_memStat.stencilReady = (*(adr+3))->value;

  MemIntfc( (*(adr+39))->value, t_visual, t_rotate, t_memReq, (*(adr+17))->value, (*(adr+16))->value, (*(adr+15))->value, (*(adr+14))->value, (*(adr+13))->value, (*(adr+12))->value, (*(adr+11))->value, &t_memStat, &new_z[0], &new_z[1], &new_z[2] );
  z = (*(adr+10));
  assert_output( z, t_memStat.dest.data [0] );
  z = (*(adr+9));
  assert_output( z, t_memStat.dest.data [1] );
  z = (*(adr+8));
  assert_output( z, t_memStat.dest.mask );
  z = (*(adr+7));
  assert_output( z, t_memStat.dataReady );
  z = (*(adr+6));
  assert_output( z, t_memStat.busy );
  z = (*(adr+5));
  assert_output( z, t_memStat.idle );
  z = (*(adr+4));
  assert_output( z, t_memStat.stencil );
  z = (*(adr+3));
  assert_output( z, t_memStat.stencilReady );
  z = (*(adr+2));
  assert_output( z, new_z[0] );
  z = (*(adr+1));
  assert_output( z, new_z[1] );
  z = (*(adr+0));
  assert_output( z, new_z[2] );
}
