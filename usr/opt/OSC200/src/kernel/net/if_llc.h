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
 *	@(#)$RCSfile: if_llc.h,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/08/31 19:00:31 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base:	if_llc.h	7.1 (Berkeley) 9/4/89
 *	Merged:	if_llc.h	7.2 (Berkeley) 6/28/90
 */

#ifndef _IF_LLC_H_
#define _IF_LLC_H_

/*
 * IEEE 802.2 Link Level Control headers, for use in conjunction with
 * 802.{3,4,5} media access control methods.
 *
 * Headers here do not use bit fields due to shortcommings in many
 * compilers.
 */

struct llc {
	u_char	llc_dsap;
	u_char	llc_ssap;
	union {
	    struct {
		u_char control;
		u_char format_id;
		u_char xid_class;
		u_char window_x2;
	    } type_u;
	    struct {
		u_char num_snd_x2;
		u_char num_rcv_x2;
	    } type_i;
	    struct {
		u_char control;
		u_char num_rcv_x2;
	    } type_s;
	    struct {
		u_short control;
	    } type_is;
	    struct {
		u_char control;
		u_char org_code[3];
		u_short ether_type;
	    } type_snap;
	} llc_un;
};
#define llc_control llc_un.type_u.control
#define llc_is_control llc_un.type_is.control
#define llc_fid llc_un.type_u.format_id
#define llc_class llc_un.type_u.xid_class
#define llc_window llc_un.type_u.window_x2
#define	llc_org_code llc_un.type_snap.org_code
#define	llc_ether_type llc_un.type_snap.ether_type
#define llc_snap llc_un.type_snap

#define LLC_UI		0x3
#define LLC_UI_P	0x13
#define LLC_XID		0xaf
#define LLC_XID_P	0xbf
#define LLC_TEST	0xe3
#define LLC_TEST_P	0xf3

#define LLC_ISO_LSAP	0xfe
#define LLC_SNAP_LSAP	0xaa

#define	LLC_XID_LEN	3
#define	LLC_TEST_LEN	3
#define	LLC_UICMD_LEN	3
#define	LLC_SNAP_LEN	8

#if BYTE_ORDER == LITTLE_ENDIAN
#ifdef FUTURE /* not all drivers (ie. defta) give us quad-word aligned data */
#define	LLC_SNAPSAP_ETHER_DATA	0x000000000003AAAAL
#define	LLC_SNAPSAP_ETHER_MASK	0x0000FFFFFFFFFFFFL

#define	LLC_ISOSAP_DATA		0x000000000003FEFEL
#define	LLC_ISOSAP_MASK		0x0000000000FFFFFFL

#define	LLC_SNAPSAP_DATA	0x000000000003AAAAL
#define	LLC_SNAPSAP_MASK	0x0000000000FFFFFFL
#else
#define	LLC_SNAPSAP1_ETHER_DATA	0x0003AAAA
#define	LLC_SNAPSAP2_ETHER_DATA	0x00000000
#define	LLC_SNAPSAP2_ETHER_MASK	0x0000FFFF

#define	LLC_ISOSAP_DATA		0x0003FEFE
#define	LLC_ISOSAP_MASK		0x00FFFFFF

#define	LLC_SNAPSAP_DATA	0x0003AAAA
#define	LLC_SNAPSAP_MASK	0x00FFFFFF
#endif
#elif BYTE_ORDER == BIG_ENDIAN
#define	LLC_SNAPSAP1_ETHER_DATA	0xAAAA0300
#define	LLC_SNAPSAP2_ETHER_DATA	0x00000000
#define	LLC_SNAPSAP2_ETHER_MASK	0xFFFF0000

#define	LLC_ISOSAP_DATA		0xFEFE0300
#define	LLC_ISOSAP_MASK		0xFFFFFF00

#define	LLC_SNAPSAP_DATA	0xAAAA0300
#define	LLC_SNAPSAP_MASK	0xFFFFFF00
#endif

#ifdef FUTURE
#define	LLC_IS_ENCAP_ETHER(ll)	(((ll)[0] & LLC_SNAPSAP_ETHER_MASK) == LLC_SNAPSAP_ETHER_DATA)
#else
#define	LLC_IS_ENCAP_ETHER(ll)	((ll)[0] == LLC_SNAPSAP1_ETHER_DATA && \
	((ll)[1] & LLC_SNAPSAP2_ETHER_MASK) == LLC_SNAPSAP2_ETHER_DATA)
#endif
#define	LLC_IS_SNAPSAP(ll)	(((ll)[0] & LLC_SNAPSAP_MASK) == LLC_SNAPSAP_DATA)
#define	LLC_IS_ISOSAP(ll)	(((ll)[0] & LLC_ISOSAP_MASK) == LLC_ISOSAP_DATA)

#endif
