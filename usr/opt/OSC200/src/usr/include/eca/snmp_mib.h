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
 * @(#)$RCSfile: snmp_mib.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:49:03 $
 */
/*
**  Copyright (c) Digital Equipment Corporation, 1990,1992,1993
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
 *---------------------------------------------------------------------------
 *
 * Facility:
 *
 *    Management 
 *
 * Abstract:
 *
 *    This the header file containing information for asn1 encode and decode
 *    routines.
 *
 * Author:
 *
 *    Mary Walker
 *
 * Date:
 *
 *    October 4, 1990
 *
 * Revision History :
 *
 *    August 12, 1992  DEM   Add some fields in snmp_stats struct for storing
 *                           the system OID & agent start time for use by the
 *                           SNMP-PE when building trap PDUs.
 *    Sept 10, 1992    DEM   Added 'initialized' and 'coldStartSent' fields,
 *                           and definition for SNMP_IMOM_SHDMEM_INIT.
 */
/*
 * This file contains the necessary definitions for the SNMP MIB variables.
 * This file is used by both the SNMP PE and the Internet MOM
 */

#ifndef SNMP_MIB_H
#define SNMP_MIB_H

#define SHARED_MEM_KEY         1234
#define SNMP_IMOM_SHDMEM_INIT  699050  /* binary pattern '...101010101010' */


/*
 *  This is the snmp_pe stats structure.  We will keep the SNMP mib-variables
 *  here, as well as some variables that will make constructing TRAP PDUs
 *  accurate and efficient.
 */
struct snmp_stats {
    int    inpkts;	      /* number of snmp pkts in */
    int    outpkts;	      /* number of snmp pkts out */
    int    badvers;           /* pkts with bad version numbers */
    int    badcommname;       /* unknown community name */
    int    badcommuse;        /* bad community use */
    int    badASN1;           /* error parsing asn1 */
    int    badtype;           /* unknown PDU type */
    int    intoobig;          /* number of too big replies */
    int    innosuch;          /* number of nosuch replies */
    int    inbadvalue;        /* number of badvalue replies */
    int    inreadonly;        /* number of readonly replies */
    int    ingenerr;          /* number of generr replies */
    int    totreqvars;        /* total number of requested vars */
    int    totsetvars;        /* total number of set variables */
    int    ingetreq;          /* total number of get requests processed */
    int    ingetnxtreq;       /* total number of get-next requests processed*/
    int    insetreq;          /* total number of set requests processed */
    int    ingetresp;         /* total number of get responses processed */
    int    intraps;           /* total number of traps processed */
    int    outtoobig;         /* number of too big replies */
    int    outnosuch;         /* number of nosuch replies */
    int    outbadvalue;       /* number of badvalue replies */
    int    outreadonly;       /* number of readonly replies */
    int    outgenerr;         /* number of generr replies */
    int    outgetreq;         /* total number of get requests created */
    int    outgetnxtreq;      /* total number of get-next requests created */
    int    outsetreq;         /* total number of set requests created */
    int    outgetresp;        /* total number of get responses created */
    int    outtraps;          /* total number of traps created */
    int    enableauthtrap;    /* enable/disable auth traps */

    /* NEXT 2 FIELDS REQUIRED FOR TRAP PDUs; THEY ARE SET BY THE INET MOM    */
    int    sysuptime;         /* timestamp of when agent (IMOM) was started  */
    int    sysobjid[ULTRIX_LENGTH+1]; /* 1 extra for storing OID length.     */
                              /* Where to store the systems's object_id;     */
                              /* the 'count' (int) is at sysobjid[0],        */
                              /* followed by the actual 'value' itself       */
                              /* starting at sysobjid[1] (not a pointer).    */

    int    initialized;       /* Set to SNMP_IMOM_SHDMEM_INIT when segment   */
                              /* has been fully initialized, else 0          */
    int    coldStartSent;     /* 1 = coldStart TRAP has been sent, else 0    */
};

#endif /* SNMP_MIB_H */
