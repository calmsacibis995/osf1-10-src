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
#ifndef	lint
static char *sccsid = "@(#)$RCSfile: dli_sockopt.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/27 15:06:51 $";
#endif

/*
 *   COPYRIGHT (c) DIGITAL EQUIPMENT CORPORATION 1991.  ALL
 *   RIGHTS RESERVED.
 *
 *   THIS  SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE
 *   USED  AND  COPIED ONLY IN ACCORDANCE WITH THE TERMS OF
 *   SUCH  LICENSE  AND  WITH  THE  INCLUSION  OF THE ABOVE
 *   COPYRIGHT  NOTICE.   THIS SOFTWARE OR ANY OTHER COPIES
 *   THEREOF   MAY   NOT  BE  PROVIDED  OR  OTHERWISE  MADE
 *   AVAILABLE  TO  ANY  OTHER  PERSON.   NO  TITLE  TO AND
 *   OWNERSHIP OF THE SOFTWARE IS HEREBY TRANSFERRED.
 *
 *   THE  INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE
 *   WITHOUT  NOTICE  AND  SHOULD  NOT  BE  CONSTRUED  AS A
 *   COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 *
 *   DIGITAL  ASSUMES  NO  RESPONSIBILITY  FOR  THE  USE OR
 *   RELIABILITY  OF  ITS SOFTWARE ON EQUIPMENT THAT IS NOT
 *   SUPPLIED BY DIGITAL.
 *
 *
 *   Telecommunications & Networks
 *
 *   IDENT HISTORY:
 *
 *	1.00 10-Jul-1985
 *		DECnet-ULTRIX   V1.0
 *
 *	2.01 18-Mar-1988
 *		DECnet-ULTRIX   V2.4
 *		Allowed use of reserved bit in individual and group SAPs
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <kern/assert.h>
#include <net/if.h>
#include <dli/dli_var.h>

/*
 *		d l i _ g e t o p t
 *
 * Return requested option.
 *
 * Returns:		Error code if error, otherwise NULL.
 *			length of data in option buffer.
 *
 * Inputs:
 *	pcb		= Pointer to the user's line table entry for this request.
 *	optbuf		= Buffer containing option.
 *	optlen		= Pointer to length of data in option buffer.
 *	optnam		= Name of option.
 */
int
dli_getopt(
    register struct dli_pcb *pcb,
    register u_char *optbuf,
    int *optlen,
    int optnam)
{
    switch (optnam) {
	/*
	 * return list of group saps that this isap has enabled
	 */
	case DLI_GETGSAP: {
	    int gsap, cnt = 0;
	    for (cnt = 0, gsap = 3; gsap < 256; gsap += 2) {
		if (dli_filter_test_gsap(pcb->dli_filter, gsap)) {
		    optbuf[cnt++] = (u_char) gsap; 
		}
	    }
	    *optlen = cnt;
	    break;
	}
	case DLI_STATE:
	    *optbuf = pcb->dli_state;
	    *optlen = sizeof(u_char);
	    break;

	case DLI_INTERNALOOP:
	    *optbuf = pcb->dli_iloop;
	    *optlen = sizeof(u_char);
	    break;

	case DLI_MULTICAST: {
	    register u_short *dp = (u_short *) optbuf;
	    register int idx;
	    for (idx = pcb->dli_dlif->dlif_addrlist_size - 1; idx >= 0; idx--) {
		if (DLI_TSTADDR_IDX(idx, &pcb->dli_filter->fltr_addrset)) {
		    struct dli_mcast *mcast = &pcb->dli_dlif->dlif_addrlist[idx];
		    *dp = mcast->dlm_addr.un_w_addr[0];
		    *dp = mcast->dlm_addr.un_w_addr[1];
		    *dp = mcast->dlm_addr.un_w_addr[2];
		}
	    }
	    *optlen = (u_char *) dp - optbuf;
	    break;
	}

	default:
	    return ENOPROTOOPT;
    }
    return 0;
}

/*
 *		d l i _ s e t o p t
 *
 * Process a DLI request.
 *
 * Returns:		Error code if error, otherwise NULL.
 *
 * Inputs:
 *	pcb		= Pointer to the user's line table entry for this request.
 *	optbuf		= Buffer containg option.
 *	optlen		= Length of data in option buffer.
 *	optnam		= Name of option.
 */
int
dli_setopt(
    register struct dli_pcb *pcb,
    register u_char *optbuf,
    int optlen,
    int optnam)
{
    register int error = 0;

    switch (optnam)	{	/* store option only if it is valid */
	case DLI_SET802CTL: {
	    struct sockaddr_802 *dli802 = &pcb->dli_lineid.choose_addr.dli_802addr;
	    u_int ctl;
	    if (pcb->dli_lineid.dli_substructype != DLI_802)
		return ENOPROTOOPT;
	    if (optlen == 0 || optlen > 2)
		return EINVAL;
	    ctl = optbuf[0];
	    if (optlen == 1) {
		if (LLC_FMT(ctl) != LLC_U_FMT)
		    return EINVAL;
		if (dli802->svc == TYPE1) {
		    /*
		     * store 802.2 LLC control field for isaps with 
		     * class 1 service, but validate it first
		     */
		    switch (ctl) {
		    case TEST_PCMD:
		    case TEST_NPCMD:
		    case XID_PCMD:
		    case XID_NPCMD:
		        if (dli802->eh_802.ssap == SNAP_SAP)
			    return EINVAL;
		    case UI_NPCMD:
			break;
		    default:
			return EINVAL;
		    }
		}
		dli802->eh_802.ctl.U_fmt = ctl;
	    } else if (dli802->svc == TYPE1 && LLC_FMT(ctl) != LLC_U_FMT) {
	        return EINVAL;
	    } else {
		dli802->eh_802.ctl.I_S_fmt = *(u_short *)optbuf;
	    }
	    break;
	}

	case DLI_DISGSAP:
	case DLI_ENAGSAP: {
	    int (*rtn)(struct dli_ifnet *, struct dli_filter *, u_int);
	    /* 
	     * enable a list of group saps for this isap
	     */
	    if (optnam == DLI_ENAGSAP)
		rtn = dli_filter_add_gsap;
	    else
		rtn = dli_filter_remove_gsap;
	    if (optlen >= 0) while (optlen-- != 0) {
		if (error = (*rtn)(pcb->dli_dlif, pcb->dli_filter, *optbuf++))
		    return error;
	    }
	    break;
	}
	case DLI_STATE:
	    return ENOPROTOOPT;

	case DLI_INTERNALOOP:
	    if (optlen != 1 || (*optbuf != DLP_IOFF && *optbuf != DLP_ION))
		return ENOPROTOOPT;
	    pcb->dli_iloop = *optbuf;
	    break;

	/*
	 * Set multicast address(es). Verify correct number and
	 * validity of addresses.
	 */
	case DLI_MULTICAST: {
	    struct ether_pa *mcast_list = (struct ether_pa *) optbuf, *mcast;
	    struct dli_addrset new_addrset;
	    union dli_addr addr;
	    int count;

	    if (optlen % sizeof(struct ether_pa) != 0)
		return ENOPROTOOPT;

	    dli_addrset_init(pcb->dli_dlif, pcb->dli_filter, &new_addrset);

	    addr.un_w_addr[3] = 0;
	    count = optlen/sizeof(struct ether_pa);
	    for (mcast = mcast_list; error == 0 && count > 0; count--, mcast++) {
		if ((mcast->ether_addr_octet[0] & 1) == 0) {
		    error = EINVAL;
		    break;
		}
		addr.un_w_addr[0] = *(u_short *) &mcast->ether_addr_octet[0];
		addr.un_w_addr[1] = *(u_short *) &mcast->ether_addr_octet[2];
		addr.un_w_addr[2] = *(u_short *) &mcast->ether_addr_octet[4];
		error = dli_addrset_add(pcb->dli_dlif, pcb->dli_filter, &new_addrset, &addr);
	    }
	    if (error) {
		dli_addrset_destroy(pcb->dli_dlif, pcb->dli_filter, &new_addrset);
		return error;
	    }
	    dli_addrset_destroy(pcb->dli_dlif, pcb->dli_filter, &pcb->dli_filter->fltr_addrset);
	    pcb->dli_filter->fltr_addrset = new_addrset;
	    break;
	}

	default:
	    return ENOPROTOOPT;
    }

    return error;
}
