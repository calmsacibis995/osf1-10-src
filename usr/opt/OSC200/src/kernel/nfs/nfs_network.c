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
static char *rcsid = "@(#)$RCSfile: nfs_network.c,v $ $Revision: 1.1.6.5 $ (DEC) $Date: 1993/07/12 11:18:20 $";
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ioctl.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <net/net_globals.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/in_var.h>
#include <netinet/ip_var.h>
#include <netinet/if_ether.h>

/*      @(#)if_ether.h  2.1 88/05/18 4.0NFSSRC SMI;     from UCB 7.1 6/5/86     */
struct ether_addr {
        u_char  ether_addr_octet[6];
};
#define ETHERTYPE_REVARP 0x8035         /* Reverse ARP */

/*      @(#)if_arp.h    2.1 88/05/18 4.0NFSSRC SMI;     from UCB 7.1 6/5/86     */
#define REVARP_REQUEST  3       /* Reverse ARP request */
#define REVARP_REPLY    4       /* Reverse ARP reply */

void revarp_start(struct ifnet *);

#ifdef notdef
/*
 * Convert Ethernet address to printable (loggable) representation.
 *    (Nuked because of redundent function name 
 */
static char *
ether_sprintf(ap)
	register u_char *ap;
{
	register int i;
	static char etherbuf[18];
	register char *cp = &etherbuf[0];
	static char digits[] = "0123456789abcdef";

	for (i = 0; i < 6; i++) {
		*cp++ = digits[*ap >> 4];
		*cp++ = digits[*ap++ & 0xf];
		*cp++ = ':';
	}
	*--cp = 0;
	return (etherbuf);
}
#endif

/*
 * REVerse Address Resolution Protocol (revarp) is used by a diskless
 * client to find out its IP address when all it knows is its Ethernet address.
 */
int revarp = 1;
struct in_addr myaddr;

revarp_myaddr(ifp)
	register struct ifnet *ifp;
{
	register struct sockaddr_in *sin;
	struct ifreq ifr;
	int s;

	/*
	 * We need to give the interface a temporary address just
	 * so it gets initialized. Hopefully, the address won't get used.
	 * Also force trailers to be off on this interface.
	 */
	bzero((caddr_t)&ifr, sizeof(ifr));
	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	sin->sin_family = AF_INET;
	sin->sin_addr = in_makeaddr(INADDR_ANY, 
		(u_int) 0xFFFFFF );
	ifp->if_flags |= IFF_NOTRAILERS | IFF_BROADCAST;
	if (in_control((struct socket *)0, SIOCSIFADDR, (caddr_t)&ifr, ifp))
		printf("revarp: can't set temp inet addr\n");
	if (revarp) {
		myaddr.s_addr = 0;
		revarp_start(ifp);
		s = splimp();
		while (myaddr.s_addr == 0)
			(void) sleep((caddr_t)&myaddr, PZERO-1);
		(void) splx(s);
		sin->sin_addr = myaddr;
		if (in_control((struct socket*)0, SIOCSIFADDR, (caddr_t)&ifr, ifp))
			printf("revarp: can't set perm inet addr\n");
	}
}

void
revarp_start(ifp)
	register struct ifnet *ifp;
{
	register struct mbuf *m;
	register struct ether_arp *ea;
	register struct ether_header *eh;
	static int retries = 0;
	struct ether_addr myether;
	struct sockaddr sa;

	if (myaddr.s_addr != 0) {
		if (retries >= 2)
			printf("Found Internet address %x\n", myaddr.s_addr);
		retries = 0;
		return;
	}
	(void) localetheraddr((struct ether_addr *)NULL, &myether);
	if (++retries == 2) {
		printf("revarp: Requesting Internet address for %s\n",
		    ether_sprintf(&myether));
	}
	if ((m = m_get(M_DONTWAIT, MT_DATA)) == NULL)
		panic("revarp: no mbufs");
	m->m_len = sizeof(struct ether_arp);
#ifdef notdef
	m->m_data = MMAXOFF - m->m_len;
#endif
	ea = mtod(m, struct ether_arp *);
	bzero((caddr_t)ea, sizeof (*ea));

	sa.sa_family = AF_UNSPEC;
	eh = (struct ether_header *)sa.sa_data;
	bcopy((caddr_t)etherbroadcastaddr,(caddr_t)eh->ether_dhost,
			sizeof(eh->ether_dhost));
	bcopy((caddr_t)&myether,(caddr_t)eh->ether_shost,sizeof(eh->ether_shost));
	eh->ether_type = ETHERTYPE_REVARP;

	ea->arp_hrd = htons(ARPHRD_ETHER);
	ea->arp_pro = htons(ETHERTYPE_IP);
	ea->arp_hln = sizeof(ea->arp_sha);	/* hardware address length */
	ea->arp_pln = sizeof(ea->arp_spa);	/* protocol address length */
	ea->arp_op = htons(REVARP_REQUEST);
	bcopy((caddr_t)&myether,(caddr_t)ea->arp_sha,sizeof(ea->arp_sha));
	bcopy((caddr_t)&myether,(caddr_t)ea->arp_tha,sizeof(ea->arp_tha));
	(*ifp->if_output)(ifp, m, &sa);
	timeout(revarp_start, (caddr_t)ifp, 3*hz);
}



int revarpdebug = 1;

/*
 * Reverse-ARP input. If this is a request we look the ethernet address
 * of the sender up in the arp table (server side).
 * If this is a response, the incoming packet contains our internet address (client).
 */
void
revarpinput(ac, m)
	register struct arpcom *ac;
	struct mbuf *m;
{
#	define ARPTAB_SIZE arptab_size
	extern int arptab_size;
	extern struct arptab *arptab;
	register struct ether_arp *ea;
	register struct arptab *at = 0;
	register struct ether_header *eh;
	struct ether_addr myether;
	struct ifnet *ifp;
	struct ifaddr *ifa;
        struct sockaddr sa;
#ifdef notdef
	IF_ADJ(m);
#endif
	ea = mtod(m, struct ether_arp *);
	if (m->m_len < sizeof *ea)
		goto out;
	if (ac->ac_if.if_flags & IFF_NOARP)
		goto out;
	if (ntohs(ea->arp_pro) != ETHERTYPE_IP)
		goto out;
	if(!revarp)
		goto out;
	switch(ntohs(ea->arp_op)) {
	case REVARP_REPLY:
		(void) localetheraddr((struct ether_addr *)NULL, &myether);
		if (bcmp((caddr_t)ea->arp_tha, (caddr_t)&myether, 6) == 0) {
			bcopy((caddr_t)ea->arp_tpa, (caddr_t)&myaddr, sizeof(myaddr));
			wakeup((caddr_t)&myaddr);
		}
		break;

	case REVARP_REQUEST:
 		for (at = arptab ; at < &arptab[ARPTAB_SIZE] ; at++) {
                        if (at->at_flags & ATF_PERM &&
                            !bcmp((caddr_t)at->at_enaddr,
                            (caddr_t)ea->arp_tha, 6))
                                break;
                }
                if (at < &arptab[ARPTAB_SIZE]) {
                        /* found a match, send it back */
                        eh = (struct ether_header *)sa.sa_data;
                        bcopy(ea->arp_sha, eh->ether_dhost, 
				sizeof(ea->arp_sha));
                        bcopy((caddr_t)(&at->at_iaddr), ea->arp_tpa,
				sizeof(at->at_iaddr));
			/* search for interface address to use */
			ifp = &ac->ac_if;
			for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
				if (ifa->ifa_ifp == ifp) {
				    bcopy((caddr_t)&((struct sockaddr_in *)&ifa->ifa_addr)->sin_addr,
					ea->arp_spa, sizeof(ea->arp_spa));
				    break;
				}
			}
			if (ifa == 0) {
				if (revarpdebug)
				    printf("revarp: can't find ifaddr\n");
				break;
			}
			bcopy((caddr_t)ac->ac_enaddr, (caddr_t)ea->arp_sha,
			    sizeof(ea->arp_sha));
			bcopy((caddr_t)ac->ac_enaddr, (caddr_t)eh->ether_shost,
			    sizeof(ea->arp_sha));
                        eh->ether_type = ETHERTYPE_REVARP;
                        ea->arp_op = htons(REVARP_REPLY);
                        sa.sa_family = AF_UNSPEC;
                        if (revarpdebug) {
                                printf("revarp reply to %X from %X\n", 
					ntohl(*(u_int *)ea->arp_tpa),
					ntohl(*(u_int *)ea->arp_spa));
			}
                        (*ac->ac_if.if_output)(&ac->ac_if, m, &sa);
                        return;
		}
		break;

	default:
		break;
	}
out:
	m_freem(m);
	return;
}

localetheraddr(hint, result)
	struct ether_addr *hint, *result;
{
	static int found = 0;
	static struct ether_addr addr;

	if (!found) {
		found = 1;
		if (hint == NULL)
			return (0);
		addr = *hint;
		printf("Ethernet address = %s\n", ether_sprintf(&addr) );
	}
	if (result != NULL)
		*result = addr;
	return (1);
}
