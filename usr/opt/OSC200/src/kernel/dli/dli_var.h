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
 *@(#)$RCSfile: dli_var.h,v $ $Revision: 4.4.7.4 $ (DEC) $Date: 1993/07/28 17:36:36 $
 *
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
 *   EDIT HISTORY
 *
 *
 * 2.00 18-Apr-1986
 *      DECnet-ULTRIX, Created.
 *	DECnet-Ultrix	V2.0
 *
 * Added sysid and point-to-point support
 *
 * 2.01 18-Mar-1988
 *      DECnet-ULTRIX   V2.4
 *		- Allowed use of reserved bit in individual and group SAPs
 *
 *	21-Sep-1989	Matt Thomas
 *		- Added more definitions in preparition for DECnet/OSI.
 */

/* Data Structures and Literals applicable to DLI */

#ifndef _DLI_DLI_VAR_H
#define	_DLI_DLI_VAR_H
/*
 * Socket queue literals
 */
#define DLI_SENDSPACE 5000
#define DLI_RECVSPACE 10000


/*
 * DLI socket literals
 */
#define DLPROTO_DLI	0		/* user interface protocol number */

/*
 * DLI i/o control flag literals
 */
#define DLI_EXCLUSIVE	001		/* exclusive use of protocol */
#define DLI_NORMAL	002		/* prot/mcast filtering */
#define DLI_DEFAULT	004		/* unmatched packets go here */

/*
 * DLI literals corresponding to the following address structures
 * and/or frame types.
 */
#define	DLI_ETHERNET	0		/* sockaddr_edl format */
#define	DLI_POINTOPOINT	1		/* sockaddr_pdl format */
#define	DLI_802		2		/* sockaddr_802 format */
#define	DLI_LOOP	3		/* loop module format */
#define	DLI_HDLC	4		/* hdlc frame format */

#define DLI_EADDRSIZE	6		/* Ethernet address size */
#define DLI_DEVSIZE	16		/* device name size */

/*
*  802 support
*/

#define	ENOTFND		-1
#define	USER		(u_char)0	/* user supplied service */
#define	TYPE1		(u_char)1	/* class 1 service */
#define	OSI802		0X5DC		/* max value of 802 len field */
#define	VSAP		(u_char)0x01	/* mask for indiv/group sap bit */
#define SNAP_SAP	(u_char)0xAA
#define	NULL_SAP	(u_char)0x00
#define	GLOBAL_SAP	(u_char)0xFF
#define	XID		(u_char)0xAF	/* xid command w/out poll bit set */
#define	TEST		(u_char)0xE3	/* test command w/out poll bit set */
#define	XID_PCMD	(u_char)0xBF	/* xid command w/poll bit set */
#define	XID_NPCMD	(u_char)0xAF	/* xid command w/out poll bit set */
#define	XID_PRSP	(u_char)0xBF	/* xid response w/poll bit set */
#define	XID_NPRSP	(u_char)0xAF	/* xid response w/out poll bit set */
#define	TEST_PCMD	(u_char)0xF3	/* test command w/poll bit set */
#define	TEST_NPCMD	(u_char)0xE3	/* test command w/out poll bit set */
#define	TEST_PRSP	(u_char)0xF3	/* test response w/poll bit set */
#define	TEST_NPRSP	(u_char)0xE3	/* test response w/out poll bit set */
#define	UI_NPCMD	(u_char)0x03	/* unnumbered info cmd w/out poll bit */
#define	MAX802DATANP	3		/* maximum user data without proto id */
#define	MAX802DATAP	8		/* maximum user data with proto id field */
#ifndef NISAPS
#define	NISAPS		128		/* also in <netinet/if_ether.h> */
#endif
#define	NGSAPS		NISAPS

struct osi_802hdr {
    u_char dst[DLI_EADDRSIZE];	/* where packet is going */
    u_char src[DLI_EADDRSIZE];	/* who packet is for (usually this node)*/
    u_short len;		/* length of user data plus 802.3 header */
    u_char dsap;		/* what dst sap the packet is going to */
    u_char ssap;		/* source individual sap */
    union {
	u_short U_fmt;		/* unnumbered format */
	u_short I_S_fmt;	/* information/supervisory format */
    } ctl;
    u_char osi_pi[5];		/* 802.3 protocol id */
    u_char priority;		/* 802.5/FDDI FC priority */
};

/*
 * address structures used by DLI
 */
struct dli_devid {
    u_char dli_devname[DLI_DEVSIZE+1];	/* device name */
    u_short dli_devnumber;		/* device number */
};

struct sockaddr_edl {
    u_char dli_ioctlflg;		/* i/o control flags */
    u_char dli_options;			/* Ethernet options */
    u_short dli_protype;		/* Ethernet protocol type */
    u_char dli_target[DLI_EADDRSIZE];	/* Ethernet address of target node */
    u_char dli_dest[DLI_EADDRSIZE];	/* Ethernet destination address */
};

struct sockaddr_802 {			/* 802.3 sockaddr struct */
    u_char ioctl;			/* i/o control flags */
    u_char svc;				/* service class for this portal */
    struct osi_802hdr eh_802;		/* OSI 802 header format */
};

struct sockaddr_dl {
#if defined(_SOCKADDR_LEN) || (defined(__osf__) && defined(_KERNEL))
    u_char dli_len;			/* length of sockaddr */
    u_char dli_family;			/* address family (AF_DLI) */
#else
    u_short dli_family;                 /* address family (AF_DLI) */
#endif
    struct dli_devid dli_device;	/* id of comm device to use */
    u_char dli_substructype;		/* id to interpret following structure */
    union {
	struct sockaddr_edl dli_eaddr;	/* Ethernet */
	struct sockaddr_802 dli_802addr; /* OSI 802 support */
	caddr_t	dli_aligner1;		/* this needs to have longword alignment */
    } choose_addr;
};

#define DLI_MAXPKT	1500	/* max number of user bytes per pkt */
#define DLI_MINPKT	46	/* min number of user bytes per pkt */
#define	DLI_ETHERPAD	0x01	/* Protocol is padded */

/*
 * DLI socket option literals
 */
#define DLI_STATE	0	/* state option requested */
#define DLI_MULTICAST	1	/* get/set multicast address(es) */
#define DLI_INTERNALOOP	2	/* change device to/from internal loopback */

#define	DLI_ENAGSAP		4	/* enable an 802.3 group sap */
#define	DLI_DISGSAP		6	/* disable an 802.3 group sap */
#define	DLI_GETGSAP		8	/* get 802.3 enabled group saps */
#define	DLI_SET802CTL		10	/* change control field format, 802 */

/* the following socket option is only there for DECnet-OSI */
#define DLI_LOCALENTITYNAME	11	/* set/get client/port name */

/*
 * The following ioctl translate a local entity name into a device id
 * suitable for use in a struct sockaddr_dl.
 */
#define SIOCXLATENAME2IF	_IOWR('s', 71, struct dli_xlatename2if)
struct dli_xlatename2if {
    caddr_t dlif_name;			/* ptr to local entity name */
    int dlif_namelen;			/* length of local entity name */
    struct dli_devid dlif_devid;	/* device id of local entity name */
};

/*
 * This ioctl is called from MOP to update the MOP Functions parameter
 * in the kernel so that the sysid generated reflects the current status of MOP
 * It is a one-byte, bitmapped field.
 */
#define SIOCMOPFUNCTIONS        _IOWR('i', 72, struct dli_mopfunc)
struct dli_mopfunc {
        char dev_name[IFNAMSIZ];                /* Physical device name */
        u_short function;                       /* Functions available */
        };

#define DLIFUNC_Loop            0x01            /* Loop Server (required) */
#define DLIFUNC_Dump            0x02            /* Dump Client */
#define DLIFUNC_PriLoader       0x04            /* Primary Loader Client */
#define DLIFUNC_MulLoader       0x08            /* Multi-block Loader Client */
#define DLIFUNC_Boot            0x10            /* Boot */
#define DLIFUNC_ConsCarrier     0x20            /* Console Carrier Server */
#define DLIFUNC_Counters        0x40            /* Data Link Counters (req'd)*/
#define DLIFUNC_CCReserved      0x80            /* Console Carrier Reserved */


/*
 * DLI state literals
 */
#define DLS_OFF		0	/* turn comm device off */
#define DLS_ON		1	/* turn comm device on */
#define DLS_SLEEP	2	/* process asleep in bind */

/*
 * DLI internal loopback literals
 */
#define DLP_IOFF	0	/* turn internal loopback on */
#define DLP_ION		1	/* turn internal loopback off */


/*
 * MOP literals used by DLI
 */
#define NO_ADDR		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#ifndef ETHERTYPE_LBACK
#define ETHERTYPE_LBACK 0x9000          /* MOP loopback protocol type */
#endif
#define PROTOID_LBACK   {0x08, 0x00, 0x2b, 0x90, 0x00}
#define LBACKMCAST      {0xcf, 0x00, 0x00, 0x00, 0x00, 0x00}
#ifndef ETHERTYPE_MOPRC
#define ETHERTYPE_MOPRC 0x6002          /* MOP CCR protocol type */
#endif
#define PROTOID_MOPRC   {0x08, 0x00, 0x2b, 0x60, 0x02}
#define SYSIDMCAST      {0xab, 0x00, 0x00, 0x02, 0x00, 0x00}

/* MOP message type codes */
#define DLI_LBACK_FWD   0x2             /* MOP loopback forward data function */
#define DLI_LBACK_LOOP  24              /* MOP point to point loop code */
#define DLI_LBACK_LOOPED 26             /* MOP point to point looped code */
#define DLI_REQCTRS     9               /* MOP request counters code */
#define DLI_SYSID       7               /* MOP SYSID response code */
#define DLI_REQSYSID    5               /* MOP Request SYSID code */
#define DLI_CTRS        11              /* MOP counters code (CSMA-CD only) */
#define DLI_CTRS2       21              /* MOP counters code (non CSMA-CD) */

/* network controller type codes used in MOP SYSID messages */
#define UNA             1       /* DEUNA (Unibus) */
#define QNA             5       /* DEQNA (Q-Bus) */
#define LUA             11      /* DELUA (Unibus) */
#define BNA             23      /* DEBNA (BI) */
#define LQA             37      /* DELQA (Q-Bus) */
#define SVA             39      /* DESVA (MV2000) */
#define MF2             61      /* DECsystem 5400 (integral) */
#define BNI             65      /* DEBNI (BI) */
#define MNA             66      /* DEMNA (XMI) */
#define PMX             67      /* DECstation 3100 (integral) */
#define QTA             75      /* DEQTA (Q-Bus) */

/* MOP SYSID parameter tags */
#define MAINTV          1
#define VER3            3
#define VER4            4
#define ECO             0
#define USER_ECO        0
#define FUNCTIONS       2
#define FNC_LOOP        0x1
#define FNC_CTRS        0x40
#define HADDR           7
#define NODEID          9
#define SYSTIME         10
#define NODENAME1       11
#define NODENAME2       12
#define STATIONID       13
#define COMDEV          100
#define DATALINK        400
#define MCASTADDR       1

/* data link types used in MOP sysid messages */
#define DLI_DLTRN       6
#define DLI_DLFDDI      5
#define DLI_DLHDLC      4
#define DLI_DLLAPB      3
#define DLI_DLDDCMP     2
#define DLI_DLCSMACD    1

#ifdef KERNEL /* the following should not be used by applications */
#include <dli/dli_kernel.h>
#endif
#endif	/* _DLI_DLI_VAR_H */
