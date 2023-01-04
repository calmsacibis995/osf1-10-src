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
/* mshsbr.h - definitions for msh */
/* @(#)$RCSfile: mshsbr.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:35:29 $ devrcs Exp Locker: devbld $ */

struct Cmd {
    char    line[BUFSIZ];
    char   *args[MAXARGS];

    char   *redirect;

    int     direction;
#define	STDIO	0		/* regular stdoutput */
#define	CRTIO	1		/* create  re-direct */
#define	APPIO	2		/* append  re-direct */
#define	PIPIO	3		/* pipe    re-direct */

    FILE   *stream;
};
#define	NULLCMD	((struct Cmd *) 0)


struct Msg {
    struct drop m_drop;
#define	m_bboard_id	m_drop.d_id
#define	m_top		m_drop.d_size
#define	m_start		m_drop.d_start
#define	m_stop		m_drop.d_stop

    char   *m_scanl;
    
    struct tws  m_tb;

    short   m_stats;
#define CUR (1 << (FFATTRSLOT + NATTRS))
#ifdef	BPOP
#define	VIRTUAL	SELECT_EMPTY
#endif	BPOP
};

/*  */

				/* FOLDER */
extern char  *fmsh;		/* folder instead of file */
extern int    modified;		/* command modified folder */
extern struct msgs *mp;		/* used a lot */
extern struct Msg  *Msgs;	/* Msgs[0] not used */

FILE  *msh_ready ();


				/* COMMAND */
extern int  interactive;	/* running from a /dev/tty */
extern int  redirected;		/* re-directing output */
extern  FILE  *sp;		/* original stdout */

extern char *cmd_name;		/* command being run */

extern char myfilter[];		/* path to mhl.forward */



extern char *BBoard_ID;		/* BBoard-ID constant */


				/* SIGNALS */
extern TYPESIG  (*istat) ();	/* original SIGINT */
extern TYPESIG  (*qstat) ();	/* original SIGQUIT */
extern int  interrupted;	/* SIGINT detected */
extern int  broken_pipe;	/* SIGPIPE detected */
extern int  told_to_quit;	/* SIGQUIT detected */

#ifdef	BSD42
extern int  should_intr;	/* signal handler should interrupt call */
extern jmp_buf sigenv;		/* the environment pointer */
#endif	BSD42


long    lseek ();
