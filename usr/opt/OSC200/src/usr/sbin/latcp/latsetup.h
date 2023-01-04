
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
 * @(#)$RCSfile: latsetup.h,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/10/18 16:12:18 $
 */

/*
 * FILE: latsetup.h
 *
 * This file contains all of the necessary header files and #defines needed 
 * for latsetup.
 *
 */

/*
 * define some macros for regexp.h - these definitions must appear 
 * before "#include <regexp.h>"
 */
			/* First arg points to RE string*/
#define INIT		register unsigned char *sp = (void *)instring;
#define GETC()		(*(unsigned char*)sp++)
#define PEEKC()		(*(unsigned char*)sp)
#define UNGETC(c)	(--sp)
#define RETURN(c)	return;
#define ERROR(x)	regerr(x)


#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <curses.h>
#include <term.h>
#include <nlist.h>
#include <unistd.h>
#include <utmp.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <machine/hal_sysinfo.h>

#include <dec/lat/lat.h>  
#include "latcp.h"
#include <regexp.h>

/* defines for main menu */
#define EXIT 		0
#define INITIAL_SETUP 	1
#define CREATE_DEV 	2
#define START_STOP 	3
#define E_D_AUTO_S_S 	4
#define UNDO_SETUP 	5
/* NUM_ENTRIES should be 1 more than last entry in list */
#define NUM_ENTRIES	6
#define MAIN_X 	25
#define MIN_Y 	8
/* MAX_Y should always equal MIN_Y plus the last entry in the main menu */
#define MAX_Y 	(MIN_Y + UNDO_SETUP)

#define CR_X	20
#define CR_Y	20

#define YES   22
#define NO    42
#define NO_CUR_YES   1
#define NO_CUR_NO    0

#define CR	'\n'	/* carriage return (0x0d) */
#define LF 	'\r'	/* line feed (0x0a) */
#define IS_RETURN(c)	((c == LF) || (c == KEY_ENTER) || (c == CR))
#define ESCAPE 	0x1b	/* escape  */

#define RUN_LEVEL_3 "run-level 3"
#define CLONE_MINOR 5000         /* the minor # of cloneable /dev/lat device */

/* these defines are used by the INITIAL_LAT_SETUP, START/STOP and */
/* ENABLE/DISABLE sections of code */
#define SHELL_NOT_EXEC  127
#define S_S_BUF_SIZE 100

/* these defines are used by the CREATE_ADDITIONAL_DEVICES section of code */
#define MAX_DEVS     	620     /* max # of LAT tty devices that can be made */
				/* LAT ttys are: /dev/tty[0-9][0-9,a-z,A-Z] */
#define DEV_DIRECTORY   "/dev"  /* nm of dir where tty devs are found & made */
#define INITTAB_FILE    "/etc/inittab"           /* name of the inittab file */
#define INITTAB_TEMPLATE   "/tmp/inittab.tmpl"	/* template inittab file */
#define INITTAB_TMP_FILE    "/tmp/inittab.tmp"	/* scratch inittab file */
#define INITTAB_SAVE    "/etc/inittab.sav"       /* copy of the inittab file */
#define INITTAB_ID  	"lat"  		/* tag for LAT inittab entries */
#define INIT_PROG       "/sbin/init"       /* full path name of init program */
#define MAIN_MENU_CALLER 0	     /* we have been called by the main menu */
#define SUCCESS	         0	     /* we successfully executed the routine */
#define FAILURE		 -1  /* we were unsuccessful in executing the rout. */
#define HDR_Y		 1                       /* starting y loc of header */
#define OUTPUT_Y	 3            /* starting y loc of our actual output */
#define X_LOC		 10           /* starting x loc of every line output */
#define OUT_STDERR	 -1                  /* send error message to stderr */
#define OUT_STDSCR	 0                   /* send error message to stdscr */


/* constants used for regular expression parser code */

#define EXPBUFSIZE	256*5		/* size of buffer for regexp's usage */
#define LINESIZE	512		/* max size of a line in a file */
#define LASTCOMPLEN	5	/* size of the last component of a ttyname */
				


/* data structures and related constants */

struct inittab_line {
#define	IDENT_LEN 14		/* same as used in init.h */
#define	FIELD_LEN 20		/* inittab(4) says 20 char max. */
					/* add 1 for NULL in the following */
	char	c_id[IDENT_LEN+1];	/* unique id string of process */
	char	c_levels[FIELD_LEN+1];	/* legal levels for process */
	char	c_action[FIELD_LEN+1];	/* type of action required */
	char	*c_command;		/* Pointer to init command */
};

#define COMMAND "/usr/sbin/getty\t%s\tconsole\tvt100"

typedef struct inittab_line i_line;

struct latttyinfo {
#define TTYNAMELEN	13		/* enough chars for "/dev/ttyXXXX\0" */
	char	name[TTYNAMELEN];
	dev_t	tdev;			/* major and minor #'s of this tty */
	int	flags;			/* see below */
	i_line	iline;			/* parsed inittab entry (if exists) */
};
					/* latttyinfo.flag bit #defines */
#define ININITTAB	0x00000001	/* this tty is used in inittab */
#define ISMAPPED	0x00000002	/* this tty mapped to an appl. port */

/* commands used by change_tty_info() */

#define CREATE_TTY_DEVICE       0
#define REMOVE_TTY_DEVICE       1
#define ADD_TO_INITTAB          2
#define REMOVE_FROM_INITTAB     3

/* constants used by copy_file() */

#define	BUFSIZE	512
#define PMODE	0644

/* flags used in init_tty_info() */

#define COUNT_INITTAB_TOTAL	0x00000001
#define INIT_ALL		0x00000002
