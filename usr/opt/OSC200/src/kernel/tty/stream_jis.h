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
 * @(#)$RCSfile: stream_jis.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:07:12 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#define AJ_SZ		4	/* 3 would probably suffice */
#define JA_SZ		8	/* room for seq., plus NULL */

/*
 * jis conversion module specific structures/definitions
 */
struct jis_s {
	int 		flags;		/* conversion flags */
	int 		flags_save;	/* for EUC_MSAVE/EUC_MREST */
	int 		ajstate;	/* kanji state ajec->jis queue */
	int 		jastate;	/* kanji state jis->ajec queue */
	int 		c1state;	/* C1 conversion state */
	int 		kanastate;	/* hankaku kana type state */
	unsigned char 	ajc[AJ_SZ];	/* chars left over ajec->jis queue */
	unsigned char 	jac[JA_SZ];	/* chars left over jis->ajec queue */
	unsigned char 	ki[JA_SZ];	/* kanji 0208 in sequence */
	unsigned char 	k2i[JA_SZ];	/* kanji 0212 in sequence */
	int		k2count;	/* state for kanji 0212 (J==>>A) */
	unsigned char 	ko[JA_SZ];	/* kanji out sequence */
	unsigned char 	sti_c[JA_SZ];	/* TIOCSTI character in progress */
	int 		sti_state;	/* TIOCSTI state */
	mblk_t 		*rspare;	/* recycling on read side */
	mblk_t 		*wspare;	/* recycling on write side */
	int 		rbid;		/* bufcall id on read side */
	int 		wbid;		/* bufcall id on write side */
	int 		rtid;		/* timeout id on read side */
	int 		wtid;		/* timeout id on write side */
};

/*
 * Kanji in and out sequences must be (JA_SZ - 1) bytes or less.
 */
#define UC_DEF_KI	"\033$B"
#define UC_DEF_K2I	"\033$+D"
#define UC_DEF_KO	"\033(B"

#define LC_DEF_KI	"\033$B"
#define LC_DEF_K2I	"\033$+D"
#define LC_DEF_KO	"\033(B"

/*
 * Worst case memory needs of a conversion.  The meaning of these constants
 * is, for example, that when converting a buffer of AJEC data to JIS, you
 * will need a maximum of:
 *
 *		(N * AJ_COEFF) + AJ_ADDEND
 *
 * bytes in the target buffer, where N is the size, in bytes, of the source
 * buffer.  The JA_* constants take care of conversion in the other direction.
 */
#define AJ_COEFF	(JA_SZ + 3)
#define AJ_ADDEND	0

#define JA_COEFF	2
#define JA_ADDEND	0

/*
 * flag bits
 */
#define KS_ICONV	0x0001	/* read conversion on */
#define KS_OCONV	0x0002	/* write conversion on */
#define KS_IEXTEN	0x0004	/* IEXTEN flag is on in line discipline */

/*
 * Indication of current character set
 */
#define KS_IN		0x01	/* kanji 0208 in state */
#define KS_2IN		0x02	/* kanji 0212 in state */
#define KS_KANA		0x04	/* kana in state */

/*
 * Streams flow control
 */
#define LC_JIS_LOWAT	128
#define LC_JIS_HIWAT	2048

#define UC_JIS_LOWAT	128
#define UC_JIS_HIWAT	2048

/*
 * Special characters
 */
#define SS2		0x8e
#define SS3		0x8f
#define SI		0x0f
#define SO		0x0e

#define INRANGE(x1, x2, vv) (((unsigned)((vv) - (x1))) <= ((unsigned)((x2) - (x1))))

/*
 * lower converter module for jis
 */
#define LC_JIS_MODULE_ID    7011
#define LC_JIS_MODULE_NAME  "lc_jis"

/*
 * upper converter module for jis
 */
#define UC_JIS_MODULE_ID    7001
#define UC_JIS_MODULE_NAME  "uc_jis"

unsigned char *
conv_jis2ajec(mblk_t *, unsigned char *, struct jis_s *, unsigned char *, int*);

unsigned char *
conv_ajec2jis(mblk_t *, unsigned char *, struct jis_s *, unsigned char *, int*);

PRIVATE int
jis_proc_sti(struct jis_s *, mblk_t *, queue_t *);

int
jis_readdata(struct jis_s *, queue_t *, mblk_t *, unsigned char *(*)(), int, int);

PRIVATE int
jis_send_sti(queue_t *q, unsigned char *, int, mblk_t *);

int
jis_writedata(struct jis_s *, queue_t *, mblk_t *, unsigned char *(*)(), int, int);

int
lc_jis_close(queue_t *, int, cred_t *);

int
jis_configure(sysconfig_op_t, char *, size_t, char *,size_t);

PRIVATE int
lc_jis_ioctl(struct jis_s *, queue_t *, mblk_t *);

int
lc_jis_open(queue_t *, dev_t *, int, int, cred_t *);

int
lc_jis_rput(queue_t *, mblk_t *);

int
lc_jis_rsrv(queue_t *);

int
lc_jis_wput(queue_t *, mblk_t *);

int
lc_jis_wsrv(queue_t *);

int
uc_jis_close(queue_t *, int, cred_t *);

PRIVATE int
uc_jis_ioctl(struct jis_s *, queue_t *, mblk_t *);

int
uc_jis_open(queue_t *, dev_t *, int, int, cred_t *);

int
uc_jis_rput(queue_t *, mblk_t *);

int
uc_jis_rsrv(queue_t *);

int
uc_jis_wput(queue_t *, mblk_t *);

int
uc_jis_wsrv(queue_t *);
