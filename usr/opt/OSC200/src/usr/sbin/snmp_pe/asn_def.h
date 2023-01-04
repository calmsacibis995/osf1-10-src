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
 * @(#)$RCSfile: asn_def.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:22:50 $
 */
/*
**  Copyright (c) Digital Equipment Corporation, 1986, 1992
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*/

/*********

MODULE ASN_DEF.H - should be renamed to ASN1_DEF.H when you have time

ABSTRACT:

This module was originally produced from an SDL file, but since it is
not needed by applications which call the MCC_ASN1 routines and is 
only used internally to these routines, it is an ordinary .H file now.

This module defines MCC_ASN1 routine constants and a context block 
data structure used to track the progress of ASN.1 encoding build/parse.

AUTHOR:    Ben England

CREATION DATE:      4/18/86

MODIFICATION HISTORY: lots

***************************************************************/

/* define compile-time parameters for message builder                       */
#define MCC_K_ASN_MAXSIZ 32500          /* maximum buffer size < 2**15      */
#define MCC_K_ASN_NESTCONS 12           /* maximum nesting of constructors  */

/* note that this constant determines minimum size ASN.1 encoding buffer
 * that MCC_ASN1 routines will accept.
 */
#define MCC_K_ASN_SAFETY_MARGIN 10      /* reserve 10 bytes at end of buf   */

/*
 * this hack is just a way to tell MCC_ASN1_FND_TAG to go to the end of 
 * a constructor.
 */
#define MCC_K_ASN_EOC 16777216          /* bigger than largest IDCode       */

/*                                                                          */
/* Message Builder's data structure for tracking constructor lengths        */
/*                                                                          */
/* since constructor elements can contain other constructor elements,       */
/* and since each constructor may require a length field, a data structure  */
/* to track constructor lengths is required.  This structure is implemented */
/* like a stack.  When a constructor is started, a new constructor length cell */
/* is allocated with a PUSH operation, when a constructor is finished,      */
/* the cell is deallocated with a POP operation.  While the cell is active, */
/* calls to the message encoding routines add to the length value in the cell. */
/*                                                                          */
/* When a constructor is finished, the length of the constructor must then  */
/* be placed into the message.  The length of the constructor serves also   */
/* as a pointer to the buffer location to receive the length encoding.      */
/*                                                                          */
/* If a constructed encoding is indefinite-length, then the length is not   */
/* maintained, since it is not needed within the encoding.  The length cell */
/* is set to -1.                                                            */
/*                                                                          */
struct ASNContext {
    short int ConsLen [MCC_K_ASN_NESTCONS];
    unsigned char *ConsAddr [MCC_K_ASN_NESTCONS];
    short int TOS;
    unsigned char *p_Pos;
    unsigned char *p_Begin;
    unsigned short int BufSize;
    unsigned int LastTag;
    unsigned char BuildParse;
    unsigned char AtTag;
    } ;

/* if this is a VMS system, then you want to use the 
** microcode to pump them bits, otherwise do it by hand
** using the routine below.   LIB$MOVC3
** routine doesn't have problem with overlapping
** byte strings, which shouldn't happen anyway.
**
** ilv_bcopy is a routine in MCC_ASN1.C
*/
/* replaced by memcpy 01-aug-1991 REJK */
#define ILV_BCOPY memcpy
/*
** #ifdef vms
** #define ILV_BCOPY( p1, p2, len ) LIB$MOVC3( &(len), p2, p1 )
** #else
** #define ILV_BCOPY( p1, p2, len ) ilv_bcopy( p1, p2, len )
** #endif
*/

/*** begin added by Jesuraj APRil 18, 90 **/

#define REAL_BUFF_LEN 10
#define MCC_S_ASNEXPOUTOFRANGE 52867616
#define MCC_S_ASNCORRUPT 52878850

/** END */
/* DEC/CMS REPLACEMENT HISTORY, Element ASN_DEF.H*/
/* *1    14-DEC-1990 14:37:38 SANKAR "new cms entry"*/
/* DEC/CMS REPLACEMENT HISTORY, Element ASN_DEF.H*/
