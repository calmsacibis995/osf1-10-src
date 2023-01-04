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
 * @(#)$RCSfile: moss_asn1.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:02:24 $
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
 *    This the header file containing public definitions for MOSS ASN1.
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
 *	Wim Colgate February 22nd, 1990:
 *
 *	Added Macros to define types (rather than striaght #defines).
 *
 *      Miriam Amos Nihart, May 14th, 1990.
 *
 *      Change the file name to reflect the 14 character restriction.
 *
 */

#ifndef	MOSS_ASN1_TYPES
#define MOSS_ASN1_TYPES

/*
 * Define the ASN1 encoding. ASN1 tags are defined to be 8 bit
 * quantities. We define the internal representation to be 32 bit
 * (unsigned long ints). The top 2 bits define the 'class' (UNIVERSAL, APPLICATION,
 * CONTEXT_SPECIFIC and PRIVATE), the next lower significant bit defines 
 * primitive (0) or constructed (1) type. The remaining low order bits define
 * the particular value.
 *
 *                                        class        P/C       value 
 */

#define UNIVERSAL( value )           ( 0x00000000 | 0x00000000 | value )
#define APPLICATION( value )         ( 0x40000000 | 0x00000000 | value )
#define CONTEXT_SPECIFIC( value )    ( 0x80000000 | 0x00000000 | value )
#define PRIVATE( value )             ( 0xc0000000 | 0x00000000 | value )

#define CONSTRUCTED( value )         ( 0x00000000 | 0x20000000 | value ) 

/*
 * Define a Macro to mask everything but the 29th bit, and shift this bit to bit 0.
 * This will determine if the value is constructed or non-constructed.
 */

#define IS_CONSTRUCTED( value )   ( ( 0x20000000 & value ) >> 29 )

/* 
 * ISO-defined data types (ASN.1) 
 */

#define ASN1_C_EOC              UNIVERSAL( 0 )   /* End Of Construction */
#define ASN1_C_BOOLEAN          UNIVERSAL( 1 ) 
#define ASN1_C_INTEGER          UNIVERSAL( 2 )
#define ASN1_C_BITSTRING        UNIVERSAL( 3 )
#define ASN1_C_OCTET_STRING     UNIVERSAL( 4 )
#define ASN1_C_NULL             UNIVERSAL( 5 )
#define ASN1_C_OBJECT_ID        UNIVERSAL( 6 )
#define ASN1_C_SEQUENCE         CONSTRUCTED( UNIVERSAL( 16 ) ) 
#define ASN1_C_SET              CONSTRUCTED( UNIVERSAL( 17 ) )
#define ASN1_C_NUMERIC_STRING   UNIVERSAL( 18 )
#define ASN1_C_PRINTABLE_STRING UNIVERSAL( 19 )
#define ASN1_C_TELETEX_STRING   UNIVERSAL( 20 )
#define ASN1_C_VIDEOTEX_STRING  UNIVERSAL( 21 )
#define ASN1_C_IA5_STRING       UNIVERSAL( 22 )
#define ASN1_C_GRAPHIC_STRING   UNIVERSAL( 25 )
#define ASN1_C_VISIBLE_STRING   UNIVERSAL( 26 )
#define ASN1_C_GENERAL_STRING   UNIVERSAL( 27 )

#endif /* end of file moss_asn1_types.h */

