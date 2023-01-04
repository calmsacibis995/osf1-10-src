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
static char	*sccsid = "@(#)$RCSfile: ps.c,v $ $Revision: 4.2.9.6 $ (DEC) $Date: 1993/11/19 16:12:36 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Copyright (c) 1989 The Regents of the University of California.
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
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/* 
 * Mach Operating System
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */


#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif


#include <sys/version.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <strings.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <mach.h>
#include <mach_init.h>

#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/table.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/errno.h>

#include <machine/vmparam.h>

#include <ctype.h>
#include <locale.h>

#include "ps_msg.h"

nl_catd catd;

#define MSGSTR(Num,Str) catgets(catd,MS_PS,Num,Str)

#undef NZERO
/*..rjg.. ORIGINAL code simply def'd NZERO = 12 ..
 */ 
#define RT 27
#if RT  
#define NZERO 44	/* Adjust for number of queues */
#else RT
#define NZERO 12
#endif RT

#ifndef	TH_USAGE_SCALE
#define	TH_USAGE_SCALE	1000
#endif	TH_USAGE_SCALE
#define usage_to_percent(u)	((u*100)/TH_USAGE_SCALE)
#define usage_to_tenths(u)	(((u*1000)/TH_USAGE_SCALE) % 10)

#define KERNBASE	0xff000000

#define FMTSEP		" \t,\n"

int	termwidth;
int	termheight;
#define UNLIMITED       0
int	totwidth;
int	sumrusage;
int	cmdstart;

#define	add_pid(p)	add_int((p), &pids_to_check, &num_pids_to_check)
#define check_pid(p)	check_int((p), pids_to_check, num_pids_to_check)
int     *pids_to_check;
int     num_pids_to_check;

#define	add_rgid(p)	add_int((p), &rgids_to_check, &num_rgids_to_check)
#define check_rgid(p)	check_int((p), rgids_to_check, num_rgids_to_check)
int     *rgids_to_check;
int     num_rgids_to_check;

#define	add_pgrp(p)	add_int((p), &pgrps_to_check, &num_pgrps_to_check)
#define check_pgrp(p)	check_int((p), pgrps_to_check, num_pgrps_to_check)
int     *pgrps_to_check;
int     num_pgrps_to_check;

#define	add_sess(p)	add_int((p), &sess_to_check, &num_sess_to_check)
#define check_sess(p)	check_int((p), sess_to_check, num_sess_to_check)
int     *sess_to_check;
int     num_sess_to_check;

#define	add_uid(p)	add_int((p), &uids_to_check, &num_uids_to_check)
#define check_uid(p)	check_int((p), uids_to_check, num_uids_to_check)
int     *uids_to_check;
int     num_uids_to_check;

#define	add_ruid(p)	add_int((p), &ruids_to_check, &num_ruids_to_check)
#define check_ruid(p)	check_int((p), ruids_to_check, num_ruids_to_check)
int     *ruids_to_check;
int     num_ruids_to_check;

#define	add_tty(p)	add_int((p), &ttys_to_check, &num_ttys_to_check)
#define check_tty(p)	check_int((p), ttys_to_check, num_ttys_to_check)
int     *ttys_to_check;
int     num_ttys_to_check;

int	aflg = 0, eflg = 0, Eflg = 0, xflg = 0, mflg = 0, gflg = 0;
int	rflg = 0;

int	prtheader;
int	lineno;

/*
 * flags to retrieve extra info
 */
int	needuser = 0, needcomm = 0;

/*
 * sorting order
 */
int	sortby;
#define	SORTMEM	1
#define	SORTRUN 2
#define	SORTCPU 3

/* UID type defines (local use only) 
 *
*/
#define ADD_UID  1
#define ADD_RUID 2

/*
 * Options and usage
 */

char *bsd_usage =	"Usage: %s [%s] [%s format] [%stty] [process number]\n";
#define BSD_USAGE_OPTS  " For BSD Compatible Syntax\nps", "aexgmlvusjrSUALwhT", "Oo", "t"
char *bsd_options = 	"?aexgmlvusjrSUALwho:O:t:Tp:";
char *sysv_usage =	"Usage: %s [%s] [%s format] [%s tlist] [%s plist] [%s ulist] [%s ulist] [%s glist] [%s glist] [%s slist] [%s nlist]\n";
#define SYSV_USAGE_OPTS  "\nps","-Aedamjflr", "-oO", "-t", "-p", "-u", "-U", "-g", "-G", "-s", "-n"
char *sysv_options =	"?Aeamdlfjrn:t:p:u:U:g:G:s:o:O:";
 
#if 0 
/*
*FYI - a few points that may be helpful to you.

 A QAR indicated that when a "ps" was performed the ps command
did not show up in the list of processes being run by the
user. Another QAR indicated that a mail process was running
amok and the user couldn't find its' PID via "ps" in order
to kill it.

 It was found  that system processes which have the
setuid bit (as one example) set WILL NOT show up in a
simple "ps" command or even "ps ax" with your name on it
unless you use the  -o  or -O  options of ps.  -O is the same as
-o except that it displays the fields pid, state, tname, cputime,
and comm by default in addition to the specifiers supplied
on the command line.

It seems that in OSF, the default search and find proc logic
sees these processes (mail, ps, etc) as belonging to root
(or whoever created the disk proc image).

When you look at the man page for ps,  the fields UID, RUID,
USER, and RUSER need clarification.

UID in the OSF context refers to the UID of the OWNER of the
process as defined by the file system. RUID on the other hand
is the UID of the user who is Really Running the command. USER
and RUSER are also similarly defined.

As an example of how to find ALL/ANY process running on your
behalf in OSF you have to us a "ps" command of the following
construct: 

		ps ax -O ruid,ruser,uid,user

Which produces:

  PID  RUID RUSER      UID USER     STAT TT       TIME COMMAND
  963     0 root       293 glaser   TWN  p1    0:00.28 /usr/users/glaser/sbell
  954  8887 nerk       293 glaser   TWN  p2    0:00.28 /usr/users/glaser/sbell
 1154   294 meatball   294 meatball TWN  p4    0:00.23 /usr/users/glaser/nsbell
  915   293 glaser     293 glaser   TWN  p5    0:00.49 vi sbell.c
  961   293 glaser     293 glaser   TWN  p5    0:00.18 sbell
 1163   293 glaser       0 root     R N  p5    0:00.74 ps axw -O ruid,ruser,uid,user


n.b. The ordering of RUID,RUSER,UID,USER in the output
follows that given by the -O switch and is not hardwired to any
specific sequence.


The example process ~glaser/sbell has the SETUID bit set (mode -rwsr-xr-x) while
example process ~glaser/nsbell does not have the SETUID bit on (mode -rwxr-xr-x).

>From the above we see that users  root, nerk, and glaser are running command
~glaser/sbell which is owned by user glaser.

It is interesting to NOTE that user meatball is running the command ~glaser/nsbell
and that the command is NOT setuid and is OWNED by user glaser. From the output of
the "ps" command it would -APPEAR- that user meatball OWNS command ~glaser/nsbell.
*
*/
#endif 0

/*
 * collected proc info
 */
typedef	struct proc_info	*proc_info_t;
struct	proc_info {
	uid_t			uid;
        uid_t		        ruid;
        uid_t		        svuid;
        gid_t		        rgid;
        gid_t		        svgid;
	pid_t			pid;
	pid_t			ppid;
	pid_t			pgrp;
        pid_t                   session;
        pid_t			tpgrp;
        pid_t                   tsession;
        int                     jobc;
	int			status;
	int			flag;
	int			state;
	int			pri;
	int			base_pri;
        char                    p_cursig;
        int                     p_sig;
        int                     p_sigmask;
        int                     p_sigignore;
        int                     p_sigcatch;
	boolean_t		all_swapped;
	time_value_t		p_utime;
        time_value_t		p_stime;
	vm_size_t		virtual_size;
	vm_size_t		resident_size;
	float			percent_mem;
	int			cpu_usage;
        dev_t                   tty;
	char			*command;
        char                    *args;
        long                    slptime;
        long                    wait_chan;
        char                    wait_mesg[WMESGLEN+1];
        struct user             *u;
        int                     suspend_count;
	int			num_threads;
	proc_info_t		threads;	/* array */
};

proc_info_t			proc_table = (proc_info_t)0;
int				max_proc_table, nprocs;

/*
 * the following describes the possible fields that can be printed
 */
enum type	{ CHAR, UCHAR, SHORT, USHORT, INT, UINT, LONG, ULONG, KPTR };

#define	UIDFMT	"u"
#define UIDLEN	5
#define PIDFMT	"d"
#define PIDLEN	5
#define	USERLEN	8

#define POFF(x)         ((int)&((proc_info_t)0)->x)
#define UOFF(x)         ((int)&((struct user *)0)->x)
#define	ROFF(x)		((int)&((struct rusage *)0)->x)

typedef struct var      *var_t;
struct var {
	char	*name[8];	/* name(s) of variable */
        int	header_msg;     /* message number for title */
	char	*header;	/* default header */
	int	flag;
#define	USER	0x01	/* requires user structure */
#define	LJUST	0x02	/* right adjust on output */
#define	COMM	0x04	/* requires exec arguments and environment (XXX) */
#define THREAD	0x08	/* print this field for threads */
	int	(*oproc)();	/* output routine */
	short	width;		/* printing width */
	/*
	 * The following (optional) elements are hooks for passing information
	 * to the generic output routine pvar
	 */
	int	off;		/* offset in structure */
	enum	type type;	/* type of element */
	char	*fmt;		/* printf format */
	/*
	 * glue to link selected fields together
	 */
	struct	var *next;
};

void	command(proc_info_t, var_t);
void	ucomm(proc_info_t, var_t);
void	logname(proc_info_t, var_t);
void	state(proc_info_t, var_t);
void	psxpri(proc_info_t, var_t);
void	uname(proc_info_t, var_t);
void	runame(proc_info_t, var_t);
void	gname(proc_info_t, var_t);
void	rgname(proc_info_t, var_t);
void	tdev(proc_info_t, var_t);
void	tname(proc_info_t, var_t);
void	longtname(proc_info_t, var_t);
void	started( proc_info_t, var_t);
void	lstarted( proc_info_t, var_t);
void	etime( proc_info_t, var_t);
void	wchan(proc_info_t, var_t);
void	vsize(proc_info_t, var_t);
void	rsize(proc_info_t, var_t);
void	cputime(proc_info_t, var_t);
void	usertime(proc_info_t, var_t);
void	systime(proc_info_t, var_t);
void	pcpu(proc_info_t, var_t);
void	pmem(proc_info_t, var_t);
void	maxrss(proc_info_t, var_t);
void	pagein(proc_info_t, var_t);
void	pnice(proc_info_t, var_t);
void	pvar(proc_info_t, var_t);
void	uvar(proc_info_t, var_t);
void	rvar(proc_info_t, var_t);
void	empty(proc_info_t, var_t);

struct var var[] = {
	{{"command", "args"},
                 COMMAND_MSG, "COMMAND", LJUST|COMM, command, 16}, 
	{{"ucomm", "comm"},
                 COMMAND_MSG, "COMMAND", LJUST, ucomm, MAXCOMLEN},
	{{"logname"},
                 LOGNAME_MSG, "LOGNAME", USER|LJUST, logname, MAXLOGNAME},
	{{"flag", "f"},
                 F_MSG, "F", 0, pvar, 8, POFF(flag), INT, "x"},
	{{"status"},
                 STATUS_MSG, "STATUS", 0, pvar, 8, POFF(status), INT, "d"},
	{{"uid"},
                 UID_MSG, "UID", 0, pvar, UIDLEN, POFF(uid),USHORT, UIDFMT},
	{{"ruid"},
                 RUID_MSG, "RUID", 0, pvar, UIDLEN, POFF(ruid), USHORT, UIDFMT},
	{{"svuid"},
                 SVUID_MSG, "SVUID", 0, pvar, UIDLEN, POFF(svuid), USHORT, UIDFMT},
	{{"rgid"},
                 RGID_MSG, "RGID", 0, pvar, UIDLEN, POFF(rgid), USHORT, UIDFMT},
	{{"svgid"},
                 SVGID_MSG, "SVGID", 0, pvar, UIDLEN, POFF(svgid), USHORT, UIDFMT},
#ifdef	__alpha
	{{"pid"},
                 PID_MSG, "PID", 0, pvar, PIDLEN, POFF(pid), INT, PIDFMT},
	{{"ppid"},
                 PPID_MSG, "PPID", 0, pvar, PIDLEN, POFF(ppid), INT, PIDFMT},
#else
	{{"pid"},
                 PID_MSG, "PID", 0, pvar, PIDLEN, POFF(pid), LONG, PIDFMT},
	{{"ppid"},
                 PPID_MSG, "PPID", 0, pvar, PIDLEN, POFF(ppid), LONG, PIDFMT},
#endif
	{{"cp", "cpu"},
                 CP_MSG, "CP", THREAD, pvar, 3, POFF(cpu_usage), UCHAR, "d"},
        {{"nwchan"},
                 WCHAN_MSG, "WCHAN", 0, pvar, 6, POFF(wait_chan), KPTR, "x"},
	{{"wchan"},
                 WCHAN_MSG, "WCHAN", LJUST|THREAD, wchan, 8},
        {{"cursig"},
                 CURSIG_MSG, "CURSIG", 0, pvar, 2, POFF(p_cursig), CHAR, "d"},
	{{"sig", "pending"},
                 PENDING_MSG, "PENDING", 0, pvar, 8, POFF(p_sig), INT, "x"},
	{{"sigmask", "blocked"},
                 BLOCKED_MSG, "BLOCKED", 0, pvar, 8, POFF(p_sigmask), INT, "x"},
	{{"sigignore", "ignored"},
                 IGNORED_MSG, "IGNORED", 0, pvar, 8, POFF(p_sigignore), INT, "x"},
	{{"sigcatch", "caught"},
                 CAUGHT_MSG, "CAUGHT", 0, pvar, 8, POFF(p_sigcatch), INT, "x"},
	{{"user", "uname"},
                 USER_MSG, "USER", LJUST, uname, USERLEN},
	{{"ruser", "runame"},
                 RUSER_MSG, "RUSER", LJUST, runame, USERLEN},
	{{"group"},
                 GROUP_MSG, "GROUP", LJUST, gname, USERLEN},
	{{"rgroup"},
                 RGROUP_MSG, "RGROUP", LJUST, rgname, USERLEN},
#ifdef	__alpha
	{{"pgid"},
                 PGID_MSG, "PGID", 0, pvar, PIDLEN, POFF(pgrp), INT, PIDFMT},
	{{"sess", "session"},
                 SESS_MSG, "SESS", 0, pvar, PIDLEN, POFF(session), INT, PIDFMT},
#else
	{{"pgid"},
                 PGID_MSG, "PGID", 0, pvar, PIDLEN, POFF(pgrp), LONG, PIDFMT},
	{{"sess", "session"},
                 SESS_MSG, "SESS", 0, pvar, PIDLEN, POFF(session), LONG, PIDFMT},
#endif
	{{"jobc"},
                 JOBC_MSG, "JOBC", 0, pvar, 4, POFF(jobc), INT, "d"},
	{{"tdev", "dev"},
                 TDEV_MSG, "TDEV", 0, tdev, 4},
	{{"tname", "tty", "tt"},
                 TT_MSG, "TT", LJUST, tname, 3},
	{{"longtname", "longtty"},
                 TT_MSG, "TT", LJUST, longtname, 8},
#ifdef	__alpha
	{{"tpgid"},
                 TPGID_MSG, "TPGID", 0, pvar, PIDLEN, POFF(tpgrp), INT, PIDFMT},
	{{"tsession", "tsess"},
                 TSESS_MSG, "TSESS", 0, pvar, PIDLEN, POFF(tsession), INT, PIDFMT},
#else
	{{"tpgid"},
                 TPGID_MSG, "TPGID", 0, pvar, PIDLEN, POFF(tpgrp), LONG, PIDFMT},
	{{"tsession", "tsess"},
                 TSESS_MSG, "TSESS", 0, pvar, PIDLEN, POFF(tsession), LONG, PIDFMT},
#endif
	{{"state", "stat"},
                 STAT_MSG, "S", LJUST|THREAD, state, 4},
	{{"pri"},
                 PRI_MSG, "PRI", 0, pvar, 3, POFF(pri), CHAR, "d"},
	{{"usrpri"},
                 UPR_MSG, "UPR", 0, pvar, 3, POFF(base_pri), CHAR, "d"},
	{{"nice", "ni"},
                 NI_MSG, "NI", THREAD, pnice, 3},
	{{"vsize", "vsz"},
                 VSZ_MSG, "VSZ", 0, vsize, 5},
	{{"rssize", "rsz", "rss", "p_rss"},
                 RSS_MSG, "RSS", 0, rsize, 4},
#undef	u_procp
        {{"u_procp", "uprocp"},
                 UPROCP_MSG, "UPROCP", USER, uvar, 6, UOFF(u_procp), KPTR, "x"},
#undef	u_cmask
	{{"umask", "u_cmask"},
                 UMASK_MSG, "UMASK", USER, uvar, 3, UOFF(u_cmask), CHAR, "o"},
#undef	u_acflag
	{{"acflag", "acflg"},
                 ACFLG_MSG, "ACFLG", USER, uvar, 3, UOFF(u_acflag), SHORT, "x"},
	{{"start"},
                 STARTED_MSG, "STARTED", USER|LJUST, started, 8},
	{{"lstart"},
                 STARTED_MSG, "STARTED", USER|LJUST, lstarted, 28},
	{{"etime"},
                 ELAPSED_MSG, "ELAPSED", USER|THREAD, etime, 11},
	{{"cputime", "time"},
                 TIME_MSG, "TIME", USER|THREAD, cputime, 11},
	{{"usertime"},
                 USER_MSG, "USER", USER|THREAD, usertime, 11},
	{{"systime"},
                 SYSTEM_MSG, "SYSTEM", USER|THREAD, systime, 11},
	{{"pcpu", "%cpu"},
                 CPU_MSG, "%CPU", USER|THREAD, pcpu, 4},
	{{"pmem", "%mem"},
                 MEM_MSG, "%MEM", USER, pmem, 4},
	{{"sl", "slp", "slptime"},
                 SL_MSG, "SL", THREAD, pvar, 8, POFF(slptime), LONG, "d"},
	{{"pagein"/*, "majflt"*/},
                 PAGEIN_MSG, "PAGEIN", USER, pagein, 6},
	{{"minflt"},
                 MINFLT_MSG, "MINFLT", USER, rvar, 4, ROFF(ru_minflt), LONG, "d"},
	{{"majflt"},
                 MAJFLT_MSG, "MAJFLT", USER, rvar, 4, ROFF(ru_majflt), LONG, "d"},
	{{"nswap"},
                 NSWAP_MSG, "NSWAP", USER, rvar, 4, ROFF(ru_nswap), LONG, "d"},
	{{"inblock", "inblk"},
                 INBLK_MSG, "INBLK", USER, rvar, 4, ROFF(ru_inblock), LONG, "d"},
	{{"oublock", "oublk"},
                 OUBLK_MSG, "OUBLK", USER, rvar, 4, ROFF(ru_oublock), LONG, "d"},
	{{"msgsnd"},
                 MSGSND_MSG, "MSGSND", USER, rvar, 4, ROFF(ru_msgsnd), LONG, "d"},
	{{"msgrcv"},
                 MSGRCV_MSG, "MSGRCV", USER, rvar, 4, ROFF(ru_msgrcv), LONG, "d"},
	{{"nsignals", "nsigs"},
                 NSIGS_MSG, "NSIGS", USER, rvar, 4, ROFF(ru_nsignals), LONG, "d"},
	{{"nvcsw", "vcsw"},
                 VCSW_MSG, "VCSW", USER, rvar, 5, ROFF(ru_nvcsw), LONG, "d"},
	{{"nivcsw", "ivcsw"},
                 IVCSW_MSG, "IVCSW", USER, rvar, 5, ROFF(ru_nivcsw), LONG, "d"},
#ifdef	__alpha
	{{"scount", "scnt"},
                 SCNT_MSG, "SCNT", THREAD, pvar, 2, POFF(suspend_count), INT, "d"},
#else
	{{"scount", "scnt"},
                 SCNT_MSG, "SCNT", THREAD, pvar, 2, POFF(suspend_count), LONG, "d"},
#endif
	{{"psxpri"},
                 PPR_MSG, "PPR", THREAD, psxpri, 3},
	NULL
};
var_t vhead, vtail;

/*
 * Canned output formats
 */
#define DFMT	"pid tname state cputime command"
#define LFMT \
	"uid pid ppid cp pri nice vsz rss wchan state tname cputime command"
#define	JFMT	"user pid ppid pgid sess jobc state tname cputime command"
#define	SFMT \
	"uid pid cursig sig sigmask sigignore sigcatch stat tname command"
#define	VFMT \
	"pid tt state time sl pagein vsz rss %cpu %mem command"
/*"pid tt state time sl re pagein vsz rss lim tsiz trs %cpu %mem command"*/
#define UFMT \
	"uname pid %cpu %mem vsz rss tt state start time command"
        
#define F5FMT \
        "uname pid ppid %cpu start tt time command"
#define L5FMT \
        "f state uid pid ppid %cpu pri nice rss wchan tt time ucomm"
#define FL5FMT \
        "f state uid pid ppid %cpu pri nice rss wchan start time command"

/* 
 * combination variables. These provide a "short-hand" for
 * frequent use.
 */
struct combovar {
	char *name;
	char *replace;
} combovar[] = {
	"RUSAGE",	"minflt majflt nswap inblock oublock msgsnd msgrcv nsigs nvcsw nivcsw",
        "THREAD",   	"user %cpu pri scnt wchan usertime systime",
        "DFMT",		DFMT,
        "LFMT",		LFMT,
        "JFMT",		JFMT,
        "SFMT",		SFMT,
        "VFMT",		VFMT,
        "UFMT",		UFMT,
        "F5FMT",	F5FMT,
        "L5FMT",	L5FMT,
        "FL5FMT",	FL5FMT,
	0, 0
};

extern char *devname();

void	add_int(int i, int **, int *);
int	check_int(int, int [], int);
int	pscomp(proc_info_t, proc_info_t);
int	parsetty(char *);
int	parsepid(char *);
int	parsepgrp(char *);
int	parsegid(char *);
int	parsesess(char *);
int	parseuid( char *, int);
void	print_all_tasks(void);
int	mach_state_order(int, long);
char *	digits(int, float);
char *	mem_to_string(int, vm_size_t);
void	parsefmt(char *);
void	scanvars(void);
void	printheader(void);
void	ptime(var_t, long, long);
void	printval(char *, struct var *);
void	get_proc_table(void);
void	save(struct tbl_procinfo *);
char *	savestr(char *);
void	savefmt(char *, char **);
void	error(int, char *, ...);
void	warn(int, char *, ...);
void	syserror(char *);
        
/*
 * Buffer to retrieve arguments and environment
 */
int	arguments_size = 4096;
char    def_arguments_buf[4096];
char	*arguments = def_arguments_buf;
char	cbuf[4096];
char	*np = cbuf;

int
main(int argc, char *argv[])
{
        char *iptr;
        struct winsize	win;
        char *fmt = NULL; 
	char *parse_bsd_options(int argc, char *argv[]); 
	char *parse_sysv_options(int argc, char *argv[]);

#if SEC_BASE
    set_auth_parameters(argc, argv);
    initprivs();

    /* raise privilege to read kernel memory and retrieve task ports */
    if (forceprivs(privvec(SEC_DEBUG, -1), (priv_t *) 0)) {
        fprintf(stderr, "%s: insufficient privileges\n", command_name);
        exit(1);
    }
#endif
        
        (void) setlocale (LC_ALL,"");
        catd = catopen((char *)MF_PS, NL_CAT_LOCALE);

        (void) mach_init();

        /* attempt to find out the size of the termainal */
        termwidth = 79;
        termheight = 24;
        if ((iptr = getenv("COLUMNS")) != NULL)
                termwidth = atoi(iptr);
        else if ((ioctl(1, TIOCGWINSZ, &win) == -1 &&
                  ioctl(2, TIOCGWINSZ, &win) == -1 &&
                  ioctl(0, TIOCGWINSZ, &win) == -1) ||
                 win.ws_col == 0)
#ifdef USE_TERMINFO
                if (getenv("TERM")) {
                        termheight = tgetnum("ro");
                        termwidth = tgetnum("co");
                }
                else
#endif
        {
                termwidth = 79;
                termheight = 24;
        }
        else {
                termwidth = win.ws_col - 1;
                termheight = win.ws_row;
        }

        /*
         * If argv[1] doesn't begin with a '-' then parse BSD options
         * otherwise use SYSV options.
         */
        if (argc > 1) {
                if (*argv[1] != '-')
                        fmt = parse_bsd_options(argc, argv);
                else
                        fmt = parse_sysv_options(argc, argv);
        }
        
	if (fmt)
		parsefmt(fmt);
        else
                parsefmt(DFMT);
        
	if (!aflg && !Eflg && !gflg &&
            num_ttys_to_check == 0 &&
            num_pids_to_check == 0 &&
            num_pgrps_to_check == 0 &&
            num_sess_to_check == 0 &&
            num_rgids_to_check == 0 &&
            num_ruids_to_check == 0 &&
            num_uids_to_check == 0)
                add_uid(getuid());

        if (prtheader)
                prtheader = termheight > 5 ? termheight - 2 : 22;

        print_all_tasks();
	if (nprocs == 0)
		exit(1);
	else
		exit(0);
}

char *
parse_bsd_options(int argc, char *argv[])
{
	char *kludge_bsdps_options(char *);
        extern char *optarg;
        extern int optind;

        int ch, errflg=0, fmtflg=0;
        char *fmt = NULL; 

        argv[1] = kludge_bsdps_options(argv[1]);

        while ((ch = getopt(argc, argv, bsd_options)) != EOF)
            switch ((char)ch) {
            case 'a':	aflg++;		break;  /* BSD: all with ttys */
            case 'e':	eflg++;		break;  /* BSD: environment */
            case 'x':	xflg++;		break;  /* BSD: everything */
            case 'm':	mflg++;		break;  /* BSD: threads */
            case 'g':	gflg++;		break;  /* BSD: session leaders */
            case 'r':	rflg++;		break;	/* BSD: print errors */
            case 'l':                           /* BSD: long */
                    fmt = LFMT;
                    fmtflg++;
                    break;
            case 'v':                           /* BSD: vm info */
                    fmt = VFMT;
                    fmtflg++;
                    sortby = SORTMEM;
                    break;
            case 'u':                           /* BSD: user info */
                    fmt = UFMT;
                    fmtflg++;
                    /*sortby = SORTRUN;*/
                    sortby = SORTCPU;
                    break;
            case 's':                           /* BSD: signals info */
                    fmt = SFMT;
                    fmtflg++;
                    break;
            case 'j':                           /* BSD: job control info */
                    fmt = JFMT;
                    fmtflg++;
                    break;
            case 'S':                           /* BSD: usage summary */
                    sumrusage++;
                    break;
            case 'U':   /* /etc/rc calls ps -U but for now we do nothing */
                    exit(0);
                    break;
            case 'A':                           /* BSD: increase arg space */
                    arguments_size += arguments_size;
                    if (arguments_size & 0x3)
                            /* Needs a word multiple, sorry */
                            arguments_size &= ~ 0x3;
                    arguments = (char *)calloc((arguments_size / sizeof(int)), sizeof(int));
                    break;
            case 'L': {                         /* BSD: list format options */
                    int i = 0;
                    struct combovar *cb = &combovar[0];
                    char *cp;
                    var_t v;
                    
                    v = &var[0];
                    for (;;) {
                            if (v->name[0] != NULL) {
                                    cp = v->name[0];
                                    v++;
                            } else if (cb->name != NULL) {
                                    cp = cb->name;
                                    cb++;
                            } else
                                    break;
                            if (termwidth && 
                                (i += strlen(cp)+1) > termwidth)
                                    i = strlen(cp), printf("\n");
                            printf("%s ", cp);
                    }       
                    printf("\n");
                    exit(0);
            }
            case 'w':                           /* BSD: widen */
                    if ((termwidth != UNLIMITED) && (termwidth < 131))
                            termwidth = 131;
                    else
                            termwidth = UNLIMITED;
                    break;
            case 'o':                           /* BSD: custom options */
                    fmt = savestr(optarg);
                    fmtflg++;
                    break;
            case 'O':                           /* BSD: canned options */
                    fmt = (char *)malloc(strlen(optarg) + 25);
                    sprintf(fmt, "pid %s state tt time comm", optarg);
                    fmtflg++;
                    break;
            case 'T':                           /* BSD: controlling tty */
                    if ((optarg = ttyname(0)) == NULL)
                            error(NOT_TTY, "%s: not a terminal", "<stdin>");
                    /*FALLTHROUGH*/
            case 't':                           /* BSD: specified tty */
                    errflg = parsetty(optarg);
                    xflg++;
                    aflg++;
                    break;
            case 'p':                           /* BSD: specific pid */
                    add_pid(atoi(optarg));
                    xflg++;
                    aflg++;
                    break;
            case 'h':                           /* BSD: extra header */
                    prtheader++;
                    break;
            case '?':
            default:
                    errflg++;
                    break;
            }

        if (errflg) {
                fprintf(stderr, MSGSTR(BSD_USAGE, bsd_usage), BSD_USAGE_OPTS);
                fprintf(stderr,"\t\t-or-\n");
                fprintf(stderr, MSGSTR(SYSV_USAGE, sysv_usage), SYSV_USAGE_OPTS); 
                exit(1);
        }

        if (fmtflg > 1)
                error(BAD_FMT, "specified too many format options");

        return(fmt);
}
        
char *
parse_sysv_options(int argc, char *argv[])
{
        extern char *optarg;
        extern int optind;
	char *uidarg, *ruidarg;
        int ch, errflg=0, fmtflg=0;
        int lfflg=0, llflg=0;
	int uidflg=0, ruidflg=0;
        char *fmt = NULL;
        
        while ((ch = getopt(argc, argv, sysv_options)) != EOF)
	    switch ((char)ch) {
	    case 'A':				/* XPG4: everything */
            case 'e':	Eflg++;	xflg++;	break;  /* SYSV: everything */
            case 'a':	aflg++;		break;  /* SYSV: all with ttys */
            case 'm':	mflg++;		break;  /* SYSV: include threads */
            case 'd':	gflg++;		break;  /* XPG4: except session */
						/*       leaders        */
            case 'r':	rflg++;		break;	/* BSD: print errors */
            case 'f':                           /* SYSV: flags info */
                    lfflg++;
                    break;
            case 'l':                           /* SYSV: long info */
                    llflg++;
                    break;
            case 'j':                           /* SYSV: job control info */
                    fmt = JFMT;
                    fmtflg++;
                    break;
	    case 'n':				/* XPG4: alternate name list */
		    /* Make sure file exists and is readable */
		    if (access(optarg, F_OK|R_OK) == -1)
			error(BAD_FILE, "Cannot access file: %s", optarg);

		    /* Point to user-specified password file */
		    setpwfile(optarg);
		    break;
            case 't':                           /* SYSV: specific ttys */
                    errflg = parsetty(optarg);
                    xflg++;
                    aflg++;
                    break;
            case 'p':                           /* SYSV: specific pids */
                    errflg = parsepid(optarg);
                    xflg++;
                    aflg++;
                    break;
            case 'g':                           /* SYSV: specific pgroups */
                    errflg = parsepgrp(optarg);
                    xflg++;
                    aflg++;
                    break;
            case 'G':                           /* SYSV: specific real groups */
                    errflg = parsegid(optarg);
                    xflg++;
                    aflg++;
                    break;
            case 's':                           /* SYSV: specific sessions */
                    errflg = parsesess(optarg);
                    xflg++;
                    aflg++;
                    break;
            case 'u':                           /* SYSV: specific users */
		    /* Can't call parseuid() yet - might have specified */
		    /* -n option to use alternate passwd file/db        */
		    uidflg++;
		    uidarg = optarg;
                    xflg++;
                    aflg++;
                    break;
            case 'U':                          /* XPG4: Use real uids */
		    /* Can't call parseuid() yet - might have specified */
		    /* -n option to use alternate passwd file/db        */
		    ruidflg++;
		    ruidarg = optarg;
                    xflg++;
                    aflg++;
                    break;
            case 'o':                           /* SYSV: custom options */
                    savefmt(optarg, &fmt);
                    break;
            case 'O':                           /* SYSV: canned options */
                    fmt = (char *)malloc(strlen(optarg) + 25);
                    sprintf(fmt, "pid %s state tt time comm", optarg);
                    break;
            case '?':
            default:
                    errflg++;
                    break;
            }

	if (ruidflg)
		errflg = parseuid(ruidarg,ADD_RUID);

	if (uidflg)
		errflg = parseuid(uidarg,ADD_UID);

        if (errflg) {
                fprintf(stderr, MSGSTR(SYSV_USAGE, sysv_usage), SYSV_USAGE_OPTS);
                fprintf(stderr,"\t\t-or-\n");
                fprintf(stderr, MSGSTR(BSD_USAGE, bsd_usage), BSD_USAGE_OPTS);
                exit(1);
        }
        
        if (lfflg && llflg && fmtflg)
                error(BAD_FMT, "specified too many format options");
        else if (lfflg && llflg)
                fmt = FL5FMT;
        else if (lfflg)
                fmt = F5FMT;
        else if (llflg)
                fmt = L5FMT;

        return(fmt);
}

void
add_int(int i, int *p[], int *n)
{
        int *ip;
        
        if (*p == NULL)
                *p = (int *)malloc(sizeof(int));
        else
                *p = (int *)realloc(*p, (*n + 1) * sizeof(int));
        ip = *p;
        
        ip[(*n)++] = i;
}

int
check_int(int i, int p[], int n)
{
        int m;
        
        /* if list is empty; return TRUE */
        if (n == 0)
                return TRUE;
        
        for (m = 0; m < n; m++)
                if (p[m] == i)
                        return TRUE;
        return FALSE;
}

int
pscomp(proc_info_t p1, proc_info_t p2)
{
	register int i;

	if (sortby == SORTCPU) {
	     return (p2->cpu_usage > p1->cpu_usage ? 1 : -1);
	}

	if (sortby == SORTMEM) {
	    return (p2->resident_size - p1->resident_size);
	}

	i = (int)p1->tty - (int)p2->tty;
	if (i == 0)
	    i = p1->pid - p2->pid;
	return (i);
}

int
parsetty(char *tp)
{
        struct stat stbuf;
        char *tname;
        char termname[MAXPATHLEN+1];

        while((tname = strtok(tp, FMTSEP)) != NULL) {
                if (strlen(tname) == 2) {
                        if (strcmp(tname, "co") == 0)
                                strcpy(termname, "/dev/console");
                        else {
                                strcpy(termname, "/dev/tty");
                                strcat(termname, tname);
                        }
                }
                else if (*tname != '/') {
                        strcpy(termname, "/dev/");
                        strcat(termname, tname);
                } else
                        strcpy(termname, tname);
                if (stat(termname, &stbuf) == -1)
                        syserror(termname);
                if ((stbuf.st_mode & S_IFMT) != S_IFCHR)
                        error(NOT_TTY, "%s: not a terminal", termname);
                add_tty(stbuf.st_rdev);
                tp = NULL;
        }
        return(0);
}

int
parsepid(char *pp)
{
        char *spid;

        while((spid = strtok(pp, FMTSEP)) != NULL) {
                if (isdigit(*spid))
                        add_pid(atoi(spid));
                else
                        error(NOT_INT, "argument must be an integer");
                pp = NULL;
        }
        return(0);
}

int
parsepgrp(char *gp)
{
        char *spgrp;

        while((spgrp = strtok(gp, FMTSEP)) != NULL) {
                if (isdigit(*spgrp))
                        add_pgrp(atoi(spgrp));
                else
                        error(NOT_INT, "argument must be an integer");
                gp = NULL;
        }
        return(0);
}

int
parsegid(char *gidarg)
{
        char *sgid;
	int gid;
	register struct group *gr;

        while((sgid = strtok(gidarg, FMTSEP)) != NULL) {
                if (isdigit(*sgid))
                        gid = atoi(sgid);
                else {
			if ((gr = getgrnam(sgid)) == (struct group *)NULL)
				error (NO_GROUP, "cannot find group: \"%s\"",sgid);
			else
				gid = gr->gr_gid;
		}
		add_rgid(gid);
                gidarg = NULL;
        }
        return(0);
}

int
parsesess(char *sp)
{
        char *sessp;

        while((sessp = strtok(sp, FMTSEP)) != NULL) {
                if (isdigit(*sessp))
                        add_sess(atoi(sessp));
                else
                        error(NOT_INT, "argument must be an integer");
                sp = NULL;
        }
        return(0);
}

int
parseuid( char *up, int uid_action)
{
        struct passwd *pw;
        char *suid;
        int uid;

        while((suid = strtok(up, FMTSEP)) != NULL) {
                if (isdigit(*suid)) {
                        uid = atoi(suid);
                }
                else {
                        if ((pw = getpwnam(suid)) == NULL)
                                error(NO_USER, "cannot find user: \"%s\"", suid);
                        uid = pw->pw_uid;
                }
                
		if (uid_action == ADD_RUID)
	                add_ruid(uid);
		else
			add_uid(uid);

                up = NULL;
        }
        return(0);
}

void
print_all_tasks(void)
{
        var_t v;
        int i, nthread;
        proc_info_t pi;
        
        /*
         * scan requested variables, noting what structures are needed,
         * and adjusting header widths as appropiate.
         */
        scanvars();

        max_proc_table = 200;
        proc_table = (proc_info_t)calloc((unsigned)max_proc_table,
                                         sizeof(struct proc_info));
        get_proc_table();

	/*
	 * print header
	 */
	printheader();
	if (nprocs == 0)
                return;
        
        /*
         * sort proc list
         */
        qsort((char *)proc_table, nprocs, sizeof(struct proc_info), pscomp);

        /*
         * for each proc, call each variable output function.
         */
        for (i = 0; i < nprocs; i++) {
                pi = &proc_table[i];
                for (v = vhead; v != NULL; v = v->next) {
                        (*v->oproc)(pi, v);
                        if (v->next != NULL)
                                putchar(' ');
                }
                putchar('\n');
                if (prtheader && lineno++ == prtheader-4) {
                        putchar('\n');
                        printheader();
                        lineno = 0;
                }
                if (mflg && pi->num_threads > 1) {
                        for (nthread = 0; nthread < pi->num_threads; nthread++) {
                                for (v = vhead; v != NULL; v = v->next) {
                                        if (v->flag & THREAD)
                                                (*v->oproc)(&pi->threads[nthread], v);
                                        else
                                                empty(&pi->threads[nthread], v);
                                        if (v->next != NULL)
                                                putchar(' ');
                                }
                                putchar('\n');
                                if (prtheader && lineno++ == prtheader-4) {
                                        putchar('\n');
                                        printheader();
                                        lineno = 0;
                                }
                        }
                }
        }
}

/*
 *	Translate thread state to a number in an ordered scale.
 *	When collapsing all the threads' states to one for the
 *	entire task, the lower-numbered state dominates.
 */
#define	STATE_MAX	7

int
mach_state_order(int s, long sleep_time)
 {
    switch (s) {
    case TH_STATE_RUNNING:      return(1);
    case TH_STATE_UNINTERRUPTIBLE:
                                return(2);
    case TH_STATE_WAITING:      return((sleep_time > 20) ? 4 : 3);
    case TH_STATE_STOPPED:      return(5);
    case TH_STATE_HALTED:       return(6);
    default:                    return(7);
    }
 }
			    /*01234567 */
char	mach_state_table[] = " RUSITH?";

char *
digits(int fw, float n)
{
        static char	tmp[10];	/* STATIC! */
	int ones,	/* amount of field width left after ... */
	    tens,	/* ... whole number plus decimal point */
	    huns;

	if (fw <= 0)
		fw = 4;	/* nominal width */

	ones = fw - 2;
	tens = fw - 3;
	huns = fw - 4;

        if ((n > 0) && (n < 10))
                sprintf(tmp, "%*.*f", fw, (ones>0) ? ones : 0, n);
        else if ((n > 0) && (n < 100))
                sprintf(tmp, "%*.*f", fw, (tens>0) ? tens : 0, n);
        else
                sprintf(tmp, "%*.*f", fw, (huns>0) ? huns : 0, n);
        return(tmp);
}

char *
mem_to_string(int w, vm_size_t	n)
{
        static char buf[12];

        /* convert to bytes */
        n /= 1024;

        if (n > 1024*1024)
                sprintf(buf, "%sG", digits(w-1,((float)n)/(1024.0*1024.0)));
        else if (n > 1024)
                sprintf(buf, "%sM", digits(w-1,(float)n/(1024.0)));
        else
                sprintf(buf, "%dK", n);

        return(buf);
}

void
parsefmt(char *fmt)
{
	char *f = fmt, *cp, *hp;
	struct var *v;
	char newbuf[1024], *nb = newbuf; /* XXX */
	char *lookupcombo(char *);
	struct var *lookupvar(char *);

	/*
	 * strtok is not &^%^& re-entrant, so we have
	 * only one level of expansion, looking for combo
	 * variables once here, and expanding the string
	 * before really parsing it.  With strtok_r,
	 * you would move the expansion to before the
	 * lookupvar inside the 2nd while loop with a
	 * recursive call to parsefmt.
	 */
	while ((cp = strtok(f, FMTSEP)) != NULL) {
		if ((hp = lookupcombo(cp)) == NULL)
			hp = cp;
		if (((nb + strlen(hp)) - newbuf) >= 1024)
			error(FMT_2BIG, "format too large");
		strcpy(nb, hp);
		while (*nb)
			nb++;
		*nb++ = ' ';
		*nb =  '\0';
		f = NULL;
	}
	f = newbuf;
	while ((cp = strtok(f, FMTSEP)) != NULL) {
		if (hp = index(cp, '='))
			*hp++ = '\0';
		v = lookupvar(cp);
		if (v == NULL)
			error(UNKNOWN_VAR, "unknown variable in format: %s", cp);
		if (v->next != NULL || vtail == v)
			error(VAR_2X, "can't specify a variable twice: %s", cp);
		if (hp)
			v->header = savestr(hp);
		if (vhead == NULL)
			vhead = vtail = v;
		else {
			vtail->next = v;
			vtail = v;
		}
		f = NULL;	/* for strtok */
	}
}

void
scanvars(void)
{
	register i;
	register struct var *v;

	for (v = vhead; v != NULL; v = v->next) {
		i = strlen(MSGSTR(v->header_msg, v->header));
		if (v->width < i)
			v->width = i;
		totwidth += v->width + 1;	/* +1 for space */
		if (v->flag & USER)
			needuser = 1;
		if (v->flag & COMM)
			needcomm = 1;
	}
	totwidth--;
}

void
printheader(void)
{
	register struct var *v;

	for (v = vhead; v != NULL; v = v->next) {
		if (v->flag & LJUST) {
			if (v->next == NULL) {
                                /* last one */
                                if (termwidth == UNLIMITED)
                                        printf("%s", MSGSTR(v->header_msg, v->header));
                                else {
                                        register left = termwidth - (totwidth - v->width);
                                        register char *cp = MSGSTR(v->header_msg, v->header);

                                        /* already wrapped, just use std width */
                                        if (left < 1)
                                                left = v->width;
                                        while (left-- && *cp)
                                                putchar(*cp++);
                                }
                        }
			else
				printf("%-*s",v->width, MSGSTR(v->header_msg, v->header));
		} else
			printf("%*s",v->width, MSGSTR(v->header_msg, v->header));
		if (v->next != NULL)
			putchar(' ');
	}
	putchar('\n');
}

void
command(proc_info_t pi, var_t v) 
{
	if (v->next == NULL) {		
		/* last field */
		if (termwidth == UNLIMITED)
			printf("%s", pi->args);
		else {
			register left = termwidth - (totwidth - v->width);
			register char *cp = pi->args;

			if (left < 1)	/* already wrapped, just use std width */
				left = v->width;
			while (left-- && *cp)
				putchar(*cp++);
		}
	} else
		printf("%-*.*s", v->width, v->width, pi->args);
}

void
ucomm(proc_info_t pi, var_t v)
{
	if (v->next == NULL) {		
		/* last field */
		if (termwidth == UNLIMITED)
			printf("%s", pi->command);
		else {
			register left = termwidth - (totwidth - v->width);
			register char *cp = pi->command;

			if (left < 1)	/* already wrapped, just use std width */
				left = v->width;
			while (left-- && *cp)
				putchar(*cp++);
		}
	} else
		printf("%-*.*s", v->width, v->width, pi->command);
}

void
logname(proc_info_t pi, var_t v)
{
        if (pi->u)
                printf("%-*s", v->width, pi->u->u_logname);
        else
                printf("%-*s", v->width, " ");
        
}

void
state(proc_info_t pi, var_t v)
{
        static char buf[5];	/*..rjg.. Increase from 4 per OSF problem # 3149*/
        char *cp = buf;
        
        *cp++ = mach_state_table[pi->state];
        if (pi->all_swapped)
                *cp++ = 'W';
        else
                *cp++ = ' ';  

        if (pi->base_pri > NZERO)
                *cp++ = 'N';
	else if (pi->base_pri < NZERO)
                *cp++ = '<';
        else
                *cp++ = ' ';
        if ((pi->flag & SCTTY) && pi->pgrp == pi->tpgrp)
                *cp++ = '+';
        *cp = '\0';
        printf("%-*s", v->width, buf);
}

void
psxpri(proc_info_t pi, var_t v)
{
	printf("%*d", v->width, 63 - pi->pri);
}

void
uname(proc_info_t pi, var_t v)
{
	register struct passwd *pw;

	pw = getpwuid(pi->uid);
	if (pw != (struct passwd *)NULL) {
		if (pw->pw_name != (char *) NULL)
			printf("%-*s", v->width, pw->pw_name);
		else
			printf("%-*d", v->width, pi->uid);
	}
	else
		printf("%-*d", v->width, pi->uid);
}

void
runame(proc_info_t pi, var_t v)
{
	register struct passwd *pw;

	pw = getpwuid(pi->ruid);

	if (pw != (struct passwd *)NULL) {
		if (pw->pw_name != (char *) NULL)
			printf("%-*s", v->width, pw->pw_name);
		else
			printf("%-*d", v->width, pi->ruid);
	}
	else
		printf("%-*d", v->width, pi->ruid);
}

void
gname(proc_info_t pi, var_t v)
{
	register struct group *grp;

	grp = getgrgid(pi->gid);
	if (grp != (struct group *) NULL) {
		if (grp->gr_name != (char *) NULL)
			printf ("%-*s", v->width, grp->gr_name);
		else
			printf ("%-*d", v->width, pi->gid);
	}
	else
		printf ("%-*d", v->width, pi->gid);

}

void
rgname(proc_info_t pi, var_t v)
{
	register struct group *grp;

	grp = getgrgid(pi->rgid);
	if (grp != (struct group *) NULL) {
		if (grp->gr_name != (char *) NULL)
			printf ("%-*s", v->width, grp->gr_name);
		else
			printf ("%-*d", v->width, pi->rgid);
	}
	else
		printf ("%-*d", v->width, pi->rgid);

}

void
tdev(proc_info_t pi, var_t v)
{
	dev_t dev = pi->tty;

	if (dev == NODEV)
		printf("%*s", v->width, "??");
	else {
		char buff[16];

		sprintf(buff, "%d/%d", major(dev), minor(dev));
		printf("%*s", v->width, buff);
	}
}


void
tname(proc_info_t pi, var_t v)
{
        dev_t dev = pi->tty;
        char *tname;

        if (dev == NODEV || (tname = devname(dev, S_IFCHR)) == NULL)
                printf("%-*s", v->width, "??");
        else {
                if (strncmp(tname, "tty", 3) == 0)
                        tname += 3;
                printf("%*.*s%c", v->width-1, v->width-1, tname,
                       (pi->flag&SCTTY)?' ':'-');
        }
}

void
longtname(proc_info_t pi, var_t v)
{
        dev_t dev = pi->tty;
        char *tname;

        if (dev == NODEV || (tname = devname(dev, S_IFCHR)) == NULL)
                printf("%-*s", v->width, "??");
        else
                printf("%-*s", v->width, tname);
}

/*
 * Print start time. If time is less than 24hrs then print the time
 * otherwise print the start date.
 */
#undef	u_start

void
started( proc_info_t pi, var_t v)
{
        char *tp = NULL;

        if (pi->u) {
                char time_str[9];
                struct tm *sttm;
                time_t stime = pi->u->u_start.tv_sec;
                
                sttm = localtime(&stime);
                if (time((time_t *)0) -  stime < 24L*60L*60L)
                        (void)strftime(time_str, sizeof(time_str), "%8X", sttm);
                else
                        (void)strftime(time_str, sizeof(time_str), "%8sD", sttm);
                tp = time_str;
        }
        else
                tp = "-";

        printf("%-*s", v->width, tp);
}

void
lstarted( proc_info_t pi, var_t v)
{
	char *tp; 

        if (pi->u) {
                char time_str[29];
                struct tm *sttm;
		time_t stime = pi->u->u_start.tv_sec;

                sttm = localtime(&stime);
                (void) strftime(time_str, sizeof(time_str), "%28c", sttm);
                tp = time_str;
        }                
	else
		tp = "-";

	printf("%-*s", v->width, tp);
}

void
etime( proc_info_t pi, var_t v)
{
        if (pi->u) {
		time_t elapsed;

		elapsed = time((time_t *)0) - (time_t) pi->u->u_start.tv_sec; 
		ptime (v, (long) elapsed, -1L);
        }                
	else
		printf("%-*s", v->width, "-");
}

void
wchan(proc_info_t pi, var_t v)
{
        char *mesg;

        mesg = pi->wait_mesg;

        switch (pi->wait_chan) {
        case 0L:
                printf("%-*s", v->width, "-");
                break;
        case -1L:
                printf("%-*s", v->width, "*");
                break;
        default:
                if (mesg && *mesg && pi->state != 2)
                        printf("%-*.*s", v->width, v->width, mesg);
                else
#ifdef	__alpha
			/* wait_chan is declared as long, but lower 32bits
			   are required					   */
                        printf("%-*lx", v->width, (pi->wait_chan & 0xffffffff));
#else
                        printf("%-*lx", v->width, (pi->wait_chan & ~KERNBASE));
#endif
                break;
        }
}
        
void
vsize(proc_info_t pi, var_t v)
{
        printf("%*.*s", v->width, v->width,
	    mem_to_string((int)v->width, pi->virtual_size));
}

void
rsize(proc_info_t pi, var_t v)
{
        printf("%*.*s", v->width, v->width,
	    mem_to_string((int)v->width, pi->resident_size));
}

void
cputime(proc_info_t pi, var_t v)
{
	long secs;
	long psecs;	/* "parts" of a second. first micro, then centi */
#undef	u_cru
        
	if (pi->status == PI_ZOMBIE || pi->u == NULL) {
		secs = 0;
		psecs = 0;
	} else {
		secs = pi->p_utime.seconds + 
			pi->p_stime.seconds;
		psecs = pi->p_utime.microseconds + 
			pi->p_stime.microseconds;
		if (sumrusage) {
			secs += pi->u->u_cru.ru_utime.tv_sec + 
				pi->u->u_cru.ru_stime.tv_sec;
			psecs += pi->u->u_cru.ru_utime.tv_usec + 
				pi->u->u_cru.ru_stime.tv_usec;
		}
		/*
		 * round and scale to 100's
		 */
		psecs = (psecs + 5000) / 10000;
		while (psecs >= 100) { /* paw: 8817 */
			psecs -= 100;
			secs++;
		}
	}
        ptime(v, secs, psecs);
}

void
usertime(proc_info_t pi, var_t v)
{
	long secs;
	long psecs;	/* "parts" of a second. first micro, then centi */
        
	if (pi->status == PI_ZOMBIE || pi->u == NULL) {
		secs = 0;
		psecs = 0;
	} else {
		secs = pi->p_utime.seconds;
		psecs = pi->p_utime.microseconds;

		/*
		 * round and scale to 100's
		 */
		psecs = (psecs + 5000) / 10000;
		while (psecs >= 100) { /* paw: 8817 */
			psecs -= 100;
			secs++;
		}
	}
        ptime(v, secs, psecs);
}

void
systime(proc_info_t pi, var_t v)
{
	long secs;
	long psecs;	/* "parts" of a second. first micro, then centi */
        
	if (pi->status == PI_ZOMBIE || pi->u == NULL) {
		secs = 0;
		psecs = 0;
	} else {
		secs = pi->p_stime.seconds;
		psecs = pi->p_stime.microseconds;

		/*
		 * round and scale to 100's
		 */
		psecs = (psecs + 5000) / 10000;
		while (psecs >= 100) { /* paw: 8817 */
			psecs -= 100;
			secs++;
		}
	}
        ptime(v, secs, psecs);
}

#define SECS_IN_HOUR	(60L * 60L)
#define SECS_IN_DAY	(24L * SECS_IN_HOUR)

void
ptime(var_t v, long secs, long psecs)
{
	char obuff[128];

        if (secs > SECS_IN_DAY)
                sprintf(obuff, "%2d-%02ld:%02ld:%02ld", secs/SECS_IN_DAY, 
			(secs/SECS_IN_HOUR)%24, (secs/60)%60, secs%60);
        else if (secs >= SECS_IN_HOUR)
                sprintf(obuff, "%02ld:%02ld:%02ld",
                                secs/SECS_IN_HOUR, (secs/60)%60, secs%60);
        else if (psecs == -1)   /* for etime */
                sprintf(obuff, "%3ld:%02ld", secs/60, secs%60);
        else
                sprintf(obuff, "%3ld:%02ld.%02ld", secs/60, secs%60, psecs);

        printf("%*s", v->width, obuff);
}

void
pcpu(proc_info_t pi, var_t v)
{
        double percent_cpu;
        
        percent_cpu = usage_to_percent(pi->cpu_usage) + (usage_to_tenths(pi->cpu_usage)/10);
        printf("%*.1f", v->width, percent_cpu);
}

void
pmem(proc_info_t pi, var_t v)
{
	printf("%*.1f", v->width, pi->percent_mem);
}

void
maxrss(proc_info_t pi, var_t v)
{
        printf("%*s", v->width, "-");
}

void
pagein(proc_info_t pi, var_t v)
{
#undef	u_ru
	printf("%*d", v->width, pi->u ? pi->u->u_ru.ru_majflt : 0);
}

void
pnice(proc_info_t pi, var_t v)
{
	printf("%*d", v->width, pi->base_pri - NZERO);
}
        
/*
 * Generic output routines.  Print fields from various prototype
 * structures.
 */
void
pvar(proc_info_t pi, var_t v)
{
	printval((char *)((char *)pi + v->off), v);
}

void
uvar(proc_info_t pi, var_t v)
{
	if (pi->u)
		printval((char *)((char *)pi->u + v->off), v);
	else
		printf("%*s", v->width, "-");
}

void
rvar(proc_info_t pi, var_t v)
{
	if (pi->u)
		printval((char *)((char *)(&pi->u->u_ru) + v->off), v);
	else
		printf("%*s", v->width, "-");
}

void
empty(proc_info_t pi, var_t v)
{
	if (v->next == NULL && termwidth != UNLIMITED) {
		/* last field */
                register left = termwidth - (totwidth - v->width);
                if (left < 1)	/* already wrapped, just use std width */
                        left = v->width;
                while (left--)
                        putchar(' ');
        }
	else
		printf("%-*.*s", v->width, v->width, " ");
}

struct var *
lookupvar(char *cp)
{
	register int i, j;

	for (i=0; var[i].name[0] != NULL; i++)
		for (j=0; var[i].name[j] != NULL; j++)
			if (strcmp(cp, var[i].name[j]) == 0)
				return (&var[i]);
	return ((struct var *)NULL);
}

char *
lookupcombo(char *cp)
{
	register struct combovar *cv = &combovar[0];

	for (; cv->name; cv++)
                if (strcmp(cp, cv->name) == 0) {
			return (cv->replace);
                }
	return ((char *)NULL);
}

void
printval(char *bp, struct var *v)
{
	static char ofmt[32] = "%";
	register char *cp = ofmt+1, *fcp = v->fmt;

	if (v->flag & LJUST)
		*cp++ = '-';
	*cp++ = '*';
	while (*cp++ = *fcp++)
		;

	switch (v->type) {
	case CHAR:
		printf(ofmt, v->width, *(char *)bp);
		break;

	case UCHAR:
		printf(ofmt, v->width, *(u_char *)bp);
		break;

	case SHORT:
		printf(ofmt, v->width, *(short *)bp);
		break;

	case USHORT:
		printf(ofmt, v->width, *(u_short *)bp);
		break;

	case INT:
		printf(ofmt, v->width, *(int *)bp);
		break;

	case UINT:
		printf(ofmt, v->width, *(u_int *)bp);
		break;

	case LONG:
		printf(ofmt, v->width, *(long *)bp);
		break;

	case ULONG:
		printf(ofmt, v->width, *(u_long *)bp);
		break;

	case KPTR:
		printf(ofmt, v->width, *(u_long *)bp & ~KERNBASE);
		break;

	default:
		error(UNKNOWN_TYPE, "unknown type %d", v->type);
	}
}

/* All of this should come out of the process manager... */

void
get_proc_table(void)
{
        register int i,j;
	int saved, checked;
        long	nproc;
#define NPROC    16
        struct tbl_procinfo proc[NPROC];
        struct tbl_procinfo *mproc;
    
        nproc = table(TBL_PROCINFO, 0, (char *)0, 32767, 0);

        for (i=0; i < nproc; i += NPROC) {
                j = table(TBL_PROCINFO, i, (char *)proc, NPROC, sizeof(proc[0]));
                for (j = j-1; j >= 0; j--) {
			saved = 0;
			checked = 0;
                        mproc = &proc[j];
                        if (mproc->pi_status == PI_EMPTY)
                                continue;
                        if (!gflg && !xflg) {
                                if (mproc->pi_status == PI_ZOMBIE ||
                                    mproc->pi_status == PI_EXITING)
                                        continue;
                        }
#if SEC_MAC
			if (!ps_proc_dominate(mproc->pi_pid))
			        continue;
#endif

			/* checked indicates that a selection   */
			/* option was specified, saved says	*/
			/* we already matched that process	*/

                        if (num_uids_to_check) {
				checked++;
                                if (check_uid(mproc->pi_uid)) {
					saved++;
                                        save(mproc);
				}
                        }
                        if (num_ruids_to_check && !saved) {
				checked++;
                                if (check_ruid(mproc->pi_ruid)) {
					saved++;
                                        save(mproc);
				}
                        }
                        if (num_rgids_to_check && !saved) {
				checked++;
                                if (check_rgid(mproc->pi_rgid)) {
					saved++;
                                        save(mproc);
				}
                        }
                        if (num_ttys_to_check && !saved) {
				checked++;
                                if (check_tty(mproc->pi_ttyd)) {
					saved++;
                                        save(mproc);
				}
                        }
                        if (num_pgrps_to_check && !saved) {
				checked++;
                                if (check_pgrp(mproc->pi_pgrp)) {
					saved++;
                                        save(mproc);
				}
                        }
                        if (num_sess_to_check && !saved) {
				checked++;
                                if (check_sess(mproc->pi_session)) {
					saved++;
                                        save(mproc);
				}
                        }
                        if (num_pids_to_check && !saved) {
				checked++;
                                if (check_pid(mproc->pi_pid)) {
					saved++;
                                        save(mproc);
				}
                        }
			/* If no selective options were given, save proc */
			if (!checked)
	                        save(mproc);
                }
        }
#undef	NPROC
}

void
save(struct tbl_procinfo *mproc)
{
        proc_info_t pi;
        struct user u;
        task_t task;

	/* Note that this logic will omit session leaders from the list   */
	/* if the -a and -d flags are both specified, even if the session */
	/* leader is associated with a tty - OK with XPG4.                */
        if (!gflg && !xflg && (mproc->pi_ttyd == NODEV || 
					(mproc->pi_flag & SCTTY) == 0))
                return;
        else if (gflg && !xflg && mproc->pi_session == mproc->pi_pid)
                return;

        nprocs++;
        if (nprocs > max_proc_table) {
                max_proc_table *= 2;
                proc_table = (proc_info_t)realloc((char *)proc_table, 
				(unsigned)max_proc_table*sizeof(*proc_table));
        }
        pi = &proc_table[nprocs-1];
        pi->tty		= mproc->pi_ttyd;
        pi->uid		= mproc->pi_uid;
        pi->ruid	= mproc->pi_ruid;
        pi->svuid	= mproc->pi_svuid;
        pi->rgid	= mproc->pi_rgid;
        pi->svgid	= mproc->pi_svgid;
        pi->pid		= mproc->pi_pid;
        pi->ppid	= mproc->pi_ppid;
        pi->pgrp	= mproc->pi_pgrp;
        pi->session	= mproc->pi_session;
        pi->tpgrp	= mproc->pi_tpgrp;
        pi->tsession	= mproc->pi_tsession;
        pi->jobc	= mproc->pi_jobc;
        pi->status	= mproc->pi_status;
        pi->flag	= mproc->pi_flag;
        pi->command	= savestr(mproc->pi_comm);
        pi->p_cursig	= (char)mproc->pi_cursig;
        pi->p_sig	= mproc->pi_sig;
        pi->p_sigmask	= mproc->pi_sigmask;
        pi->p_sigignore = mproc->pi_sigignore;
        pi->p_sigcatch	= mproc->pi_sigcatch;

        /*
         *	Find the other stuff
         */
        if (task_by_unix_pid(task_self(), pi->pid, &task) != KERN_SUCCESS) {
                pi->status = PI_ZOMBIE;
        }
        else {
                task_basic_info_data_t	ti;
                unsigned int		count;
                thread_array_t		thread_table;
                unsigned int		table_size;
                machine_info_data_t	mi;

                thread_basic_info_t	thi;
                thread_basic_info_data_t thi_data;
                int			i, t_state;

                count = TASK_BASIC_INFO_COUNT;
                if (task_info(task, TASK_BASIC_INFO, (task_info_t)&ti, &count)
                    	!= KERN_SUCCESS) {
                        pi->status = PI_ZOMBIE;
                }
                else {
                        (void) xxx_host_info(task, &mi);

                        pi->virtual_size = ti.virtual_size;
                        pi->resident_size = ti.resident_size;
                        pi->percent_mem = ((double)ti.resident_size /
                                           (double)mi.memory_size) * 100.0;

                        if((task_threads(task, &thread_table, &table_size)) != KERN_SUCCESS ){

			error(NO_MEM, "out of memory");
			}
                        
                        pi->p_utime = ti.user_time;
                        pi->p_stime = ti.system_time;

                        pi->state = STATE_MAX;
                        pi->pri = 255;
                        pi->base_pri = 255;
                        pi->all_swapped = TRUE;
                        pi->cpu_usage = 0;
                        pi->slptime = 0;
                        pi->wait_chan = 0;
                        pi->suspend_count = 0;
                        *pi->wait_mesg = '\0';
                        pi->u = NULL;

                        pi->num_threads = table_size;
                        thi = &thi_data;

                        /* Allocate space to store thread info */
                        if (mflg) {
                                if ((pi->threads = (proc_info_t)malloc(table_size * sizeof(struct proc_info))) == NULL)
                                        error(NO_MEM, "out of memory");
                        }
                        
                        for (i = 0; i < table_size; i++) {
                                count = THREAD_BASIC_INFO_COUNT;
                                if (thread_info(thread_table[i], THREAD_BASIC_INFO,
                                                (thread_info_t)thi, &count) == KERN_SUCCESS) {
                                        /* Save per thread info */
                                        if (mflg) {
                                                proc_info_t ti = &pi->threads[i];
                                                ti->cpu_usage	= thi->cpu_usage;
                                                ti->state	= mach_state_order(thi->run_state, thi->sleep_time);
                                                ti->all_swapped = ((thi->flags&TH_FLAGS_SWAPPED) != 0);
                                                ti->base_pri	= thi->base_priority;
                                                ti->pri		= thi->cur_priority;
                                                ti->wait_chan	= thi->wait_event;
                                                strncpy(ti->wait_mesg, thi->wait_mesg, WMESGLEN);
                                                ti->p_utime	= thi->user_time;
                                                ti->p_stime	= thi->system_time;
                                                ti->slptime	= thi->sleep_time;
                                                ti->suspend_count = thi->suspend_count;
                                                ti->u		= (struct user *)-1;    /* non-null */

                                        }
                                        /* Save task summary info */
                                        time_value_add(&pi->p_utime, &thi->user_time);
                                        time_value_add(&pi->p_stime, &thi->system_time);
                                        t_state = mach_state_order(thi->run_state,
                                                                   thi->sleep_time);
                                        if (t_state < pi->state)
                                                pi->state = t_state;

                                        if (thi->cur_priority < pi->pri)
                                                pi->pri = thi->cur_priority; 
                                        if (thi->base_priority < pi->base_pri)
                                                pi->base_pri = thi->base_priority;
                                        if ((thi->flags & TH_FLAGS_SWAPPED) == 0)
                                                pi->all_swapped = FALSE;
                                        pi->cpu_usage += thi->cpu_usage;
                                        pi->slptime += thi->sleep_time;
                                        pi->suspend_count += thi->suspend_count;
                                        
                                        /*
                                         * If more than one thread is sleeping
                                         * then set to -1.
                                         */
                                        if (pi->wait_chan && thi->wait_event)
                                                pi->wait_chan = -1;
                                        else if (thi->wait_event) {
                                                pi->wait_chan = thi->wait_event;
                                                strncpy(pi->wait_mesg, thi->wait_mesg, WMESGLEN);
                                        }
                                }
                        }

			(void) vm_deallocate(task_self(), 
					(vm_address_t) thread_table, 
					(vm_size_t) (table_size * sizeof(thread_t)));
                }
        }

        /*
         * If this is a ZOMBIE don't get the u_area and use defunct
         * as the command string.
         */
        if (pi->status == PI_ZOMBIE) {
                pi->args = pi->command = MSGSTR(ZOMBIE_STR, "<defunct>");
                return;
        }
        
        /*
         * Get u.area if needed
         */
        if (needuser) {
                if (table(TBL_UAREA, pi->pid, (char *)&u, 1, sizeof(u)) == 1) {
                        if ((pi->u = (struct user *)malloc(sizeof(struct user))) == NULL)
                                error(NO_MEM, "out of memory");
                        else
                                bcopy(&u, pi->u, sizeof(u));
                } else {
			/*
			 * Either we didn't find the process again (ESRCH) or
			 * kernel could not allocate more memory for copying
			 * uarea.  If ENOMEM, warn user and indicate the error
			 * by appending "<error>" to command string.
			 */
			pi->status = PI_EXITING;
			pi->u = NULL;				/* no uarea */
			if(errno == ENOMEM) {
				warn(NO_UAREA, "failed to get user info for process %d", pi->pid);
				pi->args = MSGSTR(ERROR_STR, "<error>");
				pi->command = realloc(pi->command,
				   strlen(pi->command) + strlen(pi->args)+2);
				strcat(pi->command, " ");
				strcat(pi->command, pi->args);
				pi->args = pi->command;
				return;
			}
		}
        }

        /*
         * If this is EXITING use exiting as command string
         */
        if (pi->status == PI_EXITING) {
                pi->args = pi->command = MSGSTR(EXITING_STR, "<exiting>");
                return;
        }

        /*
         * now go find the command arguments and environment
         */
        if (needcomm) {
                /*
                 *	Get command and arguments.
                 */
                int		arg_length, env_length;
                register char	*ap, *ep, *cp;

                bzero(arguments, arguments_size);
		bzero(cbuf, arguments_size);

                if (table(TBL_ARGUMENTS, pi->pid, arguments, 1, arguments_size) != 1) {
                        /* table command failed, use the short command */
                        sprintf(arguments, "[%s]", mproc->pi_comm);
                }
                ap = arguments;

                /* Find length of total arguments.
		 *..rjg.. Strings will have embedded nulls and some strings
		 * will end in spaces with no terminating NULL ..
		 */
                for (cp = &arguments[arguments_size]; (*cp == '\0' || *cp == ' ')
			&& cp > ap; --cp)
			;;
                arg_length = (cp - ap) + 1;

		if (arguments[arg_length]) {
		    /* ..rjg.. GAG.. Lost trailing null, force one on.
		     */
		    arguments[arg_length] = '\0';
		}
                /* Concat arguments.
                 * ..rjg.. ie. Remove embedded NULLs, TABs, NEWLINEs...
		 */
                for (np=cbuf,cp = ap; cp < &arguments[arg_length];np++) {
		    if ((*cp == '\0')||(*cp == '\n')||(*cp == '\t')||!(isprint(*cp)))
			*cp = ' ';
		    *np = *cp++;
		    while (((*np==' ')&&((*cp == ' ')||(*cp == '\0')||(*cp == '\n')||(*cp == '\t'))
			&& (cp < &arguments[arg_length])))
			cp++; 
		}

                arg_length = (np - cbuf) + 1;
		cbuf[arg_length] = '\0';
		ap = cbuf;

                if (ap[0] == '-' || ap[0] == '?' || ap[0] <= ' ') {
                        /*
                         *	Not enough information - add short command name
                         */
                        pi->args = malloc((unsigned)arg_length + (strlen(mproc->pi_comm) + 4));
			strcpy(pi->args,ap); 
                        (void) strcat(pi->args, " (");
			strcat(pi->args,mproc->pi_comm);
                        (void) strcat(pi->args, ")"); 
                }
                else { 
                        pi->args = malloc((unsigned)arg_length + 1 ); 
                        (void) strcpy(pi->args, ap);
                }
                /*
                 * Get environment string
                 */
                ep = &cbuf[arg_length];
                if (eflg) {
                    if (table(TBL_ENVIRONMENT, pi->pid, ep, 1, (arguments_size-arg_length)) == 1) {
		        /* Find length of total environment strings
			 */
                         for (cp = &cbuf[arguments_size];
				!(isprint(*cp)) && (cp > ep); --cp)
				;;
                         env_length = (cp - ep) + 1;

                         /* Concat arguments.
                          *..rjg.. Remove  NULLS BETWEEN  environment variables.
			  */
                          for (cp = ep; cp <  &ep[env_length]; cp++) {
			    if (*cp == '\0')
                               *cp = ' ';
			    else if (!(isprint(*cp))) *cp = '_';
			  };

                                /* Add to agument string */
                                pi->args = realloc(pi->args, ((arg_length+1) + (env_length+1)));
				(void) strcat(pi->args, " ");
                                (void) strcat(pi->args, ep);
                        }
                }
        }
}

char *
savestr(char *cp)
{
	register unsigned len;
	register char *dp;

	len = strlen(cp);
	dp = (char *)calloc(len+1, sizeof (char));
	(void) strcpy(dp, cp);
	return (dp);                  
}

void
savefmt(char *cp, char **dp)
{
	/* cp is the string to save */
	/* dp is the pointer to saved area */

	register unsigned len;
	register unsigned oldlen;

	len = strlen(cp);
	if (*dp == (char *) NULL) {
		*dp = (char *)calloc(len+1, sizeof (char));
		(void) strcpy(*dp, cp);
	}
	else {
		oldlen = strlen(*dp);
		*dp = (char *) realloc(*dp, (len+oldlen+2) * sizeof(char));
		(void) strcat(*dp, " ");
		(void) strcat(*dp, cp);
	}
	return;                  
}

void
error(int msg, char *dfltfmt, ...)
{
	va_list printargs;

	fprintf(stderr, "ps: ");

	va_start(printargs, dfltfmt);
	vfprintf(stderr, MSGSTR(msg, dfltfmt), printargs);
	va_end(printargs);

	fprintf(stderr, "\n");
	exit(1);
}

void
warn(int msg, char *dfltfmt, ...)
{
	va_list printargs;

	if(!rflg)
		return;

	fprintf(stderr, "ps: ");

	va_start(printargs, dfltfmt);
	vfprintf(stderr, MSGSTR(msg, dfltfmt), printargs);
	va_end(printargs);

	fprintf(stderr, "\n");
	return;
}

void
syserror(char *a)
{
	error("%s: %s", a, strerror(errno));
}

/*
 * ICK (all for getopt), would rather hide the ugliness
 * here than taint the main code.
 *
 *  ps foo -> ps -foo
 *  ps 34 -> ps -p34
 *
 * The old convention that 't' with no trailing tty arg means the users
 * tty, is only supported if argv[1] doesn't begin with a '-'.  This same
 * feature is available with the option 'T', which takes no argument.
 */
char *
kludge_bsdps_options(char *s)
{
	int len = strlen(s), numlen = 0;
	char *newopts, *ns, *cp;

	if ((newopts = ns = (char *)calloc(len+3 /*..rjg.. was 2*/, 1)) == NULL)
		error(NO_MEM, "out of memory");
	/*
	 * options begin with '-'
	 */
	if (*s != '-')
		*ns++ = '-';	/* add option flag */
	/*
	 * gaze to end of argv[1]
	 */
	cp = s + len - 1;
	/*
	 * if last letter is a 't' flag with no argument (in the context
	 * of the oldps options -- option string NOT starting with a '-' --
	 * then convert to 'T' (meaning *this* terminal, i.e. ttyname(0).
	 */
	if (*cp == 't' && *s != '-')
		*cp = 'T';
	else {
		/*
		 * otherwise check for trailing number, which *may* be a
		 * pid.
		 */
		while (isdigit(*cp)) {
			--cp;
			numlen++;
		}
	}
	cp++;
	bcopy(s, ns, cp - s);	/* copy everything up to trailing number */
	while (*ns)
		ns++;
	/*
	 * if there's a trailing number, and not a preceding 'p' (pid) or
	 * 't' (tty) flag, then assume it's a pid and insert a 'p' flag.
	 */
	if (isdigit(*cp) && (cp == s || *(cp-1) != 't' && *(cp-1) != 'p' &&
	   ((cp-1) == s || *(cp-2) != 't')))
		*ns++ = 'p';
	strcat(ns, cp);		/* and append the number */

	return (newopts);
}
