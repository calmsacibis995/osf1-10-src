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
 *	@(#)$RCSfile: defs.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/29 19:20:52 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

#ifndef lint

#endif

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>

#ifndef dirfd
#define dirfd(dirp)	((dirp)->dd_fd)
#endif

#define SHELLCOM	"/bin/sh"

#define RCS		"RCS"	/* name of RCS dir */
#define RCS_SUF		",v"	/* suffix of RCS files */

/*
 * to install metering, add a statement like
 * #define METERFILE "/usr/sif/make/Meter"
 * to turn metering on, set external variable meteron to 1.
 */

/*
 * define FSTATIC to be static on systems with C compilers
 * supporting file-static; otherwise define it to be null
 */
#define FSTATIC		static

#define NO		0
#define YES		1

#define unequal		strcmp
#define HASHSIZE	8191
#define INMAX		ARG_MAX                    
#define OUTMAX		ARG_MAX
#define QBUFMAX		ARG_MAX
#define MAXARGV         (NCARGS / 10)

#define ALLDEPS		1
#define SOMEDEPS	2

#define META		01
#define TERMINAL	02
extern char funny[128];

#ifdef lint
#define ALLOC(x)	((struct x *) 0)
#else
#define ALLOC(x)	((struct x *) ckalloc(sizeof(struct x)))
#endif

/*
 * RCS-specific declarations
 */
extern int	coflag;			/* auto-checkout flag */
extern int	rmflag;			/* auto-remove flag */
extern int	nocmds;			/* don't make, just check-out */
extern int	rcsdep;			/* do "foo: foo,v" dependencies */
extern struct lineblock *co_cmd;	/* auto-checkout shell command */
extern struct lineblock *rcstime_cmd;	/* rcs modtime shell command */
extern char	*RCSdir;		/* name of RCS dir */
extern char	*RCSsuf; 		/* suffix of RCS files */

extern int dbgflag;
extern int prtrflag;
extern int silflag;
extern int noexflag;
extern int keepgoing;
extern int descflag;
extern int noruleflag;
extern int noconfig;
extern int touchflag;
extern int questflag;
extern int machdep;
extern int ndocoms;
extern int ignerr;
extern int botchflag;
extern int okdel;
extern int inarglist;
extern int ininternal;
extern char *prompt;
extern char *curfname;

struct nameblock {
	struct nameblock *nxtnameblock;
	char *namep;
	char *alias;
	struct lineblock *linep;
	struct tmacblock *tmacp;
	unsigned int done:2;
	unsigned int septype:2;
	unsigned int rundep:1;
	unsigned int objarch:1;
	unsigned int dollar:1;
	unsigned int percent:1;
	unsigned int bquotes:1;
	unsigned int isarch:1;
	unsigned int aliasok:1;
	struct nameblock *archp;
	char *RCSnamep;
	int hashval;
	time_t modtime;
};
extern struct nameblock *mainname;
extern struct nameblock *firstname;
extern struct nameblock *inobjdir;

struct lineblock {
	struct lineblock *nxtlineblock;
	struct nameblock *lockp;
	struct depblock *depp;
	struct shblock *shp;
};
extern struct lineblock *sufflist;

struct depblock {
	struct depblock *nxtdepblock;
	struct nameblock *depname;
};

struct shblock {
	struct shblock *nxtshblock;
	char *shbp;
	int exok;
};

struct tmacblock {
	struct tmacblock *nxttmacblock;
	char *tmacbp;
};

struct varblock {
	struct varblock *lftvarblock;
	struct varblock *rgtvarblock;
	char *varname;
	char *varval;
	short varlen;
	unsigned int noreset:1;
	unsigned int used:1;
	unsigned int builtin:1;
};
extern struct varblock *firstvar;

struct pattern {
	struct pattern *lftpattern;
	struct pattern *rgtpattern;
	char *patval;
	short patlen;
};

struct dirhdr {
	struct dirhdr *nxtopendir;
	char **ntable;
	int nents;
	int nsize;
	DIR *dirfc;
	char *dirn;
	time_t mtime;
};
extern struct dirhdr *firstod;

struct chain {
	struct chain *nextp;
	char *datap;
};
extern struct chain *rmchain;
extern struct chain *deschain;

extern char *copys(), *concat(), *subst(), *ncat(), *ckalloc();
extern struct nameblock *srchname(), *makename(), *rcsco();
extern struct varblock *varptr(), *srchvar();
extern struct lineblock *isdependent();
extern time_t exists();
