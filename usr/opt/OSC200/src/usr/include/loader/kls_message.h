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
 *	@(#)$RCSfile: kls_message.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:09:02 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _NO_PROTO

extern int
kls_message_create_kls_load_request(char *, ldr_load_flags_t, kls_request_header_t **);

extern int
kls_message_create_kls_unload_request(ldr_module_t, kls_request_header_t **);

extern int
kls_message_create_kls_entry_request(ldr_module_t, kls_request_header_t **);

extern int
kls_message_create_kls_lookup_request(ldr_module_t, char *, kls_request_header_t **);

extern int
kls_message_create_lookup_package_request(char *, char *, kls_request_header_t **);

extern int
kls_message_create_kls_next_module_request(ldr_module_t, kls_request_header_t **);

extern int
kls_message_create_kls_inq_region_request(ldr_module_t, ldr_region_t, kls_request_header_t **);

#else

extern int
kls_message_create_kls_load_request();

extern int
kls_message_create_kls_unload_request();

extern int
kls_message_create_kls_entry_request();

extern int
kls_message_create_kls_lookup_request();

extern int
kls_message_create_lookup_package_request();

extern int
kls_message_create_kls_next_module_request();

extern int
kls_message_create_kls_inq_region_request();

#endif
