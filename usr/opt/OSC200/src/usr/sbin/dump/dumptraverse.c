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
static char	*sccsid = "@(#)$RCSfile: dumptraverse.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1994/01/07 19:20:44 $";
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

/*
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE ONLY
 * SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1980 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

#include	"dump.h"

/*	Inodes are numbered 1..n while map bits are numbered 0..n-1.	*/
/*	Low bit in map byte is bit-0 while high bit is bit-NBBY.	*/

#define	MAPWORD(map, num)	(map[(ulong_t) (num - 1) / NBBY])
#define	MAPBIT(num)		(1 << ((ulong_t) (num - 1) % NBBY))
#define	MAPBITSET(map, num)	(MAPWORD(map, num) |= MAPBIT(num))
#define	MAPBITCLEAR(map, num)	(MAPWORD(map, num) &= ~MAPBIT(num))
#define	MAPBITTEST(map, num)	(MAPWORD(map, num) & MAPBIT(num))

static int		est();
static struct dinode   *getino();
static void		blksout();
static void		dmpindir();
static void		dsrch();
static void		indir();
static void		spclrec();

static int	has_sub_dir_flag;	/* true when directory being */
					/* examined has sub-directory */
static int	has_file_to_dump_flag;	/* true when directory being
					 * examined has a descendent
					 * which will be dumped */

/*	Pass() executes function_to_do() for every inode (1..n) whose	*/
/*	bit in the do_it_map is set (or all if no do_it_map is given).	*/

void
pass(function_to_do, do_it_map)
	register void	      (*function_to_do)();
	register char	       *do_it_map;
{

/*	old code - fast and confusing

	register int		bits;
	ino_t			maxino;

	maxino = super_block->fs_ipg * super_block->fs_ncg - 1;
	for (curr_inum = (ino_t) 0; curr_inum < maxino;)
	{
		if ((int) curr_inum % NBBY == 0)
		{
			bits = ~0;
			if (do_it_map != NULL)
			{
				bits = *do_it_map++;
			}
		}
		++curr_inum;
		if (bits & 1)
		{
			(*function_to_do)(getino(curr_inum));
		}
		bits >>= 1;
	}

*/

	register ino_t		total_inodes;

	/*	Total number of inodes is product of inodes-per-	*/
	/*	cylinder-group and number-of-cylinder-groups.		*/

	total_inodes = (ino_t) (super_block->fs_ipg * super_block->fs_ncg);

	/*	Start with the root inode.				*/
	/*	Old code started with inode 1, but since this is not	*/
	/*	used (or used for bad blocks), it is better to start	*/
	/*	with the root inode (inode 2).				*/

	for (curr_inum = ROOTINO; curr_inum < total_inodes; ++curr_inum)
	{
		if (do_it_map == NULL || MAPBITTEST(do_it_map, curr_inum))
		{
			(*function_to_do)(getino(curr_inum));
		}
	}
}

/*	Mark() does the first pass marking of non-clear inodes,		*/
/*	directory inodes, and changed-since-last-dump inodes.		*/
/*	All inodes added to the to-be-dumped list are accounted for	*/
/*	in the total-blocks accumulator.				*/
/*	If any unchanged directory inodes are found and skipped,	*/
/*	a flag is set so they will be examined in the next pass.	*/

void
mark(ip)
	struct dinode  *ip;
{
	register int	relevant_flags;

	/*	Strip off mode bits which identify type of file.	*/
	relevant_flags = ip->di_mode & IFMT;

	/*	If inode is clear, just return, leaving not_clear_map	*/
	/*	with a 0 bit (indicating clear inode) in this inode's	*/
	/*	position.  Otherwise, set the bit (indicating non-clear	*/
	/*	inode).							*/

	if (relevant_flags == 0)
	{
		return;
	}
	else if (ip->di_nlink == 0)
	{
		/* ignore files without any links too */

		return;
	}
	else
	{
		MAPBITSET(not_clear_map, curr_inum);
	}

	/*	If inode is a directory, set its bit in directory_map.	*/

	if (relevant_flags == IFDIR)
	{
		MAPBITSET(directory_map, curr_inum);
	}

	/*	choose inodes which have been changed since last dump	*/

	if (ip->di_mtime >= spcl.c_ddate || ip->di_ctime >= spcl.c_ddate)
	{
		/*	mark it as to-be-dumped	*/

		MAPBITSET(to_dump_map, curr_inum);

		/*	increment the total block estimate to account	*/
		/*	for the inode itself				*/

		++est_tot_blocks;

		/*	If the inode is for a regular file, a directory, */
		/*	or a symbolic link, add the estimate of how many */
		/*	blocks will be needed for the file's data to the */
		/*	total estimate.					*/

		if (relevant_flags == IFREG || relevant_flags == IFDIR || relevant_flags == IFLNK)
		{
			est_tot_blocks += est(ip);
		}
	}
	else if (relevant_flags == IFDIR)
	{

		/*	if the inode was a directory and was skipped	*/
		/*	because it had not been changed, set flag here	*/
		/*	so it will be examined in the second pass	*/

		dir_skipped_flag = TRUE;
	}
}

/*	Add() is called in the second pass over the inodes in order	*/
/*	to select and to examine directory inodes which were unchanged	*/
/*	and therefore skipped in the first pass.			*/
/*	NB: add() is called only with directory inodes.			*/

void
add(ip)
	register struct dinode *ip;
{
	register int	i;
	long		remaining_dir_size;
	int		nblks;
	/* if the inode is already on the to-dump list, skip it (return) */

	if (MAPBITTEST(to_dump_map, curr_inum))
	{
		return;
	}

	/* set flags indicating that for this directory, we have not yet */
	/* found that it has either a sub-directory or a descendent which */
	/* on the to-be-dumped list */

	has_sub_dir_flag = FALSE;
	has_file_to_dump_flag = FALSE;

	/* get the directory's size so we know how far to search it */

	remaining_dir_size = ip->di_size;

	/* for each direct block in the directory file, search its entries */

	for (i = 0; i < NDADDR && remaining_dir_size > 0; ++i)
	{
		if (ip->di_db[i] != 0)
		{
			dsrch(ip->di_db[i], (int) dblksize(super_block, ip, i), remaining_dir_size);
		}

		/* decrease the remaining directory size so that subsequent */
		/* searching will not go beyond the end of the directory */

		remaining_dir_size -= super_block->fs_bsize;
	}

	/* for each indirect block in the directory file, search its */
	/* entries as well */

	for (i = 0; i < NIADDR && remaining_dir_size > 0; ++i)
	{
		if (ip->di_ib[i] != 0)
		{
			indir(ip->di_ib[i], i, &remaining_dir_size);
		}
	}

	/* if the directory has a descendent which is to-be-dumped, then */
	/* it too must be dumped */

	if (has_file_to_dump_flag == TRUE)
	{
		/* Set the global flag indicating that a directory was */
		/* added to the to-be-dumped list.  This necessitates */
		/* that pass two be performed at least once again to */
		/* propogate the directory-dumping up the tree and insure */
		/* that all parents of this directory will be dumped too. */

		dir_added_flag = TRUE;

		/*	mark it as to-be-dumped	*/

		MAPBITSET(to_dump_map, curr_inum);

		/*	increment the total block estimate to account	*/
		/*	for the inode itself				*/

		++est_tot_blocks;

		/*	Add the estimate of how many blocks will be	*/
		/*	needed for the file's data to the total estimate */
		est_tot_blocks += est(ip);
	}

	/* If the directory does has neither a sub-directory nor a */
	/* child which is to-be-dumped, there is no reason to dump */
	/* it, and furthermore no reason to consider it again on */
	/* subsequent passes.  So remove it from the directory list. */

	if (has_sub_dir_flag == FALSE && has_file_to_dump_flag == FALSE)
	{
		MAPBITCLEAR(directory_map, curr_inum);
	}
}

/*	Indir() supervises the examination of the indirect blocks of a */
/*	directory.  First it reads the block of block-pointers.  If the */
/*	level of indirection from this call is zero, each block pointed */
/*	to is examined directly with a call to dsrch().  If the level */
/*	of indirection is greater than zero, each block pointed to is */
/*	itself a block of pointers and is examined recursively with a */
/* 	call to indir() with the level of indirection decremented. */

static void
indir(ptrs_block_num, indirection_level, remaining_dir_size)
	daddr_t		ptrs_block_num;
	int		indirection_level;
	long	       *remaining_dir_size;
{
	register int		i;
	union
	{
		char	dummy[MAXBSIZE];
		daddr_t	b_p;
	}			blk_ptr;
	register daddr_t       *block_of_ptrs = &blk_ptr.b_p;

	/* read the block of block-pointers from the disk */

	bread(fsbtodb(super_block, ptrs_block_num), (char *) block_of_ptrs, (int) super_block->fs_bsize);

	/* if the level of indirection is zero (these pointers point to the */
	/* real blocks of data), call dsrch for each of them to examine the */
	/* directory data */

	if (indirection_level == 0)
	{

		/* for each block in the directory file, search its entries */

		for (i = 0; i < super_block->fs_nindir && *remaining_dir_size > 0; ++i)
		{
			if (block_of_ptrs[i] != (daddr_t) 0)
			{
				dsrch(block_of_ptrs[i], (int) super_block->fs_bsize, *remaining_dir_size);
			}

			/* decrease the remaining directory size so that */
			/* subsequent searching will not go beyond the end */
			/* of the directory */

			*remaining_dir_size -= super_block->fs_bsize;
		}
	}
	else
	{	
		/* for each block of indirect pointers, call indir() */
		/* recursively with decremented level of indirection */

		for (i = 0; i < super_block->fs_nindir && *remaining_dir_size > 0; ++i)
		{
			if (block_of_ptrs[i] != (daddr_t) 0)
			{
				indir(block_of_ptrs[i], indirection_level - 1, remaining_dir_size);
			}
		}
	}
}

/* Dsrch() searches the specified directory block to determine whether */
/* it has 1) an entry which is a sub-directory and 2) an entry which is */
/* a file which has already been marked as to-be-dumped */

static void
dsrch(data_block_num, blocksize, remaining_dir_size)
	daddr_t			data_block_num;
	int			blocksize;
	long			remaining_dir_size;
{
	register struct dirent *dir_entry_ptr;
	char			dir_buffer[MAXBSIZE];
	int			readsize;
	int			offset;

	if (has_file_to_dump_flag == TRUE)
	{
		return;
	}

	/* read the next block of the directory */
	/* to be searched into the buffer	*/

	readsize = (int) min(remaining_dir_size, blocksize);

	bread(fsbtodb(super_block, data_block_num), dir_buffer, readsize);

	for (offset = 0; offset < readsize; offset += dir_entry_ptr->d_reclen)
	{
		/* point to next entry in directory */

		dir_entry_ptr = (struct dirent *) (dir_buffer + offset);

		/* the entry must have a non-zero length */

		if (dir_entry_ptr->d_reclen == 0)
		{
			msg(MSGSTR(CORDIR, "Corrupted directory, i-node: %d\n"), curr_inum);
			break;
		}

		/* if the entry has no i-number, it is empty, skip it */

		if (dir_entry_ptr->d_fileno == (ulong_t) 0)
		{
			continue;
		}

		/* if the entry is named "." or "..", we are not */
		/* interested, skip it				*/

		if (strcmp(dir_entry_ptr->d_name, ".") == 0 ||
		    strcmp(dir_entry_ptr->d_name, "..") == 0)
		{
			continue;
		}

		/* If the entry is a file which is already in the */
		/* to-be-dumped list, then this directory must also */
		/* be dumped.  Set flag so it will be added to list. */

		if (MAPBITTEST(to_dump_map, dir_entry_ptr->d_fileno))
		{
			has_file_to_dump_flag = TRUE;
			return;
		}

		/* If the inode pointed to is in the directory map, */
		/* it is a directory.  Therefore the directory being */
		/* examined cannot be eliminated from consideration */
		/* unless its subdirectory is also examined and found */
		/* to contain no dumpable files later in this pass or */
		/* in a repeat of pass 2.  Set the sub_directory flag */
		/* so it will be preserved. */

		if (MAPBITTEST(directory_map, dir_entry_ptr->d_fileno))
		{
			has_sub_dir_flag = TRUE;
		}
	}
}

/*	Dirdump() dumps a directory to tape by calling dump(), */
/*	but only after checking that the inode in question has */
/*	not changed into a non-directory in the intervening time. */

void
dirdump(ip)
	struct dinode  *ip;
{
	/* watchout for dir inodes deleted and maybe reallocated */

	if ((ip->di_mode & IFMT) != IFDIR)
	{
		return;
	}
	dump(ip);
}

/* Dump() dumps the file specified by the inode argument to tape, */
/* first the directly addressed blocks and then the indirectly addressed */
/* blocks, preceded by the TS_INODE record an interspersed with TS_ADDR */
/* records, as needed. */

void
dump(ip)
	struct dinode  *ip;
{
	register int	i;
	int		relevant_flags;
	int		frags_to_write;
	long		remaining_file_size;

	/* always put the map of the inodes left to dump before the first */
	/* new inode on the tape (beginning of tape may contain end of */
	/* previous inode) */

	if (new_tape_flag == TRUE)
	{
		new_tape_flag = FALSE;
		switch (medium_flag) {
		case REGULAR_FILE:
			break;
		case TAPE:
		case CARTRIDGE:
			msg(MSGSTR(TAPEBEG, "Volume %d, tape # %04d, begins with blocks from i-node %d\n"),
			    curr_volume_num, curr_tape_num, curr_inum);
			break;
		case DISKETTE:
			msg(MSGSTR(DISKBEG, "Volume %d, diskette # %04d, begins with blocks from i-node %d\n"),
			    curr_volume_num, curr_tape_num, curr_inum);
			break;
		default:
			msg(MSGSTR(ILLMEDIUM, "%s(%d):Illegal flag %d.\n"),
					__FILE__, __LINE__, medium_flag);
			break;
		}
		bitmap(to_dump_map, TS_BITS);
	}

	/* clear this inode from the to-dump map, as we are going to dump it */
	/* next thing */

	MAPBITCLEAR(to_dump_map, curr_inum);

	/* get the spcl structure ready to write out the inode to tape */

	spcl.c_dinode = *ip;
	spcl.c_type = TS_INODE;
	spcl.c_count = 0;

	/* if the inode has been freed since we last checked, skip it and */
	/* return now */

	relevant_flags = ip->di_mode & IFMT;
	if (relevant_flags == 0)		/* free inode */
	{
		return;
	}

	/* if the inode is not a directory, regular file, or symbolic link, */
	/* or its length is zero, just write out the inode alone and return */

	if ((relevant_flags != IFDIR && relevant_flags != IFREG &&
	     relevant_flags != IFLNK) || ip->di_size == 0)
	{
		spclrec();
		return;
	}

	/* set the remaining file size to the full file size */

	remaining_file_size = ip->di_size;

#ifdef IC_FASTLINK
	/* Maybe we are dealing with a fast symbolic link? */
	if (relevant_flags == IFLNK &&
	    (ip->di_flags & IC_FASTLINK) == IC_FASTLINK)
	{
		char fastlink_tapebuf[TP_BSIZE];

		/* Since the file contents are NOT in a data block, */
		/* prepare a block containing the required data: i.e., */
		/* the file name (to which this file is linked), which */
		/* is kept in the dinode itself */
		bzero(fastlink_tapebuf, sizeof(fastlink_tapebuf));
		strncpy(fastlink_tapebuf, ip->di_symlink,
						sizeof(ip->di_symlink));

		/* to specify that the data block pointer is valid */
		/* (non-zero), put a 1 in the corresponding */
		/* tape block's map array entry */
		spcl.c_addr[0] = 1;

		/* write out the TS_INODE/TS_ADDR record, which immediately */
		/* precedes the data corresponding to the map */
		spcl.c_count = 1;
		spclrec();

		/* write the actual data */
		taprec(fastlink_tapebuf);
	}
	else
#endif /* IC_FASTLINK */
	{

		/* Figure out how many directly-addressed frag-sized blocks */
		/* to write. */

		if (remaining_file_size > NDADDR * super_block->fs_bsize)
		{
			/* If the remaining file size is greater than can be */
			/* addressed directly (number-direct-blocks */
			/* * full-block-size), then the number of frags is */
			/* the number-of-direct-blocks */
			/* 		* frags-per-full-data-block. */

			frags_to_write = NDADDR * super_block->fs_frag;
		}
		else
		{
			/* Otherwise, just round up the remaining file size */
			/* to the next full frag and that is how many */
			/* frags-worth of data to write. */

			frags_to_write = howmany(remaining_file_size,
						super_block->fs_fsize);
		}

		/* write out frags (and necessary inode and addr spcl */
		/* records) starting with the data block pointed to by the */
		/* first direct block address */

		blksout(&ip->di_db[0], frags_to_write);

		/* decrease the remaining file size by the number of bytes */
		/* which were written by the blksout() call */

		remaining_file_size -= frags_to_write * super_block->fs_fsize;

		/* if there is any remaining file size to be dumped, call */
		/* dmpindir() to dump the indirectly addressed data blocks */

		for (i = 0; i < NIADDR && remaining_file_size > 0; ++i)
		{
			dmpindir(ip->di_ib[i], i, &remaining_file_size);
		}
	}
}

/*	Dmpindir() dumps the indirectly addressed blocks of a file.	*/
/*	For levels of indirection greater than single, it calls itself	*/
/*	recursively for each of the pointers it receives. */

static void
dmpindir(ptrs_block_num, indirection_level, remaining_file_size)
	daddr_t		ptrs_block_num;
	int		indirection_level;
	long	       *remaining_file_size;
{
	int			i;
	int			frags_to_write;
	union
	{
		char	dummy[MAXBSIZE];
		daddr_t	b_p;
	}			blk_ptr;
	register daddr_t       *block_of_ptrs = &blk_ptr.b_p;

	/* fill the block of block-pointers from the appropriate source */

	if (ptrs_block_num != (daddr_t) 0)
	{
		/* if the block num is a real (non-zero) block, */
		/* read the block of block-pointers from the disk */

		bread(fsbtodb(super_block, ptrs_block_num), (char *) block_of_ptrs, (int) super_block->fs_bsize);
	}
	else
	{
		/* If the block num is a phony (zero), */
		/* fill the block of block-pointers with zeroes. */
		/* This will cause no dumping of data blocks to be done, */
		/* however the remaining file size will be properly */
		/* reduced, and the other intervening records will also */
		/* be output. */

		bzero((char *) block_of_ptrs, super_block->fs_bsize);
	}

	if (indirection_level == 0)
	{
		/* this block of block-pointers points to real data blocks */
		/* (no more indirection). */

		/* Figure out how many indirectly-addressed frag-sized */
		/* blocks to write. */

		if (*remaining_file_size > super_block->fs_nindir * super_block->fs_bsize)
		{
			/* If the remaining file size is greater than can */
			/* be addressed by a single indirect block of */
			/* block-pointers (> number-of-pointers-per-block */
			/* * full-block-size), then the number of frags is */
			/* the number-of-pointers-per-block */
			/* * frags-per-full-data-block. */

			frags_to_write = super_block->fs_nindir * super_block->fs_frag;
		}
		else
		{
			/* Otherwise, just round up the remaining file size */
			/* to the next full frag and that is how many */
			/* frags-worth of data to write. */

			frags_to_write = howmany(*remaining_file_size, super_block->fs_fsize);
		}

		/* write out frags (and necessary addr spcl records) */
		/* starting with the data block pointed to by the first */
		/* indirect block address */

		blksout(&block_of_ptrs[0], frags_to_write);

		/* decrease the remaining file size by the number of */
		/* bytes which were written by the blksout() call */

		*remaining_file_size -= frags_to_write * super_block->fs_fsize;
	}
	else
	{
		/* this block of block-pointers points to still more blocks */
		/* of block-pointers (still more levels of indirection). */

		/* for each pointer in the block call dmpindr() recursively */
		/* with a decremented indirection level */

		for (i = 0; i < super_block->fs_nindir && *remaining_file_size > 0; ++i)
		{
			dmpindir(block_of_ptrs[i], indirection_level - 1, remaining_file_size);
		}
	}
}

/* Blksout() writes out frags_to_write frags-worth of data which is in disk */
/* data blocks addressed by the block-pointers in the data_block_ptr_array. */
/* Each group of maximum size TP_BSIZE * TP_NINDIR bytes is preceded by */
/* either a TS_INODE record (for the first portion of a file), or a TS_ADDR */
/* record (for subsequent portions of a file), which contains a map telling */
/* which tape blocks are actually on the tape (because they were present */
/* in the source file) and which are not on the tape (because they were */
/* holes in the source file).  This routine will work only when the product */
/* TP_BSIZE * TP_NINDIR is a multiple of the dumped file system's block */
/* size (which is the size of the blocks pointed to by the pointers in */
/* data_block_ptr_array).  This condition is tested for early in the main */
/* routine of the dump program, so it is always true if this routine is */
/* reached. */

static void
blksout(data_block_ptr_array, frags_to_write)
	daddr_t	       *data_block_ptr_array;
	int		frags_to_write;
{
	int		tape_blocks;
	int		tblks_per_dblk;
	int		i;
	int		next_tape_block;
	int		data_block_num;
	int		tape_block_count;
	int		tape_blocks_to_dump;

	/* determine how many tape blocks will be needed for this request */

	tape_blocks = howmany(frags_to_write * super_block->fs_fsize, TP_BSIZE);

	/* determine how many tape blocks there are per full data block */

	tblks_per_dblk = super_block->fs_bsize / TP_BSIZE;

	/* loop until all of the needed tape blocks have been written out */

	for (next_tape_block = 0; next_tape_block < tape_blocks; next_tape_block += tape_block_count)
	{
		/* the number of tape blocks to write in this iteration is */
		/* the minimum of the number of tape blocks left to write, or */
		/* the number which can be mapped by one TS_INODE/TS_ADDR */
		/* record's c_addr array */

		tape_block_count = min(tape_blocks - next_tape_block, TP_NINDIR);

		/* determine whether the data for each tape block actually */
		/* exists in the source data file */

		for (i = 0; i < tape_block_count; ++i)
		{
			/* determine which data block corresponds to the tape */
			/* block */

			data_block_num = (next_tape_block + i) / tblks_per_dblk;

			/* see if the data block corresponding to the tape */
			/* block exists */

			if (data_block_ptr_array[data_block_num] != (daddr_t) 0)
			{
				/* if the data block pointer is valid */
				/* (non-zero), put a 1 in the corresponding */
				/* tape block's map array entry */

				spcl.c_addr[i] = 1;
			}
			else
			{
				/* if the data block pointer is phony */
				/* (zero), put a 0 in the corresponding */
				/* tape block's map array entry */

				spcl.c_addr[i] = 0;
			}
		}

		/* now that the tape block map has been completed, write */
		/* out the TS_INODE/TS_ADDR record, which immediately */
		/* precedes the data corresponding to the map */

		spcl.c_count = tape_block_count;
		spclrec();

		/* now loop to write out the actual data blocks */

		for (i = 0; i < tape_block_count; i += tape_blocks_to_dump)
		{
			/* the number of tape blocks to write from this data */
			/* block is the minimum of the number of tape blocks */
			/* left, or the number of tape blocks per disk block */

			tape_blocks_to_dump = min(tape_block_count - i, tblks_per_dblk);

			/* determine which data block pointer corresponds to */
			/* this next group of tape blocks */

			data_block_num = (next_tape_block + i) / tblks_per_dblk;

			/* if the data block really exists (its pointer is */
			/* non-zero), call dmpblk to write out the data block */

			if (data_block_ptr_array[data_block_num] != (daddr_t) 0)
			{
				dmpblk(data_block_ptr_array[data_block_num], tape_blocks_to_dump * TP_BSIZE);
			}
		}

		/* set the c_type field of the spcl record to TS_ADDR so that */
		/* we will know that subsequent spcl records for this file */
		/* are TS_ADDR (continuation) records, and not the TS_INODE */
		/* (initial) record */

		spcl.c_type = TS_ADDR;
	}
}

/* Bitmap() writes out the specified bit map to tape, preceded by a TS_BITS */
/* or TS_CLRI (depending on map_record_type) spcl record. */

void
bitmap(map_to_write, map_record_type)
	char	       *map_to_write;
	int		map_record_type;
{
	register int	i;
	char	       *cp;

	/* figure out how many tape blocks will be needed to write the */
	/* bit map */

	spcl.c_count = howmany(imap_size * sizeof(map_to_write[0]), TP_BSIZE);

	/* fill in the spcl record type and write the spcl record */

	spcl.c_type = map_record_type;
	spclrec();

	/* write the map itself */

	for (i = 0, cp = map_to_write; i < spcl.c_count; ++i, cp += TP_BSIZE)
	{
		taprec(cp);
	}
}

/* Volume_label() writes a volume label record at the beginning of a tape */

void
volume_label()
{
	/* put out a tape volume label TS_TAPE spcl record first thing on */
	/* each tape */

	spcl.c_flags |= DR_NEWHEADER;
	spcl.c_volume = curr_volume_num;
	spcl.c_type = TS_TAPE;
	spclrec();
	spcl.c_flags &= ~DR_NEWHEADER;
}

/* Job_trailers() writes out the trailer records at the end of the job */

void
job_trailer()
{
	int		i;

	/* put out a whole tape record of full of trailer records to */
	/* make sure the last tape blocks gets written (losing some of */
	/* these trailer records, however, is no problem) */

	spcl.c_type = TS_END;
	for (i = 0; i < blocks_per_write; ++i)
	{
		spclrec();
	}
}

/* Spclrec() writes out the global spcl record to tape. */
/* Certain of the fields of the record must have been filled in prior to */
/* calling spclrec().  Other fields will be filled in here.  Also, the */
/* checksum for the record will be calculated here and filled in. */

static void
spclrec()
{
	register int	cumulative_sum;
	register int	*next_int_ptr;
	register int	i;

	/* fill in the current inode number field from the global variable */

	spcl.c_inumber = curr_inum;

	/* fill in the magic number from the defined constant */

	spcl.c_magic = NFS_MAGIC;

	/* put zero in the checksum field before computing the checksum */

	spcl.c_checksum = 0;

	/*	compute (int) checksum of spcl record	*/

	next_int_ptr = (int *) &spcl;
	cumulative_sum = 0;

	for (i = sizeof(union u_spcl) / (4 * sizeof(int)); i > 0; --i)
	{
		cumulative_sum += *next_int_ptr++;
		cumulative_sum += *next_int_ptr++;
		cumulative_sum += *next_int_ptr++;
		cumulative_sum += *next_int_ptr++;
	}

	/* now put the computed checksum into the checksum field */

	spcl.c_checksum = CHECKSUM - cumulative_sum;

	/* write out the completed spcl record */

	taprec((char *) &spcl);
}

/*	Getino() returns a pointer to a buffer containing the requested	*/
/*	inode.  If the inode is not already in the local buffer, it is	*/
/*	read from the disk.						*/

static struct dinode  *
getino(inode_number)
	register ino_t		inode_number;
{
	static ino_t		curr_min_inum = (ino_t) ULONG_MAX;
	static ino_t		curr_max_inum = (ino_t) 0;
	static union
	{
		char		dummy[MAXBSIZE];
		struct dinode	i_b;
	}			ino_buf;
	static struct dinode   *inode_buffer = &ino_buf.i_b;

	/* see if the requested inode is already in the buffer, by virtue */
	/* of a prior request */

	if (inode_number < curr_min_inum || curr_max_inum < inode_number)
	{
		/* desired inode is not currently in the buffer, so read */
		/* in the block which contains it */

		bread(fsbtodb(super_block, itod(super_block, inode_number)), (char *) inode_buffer, (int) super_block->fs_bsize);

		/* set the min and max+1 inumber limits to the new numbers of */
		/* the first and last+1 inodes which were just read into the */
		/* buffer */

		curr_min_inum = inode_number - (inode_number % super_block->fs_inopb);
		curr_max_inum = curr_min_inum + super_block->fs_inopb - 1;
	}

	/* return a pointer to the requested inode within the inode buffer */

	return(&inode_buffer[inode_number - curr_min_inum]);
}

/*	Bread() reads from the disk into a buffer, given a starting	*/
/*	block number and the number of bytes to read (presumably an	*/
/*	integral number of blocks).  					*/

#define	BREADEMAX 16

void
bread(device_block_num, recv_buffer, bytes_wanted)
	daddr_t			device_block_num;
	register char	       *recv_buffer;
	register int		bytes_wanted;
{
	static int		breaderrors = 0;
	register int		bytes_got;

	/* enter a loop which will request smaller and smaller reads */
	/* until sucessful, or failure for an unknown reason */

	while (TRUE)
	{
		/* seek to the correct position in the raw file-system file */

		if (lseek(in_disk_fd, ((off_t)device_block_num * dev_bsize), 0) == (off_t) -1)
		{
			msg(MSGSTR(LSEEKF, "Bad lseek in input disk file: %s, block number: %d\n"),
			    disk_file_name, device_block_num);
			dump_perror("bread(): lseek()");
			bzero(recv_buffer, bytes_wanted);
			return;
		}

		/* do the read from the disk */

		bytes_got = read(in_disk_fd, recv_buffer, bytes_wanted);

		/* if we got what we wanted, ok, return */

		if (bytes_got == bytes_wanted)
		{
			return;
		}

		/* if we didn't get what we wanted, check to see if */
		/* the reason for it is acceptable */

		if (device_block_num + (bytes_wanted / dev_bsize) > fsbtodb(super_block, super_block->fs_size))
		{
			/*
			 * Trying to read the final portion of file system.
			 *
			 * NB - dump only works in TP_BSIZE blocks, hence
			 * rounds dev_bsize fragments up to TP_BSIZE pieces.
			 * It should be smarter about not actually trying to
			 * read more than it can get, but for the time being
			 * we punt and scale back the read only when it gets
			 * us into trouble. (mkm 9/25/83)
			 */

			bytes_wanted -= dev_bsize;
			bzero(recv_buffer + bytes_wanted, dev_bsize);
			continue;
		}

		/* if we get here, the read was unsuccessful, and for no */
		/* good reason, such as that cited above, so break out of */
		/* loop to error handler */

		break;
	}

	/* complain to the operator */

	msg(MSGSTR(NOTHAP, "Bad read from input disk file: %s, block number: %d, bytes wanted: %d, bytes got: %d\n"),
	    disk_file_name, device_block_num, bytes_wanted, bytes_got);
	dump_perror("bread(): read()");

	/* bump up the accumulated number of errors */

	if (++breaderrors > BREADEMAX)
	{
		/* If too many errors of this sort have accumulated, */
		/* something serious is wrong.  Ask the operator if he */
		/* really wants to continue. (He SHOULD answer no) */

		msg(MSGSTR(BREADERR, "More than %d block read errors from %s\n"), BREADEMAX, disk_file_name);
		if (notify_flag == TRUE)
		{
			broadcast(MSGSTR(DUMPAIL, "DUMP IS AILING!\7\7\n"));
		}
		msg(MSGSTR(UNRECERR, "This is an unrecoverable error.\n"));

#if	EDUMP

		abort_dump();

		/* NOTREACHED */

#else	! EDUMP

		if (query(MSGSTR(CONTINU, "Do you want to attempt to continue")) == NO)
		{
			abort_dump();

			/* NOTREACHED */
		}

		/* operator is willing to live with the errors, so reset */
		/* the error counter to zero */

		breaderrors = 0;

#endif	! EDUMP

	}

	/* fill up the rest of the receiving buffer which didn't get */
	/* was requested with zero bytes (for safety's sake) */

	if (bytes_got < 0)
	{
		bytes_got = 0;
	}
	bzero(recv_buffer + bytes_got, bytes_wanted - bytes_got);
}

/*
 * This is an estimation of the number of TP_BSIZE blocks in the file. It
 * estimates the number of blocks in files with holes by assuming that all of
 * the blocks accounted for by di_blocks are data blocks (when some of the
 * blocks are usually used for indirect pointers); hence the estimate may be
 * high.
 */

static int
est(ip)
	struct dinode  *ip;
{
	long		size_tape_blocks;
	long		block_tape_blocks;
	long		calc_tape_blocks;


	/*
	 * ip->di_size is the size of the file in bytes.
	 * ip->di_blocks stores the number of blocks actually in the file.
	 *
	 * If there are more blocks than the size would indicate, this just
	 * means that there are indirect blocks in the file or an unused
	 * portion in the last file block; we can safely ignore these blocks.
	 *
	 * If the file is bigger than the number of blocks would indicate,
	 * then the file has holes in it. In this case we must use the
	 * block count to estimate the number of data blocks used, but we
	 * use the actual size for estimating the number of indirect dump
	 * blocks (size_tape_blocks vs. block_tape_blocks in the indirect
	 * block calculation).
	 */
	size_tape_blocks = howmany(ip->di_size, TP_BSIZE);
	block_tape_blocks = howmany(dbtob(ip->di_blocks), TP_BSIZE);
	calc_tape_blocks = min(size_tape_blocks, block_tape_blocks);
	if (ip->di_size > super_block->fs_bsize * NDADDR)
	{
		/* calculate the number of indirect blocks on the dump tape */

		calc_tape_blocks += howmany(size_tape_blocks - NDADDR * super_block->fs_bsize / TP_BSIZE, TP_NINDIR);
	}
	if (calc_tape_blocks > (super_block->fs_size * super_block->fs_fsize)) {
			msg(MSGSTR(CORDIR, "Corrupted directory, i-node: %d\n"), curr_inum);
			abort_dump();
	}
	return(calc_tape_blocks);
}
