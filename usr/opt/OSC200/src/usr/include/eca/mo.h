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
 * @(#)$RCSfile: mo.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:01:50 $
 */
#ifndef mo_v1_0_included
#define mo_v1_0_included

#include "man_data.h"
#include "nbase.h"

extern unsigned int moi_get_attributes();
extern unsigned int moi_set_attributes();
extern unsigned int moi_create_instance();
extern unsigned int moi_delete_instance();
extern unsigned int moi_invoke_action();

extern char *mo_v1_0_s_ifspec;

typedef struct mo_v1_0_epv_t {
    unsigned int (*moi_get_attributes)();
    unsigned int (*moi_set_attributes)();
    unsigned int (*moi_create_instance)();
    unsigned int (*moi_delete_instance)();
    unsigned int (*moi_invoke_action)();
  } mo_v1_0_epv_t;

#endif
