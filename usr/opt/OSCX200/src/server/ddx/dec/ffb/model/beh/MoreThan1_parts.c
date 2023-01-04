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
static char *rcsid = "@(#)$RCSfile: MoreThan1_parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:31:46 $";
#endif
morethan1_code( adr )
     Net_Entry **adr;
{
  extern void MoreThan1();
  int new_z [1];
  register Net_Entry *z;


  MoreThan1( (*(adr+1))->value, &new_z[0] );
  z = (*(adr+0));
  assert_output( z, new_z[0] );
}
