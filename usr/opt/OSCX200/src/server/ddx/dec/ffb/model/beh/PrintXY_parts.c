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
static char *rcsid = "@(#)$RCSfile: PrintXY_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:34:13 $";
#endif
printxy_code( adr )
     Net_Entry **adr;
{
  extern void PrintXY();
  color24 t_drom;
  color24 t_dithered;

  t_drom.red8 = (*(adr+5))->value;
  t_drom.green8 = (*(adr+4))->value;
  t_drom.blue8 = (*(adr+3))->value;
  t_dithered.red8 = (*(adr+2))->value;
  t_dithered.green8 = (*(adr+1))->value;
  t_dithered.blue8 = (*(adr+0))->value;

  PrintXY( (*(adr+8))->value, (*(adr+7))->value, (*(adr+6))->value, t_drom, t_dithered );
}
