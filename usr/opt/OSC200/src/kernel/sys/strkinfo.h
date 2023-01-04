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
 *  @(#)$RCSfile: strkinfo.h,v $ $Revision: 4.4.8.4 $ (DEC) $Date: 1993/06/23 14:05:01 $
 */ 
/*
 */

/*
 * 24-Jun-91 walker
 *      Added ID support
 *
 * 06-Jun-91 wca
 *	Added STREAMS kernel support
 */

#ifndef _STRKINFO_H
#define _STRKINFO_H

#include <sys/kernel.h>
#include <mach/host_info.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/if_ether.h>

/*
 * kinfo module ID
 */
#define KINFO_ID             5020
#define KINFO_NAME           "kinfo"
#define KINFO_VERSION        1

/*
 * ioctl commands
 */
#define KINFO_GET_SYSTEM     1
#define KINFO_GET_INTERFACES 2
#define KINFO_GET_ICMP       3
#define KINFO_GET_TCP        4
#define KINFO_GET_IP         5
#define KINFO_GET_IP_ROUTING 6
#define KINFO_GET_AT         7
#define KINFO_GET_UDP        8
#define KINFO_GET_IF_NAMES   9
#define KINFO_GET_ID        10
/*
 * Block off 100-199 for STREAMS ioctls
 */
#define KINFO_GET_STR_CFG    100
#define KINFO_GET_STR_STATS  102          /* Get STREAMS stats information */

/*
 * kinfo limits
 */
#define MAX_KINFO_STREAMS    10
#define MAX_BUF              4096
#define MAX_KINFO_BUF        MAX_BUF 

#define KINFO_HI_MARK        2048
#define KINFO_LO_MARK        128

/*
 * information for the system group
 */
#define MAX_CPUNAME_LEN 256
typedef struct _system_blk {
       char hostname[MAXHOSTNAMELEN];
       int  hostnamelen;
       char cputype[MAX_CPUNAME_LEN];
       char kernel_version[KERNEL_VERSION_MAX];
       struct timeval bootime;
} system_blk;


/*
 ********* information for the interfaces group (only interfaces using IP) ****
 */
#define MAX_IFNAME_LEN 12
#define MAX_IFVER_LEN 100
#define MAX_IF_INFOS ((MAX_KINFO_BUF-sizeof(struct if_blk_hdr))/sizeof(struct if_info_blk))

struct if_blk_hdr {
       int total_cnt;           /* in/out: number info blks provided so far */
       int curr_cnt;            /* out: number of info blks in this packet */
       int more;                /* out: true if all info blks couldn't fit */
};

struct if_info_blk {
       char if_name[MAX_IFNAME_LEN];
       char if_version[MAX_IFVER_LEN];
       u_char if_type;
       struct in_addr ip_addr;
       int if_unit;
       int if_mtu;
       int if_operstatus;
       int if_speed;
       struct timeval if_lastchange;
       int if_inoctets;
       int if_indiscards;
       int if_inerrors;
       int if_inmcasts;
       int if_inucasts;
       int if_outmcasts;
       int if_outucasts;
       int if_noproto;
       int if_outoctets; 
       int if_outerrors;
       int if_outqlen;
       int if_outdiscards;
};

typedef struct _if_blk {
       struct if_blk_hdr  hdr;
       struct if_info_blk info[MAX_IF_INFOS];
} if_blk;


/*
 **************** information for the icmp group *********************
 */
typedef struct _icmp_blk {
       int in_msgs;
       int in_icmp_errors;
       int in_unreach;
       int in_timeexcds;
       int in_parmprobs;
       int in_srcquenchs;
       int in_redirects;
       int in_echos;
       int in_echoreps;
       int in_tstamps;
       int in_tstampreply;
       int in_maskreqs;
       int in_maskreps;
       int out_msgs;
       int out_errors;
       int out_unreach;
       int out_timeexcds;
       int out_parmprobs;
       int out_srcquenchs;
       int out_redirects;
       int out_echos;
       int out_echoreps;
       int out_tstamps;
       int out_tstampreply;
       int out_maskreqs;
       int out_maskreps;
} icmp_blk;

/*
 *********** information for the tcp group *************
 */
#define MAX_TCP_INFOS ((MAX_KINFO_BUF-sizeof(struct tcp_blk_hdr))/sizeof(struct tcp_info_blk))

struct tcp_blk_hdr{
       int active_opens;
       int passive_opens;
       int attempt_fails;
       int estab_resets;
       int curr_estab;
       int in_segs;
       int out_segs;
       int retrans_segs;
       int in_errors;
       int out_rsts;
       int total_cnt;           /* in/out: number info blks provided so far */
       int curr_cnt;            /* out: number of info blks in this packet */
       int more;                /* out: true if all info blks couldn't fit */
};

struct tcp_info_blk {
       int            state;
       struct in_addr local_addr;
       int            local_port;
       struct in_addr rem_addr;
       int            rem_port;
};

typedef struct _tcp_blk {
       struct tcp_blk_hdr hdr;
       struct tcp_info_blk info[MAX_TCP_INFOS];
} tcp_blk;

/*
 ************* information for the ip group *************************
 */
typedef struct _ip_blk {
       int forwarding;
       int default_ttl;
       int in_receives;
       int in_hdr_errors;
       int in_addr_errors;
       int forw_datagrams;
       int in_delivers;
       int out_requests;
       int out_discards;
       int out_noroutes;
       int reasm_tout;
       int reasm_reqds;
       int reasm_OKs;
       int reasm_fails;
       int frag_OKs;
       int frag_fails;
       int frag_creates;
} ip_blk;

/*
 ************** information for the ip routing group **********************
 */
#define MAX_IP_ROUTE_INFOS ((MAX_KINFO_BUF-sizeof(struct ip_routing_hdr))/sizeof(struct ip_routing_info_blk))

struct ip_routing_hdr {
       int total_cnt;           /* in/out: number info blks provided so far */
       int curr_cnt;            /* out: number of info blks in this packet */
       int more;                /* out: true if all info blks couldn't fit */
};

struct ip_routing_info_blk {
       struct in_addr rt_dst;
       struct in_addr rt_next_hop;
#ifdef __alpha
/* alpha jestabro - differentiate field from macro for compiler problem */
       struct in_addr rt_maskx;
#else
       struct in_addr rt_mask;
#endif /* __alpha */
       struct in_addr interface_ipaddr;
       int rt_flags;
       int if_index;
};

typedef struct _ip_routing_blk {
       struct ip_routing_hdr hdr;
       struct ip_routing_info_blk info[MAX_IP_ROUTE_INFOS];
} ip_routing_blk;

/*
 *************** information for the arp group **********************
 */
#define MAX_ARP_INFOS ((MAX_KINFO_BUF-sizeof(struct arp_hdr))/sizeof(struct arp_info_blk))

struct arp_hdr {
       int total_cnt;           /* in/out: number info blks provided so far */
       int curr_cnt;            /* out: number of info blks in this packet */
       int more;                /* out: true if all info blks couldn't fit */
};

struct arp_info_blk {
        int if_index;
	u_char hw_addr[14];
	struct in_addr inet_addr;
	int arp_flags;
};

typedef struct _arp_blk {
       struct arp_hdr      hdr;
       struct arp_info_blk info[MAX_ARP_INFOS];
} arp_blk;

/*
 ************ information for the udp group *********************
 */
#define MAX_UDP_INFOS ((MAX_KINFO_BUF-sizeof(struct udp_hdr))/sizeof(struct udp_info_blk))

struct udp_hdr {
       int in_datagrams;
       int no_ports;
       int in_errors;
       int out_datagrams;
       int total_cnt;           /* in/out: number info blks provided so far */
       int curr_cnt;            /* out: number of info blks in this packet */
       int more;                /* out: true if all info blks couldn't fit */
};

struct udp_info_blk {
       struct in_addr local_addr;
       int local_port;
};

typedef struct _udp_blk {
       struct udp_hdr      hdr;
       struct udp_info_blk info[MAX_UDP_INFOS];
} udp_blk;

/*
 ************  information for the network interface names **************
 */
#define MAX_IF_NAMES_INFOS ((MAX_KINFO_BUF-sizeof(struct if_names_hdr))/sizeof(struct if_names_info_blk))

struct if_names_hdr {
       int total_cnt;           /* in/out: number info blks provided so far */
       int curr_cnt;            /* out: number of info blks in this packet */
       int more;                /* out: true if all info blks couldn't fit */
};

struct if_names_info_blk {
       char if_name[MAX_IFNAME_LEN];
       int  if_unit;
};

typedef struct _if_names_blk {
       struct if_names_hdr      hdr;
       struct if_names_info_blk info[MAX_IF_NAMES_INFOS];
} if_names_blk;

/*
 ****** Structure for kinfo id - used by streams configuration program. ****
 */
typedef struct _id_blk {
        char name[100];
	int  version;
} id_blk;


/*
 * STREAMS kernel 'stuff' - wca
 */

/*
 ********  Structure which holds STREAMS configuration data for one entry ***
 */
typedef struct _str_cfg_blk {
	dev_t	scb_device;
#define SADF_DEVICE	0x00000001
#define SADF_MODULE	0x00000002
#define SADF_CLONE	0x00000004
	int	scb_flags;
	short	scb_mid;
	char	scb_name[FMNAMESZ+1];
} STR_CFG, str_cfg_blk;

typedef struct _sad_hdr {
	int	sad_cnt;
	dev_t	sad_clonedev;
} SAD_HDR, sad_hdr;

typedef struct _sad_blk {
	SAD_HDR	sad_blkhdr;
	STR_CFG sad_cfg[ (MAX_KINFO_BUF/sizeof(STR_CFG)) - 1 ];
} SAD, sad_blk;

/*
 * these defines are for backward compatability
 */
#define total_cnt      hdr.total_cnt
#define curr_cnt       hdr.curr_cnt
#define more           hdr.more
#define in_datagrams   hdr.in_datagrams
#define no_ports       hdr.no_ports
#define in_errors      hdr.in_errors
#define out_datagrams  hdr.out_datagrams
#define active_opens   hdr.active_opens
#define passive_opens  hdr.passive_opens
#define attempt_fails  hdr.attempt_fails
#define estab_resets   hdr.estab_resets
#define curr_estab     hdr.curr_estab
#define in_segs        hdr.in_segs
#define out_segs       hdr.out_segs
#define retrans_segs   hdr.retrans_segs
#define out_rsts       hdr.out_rsts

#endif



