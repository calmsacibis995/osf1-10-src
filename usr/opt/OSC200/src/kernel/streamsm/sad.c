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
static char *rcsid = "@(#)$RCSfile: sad.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 14:55:48 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1990-1991  Mentat Inc.
 ** sad.c 2.3, last change 3/1/91
 **/

/*
static	char	sccsid[] = "@(#)sad.c\t\t2.3";
*/

#include <vm/vm_kern.h>

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strstat.h>
#include <sys/sad.h>

#ifndef staticf
#define staticf	static
#endif

#define A_CNT(arr)      (sizeof(arr)/sizeof(arr[0]))
#define	SAD_HASH(maj)	&sad_hash_tbl[((unsigned int)maj) % A_CNT(sad_hash_tbl)]

/* mi_copy facility with names changed */
#define	SAD_COPY_IN		1
#define	SAD_COPY_OUT		2
#define	SAD_COPY_DIRECTION(mp)	(*(int *)&(mp)->b_cont->b_next)
#define	SAD_COPY_COUNT(mp)	(*(int *)&(mp)->b_cont->b_prev)
#define	SAD_COPY_CASE(dir,cnt)	(((cnt)<<2)|dir)
#define	SAD_COPY_STATE(mp)	SAD_COPY_CASE(SAD_COPY_DIRECTION(mp),SAD_COPY_COUNT(mp))
#define	SAD_IS_TRANSPARENT(mp)	(mp->b_cont && (mp->b_cont->b_rptr != mp->b_cont->b_wptr))

staticf	void		sad_copyin(queue_t *, mblk_t *, char *, int);
staticf	void		sad_copyout(queue_t *, mblk_t *);
staticf	mblk_t *	sad_copyout_alloc(queue_t *, mblk_t *, char *, int);
staticf	void		sad_copy_done(queue_t *, mblk_t *, int);
staticf	int		sad_copy_state(queue_t *, mblk_t *, mblk_t **);

staticf void
sad_copyin (q, mp, uaddr, len)
	queue_t	* q;
	mblk_t	* mp;
	char	* uaddr;
	int	len;
{
	struct iocblk * iocp = (struct iocblk *)mp->b_rptr;
	struct copyreq * cq;
	int	err;
	mblk_t	* mp1;
	
	if ( mp->b_datap->db_type == M_IOCTL ) {
		if (iocp->ioc_count != TRANSPARENT) {
			mp1 = mp->b_cont;
			if (!uaddr
			&& (!mp1  ||  (mp1->b_wptr - mp1->b_rptr) < len)) {
				err = EINVAL;
				goto err_ret;
			}
			mp1 = allocb(0, BPRI_MED);
			if (!mp1) {
				err = ENOMEM;
				goto err_ret;
			}
			mp1->b_cont = mp->b_cont;
			mp->b_cont = mp1;
		}
		SAD_COPY_COUNT(mp) = 0;
		mp->b_datap->db_type = M_IOCDATA;
	} else if (!uaddr) {
		err = EPROTO;
		goto err_ret;
	}
	mp1 = mp->b_cont;
	cq = (struct copyreq *)iocp;
	cq->cq_private = mp1;
	cq->cq_size = len;
	cq->cq_addr = uaddr;
	cq->cq_flag = 0;
	SAD_COPY_DIRECTION(mp) = SAD_COPY_IN;
	SAD_COPY_COUNT(mp)++;
	if (!uaddr) {
		if ( !SAD_IS_TRANSPARENT(mp) ) {
			struct copyresp * cp = (struct copyresp *)mp->b_rptr;
			mp->b_datap->db_type = M_IOCDATA;
			mp->b_wptr = mp->b_rptr + sizeof(struct copyresp);
			mp->b_cont = mp1->b_cont;
			cp->cp_private->b_cont = 0;
			cp->cp_rval = 0;
			puthere(q, mp);
			return;
		}
		bcopy((char *)mp1->b_rptr, (char *)&cq->cq_addr, sizeof(cq->cq_addr));
	}
	mp->b_cont = 0;
	mp->b_datap->db_type = M_COPYIN;
	mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
	qreply(q, mp);
	return;
err_ret:
	iocp->ioc_error = err;
	iocp->ioc_count = 0;
	if ( mp->b_cont ) {
		freemsg(mp->b_cont);
		mp->b_cont = 0;
	}
	mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
	mp->b_datap->db_type = M_IOCACK;
	qreply(q, mp);
	return;
}

staticf void
sad_copyout (q, mp)
	queue_t	* q;
	mblk_t	* mp;
{
	struct iocblk * iocp = (struct iocblk *)mp->b_rptr;
	struct copyreq * cq = (struct copyreq *)iocp;
	struct copyresp * cp = (struct copyresp *)cq;
	mblk_t	* mp1;
	mblk_t	* mp2;
	
	if ( mp->b_datap->db_type != M_IOCDATA  ||  !mp->b_cont) {
		sad_copy_done(q, mp, EPROTO);
		return;
	}
	/* Check completion of previous copyout operation. */
	mp1 = mp->b_cont;
	if ( (int)cp->cp_rval  ||  !mp1->b_cont) {
		sad_copy_done(q, mp, (int)cp->cp_rval);
		return;
	}
	if (!mp1->b_cont->b_cont  &&  !SAD_IS_TRANSPARENT(mp)) {
		mp1->b_next = 0;
		mp1->b_prev = 0;
		mp->b_cont = mp1->b_cont;
		freeb(mp1);
		mp1 = mp->b_cont;
		mp1->b_next = 0;
		mp1->b_prev = 0;
		iocp->ioc_count = mp1->b_wptr - mp1->b_rptr;
		iocp->ioc_error = 0;
		mp->b_datap->db_type = M_IOCACK;
		mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
		qreply(q, mp);
		return;
	}
	if (SAD_COPY_DIRECTION(mp) == SAD_COPY_IN) {
		/* Set up for first copyout. */
		SAD_COPY_DIRECTION(mp) = SAD_COPY_OUT;
		SAD_COPY_COUNT(mp) = 1;
	} else {
		++SAD_COPY_COUNT(mp);
	}
	cq->cq_private = mp1;
	/* Find message preceding last. */
	for ( mp2 = mp1; mp2->b_cont->b_cont; mp2 = mp2->b_cont )
		;
	if (mp2 == mp1)
		bcopy((char *)mp1->b_rptr, (char *)&cq->cq_addr, sizeof(cq->cq_addr));
	else
		cq->cq_addr = (char *)mp2->b_cont->b_next;
	mp1 = mp2->b_cont;
	mp->b_datap->db_type = M_COPYOUT;
	mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
	mp->b_cont = mp1;
	mp2->b_cont = 0;
	mp1->b_next = 0;
	cq->cq_size = mp1->b_wptr - mp1->b_rptr;
	cq->cq_flag = 0;
	qreply(q, mp);
	return;
}

staticf void
sad_copy_done (q, mp, err)
	queue_t	* q;
	mblk_t	* mp;
	int	err;
{
	struct iocblk * iocp;
	mblk_t	* mp1;
	
	if (!mp)
		return;
	if (!q  ||  (mp->b_wptr - mp->b_rptr) < sizeof(struct iocblk)) {
		freemsg(mp);
		return;
	}
	iocp = (struct iocblk *)mp->b_rptr;
	mp->b_datap->db_type = M_IOCACK;
	mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
	iocp->ioc_error = err;
	iocp->ioc_count = 0;
	if (mp1 = mp->b_cont) {
		for ( ; mp1; mp1 = mp1->b_cont) {
			mp1->b_prev = 0;
			mp1->b_next = 0;
		}
		freemsg(mp->b_cont);
		mp->b_cont = 0;
	}
	qreply(q, mp);
}

staticf int
sad_copy_state (q, mp, mpp)
	queue_t	* q;
	mblk_t	* mp;
	mblk_t	* * mpp;
{
	struct iocblk * iocp = (struct iocblk *)mp->b_rptr;
	struct copyresp * cp = (struct copyresp *)iocp;
	mblk_t	* mp1;
	
	mp1 = mp->b_cont;
	mp->b_cont = cp->cp_private;
	if (mp1)
		linkb(mp->b_cont, mp1);
	if ( cp->cp_rval ) {
		sad_copy_done(q, mp, (int)cp->cp_rval);
		return -1;
	}
	if ( mpp  &&  SAD_COPY_DIRECTION(mp) == SAD_COPY_IN )
		*mpp = mp1;
	return SAD_COPY_STATE(mp);
}

/* end mi_copy */

typedef	struct msgb	* MBLKP;
typedef	struct msgb	** MBLKPP;

/* Structure of each element in the sad_hash_tbl */
typedef struct sad_hash_s {
	struct sad_hash_s	* sad_next;
	int			sad_privileged;
	struct strapush		sad_strapush;
} SAD, * SADP, ** SADPP;

staticf	int	sad_close(queue_t *, int, cred_t *);
staticf	int	sad_open(queue_t *, dev_t *, int, int, cred_t *);
staticf	int	sad_wput(queue_t *, MBLKP);

static struct module_info minfo =  {
#define	MODULE_ID	45
	MODULE_ID, "sad", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	NULL, NULL, sad_open, sad_close, NULL, &minfo
};

static struct qinit winit = {
	sad_wput, NULL, NULL, NULL, NULL, &minfo
};

struct streamtab sadinfo = { &rinit, &winit };

static	SADP	sad_hash_tbl[32];
/*
 * We depend on module synch to prevent multiple sad updates, but
 * still require a lock to protect sad_get_autopush() lookups.
 */
decl_simple_lock_data(static, sad_lock)

void
sad_init()
{
	simple_lock_init(&sad_lock);
}

int
sad_get_autopush (major_num, minor_num, stra)
	int	major_num;
	int	minor_num;
	struct strapush * stra;
{
	SADP	sad;
	int	ret = 0;

	sad = *SAD_HASH(major_num);
	simple_lock(&sad_lock);
	for ( ; sad; sad = sad->sad_next) {
		if (sad->sad_strapush.sap_major == major_num
		&& (sad->sad_strapush.sap_minor == -1
		|| (sad->sad_strapush.sap_minor <= minor_num
		&&  sad->sad_strapush.sap_lastminor >= minor_num))) {
			if (stra)
				*stra = sad->sad_strapush;
			ret = 1;
			break;
		}
	}
	simple_unlock(&sad_lock);
	return ret;
}

staticf int
sad_close (q, flag, credp)
	queue_t	* q;
	int	flag;
	cred_t	* credp;
{
	return streams_close_comm(q, flag, credp);
}

staticf int
sad_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	int	err;

	err = streams_open_comm(sizeof (int), q, devp, flag, sflag, credp);
	if (err == 0 && drv_priv(credp) == 0)
		*(int *)q->q_ptr = 1;
	return err;
}

staticf int
sad_wput (q, mp)
	queue_t	* q;
	MBLKP	mp;
{
	int	copyin_size;
	int	err;
	int	i1;
	struct iocblk	* iocp;
	MBLKP	mp1;
	struct str_list	* sl;
	struct str_mlist	* sml;
	struct strapush	* stra;
	SADP	sad;
	SADPP	sadp;

	switch (mp->b_datap->db_type) {
	case M_PCPROTO:
	case M_PROTO:
	case M_DATA:
		break;
	case M_IOCTL:
		/* All SAD IOCTLs are handled both in I_STR and 
		 * TRANSPARENT form using the sad_copy facility.
		 */
		iocp = (struct iocblk *)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case SAD_SAP:
			if (*(int *)q->q_ptr == 0) {
				iocp->ioc_error = EACCES;
				goto bad;
			} /* else fall through */
		case SAD_GAP:
			copyin_size = sizeof(struct strapush);
			break;
		case SAD_VML:
			copyin_size = sizeof(struct str_list);
			break;
		default:
		bad:
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = 0;
			qreply(q, mp);
			return 0;
		}
		sad_copyin(q, mp, (char *)0, copyin_size);
		return 0;
	case M_IOCDATA:
		iocp = (struct iocblk *)mp->b_rptr;
		err = 0;
		switch ( sad_copy_state(q, mp, &mp1) ) {
		case -1:
			/* sad_copy_state cleans up and completes the ioctl. */
			return 0;
		case SAD_COPY_CASE(SAD_COPY_IN, 1):
			/* Completed first copyin. */
			switch ( iocp->ioc_cmd ) {
			case SAD_SAP:
				stra = (struct strapush *)mp1->b_rptr;
				switch (stra->sap_cmd) {
				case SAP_ONE:
					stra->sap_lastminor = stra->sap_minor;
					if (sad_get_autopush(stra->sap_major,
							stra->sap_minor,
							(struct strapush *)0)) {
						err = EEXIST;
						goto iocack;
					}
					break;
				case SAP_ALL:
					stra->sap_minor = -1;
					stra->sap_lastminor = -1;
					/* Check to make sure there are no
					 * settings for this major number */
					sad = *SAD_HASH(stra->sap_major);
					for ( ; sad; sad = sad->sad_next) {
						if (sad->sad_strapush.sap_major
						==  stra->sap_major) {
							err = EEXIST;
							goto iocack;
						}
					}
					break;
				case SAP_RANGE:
					if (stra->sap_lastminor <
							stra->sap_minor) {
						err = ERANGE;
						goto iocack;
					}
					/* Check to make sure there are no
					 * overlapping settings for this
					 * major number */
					sad = *SAD_HASH(stra->sap_major);
					for ( ; sad; sad = sad->sad_next) {
						if (sad->sad_strapush.sap_major
						!=  stra->sap_major)
							continue;
						if (stra->sap_lastminor >=
						    sad->sad_strapush.sap_minor
						||  stra->sap_minor <
						    sad->sad_strapush.sap_lastminor) {
							err = EEXIST;
							goto iocack;
						}
					}
					break;
				case SAP_CLEAR:
					sadp = SAD_HASH(stra->sap_major);
					simple_lock(&sad_lock);
for ( ; sad = *sadp; sadp = &sad->sad_next) {
	if (stra->sap_major == sad->sad_strapush.sap_major) {
		if (stra->sap_minor == sad->sad_strapush.sap_minor
		|| (stra->sap_minor == 0 && sad->sad_strapush.sap_minor == -1)){
			*sadp = sad->sad_next;
			simple_unlock(&sad_lock);
			kmem_free(sad,sizeof(SAD));
			goto iocack;
		}
		if (stra->sap_minor >= sad->sad_strapush.sap_minor
		&&  stra->sap_minor <= sad->sad_strapush.sap_lastminor) {
			err = ERANGE;
			break;
		}
	}
}
					simple_unlock(&sad_lock);
					if (err)
						goto iocack;
					err = ENODEV;
					goto iocack;
				default:
					err = EINVAL;
					goto iocack;
				}
				if (dcookie_to_dindex(stra->sap_major) == -1
				||  stra->sap_npush < 0
				||  stra->sap_npush > MAXAPUSH) {
					err = EINVAL;
					break;
				}
				for (i1 = 0; i1 < stra->sap_npush; i1++) {
					if (!fname_to_str(stra->sap_list[i1])) {
						err = EINVAL;
						goto iocack;
					}
				}
				sad = (SADP)kmem_alloc(kernel_map,sizeof(SAD));
				if (sad == NULL) {
					err = ENOSR;
					break;
				}
				sad->sad_privileged = *(int *)q->q_ptr;
				sad->sad_strapush = *stra;
				sadp = SAD_HASH(stra->sap_major);
				simple_lock(&sad_lock);
				sad->sad_next = *sadp;
				*sadp = sad;
				simple_unlock(&sad_lock);
				break;
			case SAD_GAP:
				/* We have the input strapush structure, find
				 * the information requested and copyout */
				stra = (struct strapush *)mp1->b_rptr;
				if (sad_get_autopush(stra->sap_major,
						stra->sap_minor, stra)) {
					mp1->b_wptr = mp1->b_rptr +
							sizeof(struct strapush);
					sad_copyout(q, mp);
					return 0;
				}
				if (dcookie_to_dindex(stra->sap_major) == -1)
					err = ENOSTR;
				else
					err = ENODEV;
				break;
			case SAD_VML:
				sl = (struct str_list *)mp1->b_rptr;
				if (sl->sl_nmods <= 0) {
					err = EINVAL;
					break;
				}
				/* Copy in the module list. */
				sad_copyin(q, mp, (char *)sl->sl_modlist,
				    sl->sl_nmods * sizeof(struct str_mlist));
				return 0;
			default:
				err = EPROTO;
				goto iocack;
			}
			break;
		case SAD_COPY_CASE(SAD_COPY_IN, 2):
			/* Completed second copyin. */
			switch ( iocp->ioc_cmd ) {
			case SAD_VML:
				/* Now we have the module list. */
				if ( !mp->b_cont  ||  !mp->b_cont->b_cont ) {
					err = EPROTO;
					break;
				}
				sl = (struct str_list *)mp->b_cont->b_cont->b_rptr;
				sml = (struct str_mlist *)mp1->b_rptr;
				iocp->ioc_rval = 0;
				for (i1 = 0; i1 < sl->sl_nmods; i1++) {
					if (!fname_to_str(sml[i1].l_name)) {
						iocp->ioc_rval = 1;
						break;
					}
				}
				break;
			default:
				err = EPROTO;
				break;
			}
			break;
		case SAD_COPY_CASE(SAD_COPY_OUT, 1):
			/* Completed copyout. */
			break;
		default:
			err = EPROTO;
			break;
		}
iocack:;
		sad_copy_done(q, mp, err);
		return 0;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 0;
		}
		break;
	}
	freemsg(mp);
	return 0;
}
