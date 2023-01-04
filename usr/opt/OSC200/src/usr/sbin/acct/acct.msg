 
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$
$ HISTORY
 
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$
$ @(#)$RCSfile: acct.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:59:24 $
$
$ RESTRICTED RIGHTS LEGEND
$ Use, Duplication or Disclosure by the Government is subject to
$ restrictions as set forth in paragraph (b)(3)(B) of the rights in
$ Technical Data and Computer Software clause in DAR 7-104.9(a).
$
$ acct.msg	1.4  com/cmd/acct,3.1,8943 10/24/89 10:48:51
$
$ COMPONENT_NAME: (CMDACCT) Command Accounting
$
$ FUNCTIONS: none
$
$ ORIGINS: 9,27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set 1
1 "can't find login name "
2 "charge invalid: "
3 "ckpacct: %s still low on space (%s blks); acctg still off\n"
4 "ckpacct: %s too low on space (%s blks); turning acctg off\n"
8 "ckpacct: %s free space restored; turning acctg on\n"
9  "Usage: dodisk [ -o ] [ File ... ]"
10 "is not a directory -- ignored\n"
11 "dodisk: No data\n"
12 "MAJ/MIN			CONNECT SECONDS	START TIME	SESSION START\n"
13 "DEVICE	UID	LOGIN	PRIME	NPRIME	(NUMERIC)	DATE	TIME\n"
14 "SESSIONS, SORTED BY ENDING TIME"
$  	The above are headings for the prctmp report; keep columns the same
15 "Usage: prdaily [[-l] [mmdd]] | [-c]\n"
16 "Cannot print command exception reports except for %s\n" 
17 "DAILY REPORT FOR %s\n"
18 "DAILY USAGE REPORT FOR %s\n"
19 "DAILY COMMAND SUMMARY\n"
20 "MONTHLY TOTAL COMMAND SUMMARY\n"
21 "LAST LOGIN\n"
22 "Usage: prtacct [-f Specification ] [-v] File ['Heading']\n"
23 "\r\n************ ACCT ERRORS : see %s%s********\r\n"
24 "\r\n*********** 2 CRONS or ACCT PROBLEMS***********\r\n"
25 "ERROR: locks found, run aborted\n" 
26 "runacct: Insufficient space on /dev/%1$s (%2$s blks); Terminating procedure\n"
27 "ERROR: acctg already run for %1$s : check %2$s \n"
28 "\r\n**********  SYSTEM ACCOUNTING STARTED %s **********\r\n" 
29 "restarting acctg for %1$s at %2$s\n"
30 "\r\n********** SYSTEM ACCOUNTING RESTARTED %s **********\r\n" 
31 "previous state was %s\n"
32 "ERROR: runacct called with invalid arguments\n"
33 "ERROR: turnacct switch returned rc=%s\n"
34 "ERROR: S%s.%s already exists\n" 
35 "file setups probably already run\n"
36 "ERROR: S%s.%s already exists: run setup manually\n"
37 "files setups complete\n"
38  "ERROR: wtmpfix errors see %s/wtmperror%s\n" 
39 "wtmp processing complete\n"
40 "connect acctg complete\n"
41 "WARNING: accounting already run for %s\n" 
42 "WARNING: remove %s/%s to rerun\n" 
43 "process acctg complete for %s\n" 
44 "all process acctg complete for %s\n" 
45 "tacct merge to create daytacct complete\n"
46 "merged fees\n"
47 "no fees\n"
48 "merged disk records\n"
49 "no disk records\n"
50 "merged queueing system records\n"
51 "no queueing system records\n"
52 "WARNING: recreating %s/sum/tacct \n"
53 "updated sum/tacct\n"
54 "WARNING: recreating %s/sum/cms \n" 
55 "command summaries complete\n"
56 "system accounting completed at %s\n" 
57 "********** SYSTEM ACCOUNTING COMPLETED %s **********" 
58 "ERROR: invalid state, check "
63 "acctg on"
64 "Usage: turnacct on|off|switch\n"
65 "Usage: chargefee User Number\n"
66 "TOTAL COMMAND SUMMARY FOR FISCAL"
67 "LAST LOGIN"
68 "acctg off"
$
$quote "
$set MS_ACCT
NOMEM		"%s: Cannot allocate memory\n"
CANTOPEN 	"%1$s: Cannot open %2$s\n"
USAGECMS	"Usage: %s [-acjnspot] [file ...]\n"
HASHOVR 	"acctcms: Hash table overflow. Increase CSIZE\n"
THEADER1 	"COMMAND   NUMBER      TOTAL       TOTAL       TOTAL   MEAN     MEAN     HOG      CHARS        BLOCKS\n"
THEADER2	"NAME        CMDS    KCOREMIN     CPU-MIN     REAL-MIN SIZE-K  CPU-MIN  FACTOR   TRNSFD         READ\n"
TOTALS		"TOTALS"
TPR_OUT1	"%1$-8.8s%2$8ld%3$12.2f%4$11.2f%5$13.2f%6$7.2f%7$8.2f "
TPR_OUT2	"%1$8.2f%2$12ld%3$12ld\n"
HDP		"                                PRIME/NON-PRIME TIME COMMAND SUMMARY\n"
HDNONPRIME	"                                  NON-PRIME TIME COMMAND SUMMARY\n"
HDTOT		"                                     TOTAL COMMAND SUMMARY\n"
HDPRIME		"                                   PRIME TIME COMMAND SUMMARY\n"
HD1		"COMMAND   NUMBER      TOTAL       TOTAL       TOTAL   MEAN    MEAN     HOG         CHARS     BLOCKS\n"
HD2		"NAME        CMDS    KCOREMIN     CPU-MIN   REAL-MIN  SIZE-K  CPU-MIN  FACTOR      TRNSFD      READ\n"
HD3		"COMMAND        NUMBER         TOTAL          CPU-MIN                 REAL-MIN        MEAN    MEAN      HOG       CHARS       BLOCKS\n"
HD4		"NAME         (P)    (NP)   KCOREMIN       (P)      (NP)          (P)         (NP)  SIZE-K  CPU-MIN   FACTOR     TRNSFD        READ\n"
PF_FORMAT	" %8.2lf"
PS_FORMAT	" %8.8s"
GUNKNOWN	"Unknown group"
UUNKNOWN	"%1$s: Unknown user %2$s\n"
OPENERR		"open error on output file"
ARFHD		"\nACCOUNTING RECORDS FROM:  %s"
STAFT		"START AFTER : %s"
STBEF		"START BEFORE: %s"
ENDAFT		"END AFTER   : %s"
ENDBEF		"END BEFORE  : %s"
CMSHED1		"COMMAND                      START    END          REAL"
SECSSTR		"(SECS)"
CHARSTR		"CHARS"
BLOCKSTR	"BLOCKS"
CPUSTR		"CPU"
HOGSTR		"HOG"
MEANSTR		"MEAN"
KDCORESTR	"KCORE"
CMSHED2		"NAME       USER     TTYNAME  TIME     TIME       (SECS)"
SYSSTR		"SYS"
USERSTR		"USER"
TRNSFDSTR	"TRNSFD"
READSTR		"READ"
FACTORSTR	"FACTOR"
SIZEKSTR	"SIZE(K)"
MINSTR		"MIN"
FSTATSTR	"  F STAT"
BADTIME		"acctcom: bad time:"
PATRNERR	"pattern syntax"
COM_OUT1 	"\ncmds=%1$ld Real=%2$-6.2f CPU=%3$-6.2f USER=%4$-6.2f SYS=%5$-6.2f "
COM_OUT2	"CHAR=%1$-8.2f BLK=%2$-8.2f USR/TOT=%3$-4.2f HOG=%4$-4.2f \n"
NOCMDS		"\nNo commands matched\n"
NOSTATIN	"acctcom: cannot stat stdin\n"
NOSTATNULL	"acctcom: cannot stat /dev/null\n"
COMUSAGE1	"Usage: acctcom [options] [files]\n"
COMUSAGE2	"\nWhere options can be:\n"
COMUSAGE3	"-b	read backwards through file"
COMUSAGE4	"-f	print the fork/exec flag and exit status"
COMUSAGE5	"-h	print hog factor (total-CPU-time/elapsed-time)"
COMUSAGE6	"-i	print I/O counts"
COMUSAGE7	"-k	show total Kcore minutes instead of memory size"
COMUSAGE8	"-m	show mean memory size"
COMUSAGE9	"-r	show CPU factor (user-time/(sys-time + user-time))"
COMUSAGE10	"-t	show separate system and user CPU times"
COMUSAGE11	"-v	don't print column headings"
COMUSAGE12	"-a	print average statistics of selected commands"
COMUSAGE13	"-q	print average statistics only"
COMUSAGE14	"-l line	\tshow processes belonging to terminal /dev/line"
COMUSAGE15	"-u user	\tshow processes belonging to user name or user ID"
COMUSAGE16	"-u #	\tshow processes executed by super-user"
COMUSAGE17	"-u ?	\tshow processes executed by unknown UID's"
COMUSAGE18	"-g group	show processes belonging to group name of group ID"
COMUSAGE19	"-s time	\tshow processes ending after time (hh[:mm[:ss]])"
COMUSAGE20	"-e time	\tshow processes starting before time"
COMUSAGE21	"-S time	\tshow processes starting after time"
COMUSAGE22	"-E time	\tshow processes ending before time"
COMUSAGE23	"-n regex	select commands matching the ed(1) regular expression"
COMUSAGE24	"-o file	\tdo not print, put selected pacct records into file"
COMUSAGE25	"-H factor	show processes that exceed hog factor"
COMUSAGE26	"-O sec	\tshow processes that exceed CPU system time sec"
COMUSAGE27	"-C sec	\tshow processes that exceed total CPU time sec"
COMUSAGE28	"-I chars	show processes that transfer more than char chars"
REG_ERROR	"acctcom: Error %d in compiling reg. expression\n"
BADWTMP		"bad wtmp: offset %lu.\n"
WTMPREC		"bad record is:  %1$.*s\t%2$.*s\t%3$lu"
CON1BADTYPE	"acctcon1: invalid type %1$d for %2$.*s %3$.*s %4$s"
CON1TOOSMALL	"acctcon1: recompile with larger NSYS\n"
BADTIMEOLD	"%1$s: bad times: old: %2$s"
BADTIMENEW	"new: %s"
CON1FROM	"from %s"
CON1TO		"to   %s"
DATECHGS	"%d\tdate changes\n"
DATECHG		"%d\tdate change\n"
CON1TOT1	"TOTAL DURATION: %.0f MINUTES\n\n"
CON1TOT2	"LINE\tMINUTES\tPERCENT\t# SESS\t# ON\t# OFF\n"
CON1TOT3	"TOTALS\t%1$.0f\t--\t%2$d\t%3$d\t%4$d\n"
INVALARG	"Invalid argument: %s\n"
NOCORE		"out of core\n"
BADSPEC		"%1$s: bad fieldspec `%2$s'\n"
ONESPEC		"%s: only one input fieldspec allowed\n"
TOOMANYFILES	"%1$s: >%2$d files\n"
ACCTUID		"UID"
ACCTLOGNAME	"LOGNAME"
ACCTPRI_CPU	"PRI_CPU"
ACCTNPRI_CPU	"NPRI_CPU"
ACCTPRI_MEM	"PRI_MEM"
ACCTNPRI_MEM	"NPRI_MEM"
ACCTPRI_RD_WR	"PRI_RD/WR"
ACCTNPRI_RD_WR	"NPRI_RD/WR"
ACCTPRI_BLKIO	"PRI_BLKIO"
ACCTNPRI_BLKIO	"NPRI_BLKIO"
ACCTPRI_CONNECT	"PRI_CONNECT"
ACCTNPRI_CONNECT	"NPRI_CONNECT"
ACCTDSK_BLOCKS	"DSK_BLOCKS"
ACCTPRINT	"PRINT"
ACCTFEES	"FEES"
ACCTPROCESSES	"PROCESSES"
ACCTSESS	"SESS"
ACCTDSAMPS	"DSAMPS"
ACCTBUSY	"accounting is busy: cannot turn accounting ON"
ACCTDENY	"%s: permission denied - UID must be root/adm\n"
ACCTCANTON	"cannot turn accounting ON"
ACCTNOOFF	"cannot turn accounting OFF"
PNPHOLTBLERR	"pnpsplit: holidays table setup failed\n"
UPDTHOL		"***UPDATE %s WITH NEW HOLIDAYS***\n"
BADYEARCONV	"%s: bad {yr ptime nptime} conversion\n"
BADYEAR		"pnpsplit: invalid year: %d\n"
PNPINVAL	"pnpsplit: invalid p/np hours\n"
PNPHOL1		"pnpsplit: too many holidays, "
PNPHOL2		"recompile with larger NHOLIDAYS\n"
PNPBADAY	"pnpsplit: invalid day of year %d\n"
NOPWENT		"No entry for %s in password file.\n" 
BADUIDGID	"%1$s: uid/gid not %2$s\n"
BADMODE		"file mode not 0664"
CANTREAD	"%1$s: Can't read %2$s\n"
OUTOFMEM	"%1$s: Out of memory -- %2$s not used\n"
USAGEWTMP	"Usage: %1$s 'Reason'\n"
DUSAGE		"Usage: %s [-U number]  [-sv] [-p pw_file] [-u ufile] [-i ignlist] [file ...]\n"));
BADUVALUE     	"%s: Value for -U must be > 0\n"
FSINVAL		"%1$s: Invalid filesystem type %2$s\n"
NOTREG		"%1$s: %2$s is not a regular file -- ignored\n" 
NOTSPECL	"%1$s: %2$s is not a special file -- ignored\n"
CANTSTAT	"%1$s: Cannot stat %2$s\n"
BADUID		"BAD UID: file system = %1$s, inode = %2$u, uid = %3$u\n"
IGNOVR		"%s: ignore list overflow. Recompile with larger MAXIGN\n"
IOERROR		"%1$s: IO error. off=%2$d, cnt=%3$d, seek=%4$d, read=%5$d\n"
USERLIM		"%1$s: User limit reached, use -U # flag > %2$d\n"
CANTMKTMP	"cannot make temporary: %s"
LOGCHG		"wtmpfix: logname \"%*.*s\" changed to \"INVALID\"\n"
INTRUC		"Input truncated at offset %ld"
NEWDTEXPTED	"New date expected at offset %ld"
CATREADTMP	"Cannot read from temp: %s"
BADFILOFF	"Bad file at offset %ld\n"
PWUSAGE		"Usage: %s [-acdgsu]\n"
