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
 * @(#)$RCSfile: rrip.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/07/16 13:06:03 $
 */
#ifndef	_CDFS_RRIP_INCLUDE
#define	_CDFS_RRIP_INCLUDE	1
/*
 * Rock Ridge Interchange Protocol, version 1 (rev 1.09) (August 1991)
 */

#include <cdfs/susp.h>			/* RRIP builds on
					   System Use Sharing Protocol */

#define RRIP_SIG_PX		"PX"
#define RRIP_SIG_PN		"PN"
#define RRIP_SIG_SL		"SL"
#define RRIP_SIG_NM		"NM"
#define RRIP_SIG_CL		"CL"
#define RRIP_SIG_PL		"PL"
#define RRIP_SIG_RE		"RE"
#define RRIP_SIG_TF		"TF"
#define RRIP_SIG_RR		"RR"

#define RRIP_SIG_PX_0		'P'
#define RRIP_SIG_PX_1		'X'
#define RRIP_SIG_PN_0		'P'
#define RRIP_SIG_PN_1		'N'
#define RRIP_SIG_SL_0		'S'
#define RRIP_SIG_SL_1		'L'
#define RRIP_SIG_NM_0		'N'
#define RRIP_SIG_NM_1		'M'
#define RRIP_SIG_CL_0		'C'
#define RRIP_SIG_CL_1		'L'
#define RRIP_SIG_PL_0		'P'
#define RRIP_SIG_PL_1		'L'
#define RRIP_SIG_RE_0		'R'
#define RRIP_SIG_RE_1		'E'
#define RRIP_SIG_TF_0		'T'
#define RRIP_SIG_TF_1		'F'
#define RRIP_SIG_RR_0		'R'
#define RRIP_SIG_RR_1		'R'

/* take signature name and turn it into a short */
#define RRIP_SHORTIFY_SIG(sig) ((RRIP_SIG_ ## sig ## _0 << 8)|(RRIP_SIG_ ## sig ## _1))

/*
 * It would be nice to have the lsb/msb portions declared as "int"s, but
 * that won't work on machines with multi-byte cell alignment restrictions.
 */

/* PX: POSIX file attributes */
struct rrip_px {
    /* hdr.suf_sig_word == "PX"
       hdr.suf_length = 36
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char px_mode_lsb[4];		/* st_mode field */
    unsigned char px_mode_msb[4];
    unsigned char px_nlink_lsb[4];		/* st_nlink field */
    unsigned char px_nlink_msb[4];
    unsigned char px_uid_lsb[4];		/* st_uid field */
    unsigned char px_uid_msb[4];
    unsigned char px_gid_lsb[4];		/* st_gid field */
    unsigned char px_gid_msb[4];
};

/* PN: POSIX device modes.  Per 4.2, must be present on every dir entry */
struct rrip_pn {
    /* hdr.suf_sig_word == "PN"
       hdr.suf_length = 20
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char pn_dev_t_high_lsb[4];	/* st_rdev field, high 32 bits */
    unsigned char pn_dev_t_high_msb[4];
    unsigned char pn_dev_t_low_lsb[4];	/* st_rdev field, low 32 bits */
    unsigned char pn_dev_t_low_msb[4];
};

/* SL: symlink components */
struct rrip_sl {
    /* hdr.suf_sig_word == "SL"
       hdr.suf_length = 5 + length of component field
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char sl_flags;
#define RRIP_SL_CONTINUE	0x01
    unsigned char sl_components[1];	/* probably bigger than 1 */
};

struct rrip_sl_component {
    unsigned char slc_flags;
#define	RRIP_SLC_CONTINUE	0x01
#define	RRIP_SLC_CURRENT	0x02
#define	RRIP_SLC_PARENT		0x04
#define	RRIP_SLC_ROOT		0x08
#define	RRIP_SLC_VOLROOT	0x10
#define	RRIP_SLC_HOST		0x20
    unsigned char slc_len;		/* length of component ONLY */
    unsigned char slc_component[1];	/* maybe zero, maybe more than 1 */
};

/* NM: alternate name for this file */
struct rrip_nm {
    /* hdr.suf_sig_word == "NM"
       hdr.suf_length = 5 + length of portion of alternate name field
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char nm_flags;
#define	RRIP_NM_CONTINUE	0x01
#define	RRIP_NM_CURRENT		0x02
#define	RRIP_NM_PARENT		0x04
#define	RRIP_NM_RSVD1		0x08
#define	RRIP_NM_RSVD2		0x10
#define	RRIP_NM_HOST		0x20
    unsigned char nm_component[1];	/* maybe zero, maybe more than 1 */
};

/* CL (child link): point to relocated directory to create trees deeper
   than ISO-9660 permits */
struct rrip_cl {
    /* hdr.suf_sig_word == "CL"
       hdr.suf_length = 12
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char cl_child_lbn_lsb[4];	/* LBN of directory this points to */
    unsigned char cl_child_lbn_msb[4];
};

/* PL (parent link): point from relocated directory to logical parent */
struct rrip_pl {
    /* hdr.suf_sig_word == "PL"
       hdr.suf_length = 12
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char pl_parent_lbn_lsb[4];	/* LBN of directory this points to */
    unsigned char pl_parent_lbn_msb[4];
};

/* RE: mark directory as a "relocated" directory which does not logically
   belong at its current place in the hierarchy */
struct rrip_re {
    /* hdr.suf_sig_word == "RE"
       hdr.suf_length = 4
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    /* nothing else */
};

/* TF: provide POSIX time stamps */
struct rrip_tf {
    /* hdr.suf_sig_word == "TF"
       hdr.suf_length = 5 + length of timestamps
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char tf_flags;
#define	RRIP_TF_CREATION	0x01
#define	RRIP_TF_MODIFY		0x02	/* st_mtime */
#define	RRIP_TF_ACCESS		0x04	/* st_atime */
#define	RRIP_TF_ATTRIBUTES	0x08	/* st_ctime */
#define	RRIP_TF_BACKUP		0x10
#define	RRIP_TF_EXPIRATION	0x20
#define	RRIP_TF_EFFECTIVE	0x40
#define	RRIP_TF_LONG_FORM	0x80
    unsigned char tf_timestamps[1];	/* probably more than 1 byte */
};
/* if (tf_flags & RRIP_TF_LONG_FORM) then all time stamps are
   per 8.4.26.1 (YYYYMMDDHHmmssddO, dd=hundredth of secs, O = GMT offset
   in 15 min intervals)
   if !(tf_flags & RRIP_TF_LONG_FORM), time stamps are per 9.1.5
   (YMDHMSO, each byte unsigned, Y=since 1900, O= GMT offset in 15 min
   intervals)
   */
/* this macro computes the offset inside tf_timestamps[] for the
   num'th timestamp */
#define	RRIP_TF_OFFSET(tfp,num)	\
    (((tfp->tf_flags & RRIP_TF_LONG_FORM) ? 17 : 7)*(num-1))
#define	RRIP_TF_TSPTR(tfp,num) (signed char *)(&(tfp)->tf_timestamps[RRIP_TF_OFFSET(tfp,num)])

/* RR: record which SUFs are present for this directory entry */
struct rrip_rr {
    /* hdr.suf_sig_word == "RR"
       hdr.suf_length = 5
       hdr.suf_version = 1 */
    struct cd_suf_header hdr;
    unsigned char rr_present;
#define	RRIP_RR_PX_PRESENT	0x01
#define	RRIP_RR_PN_PRESENT	0x02
#define	RRIP_RR_SL_PRESENT	0x04
#define	RRIP_RR_NM_PRESENT	0x08
#define	RRIP_RR_CL_PRESENT	0x10
#define	RRIP_RR_PL_PRESENT	0x20
#define	RRIP_RR_RE_PRESENT	0x40
#define	RRIP_RR_TF_PRESENT	0x80
#define	RRIP_RR_LARGEST RRIP_RR_TF_PRESENT
};

#define	RRIP_ER_VERSION	1
#define	RRIP_ER_IDENT	"RRIP_1991A"
#define	RRIP_ER_IDENT_LEN	10
#define	RRIP_ER_DESC	"THE ROCK RIDGE INTERCHANGE PROTOCOL PROVIDES SUPPORT FOR POSIX FILE SYSTEM SEMANTICS."
#define	RRIP_ER_DESC_LEN	84
#define	RRIP_ER_SRC	"PLEASE CONTACT DISC PUBLISHER FOR SPECIFICATION SOURCE.  SEE PUBLISHER IDENTIFIER IN PRIMARY VOLUME DESCRIPTOR FOR CONTACT INFORMATION."
#define	RRIP_ER_SRC_LEN	135

#define	CD_MAXDMAP	50		/* XXX dynamic, config'able? */

#define CD_SETDMAP	0		/* Set device file mapping */
#define	CD_UNSETDMAP	1		/* Unset device file mapping */

struct rrip_map_arg {
    int major;
    int minor;
    char *path;
};

struct rrip_map_idx_arg {
    int major;
    int minor;
    char *path;
    int index;
    int pathlen;
};
struct rrip_suf_arg {
    int fsec;
    unsigned int sig_index;
    char *buf;
    char *path;
    int buflen;
    char sig[2];
    char sigok;
};

#define CDIOCSETDMAP	_IOW('C', 0, struct rrip_map_arg)
#define CDIOCUNSETDMAP	_IOWR('C', 1, struct rrip_map_arg)
#define CDIOCGETDMAP	_IOWR('C', 2, struct rrip_map_arg)
#define CDIOCGETDMAPIDX	_IOWR('C', 3, struct rrip_map_idx_arg)
#define CDIOCGETSUF	_IOWR('C', 4, struct rrip_suf_arg)

#ifndef _KERNEL
#include <sys/types.h>			/* for time_t, below */

extern int cd_suf(char *path, int fsec, char signature[2], int index,
		  char *buf, int buflen); /* RRIP 5.4.1 */
extern int cd_setdevmap(char *path, int cmd, int *new_major,
			int *new_minor); /*  RRIP 5.4.2 */
extern int cd_getdevmap(char *path, int pathlen, int index, int *new_major,
			int *new_minor); /*  RRIP 5.4.3 */
time_t rrip_convert_tf_ts(int, signed char *); /* extra */

#endif

#endif _CDFS_RRIP_INCLUDE
