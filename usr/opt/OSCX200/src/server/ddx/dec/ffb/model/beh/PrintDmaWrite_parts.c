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
static char *rcsid = "@(#)$RCSfile: PrintDmaWrite_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:32:41 $";
#endif
printdmawrite_code( adr )
     Net_Entry **adr;
{
  extern void PrintDmaWrite();
  B2x32 t_dmaData;
  int new_z [1];
  register Net_Entry *z;

  t_dmaData [0] = (*(adr+8))->value;
  t_dmaData [1] = (*(adr+7))->value;

  PrintDmaWrite( t_dmaData, (*(adr+6))->value, (*(adr+5))->value, (*(adr+4))->value, (*(adr+3))->value, (*(adr+2))->value, (*(adr+1))->value, &new_z[0] );
  z = (*(adr+0));
  assert_output( z, new_z[0] );
}
