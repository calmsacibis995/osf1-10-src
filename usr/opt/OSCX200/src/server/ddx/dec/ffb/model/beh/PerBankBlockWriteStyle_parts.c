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
static char *rcsid = "@(#)$RCSfile: PerBankBlockWriteStyle_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:31:58 $";
#endif
perbankblockwritestyle_code( adr )
     Net_Entry **adr;
{
  extern void PerBankBlockWriteStyle();
  BlkStyleType t_blkCtl;
  register Net_Entry *z;

  t_blkCtl.maskLowNibble = (*(adr+1))->value;
  t_blkCtl.maskHighNibble = (*(adr+0))->value;

  PerBankBlockWriteStyle( (*(adr+3))->value, (*(adr+2))->value, &t_blkCtl );
  z = (*(adr+1));
  assert_output( z, t_blkCtl.maskLowNibble );
  z = (*(adr+0));
  assert_output( z, t_blkCtl.maskHighNibble );
}
