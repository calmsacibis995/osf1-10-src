%{
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
 * @(#)$RCSfile: config.l,v $ $Revision: 4.2.15.3 $ (DEC) $Date: 1993/10/29 21:11:47 $
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)config.l	5.2 (Berkeley) 4/18/86
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */


/*
 * Modification History
 *
 * 27-Oct-91	Fred Canter
 *		Added maxssiz, dflssiz, dfldsiz tokens for configuring
 *		stack and data size limits.
 *		Make System V IPC definitions configurable.
 *
 * 6-June-1991	Brian Stevens
 *		Added new config file options maxuprc, bufcache,
 *		maxcallouts, and maxthreads (per task).
 *
 * 3-20-91	robin-
 *		Made changes to support new device data structures.
 *
 *
 */


#include <ctype.h>
#include "y.tab.h"
#include "config.h"

#define tprintf if (do_trace) printf

extern long octal();
extern long hex();


/*
 * Key word table
 */

struct kt {
	char *kt_name;
	int kt_val;
} key_words[] = {
	{ "adapter",	CONTROLLER },
	{ "am",		ADDRMOD },		/* MIPS */
	{ "and",	AND },
	{ "anonklpages", ANONKLPAGES },
	{ "anonklshift", ANONKLSHIFT },
	{ "args",	ARGS },
	{ "at",		AT },
        { "at_start",	AT_START},              /* Callout points       */
        { "at_exit",	AT_EXIT},               /*      .               */
	{ "at_success",	AT_SUCCESS},            /*      .               */
	{ "before_h",	BEFORE_H},              /*      .               */
	{ "after_h",	AFTER_H},               /*      .               */
	{ "before_makefile",	BEFORE_MAKEFILE},/*     .               */
	{ "after_makefile",	AFTER_MAKEFILE},/*      .               */
	{ "before_c",	BEFORE_C},              /*      .               */
	{ "after_c",	AFTER_C},               /*      .               */
	{ "before_conf",	BEFORE_CONF},   /*      .               */
        { "after_conf", AFTER_CONF},            /* Last in Callout list */
	{ "bin",	BIN },			/* SQT ONLY */
	{ "bufcache",   BUFCACHE },
        { "bus",        BUS},
        { "callout",    CALLOUT},
	{ "cinode",	REMOTE_CONTROLLER },
	{ "clustermap",	CLUSTERMAP },
	{ "clustersize", CLUSTERSIZE },
	{ "config",	CONFIG },
	{ "controller",	CONTROLLER },
	{ "cowfaults",	COWFAULTS },
	{ "cpu",	CPU },
	{ "csr",	CSR },
	{ "csubmapsize", CSUBMAPSIZE },
	{ "device",	DEVICE },
        { "delete",     NOT },
	{ "dfldsiz",	DFLDSIZ },
	{ "dflssiz",	DFLSSIZ },
        { "disable",    NOT },
	{ "disk",	DISK },
	{ "drive",	DRIVE },
	{ "dst",	DST },
	{ "dumps",	DUMPS },
	{ "dynamic",	DYNAMIC },
	{ "flags",	FLAGS },
	{ "heappercent", HEAPPERCENT },
	{ "hz",		HZ },
	{ "ident",	IDENT },
	{ "ignore",	NOT },
        { "kentry_zone_size",   KENTRY_ZONE_SIZE },
	{ "lun",	LUN },			/* MMAX only */
	{ "machine",	MACHINE },
	{ "major",	MAJOR },
	{ "makeoptions", MAKEOPTIONS },
	{ "makevariables", MAKEOPTIONS },
	{ "mapentries",	MAPENTRIES },
	{ "master",	MASTER },
	{ "maxcallouts", MAXCALLOUTS },
	{ "maxdsiz",	MAXDSIZ },
	{ "maxssiz",	MAXSSIZ },
	{ "maxthreads", MAXTHREADS },
	{ "threadmax",  THREADMAX },
	{ "taskmax",    TASKMAX },
	{ "maxuprc",    MAXUPRC },
	{ "maxusers",	MAXUSERS },
	{ "nclist",	NCLIST },
	{ "maxproc",    MAXPROC },
	{ "sys_v_mode",	SYS_V_MODE },
	{ "maxvas",	MAXVAS },
	{ "maxwire",	MAXWIRE },
	{ "mscp",	MSCP },
	{ "msinode",	REMOTE_CONTROLLER },
	{ "msgmax",	MSGMAX },
	{ "msgmnb",	MSGMNB },
	{ "msgmni",	MSGMNI },
	{ "msgtql",	MSGTQL },
	{ "mba",	MBA },
	{ "mbii",	MBII },			/* MIPS */
	{ "minor",	MINOR },
	{ "nexus",	NEXUS },
	{ "node",	SLOT },
	{ "not",	NOT },
	{ "on",		ON },
	{ "options",	OPTIONS },
        { "port",       PORT },
	{ "priority",	PRIORITY },
	{ "processors",	PROCESSORS },
	{ "pseudo-device",PSEUDO_DEVICE },
	{ "readio_kluster", READIO_KLUSTER},
	{ "readonly",	READONLY },
	{ "release",	RELEASE },
	{ "root",	ROOT },
	{ "scs_sysid",	SCSID },
        { "scsiid",     REMOTE_CONTROLLER },
	{ "segmentation", SEGMENTATION},
	{ "semmni",	SEMMNI },
	{ "semmns",	SEMMNS },
	{ "semmsl",	SEMMSL },
	{ "semopm",	SEMOPM },
	{ "semume",	SEMUME },
	{ "semvmx",	SEMVMX },
	{ "semaem",	SEMAEM },
	{ "shmmin",	SHMMIN },
	{ "shmmax",	SHMMAX },
	{ "shmmni",	SHMMNI },
	{ "shmseg",	SHMSEG },
        { "size",	SIZE },
	{ "slave",	SLAVE },
	{ "slot",	SLOT },			/* MMAX only */
	{ "swap",	SWAP },
	{ "swapbuffers", SWAPBUFFERS },
	{ "syswiredpercent", SYSWIREDPERCENT },
	{ "tape",	TAPE },
	{ "timezone",	TIMEZONE },
	{ "trace",	TRACE },
	{ "uba",	UBA },
	{ "ubcbuffers", UBCBUFFERS },
	{ "ubcdirtypercent", UBCDIRTYPERCENT },
	{ "ubcminpercent", UBCMINPERCENT},
	{ "ubcmaxpercent", UBCMAXPERCENT},
	{ "ubcpagesteal", UBCPAGESTEAL },
	{ "ubcseqpercent", UBCSEQPERCENT },
	{ "ubcseqstartpercent", UBCSEQSTARTPERCENT },
        { "unit",       UNIT },
        { "vaxbi",      VAXBI },
        { "vba",        VBA },
	{ "vector",	VECTOR },
        { "version",    VERSION },
 	{ "vme",	VME },			/* MIPS */
	{ "vpagemax",	VPAGEMAX },
	{ "writeio_kluster", WRITEIO_KLUSTER},
 	{ "writeonly",	WRITEONLY },
        { "zone_size",  ZONE_SIZE },
	{ 0, 0 },
};
%}
WORD	([A-Za-z_][-A-Za-z_]*|[A-Z][-A-Za-z_0-9]*)
%%
{WORD}		{
			int i;

			if ((i = kw_lookup(yytext)) == -1)
			{
				yylval.str = (char *)yytext;
				tprintf("id(%s) ", yytext);
				return ID;
			}
			tprintf("(%s) ", yytext);
			return i;
		}
\"[^"]+\"	{
			yytext[strlen(yytext)-1] = '\0';
			yylval.str = (char *)yytext + 1;
			return ID;
		}
0[0-7]*		{
			yylval.val = octal(yytext);
			tprintf("#O:%lo ", yylval.val);
			return NUMBER;
		}
0x[0-9a-fA-F]+	{
			yylval.val = hex(yytext);
			tprintf("#X:%lx ", yylval.val);
			return NUMBER;
		}
[1-9][0-9]*	{
			yylval.val = atol(yytext);
			tprintf("#D:%ld ", yylval.val);
			return NUMBER;
		}
[0-9]"."[0-9]*	{
			double atof();
			yylval.fval = atof(yytext);
			return FPNUMBER;
		}
"-"		{
			return MINUS;
		}
"?"		{
			yylval.val = -1;
			tprintf("? ");
			return NUMBER;
		}
\n/[ \t]	{
			yyline++;
			tprintf("\n... ");
		}
\n		{
			yyline++;
			tprintf("\n");
			return SEMICOLON;
		}
#.*		{	/* Ignored (comment) */;	}
[ \t]*		{	/* Ignored (white space) */;	}
";"		{	return SEMICOLON;		}
","		{	return COMMA;			}
"="		{	return EQUALS;			}
"@"		{	return AT;			}
.		{	return yytext[0];		}

vme16           {       return VME16D16;                }
vme24           {       return VME24D16;                }
vme32           {       return VME32D32;                }

vme16d16        {       return VME16D16;                }
vme24d16        {       return VME24D16;                }
vme32d16        {       return VME32D16;                }
vme16d32        {       return VME16D32;                }
vme24d32        {       return VME24D32;                }
vme32d32        {       return VME32D32;                }

%%
/*
 * kw_lookup
 *	Look up a string in the keyword table.  Returns a -1 if the
 *	string is not a keyword otherwise it returns the keyword number
 */

kw_lookup(word)
register char *word;
{
	register struct kt *kp;

	for (kp = key_words; kp->kt_name != 0; kp++)
		if (eq(word, kp->kt_name))
			return kp->kt_val;
	return -1;
}

/*
 * Number conversion routines
 */

long
octal(str)
char *str;
{
	long num;

	(void) sscanf(str, "%lo", &num);
	return num;
}

long
hex(str)
char *str;
{
	long num;

	(void) sscanf(str+2, "%lx", &num);
	return num;
}
