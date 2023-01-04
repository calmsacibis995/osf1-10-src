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
static char	*sccsid = "@(#)$RCSfile: llib-lc.c,v $ $Revision: 4.2.8.5 $ (DEC) $Date: 1993/07/30 19:11:34 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilities
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27; 10
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * llib-lc	1.2  R2/cmd/prog/lint,3.1,8943 10/16/89 10:30:32 
 */

#define _NO_PROTO

/*NOTUSED*/
/*NOTDEFINED*/

#include <stdio.h>
#undef fileno
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pwd.h>
#include <unistd.h>
#include <NLchar.h>

#define const /* */

/* ACCESS(2) */
int	access(f, m) char *f; { return (m); }
/* ACCT(2) */
int	acct(f) char *f; { return (0); }
/* ALARM(2) */
unsigned alarm(s) unsigned s; { return (s); }
/* BRK(2) */
int	brk(e) char *e; { return (0); }
char *	sbrk(i) int i; { return (""); }
/* CHDIR(2) */
int	chdir(s) char *s; { return (0); }
/* CHMOD(2) */
int	chmod(s, m) char *s; mode_t m; { return (m); }
/* CHOWN(2) */
int	chown(s, o, g) char *s; uid_t o; gid_t g; { return (o); }
/* CHROOT(2) */
int	chroot(p) char *p; { return (0); }
/* CLOSE(2) */
int	close(f) int f; { return (f); }
/* CREAT(2) */
int	creat(s, m) char *s; mode_t m; { return (0); }
/* DUP(2) */
int	dup(f) int f; { return (f); }
int	dup2(o,n) int o,n; { return (n); }
/* EXEC(2) */
char **	environ;
	/*VARARGS1*/
int	execl(f, a) char *f, *a; { return (0); }
int	execv(s, v) char *s, *v[]; { return (0); }
	/*VARARGS1*/
int	execle(f, a, e) char *f, *a, *e[]; { return (0); }
int	execve(f, a, e) char *f, *a[], *e[]; { return (0); }
	/*VARARGS1*/
int	execlp(f, a) char *f, *a; { return (0); }
int	execvp(s, v) char *s, *v[]; { return (0); }
/* EXIT(2) */
void	exit(s) int s; {}
void	_exit(s) int s; {}
/* FCLEAR(2) */
long	fclear(f, n) int f; unsigned long n; { return (n); }
/* FCNTL(2) */
int	fcntl(f, c, a) int f, c, a; { return (f); }
/* FORK(2) */
pid_t	fork() { return (0); }
/* FSYNC(2) */
int	fsync(f) int f; { return (f); }
/* FTRUNCATE(2) */
int	ftruncate(f,n) int f; unsigned long n; { return (f); }
/* GETGROUPS(2) */
int	getgroups(n,g) int n; gid_t g[]; { return (n); }
/* GETPID(2) */
pid_t	getpid() { return (0); }
pid_t	getpgrp() { return (0); }
pid_t	getppid() { return (0); }
/* GETUID(2) */
uid_t	getuid() { return (0); }
uid_t	geteuid() { return (0); }
gid_t	getgid() { return (0); }
gid_t	getegid() { return (0); }
/* IOCTL(2) */
	/*VARARGS2*/
int	ioctl(f, r, a) int f, r; char *a; { return (f); }
/* KILL(2) */
int	kill(i, s) pid_t i; int s; { return (i); }
/* LINK(2) */
int	link(a, b) char *a, *b; { return (0); }
/* LOCKF(2) */
int	lockf(f,o,n) int f, o; off_t n; { return (0); }
/* LSEEK(2) */
off_t	lseek(f, o, w) int f, w; off_t o; { return (o); }
/* MKNOD(2) */
int	mknod(n, m, d) char *n; int m; dev_t d; { return (m); }
/* MOUNT(2) */
int	mount(s, d, r) char *s, *d; int r; { return (r); }
/* MSGCTL(2) */
int	msgctl(m, c, b) int m, c; struct msqid_ds *b; { return (m); }
/* MSGGET(2) */
int	msgget(k, m) key_t k; int m; { return (m); }
/* MSGOP(2) */
int	msgsnd(q, p, s, f) int q, f; void *p; size_t s; { return (q); }
int	msgrcv(q, p, s, t, f) int q, f; void *p; size_t s; long t;
		{ return (q); }
int	msgxrcv(q, p, s, t, f) int q, s,f; struct msgxbuf *p; long t;
		{ return (q); }
/* NICE(2) */
int	nice(i) int i; { return (i); }
/* OPEN(2) */
	/*VARARGS2*/
int	open(f, o, m) char *f; int o, m; { return (o); }
/* PAUSE(2) */
int	pause() { return (0); }
/* PIPE(2) */
int	pipe(f) int f[2]; { return (0); }
/* PLOCK(2) */
int	plock(o) int o; { return (o); }
/* PROFIL(2) */
void	profil(b, s, o, i) short *b; unsigned s, o, i; {}
/* PTRACE(2) */
int	ptrace(r, i, a, d, b) int r, i, d; int *a, *b; { return (r); }
/* READ(2) */
#ifdef	_NONSTD_TYPES
int	read(f, b, n) int f; char *b; unsigned n; { return (f); }
#else
ssize_t	read(f, b, n) int f; char *b; unsigned n; { return (f); }
#endif
/* REBOOT(2) */
int	reboot(s) char *s; { return (0); }
/* SEMCTL(2) */
	/*VARARGS3*/
int	semctl(i, n, c, a) int i, n, c, a; { return (i); }
/* SEMGET(2) */
int	semget(k, n, s) key_t k; int n, s; { return (n); }
/* SEMOP(2) */
int	semop(i, o, n) int i; struct sembuf *o; unsigned n; { return (i); }
/* SETGROUPS(2) */
int	setgroups(n,p) int n; int *p; { return(n); }
/* SETPGRP(2) */
int	setpgrp() { return (0); }
/* SETUID(2) */
int	setuid(u) uid_t u; { return (u); }
int	setgid(g) gid_t g; { return (g); }
/* SHMCTL(2) */
#include <sys/shm.h>
int	shmctl(s, c, b) int s, c; struct shmid_ds *b; { return (s); }
/* SHMGET(2) */
int	shmget(k, s, f) key_t k; int s, f; { return (s); }
/* SHMOP(2) */
char *	shmat(i, a, f) int i, f; char *a; { return (a); }
int	shmdt(a) char *a; { return (0); }
/* SIGBLOCK(2) */
int	sigblock(m) int m; { return(0L); }
/* SIGNAL(2) */
#include <signal.h>
void (*	signal(s, a))() int s; void (*a)(); { return (a); }
/* SIGSET(2) */
void (*	sigset(s, a))() int s; void (*a)(); { return (a); }
/* SIGIGNORE(2) */
int	sigignore(s) int s; { return( (void *) 0); }
/* SIGPAUSE(2) */
int	sigpause(m) int m; { }
/* SIGSETMASK(2) */
int	sigsetmask(m) int m; { return(0L); }
/* SIGHOLD(2) */
int	sighold(s) int s; { return (s); }
/* SIGRELSE(2) */
int	sigrelse(s) int s; { return (s); }
/* SIGSTACK(2) */
int	sigstack(ss, oss) struct sigstack *ss, *oss; { return(0); }
/* SIGVEC(2) */
int	sigvec(s, nv, ov) int s; struct sigvec *nv, *ov; { return (0); }
/* STAT(2) */

/* #include <sys/fcntl.h>
*  #include <sys/flock.h>
*/

#include <sys/stat.h>
int	stat(s, b) char *s; struct stat *b; { return (0); }
int	fstat(f, b) int f; struct stat *b; { return (f); }
int	lstat(s, b) char *s; struct stat *b; { return (0); }
/* STIME(2) */
long	stime(t) long *t; { return (0); }
/* SYNC(2) */
void	sync() {}
/* TIME */
time_t	time(t) time_t *t; { return (0L);}
/* TIMES(2) */
#include <sys/times.h>
clock_t	times(b) struct tms *b; { return (0L); }
/* ULIMIT(2) */
off_t	ulimit(c, n) int c; off_t n; { return (n); }
/* UMASK(2) */
mode_t	umask(c) mode_t c; { return (c); }
/* UMOUNT(2) */
int	umount(s) char *s; { return (0); }
/* UNAME(2) */
#include <sys/utsname.h>
int	uname(n) struct utsname *n; { return (0); }
/* UNLINK(2) */
int	unlink(s) char *s; { return (0); }
/* USRINFO(2) */
int	usrinfo(c, b, n) int c, n; char *b; { return (0); }
/* USTAT(2) */
#include <ustat.h>
int	ustat(d, b) dev_t d; struct ustat *b; { return (d); }
/* UTIME(2) */
int	utime(f, t) char *f; struct { time_t x, y; } *t; { return (0); }
/* WAIT(2) */
int	wait(s) int *s; { return (0); }
/* WRITE(2) */
#ifdef	_NONSTD_TYPES
int	write(f, b, n) int f; const void *b; size_t n; { return (f); }
#else
ssize_t	write(f, b, n) int f; const void *b; size_t n; { return (f); }
#endif

/* A64L(3C) */
long	a64l(s) char *s; { return (0L); }
char *	l64a(l) long l; { return (""); }
/* ABORT(3C) */
void	abort() {}
/* ABS(3C) */
int	abs(i) int i; { return (i); }
/* ASSERT(3X) */
void	_assert(a, f, n) char *a, *f; int n; {}
/* BSEARCH(3C) */
void *	bsearch(k, b, n, w, c) const void *k, *b; size_t n, w; int (*c)();
	{ return (k); }
/* CONV(3C) */
#undef _toupper
#undef _tolower
#undef toascii
int	toupper(i) int i; { return (i); }
int	tolower(i) int i; { return (i); }
int	_toupper(i) int i; { return (i); }
int	_tolower(i) int i; { return (i); }
int	toascii(i) int i; { return (i); }
/* CLOCK(3C) */
clock_t	clock() { return (0L); }
/* CRYPT(3C) */
char *	crypt(k, s) char *k, *s; { return (k); }
void	setkey(k) const char *k; {}
void	encrypt(b, f) char *b; int f; {}
/* CTERMID(3S) */
char *	ctermid(s) char *s; { return (s); }
/* CTIME(3C) */
char *	ctime(t) const time_t *t; { return (""); }
#include <time.h>
struct tm *	localtime(c) const time_t *c;
	{ static struct tm x; return (&x); }
struct tm *	gmtime(c) const time_t *c; { static struct tm x; return (&x); }
char *	asctime(t) const struct tm *t; { return (""); }
void	tzset() {}
long  timezone;
int	daylight;
char *	tzname[2];
unsigned char *	NLctime(t) long *t; { return ((unsigned char *)""); }
unsigned char *	NLasctime(t) struct tm *t; { return ((unsigned char *)""); }
/* CTYPE(3C) */
#undef isalpha
#undef isupper
#undef islower
#undef isdigit
#undef isxdigit
#undef isalnum
#undef isspace
#undef ispunct
#undef isprint
#undef isgraph
#undef iscntrl
#undef isascii
int	isalpha(c) int c; { return (c); }
int	isupper(c) int c; { return (c); }
int	islower(c) int c; { return (c); }
int	isdigit(c) int c; { return (c); }
int	isxdigit(c) int c; { return (c); }
int	isalnum(c) int c; { return (c); }
int	isspace(c) int c; { return (c); }
int	ispunct(c) int c; { return (c); }
int	isprint(c) int c; { return (c); }
int	isgraph(c) int c; { return (c); }
int	iscntrl(c) int c; { return (c); }
int	isascii(c) int c; { return (c); }
char	_ctype[129];
/* CUSERID(3S) */
char *	cuserid(s) char *s; { return (s); }
/* DRAND48(3C) */
double	drand48() { return (0.0); }
double	erand48(x) unsigned short x[3]; { return (0.0); }
long	lrand48() { return (0L); }
long	nrand48(x) unsigned short x[3]; { return (0L); }
long	mrand48() { return (0L); }
long	jrand48(x) unsigned short x[3]; { return (0L); }
void	srand48(s) long s; {}
unsigned short *	seed48(s) unsigned short s[3]; { return (s); }
void	lcong48(p) unsigned short p[7]; {}
/* ECVT(3C) */
char *	ecvt(v, n, d, s) double v; int n; int *d, *s; { return (""); }
char *	fcvt(v, n, d, s) double v; int n; int *d, *s; { return (""); }
char *	gcvt(v, n, b) double v; int n; char *b; { return (b); }
/* END(3C) */
int	end, etext, edata;
/* FCLOSE(3S) */
int	fclose(f) FILE *f; { return (0); }
int	fflush(f) FILE *f; { return (0); }
/* FERROR(3S) */
#undef feof
#undef ferror
#undef clearerr
#undef fileno
int	feof(f) FILE *f; { return (0); }
int	ferror(f) FILE *f; { return (0); }
void	clearerr(f) FILE *f; {}
int	fileno(f) FILE *f; { return (0); }
/* FOPEN(3S) */
FILE *	fopen(f, t) const char *f, *t; { return (stdin); }
FILE *	freopen(f, t, s) const char *f, *t; FILE *s; { return (s); }
FILE *	fdopen(f, t) int f; char *t; { return (stdin); }
/* FPTRAP(3X) */
/* FREAD(3S) */
#ifdef	_NONSTD_TYPES
int	fread(b, s, n, f) void *b; size_t s, n; FILE *f; { return (n); }
int	fwrite(b, s, n, f) const void *b; size_t s, n; FILE *f; { return (n); }
#else
size_t	fread(b, s, n, f) void *b; size_t s, n; FILE *f; { return (n); }
size_t	fwrite(b, s, n, f) const void *b; size_t s, n; FILE *f; { return (n); }
#endif
/* FREXP(3C) */
double	frexp(x, e) double x; int *e; { return (x); }
double	ldexp(v, e) double v; int e; { return (v); }
double	modf(v, i) double v, *i; { return (v); }
/* FSEEK(3S) */
int	fseek(f, o, p) FILE *f; long o; int p; { return (p); }
long	ftell(f) FILE *f; { return (0L); }
void	rewind(f) FILE *f; {}
/* FTW(3C) */
int	ftw(p, f, d) const char *p; int (*f)(); int d; { return (d); }
/* GETC(3S) */
#undef getc
#undef getchar
int	getc(f) FILE *f; { return (0); }
int	getchar() { return (0); }
int	fgetc(f) FILE *f; { return (0); }
int	getw(f) FILE *f; { return (0); }
/* GETCWD(3C) */
char *	getcwd(b, s) char *b; size_t s; { return (b); }
/* GETENV(3C) */
char *	getenv(n) const char *n; { return (""); }
char *	NLgetenv(name) char *name; { return (name); }
/* GETGRENT(3C) */
#include <grp.h>
struct group *	getgrent() { static struct group x; return (&x); }
struct group *	getgrgid(g) gid_t g; { static struct group x; return (&x); }
struct group *	getgrnam(n) char *n; { static struct group x; return (&x); }
int	setgrent() { return (1);}
void	endgrent() {}
/* GETLOGIN(3C) */
char *	getlogin() { return (""); }
/* GETOPT(3C) */
int	getopt(c, v, o) int c; char **v, *o; { return (c); }
char *	optarg;
int	optind;
int	opterr;
int	optopt; /* undocumented */
/* GETPASS(3C) */
char *	getpass(s) char *s; { return (s); }
/* GETPW(3C) */
int	getpw(u, b) uid_t u; char *b; { return (u); }
/* GETPWENT(3C) */
struct passwd *	getpwent() { static struct passwd x; return (&x); }
struct passwd *	getpwuid(u) uid_t u; { static struct passwd x; return (&x); }
struct passwd *	getpwnam(n) char *n; { static struct passwd x; return (&x); }
int	setpwent() { return (1);}
void	endpwent() {}
void	setpwfile(f) const char *f; {}
/* GETS(3S) */
char *	gets(s) char *s; { return (s); }
char *	fgets(s, n, f) char *s; int n; FILE *f; { return (s); }
/* HSEARCH(3C) */
#include <search.h>
ENTRY *	hsearch(i, a) ENTRY i; ACTION a; { return (&i); }
int	hcreate(n) unsigned n; { return (0); }
void	hdestroy() {}
/* L3TOL(3C) */
void	l3tol(l, c, n) long *l; char *c; int n; {}
void	ltol3(c, l, n) char *c; long *l; int n; {}
/* LD...(3X) libld -- shouldn't be documented in Section 3! */
/* LOGNAME(3X) libPW -- shouldn't be documented */
/* LSEARCH(3C) */
void *	lsearch(k, b, n, w, c) void *k, *b; size_t *n, w; int (*c)();
	{ return (k); }
void *	lfind(k, b, n, w, c) void *k, *b; size_t *n, w; int (*c)();
	{ return (k); }
/* MALLOC(3C) */
void *	malloc(s) size_t s; { return (""); }
void	free(s) char *s; {}
void *	realloc(s, n) char *s; size_t n; { return (s); }
void *	calloc(n, e) size_t n, e; { return (""); }
void	cfree(p,n,s) void *p; unsigned n,s; {}
#include <malloc.h>
int	mallopt(c,v) int c,v; { return (0); }
struct mallinfo mallinfo(void) { struct mallinfo x; return x; }
/* MEMORY(3C) */
#include <string.h>
/* MEMORY(3C) */
#ifdef	_NONSTD_TYPES
char *	memccpy(a, b, c, n) void *a, *b; int c; size_t n; { return (a); }
char *	memchr(s, c, n) const void *s; int c; size_t n; { return (s); }
char *	memcpy(a, b, n) char *a, *b; size_t n; { return (a); }
char *	memset(s, c, n) char *s; int c; size_t n; { return (s); }
#else
void *	memccpy(a, b, c, n) void *a, *b; int c; size_t n; { return (a); }
void *	memchr(s, c, n) const void *s; int c; size_t n; { return (s); }
void *	memcpy(a, b, n) char *a, *b; size_t n; { return (a); }
void *	memset(s, c, n) char *s; int c; size_t n; { return (s); }
#endif /* _NONSTD_TYPES */
int	memcmp(a, b, n) char *a, *b; size_t n; { return (n); }
void	*memmove(s1,s2,n) void *s1; const void *s2; size_t n; { return s1; }
/* MKTEMP(3C) */
char *	mktemp(t) char *t; { return (t); }
/* MONITOR(3C) */
#include <mon.h>
int	monitor(l, h, b, s, n) char *l, *h; short *b; int s, n; {}
int	monstartup(l,h) char *l,*h; { return (0); }
/* NCSTRING(3C) */
NLchar *	NCstrcat(a, b) NLchar *a, *b; { return (a); }
NLchar *	NCstrncat(a, b, n) NLchar *a, *b; int n; { return (a); }
int	NCstrcmp(a, b) NLchar *a, *b; { return (0); }
int	NCstrncmp(a, b, n) NLchar *a, *b; int n; { return (n); }
NLchar *	NCstrcpy(a, b) NLchar *a, *b; { return (a); }
NLchar *	NCstrncpy(a, b, n) NLchar *a, *b; int n; { return (a); }
int	NCstrlen(s) NLchar *s; { return (0); }
NLchar *	NCstrchr(a, b) NLchar *a, b; { return (a); }
NLchar *	NCstrrchr(a, b) NLchar *a, b; { return (a); }
NLchar *	NCstrpbrk(a, b) NLchar *a; char *b; { return (a); }
int	NCstrspn(a, b) NLchar *a; char *b; { return (0); }
int	NCstrcspn(a, b) NLchar *a; char *b; { return (0); }
NLchar *	NCstrtok(a, b) NLchar *a; char *b; { return (a); }
/* NLCONV(3C) */
#undef NCdechr
#undef NCdec
#undef _NCdec2
#undef NCenc
#undef NCchrlen
#undef NCisNLcp
#undef NLchrlen
#undef NCcollate
#undef NCcoluniq
#undef NCeqvmap
#undef NLisNLcp
NLchar	NCdechr(c) char *c; { return (0); }
int	NCdec(c, nlc) char *c; NLchar *nlc; { return (0); }
int	_NCdec2(c0, c1, nlc) char c0, c1; NLchar nlc; { return (0); }
int	NCenc(nlc, c) NLchar *nlc; char *c; { return (0); }
int	NCchrlen(nlc) NLchar nlc; { return (0); }
int	NLisNLcp(c) char *c; { return (0); }
int	NLchrlen(c) char *c; { return (0); }
int	NCdecode(c, nlc) char *c; NLchar *nlc; { return (0); }
int	NCdecstr(c, nlc, len) char *c; NLchar *nlc; int len; { return (0); }
int	NCencode(nlc, c) wchar_t *nlc; unsigned char *c; { return (0); }
int	NCencstr(nlc, c, len) NLchar *nlc; char *c; int len; { return (0); }
int	NCcollate(nlc) NLchar nlc; { return (0); }
int	NCcoluniq(nlc) NLchar nlc; { return (0); }
int	NCeqvmap(ucval) int ucval; { return (0); }
int	_NCxcol(x, str, rstr) int x; NLchar **str, **rstr; { return (0); }
int	_NLxcol(x, str, rstr) int x; char **str; NLchar **rstr; { return (0); }
/* NLCTYPE(3C) */
#undef NCisshift
#undef NCisalpha
#undef NCisupper
#undef NCislower
#undef NCisdigit
#undef NCisxdigit
#undef NCisalnum
#undef NCisspace
#undef NCispunct
#undef NCisprint
#undef NCisgraph
#undef NCiscntrl
#undef NCisNLchar
#undef _NCtoupper
#undef _NCtolower
#undef NCflatchr
#undef NCtoNLchar
#undef NCesc
#undef NCunesc
int	NCisshift(c) int c; { return (c); }
int	NCisalpha(c) int c; { return (c); }
int	NCisupper(c) int c; { return (c); }
int	NCislower(c) int c; { return (c); }
int	NCisdigit(c) int c; { return (c); }
int	NCisxdigit(c) int c; { return (c); }
int	NCisalnum(c) int c; { return (c); }
int	NCisspace(c) int c; { return (c); }
int	NCispunct(c) int c; { return (c); }
int	NCisprint(c) int c; { return (c); }
int	NCisgraph(c) int c; { return (c); }
int	NCiscntrl(c) int c; { return (c); }
int	NCisNLchar(c) int c; { return (c); }
int	_NCtoupper(c) int c; { return (c); }
int	_NCtolower(c) int c; { return (c); }
int	NCflatchr(c) int c; { return (c); }
int	NCtoNLchar(c) int c; { return (c); }
int	NCesc(nlc, c) NLchar *nlc; char *c; { return (0); }
int	NCunesc(c, nlc) char *c; NLchar *nlc; { return (0); }
int	NCtolower(c) int c; { return (c); }
int	NCtoupper(c) int c; { return (c); }
int	NLescstr(src, dest, dlen) char *src, *dest; int dlen; { return (0); }
int	NLflatstr(src, dest, dlen) char *src, *dest; int dlen; { return (0); }
int	NLunescstr(src, dest, dlen) char *src, *dest; int dlen; { return (0); }

#ifdef KJI
int	NCwunesc(src, dest) NLchar *src, *dest; { return (0); }
unsigned char *ujtosj(s1, s2) unsigned char *s1, *s2; { return s1; }
unsigned char *ujtojis(s1, s2) unsigned char *s1, *s2; { return s1; }
int	tojupper(c) register c; { return (0); }
int	tojlower(c) register c; { return (0); }
int	tojkata(c) register c; { return (0); }
int	_tojkata(c) int c; { return (0); }
int	tojhira(c) register c; { return (0); }
int	_tojhira(c) int c; { return (0); }
unsigned char *sjtouj(s1, s2) unsigned char *s1, *s2; { return s1; }
unsigned char *sjtojis(s1, s2) unsigned char *s1, *s2; { return s1; }
int	kutentojis(c) int c; { return (0); }
unsigned char *jistouj(s1, s2) unsigned char *s1, *s2; { return s1; }
int	jistoa(c) register c; { return (0); }
int	atojis(c) register c; { return (0); }
unsigned char *jistosj(s1, s2) unsigned char *s1, *s2; { return s1; }
unsigned char *cjistosj(s1, s2) unsigned char *s1, *s2; { return s1; }
unsigned char *cjistouj(s1, s2) unsigned char *s1, *s2; { return s1; }
unsigned char *csjtojis(s1, s2) unsigned char *s1, *s2; { return s1; }
unsigned char *csjtouj(s1, s2) unsigned char *s1, *s2; { return s1; }
unsigned char *cujtojis(s1, s2) unsigned char *s1, *s2; { return s1; }
unsigned char *cujtosj(s1, s2) unsigned char *s1, *s2; { return s1; }
#endif
/* NLIST(3C) */
#include <a.out.h>
int	nlist(f, n) const char *f; struct nlist *n; { return (0); }
/* NLSTRING(3C) */
unsigned char *	NLstrcat(a, b) unsigned char *a, *b; { return (a); }
unsigned char *	NLstrncat(a, b, n) unsigned char *a, *b; int n; { return (a); }
int	NLstrcmp(a, b) const char *a, *b; { return (0); }
int	NLstrncmp(a, b, n) char *a, *b; int n; { return (n); }
char *	NLstrcpy(a, b) char *a, *b; { return (a); }
unsigned char *	NLstrncpy(a, b, n) unsigned char *a, *b; int n; { return (a); }
int	NLstrlen(s) char *s; { return (0); }
int	NLstrdlen(s) char *s; { return (0); }
unsigned char *	NLstrchr(a, b) unsigned char *a; NLchar b; { return (a); }
unsigned char *	NLstrrchr(a, b) unsigned char *a; NLchar b; { return (a); }
unsigned char *	NLstrpbrk(a, b) unsigned char *a; char *b; { return (a); }
int	NLstrspn(a, b) char *a, *b; { return (0); }
int	NLstrcspn(a, b) char *a, *b; { return (0); }
unsigned char *	NLstrtok(a, b) unsigned char *a, *b; { return (a); }
/* NLSTRTIME(3C) */
char	*NLstrtime(s, l, f, d) char *s; size_t l; const char *f; const struct tm *d;
	{ return (s); }
/* NLTMTIME(3C) */
int	NLtmtime(s, f, d) char *s, *f; struct tm *d; { return (0); }
/* PERROR(3C) */
void	perror(s) const char *s; {}
int	errno;
char *	sys_errlist[1];
int	sys_nerr;
/* PLOT(3X) not in libc */
/* POPEN(3S) */
FILE *	popen(c, t) char *c, *t; { return (stdin); }
int	pclose(f) FILE *f; { return (0); }
/* PRINTF(3S) */
	/*VARARGS1 PRINTFLIKE1*/
int	printf(s) const char *s; { return (0); }
	/*VARARGS2 PRINTFLIKE2*/
int	fprintf(f, s) FILE *f; const char *s; { return (0); }
	/*VARARGS2 PRINTFLIKE2*/
int	sprintf(p, s) char *p; const char *s; { return (0); }
	/*VARARGS1 PRINTFLIKE1*/
int	NLprintf(s) char *s; { return (0); }
	/*VARARGS2 PRINTFLIKE2*/
int	NLfprintf(f, s) FILE *f; char *s; { return (0); }
	/*VARARGS2 PRINTFLIKE2*/
int	NLsprintf(p, s) char *p, *s; { return (0); }
/* PUTC(3S) */
#undef putc
#undef putchar
int	putc(c, f) int c; FILE *f; { return (c); }
int	putchar(c) int c; { return (c); }
int	fputc(c, f) int c; FILE *f; { return (c); }
int	putw(w, f) int w; FILE *f; { return (w); }
/* PUTENV(3C) */
int	putenv(s) char *s; { return (0); }
/* PUTPWENT(3C) */
int	putpwent(s, f) struct passwd *s; FILE *f; { return (0); }
/* PUTS(3S) */
int	puts(s) const char *s; { return (0); }
int	fputs(s, f) const char *s; FILE *f; { return (0); }
/* QSORT(3C) */
void	qsort(b, n, w, c) void *b; size_t n, w; int (*c)(); {}
/* RAND(3C) */
void	srand(s) unsigned s; {}
int	rand() { return (0); }
/* REGCMP(3X) libPW */
/* RENAME(2) LIBC */
int	rename(from, to) const char *from, *to; { return (0); }
/* SCANF(3S) */
	/*VARARGS1 SCANFLIKE1*/
int	scanf(s) const char *s; { return (0); }
	/*VARARGS2 SCANFLIKE2*/
int	fscanf(f, s) FILE *f; const char *s; { return (0); }
	/*VARARGS2 SCANFLIKE2*/
int	sscanf(p, s) const char *p, *s; { return (0); }
	/*VARARGS1 SCANFLIKE1*/
int	NLscanf(s) char *s; { return (0); }
	/*VARARGS2 SCANFLIKE2*/
int	NLfscanf(f, s) FILE *f; char *s; { return (0); }
	/*VARARGS2 SCANFLIKE2*/
int	NLsscanf(p, s) char *p, *s; { return (0); }
/* SETBUF(3S) */
void	setbuf(f, b) FILE *f; char *b; {}
int	setvbuf(f, t, b, s) FILE *f; char *t; int b; size_t s;
	{ return ((int)t); }
/* SETJMP(3C) */
#include <setjmp.h>
int	setjmp(e) jmp_buf e; { return (0); }
void	longjmp(e, v) jmp_buf e; int v; {}
/* SLEEP(3C) */
unsigned	sleep(n) unsigned n; { return (n); }
/* SSIGNAL(3C) */
void (*	ssignal(s, a))() int s; void (*a)(); { return (a); }
int	gsignal(s) int s; { return (s); }
/* STDIO(3S) */
#ifndef	_NIOBRW
#define	_NIOBRW	16
#endif	/* _NIOBRW */
FILE	_iob[_NIOBRW];
unsigned char *	_bufendtab[_NFILE];
/* STDIPC(3C) */
key_t	ftok(s, i) char *s, i; { return ((key_t)0); }
/* STRING(3C) */
char 	*strdup(a) char *a; { return (a); }
int	strcasecmp(a, b) char *a, *b; { return (0); }
int	strncasecmp(a, b, n) const char *a, *b; size_t n; { return (n); }
char *	strcat(a, b) char *a, *b; { return (a); }
char *	strncat(a, b, n) char *a; const char *b; size_t n; { return (a); }
int	strcmp(a, b) char *a, *b; { return (0); }
int	strncmp(a, b, n) const char *a, *b; size_t n; { return (n); }
char *	strcpy(a, b) char *a, *b; { return (a); }
char *	strncpy(a, b, n) char *a; const char *b; size_t n; { return (a); }
#ifdef	_NONSTD_TYPES
int	strlen(s) char *s; { return (0); }
#else
size_t	strlen(s) char *s; { return (0); }
#endif /*_NONSTD_TYPES*/
char *	strchr(a, b) const char *a; int b; { return (""); }
char *	strrchr(a, b) const char *a; int b; { return (""); }
char *	strpbrk(a, b) const char *a, *b; { return (""); }
#ifdef	_NONSTD_TYPES
int	strspn(a, b) const char *a, *b; { return (0); }
int	strcspn(a, b) const char *a, *b; { return (0); }
#else
size_t	strspn(a, b) const char *a, *b; { return (0); }
size_t	strcspn(a, b) const char *a, *b; { return (0); }
#endif /*_NONSTD_TYPES*/
char *	strtok(a, b) char *a; const char *b; { return (a); }
/* STRTOD(3C) */
double	strtod(s, t) const char *s; char **t; { return (0.0); }
double	atof(s) const char *s; { return (0.0); }
float	atoff(s) char *s; { return ( (float) 0.0); }
float	strtof(s, t) char *s, **t; { return ( (float) 0.0); }
/* STRTOL(3C) */
long	strtol(s, t, b) const char *s; char **t; int b; { return (0L); }
long	atol(s) const char *s; { return (0L); }
int	atoi(s) const char *s; { return (0); }
/* SWAB(3C) */
void	swab(f, t, n) const char *f; char *t; int n; {}
/* SYSTEM(3S) */
int	system(s) const char *s; { return (0); }
/* TMPFILE(3S) */
FILE *	tmpfile() { return (stdin); }
/* TMPNAM(3S) */
char *	tmpnam(s) char *s; { return (s); }
char *	tempnam(d, s) char *d, *s; { return (d); }
/* TSEARCH(3C) */
void *	tsearch(k, r, c) void *k, **r; int (*c)(); { return (k); }
void *	tfind(k, r, c) char *k, **r; int (*c)(); { return (k); }
void *	tdelete(k, r, c) char *k, **r; int (*c)(); { return (k); }
void	twalk(r, f) char *r; void (*f)(); {}
/* TTYNAME(3C) */
char *	ttyname(f) int f; { return (""); }
int	isatty(f) int f; { return (f); }
/* UNGETC(3S) */
int	ungetc(c, f) int c; FILE *f; { return (c); }
int	ungetwc(c, f) int c; FILE *f; { return (c); }
/* VPRINTF(3S) */
#include <varargs.h>
int	vprintf(s, v) const char *s; va_list v; { return (0); }
int	vfprintf(f, s, v) FILE *f; const char *s; va_list v; { return (0); }
int	vsprintf(p, s, v) char *p; const char *s; va_list v; { return (0); }

/* UNDOCUMENTED (not for general use) */
	/*VARARGS3*/
int	syscall(n, r, s, a) int n, r, s, a; { return (n); }
long	tell(f) int f; { return (0L); }
int	ttyslot() { return (0); }
int	_filbuf(f) FILE *f; { return (0); }
int	_flsbuf(c, f) int c; FILE *f; { return (c); }
#undef _NCbufstr
#undef _NCfreebuf
void	NLgetctab(nlfile) char *nlfile; {}
NLchar *	_NCbufstr(s, d, l) char *s; NLchar *d; int l; { return (d); }
void	_NCfreebuf(s) NLchar *s; {}
NLchar *	_NCgetbuf(src, dlen) char *src; int dlen;
	{ return ((NLchar *)"\0"); }
int	_NLunescval(src, slen, dest) char *src; int slen; NLchar *dest;
	{ return (0); }

char	*basename(path) char *path; { return (path); }
char	*dirname(path) char *path; { return (path); }
void	endvfsent() {}
int	fshelp(fsdev, vfsnam, op, mode, debuglevel, opflags, opdata)
	char *fsdev, *vfsnam; int op, mode, debuglevel; 
	char *opflags; caddr_t opdata; { return (op); }
char	*getuinfo (name) char *name; { return (name); }
char	*getusershell () { return (""); }
int	endusershell () {}
int	setusershell () {}

int	initgroups (u, agroup) char *u; int agroup;
	{ return agroup; }
int	putgrent (g, f) struct group *g; FILE *f; { return 0; }
int	setegid (gid) gid_t gid; { return 0; }
int	setregid (gid) gid_t gid; { return 0; }
int	setrgid (gid) gid_t gid; { return 0; }
int	setruid (uid) uid_t uid; { return 0; }

int	itrunc(x) double x; { return (0); }
long int labs(j) long int j; { return (j); }
double	rint(x) double x; { return (x); }
unsigned long int strtoul(np, ep, b) const char *np; char **ep; int b;
	{ return (0UL); }
unsigned uitrunc(x) double x; { return (0u); }

int	bcmp(b1,b2,l) char *b1,*b2; int l; { return(l); }
void	bcopy(s,d,l) char *s,*d; int l; {}
void	bzero(b,l) char *b; int l; {}
int	creadir(p,o,g,m) char *p; uid_t o; gid_t g; int m; { return(m); }
#include <stdlib.h>
struct div_t div(n,d) int n,d; { struct div_t x; return x; }
struct ldiv_t ldiv(n,d) long int n,d; { struct ldiv_t x; return x; }
int	ffs(m) int m; { return m; }
struct	qelem {
		struct qelem *next;
		struct qelem *prev;
		char	data[1];
	};
void	insque(e,p) struct qelem *e, *p; {}
void	remque(e) struct qelem *e; {}
#include <locale.h>
struct lconv *localeconv(void) { struct lconv x; return (&x); }
int	mkfifo(p,m) char *p; mode_t m; { return (0); }
int	mkstemp(a) char *a; { return (0); }
#include <nl_types.h>
char	*nl_langinfo(i) nl_item i; { return (""); }
long	pathconf(p,n) char *p; int n; { return (0L); }
long	fpathconf(f,n) int f,n; { return (0L); }
int	psignal(sig, s) unsigned sig; char *s; {}
int	random() { return (0); }
int	srandom(x) unsigned x; {}
char	*re_comp(s) char *s; { return (s); }
int	re_exec(p) char *p; { return (0); }
void	longjmperror() {}
char	*setlocale(c,l) int c; const char *l; { return (""); }
int	siginterrupt(s,f) int s,f; { return (s); }
int	sigfillset(s) sigset_t *s; { return (0); }
int	siginitset(s) sigset_t *s; { return (0); }
int	sigaddset(s, n) sigset_t *s; int n; { return(0); }
int	sigdelset(s,n) sigset_t *s; int n; { return (0); }
int	sigismember(s,n) sigset_t *s; int n; { return (0); }
int	sigemptyset(s) sigset_t *s; { return (0); }
size_t	strftime(s,m,f,t) char *s; size_t m; const char *f; 
	const struct tm *t; { return (m); }
long	sysconf(n) int n; { return (0L); }
int	syslog(pri,f,p0,p1,p2,p3,p4) int pri; const char *f; 
	int p0,p1,p2,p3,p4; { return (pri); }
int	setlogmask(p) int p; { return (p); }
int	tcb(c,p) int c; char *p; { return (c); }

#include <netinet/in.h>
u_int	inet_addr(cp) char *cp; { return ((u_long) 0); }
u_int	inet_lnaof(in) struct in_addr in; { return (0); }
struct in_addr	inet_mkadr(n,h) int n,h; { struct in_addr x; return (x); }
u_int	inet_netof(in) struct in_addr in; { return (0); }
u_int	inet_network(cp) char *cp; { return ((u_long) 0); }
char	*inet_ntoa(in) struct in_addr in; { return (""); }

#include <errno.h>
int	fgetpos(s,p) FILE *s; fpos_t *p; { return (0); }
int	fgetwc(s) FILE *s; { return (0); }
int	fputwc(c,f) int c; FILE *f; { return (c); }
int	fsetpos(s,p) FILE *s; const fpos_t *p; { return (0); }

int	getdirentries(f,b,s,o) int f; caddr_t b; int s; 
	off_t *o; { return (0); }
int	getdtablesize() { return (0); }
void	endttyent() {}
int	setttyent() {}
#include <ttyent.h>
struct ttyent *getttyent() { return ((struct ttyent *) 0); }
struct ttyent *getttynam(tty) const char *tty; { return ((struct ttyent *) 0); }
int	getwchar() { return (0); }
int	putwchar(c) int c; { return (c); }
int	remove(f) const char *f; { return (0); }
#include <dirent.h>
int	scandir(d,n,s,dcmp) const char *d; struct dirent *(*n[]); 
	int (*s)(), (*dcmp)(); { return (0); }
int	alphasort(d1,d2) struct dirent **d1, **d2; { return (0); }
void setbuffer(f,b,s) FILE *f; char *b; int s; {}
void setlinebuf(f) FILE *f; {}

#undef	catgets(catd,setno,msgno,def)
char	*catgets(ctd,sn,mn,d) nl_catd ctd; int sn,mn; char *d; 
	{ return (d); }
char	*NLfcatgets(ctd,sn,mn,d) nl_catd ctd; int sn,mn; char *d; 
	{ return (d); }
char	*NLgetamsg(c,sn,mn,d) char *c; int sn,mn; char *d; { return (c); }
int	catclose(catd) nl_catd catd; { return (0); }
#undef	catgetmsg(catd,setno,msgno,buf,def)
char	*catgetmsg(ctd,sn,mn,b,l) nl_catd ctd; int sn,mn; char *b; 
	int l; { return (b); }
char	*fcatgetmsg(ctd,sn,mn,b,l) nl_catd ctd; int sn,mn; char *b; 
	int l; { return (b); }
char	*fcatgets(ctd,sn,mn,d) nl_catd ctd; int sn,mn; char *d; 
	{ return (d); }
nl_catd	catopen(c,d) char *c; int d; { return ((nl_catd) 0); }
#include <ndbm.h>
DBM	*dbm_open(f,fg,m) char *f; int fg,m; { return ( (DBM *) 0); }
void	dbm_close(db) DBM *db; {}
long	dbm_forder(db,k) DBM *db; datum k; { return (0L); }
datum	dbm_fetch(db, key) DBM *db; datum key; { return (key); }
int	dbm_delete(db,k) DBM *db; datum k; { return (0); }
int	dbm_store(db,k,d,r) DBM *db; datum k, d; int r; { return (r); }
datum	dbm_firstkey(db) DBM *db; { datum k; return (k); }
datum	dbm_nextkey(db) DBM *db; { datum k; return (k); }

#include <sys/uio.h>
/* #include <sys/mbuf.h> */
#include <sys/socket.h>
int	bindresvport(n,s) int n; struct sockaddr_in *s; { return (n); }
char	*ether_ntoa(e) struct ether_addr *e; { return (""); }
struct ether_addr *ether_aton(s) char *s; 
	{ return ( (struct ether_addr *) 0); }
int	ether_hostton(h,e) char *h; struct ether_addr *e; { return(0); }
int	ether_ntohost(h,e) char *h; struct ether_addr *e; { return(0); }
void	sethostfile(n)  char *n; {}
#undef n_name
#include <netdb.h>
struct netent *getnetbyaddr(n,t) int n,t; { return( (struct netent*) 0); }
struct netent *getnetbyname(a) char *a; { return( (struct netent*) 0); }
struct rpcent *getrpcbynumber(n) int n; { return ( (struct rpcent *) 0); }
struct rpcent *getrpcbyname(n) char *n; { return ( (struct rpcent *) 0); }
struct servent *getservbyport(p,pr) int p; char *pr; 
	{ return ( (struct servent *) 0); }
struct servent *getservbyname(n,p) char *n,*p; {return ((struct servent*) 0);}
int	setnetgrent(g) char *g; { return(0); }
int	getnetgrent(m,n,d) char **m, **n, **d; { return (0); }
struct protoent *getprotobynumber(n) int n; { return ( (struct protoent*) 0); }
struct protoent *getprotobyname(n) char *n; { return ( (struct protoent*) 0); }
int	herror(s) char *s; {}
int	rcmd(a,r,l,ru,c,f) char **a; u_short r; char *l,*ru,*c; 
	int *f; { return (0); }
int	fp_query(m,f) char *m; FILE *f; {}
int	res_search(n,c,t,a,al) char *n; int c,t; u_char *a; int al;
	{ return (c); }
int	rexec(a,r,n,p,c,f) char **a; int r; char *n,*p,*c; int *f;
{ return (r); }
int	mkpwunclear(s1,m,s2) char *s1,m,*s2; {}
int	mkpwclear(s1,m,s2) char *s1,m,*s2; {}

char	*NLcsv() { return (""); }
void	_NLinit() {}
int	NLxin(t,s,n) char *t,*s; int n; { return (0); }
int	NLxout(t,s,n) char *t,*s; int n; { return (0); }
void	NLxstart() {}
int	NLyesno(s) char *s; { return (0); }
char	*mbsncat(s1,s2,n) char *s1,*s2; size_t n; { return (s1); }
char	*mbsncpy(s1,s2,n) char *s1,*s2; size_t n; { return (s1); }
char	*mbsrchr(s,m) char *s; int m; { return (s); }
int	mbstoint(m) char *m; { return (0); }
char	*mbschr(s,m) const char *s; const int m; { return (""); }
char	*mbscat(s,t) char *s; char *t; { return (s); }
int	mbscmp(s,t) char *s,*t; { return (0); }
int	wcscmp(s,t) wchar_t *s,*t; { return (0); }
char	*mbscpy(s,t) char *s; const char *t; { return (s); }
wchar_t	*wcscpy(s,t) wchar_t *s,*t; { return (s); }
int	mbsncmp(s1,s2,n) char *s1,*s2; size_t n; { return (0); }
int	wcsncmp(s1,s2,n) wchar_t *s1,*s2; size_t n; { return (0); }
char	*mbspbrk(s,b) char *s,*b; { return (""); }
size_t	mbstowcs(p,s,n) wchar_t *p; const char *s; size_t n;
	{ return (n); }
int	mbtowc(p,s,n) wchar_t *p; const char *s; size_t n;
	{ return (0); }
int	toujis(c) int c; { return (0); }
wchar_t	*wcscat(s,t) wchar_t *s,*t; { return (s); }
wchar_t	*wcschr(s,c) wchar_t *s,c; { return (s); }
size_t	wcscspn(s,t) wchar_t *s,*t; { return ( (size_t) 0); }
size_t	wcslen(s) wchar_t *s; { return ( (size_t) 0); }
wchar_t	*wcsncat(s,t,n) wchar_t *s,*t; size_t n; { return (s); }
wchar_t	*wcsncpy(s,t,n) wchar_t *s,*t; size_t n; { return (s); }
wchar_t	*wcspbrk(s,t) wchar_t *s,*t; { return (s); }
wchar_t	*wcsrchr(s,c) wchar_t *s,c; { return (s); }
size_t	wcsspn(s,t) wchar_t *s,*t; { return ( (size_t) 0); }
size_t	wcstombs(s,t,n) char *s; const wchar_t *t; size_t n; 
	{ return (n); }
wchar_t	*wcswcs(s,t) wchar_t *s,*t; { return (s); }
int	wctomb(s,t) char *s; wchar_t t; { return (0); }

int	NLvfprintf(p,f,a) FILE *p; char *f; va_list a; { return (0); }
int	NLvprintf(f,a) char *f; va_list a; { return (0); }
int	NLvsprintf(s,f,a) char *s; char *f; va_list a; { return (0); }
	/*VARARGS2 PRINTFLIKE2*/
int	wsprintf(p, s) char *p; const char *s; { return (0); }

int	NLcplen(s) char *s; { return (0); }

#ifdef KJI
wchar_t	*wstrdup(s) wchar_t *s; { return (s); }
wchar_t *	wstrcat(a, b) wchar_t *a, *b; { return (a); }
wchar_t *	wstrchr(a, b) wchar_t *a; int b; { return (a); }
int	wstrcmp(a, b) wchar_t *a, *b; { return (0); }
wchar_t *	wstrcpy(a, b) wchar_t *a, *b; { return (a); }
int	wstrcspn(a, b) wchar_t *a, *b; { return (0); }
int	wstrlen(s) wchar_t *s; { return (0); }
wchar_t *	wstrncat(a, b, n) wchar_t *a, *b; int n; { return (a); }
int	wstrncmp(a, b, n) wchar_t *a, *b; int n; { return (n); }
wchar_t *	wstrncpy(a, b, n) wchar_t *a, *b; int n; { return (a); }
wchar_t *	wstrpbrk(a, b) wchar_t *a; char *b; { return (a); }
wchar_t *	wstrrchr(a, b) wchar_t *a; int b; { return (a); }
int	wstrspn(a, b) wchar_t *a, *b; { return (0); }
#endif

char	*index(s,c) const char *s; int c; { return (""); }
char	*rindex(s,c) const char *s; int c; { return (""); }
int	strcoll(a,b) const char *a, *b; { return (0); }
char	*strerror(e) int e; { return (""); }
char	*strstr(a,b) const char *a, *b; { return (""); }
size_t	strxfrm(a,b,n) char *a; const char *b; size_t n; { return n; }

int 	exect(n,a,e) char *n; char *a[]; char *e[]; { return (0); }
int	getpagesize(void) { return (0); }
int	killpg(p,s) int p,s; { return (0); }
int	vfork() { return (0); }
int	vlimit(l,v) int l,v; { return (0); }
#include <sys/resource.h>
pid_t	wait3(s,o,r) union wait *s; int o; struct rusage *r; 
	{ return ( (pid_t) 0 ); }
pid_t	waitpid(p, s, o) pid_t p; int *s; int o; { return (p); }

double	difftime(s,t) time_t s, t; { return( (double) 0 ); }
#include <sys/time.h>
int	getitimer(w,v) int w; struct itimerval *v; { return 0; }
int	gettimeofday(t,tz) struct timeval *t; struct timezone *tz;
	{ return (0); }
time_t	mktime(t) struct tm *t; { return ( (time_t) 0); }
int	setitimer(w,v,o) int w; struct itimerval *v, *o;
	{ return (0); }
int	settimeofday(t,tz) struct timeval *t; struct timezone *tz;
	{ return (0); }

#include <float.h>
/* float.h is included indirectly by stdio.h */
unsigned   int SINFINITY;
unsigned   int DINFINITY[2];
unsigned   int SQNAN;
unsigned   int DQNAN[2];
unsigned   int SSNAN;
unsigned   int DSNAN[2];
unsigned read_rnd() { return (0u); }
unsigned write_rnd(rnd) unsigned rnd; { return (rnd); }

