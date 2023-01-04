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
static char	*sccsid = "@(#)$RCSfile: restoreextern.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:25:31 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

#include	"restore.h"

/*
 * Flags
 */

int		old_format_flag = FALSE;/* convert from old tape format */
int		block_size_flag = FALSE;/* set input block size */
int		debug_flag = FALSE;	/* print out debugging info */
int		children_flag = TRUE;	/* restore heirarchies */
int		by_name_flag = TRUE;	/* restore by name instead of inumber */
int		verbose_flag = FALSE;	/* print out actions taken */
int		auto_retry_flag = FALSE;/* always try to recover */
					/* from tape errors */

int		byte_swap_flag = FALSE;	/* Swap Bytes (for CCI or sun) */
int		quad_swap_flag = FALSE;	/* Swap quads (for sun) */
int		Nflag = FALSE;		/* do not write the disk */
int		overwrite_flag = OVERWRITE_DEFAULT;


/*
 * Global variables
 */

char	       *dumpmap;	/* map of inodes on this dump tape */
char	       *clrimap;	/* map of inodes to be deleted */
ino_t		maxino;		/* highest numbered inode in this file system */
long		dumpnum = 1;	/* location of the dump on this tape */
long		volno = 0;	/* current volume being read */
long		ntrec;		/* number of TP_BSIZE records per tape block */
time_t		dumptime;	/* time that this dump begins */
time_t		dumpdate;	/* time that this dump was made */
char		command = '\0';	/* opration being performed */
FILE	       *terminal_fp;	/* file descriptor for the terminal input */
FILE	       *command_fp=NULL;/* file descriptor for the command input */

/*
 * These variables describe the next file available on the tape
 */

char	       *curr_file_name;	/* name of file */
ino_t		curr_inumber;	/* inumber of file */
struct dinode  *curr_inode;	/* pointer to inode */
char		curr_action;	/* action being taken on this file */

chfl	nullcfs = { '\0', NOFLG };

nl_catd		catd;
