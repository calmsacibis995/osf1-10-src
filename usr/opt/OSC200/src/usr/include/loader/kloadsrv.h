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
 *	@(#)$RCSfile: kloadsrv.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:08:56 $
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

#define	KLS_SERVER_NAME			"kloadsrv"
#define	KLS_SERVER_PATHNAME		"/sbin/kloadsrv"
#define	KLS_SERVER_PID_PATHNAME		"/var/run/kloadsrv.pid"

#define	KLS_MAX_MESSAGE_SIZE		8192

/*
 * Request Message Types
 */
#define	KLS_NULL_REQUEST		0
#define	KLS_LOAD_REQUEST		1
#define	KLS_UNLOAD_REQUEST		2
#define	KLS_ENTRY_REQUEST		3
#define	KLS_LOOKUP_REQUEST		4
#define	KLS_LOOKUP_PACKAGE_REQUEST	5
#define	KLS_NEXT_MODULE_REQUEST		6
#define	KLS_INQ_MODULE_REQUEST		7
#define	KLS_INQ_REGION_REQUEST		8
#define	KLS_UNKNOWN_REQUEST		9
#define	KLS_LAST_REQUEST		(KLS_UNKNOWN_REQUEST)

/*
 * Reply Message Types
 */
#define	KLS_LOAD_REPLY			(KLS_LOAD_REQUEST+KLS_LAST_REQUEST)
#define	KLS_UNLOAD_REPLY		(KLS_UNLOAD_REQUEST+KLS_LAST_REQUEST)
#define	KLS_ENTRY_REPLY			(KLS_ENTRY_REQUEST+KLS_LAST_REQUEST)
#define	KLS_LOOKUP_REPLY		(KLS_LOOKUP_REQUEST+KLS_LAST_REQUEST)
#define	KLS_LOOKUP_PACKAGE_REPLY	(KLS_LOOKUP_PACKAGE_REQUEST+KLS_LAST_REQUEST)
#define	KLS_NEXT_MODULE_REPLY		(KLS_NEXT_MODULE_REQUEST+KLS_LAST_REQUEST)
#define	KLS_INQ_MODULE_REPLY		(KLS_INQ_MODULE_REQUEST+KLS_LAST_REQUEST)
#define	KLS_INQ_REGION_REPLY		(KLS_INQ_REGION_REQUEST+KLS_LAST_REQUEST)
#define	KLS_UNKNOWN_REPLY		(KLS_UNKNOWN_REQUEST+KLS_LAST_REQUEST)

/*
 * Request Header
 */
typedef struct kls_request_header {
	long	klsi_msg_type;
	long	klsi_msg_size;
	int	klsi_reply_key;
} kls_request_header_t;

/*
 * Reply Header
 */
typedef struct kls_reply_header {
	long	klsi_msg_type;
	long	klsi_msg_size;
	int	klsi_return_value;
} kls_reply_header_t;

#define kls_msg_type		kls_header.klsi_msg_type
#define kls_msg_size		kls_header.klsi_msg_size
#define kls_reply_key		kls_header.klsi_reply_key
#define kls_return_value	kls_header.klsi_return_value


/*
 * int
 * ldr_xload(ldr_process_t process, char *file_pathname, 
 *	     ldr_load_flags_t load_flags, ldr_module_t *mod_id_ptr);
 */
typedef struct kls_load_request {
	kls_request_header_t	kls_header;
	ldr_load_flags_t	kls_load_flags;
	char			kls_file_pathname[1];
} kls_load_request_t;

typedef struct kls_load_reply {
	kls_reply_header_t	kls_header;
	ldr_module_t		kls_module;
} kls_load_reply_t;


/*
 * int
 * ldr_xunload(ldr_process_t process, ldr_module_t mod_id);
 */
typedef struct kls_unload_request {
	kls_request_header_t	kls_header;
	ldr_module_t		kls_module;
} kls_unload_request_t;

typedef struct kls_unload_reply {
	kls_reply_header_t	kls_header;
} kls_unload_reply_t;


/*
 * int
 * ldr_xentry(ldr_process_t process, ldr_module_t mod_id,
 *	      ldr_entry_pt_t *entry_ptr);
 */
typedef struct kls_entry_request {
	kls_request_header_t	kls_header;
	ldr_module_t		kls_module;
} kls_entry_request_t;

typedef struct kls_entry_reply {
	kls_reply_header_t	kls_header;
	ldr_entry_pt_t		kls_entry_pt;
} kls_entry_reply_t;


/*
 * int
 * ldr_xlookup(ldr_process_t process, ldr_module_t mod_id,
 *	    char *symbol_name, void **symbol_addr_ptr)
 */
typedef struct kls_lookup_request {
	kls_request_header_t	kls_header;
	ldr_module_t		kls_module;
	char			kls_symbol_name[1];
} kls_lookup_request_t;

typedef struct kls_lookup_reply {
	kls_reply_header_t	kls_header;
	void *			kls_symbol_addr;
} kls_lookup_reply_t;


/*
 * int
 * ldr_xlookup_package(ldr_process_t process, char *package_name,
 *	char *symbol_name, void **symbol_addr_ptr)
 */
typedef struct kls_lookup_package_request {
	kls_request_header_t	kls_header;
	off_t			kls_package_name_offset;
	off_t			kls_symbol_name_offset;
	char			kls_strings[1];
} kls_lookup_package_request_t;

typedef struct kls_lookup_package_reply {
	kls_reply_header_t	kls_header;
	void *			kls_symbol_addr;
} kls_lookup_package_reply_t;


/*
 * int
 * ldr_next_module(ldr_process_t process, ldr_module_t *mod_id_ptr)
 */
typedef struct kls_next_module_request {
	kls_request_header_t	kls_header;
	ldr_module_t		kls_module;
} kls_next_module_request_t;

typedef struct kls_next_module_reply {
	kls_reply_header_t	kls_header;
	ldr_module_t		kls_module;
} kls_next_module_reply_t;


/*
 * int
 * ldr_inq_module(ldr_process_t process, ldr_module_t mod_id,
 *	       ldr_module_info_t *info, size_t info_size, size_t *ret_size)
 */
typedef struct kls_inq_module_request {
	kls_request_header_t	kls_header;
	ldr_module_t		kls_module;
} kls_inq_module_request_t;

typedef struct kls_inq_module_reply {
	kls_reply_header_t	kls_header;
	ldr_module_info_t	kls_info;
	size_t			kls_ret_size;
} kls_inq_module_reply_t;


/*
 * int
 * ldr_inq_region(ldr_process_t process, ldr_module_t mod_id, ldr_region_t region,
 *	       ldr_region_info_t *info, size_t info_size, size_t *ret_size)
 */
typedef struct kls_inq_region_request {
	kls_request_header_t	kls_header;
	ldr_module_t		kls_module;
	ldr_region_t		kls_region;
} kls_inq_region_request_t;

typedef struct kls_inq_region_reply {
	kls_reply_header_t	kls_header;
	ldr_region_info_t	kls_info;
	size_t			kls_ret_size;
} kls_inq_region_reply_t;


/*
 * Unknown Reply
 */
typedef struct kls_unknown_reply {
	kls_reply_header_t	kls_header;
} kls_unknown_reply_t;
