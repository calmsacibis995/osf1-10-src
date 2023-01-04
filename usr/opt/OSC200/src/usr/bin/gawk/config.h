/*
 * *********************************************************************
 * *                                                                   *
 * *       Modified by Digital Equipment Corporation, 1991, 1994       *
 * *                                                                   *
 * *       This file no longer matches the original Free Software      *
 * *       Foundation file.                                            *
 * *                                                                   *
 * *********************************************************************
 */
/*
 * HISTORY
 */
/*
 * @(#)$RCSfile: config.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/25 20:06:58 $
 */

/* config.h.  Generated automatically by configure.  */
#define HAVE_STRING_H 1 /* have <string.h> */
#ifdef HAVE_STRING_H
#undef	NEED_MEMORY_H	/* need <memory.h> to declare memcpy() et al. */
#else
#undef	HAVE_STRINGS_H	/* have <strings.h> */
#endif

#define STDC_HEADERS 1 /* have the usual ANSI header files */
#define HAVE_UNISTD_H 1 /* have <unistd.h> */
#undef	HAVE_ALLOCA_H	/* have <alloca.h> -- only used if bison is used */
#undef	HAVE_SIGNUM_H	/* have <signum.h> */
#define REGEX_MALLOC 1 /* don't use alloca in regex.c */

#define HAVE_VPRINTF 1 /* have vprintf() */
#ifndef HAVE_VPRINTF
#define	HAVE_DOPRNT	/* lacking vprintf(), but have _doprnt() to fake it */
#endif

#define HAVE_RANDOM 1 /* have random(), or using missing/random.c */
#ifndef HAVE_RANDOM
#define	HAVE_LRAND48	/* have the SysV lrand48() */
#endif

#define HAVE_STRCHR 1 /* have strchr() (and hopefully strrchr()!) */
#ifndef	HAVE_STRCHR
#ifdef	HAVE_RINDEX	/* use index() and rindex() if present */
#define	strchr	index
#define	strrchr	rindex
#endif
#endif

#define HAVE_FMOD 1 /* have fmod(), otherwise use modf() */
#define HAVE__SETJMP 1 /* have _setjmp() (BSD systems) */

#define HAVE_MEMCPY 1 /* have memcpy() et al. */
#ifndef HAVE_MEMCPY
#define HAVE_BCOPY	/* fake memcpy() et al. by bcopy() et al. */
#endif

#define HAVE_ST_BLKSIZE 1 /* have st_blksize member in the stat(2) structure */

#define HAVE_STRFTIME 1
#ifndef	HAVE_STRFTIME
#undef	TM_IN_SYS_TIME	/* <sys/time.h> declares struct tm */
#undef	HAVE_TZNAME	/* use tzname array */
#endif

#define HAVE_STRINGIZE 1 /* have ANSI "stringizing" capability */

#undef	const		/* define to nothing if compiler doesn't grok const */

#undef	__CHAR_UNSIGNED__	/* default char is unsigned */

#undef	size_t		/* define if typedef doesn't exist */
#undef	pid_t		/* define if the typedef doesn't exist */

#define	RETSIGTYPE	void	/* type returned by signal() */
#define SPRINTF_RET int /* type returnewd by sprintf() */

#undef	_ALL_SOURCE	/* on AIX, used to get some BSD functions */

#undef	_POSIX_SOURCE	/* on Minix, used to get Posix functions */
#undef	_MINIX		/* on Minix, used to get Posix functions */
#undef	_POSIX_1_SOURCE	/* on Minix, define to 2 */
