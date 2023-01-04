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
static char *rcsid = "@(#)$RCSfile: print-llc.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/21 14:34:30 $";
#endif
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Code by Matt Thomas, Digital Equipment Corporation
 *	with an awful lot of hacking by Jeffrey Mogul, DECWRL
 */

/*
 * Based on:
 * static  char rcsid[] = "print-llc.c,v 1.3 92/07/01 17:49:53 mogul Exp $";
 */

#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>

#include "interface.h"
#include "addrtoname.h"

/* JSD: added these externs; build environment doesn't find my "addrtoname.h" */
extern char *dnaddr_string();
extern char *etheraddr_string();
extern char *etherproto_string();
extern char *llcsap_string();
extern char *tcpport_string();
extern char *udpport_string();
extern char *getname();
extern char *intoa();

#include "llc.h"

/*
 * Returns non-zero IFF it succeeds in printing the header
 */
llc_print(p, length, caplen, esrc, edst)
	u_char *p;
	int length;
	int caplen;
	u_char *esrc;		/* src Ether/FDDI host */
	u_char *edst;		/* dst Ether/FDDI host */
{
	struct llc llc;

	if (caplen < 3) {
		(void)printf("[|llc]");
		default_print((u_short *)p, caplen);
		return(0);
	}

	/* Watch out for possible alignment problems */
	bcopy((char *)p, (char *)&llc, min(caplen, sizeof(llc)));

	if (llc.ssap == LLCSAP_ISONS && llc.dsap == LLCSAP_ISONS
	    && llc.llcui == LLC_UI) {
#ifdef	ISO
		return(isoclns_print(p + 3, length - 3, caplen - 3,
					esrc, edst);
#else
		printf("ISO CLNS");
		return(1);
#endif
	}

	if (llc.ssap == LLCSAP_SNAP && llc.dsap == LLCSAP_SNAP
	    && llc.llcui == LLC_UI) {
		if (caplen < sizeof(llc)) {
		    (void)printf("[|llc-snap]");
		    default_print((u_short *)p, caplen);
		    return(0);
		}
		if (vflag) 
			(void)printf("snap %s ", protoid_string(llc.llcpi));

		caplen -= sizeof(llc);
		length -= sizeof(llc);
		p += sizeof(llc);

		/* XXX
		 * Sometimes I see org-code = 08:00:2B, which
		 * looks rather DEC-ish; what should we do about
		 * that?
		 */
		if (llc.orgcode[0] == 0 && llc.orgcode[1] == 0 &&
					    llc.orgcode[2] == 0) {
		    /* This is an encapsulated Ethernet packet */
		    u_short et;

		    /* Misalignment not possible here */
		    et = *((u_short *)llc.ethertype);
		    et = ntohs(et);
		    return(ether_encap_print(et, p, length, caplen));
		}
		
		if (caplen > 0)
			default_print(p, caplen);
		return(1);
	}
	
	if ((llc.ssap & ~LLC_GSAP) == llc.dsap) {
		if (eflag)
			(void)printf("%s ", llcsap_string(llc.dsap));
		else
			(void)printf("%s > %s %s ",
					etheraddr_string(esrc),
					etheraddr_string(edst),
					llcsap_string(llc.dsap));
	} else {
		if (eflag)
			(void)printf("%s > %s ",
				llcsap_string(llc.ssap & ~LLC_GSAP),
				llcsap_string(llc.dsap));
		else
			(void)printf("%s %s > %s %s ",
				etheraddr_string(esrc),
				llcsap_string(llc.ssap & ~LLC_GSAP),
				etheraddr_string(edst),
				llcsap_string(llc.dsap));
	}

	if ((llc.llcu & LLC_U_FMT) == LLC_U_FMT) {
		char *m, f, msg[3];
		switch (LLC_U_CMD(llc.llcu)) {
			case LLC_UI:
				m = "ui";
				break;
			case LLC_TEST:
				m = "test";
				break;
			case LLC_XID:
				m = "xid";
				break;
			case LLC_UA:
				m = "ua";
				break;
			case LLC_DISC:
				m = "disc";
				break;
			case LLC_DM:
				m = "dm";
				break;
			case LLC_SABME:
				m = "sabme";
				break;
			case LLC_FRMR:
				m = "frmr";
				break;
			default:
				(void)sprintf(msg, "%02x",
						LLC_U_CMD(llc.llcu));
				m = msg;
				break;
		}

		switch ((llc.ssap & LLC_GSAP) | (llc.llcu & LLC_U_POLL)) {
		    case 0:			f = 'C'; break;
		    case LLC_GSAP:		f = 'R'; break;
		    case LLC_U_POLL:		f = 'P'; break;
		    case LLC_GSAP|LLC_U_POLL:	f = 'F'; break;
		}

		printf("%s/%c", m, f);

		p += 3;
		length -= 3;
		caplen -= 3;

		if ((llc.llcu & ~LLC_U_POLL) == LLC_XID) {
		    if (*p == LLC_XID_FI) {
			printf(": %02x %02x", p[1], p[2]);
			p += 3;
			length -= 3;
			caplen -= 3;
		    }
		}
	} else {
		char f;
		llc.llcis = ntohs(llc.llcis);
		switch ((llc.ssap & LLC_GSAP) | (llc.llcu & LLC_U_POLL)) {
		    case 0:			f = 'C'; break;
		    case LLC_GSAP:		f = 'R'; break;
		    case LLC_U_POLL:		f = 'P'; break;
		    case LLC_GSAP|LLC_U_POLL:	f = 'F'; break;
		}

		if ((llc.llcu & LLC_S_FMT) == LLC_S_FMT) {
			static char *llc_s[] = { "rr", "rej", "rnr", "03" };
			(void)printf("%s (r=%d,%c)",
				llc_s[LLC_S_CMD(llc.llcis)],
				LLC_IS_NR(llc.llcis),
				f);
		} else {
			(void)printf("I (s=%d,r=%d,%c)",
				LLC_I_NS(llc.llcis),
				LLC_IS_NR(llc.llcis),
				f);
		}
		p += 4;
		length -= 4;
		caplen -= 4;
	}
	(void)printf(" len=%d", length);
	if (caplen > 0) {
		default_print_unaligned(p, caplen);
	}
	return(1);
}
