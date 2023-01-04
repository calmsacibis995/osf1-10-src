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
static char *rcsid = "@(#)$RCSfile: tsnd.c,v $ $Revision: 4.3.7.4 $ (DEC) $Date: 1993/11/16 15:29:34 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** tsnd.c 1.2, last change 1/29/90
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <stropts.h>
#include <tli/tihdr.h>

int
t_snd (
	int			fd,
	char *			buf,
	unsigned 		nbytes,
	int			flags)
{
	struct strbuf		ctlbuf;
	struct strbuf		databuf;
	long			max_tsdu_size;
	struct T_data_req	tdr;
	struct tli_st *		tli;
	long			total;
        int			code;

        code = -1;
	if(!(tli = iostate_sw(fd, IOSTATE_VERIFY)))
		goto badfd;

	if ( tli->tlis_servtype == T_CLTS ) {
		_Set_terrno(TNOTSUPPORT);
		goto rtn;
	}
	
#ifdef XTI
	if ( tli->tlis_state != T_DATAXFER  &&  tli->tlis_state != T_INREL ) {
		if (t_ilook(fd) == T_DISCONNECT) {
			_Set_terrno(TLOOK);
		} else {
			_Set_terrno(TOUTSTATE);
		}
		goto rtn;
	}
	if (flags & ~(T_MORE | T_EXPEDITED)) {
		_Set_terrno(TBADFLAG);
		goto rtn;
	}
#endif
	/* 
	 * Make sure that we have not accumulated a 0-byte TSDU.
	 * If this has a more flag set then just return (no need to send it to
	 * the transport provider)
	 */
	if (nbytes == 0) {
		if (flags & T_MORE) {
			/* no error, just simulate the packet to be 'sent' */
			code = 0;
			goto rtn;
		} else {
			/* this is the 'trailer' packet, or is it really? */
			if (!(tli->tlis_flags & TLIS_MORE_DATA)) {
				/* no preceeding packets, log an error */
				_Set_terrno(TBADDATA);
				goto rtn;
			}

		}
		/**/
	}
	/* If the T_MORE is set, save it for future reference */
	if (flags & T_MORE) {
		tli->tlis_flags |= TLIS_MORE_DATA;
	} else {
		tli->tlis_flags &= ~TLIS_MORE_DATA;
	}

	if (tli->tlis_tidu_size <= 0) {
	/*
	 * This shouldn't happen -- it means that the transport
	 * has given us a bogus "packet" size.
	 */
		_Set_terrno(TSYSERR);
		_Seterrno(EPROTO);
		goto rtn;
	}
	max_tsdu_size = (flags & T_EXPEDITED) ? 
		tli->tlis_etsdu_size : tli->tlis_tsdu_size;

	if (max_tsdu_size == -2) {
		_Set_terrno( TSYSERR);
		_Seterrno( EPROTO);
		goto rtn;
	}

	if (max_tsdu_size <= 0 || (max_tsdu_size > tli->tlis_tidu_size))
		max_tsdu_size = tli->tlis_tidu_size;

	max_tsdu_size = MIN(max_tsdu_size, _DEFAULT_STRMSGSZ);

	if(flags & T_EXPEDITED)
		tdr.PRIM_type = T_EXDATA_REQ;
	else
		tdr.PRIM_type = T_DATA_REQ;


	ctlbuf.buf = (char *)&tdr;
	ctlbuf.len = sizeof(tdr);
	databuf.buf = buf;
	
	total = 0;
        do {
                databuf.len = MIN(nbytes, max_tsdu_size);
                nbytes -= databuf.len;
		if ( (flags & T_MORE) || nbytes != 0)
                        tdr.MORE_flag = T_MORE;
		else
                        tdr.MORE_flag = 0;
			
                if (putmsg(fd, &ctlbuf, &databuf, 0) == -1) {
			t_unix_to_tli_error();
			if (total > 0) {
			    code = total;
#ifdef XTI
				if (flags & T_EXPEDITED)
					tli->tlis_flags |= TLIS_EXDATA_STOPPED;
                                else
					tli->tlis_flags |= TLIS_DATA_STOPPED;
#endif
			    goto rtn;
			}
#ifdef  XTI
                        if ( _Geterrno() == ERANGE ) {
                                /* 
				 * Secret message from TIMOD!
				 * T_DISCONNECT or T_ORDREL waiting
				 */
				_Set_terrno(TLOOK);
				goto rtn;
			}
#endif
			if ( _Get_terrno() == TNODATA ) {
				_Set_terrno(TFLOW);
#ifdef XTI
				if (flags & T_EXPEDITED)
					tli->tlis_flags |= TLIS_EXDATA_STOPPED;
                                else
					tli->tlis_flags |= TLIS_DATA_STOPPED;
#endif
                        }
                       	goto rtn; 
                }
                databuf.buf += databuf.len;
                total += databuf.len;
        } while (nbytes > 0);
#ifdef XTI
	if (flags & T_EXPEDITED)
		tli->tlis_flags &= ~TLIS_EXDATA_STOPPED;
	else
		tli->tlis_flags &= ~TLIS_DATA_STOPPED;
#endif

	code = total;
rtn:
#ifndef XTI
	if ( tli->tlis_state == TLI_TSTATECHNG) {
		_Set_terrno(TSYSERR);
		_Seterrno(EPROTO);
		code = -1;
	} else
#endif
		TLI_NEXTSTATE(tli, TLI_SND);
	TLI_UNLOCK(tli);

#ifdef XTIDBG
	tr_snd (fd, buf, nbytes, flags, code);
#endif

badfd:
	return code;
}
