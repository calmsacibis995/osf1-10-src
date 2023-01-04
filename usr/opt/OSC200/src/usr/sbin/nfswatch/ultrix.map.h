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
 * @(#)$RCSfile: ultrix.map.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/21 14:47:57 $
 */
/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Lawrence Berkeley Laboratory,
 * Berkeley, CA.  The name of the University may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Based on:
 * /home/harbor/davy/system/nfswatch/RCS/ultrix.map.h,v 4.0 1993/03/01 19:59:00 davy Exp $ (LBL)
 */

/*
 * Cloned from sunos4.map.h, although this could really be a single
 * file.
 */

/* Arcane method for discovering if this is Ultrix 4.0 */
#include <machine/cpuconf.h>
#ifndef ST_DS5100
#define ULTRIX40
#endif /* ST_DS5100 */

#ifndef	IPPROTO_ND
/* Trying to compile this not on a Sun system */
#define	IPPROTO_ND	77	/* From <netinet/in.h> on a Sun somewhere */
#endif	/* IPPROTO_ND */

#ifndef ETHERTYPE_REVARP
/* some systems don't define this */
#define ETHERTYPE_REVARP	0x8035
#define REVARP_REQUEST		3
#define REVARP_REPLY		4
#endif  /* ETHERTYPE_REVARP */

#ifdef ULTRIX40
struct ether_addr {
        u_char  ether_addr_octet[6];
};
#endif /* ULTRIX40 */

/* Map things in the ether_arp struct */
#define arp_xsha arp_sha
#define arp_xspa arp_spa
#define arp_xtha arp_tha
#define arp_xtpa arp_tpa

/* Map protocol types */
#define ETHERPUP_IPTYPE ETHERTYPE_IP
#define ETHERPUP_REVARPTYPE ETHERTYPE_REVARP
#define ETHERPUP_ARPTYPE ETHERTYPE_ARP

/* newish RIP commands */
#ifndef	RIPCMD_POLL
#define	RIPCMD_POLL	5
#endif	/* RIPCMD_POLL */
#ifndef	RIPCMD_POLLENTRY
#define	RIPCMD_POLLENTRY	6
#endif	/* RIPCMD_POLLENTRY */
