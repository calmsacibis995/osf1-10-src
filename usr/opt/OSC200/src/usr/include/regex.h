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
 *	@(#)$RCSfile: regex.h,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/09/03 18:48:29 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * definitions for the reentrant versions of regular expression functions
 */

#ifndef	_REGEX_H_
#define	_REGEX_H_

#include <standards.h>
#include <sys/types.h>

#define _REG_SUBEXP_MAX  9       /* Maximum # of subexpressions          */

/* regcomp() cflags */

#define REG_EXTENDED    0x001   /* Use Extended RE syntax rules         */
#define REG_ICASE       0x002   /* Ignore case in match                 */
#define REG_NEWLINE     0x004   /* Convert <backslash><n> to <newline>  */
#define REG_NOSUB       0x008   /* regexec() not report subexpressions  */

/* regexec() eflags */

#define REG_NOTBOL      0x100   /* First character not start of line    */
#define REG_NOTEOL      0x200   /* Last character not end of line       */


/* Regular Expression error codes */

#define REG_NOMATCH     1       /* RE pattern not found                 */
#define REG_BADPAT      2       /* Invalid Regular Expression           */
#define REG_ECOLLATE    3       /* Invalid collating element            */
#define REG_ECTYPE      4       /* Invalid character class              */
#define REG_EESCAPE     5       /* Last character is \                  */
#define REG_ESUBREG     6       /* Invalid number in \digit             */
#define REG_EBRACK      7       /* [] imbalance                         */
#define REG_EPAREN      8       /* \( \) or () imbalance                */
#define REG_EBRACE      9       /* \{ \} or { } imbalance               */
#define REG_BADBR       10      /* Invalid \{ \} range exp              */
#define REG_ERANGE      11      /* Invalid range exp endpoint           */
#define REG_ESPACE      12      /* Out of memory                        */
#define REG_BADRPT      13      /* ?*+ not preceded by valid RE         */
#define REG_ECHAR       14      /* invalid multibyte character          */

#ifdef _XOPEN_SOURCE
#define REG_ENOSYS	17	/* Implementation doesn't support function */
#endif

typedef struct {                /* regcomp() data saved for regexec()   */
        size_t  re_nsub;        /* # of subexpressions in RE pattern    */
        void    *re_comp;       /* compiled RE; freed by regfree()      */
        int     re_cflags;      /* saved cflags for regexec()           */
        size_t  re_erroff;      /* RE pattern error offset              */
        size_t  re_len;         /* # wchar_t chars in compiled pattern  */
        wchar_t re_ucoll[2];    /* min/max unique collating values      */
        void    *re_lsub[_REG_SUBEXP_MAX+1]; /* start subexp            */
        void    *re_esub[_REG_SUBEXP_MAX+1]; /* end subexp              */
        uchar_t re_map[256];    /* map of valid pattern characters      */
} regex_t;

typedef off_t regoff_t;


typedef struct {                /* substring locations - from regexec() */
        regoff_t   rm_so;	/* Byte offset from start of string to  */
                                /*   start of substring                 */
        regoff_t   rm_eo;	/* Byte offset from start of string of  */
                                /*   first character after substring    */
} regmatch_t;


/* Regular Expression function prototypes */

#if defined(__cplusplus)
extern "C" {
#endif
extern  int     regcomp  __((regex_t *, const char *, int));
extern  int     regexec  __((const regex_t *, const char *, size_t, regmatch_t *, int));
extern  size_t  regerror __((int, const regex_t *, char *, size_t));
extern  void    regfree  __((regex_t *));
#if defined(__cplusplus)
}
#endif

#ifdef _OSF_SOURCE
#if defined(_REENTRANT) || defined(_THREAD_SAFE)

#define	ESIZE	512
#define	NBRA	9

typedef	struct regex_data {
	char	expbuf[ESIZE],
		*braslist[NBRA],
		*braelist[NBRA],
		circf;
} REGEXD;

#if defined(__cplusplus)
extern "C" {
#endif
extern	char	*re_comp_r __((char *, REGEXD *));
extern	int	 re_exec_r __((char *, REGEXD *));
#if defined(__cplusplus)
}
#endif
#endif	/* _REENTRANT || _THREAD_SAFE */
#endif  /* _OSF_SOURCE */

#endif	/* _REGEX_H_ */
