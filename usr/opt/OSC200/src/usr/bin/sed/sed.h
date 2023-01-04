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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: sed.h,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/06/10 15:01:33 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 * COMPONENT_NAME: (CMDEDIT) sed.h
 *
 * FUNCTIONS: none
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1984, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1975 Bell Telephone Laboratories, Incorporated
 *
 * 1.8 com/cmd/edit/sed.h, cmdedit, bos320, 9123320e 6/5/91 23:35:13
 */

#include	<stdlib.h>
#include 	<stdio.h>
#include 	<regex.h>

/*
 *	Define variables and macros for message catalog.
 */
#include	<locale.h>
#include 	"sed_msg.h"
extern	nl_catd	catd;
#define	MSGSTR(num,str)	catgets(catd, MS_SED, num, str)
#define	exiting(n)	{ catclose(catd); exit(n); }

extern char    CGMES[];
extern char    MALLOCMES[];

#define CEND    16
#define CLNUM   14

#define NLINES  256
#define DEPTH   20
#define PTRSIZE 100
#define RESIZE  25000 
#define ABUFSIZE        20
#define LBSIZE  4000
#define ESIZE   256
#define LABSIZE 50
#define GLOBAL_SUB -1	/* global substitution */

extern union reptr     *abuf[];
extern union reptr **aptr;

/* linebuf start, end, and size */
extern char    *linebuf;
extern char    *lbend;
extern int     lsize;

/* holdsp start, end, and size */
extern char    *holdsp;
extern char    *hend;
extern int     hsize;
extern char    *hspend;

/* gen buf start, end, and size */
extern char    *genbuf;
extern char    *gend;
extern int     gsize;

extern long    lnum;
extern char    *spend;
extern int     nflag;
extern long    tlno[];

/*
 *	Define command flags.
 */
#define ACOM    01
#define BCOM    020
#define CCOM    02
#define CDCOM   025
#define CNCOM   022
#define COCOM   017
#define CPCOM   023
#define DCOM    03
#define ECOM    015
#define EQCOM   013
#define FCOM    016
#define GCOM    027
#define CGCOM   030
#define HCOM    031
#define CHCOM   032
#define ICOM    04
#define LCOM    05
#define NCOM    012
#define PCOM    010
#define QCOM    011
#define RCOM    06
#define SCOM    07
#define TCOM    021
#define WCOM    014
#define CWCOM   024
#define YCOM    026
#define XCOM    033

/*
 *	Define some error conditions.
 */
#define REEMPTY     01    /* An empty regular expression */
#define NOADDR      02    /* No address field in command */
#define BADCMD      03    /* Fatal error !! */
#define MORESPACE   04    /* Need to increase size of buffer */

/*
 *	Define types an address can take.
 */
#define STRA	10	/* string */
#define REGA	20	/* regular expression */

/*
 *	Structure to hold address information.
 */
struct 	addr {
	int	afl;		/* STRA or REGA */
	union	adbuf {
		char	*str;
		regex_t *re;
	} ad;
};

/*
 *	Structure to hold sed commands.
 */
union   reptr {
	struct {
		struct addr    *ad1;
		struct addr    *ad2;
		struct addr    *re1;
		char    *rhs;
		wchar_t *ytxt;
		FILE    *fcode;
		char    command;
		short   gfl;
		char    pfl;
		char    inar;
		char    negfl;
	} r1;
	struct {
		struct addr    *ad1;
		struct addr    *ad2;
		union reptr     *lb1;
		char    *rhs;
		wchar_t *ytxt;
		FILE    *fcode;
		char    command;
		short   gfl;
		char    pfl;
		char    inar;
		char    negfl;
	} r2;
};
extern union reptr ptrspace[];

struct label {
	char    asc[9];
	union reptr     *chain;
	union reptr     *address;
};
extern int     eargc;

extern union reptr     *pending;
extern char	*badp;

void 	execute(char *);
void	growbuff(int *, char **, char **, char **);
