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
static char *rcsid = "@(#)$RCSfile: pckt.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/09/07 21:14:32 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/sysconfig.h>
#ifndef staticf
#define staticf static
#endif

staticf	int	pckt_close(queue_t *, int, cred_t *);
staticf	int	pckt_open(queue_t *, dev_t *, int, int, cred_t *);
staticf	int	pckt_rput(queue_t *, mblk_t *);
staticf	int	pckt_rsrv(queue_t *);
staticf	int	pckt_wput(queue_t *, mblk_t *);

static struct module_info minfo =  {
	5004, "pckt", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	pckt_rput, pckt_rsrv, pckt_open, pckt_close, NULL, &minfo
};

static struct qinit winit = {
	pckt_wput, NULL, NULL, NULL, NULL, &minfo
};

struct streamtab pcktinfo = { &rinit, &winit };

int
pckt_configure(op, indata, indatalen, outdata, outdatalen)
        sysconfig_op_t  op;
        str_config_t *  indata;
        size_t          indatalen;
        str_config_t *  outdata;
        size_t          outdatalen;
{
	struct streamadm	sa;
	dev_t			devno;

	if (op != SYSCONFIG_CONFIGURE)
		return EINVAL;

	if (indata != NULL && indatalen == sizeof(str_config_t)
			&& indata->sc_version == OSF_STREAMS_CONFIG_10)
		devno = indata->sc_devnum;
	else
		devno = NODEV;

	sa.sa_version		= OSF_STREAMS_10;
	sa.sa_flags		= STR_IS_MODULE|STR_SYSV4_OPEN;
	sa.sa_ttys		= 0;
	sa.sa_sync_level	= SQLVL_QUEUE;
	sa.sa_sync_info		= 0;
	strcpy(sa.sa_name, 	"pckt");

	if ( (devno = strmod_add(devno, &pcktinfo, &sa)) == NODEV ) {
		return ENODEV;
	} 

	if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
		outdata->sc_version = OSF_STREAMS_CONFIG_10;
		outdata->sc_devnum = NODEV;
		outdata->sc_sa_flags = sa.sa_flags;
		strcpy(outdata->sc_sa_name, sa.sa_name);
	}

	return 0;
}

staticf int
pckt_close (q, flag, credp)
	queue_t	* q;
    	int flag;
    	cred_t *credp;
{
	return 0;
}

staticf int
pckt_open (q, devp, flag, sflag, credp)
    queue_t *q;
    dev_t *devp;
    int	flag;
    int	sflag;
    cred_t *credp;
{
    return 0;
}
staticf int
pckt_wput (q, mp)
    queue_t *q;
    mblk_t *mp;
{
    putnext(q, mp);
    return 0;
}

static int
pckt_rput (q, mp)
    queue_t *q;
    mblk_t *mp;
{
    putq(q, mp);
    return 0;
}

staticf int
pckt_rsrv (q)
    queue_t *q;
{
    register mblk_t *mp;
    register mblk_t *mp1;
    register mblk_t *mp2;
    unsigned char flush;
    
    while (mp = getq(q)) {
	if (!canput(q->q_next))
	    return putbq(q, mp);
	switch (mp->b_datap->db_type) {
   	/*
	 * the Streams Prog. Guide (and SVR4)
	 * are in error as far as correct flush processing goes...
	 * the correct behaviour is to send a FLUSHW if the FLUSHW
	 * bit is set.  The Streams Prog. Guide incorrectly says to
	 * send a FLUSHW if the FLUSH bit is set.
         */
	case M_FLUSH:
	    mp1 = allocb(4,BPRI_HI);
	    if (mp1) {
		mp1->b_datap->db_type = M_PROTO;
		*(int *)mp1->b_wptr = mp->b_datap->db_type;
		mp1->b_wptr += 4;
		mp1->b_cont = mp;
	    } else {
		bufcall(4, BPRI_HI, qenable, q);
		return putbq(q, mp);
	    }
	    flush = *mp->b_rptr;
	    switch (flush) {
		case FLUSHW:
			*mp->b_rptr = FLUSHR;
		case (FLUSHR|FLUSHW):
			mp2 = allocb(1, BPRI_HI);
			if (!mp2) {
				/* perform buf call for total byes needed */
				bufcall(5, BPRI_HI, qenable, q);
				free(mp1);
				return putbq(q, mp);
			}
			mp2->b_datap->db_type = M_FLUSH;
			*mp->b_rptr = FLUSHW;
	    		putnext(q, mp2);
			break;
		case FLUSHR:
			*mp->b_rptr = FLUSHW; 
			break;
		/* leave flushband data intact */
		default:
			break;
	    }
	    mp = mp1;
	    putnext(q, mp);
	    break;
	case M_PROTO:
	case M_PCPROTO:
	case M_STOP:
	case M_START:
	case M_STOPI:
	case M_STARTI:
	case M_IOCTL:
	case M_DATA:
	case M_READ:
	    mp1 = allocb(4,BPRI_HI);
	    if (mp1) {
		mp1->b_datap->db_type = M_PROTO;
		*(int *)mp1->b_wptr = mp->b_datap->db_type;
		mp1->b_wptr += 4;
		mp1->b_cont = mp;
		mp = mp1;
	    } else {
		bufcall(4, BPRI_HI, qenable, q);
		return putbq(q, mp);
	    }
	/* Fall Through */
	default:
	    putnext(q, mp);
	}
    }
    return 0;
}
