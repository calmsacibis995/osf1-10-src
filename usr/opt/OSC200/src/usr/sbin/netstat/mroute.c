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
static char *rcsid = "@(#)$RCSfile: mroute.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1994/01/11 22:43:54 $";
#endif

/*
 * Print DVMRP multicast routing structures and statistics.
 *
 * MROUTING 1.0
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <netinet/in.h>
#include <netinet/igmp.h>
#define _KERNEL 1
#include <netinet/ip_mroute.h>
#undef _KERNEL

extern int kmem;
extern int nflag;
extern char *routename();
extern char *netname();
extern char *plural();
extern char *plurales();

void
mroutepr(mrpaddr, mrtaddr, vifaddr)
	off_t mrpaddr, mrtaddr, vifaddr;
{
	u_int mrtproto;
	struct mbuf *mrttable[MRTHASHSIZ];
	struct vif viftable[MAXVIFS];
	struct mrt *mrt;
	struct mbuf mb;
	struct mbuf *mp;
	register struct vif *v;
	register vifi_t vifi;
	struct in_addr *grp;
	int i;
	int banner_printed;
	int saved_nflag;

	if (mrpaddr == 0) {
		printf("netstat: ip_mrtproto symbol not in namelist\n");
		return;
	}

	klseek(kmem, mrpaddr, 0);
	read(kmem, (char *)&mrtproto, sizeof(mrtproto));
	switch (mrtproto) {
	    case 0:
		printf("netstat: no multicast routing compiled into this system\n");
		return;

	    case IGMP_DVMRP:
		break;

	    default:
		printf("netstat: multicast routing protocol %u, unknown\n", 
			mrtproto);
		return;
	}

	if (mrtaddr == 0) {
		printf("netstat: mrttable symbol not in namelist\n");
		return;
	}
	if (vifaddr == 0) {
		printf("netstat: viftable symbol not in namelist\n");
		return;
	}

	saved_nflag = nflag;
	nflag = 1;

	klseek(kmem, vifaddr, 0);
	read(kmem, (char *)viftable, sizeof(viftable));
	banner_printed = 0;
	for (vifi = 0, v = viftable; vifi < MAXVIFS; ++vifi, ++v) {

		if (v->v_lcl_addr.s_addr == 0)
			continue;

		if (!banner_printed) {
			printf("\nVirtual Interface Table\n%s%s",
			       " Vif   Threshold   Local-Address   ",
			       "Remote-Address   Groups\n");
			banner_printed = 1;
		}

		printf(" %2u       %3u      %-15.15s",
			vifi, v->v_threshold, routename(v->v_lcl_addr));
		printf(" %-15.15s\n",
			(v->v_flags & VIFF_TUNNEL) ?
				routename(v->v_rmt_addr) : "");

		for (mp = v->v_lcl_groups; mp != NULL; mp = mb.m_next) {
			klseek(kmem, (off_t)mp, 0);
			read(kmem, (char *)&mb, sizeof(mb));
			grp = (struct in_addr *)mb.m_dat;
			i = mb.m_len / 4;
			while (i--) {
				printf("%51s %-15.15s\n", "",
					routename(*grp++));
			}
		}
	}
	if (!banner_printed)
		printf("\nVirtual Interface Table is empty\n");

	klseek(kmem, mrtaddr, 0);
	read(kmem, (char *)mrttable, sizeof(mrttable));
	banner_printed = 0;
	for (i = 0; i < MRTHASHSIZ; ++i) {
	    for (mp = mrttable[i]; mp != NULL; mp = mb.m_next) {

		if (!banner_printed) {
			printf("\nMulticast Routing Table\n%s",
			       " Hash  Origin-Subnet  In-Vif  Out-Vifs\n");
			banner_printed = 1;
		}

		klseek(kmem, (off_t)mp, 0);
		read(kmem, (char *)&mb, sizeof(mb));
		mrt = (struct mrt *)mb.m_dat;
		printf(" %3u   %-15.15s  %2u   ",
			i,
			netname(mrt->mrt_origin,
				ntohl(mrt->mrt_originmask.s_addr)),
			mrt->mrt_parent);
		for (vifi = 0; vifi < MAXVIFS; ++vifi) {
			if (VIFM_ISSET(vifi, mrt->mrt_children)) {
				printf(" %u%c",
					vifi,
					VIFM_ISSET(vifi, mrt->mrt_leaves) ?
					'*' : ' ');
			}
		}
		printf("\n");
	    }
	}
	if (!banner_printed)
		printf("\nMulticast Routing Table is empty\n");

	printf("\n");
	nflag = saved_nflag;
}

void
mrt_stats(mrpaddr, mstaddr)
	off_t mrpaddr, mstaddr;
{
	u_int mrtproto;
	struct mrtstat mrtstat;

	if(mrpaddr == 0) {
		printf("netstat: ip_mrtproto symbol not in namelist\n");
		return;
	}

	klseek(kmem, mrpaddr, 0);
	read(kmem, (char *)&mrtproto, sizeof(mrtproto));
	switch (mrtproto) {
	    case 0:
		printf("netstat: no multicast routing compiled into this system\n");
		return;

	    case IGMP_DVMRP:
		break;

	    default:
		printf("netstat: multicast routing protocol %u, unknown\n", 
			mrtproto);
		return;
	}

	if (mstaddr == 0) {
		printf("netstat: mrtstat symbol not in namelist\n");
		return;
	}

	klseek(kmem, mstaddr, 0);
	read(kmem, (char *)&mrtstat, sizeof(mrtstat));
	printf("multicast routing:\n");
	printf(" %10u multicast route lookup%s\n",
	  mrtstat.mrts_mrt_lookups, plural(mrtstat.mrts_mrt_lookups));
	printf(" %10u multicast route cache miss%s\n",
	  mrtstat.mrts_mrt_misses, plurales(mrtstat.mrts_mrt_misses));
	printf(" %10u group address lookup%s\n",
	  mrtstat.mrts_grp_lookups, plural(mrtstat.mrts_grp_lookups));
	printf(" %10u group address cache miss%s\n",
	  mrtstat.mrts_grp_misses, plurales(mrtstat.mrts_grp_misses));
	printf(" %10u datagram%s with no route for origin\n",
	  mrtstat.mrts_no_route, plural(mrtstat.mrts_no_route));
	printf(" %10u datagram%s with malformed tunnel options\n",
	  mrtstat.mrts_bad_tunnel, plural(mrtstat.mrts_bad_tunnel));
	printf(" %10u datagram%s with no room for tunnel options\n",
	  mrtstat.mrts_cant_tunnel, plural(mrtstat.mrts_cant_tunnel));
	printf(" %10u datagram%s arrived on wrong interface\n",
	  mrtstat.mrts_wrong_if, plural(mrtstat.mrts_wrong_if));
}
