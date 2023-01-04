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
 * @(#)$RCSfile: moss_d_table.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:31:58 $
 */
/*
 *  static char *sccsid = "%W%	DECwest	%G%" ;
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
 *    This the header file contains comparison table for DNA Phase V attributes.
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    May 1st, 1990.
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 *
 */

#ifndef MOSS_D_TABLE
#define MOSS_D_TABLE

#include "moss_cmp.h"
#include "moss_dna.h"

#define DNA_C_VERSION_SIZE 4
/*
 *  DNA Data type comparison routines
 */

extern
man_status 
match_simple_name() ;

extern
man_status
moss_dna_unsigned16_cmp() ;

extern
man_status
moss_dna_unsigned32_cmp() ;

extern
man_status
moss_dna_unsigned64_cmp() ;

extern
man_status
moss_dna_hex_str_cmp() ;

extern
man_status
moss_dna_latin1_str_cmp() ;

extern
man_status
moss_dna_version_cmp() ;

extern
man_status
moss_dna_version_with_edit_cmp() ;

extern
man_status
moss_asn1_null_cmp() ;

extern
man_status
moss_dna_octet_cmp() ;

extern
man_status
moss_dna_known_cmp() ;

extern
man_status
moss_dna_ip_address_cmp() ;

extern
man_status
moss_dna_id802_cmp() ;

extern
man_status
moss_dna_idenet_type_cmp() ;

extern
man_status
moss_dna_id802_snap_cmp() ;

/*
 * The following table represents the DNA APPLICATION data types.
 * The first item is the data type. 
 * second is the routine to perform the comparison. 
 * The third is the allowable comparative operations. 
 * The table must always in a NULL entry (0,0,0).
 */

comparison dna_application_table[] = 
     {
           { DNA_C_COUNTER16,     moss_dna_unsigned16_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_COUNTER32,     moss_dna_unsigned32_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_COUNTER64,     moss_dna_unsigned64_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_COUNTER,       moss_dna_unsigned64_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_CHAR_ABS_TIME, moss_asn1_octet_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } , 
           { DNA_C_BIN_ABS_TIME,  moss_asn1_octet_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_CHAR_REL_TIME, moss_asn1_octet_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } , 
           { DNA_C_BIN_REL_TIME,  moss_asn1_octet_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } , 
           { DNA_C_LATIN1_STRING, moss_dna_latin1_str_cmp,  (unsigned int) (CMIS_M_INITIAL_STRING | CMIS_M_FINAL_STRING | 
                                                             CMIS_M_ANY_STRING | CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_SIMPLE_NAME,   match_simple_name,        (unsigned int) CMIS_M_EQUALITY } ,
           { DNA_C_FULL_NAME,     moss_dna_latin1_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_INITIAL_STRING | 
                                                             CMIS_M_FINAL_STRING | CMIS_M_ANY_STRING) } ,
           { DNA_C_UID,           moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
           { DNA_C_VERSION,       moss_dna_version_cmp,     (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } , 
           { DNA_C_VERSION_EDIT,  moss_dna_version_with_edit_cmp, (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_DTE_ADDRESS,   moss_dna_latin1_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_INITIAL_STRING | 
                                                             CMIS_M_FINAL_STRING | CMIS_M_ANY_STRING) } , 
           { DNA_C_FILE_SPEC,     moss_dna_latin1_str_cmp,  (unsigned int) (CMIS_M_INITIAL_STRING | CMIS_M_FINAL_STRING | 
                                                             CMIS_M_ANY_STRING | CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_NSAP_ADDRESS,  moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } , 
           { DNA_C_NET_ENT_TITLE, moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } , 
           { DNA_C_AREA_ADDRESS,  moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } , 
           { DNA_C_END_USER_SPEC, moss_asn1_octet_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } , 
           { DNA_C_T_SELECTOR,    moss_asn1_octet_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } , 
           { DNA_C_PHASE_4_NAME,  moss_dna_latin1_str_cmp,  (unsigned int) (CMIS_M_INITIAL_STRING | CMIS_M_FINAL_STRING | 
                                                             CMIS_M_ANY_STRING | CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_PHASE_4_ADDRESS,moss_dna_unsigned16_cmp, (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_KNOWN,         moss_dna_known_cmp,       (unsigned int) CMIS_M_EQUALITY } ,
           { DNA_C_OCTET,         moss_dna_octet_cmp,       (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_HEX_STRING,    moss_dna_hex_str_cmp,     (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_SESSION_SELECTOR,  moss_asn1_octet_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } , 
           { DNA_C_PRESENT_SELECTOR,  moss_asn1_octet_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } , 
           { DNA_C_DNA_CMIP_MESSAGE,  moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } , 
           { DNA_C_COMPONENT_NAME,  moss_dna_unsigned32_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
           { DNA_C_IP_ADDRESS,    moss_dna_ip_address_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
           { DNA_C_ID802,         moss_dna_id802_cmp,       (unsigned int) CMIS_M_EQUALITY } ,
           { DNA_C_IDENET_TYPE,   moss_dna_idenet_type_cmp, (unsigned int) CMIS_M_EQUALITY } ,
           { DNA_C_ID802_SNAP,    moss_dna_id802_snap_cmp,  (unsigned int) CMIS_M_EQUALITY } ,

           /* Are there comparison routines for these data types ?
              DNA_C_FULL_ENTITY_NAME ???? 
              DNA_C_DEFAULT_VALUE ????
              DNA_C_ADDRESS_PREFIX ????
              DNA_C_TOWER_SET ????
              DNA_C_IMPLEMENTATION ????
              DNA_C_DNS_TIME_STAMP ????
              DNA_C_ENTITY_CLASS ????
              DNA_C_FLOOR ????
              DNA_C_PROTOCOL_TOWER ????
              DNA_C_RANGE ????
            */

           { NULL,                 NULL,                        NULL }
     } ;

comparison dna_context_specific_table[] = 
     {
           { NULL,  NULL, NULL }
     } ;

comparison dna_private_table[] =
     {
           { NULL, NULL, NULL }
     } ;

#endif
