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
static char *rcsid = "@(#)$RCSfile: stream_jis.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:06:57 $";
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

#include <sys/sysconfig.h>
#include <sys/termios.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/eucioctl.h>
#include <sys/jisioctl.h>
#include <tty/stream_tty.h>
#include <tty/stream_jis.h>

PRIVATE struct module_info uc_jis_modinfo = {
        UC_JIS_MODULE_ID, UC_JIS_MODULE_NAME, 0, INFPSZ, UC_JIS_HIWAT,
        UC_JIS_LOWAT
};

PRIVATE struct qinit uc_jis_rinit = {
        uc_jis_rput, uc_jis_rsrv,
        uc_jis_open, uc_jis_close, 0, &uc_jis_modinfo, 0
};

PRIVATE struct qinit uc_jis_winit = {
        uc_jis_wput, uc_jis_wsrv, 0, 0, 0, &uc_jis_modinfo, 0
};

struct streamtab uc_jis_info = { &uc_jis_rinit, &uc_jis_winit };

PRIVATE struct module_info lc_jis_modinfo = {
        LC_JIS_MODULE_ID, LC_JIS_MODULE_NAME, 0, INFPSZ, LC_JIS_HIWAT,
        LC_JIS_LOWAT
};

PRIVATE struct qinit lc_jis_rinit = {
        lc_jis_rput, lc_jis_rsrv,
        lc_jis_open, lc_jis_close, 0, &lc_jis_modinfo, 0
};

PRIVATE struct qinit lc_jis_winit = {
        lc_jis_wput, lc_jis_wsrv, 0, 0, 0, &lc_jis_modinfo, 0
};

struct streamtab lc_jis_info = { &lc_jis_rinit, &lc_jis_winit };

#define PUT_KIN(to, jp) do { 					\
	register int ii; 					\
	for (ii = 0; (jp)->ki[ii]; ii++) 			\
		*(to)++ = (jp)->ki[ii]; 			\
	(jp)->ajstate &= ~KS_2IN; 				\
	(jp)->ajstate |= KS_IN; 				\
	} while (0)

#define PUT_K2IN(to, jp) do { 					\
	register int ii; 					\
	for (ii = 0; (jp)->k2i[ii]; ii++) 			\
		*(to)++ = (jp)->k2i[ii]; 			\
	(jp)->ajstate &= ~KS_IN; 				\
	(jp)->ajstate |= KS_2IN; 				\
	} while (0)

#define PUT_KOUT(to, jp) do { 					\
	register int ii; 					\
	for (ii = 0; (jp)->ko[ii]; ii++) 			\
		*(to)++ = (jp)->ko[ii]; 			\
	(jp)->ajstate &= ~(KS_IN | KS_2IN);			\
	} while (0)

unsigned char *
conv_ajec2jis(
	register mblk_t *from,
	register unsigned char *to,
	register struct jis_s *jp,
	register unsigned char *in_prog,
	register int *state
	)
{
	unsigned char uc, uc1, uc2;

	if (!in_prog)
		in_prog = jp->ajc;
	if (!state)
		state = &jp->ajstate;

	while (from) {
	    while (from->b_rptr < from->b_wptr) {
		uc = *from->b_rptr++;
		uc1 = in_prog[0];
		uc2 = in_prog[1];
		if (!uc1) {
			if (INRANGE(0x00, 0x7f, uc)) {
				/*
				 * ascii and C0
				 */
				if (*state & (KS_IN|KS_2IN))
					PUT_KOUT(to, jp);
				if ((jp->kanastate == JIS_KANA_7BIT) &&
				    (*state & KS_KANA)) {
					*state &= ~KS_KANA;
					*to++ = SI;
				}
				*to++ = uc;
			} else if (INRANGE(0x81, 0x8d, uc) ||
				 INRANGE(0x90, 0xa0, uc)) {
				/*
				 * C1
				 */
				if (*state & (KS_IN|KS_2IN))
					PUT_KOUT(to, jp);
				if (jp->c1state == JIS_C1_PASS) {
					/*
					 * pass it as is
					 */
					*to++ = uc;
				} else if (jp->c1state == JIS_C1_C0) {
					/*
					 * convert to C0
					 */
					*to++ = 0x1b;
					*to++ = uc - 0x40;
				} else if (jp->c1state == JIS_C1_THROW) {
					/*
					 * throw away
					 */
				} else {
					*to++ = uc & 0x7f;
				}
			} else if ((uc == 0xff) || (uc == 0x80)) {
				*to++ = uc;
			} else {
				/*
				 * could be first byte of kanji or
				 * hankaku-kana
				 */
				in_prog[0] = uc;
			}
		} else if (uc1 == SS2) {
			/*
			 * hankaku-kana
			 */
			if (INRANGE(0xa1, 0xfe, uc)) {
				if (*state & (KS_IN|KS_2IN))
					PUT_KOUT(to, jp);
				if (jp->kanastate == JIS_KANA_7BIT) {
					if (*state & KS_KANA) {
						*to++ = uc & 0x7f;
					} else {
						*state |= KS_KANA;
						*to++ = SO;
						*to++ = uc & 0x7f;
					}
				} else {	/* JIS_KANA_8BIT */
					*to++ = uc;
				}
			} else {
				/*
				 * invalid code
				 */
				if (*state & (KS_IN|KS_2IN))
					PUT_KOUT(to, jp);
				*to++ = uc & 0x7f;
			}
			in_prog[0] = 0;
		} else if (INRANGE(0xa1, 0xfe, uc1)) {
			/*
			 * X0208 kanji
			 */
			if (INRANGE(0xa1, 0xfe, uc)) {
				/*
				 * valid kanji
				 */
				if ((jp->kanastate == JIS_KANA_7BIT) &&
				    (*state & KS_KANA)) {
					*state &= ~KS_KANA;
					*to++ = SI;
				}
				if (!(*state & KS_IN))
					PUT_KIN(to, jp);
				*to++ = in_prog[0] & 0x7f;
				*to++ = uc & 0x7f;
			} else {
				/*
				 * invalid code
				 */
				if (*state & (KS_IN|KS_2IN))
					PUT_KOUT(to, jp);
				*to++ = in_prog[0] & 0x7f;
				*to++ = uc & 0x7f;
			}
			in_prog[0] = 0;
		} else if (uc1 == SS3) {
			/* may be X0212 Kanji */
			if (uc2) {
				if (INRANGE(0xa1, 0xfe, uc)) {
					/*
					 * valid kanji
					 */
					if ((jp->kanastate == JIS_KANA_7BIT) &&
					    (*state & KS_KANA)) {
						*state &= ~KS_KANA;
						*to++ = SI;
					}
					if (!(*state & KS_2IN))
						PUT_K2IN(to, jp);
					*to++ = in_prog[1] & 0x7f;
					*to++ = uc & 0x7f;
				} else {
					/*
					 * invalid code
					 */
					if (*state & (KS_IN|KS_2IN))
						PUT_KOUT(to, jp);
					*to++ = in_prog[1] & 0x7f;
					*to++ = uc & 0x7f;
				}
				in_prog[0] = in_prog[1] = 0;
			} else if (INRANGE(0xa1, 0xfe, uc)) {
				/* may be X0212 Kanji */
				in_prog[1] = uc;
			} else {
				/*
				 * invalid code
				 */
				if (*state & (KS_IN|KS_2IN))
					PUT_KOUT(to, jp);
				*to++ = in_prog[0] & 0x7f;
				*to++ = uc & 0x7f;
			}
		} else {
			/*
			 * error
			 */
			if (*state & (KS_IN|KS_2IN))
				PUT_KOUT(to, jp);
			*to++ = uc1 & 0x7f;
			*to++ = uc & 0x7f;
			in_prog[0] = 0;
		}
	    }
	    from = from->b_cont;
	}
	return(to);
}

unsigned char *
conv_jis2ajec(
	register mblk_t *from,
	register unsigned char *to,
	register struct jis_s *jp,
	register unsigned char *in_prog,
	register int *state
	)
{
	register unsigned char *ucp, *ukpi, *ukp2i, *ukpo;
	register int m_in, m_2in, m_out;

	if (!in_prog)
		in_prog = jp->jac;
	if (!state)
		state = &jp->jastate;
	while (from) {
	    while (from->b_rptr < from->b_wptr) {
		ucp = in_prog;
		while (*ucp)
			ucp++;
		*ucp = *from->b_rptr++;
		/*
		 * try to match kanji in or kanji out
		 */
		ucp = in_prog;
		ukpi = jp->ki;
		ukp2i = jp->k2i;
		ukpo = jp->ko;
		m_in = m_2in = m_out = 1;
		while (*ucp && (m_in || m_2in || m_out)) {
			if (m_in && (*ucp != *ukpi))
				m_in = 0;
			if (m_2in && (*ucp != *ukp2i))
				m_2in = 0;
			if (m_out && (*ucp != *ukpo))
				m_out = 0;
			if (m_in || m_2in || m_out)
				ucp++;
			if (m_in)
				ukpi++;
			if (m_2in)
				ukp2i++;
			if (m_out)
				ukpo++;
		}
		if (m_in && (!*ukpi)) {
			/*
			 * kanji in
			 */
			*state |= KS_IN;
			bzero(in_prog, JA_SZ);
		} else if (m_2in && (!*ukp2i)) {
			/*
			 * kanji 0212 in
			 */
			*state |= KS_2IN;
			jp->k2count = 0;
			bzero(in_prog, JA_SZ);
		} else if (m_out && (!*ukpo)) {
			/*
			 * kanji out
			 */
			*state &= ~(KS_IN|KS_2IN);
			bzero(in_prog, JA_SZ);
		} else if ((m_in && *ukpi) || (m_2in && *ukp2i) ||  (m_out && *ukpo)) {
			/*
			 * partial match - don't do anything
			 */
		} else {
			/*
			 * not kanji in or kanji out
			 */
			ucp = in_prog;
			while (*ucp) {
				if (INRANGE(0x81, 0xa0, *ucp)) {
					/*
					 * C1
					 */
					if (jp->c1state == JIS_C1_PASS) {
						*to++ = *ucp;
					} else if (jp->c1state == JIS_C1_C0) {
						*to++ = 0x1b;
						*to++ = *ucp - 0x40;
					} else if (jp->c1state == JIS_C1_THROW){
						/* do nothing */
					} else {
						*to++ = *ucp & 0x7f;
					}
				} else if (*ucp == SI) {
					if (jp->kanastate == JIS_KANA_7BIT)
						*state &= ~KS_KANA;
					else
						*to++ = *ucp;
				} else if (*ucp == SO) {
					if (jp->kanastate == JIS_KANA_7BIT)
						*state |= KS_KANA;
					else
						*to++ = *ucp;
				} else if (*ucp & 0x80) {
					/*
					 * hankaku-kana if 8BIT
					 * otherwise just strip it
					 */
					if (jp->kanastate == JIS_KANA_8BIT) {
						*to++ = SS2;
						*to++ = *ucp;
					} else
						*to++ = *ucp & 0x7f;
				} else if (*state & KS_KANA) {
					/*
					 * hankaku-kana
					 */
					if (INRANGE(0x21, 0x5f, *ucp)) {
						*to++ = SS2;
						*to++ = (*ucp | 0x80);
					} else {
						*to++ = *ucp;
					}
				} else if (*state & KS_IN) {
					/*
					 * kanji
					 */
					if (INRANGE(0x21, 0x7e, *ucp))
						*to++ = (*ucp | 0x80);
					else
						*to++ = *ucp;
				} else if (*state & KS_2IN) {
					/*
					 * kanji 0212
					 */
					if (!(jp->k2count % 2))
						*to++ = SS3;
					jp->k2count++;
					if (INRANGE(0x21, 0x7e, *ucp))
						*to++ = (*ucp | 0x80);
					else
						*to++ = *ucp;
				} else {
					/*
					 * ascii
					 */
					*to++ = *ucp;
				}
				ucp++;
			}
			bzero(in_prog, JA_SZ);
		}
	    }
	    from = from->b_cont;
	}
	return(to);
}

int
jis_readdata(
	register struct jis_s *jp,
	queue_t *q,
	mblk_t *mp,
	unsigned char *(*conv_func)(),
	int mem_coeff,
	int mem_addend
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
	maxmsg = (msgdsize(mp) * mem_coeff) + mem_addend;
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

	next->b_wptr = (*conv_func)(mp, next->b_wptr, jp, 0, 0);

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
jis_writedata(
	register struct jis_s *jp,
	queue_t *q,
	mblk_t *mp,
	unsigned char *(*conv_func)(),
	int mem_coeff,
	int mem_addend
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

	maxmsg = (msgdsize(mp) * mem_coeff) + mem_addend;
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

	next->b_wptr = (*conv_func)(mp, next->b_wptr, jp, 0, 0);

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
 * jis_configure() - jis code converter entry point
 */
int
jis_configure(sysconfig_op_t  op,
              char *          indata,
              size_t          indatalen,
              char *          outdata,
              size_t          outdatalen)
{
        static struct streamadm lc_sa;
        static struct streamadm uc_sa;
        dev_t                   devno;
        int                     configured;
        struct subsystem_info   info;
        int                     size;
        int                     ret = 0;
        static config_attr_t attr[] = {
        { SUBSYS_NAME, STRTYPE, (caddr_t)"jis" , CONSTANT, {0, NULL} },
        { "lc_sa_flags", INTTYPE, (caddr_t)&lc_sa.sa_flags, CONSTANT, {0} },
        { "uc_sa_flags", INTTYPE, (caddr_t)&uc_sa.sa_flags, CONSTANT, {0} },
        { "lc_sa_name", STRTYPE, (caddr_t)lc_sa.sa_name, CONSTANT, {0, NULL} },
        { "uc_sa_name", STRTYPE, (caddr_t)uc_sa.sa_name, CONSTANT, {0, NULL} }
        };
#define NUM_ATT sizeof(attr)/sizeof(config_attr_t)

        /*
         * Check to see if subsystem has already been configured.
         */
        strcpy(info.subsystem_name, "jis");

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
             * If an input buffer has been supplied, then look for a devno
             * entry.  If found then devno is set to the given value.  If a
             * user hasn't specified a device number, then initialize it to
             * NODEV.
             */

            devno = NODEV;
            lc_sa.sa_version    = OSF_STREAMS_11;
            lc_sa.sa_flags      = STR_IS_MODULE | STR_SYSV4_OPEN;
            lc_sa.sa_ttys       = 0;
            lc_sa.sa_sync_level = SQLVL_QUEUEPAIR;
            lc_sa.sa_sync_info  = 0;
            strcpy(lc_sa.sa_name, LC_JIS_MODULE_NAME);

            if ((devno = strmod_add(devno, &lc_jis_info, &lc_sa)) == NODEV)
                ret = ENODEV;

            uc_sa.sa_version     = OSF_STREAMS_10;
            uc_sa.sa_flags       = STR_IS_MODULE | STR_SYSV4_OPEN;
            uc_sa.sa_ttys        = (caddr_t) 0;
            uc_sa.sa_sync_level  = SQLVL_QUEUEPAIR;
            uc_sa.sa_sync_info   = (caddr_t) 0;
            strcpy(uc_sa.sa_name, UC_JIS_MODULE_NAME);

            if ((devno = strmod_add(devno, &uc_jis_info, &uc_sa)) == NODEV) {
                return ENODEV;
            }

            if (outdata && outdatalen>=0)
                bcopy(indata,outdata,outdatalen);
            break;

        case SYSCONFIG_UNCONFIGURE:
            ret = strmod_del(devno, &uc_jis_info, &uc_sa);
            ret = strmod_del(devno, &lc_jis_info, &lc_sa);
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
