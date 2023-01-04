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
 * @(#)$RCSfile: moss_u_table.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:33:48 $
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
 *    Wim Colgate, November 18th, 1991.
 *
 *    Added NULL comparions routine definition.
 */

#ifndef MOSS_U_TABLE
#define MOSS_U_TABLE

#include "moss_asn1.h"
#include "moss_cmp.h"

/*
 *  ASN1 Data type comparison routines
 */

extern
man_status
moss_compare() ;

extern
man_status
moss_asn1_boolean_cmp() ;

extern
man_status
moss_asn1_integer_cmp() ;

extern
man_status
moss_asn1_octet_str_cmp() ;

extern
man_status
moss_asn1_bit_string_cmp() ;

extern
man_status
moss_asn1_null_cmp() ;

/*
 * The following table represents the UNIVERSAL data types supported.
 * The first item is the data type (defined in moss_asn1_types.h), the
 * second is the routine to perform the comparison (defined in moss_asn1_cmp.h)
 * The third is the allowable comparative operations (defined in moss_asn1_types.h)
 * The table must always in a NULL entry (0,0,0).
 */

comparison universal_table[] =
    {
        { ASN1_C_BOOLEAN,          moss_asn1_boolean_cmp,    (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_INTEGER,          moss_asn1_integer_cmp,    (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
        { ASN1_C_BITSTRING,        moss_asn1_bit_string_cmp, (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_OCTET_STRING,     moss_asn1_octet_str_cmp,  (unsigned int) (CMIS_M_EQUALITY | CMIS_M_GTE | CMIS_M_LTE) } ,
        { ASN1_C_NULL,             moss_asn1_null_cmp,       (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_NUMERIC_STRING,   moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_PRINTABLE_STRING, moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_TELETEX_STRING,   moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_VIDEOTEX_STRING,  moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_IA5_STRING,       moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_GRAPHIC_STRING,   moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_VISIBLE_STRING,   moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_GENERAL_STRING,   moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
        { ASN1_C_OBJECT_ID,        moss_asn1_octet_str_cmp,  (unsigned int) CMIS_M_EQUALITY } ,
        { NULL,                    0,                        0 }
    } ;

#endif
