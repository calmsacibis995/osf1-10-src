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
 *	@(#)$RCSfile: ntpq.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:42:14 $
 */
/*
 */

/*
 * ntpq.h - definitions of interest to ntpq
 */

/*
 * Maximum number of arguments
 */
#define	MAXARGS	4

/*
 * xntpdc includes a command parser which could charitably be called
 * crude.  The following structure is used to define the command
 * syntax.
 */
struct xcmd {
	char *keyword;		/* command key word */
	void (*handler)();	/* command handler */
	u_char arg[MAXARGS];	/* descriptors for arguments */
	char *desc[MAXARGS];	/* descriptions for arguments */
	char *comment;
};


/*
 * Flags for forming descriptors.
 */
#define	OPT	0x80		/* this argument is optional, or'd with type */

#define	NO	0x0
#define	STR	0x1		/* string argument */
#define	UINT	0x2		/* unsigned integer */
#define	INT	0x3		/* signed integer */
#define	ADD	0x4		/* IP network address */

/*
 * Arguments are returned in a union
 */
typedef union {
	char *string;
	int ival;
	u_int uval;
	u_int netnum;
} arg_v;


/*
 * Structure for passing parsed command line
 */
struct parse {
	char *keyword;
	arg_v argval[MAXARGS];
	int nargs;
};


/*
 * Types of things we may deal with
 */
#define	TYPE_SYS	1
#define	TYPE_PEER	2
#define	TYPE_CLOCK	3


/*
 * Structure to hold association data
 */
struct association {
	u_short assid;
	u_short status;
};

#define	MAXASSOC	1024
