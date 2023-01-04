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
 * @(#)$RCSfile: moss.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:48:43 $
 */
/*
 *  static char *sccsid = "%W%    DECwest    %G%"
 */
/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1989, 1990, 1991, 1992.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    This the header file containing public definitions for MOSS.
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    October 31, 1989
 *
 * Revision History :
 *
 *    Richard J. Bouchard, Jr., July 24, 1992 (RJB1217)
 *  
 *    Add prototypes for a bunch of VMS-supplied MOSS extensions
 *
 *    Oscar Newkerk  October 16, 1990
 *
 *    Add the declaration for moss_get_time routine.
 *
 *    Oscar Newkerk  October 4, 1990
 *
 *    Add the declarations for moss_avl_exit_construction, moss_avl_find_item,
 *    and  moss_avl_index_buff.
 *
 *    Wim Colgate, February 22nd, 1990:
 *
 *    Modified enum's to private ASN1 types that are constructed
 *    to map to how AVL now handle constructed types.
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 *
 *    Miriam Amos Nihart, June 4th, 1990.
 *
 *    Add the declarations for moss_oid_to_octet and moss_octet_to_oid.
 *
 *    Miriam Amos Nihart, July 1st, 1990.
 *
 *    Change the names of moss_avl_*_constructed_field() to moss_avl_*_cons_field.
 *
 *    Kathy Faust, July 11th, 1990.
 *
 *    Add declaration for moss_avl_copy.
 *
 *    Kathy Faust, July 17th, 1990.
 *
 *    Add declaration for moss_compare_partial_oid().
 *
 *    Oscar Newkerk, September 24, 1990.
 *
 *    Add moss_avl_next_item and moss_avl_previous_item.
 *
 *    Miriam Amos Nihart, October 12th, 1990.
 *
 *    Add moss_oid_append.
 *
 *    Miriam Amos Nihart, October 15th, 1990.
 *
 *    Remove the declaration for moss_avl_point_cons_field().
 *
 *    Miriam Amos Nihart, November 26th, 1990.
 *
 *    Rename moss_avl_index_buff to moss_avl_index_buf.
 *
 *    Miriam Amos Nihart, October 16th, 1991. 
 *
 *    Put in prototyping.
 *
 *    Kathy Faust, October 18th, 1991.
 *
 *    Update moss_start_server prototype for new args.
 *
 *    Miriam Amos Nihart, November 8th, 1991.
 *
 *    Add prototypes for moss avl mutex routines.
 *
 *    Wim Colgate, November 18th, 1991. 
 *
 *    Fixed prototype for moss_match_instance_name to a **comparison,
 *    from a comparison.
 *
 *    Miriam Amos Nihart, December 6th, 1991.
 *
 *    Change prototype for moss_avl_exit_construction.
 *
 *    Kathy Faust, December 11th, 1991.
 *
 *    Add prototype for moss_stop_server.
 *
 *    Russell N. Murray, January 15th, 1992
 *    
 *    Change MAKE_MASK to allow for standard compilation.
 *
 *    Richard J. Bouchard, Jr.  July 24, 1992 (RJB1217)
 *  
 *    Add in MOSS OID extensions from VMS.
 */

#ifndef MOSS_HEADER
#define MOSS_HEADER

#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
#define PROTOTYPE(args) args
#else
#define PROTOTYPE(args) ()
#endif

/*
 ** NOTE:  The development libraries and the generated code are designed to support multithreaded
 ** MOMs using the DECthreads POSIX 1003.4 interface. 
 ** However, on ULTRIX this capability was not bundled until V4.3 and then 
 ** several patches are required.
 */
#if defined(__osf__)
# define PTHREADS_SUPPORTED
#endif 

#ifndef NOIPC
# ifdef PTHREADS_SUPPORTED
#  include <pthread.h>
# endif
#endif

#include "man.h"
#include "man_data.h"
#include "moss_cmp.h"

#include "moss_asn1.h"

#define MAKE_MASK( value ) ( 1 << ( ( int )value & 0xffff ) )
#define CMIS_C_FILTER_TYPE   CONTEXT_SPECIFIC( CONSTRUCTED( 0x10010 ) )
#define CMIS_C_BASE ( 0x10000 )

typedef enum {
    CMIS_C_FILTER_ITEM   = CONTEXT_SPECIFIC( CONSTRUCTED( 0x10011 ) ) ,
    CMIS_C_AND_SET       = CONTEXT_SPECIFIC( CONSTRUCTED( 0x10012 ) ) ,
    CMIS_C_OR_SET        = CONTEXT_SPECIFIC( CONSTRUCTED( 0x10013 ) ) ,
    CMIS_C_NOT           = CONTEXT_SPECIFIC( CONSTRUCTED( 0x10014 ) )
} cmis_filter ;

typedef enum {
    CMIS_C_EQUALITY       = CONTEXT_SPECIFIC( 0x10000 ) ,
    CMIS_C_GTE            = CONTEXT_SPECIFIC( 0x10001 ) ,
    CMIS_C_LTE            = CONTEXT_SPECIFIC( 0x10002 ) ,
    CMIS_C_PRESENT        = CONTEXT_SPECIFIC( 0x10003 ) ,
    CMIS_C_SUBSET         = CONTEXT_SPECIFIC( 0x10004 ) ,
    CMIS_C_SUPERSET       = CONTEXT_SPECIFIC( 0x10005 ) ,
    CMIS_C_INTERSECTION   = CONTEXT_SPECIFIC( 0x10006 ) ,
    CMIS_C_INITIAL_STRING = CONTEXT_SPECIFIC( 0x10007 ) ,
    CMIS_C_ANY_STRING     = CONTEXT_SPECIFIC( 0x10008 ) ,
    CMIS_C_FINAL_STRING   = CONTEXT_SPECIFIC( 0x10009 ) 
} cmise_filter_relation ;

typedef enum {
    CMIS_M_EQUALITY      = MAKE_MASK( CMIS_C_EQUALITY ) ,
    CMIS_M_GTE           = MAKE_MASK( CMIS_C_GTE ) ,
    CMIS_M_LTE           = MAKE_MASK( CMIS_C_LTE ) ,
    CMIS_M_PRESENT       = MAKE_MASK( CMIS_C_PRESENT ) ,
    CMIS_M_SUBSET        = MAKE_MASK( CMIS_C_SUBSET ) ,
    CMIS_M_SUPERSET      = MAKE_MASK( CMIS_C_SUPERSET ) ,
    CMIS_M_INTERSECTION  = MAKE_MASK( CMIS_C_INTERSECTION ) ,
    CMIS_M_INITIAL_STRING= MAKE_MASK( CMIS_C_INITIAL_STRING ) ,
    CMIS_M_ANY_STRING    = MAKE_MASK( CMIS_C_ANY_STRING ) ,
    CMIS_M_FINAL_STRING  = MAKE_MASK( CMIS_C_FINAL_STRING ) 
} comparison_mask ;

typedef struct _octet_string
{
    unsigned int length ;
    unsigned int data_type ;
    char *string ;
} octet_string ;

/*
 * The opaque AVL is defined in man_data.h
 */

man_status
moss_avl_init PROTOTYPE((
avl **
)) ;

man_status
moss_avl_start_construct PROTOTYPE((
avl * ,
object_id * ,
unsigned int ,
unsigned int ,
octet_string *
)) ;

man_status
moss_avl_end_construct PROTOTYPE((
avl *
)) ;

man_status
moss_avl_reset PROTOTYPE((
avl *
)) ;

man_status
moss_avl_free PROTOTYPE((
avl ** ,
int
)) ;

man_status
moss_avl_add PROTOTYPE((
avl * ,
object_id * ,
unsigned int ,
unsigned int ,
octet_string *
)) ;

man_status
moss_avl_add_cons_field PROTOTYPE((
avl * ,
unsigned int ,
unsigned int ,
octet_string *
)) ;

man_status
moss_avl_point PROTOTYPE((
avl * ,
object_id ** ,
unsigned int * ,
unsigned int * ,
octet_string ** ,
int *
)) ;

man_status
moss_avl_backwards_point PROTOTYPE((
avl * ,
object_id ** ,
unsigned int * ,
unsigned int * ,
octet_string ** ,
int *
)) ;

man_status
moss_avl_backup PROTOTYPE((
avl *
)) ;

man_status
moss_avl_remove PROTOTYPE((
avl *
)) ;

man_status
moss_avl_append PROTOTYPE((
avl * ,
avl * ,
int
)) ;

man_status
moss_avl_next PROTOTYPE((
avl *
)) ;

man_status
moss_avl_to_buf PROTOTYPE((
avl * ,
char ** ,
int *
))  ;

man_status
moss_avl_to_mybuf PROTOTYPE((
avl * ,
char * ,
int
)) ;

man_status 
moss_avl_length PROTOTYPE((
avl * ,
int *
)) ;

man_status
moss_avl_from_buf PROTOTYPE((
avl * ,
char *
)) ;

man_status
moss_avl_copy PROTOTYPE((
avl * ,
avl * ,
int ,
object_id * ,
unsigned int * ,
int *
)) ;

man_status
moss_avl_exit_construction PROTOTYPE((
avl * ,
int ,
int ,
int *
)) ;

man_status
moss_avl_find_item PROTOTYPE((
avl * ,
object_id * ,
unsigned int * ,
unsigned int * ,
octet_string **
)) ;

man_status
moss_avl_find_item_always PROTOTYPE((
avl * ,
object_id * ,
unsigned int * ,
unsigned int * ,
octet_string **
)) ;

#ifdef RPCV2
man_status
moss_avl_init_mutex PROTOTYPE((
avl * ,
int ,
char **
)) ;

man_status
moss_avl_free_mutex PROTOTYPE((
avl *
)) ;

man_status
moss_avl_lock_mutex PROTOTYPE((
avl *
)) ;

man_status
moss_avl_unlock_mutex PROTOTYPE((
avl *
)) ;

man_status
moss_avl_get_user_area PROTOTYPE((
avl * ,
char **
)) ;
#endif /* RPCV2 */

man_status
moss_avl_index_buf PROTOTYPE((
avl * ,
char *
)) ;

man_status
moss_avl_copy_all PROTOTYPE((
avl * ,
avl * ,
int
)) ;

man_status
moss_avl_remove_construct PROTOTYPE((
avl *
)) ;

man_status
moss_init_cmise_filter PROTOTYPE((
avl **
)) ;

man_status
moss_add_cmise_filter_item PROTOTYPE((
avl * ,
object_id * ,
unsigned int ,
octet_string * ,
cmise_filter_relation
)) ;

man_status
moss_finish_cmise_filter PROTOTYPE((
avl *
)) ;

man_status
moss_free_cmise_filter PROTOTYPE((
avl **
)) ;

man_status
moss_start_cmise_or_set PROTOTYPE((
avl *
)) ;

man_status
moss_start_cmise_and_set PROTOTYPE((
avl *
)) ;

man_status
moss_add_cmise_not PROTOTYPE((
avl *
)) ;

man_status
moss_end_cmise_andornot_set PROTOTYPE((
avl *
)) ;

man_status
moss_point_filter PROTOTYPE((
avl * ,
object_id ** ,
unsigned int * ,
unsigned int * ,
octet_string ** ,
int *
)) ;

man_status
moss_start_cmise_filter_conitem PROTOTYPE((
avl * ,
object_id * ,
unsigned int ,
octet_string * ,
cmise_filter_relation
)) ;

man_status
moss_add_cmise_conitem_field PROTOTYPE((
avl * ,
unsigned int ,
unsigned int ,
octet_string *
)) ;

man_status
moss_start_cmise_nested_conitem PROTOTYPE((
avl * ,
unsigned int ,
unsigned int ,
octet_string *
)) ;

man_status
moss_end_cmise_nested_conitem PROTOTYPE((
avl *
)) ;

man_status
moss_end_cmise_filter_conitem PROTOTYPE((
avl *
)) ;

man_status
moss_apply_smi_filter PROTOTYPE((
avl * ,
avl * ,
PF * ,
char * ,
comparison **
)) ;

man_status
moss_validate_filter PROTOTYPE((
avl * ,
comparison **
)) ;

man_status
moss_create_oid PROTOTYPE((
int ,
unsigned int * ,
object_id **
)) ;

man_status
moss_free_oid PROTOTYPE((
object_id *
)) ;

man_status
moss_get_oid_len PROTOTYPE((
object_id * ,
int *
)) ;

man_status
moss_parse_oid PROTOTYPE((
object_id * ,
int * ,
int *
)) ;

man_status
moss_compare_oid PROTOTYPE((
object_id * ,
object_id *
)) ;

man_status
moss_compare_partial_oid PROTOTYPE((
object_id * ,
object_id * ,
int ,
int ,
int
)) ;

man_status
moss_concat_oids PROTOTYPE((
object_id * ,
object_id * ,
object_id **
)) ;

man_status
moss_text_to_oid PROTOTYPE((
char * ,
object_id **
)) ;

man_status
moss_oid_to_text PROTOTYPE((
object_id * ,
char * ,
char * ,
char * ,
char **
)) ;

man_status
moss_octet_to_oid PROTOTYPE((
octet_string * ,
object_id **
)) ;

man_status
moss_oid_to_octet PROTOTYPE((
object_id * ,
octet_string *
)) ;

man_status
moss_oid_append PROTOTYPE((
object_id * ,
int ,
object_id **
)) ;

man_status
moss_compare_oid_prefix PROTOTYPE((
object_id * ,
object_id *
)) ;

man_status
moss_oid_from_buf PROTOTYPE((
object_id ** ,
char *
)) ;

man_status 
moss_oid_length PROTOTYPE((
object_id * ,
int *
)) ;

man_status
moss_oid_to_buf PROTOTYPE((
object_id * ,
char **,
int *
)) ;

man_status
moss_oid_to_mybuf PROTOTYPE((
object_id * ,
char * ,
int
)) ;

man_status
moss_oid_to_flat PROTOTYPE((
object_id * ,
char * ,
int * ,
int ,
int
)) ;

man_status
moss_alloc_pe_handle PROTOTYPE((
management_handle * ,
man_binding_handle *
)) ;

man_status
moss_free_pe_handle PROTOTYPE((
man_binding_handle
)) ;

man_status
moss_create_management_handle PROTOTYPE((
management_handle **
)) ;

man_status
moss_alloc_mold_handle PROTOTYPE((
man_binding_handle *
)) ;

man_status
moss_free_mold_handle PROTOTYPE((
man_binding_handle
)) ;

man_status
moss_match_instance_name PROTOTYPE((
avl * ,
avl * ,
unsigned int ,
comparison **
)) ;

man_status
moss_start_server PROTOTYPE((
PFV * ,
int ,
management_handle *
)) ;

#ifdef RPCV2
man_status
moss_stop_server PROTOTYPE((
management_handle *
)) ;
#endif /* RPCV2 */

man_status
moss_register PROTOTYPE((
man_binding_handle ,
management_handle * ,
object_id * ,
object_id *
)) ;

man_status
moss_deregister PROTOTYPE((
man_binding_handle ,
management_handle * ,
object_id *
)) ;

man_status
moss_send_get_reply PROTOTYPE((
man_binding_handle ,
int ,
reply_type ,
object_id * ,
avl * ,
uid * ,
mo_time ,
avl * ,
int
)) ;

man_status
moss_send_set_reply PROTOTYPE((
man_binding_handle ,
int ,
reply_type ,
object_id * ,
avl * ,
uid * ,
mo_time ,
avl * ,
int
)) ;

man_status
moss_send_create_reply PROTOTYPE((
man_binding_handle ,
int ,
reply_type ,
object_id * ,
avl * ,
uid * ,
mo_time ,
avl *
)) ;

man_status
moss_send_delete_reply PROTOTYPE((
man_binding_handle ,
int ,
reply_type ,
object_id * ,
avl * ,
uid * ,
mo_time ,
avl * ,
int
)) ;

man_status
moss_send_action_reply PROTOTYPE((
man_binding_handle ,
int ,
reply_type ,
object_id * ,
avl * ,
uid * ,
mo_time ,
object_id * ,
object_id * ,
avl * ,
int
)) ;


/* From MOSS_MISC */

man_status
moss_free_time PROTOTYPE((
mo_time *
)) ;

man_status
moss_get_time PROTOTYPE((
mo_time **
)) ;

man_status
moss_get_pid PROTOTYPE((
process_id *
)) ;

man_status
moss_reply_required PROTOTYPE((
management_handle *
)) ;

/* From MOSS_DEBUG */

man_status
moss_print_dna_avl PROTOTYPE((
avl *
)) ;

/* From MOSS_UID */

/* "format" specifiers for moss_uid_to_text() */
#define MAN_K_UID_FORMAT_NONE		0
#define MAN_S_UID_FORMAT_NONE		(38 + 1)
#define MAN_K_UID_FORMAT_UNKNOWN	0
#define MAN_S_UID_FORMAT_UNKNOWN	(38 + 1)
#define MAN_K_UID_FORMAT_DNA		1
#define MAN_S_UID_FORMAT_DNA		(36 + 1)
#define MAN_K_UID_FORMAT_OSF		2
#define MAN_S_UID_FORMAT_OSF		(36 + 1)
#define MAN_K_UID_FORMAT_DCE		2
#define MAN_S_UID_FORMAT_DCE		(36 + 1)
#define MAN_K_UID_FORMAT_NCS		3
#define MAN_S_UID_FORMAT_NCS		(36 + 1)
#define MAN_K_UID_FORMAT_HP		3
#define MAN_S_UID_FORMAT_HP		(36 + 1)
#define MAN_K_UID_FORMAT_APOLLO		3
#define MAN_S_UID_FORMAT_APOLLO		(36 + 1)
#define MAN_K_UID_FORMAT_MICROSOFT	4
#define MAN_S_UID_FORMAT_MICROSOFT	(36 + 1)


/* Required length of text_buffer for moss_uid_to_text() */
/* ASSERTION:  MAN_S_UID_FORMAT_NONE == _UNKNOWN */
/* ASSERTION:  MAN_S_UID_FORMAT_DNA == _OSF == _DCE == _NCS == _HP == _APOLLO == _MICROSOFT */
#define MAN_S_UID_TEXT_BUFFER		(MAN_S_UID_FORMAT_NONE > MAN_S_UID_FORMAT_DNA		\
					 ? MAN_S_UID_FORMAT_NONE				\
					 : MAN_S_UID_FORMAT_DNA)

man_status
moss_free_uid PROTOTYPE((
uid **
)) ;

man_status
moss_get_uid PROTOTYPE((
uid **
)) ;

man_status
moss_uid_to_text PROTOTYPE((
uid * ,
int ,
char **
)) ;

/* The following routine is unix only and in agent_authentication.c */

#ifndef VMS

int
authenticate_client PROTOTYPE((
int ,
avl *
)) ;

#endif

#endif /* end of file moss.h */
