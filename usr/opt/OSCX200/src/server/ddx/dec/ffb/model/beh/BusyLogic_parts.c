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
static char *rcsid = "@(#)$RCSfile: BusyLogic_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:24:55 $";
#endif
busylogic_code( adr )
     Net_Entry **adr;
{
  extern void BusyLogic();
  ModeType t_mode;
  CmdType t_cmd;
  DmaStatusType t_dmaStatus;
  int new_z [2];
  register Net_Entry *z;

  t_mode.z16 = (*(adr+24))->value;
  t_mode.rotate.src = (*(adr+23))->value;
  t_mode.rotate.dst = (*(adr+22))->value;
  t_mode.visual.src = (*(adr+21))->value;
  t_mode.visual.dst = (*(adr+20))->value;
  t_mode.mode = (*(adr+19))->value;
  t_mode.simple = (*(adr+18))->value;
  t_mode.stipple = (*(adr+17))->value;
  t_mode.line = (*(adr+16))->value;
  t_mode.copy = (*(adr+15))->value;
  t_mode.dmaRd = (*(adr+14))->value;
  t_mode.dmaWr = (*(adr+13))->value;
  t_mode.z = (*(adr+12))->value;
  t_cmd.readFlag0 = (*(adr+11))->value;
  t_cmd.newAddr = (*(adr+10))->value;
  t_cmd.newError = (*(adr+9))->value;
  t_cmd.copy64 = (*(adr+8))->value;
  t_cmd.color = (*(adr+7))->value;
  t_cmd.planeMask = (*(adr+6))->value;
  t_dmaStatus.first = (*(adr+5))->value;
  t_dmaStatus.second = (*(adr+4))->value;
  t_dmaStatus.last = (*(adr+3))->value;

  BusyLogic( (*(adr+35))->value, (*(adr+34))->value, (*(adr+33))->value, (*(adr+32))->value, (*(adr+31))->value, (*(adr+30))->value, (*(adr+29))->value, (*(adr+28))->value, (*(adr+27))->value, (*(adr+26))->value, (*(adr+25))->value, t_mode, t_cmd, t_dmaStatus, (*(adr+2))->value, &new_z[0], &new_z[1] );
  z = (*(adr+1));
  assert_output( z, new_z[0] );
  z = (*(adr+0));
  assert_output( z, new_z[1] );
}
