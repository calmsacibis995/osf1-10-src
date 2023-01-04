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
static char *rcsid = "@(#)$RCSfile: BuildOpMask_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:24:44 $";
#endif
buildopmask_code( adr )
     Net_Entry **adr;
{
  extern void BuildOpMask();
  CtlBuildOpMaskType t_ctl;
  int new_z [2];
  register Net_Entry *z;

  t_ctl.blockMode = (*(adr+4))->value;
  t_ctl.visual32 = (*(adr+3))->value;
  t_ctl.unaligned = (*(adr+2))->value;

  BuildOpMask( (*(adr+5))->value, t_ctl, &new_z[0], &new_z[1] );
  z = (*(adr+1));
  assert_output( z, new_z[0] );
  z = (*(adr+0));
  assert_output( z, new_z[1] );
}
