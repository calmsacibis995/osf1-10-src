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
 *	@(#)$RCSfile: lvmcmds.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/22 16:07:22 $
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
 *   lvmcmds.h
 *   
 *   Contents:
 *	This file gets #include'd by all the source files of the
 *	LVM system administrator commands. Hence, here are all the
 *	things that are required by at least 2 of these commands;
 *	in general, the things here contained are necessary to ALL
 *	of the commands.
 */

/*
 *   Include files
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <lvm/lvm.h>
#include <langinfo.h>
#include "options.h"
#include "user_interf.h"
#include "utilities.h"
#include "lvmtab.h"
#include "maps.h"
#include "logvol.h"
#include "dump.h"
#include "lvmdefmsg.h"



/*
 *   Usual constants
 */

#ifndef FALSE
#   define FALSE	0
#endif /* FALSE */

#ifndef TRUE
#   define TRUE		(!FALSE)
#endif /* TRUE */

#define OK	0
#define NOT_OK	(!OK)
#define RETRY	(NOT_OK + 1)

#define SUCCESS	OK
#define FAILURE	(!SUCCESS)

/* Operation modes for functions reducing/extending logical volumes */
#define REMOVE_LV_MIRRORS	0
#define REMOVE_LV_EXTENTS	1
#define ADD_LV_MIRRORS		2
#define ADD_LV_EXTENTS		3

/* Values supplied to exit() are defined here. Note that all main() return 0 */
#define FATAL_ERROR		1


/*
 *   Default paths/names
 */

#define	VGPATH		"/dev/vg"
#define LVPREFIX	"lvol"
#define GROUP		"group"
#define DEVPATH		"/dev/"



/*
 *   General-purpose macros
 */

#define in_range(var, lo, hi)	((var) >= (lo) && (var) <= (hi))
#define in_string(ch, str)	(strchr((str), (ch)) != NULL)
#define eq_string(s1, s2)	(strcmp((s1), (s2)) == 0)
#define print_prgname()		fprintf(stderr, "%s: ", program_name())
#define prgname_perror(str)	{ print_prgname(); perror(str); }
#define entries(array)		(sizeof(array) / sizeof(array[0]))



/*
 *   Non-int functions
 */

void *malloc(unsigned int count);



/*
 *   Global variables that can be useful to ALL of the lvm commands.
 *   Under #ifdef, so they belong to exactly one module at a time.
 *   The "global" pseudo-keyword allows the variables to be seen as:
 *   - "normal" variables from the file where main() is contained;
 *   - extern variables from the other files
 */

#ifdef LVM_CMD_MAIN_FILE

#  define global		/* purposely nothing */
   char msgbuf[100];

#else /* LVM_CMD_MAIN_FILE */

#  define global		extern
   extern char msgbuf[];

#endif /* LVM_CMD_MAIN_FILE */

extern int errno;


/*
 *   Tricky tricks
 */

/*
 *   Debug actions (prints, and whatever) are constrained by
 *   two factors:
 *	-DDEBUG at compile time;
 *	-D option at execution time
 *   So you can feel free to store
 *	debug_msg("%d", x)
 *   or
 *	debug(stupid_var = 100)
 *   throughout the code: this eliminates the need of the ugly
 *   #ifdef DEBUG in the sources; moreover, compiling with -UDEBUG
 *   will remove all of the extra-code.
 */

#ifdef DEBUG

#  include "debug.h"

#  define DEBUG_OPT_CHAR	'D'
#  define DEBUG_FNAME		"LVMCMDDBG"
   global int debugflag;	/* TRUE if "-D" is used */
   global FILE *debugfile;	/* Where we store debug messages */
#  define set_debug_opt()	legal_opt(DEBUG_OPT_CHAR, WITHOUT_VALUE)
#  define init_debug()		{debugflag = used_opt(DEBUG_OPT_CHAR); \
				 debugfile = fopen(DEBUG_FNAME, "a"); \
				 if (debugfile == NULL) debugfile = stderr;}
#  define debug(action)		if (debugflag) action
#  define debug_msg(fmt, val)	if (debugflag) \
				   dbg_indent(), \
				   fprintf(debugfile, (fmt), (val))

#else  /* DEBUG */

#  define set_debug_opt()	/* purposely nothing */
#  define init_debug()		/* purposely nothing */
#  define debug(action)		/* purposely nothing */
#  define debug_msg(fmt, val)	/* purposely nothing */

#endif /* DEBUG */



/* Internationalization (I18N) support */

#ifdef	MSG

#  include <nl_types.h>
#  include <locale.h>
   nl_catd catd, catopen(char *name, int oflag);
   char *catgets(nl_catd catd, int set_id, int msg_id, char *str);
#  define msg_init()		(setlocale(LC_ALL, ""),\
				 catd=catopen(MF_LVM, 0))
#  define MSGSTR(num, str)	catgets(catd, MS_LVM, num, str)

#else /* MSG */

#  define msg_init()		/* purposely nothing */
#  define MSGSTR(num, str)	str

#endif /* MSG */
