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
 * @(#)$RCSfile: mom_defs.h,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/07/13 22:19:24 $
 */
/*
**++
**  FACILITY:	MOMGEN
**
**  MODULE DESCRIPTION:
**
**	 This header file for the MOMGEN builder containing the structure 
**	 definitions and some miscellaneous strings.
**
**  AUTHORS:
**
**	Rich Bouchard
**
**  CREATION DATE:  18-March-1991
**
**  MODIFICATION HISTORY:
**
**   18-May-1992   Mike Densmore	Modified for use on U**X platforms
**				        see code ifdef'd by "UNIXPLATFORM"
**
**	 3-Mar-1993	M. Ashraf	Updated for IPC changes
**
**--
*/

/* MOM Generator Version */

#define builder_version "V1.1"

#ifndef UNIXPLATFORM
#define system_default "VMS"
#define signed_int
#else
#define system_default "ULTRIX"
#if defined(sun) || defined(sparc)
#define signed_int
#else
#define signed_int signed
#endif
#endif

#include "types_setup.h"

#ifdef MCC
#include "mcc_interface_def.h"
#endif

#ifndef UNIXPLATFORM
#include "resobj$:sme_common.h"
#include "ssdef.h"
#include "libdef.h"
#include "rmsdef.h"
#include "climsgdef.h"
#else
#include "mg_common.h"
#endif

#include "stdio.h"

#ifndef UNIXPLATFORM
#include "file.h"
#endif

#include "man_data.h"
#include "man_status.h"


#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

#define return_on_error( cond ) if (((cond) & 1) == 0) \
				   return (cond);

#define DNA_CMIP_OID		1
#define OSI_CMIP_OID		2
#define SNMP_OID		3
#define OID			4

#define VMS_STR			"VMS"
#define OSF_STR			"OSF"
#define ULTRIX_STR		"Ultrix"
#define SYS5_STR		"SystemV"
#define DECC_STR		"DECC"
#define ALPHA_STR		"Alpha"

#define DNA_CMIP_OID_STR	"DNA_CMIP_OID"
#define OSI_CMIP_OID_STR	"OSI_CMIP_OID"
#define SNMP_OID_STR		"SNMP_OID"
#define OID_STR			"OID"

#define GROUP_IDENTIFIERS	1
#define GROUP_STATUS		2
#define GROUP_COUNTERS		3
#define GROUP_CHARACTERISTICS	4
#define GROUP_REFERENCES	5
#define GROUP_STATISTICS	6
#define GROUP_UNKNOWN		7

#ifdef UNIXPLATFORM

#define SS$_NORMAL 1
#define SS$_BADPARAM MAN_K_MOMGEN_ERROR
#define MAN_K_NO_CLASS_PARENT 0
#define MAN_K_UNKNOWN_OID_TYPE 0
#define MAN_K_NO_CLASS_ACL 0
#define MAN_K_INV_NUM_ACL_CLASSES 0

typedef enum {
  MOMGEN_C_OPENWRITEERROR 	= MAN_K_MOMGEN_ERROR,
  MOMGEN_C_OPENREADERROR 	= MAN_K_MOMGEN_ERROR,
  MOMGEN_C_SUCCESS 		= SS$_NORMAL,
  MOMGEN_C_VALUE_REQUIRED	= MAN_K_MOMGEN_ERROR,
  MOMGEN_C_ABSENT 		= MAN_K_MOMGEN_ERROR,
  MOMGEN_C_INSVIRMEM 		= MAN_K_MOMGEN_ERROR,
  MOMGEN_C_NO_SUCH_CLASS 	= MAN_K_NO_SUCH_CLASS,
  MOMGEN_C_PROCESSING_FAILURE 	= MAN_K_PROCESSING_FAILURE,
  MOMGEN_C_MOMGEN_ERROR		= MAN_K_MOMGEN_ERROR,
  MOMGEN_C_MIR_NOT_FOUND 	= MAN_K_MIR_NOT_FOUND,
  MOMGEN_C_MIR_ERROR 		= MAN_K_MIR_ERROR,
  MOMGEN_C_MCC_ERROR 		= MAN_K_MCC_ERROR,
  MOMGEN_C_INV_INFO_FILE 	= MAN_K_INV_INFO_FILE,
  MOMGEN_C_INT_MOMGEN_ERROR 	= MAN_K_INT_MOMGEN_ERROR,
  MOMGEN_C_NO_OID_FOUND 	= MAN_K_NO_OID_FOUND,
  MOMGEN_C_NO_CLASSES 		= MAN_K_NO_CLASSES,
  MOMGEN_C_INV_NUM_PARENT_CLASSES = MAN_K_INV_NUM_PARENT_CLASSES,
  MOMGEN_C_NO_AUTHOR		= MAN_K_NO_AUTHOR,
  MOMGEN_C_NO_FACILITY		= MAN_K_NO_FACILITY,
  MOMGEN_C_NO_ORGANIZATION 	= MAN_K_NO_ORGANIZATION,
  MOMGEN_C_NO_PARENT_CLASS 	= MAN_K_NO_PARENT_CLASS,
  MOMGEN_C_NO_CLASS_PREFIX 	= MAN_K_NO_CLASS_PREFIX,
  MOMGEN_C_INV_NUM_PREFIXES 	= MAN_K_INV_NUM_PREFIXES,
  MOMGEN_C_NO_MOM_NAME		= MAN_K_NO_MOM_NAME,
  MOMGEN_C_NO_DNA_CMIP_INT 	= MAN_K_NO_DNA_CMIP_INT,
  MOMGEN_C_NO_CLASS_PARENT 	= MAN_K_NO_CLASS_PARENT, 
  MOMGEN_C_UNKNOWN_OID_TYPE 	= MAN_K_UNKNOWN_OID_TYPE,
  MOMGEN_C_NO_CLASS_ACL		= MAN_K_NO_CLASS_ACL,
  MOMGEN_C_INV_NUM_ACL_CLASSES  = MAN_K_INV_NUM_ACL_CLASSES
} momgen_status;
#else
typedef enum {
  MOMGEN_C_OPENWRITEERROR 	= RMS$_FNF,
  MOMGEN_C_OPENREADERROR 	= SS$_NOSUCHFILE,
  MOMGEN_C_SUCCESS 		= SS$_NORMAL,
  MOMGEN_C_VALUE_REQUIRED	= CLI$_VALREQ,
  MOMGEN_C_ABSENT 		= CLI$_ABSENT,
  MOMGEN_C_INSVIRMEM 		= LIB$_INSVIRMEM,
  MOMGEN_C_NO_SUCH_CLASS 	= MAN_K_NO_SUCH_CLASS,
  MOMGEN_C_PROCESSING_FAILURE 	= MAN_K_PROCESSING_FAILURE,
  MOMGEN_C_MOMGEN_ERROR		= MAN_K_MOMGEN_ERROR,
  MOMGEN_C_MIR_NOT_FOUND 	= MAN_K_MIR_NOT_FOUND,
  MOMGEN_C_MIR_ERROR 		= MAN_K_MIR_ERROR,
  MOMGEN_C_MCC_ERROR 		= MAN_K_MCC_ERROR,
  MOMGEN_C_INV_INFO_FILE 	= MAN_K_INV_INFO_FILE,
  MOMGEN_C_INT_MOMGEN_ERROR 	= MAN_K_INT_MOMGEN_ERROR,
  MOMGEN_C_NO_OID_FOUND 	= MAN_K_NO_OID_FOUND,
  MOMGEN_C_NO_CLASSES 		= MAN_K_NO_CLASSES,
  MOMGEN_C_INV_NUM_PARENT_CLASSES = MAN_K_INV_NUM_PARENT_CLASSES,
  MOMGEN_C_NO_AUTHOR		= MAN_K_NO_AUTHOR,
  MOMGEN_C_NO_FACILITY		= MAN_K_NO_FACILITY,
  MOMGEN_C_NO_ORGANIZATION 	= MAN_K_NO_ORGANIZATION,
  MOMGEN_C_NO_PARENT_CLASS 	= MAN_K_NO_PARENT_CLASS,
  MOMGEN_C_NO_CLASS_PREFIX 	= MAN_K_NO_CLASS_PREFIX,
  MOMGEN_C_INV_NUM_PREFIXES 	= MAN_K_INV_NUM_PREFIXES,
  MOMGEN_C_NO_MOM_NAME		= MAN_K_NO_MOM_NAME,
  MOMGEN_C_NO_DNA_CMIP_INT 	= MAN_K_NO_DNA_CMIP_INT,
  MOMGEN_C_NO_CLASS_PARENT 	= MAN_K_NO_CLASS_PARENT,
  MOMGEN_C_UNKNOWN_OID_TYPE 	= MAN_K_UNKNOWN_OID_TYPE,
  MOMGEN_C_NO_CLASS_ACL		= MAN_K_NO_CLASS_ACL,
  MOMGEN_C_INV_NUM_ACL_CLASSES  = MAN_K_INV_NUM_ACL_CLASSES
} momgen_status;
#endif

typedef struct _ARGUMENT_DEF{
    struct _ARGUMENT_DEF *next;
    char	    *argument_type;
    char	    *argument_length;
    char	    *argument_name;
    int		    argument_number;
    char	    *data_type;
    char	    *dna_data_type;
    int 	    avl;
    object_id	    *oid;
    object_id	    *snmp_oid;
    object_id	    *osi_oid;
    int             dna_cmip_int;
    signed_int int dna_cmip_int_len;
    char 	    *snmp_oid_text;
    signed_int int snmp_oid_len;
    char	    *osi_oid_text;
    signed_int int osi_oid_len;    
    char	    *oid_text;
    signed_int int oid_len;
    char	    *original_name;
    int		    mg_type;
    int	            sign;
} ARGUMENT_DEF;

typedef struct _EVENT_DEF{
    struct _EVENT_DEF *next;
    int		    event_number;
    char	    *event_routine;
    char	    *event_name;
    int		    num_event_args;
    ARGUMENT_DEF    *event_arg;
    object_id	    *oid;
    object_id	    *snmp_oid;
    object_id	    *osi_oid;
    int             dna_cmip_int;
    signed_int int dna_cmip_int_len;
    char 	    *snmp_oid_text;
    signed_int int snmp_oid_len;
    char	    *osi_oid_text;
    signed_int int osi_oid_len;    
    char	    *oid_text;
    signed_int int oid_len;
    char	    *original_name;
} EVENT_DEF;

typedef struct _REQUEST_DEF{
    struct _REQUEST_DEF *next;
    char	    *request_name;
    char	    *name;
    int		    request_number;
    int		    num_arguments;
    ARGUMENT_DEF    *arg;
    object_id	    *oid;
    object_id	    *snmp_oid;
    object_id	    *osi_oid;
    int             dna_cmip_int;
    signed_int int dna_cmip_int_len;
    char 	    *snmp_oid_text;
    signed_int int snmp_oid_len;
    char	    *osi_oid_text;
    signed_int int osi_oid_len;    
    char	    *oid_text;
    signed_int int oid_len;
    char	    *original_name;
} REQUEST_DEF;

typedef struct _RESPONSE_DEF{
    struct _RESPONSE_DEF *next;
    char	    *response_name;
    char	    *name;
    int		    response_number;
    int		    num_arguments;
    ARGUMENT_DEF    *arg;
    object_id	    *oid;
    object_id	    *snmp_oid;
    object_id	    *osi_oid;
    int             dna_cmip_int;
    signed_int int dna_cmip_int_len;
    char 	    *snmp_oid_text;
    signed_int int snmp_oid_len;
    char	    *osi_oid_text;
    signed_int int osi_oid_len;    
    char	    *oid_text;
    signed_int int oid_len;
    char	    *original_name;
} RESPONSE_DEF;

typedef struct _EXCEPTION_DEF {
    struct _EXCEPTION_DEF *next;
    char	    *exception_name;
    char	    *name;
    int		    exception_number;
    int		    num_arguments;
    ARGUMENT_DEF    *arg;
    object_id	    *oid;
    object_id	    *snmp_oid;
    object_id	    *osi_oid;
    int             dna_cmip_int;
    signed_int int dna_cmip_int_len;
    char 	    *snmp_oid_text;
    signed_int int snmp_oid_len;
    char	    *osi_oid_text;
    signed_int int osi_oid_len;    
    char	    *oid_text;
    signed_int int oid_len;
    char	    *original_name;
} EXCEPTION_DEF;

typedef struct _DIRECTIVE_DEF { 
    struct _DIRECTIVE_DEF *next;
    char	    *directive_name;
    char	    *directive;
    int		    directive_number;
    int		    type;
    int		    num_requests;
    int		    num_responses;
    int		    num_exceptions;
    ARGUMENT_DEF    *arg;
    REQUEST_DEF     *req;
    RESPONSE_DEF    *resp;
    EXCEPTION_DEF   *exc;        
    object_id	    *oid;
    object_id	    *snmp_oid;
    object_id	    *osi_oid;
    int             dna_cmip_int;
    signed_int int dna_cmip_int_len;
    char 	    *snmp_oid_text;
    signed_int int snmp_oid_len;
    char	    *osi_oid_text;
    signed_int int osi_oid_len;    
    char	    *oid_text;
    signed_int int oid_len;
    int  	    current_dup_char;
} DIRECTIVE_DEF;

typedef struct _ACTION_DEF{
    struct _ACTION_DEF *next;
    int		    action_number;
    int		    original_action_number;
    char	    *action_routine;
    char	    *action_name;
    int		    action_done;
    DIRECTIVE_DEF   *directive;
} ACTION_DEF;

typedef struct _ATTRIBUTE_DEF {
    struct _ATTRIBUTE_DEF *next;
    char	    *attribute_name;
    char	    *length;
    char	    *data_type;
    int		    name;
    int		    uid;
    int		    group;
    int		    attribute_number;
    char 	    *dna_data_type;
    int		    avl;
    char	    *original_name;
    object_id	    *oid;
    object_id	    *snmp_oid;
    object_id	    *osi_oid;
    int             dna_cmip_int;
    signed_int int dna_cmip_int_len;
    char 	    *snmp_oid_text;
    signed_int int snmp_oid_len;
    char	    *osi_oid_text;
    signed_int int osi_oid_len;    
    char	    *oid_text;
    signed_int int oid_len;
    int		    mg_type;
    int		    sign;
} ATTRIBUTE_DEF;

typedef struct _CLASS_DEF {
    struct _CLASS_DEF *next;
    char	    *class_name;
    char	    *class_name_ptr;
    char	    *class_void_name;
    char 	    *orig_class_string;
    char 	    *class_string;
    char 	    *parent_class_string;
    char	    *orig_parent_class_string;
    signed_int int parent_class_string_len;
    char	    *parent;
    int		    steps_below_node;
    char	    *oid_below_node;
    int		    num_attributes;
    ATTRIBUTE_DEF   *attribute;
    DIRECTIVE_DEF   *directive;
    int		    num_directives;
    int		    num_create_args;
#ifdef MCC
    MCC_A_AES	    *aes;
#endif /* MCC */
    object_id	    *class_oid;
    EVENT_DEF	    *event;
    ARGUMENT_DEF    *create_arg;
    ARGUMENT_DEF    *event_arg;
    int		    num_actions;
    int		    num_events;
    ACTION_DEF	    *action;
    ACTION_DEF	    *current_action;
    char	    *prefix;
    char            *parent_prefix;
    char	    *prefix_k;
    char	    *prefix_m;
    char	    *prefix_mcc;
    char	    *prefix_dna;
    int		    support_instances;
    int		    support_add;
    int		    support_remove;
    int		    primary_id;
    int		    class_number;
    int		    avl_attribute;
    int		    multiple_oids;
    object_id	    *oid;
    object_id	    *snmp_oid;
    object_id	    *osi_oid;
    int             dna_cmip_int;
    signed_int int dna_cmip_int_len;
    char 	    *snmp_oid_text;
    signed_int int snmp_oid_len;
    char	    *osi_oid_text;
    signed_int int osi_oid_len;    
    char	    *oid_text;
    signed_int int oid_len;
    object_id	    *parent_oid;
    object_id	    *parent_snmp_oid;
    object_id	    *parent_osi_oid;
    int             parent_dna_cmip_int;
    signed_int int parent_dna_cmip_int_len;
    char 	    *parent_snmp_oid_text;
    signed_int int parent_snmp_oid_len;
    char	    *parent_osi_oid_text;
    signed_int int parent_osi_oid_len;    
    char	    *parent_oid_text;
    signed_int int parent_oid_len;
    int 	    current_dup_char;
    int 	    current_dup_char_argc;
    int 	    current_dup_char_req;
    int 	    current_dup_char_resp;
    int 	    current_dup_char_exc;
    int 	    current_dup_char_evt;
    int 	    current_dup_char_evt_arg;
    int		    support_delete;
    int		    support_create;
    int		    support_action;
    int		    support_cancel_get;
    int		    support_get;
    int		    support_getnext;
    int		    support_set;
    int		    parent_generated;
    int		    ACL_attribute_num;
    int		    ACL_argument_num;
} CLASS_DEF;

typedef struct {
    char	    *author;
    char	    *organization;
    char	    *copyright_date;
    char	    *copyright_owner;
    char	    *creation_date;
    char	    *mom_name;
    char	    *facility;
    char	    *include_file;
    char	    *include_file_inc;
    char	    *msl_file;
    char	    *prefix;
    int		    num_include_files;
    char	    *extern_common;
    char 	    *source_file;
    char	    *link_file;
    char	    *descrip_file;
    char	    *info_file;
    char            *mir_file;
    char	    *spaces;
    CLASS_DEF	    *current_class;
    char	    *temp;
    int		    create_done;
    int		    delete_done;
    int		    enable_done;
    int		    disable_done;
    int		    resume_done;
    int		    suspend_done;
    int		    connect_done;
    int		    disconnect_done;
    int		    test_done;
    int		    build_all;
    int		    build_access;
    int		    build_action;
    int		    build_create;
    int		    build_command_file;
    int		    build_perform_create;
    int		    build_delete;
    int		    build_perform_delete;
    int		    build_cancel_get;
    int		    build_descrip;
    int		    build_directive;
    int		    build_dir;
    int		    build_find;
    int		    build_get;
    int		    build_getnext;
    int		    build_get_next;
    int		    build_get_candidate;
    int		    build_makefile;
    int		    build_perform_get;
    int		    build_init;
    int		    build_perform_init;
    int		    build_set;
    int		    build_perform_set;
    int		    build_common;
    int		    build_util;
    int		    build_options;
    int		    build_shut;
    int		    build_oids;
    int		    build_prototypes;
    int		    oid_type;
    int		    OID_EMA;
    int		    OID_SNMP;
    int		    default_oid;
    int		    multi_oids;
    int		    multiclass;
    int		    support_delete;
    int		    support_create;
    int		    support_action;
    int		    support_cancel_get;
    int		    support_get;
    int		    support_getnext;
    int		    support_set;
    int		    num_classes;
    int		    log;
    int		    support_instances;
    int		    MCC_dictionary;
    int		    debug;
    CLASS_DEF	    *class;
    int		    use_symbols;
    char	    *system;
    char	    *input_filename;
    int		    attr;
    char 	    *log_file_str;
    FILE	    *log_file;
    int		    num_threads;
    char 	    *mom_in;
    char	    *mom_out_src;
    char	    *mom_out_build;
    int		    invalidattr_exc_code;
    int		    missingattr_exc_code;
    int		    nosuchattr_exc_code;
    int		    unexpected_exc_code;
    int		    duplicate_exc_code;
    char            *mom_target;
} MOM_BUILD;

#define MOMGEN_K_DIR_EXAMINE 1
#define MOMGEN_K_DIR_MODIFY  2
#define MOMGEN_K_DIR_ACTION  3 
#define MOMGEN_K_DIR_EVENT   4
#define MOMGEN_K_DIR_UNKNOWN 5

#define MAX_THREADS 100

#define underscore		"_"
#define underscore_k		"_K_"

#define dna_prefix		"D_"
#define mcc_prefix 	        "M_"

#define cls_prefix		"CLASS_"
#define attr_prefix		"ATTR_"
#define dir_prefix		"DIR_"
#define resp_prefix		"RESP_"   
#define req_prefix		"REQ_"
#define exc_prefix		"EXC_"
#define	arg_prefix		"ARG_"
#define	argc_prefix		"ARGC_"
#define evp_prefix		"EVP_"
#define evg_prefix		"EVG_"
#define evt_prefix		"EVT_"

#define req_type		"ACTION_REQ_ARG_SEQ"
#define resp_type		"ACTION_RESP_ARG_SEQ"
#define excep_type		"ACTION_EXCEP_ARG_SEQ"
#define event_type		"EVENT_ARG_SEQ"
#define req_type_l		"ACTION_REQ_ARG_LENGTH"
#define resp_type_l		"ACTION_RESP_ARG_LENGTH"
#define excep_type_l		"ACTION_EXCEP_ARG_LENGTH"
#define event_type_l		"EVENT_ARG_LENGTH"

#ifndef UNIXPLATFORM
# define MOM_IN_DEFAULT		"SYS$LIBRARY:"
# define MOM_OUT_SRC_DEFAULT     ""
# define MOM_OUT_BUILD_DEFAULT   ""
#else
# if defined(ultrix) || defined(__ultrix)
#  define MOM_IN_DEFAULT		"/usr/lib/eca/"
# else
#  define MOM_IN_DEFAULT		"/usr/ccs/lib/eca/"  
# endif
#endif

#define mark_insert_start   "[["
#define mark_insert_start_len 2
#define mark_insert_end	    "]]"
#define mark_insert_end_len 2

#define standard_create_routine "$STANDARD_CREATE"
#define standard_delete_routine "$STANDARD_DELETE"

#define variable_length	    "0 /** insert proper length **/"
#define length_uid	    "16"
#define length_time	    "16"

#define string_null		""

#define MG_C_TIME	1
#define MG_C_AVL	2
#define MG_C_INTEGER	3
#define MG_C_STRING	4
#define MG_C_BOOLEAN	5
#define	MG_C_UID	6
#define MG_C_BIT	7
#define MG_C_KNOWN	8
#define MG_C_NULL	9
#define MG_C_OID	10
#define MG_C_UNKNOWN	11
#define MG_C_VERSION	12
#define MG_C_ACL	13
#define MG_C_SIGNED	14
#define MG_C_UNSIGNED   15
