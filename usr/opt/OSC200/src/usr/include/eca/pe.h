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
 * @(#)$RCSfile: pe.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:48:57 $
 */
#ifndef pe_included
#define pe_included

#include "man.h"
#include "man_data.h"
#include "nbase.h"

#define idl_long_int unsigned int

extern int pei_send_get_reply();
extern int pei_send_set_reply();
extern int pei_send_create_reply();
extern int pei_send_delete_reply();
extern int pei_send_action_reply();
extern man_status evd_create_queue_handle();
extern man_status evd_delete_queue_handle();
extern man_status evd_post_event();

typedef struct pe_v1_0_epv_t {
    int (*pei_send_get_reply)();
    int (*pei_send_set_reply)();
    int (*pei_send_create_reply)();
    int (*pei_send_delete_reply)();
    int (*pei_send_action_reply)();
  } pe_v1_0_epv_t;

extern unsigned int *pe_v1_0_c_ifspec;
extern char *pe_v1_0_s_ifspec;

#endif
