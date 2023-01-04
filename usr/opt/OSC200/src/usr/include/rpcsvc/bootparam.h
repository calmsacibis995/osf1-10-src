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
 *	@(#)$RCSfile: bootparam.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/19 17:45:55 $
 */ 
/*
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 */

#ifndef _rpcsvc_bootparam_h
#define _rpcsvc_bootparam_h

#ifndef KERNEL
#include <rpc/types.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/mount.h>
/* #include <nfs/nfsv2.h> */
#include <nfs/nfs.h>
#endif
#define	MAX_MACHINE_NAME 255
#define	MAX_PATH_LEN	1024
#define	MAX_FILEID	32
#define	IP_ADDR_TYPE	1

typedef char *bp_machine_name_t;


typedef char *bp_path_t;


typedef char *bp_fileid_t;


struct ip_addr_t {
	char net;
	char host;
	char lh;
	char impno;
};
typedef struct ip_addr_t ip_addr_t;


struct bp_address {
	int address_type;
	union {
		ip_addr_t ip_addr;
	} bp_address;
};
typedef struct bp_address bp_address;


struct bp_whoami_arg {
	bp_address client_address;
};
typedef struct bp_whoami_arg bp_whoami_arg;


struct bp_whoami_res {
	bp_machine_name_t client_name;
	bp_machine_name_t domain_name;
	bp_address router_address;
};
typedef struct bp_whoami_res bp_whoami_res;


struct bp_getfile_arg {
	bp_machine_name_t client_name;
	bp_fileid_t file_id;
};
typedef struct bp_getfile_arg bp_getfile_arg;


struct bp_getfile_res {
	bp_machine_name_t server_name;
	bp_address server_address;
	bp_path_t server_path;
};
typedef struct bp_getfile_res bp_getfile_res;


#define BOOTPARAMPROG 100026
#define BOOTPARAMVERS 1
#define BOOTPARAMPROC_WHOAMI 1
#define BOOTPARAMPROC_GETFILE 2

bool_t xdr_bp_machine_name_t();
bool_t xdr_bp_path_t();
bool_t xdr_bp_fileid_t();
bool_t xdr_ip_addr_t();
bool_t xdr_bp_address();
bool_t xdr_bp_whoami_arg();
bool_t xdr_bp_whoami_res();
bool_t xdr_bp_getfile_arg();
bool_t xdr_bp_getfile_res();

#endif /*!_rpcsvc_bootparam_h*/
