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
static char	*sccsid = "@(#)$RCSfile: kls_client.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:22:02 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak kls_client_entry = __kls_client_entry
#pragma weak kls_client_inq_module = __kls_client_inq_module
#pragma weak kls_client_inq_region = __kls_client_inq_region
#pragma weak kls_client_load = __kls_client_load
#pragma weak kls_client_lookup = __kls_client_lookup
#pragma weak kls_client_lookup_package = __kls_client_lookup_package
#pragma weak kls_client_next_module = __kls_client_next_module
#pragma weak kls_client_unload = __kls_client_unload
#endif
#endif
#include <sys/types.h>
#include <loader.h>

#include <loader/kloadsrv.h>
#include <loader/kls_ipc.h>
#include <loader/kls_message.h>

int
kls_client_load(file_pathname, load_flags, module)
	char *file_pathname;
	ldr_load_flags_t load_flags;
	ldr_module_t *module;
{
	kls_request_header_t *request;
	kls_load_reply_t *reply;
	int rc;

	if ((rc = kls_message_create_load_request(file_pathname, load_flags,
	    &request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_send_request(request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_receive_reply((kls_reply_header_t **)&reply)) < 0)
		return(rc);
	*module = reply->kls_module;
	return(reply->kls_return_value);
}

int
kls_client_unload(module)
	ldr_module_t *module;
{
	kls_request_header_t *request;
	kls_unload_reply_t *reply;
	int rc;

	if ((rc = kls_message_create_unload_request(module, &request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_send_request(request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_receive_reply((kls_reply_header_t **)&reply)) < 0)
		return(rc);
	return(reply->kls_return_value);
}

int
kls_client_entry(module, entry_pt)
	ldr_module_t module;
	ldr_entry_pt_t *entry_pt;
{
	kls_request_header_t *request;
	kls_entry_reply_t *reply;
	int rc;

	if ((rc = kls_message_create_entry_request(module, &request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_send_request(request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_receive_reply((kls_reply_header_t **)&reply)) < 0)
		return(rc);
	*entry_pt = reply->kls_entry_pt;
	return(reply->kls_return_value);
}

int
kls_client_lookup(module, symbol_name, symbol_addr_ptr)
	ldr_module_t *module;
	char *symbol_name;
	void **symbol_addr_ptr;
{
	kls_request_header_t *request;
	kls_lookup_reply_t *reply;
	int rc;

	if ((rc = kls_message_create_lookup_request(module, symbol_name,
	    &request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_send_request(request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_receive_reply((kls_reply_header_t **)&reply)) < 0)
		return(rc);
	*symbol_addr_ptr = reply->kls_symbol_addr;
	return(reply->kls_return_value);
}

int
kls_client_lookup_package(package_name, symbol_name, symbol_addr_ptr)
	char *package_name;
	char *symbol_name;
	void **symbol_addr_ptr;
{
	kls_request_header_t *request;
	kls_lookup_package_reply_t *reply;
	int rc;

	if ((rc = kls_message_create_lookup_package_request(package_name, symbol_name,
	    &request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_send_request(request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_receive_reply((kls_reply_header_t **)&reply)) < 0)
		return(rc);
	*symbol_addr_ptr = reply->kls_symbol_addr;
	return(reply->kls_return_value);
}

int
kls_client_next_module(module)
	ldr_module_t *module;
{
	kls_request_header_t *request;
	kls_next_module_reply_t *reply;
	int rc;

	if ((rc = kls_message_create_next_module_request(*module, &request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_send_request(request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_receive_reply((kls_reply_header_t **)&reply)) < 0)
		return(rc);
	*module = reply->kls_module;
	return(reply->kls_return_value);
}

int
kls_client_inq_module(module, info, info_size, ret_size)
	ldr_module_t module;
	ldr_module_info_t *info;
	size_t info_size;
	size_t *ret_size;
{
	kls_request_header_t *request;
	kls_inq_module_reply_t *reply;
	int rc;

	if ((rc = kls_message_create_inq_module_request(module, &request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_send_request(request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_receive_reply((kls_reply_header_t **)&reply)) < 0)
		return(rc);
	(void)memcpy((void *)info, (void *)&reply->kls_info, info_size);
	*ret_size = reply->kls_ret_size;
	return(reply->kls_return_value);
}

int
kls_client_inq_region(module, region, info, info_size, ret_size)
	ldr_module_t module;
	ldr_region_t region;
	ldr_region_info_t *info;
	size_t info_size;
	size_t *ret_size;
{
	kls_request_header_t *request;
	kls_inq_region_reply_t *reply;
	int rc;

	if ((rc = kls_message_create_inq_region_request(module, region,
	    &request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_send_request(request)) < 0)
		return(rc);
	if ((rc = kls_client_ipc_receive_reply((kls_reply_header_t **)&reply)) < 0)
		return(rc);
	(void)memcpy((void *)info, (void *)&reply->kls_info, info_size);
	*ret_size = reply->kls_ret_size;
	return(reply->kls_return_value);
}
