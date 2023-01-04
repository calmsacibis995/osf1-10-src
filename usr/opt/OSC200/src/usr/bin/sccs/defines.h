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
 *	@(#)$RCSfile: defines.h,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/09/29 02:03:48 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: N/A
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* defines.h    1.4 9/13/89 21:17:48 */


/* debug stuff */
#define fmterr(s) Fmterr((s), __FILE__, __LINE__)

# include	"sys/types.h"
# include	"stdio.h"
# include	"macros.h"
# include	"fatal.h"
# include	"time.h"

#ifdef MSG
#define MF_SCCS "sccs.cat"
# include	"co_msg.h"
# include	"cm_msg.h"
# define	MS_SCCSDIFF 1
# define	MS_CM	2
# define	MS_CO	3
# define	MS_ADMIN 4
# define	MS_COMB 5
# define	MS_DELTA 6
# define	MS_GET 7
# define	MS_SCCSHELP 8
# define	MS_PRS 9
# define	MS_RMCHG 10
# define	MS_UNGET 11
# define	MS_VAL 12
# define	MS_VC 13
# define	MS_BSD 14
# define	MS_WHAT 15

extern nl_catd	catd;
#define MSGCO(Num, Str) NLcatgets(catd, MS_CO, Num, Str)
#define MSGCM(Num, Str) NLcatgets(catd, MS_CM, Num, Str)
#else
#define MSGCO(Num, Str) Str
#define MSGCM(Num, Str) Str
#endif

# define CTLSTR		"%c%c\n"

# define CTLCHAR	1
# define HEAD		'h'

# define STATS		's'

# define BDELTAB	'd'
# define INCLUDE	'i'
# define EXCLUDE	'x'
# define IGNORE		'g'
# define MRNUM		'm'
# define COMMENTS	'c'
# define EDELTAB	'e'

# define BUSERNAM	'u'
# define EUSERNAM	'U'

# define NFLAGS	26

# define FLAG		'f'
# define NULLFLAG	'n'
# define JOINTFLAG	'j'
# define DEFTFLAG	'd'
# define TYPEFLAG	't'
# define VALFLAG	'v'
#ifdef CASSI
# define CMFFLAG	'z'
#endif
# define BRCHFLAG	'b'
# define IDFLAG		'i'
# define MODFLAG	'm'
# define FLORFLAG	'f'
# define CEILFLAG	'c'
# define QSECTFLAG	'q'
# define LOCKFLAG	'l'

# define BUSERTXT	't'
# define EUSERTXT	'T'

# define INS		'I'
# define DEL		'D'
# define END		'E'

# define MINR		1		/* minimum release number */
# define MAXR		 9999	/* maximum release number */
# define FILESIZE	510
# define DELTA_LN_LNGTH 1024	/*max line length in a file to be deltaed*/
# define MAXLINE	512
# define DELIVER	'*'
# define LOGSIZE	(16)
# define FAILPUT    xmsg("","fputs")
/*
	Declares for external subroutines and/or functions
*/

extern	char	*sname();
extern	char	*cat(char *, ...);
extern	char	*dname();
extern	char	*repeat();
extern	char	*satoi();
extern	char	*strend();
extern	char	*trnslat();
extern	char	*zero();
extern	char	*zeropad();

/*
	SCCS Internal Structures.
*/

struct apply {
	char	a_inline;	/* in the line of normally applied deltas */
	int	a_code;		/* APPLY, NOAPPLY or SX_EMPTY */
	int	a_reason;
};
#define APPLY	  (1)
#define NOAPPLY  (-1)
#define SX_EMPTY	  (0)

# define IGNR		0100
# define USER		040
# define INCL		1
# define EXCL		2
# define CUTOFF		4
# define INCLUSER	(USER | INCL)
# define EXCLUSER	(USER | EXCL)
# define IGNRUSER	(USER | IGNR)


struct queue {
	struct queue *q_next;
	int    q_sernum;	/* serial number */
	char    q_keep;		/* keep switch setting */
	char	q_iord;		/* INS or DEL */
	char	q_ixmsg;	/* caused inex msg */
	char	q_user;		/* inex'ed by user */
};
#define YES	 (1)
#define NO	(-1)


struct	sid {
	int	s_rel;
	int	s_lev;
	int	s_br;
	int	s_seq;
};


struct	deltab {
	struct	sid	d_sid;
	int	d_serial;
	int	d_pred;
	time_t	d_datetime;
	char	d_pgmr[LOGSIZE];
	char	d_type;
};

struct	ixg {
	struct	ixg	*i_next;
	char	i_type;
	char	i_cnt;
	int	i_ser[1];
};


struct	idel {
	struct	sid	i_sid;
	struct	ixg	*i_ixg;
	int	i_pred;
	time_t	i_datetime;
};


# define maxser(pkt)	((pkt)->p_idel->i_pred)
# define sccsfile(f)	imatch("s.", sname(f))

struct packet {
	char	p_file[FILESIZE];	/* file name containing module */
	struct	sid	p_reqsid;	/* requested SID, then new SID */
	struct	sid	p_gotsid;	/* gotten SID */
	struct	sid	p_inssid;	/* SID which inserted current line */
	char	p_verbose;	/* verbose flags (see #define's below) */
	char	p_upd;		/* update flag (!0 = update mode) */
	time_t	p_cutoff;	/* specified cutoff date-time */
	int	p_ihash;	/* initial (input) hash */
	int	p_chash;	/* current (input) hash */
	int	p_nhash;	/* new (output) hash */
	int	p_glnno;	/* line number of current gfile line */
	int	p_slnno;	/* line number of current input line */
	char	p_wrttn;		/* written flag (!0 = written) */
	char	p_keep;		/* keep switch for readmod() */
	struct	apply	*p_apply;	/* ptr to apply array */
	struct	queue	*p_q;	/* ptr to control queue */
	FILE	*p_iop;		/* input file */
	char	p_buf[BUFSIZ];	/* input file buffer */
	char	p_line[BUFSIZ];	/* buffer for getline() */
	time_t	p_cdt;		/* date/time of newest applied delta */
	char	*p_lfile;	/* 0 = no l-file; else ptr to l arg */
	struct	idel	*p_idel;	/* ptr to internal delta table */
	FILE	*p_stdout;	/* standard output for warnings and messages */
	FILE	*p_gout;	/* g-file output file */
	char	p_user;		/* !0 = user on user list */
	char	p_chkeof;	/* 0 = eof generates error */
	int	p_maxr;		/* largest release number */
	int	p_ixmsg;	/* inex msg counter */
	int	p_reopen;	/* reopen flag used by getline on eof */
	int	p_ixuser;	/* HADI | HADX (in get) */
	int	do_chksum;	/* for getline(), 1 = do check sum */
};
/*
	Masks for p_verbose
*/

# define RLACCESS	(1)
# define NLINES		(2)
# define DOLIST		(4)
# define UNACK		(8)
# define NEWRL		(16)
# define WARNING	(32)


struct	stats {
	int	s_ins;
	int	s_del;
	int	s_unc;
};


struct	pfile	{
	struct	sid	pf_gsid;
	struct	sid	pf_nsid;
	char	pf_user[LOGSIZE];
	time_t	pf_date;
	char	*pf_ilist;
	char	*pf_elist;
#ifdef CASSI
	char 	*pf_cmrlist;
#endif
};


# define NVARGS	64
# define VSTART 3
