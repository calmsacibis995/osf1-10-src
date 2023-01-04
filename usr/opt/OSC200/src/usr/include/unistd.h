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
 *	@(#)$RCSfile: unistd.h,v $ $Revision: 4.2.14.11 $ (DEC) $Date: 1993/12/15 22:14:37 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 * 
 * unistd.h	1.17  com/inc,3.1,8943 9/22/89 15:13:11
 */

#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <standards.h>
#include <sys/types.h>  /* for time_t and size_t */
#include <sys/access.h>	/* for the "access" function */

/*
 * POSIX requires that certain values be included in unistd.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */

#ifdef _POSIX_SOURCE
/* Symbolic constants for the "lseek" function: */

#include <sys/seek.h>
#include <getopt.h>

/***AAB-XPG4*** must protect from XPG4 ***/
#ifdef _OSF_SOURCE
extern char **environ;          /* Environment description variable */
#endif /*OSF_SOURCE*/


#ifdef _NO_PROTO

extern int access();
extern unsigned int alarm();
extern int chdir();
extern int chown();
extern int chroot();
extern int close();
extern size_t confstr();
#ifndef _XOPEN_SOURCE
extern char *cuserid();
#endif
extern int dup();
extern int dup2();
extern int execl();
extern int execv();
extern int execle(); 
extern int execve();
extern int execlp();
extern int execvp();
extern void _exit();
extern pid_t fork();
extern long fpathconf();
extern char *getcwd();
extern gid_t getegid();
extern uid_t geteuid();
extern gid_t getgid();
extern int getgroups();
extern char *getlogin();
extern pid_t getpgrp();
extern pid_t getpid();
extern pid_t getppid();
extern uid_t getuid();
extern int isatty();
extern int link();
extern off_t lseek();
extern long pathconf();
extern int pause();
extern int pipe();
extern int read();
extern int rmdir();
extern int setgid();
extern int setpgid();
extern int setsid(); 
extern int setuid();
extern unsigned int sleep();
extern long sysconf();
extern pid_t tcgetpgrp();
extern int tcsetpgrp();
extern char *ttyname();
extern int unlink();
extern int write(); 

/* POSIX REENTRANT FUNCTIONS */
#if defined(_THREAD_SAFE) || defined(_REENTRANT)
extern int getlogin_r();		/* _POSIX_REENTRANT_FUNCTIONS */
extern int ttyname_r();		/* _POSIX_REENTRANT_FUNCTIONS */
#endif

#else		/* POSIX required prototypes */
#if defined(__STDC__) || defined(__cplusplus)
_BEGIN_CPLUSPLUS
extern int access(const char *, int );
extern unsigned int alarm(unsigned int );
extern int chdir(const char *);
extern int chown(const char *, uid_t , gid_t );
extern int chroot(const char *);
extern int close(int );
extern size_t confstr(int, char *, size_t);
#ifndef _XOPEN_SOURCE
extern char *cuserid(char *);
#endif
extern int dup(int );
extern int dup2(int , int );

/***AAB-XPG4 Parameter names not allowed in ANSI-C ***/
extern int execl(const char *, const char *, ...);
extern int execv(const char *, char *const[]);
extern int execle(const char *, const char *, ...); 
extern int execve(const char *, char *const[], char *const[]);
extern int execlp(const char *, const char *, ...); 
extern int execvp(const char *, char *const[]);
/*AAB_XPG4*/

extern void _exit(int );
extern pid_t fork(void);
extern long fpathconf(int , int );
extern int fsync(int);
extern char *getcwd(char *, size_t );
extern gid_t getegid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);

/***AAB-XPG4 Parameter names not allowed in ANSI-C ***/
extern int getgroups(int , gid_t []);

extern char *getlogin(void);
extern pid_t getpgrp(void);
extern pid_t getpid(void);
extern pid_t getppid(void);
extern uid_t getuid(void);
extern int isatty(int );
extern int link(const char *, const char *);
extern off_t lseek(int , off_t , int );
extern long pathconf(const char *, int );
extern int pause(void);

/***AAB-XPG4 Parameter names not allowed in ANSI-C ***/
extern int pipe(int []);

extern ssize_t read(int , void *, size_t);
extern int rmdir(const char *);
extern int setgid(gid_t );
extern int setpgid(pid_t , pid_t );
extern pid_t setsid(void);
extern int setuid(uid_t );
extern unsigned int sleep(unsigned int );
extern long sysconf(int );
extern pid_t tcgetpgrp(int );
extern int tcsetpgrp(int , pid_t );
extern char *ttyname(int );
extern int unlink(const char *);
extern ssize_t write(int , const void *, size_t); 
#if !defined(__cplusplus) || !defined(_STRING_H_)
extern void swab(const void *, void *, ssize_t);
#endif

/* POSIX REENTRANT FUNCTIONS */
#if defined(_THREAD_SAFE) || defined(_REENTRANT)
extern int getlogin_r(char *, int);		/* _POSIX_REENTRANT_FUNCTIONS */
extern int ttyname_r(int, char *, int);	/* _POSIX_REENTRANT_FUNCTIONS */
#endif

_END_CPLUSPLUS
#endif
#endif		/* !_NO_PROTO	*/

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#define _POSIX_VERSION		199009L

#define _POSIX_CHOWN_RESTRICTED	 0 /* chown requires appropriate privileges */
#define _POSIX_NO_TRUNC	     	 0 /* too-long path components gen errors */
#define _POSIX_VDISABLE		0377

#define	_POSIX_THREADS		/* 1003.4a (pthreads) comformant */
#define _POSIX_THREAD_ATTR_STACKSIZE /* support for specifiable stack sizes */
#define	_POSIX_REENTRANT_FUNCTIONS  /* multithreaded 1003.1 interfaces */
#define _POSIX_JOB_CONTROL       1  /* implementation supports job control */
#define _POSIX_SAVED_IDS         1  /* saved set-user-id and set-group-id */

#define _POSIX2_C_BIND     1  /* supports the C language binding */
#define _POSIX2_C_DEV      1  /* supports the C development env  */
#define _POSIX2_LOCALEDEF  1  /* supports creation of lacales with localedef */
#define _POSIX2_SW_DEV     1  /* supports software development */
#define _POSIX2_CHAR_TERM  1  /* supports a least one terminal */
#define _POSIX2_UPE        1  /* supports the User Portability Environment */

#undef _POSIX2_FORT_DEV       /* no support for FORTRAN developemnt */
#undef _POSIX2_FORT_RUN       /* no support for FORTRAN runtime */

#define _POSIX2_C_VERSION     199209L /* Support ISO POSIX-2 */
#define _POSIX2_VERSION       199209L /* POSIX.2 standard */


#ifdef _POSIX_4SOURCE
/*
 * POSIX 1003.4 Feature Test Macros
 * Based on POSIX 1003.4 Draft 11 (Oct. 7, 1991).
 */

/* Supported features */
#define _POSIX4_VERSION 199110L
#define _POSIX_ASYNCHRONOUS_IO
#define _POSIX_BINARY_SEMAPHORES
#define _POSIX_FSYNC
#define _POSIX_MAPPED_FILES
#define _POSIX_MEMLOCK
#define _POSIX_MEMLOCK_RANGE
#define _POSIX_MEMORY_PROTECTION
#define _POSIX_PRIORITY_SCHEDULING
#define _POSIX_SHARED_MEMORY_OBJECTS
#define _POSIX_TIMERS

#ifdef POSIX_4D10
/* Draft 10 compatibility */
#define _POSIX_MEMLK
#define _POSIX_MEMLK_RANGE
#define _POSIX_PRIORITIZED_IO
#endif /* POSIX_4D10 */

/*
 * NOTE: _SC_PAGESIZE for sysconf() is defined both in Posix 1003.4, draft 11,
 * and in AES, below. Keep same as AES definition.
 */
#ifndef _AES_SOURCE
#define _SC_PAGESIZE		43
#endif /* ifndef _AES_SOURCE */

/* Realtime arguments for the sysconf() function */
#define _SC_4VERSION		100
#define _SC_LISTIO_AIO_MAX	101
#define _SC_AIO_MAX		102
#define _SC_CLOCKDRIFT_MAX	103
#define _SC_DELAYTIMER_MAX	104
#define _SC_RTSIG_MAX		105
#define _SC_SEM_NAME_MAX	106
#define _SC_SEM_NSEMS_MAX	107
#define _SC_SEM_NSETS_MAX	108
#define _SC_TIMER_MAX		109
#define _SC_ASYNCHRONOUS_IO	110
#define _SC_BINARY_SEMAPHORES	111
#define _SC_FSYNC		112
#define _SC_MAPPED_FILES	113
#define _SC_MEMLOCK		114
#define _SC_MEMLOCK_RANGE	115
#define _SC_MEMORY_PROTECTION	116
#define _SC_MESSAGE_PASSING	117
#define _SC_PRIORITIZED_IO	118
#define _SC_PRIORITY_SCHEDULING	119
#define _SC_REALTIME_FILES	120
#define _SC_REALTIME_SIGNALS	121
#define _SC_SHARED_MEMORY_OBJECTS 122
#define _SC_SYNCHRONIZED_IO	123
#define _SC_TIMERS		124

#endif /* _POSIX_4SOURCE */

#ifndef NULL
#define NULL	0L
#endif


/* arguments for the confstr() function */

#define	_CS_PATH		1
#define _CSPATH			"/usr/bin"

/* arguments for the sysconf() function */

#define _SC_ARG_MAX		1
#define _SC_CHILD_MAX		2
#define _SC_CLK_TCK		3
#define _SC_JOB_CONTROL		4
#define _SC_NGROUPS_MAX		5
#define _SC_OPEN_MAX		6
#define _SC_SAVED_IDS		8
#define _SC_VERSION		9
#define _SC_STREAM_MAX		13
#define _SC_TZNAME_MAX		14
#define _SC_BC_BASE_MAX         15
#define _SC_BC_DIM_MAX          16
#define _SC_BC_SCALE_MAX        17
#define _SC_BC_STRING_MAX       18
#define _SC_COLL_WEIGHTS_MAX    20
#define _SC_EXPR_NEST_MAX       21
#define _SC_LINE_MAX            22
#define _SC_2_C_DEV             23
#define _SC_2_FORT_DEV          24
#define _SC_2_FORT_RUN          25
#define _SC_2_LOCALEDEF         26
#define _SC_2_SW_DEV            27
#define _SC_2_VERSION           28
#define _SC_RE_DUP_MAX          29
#define _SC_THREAD_MAX          34
#define _SC_TASK_MAX            35
#define _SC_MAXUTHREADS         36
#define _SC_STACK_SIZE          37
#define _SC_STACK_SIZE_MAX      38
#define _SC_DATA_SIZE           39
#define _SC_DATA_SIZE_MAX       40
#define _SC_ADDR_SPACE          41
#define _SC_ADDR_SPACE_MAX      42
#define _SC_2_UPE               47

#ifdef _XOPEN_SOURCE

#define _SC_XOPEN_CRYPT         48
#define _SC_XOPEN_ENH_I18N      49
#define _SC_XOPEN_SHM           50
#define _SC_XOPEN_XCU_VERSION   51
#define _SC_2_C_BIND            52
#endif

#define _SC_2_C_VERSION         53
#define _SC_2_CHAR_TERM         54



/* arguments for the pathconf() function */

#define _PC_CHOWN_RESTRICTED	10
#define _PC_LINK_MAX		11
#define _PC_MAX_CANON		12
#define _PC_MAX_INPUT		13
#define _PC_NAME_MAX		14
#define _PC_NO_TRUNC		15
#define _PC_PATH_MAX		16
#define _PC_PIPE_BUF		17
#define _PC_VDISABLE 		18

#endif /* _POSIX_SOURCE */

#ifdef _XOPEN_SOURCE

#define _SC_PASS_MAX		7
#define _SC_XOPEN_VERSION	19

#define _XOPEN_VERSION		4
#define _XOPEN_XCU_VERSION	4
#define _XOPEN_XPG4		4

#define	_XOPEN_CRYPT		1
#define _XOPEN_ENH_I18N		1 /* Supports enhanced internationalization */
#define _XOPEN_SHM		1 /* Supports Shared Memory Feature Group */

/*** AAB-XPG4 Illegal symbols ***/
#ifdef _OSF_SOURCE
#define F_ULOCK	0	/* Unlock a previously locked region */
#define F_LOCK	1	/* Lock a region for exclusive use */
#define F_TLOCK	2	/* Test and lock a region for exclusive use */
#define F_TEST	3	/* Test a region for other processes locks */
#endif /*_OSF_SOURCE*/
/*AAB-XPG4*/

#endif /* _XOPEN_SOURCE */

#ifdef _AES_SOURCE

#define _SC_ATEXIT_MAX		10
#define _SC_PAGE_SIZE		11
#define _SC_AES_OS_VERSION	12
#define _SC_PAGESIZE		43  /* NOTE: keep synch with realtime, above */

#define _AES_OS_VERSION		1

#ifdef _NO_PROTO

extern int  fchown();
extern int  fsync();
extern int  ftruncate();
extern int  readlink();
extern int  setgroups();
extern int  symlink();
extern int  truncate();
#else
#if defined(__STDC__) || defined(__cplusplus)
_BEGIN_CPLUSPLUS
extern int  fchown(int, uid_t, gid_t);
extern int  ftruncate(int, off_t);
extern int  readlink(const char *, char *, int);
extern int  setgroups(int, gid_t []);
extern int  symlink(const char *, const char *);
extern int  truncate(const char *, off_t);
_END_CPLUSPLUS
#endif
#endif

#endif 	/* _AES_SOURCE */

#ifdef _XOPEN_SOURCE
#ifdef _NO_PROTO
extern char *crypt();
extern char *cuserid();
extern char *ctermid();
extern void encrypt();
extern char *getpass();
extern int  nice();

#else
#if defined(__STDC__) || defined(__cplusplus)
_BEGIN_CPLUSPLUS
extern char *crypt(const char *, const char *);
extern char *cuserid(char *);
extern char *ctermid(char *);
extern void encrypt(char [], int);
extern char *getpass(const char *);
extern int  nice(int);
_END_CPLUSPLUS
#endif
#endif /* _NO_PROTO */

#define _SC_MAJ_NUM_SHIFT          53

#endif /* _XOPEN_SOURCE */

/*** AAB-XPG4 The protos below were removed from above since XPG4
 does not allow them to be visible. They were put here just in case
 they are needed for compilation. Should remove them if not needed ***/

#ifdef _OSF_SOURCE
#ifdef _NO_PROTO

/***from POSIX_SOURCE above***/
extern char *getenv();
extern int rename();

/***from XOPEN_SOURCE above**/
extern int  brk();
extern char *sbrk();
extern int  getpagesize();
extern pid_t getsid();
extern char *getwd();
extern int  seteuid();
extern int  setegid();
extern int  setlogin();
extern pid_t setpgrp();
extern int  setregid();
extern int  setreuid();
extern int  setrgid();
extern int  setruid();
extern unsigned int ualarm();

#else
#if defined(__STDC__) || defined(__cplusplus)
_BEGIN_CPLUSPLUS
/***from POSIX_SOURCE above***/
extern char *getenv(const char *);
extern int rename(const char *, const char *);

/***from XOPEN_SOURCE above**/
extern int  brk(char *);
extern char *sbrk(ssize_t);
extern int  getpagesize(void);
extern pid_t getsid(pid_t);
extern char *getwd(char *);
extern int  seteuid(uid_t);
extern int  setegid(gid_t);
extern int  setlogin(char *);
extern pid_t setpgrp(void);
extern int  setregid(gid_t, gid_t);
extern int  setreuid(uid_t, uid_t);
extern int  setrgid(gid_t);
extern int  setruid(uid_t);
extern unsigned int ualarm(unsigned int, unsigned int);
_END_CPLUSPLUS
#endif /*__STDC__*/
#endif /*_NO_PROTO*/
#endif /*_OSF_SOURCE*/
#endif /* _UNISTD_H_ */
