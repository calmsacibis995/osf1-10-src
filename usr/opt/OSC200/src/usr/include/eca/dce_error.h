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
 * @(#)$RCSfile: dce_error.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:00:27 $
 */
/* Constants */

#define dce_c_error_string_len 160 /* length of returned string */

/* Typedefs */

typedef unsigned char dce_error_string_t[dce_c_error_string_len];

/* Prototypes */

extern void dce_error_inq_text (
    unsigned int             status_to_convert,
    unsigned char           *error_text,
    int                     *status
);

#define rpc_s_ok error_status_ok

extern EXCEPTION rpc_x_comm_failure; 
