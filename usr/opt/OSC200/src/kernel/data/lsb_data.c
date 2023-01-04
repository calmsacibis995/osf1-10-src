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
static char     *sccsid = "@(#)lsb_data.c	9.3  (ULTRIX/OSF)    11/18/91";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

#include <io/dec/lsb/lsbreg.h>

/*
 * Modification History: ./dec/data/lsb_data.c
 *
 * Sep-91	jac: created this file for lsb support of Laser/Ruby
 *
 */

#ifdef ALPHARUBY
int lsb_iopint();
int lsb_memint();
int lsb_mplepint();
#else
#define lsb_iopint lsbnotconf
#define lsb_memint lsbnotconf
#define lsb_mplepint lsbnotconf
#endif /* ALPHARUBY */

int
nolsbreset()
{
return(0);
}

int
lsb_iopint()
{
return(0);
}

int (*lsb_iopprobes[])() = {  lsb_iopint, 0};
int (*lsb_memprobes[])() = {  lsb_memint, 0};
int (*lsb_lepprobes[])() = {  lsb_mplepint, 0};

struct lsbsw lsbsw [] =
{
  { LSB_IOP,      "iop",          lsb_iopprobes,     nolsbreset,
          LSBF_NOCONF},
  { LSB_MEM,      "mem",          lsb_memprobes ,     nolsbreset,
          LSBF_NOCONF},
  { LSB_BBMEM,      "ms7bb",          lsb_memprobes ,     nolsbreset,
          LSBF_NOCONF},
  { LSB_LEP,      "lep",          lsb_lepprobes ,     nolsbreset,
          LSBF_NOCONF},
  { 0 }
};

int nlsbtypes = sizeof (lsbsw) / sizeof (lsbsw[0]);

