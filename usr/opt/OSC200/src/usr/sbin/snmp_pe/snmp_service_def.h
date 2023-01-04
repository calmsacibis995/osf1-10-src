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
 * @(#)$RCSfile: snmp_service_def.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/20 18:32:54 $
 */
/*
**  Copyright (c) Digital Equipment Corporation, 1992
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

#define SNMP_SERVICE_DEF	1
#ifndef CONTEXT_DEF
#define SNMP_MAX_ATTR_ENTRIES	8
#endif

#define	SNMP_MAX_REQUEST_SIZE	485
#define	SNMP_MAX_RESPONSE_SIZE	1500
#define	SNMP_MAX_COMMUNITY_SIZE	32
#define	SNMP_MAX_TREE_ENTRIES	32
#define	SNMP_MAX_ATTR_ENTRIES	8

typedef unsigned char	SNMPRequest[ SNMP_MAX_REQUEST_SIZE ];
typedef unsigned char	SNMPResponse[ SNMP_MAX_RESPONSE_SIZE ];
typedef unsigned char	SNMPCom[ SNMP_MAX_COMMUNITY_SIZE ];


struct	ObjectID	{

			/*
			 *  elements start @ 1.  tree[ 0 ] should always
			 *  = entries.  (Used this way by low-level routines.)
			 */
			unsigned int	entries;
			unsigned int	tree[ SNMP_MAX_TREE_ENTRIES ];

			};


struct	AttributeInfo	{
			BOOLEAN		valid;

			unsigned int	entries;
			unsigned int	idx;
			unsigned int	error_idx;

			unsigned int	id_list[ SNMP_MAX_ATTR_ENTRIES ];
			unsigned int	reason_list[ SNMP_MAX_ATTR_ENTRIES ];

		struct AttributeProfile	*profile_list[ SNMP_MAX_ATTR_ENTRIES ];
			SNMPResponse	value_list[ SNMP_MAX_ATTR_ENTRIES ];
			MCC_T_Descriptor desc_list[ SNMP_MAX_ATTR_ENTRIES ];
			};
