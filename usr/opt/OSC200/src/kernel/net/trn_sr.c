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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: trn_sr.c,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/12/20 20:56:49 $";
#endif

#ifdef __alpha
#include "machine/endian.h"
#endif

#include "kern/kalloc.h"

#include "sys/kernel.h"
#include "sys/time.h"
#include "sys/errno.h"
#include "sys/ioctl.h"
#include "sys/syslog.h"
#include "sys/mbuf.h"
#include "sys/socket.h"

#include "net/if.h"
#include "net/if_llc.h"
#include "net/netisr.h"
#include "net/net_globals.h"
#include "net/if_trn_sr.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/if_ether.h"
#include "netinet/if_trn.h"


extern void (*arpreq)();
void (*trn_init_dli)();

void sr_in_update(), arpsrtab_entry_new(), srtab_entry_refresh();
void updateip(), arp_discover(), sr_init(), srtab_clear(), arpsrtab_clear();
void dump_srtab(), dump_entry(), dump_arp_entry();
int sr_search_arp(), sr_search_xid(), build_STE(), build_ARE();
int srt_timeout(), xid_discover(), XID_frame(), sr_ioctl();
struct srtab *srtab_entry_new();
void sr_read_attrs(), sr_set_attrs(), sr_read_srtab_entry();
void sr_enable(), sr_disable(), sr_read_table();
int sr_entry_disable(), sr_entry_delete(), sr_find_entity();


/* NETSPL and NETSPLX are defined in net/net_globals.h */
#if NETSYNC_LOCK	/* do process sync with locks */
lock_data_t srt_thread_lock;
#define SRT_LOCKINIT()	lock_init2(&srt_thread_lock, TRUE, LTYPE_SRT)
#define SRT_LOCK()	{ s = splimp(); lock_write(&srt_thread_lock); }
#define SRT_UNLOCK()	{ lock_done(&srt_thread_lock); splx(s); }
#else
#define SRT_LOCKINIT()
#define SRT_LOCK()	{ s = splimp(); }
#define SRT_UNLOCK()	{ splx(s); }
#endif

/* timer values */
#define SRT_AGE_UPDATE 	1		/* Update the Aging Timer every 1 sec */
#define RD_TIMER_LOCAL	1
#define RD_TIMER_EXT	5
#define SR_AGING_TIMER	120
#define KCENTRY_TIMER	1200		/* kill completed entry in 20 min */
#define KIENTRY_TIMER	180		/* kill incomplete entry in 3 min */
#define MAX_BCKT_SIZE	4
#define MAX_SRTB_SIZE	1024

int total_entry = 0;		    /* Number of entries to check on timeout */
int srtb_actual_size;		    /* Number of SR Table allocated entries */
struct srt_attr srt_var;	    /* SR management structure */
struct srtab *G_srt_tab[SRT_NBCKTS];		/* SR Hash Table */

int arp_srt_bsize = MAX_BCKT_SIZE;			
struct arpsrtab *arp_srtab[ARP_SRT_NBCKS];	/* arp request SR table */
int srtab_entry_size = sizeof(struct srtab);    /* Initialize SR Table entry size -- This value may be changed by protocol users of Source Routing */

extern int SR_enabled;     	/* Indicates whether SR is enabled or not
				   defined in if_ethersubr.c  */
int srdebug = 0;
int first_time = 1;

#define SRTAB_MAC_HASH(macaddr) \
	((((u_short *)macaddr)[0] ^ \
                ((u_short *)macaddr)[1] ^ \
                        ((u_short *)macaddr)[2]) & (SRT_NBCKTS-1))

#define MAC_ADDR_CMP(addr1, addr2) \
        (((u_short *)addr1)[2] == ((u_short *)addr2)[2] \
            &&  ((u_short *)addr1)[1] == ((u_short *)addr2)[1] \
                &&  ((u_short *)addr1)[0] == ((u_short *)addr2)[0])

#define SRTAB_MAC_SEARCH(srm, macaddr, ifp) \
{ \
	for (srm = G_srt_tab[SRTAB_MAC_HASH(macaddr)]; srm != NULL; \
             srm = srm->srt_next_hash) \
        { \
                if (srm->srt_ifp == ifp && \
                    MAC_ADDR_CMP(macaddr, srm->srt_mac_addr)) \
                        break; \
        } \
}
	

#define SRTAB_IP_HASH(a) \
	((u_long)(a) % ARP_SRT_NBCKS)

#define ARP_SRTAB_IP_SEARCH(sri, addr, ifp) \
{ \
	for (sri = arp_srtab[SRTAB_IP_HASH(addr)]; sri != NULL; \
	     sri=sri->srt_next_hash) \
	    if (sri->srt_ifp == (ifp) && sri->srt_ip_addr.s_addr == (addr)) \
			break; \
}


/*
 *                      s r t a b _ u p d a t e
 *
 *      This routine will update the Source Routing Table and will be
 * called from ether_input(). It will be called as a result of receiving
 * either an XID frame (Route Discovery in Phase V), or an Ethernet Loopback
 * frame (Route Discovery in Phase IV) or an ARP frame (Route Discovery per
 * RFC1042) only. These frames may or may not contain Source Routing
 * information, that is they can be ARE, SRF or NSR frames.
 *
 *      Input : - A pointer to the network visible ifnet structure
 *              - A pointer to the extended (SR Info present) tokenring header
 *              - A pointer to an IP address (Non-Null for IP or ARP frames)
 *
 */
void
 srtab_update(ifp, trn_sr_hdr, ip_addr)
struct ifnet *ifp;
register struct trn_header *trn_sr_hdr;
register struct in_addr *ip_addr;
{

#ifdef SRDEBUG
	if (srdebug)
		log(LOG_INFO, "SR: srtab_update \n"); 
#endif

        if (trn_sr_hdr->trn_shost[0] & RIIndicator) /* SR information present */
        {
            if (!trn_sr_hdr->trn_hdr_rif.rif_lth) {
            /*
             * Length of SR Information field is zero.  Something is wrong.
             */
                 log(LOG_WARNING, "SR: %s%d: While Routing Information Indicator set, no SR field present.\n", ifp->if_name, ifp->if_unit);
                 return;
            }
	    
	    if (trn_sr_hdr->trn_hdr_rif.rif_rt == SR_ALL_ROUTE_EXPLORE)
		 srt_var.ARE_rcvd += 1;

	    trn_sr_hdr->trn_hdr_rif.rif_rt = SR_SPECIFIC_ROUTE;
            trn_sr_hdr->trn_hdr_rif.rif_dir = ~trn_sr_hdr->trn_hdr_rif.rif_dir;
						/* Reverse direction bit */
	    trn_sr_hdr->trn_shost[0] &= ~RIIndicator;	/* turn off SR indicator */

            sr_in_update(ifp, trn_sr_hdr->trn_shost, ip_addr,
			 &trn_sr_hdr->trn_hdr_rif);
        }
        else {            /* No SR Info present, frame from local ring node */
            sr_in_update(ifp, trn_sr_hdr->trn_shost, ip_addr, NULL);
        }

        return;
}


/*
 *			s r _ i n _ u p d a t e 
 * 
 * 	This routine updates the SR Table.
 * NULL p_sr_info means the frame source node is on the local ring.
 *
 *	Input : The network visible ifnet structure
 *		The MAC address to be used as search key
 * ---		The ip addr to be used as search key for ARP request table entry
 * 		The SR information
 *	        
 */
void
 sr_in_update(ifp, mac_addr, ip_addr, p_sr_info)
struct ifnet *ifp;
register u_char *mac_addr;
struct in_addr *ip_addr;
struct trn_rif *p_sr_info;
{
	register struct srtab *p_srt_entry;
	u_char sr_src_addr[MAC_ADDR_LEN];
	NETSPL_DECL(s)

	bcopy(mac_addr, &sr_src_addr[0], MAC_ADDR_LEN);

#ifdef SRDEBUG
	if (srdebug) {
		log(LOG_INFO, "SR: sr_in_update, mac=%s\n", ether_sprintf(sr_src_addr)); 
		if (ip_addr != NULL)
			log(LOG_INFO, "SR: sr_in_update, ip=%s\n", inet_ntoa(*ip_addr));
	}
#endif

	SRT_LOCK();
        /* 
	 * Use "mac_addr" as key to search the SR Table.
	 */
	SRTAB_MAC_SEARCH(p_srt_entry, (caddr_t)sr_src_addr, ifp);
	if (p_srt_entry == NULL)        /* Entry Not Found */
	{
	    /*
             * Create an entry in the SR Table
             */
	    p_srt_entry = srtab_entry_new(ifp, (caddr_t)sr_src_addr, NULL,
					  SRTAB_UNKNOWN);

	    if (p_srt_entry == NULL)
	    {
	        /* Unable to allocate a new entry; return w/ empty p_sr_info */
	        SRT_UNLOCK();
		return;
	    }

   	    /* new entry */

 	    if ((p_sr_info == NULL) || (p_sr_info->rif_lth <= 2)) 
	    {
		/* Frame came from local ring */
		p_srt_entry->srt_status = RIF_ONRING;
#ifdef SRDEBUG
	    	if (srdebug)
			log(LOG_INFO, "SR: sr_in_update onring\n");
#endif
	    } 
	    else 
	    {	/* Frame contained SR Info */
		p_srt_entry->srt_status = RIF_HAVEROUTE;
		bcopy(p_sr_info, &p_srt_entry->srtab_rif, 
		      p_sr_info->rif_lth); 	
#ifdef SRDEBUG
		if (srdebug)
			log(LOG_INFO, "SR: sr_in_update have route\n");
#endif
	    }
	} 
	else 
	{		
#ifdef SRDEBUG
		if (srdebug)
			log(LOG_INFO, "SR: sr_in_update found exist entry\n");
#endif
		/* 
		 * Entry found in SR Table 
	 	 * The policy here is to use the rif of the first response 
		 * packet.
		 */
	    	if ((p_sr_info == NULL) || (p_sr_info->rif_lth <= 2)) 
		{
		   /* Frame came from local ring. */
		   if (p_srt_entry->srt_status != RIF_ONRING) 
		   {
	    		p_srt_entry->srt_aging_timer = 0;
		   	p_srt_entry->srt_status = RIF_ONRING;
		   	bzero(&p_srt_entry->srtab_rif, sizeof(struct trn_rif));
#ifdef SRDEBUG
		   	if (srdebug)
				log(LOG_INFO, "SR: sr_in_update entry status set to on ring\n");
#endif
		   }
	    	}
	    	else 
		{
		   /* Frame came from remote ring. */
		   if (p_srt_entry->srt_status != RIF_HAVEROUTE) 
		   {
	    		p_srt_entry->srt_aging_timer = 0;
		   	p_srt_entry->srt_status = RIF_HAVEROUTE;
		   	bcopy(p_sr_info, &p_srt_entry->srtab_rif, 
				p_sr_info->rif_lth);
#ifdef SRDEBUG
			if (srdebug)
				log(LOG_INFO, "SR: sr_in_update entry status set to have route\n");
#endif
		   }
		}
	}
	if (ip_addr != NULL) {
		updateip(p_srt_entry, ip_addr, ifp);
	}
	SRT_UNLOCK();	
#ifdef SRDEBUG
	if (srdebug) {
		int i;
		i = SRTAB_MAC_HASH((caddr_t)sr_src_addr); 
		log(LOG_INFO, "SR: source routing entry at %d bucket\n", i);
		dump_entry(p_srt_entry);
		if (srdebug > 1)
			dump_srtab();
	}
#endif
	return;
}

			
extern int bootp_active;	/* whether we are currently using bootp */
 

/*
 *			s r _ s e a r c h
 *
 *	This routine will be called by ether_output() to search the SR Table 
 *  for all the protocol packets (IP, ARP, and DECnet).
 *
 *	Input : - A pointer to the visible network interface
 *		- The Ether type of the frame
 *		- A pointer to the data mbuf
 *		- The destination MAC address
 *		- The destination Inet address
 *		- A pointer to the SR Info to be returned
 *
 */
int 
 sr_search(ifp, type, m, macdst, inet_dst, p_sr_info)
register struct ifnet *ifp;
u_short type;
struct mbuf *m;
register u_char *macdst;
register struct in_addr *inet_dst;
register struct trn_rif *p_sr_info;
{
	u_char srt_dst[MAC_ADDR_LEN];
	struct arphdr *ah;
	int ret = 0;

#ifdef SRDEBUG
	if (srdebug)
		log(LOG_INFO, "SR: sr_search, type=0x%X \n", type); 
#endif

	bcopy(macdst, &srt_dst[0], MAC_ADDR_LEN);
	bzero(p_sr_info, sizeof(struct trn_rif));

	if (type == ETHERTYPE_ARP)
	{
		/* 
		 * Search the SR Table with MAC or IP address as key. 
		 * If an SR entry does not exist, create one
		 * and return zeroed p_sr_info indicates send arp 
		 * packet onring.  The timeout routine will resent
		 * an ARE arp packet.
		 * If an valid entry exists, return rif in p_sr_info.
		 */ 
#ifdef SRDEBUG
		if (srdebug) {
			log(LOG_INFO, "SR: sr_search, mac=%s\n", ether_sprintf(srt_dst)); 
			log(LOG_INFO, "SR: sr_search, ip=%s\n", inet_ntoa(*inet_dst)); 
		}
#endif
		ret = sr_search_arp(ifp, srt_dst, inet_dst, p_sr_info);
	} 
	else 	
	{	/* IP or DECnet/OSI frame */
	   if (srt_dst[0] & 0x1)
	   /*
            * Multicast address, send as STE frame.
            */
           {
		   if (bootp_active > 0)	/* SAS only, RFC 1532 */
			build_ARE(p_sr_info);
		   else
			build_STE(p_sr_info);
                   ret = 1;
#ifdef SRDEBUG
		   if (srdebug)
			log(LOG_INFO, "SR: Multicast address %s send as STE\n", ether_sprintf(srt_dst)); 
#endif
           }
	   else {
#ifdef SRDEBUG
		if (srdebug) {
			log(LOG_INFO, "SR: sr_search, mac=%s\n", ether_sprintf(srt_dst)); 
			if (type == ETHERTYPE_IP)
				log(LOG_INFO, "SR: sr_search, ip=%s\n", inet_ntoa(*inet_dst)); 
		}
#endif
	   	ret = sr_search_xid(ifp, m, srt_dst, inet_dst, p_sr_info);
	   }
	}

	if (p_sr_info->rif_rt == SR_ALL_ROUTE_EXPLORE)
		srt_var.ARE_sent += 1;

#ifdef SRDEBUG
	if (srdebug) {
		log(LOG_INFO, "SR: sr_search return %d\n", ret);
		if (srdebug > 1)
			dump_srtab();
	}
#endif
	return(ret);
}


/*
 *                      s r _ s e a r c h _ a r p
 *
 * 
 * If the dst MAC address is broadcast, this routine looks up 
 * the ARP request SR table.  If entry not found, it creates one.
 * If the dst MAC address is unicast, this routine will search through 
 * the SR Table to find a matched entry. If an entry exists, it appends
 * the RI field from the table to p_sr_info and return 1. If an entry 
 * does not exist, it creates one. 
 * It return zeroed p_sr_info for On Ring Route Discovery. When timeout, 
 * an ARE arp will be sent.
 *
 *      Input : -A pointer to the network visible interface (ifnet)
 *		-The destination MAC address (unicast or broadcast)
 *		-The destination INET address 
 *              -A pointer to the Routing Information to be filled
 *
 *      Returns: 1 Address was resolved in the SR Table 
 *               0 Address was not resolved in the SR Table
 */
int 
 sr_search_arp(ifp, dst, inet_dst, p_sr_info)
register struct ifnet *ifp;
register u_char *dst;
register struct in_addr *inet_dst;
register struct trn_rif *p_sr_info;
{
   register u_char srt_dst[MAC_ADDR_LEN];
   register struct srtab *p_srt_entry;
   register struct arpsrtab *p_asrt_entry;
   int	ret = 0;
   char *logmsg;
   NETSPL_DECL(s)

	
   bcopy(dst, &srt_dst[0], MAC_ADDR_LEN);

   if (srt_dst[0] & 0x1) {	/* broadcast MAC address */
#ifdef SRDEBUG
	if (srdebug)
		log(LOG_INFO, "SR: sr_search_arp/broadcast MAC address \n"); 
#endif

	SRT_LOCK();

arptab:

	ARP_SRTAB_IP_SEARCH(p_asrt_entry, inet_dst->s_addr, ifp);
	if (p_asrt_entry == NULL) {
	    /* Create an entry in ARP_SR Table use ip addr */
	    arpsrtab_entry_new(ifp,inet_dst);
	    SRT_UNLOCK();
	    return(ret);
	}
	/* entry found in ARP request SR table */
	p_asrt_entry->srt_aging_timer = 0;
	switch (p_asrt_entry->srt_status) {
	case RIF_STALE:
		p_asrt_entry->srt_status = RIF_NOROUTE;
	    	SRT_UNLOCK();
#ifdef SRDEBUG
		if (srdebug)
			logmsg = "SR: stale ARP entry -> no route\n";
#endif
		break;
	case RIF_NOROUTE:
		p_asrt_entry->srt_status = RIF_REDISCOVER;
	    	SRT_UNLOCK();
		build_ARE(p_sr_info);
#ifdef SRDEBUG
		if (srdebug)
			logmsg = "SR: noroute ARP entry -> rediscover\n";
#endif
		break;
	case RIF_REDISCOVER:
	    	SRT_UNLOCK();
		build_ARE(p_sr_info);
#ifdef SRDEBUG
		if (srdebug)
			logmsg = "SR: rediscover ARP entry -> rediscover\n";
#endif
		break;
	}
#ifdef SRDEBUG
	if (srdebug) {
		log(LOG_INFO, logmsg);
		dump_arp_entry(p_asrt_entry);
	}
#endif
	return(ret);
   }	
	/* unicast MAC address */
#ifdef SRDEBUG
	if (srdebug) {
		log(LOG_INFO, "SR: sr_search_arp/unicast MAC address \n"); 
	}
#endif
	SRT_LOCK();
	/*  Use "srt_dst" as key to search the SR Table. */
	SRTAB_MAC_SEARCH(p_srt_entry, (caddr_t)srt_dst, ifp);
	if (p_srt_entry == NULL) {	/* Entry Not Found */
	    /*
	     * Create an entry in the SR Table, with ip/mac
	     */
	   (void)srtab_entry_new(ifp,(caddr_t)srt_dst,inet_dst,SRTAB_ARP);
	   SRT_UNLOCK();
           return (ret);
	}
	/* entry found in SR table */
	if (p_srt_entry->srt_ip_addr.s_addr != inet_dst->s_addr)
	{
	   /* ARP command with different IP address. */
	   if (p_srt_entry->srt_ip_addr.s_addr)
	   {
		log(LOG_WARNING, "SR: ARP command with mac %s ", ether_sprintf(p_srt_entry->srt_mac_addr));
		log(LOG_WARNING, "SR: and ip addr %s\n", inet_ntoa(*inet_dst));
		log(LOG_WARNING, "SR: matches SR table entry with different ip addr %s\n", inet_ntoa(p_srt_entry->srt_ip_addr));
	   }
	   /*
	    * Create entry in ARP_SR table to avoid mismatch IP address of
	    * exist SR table entry. 
	    */
	   goto arptab;
	}
	p_srt_entry->srt_aging_timer = 0;
	p_srt_entry->srt_discovery_type = SRTAB_ARP;
	/* 
	 * The policy here is not to back-off the upper layer's transmit. 
	 * We send ARP as SRF only if there is a known route and not stale, 
	 * otherwise send as ARE if 1. have route but stale or
	 *			    2. entry is in rediscovery state
	 * or send as on-ring (i.e. no RIF returned).
	 */
	if (p_srt_entry->srtab_rif.rif_lth > 2) {
	   if (p_srt_entry->srt_status != RIF_STALE) {
		bcopy(&p_srt_entry->srtab_rif, p_sr_info, 
	   	 	p_srt_entry->srtab_rif.rif_lth);
		SRT_UNLOCK();
		ret = 1;
#ifdef SRDEBUG
		if (srdebug)
			logmsg = "SR: valid SR entry - have route\n";
#endif
	   } else {
		/* have route but stale; send ARE ARP with unicat MAC addr */
		srtab_entry_refresh(p_srt_entry, SRTAB_ARP);
		p_srt_entry->srt_status = RIF_REDISCOVER;
		SRT_UNLOCK();
		build_ARE(p_sr_info);
#ifdef SRDEBUG
		if (srdebug)
			logmsg = "SR: stale SR entry -> rediscover\n";
#endif
	   } 
	} else {
	   switch (p_srt_entry->srt_status) {
	   case RIF_ONRING:
		SRT_UNLOCK();
		ret = 1;
#ifdef SRDEBUG
		if (srdebug)
			logmsg = "SR: valid SR entry - on ring\n";
#endif
		break;
	   case RIF_STALE:
		srtab_entry_refresh(p_srt_entry, SRTAB_ARP);
		p_srt_entry->srt_status = RIF_NOROUTE;
		SRT_UNLOCK();
#ifdef SRDEBUG
		if (srdebug)
			logmsg = "SR: stale SR entry -> no route\n";
#endif
		break;
	   case RIF_NOROUTE:
		p_srt_entry->srtab_rif.rif_rt = SR_ALL_ROUTE_EXPLORE;
		p_srt_entry->srt_status = RIF_REDISCOVER;
		SRT_UNLOCK();
		build_ARE(p_sr_info);
#ifdef SRDEBUG
		if (srdebug)
			logmsg = "SR: no route SR entry -> rediscover\n";
#endif
		break;
	   case RIF_REDISCOVER:
		SRT_UNLOCK();
		build_ARE(p_sr_info);
#ifdef SRDEBUG
		if (srdebug)
			logmsg = "SR: rediscover SR entry -> send ARE\n";
#endif
		break;
	   }
	} /* srtab_rif.rif_lth <= 2 */

	if (p_sr_info->rif_rt == SR_ALL_ROUTE_EXPLORE)
		srt_var.ARE_sent += 1;

#ifdef SRDEBUG
	if (srdebug) {
		log(LOG_INFO, logmsg);
		dump_entry(p_srt_entry);
	}
#endif
	return(ret);
}

/*
 *                      s r _ s e a r c h _ x i d
 *
 *      This routine is called to process outgoing IP or DECnet packets
 * contaning a destination MAC address. It will search through the SR Table
 * to find whether there is an entry for the "mac_dst" MAC  address. If
 * there is, it will append the RI field from the table to "p_sr_info" and
 * return 1. If an entry does not exist, it will create and send an XID
 * frame for Route Discovery, it will fill the routing information
 * parameter with the two bytes of an STE frame and return zero.
 *      Multicast frames are always going to be transmitted as STE ones. 
 *
 *      Input : -A pointer to the network visible interface (ifnet)
 *		-A pointer to the data mbuf
 *              -The destination MAC address
 *              -The destination INET address (non zero if called for IP frames)
 *              -A pointer to the Routing Information to be filled
 *
 *      Returns: 1 Address was resolved in the SR Table
 *               0 Address was not resolved in the SR Table
 */
int 
 sr_search_xid(ifp, m, mac_dst, inet_dst, p_sr_info)
register struct ifnet *ifp;
struct mbuf *m;
register u_char *mac_dst;
register struct in_addr *inet_dst;
register struct trn_rif *p_sr_info;
{
   register u_char srt_dst[MAC_ADDR_LEN];
   register struct srtab *p_srt_entry;
   int ret = 0;
   NETSPL_DECL(s)

	bcopy(mac_dst, &srt_dst[0], MAC_ADDR_LEN*sizeof(u_char));

	SRT_LOCK();
	/* 
  	 * Use "srt_dst" as key to search the SR Table. 
	 */
	SRTAB_MAC_SEARCH(p_srt_entry, (caddr_t)srt_dst, ifp);
	if (p_srt_entry == NULL)	/* Entry Not Found */
	{	
	    /*
	     * Create an entry in the SR Table
	     */
	    p_srt_entry = srtab_entry_new(
				ifp, (caddr_t)srt_dst, inet_dst, SRTAB_XID
					 ); 
	    SRT_UNLOCK();
	    build_STE(p_sr_info);
	    if (!p_srt_entry)
	    /* 
	     * Unable to allocate a new entry. STE SR information is returned 
	     * so the data frame is not dropped. 
	     */ 
	    	return (ret);
	    /* 
	     * Construct and send an XID to NULL SAP 
	     * This applies to DECnet and IP packets.
	     */
	    xid_discover(ifp, (caddr_t)srt_dst);
	}
	else	/* Entry Found */
	{
#ifdef SRDEBUG
	    if (srdebug)
		log(LOG_INFO, "SR: sr_search_xid entry found\n");
#endif
	
	    p_srt_entry->srt_aging_timer = 0; 
	    p_srt_entry->srt_discovery_type = SRTAB_XID;

	    if (p_srt_entry->srtab_rif.rif_lth > 2) 
	    {
		if (p_srt_entry->srt_status != RIF_STALE)
		{
			bcopy(
			      &p_srt_entry->srtab_rif, p_sr_info, 
			      p_srt_entry->srtab_rif.rif_lth
			     );
			SRT_UNLOCK();
#ifdef SRDEBUG
			if (srdebug)
				log(LOG_INFO, "SR: have route entry\n");
#endif
		}
		else 
		{
			/* Have route but stale; ARE route discovery */
			/* zero ip addr since it may not be valid */
			p_srt_entry->srt_ip_addr.s_addr = 0;
			srtab_entry_refresh(p_srt_entry, SRTAB_XID);
			p_srt_entry->srt_status = RIF_REDISCOVER;
			SRT_UNLOCK();
#ifdef SRDEBUG
			if (srdebug)
				log(LOG_INFO, "SR: have route but stale entry\n");
#endif
			xid_discover(ifp, srt_dst);
			build_STE(p_sr_info);
		}
		ret = 1;
	    } 
	    else
	    {
                /*
		 * Entry contains valid entry for local ring station, or it
		 * was created as a result of XID route discovery.
                 * Check whether route discovery is in progress for this entry.
		 * If it is and the frame is a regular data frame, return STE
		 * routing info (do not re-initiate route discovery). If it is 
		 * an XID frame, then, if the state is RIF_NOROUTE, return w/out
		 * routing information (local ring discovery). If it is 
		 * RIF_REDISCOVER, then return ARE type info (extended LAN 
		 * discovery).
		 */
		switch (p_srt_entry->srt_status) {
		case RIF_NOROUTE:
		    SRT_UNLOCK();
		    if (!XID_frame(m)) {
			build_STE(p_sr_info);  
#ifdef SRDEBUG
			if (srdebug)
				log(LOG_INFO, "SR: no route entry\n");
#endif
		    }
#ifdef SRDEBUG
		    else
			if (srdebug)
				log(LOG_INFO, "SR: xid frame/no route entry\n");
#endif
		    break;
		case RIF_REDISCOVER:
		    SRT_UNLOCK();
		    if (!XID_frame(m)) {
			build_STE(p_sr_info);
#ifdef SRDEBUG
			if (srdebug)
				log(LOG_INFO, "SR: rediscover entry\n");
#endif
		    } else {
			build_ARE(p_sr_info);
#ifdef SRDEBUG
			if (srdebug)
				log(LOG_INFO, "SR: xid frame/rediscover entry\n");
#endif
		    }
		    break;
		case RIF_STALE:
		    /* on ring discovery */
		    /* zero ip addr since it may not be valid */
		    p_srt_entry->srt_ip_addr.s_addr = 0;
		    srtab_entry_refresh(p_srt_entry, SRTAB_XID);
		    p_srt_entry->srt_status = RIF_NOROUTE;
		    SRT_UNLOCK();
#ifdef SRDEBUG
		    if (srdebug)
		    	log(LOG_INFO, "SR: rif <= 2  stale entry\n");
#endif
		    xid_discover(ifp, srt_dst);
		    build_STE(p_sr_info);
		    break;
		case RIF_ONRING:
		    SRT_UNLOCK();
#ifdef SRDEBUG
		    if (srdebug)
		    	log(LOG_INFO, "SR: on ring entry\n");
#endif
		    break;
		}
		ret = 1;
	    } /* lth <= 2 */
#ifdef SRDEBUG
	    if (srdebug)
		dump_entry(p_srt_entry);	
#endif
	} /* entry found in SR table */
	return(ret);

}


/*
 * 			b u i l d _ S T E
 *	
 *	This routine will build an STE trn_rif structure.
 *
 *	Input : A pointer to the trn_rif to be constructed
 *
 */
int build_STE(p_sr_info)
struct trn_rif *p_sr_info;
{
        p_sr_info->rif_rt = SR_SPAN_TREE_EXPLORE;   
        p_sr_info->rif_lth = 2;                    /* 2 bytes; No RDs */
        p_sr_info->rif_dir = 0;
        p_sr_info->rif_lf = TRN_LF_8144;
        p_sr_info->rif_res = 0;
}



/*
 * 			b u i l d _ A R E
 *	
 *	This routine will build an ARE trn_rif structure.
 *
 *	Input : A pointer to the trn_rif to be constructed
 *
 */
int build_ARE(p_sr_info)
struct trn_rif *p_sr_info;
{
        p_sr_info->rif_rt = SR_ALL_ROUTE_EXPLORE;	/* 0x04 */
        p_sr_info->rif_lth = 2;                         /* 2 bytes; No RDs */
        p_sr_info->rif_dir = 0;
        p_sr_info->rif_lf = TRN_LF_8144;
        p_sr_info->rif_res = 0;
}


/*
 *			s r t _ t i m e o u t
 *
 * 	This routine will update the Source Routing Table entries and their 
 * aging timer. It will be called initially by the routine which initialize 
 * the SR Table. 
 * 
 *
 */
int srt_timeout()
{
	register struct srtab *p_tr_srt, *p_tmp_srentry;
	register struct arpsrtab *p_arp_srt, *p_tmp_asrentry;
	register i;
	register visit=0, delete=0;	/* we only need to visit "total_entry",
					 * not the entire table. */

	NETSPL_DECL(s)

	if (SR_enabled) {
	if (total_entry > 0) {
	SRT_LOCK();

	for (i=0; i<SRT_NBCKTS; i++)
	{
	  p_tr_srt = G_srt_tab[i];
	  while (p_tr_srt)
	  {
	    switch (p_tr_srt->srt_status)
	    {
	    case RIF_NOTUSE:
		break;
	    case RIF_STALE:
		if ((++p_tr_srt->srt_aging_timer >= srt_var.srt_kcentry_timer)
		   || ((p_tr_srt->srtab_rif.rif_rt == SR_ALL_ROUTE_EXPLORE) &&
		     (p_tr_srt->srt_aging_timer >= srt_var.srt_kientry_timer)))
		{
                   p_tmp_srentry = p_tr_srt->srt_next_hash;
                   bzero(p_tr_srt, srtab_entry_size);
		   ++delete;
                   p_tr_srt->srt_next_hash = p_tmp_srentry;
		}
		if (++visit >= total_entry)
			goto done;
		break;
	    case RIF_ONRING:
		/* fall through */
	    case RIF_HAVEROUTE:
		if (++p_tr_srt->srt_aging_timer >= srt_var.SR_aging_timer)
			p_tr_srt->srt_status = RIF_STALE;
		if (++visit >= total_entry)
			goto done;
		break;
	    case RIF_NOROUTE:
		if (++p_tr_srt->srt_aging_timer >= srt_var.SRDT_local) 
		{
		  p_tr_srt->srtab_rif.rif_rt = SR_ALL_ROUTE_EXPLORE;
		  p_tr_srt->srt_status = RIF_REDISCOVER;
		  p_tr_srt->srt_aging_timer = 0;
		  srt_var.SR_discover_fail += 1;
		  /* construct and send an All Route Explorer */
    	   	  if (p_tr_srt->srt_discovery_type == SRTAB_ARP )
		  {
		     SRT_UNLOCK();
		     arp_discover(p_tr_srt->srt_ifp, &(p_tr_srt->srt_ip_addr), 
			p_tr_srt->srt_mac_addr);
		     SRT_LOCK();
		  }
		  else
		  {
		     SRT_UNLOCK();
		     xid_discover(p_tr_srt->srt_ifp, p_tr_srt->srt_mac_addr);
		     SRT_LOCK();
		  }
		}
		if (++visit >= total_entry)
			goto done;
		break;
	    case RIF_REDISCOVER: 
		if (++p_tr_srt->srt_aging_timer >= srt_var.SRDT_extended)
		 {
			p_tr_srt->srt_status = RIF_STALE;
			srt_var.SR_discover_fail += 1;
		 }
		if (++visit >= total_entry)
			goto done;
		break;
	    }
	    p_tr_srt = p_tr_srt->srt_next_hash;
	  }
	}

	/* ARP request SR table */
	for (i=0; i < ARP_SRT_NBCKS; i++)
	{
	  p_arp_srt = arp_srtab[i];
	  while (p_arp_srt)
	  {
	    switch (p_arp_srt->srt_status)
	    {
	    case RIF_NOTUSE:
		break;
	    case RIF_STALE:
		if (++p_arp_srt->srt_aging_timer >= srt_var.srt_kientry_timer)
		{
                   p_tmp_asrentry = p_arp_srt->srt_next_hash;
                   bzero(p_arp_srt, sizeof(struct arpsrtab));
		   ++delete;
                   p_arp_srt->srt_next_hash = p_tmp_asrentry;
		}
		if (++visit >= total_entry)
			goto done;
		break;
	    case RIF_NOROUTE:
		if (++p_arp_srt->srt_aging_timer >= srt_var.SRDT_local)
		{
			p_arp_srt->srt_aging_timer = 0;
			p_arp_srt->srt_status = RIF_REDISCOVER;
			srt_var.SR_discover_fail += 1;
			/* send ARE ARP */
			SRT_UNLOCK();
			arp_discover(p_arp_srt->srt_ifp,
				    &(p_arp_srt->srt_ip_addr), NULL);
     			SRT_LOCK();
		}
		if (++visit >= total_entry)
			goto done;
		break;
	    case RIF_REDISCOVER:
		if (++p_arp_srt->srt_aging_timer >= srt_var.SRDT_extended) {
			p_arp_srt->srt_status = RIF_STALE;
			srt_var.SR_discover_fail += 1;
		}
		if (++visit >= total_entry)
			goto done;
		break;
	    }
	    p_arp_srt = p_arp_srt->srt_next_hash;
	  }
	}

done:						  
	total_entry -= delete;
	SRT_UNLOCK();
	} /* total_entry > 0 */
#if	!NETISR_THREAD
	timeout(srt_timeout, (caddr_t)0, SRT_AGE_UPDATE*hz);
	return (0);
#else
	return(SRT_AGE_UPDATE * hz);
#endif
	} else {
	/* !SR_enabled; stop timeout */
	return (0);
	}
}


/* 
 * 			s r t a b _ e n t r y _ n e w 
 *
 *	This routine will enter an entry in the SR Table. 
 *	pick the first found empty entry or a staled entry.
 *
 *	Input : - The network visible ifnet structure
 *		- The destination MAC address
 *		- The destination Inet address.
 * 		- The reason the entry was created (result of ARP, XID or
 *		   Ether Loopback message
 * 
 *	Return : A pointer to the created SR Table entry or NULL if
 * 		 unable to create the entry.
 */
struct srtab *
 srtab_entry_new(ifp, p_mac_addr, p_inet_addr, reason)
struct ifnet *ifp;
u_char *p_mac_addr;
struct in_addr *p_inet_addr;
u_char reason;
{
	register struct srtab *srt, *p_srt_hd, *p_stale_ent=NULL; 
	register struct srtab *srt_next_hash;
	int i;

	for ( (srt=p_srt_hd=G_srt_tab[SRTAB_MAC_HASH(p_mac_addr)]), i=0; 
		srt != NULL; (srt = srt->srt_next_hash), i++)
	{
		if (srt->srt_status == RIF_NOTUSE)
			goto gotentry;
		if (p_stale_ent == NULL) 
			if (srt->srt_status == RIF_STALE)
				p_stale_ent = srt;
  	}

	/*
	 * If reached the suggested bucket size or the allowed SR table
	 * size limit, overwrite a stale entry if one was found.
	 * Otherwise create a new entry if under the max SR table size
	 * or return NULL.
	 */
	if ( p_stale_ent &&
		((i >= srt_var.srt_bcktsize) || 
		  (srtb_actual_size >= srt_var.SR_table_size)) )
	{
                /*
                 * Clear the stale entry before using it, but preserve the
                 * bucket number and index, and the list pointer.
                 */
                srt = p_stale_ent;
		srt_next_hash = srt->srt_next_hash;
		bzero((caddr_t)srt, sizeof *srt);
		--total_entry;
		srt->srt_next_hash = srt_next_hash;
	}
	else
	{
	  if (srtb_actual_size < srt_var.SR_table_size)
	  {
	  	/* 
		 * if (i < srt_var.srt_bcktsize) or full bucket with no stale 
		 * entries, allocate a new entry in this bucket.
		 */
		srt = (struct srtab *)kalloc(srtab_entry_size);
		if (!srt) {
		    srt_var.srt_crt_entry_fail++;
		    log(LOG_WARNING, "SR: srtab_entry_new, can't kalloc\n");	
		    return (NULL);
		}
		/* only this field, srtab_rif, is not reassigned new value */
		bzero(&srt->srtab_rif, sizeof(struct trn_rif));
		srtb_actual_size++;
		srt->srt_next_hash = p_srt_hd;
		G_srt_tab[SRTAB_MAC_HASH(p_mac_addr)] = srt; 
	  }
	  else
	  {
		log(LOG_WARNING, "SR: srtab_entry_new, the actual SR table entries %d reached the max allowed SR Table size\n", srtb_actual_size);
		return (NULL);
	  }
	}
	   
gotentry:
	++total_entry;
	bcopy(
	      (caddr_t)p_mac_addr, (caddr_t)srt->srt_mac_addr, MAC_ADDR_LEN);
	if (p_inet_addr) 
	{
		srt->srt_ip_addr = *p_inet_addr;
	} else
		srt->srt_ip_addr.s_addr = 0;
	srt->srt_ifp = ifp;
	srt->srt_aging_timer = 0;
	srt->srt_status = RIF_NOROUTE;
    	srt->srt_discovery_type = reason;

        if (trn_init_dli)
                (*trn_init_dli)(srt);

#ifdef SRDEBUG
	if (srdebug) {
		i = SRTAB_MAC_HASH(p_mac_addr);
		log(LOG_INFO, "SR: new entry at %d bucket\n", i);
		dump_entry(srt);
	}
#endif
	return(srt);
} 

/* 
 * 			a r p s r t a b _ e n t r y _ n e w 
 *
 *	This routine will enter an entry in the ARP_SR Table. 
 *	pick the first found staled entry or if the bucket is not full,
 *	allocate a new one.
 *
 *	Input : - The network visible ifnet structure
 *		- The destination Inet address.
 * 
 *	Return : A pointer to the created ARP_SR Table entry or NULL if
 * 		 unable to create the entry.
 */
void
 arpsrtab_entry_new(ifp, p_inet_addr)
struct ifnet *ifp;
struct in_addr *p_inet_addr;
{
	register struct arpsrtab *asrt, *ap_srt_hd, *ap_stale_ent=NULL; 
	register struct arpsrtab *asrt_next_hash;
	int i;

	for ((asrt=ap_srt_hd=arp_srtab[SRTAB_IP_HASH(p_inet_addr->s_addr)]), 
	      i=0; 
	      asrt != NULL; (asrt = asrt->srt_next_hash), i++)
	{
		if (asrt->srt_status == RIF_NOTUSE)
			goto gotentry;
		else if (asrt->srt_status == RIF_STALE)
			break;
	
	}
	if (asrt) {
		asrt_next_hash = asrt->srt_next_hash;
		bzero(asrt, sizeof(struct arpsrtab));
		--total_entry;
		asrt->srt_next_hash = asrt_next_hash;
	} else {
		if (i > arp_srt_bsize) {
			log(LOG_WARNING, "all the buckets are in use\n");
			return;
		}
	      	/* 
		 * if no stale entry and (i <= arp_srt_bsize), 
		 * allocate a new entry in this bucket.
		 */
		asrt = (struct arpsrtab *)kalloc(sizeof(struct arpsrtab)); 
		if (!asrt) {
		    srt_var.srt_crt_entry_fail++;
		    log(LOG_WARNING, "SR: arpsrtab_entry_new, can't kalloc\n");
		    return;
		}
		asrt->srt_next_hash = ap_srt_hd;
		arp_srtab[SRTAB_IP_HASH(p_inet_addr->s_addr)] = asrt;	
	}
gotentry:
	++total_entry;
	asrt->srt_ifp = ifp;
	asrt->srt_ip_addr = *p_inet_addr;
	asrt->srt_aging_timer = 0;
	asrt->srt_status = RIF_NOROUTE;

#ifdef SRDEBUG
	if (srdebug) {
   		i = SRTAB_IP_HASH(p_inet_addr->s_addr); 
		log(LOG_INFO, "SR: new ARP entry at %d bucket\n", i);
		dump_arp_entry(asrt);
	}
#endif
	return;
}


/*
 *		s r t a b _ e n t r y _ r e f r e s h 
 *
 *	This routine will refresh a Source Routing table entry before
 *  it issues an ARE route discover frame.
 *
 *	Input : -A pointer to the SR Table entry to be refreshed
 *		-Type of frame for route discovery.
 *
 */
void 
 srtab_entry_refresh(p_srtab_entry, type)
struct srtab *p_srtab_entry;
u_char	type;
{
	
	p_srtab_entry->srt_aging_timer = 0;
	bzero((caddr_t)&(p_srtab_entry->srtab_rif), sizeof(struct trn_rif));
	p_srtab_entry->srtab_rif.rif_rt = SR_ALL_ROUTE_EXPLORE;
        p_srtab_entry->srt_discovery_type = type;

	return;
}


/*
 *			u p d a t e i p
 *
 *	This routine updates IP address in SR table.
 *	It also frees up the ARP request SR table entry.
 *
 *	Input: -A pointer to SR table entry
 *	       -A pointer to IP address
 *	       -A pointer to ifnet structure
 */
void
 updateip(srt, ip_addr, ifp)
struct srtab *srt;
struct in_addr *ip_addr;
struct ifnet *ifp;
{
	register struct arpsrtab *asrt, *asrt_next_hash;

#ifdef SRDEBUG
	if (srdebug)
		log(LOG_INFO, "SR: updateip, ip addr %s\n", inet_ntoa(*ip_addr));
#endif

	if ((srt->srt_ip_addr.s_addr) && (srt->srt_ip_addr.s_addr != ip_addr->s_addr))
{
		log(LOG_WARNING, "SR: mac %s changes ", ether_sprintf(srt->srt_mac_addr));
		log(LOG_WARNING, "SR: old ip addr %s ", inet_ntoa(srt->srt_ip_addr));
		log(LOG_WARNING, "SR: new ip address %s\n", inet_ntoa(*ip_addr));
}
			
	srt->srt_ip_addr = *ip_addr;

	ARP_SRTAB_IP_SEARCH(asrt, ip_addr->s_addr, ifp);
	if (asrt) {
		asrt_next_hash = asrt->srt_next_hash;
		bzero(asrt, sizeof(struct arpsrtab));
		--total_entry;
		asrt->srt_next_hash = asrt_next_hash;
	} 

}


/*
 *			x i d _ d i s c o v e r
 *
 *	This routine will construct and transmit an  XID to NULL SAP, to 
 * 	perform Route Discovery.
 *
 *      Input   : -A pointer to the network visible interface structure
 *                -The MAC address to be used for DA.
 *
 *
 *      Returns :  0 On Success
 *                -1 On Failure
 */
int
 xid_discover(ifp, xid_mac_dest)
struct ifnet *ifp;
u_char *xid_mac_dest;
{
        register struct sockaddr tr_dst;
        register struct mbuf *m;
        register u_char *ptr;
        struct ether_header *p_eh = (struct ether_header *)tr_dst.sa_data;

#ifdef SRDEBUG
	if (srdebug)
		log(LOG_INFO, "SR: xid_discover\n");
#endif

        tr_dst.sa_family = AF_UNSPEC;
        tr_dst.sa_len = sizeof(tr_dst);

        bzero((caddr_t)p_eh->ether_shost, sizeof(p_eh->ether_shost));
        bcopy(xid_mac_dest, (caddr_t)p_eh->ether_dhost,
		sizeof(p_eh->ether_dhost));
	p_eh->ether_type = 0;

        MGETHDR(m, M_DONTWAIT, MT_HEADER);
        if (m == NULL)
                return (-1);

	MH_ALIGN(m, 6);
        ptr = mtod(m, u_char *);
        *ptr++ = 0x0;   /* Null SAP for DSAP */
        *ptr++ = 0x0;   /* Null SAP for SSAP */
        *ptr++ = LLC_XID;
   	*ptr++ = 0x81;
	*ptr++ = 0x1;
	*ptr   = 0x0;
	
	m->m_len = m->m_pkthdr.len = 6;

        (*ifp->if_output)(ifp, m, &tr_dst, (struct rtentry *)NULL);

        return (0);
}


/*
 *			a r p _ d i s c o v e r
 *
 *	This routine calls arpreq() to send an ARP packet.
 *
 *      Input   : -A pointer to the network visible interface structure
 *		  -The IP address to be used for DA
 *                -The MAC address to be used for DA.
 *
 */
void
 arp_discover(ifp, ip_addr, mac_addr)
struct ifnet *ifp;
struct in_addr *ip_addr;
u_char *mac_addr;
{

#ifdef SRDEBUG
	if (srdebug)
		log(LOG_INFO, "SR: arp_discover, ip addr %s\n", inet_ntoa(*ip_addr));
#endif
	if (arpreq)
		(*arpreq)((struct arpcom *)ifp, ip_addr, mac_addr);
}
 

/*
 *                              X I D _ f r a m e
 *
 * 	This routine will test whether a frame is an XID to NULL SAP.
 *
 *      Input   : A pointer to the data mbuf
 *
 *      Returns : 1 If it is an XID to NULL SAP
 *                0 If it is not
 */
int
 XID_frame(m)
struct mbuf *m;
{
        register struct llc *l;

        l = mtod(m, struct llc *);
        if (
            (l->llc_control == LLC_XID) &&
            (l->llc_dsap == 0) && (l->llc_ssap == 0)
           )
                return (1);
        else
                return (0);
}


/*
 *				s r t _ i n i t
 *
 * This routine will be called when Source Routing is configured
 * to initialize the SR attributes and hash tables.
 * It will be called for the first time from netinit() in netisr.c;
 * or from srt_ioctl().
 */
void
 sr_init()
{
	int i, j;
	struct ifnet *ifp;
	NETSPL_DECL(s)

	if (first_time) {
		SRT_LOCKINIT();	
	}

	/*
	 * Initialize the entries in the SR Hash Table 	
	 */

	SRT_LOCK();
	srtb_actual_size = 0;
	total_entry = 0;
	for (i=0; i<SRT_NBCKTS; i++) { 
		G_srt_tab[i] = (struct srtab *)kalloc(srtab_entry_size); 
		if (G_srt_tab[i] == NULL)
			break;
		bzero(G_srt_tab[i], srtab_entry_size );
		G_srt_tab[i]->srt_next_hash = NULL;
		srtb_actual_size++;
	}
	if (srtb_actual_size < SRT_NBCKTS)
	{
		printf("SR: Can not enable source routing due to lack of memory resources\n");
		for (i=srtb_actual_size; i<SRT_NBCKTS; i++)
                        G_srt_tab[i] = NULL;
		srtab_clear();
		SRT_UNLOCK();
		return;
	}
	for (i=0; i<ARP_SRT_NBCKS; i++) { 
		arp_srtab[i] = (struct arpsrtab *)kalloc(sizeof(struct arpsrtab)); 
		if (arp_srtab[i] == NULL)
			break;
		bzero(arp_srtab[i], sizeof(struct arpsrtab));
		arp_srtab[i]->srt_next_hash = NULL;
	}
	if (i < ARP_SRT_NBCKS)
	{
		printf("SR: Can not enable source routing due to lack of memory resources\n");
		srtab_clear();
		for (j=i; j<ARP_SRT_NBCKS; j++)
                        arp_srtab[i]=NULL;
		arpsrtab_clear();
		SRT_UNLOCK();
		return;
	}
		
	/* set the source routing global variable */
	SR_enabled = 1;

	/*
	 * Initialize the SR management structure.
	 */
	srt_var.SR_aging_timer = SR_AGING_TIMER;
	srt_var.SRDT_local = RD_TIMER_LOCAL;
	srt_var.SRDT_extended = RD_TIMER_EXT;
	srt_var.srt_bcktsize = MAX_BCKT_SIZE;
	srt_var.SR_table_size = MAX_SRTB_SIZE;
	srt_var.srt_kcentry_timer = KCENTRY_TIMER;
	srt_var.srt_kientry_timer = KIENTRY_TIMER;
	srt_var.ARE_sent = 0;
	srt_var.ARE_rcvd = 0;
	srt_var.SR_discover_fail = 0;
	srt_var.srt_crt_entry_fail = 0;

	SRT_UNLOCK();

#if	!NETISR_THREAD
	if (first_time)
		first_time = 0;
	srt_timeout();
#else
	if (first_time) {
		first_time = 0;
		net_threadstart(srt_timeout, 0);
	} else
		thread_wakeup((vm_offset_t)&srt_timeout);
#endif
}


/*
 *                         s r t a b _ c l e a r
 *
 * This routine will clear the SR Table freeing all the allocated entries.
 */
void
 srtab_clear()
{
        register i;
        register struct srtab *p_tr_srt, *p_srt_next;

        for (i=0; i<SRT_NBCKTS; i++)
        {
            p_tr_srt = G_srt_tab[i];
            while (p_tr_srt)
            {
                p_srt_next = p_tr_srt->srt_next_hash;
                kfree(p_tr_srt, srtab_entry_size);
                p_tr_srt = p_srt_next;
            }
	    G_srt_tab[i] = NULL;
        }
	srtb_actual_size = 0;
        return;
}


/*
 *                         a r p s r t a b _ c l e a r
 *
 * This routine will clear the ARP_SR Table freeing all the allocated entries.
 */
void
 arpsrtab_clear()
{
        register i;
        register struct arpsrtab *p_tr_srt, *p_srt_next;

        for (i=0; i<ARP_SRT_NBCKS; i++)
        {
            p_tr_srt = arp_srtab[i];
            while (p_tr_srt)
            {
                p_srt_next = p_tr_srt->srt_next_hash;
                kfree(p_tr_srt, sizeof(struct arpsrtab));
                p_tr_srt = p_srt_next;
            }
	    arp_srtab[i] = NULL;
        }
	return;
}


/*
 *                              s r _ i o c t l
 *
 * This routine will set and read SR Table entries and other SR attributes.
 *
 *      Input:    cmd - source routing ioctl command
 *                data - source routing request data
 *
 *      Output :  data
 *
 */
int
 sr_ioctl(cmd, data)
int     cmd;
caddr_t data;
{
	register struct srreq *sr_ioctl = (struct srreq *)data;
        register struct srtab *srt;
        int error=0;
	struct ifnet *ifp;
	void srtab_to_srreq();

	NETSPL_DECL(s)

	SRT_LOCK();
        switch (cmd)
        {
	case SIOCSRREQR:	/* SR ioctl read request */
	  if (SR_enabled) 
	  {
	   switch (sr_ioctl->srreq_type)
	   {
           case SR_RENT:	/* Read an entry in the SR Table */
#ifdef SRDEBUG
	    	if (srdebug)
    		   log(LOG_INFO, "sr_ioctl SIOCSRRENT: MAC=%s:\n", ether_sprintf(sr_ioctl->srreq_data.srtab.srt_mac_addr));
#endif
		SRTAB_MAC_SEARCH(srt, 
			(caddr_t)sr_ioctl->srreq_data.srtab.srt_mac_addr, 
			srt->srt_ifp);
                if (srt == NULL)
	    	{
#ifdef SRDEBUG
	           if (srdebug)
    			 log(LOG_INFO, "sr_ioctl SIOCSRRENT: SRTAB_MAC_SEARCH() returned NULL pointer\n");
#endif
                   error = ENXIO;
		}
                else
		   srtab_to_srreq(srt, &(sr_ioctl->srreq_data.srtab));
                break;

           case SR_RATTR:	/* Read SR attributes */
		bcopy(&srt_var, &(sr_ioctl->srreq_data.attrs), 
			sizeof(struct srt_attr));
		break;
	   case SR_ISENAB:	/* Is SR enabled ? */
		break;
	   default:
		error = EINVAL;
		break;
	   }
	  } /* SR_enabled */
	  else
	   error = ENODEV;
	break;

	case SIOCSRREQW:	/* SR ioctl write request */
	  if (SR_enabled)
	  {
	   switch (sr_ioctl->srreq_type)
	   {
           case SR_ZATTR:	/* Clear SR counters */
                srt_var.ARE_sent = 0;
                srt_var.ARE_rcvd = 0;
		srt_var.SR_discover_fail = 0;
                break;

           case SR_SATTR:	/* Set one or more attributes in srt_attr */
                if (sr_ioctl->srreq_data.attrs.SR_aging_timer)
			srt_var.SR_aging_timer = sr_ioctl->srreq_data.attrs.SR_aging_timer;
                if (sr_ioctl->srreq_data.attrs.SRDT_local)
			srt_var.SRDT_local = sr_ioctl->srreq_data.attrs.SRDT_local;
                if (sr_ioctl->srreq_data.attrs.SRDT_extended)
			srt_var.SRDT_extended = sr_ioctl->srreq_data.attrs.SRDT_extended;
                if (sr_ioctl->srreq_data.attrs.srt_bcktsize)
			srt_var.srt_bcktsize = sr_ioctl->srreq_data.attrs.srt_bcktsize;
                if (sr_ioctl->srreq_data.attrs.SR_table_size)
		   if (sr_ioctl->srreq_data.attrs.SR_table_size % SRT_NBCKTS)
			error = EINVAL;
		   else
		   {
			if ( (srt_var.SR_table_size < sr_ioctl->srreq_data.attrs.SR_table_size) && (srt_var.SR_table_size < 8 * SRT_NBCKTS) )
			{
				srt_var.SR_table_size =
				   sr_ioctl->srreq_data.attrs.SR_table_size;
				srt_var.srt_bcktsize = (int)
				   (sr_ioctl->srreq_data.attrs.SR_table_size / 
				    SRT_NBCKTS);
			}
			else
				error = EINVAL;
		   }
                break;

	   case SR_ENAB:	/* Enable Source Routing */
		error = EALREADY;
		break;

           case SR_DISAB:	/* Disable Source Routing */
		SR_enabled = 0;
		srtab_clear();
		arpsrtab_clear();
		total_entry = 0;
                break;

           case SR_DELENT:	/* Delete (clear) a SR Table entry */
                if (sr_entry_delete(sr_ioctl->srreq_data.srtab.srt_mac_addr, NULL))
		    	error = ENXIO;
                break;

           case SR_DISENT:	/* Disable a SR Table entry */
                if (sr_entry_disable(sr_ioctl->srreq_data.srtab.srt_mac_addr, NULL)) 
	    		error = ENXIO;
                break;

           default:
                error = EINVAL;
                break;
           }
	  } /* SR_enabled */
	  else
	  {
	   switch (sr_ioctl->srreq_type)
	   {
	   case SR_ENAB:	/* Enable Source Routing */
		SRT_UNLOCK();
		sr_init();
		if (!SR_enabled)
			error = ENOSPC;
		return error;

	   default:
		error = ENODEV;
		break;
	   }
	  }
	break;

	default:
	   error = EINVAL;
	   break;
	} /* switch (cmd) */

	SRT_UNLOCK();
        return error;
}


void
 srtab_to_srreq(tb_str, req_str)
struct srtab *tb_str;
struct srreq_tb *req_str;
{
        int i;

        req_str->srt_ip_addr = tb_str->srt_ip_addr;
        for (i=0; i<=MAC_ADDR_LEN; i++)
                req_str->srt_mac_addr[i] = tb_str->srt_mac_addr[i];
        req_str->srtab_rif = tb_str->srtab_rif;
        req_str->srt_aging_timer = tb_str->srt_aging_timer;
        req_str->srt_status = tb_str->srt_status;
        req_str->srt_discovery_type = tb_str->srt_discovery_type;
}



#ifdef SRDEBUG
void
dump_srtab()
{
	register int n=0, i, j=0;
	register struct srtab *p_srt_entry;
	register struct arpsrtab *p_asrt_entry;
	
	log(LOG_INFO, "SR: start SR table dump\n");

	for (i = 0; i < SRT_NBCKTS; i++) {
		p_srt_entry = G_srt_tab[i];
		while (p_srt_entry) {
			n++;
		if (p_srt_entry->srt_status != RIF_NOTUSE) {
			j++;
			log(LOG_INFO, "SR: dump bucket %d -> \n", i);
			dump_entry(p_srt_entry);
		}
			p_srt_entry = p_srt_entry->srt_next_hash;
		}
	}
	log(LOG_INFO, "SR: total %d entries, %d entries in use.\n", n, j);

	log(LOG_INFO, "SR: start ARP SR table dump\n");

	n = 0; j = 0;
	for (i = 0; i < ARP_SRT_NBCKS; i++) {
		p_asrt_entry = arp_srtab[i];
		while (p_asrt_entry) {
			n++;
		if (p_asrt_entry->srt_status != RIF_NOTUSE) {
			j++;
			log(LOG_INFO, "SR: dump bucket %d -> \n", i);
			dump_arp_entry(p_asrt_entry);
		}
			p_asrt_entry = p_asrt_entry->srt_next_hash;
		}
	}
	log(LOG_INFO, "SR: total %d entries, %d entries in use.\n", n, j);

}
	
void
dump_entry(p_srt_entry)
struct srtab *p_srt_entry;
{
	union short_char {
		short	sh[SR_RD_SIZE];
		u_char	uch[SR_RD_SIZE * 2];
	} *sh_uch;
	
	if (p_srt_entry->srt_status == RIF_NOTUSE)
		log(LOG_INFO, "SR: entry not in use\n");
	else {
		log(LOG_INFO, "SR: ip=%s:\n", inet_ntoa(p_srt_entry->srt_ip_addr));
		log(LOG_INFO, "SR: mac=%s:\n", ether_sprintf(p_srt_entry->srt_mac_addr));
		log(LOG_INFO, "SR: rif=[rt=%d:lth=%d:dir=%d:lf=%d:res=%d]:\n",
			     p_srt_entry->srtab_rif.rif_rt, 
			     p_srt_entry->srtab_rif.rif_lth,
			     p_srt_entry->srtab_rif.rif_dir,
			     p_srt_entry->srtab_rif.rif_lf,
			     p_srt_entry->srtab_rif.rif_res);
		sh_uch = (union short_char *)p_srt_entry->srtab_rif.rif_rd;
		log(LOG_INFO, "SR: rd=[0x%02X%02X 0x%02X%02X 0x%02X%02X 0x%02X%02X 0x%02X%02X 0x%02X%02X 0x%02X%02X 0x%02X%02X]\n",
		sh_uch->uch[0], sh_uch->uch[1], sh_uch->uch[2], sh_uch->uch[3],
		sh_uch->uch[4], sh_uch->uch[5], sh_uch->uch[6], sh_uch->uch[7],
		sh_uch->uch[8], sh_uch->uch[9],sh_uch->uch[10],sh_uch->uch[11],
		sh_uch->uch[12],sh_uch->uch[13],sh_uch->uch[14],sh_uch->uch[15]);
		log(LOG_INFO, "SR: aging=%d:stat=%d:dtyp=%d\n", 
				p_srt_entry->srt_aging_timer,
				p_srt_entry->srt_status,
				p_srt_entry->srt_discovery_type);
	}
}

void
dump_arp_entry(p_asrt_entry)
struct arpsrtab *p_asrt_entry;
{
	
	if (p_asrt_entry->srt_status == RIF_NOTUSE)
		log(LOG_INFO, "SR: entry not in use\n");
	else {
		log(LOG_INFO, "SR: ip=%s:\n", inet_ntoa(p_asrt_entry->srt_ip_addr));
		log(LOG_INFO, "SR: aging=%d:stat=%d\n", 
				p_asrt_entry->srt_aging_timer,
				p_asrt_entry->srt_status);
	}
}
#endif


/*
 *	Subroutines for SR attribute and table entries management.
 */

/*
 *			s r _ r e a d _ a t t r s
 *
 * This routine will be used to read the SR attributes (characteristics
 * and counters).
 *
 *	Input : A pointer to the attribute structure to be filled (OUT)
 *
 */
void
 sr_read_attrs(sr_attr_val)
struct srt_attr *sr_attr_val;
{

	bcopy(&srt_var, sr_attr_val, sizeof(struct srt_attr));
	return;
}



/*
 *			s r _ s e t _ a t t r s 
 *
 * This routine will be used to set one or more of the SR attributes.
 *
 *	Input : The attribute structure containing the new attribute values. 
 * 		If an attribute is to remain unchanged, its structure element 
 * 		has to be zero. (IN)
 */
void
 sr_set_attrs(attr_ioctl)
struct srt_attr *attr_ioctl;
{
	if (attr_ioctl->SR_aging_timer) 
	        srt_var.SR_aging_timer = attr_ioctl->SR_aging_timer;
        if (attr_ioctl->SRDT_local)
                srt_var.SRDT_local = attr_ioctl->SRDT_local;
        if (attr_ioctl->SRDT_extended)
                srt_var.SRDT_extended = attr_ioctl->SRDT_extended;
	if (attr_ioctl->srt_bcktsize)
		srt_var.srt_bcktsize = attr_ioctl->srt_bcktsize;
	if (attr_ioctl->SR_table_size)
		srt_var.SR_table_size = attr_ioctl->SR_table_size;

	return;
}


/*
 *			s r _ r e a d _ s r t a b _ e n t r y
 * 
 * This routine will be called to read a particular entry in the SR Table.
 * The sratab_entry argument when this routine is called will include
 * the MAC address of the entry to be read. If an entry matching the given 
 * MAC address is found, upon return srtab_entrywill contain the entry, 
 * otherwise it will be NULL.
 *
 * 	Input : A pointer to an SR Table entry (IN-OUT)
 *
 */ 
void 
 sr_read_srtab_entry(srtab_entry, ifp)
struct srtab *srtab_entry;
struct ifnet *ifp;
{
	register struct srtab *srt;
	NETSPL_DECL(s)

	SRT_LOCK();
        SRTAB_MAC_SEARCH(srt, (caddr_t)srtab_entry->srt_mac_addr, ifp);
        if (srt == NULL)
              	srtab_entry = NULL;
        else
        	bcopy(srt, srtab_entry, srtab_entry_size);
	
        SRT_UNLOCK();

	return;
}



/*
 *                      s r _ r e a d _ t a b l e
 *
 *      This routine will be called to read the entire SR Table. It
 * will return the next non-NULL entry except when the whole table has
 * been read, in which case it will also return more=0. The bucket and
 * index parameters are used as context handle and, upon routine return,
 * will point to the next entry to be read.
 *
 *      Input : A pointer to to a pointer to a table entry to be returned (OUT)
 *              A pointer to the bucket number (IN-OUT)
 *              A pointer to the index within the bucket (IN-OUT)
 *              A value indicating if more entries exist to be read (IN-OUT)
 *
 */
void
 sr_read_table(srt, bktno, index, more)
struct srtab **srt;
int *bktno ;
int *index;
int *more;
{
    struct srtab *srt_p;
    int i;

#ifdef SRDEBUG
        if (srdebug)
                log(LOG_DEBUG, "SR:sr_read_table() begin-- bucket=%d, bucket index=%d\n", *bktno, *index);
#endif

    srt_p = G_srt_tab[*bktno];
    for (i=0; i<(*index), srt_p!=NULL ; i++, srt_p = srt_p->srt_next_hash) ;

    if (srt_p)          /* i == *index */
    {
        if (srt_p->srt_next_hash)
                (*index)++;
        else    /* Move to the next bucket */
        {
                (*bktno)++;
                if (*bktno > SRT_NBCKTS)
                     *more = 0;
                *index = 0;
        }
    }
    else        /* srt_p == NULL */
    {
        (*bktno)++;
        if (*bktno > SRT_NBCKTS)
                *more = 0;
        else
        {
                while (G_srt_tab[*bktno] == NULL)
                {
                    if (*bktno > SRT_NBCKTS)
                    {
                        *more = 0;
                        goto read_exit;
                    }
                    (*bktno)++;
                }
                *index = 1;
                srt_p = G_srt_tab[*bktno];
        }
    }

read_exit : 
#ifdef SRDEBUG
        if (srdebug)
	{
                log(LOG_DEBUG, "SR:sr_read_table() end-- bucket=%d, bucket index=%d ", *bktno, *index);
		log(LOG_DEBUG, "and more flag=%d\n", *more);
		if ( (*srt)== NULL )
		    log(LOG_DEBUG, "SR:sr_read_table() end-- NULL table pointer returned\n");
	}
#endif

    *srt = srt_p;
    return;
}




/*
 *                      s r _ e n a b l e
 *
 * This routine will be called to enable the Source Routing process.
 * It can also be used to reset the SR attributes and SR Table to their
 * default values.
 */
void
 sr_enable()
{
#ifdef SRDEBUG
        if (srdebug)
                log(LOG_INFO, "SR_DLI: Entered sr_enable() \n");
#endif

    if (!SR_enabled)
        sr_init();

#ifdef SRDEBUG
        if (srdebug)
                log(LOG_INFO, "SR_DLI: sr_enable()--SR Enabled succesfully \n");
#endif
}


/*
 *                      s r _ d i s a b l e
 *
 * This routine will be called to disable the Source Routing process.
 */
void
 sr_disable()
{
#ifdef SRDEBUG
        if (srdebug)
                log(LOG_INFO, "SR_DLI: Entered sr_disable() with SR_enabled=%d \n", SR_enabled);
#endif

        if (SR_enabled)
        {
                SR_enabled = 0;
                srtab_clear();
                arpsrtab_clear();
        }
#ifdef SRDEBUG
        if (srdebug)
                log(LOG_INFO, "SR_DLI: sr_disable()--SR disabled succesfully \n"
);
#endif

        return;
}


/*
 *              s r _ e n t r y _ d i s a b l e 
 *
 *      This routine will disable/reset (set the state to STALE of) an entry
 * in the SR Table.
 *
 *      Input:  The MAC address of the entry to be disabled (IN)
 *              The interface structure of the entry to be disabled (IN)
 *
 *      Returns: 0 if entry was disabled successfully
 *               1 if entry was not found
 */
sr_entry_disable(entity_addr, ifp)
u_char *entity_addr;
struct ifnet *ifp;
{
    register struct srtab *srt;
    u_char sr_mac_addr[MAC_ADDR_LEN];
    int ret = 0;

    bcopy(entity_addr, &sr_mac_addr[0], MAC_ADDR_LEN);

#ifdef SRDEBUG
    if (srdebug) 
           log(LOG_INFO, "SR_DLI: sr_entry_disable(), Disable SR entry with MAC address=%s\n", ether_sprintf(sr_mac_addr));
#endif

    if (ifp)
    {
    	SRTAB_MAC_SEARCH(srt, (caddr_t)sr_mac_addr, ifp);
    }
    else
    {
	SRTAB_MAC_SEARCH(srt, (caddr_t)sr_mac_addr, srt->srt_ifp);
    }

    if (srt == NULL)
        ret = 1;
    else
        srt->srt_status = RIF_STALE;

#ifdef SRDEBUG
    if (srdebug)
         log(LOG_INFO, "SR_DLI: sr_entry_disable() exiting with ret=%d \n", ret);
#endif

    return (ret);
}


/*
 *              s r _ e n t r y _ d e l e t e
 *
 *      This routine will delete (clear) an entry in the SR Table.
 *
 *      Input:  The MAC address of the entry to be deleted (IN)
 *              The interface structure of the entry to be deleted (IN)
 *
 *      Returns: 0 if entry was deleted successfully
 *               1 if entry was not found
 */
sr_entry_delete(entity_addr, ifp)
u_char *entity_addr;
struct ifnet *ifp;
{
    register struct srtab *srt, *p_tmp_srentry;
    u_char sr_mac_addr[MAC_ADDR_LEN];
    int ret = 0;

    bcopy(entity_addr, &sr_mac_addr[0], MAC_ADDR_LEN);

#ifdef SRDEBUG
    if (srdebug)
           log(LOG_INFO, "SR_DLI: sr_entry_delete(), Delete SR entry with MAC address=%s\n", ether_sprintf(sr_mac_addr));
#endif

    if (ifp)
    {
    	SRTAB_MAC_SEARCH(srt, (caddr_t)sr_mac_addr, ifp);
    } 
    else
    {
	SRTAB_MAC_SEARCH(srt, (caddr_t)sr_mac_addr, srt->srt_ifp);
    }

    if (srt == NULL)
        ret = 1;
    else   /* Clear the entry/entity */
    {
        p_tmp_srentry = srt->srt_next_hash;
        bzero(srt, srtab_entry_size);
	--total_entry;
        srt->srt_next_hash = p_tmp_srentry;
     }

#ifdef SRDEBUG
    if (srdebug)
         log(LOG_INFO, "SR_DLI: sr_entry_delete() exiting with ret=%d \n", ret);
#endif

     return (ret);
}



/*
 * 			s r _ f i n d _ e n t i t y 
 *
 *      This routine will search and return a SR Table entry with a specific
 * MAC Address.
 *
 *      Returns: 0 if entry was found 
 *               1 if entry was not found
 */
sr_find_entity(srte, entity_addr, ifp)
struct srtab **srte;
u_char *entity_addr;
struct ifnet *ifp;
{
    u_char sr_mac_addr[MAC_ADDR_LEN];
    struct srtab *srt_p;
    int ret = 0;
    NETSPL_DECL(s)

    bcopy(entity_addr, &sr_mac_addr[0], MAC_ADDR_LEN);

#ifdef SRDEBUG
    if (srdebug)
         log(LOG_INFO, "SR_DLI: sr_find_entity()--Find SR entry with MAC address=%s\n", ether_sprintf(sr_mac_addr));
#endif

    SRT_LOCK();
    SRTAB_MAC_SEARCH(srt_p, (caddr_t)sr_mac_addr, ifp);
    if (srt_p == NULL)
        ret = 1;
    SRT_UNLOCK();

#ifdef SRDEBUG
    if (srdebug)
         log(LOG_INFO, "SR_DLI: sr_find_entity() exiting with ret=%d \n", ret) ;
#endif

    *srte = srt_p;
    return (ret);
}


