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
 *	@(#)$RCSfile: setlocale.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/30 17:27:14 $
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
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: setlocale.h 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_SETLOCALE
#define _H_SETLOCALE

#include <sys/limits.h>

#define	FAIL		-1
#define	PASS		 0
#define	FOUND		 1
#define	NOTFOUND	~FOUND


#define	MAXLOCAL 	6
/*
**	Define macros to access the loc_t structure.
*/
#define	lc_col_lcname	_locp->lc_coltbl->lc_locale_name
#define	lc_chr_lcname	_locp->lc_chrtbl->lc_locale_name
#define	lc_mon_lcname	_locp->lc_montbl->lc_locale_name
#define	lc_num_lcname	_locp->lc_numtbl->lc_locale_name
#define	lc_tim_lcname	_locp->lc_timtbl->lc_locale_name
#define	lc_msg_lcname	_locp->lc_msgtbl->lc_locale_name

/*
**	Per process data
*/
/*
**	This is similar to loc_t structure. Whereas loc_t can have
**	pointers to different locales, env_t points to only one 
**	locale. One can view loc_t as a per process locale information
**	and env_t as a per locale information. loc_t will 
**	obtain its information from a set of linked lists of env_t
**	structures. The envp points to the head of the linked list.
*/
typedef struct env {
	char 		locale[128]; /* locale */
	char		locale_en[128]; /* locale for .en */
	char		*buffer;	/* buffer */
	struct env	*next;		/* Linked list */
	loc_t		loc_info;	/* locale info itself */
} env_t;

#define	le_mag0		loc_info.lc_mag0
#define	le_mag1		loc_info.lc_mag1
#define	le_version	loc_info.lc_version
#define	le_code_type	loc_info.lc_code_type
#define	le_length	loc_info.lc_length
#define	le_coltbl	loc_info.lc_coltbl
#define	le_chrtbl	loc_info.lc_chrtbl
#define	le_montbl	loc_info.lc_montbl
#define	le_numtbl	loc_info.lc_numtbl
#define	le_timtbl	loc_info.lc_timtbl
#define	le_msgtbl	loc_info.lc_msgtbl
#define	le_maptbl	loc_info.lc_maptbl

/*
** Structure built during setlocale to keep track of locale name
** and status for each locale being changed.
*/
struct locale_info {
		char 	*name;		/* NULL ==> use current locale */
		env_t	*newenv;	/* ptr to env for new locale */
		int	flags;		/* flags for this locale */
};

#define	FLAG_NONE	0
#define	FLAG_DOINGALL	0x1		/* doing LC_ALL instead of individual category */

/*
**	envp is used to link list the information 
**	obtained from disk files.
*/

extern  env_t	*_envp;
extern	loc_t	*_locp;
/*
**      setlocale() uses the following static routines:
*/

#ifdef _NO_PROTO

static 	int	get_ctab();
static  int	get_locinfo();

static  int     alloc_newenv();
static	env_t	*lookup_oldenv();
static  int     copy_collate();
static  int     copy_ctype();
static  int     copy_messages();
static  int     copy_monetary();
static  int     copy_numeric();
static  int     copy_time();
static  char    *current_loc();
static  int     get_diskfile();
static  int     get_file();
static  int     get_filenames();
static  char    *get_line();
static  int    	get_locale_names();
static  int     parse_entries();
static  void    reloc_ctab();
static  void    reloc_ctype();
static  void    update_locale();
static char	*get_default_locale();

#else

static char 	*current_loc( int category );
static int	get_locale_names(int category, char *locale, struct locale_info *info );
static int	get_diskfile( int lc_cat, struct locale_info *info);
static int	get_filenames( int lc_cat, struct locale_info *info, char *buf);
static int	get_file(int lc_cat, struct locale_info *info, char *localefile);
static env_t	*lookup_oldenv(char *locale, int category);
static int	get_ctab( int category, struct locale_info *info, char *locfile);
static int 	alloc_newenv( char *locale, int category);
static int	get_locinfo( int category, struct locale_info *info, char *locfile);
static int 	copy_collate( env_t *p);
static int 	copy_ctype( env_t *p);
static int 	copy_monetary( env_t *p);
static int 	copy_numeric( env_t *p);
static int 	copy_time( env_t *p);
static int 	copy_messages( env_t *p);
static int 	parse_entries( char *ptr, char *locale);
static char 	*get_line(char **sbufp);
static void 	reloc_ctab( loc_t *ctab);
static void 	reloc_ctype( loc_t * ctab);
static void 	update_locale(struct locale_info *info);
static char	*get_default_locale();

#endif /* _NO_PROTO */

#endif    /* _H_SETLOCALE */


