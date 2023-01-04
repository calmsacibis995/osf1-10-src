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
static char *rcsid = "@(#)$RCSfile: parts.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:42:04 $";
#endif
#include <stdio.h>
#include "parts_c.h"

v2s_1_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;

  new_z = adr[1]->value;

  uncond_assert_output (z, new_z);
}

v2s_2_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=2; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_3_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=3; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_4_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=4; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_5_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=5; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_6_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=6; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_7_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=7; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_8_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=8; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_9_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=9; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_10_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=10; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_11_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=11; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_12_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=12; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_13_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=13; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_14_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=14; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_15_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=15; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_16_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=16; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_17_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=17; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_18_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=18; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_19_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=19; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_20_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=20; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_21_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=21; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_22_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=22; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_23_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=23; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_24_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=24; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_25_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=25; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_26_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=26; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_27_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=27; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_28_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=28; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_29_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=29; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_30_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=30; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_31_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=31; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

v2s_32_code(adr)
Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=32; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

s2v_1_code(adr)
Net_Entry **adr;
{
  register int val;
  register Net_Entry *z;

  val = (adr[0]->value ? -1 : 0);
  z = adr[1];
  uncond_assert_output( z, val );
}

s2v_2_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=2; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_3_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=3; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_4_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=4; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_5_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=5; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_6_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=6; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_7_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=7; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_8_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=8; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_9_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=9; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_10_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=10; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_11_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=11; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_12_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=12; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_13_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=13; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_14_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=14; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_15_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=15; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_16_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=16; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_17_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=17; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_18_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=18; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_19_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=19; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_20_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=20; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_21_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=21; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_22_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=22; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_23_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=23; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_24_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=24; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_25_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=25; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_26_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=26; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_27_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=27; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_28_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=28; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_29_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=29; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_30_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=30; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_31_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=31; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

s2v_32_code(adr)
Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=32; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

v2s_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *z = adr[0];
  register int new_z;
  register int i;

  new_z = 0;
  for (i=1; i<=32; ++i)
    new_z = (new_z << 1) | (1&(adr[i]->value));

  uncond_assert_output (z, new_z);
}

/*
 * rsm, 6-July-1992
 *     value to put in vector changed to -1 or 0.
 */
s2v_code(adr)
     Net_Entry **adr;
{
  register int vector = adr[0]->value;
  register Net_Entry *z;
  register int i, val;

  for (i=32; i>=1; --i) {
    val = (vector & 1 ? -1 : 0);
    vector >>= 1;
    z = adr[i];
    assert_output( z, val );
  }
}

ad4ful_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u2 = adr[0];
  register Net_Entry *u1 = adr[1];
  register Net_Entry *u0 = adr[2];
  register Net_Entry *S0 = adr[3];
  register int new_S0;
  register Net_Entry *S1 = adr[4];
  register int new_S1;
  register Net_Entry *S2 = adr[5];
  register int new_S2;
  register Net_Entry *S3 = adr[6];
  register int new_S3;
  register Net_Entry *CO = adr[7];
  register int new_CO;
  register int A0 = adr[8]->value;
  register int A1 = adr[9]->value;
  register int A2 = adr[10]->value;
  register int A3 = adr[11]->value;
  register int B0 = adr[12]->value;
  register int B1 = adr[13]->value;
  register int B2 = adr[14]->value;
  register int B3 = adr[15]->value;
  register int CI = adr[16]->value;
  
#ifdef DEBUG
  printf("enter ad4ful_code\n");
#endif
  new_S0 = A0 & ~B0 & ~CI | ~A0 & ~B0 & CI | ~A0 & B0 & ~CI | A0 & B0 & CI;
  assert_output (S0, new_S0);
  u0->value = ((A0 & B0) | ((A0 | B0) & CI));
  new_S1 = A1 & ~B1 & ~u0->value | ~A1 & ~B1 & u0->value | ~A1 & B1 & ~u0->value | A1 & B1 & u0->value;
  assert_output (S1, new_S1);
  u1->value = ((A1 & B1) | ((A1 | B1) & u0->value));
  new_S2 = A2 & ~B2 & ~u1->value | ~A2 & ~B2 & u1->value | ~A2 & B2 & ~u1->value | A2 & B2 & u1->value;
  assert_output (S2, new_S2);
  u2->value = ((A2 & B2) | ((A2 | B2) & u1->value));
  new_S3 = A3 & ~B3 & ~u2->value | ~A3 & ~B3 & u2->value | ~A3 & B3 & ~u2->value | A3 & B3 & u2->value;
  assert_output (S3, new_S3);
  new_CO = ((A3 & B3) | ((A3 | B3) & u2->value));
  assert_output (CO, new_CO);
}

ad4fula_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u2 = adr[0];
  register Net_Entry *u1 = adr[1];
  register Net_Entry *u0 = adr[2];
  register Net_Entry *S0 = adr[3];
  register int new_S0;
  register Net_Entry *S1 = adr[4];
  register int new_S1;
  register Net_Entry *S2 = adr[5];
  register int new_S2;
  register Net_Entry *S3 = adr[6];
  register int new_S3;
  register Net_Entry *CO = adr[7];
  register int new_CO;
  register int A0 = adr[8]->value;
  register int A1 = adr[9]->value;
  register int A2 = adr[10]->value;
  register int A3 = adr[11]->value;
  register int B0 = adr[12]->value;
  register int B1 = adr[13]->value;
  register int B2 = adr[14]->value;
  register int B3 = adr[15]->value;
  register int CI = adr[16]->value;
  
#ifdef DEBUG
  printf("enter ad4fula_code\n");
#endif
  new_S0 = A0 & ~B0 & ~CI | ~A0 & ~B0 & CI | ~A0 & B0 & ~CI | A0 & B0 & CI;
  assert_output (S0, new_S0);
  u0->value = ((A0 & B0) | ((A0 | B0) & CI));
  new_S1 = A1 & ~B1 & ~u0->value | ~A1 & ~B1 & u0->value | ~A1 & B1 & ~u0->value | A1 & B1 & u0->value;
  assert_output (S1, new_S1);
  u1->value = ((A1 & B1) | ((A1 | B1) & u0->value));
  new_S2 = A2 & ~B2 & ~u1->value | ~A2 & ~B2 & u1->value | ~A2 & B2 & ~u1->value | A2 & B2 & u1->value;
  assert_output (S2, new_S2);
  u2->value = ((A2 & B2) | ((A2 | B2) & u1->value));
  new_S3 = A3 & ~B3 & ~u2->value | ~A3 & ~B3 & u2->value | ~A3 & B3 & ~u2->value | A3 & B3 & u2->value;
  assert_output (S3, new_S3);
  new_CO = ((A3 & B3) | ((A3 | B3) & u2->value));
  assert_output (CO, new_CO);
}

ad4pg_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *x3 = adr[0];
  register Net_Entry *x2 = adr[1];
  register Net_Entry *x1 = adr[2];
  register Net_Entry *x0 = adr[3];
  register Net_Entry *o3 = adr[4];
  register Net_Entry *o2 = adr[5];
  register Net_Entry *o1 = adr[6];
  register Net_Entry *o0 = adr[7];
  register Net_Entry *u2 = adr[8];
  register Net_Entry *u1 = adr[9];
  register Net_Entry *u0 = adr[10];
  register Net_Entry *S0 = adr[11];
  register int new_S0;
  register Net_Entry *S1 = adr[12];
  register int new_S1;
  register Net_Entry *S2 = adr[13];
  register int new_S2;
  register Net_Entry *S3 = adr[14];
  register int new_S3;
  register Net_Entry *PR = adr[15];
  register int new_PR;
  register Net_Entry *GE = adr[16];
  register int new_GE;
  register int A0 = adr[17]->value;
  register int A1 = adr[18]->value;
  register int A2 = adr[19]->value;
  register int A3 = adr[20]->value;
  register int B0 = adr[21]->value;
  register int B1 = adr[22]->value;
  register int B2 = adr[23]->value;
  register int B3 = adr[24]->value;
  register int CI = adr[25]->value;
  
#ifdef DEBUG
  printf("enter ad4pg_code\n");
#endif
  new_S0 = A0 & ~B0 & ~CI | ~A0 & ~B0 & CI | ~A0 & B0 & ~CI | A0 & B0 & CI;
  assert_output (S0, new_S0);
  u0->value = ((A0 & B0) | ((A0 | B0) & CI));
  new_S1 = A1 & ~B1 & ~u0->value | ~A1 & ~B1 & u0->value | ~A1 & B1 & ~u0->value | A1 & B1 & u0->value;
  assert_output (S1, new_S1);
  u1->value = ((A1 & B1) | ((A1 | B1) & u0->value));
  new_S2 = A2 & ~B2 & ~u1->value | ~A2 & ~B2 & u1->value | ~A2 & B2 & ~u1->value | A2 & B2 & u1->value;
  assert_output (S2, new_S2);
  u2->value = ((A2 & B2) | ((A2 | B2) & u1->value));
  new_S3 = A3 & ~B3 & ~u2->value | ~A3 & ~B3 & u2->value | ~A3 & B3 & ~u2->value | A3 & B3 & u2->value;
  assert_output (S3, new_S3);
  o0->value = A0 | B0;
  o1->value = A1 | B1;
  o2->value = A2 | B2;
  o3->value = A3 | B3;
  new_PR = (o0->value & o1->value & o2->value & o3->value);
  assert_output (PR, new_PR);
  x0->value = A0 & B0;
  x1->value = A1 & B1;
  x2->value = A2 & B2;
  x3->value = A3 & B3;
  new_GE = (x0->value & o1->value & o2->value & o3->value | x1->value & o2->value & o3->value | x2->value & o3->value | x3->value);
  assert_output (GE, new_GE);
}

add5_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u = adr[0];
  register Net_Entry *S0 = adr[1];
  register int new_S0;
  register Net_Entry *S1A = adr[2];
  register int new_S1A;
  register Net_Entry *S1B = adr[3];
  register int new_S1B;
  register int A = adr[4]->value;
  register int B = adr[5]->value;
  register int C = adr[6]->value;
  register int D = adr[7]->value;
  register int E = adr[8]->value;
  
#ifdef DEBUG
  printf("enter add5_code\n");
#endif
  u->value = C & ~D & ~E | ~C & ~D & E | ~C & D & ~E | C & D & E;
  new_S0 = A & ~B & ~u->value | ~A & ~B & u->value | ~A & B & ~u->value | A & B & u->value;
  assert_output (S0, new_S0);
  new_S1B = ((C & D) | ((C | D) & E));
  assert_output (S1B, new_S1B);
  new_S1A = ((A & B) | ((A | B) & u->value));
  assert_output (S1A, new_S1A);
}

add5a_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u = adr[0];
  register Net_Entry *S0 = adr[1];
  register int new_S0;
  register Net_Entry *S1A = adr[2];
  register int new_S1A;
  register Net_Entry *S1B = adr[3];
  register int new_S1B;
  register int A = adr[4]->value;
  register int B = adr[5]->value;
  register int C = adr[6]->value;
  register int D = adr[7]->value;
  register int E = adr[8]->value;
  
#ifdef DEBUG
  printf("enter add5a_code\n");
#endif
  u->value = C & ~D & ~E | ~C & ~D & E | ~C & D & ~E | C & D & E;
  new_S0 = A & ~B & ~u->value | ~A & ~B & u->value | ~A & B & ~u->value | A & B & u->value;
  assert_output (S0, new_S0);
  new_S1B = ((C & D) | ((C | D) & E));
  assert_output (S1B, new_S1B);
  new_S1A = ((A & B) | ((A | B) & u->value));
  assert_output (S1A, new_S1A);
}

adful_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *CO = adr[0];
  register int new_CO;
  register Net_Entry *S = adr[1];
  register int new_S;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int CI = adr[4]->value;
  
#ifdef DEBUG
  printf("enter adful_code\n");
#endif
  new_CO = ((A & B) | ((A | B) & CI));
  assert_output (CO, new_CO);
  new_S = A & ~B & ~CI | ~A & ~B & CI | ~A & B & ~CI | A & B & CI;
  assert_output (S, new_S);
}

adfulh_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *CO = adr[0];
  register int new_CO;
  register Net_Entry *S = adr[1];
  register int new_S;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int CI = adr[4]->value;
  
#ifdef DEBUG
  printf("enter adfulh_code\n");
#endif
  new_CO = ((A & B) | ((A | B) & CI));
  assert_output (CO, new_CO);
  new_S = A & ~B & ~CI | ~A & ~B & CI | ~A & B & ~CI | A & B & CI;
  assert_output (S, new_S);
}

adfulha_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *CO = adr[0];
  register int new_CO;
  register Net_Entry *S = adr[1];
  register int new_S;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int CI = adr[4]->value;
  
#ifdef DEBUG
  printf("enter adfulha_code\n");
#endif
  new_CO = ((A & B) | ((A | B) & CI));
  assert_output (CO, new_CO);
  new_S = A & ~B & ~CI | ~A & ~B & CI | ~A & B & ~CI | A & B & CI;
  assert_output (S, new_S);
}

adhalf_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *CO = adr[0];
  register int new_CO;
  register Net_Entry *S = adr[1];
  register int new_S;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  
#ifdef DEBUG
  printf("enter adhalf_code\n");
#endif
  new_CO = (A & B);
  assert_output (CO, new_CO);
  new_S = A & ~B | ~A & B;
  assert_output (S, new_S);
}

adhalfh_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *CO = adr[0];
  register int new_CO;
  register Net_Entry *S = adr[1];
  register int new_S;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  
#ifdef DEBUG
  printf("enter adhalfh_code\n");
#endif
  new_CO = (A & B);
  assert_output (CO, new_CO);
  new_S = A & ~B | ~A & B;
  assert_output (S, new_S);
}

and2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter and2_code\n");
#endif
  new_X = A & B;
  assert_output (X, new_X);
}

and2h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter and2h_code\n");
#endif
  new_X = A & B;
  assert_output (X, new_X);
}

and3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter and3_code\n");
#endif
  new_X = A & B & C;
  assert_output (X, new_X);
}

and3h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter and3h_code\n");
#endif
  new_X = A & B & C;
  assert_output (X, new_X);
}

and4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter and4_code\n");
#endif
  new_X = A & B & C & D;
  assert_output (X, new_X);
}

and4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter and4h_code\n");
#endif
  new_X = A & B & C & D;
  assert_output (X, new_X);
}

and8h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u = adr[0];
  register Net_Entry *X = adr[1];
  register int new_X;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int C = adr[4]->value;
  register int D = adr[5]->value;
  register int E = adr[6]->value;
  register int F = adr[7]->value;
  register int G = adr[8]->value;
  register int H = adr[9]->value;
  
#ifdef DEBUG
  printf("enter and8h_code\n");
#endif
  u->value = A & B & C & D & E;
  new_X = (u->value & F & G & H);
  assert_output (X, new_X);
}

andoi22_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter andoi22_code\n");
#endif
  new_X = ~((A & B) | ~(C | D));
  assert_output (X, new_X);
}

andoi22h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter andoi22h_code\n");
#endif
  new_X = ~((A & B) | ~(C | D));
  assert_output (X, new_X);
}

ao21h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter ao21h_code\n");
#endif
  new_X = ((A & B) | C);
  assert_output (X, new_X);
}

ao22h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter ao22h_code\n");
#endif
  new_X = ((A & B) | (C & D));
  assert_output (X, new_X);
}

ao321h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  register int E = adr[5]->value;
  register int F = adr[6]->value;
  
#ifdef DEBUG
  printf("enter ao321h_code\n");
#endif
  new_X = ((A & B & C) | (D & E) | F);
  assert_output (X, new_X);
}

ao4321h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  register int E = adr[5]->value;
  register int F = adr[6]->value;
  register int G = adr[7]->value;
  register int H = adr[8]->value;
  register int I = adr[9]->value;
  register int J = adr[10]->value;
  
#ifdef DEBUG
  printf("enter ao4321h_code\n");
#endif
  new_X = ((A & B & C & D) | (E & F & G) | (H & I) | J);
  assert_output (X, new_X);
}

ondai22_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter ondai22_code\n");
#endif
  new_X = ~((A | B) & (~( C & D)));
  assert_output (X, new_X);
}

ondai22h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter ondai22h_code\n");
#endif
  new_X = ~((A | B) & (~( C & D)));
  assert_output (X, new_X);
}


aoi21_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter aoi21_code\n");
#endif
  new_X = ~((A & B) | C);
  assert_output (X, new_X);
}

aoi211_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter aoi211_code\n");
#endif
  new_X = ~((A & B) | C | D);
  assert_output (X, new_X);
}

aoi211h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter aoi211h_code\n");
#endif
  new_X = ~((A & B) | C | D);
  assert_output (X, new_X);
}

aoi21h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter aoi21h_code\n");
#endif
  new_X = ~((A & B) | C);
  assert_output (X, new_X);
}

aoi22_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter aoi22_code\n");
#endif
  new_X = ~((A & B) | (C & D));
  assert_output (X, new_X);
}

aoi22h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter aoi22h_code\n");
#endif
  new_X = ~((A & B) | (C & D));
  assert_output (X, new_X);
}

bici_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DI = adr[0];
  register int new_DI;
  register int BC = adr[1]->value;
  
#ifdef DEBUG
  printf("enter bici_code\n");
#endif
  new_DI = ~BC;
  assert_output (DI, new_DI);
}

bicn_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DI = adr[0];
  register int new_DI;
  register int BC = adr[1]->value;
  
#ifdef DEBUG
  printf("enter bicn_code\n");
#endif
  new_DI = BC;
  assert_output (DI, new_DI);
}

bitn_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DI = adr[0];
  register int new_DI;
  register int BC = adr[1]->value;
  
#ifdef DEBUG
  printf("enter bitn_code\n");
#endif
  new_DI = BC;
  assert_output (DI, new_DI);
}

bon2t_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register Net_Entry *BIC = adr[1];
  register int new_BIC;
  register int DO = adr[2]->value;
  register int EN = adr[3]->value;
  
#ifdef DEBUG
  printf("enter bon2t_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
  new_BIC = (EN & DO) | (~EN & PAD->value);
  assert_output (BIC, new_BIC);
}

bon4t_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register Net_Entry *BIC = adr[1];
  register int new_BIC;
  register int DO = adr[2]->value;
  register int EN = adr[3]->value;
  
#ifdef DEBUG
  printf("enter bon4t_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
  new_BIC = (EN & DO) | (~EN & PAD->value);
  assert_output (BIC, new_BIC);
}

bon6t_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register Net_Entry *BIC = adr[1];
  register int new_BIC;
  register int DO = adr[2]->value;
  register int EN = adr[3]->value;
  
#ifdef DEBUG
  printf("enter bon6t_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
  new_BIC = (EN & DO) | (~EN & PAD->value);
  assert_output (BIC, new_BIC);
}

on6t_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  register int EN = adr[2]->value;
  
#ifdef DEBUG
  printf("enter on6t_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
}

bon4ods2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register Net_Entry *BIC = adr[1];
  register int new_BIC;
  register int DO = adr[2]->value;
  
#ifdef DEBUG
  printf("enter bon4ods2_code\n");
#endif
  new_PAD = DO;
  assert_output (PAD, new_PAD);
  new_BIC = DO;
  assert_output (BIC, new_BIC);
}

bon4ts2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register Net_Entry *BIC = adr[1];
  register int new_BIC;
  register int DO = adr[2]->value;
  register int EN = adr[3]->value;
  
#ifdef DEBUG
  printf("enter bon4ts2_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
  new_BIC = (EN & DO) | (~EN & PAD->value);
  assert_output (BIC, new_BIC);
}

bon4ts4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register Net_Entry *BIC = adr[1];
  register int new_BIC;
  register int DO = adr[2]->value;
  register int EN = adr[3]->value;
  
#ifdef DEBUG
  printf("enter bon4ts4_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
  new_BIC = (EN & DO) | (~EN & PAD->value);
  assert_output (BIC, new_BIC);
}

bon8t_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register Net_Entry *BIC = adr[1];
  register int new_BIC;
  register int DO = adr[2]->value;
  register int EN = adr[3]->value;
  
#ifdef DEBUG
  printf("enter bon8t_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
  new_BIC = (EN & DO) | (~EN & PAD->value);
  assert_output (BIC, new_BIC);
}

bon8ts2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register Net_Entry *BIC = adr[1];
  register int new_BIC;
  register int DO = adr[2]->value;
  register int EN = adr[3]->value;
  
#ifdef DEBUG
  printf("enter bon8ts2_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
  new_BIC = (EN & DO) | (~EN & PAD->value);
  assert_output (BIC, new_BIC);
}

bon8ts4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register Net_Entry *BIC = adr[1];
  register int new_BIC;
  register int DO = adr[2]->value;
  register int EN = adr[3]->value;
  
#ifdef DEBUG
  printf("enter bon8ts4_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
  new_BIC = (EN & DO) | (~EN & PAD->value);
  assert_output (BIC, new_BIC);
}

buf_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter buf_code\n");
#endif
  new_X = A;
  uncond_assert_output (X, new_X);
}

bufx_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter bufx_code\n");
#endif
  new_X = A;
  assert_output (X, new_X);
}

buf2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter buf2_code\n");
#endif
  new_X = A;
  uncond_assert_output (X, new_X);
}

buf2c_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register Net_Entry *Y = adr[1];
  register int new_X;
  register int new_Y;
  register int A = adr[2]->value;
  
#ifdef DEBUG
  printf("enter buf2c_code\n");
#endif
  new_X = ~A;
  new_Y = A;
  uncond_assert_output (X, new_X);
  uncond_assert_output (Y, new_Y);
}

buf2b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter buf2b_code\n");
#endif
  new_X = A;
  uncond_assert_output (X, new_X);
}

buf3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter buf3_code\n");
#endif
  new_X = A;
  uncond_assert_output (X, new_X);
}

buf3b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter buf3b_code\n");
#endif
  new_X = A;
  uncond_assert_output (X, new_X);
}

buf4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter buf4_code\n");
#endif
  new_X = A;
  uncond_assert_output (X, new_X);
}

buf4b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter buf4b_code\n");
#endif
  new_X = A;
  uncond_assert_output (X, new_X);
}

buf8_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter buf8_code\n");
#endif
  new_X = A;
  uncond_assert_output (X, new_X);
}

buf8b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter buf8b_code\n");
#endif
  new_X = A;
  uncond_assert_output (X, new_X);
}
#if 0
dcr4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *S0 = adr[0];
  register int new_S0;
  register Net_Entry *S1 = adr[1];
  register int new_S1;
  register Net_Entry *S2 = adr[2];
  register int new_S2;
  register Net_Entry *S3 = adr[3];
  register int new_S3;
  register Net_Entry *CO = adr[4];
  register int new_CO;
  register Net_Entry *PR = adr[5];
  register int new_PR;
  register int A0 = adr[6]->value;
  register int A1 = adr[7]->value;
  register int A2 = adr[8]->value;
  register int A3 = adr[9]->value;
  register int CI = adr[10]->value;
  
#ifdef DEBUG
  printf("enter dcr4_code\n");
#endif
  new_S0 = CI & A0 | ~CI & ~A0;
  assert_output (S0, new_S0);
  new_S1 = (CI | A0) & A1 | ~(CI | A0) & ~A1;
  assert_output (S1, new_S1);
  new_S2 = (CI | A0 | A1) & A2 | ~(CI | A0 | A1) & ~A2;
  assert_output (S2, new_S2);
  new_S3 = (CI | A0 | A1 | A2) & A3 | ~(CI | A0 | A1 | A2) & ~A3;
  assert_output (S3, new_S3);
  new_PR = A0 | A1 | A2 | A3;
  assert_output (PR, new_PR);
  new_CO = CI | PR->value;
  assert_output (CO, new_CO);
}

dcr4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *S0 = adr[0];
  register int new_S0;
  register Net_Entry *S1 = adr[1];
  register int new_S1;
  register Net_Entry *S2 = adr[2];
  register int new_S2;
  register Net_Entry *S3 = adr[3];
  register int new_S3;
  register Net_Entry *CO = adr[4];
  register int new_CO;
  register Net_Entry *PR = adr[5];
  register int new_PR;
  register int A0 = adr[6]->value;
  register int A1 = adr[7]->value;
  register int A2 = adr[8]->value;
  register int A3 = adr[9]->value;
  register int CI = adr[10]->value;
  
#ifdef DEBUG
  printf("enter dcr4h_code\n");
#endif
  new_S0 = CI & A0 | ~CI & ~A0;
  assert_output (S0, new_S0);
  new_S1 = (CI | A0) & A1 | ~(CI | A0) & ~A1;
  assert_output (S1, new_S1);
  new_S2 = (CI | A0 | A1) & A2 | ~(CI | A0 | A1) & ~A2;
  assert_output (S2, new_S2);
  new_S3 = (CI | A0 | A1 | A2) & A3 | ~(CI | A0 | A1 | A2) & ~A3;
  assert_output (S3, new_S3);
  new_PR = A0 | A1 | A2 | A3;
  assert_output (PR, new_PR);
  new_CO = CI | PR->value;
  assert_output (CO, new_CO);
}

dec1of8_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0B = adr[0];
  register int new_X0B;
  register Net_Entry *X1B = adr[1];
  register int new_X1B;
  register Net_Entry *X2B = adr[2];
  register int new_X2B;
  register Net_Entry *X3B = adr[3];
  register int new_X3B;
  register Net_Entry *X4B = adr[4];
  register int new_X4B;
  register Net_Entry *X5B = adr[5];
  register int new_X5B;
  register Net_Entry *X6B = adr[6];
  register int new_X6B;
  register Net_Entry *X7B = adr[7];
  register int new_X7B;
  register int SL0 = adr[8]->value;
  register int SL1 = adr[9]->value;
  register int SL2 = adr[10]->value;
  register int ENB = adr[11]->value;
  
#ifdef DEBUG
  printf("enter dec1of8_code\n");
#endif
  new_X0B = ENB | SL0 | SL1 | SL2;
  assert_output (X0B, new_X0B);
  new_X1B = ENB | ~SL0 | SL1 | SL2;
  assert_output (X1B, new_X1B);
  new_X2B = ENB | SL0 | ~SL1 | SL2;
  assert_output (X2B, new_X2B);
  new_X3B = ENB | ~SL0 | ~SL1 | SL2;
  assert_output (X3B, new_X3B);
  new_X4B = ENB | SL0 | SL1 | ~SL2;
  assert_output (X4B, new_X4B);
  new_X5B = ENB | ~SL0 | SL1 | ~SL2;
  assert_output (X5B, new_X5B);
  new_X6B = ENB | SL0 | ~SL1 | ~SL2;
  assert_output (X6B, new_X6B);
  new_X7B = ENB | ~SL0 | ~SL1 | ~SL2;
  assert_output (X7B, new_X7B);
}

dec4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0B = adr[0];
  register int new_X0B;
  register Net_Entry *X1B = adr[1];
  register int new_X1B;
  register Net_Entry *X2B = adr[2];
  register int new_X2B;
  register Net_Entry *X3B = adr[3];
  register int new_X3B;
  register int SL0 = adr[4]->value;
  register int SL1 = adr[5]->value;
  
#ifdef DEBUG
  printf("enter dec4_code\n");
#endif
  new_X0B = SL0 | SL1;
  assert_output (X0B, new_X0B);
  new_X1B = ~SL0 | SL1;
  assert_output (X1B, new_X1B);
  new_X2B = SL0 | ~SL1;
  assert_output (X2B, new_X2B);
  new_X3B = ~SL0 | ~SL1;
  assert_output (X3B, new_X3B);
}

dec4a_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0 = adr[0];
  register int new_X0;
  register Net_Entry *X1 = adr[1];
  register int new_X1;
  register Net_Entry *X2 = adr[2];
  register int new_X2;
  register Net_Entry *X3 = adr[3];
  register int new_X3;
  register int SL0 = adr[4]->value;
  register int SL1 = adr[5]->value;
  register int EN = adr[6]->value;
  
#ifdef DEBUG
  printf("enter dec4a_code\n");
#endif
  new_X0 = EN & ~SL0 & ~SL1;
  assert_output (X0, new_X0);
  new_X1 = EN & SL0 & ~SL1;
  assert_output (X1, new_X1);
  new_X2 = EN & ~SL0 & SL1;
  assert_output (X2, new_X2);
  new_X3 = EN & SL0 & SL1;
  assert_output (X3, new_X3);
}

dec4ah_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0 = adr[0];
  register int new_X0;
  register Net_Entry *X1 = adr[1];
  register int new_X1;
  register Net_Entry *X2 = adr[2];
  register int new_X2;
  register Net_Entry *X3 = adr[3];
  register int new_X3;
  register int SL0 = adr[4]->value;
  register int SL1 = adr[5]->value;
  register int EN = adr[6]->value;
  
#ifdef DEBUG
  printf("enter dec4ah_code\n");
#endif
  new_X0 = EN & ~SL0 & ~SL1;
  assert_output (X0, new_X0);
  new_X1 = EN & SL0 & ~SL1;
  assert_output (X1, new_X1);
  new_X2 = EN & ~SL0 & SL1;
  assert_output (X2, new_X2);
  new_X3 = EN & SL0 & SL1;
  assert_output (X3, new_X3);
}

dec4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0B = adr[0];
  register int new_X0B;
  register Net_Entry *X1B = adr[1];
  register int new_X1B;
  register Net_Entry *X2B = adr[2];
  register int new_X2B;
  register Net_Entry *X3B = adr[3];
  register int new_X3B;
  register int SL0 = adr[4]->value;
  register int SL1 = adr[5]->value;
  
#ifdef DEBUG
  printf("enter dec4h_code\n");
#endif
  new_X0B = SL0 | SL1;
  assert_output (X0B, new_X0B);
  new_X1B = ~SL0 | SL1;
  assert_output (X1B, new_X1B);
  new_X2B = SL0 | ~SL1;
  assert_output (X2B, new_X2B);
  new_X3B = ~SL0 | ~SL1;
  assert_output (X3B, new_X3B);
}

dec8_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0 = adr[0];
  register int new_X0;
  register Net_Entry *X1 = adr[1];
  register int new_X1;
  register Net_Entry *X2 = adr[2];
  register int new_X2;
  register Net_Entry *X3 = adr[3];
  register int new_X3;
  register Net_Entry *X4 = adr[4];
  register int new_X4;
  register Net_Entry *X5 = adr[5];
  register int new_X5;
  register Net_Entry *X6 = adr[6];
  register int new_X6;
  register Net_Entry *X7 = adr[7];
  register int new_X7;
  register int SL0 = adr[8]->value;
  register int SL1 = adr[9]->value;
  register int SL2 = adr[10]->value;
  
#ifdef DEBUG
  printf("enter dec8_code\n");
#endif
  new_X0 = ~SL0 & ~SL1 & ~SL2;
  assert_output (X0, new_X0);
  new_X1 = SL0 & ~SL1 & ~SL2;
  assert_output (X1, new_X1);
  new_X2 = ~SL0 & SL1 & ~SL2;
  assert_output (X2, new_X2);
  new_X3 = SL0 & SL1 & ~SL2;
  assert_output (X3, new_X3);
  new_X4 = ~SL0 & ~SL1 & SL2;
  assert_output (X4, new_X4);
  new_X5 = SL0 & ~SL1 & SL2;
  assert_output (X5, new_X5);
  new_X6 = ~SL0 & SL1 & SL2;
  assert_output (X6, new_X6);
  new_X7 = SL0 & SL1 & SL2;
  assert_output (X7, new_X7);
}

dec8a_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0 = adr[0];
  register int new_X0;
  register Net_Entry *X1 = adr[1];
  register int new_X1;
  register Net_Entry *X2 = adr[2];
  register int new_X2;
  register Net_Entry *X3 = adr[3];
  register int new_X3;
  register Net_Entry *X4 = adr[4];
  register int new_X4;
  register Net_Entry *X5 = adr[5];
  register int new_X5;
  register Net_Entry *X6 = adr[6];
  register int new_X6;
  register Net_Entry *X7 = adr[7];
  register int new_X7;
  register int SL0 = adr[8]->value;
  register int SL1 = adr[9]->value;
  register int SL2 = adr[10]->value;
  register int EN = adr[11]->value;
  
#ifdef DEBUG
  printf("enter dec8a_code\n");
#endif
  new_X0 = EN & ~SL0 & ~SL1 & ~SL2;
  assert_output (X0, new_X0);
  new_X1 = EN & SL0 & ~SL1 & ~SL2;
  assert_output (X1, new_X1);
  new_X2 = EN & ~SL0 & SL1 & ~SL2;
  assert_output (X2, new_X2);
  new_X3 = EN & SL0 & SL1 & ~SL2;
  assert_output (X3, new_X3);
  new_X4 = EN & ~SL0 & ~SL1 & SL2;
  assert_output (X4, new_X4);
  new_X5 = EN & SL0 & ~SL1 & SL2;
  assert_output (X5, new_X5);
  new_X6 = EN & ~SL0 & SL1 & SL2;
  assert_output (X6, new_X6);
  new_X7 = EN & SL0 & SL1 & SL2;
  assert_output (X7, new_X7);
}

dec8ah_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0 = adr[0];
  register int new_X0;
  register Net_Entry *X1 = adr[1];
  register int new_X1;
  register Net_Entry *X2 = adr[2];
  register int new_X2;
  register Net_Entry *X3 = adr[3];
  register int new_X3;
  register Net_Entry *X4 = adr[4];
  register int new_X4;
  register Net_Entry *X5 = adr[5];
  register int new_X5;
  register Net_Entry *X6 = adr[6];
  register int new_X6;
  register Net_Entry *X7 = adr[7];
  register int new_X7;
  register int SL0 = adr[8]->value;
  register int SL1 = adr[9]->value;
  register int SL2 = adr[10]->value;
  register int EN = adr[11]->value;
  
#ifdef DEBUG
  printf("enter dec8ah_code\n");
#endif
  new_X0 = EN & ~SL0 & ~SL1 & ~SL2;
  assert_output (X0, new_X0);
  new_X1 = EN & SL0 & ~SL1 & ~SL2;
  assert_output (X1, new_X1);
  new_X2 = EN & ~SL0 & SL1 & ~SL2;
  assert_output (X2, new_X2);
  new_X3 = EN & SL0 & SL1 & ~SL2;
  assert_output (X3, new_X3);
  new_X4 = EN & ~SL0 & ~SL1 & SL2;
  assert_output (X4, new_X4);
  new_X5 = EN & SL0 & ~SL1 & SL2;
  assert_output (X5, new_X5);
  new_X6 = EN & ~SL0 & SL1 & SL2;
  assert_output (X6, new_X6);
  new_X7 = EN & SL0 & SL1 & SL2;
  assert_output (X7, new_X7);
}
#endif
dff1a_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *SQ = adr[1];
  register int new_SQ;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  register int SD = adr[4]->value;
  register int SE = adr[5]->value;
  register int E1 = adr[6]->value;
  register int E2 = adr[7]->value;
  
#ifdef DEBUG
  printf("enter dff1a_code\n");
#endif
  new_Q = SD & SE | D & (E1 & E2) & ~SE | Q->value & ~(E1 & E2) & ~SE;
  assert_output (Q, new_Q);
  new_SQ = Q->value;
  assert_output (SQ, new_SQ);
}

dff4a_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register Net_Entry *SQ3 = adr[4];
  register int new_SQ3;
  register int D0 = adr[5]->value;
  register int D1 = adr[6]->value;
  register int D2 = adr[7]->value;
  register int D3 = adr[8]->value;
  register int CK = adr[9]->value;
  register int SD0 = adr[10]->value;
  register int SE = adr[11]->value;
  register int E1 = adr[12]->value;
  register int E2 = adr[13]->value;
  
#ifdef DEBUG
  printf("enter dff4a_code\n");
#endif
  new_Q0 = SD0 & SE | D0 & (E1 & E2) & ~SE | Q0->value & ~(E1 & E2) & ~SE;
  assert_output (Q0, new_Q0);
  new_Q1 = Q0->value & SE | D1 & (E1 & E2) & ~SE | Q1->value & ~(E1 & E2) & ~SE;
  assert_output (Q1, new_Q1);
  new_Q2 = Q1->value & SE | D2 & (E1 & E2) & ~SE | Q2->value & ~(E1 & E2) & ~SE;
  assert_output (Q2, new_Q2);
  new_Q3 = Q2->value & SE | D3 & (E1 & E2) & ~SE | Q3->value & ~(E1 & E2) & ~SE;
  assert_output (Q3, new_Q3);
  new_SQ3 = Q3->value;
  assert_output (SQ3, new_SQ3);
}

dff8a_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register Net_Entry *Q4 = adr[4];
  register int new_Q4;
  register Net_Entry *Q5 = adr[5];
  register int new_Q5;
  register Net_Entry *Q6 = adr[6];
  register int new_Q6;
  register Net_Entry *Q7 = adr[7];
  register int new_Q7;
  register Net_Entry *SQ7 = adr[8];
  register int new_SQ7;
  register int D0 = adr[9]->value;
  register int D1 = adr[10]->value;
  register int D2 = adr[11]->value;
  register int D3 = adr[12]->value;
  register int D4 = adr[13]->value;
  register int D5 = adr[14]->value;
  register int D6 = adr[15]->value;
  register int D7 = adr[16]->value;
  register int CK = adr[17]->value;
  register int SD = adr[18]->value;
  register int SE = adr[19]->value;
  register int E1 = adr[20]->value;
  register int E2 = adr[21]->value;
  
#ifdef DEBUG
  printf("enter dff8a_code\n");
#endif
  new_Q0 = SD & SE | D0 & (E1 & E2) & ~SE | Q0->value & ~(E1 & E2) & ~SE;
  assert_output (Q0, new_Q0);
  new_Q1 = Q0->value & SE | D1 & (E1 & E2) & ~SE | Q1->value & ~(E1 & E2) & ~SE;
  assert_output (Q1, new_Q1);
  new_Q2 = Q1->value & SE | D2 & (E1 & E2) & ~SE | Q2->value & ~(E1 & E2) & ~SE;
  assert_output (Q2, new_Q2);
  new_Q3 = Q2->value & SE | D3 & (E1 & E2) & ~SE | Q3->value & ~(E1 & E2) & ~SE;
  assert_output (Q3, new_Q3);
  new_Q4 = Q3->value & SE | D4 & (E1 & E2) & ~SE | Q4->value & ~(E1 & E2) & ~SE;
  assert_output (Q4, new_Q4);
  new_Q5 = Q4->value & SE | D5 & (E1 & E2) & ~SE | Q5->value & ~(E1 & E2) & ~SE;
  assert_output (Q5, new_Q5);
  new_Q6 = Q5->value & SE | D6 & (E1 & E2) & ~SE | Q6->value & ~(E1 & E2) & ~SE;
  assert_output (Q6, new_Q6);
  new_Q7 = Q6->value & SE | D7 & (E1 & E2) & ~SE | Q7->value & ~(E1 & E2) & ~SE;
  assert_output (Q7, new_Q7);
  new_SQ7 = Q7->value;
  assert_output (SQ7, new_SQ7);
}

dfflp_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  register int SDI = adr[4]->value;
  register int SE = adr[5]->value;
  
#ifdef DEBUG
  printf("enter dfflp_code\n");
#endif
  new_Q = D & ~SE | SDI & SE;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

dfflpa_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  register int SDI = adr[4]->value;
  register int SE = adr[5]->value;
  
#ifdef DEBUG
  printf("enter dfflpa_code\n");
#endif
  new_Q = D & ~SE | SDI & SE;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

dfflpah_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  register int SDI = adr[4]->value;
  register int SE = adr[5]->value;
  
#ifdef DEBUG
  printf("enter dfflpah_code\n");
#endif
  new_Q = D & ~SE | SDI & SE;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

dfflpb_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *SQ = adr[1];
  register int new_SQ;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  register int SDI = adr[4]->value;
  register int SE = adr[5]->value;
  
#ifdef DEBUG
  printf("enter dfflpb_code\n");
#endif
  new_Q = D & ~SE | SDI & SE;
  assert_output (Q, new_Q);
  new_SQ = Q->value;
  assert_output (SQ, new_SQ);
}

dfflpbh_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *SQ = adr[1];
  register int new_SQ;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  register int SDI = adr[4]->value;
  register int SE = adr[5]->value;
  
#ifdef DEBUG
  printf("enter dfflpbh_code\n");
#endif
  new_Q = D & ~SE | SDI & SE;
  assert_output (Q, new_Q);
  new_SQ = Q->value;
  assert_output (SQ, new_SQ);
}

dffmx1_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *SQ = adr[1];
  register int new_SQ;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int SL = adr[4]->value;
  register int CK = adr[5]->value;
  register int SD = adr[6]->value;
  register int SE = adr[7]->value;
  register int E1 = adr[8]->value;
  register int E2 = adr[9]->value;
  
#ifdef DEBUG
  printf("enter dffmx1_code\n");
#endif
  new_Q = SD & SE | A & ~SL & (E1 & E2) & ~SE | B & SL & (E1 & E2) & ~SE | Q->value & ~(E1 & E2) & ~SE;
  assert_output (Q, new_Q);
  new_SQ = Q->value;
  assert_output (SQ, new_SQ);
}

dffmx4a_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register Net_Entry *SQ3 = adr[4];
  register int new_SQ3;
  register int A0 = adr[5]->value;
  register int B0 = adr[6]->value;
  register int A1 = adr[7]->value;
  register int B1 = adr[8]->value;
  register int A2 = adr[9]->value;
  register int B2 = adr[10]->value;
  register int A3 = adr[11]->value;
  register int B3 = adr[12]->value;
  register int SL = adr[13]->value;
  register int CK = adr[14]->value;
  register int SD0 = adr[15]->value;
  register int SE = adr[16]->value;
  register int E1 = adr[17]->value;
  register int E2 = adr[18]->value;
  
#ifdef DEBUG
  printf("enter dffmx4a_code\n");
#endif
  new_Q0 = SD0 & SE | A0 & ~SL & (E1 & E2) & ~SE | B0 & SL & (E1 & E2) & ~SE | Q0->value & ~(E1 & E2) & ~SE;
  assert_output (Q0, new_Q0);
  new_Q1 = Q0->value & SE | A1 & ~SL & (E1 & E2) & ~SE | B1 & SL & (E1 & E2) & ~SE | Q1->value & ~(E1 & E2) & ~SE;
  assert_output (Q1, new_Q1);
  new_Q2 = Q1->value & SE | A2 & ~SL & (E1 & E2) & ~SE | B2 & SL & (E1 & E2) & ~SE | Q2->value & ~(E1 & E2) & ~SE;
  assert_output (Q2, new_Q2);
  new_Q3 = Q2->value & SE | A3 & ~SL & (E1 & E2) & ~SE | B3 & SL & (E1 & E2) & ~SE | Q3->value & ~(E1 & E2) & ~SE;
  assert_output (Q3, new_Q3);
  new_SQ3 = Q3->value;
  assert_output (SQ3, new_SQ3);
}

dffp_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  
#ifdef DEBUG
  printf("enter dffp_code\n");
#endif
  new_Q = D;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

dffp4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register int D0 = adr[4]->value;
  register int D1 = adr[5]->value;
  register int D2 = adr[6]->value;
  register int D3 = adr[7]->value;
  register int CK = adr[8]->value;
  
#ifdef DEBUG
  printf("enter dffp4_code\n");
#endif
  new_Q0 = D0;
  assert_output (Q0, new_Q0);
  new_Q1 = D1;
  assert_output (Q1, new_Q1);
  new_Q2 = D2;
  assert_output (Q2, new_Q2);
  new_Q3 = D3;
  assert_output (Q3, new_Q3);
}

dffp4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register int D0 = adr[4]->value;
  register int D1 = adr[5]->value;
  register int D2 = adr[6]->value;
  register int D3 = adr[7]->value;
  register int CK = adr[8]->value;
  
#ifdef DEBUG
  printf("enter dffp4h_code\n");
#endif
  new_Q0 = D0;
  assert_output (Q0, new_Q0);
  new_Q1 = D1;
  assert_output (Q1, new_Q1);
  new_Q2 = D2;
  assert_output (Q2, new_Q2);
  new_Q3 = D3;
  assert_output (Q3, new_Q3);
}

dffph_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  
#ifdef DEBUG
  printf("enter dffph_code\n");
#endif
  new_Q = D;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

dffr1_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *SQ = adr[1];
  register int new_SQ;
  register int RB = adr[2]->value;
  register int D = adr[3]->value;
  register int CK = adr[4]->value;
  register int SD = adr[5]->value;
  register int SE = adr[6]->value;
  register int E1 = adr[7]->value;
  register int E2 = adr[8]->value;
  
#ifdef DEBUG
  printf("enter dffr1_code\n");
#endif
  new_Q = RB & (SD & SE | D & (E1 & E2) & ~SE | Q->value & ~(E1 & E2) & ~SE);
  assert_output (Q, new_Q);
  new_SQ = Q->value;
  assert_output (SQ, new_SQ);
}

dffr1h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *SQ = adr[1];
  register int new_SQ;
  register int RB = adr[2]->value;
  register int D = adr[3]->value;
  register int CK = adr[4]->value;
  register int SD = adr[5]->value;
  register int SE = adr[6]->value;
  register int E1 = adr[7]->value;
  register int E2 = adr[8]->value;
  
#ifdef DEBUG
  printf("enter dffr1h_code\n");
#endif
  new_Q = RB & (SD & SE | D & (E1 & E2) & ~SE | Q->value & ~(E1 & E2) & ~SE);
  assert_output (Q, new_Q);
  new_SQ = Q->value;
  assert_output (SQ, new_SQ);
}

dffr4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register Net_Entry *SQ3 = adr[4];
  register int new_SQ3;
  register int RB = adr[5]->value;
  register int D0 = adr[6]->value;
  register int D1 = adr[7]->value;
  register int D2 = adr[8]->value;
  register int D3 = adr[9]->value;
  register int CK = adr[10]->value;
  register int SD = adr[11]->value;
  register int SE = adr[12]->value;
  register int E1 = adr[13]->value;
  register int E2 = adr[14]->value;
  
#ifdef DEBUG
  printf("enter dffr4_code\n");
#endif
  new_Q0 = RB & (SD & SE | D0 & (E1 & E2) & ~SE | Q0->value & ~(E1 & E2) & ~SE);
  assert_output (Q0, new_Q0);
  new_Q1 = RB & (Q0->value & SE | D1 & (E1 & E2) & ~SE | Q1->value & ~(E1 & E2) & ~SE);
  assert_output (Q1, new_Q1);
  new_Q2 = RB & (Q1->value & SE | D2 & (E1 & E2) & ~SE | Q2->value & ~(E1 & E2) & ~SE);
  assert_output (Q2, new_Q2);
  new_Q3 = RB & (Q2->value & SE | D3 & (E1 & E2) & ~SE | Q3->value & ~(E1 & E2) & ~SE);
  assert_output (Q3, new_Q3);
  new_SQ3 = Q3->value;
  assert_output (SQ3, new_SQ3);
}

dffr4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register Net_Entry *SQ3 = adr[4];
  register int new_SQ3;
  register int RB = adr[5]->value;
  register int D0 = adr[6]->value;
  register int D1 = adr[7]->value;
  register int D2 = adr[8]->value;
  register int D3 = adr[9]->value;
  register int CK = adr[10]->value;
  register int SD = adr[11]->value;
  register int SE = adr[12]->value;
  register int E1 = adr[13]->value;
  register int E2 = adr[14]->value;
  
#ifdef DEBUG
  printf("enter dffr4h_code\n");
#endif
  new_Q0 = RB & (SD & SE | D0 & (E1 & E2) & ~SE | Q0->value & ~(E1 & E2) & ~SE);
  assert_output (Q0, new_Q0);
  new_Q1 = RB & (Q0->value & SE | D1 & (E1 & E2) & ~SE | Q1->value & ~(E1 & E2) & ~SE);
  assert_output (Q1, new_Q1);
  new_Q2 = RB & (Q1->value & SE | D2 & (E1 & E2) & ~SE | Q2->value & ~(E1 & E2) & ~SE);
  assert_output (Q2, new_Q2);
  new_Q3 = RB & (Q2->value & SE | D3 & (E1 & E2) & ~SE | Q3->value & ~(E1 & E2) & ~SE);
  assert_output (Q3, new_Q3);
  new_SQ3 = Q3->value;
  assert_output (SQ3, new_SQ3);
}

dffrlp_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  register int SDI = adr[4]->value;
  register int SE = adr[5]->value;
  register int RB = adr[6]->value;
  
#ifdef DEBUG
  printf("enter dffrlp_code\n");
#endif
  new_Q = (D & ~SE | SDI & SE) & RB;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

dffrlph_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int D = adr[2]->value;
  register int CK = adr[3]->value;
  register int SDI = adr[4]->value;
  register int SE = adr[5]->value;
  register int RB = adr[6]->value;
  
#ifdef DEBUG
  printf("enter dffrlph_code\n");
#endif
  new_Q = (D & ~SE | SDI & SE) & RB;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

dffrp_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int RB = adr[2]->value;
  register int D = adr[3]->value;
  register int CK = adr[4]->value;
  
#ifdef DEBUG
  printf("enter dffrp_code\n");
#endif
  new_Q = D & RB;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

dffrp4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register int RB = adr[4]->value;
  register int D0 = adr[5]->value;
  register int D1 = adr[6]->value;
  register int D2 = adr[7]->value;
  register int D3 = adr[8]->value;
  register int CK = adr[9]->value;
  
#ifdef DEBUG
  printf("enter dffrp4_code\n");
#endif
  new_Q0 = D0 & RB;
  assert_output (Q0, new_Q0);
  new_Q1 = D1 & RB;
  assert_output (Q1, new_Q1);
  new_Q2 = D2 & RB;
  assert_output (Q2, new_Q2);
  new_Q3 = D3 & RB;
  assert_output (Q3, new_Q3);
}

dffrp4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register int RB = adr[4]->value;
  register int D0 = adr[5]->value;
  register int D1 = adr[6]->value;
  register int D2 = adr[7]->value;
  register int D3 = adr[8]->value;
  register int CK = adr[9]->value;
  
#ifdef DEBUG
  printf("enter dffrp4h_code\n");
#endif
  new_Q0 = D0 & RB;
  assert_output (Q0, new_Q0);
  new_Q1 = D1 & RB;
  assert_output (Q1, new_Q1);
  new_Q2 = D2 & RB;
  assert_output (Q2, new_Q2);
  new_Q3 = D3 & RB;
  assert_output (Q3, new_Q3);
}

dffrph_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int RB = adr[2]->value;
  register int D = adr[3]->value;
  register int CK = adr[4]->value;
  
#ifdef DEBUG
  printf("enter dffrph_code\n");
#endif
  new_Q = D & RB;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

dly100_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter dly100_code\n");
#endif
  new_X = A;
  assert_output (X, new_X);
}

dly8_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter dly8_code\n");
#endif
  new_X = A;
  assert_output (X, new_X);
}

ecomp4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *AEBO = adr[0];
  register int new_AEBO;
  register int A0 = adr[1]->value;
  register int A1 = adr[2]->value;
  register int A2 = adr[3]->value;
  register int A3 = adr[4]->value;
  register int B0 = adr[5]->value;
  register int B1 = adr[6]->value;
  register int B2 = adr[7]->value;
  register int B3 = adr[8]->value;
  register int AEBI = adr[9]->value;
  
#ifdef DEBUG
  printf("enter ecomp4_code\n");
#endif
  new_AEBO = AEBI & (A0 & B0 | ~A0 & ~B0) & (A1 & B1 | ~A1 & ~B1) & (A2 & B2 | ~A2 & ~B2) & (A3 & B3 | ~A3 & ~B3);
  assert_output (AEBO, new_AEBO);
}

exnor_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter exnor_code\n");
#endif
  new_X = ~(A ^ B);
  assert_output (X, new_X);
}

exnor3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter exnor3_code\n");
#endif
  new_X = ~(A ^ B ^ C);
  assert_output (X, new_X);
}

exnor3h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter exnor3h_code\n");
#endif
  new_X = ~(A ^ B ^ C);
  assert_output (X, new_X);
}

exnora_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter exnora_code\n");
#endif
  new_X = ~(A ^ B);
  assert_output (X, new_X);
}

exnorh_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter exnorh_code\n");
#endif
  new_X = ~(A ^ B);
  assert_output (X, new_X);
}

exor_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter exor_code\n");
#endif
  new_X = A ^ B;
  assert_output (X, new_X);
}

exor3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter exor3_code\n");
#endif
  new_X = A ^ B ^ C;
  assert_output (X, new_X);
}

exor3h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter exor3h_code\n");
#endif
  new_X = A ^ B ^ C;
  assert_output (X, new_X);
}

exor4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter exor4_code\n");
#endif
  new_X = A ^ B ^ C ^ D;
  assert_output (X, new_X);
}

exor4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter exor4h_code\n");
#endif
  new_X = A ^ B ^ C ^ D;
  assert_output (X, new_X);
}

exor9h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  register int E = adr[5]->value;
  register int F = adr[6]->value;
  register int G = adr[7]->value;
  register int H = adr[8]->value;
  register int I = adr[9]->value;
  
#ifdef DEBUG
  printf("enter exor9h_code\n");
#endif
  new_X = A ^ B ^ C ^ D ^ E ^ F ^ G ^ H ^ I;
  assert_output (X, new_X);
}

exora_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter exora_code\n");
#endif
  new_X = A ^ B;
  assert_output (X, new_X);
}

exorh_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter exorh_code\n");
#endif
  new_X = A ^ B;
  assert_output (X, new_X);
}

ici_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DI = adr[0];
  register int new_DI;
  register int PAD = adr[1]->value;
  register int IC = adr[2]->value;
  
#ifdef DEBUG
  printf("enter ici_code\n");
#endif
  new_DI = ~PAD;
  assert_output (DI, new_DI);
}

icih_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DI = adr[0];
  register int new_DI;
  register int PAD = adr[1]->value;
  register int IC = adr[2]->value;
  
#ifdef DEBUG
  printf("enter icih_code\n");
#endif
  new_DI = ~PAD;
  assert_output (DI, new_DI);
}

icn_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DI = adr[0];
  register int new_DI;
  register int PAD = adr[1]->value;
  register int IC = adr[2]->value;
  
#ifdef DEBUG
  printf("enter icn_code\n");
#endif
  new_DI = PAD;
  assert_output (DI, new_DI);
}

icnh_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DI = adr[0];
  register int new_DI;
  register int PAD = adr[1]->value;
  register int IC = adr[2]->value;
  
#ifdef DEBUG
  printf("enter icnh_code\n");
#endif
  new_DI = PAD;
  assert_output (DI, new_DI);
}

inc4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *S0 = adr[0];
  register int new_S0;
  register Net_Entry *S1 = adr[1];
  register int new_S1;
  register Net_Entry *S2 = adr[2];
  register int new_S2;
  register Net_Entry *S3 = adr[3];
  register int new_S3;
  register Net_Entry *CO = adr[4];
  register int new_CO;
  register Net_Entry *PR = adr[5];
  register int new_PR;
  register int A0 = adr[6]->value;
  register int A1 = adr[7]->value;
  register int A2 = adr[8]->value;
  register int A3 = adr[9]->value;
  register int CI = adr[10]->value;
  
#ifdef DEBUG
  printf("enter inc4_code\n");
#endif
  new_S0 = CI & ~A0 | ~CI & A0;
  assert_output (S0, new_S0);
  new_S1 = (CI & A0) & ~A1 | ~(CI & A0) & A1;
  assert_output (S1, new_S1);
  new_S2 = (CI & A0 & A1) & ~A2 | ~(CI & A0 & A1) & A2;
  assert_output (S2, new_S2);
  new_S3 = (CI & A0 & A1 & A2) & ~A3 | ~(CI & A0 & A1 & A2) & A3;
  assert_output (S3, new_S3);
  new_PR = A0 & A1 & A2 & A3;
  assert_output (PR, new_PR);
  new_CO = CI & PR->value;
  assert_output (CO, new_CO);
}

inc4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *S0 = adr[0];
  register int new_S0;
  register Net_Entry *S1 = adr[1];
  register int new_S1;
  register Net_Entry *S2 = adr[2];
  register int new_S2;
  register Net_Entry *S3 = adr[3];
  register int new_S3;
  register Net_Entry *CO = adr[4];
  register int new_CO;
  register Net_Entry *PR = adr[5];
  register int new_PR;
  register int A0 = adr[6]->value;
  register int A1 = adr[7]->value;
  register int A2 = adr[8]->value;
  register int A3 = adr[9]->value;
  register int CI = adr[10]->value;
  
#ifdef DEBUG
  printf("enter inc4h_code\n");
#endif
  new_S0 = CI & ~A0 | ~CI & A0;
  assert_output (S0, new_S0);
  new_S1 = (CI & A0) & ~A1 | ~(CI & A0) & A1;
  assert_output (S1, new_S1);
  new_S2 = (CI & A0 & A1) & ~A2 | ~(CI & A0 & A1) & A2;
  assert_output (S2, new_S2);
  new_S3 = (CI & A0 & A1 & A2) & ~A3 | ~(CI & A0 & A1 & A2) & A3;
  assert_output (S3, new_S3);
  new_PR = A0 & A1 & A2 & A3;
  assert_output (PR, new_PR);
  new_CO = CI & PR->value;
  assert_output (CO, new_CO);
}

inv_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter inv_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

inv2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter inv2_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

inv2b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter inv2b_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

inv3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter inv3_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

inv3b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter inv3b_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

inv4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter inv4_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

inv4b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter inv4b_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

inv8_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter inv8_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

inv8b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter inv8b_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

invb_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter invb_code\n");
#endif
  new_X = ~A;
  uncond_assert_output (X, new_X);
}

invt_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DB = adr[1];
  register int new_DB;
  register int D = adr[2]->value;
  register int ENB = adr[3]->value;
  
#ifdef DEBUG
  printf("enter invt_code\n");
#endif
  if (~ENB) {
    new_DB = ~D;
    assert_output (DB, new_DB);
  }
}

itn_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DI = adr[0];
  register int new_DI;
  register int PAD = adr[1]->value;
  register int IC = adr[2]->value;
  
#ifdef DEBUG
  printf("enter itn_code\n");
#endif
  new_DI = PAD;
  assert_output (DI, new_DI);
}

itnh_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *DI = adr[0];
  register int new_DI;
  register int PAD = adr[1]->value;
  register int IC = adr[2]->value;
  
#ifdef DEBUG
  printf("enter itnh_code\n");
#endif
  new_DI = PAD;
  assert_output (DI, new_DI);
}

lacg4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *CNX = adr[0];
  register int new_CNX;
  register Net_Entry *CNY = adr[1];
  register int new_CNY;
  register Net_Entry *CNZ = adr[2];
  register int new_CNZ;
  register Net_Entry *CO = adr[3];
  register int new_CO;
  register Net_Entry *PR = adr[4];
  register int new_PR;
  register Net_Entry *GE = adr[5];
  register int new_GE;
  register int CI = adr[6]->value;
  register int P0 = adr[7]->value;
  register int G0 = adr[8]->value;
  register int P1 = adr[9]->value;
  register int G1 = adr[10]->value;
  register int P2 = adr[11]->value;
  register int G2 = adr[12]->value;
  register int P3 = adr[13]->value;
  register int G3 = adr[14]->value;
  
#ifdef DEBUG
  printf("enter lacg4_code\n");
#endif
  new_PR = (P0 & P1 & P2 & P3);
  assert_output (PR, new_PR);
  new_GE = (G0 & P1 & P2 & P3) | (G1 & P2 & P3) | (G2 & P3) | G3;
  assert_output (GE, new_GE);
  new_CNX = (P0 & CI) | G0;
  assert_output (CNX, new_CNX);
  new_CNY = (P0 & P1 & CI) | (P1 & G0) | G1;
  assert_output (CNY, new_CNY);
  new_CNZ = (P0 & P1 & P2 & CI) | (P2 & P1 & G0) | (P2 & G1) | G2;
  assert_output (CNZ, new_CNZ);
  new_CO = (PR->value & CI) | GE->value;
  assert_output (CO, new_CO);
}

latp4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register int D0 = adr[4]->value;
  register int D1 = adr[5]->value;
  register int D2 = adr[6]->value;
  register int D3 = adr[7]->value;
  register int GB = adr[8]->value;
  
#ifdef DEBUG
  printf("enter latp4_code\n");
#endif
  new_Q0 = GB & Q0->value | ~GB & D0;
  assert_output (Q0, new_Q0);
  new_Q1 = GB & Q1->value | ~GB & D1;
  assert_output (Q1, new_Q1);
  new_Q2 = GB & Q2->value | ~GB & D2;
  assert_output (Q2, new_Q2);
  new_Q3 = GB & Q3->value | ~GB & D3;
  assert_output (Q3, new_Q3);
}

latpi4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *QB0 = adr[0];
  register int new_QB0;
  register Net_Entry *QB1 = adr[1];
  register int new_QB1;
  register Net_Entry *QB2 = adr[2];
  register int new_QB2;
  register Net_Entry *QB3 = adr[3];
  register int new_QB3;
  register int D0 = adr[4]->value;
  register int D1 = adr[5]->value;
  register int D2 = adr[6]->value;
  register int D3 = adr[7]->value;
  register int GB = adr[8]->value;
  
#ifdef DEBUG
  printf("enter latpi4_code\n");
#endif
  new_QB0 = GB & QB0->value | ~GB & ~D0;
  assert_output (QB0, new_QB0);
  new_QB1 = GB & QB1->value | ~GB & ~D1;
  assert_output (QB1, new_QB1);
  new_QB2 = GB & QB2->value | ~GB & ~D2;
  assert_output (QB2, new_QB2);
  new_QB3 = GB & QB3->value | ~GB & ~D3;
  assert_output (QB3, new_QB3);
}

latp4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q0 = adr[0];
  register int new_Q0;
  register Net_Entry *Q1 = adr[1];
  register int new_Q1;
  register Net_Entry *Q2 = adr[2];
  register int new_Q2;
  register Net_Entry *Q3 = adr[3];
  register int new_Q3;
  register int D0 = adr[4]->value;
  register int D1 = adr[5]->value;
  register int D2 = adr[6]->value;
  register int D3 = adr[7]->value;
  register int GB = adr[8]->value;
  
#ifdef DEBUG
  printf("enter latp4h_code\n");
#endif
  new_Q0 = GB & Q0->value | ~GB & D0;
  assert_output (Q0, new_Q0);
  new_Q1 = GB & Q1->value | ~GB & D1;
  assert_output (Q1, new_Q1);
  new_Q2 = GB & Q2->value | ~GB & D2;
  assert_output (Q2, new_Q2);
  new_Q3 = GB & Q3->value | ~GB & D3;
  assert_output (Q3, new_Q3);
}

latn_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register Net_Entry *QB = adr[1];
  register int new_QB;
  register int D = adr[2]->value;
  register int G = adr[3]->value;
  
#ifdef DEBUG
  printf("enter latn_code\n");
#endif
  new_Q = ~G & Q->value | G & D;
  assert_output (Q, new_Q);
  new_QB = ~Q->value;
  assert_output (QB, new_QB);
}

latpa_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register int D = adr[1]->value;
  register int GB = adr[2]->value;
  
#ifdef DEBUG
  printf("enter latpa_code\n");
#endif
  new_Q = GB & Q->value | ~GB & D;
  assert_output (Q, new_Q);
}

latpah_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *Q = adr[0];
  register int new_Q;
  register int D = adr[1]->value;
  register int GB = adr[2]->value;
  
#ifdef DEBUG
  printf("enter latpah_code\n");
#endif
  new_Q = GB & Q->value | ~GB & D;
  assert_output (Q, new_Q);
}

maj3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter maj3_code\n");
#endif
  new_X = ~((A & B) | (A & C) | (B & C));
  assert_output (X, new_X);
}

maj3h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter maj3h_code\n");
#endif
  new_X = ~((A & B) | (A & C) | (B & C));
  assert_output (X, new_X);
}

mcomp2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *ul = adr[0];
  register Net_Entry *ug = adr[1];
  register Net_Entry *AGO = adr[2];
  register int new_AGO;
  register Net_Entry *AEO = adr[3];
  register int new_AEO;
  register Net_Entry *ALO = adr[4];
  register int new_ALO;
  register int A1 = adr[5]->value;
  register int B1 = adr[6]->value;
  register int A0 = adr[7]->value;
  register int B0 = adr[8]->value;
  register int AGI = adr[9]->value;
  register int AEI = adr[10]->value;
  register int ALI = adr[11]->value;
  
#ifdef DEBUG
  printf("enter mcomp2_code\n");
#endif
  ug->value = A1 & ~B1 | A0 & ~B0 & ~(~A1 & B1);
  ul->value = ~A1 & B1 | ~A0 & B0 & ~(A1 & ~B1);
  new_AGO = AGI & ~(ug->value | ul->value) | ug->value;
  assert_output (AGO, new_AGO);
  new_ALO = ALI & ~(ug->value | ul->value) | ul->value;
  assert_output (ALO, new_ALO);
  new_AEO = AEI & ~(ug->value | ul->value);
  assert_output (AEO, new_AEO);
}

mcomp4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *AEO = adr[0];
  register int new_AEO;
  register Net_Entry *ALO = adr[1];
  register int new_ALO;
  register int A0 = adr[2]->value;
  register int A1 = adr[3]->value;
  register int A2 = adr[4]->value;
  register int A3 = adr[5]->value;
  register int B0 = adr[6]->value;
  register int B1 = adr[7]->value;
  register int B2 = adr[8]->value;
  register int B3 = adr[9]->value;
  
#ifdef DEBUG
  printf("enter mcomp4_code\n");
#endif
  new_AEO = (A0 & B0 | ~A0 & ~B0) & (A1 & B1 | ~A1 & ~B1) & (A2 & B2 | ~A2 & ~B2) & (A3 & B3 | ~A3 & ~B3);
  assert_output (AEO, new_AEO);
  new_ALO = ~A3 & B3 | ~A2 & B2 & ~(A3 & ~B3) | ~A1 & B1 & ~(A3 & ~B3) & ~(A2 & ~B2) | ~A0 & B0 & ~(A3 & ~B3) & ~(A2 & ~B2) & ~(A1 & ~B1);
  assert_output (ALO, new_ALO);
}

mux2a_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int SL = adr[2]->value;
  register int B = adr[3]->value;
  
#ifdef DEBUG
  printf("enter mux2a_code\n");
#endif
  new_X = (A & ~SL | B & SL);
  assert_output (X, new_X);
}

mux2h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int SL = adr[2]->value;
  register int B = adr[3]->value;
  
#ifdef DEBUG
  printf("enter mux2h_code\n");
#endif
  new_X = (A & ~SL | B & SL);
  assert_output (X, new_X);
}

mux2i_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *XB = adr[0];
  register int new_XB;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int SL = adr[3]->value;
  
#ifdef DEBUG
  printf("enter mux2i_code\n");
#endif
  new_XB = ~(A & ~SL | B & SL);
  assert_output (XB, new_XB);
}

mux2ih_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *XB = adr[0];
  register int new_XB;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int SL = adr[3]->value;
  
#ifdef DEBUG
  printf("enter mux2ih_code\n");
#endif
  new_XB = ~(A & ~SL | B & SL);
  assert_output (XB, new_XB);
}

mux4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  register int SL0 = adr[5]->value;
  register int SL1 = adr[6]->value;
  
#ifdef DEBUG
  printf("enter mux4_code\n");
#endif
  new_X = A & ~SL1 & ~SL0 | B & ~SL1 & SL0 | C & SL1 & ~SL0 | D & SL1 & SL0;
  assert_output (X, new_X);
}

mux41a_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0 = adr[0];
  register int new_X0;
  register Net_Entry *X1 = adr[1];
  register int new_X1;
  register Net_Entry *X2 = adr[2];
  register int new_X2;
  register Net_Entry *X3 = adr[3];
  register int new_X3;
  register int A0 = adr[4]->value;
  register int B0 = adr[5]->value;
  register int A1 = adr[6]->value;
  register int B1 = adr[7]->value;
  register int A2 = adr[8]->value;
  register int B2 = adr[9]->value;
  register int A3 = adr[10]->value;
  register int B3 = adr[11]->value;
  register int SL = adr[12]->value;
  
#ifdef DEBUG
  printf("enter mux41a_code\n");
#endif
  new_X0 = A0 & ~SL | B0 & SL;
  assert_output (X0, new_X0);
  new_X1 = A1 & ~SL | B1 & SL;
  assert_output (X1, new_X1);
  new_X2 = A2 & ~SL | B2 & SL;
  assert_output (X2, new_X2);
  new_X3 = A3 & ~SL | B3 & SL;
  assert_output (X3, new_X3);
}

mux41ah_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X0 = adr[0];
  register int new_X0;
  register Net_Entry *X1 = adr[1];
  register int new_X1;
  register Net_Entry *X2 = adr[2];
  register int new_X2;
  register Net_Entry *X3 = adr[3];
  register int new_X3;
  register int A0 = adr[4]->value;
  register int B0 = adr[5]->value;
  register int A1 = adr[6]->value;
  register int B1 = adr[7]->value;
  register int A2 = adr[8]->value;
  register int B2 = adr[9]->value;
  register int A3 = adr[10]->value;
  register int B3 = adr[11]->value;
  register int SL = adr[12]->value;
  
#ifdef DEBUG
  printf("enter mux41ah_code\n");
#endif
  new_X0 = A0 & ~SL | B0 & SL;
  assert_output (X0, new_X0);
  new_X1 = A1 & ~SL | B1 & SL;
  assert_output (X1, new_X1);
  new_X2 = A2 & ~SL | B2 & SL;
  assert_output (X2, new_X2);
  new_X3 = A3 & ~SL | B3 & SL;
  assert_output (X3, new_X3);
}

mux41i_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *XB0 = adr[0];
  register int new_XB0;
  register Net_Entry *XB1 = adr[1];
  register int new_XB1;
  register Net_Entry *XB2 = adr[2];
  register int new_XB2;
  register Net_Entry *XB3 = adr[3];
  register int new_XB3;
  register int A0 = adr[4]->value;
  register int B0 = adr[5]->value;
  register int A1 = adr[6]->value;
  register int B1 = adr[7]->value;
  register int A2 = adr[8]->value;
  register int B2 = adr[9]->value;
  register int A3 = adr[10]->value;
  register int B3 = adr[11]->value;
  register int SL = adr[12]->value;
  
#ifdef DEBUG
  printf("enter mux41i_code\n");
#endif
  new_XB0 = ~(A0 & ~SL | B0 & SL);
  assert_output (XB0, new_XB0);
  new_XB1 = ~(A1 & ~SL | B1 & SL);
  assert_output (XB1, new_XB1);
  new_XB2 = ~(A2 & ~SL | B2 & SL);
  assert_output (XB2, new_XB2);
  new_XB3 = ~(A3 & ~SL | B3 & SL);
  assert_output (XB3, new_XB3);
}

mux41ih_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *XB0 = adr[0];
  register int new_XB0;
  register Net_Entry *XB1 = adr[1];
  register int new_XB1;
  register Net_Entry *XB2 = adr[2];
  register int new_XB2;
  register Net_Entry *XB3 = adr[3];
  register int new_XB3;
  register int A0 = adr[4]->value;
  register int B0 = adr[5]->value;
  register int A1 = adr[6]->value;
  register int B1 = adr[7]->value;
  register int A2 = adr[8]->value;
  register int B2 = adr[9]->value;
  register int A3 = adr[10]->value;
  register int B3 = adr[11]->value;
  register int SL = adr[12]->value;
  
#ifdef DEBUG
  printf("enter mux41ih_code\n");
#endif
  new_XB0 = ~(A0 & ~SL | B0 & SL);
  assert_output (XB0, new_XB0);
  new_XB1 = ~(A1 & ~SL | B1 & SL);
  assert_output (XB1, new_XB1);
  new_XB2 = ~(A2 & ~SL | B2 & SL);
  assert_output (XB2, new_XB2);
  new_XB3 = ~(A3 & ~SL | B3 & SL);
  assert_output (XB3, new_XB3);
}

mux4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  register int SL0 = adr[5]->value;
  register int SL1 = adr[6]->value;
  
#ifdef DEBUG
  printf("enter mux4h_code\n");
#endif
  new_X = A & ~SL1 & ~SL0 | B & ~SL1 & SL0 | C & SL1 & ~SL0 | D & SL1 & SL0;
  assert_output (X, new_X);
}

mux8h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int D0 = adr[1]->value;
  register int D1 = adr[2]->value;
  register int D2 = adr[3]->value;
  register int D3 = adr[4]->value;
  register int D4 = adr[5]->value;
  register int D5 = adr[6]->value;
  register int D6 = adr[7]->value;
  register int D7 = adr[8]->value;
  register int SL0 = adr[9]->value;
  register int SL1 = adr[10]->value;
  register int SL2 = adr[11]->value;
  
#ifdef DEBUG
  printf("enter mux8h_code\n");
#endif
  new_X = ((D0 & ~SL0 | D1 & SL0) & ~SL1 | (D2 & ~SL0 | D3 & SL0) & SL1) & ~SL2 | ((D4 & ~SL0 | D5 & SL0) & ~SL1 | (D6 & ~SL0 | D7 & SL0) & SL1) & SL2;
  assert_output (X, new_X);
}

mux8ah_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int D0 = adr[1]->value;
  register int D1 = adr[2]->value;
  register int D2 = adr[3]->value;
  register int D3 = adr[4]->value;
  register int D4 = adr[5]->value;
  register int D5 = adr[6]->value;
  register int D6 = adr[7]->value;
  register int D7 = adr[8]->value;
  register int SL0 = adr[9]->value;
  register int SL1 = adr[10]->value;
  register int SL2 = adr[11]->value;
  
#ifdef DEBUG
  printf("enter mux8ah_code\n");
#endif
  new_X = ((D0 & ~SL0 | D1 & SL0) & ~SL1 | (D2 & ~SL0 | D3 & SL0) & SL1) & ~SL2 | ((D4 & ~SL0 | D5 & SL0) & ~SL1 | (D6 & ~SL0 | D7 & SL0) & SL1) & SL2;
  assert_output (X, new_X);
}

mx41_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int SLA = adr[2]->value;
  register int B = adr[3]->value;
  register int SLB = adr[4]->value;
  register int C = adr[5]->value;
  register int SLC = adr[6]->value;
  register int D = adr[7]->value;
  register int SLD = adr[8]->value;
  
#ifdef DEBUG
  printf("enter mx41_code\n");
#endif
  new_X = A & SLA | B & SLB | C & SLC | D & SLD;
  assert_output (X, new_X);
}

mx41h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int SLA = adr[2]->value;
  register int B = adr[3]->value;
  register int SLB = adr[4]->value;
  register int C = adr[5]->value;
  register int SLC = adr[6]->value;
  register int D = adr[7]->value;
  register int SLD = adr[8]->value;
  
#ifdef DEBUG
  printf("enter mx41h_code\n");
#endif
  new_X = A & SLA | B & SLB | C & SLC | D & SLD;
  assert_output (X, new_X);
}

mx81_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int SLA = adr[2]->value;
  register int B = adr[3]->value;
  register int SLB = adr[4]->value;
  register int C = adr[5]->value;
  register int SLC = adr[6]->value;
  register int D = adr[7]->value;
  register int SLD = adr[8]->value;
  register int E = adr[9]->value;
  register int SLE = adr[10]->value;
  register int F = adr[11]->value;
  register int SLF = adr[12]->value;
  register int G = adr[13]->value;
  register int SLG = adr[14]->value;
  register int H = adr[15]->value;
  register int SLH = adr[16]->value;
  
#ifdef DEBUG
  printf("enter mx81_code\n");
#endif
  new_X = A & SLA | B & SLB | C & SLC | D & SLD | E & SLE | F & SLF | G & SLG | H & SLH;
  assert_output (X, new_X);
}

mx81h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int SLA = adr[2]->value;
  register int B = adr[3]->value;
  register int SLB = adr[4]->value;
  register int C = adr[5]->value;
  register int SLC = adr[6]->value;
  register int D = adr[7]->value;
  register int SLD = adr[8]->value;
  register int E = adr[9]->value;
  register int SLE = adr[10]->value;
  register int F = adr[11]->value;
  register int SLF = adr[12]->value;
  register int G = adr[13]->value;
  register int SLG = adr[14]->value;
  register int H = adr[15]->value;
  register int SLH = adr[16]->value;
  
#ifdef DEBUG
  printf("enter mx81h_code\n");
#endif
  new_X = A & SLA | B & SLB | C & SLC | D & SLD | E & SLE | F & SLF | G & SLG | H & SLH;
  assert_output (X, new_X);
}

nan2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter nan2_code\n");
#endif
  new_X = ~(A & B);
  assert_output (X, new_X);
}

nan2b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter nan2b_code\n");
#endif
  new_X = ~(A & B);
  assert_output (X, new_X);
}

nan2h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter nan2h_code\n");
#endif
  new_X = ~(A & B);
  assert_output (X, new_X);
}

nan3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter nan3_code\n");
#endif
  new_X = ~(A & B & C);
  assert_output (X, new_X);
}

nan3h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter nan3h_code\n");
#endif
  new_X = ~(A & B & C);
  assert_output (X, new_X);
}

nan4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter nan4_code\n");
#endif
  new_X = ~(A & B & C & D);
  assert_output (X, new_X);
}

nan4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter nan4h_code\n");
#endif
  new_X = ~(A & B & C & D);
  assert_output (X, new_X);
}

nan5_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  register int E = adr[5]->value;
  
#ifdef DEBUG
  printf("enter nan5_code\n");
#endif
  new_X = ~(A & B & C & D & E);
  assert_output (X, new_X);
}

nan5h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  register int E = adr[5]->value;
  
#ifdef DEBUG
  printf("enter nan5h_code\n");
#endif
  new_X = ~(A & B & C & D & E);
  assert_output (X, new_X);
}

nan6ch_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register Net_Entry *Y = adr[1];
  register int new_X;
  register int new_Y;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int C = adr[4]->value;
  register int D = adr[5]->value;
  register int E = adr[6]->value;
  register int F = adr[7]->value;
  
#ifdef DEBUG
  printf("enter nan6ch_code\n");
#endif
  new_X = ~(A & B & C & D & E & F);
  new_Y = ~new_X;
  assert_output (X, new_X);
  assert_output (Y, new_Y);
}

nan8h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u = adr[0];
  register Net_Entry *X = adr[1];
  register int new_X;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int C = adr[4]->value;
  register int D = adr[5]->value;
  register int E = adr[6]->value;
  register int F = adr[7]->value;
  register int G = adr[8]->value;
  register int H = adr[9]->value;
  
#ifdef DEBUG
  printf("enter nan8h_code\n");
#endif
  u->value = A & B & C & D & E;
  new_X = ~(u->value & F & G & H);
  assert_output (X, new_X);
}

nor2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter nor2_code\n");
#endif
  new_X = ~(A | B);
  assert_output (X, new_X);
}

nor2b_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter nor2b_code\n");
#endif
  new_X = ~(A | B);
  assert_output (X, new_X);
}

nor2h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter nor2h_code\n");
#endif
  new_X = ~(A | B);
  assert_output (X, new_X);
}

nor3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter nor3_code\n");
#endif
  new_X = ~(A | B | C);
  assert_output (X, new_X);
}

nor3h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter nor3h_code\n");
#endif
  new_X = ~(A | B | C);
  assert_output (X, new_X);
}

nor4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter nor4_code\n");
#endif
  new_X = ~(A | B | C | D);
  assert_output (X, new_X);
}

nor4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter nor4h_code\n");
#endif
  new_X = ~(A | B | C | D);
  assert_output (X, new_X);
}

nor5_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  register int E = adr[5]->value;
  
#ifdef DEBUG
  printf("enter nor5_code\n");
#endif
  new_X = ~(A | B | C | D | E);
  assert_output (X, new_X);
}

nor5h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  register int E = adr[5]->value;
  
#ifdef DEBUG
  printf("enter nor5h_code\n");
#endif
  new_X = ~(A | B | C | D | E);
  assert_output (X, new_X);
}

nor6ch_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register Net_Entry *Y = adr[1];
  register int new_X;
  register int new_Y;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int C = adr[4]->value;
  register int D = adr[5]->value;
  register int E = adr[6]->value;
  register int F = adr[7]->value;
  
#ifdef DEBUG
  printf("enter nor6ch_code\n");
#endif
  new_X = ~(A | B | C | D | E | F);
  new_Y = ~new_X;
  assert_output (X, new_X);
  assert_output (Y, new_Y);
}

nor8h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u = adr[0];
  register Net_Entry *X = adr[1];
  register int new_X;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int C = adr[4]->value;
  register int D = adr[5]->value;
  register int E = adr[6]->value;
  register int F = adr[7]->value;
  register int G = adr[8]->value;
  register int H = adr[9]->value;
  
#ifdef DEBUG
  printf("enter nor8h_code\n");
#endif
  u->value = A | B | C | D | E;
  new_X = ~(u->value | F | G | H);
  assert_output (X, new_X);
}

oa211h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter oa211h_code\n");
#endif
  new_X = ((A | B) & C & D);
  assert_output (X, new_X);
}

oa21h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter oa21h_code\n");
#endif
  new_X = ((A | B) & C);
  assert_output (X, new_X);
}

oa22h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter oa22h_code\n");
#endif
  new_X = ((A | B) & (C | D));
  assert_output (X, new_X);
}

oai21_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter oai21_code\n");
#endif
  new_X = ~((A | B) & C);
  assert_output (X, new_X);
}

oai211_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter oai211_code\n");
#endif
  new_X = ~((A | B) & C & D);
  assert_output (X, new_X);
}

oai211h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter oai211h_code\n");
#endif
  new_X = ~((A | B) & C & D);
  assert_output (X, new_X);
}

oai21h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter oai21h_code\n");
#endif
  new_X = ~((A | B) & C);
  assert_output (X, new_X);
}

oai22_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter oai22_code\n");
#endif
  new_X = ~((A | B) & (C | D));
  assert_output (X, new_X);
}

oai22h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter oai22h_code\n");
#endif
  new_X = ~((A | B) & (C | D));
  assert_output (X, new_X);
}

on2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  
#ifdef DEBUG
  printf("enter on2_code\n");
#endif
  new_PAD = DO;
  assert_output (PAD, new_PAD);
}

on2t_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  register int EN = adr[2]->value;
  
#ifdef DEBUG
  printf("enter on2t_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
}

on4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  
#ifdef DEBUG
  printf("enter on4_code\n");
#endif
  new_PAD = DO;
  assert_output (PAD, new_PAD);
}

on4s2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  
#ifdef DEBUG
  printf("enter on4s2_code\n");
#endif
  new_PAD = DO;
  assert_output (PAD, new_PAD);
}

on4s4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  
#ifdef DEBUG
  printf("enter on4s4_code\n");
#endif
  new_PAD = DO;
  assert_output (PAD, new_PAD);
}

on4t_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  register int EN = adr[2]->value;
  
#ifdef DEBUG
  printf("enter on4t_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
}

on4ts2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  register int EN = adr[2]->value;
  
#ifdef DEBUG
  printf("enter on4ts2_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
}

on4ts4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  register int EN = adr[2]->value;
  
#ifdef DEBUG
  printf("enter on4ts4_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
}

on8_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  
#ifdef DEBUG
  printf("enter on8_code\n");
#endif
  new_PAD = DO;
  assert_output (PAD, new_PAD);
}

on8s2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  
#ifdef DEBUG
  printf("enter on8s2_code\n");
#endif
  new_PAD = DO;
  assert_output (PAD, new_PAD);
}

on8s4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  
#ifdef DEBUG
  printf("enter on8s4_code\n");
#endif
  new_PAD = DO;
  assert_output (PAD, new_PAD);
}

on8t_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  register int EN = adr[2]->value;
  
#ifdef DEBUG
  printf("enter on8t_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
}

on8ts2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  register int EN = adr[2]->value;
  
#ifdef DEBUG
  printf("enter on8ts2_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
}

on8ts4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *PAD = adr[0];
  register int new_PAD;
  register int DO = adr[1]->value;
  register int EN = adr[2]->value;
  
#ifdef DEBUG
  printf("enter on8ts4_code\n");
#endif
  if (~(~EN)) {
    new_PAD = DO;
    assert_output (PAD, new_PAD);
  }
}

or2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter or2_code\n");
#endif
  new_X = (A | B);
  assert_output (X, new_X);
}

or2h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  
#ifdef DEBUG
  printf("enter or2h_code\n");
#endif
  new_X = (A | B);
  assert_output (X, new_X);
}

or3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter or3_code\n");
#endif
  new_X = (A | B | C);
  assert_output (X, new_X);
}

or3h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  
#ifdef DEBUG
  printf("enter or3h_code\n");
#endif
  new_X = (A | B | C);
  assert_output (X, new_X);
}

or4_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter or4_code\n");
#endif
  new_X = (A | B | C | D);
  assert_output (X, new_X);
}

or4h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int B = adr[2]->value;
  register int C = adr[3]->value;
  register int D = adr[4]->value;
  
#ifdef DEBUG
  printf("enter or4h_code\n");
#endif
  new_X = (A | B | C | D);
  assert_output (X, new_X);
}

or8h_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u = adr[0];
  register Net_Entry *X = adr[1];
  register int new_X;
  register int A = adr[2]->value;
  register int B = adr[3]->value;
  register int C = adr[4]->value;
  register int D = adr[5]->value;
  register int E = adr[6]->value;
  register int F = adr[7]->value;
  register int G = adr[8]->value;
  register int H = adr[9]->value;
  
#ifdef DEBUG
  printf("enter or8h_code\n");
#endif
  u->value = A | B | C | D | E;
  new_X = (u->value | F | G | H);
  assert_output (X, new_X);
}

tbuf_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  register int ENB = adr[2]->value;
  
#ifdef DEBUG
  printf("enter tbuf_code\n");
#endif
  if (~(ENB)) {
    new_X = A;
    assert_output (X, new_X);
  }
}

treebuf2_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter treebuf2_code\n");
#endif
  new_X = A;
  assert_output (X, new_X);
}

treebuf3_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *X = adr[0];
  register int new_X;
  register int A = adr[1]->value;
  
#ifdef DEBUG
  printf("enter treebuf3_code\n");
#endif
  new_X = ~A;
  assert_output (X, new_X);
}

static void motoRam(int       words,
		    int       bits,
		    int       *u,
		    Net_Entry **AB,
		    Net_Entry **AA,
		    int       WBA,
		    Net_Entry **DINA,
		    Net_Entry **DOB)
{
  register char *storage;
  register unsigned rdAddr;
  register unsigned wrAddr;
  register int addrBits;
  register int i;

  if (*u == NULL) {
    *u = (int) /* ahem! */ malloc(words*bits*sizeof(char));
    if (*u == NULL) {
      fprintf(stderr, "PANIC! rda%dx%d: malloc returned NULL\n", words, bits);
      exit(1);
    }
  }
  storage = (char *) *u;

  if      (words ==  8) addrBits = 2;
  else if (words == 16) addrBits = 3;
  else if (words == 32) addrBits = 4;
  else {
    fprintf(stderr, "PANIC! rda%dx%d: unsupported word size\n", words, bits);
    exit(1);
  }

  if (WBA == 0) {	/* low true write pulse */
    wrAddr = 0;
    for (i=addrBits; (i>=0); --i)
      wrAddr = (wrAddr << 1) + (AA[i]->value ? 1 : 0);
    for (i=bits-1; (i>=0); --i)
      storage[wrAddr*bits + i] = (DINA[i]->value ? -1 : 0);
  }

  rdAddr = 0;
  for (i=addrBits; i>=0; --i)
    rdAddr = (rdAddr << 1) + (AB[i]->value ? 1 : 0);
  for (i=bits-1; (i>=0); --i) {
    register Net_Entry *z = DOB[i];
    register int new_z =  storage[rdAddr*bits + i];

    assert_output (z, new_z);
  }
}

rda16x18_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u     = adr[0];
  register Net_Entry **DOB  = adr+1;
  register Net_Entry **AA   = adr+19;
  register Net_Entry **AB   = adr+23;
  register Net_Entry **DINA = adr+27;
  register int WBA = adr[45]->value;
  
  motoRam(16, 18, &u->value, AB, AA, WBA, DINA, DOB);
}

rda16x36_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u    = adr[0];
  register Net_Entry **DOB  = adr+1;
  register Net_Entry **AA   = adr+37;
  register Net_Entry **AB   = adr+41;
  register Net_Entry **DINA = adr+45;
  register int WBA = adr[81]->value;

  motoRam(16, 36, &u->value, AB, AA, WBA, DINA, DOB);
}

rda16x9_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u     = adr[0];
  register Net_Entry **DOB  = adr+1;
  register Net_Entry **AA   = adr+10;
  register Net_Entry **AB   = adr+14;
  register Net_Entry **DINA = adr+18;
  register int WBA = adr[27]->value;
  
  motoRam(16, 9, &u->value, AB, AA, WBA, DINA, DOB);
}

rda8x18_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u     = adr[0];
  register Net_Entry **DOB  = adr+1;
  register Net_Entry **AA   = adr+19;
  register Net_Entry **AB   = adr+22;
  register Net_Entry **DINA = adr+25;
  register int WBA = adr[43]->value;
  
  motoRam(8, 18, &u->value, AB, AA, WBA, DINA, DOB);
}

rda8x36_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u     = adr[0];
  register Net_Entry **DOB  = adr+1;
  register Net_Entry **AA   = adr+37;
  register Net_Entry **AB   = adr+40;
  register Net_Entry **DINA = adr+43;
  register int WBA = adr[79]->value;
  
  motoRam(8, 36, &u->value, AB, AA, WBA, DINA, DOB);
}

rda8x72_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u     = adr[0];
  register Net_Entry **DOB  = adr+1;
  register Net_Entry **AA   = adr+73;
  register Net_Entry **AB   = adr+76;
  register Net_Entry **DINA = adr+79;
  register int WBA = adr[151]->value;
  
  motoRam(8, 72, &u->value, AB, AA, WBA, DINA, DOB);
}

rda8x9_code(adr)
     Net_Entry **adr;
{
  register Net_Entry *u     = adr[0];
  register Net_Entry **DOB  = adr+1;
  register Net_Entry **AA   = adr+10;
  register Net_Entry **AB   = adr+13;
  register Net_Entry **DINA = adr+16;
  register int WBA = adr[25]->value;

  motoRam(8, 9, &u->value, AB, AA, WBA, DINA, DOB);
}
