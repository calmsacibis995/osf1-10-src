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
static char *rcsid = "@(#)$RCSfile: PixelGenControlNoLoop_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:32:30 $";
#endif
pixelgencontrolnoloop_code( adr )
     Net_Entry **adr;
{
  extern void PixelGenControlNoLoop();
  ModeType t_mode;
  CmdType t_cmd;
  CtlAddrGen2Type t_ctl;
  int new_z [4];
  register Net_Entry *z;

  t_mode.z16 = (*(adr+28))->value;
  t_mode.rotate.src = (*(adr+27))->value;
  t_mode.rotate.dst = (*(adr+26))->value;
  t_mode.visual.src = (*(adr+25))->value;
  t_mode.visual.dst = (*(adr+24))->value;
  t_mode.mode = (*(adr+23))->value;
  t_mode.simple = (*(adr+22))->value;
  t_mode.stipple = (*(adr+21))->value;
  t_mode.line = (*(adr+20))->value;
  t_mode.copy = (*(adr+19))->value;
  t_mode.dmaRd = (*(adr+18))->value;
  t_mode.dmaWr = (*(adr+17))->value;
  t_mode.z = (*(adr+16))->value;
  t_cmd.readFlag0 = (*(adr+14))->value;
  t_cmd.newAddr = (*(adr+13))->value;
  t_cmd.newError = (*(adr+12))->value;
  t_cmd.copy64 = (*(adr+11))->value;
  t_cmd.color = (*(adr+10))->value;
  t_cmd.planeMask = (*(adr+9))->value;
  t_ctl.selData = (*(adr+7))->value;
  t_ctl.buildOpMask.blockMode = (*(adr+6))->value;
  t_ctl.buildOpMask.visual32 = (*(adr+5))->value;
  t_ctl.buildOpMask.unaligned = (*(adr+4))->value;

  PixelGenControlNoLoop( t_mode, (*(adr+15))->value, t_cmd, (*(adr+8))->value, &t_ctl, &new_z[0], &new_z[1], &new_z[2], &new_z[3] );
  z = (*(adr+7));
  assert_output( z, t_ctl.selData );
  z = (*(adr+6));
  assert_output( z, t_ctl.buildOpMask.blockMode );
  z = (*(adr+5));
  assert_output( z, t_ctl.buildOpMask.visual32 );
  z = (*(adr+4));
  assert_output( z, t_ctl.buildOpMask.unaligned );
  z = (*(adr+3));
  assert_output( z, new_z[0] );
  z = (*(adr+2));
  assert_output( z, new_z[1] );
  z = (*(adr+1));
  assert_output( z, new_z[2] );
  z = (*(adr+0));
  assert_output( z, new_z[3] );
}
