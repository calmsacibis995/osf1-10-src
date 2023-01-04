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
 * @(#)$RCSfile: snmp_def.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:23:30 $
 */
/*
**  Copyright (c) Digital Equipment Corporation, 1989, 1992
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
/****************************************************************************************************************************/
/* Created  3-OCT-1989 14:54:52 by VAX SDL V3.2-10     Source:  3-OCT-1989 14:53:46 USER$43:[SANKAR.DDM.SNMP]SNMP_DEF.SDL;2 */
/****************************************************************************************************************************/
 
/*** MODULE SNMP_DEF ***/
struct SNMP_BUFFER_INFO {               /*  SNMP_BUFFER_INFO                */
    unsigned short int snmp_w_maxstrlen; /* allocated buffer length         */
    unsigned short int snmp_w_curlen;   /* used length                      */
    char *snmp_a_pointer;               /* Ptr to name buffer               */
    } ;
/*                                                                          */
/*	Constants to define SNMP types..                                    */
/*                                                                          */
#define SNMP_K_DT_INTEGER 2             /* SNMP data type integer           */
#define SNMP_K_DT_OCTETSTR 4            /* SNMP data type octet string      */
#define SNMP_K_DT_NULL 5                /* SNMP data type null              */
#define SNMP_K_DT_OBJECTID 6            /* SNMP data type object Id         */
#define SNMP_K_DT_SEQUENCE 16           /* SNMP data type sequence          */
#define SNMP_K_DT_IPADDRESS 20          /* SNMP data type IP address        */
#define SNMP_K_DT_COUNTER 21            /* SNMP data type Counter           */
#define SNMP_K_DT_GAUGE 22              /* SNMP data type Gauge             */
#define SNMP_K_DT_TIMETICKS 23          /* SNMP data type Time Ticks        */
#define SNMP_K_DT_OPAQUE 24             /* SNMP data type Opaque            */

/*
 *	Special data type to indicate simply "use the DATATABLE
 *	Must be != any data type given special case handling in the
 *	routine which encodes Out_P.
 */
#define SNMP_K_DT_USETABLE 99


/*                                                                          */
/*	Constants to define SMI application data types..                    */
/*                                                                          */
#define APPL_K_IPADDRESS 0              /* SMI Application data type IP address */
#define APPL_K_COUNTER 1                /* SMI Application data type Counter */
#define APPL_K_GAUGE 2                  /* SMI Application data type Gauge  */
#define APPL_K_TIMETICKS 3              /* SMI Application data type Time Ticks */
#define APPL_K_OPAQUE 4                 /* SMI Application data type Opaque  */
#define SNMP_K_GETREQ_CONT 0            /* SNMP GetRequest PDU              */
#define SNMP_K_GETNEXT_CONT 1           /* SNMP GetNextRequest PDU          */
#define SNMP_K_GETRES_CONT 2            /* SNMP GetResponse PDU             */
#define SNMP_K_SETREQ_CONT 3            /* SNMP SetRequest PDU              */
#define SNMP_K_TRAP_CONT 4              /* SNMP Trap PDU                    */
/*                                                                          */
/*   Symbols for Pre defined parts of object Id in SNMP                     */
/*                                                                          */
#define SNMP_K_ISO 1                    /* ISO code                         */
#define SNMP_K_ORG 3                    /* Identified Organization code     */
#define SNMP_K_DOD 6                    /* DOD code                         */
#define SNMP_K_INTERNET 1               /* Internet code                    */
#define SNMP_K_MGMT 2                   /* Mgmt code                        */
#define SNMP_K_MIB 1                    /* MIB code                         */
#define SNMP_K_SYSTEM 1                 /* System code                      */
#define SNMP_K_INTERFACE 2              /* Interface code                   */
#define SNMP_K_AT 3                     /* At code                          */
#define SNMP_K_IP 4                     /* IP code                          */
#define SNMP_K_ICMP 5                   /* ICMP code                        */
#define SNMP_K_TCP 6                    /* TCP code                         */
#define SNMP_K_UDP 7                    /* UDP code                         */
#define SNMP_K_EGP 8                    /* EGP code                         */
/*                                                                          */
/*   Symbols for SNMP errors                                                */
/*                                                                          */
#define SNMP_K_NOERROR 0                /* no error                         */
#define SNMP_K_TOOBIG 1                 /* error : too big                  */
#define SNMP_K_NOSUCHNAME 2             /* error : no such name             */
#define SNMP_K_BADVALUE 3               /* error : bad value                */
#define SNMP_K_READONLY 4               /* error : read only                */
#define SNMP_K_GENERR 5                 /* error : General error            */
#define MCC_S_INVALID_PDU 4
#define MCC_S_INVALID_PROB 6
#define MCC_S_INVALID_ERROR 8
