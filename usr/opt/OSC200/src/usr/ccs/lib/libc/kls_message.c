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
static char	*sccsid = "@(#)$RCSfile: kls_message.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/05 21:02:54 $";
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

/*
 * kls_message.c
 *
 * The file implements the functions that package requests
 * into messages.  Here are a few points about this implementation.
 * The only entity that creates requests is the client and the only
 * entity that creates replies is the server.  In general, messages
 * must only be created in order to be sent.  Each client and the
 * server are expected to send and/or receive only one message at a
 * time.  Single static buffers are used to hold the messages.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak kls_message_create_entry_request = __kls_message_create_entry_request
#pragma weak kls_message_create_inq_module_request = __kls_message_create_inq_module_request
#pragma weak kls_message_create_inq_region_request = __kls_message_create_inq_region_request
#pragma weak kls_message_create_load_request = __kls_message_create_load_request
#pragma weak kls_message_create_lookup_package_request = __kls_message_create_lookup_package_request
#pragma weak kls_message_create_lookup_request = __kls_message_create_lookup_request
#pragma weak kls_message_create_next_module_request = __kls_message_create_next_module_request
#pragma weak kls_message_create_unload_request = __kls_message_create_unload_request
#endif
#endif
#include <sys/types.h>
#include <errno.h>
#include <loader.h>

#include <loader/kloadsrv.h>
#include <loader/kls_message.h>

#include <malloc.h>
#include <assert.h>

static char  *client_send_buffer = NULL;

extern int errno;

/*
 * load()
 */
int
kls_message_create_load_request(file_pathname, load_flags, request)
	char *file_pathname;
	ldr_load_flags_t load_flags;
	kls_request_header_t **request;
{
	kls_load_request_t *p;
	long size;

	size = 	p->kls_file_pathname - ((char *)p) + strlen(file_pathname) + 1;
	if (size > KLS_MAX_MESSAGE_SIZE) {
		errno = EMSGSIZE;
		return(-errno);
	}
	if(client_send_buffer == NULL) {
                client_send_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_send_buffer);
        }
	p = (kls_load_request_t *)client_send_buffer;
	p->kls_msg_type = KLS_LOAD_REQUEST;
	p->kls_msg_size = size;
	p->kls_load_flags = load_flags;
	(void)strcpy(p->kls_file_pathname, file_pathname);
	*request = (kls_request_header_t *)p;
	return(0);
}

/*
 * unload()
 */
int
kls_message_create_unload_request(module, request)
	ldr_module_t module;
	kls_request_header_t **request;
{
	kls_unload_request_t *p;

	if(client_send_buffer == NULL) {
                client_send_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_send_buffer);
        }
	p = (kls_unload_request_t *)client_send_buffer;
	p->kls_msg_type = KLS_UNLOAD_REQUEST;
	p->kls_msg_size = sizeof(*p);
	p->kls_module = module;
	*request = (kls_request_header_t *)p;
	return(0);
}

/*
 * entry()
 */
int
kls_message_create_entry_request(module, request)
	ldr_module_t module;
	kls_request_header_t **request;
{
	kls_entry_request_t *p;

	if(client_send_buffer == NULL) {
                client_send_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_send_buffer);
        }
	p = (kls_entry_request_t *)client_send_buffer;
	p->kls_msg_type = KLS_ENTRY_REQUEST;
	p->kls_msg_size = sizeof(*p);
	p->kls_module = module;
	*request = (kls_request_header_t *)p;
	return(0);
}

/*
 * lookup()
 */
int
kls_message_create_lookup_request(module, symbol_name, request)
	ldr_module_t module;
	char *symbol_name;
	kls_request_header_t **request;
{
	kls_lookup_request_t *p;
	long size;

	size = 	p->kls_symbol_name - ((char *)p) + strlen(symbol_name) + 1;
	if (size > KLS_MAX_MESSAGE_SIZE) {
		errno = EMSGSIZE;
		return(-errno);
	}
	if(client_send_buffer == NULL) {
                client_send_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_send_buffer);
        }
	p = (kls_lookup_request_t *)client_send_buffer;
	p->kls_msg_type = KLS_LOOKUP_REQUEST;
	p->kls_msg_size = size;
	p->kls_module = module;
	(void)strcpy(p->kls_symbol_name, symbol_name);
	*request = (kls_request_header_t *)p;
	return(0);
}

/*
 * lookup_package()
 */
int
kls_message_create_lookup_package_request(package_name, symbol_name, request)
	char *package_name;
	char *symbol_name;
	kls_request_header_t **request;
{
	kls_lookup_package_request_t *p;
	long lpn, lsn;
	long size;

	lpn = strlen(package_name) + 1;
	lsn = strlen(symbol_name) + 1;
	size = 	p->kls_strings - ((char *)p) + lpn + lsn;
	if (size > KLS_MAX_MESSAGE_SIZE) {
		errno = EMSGSIZE;
		return(-errno);
	}
	if(client_send_buffer == NULL) {
                client_send_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_send_buffer);
        }
	p = (kls_lookup_package_request_t *)client_send_buffer;
	p->kls_msg_type = KLS_LOOKUP_PACKAGE_REQUEST;
	p->kls_msg_size = size;
	p->kls_package_name_offset = 0;
	p->kls_symbol_name_offset = p->kls_package_name_offset + lpn;
	(void)strcpy(&p->kls_strings[p->kls_package_name_offset], package_name);
	(void)strcpy(&p->kls_strings[p->kls_symbol_name_offset], symbol_name);
	*request = (kls_request_header_t *)p;
	return(0);
}

/*
 * next_module()
 */
int
kls_message_create_next_module_request(module, request)
	ldr_module_t module;
	kls_request_header_t **request;
{
	kls_next_module_request_t *p;

	if(client_send_buffer == NULL) {
                client_send_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_send_buffer);
        }
	p = (kls_next_module_request_t *)client_send_buffer;
	p->kls_msg_type = KLS_NEXT_MODULE_REQUEST;
	p->kls_msg_size = sizeof(*p);
	p->kls_module = module;
	*request = (kls_request_header_t *)p;
	return(0);
}

/*
 * inq_module()
 */
int
kls_message_create_inq_module_request(module, request)
	ldr_module_t module;
	kls_request_header_t **request;
{
	kls_inq_module_request_t *p;

	if(client_send_buffer == NULL) {
                client_send_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_send_buffer);
        }
	p = (kls_inq_module_request_t *)client_send_buffer;
	p->kls_msg_type = KLS_INQ_MODULE_REQUEST;
	p->kls_msg_size = sizeof(*p);
	p->kls_module = module;
	*request = (kls_request_header_t *)p;
	return(0);
}

/*
 * inq_region()
 */
int
kls_message_create_inq_region_request(module, region, request)
	ldr_module_t module;
	kls_request_header_t **request;
{
	kls_inq_region_request_t *p;

	if(client_send_buffer == NULL) {
                client_send_buffer = (char *)malloc(KLS_MAX_MESSAGE_SIZE);
                assert(client_send_buffer);
        }
	p = (kls_inq_region_request_t *)client_send_buffer;
	p->kls_msg_type = KLS_INQ_REGION_REQUEST;
	p->kls_msg_size = sizeof(*p);
	p->kls_module = module;
	p->kls_region = region;
	*request = (kls_request_header_t *)p;
	return(0);
}
