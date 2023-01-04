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
 * @(#)$RCSfile: screentab.h,v $ $Revision: 1.1.3.6 $ (DEC) $Date: 1993/01/05 18:27:55 $
 */

/*
 * screentab.h
 * Internal definitions for screen table functions
 *
 * 19 December 1988	Jeffrey Mogul/DECWRL
 *	Created.
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

/*
 * Maps from network number to appropriate subnet mask
 */
struct NetmaskData {
	struct in_addr	network;
	struct in_addr	mask;
};

/*
 * Address specifier
 */
struct AddrSpec {
	int	addrtype;
	union {
		struct in_addr	network;	/* INADDRANY == ANY */
		struct in_addr	subnet;
		struct in_addr	host;
	} aval;
};

/* values for addrtype */
#define	ASAT_ANY	0
#define	ASAT_NET	1
#define	ASAT_SUBNET	2
#define	ASAT_HOST	3

/*
 * Port value
 *	This is a composite value because it can either be
 *	an actual number, or a code representing a range.
 */
struct PortValue {
	short discrim;		/* see codes below */
	short value;		/* actual port number */
};

/*
 *	Some day, the ranges might be configured dynamically
 */
#define	PORTV_EXACT	0		/* value must match */
#define	PORTV_ANY	-1		/* Any port */
#define	PORTV_RESERVED	-2		/* between 0 and IPPORT_RESERVED */
#define	PORTV_XSERVER	-3		/* between XSERVERPORT_MIN and
						XSERVERPORT_MAX */
#define	XSERVERPORT_MIN	6000		/* shouldn't be here, but ... */
#define	XSERVERPORT_MAX	6100

/*
 * Note: We must use ICMP type = 0 for ICMPV_ANY, since that is
 * the "unspecified" state of a PortSpec code. However, this means
 * we have to use a special internal value for ECHOREPLY, which
 * (externally) has the value 0. We also need to special-case
 * this value in several places.
 */
#define ICMPV_ANY	0		/* Any ICMP type */
#define ICMPV_ECHOREPLY	-1		/* Hack for 0-valued ICMP type */
#define ICMPV_INFOTYPE	-2		/* Any ICMP Info type */

/*
 * Port specifier
 */
struct PortSpec {
	int	proto;				/* 0 == ANY */
	struct {
		struct PortValue port;
		short	code;			/* 0 == ANY */
	} pval;
};

/*
 * Complete object specifier
 */
struct ObjectSpec {
	struct AddrSpec aspec;
	struct PortSpec pspec;
	int		flags;	/* defs below */
};

/* values for ObjectSpec.flag */
#define	OSF_DEFAULT		0x0	/* no flags set */
#define	OSF_NOTADDR		0x1	/* looking for non-matching addr */
#define	OSF_NOTPROTO		0x2	/* looking for non-matching proto */
#define	OSF_NOTPORT		0x4	/* looking for non-matching port */
		/* OSF_NOTPROTO|OSF_NOTPORT makes no sense! */

/* Complete action specifier */
struct ActionSpec {
	struct ObjectSpec from;
	struct ObjectSpec to;
	int		  action;
};

/* values for action (bit fields) */
#define	ASACTION_REJECT		0x0
#define	ASACTION_ACCEPT		0x1
#define	ASACTION_NOTIFY		0x2
#define	ASACTION_NONOTIFY	0x0
#define	ASACTION_LOG		0x4
#define	ASACTION_NOLOG		0x0

/* AddrSpec marked with flags */
struct AddrSpecX {
	struct AddrSpec aspec;
	int		flags;
};

/* PortSpec marked with flags */
struct PortSpecX {
	struct PortSpec pspec;
	int		flags;
};


/*
 * Unpacked abstraction of an IP datagram header stack
 */

struct unpacked_hdr {
	struct in_addr	addr;
	short		port;	/* could be ICMP type code if "src"*/
};

struct unpacked_hdrs {
	struct unpacked_hdr src;
	struct unpacked_hdr dst;
	int		proto;
};

struct annotation {
	struct in_addr	net;
	struct in_addr	subnet;
};

/*
 * Annotated unpacked header
 */
struct annotated_hdrs {
	struct unpacked_hdrs	hdrs;
	/* The following are INADDR_ANY (0) if not initialized yet */
	struct annotation	srcnote;
	struct annotation	dstnote;
};

struct in_addr_byte {
	union {
		struct { u_char s_b1, s_b2, s_b3, s_b4; } S_un_b;
		u_int s_addr; 
	} addr_un;
};

