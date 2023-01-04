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
 * @(#)$RCSfile: moss_cmp.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:02:32 $
 */
/****************************************************************************
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
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file name to reflect the 14 character restriction.
 *
 */

#include "moss.h"

#ifndef MOSS_ASN1_CMP
#define MOSS_ASN1_CMP

typedef man_status PF() ;

typedef struct _comparison {
    unsigned int code ;
    PF *compare_routine ;
    unsigned int valid_comparisons ;
} comparison ;

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
moss_asn1_bit_string_str_cmp() ;

#endif /* end of file moss_asn1_cmp.h */
