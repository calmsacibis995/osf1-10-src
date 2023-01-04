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
static char *sccsid = "@(#)$RCSfile: dli_timer.c,v $ $Revision: 4.4.6.3 $ (DEC) $Date: 1993/10/22 22:21:03 $";
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
 * IDENT HISTORY:
 *
 * 1.00 10-Jul-1985
 *      DECnet-ULTRIX   V1.0
 *
 * 2.00 18-Apr-1986
 *		DECnet-Ultrix	V2.0
 *
 * Added sysid and point-to-point support
 *
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
 *		d l i _ s l o w t i m o
 *
 * Slow timeout routine is entered every 500ms.  If loopback timer is
 * nonzero, it is decremented.  If the loopback timers makes a transition
 * to zero, a "passive loopback terminated" message is forwarded to EVL.
 *
 * Returns:		Nothing
 *
 * Inputs:		None
 */
void
dli_slowtimo(
    void)
{
    register struct dli_ifnet *dlif;
    int s = splnet();

    for (dlif = dli_dlifs; dlif != NULL; dlif = dlif->dlif_next) {
	if (dlif != NULL && (dlif->dlif_flags & DLIF_RUNNING)) {
	    if (dlif->dlif_to && --dlif->dlif_to == 0) {	/* time to tx? */
		dlif->dlif_to = dlif->dlif_tr;			/* reset timers */
		/*
		 * Transmit V3 then V4 MOP SYSID
		 */
		dli_snd_sysid(dlif, dlif->dlif_mopv3_filter, 0, &dli_sysid_mcast);
		dli_snd_sysid(dlif, dlif->dlif_mopv4_filter, 0, &dli_sysid_mcast);
	    }
	}
    }
    splx(s);

    return;
}

/*
 *		d l i _ s n d _ s y s i d
 *
 * This routine is called to transmit the sysid for a particular qna
 *
 * Note:  Protocol lock must be asserted before routine called.
 *
 * Outputs:		None.
 *
 * Inputs:		ptr to qna who is to transmit it's sysid.
 *			receipt for the sysid message.
 *			device type value for the sysid message.
 *
 * Version History:
 *
 */
void
dli_snd_sysid(
    register struct dli_ifnet *dlif,
    struct dli_filter *filter,
    int receipt,
    const union dli_addr *dst)
{
    struct mbuf *m;
    register u_char *dp;

    MGETHDR(m, M_DONTWAIT, MT_DATA);
    if (m == NULL)
        return;

    m->m_data += dlif->dlif_hdrlen;
    dp = mtod(m, u_char *);
    LE_PUT8(dp, DLI_SYSID);	/* +4 */	/* sysid code */
    LE_PUT8(dp, 0);				/* reserved */
    LE_PUT16(dp, receipt);			/* receipt number */

    LE_PUT16(dp, MAINTV);	/* +6 */	/* info type, maint ver */
    LE_PUT8(dp, (3*sizeof(u_char)));		/*   length of info */
    LE_PUT8(dp, VER4);				/*   V4 */
    LE_PUT8(dp, ECO);				/*   eco */
    LE_PUT8(dp, USER_ECO);			/*   user eco */

    LE_PUT16(dp, FUNCTIONS);	/* +5 */	/* info type, functions */
    LE_PUT8(dp, sizeof(u_short));		/*   length of info */
    if (filter == dlif->dlif_mopv3_filter) {
	LE_PUT16(dp, dlif->dlif_mopv3_functions);
    } else {
	LE_PUT16(dp, dlif->dlif_mopv4_functions);
    }

    LE_PUT16(dp, COMDEV);	/* +4 */	/* info type, device */
    LE_PUT8(dp, sizeof(u_char));		/*  length of info */
    LE_PUT8(dp, dlif->dlif_ifp->if_sysid_type);	/*  SYSID type code */

    LE_PUT16(dp, DATALINK);	/* +4 */	/* info type, datalink */
    LE_PUT8(dp, sizeof(u_char));		/*  length of info */
    switch (dlif->dlif_iftype) {
        case IFT_ETHER:
	    LE_PUT8(dp, DLI_DLCSMACD);
	    break;
        case IFT_FDDI:
	    LE_PUT8(dp, DLI_DLFDDI);
	    LE_PUT16(dp, STATIONID); /* +11 */	/* station id code */
	    LE_PUT8(dp, 8);			/* length of station id field */
	    LE_PUT8(dp, 0);			/* clear out and move ptr */
	    LE_PUT8(dp, 0);			/* clear out and move ptr */
	    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[0]);
	    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[1]);
	    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[2]);
	    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[3]);
	    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[4]);
	    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[5]);
	    break;
        case IFT_ISO88025:
	    LE_PUT8(dp, DLI_DLTRN);
	    break;
        default:
	    dp -= sizeof(u_short) + sizeof(u_char);
	    break;
    }

    LE_PUT16(dp, HADDR);	/* +9 */	/* info type, hardware addr */
    LE_PUT8(dp, 6);				/*  length of info */
    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[0]);
    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[1]);
    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[2]);
    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[3]);
    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[4]);
    LE_PUT8(dp, dlif->dlif_default_addr.un_addr[5]);

    if (filter == dlif->dlif_mopv4_filter) {
	struct utc utc;
	LE_PUT16(dp, SYSTIME);	/* +19 */
	LE_PUT8(dp, sizeof(struct utc));	/* length of systime */
	getutc(&utc);				/* get system time */
	bcopy((caddr_t) &utc, dp, sizeof(struct utc));
	dp += sizeof(struct utc);
    }

    m->m_len = dp - mtod(m, u_char *);
    m->m_pkthdr.len = m->m_len;

    /*
     * call this function for DECnet-OSI-specific MOP SYSID fields
     */
    if (dlimgtsw != NULL)
	(*dlimgtsw->mgt_sysid_extra)(dlif, filter->fltr_flags, m);

    if (dli_add_header(dlif, &m, &filter->fltr_llcdata, filter->fltr_flags) != 0)
	return;
    dli_output(dlif, filter, m, dst);
}
