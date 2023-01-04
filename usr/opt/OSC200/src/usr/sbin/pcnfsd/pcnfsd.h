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
 * @(#)$RCSfile: pcnfsd.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1992/09/29 14:19:07 $
 */
/* RE_SID: @(%)/tmp_mnt/vol/dosnfs/shades_SCCS/unix/pcnfsd/v2/src/SCCS/s.pcnfsd.h 1.2 92/08/18 12:54:43 SMI */
/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <rpc/types.h>

#define IDENTLEN 32
#define PASSWORDLEN 64
#define CLIENTLEN 64
#define PRINTERNAMELEN 64
#define USERNAMELEN 64
#define SPOOLNAMELEN 64
#define OPTIONSLEN 64
#define SPOOLDIRLEN 255
#define EXTRAGIDLEN 16
#define HOMEDIRLEN 255
#define COMMENTLEN 255
#define PRINTJOBIDLEN 255
#define PRLISTMAX 32
#define PRQUEUEMAX 128
#define FACILITIESMAX 32
#define MESSAGELEN 512

typedef char *ident;
bool_t xdr_ident();

typedef char *message;
bool_t xdr_message();

typedef char *password;
bool_t xdr_password();

typedef char *client;
bool_t xdr_client();

typedef char *printername;
bool_t xdr_printername();

typedef char *username;
bool_t xdr_username();

typedef char *comment;
bool_t xdr_comment();

typedef char *spoolname;
bool_t xdr_spoolname();

typedef char *printjobid;
bool_t xdr_printjobid();

typedef char *homedir;
bool_t xdr_homedir();

typedef char *options;
bool_t xdr_options();

enum arstat {
	AUTH_RES_OK = 0,
	AUTH_RES_FAKE = 1,
	AUTH_RES_FAIL = 2,
};
typedef enum arstat arstat;
bool_t xdr_arstat();

enum alrstat {
	ALERT_RES_OK = 0,
	ALERT_RES_FAIL = 1,
};
typedef enum alrstat alrstat;
bool_t xdr_alrstat();

enum pirstat {
	PI_RES_OK = 0,
	PI_RES_NO_SUCH_PRINTER = 1,
	PI_RES_FAIL = 2,
};
typedef enum pirstat pirstat;
bool_t xdr_pirstat();

enum pcrstat {
	PC_RES_OK = 0,
	PC_RES_NO_SUCH_PRINTER = 1,
	PC_RES_NO_SUCH_JOB = 2,
	PC_RES_NOT_OWNER = 3,
	PC_RES_FAIL = 4,
};
typedef enum pcrstat pcrstat;
bool_t xdr_pcrstat();

enum psrstat {
	PS_RES_OK = 0,
	PS_RES_ALREADY = 1,
	PS_RES_NULL = 2,
	PS_RES_NO_FILE = 3,
	PS_RES_FAIL = 4,
};
typedef enum psrstat psrstat;
bool_t xdr_psrstat();

enum mapreq {
	MAP_REQ_UID = 0,
	MAP_REQ_GID = 1,
	MAP_REQ_UNAME = 2,
	MAP_REQ_GNAME = 3,
};
typedef enum mapreq mapreq;
bool_t xdr_mapreq();

enum maprstat {
	MAP_RES_OK = 0,
	MAP_RES_UNKNOWN = 1,
	MAP_RES_DENIED = 2,
};
typedef enum maprstat maprstat;
bool_t xdr_maprstat();

struct auth_args {
	ident id;
	password pw;
};
typedef struct auth_args auth_args;
bool_t xdr_auth_args();

struct auth_results {
	arstat stat;
	u_int uid;
	u_int gid;
};
typedef struct auth_results auth_results;
bool_t xdr_auth_results();

struct pr_init_args {
	client system;
	printername pn;
};
typedef struct pr_init_args pr_init_args;
bool_t xdr_pr_init_args();

struct pr_init_results {
	pirstat stat;
	spoolname dir;
};
typedef struct pr_init_results pr_init_results;
bool_t xdr_pr_init_results();

struct pr_start_args {
	client system;
	printername pn;
	username user;
	spoolname file;
	options opts;
};
typedef struct pr_start_args pr_start_args;
bool_t xdr_pr_start_args();

struct pr_start_results {
	psrstat stat;
};
typedef struct pr_start_results pr_start_results;
bool_t xdr_pr_start_results();

struct v2_info_args {
	comment vers;
	comment cm;
};
typedef struct v2_info_args v2_info_args;
bool_t xdr_v2_info_args();

struct v2_info_results {
	comment vers;
	comment cm;
	struct {
		u_int facilities_len;
		int *facilities_val;
	} facilities;
};
typedef struct v2_info_results v2_info_results;
bool_t xdr_v2_info_results();

struct v2_pr_init_args {
	client system;
	printername pn;
	comment cm;
};
typedef struct v2_pr_init_args v2_pr_init_args;
bool_t xdr_v2_pr_init_args();

struct v2_pr_init_results {
	pirstat stat;
	spoolname dir;
	comment cm;
};
typedef struct v2_pr_init_results v2_pr_init_results;
bool_t xdr_v2_pr_init_results();

struct v2_pr_start_args {
	client system;
	printername pn;
	username user;
	spoolname file;
	options opts;
	int copies;
	comment cm;
};
typedef struct v2_pr_start_args v2_pr_start_args;
bool_t xdr_v2_pr_start_args();

struct v2_pr_start_results {
	psrstat stat;
	printjobid id;
	comment cm;
};
typedef struct v2_pr_start_results v2_pr_start_results;
bool_t xdr_v2_pr_start_results();

typedef struct pr_list_item *pr_list;
bool_t xdr_pr_list();

struct pr_list_item {
	printername pn;
	printername device;
	client remhost;
	comment cm;
	pr_list pr_next;
};
typedef struct pr_list_item pr_list_item;
bool_t xdr_pr_list_item();

struct v2_pr_list_results {
	comment cm;
	pr_list printers;
};
typedef struct v2_pr_list_results v2_pr_list_results;
bool_t xdr_v2_pr_list_results();

struct v2_pr_queue_args {
	printername pn;
	client system;
	username user;
	bool_t just_mine;
	comment cm;
};
typedef struct v2_pr_queue_args v2_pr_queue_args;
bool_t xdr_v2_pr_queue_args();

typedef struct pr_queue_item *pr_queue;
bool_t xdr_pr_queue();

struct pr_queue_item {
	int position;
	printjobid id;
	comment size;
	comment status;
	client system;
	username user;
	spoolname file;
	comment cm;
	pr_queue pr_next;
};
typedef struct pr_queue_item pr_queue_item;
bool_t xdr_pr_queue_item();

struct v2_pr_queue_results {
	pirstat stat;
	comment cm;
	bool_t just_yours;
	int qlen;
	int qshown;
	pr_queue jobs;
};
typedef struct v2_pr_queue_results v2_pr_queue_results;
bool_t xdr_v2_pr_queue_results();

struct v2_pr_cancel_args {
	printername pn;
	client system;
	username user;
	printjobid id;
	comment cm;
};
typedef struct v2_pr_cancel_args v2_pr_cancel_args;
bool_t xdr_v2_pr_cancel_args();

struct v2_pr_cancel_results {
	pcrstat stat;
	comment cm;
};
typedef struct v2_pr_cancel_results v2_pr_cancel_results;
bool_t xdr_v2_pr_cancel_results();

struct v2_pr_status_args {
	printername pn;
	comment cm;
};
typedef struct v2_pr_status_args v2_pr_status_args;
bool_t xdr_v2_pr_status_args();

struct v2_pr_status_results {
	pirstat stat;
	bool_t avail;
	bool_t printing;
	int qlen;
	bool_t needs_operator;
	comment status;
	comment cm;
};
typedef struct v2_pr_status_results v2_pr_status_results;
bool_t xdr_v2_pr_status_results();

struct v2_pr_admin_args {
	client system;
	username user;
	printername pn;
	comment cm;
};
typedef struct v2_pr_admin_args v2_pr_admin_args;
bool_t xdr_v2_pr_admin_args();

struct v2_pr_admin_results {
	pirstat stat;
	comment cm;
};
typedef struct v2_pr_admin_results v2_pr_admin_results;
bool_t xdr_v2_pr_admin_results();

struct v2_pr_requeue_args {
	printername pn;
	client system;
	username user;
	printjobid id;
	int qpos;
	comment cm;
};
typedef struct v2_pr_requeue_args v2_pr_requeue_args;
bool_t xdr_v2_pr_requeue_args();

struct v2_pr_requeue_results {
	pcrstat stat;
	comment cm;
};
typedef struct v2_pr_requeue_results v2_pr_requeue_results;
bool_t xdr_v2_pr_requeue_results();

struct v2_pr_hold_args {
	printername pn;
	client system;
	username user;
	printjobid id;
	comment cm;
};
typedef struct v2_pr_hold_args v2_pr_hold_args;
bool_t xdr_v2_pr_hold_args();

struct v2_pr_hold_results {
	pcrstat stat;
	comment cm;
};
typedef struct v2_pr_hold_results v2_pr_hold_results;
bool_t xdr_v2_pr_hold_results();

struct v2_pr_release_args {
	printername pn;
	client system;
	username user;
	printjobid id;
	comment cm;
};
typedef struct v2_pr_release_args v2_pr_release_args;
bool_t xdr_v2_pr_release_args();

struct v2_pr_release_results {
	pcrstat stat;
	comment cm;
};
typedef struct v2_pr_release_results v2_pr_release_results;
bool_t xdr_v2_pr_release_results();

typedef struct mapreq_arg_item *mapreq_arg;
bool_t xdr_mapreq_arg();

struct mapreq_arg_item {
	mapreq req;
	int id;
	username name;
	mapreq_arg mapreq_next;
};
typedef struct mapreq_arg_item mapreq_arg_item;
bool_t xdr_mapreq_arg_item();

typedef struct mapreq_res_item *mapreq_res;
bool_t xdr_mapreq_res();

struct mapreq_res_item {
	mapreq req;
	maprstat stat;
	int id;
	username name;
	mapreq_res mapreq_next;
};
typedef struct mapreq_res_item mapreq_res_item;
bool_t xdr_mapreq_res_item();

struct v2_mapid_args {
	comment cm;
	mapreq_arg req_list;
};
typedef struct v2_mapid_args v2_mapid_args;
bool_t xdr_v2_mapid_args();

struct v2_mapid_results {
	comment cm;
	mapreq_res res_list;
};
typedef struct v2_mapid_results v2_mapid_results;
bool_t xdr_v2_mapid_results();

struct v2_auth_args {
	client system;
	ident id;
	password pw;
	comment cm;
};
typedef struct v2_auth_args v2_auth_args;
bool_t xdr_v2_auth_args();

struct v2_auth_results {
	arstat stat;
	u_int uid;
	u_int gid;
	struct {
		u_int gids_len;
		u_int *gids_val;
	} gids;
	homedir home;
	int def_umask;
	comment cm;
};
typedef struct v2_auth_results v2_auth_results;
bool_t xdr_v2_auth_results();

struct v2_alert_args {
	client system;
	printername pn;
	username user;
	message msg;
};
typedef struct v2_alert_args v2_alert_args;
bool_t xdr_v2_alert_args();

struct v2_alert_results {
	alrstat stat;
	comment cm;
};
typedef struct v2_alert_results v2_alert_results;
bool_t xdr_v2_alert_results();

#define PCNFSDPROG ((u_int)150001)
#define PCNFSDVERS ((u_int)1)
#define PCNFSD_NULL ((u_int)0)
extern void *pcnfsd_null_1();
#define PCNFSD_AUTH ((u_int)1)
extern auth_results *pcnfsd_auth_1();
#define PCNFSD_PR_INIT ((u_int)2)
extern pr_init_results *pcnfsd_pr_init_1();
#define PCNFSD_PR_START ((u_int)3)
extern pr_start_results *pcnfsd_pr_start_1();
#define PCNFSDV2 ((u_int)2)
#define PCNFSD2_NULL ((u_int)0)
extern void *pcnfsd2_null_2();
#define PCNFSD2_INFO ((u_int)1)
extern v2_info_results *pcnfsd2_info_2();
#define PCNFSD2_PR_INIT ((u_int)2)
extern v2_pr_init_results *pcnfsd2_pr_init_2();
#define PCNFSD2_PR_START ((u_int)3)
extern v2_pr_start_results *pcnfsd2_pr_start_2();
#define PCNFSD2_PR_LIST ((u_int)4)
extern v2_pr_list_results *pcnfsd2_pr_list_2();
#define PCNFSD2_PR_QUEUE ((u_int)5)
extern v2_pr_queue_results *pcnfsd2_pr_queue_2();
#define PCNFSD2_PR_STATUS ((u_int)6)
extern v2_pr_status_results *pcnfsd2_pr_status_2();
#define PCNFSD2_PR_CANCEL ((u_int)7)
extern v2_pr_cancel_results *pcnfsd2_pr_cancel_2();
#define PCNFSD2_PR_ADMIN ((u_int)8)
extern v2_pr_admin_results *pcnfsd2_pr_admin_2();
#define PCNFSD2_PR_REQUEUE ((u_int)9)
extern v2_pr_requeue_results *pcnfsd2_pr_requeue_2();
#define PCNFSD2_PR_HOLD ((u_int)10)
extern v2_pr_hold_results *pcnfsd2_pr_hold_2();
#define PCNFSD2_PR_RELEASE ((u_int)11)
extern v2_pr_release_results *pcnfsd2_pr_release_2();
#define PCNFSD2_MAPID ((u_int)12)
extern v2_mapid_results *pcnfsd2_mapid_2();
#define PCNFSD2_AUTH ((u_int)13)
extern v2_auth_results *pcnfsd2_auth_2();
#define PCNFSD2_ALERT ((u_int)14)
extern v2_alert_results *pcnfsd2_alert_2();
#if RPC_SVC
 void msg_out(msg) char *msg; {_msgout(msg);}
#endif
#if 1
 extern void msg_out();
#endif