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
 * @(#)$RCSfile: stream_sjis.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:07:43 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/*
 * shift jis conversion module specific structures/definitions
 */
struct sjis_s {
	int flags;		/* conversion flags */
	int flags_save;		/* for EUC_MSAVE/EUC_MREST */
	int c1state;		/* C1 conversion state */
	int rbid;		/* bufcall id on read side */
	int wbid;		/* bufcall id on write side */
	int rtid;		/* timeout id on read side */
	int wtid;		/* timeout id on write side */
	mblk_t *rspare;		/* spare mblk on read side */
	mblk_t *wspare;		/* spare mblk on write side */
	unsigned char sac;	/* char left over sjis->ajec queue */
	unsigned char asc[2];	/* chars left over ajec->sjis queue */
	unsigned char sti_c;	/* TIOCSTI character */
};

#define KS_ICONV	0x0001	/* read conversion on */
#define KS_OCONV	0x0002	/* write conversion on */
#define KS_IEXTEN	0x0004	/* IEXTEN flag is on in line discipline */

#define LC_SJIS_LOWAT	128
#define LC_SJIS_HIWAT	2048

#define UC_SJIS_LOWAT	128
#define UC_SJIS_HIWAT	2048

#define SS2		0x8e
#define SS3		0x8f

#define INRANGE(x1, x2, vv) (((unsigned)((vv) - (x1))) <= ((unsigned)((x2) - (x1))))

/*
 * lower converter module for shift jis
 */
#define LC_SJIS_MODULE_ID    7012
#define LC_SJIS_MODULE_NAME  "lc_sjis"

/*
 * upper converter module for shift jis
 */
#define UC_SJIS_MODULE_ID    7002
#define UC_SJIS_MODULE_NAME  "uc_sjis"

void
conv_sjis2ajec(mblk_t *, mblk_t *, struct sjis_s *);

void
conv_ajec2sjis(mblk_t *, mblk_t *, struct sjis_s *);

int
lc_sjis_close(queue_t *, int, cred_t *);

int
sjis_configure(sysconfig_op_t, char *, size_t, char *, size_t);

PRIVATE int
lc_sjis_ioctl(struct sjis_s *, queue_t *, mblk_t *);

int
lc_sjis_open(queue_t *, dev_t *, int, int, cred_t *);

int
lc_sjis_rput(queue_t *, mblk_t *);

int
lc_sjis_rsrv(queue_t *);

int
lc_sjis_wput(queue_t *, mblk_t *);

int
lc_sjis_wsrv(queue_t *);

int
sjis_readdata(struct sjis_s *, queue_t *, mblk_t *, void (*)(), int);

int
sjis_writedata(struct sjis_s *, queue_t *, mblk_t *, void (*)(), int);

int
uc_sjis_close(queue_t *, int, cred_t *);

int
uc_sjis_open(queue_t *, dev_t *, int, int, cred_t *);

PRIVATE int
sjis_proc_sti(struct sjis_s *, unsigned char, queue_t *);

int
uc_sjis_rput(queue_t *, mblk_t *);

int
uc_sjis_rsrv(queue_t *);

PRIVATE int
sjis_send_sti(queue_t *, unsigned char *, int);

int
uc_sjis_wput(queue_t *, mblk_t *);

int
uc_sjis_wsrv(queue_t *);
