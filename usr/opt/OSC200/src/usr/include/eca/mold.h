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
/*
 * @(#)$RCSfile: mold.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:02:01 $
 */
#ifndef mold_v1_0_included
#define mold_v1_0_included

#include "man_data.h"
#include "nbase.h"

extern unsigned int mold_register_mom();
extern unsigned int mold_deregister_mom();
extern unsigned int mold_find_mom();
extern unsigned int mold_find_next_mom();

typedef struct mold_v1_0_epv_t {
    unsigned int (*mold_register_mom)();
    unsigned int (*mold_deregister_mom)();
    unsigned int (*mold_find_mom)();
    unsigned int (*mold_find_next_mom)();
  } mold_v1_0_epv_t;

extern unsigned int *mold_v1_0_c_ifspec;
extern char *mold_v1_0_s_ifspec;

#endif
