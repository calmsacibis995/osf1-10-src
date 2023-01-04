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
 *	@(#)$RCSfile: ether.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:20:25 $
 */ 
/*
 */
/*
 * OSF/1 Release 1.0
 */

/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 * @(#) from SUN 1.5
 */

#ifndef _rpcsvc_ether_h
#define _rpcsvc_ether_h

#define NPROTOS 6
#define NDPROTO 0
#define ICMPPROTO 1
#define UDPPROTO 2
#define TCPPROTO 3
#define ARPPROTO 4
#define OTHERPROTO 5

#define NBUCKETS 16
#define MINPACKETLEN 60
#define MAXPACKETLEN 1514
#define BUCKETLNTH ((MAXPACKETLEN - MINPACKETLEN + NBUCKETS - 1)/NBUCKETS)

#define HASHSIZE 256

#define ETHERSTATPROC_GETDATA 1
#define ETHERSTATPROC_ON 2
#define ETHERSTATPROC_OFF 3
#define ETHERSTATPROC_GETSRCDATA 4
#define ETHERSTATPROC_GETDSTDATA 5
#define ETHERSTATPROC_SELECTSRC 6
#define ETHERSTATPROC_SELECTDST 7
#define ETHERSTATPROC_SELECTPROTO 8
#define ETHERSTATPROC_SELECTLNTH 9
#define ETHERSTATPROG 100010
#define ETHERSTATVERS 1

int xdr_etherstat();
int xdr_etherhbody();
int xdr_etherhmem();
int xdr_etherhtable();
int xdr_etheraddrs();
int xdr_addrmask();

/*
 * all ether stat's except src, dst addresses
 */
struct etherstat {
	struct timeval	e_time;
	unsigned long	e_bytes;
	unsigned long	e_packets;
	unsigned long	e_bcast;
	unsigned long	e_size[NBUCKETS];
	unsigned long	e_proto[NPROTOS];
};

/*
 * member of address hash table
 */
struct etherhmem {
	int ht_addr;
	unsigned ht_cnt;
	struct etherhmem *ht_nxt;
};

/*
 * src, dst address info
 */
struct etheraddrs {
	struct timeval	e_time;
	unsigned long	e_bytes;
	unsigned long	e_packets;
	unsigned long	e_bcast;
	struct etherhmem **e_addrs;
};

/*
 * for size, a_addr is lowvalue, a_mask is high value
 */
struct addrmask {
	int a_addr;
	int a_mask;		/* 0 means wild card */
};

extern char *protoname[];
extern int if_fd;

#endif /*!_rpcsvc_ether_h*/
