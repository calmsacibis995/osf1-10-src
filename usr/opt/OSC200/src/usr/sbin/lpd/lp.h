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
 *	@(#)$RCSfile: lp.h,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/04/16 08:42:26 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	lp.h	5.3 (Berkeley) 6/30/88
 * 	lp.h	4.1 15:58:36 7/19/90 SecureWare 
 */

/*
 * Global definitions for the line printer system.
 */

#include <sys/secdefines.h>

#include <stdio.h>
#include <strings.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/wait.h>
#include <sgtty.h>
#include <ctype.h>
#include <errno.h>
#include "lp.local.h"
#include "printer_msg.h"

extern int h_errno;		/* get resolver error stat */

extern char *CT_choices[];
#define DEFCT                CT_choices[0]

/*
 * print_type_e -- enumeration to select job building functions
 */
enum printer_type_e {
        pt_non_PS, pt_LN03R, pt_LPS, pt_upb
};


extern int	DU;		/* daeomon user-id */
extern int	MX;		/* maximum number of blocks to copy */
extern int	MC;		/* maximum number of copies allowed */
extern char	*LP;		/* line printer device name */
extern char	*RM;		/* remote machine name */
extern char	*RG;		/* restricted group */
extern char	*RP;		/* remote printer name */
extern char	*LO;		/* lock file name */
extern char	*ST;		/* status file name */
extern char	*SD;		/* spool directory */
extern char	*AF;		/* accounting file */
extern char	*LF;		/* log file for error messages */
extern char	*OF;		/* name of output filter (created once) */
extern char	*IF;		/* name of input filter (created per job) */
extern char	*RF;		/* name of fortran text filter (per job) */
extern char	*TF;		/* name of troff(1) filter (per job) */
extern char	*NF;		/* name of ditroff(1) filter (per job) */
extern char	*DF;		/* name of tex filter (per job) */
extern char	*GF;		/* name of graph(1G) filter (per job) */
extern char	*VF;		/* name of raster filter (per job) */
extern char	*CF;		/* name of cifplot filter (per job) */
extern char	*XF;		/* name of non-filter filter (per job) */
extern char	*FF;		/* form feed string */
extern char	*TR;		/* trailer string to be output when Q empties */
/* CMU extension */
extern char	*HN;		/* host name for filter (-i option) */
extern char	*FA;		/* additional argument to filter (-A option) */

extern short	SC;		/* suppress multiple copies */
extern short	SF;		/* suppress FF on each print job */
extern short	SH;		/* suppress header page */
extern short	SB;		/* short banner instead of normal header */
extern short	HL;		/* print header last */
extern short	RW;		/* open LP for reading and writing */
extern short	PW;		/* page width */
extern short	PX;		/* page width in pixels */
extern short	PY;		/* page length in pixels */
extern short	PL;		/* page length */
extern short	BR;		/* baud rate if lp is a tty */
extern int	FC;		/* flags to clear if lp is a tty */
extern int	FS;		/* flags to set if lp is a tty */
extern int	XC;		/* flags to clear for local mode */
extern int	XS;		/* flags to set for local mode */
extern short	RS;		/* restricted to those with local accounts */
#if SEC_MAC
extern short	PS;		/* printer expects PostScript input */
#endif
extern char     *TS;		/* LAT terminal server name */
extern char     *OP;		/* LAT terminal server port ID */
extern char     *OS;		/* LAT service to print to */
extern char	 *CT;

extern char	line[BUFSIZ];
extern char	pbuf[];		/* buffer for printcap entry */
extern char	*bp;		/* pointer into ebuf for pgetent() */
extern char	*name;		/* program name */
extern char	*printer;	/* printer name */
extern char	host[MAXHOSTNAMELEN];	/* host machine name */
extern char	*from;		/* client's machine name */
extern int	errno;

/*
 * Structure used for building a sorted list of control files.
 */
struct queue {
	time_t	q_time;			/* modification time */
	char	q_name[NAME_MAX+1];	/* control file name */
};

char	*pgetstr();
char	*malloc();
char	*getenv();
/* char	*index();
char	*rindex(); 
*/

/*
 * Message catalog support 
 */
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRINTER,n,s)
#ifdef	SEC_BASE
#define	MSGSTR_SEC(n,s)	catgets(catd,MS_PRINTER_SEC,n,s)
#endif

/*
 * This enumeration is used for the connection type
 * which is stored in the connection object on initialisation
 */
enum connection_type_e {
	con_dev, con_lat, con_remote, con_network, con_tcp, con_dqs
};

/*
 * This enumeration is used to record the connection state
 * Note that the start/stop state is maintained by the
 * filter object, if there is one and therefore does not
 * need to be tracked by the connection object as well
 */
enum cx_state_e { cxs_closed, cxs_open, };

/*
 * This structure is the implementation of the connection object
 */
struct connection {
	enum cx_state_e cx_state; /* connection state */
	int cx_out_fd;		/* output file descriptor (was ofd) */
	int cx_pr_fd;		/* printer file descriptor (was pfd) */
	char * cx_output_filter;	/* the output filter */
	enum connection_type_e cx_type;
};

typedef struct connection *CXP;	/* short hand for object pointer */

/*
 * Structure for table of functions implementing operations
 * on connection object
 */
struct cx_fns {
	int (*cxf_open)(/* CXP */);	/* Open the connection */
	int (*cxf_close)(/* CXP */);	/* Close the connection */
	int (*cxf_stop)(/* CXP */);	/* Stop & bypass the filter */
	int (*cxf_start)(/* CXP */);	/* Restart the filter */
};

/*
 * These are the exported calls available on a connection object
 */

extern void cx_init(/* CXP cxp; enum connection_type_e connection_type */);
extern void cx_delete(/* CXP cxp; int on_heap */);

extern int cx_open(/* CXP cxp */);
extern int cx_close(/* CXP cxp */);
extern int cx_stop(/* CXP cxp */);
extern int cx_start(/* CXP cxp */);
