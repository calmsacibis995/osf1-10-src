
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
static char	*sccsid = "@(#)$RCSfile: dumpextern.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/12/16 15:19:38 $";
#endif 


/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 */

#if !defined( lint) && !defined(_NOIDENT)

#endif


#include	"dump.h"

/* actual instance of important spcl record */

union u_spcl	u_spcl;

/*	local external variables	*/

int		imap_size = 0;		/* inode map size in bytes	*/
char	       *not_clear_map = NULL;	/* bit map of inodes which are	*/
					/* (not) clear at time of dump	*/
					/* (0 == clear, 1 == not clear)	*/
char	       *directory_map = NULL;	/* bit map of inodes which are	*/
					/* directories and eligible for	*/
					/* dumping			*/
					/* (0 == not dir, 1 == is dir)	*/
char	       *to_dump_map = NULL;	/* bit map of inodes which are	*/
					/* still to be dumped		*/
					/* (0 == do not dump, 1 == do)	*/

char   *disk_file_name = DEFAULT_DISK;		/* name of the disk file */
char   *tape_file_name = DEFAULT_TAPE;		/* name of the tape file */
char   *dump_hist_file_name = DUMP_HISTORY;	/* name of the file with
						 * dump history information */

/* if no argument is given, then 9u is implied */

char		incr_num = '9';		/* increment number */
char		last_incr_num;		/* increment number of previous
					 * dump */

ino_t		curr_inum;		/* current inumber */

int		new_tape_flag = TRUE;	/* new tape flag */
int		curr_tape_num = 1;	/* current tape number */
int		curr_volume_num = 1;	/* current volume number */

int		dir_added_flag;		/* true if added directory to to-be-
					 * dumped list on last pass-two */
int		dir_skipped_flag = FALSE; /* set TRUE in mark() if any
					 * directories are skipped; this
					 * lets us avoid map pass 2 in
					 * some cases */

int		by_blocks_flag = FALSE; /* TRUE when measuring output by blocks,
					 * FALSE when by inches */

long		full_size = 0;             /* full dump_file size, blocks or feet */
long		full_size_blocks = 0;	   /* full size of medium in blocks */
double		full_size_inches = 0.0;	   /* full size of medium in inches */
long		blocks_written = 0;        /* number of blocks written on
					    * current volume */
double		inches_written = 0.0;      /* number of inches written on
					    * current volume */
long		est_tot_blocks = 0;	   /* estimated total number of blocks */
double		est_tot_inches = 0.0;	   /* estimated total number of inches */
int		est_tot_tapes;		   /* estimated number of tapes */
int		tape_density = 0;	   /* density in bytes/inch */
double		tape_record_gap = 0.0;     /* inter-record gap (inches) */
int		blocks_per_write = NTREC;  /* blocking factor on tape */
double		inches_per_write = 0.0;	   /* inches for each tape write */

long		dev_bsize;	           /* machine device block size in bytes */

int		in_disk_fd;		   /* disk file descriptor */
int		out_tape_fd;		   /* tape file descriptor */

int		pipe_out_flag = FALSE;	   /* true => output to standard output */
int		medium_flag = NO_MEDIUM;   /* medium not yet known */
int		no_rewind_flag = FALSE;	   /* do not rewind tape */
int             estimate_only_flag = FALSE; /* print estimate information, only */
int             invalid_file_system=FALSE; /* output file abort dump msg*/
int		notify_flag = FALSE;	   /* notify operators */

int		num_idate_records = 0;	/* number of idate records (might
					 * be zero) */
struct idates **idate_array = NULL;	/* pointer to array of pointers to */
					/* dump_history idates structures */

/*	 buffer for the file system super block		*/

union
{
	char		dummy[MAXBSIZE];
	struct fs	s_blk;
}		sup_blk;

struct fs      *super_block = &sup_blk.s_blk;

/*	NLS stuff	*/
nl_catd		catd;
char	       *yes_str = "yes";
char	       *no_str = "no";
int		yes_flag = FALSE;
int		no_flag = FALSE;
