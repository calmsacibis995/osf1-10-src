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
 * @(#)$RCSfile: moss_dna.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:02:48 $
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
 *    May 4th, 1990
 *
 * Revision History :
 *
 *    Ed Tan			08-Sep-1992
 *	Add definitions for SessionSelector, DNACMIPMessage,
 *	PresentationSelector, EthernetProtocolType, IEEE802SNMPPID,
 *	IPAddress
 *
 *    Steve Pitcher		01-Sep-1992
 *	Add definitions for EntityInstanceEvent and EntityClassEvent datatypes
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 */

#ifndef	MOSS_DNA_TYPES
#define MOSS_DNA_TYPES

#include "moss_asn1.h"

/*
 * Define the DNA encoding, though conforming to the rules used for ASN1 internal encoding.
 */

/* 
 * DSM-defined data types (Entity Model Ch. 11) 
 */

#define DNA_C_COUNTER16         APPLICATION( 40 ) 
#define DNA_C_COUNTER32         APPLICATION( 41 )
#define DNA_C_COUNTER64         APPLICATION( 42 )
#define DNA_C_COUNTER           APPLICATION( 42 )
#define DNA_C_OCTET             APPLICATION( 4 )

/* 
 * DSM-defined and DSSR-registered data types 
 */

/* ???
#define DNA_C_SEQUENCE          CONSTRUCTED( APPLICATION( 16 ) )
 */

#define DNA_C_CHAR_ABS_TIME     APPLICATION( 16 )
#define DNA_C_BIN_ABS_TIME      APPLICATION( 17 )
#define DNA_C_CHAR_REL_TIME     APPLICATION( 18 )
#define DNA_C_BIN_REL_TIME      APPLICATION( 19 )
#define DNA_C_LATIN1_STRING     APPLICATION( 20 )
#define DNA_C_SIMPLE_NAME       APPLICATION( 26 )
#define DNA_C_FULL_NAME         APPLICATION( 27 )
#define DNA_C_UID               APPLICATION( 28 ) /* DNA UID */
#define DNA_C_KNOWN             APPLICATION( 29 ) /* for wildcarding */
#define DNA_C_FULL_ENTITY_NAME  CONSTRUCTED( APPLICATION( 30 ) )
#define DNA_C_LOCAL_ENTITY_NAME CONSTRUCTED( APPLICATION( 31 ) )
#define DNA_C_DEFAULT_VALUE     APPLICATION( 32 )
#define DNA_C_VERSION           APPLICATION( 33 )
#define DNA_C_ID802             APPLICATION( 34 )
#define DNA_C_DTE_ADDRESS       APPLICATION( 35 )
#define DNA_C_FILE_SPEC         APPLICATION( 36 )
#define DNA_C_NSAP_ADDRESS      APPLICATION( 37 )
#define DNA_C_NET_ENT_TITLE     APPLICATION( 38 )
#define DNA_C_AREA_ADDRESS      APPLICATION( 39 )
#define DNA_C_ADDRESS_PREFIX    APPLICATION( 43 )
#define DNA_C_TOWER_SET         CONSTRUCTED( APPLICATION( 44 ) )
#define DNA_C_END_USER_SPEC     APPLICATION( 45 )
#define DNA_C_T_SELECTOR        APPLICATION( 46 )
#define DNA_C_PHASE_4_NAME      APPLICATION( 47 )
#define DNA_C_PHASE_4_ADDRESS   APPLICATION( 48 )
#define DNA_C_IMPLEMENTATION    CONSTRUCTED( APPLICATION( 49 ) )
#define DNA_C_VERSION_EDIT      CONSTRUCTED( APPLICATION( 50 ) )
#define DNA_C_COMPONENT_NAME    APPLICATION( 51 )
#define DNA_C_DNS_TIME_STAMP    APPLICATION( 52 )
#define DNA_C_ENTITY_CLASS      APPLICATION( 53 )
#define DNA_C_HEX_STRING        APPLICATION( 54 )
#define DNA_C_FLOOR             CONSTRUCTED( APPLICATION( 55 ) )
#define DNA_C_PROTOCOL_TOWER    CONSTRUCTED( APPLICATION( 56 ) )
#define DNA_C_SESSION_SELECTOR  APPLICATION( 57 )
#define DNA_C_DNA_CMIP_MESSAGE  APPLICATION( 58 )
#define DNA_C_PRESENT_SELECTOR  APPLICATION( 59 )
#define DNA_C_RANGE             CONSTRUCTED( APPLICATION( 60 ) )
#define DNA_C_ENTITY_CLASS_EVENT CONSTRUCTED( APPLICATION( 61 ) )
#define DNA_C_ENTITY_INSTANCE_EVENT CONSTRUCTED( APPLICATION( 62 ) )
#define DNA_C_IDENET_TYPE       APPLICATION( 66 )
#define DNA_C_ID802_SNAP        APPLICATION( 67 )
#define DNA_C_IP_ADDRESS        APPLICATION( 81 )

#endif /* end of file moss_dna_types.h */

