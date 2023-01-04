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
 * @(#)$RCSfile: os-ultrix.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 18:40:54 $
 */
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
 *
 * Based on:
 * os-ultrix.h,v 1.8 90/09/21 02:12:14 mccanne Exp $ (LBL)
 */

#ifndef	IPPROTO_ND
/* Trying to compile this not on a Sun system */
#define	IPPROTO_ND 77	/* From <netinet/in.h> on a Sun somewhere */
#endif

#ifndef ETHERTYPE_REVARP
/* some systems don't define this */
#define ETHERTYPE_REVARP	0x8035
#define REVARP_REQUEST		3
#define REVARP_REPLY		4
#endif

/* Map things in the ether_arp struct */
#define SHA(ap) ((ap)->arp_sha)
#define SPA(ap) ((ap)->arp_spa)
#define THA(ap) ((ap)->arp_tha)
#define TPA(ap) ((ap)->arp_tpa)

#define EDST(ep) ((ep)->ether_dhost)
#define ESRC(ep) ((ep)->ether_shost)

/* Map protocol types */
#define ETHERPUP_IPTYPE ETHERTYPE_IP
#define ETHERPUP_REVARPTYPE ETHERTYPE_REVARP
#define ETHERPUP_ARPTYPE ETHERTYPE_ARP

/* newish RIP commands */
#ifndef	RIPCMD_POLL
#define	RIPCMD_POLL 5
#endif
#ifndef	RIPCMD_POLLENTRY
#define	RIPCMD_POLLENTRY 6
#endif
