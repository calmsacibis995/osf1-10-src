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
static char *rcsid = "@(#)$RCSfile: print-isoclns.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/21 14:34:28 $";
#endif
/*
 * Copyright (c) 1988-1990 The Regents of the University of California.
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
 * Original code by Matt Thomas, Digital Equipment Corporation
 */

/*
 * Based on:
 * static char rcsid[] = "/staff_3/mogul/code/tcpdump/tcpdump-2.2.1/RCS/print-isoclns.c,v 1.1 1992/09/17 20:09:50 mogul Exp $ (LBL)";
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include "interface.h"
#include "addrtoname.h"
#include "etherproto.h"

#define	CLNS	129
#define	ESIS	130
#define	ISIS	131
#define	NULLNS	0

void
isoclns_print(p, length, caplen, esrc, edst)
	u_char *p;
	int length;
	int caplen;
	u_char *esrc;
	u_char *edst;
{
	if (caplen < 1) {
	    printf("[|iso-clns] ");
	    if (!eflag)
		printf("%s > %s",
			etheraddr_string(esrc),
			etheraddr_string(edst));
	    return;
	}

	switch (*p) {
		case CLNS:
			/* esis_print(&p, &length); */
			printf("iso-clns");
			if (!eflag)
				(void)printf(" %s > %s",
					etheraddr_string(esrc),
					etheraddr_string(edst));
			break;
		case ESIS:
 			printf("iso-esis");
			if (!eflag)
				(void)printf(" %s > %s",
					etheraddr_string(esrc),
					etheraddr_string(edst));
			esis_print(p, length);
			return;
		case ISIS:
			printf("iso-isis");
			if (!eflag)
				(void)printf(" %s > %s",
					etheraddr_string(esrc),
					etheraddr_string(edst));
			/* isis_print(&p, &length); */
			(void)printf(" len=%d ", length);
			if (caplen > 1) {
				default_print_unaligned(p, caplen);
			}
			break;
		case NULLNS:
			printf("iso-nullns");
			if (!eflag)
				(void)printf(" %s > %s",
					etheraddr_string(esrc),
					etheraddr_string(edst));
			break;
		default:
			printf("iso-clns %02x", p[0]);
			if (!eflag)
				(void)printf(" %s > %s",
					etheraddr_string(esrc),
					etheraddr_string(edst));
			(void)printf(" len=%d ", length);
			if (caplen > 1) {
				default_print_unaligned(p, caplen);
			}
			break;
	}

}

#define	ESIS_REDIRECT	6
#define	ESIS_ESH	2
#define	ESIS_ISH	4

struct esis_hdr {
	u_char version;
	u_char reserved;
	u_char type;
	u_char tmo[2];
	u_char cksum[2];
};

esis_print(p, length)
	u_char *p;
	int length;
{
	u_char *ep;
	int li = p[1]; 
	struct esis_hdr *eh = (struct esis_hdr *) &p[2];
	u_char cksum[2];


	if (length == 2) {
		if (qflag)
			printf(" bad pkt!");
		else
			printf(" no header at all!");
		return;
	}

	ep = p + li;

	if (li > length) {
		if (qflag)
			printf(" bad pkt!");
		else
			printf(" LI(%d) > PDU size (%d)!", li, length);
		return;
	}
	if (li < sizeof(struct esis_hdr) + 2) {
		if (qflag)
			printf(" bad pkt!");
		else {
			printf(" too short for esis header %d:", li);
			while (length--)
				printf("%02X", *p++);
			printf("");
		}
		return;
	}
	switch (eh->type & 0x1f) {
		case ESIS_REDIRECT:
			printf(" redirect");
			break;
		case ESIS_ESH:
			printf(" esh");
			break;
		case ESIS_ISH:
			printf(" ish");
			break;
		default:
			printf(" type %d", eh->type & 0x1f);
			break;
	}

	if (vflag && osi_cksum(p, li, eh->cksum, cksum)) {
		printf(" bad cksum (got %02x%02x want %02x%02x)",
			eh->type & 0x1f,
			eh->cksum[1], eh->cksum[0], cksum[1], cksum[0]);
		return;
	}

	if (eh->version != 1) {
		printf(" unsupported version %d", eh->version);
		return;
	}

	p += sizeof(*eh) + 2;
	li -= sizeof(*eh) + 2;	/* protoid * li */

	switch (eh->type & 0x1f) {
		case ESIS_REDIRECT: {
			u_char *dst, *snpa, *is;
			dst = p; p += *p + 1;
			if (p > snapend)
				return;
			printf(" %s", isonsap_string(dst));
			snpa = p; p += *p + 1;
			is = p;   p += *p + 1;
			if (p > snapend)
				return;
			if (p > ep) {
				printf(" [bad li]");
				return;
			}
			if (is[0] == 0) {
				printf(" > %s", etheraddr_string(&snpa[1]));
			} else {
				printf(" > %s", isonsap_string(is));
			}
			li = ep - p;
			break;
		}
#if 0
		case ESIS_ESH:
			printf(" esh");
			break;
#endif
		case ESIS_ISH: {
			u_char *is;
			is = p; p += *p + 1;
			if (p > ep) {
				printf(" [bad li]");
				return;
			}
			if (p > snapend)
				return;
			printf(" %s", isonsap_string(is));
			li = ep - p;
			break;
		}
		default:
			(void)printf(" len=%d", length);
			if (length && p < snapend) {
				length = snapend - p;
				if ((p - (u_char *) 0) & 1) {
					memmove(p - 1, p, length);
					p--;
				}
				default_print((u_short *)p, length);
			}
			return;
	}

	if (vflag)
		while (p < ep && li) {
			int op, opli;
			u_char *q;
			if (snapend - p < 2)
				return;
			if (li < 2) {
				printf(" bad opts/li");
				return;
			}
			op = *p++;
			opli = *p++;
			li -= 2;
			if (opli > li) {
				printf(" opt (%d) too long", op);
				return;
			}
			li -= opli;
			q = p;
			p += opli;
			if (snapend < p)
				return;
			switch (op) {
			    case 198:
				if (opli == 2) {
					printf(" tmo=%d", q[0] * 256 + q[1]);
					continue;
				}
			    default:
				break;
			}
			printf (" %d:<", op);
			while (opli--)
				printf ("%02x", *q++);
			printf (">");
		}
}

int osi_cksum(p, len, off, cksum)
	register u_char *p;
	register int len;
	u_char *off;
	u_char *cksum;
{
	int x, y, f = (len - ((off - p) + 1));
	long c0 = 0, c1 = 0;

	if ((cksum[0] = off[0]) == 0 && (cksum[1] = off[1]) == 0)
	    return 0;

	off[0] = off[1] = 0;
	for (; len > 0; p++, len--) {
		c0 += *p;
		c1 += c0;
		c0 %= 255;
		c1 %= 255;
	}

	x = (c0 * f - c1);
	if (x < 0)
		x = 255 - (-x % 255);
	else
		x %= 255;
	y = -1 * (x + c0);
	if (y < 0)
		y = 255 - (-y % 255);
	else
		y %= 255;

	off[0] = x;
	off[1] = y;

	return (off[0] != cksum[0] || off[1] != cksum[1]);
}
