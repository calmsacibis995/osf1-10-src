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
 * @(#)$RCSfile: snmppe_snmplib.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:33:02 $
 */
/*
 **  Copyright (c) Digital Equipment Corporation, 1991, 1992
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
 *
 * MODULE DESCRIPTION:
 *
 * Common Agent SNMP Protocol Engine
 *
 * Module SNMPPE.H
 *      Contains prototypes required by the SNMP PE code to
 *      access Arun Sankar' SNMP_LIB functions.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   October 1991
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *          This module is included into the compilations of modules that
 *          comprise the SNMP Protocol Engine for the Common Agent that
 *          need to invoke functions in "snmp_lib.c"
 *
 * History
 *      V0.0    October 1991               D. D. Burns
 */


/*
| Define a symbol to distinguish the compilers that allow argument lists in
| prototypes from those that don't.  Then define a macro to conditionalize
| prototype argument lists.  Note that two sets of parentheses are required.
| Example: char *f_foobar PROTOTYPE ((int *arg1, char arg2));
*/

#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
# define PROTOTYPE(args) args
#else
# define PROTOTYPE(args) ()
#endif

/* 
|
|  Function prototypes for "SNMP_LIB".
|
*/
man_status
SNMP_ENCODE_PDU PROTOTYPE((

MCC_T_Descriptor       	    *,  /* descriptor of output buffer */
unsigned int                *,  /* mcc error code return       */
unsigned int    	    *,  /* Protocal Version number     */
MCC_T_Descriptor       	    *,  /* descriptor for community    */
unsigned int    	    *,  /* GET, SET, GETNEXT           */
unsigned int    	    *,  /* SNMP request id             */
unsigned int    	    *,  /* SNMP error status           */
unsigned int    	    *,  /* SNMP error index            */
MCC_T_Descriptor            *   /* VarBind list in ASN.1 form  */
));


man_status
SNMP_DECODE_PDU PROTOTYPE((

MCC_T_Descriptor       	    *,  /* descriptor of input buffer  */
unsigned int                *,  /* mcc error code return       */
unsigned int    	    *,  /* Protocal Version number     */
MCC_T_Descriptor       	    *,  /* descriptor for community    */
unsigned int    	    *,  /* GET, SET, GETNEXT           */
unsigned int    	    *,  /* SNMP request id             */
unsigned int    	    *,  /* SNMP error status           */
unsigned int    	    *,  /* SNMP error index            */
MCC_T_Descriptor            *   /* VarBind list buffer descptor*/
));


man_status
SNMP_ENCODE_TRAP PROTOTYPE((

MCC_T_Descriptor	    *,  /* descriptor of  output buffer */
int                         *,  /* mcc error code return        */
int                          ,  /* trap type                    */
int                          ,  /* SNMP version number          */
MCC_T_Descriptor	    *,  /* descriptor for community name*/
object_id                   *,  /* OID for System Object        */
MCC_T_Descriptor	    *,  /* OCTET STRING of agent addr   */
unsigned int                 ,  /* time-up in timeticks         */
int                          ,  /* specific-trap for enterprise */
MCC_T_Descriptor            *   /* descriptor of varbind list   */
));


int
var_bind_list_build_init PROTOTYPE((
MCC_T_Descriptor            *
));

int
var_bind_list_build_end PROTOTYPE((
MCC_T_Descriptor            *
));

int
var_bind_list_parse_init PROTOTYPE((
MCC_T_Descriptor       	    *
));

int
encode_var_bindlist PROTOTYPE((
MCC_T_Descriptor 	    *,
struct ASNContext   *
));

int
decode_var_bindlist PROTOTYPE((
MCC_T_Descriptor 	    *,
struct ASNContext   *
));

int
get_ctx PROTOTYPE((
MCC_T_Descriptor 	    *,
struct ASNContext   **,
MCC_T_Descriptor 	    *
));

int
snmp_dump PROTOTYPE((
MCC_T_Descriptor 	    *
));

/* avl_to_asn1 - Encode AVL from Varbind List into ASN.1 */
int
avl_to_asn1 PROTOTYPE((
avl              *,  /* -> AVL containing the varbind list information */
MCC_T_Descriptor *,  /* -> MCC descriptor for buffer to receive ASN.1  */
man_status       *,  /* -> man_status cell to receive MOSS error code  */
BOOL             ,   /* TRUE: Starting Build on this call              */
BOOL                 /* TRUE: Ending Build on this call                */
));

/* asn1_to_avl - Decode ASN.1 for Varbind List into AVL */
int
asn1_to_avl PROTOTYPE((
MCC_T_Descriptor *,  /* -> MCC dsc for buffer to containing ASN.1      */
avl              **, /* ->> AVL to receive varbind list information    */
man_status       *,  /* -> man_status cell to receive MOSS error code  */
BOOL                 /* TRUE: Starting new parse on this call          */
));



