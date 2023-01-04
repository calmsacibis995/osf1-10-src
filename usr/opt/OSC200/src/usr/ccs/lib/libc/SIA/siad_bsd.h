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
 * @(#)$RCSfile: siad_bsd.h,v $ $Revision: 1.1.10.3 $ (DEC) $Date: 1993/08/04 21:22:01 $
 */
#ifdef MSG
#include "libc_msg.h"
#define MSGSTR(n,s) sia_getamsg(MF_LIBC,MS_LIBSIA,n,s)
#define	GETMSGSTR(g,n,s) sia_getamsg(MF_LIBC,g,n,s)
#else /* MSG */
#define MSGSTR(n,s) s
#define GETMSGSTR(g,n,s) s
#endif /* MSG */

#define	MAXGRP	        200
#define	MAXPWD	        200
#define MAXINT          0x7fffffff
#define	GROUP	        "/etc/group"
#define	PASSWD		"/etc/passwd"
#define NIS_DISALLOW    99
#define MAXGIDLEN       20
#define MAXUIDLEN       20
#define NOBODY          -2
#define MAX_NETGRNAME   256

#define	PWSKIP(s)	while (*(s) && *(s) != ':' && *(s) != '\n') \
				++(s); \
			if (*(s) && *(s) == ':') colon_count++; \
			if (*(s)) *(s)++ = 0;

#define	GRSKIP(s)	while (*(s) && *(s) != ':' && *(s) != '\n') \
				++(s); \
			if (*(s) && *(s) == ':') colon_count++; \
			if (*(s)) *(s)++ = 0;

#define ATOI(p,pout) {\
		register char *p_atoi = p;	\
		register int n_atoi;		\
		register int f_atoi;		\
		n_atoi = 0;			\
		f_atoi = 0;			\
		for(;;p_atoi++) {		\
			switch(*p_atoi) {	\
			case ' ':		\
			case '\t':		\
				continue;	\
			case '-':		\
				f_atoi++;	\
			case '+':		\
				p_atoi++;	\
			}			\
			break;			\
		}				\
		while(*p_atoi >= '0' && *p_atoi <= '9')			\
			n_atoi = n_atoi*10 + *p_atoi++ - '0';		\
		(pout) = (f_atoi ? -n_atoi : n_atoi);			\
	}


typedef struct list {
  char        *name;
  struct list *nxt;
} List;

#ifdef BSDGLOBAL

struct passwd siad_bsd_passwd;
struct group siad_bsd_group;
char siad_bsd_getpwbuf[SIABUFSIZ];
char siad_bsd_getgrbuf[BUFSIZ];

#else /* BSDGLOBAL */

extern struct passwd siad_bsd_passwd;
extern struct group siad_bsd_group;
extern char siad_bsd_getgrbuf[];
extern char siad_bsd_getpwbuf[];

#endif /* BSDGLOBAL */








