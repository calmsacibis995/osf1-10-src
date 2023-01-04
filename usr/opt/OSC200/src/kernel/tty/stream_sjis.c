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
static char *rcsid = "@(#)$RCSfile: stream_sjis.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:07:25 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/systm.h>

#include <sys/termios.h>
#include <sys/sysconfig.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/eucioctl.h>
#include <sys/sjisioctl.h>
#include <tty/stream_tty.h>
#include <tty/stream_sjis.h>

PRIVATE struct module_info uc_sjis_modinfo = {
        UC_SJIS_MODULE_ID, UC_SJIS_MODULE_NAME, 0, INFPSZ, UC_SJIS_HIWAT,
        UC_SJIS_LOWAT
};

PRIVATE struct qinit uc_sjis_rinit = {
        uc_sjis_rput, uc_sjis_rsrv,
        uc_sjis_open, uc_sjis_close, 0, &uc_sjis_modinfo, 0
};

PRIVATE struct qinit uc_sjis_winit = {
        uc_sjis_wput, uc_sjis_wsrv, 0, 0, 0, &uc_sjis_modinfo, 0
};

struct streamtab uc_sjis_info = { &uc_sjis_rinit, &uc_sjis_winit };

PRIVATE struct module_info lc_sjis_modinfo = {
        LC_SJIS_MODULE_ID, LC_SJIS_MODULE_NAME, 0, INFPSZ, LC_SJIS_HIWAT,
        LC_SJIS_LOWAT
};

PRIVATE struct qinit lc_sjis_rinit = {
        lc_sjis_rput, lc_sjis_rsrv,
        lc_sjis_open, lc_sjis_close, 0, &lc_sjis_modinfo, 0
};

PRIVATE struct qinit lc_sjis_winit = {
        lc_sjis_wput, lc_sjis_wsrv, 0, 0, 0, &lc_sjis_modinfo, 0
};

struct streamtab lc_sjis_info = { &lc_sjis_rinit, &lc_sjis_winit };

/*
 * Conversion routines for AJEC--->>>SJIS and vice-versa.
 */

void
conv_ajec2sjis(
	register mblk_t *from,
	register mblk_t *to,
	register struct sjis_s *jp
	)
{
	unsigned char uc, uc1, uc2, n1, n2;

	while (from) {
		while (from->b_rptr < from->b_wptr) {
			uc = *from->b_rptr++;
			uc1 = jp->asc[0];
			uc2 = jp->asc[1];
			if (!uc1) {
				if (INRANGE(0x00, 0x7f, uc)) {
					/*
					 * ascii and C0
					 */
					*to->b_wptr++ = uc;
				} else if (INRANGE(0x81, 0x8d, uc) ||
					 INRANGE(0x90, 0xa0, uc)) {
					/*
					 * C1
					 */
					if (jp->c1state == SJIS_C1_PASS) {
						/*
						 * pass it as is
						 */
						*to->b_wptr++ = uc;
					} else if (jp->c1state == SJIS_C1_C0) {
						/*
						 * convert to C0
						 */
						*to->b_wptr++ = 0x1b;
						*to->b_wptr++ = uc - 0x40;
					} else if (jp->c1state == SJIS_C1_THROW) {
						/*
						 * throw away
						 */
					} else {
						*to->b_wptr++ = uc & 0x7f;
					}
				} else if ((uc == 0xff) || (uc == 0x80)) {
					*to->b_wptr++ = uc;
				} else {
					/*
					 * could be first byte of kanji or
					 * hankaku-kana
					 */
					jp->asc[0] = uc;
				}
			} else if (uc1 == SS2) {
				/*
				 * hankaku-kana
				 */
				*to->b_wptr++ = uc;
				jp->asc[0] = 0;
			} else if (INRANGE(0xa1, 0xde, uc1)) {
				/*
				 * kanji
				 */
				if (uc1 & 1) {
					if (INRANGE(0xa1, 0xdf, uc)) {
						n1 = ((uc1 - 0xa1)>>1) + 0x81;
						n2 = uc - 0x61;
					} else if (INRANGE(0xe0, 0xfe, uc)) {
						n1 = ((uc1 - 0xa1)>>1) + 0x81;
						n2 = uc - 0x60;
					} else {
						n1 = uc1 & 0x7f;
						n2 = uc & 0x7f;
					}
				} else {
					if (INRANGE(0xa1, 0xfe, uc)) {
						n1 = ((uc1 - 0xa2)>>1) + 0x81;
						n2 = uc - 2;
					}
					else {
						n1 = uc1 & 0x7f;
						n2 = uc & 0x7f;
					}
				}
				*to->b_wptr++ = n1;
				*to->b_wptr++ = n2;
				jp->asc[0] = 0;
			} else if (INRANGE(0xdf, 0xfe, uc1)) {
				/*
				 * kanji
				 */
				if (uc1 & 1) {
					if (INRANGE(0xa1, 0xdf, uc)) {
						n1 = ((uc1 - 0xdf)>>1) + 0xe0;
						n2 = uc - 0x61;
					}
					else if (INRANGE(0xe0, 0xfe, uc)) {
						n1 = ((uc1 - 0xdf)>>1) + 0xe0;
						n2 = uc - 0x60;
					}
					else {
						n1 = uc1 & 0x7f;
						n2 = uc & 0x7f;
					}
				} else {
					if (INRANGE(0xa1, 0xfe, uc)) {
						n1 = ((uc1 - 0xe0)>>1) + 0xe0;
						n2 = uc - 2;
					}
					else {
						n1 = uc1 & 0x7f;
						n2 = uc & 0x7f;
					}
				}
				*to->b_wptr++ = n1;
				*to->b_wptr++ = n2;
				jp->asc[0] = 0;
			} else if (uc1 == SS3) {
				if (uc2) {
					if (INRANGE(0xa1, 0xfc, uc)) {
						/* X0212 Kanji -- doesn't
						 * exist in SJIS; these are
						 * the prescribed undefined
						 * values. (XXX)
						 */
						*to->b_wptr++ = 0xfc;
						*to->b_wptr++ = 0xfc;
					} else {
						*to->b_wptr++ = uc2 & 0x7f;
						*to->b_wptr++ = uc & 0x7f;
					}
					jp->asc[0] = jp->asc[1] = 0;
				} else {
					if (INRANGE(0xa1, 0xfc, uc)) {
						jp->asc[1] = uc;
					} else {
						/* error */
						*to->b_wptr++ = uc & 0x7f;
						jp->asc[0] = 0;
					}
				}
			} else {
				/*
				 * error
				 */
				*to->b_wptr++ = uc1 & 0x7f;
				*to->b_wptr++ = uc & 0x7f;
				jp->asc[0] = 0;
			}
		}
		from = from->b_cont;
	}
}

void
conv_sjis2ajec(
	register mblk_t *from,
	register mblk_t *to,
	register struct sjis_s *jp
	)
{
	unsigned char uc, uc1, n1, n2;

	while (from) {
		while (from->b_rptr < from->b_wptr) {
			uc = *from->b_rptr++;
			uc1 = jp->sac;
			if (!uc1) {
				if (INRANGE(0x00, 0x7f, uc)) {
					/*
					 * ascii and C0
					 */
					*to->b_wptr++ = uc;
				} else if (INRANGE(0xa1, 0xdf, uc)) {
					/*
					 * hankaku-kana
					 */
					*to->b_wptr++ = SS2;
					*to->b_wptr++ = uc;
				} else if (INRANGE(0x81, 0x9f, uc) ||
					    INRANGE(0xe0, 0xef, uc)) {
					/*
					 * first byte of kanji
					 */
					jp->sac = uc;
				} else {
					*to->b_wptr++ = uc & 0x7f;
				}
			} else if (INRANGE(0x81, 0x9f, uc1)) {
				/*
				 * kanji
				 */
				if (INRANGE(0x40, 0x7e, uc)) {
					n1 = ((uc1 - 0x81)<<1) + 0xa1;
					n2 = uc + 0x61;
				} else if (INRANGE(0x80, 0x9e, uc)) {
					n1 = ((uc1 - 0x81)<<1) + 0xa1;
					n2 = uc + 0x60;
				} else if (INRANGE(0x9f, 0xfc, uc)) {
					n1 = ((uc1 - 0x81)<<1) + 0xa2;
					n2 = uc + 2;
				} else {
					n1 = uc1 & 0x7f;
					n2 = uc & 0x7f;
				}
				*to->b_wptr++ = n1;
				*to->b_wptr++ = n2;
				jp->sac = 0;
			} else if (INRANGE(0xe0, 0xef, uc1)) {
				/*
				 * also kanji
				 */
				if (INRANGE(0x40, 0x7e, uc)) {
					n1 = ((uc1 - 0xe0)<<1) + 0xdf;
					n2 = uc + 0x61;
				} else if (INRANGE(0x80, 0x9e, uc)) {
					n1 = ((uc1 - 0xe0)<<1) + 0xdf;
					n2 = uc + 0x60;
				} else if (INRANGE(0x9f, 0xfc, uc)) {
					n1 = ((uc1 - 0xe0)<<1) + 0xe0;
					n2 = uc + 2;
				} else {
					n1 = uc1 & 0x7f;
					n2 = uc & 0x7f;
				}
				*to->b_wptr++ = n1;
				*to->b_wptr++ = n2;
				jp->sac = 0;
			} else {
				*to->b_wptr++ = uc1 & 0x7f;
				*to->b_wptr++ = uc & 0x7f;
				jp->sac = 0;
			}
		} /* inner while */
		from = from->b_cont;
	} /* outer while */
}

int
sjis_readdata(
	register struct sjis_s *jp,
	queue_t *q,
	mblk_t *mp,
	void (*conv_func)(),
	int mem_coeff
	)
{
	register mblk_t *next;
	register int maxmsg;
	int flag = (mp->b_flag & (~MSGNOTIFY));
	mblk_t *mp1, *mp2;
	int notify_count = 0;

	for (mp1 = mp; mp1; mp1 = mp1->b_cont)
		if (mp1->b_flag & MSGNOTIFY)
			notify_count += (mp1->b_wptr - mp1->b_rptr);

	if (notify_count) {
		mp2 = allocb(sizeof(int), BPRI_HI);
		if (!mp2) {
			if (jp->rbid)
				unbufcall(jp->rbid);
			if (!(jp->rbid = bufcall(sizeof(int), BPRI_HI, qenable, q))) {
				if (jp->rtid)
					untimeout(jp->rtid);
				jp->rtid = timeout(qenable, q, hz*2);
			}
			return(0);
		}
		for (mp1 = mp; mp1; mp1 = mp1->b_cont)
			mp1->b_flag &= ~MSGNOTIFY;
		*((int *)mp2->b_rptr) = notify_count;
		mp2->b_wptr = mp2->b_rptr + sizeof(int);
		mp2->b_datap->db_type = M_NOTIFY;
		putnext(q->q_other, mp2);
	}

	if (!canput(q->q_next))
		return(0);

	if (!(jp->flags & KS_ICONV) ||
	    !(jp->flags & KS_IEXTEN)) {
		putnext(q, mp);
		return(1);
	}

	/*
	 * pass blank messages up as they are
	 */
	if (msgdsize(mp) == 0) {
		putnext(q, mp);
		return(1);
	}
	maxmsg = msgdsize(mp) * mem_coeff; /* sjis -> ajec */
	if (jp->rspare && (jp->rspare->b_datap->db_size >= maxmsg)) {
		next = jp->rspare;
		jp->rspare = 0;
	} else
		next = allocb(maxmsg, BPRI_MED);

	if (!next) {
		if (jp->rbid)
			unbufcall(jp->rbid);
		if (!(jp->rbid = bufcall(maxmsg, BPRI_MED, qenable, q))) {
			if (jp->rtid)
				untimeout(jp->rtid);
			jp->rtid = timeout(qenable, q, hz*2);
		}
		return(0);
	}

	(*conv_func)(mp, next, jp);

	if (jp->rspare || (mp->b_datap->db_size < 2))
		freemsg(mp);
	else {
		mblk_t *mp1;

		if (mp1 = unlinkb(mp))
			freemsg(mp1);

		mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		jp->rspare = mp;
	}

	if (next->b_rptr < next->b_wptr) {
		next->b_flag |= flag;
		putnext(q, next);
	} else
		freemsg(next);
	return(1);
}

int
sjis_writedata(
	register struct sjis_s *jp,
	queue_t *q,
	mblk_t *mp,
	void (*conv_func)(),
	int mem_coeff
	)
{
	register mblk_t *next;
	register int maxmsg;

	if (!canput(q->q_next))
		return(0);

	if (!(jp->flags & KS_OCONV)) {
		putnext(q, mp);
		return(1);
	}

	maxmsg = msgdsize(mp) * mem_coeff;
	if (jp->wspare && (jp->wspare->b_datap->db_size >= maxmsg)) {
		next = jp->wspare;
		jp->wspare = 0;
	} else
		next = allocb(maxmsg, BPRI_MED);

	if (!next) {
		if (jp->wbid)
			unbufcall(jp->wbid);
		if (!(jp->wbid = bufcall(maxmsg, BPRI_MED, qenable, q))) {
			if (jp->wtid)
				untimeout(jp->wtid);
			jp->wtid = timeout(qenable, q, hz*2);
		}
		return(0);
	}

	(*conv_func)(mp, next, jp);

	if (jp->wspare || (mp->b_datap->db_size == 0))
		freemsg(mp);
	else {
		mblk_t *mp1;

		if (mp1 = unlinkb(mp))
			freemsg(mp1);

		mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		jp->wspare = mp;
	}

	if (next->b_rptr < next->b_wptr)
		putnext(q, next);
	else
		freemsg(next);
	return(1);
}



/*
 * sjis_configure() - sjis code converter entry point.
 */
int
sjis_configure(sysconfig_op_t  op,
               char *          indata,
               size_t          indatalen,
               char *          outdata,
               size_t          outdatalen)
{
        static struct streamadm uc_sa;
        static struct streamadm lc_sa;
        dev_t                   devno;
        int                     configured;
        struct subsystem_info   info;
        int                     size;
        int                     ret = 0;
        static config_attr_t attr[] = {
        { SUBSYS_NAME, STRTYPE, (caddr_t)"sjis" , CONSTANT, {0, NULL} },
        { "lc_sa_flags", INTTYPE, (caddr_t)&lc_sa.sa_flags, CONSTANT, {0} },
        { "uc_sa_flags", INTTYPE, (caddr_t)&uc_sa.sa_flags, CONSTANT, {0} },
        { "lc_sa_name", STRTYPE, (caddr_t)lc_sa.sa_name, CONSTANT, {0, NULL} },
        { "uc_sa_name", STRTYPE, (caddr_t)uc_sa.sa_name, CONSTANT, {0, NULL} }
        };
#define NUM_ATT sizeof(attr)/sizeof(config_attr_t)

        /*
         * Check to see if subsystem has already been configured.
         */
        strcpy(info.subsystem_name, "sjis");

        configured = ((!subsys_reg(SUBSYS_GET_INFO,&info)) && info.config_flag);

        /*
         * If the subsystem has been configured and the requested operation
         * is configure, SYSCONFIG_CONFIGURE, then return an error.
         * If the subsystem has not been configured and the requested operation
         * is anything besides configure, SYSCONFIG_CONFIGURE, then return an
         * error.
         */
        if ((configured && (op == SYSCONFIG_CONFIGURE)) ||
            (!configured && (op != SYSCONFIG_CONFIGURE)))
                return EALREADY;

        switch (op) {

        case SYSCONFIG_CONFIGURE:

            /*
             * Initialize it to NODEV.
             */

            devno = NODEV;
            lc_sa.sa_version    = OSF_STREAMS_10;
            lc_sa.sa_flags      = STR_IS_MODULE | STR_SYSV4_OPEN;
            lc_sa.sa_ttys       = 0;
            lc_sa.sa_sync_level = SQLVL_QUEUEPAIR;
            lc_sa.sa_sync_info  = 0;
            strcpy(lc_sa.sa_name, LC_SJIS_MODULE_NAME);

            if ((devno = strmod_add(devno, &lc_sjis_info, &lc_sa)) == NODEV) {
              return ENODEV;
            }

            uc_sa.sa_version    = OSF_STREAMS_10;
            uc_sa.sa_flags      = STR_IS_MODULE | STR_SYSV4_OPEN;
            uc_sa.sa_ttys       = (caddr_t) 0;
            uc_sa.sa_sync_level = SQLVL_QUEUEPAIR;
            uc_sa.sa_sync_info  = (caddr_t) 0;
            strcpy(uc_sa.sa_name, UC_SJIS_MODULE_NAME);

            if ((devno = strmod_add(devno, &uc_sjis_info, &uc_sa)) == NODEV) {
              return ENODEV;
            }

            if (outdata && outdatalen>=0)
                bcopy(indata,outdata,outdatalen);
            break;

        case SYSCONFIG_UNCONFIGURE:
            ret = strmod_del(devno, &uc_sjis_info, &uc_sa);
            ret = strmod_del(devno, &lc_sjis_info, &lc_sa);
            break;

        case SYSCONFIG_RECONFIGURE:
            ret = EINVAL;
            break;

        case SYSCONFIG_QUERYSIZE:
            if (outdata && outdatalen >= sizeof(int)) {
                *(int *)outdata = do_querysize(attr,NUM_ATT);
            } else {
                ret = ENOMEM;
            }
            break;

        case SYSCONFIG_QUERY:
            size = do_querysize(attr,NUM_ATT);
            if ((outdata == NULL) || (outdatalen < size)) {
                ret = ENOMEM;
                break;
            }
            ret = do_query(attr,NUM_ATT,outdata,outdatalen);
            break;

        default:
            ret = EINVAL;
            break;
        }

        return(ret);

}
