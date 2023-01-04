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
 * @(#)$RCSfile: iso_defs.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:00:52 $
 */
/*
 *     *sccsid = "@(#)iso_defs.h	1.0    DECWest Engineering     7/3/91";
 */
/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1990, 1991, 1992.
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
 *    ISO defs
 *
 * Author:
 *
 *    Kathy Faust; based on work by Steve Pitcher
 *
 * Date:
 *
 *    June 3, 1991
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, Feb 14, 1992
 *       Added defs for Internet and OIM-Naming
 *
 *  STP001		Steve Pitcher		6-Aug-1991
 *	Define the various Universal Attributes OIDs.
 *
 *  F-3		STP002	Steve Pitcher		8-Oct-1991
 *	Insert definitions for the Null Instance Attribute identifiers.
 *
 *  F-4		ND1051	Nestor Dutko		14-Jan-1992
 *	Add definitions for general_security which resides directly beneath
 *	EMA.
 */

#ifndef ISO_DEFS_H
#define ISO_DEFS_H

/* 
 *  OBJECT IDENTIFIER element values -- from Entity Model, T1.0.1, App A
 */

#define ISO_CODE 1
#define ISO 1
#define ISO_SEQ ISO_CODE
#define ISO_LENGTH 1

#define ICD_CODE 3
#define ICD 3
#define ICD_SEQ ISO_SEQ, ICD_CODE
#define ICD_LENGTH (ISO_LENGTH + 1)

#define ECMA_CODE 12
#define ECMA 12
#define ECMA_SEQ ICD_SEQ, ECMA_CODE
#define ECMA_LENGTH (ICD_LENGTH + 1)

#ifndef NO_NEW_CODES  /*----------------------------------------------------*/

#define MEMBER_CO_CODE 2
#define MEMBER_CO 2
#define MEMBER_CO_SEQ ECMA_SEQ, MEMBER_CO_CODE
#define MEMBER_CO_LENGTH (ECMA_LENGTH + 1)

#define DEC_CODE 1011
#define DEC 1011
#define DEC_SEQ MEMBER_CO_SEQ, DEC_CODE
#define DEC_LENGTH (MEMBER_CO_LENGTH + 1)

#else /* NO_NEW_CODES --------------------------*/

#define DEC_CODE 1011
#define DEC 1011
#define DEC_SEQ ECMA_SEQ, DEC_CODE
#define DEC_LENGTH (ECMA_LENGTH + 1)

#endif /* NO_NEW_CODES -----------------------------------------------------*/

#define EMA_CODE 2
#define EMA 2
#define EMA_SEQ DEC_SEQ, EMA_CODE
#define EMA_LENGTH (DEC_LENGTH + 1)

#define ENTITIES_CODE 1
#define ENTITIES 1
#define ENTITIES_SEQ EMA_SEQ, ENTITIES_CODE
#define ENTITIES_LENGTH (EMA_LENGTH + 1)

#define ATTRIBUTES_CODE 2
#define ATTRIBUTES 2
#define ATTRIBUTES_SEQ EMA_SEQ, ATTRIBUTES_CODE
#define ATTRIBUTES_LENGTH (EMA_LENGTH + 1)

#define ACTIONS_CODE 3
#define ACTIONS 3
#define ACTIONS_SEQ EMA_SEQ, ACTIONS_CODE
#define ACTIONS_LENGTH (EMA_LENGTH + 1)

#define ACTION_REQ_ARG_CODE 4
#define ACTION_REQUEST_ARG 4
#define ACTION_REQ_ARG_SEQ EMA_SEQ, ACTION_REQ_ARG_CODE
#define ACTION_REQ_ARG_LENGTH (EMA_LENGTH + 1)

#define ACTION_RESP_CODE 5
#define ACTION_RESPONSE 5
#define ACTION_RESP_SEQ EMA_SEQ, ACTION_RESP_CODE
#define ACTION_RESP_LENGTH (EMA_LENGTH + 1)

#define ACTION_RESP_ARG_CODE 6
#define ACTION_RESP_ARG 6
#define ACTION_RESP_ARG_SEQ EMA_SEQ, ACTION_RESP_ARG_CODE
#define ACTION_RESP_ARG_LENGTH (EMA_LENGTH + 1)

#define ACTION_EXCEP_CODE 7
#define ACTION_EXCEPTION 7
#define ACTION_EXCEP_SEQ EMA_SEQ, ACTION_EXCEP_CODE
#define ACTION_EXCEP_LENGTH (EMA_LENGTH + 1)

#define ACTION_EXCEP_ARG_CODE 8
#define ACTION_EXCEP_ARG 8
#define ACTION_EXCEP_ARG_SEQ EMA_SEQ, ACTION_EXCEP_ARG
#define ACTION_EXCEP_ARG_LENGTH (EMA_LENGTH + 1)

#define EVENT_CODE 9
#define EVENT 9
#define EVENT_SEQ EMA_SEQ, EVENT_CODE
#define EVENT_LENGTH (EMA_LENGTH + 1)

#define EVENT_ARG_CODE 10
#define EVENT_ARG 10
#define EVENT_ARG_SEQ EMA_SEQ, EVENT_ARG_CODE
#define EVENT_ARG_LENGTH (EMA_LENGTH + 1)

#define UNIV_ATTR_GROUP_CODE 11
#define UNIVERSAL_ATTR_GROUP 11
#define UNIV_ATTR_GROUP_SEQ EMA_SEQ, UNIV_ATTR_GROUP_CODE
#define UNIV_ATTR_GROUP_LENGTH (EMA_LENGTH + 1)

#define ENTITY_CLASS_ATTR_GROUP_CODE 12
#define ENTITY_CLASS_GROUP 12
#define ENTITY_CLASS_ATTR_GROUP_SEQ EMA_SEQ, ENTITY_CLASS_ATTR_GROUP_CODE
#define ENTITY_CLASS_ATTR_GROUP_LENGTH (EMA_LENGTH + 1)

#define NULL_INST_ATTR_CODE 13
#define NULL_INST_ATTR_SEQ EMA_SEQ, NULL_INST_ATTR_CODE
#define NULL_INST_ATTR_LENGTH (EMA_LENGTH + 1)

#define NULL_ID_CODE 13
#define NULL_ID 13
#define NULL_ID_SEQ EMA_SEQ, NULL_ID_CODE
#define NULL_ID_LENGTH (EMA_LENGTH + 1)

#define EOECCS 126 /* End of Entity Class Code Sequence */

/*
 * EMA security definitions
 */

#define GENERAL_SECURITY_CODE 24
#define GENERAL_SECURITY 24
#define GENERAL_SECURITY_SEQ EMA_SEQ, GENERAL_SECURITY_CODE
#define GENERAL_SECURITY_LENGTH (EMA_LENGTH + 1)

/*
 * The three arcs for general security. All appear directly under 
 * EMA.GENERAL_SECURITY
 */

#define ACCESS_CONTROL_CODE 1
#define ACCESS_CONTROL 1
#define ACCESS_CONTROL_SEQ GENERAL_SECURITY_SEQ, ACCESS_CONTROL_CODE
#define ACCESS_CONTROL_LENGTH (GENERAL_SECURITY_LENGTH + 1)

#define IDENTITY_CODE 2
#define IDENTITY 2
#define IDENTITY_SEQ GENERAL_SECURITY_SEQ, IDENTITY_CODE
#define IDENTITY_LENGTH (GENERAL_SECURITY_LENGTH + 1)

#define ACCESS_CODE 3
#define ACCESS 3
#define ACCESS_SEQ GENERAL_SECURITY_SEQ, ACCESS_CODE
#define ACCESS_LENGTH (GENERAL_SECURITY_LENGTH + 1)

/*
 * The following are things found under EMA.GENERAL_SECURITY.ACCESS_CONTROLS
 */
#define DCE_ACL_CODE 1
#define DCE_ACL 1
#define DCE_ACL_SEQ ACCESS_CONTROL_SEQ, DCE_ACL_CODE
#define DCE_ACL_LENGTH (ACCESS_CONTROL_LENGTH + 1)

#define VMS_SOGW_PROTECTION_CODE 2
#define VMS_SOGW_PROTECTION 2
#define VMS_SOGW_PROTECTION_SEQ ACCESS_CONTROL_SEQ, VMS_SOGW_PROTECTION_CODE
#define VMS_SOGW_PROTECTION_LENGTH (ACCESS_CONTROL_LENGTH + 1)

#define VMS_IDENTIFIER_ACL_CODE 3
#define VMS_IDENTIFIER_ACL 3
#define VMS_IDENTIFIER_ACL_SEQ ACCESS_CONTROL_SEQ, VMS_IDENTIFIER_ACL_CODE
#define VMS_IDENTIFIER_ACL_LENGTH (ACCESS_CONTROL_LENGTH + 1)

#define POSIX_ACL_CODE 4
#define POSIX_ACL 4
#define POSIX_ACL_SEQ ACCESS_CONTROL_SEQ, POSIX_ACL_CODE
#define POSIX_ACL_LENGTH (ACCESS_CONTROL_LENGTH + 1)

#define DNS_ACL_CODE 5
#define DNS_ACL 5
#define DNS_ACL_SEQ ACCESS_CONTROL_SEQ, DNS_ACL_CODE
#define DNS_ACL_LENGTH (ACCESS_CONTROL_LENGTH + 1)

#define VMS_OWNER_CODE 6
#define VMS_OWNER 6
#define VMS_OWNER_SEQ ACCESS_CONTROL_SEQ, VMS_OWNER_CODE
#define VMS_OWNER_LENGTH (ACCESS_CONTROL_LENGTH + 1)


/*
 * The following are things found under EMA.GENERAL_SECURITY.IDENTITY
 */

#define X500_NAME_CODE 1
#define X500_NAME 1
#define X500_NAME_SEQ IDENTITY_SEQ, X500_NAME_CODE
#define X500_NAME_LENGTH (IDENTITY_LENGTH + 1)

#define DASS_CERTIFICATE_CODE 2
#define DASS_CERTIFICATE 2
#define DASS_CERTIFICATE_SEQ IDENTITY_SEQ, DASS_CERTIFICATE_CODE
#define DASS_CERTIFICATE_LENGTH (IDENTITY_LENGTH + 1)

#define DCE_CERTIFICATE_CODE 3
#define DCE_CERTIFICATE 3
#define DCE_CERTIFICATE_SEQ IDENTITY_SEQ, DCE_CERTIFICATE_CODE
#define DCE_CERTIFICATE_LENGTH (IDENTITY_LENGTH + 1)

#define VMS_CREDENTIAL_ID_CODE 4
#define VMS_CREDENTIAL_ID 4
#define VMS_CREDENTIAL_ID_SEQ IDENTITY_SEQ, VMS_CREDENTIAL_ID_CODE
#define VMS_CREDENTIAL_ID_LENGTH (IDENTITY_LENGTH + 1)

#define NODE_USER_CODE 5
#define NODE_USER 5
#define NODE_USER_SEQ IDENTITY_SEQ, NODE_USER_CODE
#define NODE_USER_LENGTH (IDENTITY_LENGTH + 1)

#define SNMP_CERTIFICATE_CODE 6
#define SNMP_CERTIFICATE 6
#define SNMP_CERTIFICATE_SEQ IDENTITY_SEQ, SNMP_CERTIFICATE_CODE
#define SNMP_CERTIFICATE_LENGTH (IDENTITY_LENGTH + 1)

/*
 * The following are things found under EMA.GENERAL_SECURITY.ACCESS they should
 * map on a one-to-one basis with object ids under 
 * EMA.GENERAL_SECURITY.ACCESS_CONTROLS
 */

#define DCE_RIGHTS_CODE 1
#define DCE_RIGHTS 1
#define DCE_RIGHTS_SEQ ACCESS_SEQ, DCE_RIGHTS_CODE
#define DCE_RIGHTS_LENGTH (ACCESS_LENGTH + 1)

#define VMS_ACCESS_CODE 2
#define VMS_ACCESS 2
#define VMS_ACCESS_SEQ ACCESS_SEQ, VMS_ACCESS_CODE
#define VMS_ACCESS_LENGTH (ACCESS_LENGTH + 1)

/*
 * The following are not declared as the access for UIC and Identifier ACL
 * based protections MUST be the same.
 *
 *  #define VMS_IDACL_ACCESS_CODE 3
 *  #define VMS_IDACL_ACCESS 3
 *  #define VMS_IDACL_ACCESS_SEQ ACCESS_SEQ, VMS_IDACL_ACCESS_CODE
 *  #define VMS_IDACL_ACCESS_LENGTH (ACCESS_LENGTH + 1)
 *
 */

#define POSIX_ACCESS_CODE 4
#define POSIX_ACCESS 4
#define POSIX_ACCESS_SEQ ACCESS_SEQ, POSIX_ACCESS_CODE
#define POSIX_ACCESS_LENGTH (ACCESS_LENGTH + 1)

#define DNS_ACCESS_CODE 5
#define DNS_ACCESS 5
#define DNS_ACCESS_SEQ ACCESS_SEQ, DNS_ACCESS_CODE
#define DNS_ACCESS_LENGTH (ACCESS_LENGTH + 1)

#define NODE_CODE 1
#define NODE 1
#define NODE_CLASS_SEQ NODE_CODE
#define NODE_CLASS_LENGTH 1
#define NODE_SEQ ENTITIES_SEQ, NODE_CLASS_SEQ
#define NODE_LENGTH (ENTITIES_LENGTH + NODE_CLASS_LENGTH)

#define NODE_NAME_CODE 1
#define NODE_NAME 1
#define NODE_NAME_SEQ ATTRIBUTES_SEQ, NODE_CLASS_SEQ, EOECCS, NODE_NAME_CODE
#define NODE_NAME_LENGTH (ATTRIBUTES_LENGTH + NODE_CLASS_LENGTH + 1 + 1)

/*
 * Define the EMA Universal Group Attribute Codes
 */

#define GRP_ALL_CODE 0
#define GRP_ALL 0
#define GRP_IDENTIFIERS_CODE 1
#define GRP_IDENTIFIERS 1
#define GRP_STATUS_CODE 2
#define GRP_STATUS 2
#define GRP_COUNTERS_CODE 3
#define GRP_COUNTERS 3
#define GRP_CHARACTERISTICS_CODE 4
#define GRP_CHARACTERISTICS 4
#define GRP_MIN GRP_ALL_CODE
#define GRP_MAX GRP_CHARACTERISTICS_CODE

#define UNIV_ALL_GROUP_SEQ UNIV_ATTR_GROUP_SEQ, GRP_ALL_CODE
#define UNIV_ALL_GROUP_LENGTH (UNIV_ATTR_GROUP_LENGTH + 1)

#define UNIV_ID_GROUP_SEQ UNIV_ATTR_GROUP_SEQ, GRP_IDENTIFIERS_CODE
#define UNIV_ID_GROUP_LENGTH (UNIV_ATTR_GROUP_LENGTH + 1)

#define UNIV_STATUS_GROUP_SEQ UNIV_ATTR_GROUP_SEQ, GRP_STATUS_CODE
#define UNIV_STATUS_GROUP_LENGTH (UNIV_ATTR_GROUP_LENGTH + 1)

#define UNIV_COUNTER_GROUP_SEQ UNIV_ATTR_GROUP_SEQ, GRP_COUNTERS_CODE
#define UNIV_COUNTER_GROUP_LENGTH (UNIV_ATTR_GROUP_LENGTH + 1)

#define UNIV_CHAR_GROUP_SEQ UNIV_ATTR_GROUP_SEQ, GRP_CHARACTERISTICS_CODE
#define UNIV_CHAR_GROUP_LENGTH (UNIV_ATTR_GROUP_LENGTH + 1)


/*
 * Object identifier lengths
 */

#define EMA_PREFIX_LENGTH EMA_LENGTH

#define IS_UNIVERSAL( value )         ( ( value >> 30 ) == 0 )
#define IS_APPLICATION( value )       ( ( value >> 30 ) == 1 )
#define IS_CONTEXT_SPECIFIC( value )  ( ( value >> 30 ) == 2 )
#define IS_PRIVATE( value )           ( ( value >> 30 ) == 3 )
#define TAG_VALUE( value )            ( value & 0x1fffffff )

/*
 *   Inserted SNMP codes
 */

#define DOD_CODE 6
#define DOD_SEQ ICD_SEQ, DOD_CODE
#define DOD_LENGTH (ICD_LENGTH + 1)

#define INTERNET_CODE 1
#define INTERNET_SEQ DOD_SEQ, INTERNET_CODE
#define INTERNET_LENGTH (DOD_LENGTH + 1)

/*
 *  Internet management and OIM-naming codes 
 */

#define MGMT_CODE 2
#define MGMT 2
#define MGMT_SEQ INTERNET_SEQ, MGMT_CODE
#define MGMT_LENGTH (INTERNET_LENGTH + 1)

#define MIB_II_CODE 1
#define MIB_II 1
#define MIB_II_SEQ MGMT_SEQ, MIB_II_CODE
#define MIB_II_LENGTH (MGMT_LENGTH + 1)

#define OIM_CODE 9
#define OIM 9
#define OIM_SEQ MIB_II_SEQ, OIM_CODE
#define OIM_LENGTH (MIB_II_LENGTH + 1)

#define OIM_ATTRIBUTES_CODE 6
#define OIM_ATTRIBUTES 6
#define OIM_ATTRIBUTES_SEQ OIM_SEQ, OIM_ATTRIBUTES
#define OIM_ATTRIBUTES_LENGTH (OIM_LENGTH + 1)

/*
 *  End Internet management and OIM-naming codes
 */

#define PRIVATE_CODE 4
#define PRIVATE_SEQ INTERNET_SEQ, PRIVATE_CODE
#define PRIVATE_LENGTH (INTERNET_LENGTH + 1)

#define ENTERPRISES_CODE 1
#define ENTERPRISES_SEQ PRIVATE_SEQ, ENTERPRISES_CODE
#define ENTERPRISES_LENGTH (PRIVATE_LENGTH + 1)

#define DEC_DOD_CODE 36
#define DEC_DOD_SEQ ENTERPRISES_SEQ, DEC_DOD_CODE
#define DEC_DOD_LENGTH (ENTERPRISES_LENGTH + 1)

#define EMA_DOD_CODE 2
#define EMA_DOD_SEQ DEC_DOD_SEQ, EMA_DOD_CODE
#define EMA_DOD_LENGTH (DEC_DOD_LENGTH + 1)

#define SYSOBJID_CODE 18
#define SYSOBJID_CLASS_SEQ SYSOBJID_CODE
#define SYSOBJID_CLASS_LENGTH 1
#define SYSOBJID_SEQ EMA_DOD_SEQ, SYSOBJID_CLASS_SEQ
#define SYSOBJID_LENGTH (EMA_DOD_LENGTH + SYSOBJID_CLASS_LENGTH)

/**** CA V1.0 DSRG Registered SysObjId OID: ==> "#define ULTRIX_CODE 25" ****/
#define ULTRIX_CODE 26         /* CA V1.1 DSRG Registered SysObjId OID (26) */
#define ULTRIX_CLASS_LENGTH 1
#define ULTRIX_SEQ SYSOBJID_SEQ, ULTRIX_CODE
#define ULTRIX_LENGTH (SYSOBJID_LENGTH + ULTRIX_CLASS_LENGTH)

/*
 *   End of SNMP codes
 */

/*
|==============================================================================
|  SNMP Event Argument ("Well Known") OID Prefixes
|
|  The following definitions are "well-known" OIDs needed to identify the
|  SNMPv1 event arguments in the Event Parameters AVL and in the EVD post_event
|  API function call.
|
|==============================================================================
*/

#define EVD_TRAP_PREFIX_CODE 129
#define EVD_TRAP_PREFIX 129
#define EVD_TRAP_PREFIX_SEQ EMA_SEQ, EVD_TRAP_PREFIX_CODE
#define EVD_TRAP_PREFIX_LENGTH (EMA_LENGTH + 1)


#define PREFIX_EVENTTYPE_CODE      6
#define PREFIX_EVENTTYPE_SEQ       EVD_TRAP_PREFIX_SEQ, PREFIX_EVENTTYPE_CODE
#define PREFIX_EVENTTYPE_LENGTH    (EVD_TRAP_PREFIX_LENGTH + 1)         

#define PREFIX_ENTERPRISE_CODE     7
#define PREFIX_ENTERPRISE_SEQ      EVD_TRAP_PREFIX_SEQ, PREFIX_ENTERPRISE_CODE
#define PREFIX_ENTERPRISE_LENGTH   (EVD_TRAP_PREFIX_LENGTH + 1)

#define PREFIX_AGENTADDR_CODE      8
#define PREFIX_AGENTADDR_SEQ       EVD_TRAP_PREFIX_SEQ, PREFIX_AGENTADDR_CODE
#define PREFIX_AGENTADDR_LENGTH    (EVD_TRAP_PREFIX_LENGTH + 1)

#define PREFIX_GENERICTRAP_CODE    9
#define PREFIX_GENERICTRAP_SEQ     EVD_TRAP_PREFIX_SEQ, PREFIX_GENERICTRAP_CODE
#define PREFIX_GENERICTRAP_LENGTH  (EVD_TRAP_PREFIX_LENGTH + 1)

#define PREFIX_SPECIFICTRAP_CODE   10
#define PREFIX_SPECIFICTRAP_SEQ    EVD_TRAP_PREFIX_SEQ, PREFIX_SPECIFICTRAP_CODE
#define PREFIX_SPECIFICTRAP_LENGTH (EVD_TRAP_PREFIX_LENGTH + 1)

#define PREFIX_EVENTTIME_CODE      11
#define PREFIX_EVENTTIME_SEQ       EVD_TRAP_PREFIX_SEQ, PREFIX_EVENTTIME_CODE
#define PREFIX_EVENTTIME_LENGTH    (EVD_TRAP_PREFIX_LENGTH + 1)

#define PREFIX_VARBINDARG_CODE     12
#define PREFIX_VARBINDARG_SEQ      EVD_TRAP_PREFIX_SEQ, PREFIX_VARBINDARG_CODE
#define PREFIX_VARBINDARG_LENGTH   (EVD_TRAP_PREFIX_LENGTH + 1)


#endif /* end of file iso_defs.h */

