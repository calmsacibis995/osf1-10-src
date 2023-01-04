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
static char	*sccsid = "@(#)$RCSfile: kls_message.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:38:44 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * kls_message.c
 *
 * The file implements the functions that package requests and replies
 * into messages.  Here are a few points about this implementation.
 * The only entity that creates requests is the client and the only
 * entity that creates replies is the server.  In general, messages
 * must only be created in order to be sent.  Each client and the
 * server are expected to send and/or receive only one message at a
 * time.  Single static buffers are used to hold the messages.
 */

#include <sys/types.h>
#include <errno.h>
#include <loader.h>

#include <loader/kloadsrv.h>

#include "kls_message.h"

static char  server_send_buffer[KLS_MAX_MESSAGE_SIZE];

extern int errno;

/*
 * load()
 */
int
kls_message_create_load_reply(return_value, module, reply)
	int return_value;
	ldr_module_t module;
	kls_reply_header_t **reply;
{
	kls_load_reply_t *p;

	p = (kls_load_reply_t *)server_send_buffer;
	p->kls_msg_type = KLS_LOAD_REPLY;
	p->kls_msg_size = sizeof(*p);
	p->kls_return_value = return_value;
	p->kls_module = module;
	*reply = (kls_reply_header_t *)p;
	return(0);
}

/*
 * unload()
 */
int
kls_message_create_unload_reply(return_value, reply)
	int return_value;
	kls_reply_header_t **reply;
{
	kls_unload_reply_t *p;

	p = (kls_unload_reply_t *)server_send_buffer;
	p->kls_msg_type = KLS_UNLOAD_REPLY;
	p->kls_msg_size = sizeof(*p);
	p->kls_return_value = return_value;
	*reply = (kls_reply_header_t *)p;
	return(0);
}

/*
 * entry()
 */
int
kls_message_create_entry_reply(return_value, entry_pt, reply)
	int return_value;
	ldr_entry_pt_t entry_pt;
	kls_reply_header_t **reply;
{
	kls_entry_reply_t *p;

	p = (kls_entry_reply_t *)server_send_buffer;
	p->kls_msg_type = KLS_ENTRY_REPLY;
	p->kls_msg_size = sizeof(*p);
	p->kls_return_value = return_value;
	p->kls_entry_pt = entry_pt;
	*reply = (kls_reply_header_t *)p;
	return(0);
}

/*
 * lookup()
 */
int
kls_message_create_lookup_reply(return_value, symbol_addr, reply)
	int return_value;
	void *symbol_addr;
	kls_reply_header_t **reply;
{
	kls_lookup_reply_t *p;

	p = (kls_lookup_reply_t *)server_send_buffer;
	p->kls_msg_type = KLS_LOOKUP_REPLY;
	p->kls_msg_size = sizeof(*p);
	p->kls_return_value = return_value;
	p->kls_symbol_addr = symbol_addr;
	*reply = (kls_reply_header_t *)p;
	return(0);
}

/*
 * lookup_package()
 */
int
kls_message_create_lookup_package_reply(return_value, symbol_addr, reply)
	int return_value;
	void *symbol_addr;
	kls_reply_header_t **reply;
{
	kls_lookup_package_reply_t *p;

	p = (kls_lookup_package_reply_t *)server_send_buffer;
	p->kls_msg_type = KLS_LOOKUP_PACKAGE_REPLY;
	p->kls_msg_size = sizeof(*p);
	p->kls_return_value = return_value;
	p->kls_symbol_addr = symbol_addr;
	*reply = (kls_reply_header_t *)p;
	return(0);
}

/*
 * next_module()
 */
int
kls_message_create_next_module_reply(return_value, module, reply)
	int return_value;
	ldr_module_t module;
	kls_reply_header_t **reply;
{
	kls_next_module_reply_t *p;

	p = (kls_next_module_reply_t *)server_send_buffer;
	p->kls_msg_type = KLS_NEXT_MODULE_REPLY;
	p->kls_msg_size = sizeof(*p);
	p->kls_return_value = return_value;
	p->kls_module = module;
	*reply = (kls_reply_header_t *)p;
	return(0);
}

/*
 * inq_module()
 */
int
kls_message_create_inq_module_reply(return_value, info, reply)
	int return_value;
	ldr_module_info_t *info;
	kls_reply_header_t **reply;
{
	kls_inq_module_reply_t *p;
	long size;

	size = 	sizeof(*p);
	if (size > KLS_MAX_MESSAGE_SIZE) {
		errno = EMSGSIZE;
		return(-errno);
	}
	p = (kls_inq_module_reply_t *)server_send_buffer;
	p->kls_msg_type = KLS_INQ_MODULE_REPLY;
	p->kls_msg_size = size;
	p->kls_return_value = return_value;
	p->kls_info = *info;
	p->kls_ret_size = sizeof(*info);
	*reply = (kls_reply_header_t *)p;
	return(0);
}

/*
 * inq_region()
 */
int
kls_message_create_inq_region_reply(return_value, info, reply)
	int return_value;
	ldr_region_info_t *info;
	kls_reply_header_t **reply;
{
	kls_inq_region_reply_t *p;
	long size;

	size = 	sizeof(*p);
	if (size > KLS_MAX_MESSAGE_SIZE) {
		errno = EMSGSIZE;
		return(-errno);
	}
	p = (kls_inq_region_reply_t *)server_send_buffer;
	p->kls_msg_type = KLS_INQ_REGION_REPLY;
	p->kls_msg_size = size;
	p->kls_return_value = return_value;
	p->kls_info = *info;
	p->kls_ret_size = sizeof(*info);
	*reply = (kls_reply_header_t *)p;
	return(0);
}

/*
 * unknown()
 */
int
kls_message_create_unknown_reply(return_value, reply)
	int return_value;
	kls_reply_header_t **reply;
{
	kls_unknown_reply_t *p;

	p = (kls_unknown_reply_t *)server_send_buffer;
	p->kls_msg_type = KLS_UNKNOWN_REPLY;
	p->kls_msg_size = sizeof(*p);
	p->kls_return_value = return_value;
	*reply = (kls_reply_header_t *)p;
	return(0);
}
