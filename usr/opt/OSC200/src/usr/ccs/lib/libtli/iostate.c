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
static char *rcsid = "@(#)$RCSfile: iostate.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/05/12 16:24:28 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1990  Mentat Inc.
 ** iostate.c 1.1, last change 6/4/90
 **/

/*** "iostate.c\t\t1.1"; ***/

#include <tli/common.h>
#include <sys/stream.h>
#include <tli/tihdr.h>
#include <tli/timod.h>


#define	FD_HASH_LIST_LEN	32

static	struct tli_st	*iostate[FD_HASH_LIST_LEN];

#ifdef _THREAD_SAFE
/**
 ** NOTE: iostate_sw() returns a locked tli structure.
 ** NOTE: It is the resposibility of the t_*() to unlock it.
 ** NOTE: There is a unlocked version of t_look for for thread safe
 ** NOTE: t_*() calls. 
#include <lib_lock.h>
#include <lib_data.h>
 **/

/*
extern lib_lock_functions_t	__t_lock_funcs;
*/
extern lib_data_functions_t	__t_data_funcs;
extern lib_mutex_t		__t_mutex;
#endif /* _THREAD_SAFE */

#define xx (-1)
char tli_state[TLI_NUM_EVENTS][TLI_NUM_STATES] = {
/*		   0	 1     2     3     4      5        6     7	8     */
/* STATES-------UNINIT UNBND IDLE OUTCON INCON DATAXFER OUTREL INREL STATECHG-*/
/* EVENTS   */
/* 0 OPEN   */	{  1,   xx,   xx,   xx,   xx,    xx,      xx,   xx,	 8    },
/* 1 BIND   */	{ xx,    2,   xx,   xx,   xx,    xx,      xx,   xx,      8    },
/* 2 OPTMGMT*/	{ xx,   xx,    2,   xx,   xx,    xx,      xx,   xx,      8    },
/* 3 UNBIND */	{ xx,   xx,    1,   xx,   xx,    xx,      xx,   xx,      8    },
/* 4 CLOSE  */	{ xx,    0,    0,    0,    0,     0,       0,    0,      8    },
/* 5 CONNECT1*/	{ xx,   xx,    5,   xx,   xx,    xx,      xx,   xx,      8    },
/* 6 CONNECT2*/	{ xx,   xx,    3,   xx,   xx,    xx,      xx,   xx,      8    },
/* 7 RCVCONN*/	{ xx,   xx,   xx,    5,   xx,    xx,      xx,   xx,      8    },
/* 8 LISTEN */	{ xx,   xx,    4,   xx,    4,    xx,      xx,   xx,      8    },
/* 9 ACCEPT1*/	{ xx,   xx,   xx,   xx,    5,    xx,      xx,   xx,      8    },
/*10 ACCEPT2*/	{ xx,   xx,   xx,   xx,    2,    xx,      xx,   xx,      8    },
/*11 ACCEPT3*/	{ xx,   xx,   xx,   xx,    4,    xx,      xx,   xx,      8    },
/*12 SND    */	{  8,    8,    8,    8,    8,     5,       8,    7,      8    },
/*13 RCV    */	{ xx,   xx,   xx,   xx,   xx,     5,       6,   xx,      8    },
/*14 SNDDIS1*/	{ xx,   xx,   xx,    2,    2,     2,       2,    2,      8    },
/*15 SNDDIS2*/	{ xx,   xx,   xx,   xx,    4,    xx,      xx,   xx,      8    },
/*16 RCVDIS1*/	{ xx,   xx,   xx,    2,   xx,     2,       2,    2,      8    },
/*17 RCVDIS2*/	{ xx,   xx,   xx,   xx,    2,    xx,      xx,   xx,      8    },
/*18 RCVDIS2*/	{ xx,   xx,   xx,   xx,    4,    xx,      xx,   xx,      8    },
/*19 SNDREL */	{ xx,   xx,   xx,   xx,   xx,     6,      xx,    2,      8    },
/*20 RCVREL */	{ xx,   xx,   xx,   xx,   xx,     7,       2,   xx,      8    },
/*21 PASSCON*/	{ xx,   xx,    5,   xx,   xx,    xx,      xx,   xx,      8    },
/*22 SNDUDATA*/	{ xx,   xx,    2,   xx,   xx,    xx,      xx,   xx,      8    },
/*23 RCVUDATA*/	{ xx,   xx,    2,   xx,   xx,    xx,      xx,   xx,      8    },
/*24 RCVUDATA*/	{ xx,   xx,    2,   xx,   xx,    xx,      xx,   xx,      8    }
		};
#undef xx


struct tli_st *
iostate_sw (
	int			fd,
	int			cmd)
{
	static int		initialized = false;
	struct tli_st **	tlipp;
	struct tli_st *		tli, *tlip;
	struct T_info_ack 	tinfo;

	
	IOSTATE_LOCK();
	tli = NULL;
	if ( !initialized ) {
                for ( tlipp = iostate; tlipp < A_END(iostate); tlipp++ )
                        *tlipp = nilp(struct tli_st);
                initialized = true;
        }

	for (tlipp = &iostate[fd % FD_HASH_LIST_LEN];
	    tlip = *tlipp; tlipp = &tlip->tlis_next) {
		if (tlip->tlis_fd == fd) {
			tli = tlip;
			break;
		}	
	}

	switch(cmd) {

	case IOSTATE_VERIFY:
		if (!tli) 
			_Set_terrno(TBADF);
		else
			TLI_LOCK(tli);
		IOSTATE_UNLOCK();
		break;

	case IOSTATE_SYNC:
		tinfo.PRIM_type = T_INFO_REQ;
		if (tli_ioctl(fd, TI_GETINFO, (char *)&tinfo, sizeof(tinfo)) == -1) {
			IOSTATE_UNLOCK();
			_Seterrno(EBADF);
			_Set_terrno(TBADF);
			tli = 0;
			break;
		}
		if(!tli) { 
			tli = (struct tli_st *)malloc(sizeof(struct tli_st) + 
			    _DEFAULT_STRCTLSZ);
			if ( !tli ) {
				IOSTATE_UNLOCK();
				_Set_terrno(TSYSERR);
				_Seterrno(ENOMEM);
				break;
			}
			if (!(TLI_LOCKCREATE(tli))) {
				free(tli);
				tli = 0;
				IOSTATE_UNLOCK();
				_Set_terrno(TSYSERR);
				_Seterrno(ENOMEM);
				break;
			}
			tli->tlis_next = nilp(struct tli_st);
			tli->tlis_fd = fd;
			*tlipp = tli;
			tli->tlis_flags = 0;
			tli->tlis_proto_buf = (char *)&tli[1];
		}
		TLI_LOCK(tli);
		IOSTATE_UNLOCK();
                tli->tlis_servtype = tinfo.SERV_type;
                tli->tlis_etsdu_size = tinfo.ETSDU_size;
                tli->tlis_tsdu_size = tinfo.TSDU_size;
                tli->tlis_tidu_size = tinfo.TIDU_size;;
		tli->tlis_state =  _txstate(tinfo.CURRENT_state);
		break;

	case IOSTATE_FREE:
		if (!tli) {
			IOSTATE_UNLOCK();
			_Seterrno(EBADF);
			_Set_terrno(TBADF);
			break;
		}
		TLI_LOCK(*tlipp);
		TLI_LOCK(tli);
		*tlipp = tli->tlis_next;
		if (*tlipp)
			TLI_UNLOCK(*tlipp);
		IOSTATE_UNLOCK();
		TLI_UNLOCK(tli);
		TLI_LOCKDELETE(tli);
		free(tli);
		break;
	   default:
		IOSTATE_UNLOCK();
		break;
	}
	return tli;
}

int
_txstate(
	int	st)
{
	switch (st) {
        case TS_UNBND:
                return T_UNBND;
        case TS_IDLE:
                return T_IDLE;
        case TS_WCON_CREQ:
                return T_OUTCON;
        case TS_WRES_CIND:
                return T_INCON;
        case TS_DATA_XFER:
                return T_DATAXFER;
        case TS_WIND_ORDREL:
                return T_OUTREL;
        case TS_WREQ_ORDREL:
                return T_INREL;
        case TS_WACK_BREQ:
        case TS_WACK_CREQ:
        case TS_WACK_CRES:
        case TS_WACK_DREQ6:
        case TS_WACK_DREQ7:
        case TS_WACK_DREQ9:
        case TS_WACK_DREQ10:
        case TS_WACK_DREQ11:
        case TS_WACK_OPTREQ:
        case TS_WACK_ORDREL:
        case TS_WACK_UREQ:
		return TSTATECHNG;
        default:
                break;
        }
	return -1;
}
