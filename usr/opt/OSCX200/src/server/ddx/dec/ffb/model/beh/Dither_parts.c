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
static char *rcsid = "@(#)$RCSfile: Dither_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:25:42 $";
#endif
dither_code( adr )
     Net_Entry **adr;
{
  extern void Dither();
  color19 t_matrixIn;
  color27 t_ditherIn;
  color24 t_colorDither;
  register Net_Entry *z;

  t_matrixIn.red6 = (*(adr+9))->value;
  t_matrixIn.green6 = (*(adr+8))->value;
  t_matrixIn.blue7 = (*(adr+7))->value;
  t_ditherIn.red9 = (*(adr+6))->value;
  t_ditherIn.green9 = (*(adr+5))->value;
  t_ditherIn.blue9 = (*(adr+4))->value;
  t_colorDither.red8 = (*(adr+3))->value;
  t_colorDither.green8 = (*(adr+2))->value;
  t_colorDither.blue8 = (*(adr+1))->value;

  Dither( t_matrixIn, t_ditherIn, &t_colorDither, (*(adr+0))->value );
  z = (*(adr+3));
  assert_output( z, t_colorDither.red8 );
  z = (*(adr+2));
  assert_output( z, t_colorDither.green8 );
  z = (*(adr+1));
  assert_output( z, t_colorDither.blue8 );
}
