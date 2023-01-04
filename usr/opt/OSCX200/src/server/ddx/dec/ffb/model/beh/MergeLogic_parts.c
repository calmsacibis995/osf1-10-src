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
static char *rcsid = "@(#)$RCSfile: MergeLogic_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:31:23 $";
#endif
mergelogic_code( adr )
     Net_Entry **adr;
{
  extern void MergeLogic();
  FromPixelGenType t_curReq;
  FromPixelGenType t_lastReq;
  int new_z [6];
  register Net_Entry *z;

  t_curReq._addr.row.bank = (*(adr+21))->value;
  t_curReq._addr.row.addr = (*(adr+20))->value;
  t_curReq._addr.col.bank = (*(adr+19))->value;
  t_curReq._addr.col.addr = (*(adr+18))->value;
  t_curReq.cmd = (*(adr+17))->value;
  t_curReq.m4 = (*(adr+16))->value;
  t_curReq.barrier = (*(adr+15))->value;
  t_lastReq._addr.row.bank = (*(adr+13))->value;
  t_lastReq._addr.row.addr = (*(adr+12))->value;
  t_lastReq._addr.col.bank = (*(adr+11))->value;
  t_lastReq._addr.col.addr = (*(adr+10))->value;
  t_lastReq.cmd = (*(adr+9))->value;
  t_lastReq.m4 = (*(adr+8))->value;
  t_lastReq.barrier = (*(adr+7))->value;

  MergeLogic( (*(adr+25))->value, (*(adr+24))->value, (*(adr+23))->value, (*(adr+22))->value, t_curReq, (*(adr+14))->value, t_lastReq, (*(adr+6))->value, &new_z[0], &new_z[1], &new_z[2], &new_z[3], &new_z[4], &new_z[5] );
  z = (*(adr+5));
  assert_output( z, new_z[0] );
  z = (*(adr+4));
  assert_output( z, new_z[1] );
  z = (*(adr+3));
  assert_output( z, new_z[2] );
  z = (*(adr+2));
  assert_output( z, new_z[3] );
  z = (*(adr+1));
  assert_output( z, new_z[4] );
  z = (*(adr+0));
  assert_output( z, new_z[5] );
}
