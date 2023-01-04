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
 * @(#)$RCSfile: if_trn_sr.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1993/04/02 23
:17:59 $
 */
 /* ---------------------------------------------------------------------
  *
  * Modification History:
  * 	
  * 9-NOV-92	Vaios P. Papadimitriou 
  *		Julia Fey
  * Created this module for Token Ring Source Routing
  * ---------------------------------------------------------------------
  */

#ifndef _IF_TRN_SR_H_DEFINED
#define _IF_TRN_SR_H_DEFINED

#ifdef _KERNEL
#include "netinet/if_trn.h"
#include "netinet/in.h"
#else
#include <netinet/if_trn.h>
#include <netinet/in.h>
#endif

#define SRT_NBCKTS	256		/* SRT Number of buckets */
#define ARP_SRT_NBCKS	37		/* ARP SRT Number of buckets */

/*
 * Source Routing Table 
 */ 
struct srtab {
	struct ifnet *srt_ifp;		/* Network visible interface */
	struct in_addr srt_ip_addr;
	u_char srt_mac_addr[MAC_ADDR_LEN]; /* Destination MAC address */
	struct srtab *srt_next_hash;	/* Used for SR Table hashing function */
	struct trn_rif srtab_rif;
	short  srt_aging_timer;		/* Aging timer */
	u_char srt_status;		/* Entry Status */
	u_char srt_discovery_type;	/* The type of the Route Discovery used
					 * to create this table entry.  One 
					 * of these values: */
#define SRTAB_UNKNOWN     0
#define SRTAB_ARP         1
#define SRTAB_XID         2
#define SRTAB_ETHER_LOOP  3
};

/*
 * ARP request Source Routing Table 
 */ 
struct arpsrtab {
	struct ifnet *srt_ifp;		/* Network visible interface */
	struct in_addr srt_ip_addr;
	struct arpsrtab *srt_next_hash; 
	short  srt_aging_timer;		/* Aging timer */
	u_char srt_status;		/* Entry Status */
};


/*
 * Definition for Routing Type of the RI field 
 */
#define SR_SPECIFIC_ROUTE		0x0     /* 0xx Specifically Routed */
#define SR_ALL_ROUTE_EXPLORE		0x04	/* 10x All-Route Explorer */
#define SR_SPAN_TREE_EXPLORE		0x06	/* 11x Spanning Tree Explorer */

/* Definitions compatible with other SR implementations */
#define TRN_NON_BROADCAST		SR_SPECIFIC_ROUTE
#define TRN_ALL_BROADCAST		SR_ALL_ROUTE_EXPLORE	
#define TRN_SINGLE_BROADCAST  		SR_SPAN_TREE_EXPLORE	

/*
 * RIF Status Flags
 */
#define RIF_NOTUSE	0		/* No Path Known */
#define RIF_NOROUTE	1		/* No Path Known */
#define RIF_ONRING	2		/* On Ring - No Path Need -VPP */
#define RIF_HAVEROUTE	3		/* Have a Route stored */
#define RIF_REDISCOVER  4		/* Discovery in process */
#define RIF_STALE	5		/* Have Route, Aging Timer expired VPP*/


#if defined (__alpha)
typedef unsigned long	u_size64;
#elif define (mips)
typedef unsigned long long u_size64;
#endif

/* 
 * Declarations for Source Routing management.
 */
struct srt_attr {
	int     SR_aging_timer;		/* Route Cache Aging Timer */
	u_short SRDT_local;		/* Timer for OnRing discovery */
	u_short SRDT_extended;		/* Timer for Extended LAN discovery */ 
	int 	srt_bcktsize;		/* Size of the Hash buckets */
	int	SR_table_size;		/* Maximum allowed SR Table size */
	int 	srt_kcentry_timer;	/* Kill (zero) completed entry timer */
	int 	srt_kientry_timer;	/* Kill (zero) incomplete entry timer */
	u_size64 ARE_sent;		/* Number of ARE frames sent */
	u_size64 ARE_rcvd;		/* Number of ARE frames received */
	u_size64 SR_discover_fail;	/* Number route discovery failed */
	u_size64 srt_crt_entry_fail;	/* Number of entry creation failure */
};

/*
 * SR table entry for application
 */
struct srreq_tb {
        struct in_addr srt_ip_addr;
        u_char srt_mac_addr[MAC_ADDR_LEN];
        struct trn_rif srtab_rif;
        short  srt_aging_timer;
        u_char srt_status;
        u_char srt_discovery_type;
	struct srt_attr srt_attrs;
};

/*
 * Structure for ioctl requests to SR
 */
struct srreq {
	char	srreq_type;		/* SR ioctl request type */
	union {
		struct srreq_tb srtab;	/* SR table entry */
		struct srt_attr attrs;	/* SR attributes */
	} srreq_data;
};

/* 
 * Source Routing ioctls
 */
#define SR_RENT		1	/* Read SR table entry */
#define SR_RATTR	2	/* Read SR attributes */
#define SR_ZATTR	3	/* Zero SR attributes */
#define SR_SATTR	4	/* Set SR attributes */
#define SR_ISENAB	5	/* Is SR enabled */
#define SR_ENAB		6	/* Enable SR */
#define SR_DISAB	7	/* Disable SR */
#define SR_DELENT	8	/* Delete SR table entry */
#define SR_DISENT	9	/* Disable SR table entry */

/*
 * Context block used as a handle, when reading the entire SR Table.
 */
struct srt_context {
	int tab_element;
	char *entry_value;
};

#endif /* _IF_TRN_SR_H_DEFINED */

