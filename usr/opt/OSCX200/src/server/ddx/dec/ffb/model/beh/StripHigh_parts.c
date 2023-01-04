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
static char *rcsid = "@(#)$RCSfile: StripHigh_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:36:00 $";
#endif
striphigh_code( adr )
     Net_Entry **adr;
{
  extern void StripHigh();
  RGB t_TwentyFourBit;
  color24 t_EightBit;
  color27 t_NineBit;
  register Net_Entry *z;

  t_TwentyFourBit.red = (*(adr+8))->value;
  t_TwentyFourBit.green = (*(adr+7))->value;
  t_TwentyFourBit.blue = (*(adr+6))->value;
  t_EightBit.red8 = (*(adr+5))->value;
  t_EightBit.green8 = (*(adr+4))->value;
  t_EightBit.blue8 = (*(adr+3))->value;
  t_NineBit.red9 = (*(adr+2))->value;
  t_NineBit.green9 = (*(adr+1))->value;
  t_NineBit.blue9 = (*(adr+0))->value;

  StripHigh( t_TwentyFourBit, &t_EightBit, &t_NineBit );
  z = (*(adr+5));
  assert_output( z, t_EightBit.red8 );
  z = (*(adr+4));
  assert_output( z, t_EightBit.green8 );
  z = (*(adr+3));
  assert_output( z, t_EightBit.blue8 );
  z = (*(adr+2));
  assert_output( z, t_NineBit.red9 );
  z = (*(adr+1));
  assert_output( z, t_NineBit.green9 );
  z = (*(adr+0));
  assert_output( z, t_NineBit.blue9 );
}
