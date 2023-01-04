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
static char	*sccsid = "@(#)$RCSfile: dumptape.c,v $ $Revision: 4.2.5.7 $ (DEC) $Date: 1993/12/16 15:23:29 $";
#endif 

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
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
 *
 * Copyright (c) 1980 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

#include	"dump.h"

#include	<sys/ipc.h>
#include	<sys/sem.h>
#include	<errno.h>
#include	<sys/ioctl.h>
#include	<sys/mtio.h>

static int	atomic();
static void	alloc_tape();
static void	change_tape();
static void     check_point();
static void	end_slaves();
static void	flusht();
static void	get_semaphore();
static char     *getdevinfo();
static void	initialize_slaves();
static void	kill_slaves();
static void	mount_tape();
static void	remove_semaphore();
static void	remove_shared_memory();
static void	rewind_tape();
static void	sigpipe();
static void	slave_block();
static void	slave_unblock();
static void	slave_wait();
static void	slave_work();
static void	start_slaves();
static void	tape_error();
static void	timeest();
static void	unmount_tape();
static void	write_to_slave();

/*
 * This module provides the routines that support the
 * concurrent disk block reading and tape writing.  A master
 * process forks several slave processes.  The master queues up
 * requests and each slave will process the request by reading
 * a disk block and/or writing the data to tape.  It is
 * expected that while one slave is writing to tape, the other
 * slaves are reading disk blocks into their buffers.
 * Meanwhile, the master is traversing the filesystem queing up
 * more requests.
 * 
 * Using semaphores, the master reserves a FIFO element and
 * starts collect requests until it has blocks_per_write tape
 * blocks' worth (dmpblk(), taprec(), and flusht()).  It then,
 * releases the FIFO element for the next available slave.  The
 * master then waits for the next available FIFO element and
 * continues.
 * 
 * Meanwhile, a slave is waiting for the FIFO element to be
 * ready to process.  Once the master gives the OK, it starts
 * processing the request.  After the slave has finished
 * reading the disk blocks, it waits for the previous slave to
 * finish writing unblock the waiting slave.  Once unblocked,
 * the slave issues a write and then unblocks the next slave to
 * start writting.
 * 
 * There are two requests 1) data which is written directly to
 * tape and 2) disk, which is a disk address and length.  There
 * is not additional work for the slave to process a data
 * request.  A disk request will require the slave to read the
 * specified disk blocks.
 * 
 * The FIFO is a piece of shared memory that contains a number
 * of request buffers and tape buffers.  The tape buffers is
 * the actual data that is written to the tape.  When the
 * master issues a data reqest it will copy the data directly
 * into the tape buffer.
 */

#define RQTYPE_DATA      0
#define RQTYPE_ADDRESS   1

#define	NUM_SLAVES	 3              /* why 3?
					 * 1 slave writing
					 * 1 slave reading
					 * 1 slave for slack
					 */

#define	NUM_FIFO_BLOCKS	(NUM_SLAVES+1)  /* why n slaves + 1?
					 * 1 per slave to process
					 * 1 for master while slaves are
					 *   working
					 */

static int	master_pid = 0;	/* pid of master, for sending error signals */
static int	next_slave;	/* next slave to be instructed */

static int	slave_pid[NUM_SLAVES] = {0};	/* slave pids used by kill_slaves() */
static int	slave_semid = 0;		/* handle for slave semaphores */
static int      fifo_free_semid = 0;            /* handle for fifo free semaphores */
static int      fifo_ready_semid = 0;           /* handle for fifo ready semaphores */

static int      fifo_shmid = 0;                 /* handle for fifo shared memory */
static int      fifo_index = 0;                 /* this varialble is the current fifo */
						/* index for the process.  The value */
						/* is differs between the master and */
						/* each of the slaves. */
static char	maybeTA90[24];			/* Hold whatever device used */

/* miscellaneous shared memory block structure, facilitates communication
 * between the master and slaves.
 */

struct sb_block
{
	long slaves_done_flag;
};


/* request packet sent from master to slave */

struct rq_block
{
	long            type;
	daddr_t		device_block_num;
	long		num_blocks;
};


/* tape block structure, used in both tape-buffer and pipe-block array */

struct tp_block
{
	char		dummy[TP_BSIZE];
};


/* fifo block structure, shared memory structure used to deliver the
 * requests from the parent to the slave 
 */

struct fifo_block
{
	struct rq_block *request_buffer;
	struct tp_block *tape_buffer;
};

/* miscellaneous shared memory variables */

static struct sb_block *slave_buffer;
static long slave_buffer_size;

/* these variables used to collect requests which are sent down the pipe */
/* from master to slave */

static struct rq_block  *request_buffer;
static long request_buffer_size;


/* fifo variables */

static struct fifo_block *fifo;
static long fifo_size;

/* shared memory variables, these variables encompass the */
/* entire shared memory buffer space used in the program */

static char *shm;
static u_int shm_size;

/* these variables used by the slave to assemble the data, from both the */
/* pipe and the disk, which are to be written to tape */

static struct tp_block *tape_buffer;
static long             tape_buffer_size;
static long             tape_buffer_index;
static long             tape_write_size;

/* these variables are used for keeping track of how much of the tape */
/* and how many blocks have been used so far */

static time_t	time_start_write;	/* when started writing the first
					 * tape block on first tape */
static long	tot_blocks_written = 0; /* total number of blocks written
					 * so far */

long	lastspclrec = -1;	/* tape block number of last written header */

/* alloc_tape() is the initialization routine for the output processes. */
/* Its primary duty is to allocate buffer space for each the request buffer */
/* and tape-buffer. */

static void
alloc_tape()
{
	long            i;
	unsigned long	offset;
	char	        *buffer;

	/* calculate buffer sizes */

	slave_buffer_size = sizeof(struct sb_block);
	fifo_size = sizeof(struct fifo_block);
	request_buffer_size = blocks_per_write * sizeof(struct rq_block);
	tape_buffer_size = (blocks_per_write * sizeof(struct tp_block)) + getpagesize();
	shm_size = slave_buffer_size + (NUM_FIFO_BLOCKS * (fifo_size + request_buffer_size + tape_buffer_size));

	tape_write_size = blocks_per_write * sizeof(struct tp_block);

	/* allocate shared memory  */

	if ((fifo_shmid = shmget (IPC_PRIVATE, shm_size, 0600|IPC_CREAT)) == -1)
	{
		msg(MSGSTR(SHMGETE, "Cannot get shared memory\n"));
		dump_perror("alloc_tape(): shmget()");
	}
	if ((shm = (char *) shmat(fifo_shmid,(void *)0,0)) == (char *) -1)
	{
		msg(MSGSTR(SHMATTE, "Cannot attach shared memory\n"));
		dump_perror("alloc_tape(): shmat()");
	}
	
	/* calcualte pointers, note that the tape buffer is larger */
	/* than the required block size so that its base address is */
	/* page aligned to insure optimal tape speed on a variety */
	/* of hardware platforms.  */

	(char *) slave_buffer = (char *) shm;
	(char *) fifo = (char *) slave_buffer + slave_buffer_size;
	(char *) request_buffer = (char *) fifo + (NUM_FIFO_BLOCKS * fifo_size);
        (char *) tape_buffer = (char *) request_buffer + (NUM_FIFO_BLOCKS * request_buffer_size);
	for (i=0; i<NUM_FIFO_BLOCKS; i++)
	{
		(char *) fifo[i].request_buffer = (char *) (request_buffer) + (i * request_buffer_size);
		offset = getpagesize() - 1;		
		(char *) fifo[i].tape_buffer = (char *) (tape_buffer) + (i * tape_buffer_size);
		(char *) fifo[i].tape_buffer = (char *) (((unsigned long) fifo[i].tape_buffer + offset) & ~offset);
	}

}

/* Taprec() writes to the slave pipe (puts in the next request & pipe */
/* buffers) a single tape block pointed to by the argument.  It is called */
/* by spclrec() to output single spcl records, and iteratively by bitmap() */
/* to output bit maps. */

void
taprec(block_to_write)
	char	       *block_to_write;
{

	/* fill the next request block as follows:
	 *
	 *                  type = RQTYPE_DATA
	 *      device_block_num = not used
	 *            num_blocks = 1
	 *
	 *           tape_buffer = contents of block_to_write;
	 */

	request_buffer[tape_buffer_index].type = RQTYPE_DATA;
	request_buffer[tape_buffer_index].num_blocks = 1;
	tape_buffer[tape_buffer_index] = *((struct tp_block *) block_to_write);

	/* increment the record number in the spcl record */

	lastspclrec = spcl.c_tapea;
	++spcl.c_tapea;

	/* increment the count of tape blocks and call flusht() if enough */
	/* tape blocks' worth have been accumulated for a full tape write */

	tape_buffer_index++;
	if (tape_buffer_index == blocks_per_write)
	{
		flusht();
	}
}

/* Dmpblk() arranges for the writing of the file system data block whose */
/* block number is disk_block_num and has length byte_length.  It does not */
/* actually access the data, but only puts the request in the next slot */
/* in the request array. */

void
dmpblk(disk_block_num, byte_length)
	daddr_t		disk_block_num;
	int		byte_length;
{
	int		device_block_num;
	int		remaining_tape_blocks;
	int		blocks_to_write;

	device_block_num = fsbtodb(super_block, disk_block_num);

	remaining_tape_blocks = byte_length / TP_BSIZE;

	/* Figure out how many tape blocks to write this iteration. */
	/* This may involve splitting the request over multiple */
	/* tape/pipe-writes. */

	while ((blocks_to_write = min(remaining_tape_blocks, blocks_per_write - tape_buffer_index)) > 0)
	{
		/* fill the next request block as follows:
		 *
		 *                  type = RQTYPE_ADDRESS
		 *      device_block_num = startign disk block address
		 *            num_blocks = number of blocks to write
		 *
		 *           tape_buffer = contents of block_to_write;
		 */
		
		request_buffer[tape_buffer_index].type = RQTYPE_ADDRESS;
		request_buffer[tape_buffer_index].device_block_num = device_block_num;
		request_buffer[tape_buffer_index].num_blocks = blocks_to_write;

		spcl.c_tapea += blocks_to_write;

		/* increment the count of tape blocks and call flusht() */
		/* if enough tape blocks have been accumulated for a */
		/* full tape write */

		tape_buffer_index += blocks_to_write;
		if (tape_buffer_index == blocks_per_write)
		{
			flusht();
		}

		device_block_num += blocks_to_write * (TP_BSIZE / dev_bsize);
		remaining_tape_blocks -= blocks_to_write;
	}
}

/* Flusht() is called when the request array is full of enough requests */
/* to do a whole tape write.  The work is dispatched to the next slave */
/* in the ring. */

static void
flusht()
{
	struct sembuf  sembuf[2];

	/* Set FIFO entry ready, 
	 * A slave has decremented the semaphore and is sleeping 
	 * waiting for the semaphore to increase (or become zero).
	 * So, when incrementing the semaphore the master makes 
	 * the FIFO entry ready and the slave starts processing the
	 * request.  See slave_work() for the additional information.
	 */

	sembuf[0].sem_num = fifo_index;
	sembuf[0].sem_op = 1;
	sembuf[0].sem_flg = 0;

	if (semop(fifo_ready_semid, sembuf, 1) < 0)
	{
		msg(MSGSTR(CNFIFORDY, "Cannot set fifo_ready_semid[%d]\n"),fifo_index);
		dump_perror("flusht(): semop()");
		abort_dump();

		/* NOT REACHED */
	}


	/* Any space left? 
	 * If dump is writing to a removeable disk then the 
	 * "blocks_written" is based on an ioctl which provides a 
	 * very accurate capacity measurement.  That value will be used 
	 * to determine when the medium is full.  For tape or 
	 * cartridge, the mechanism is the early warning mark.  The 
	 * early warning mark communicates to to the application that 
	 * end-of-tape is close and it should write out its buffers. 
	 * For dump, this is primarily done in slave_work() and 
	 * flusht().  When slave_work() detects early warning, ENOSPC, 
	 * it will set a flag and continue writing.  This routine, 
	 * flusht(), will wait for the slaves to complete writing out 
	 * the buffers.  Capacity limitations do not exists with other 
	 * mediums including pipe and regular file. 
	 */
 
	/* This if statement MUST BE THE SAME with the if statement in end_slaves() */

	if (((medium_flag == DISKETTE) && (blocks_written >= full_size_blocks)) || 
	    ((medium_flag == TAPE) && (slave_buffer->slaves_done_flag == TRUE)))
	{
		change_tape();
	}
	else
	{

		/* get next fifo index */

		fifo_index = (fifo_index + 1) % NUM_FIFO_BLOCKS;

		/* Wait for FIFO entry to be free and set to "not-free".
		 * The master will sleep until the respective semaphore becomes
		 * zero by the slave.  Once the semaphore value becomes zero, the 
		 * semaphore will be incremented and the master will be free to 
		 * add the request.
		 */

		sembuf[0].sem_num = fifo_index;
		sembuf[0].sem_op = 0;
		sembuf[0].sem_flg = 0;
		
		sembuf[1].sem_num = fifo_index;
		sembuf[1].sem_op = 1;
		sembuf[1].sem_flg = 0;
		
		if (semop(fifo_free_semid, sembuf, 2) < 0)
		{
			msg(MSGSTR(CNFIFOFREE, "Cannot set fifo_free_semid[%d]\n"),fifo_index);
			dump_perror("flusht(): semop()");
			abort_dump();
			
			/* NOT REACHED */
		}

		/* update counters, pointers, and indices */

		blocks_written += blocks_per_write;
		inches_written += inches_per_write;

		request_buffer = fifo[fifo_index].request_buffer;
		tape_buffer = fifo[fifo_index].tape_buffer;
		tape_buffer_index = 0;
	}


	/* update summary statistics and report information to user */
	
	tot_blocks_written += blocks_per_write;
	timeest();
}

/*
 * print out an estimate of the amount of time left to do the dump
 */

static void
timeest()
{
	static time_t	time_next_message = (time_t) 0;
	time_t		time_now;
	double		portion_finished;
	time_t		time_writing;
	time_t		total_time_needed;
	time_t		time_remaining;

	/* get the current time */

	(void) time(&time_now);

	/* if this is the first time in this routine, initialize the */
	/* next-message time to now */

	if (time_next_message == (time_t) 0)
	{
		time_next_message = time_now;
	}

	/* if fewer than 500 blocks have been written, then making an */
	/* estimate now would not be a good idea, because the potential */
	/* error would be too significant */

	if (tot_blocks_written < 500)
	{
		return;
	}

	/* if it's time to issue the next message (at least five minutes */
	/* after the last one), then do it */

	if (time_now >= time_next_message)
	{
		/* figure out what portion of the work is finished */

		portion_finished = (double) tot_blocks_written / est_tot_blocks;

		/* figure out how much more time is needed */

		time_writing = time_now - time_start_write;
		total_time_needed = (time_t) (time_writing / portion_finished);
		time_remaining = total_time_needed - time_writing;

		/* tell the operator */

		msg(MSGSTR(FINISH, "%3.2f%% done -- finished in %02d:%02d\n"),
		    portion_finished * 100.0,
		    time_remaining / HOUR,
		    (time_remaining % HOUR) / MINUTE);

		/* set the time of the next message to at least five */
		/* minutes from now */

		time_next_message = time_now + 300;
	}
}

void
open_at_start()
{
	/* alloc the request, pipe, and tape buffers */

	alloc_tape();

	/* open tape file */

	open_tape();

	spcl.c_tapea = 0;

	if (estimate_only_flag == FALSE)
	{
		/* Each tape's child process is a new master and gets */
		/* a new set of slaves. */

		initialize_slaves();

		/* get the current time so we know when we started writing the */
		/* first tape */

		(void) time(&time_start_write);

		/* put volume label on tape first thing */

		volume_label();
	}
}

static void
change_tape()
{
	int		blocks_in_prev_c_addr;
	int		i;
	struct mtop	mt;

	/* close up slaves and wait for them to finish */

	end_slaves();

	/* remove semaphore structures */

	remove_semaphore();

	/* force tape to rewind to load point */

	rewind_tape(TRUE);

	if (!strcmp(maybeTA90,"TA90") || !strcmp(maybeTA90,"TA91")) {
	    msg(MSGSTR(CHANGT3,"Allowing TA90 to rewind and eject ...\n" )); 
	    sleep(60);
	}

	/* ask to mount new tape, and mount it */

	mount_tape();

	/* open tape file */

	open_tape();

	++curr_tape_num;
	++curr_volume_num;
	new_tape_flag = TRUE;

	/* Each tape's child process is a new master and gets */
	/* a new set of slaves. */

	initialize_slaves();

	/* calculate the number of data blocks remaining in the current */
	/* section of the file which was begun on the previous tape; */
	/* these data blocks will immediately follow the tape label, and */
	/* will be followed by the next spcl record of this tape */

	blocks_in_prev_c_addr = 0;
	for (i = 0; i < spcl.c_count; ++i)
	{
		if (spcl.c_addr[i] != 0)
		{
			++blocks_in_prev_c_addr;
		}
	}
	spcl.c_count = blocks_in_prev_c_addr - (spcl.c_tapea - (lastspclrec + 1));

	/* put volume label on tape first thing */

	volume_label();
}

void
close_at_end()
{
	/* 


	/* close up slaves and wait for them to finish */
	end_slaves();

	/* remove semaphore structures */
	remove_semaphore();

	/* rewind the last tape, but do not force if no rewind specified */
	rewind_tape(FALSE);

	/* prompt to unmount it */
	unmount_tape();

	/* remove fifo shared memory buffer */
	remove_shared_memory();
}


void
rewrite_tape()
{
	/* kill slaves and wait for them to quit */

	kill_slaves();

	/* remove semaphore structures */

	remove_semaphore();

	/* force tape to rewind to load point */

	rewind_tape(TRUE);

	/* mount tape */

	mount_tape();

	/* When this process exits with status X_REWRITE, its parent, */
	/* who is waiting, starts over again from the last checkpoint */
	/* on the tape this process was trying to write. */

	Exit(X_REWRITE);

	/* NOT REACHED */
}


/* Rewind_tape rewinds the tape and waits until the tape device */
/* file can be reopened, which means it is at load point. */

static void
rewind_tape(force_rewind_flag)
	int		force_rewind_flag;
{
	struct mtop	mt;
	
	/* close the tape device file
	 *
	 * In order to rewind (or in this case take the tape
	 * offline) the proper tape marks must be written.
	 * Some tapes like the QIC differ how to write an EOF
	 * mark.  The following code is intended to insure
	 * the proper EOF mark is written on the tape wihtout
	 * "special casing" the code for each device.  The
	 * code will close the device which will write the
	 * tape marks.  It will then re-open the device,
	 * issue the MTOFFL tape operation.  In either case,
	 * the routine will exit with the device closed.
	 */

	if ((medium_flag == TAPE) && (no_rewind_flag == FALSE || force_rewind_flag == TRUE) )
	{
		msg(MSGSTR(TREWIND, "Rewinding and unloading tape\n"));

#ifdef REMOTE
	
		rmtclose();
	
		if ((out_tape_fd = rmtopen(tape_file_name, O_WRONLY | O_CREAT | O_TRUNC)) < 0)
		{
			msg(MSGSTR(OPENTF, "Cannot open device file %s\n"), tape_file_name);
			dump_perror("rewind_tape(): open()");
		}

		if (rmtioctl(MTOFFL, 1) < 0)
		{
			msg(MSGSTR(TREWF, "Tape rewind failed\n"));
			dump_perror("rewind_tape(): ioctl()");
		}

#else /* REMOTE */	       

		if (close(out_tape_fd) < 0)
		{
			msg(MSGSTR(CLOSEE, "Close error\n"));
			dump_perror("rewind_tape(): close()");
		}


		if ((out_tape_fd = open(tape_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
		{
			msg(MSGSTR(OPENTF, "Cannot open device file %s\n"), tape_file_name);
			dump_perror("rewind_tape(): open()");
		}

		mt.mt_op = MTOFFL;
		mt.mt_count = 1;
		if (ioctl(out_tape_fd, MTIOCTOP, &mt) < 0)
		{
			msg(MSGSTR(TREWF, "Tape rewind failed\n"));
			dump_perror("rewind_tape(): ioctl()");
		}

#endif /* REMOTE */

	}

	/* close the tape device file */

#ifdef REMOTE

	rmtclose();

#else /* REMOTE */

	if (close(out_tape_fd) < 0)
	{
		msg(MSGSTR(CLOSEE, "Close error\n"));
		dump_perror("rewind_tape(): close()");
	}

#endif /* REMOTE */

}

/* mount_tape() asks the user to mount a tape. */

static void
mount_tape()
{
	switch (medium_flag) {
		
	case REGULAR_FILE:
		break;
		
	case TAPE:
		msg(MSGSTR(CHANGT1, "Change Tape: Mount volume %d, tape # %04d\n"), curr_volume_num + 1, curr_tape_num + 1);
		if (notify_flag == TRUE)
		{
			broadcast(MSGSTR(CHANGT2, "CHANGE TAPE!\7\7\n"));
		}
		break;
		
	case DISKETTE:
		msg(MSGSTR(CHANGD1, "Change Diskette: Insert volume %d, diskette # %04d\n"), curr_volume_num + 1, curr_tape_num + 1);
		if (notify_flag == TRUE)
		{
			broadcast(MSGSTR(CHANGD2, "CHANGE DISKETTE!\7\7\n"));
		}
		break;
	default:
		msg(MSGSTR(ILLMEDIUM, "%s(%d):Illegal flag %d.\n"),
		    __FILE__, __LINE__, medium_flag);
		break;
	}
}


static void
unmount_tape()
{
	if (pipe_out_flag == TRUE || no_rewind_flag == TRUE)
	{
		return;
	}

	switch (medium_flag) {
	case REGULAR_FILE:
		break;
	case TAPE:
		msg(MSGSTR(UNMNTT1, "Unmount last tape\n"));
		if (notify_flag == TRUE)
		{
			broadcast(MSGSTR(UNMNTT2, "UNMOUNT LAST TAPE!\7\7\n"));
		}
		break;
	case DISKETTE:
		msg(MSGSTR(UNMNTD1, "Remove last diskette\n"));
		if (notify_flag == TRUE)
		{
			broadcast(MSGSTR(UNMNTD2, "REMOVE LAST DISKETTE!\7\7\n"));
		}
		break;
	default:
		msg(MSGSTR(ILLMEDIUM, "%s(%d):Illegal flag %d.\n"),
				__FILE__, __LINE__, medium_flag);
		break;
	}
}

/* tape_error() is called when a SIGUSR1 is received by the master from a */
/* slave who experienced a tape write error.  */

static void
tape_error()
{
	if (by_blocks_flag == TRUE)
	{
		msg(MSGSTR(WRTBE, "Write error - %s, volume %d, %d blocks  -- cannot recover\n"), tape_file_name, curr_volume_num, blocks_written);
	}
	else
	{
		msg(MSGSTR(WRTFE, "Write error - %s, volume %d, %d feet -- cannot recover\n"), tape_file_name, curr_volume_num, (int) ((inches_written / 12) + 0.5));
	}
	
	if (notify_flag)
	{
		broadcast(MSGSTR(WRTNE, "WRITE ERROR!\7\7\n"));
	}
	
	/* allow the user and opportunity to re-dump a tape or diskette */

	if (medium_flag == TAPE || medium_flag == DISKETTE)
	{
		if (query(MSGSTR(RESTART, "Do you want to restart this volume")) == NO)
		{
			abort_dump();

			/* NOT REACHED */
		}

		switch (medium_flag) {
		case TAPE:
			msg(MSGSTR(REPLACET1, "Replace the faulty tape with a new one\n"));
			msg(MSGSTR(REPLACET2, "after it has finished rewinding\n"));
			break;
		case DISKETTE:
			msg(MSGSTR(REPLACED, "Replace the faulty diskette with a new one\n"));
			break;
		}

		msg(MSGSTR(REWRITE1, "This dump volume will be rewritten\n"));
		msg(MSGSTR(REWRITE2, "If this volume contained more than one\n"));
		msg(MSGSTR(REWRITE3, "dump, only the last one will be rewritten\n"));
		msg(MSGSTR(REWRITE4, "on the new volume; previous dumps are still\n"));
		msg(MSGSTR(REWRITE5, "intact on the faulty volume\n"));

		rewrite_tape();

		/* NOT REACHED */
	}
	else
	{
		abort_dump();
		
		/* NOT REACHED */
	}
}


/*
 * We implement taking and restoring checkpoints on the tape level.
 *
 * Before each tape is opened, check_point() is called to created a new
 * process by forking; this saves all of the necessary context in the parent.
 *
 * The child continues the dump; the parent waits around, saving the context.
 *
 * If the child returns X_REWRITE, then it had problems writing that tape;
 * this causes the parent to fork again, duplicating the context, and
 * everything continues as if nothing had happened.
 */

static void
check_point()
{
	int		parent_pid;
	int		child_pid;
	int		child_status;
	int		waitpid;
	void	      (*save_int)();

	parent_pid = getpid();

	/* save the interrupt handler so that it can be restored */
	/* by the parent each time just before it is going to fork a new */
	/* child */

	save_int = signal(SIGINT, SIG_IGN);

	while (TRUE)
	{

		/* restore the interrupt handler for the child */

		(void) signal(SIGINT, save_int);

		if ((child_pid = fork()) < 0)
		{
			msg(MSGSTR(FORKF, "Checkpoint fork fails in parent %d\n"), parent_pid);
			dump_perror("check_point(): fork()");
			abort_dump();

			/* NOT REACHED */
		}

		if (child_pid != 0)
		{
			/*
			 * PARENT: save the context by waiting until the child
			 * doing all of the work exits.
			 *
			 * Don't catch the interrupt, child will do it
			 */

			(void) signal(SIGINT, SIG_IGN);

#if TDEBUG
			msg("volume: %d, tape: %d, parent process: %d, child process %d\n",
			    curr_volume_num + 1, curr_tape_num + 1, parent_pid, child_pid);
#endif /* TDEBUG */

			/* wait for the child to return */

			while ((waitpid = wait(&child_status)) != child_pid)
			{
				msg(MSGSTR(DUMPPWAIT, "Parent %d waiting for child %d has another child %d return\n"),
				    parent_pid, child_pid, waitpid);
			}

			if (child_status & 0xFF)
			{
				msg(MSGSTR(CHILDR, "Child %d returns low-byte status %o\n"), child_pid, child_status & 0xFF);
			}

			child_status = (child_status >> 8) & 0xFF;

			/* exit with child's return status unless it was */
			/* X_REWRITE, in which case loop and try to write */
			/* the tape again. */

			switch (child_status)
			{
			case X_FINOK:

#ifdef TDEBUG
				msg("Child %d exits X_FINOK\n", child_pid);
#endif /* TDEBUG */

				Exit(X_FINOK);

				/* NOT REACHED */

			case X_ABORT:

#ifdef TDEBUG
				msg("Child %d exits X_ABORT\n", child_pid);
#endif /* TDEBUG */

				abort_dump();

				/* NOT REACHED */

			case X_REWRITE:
#ifdef TDEBUG
				msg("Child %d exits X_REWRITE\n", child_pid);
#endif	TDEBUG
				continue;

			default:
#ifdef TDEBUG
				msg("Child %d exits unknown %d\n", child_pid, child_status);
#endif /* TDEBUG */

				msg(MSGSTR(BADCODE, "Bad exit status from child %d: %d\n"), child_pid, child_status);
				abort_dump();

				/* NOT REACHED */
			}

			/* NOT REACHED */
		}

		/* we are the child process */

		/* break out of loop and return to caller to do the */
		/* rest of the work */

#ifdef TDEBUG
		/* allow time for parent's message to get out */

		(void) sleep(5);

		msg("Volume: %d, tape: %d, parent process: %d, child process %d\n",
		    curr_volume_num + 1, curr_tape_num + 1, parent_pid, getpid());
#endif /* TDEBUG */
		break;

	}

	/* only the child process reaches here to return to caller */
}


void
open_tape()
{
	struct mtop	mt;

	/* call check_point() as a lazy man's way */
	/* to preserve the WHOLE CONTEXT of the */
	/* process */

	check_point();

#ifdef REMOTE

	while ((out_tape_fd = rmtopen(tape_file_name, O_WRONLY | O_CREAT | O_TRUNC)) < 0)
	{
		msg(MSGSTR(OPENRTF, "Cannot open remote device file %s\n"), tape_file_name);

#else /* REMOTE */

	while ((out_tape_fd = (pipe_out_flag == TRUE)? 1: open(tape_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
	{
		msg(MSGSTR(OPENTF, "Cannot open device file %s\n"), tape_file_name);

#endif /* REMOTE */

		if (query(MSGSTR(RTRYOP, "Do you want to retry the open")) == NO)
		{
			abort_dump();

			/* NOT REACHED */
		}
	}

	strcpy(maybeTA90,getdevinfo(tape_file_name));  /* get device information */
	blocks_written = 0;
	inches_written = 0.0;

	/* Enable EOT detection so that the "early warning" is */
	/* detected.  This section is redundant the first time, but the */
	/* routine is called from change_tape() where the EOT detection */
	/* was disabled so that dump could write after the "early */
	/* warning" and before true EOT. */

	if (medium_flag == TAPE)
	{
		mt.mt_op = MTENAEOT;
		mt.mt_count = 1;

#ifdef REMOTE

		rmtioctl (MTENAEOT,1);

#else /* REMOTE */

		if(ioctl(out_tape_fd, MTIOCTOP, &mt) < 0)
		{
			msg (MSGSTR(MTENAEOTF, "Cannot enable end-of-tape\n"));
			dump_perror("open_tape(): ioctl()");
		}

#endif /* REMOTE */

	}

		
}

/* abort_dump() causes entire dump job to abort immediately */

void
abort_dump()
{
	if (master_pid == 0)
	{
		/* remove shared memory buffer */
		remove_shared_memory();		

		/* no master nor slaves yet */
		if (invalid_file_system == TRUE)
			msg(MSGSTR(ENTIRE, "The ENTIRE dump is aborted\n"));
		else
			if (estimate_only_flag == FALSE)
			{
				(void) kill(master_pid, SIGTERM);
				msg(MSGSTR(ENTIRE, "The ENTIRE dump is aborted\n"));
			}

	}
	else if (master_pid == getpid())
	{
		/* master, kill all slaves */
		kill_slaves();

		/* remove semaphore structures */
		remove_semaphore();
		
		/* remove shared memory buffer */
		remove_shared_memory();		
	}
	else
	{
		/* is slave */
		/* Signal master to call abort_dump */

		(void) kill(master_pid, SIGTERM);
	}
	Exit(X_ABORT);

	/* NOT REACHED */
}

/* initialize semaphores for slave control, and start up the slaves */

static void
initialize_slaves()
{
	int             i;
	int		slave_num;

	/* initialize fifo and shared memory variables */

	fifo_index = 0;
	request_buffer = fifo[fifo_index].request_buffer;
	tape_buffer = fifo[fifo_index].tape_buffer;
	tape_buffer_index = 0;
	slave_buffer->slaves_done_flag = FALSE;	

	/* allocate the semaphores with which the slaves will coordinate */
	/* the tape writing */

	get_semaphore();

	/* block all slaves from writing the tape initially */

	for (slave_num = 0; slave_num < NUM_SLAVES; ++slave_num)
	{
		slave_block(slave_num);
	}

	/* start up the slaves which will do the disk reading and tape */
	/* writing; the master will just do the inode traversal */

	start_slaves();

	/* pick first (zeroth) slave as next one */

	next_slave = 0;

	/* unblock the first slave to start the slave coordination loop */

	slave_unblock(next_slave);
}


/* This routine is either called when the medium has filled up (EOT) or
 * dump has completed backing up the filesystem.  If the medium has
 * filled up then all "non-free buffers" can be written.  If the dump has
 * completed backing up the filesystem then the last buffer is normally
 * not written.  This occurs because dump writes a blocks_per_write
 * TS_END blocks and dumpfiles are rarely perfect multiples of
 * blocks_per_write.  Thus, some of the TS_END blocks overflow into
 * another tape block.  The dump format requires only one TS_END block.
 * When the routine is called by change_tape() the code waits for all the
 * slaves to finish writing their buffers.  When the routine is called by
 * close_at_end() the code frees the current FIFO entry and then waits
 * for the slaves to finish emptying the tape buffers.  The master kills
 * the slaves and waits for them to exit.
 */

static void
end_slaves()
{
	int		fifo_num;
	int		slave_num;
	struct sembuf   sembuf[NUM_FIFO_BLOCKS];

	/* Wait for buffers to become free by slaves.  If
	 * end of dump (ie. not changing tapes) then delete
	 * last request buffer.  That buffer contains extra
	 * TS_END blocks that do not need to be written.  See
	 * above for more information.
	 */

	for (fifo_num = 0; fifo_num < NUM_FIFO_BLOCKS; fifo_num++)
	{
		sembuf[fifo_num].sem_num = fifo_num;
		sembuf[fifo_num].sem_op = 0;
		sembuf[fifo_num].sem_flg = 0;
	}

	/* This if statement MUST BE THE SAME with the if statement in flusht() */

	if (((medium_flag == DISKETTE) && (blocks_written >= full_size_blocks)) || 
	    ((medium_flag == TAPE) && (slave_buffer->slaves_done_flag == TRUE)))
	{
		sembuf[fifo_index].sem_op = 0;
	}
	else
	{
		sembuf[fifo_index].sem_op = -1;
	}

	if (semop(fifo_free_semid, sembuf, NUM_FIFO_BLOCKS) < 0)
	{
		msg(MSGSTR(CNFIFOFREE, "Cannot set fifo_free_semid[%d]\n"),fifo_index);
		dump_perror("flusht(): end_slaves()");
		abort_dump();
		
		/* NOT REACHED */
	}

	/* Kill slaves and wait for slaves ... */

	for (slave_num = 0; slave_num < NUM_SLAVES; ++slave_num)
	{
		if (slave_pid[slave_num] != 0)
		{
			(void) kill(slave_pid[slave_num], SIGKILL);
		}
		slave_pid[slave_num] = 0;
	}

	while (wait(NULL) >= 0)
	{
		;
	}
}


/* This routine sends a SIGKILL signal to each slave and waits 
 * for them to finish
 */

static void
kill_slaves()
{
	register int	slave_num;

	/* Kill slaves and wait for slaves ... */

	for (slave_num = 0; slave_num < NUM_SLAVES; ++slave_num)
	{
		if (slave_pid[slave_num] != 0)
		{
			(void) kill(slave_pid[slave_num], SIGKILL);
		}
		slave_pid[slave_num] = 0;
	}
	while (wait(NULL) >= 0)
	{
		;
	}
}

/* Exit() simply passes arg through to exit after print message in debug mode */

void
Exit(status)
{

#ifdef TDEBUG

	msg("Pid %d exits with status %d\n", getpid(), status);

#endif /* TDEBUG */

	exit(status);

	/* NOT REACHED */
}

/*
 * Start the slave processes.  Each slave will enter the routine
 * slave_work() and never return.  Only the master returns to the
 * calling routine.
 */

static void
start_slaves()
{
	void sigpipe();
	void tape_error();
	void abort_dump();
	register int	slave_num;

	master_pid = getpid();

	(void) signal(SIGTERM, abort_dump);	/* Slave sends SIGTERM on abort_dump() */
	(void) signal(SIGUSR1, tape_error);	/* Slave sends SIGUSR1 on tape errors */

	for (slave_num = 0; slave_num < NUM_SLAVES; ++slave_num)
	{
		slave_pid[slave_num] = 0;
	}

	for (slave_num = 0; slave_num < NUM_SLAVES; ++slave_num)
	{
		if ((slave_pid[slave_num] = fork()) < 0)
		{
			msg(MSGSTR(SLAVES2, "Cannot fork slave %d\n"), slave_num);
			dump_perror("start_slaves(): fork()");
			abort_dump();

			/* NOT REACHED */
		}

#ifdef TDEBUG

		msg("slavepid[%d] %d\n", slave_num, slave_pid[slave_num]);

#endif /* TDEBUG */

		if (slave_pid[slave_num] == 0)
		{	
			/* Slave starts up here */

			/* ignore interrupt; master will catch it */

			(void) signal(SIGINT, SIG_IGN);

			/* do slave work and do not return */

			slave_work(slave_num);

			/* NOT REACHED */
		}

	}

	/* only master returns */

}

/*
 * Synchronization - each slave has an associated semaphore. 
 * When one slave completes its tape write, it increments the semaphore
 * value of the next slave in the loop, so the next slave can awaken.
 * The current slave then decrements its own semaphore value, to zero,
 * at which point the slave will block waiting for the previous slave in
 * the loop to increment it again.  This essentially a circular token
 * passing scheme of slaves.
 */

/* slave_work() is where the slave stays until the master inserts */
/* another instuction in the FIFO.  When the slave leaves this routine */
/* it will exit immediately in the calling routine. */

static void
slave_work(slave_num)
	int		slave_num;
{
	long             i;
	long		 bytes_written;
	long		 next_slave_in_loop;
	struct mtop	 mt;
	struct sembuf    sembuf[1];

	next_slave_in_loop = (slave_num + 1) % NUM_SLAVES;

	/* get slave's own seek pointer into input disk file by closing */
	/* and reopening the file */

	(void) close(in_disk_fd);
	if ((in_disk_fd = open(disk_file_name, O_RDONLY)) < 0)
	{
		msg(MSGSTR(CRODF, "Cannot re-open file system file %s\n"), disk_file_name);
		dump_perror("slave_work(): open()");
		abort_dump();

		/* NOT REACHED */
	}


	/* INFINITE LOOP that processes requests from the master */

	for (fifo_index = slave_num; ; fifo_index = (fifo_index + NUM_SLAVES) % NUM_FIFO_BLOCKS )
	{

		/* Wait for FIFO entry to be ready
		 *
		 * Here, the slave will decrement the
		 * semaphore and sleep until the semaphore
		 * value increases.  If result of the
		 * semaphore operation is zero or when the
		 * semaphore value increases the slave will
		 * begin processing the request.  See flusht()
		 * for additional information.
		 */

		sembuf[0].sem_num = fifo_index;
		sembuf[0].sem_op = -1;
		sembuf[0].sem_flg = 0;
		
		if (semop(fifo_ready_semid, sembuf, 1) < 0)
		{
			msg(MSGSTR(CNFIFORDY, "Cannot set fifo_ready_semid[%d]\n"),fifo_index);
			dump_perror("slave_work(): semop()");
			abort_dump();
			
			/* NOT REACHED */
		}

		/* get pointers for request buffer and tape buffer */

		request_buffer = fifo[fifo_index].request_buffer;
		tape_buffer = fifo[fifo_index].tape_buffer;

		/* Fill in the blank subblocks of the tape buffer. */
		/* Each request contains an address or data.  If the */
		/* request is data, it has already been copied into */
		/* its respective location in the tape buffer.  If it */
		/* is an address then this code segment will issue a */
		/* disk read for the requested address */

		for (i = 0; i < blocks_per_write; i += request_buffer[i].num_blocks)
		{
			/* This section of code will fill in the tape */
			/* blocks that need to be read by the disk. */
			/* If the type is RQTYPE_ADDRESS then the */
			/* process will starting reading the data from */
			/* the disk */

			if (request_buffer[i].type == RQTYPE_ADDRESS)
			{
				bread(request_buffer[i].device_block_num,
				      (char *) &tape_buffer[i],
				      (int) (request_buffer[i].num_blocks * sizeof(struct tp_block)));
			}
		}

		/* We now have a full buffer to write to the tape.
		 * We must wait for the last slave to complete
		 * writing its buffer.  This insures proper write
		 * synchronization.  Note, it also provides a set of
		 * locking for modifying some of the shared memory
		 * values.
		 */

		slave_wait(slave_num);

		/* paranoia, clear errno to insure it is zero

		errno = 0;

		/* Note, a while is needed here when the write fails
		 * due to ENOSPC.  In that situation, EOT will be
		 * disabled and the write will be reissued.
		 */

#ifdef REMOTE
		while ((bytes_written = rmtwrite((char *) tape_buffer, tape_write_size)) != tape_write_size)
#else /* REMOTE */ 
		while ((bytes_written = write(out_tape_fd, (char *) tape_buffer, tape_write_size)) != tape_write_size)
#endif /* REMOTE */
		{

			/* Write the buffer.  This region of code is
			 * dependent upon the EOT handling implemented within
			 * the basesystem.  Most tapes have a mechanism for
			 * reporting to the application that its head is
			 * nearing the true EOT.  This is done using a foil
			 * marker for magtapes and a thing called "early
			 * warning" mark for SCSI devices.  When the tape
			 * head reaches this point on the tape, the write
			 * call is expected to return a -1 and errno =
			 * ENOSPC.  The application will then disable "early
			 * EOT" detection and finish writing any pending
			 * buffers.  Any remaining errors or true EOT will be
			 * returned as a write error.
			 */

			if ((medium_flag == TAPE) && (bytes_written == -1) && (errno == ENOSPC))
			{
				/* Early warning EOT detected - notify master,  */
				/* disable EOT, and reset variables. */

				mt.mt_op = MTDISEOT;
				mt.mt_count = 1;

#ifdef REMOTE

				rmtioctl (MTDISEOT,1);

#else /* REMOTE */

				if(ioctl(out_tape_fd, MTIOCTOP, &mt) < 0)
				{
					msg (MSGSTR(MTDISEOTF, "Cannot disable end-of-tape\n"));
					dump_perror("slave_work(): ioctl()");
					abort_dump();

					/* NOT REACHED */
				}
				mt.mt_op = MTCSE;
				mt.mt_count=1;
				if (ioctl(out_tape_fd, MTIOCTOP, &mt) < 0)
				{
				     msg (MSGSTR(MTCSE ,"MTCSE failure!\n" ));
				     abort_dump();

					/* NOT REACHED */
				}
#endif /* REMOTE */
				slave_buffer->slaves_done_flag = TRUE;
				errno = 0;
			}
			else
			{
				if ((pipe_out_flag == TRUE) || (medium_flag == REGULAR_FILE)) break;
				/* Write error - report error to user, signal
				 * the master, and wait ... 
				 */
  
				msg(MSGSTR(WRTE, "Write error -- wanted to write: %d, only wrote: %d\n"),tape_write_size, bytes_written);
				dump_perror("slave_work(): write()");

				(void) kill(master_pid, SIGUSR1);
				for (;;)
				{
					(void) sigpause(0);
				}
			}
		}

		/* free fifo block */

		sembuf[0].sem_num = fifo_index;
		sembuf[0].sem_op = -1;
		sembuf[0].sem_flg = 0;
		
		if (semop(fifo_free_semid, sembuf, 1) < 0)
		{
			msg(MSGSTR(CNFIFOFREE, "Cannot set fifo_free_semid[%d]\n"),fifo_index);
			dump_perror("slave_work(): semop()");
			abort_dump();
			
			/* NOT REACHED */
		}
		
		/* block ourself until so the previous slave can wake */
		/* us up next time around */

		slave_block(slave_num);

		/* unblock the next slave so he can write the next tape block */

		slave_unblock(next_slave_in_loop);
	}

}

/*
 * gets a semaphore id associated with an array of NUM_SLAVES semaphores
 * to be used to coordinate tape writes
 */

static void
get_semaphore()
{
	int               slave_num;
	int               fifo_num;
	/*key_t*/ long    key;
	struct sembuf     sembuf[1];

#ifdef FILE_TO_KEY

	/* make keys from ftok() */

	char			tmpname[256];
	int			fd;
	extern /*key_t*/ long	ftok();

	(void) strcpy(tmpname, "/tmp/dumplockXXXXXX");
	(void) mktemp(tmpname);

	if ((fd = open(tmpname, O_CREAT | O_EXCL, 0600)) < 0)
	{
		msg(MSGSTR(LOCKF, "Cannot create key-creation lock file %s\n", tmpname));
		dump_perror("get_semaphore(): open()");
		abort_dump();

		/* NOT REACHED */
	}

	if ((key = ftok(tmpname, 's')) < 0)
	{
		msg(MSGSTR(FTOKF, "Cannot obtain key\n"));
		dump_perror("get_semaphore(): ftok()");
		abort_dump();

		/* NOT REACHED */
	}

	(void) close(fd);
	(void) unlink(tmpname);

#endif /* FILE_TO_KEY */
#ifdef MAKE_KEY

	/* make key with top-bytes 'b' and 'r', and last-bytes getpid() */

	key = (/*key_t*/ long) ((((long) 'b') << 24) |
				(((long) 'r') << 16) |
				(((long) getpid()) & 0xFFFF));

#else /* MAKE_KEY */

	key = IPC_PRIVATE;

#endif /* MAKE_KEY */

	/*
	 * Get and initialize NUM_FIFO_BLOCKS "fifo_free_semid" semaphores.
	 * FIFO entry is free when semaphore value is zero.  Otherwise, fifo
	 * entry is not free.
	 */

	if ((fifo_free_semid = semget(key, NUM_FIFO_BLOCKS, 0600 | IPC_CREAT)) < 0)
	{
		msg(MSGSTR(SEMGETE, "Cannot get semaphores\n"));
		dump_perror("get_semaphore(): semget() for fifo_free_semid");
		abort_dump();

		/* NOT REACHED */
	}

	for (fifo_num = 0; fifo_num < NUM_FIFO_BLOCKS; ++fifo_num)
	{
		if (semctl(fifo_free_semid, fifo_num, SETVAL, 0) < 0)
		{
			msg(MSGSTR(SEMCTLE, "Cannot set semaphore values\n"));
			dump_perror("get_semaphore(): semctl() for fifo_free_semid");
			abort_dump();

			/* NOT REACHED */
		}
	}


	/* Set first FIFO entry to "not-free".  Some of the other routines
	 * that interact with this semaphore is flusht() and slave_work().
	 */

	sembuf[0].sem_num = fifo_index;
	sembuf[0].sem_op = 1;
	sembuf[0].sem_flg = 0;
		
	if (semop(fifo_free_semid, sembuf, 1) < 0)
	{
		msg(MSGSTR(CNFIFOFREE, "Cannot set fifo_free_semid[%d]\n"),fifo_index);
		dump_perror("get_semaphore(): semop()");
		abort_dump();
		
		/* NOT REACHED */
	}


	/*
	 * Get and initialize NUM_FIFO_BLOCKS "fifo_ready_semid" semaphores.
	 * FIFO entry is ready when semaphore value increases.  Otherwise, fifo
	 * entry is not ready.
	 */

	if ((fifo_ready_semid = semget(key, NUM_FIFO_BLOCKS, 0600 | IPC_CREAT)) < 0)
	{
		msg(MSGSTR(SEMGETE, "Cannot get semaphores\n"));
		dump_perror("get_semaphore(): semget() for fifo_ready_semid");
		abort_dump();

		/* NOT REACHED */
	}

	for (fifo_num = 0; fifo_num < NUM_FIFO_BLOCKS; ++fifo_num)
	{
		if (semctl(fifo_ready_semid, fifo_num, SETVAL, 0) < 0)
		{
			msg(MSGSTR(SEMCTLE, "Cannot set semaphore values\n"));
			dump_perror("get_semaphore(): semctl() for fifo_ready_semid");
			abort_dump();

			/* NOT REACHED */
		}
	}

	/*
	 * Get and initialize NUM_SLAVES semaphores
	 */

	if ((slave_semid = semget(key, NUM_SLAVES, 0600 | IPC_CREAT)) < 0)
	{
		msg(MSGSTR(SEMGETE, "Cannot get semaphores\n"));
		dump_perror("get_semaphore(): semget() for slave_semid");
		abort_dump();

		/* NOT REACHED */
	}

	for (slave_num = 0; slave_num < NUM_SLAVES; ++slave_num)
	{
		if (semctl(slave_semid, slave_num, SETVAL, 0) < 0)
		{
			msg(MSGSTR(SEMCTLE, "Cannot set semaphore values\n"));
			dump_perror("get_semaphore(): semctl() for slave_semid");
			abort_dump();

			/* NOT REACHED */
		}
	}
}

/* block the slave whose number is passed as the argument by adding 1 to */
/* the associated semaphore in the semaphore array bringing its value up to 1 */
/* so that when the blocked slave does slave_wait() it will block until */
/* its predecessor unblocks it by subtracting 1 from its semaphore bringing */
/* its value back to 0 */

static void
slave_block(slave_num)
	int		slave_num;
{
	struct sembuf	sembuf;

	sembuf.sem_num = slave_num;
	sembuf.sem_op = 1;
	sembuf.sem_flg = IPC_NOWAIT;

	if (semop(slave_semid, &sembuf, 1) < 0 && errno != EAGAIN)
	{
		msg(MSGSTR(CNBSP, "Cannot block slave process %d\n"), slave_num);
		dump_perror("slave_block(): semop()");
		abort_dump();

		/* NOT REACHED */
	}
}

/* unblock the slave whose number is passed as the argument by subtracting 1 */
/* from the associated semaphore in the semaphore array bringing its value */
/* back to 0 so that the blocked slave which is waiting in slave_wait() */
/* will unblock and be allowed to proceed */

static void
slave_unblock(slave_num)
	int		slave_num;
{
	struct sembuf	sembuf;

	sembuf.sem_num = slave_num;
	sembuf.sem_op = -1;
	sembuf.sem_flg = IPC_NOWAIT;

	if (semop(slave_semid, &sembuf, 1) < 0 && errno != EAGAIN)
	{
		msg(MSGSTR(CNUSP, "Cannot unblock slave process %d\n"), slave_num);
		dump_perror("slave_unblock(): semop()");
		abort_dump();

		/* NOT REACHED */
	}
}

/* suspend execution until the semaphore in the semaphore array indexed */
/* by slave_num is unblocked (reset to zero) by the previous slave */

static void
slave_wait(slave_num)
	int		slave_num;
{
	struct sembuf	sembuf;

	sembuf.sem_num = slave_num;
	sembuf.sem_op = 0;
	sembuf.sem_flg = 0;

	if (semop(slave_semid, &sembuf, 1) < 0)
	{
		msg(MSGSTR(CNWSP, "Slave process %d cannot wait successfully\n"), slave_num);
		dump_perror("slave_wait(): semop()");
		abort_dump();

		/* NOT REACHED */
	}
}

/*
 * This routine will remove all the semaphores including the FIFO free semaphores, 
 * FIFO ready semaphores, and slave semaphores.
 */

static void
remove_semaphore()
{

	if (fifo_free_semid != 0)
	{
		if (semctl(fifo_free_semid, 0, IPC_RMID, 0) < 0)
		{
			msg(MSGSTR(SEMCTL2, "Cannot remove semaphores\n"));
			dump_perror("remove_semaphore(): semctl() for fifo_free_semid");
		}
		fifo_free_semid = 0;
	}

	if (fifo_ready_semid != 0)
	{
		if (semctl(fifo_ready_semid, 0, IPC_RMID, 0) < 0)
		{
			msg(MSGSTR(SEMCTL2, "Cannot remove semaphores\n"));
			dump_perror("remove_semaphore(): semctl() for fifo_ready_semid");
		}
		fifo_ready_semid = 0;
	}

	if (slave_semid != 0)
	{
		if (semctl(slave_semid, 0, IPC_RMID, 0) < 0)
		{
			msg(MSGSTR(SEMCTL2, "Cannot remove semaphores\n"));
			dump_perror("remove_semaphore(): semctl() for slave");
		}
		slave_semid = 0;
	}
}

/*
 * The tape is completed, so we can now remove the semaphore id.
 */

static void
remove_shared_memory()
{
	if (fifo_shmid != 0)
	{
		if (shmctl(fifo_shmid, IPC_RMID, 0) < 0)
		{
			msg(MSGSTR(SHMRME, "Cannot remove shared memory\n"));
			dump_perror("remove_shared_memory(): shmctl()");
		}
		fifo_shmid = 0;
	}	
}

/*
 * Since a read from a pipe may not return all we asked for, or a write may not
 * write all we ask if we get a signal, loop until the count is satisfied (or
 * error).
 */

static int
atomic(func, fd, buf, count)
	int	      (*func)(), fd, count;
	char	       *buf;
{
	int		got, need = count;

	while ((got = (*func)(fd, buf, need)) > 0 && (need -= got) > 0)
	{
		buf += got;
	}
	return((got < 0)? got: count - need);
}


struct tape_density_element_t {
	long	category_stat;  /* bitmask value of density as defined from 
				 * DEVIOCGET ioctl structure
				 */

	long	density;        /* numerical value of density as defined from
				 * DEVIOCGET ioctl structure */

};

struct tape_info_element_t {
	char   *device;   /* name of the device.  For example, "TLZ04". */

	size_t blocks;    /* capacity of device.  This structure contains
			   * the number of blocks that can be contained
			   * by the specified device.  The blocksize is
			   * specified as TP_BSIZE in <dumprestore.h>.
			   * The capacity was reduced by 5% for tapes less
			   * than 512MB and 10% for tapes greater than or
			   * equal to 512MB.
			   */

	size_t feet;      /* length of tape.  The length according to the
			   * device specification was not used.  The formal
			   * for calculating the tape length is as follows:
			   *
			   * feet =   blocks * 1024
			   *         ----------------
			   *         density_bpi * 12
			   *
			   * The density_bpi is based on the expected value
			   * returned by the DEVIOCGET ioctl.
			   */

	double record_gap;   
	                  /* record gap - not too accurate.  The previous
			   * code used 0.45 inches and a maximum value which
			   * has been maintained for this code.
			   */
};


struct devget   devinfo;
DEVGEOMST       devgeom;

static struct tape_density_element_t tape_density_table[] = {
	DEV_800BPI,	800,
	DEV_1600BPI,	1600,
	DEV_6250BPI,	6250,
	DEV_6666BPI,	6666,
	DEV_10240BPI,	10240,
	DEV_38000BPI,	38000,
	DEV_38000_CP,	38000,
	DEV_76000BPI,	76000,
	DEV_76000_CP,	76000,
	DEV_8000_BPI,	8000,
	DEV_10000_BPI,	10000,
	DEV_16000_BPI,	16000,
	DEV_61000_BPI,	61000,
	DEV_54000_BPI,	54000,
	DEV_42500_BPI,	42500, /* Frey added this for TZ85, TA857, TZ86*/
	DEV_45434_BPI,	45434, /* Frey added this also for TKZ09  */
	0,		0
};

static struct tape_info_element_t tape_info_table[] = {
	DEV_TA78,	    141056,       1925,       0.45,
	DEV_TA79,	    141056,       1925,       0.45,
	DEV_TA81,	    141056,       1925,       0.45,
	DEV_TA90,	    194560,        436,       0.45,
	DEV_TA91,	    194560,        436,       0.45,
	DEV_TF30,	     92416,       1182,       0.00,
	DEV_TF70,	    287948,       2457,       0.00,
	DEV_TF70L,	    287948,       2457,       0.00,
	DEV_TK50,	     92416,       1182,       0.00,
	DEV_TK70,	    287948,       2457,       0.00,
	DEV_TKZ09,	   4718592,       7456,       0.00,
	DEV_TLZ04,	   1132646,       1584,       0.00,
	DEV_TLZ06,	   1887436,       2640,       0.00,
	DEV_TS05,	     38912,       2075,       0.45,
	DEV_TU77,	    141056,       1925,       0.45,
	DEV_TU78,	    141056,       1925,       0.45,
	DEV_TU80,	    141056,       1925,       0.45,
	DEV_TU81,	    141056,       1925,       0.45,
	DEV_TU81E,	    141056,       1925,       0.45,
	DEV_TZ05,	     38912,       2075,       0.45,
	DEV_TZ07,	    141056,       1925,       0.45,
	DEV_TZ30,	     92416,       1182,       0.00,
	DEV_TZ85,	   2453299,       4925,       0.00,
	DEV_TZ86,	   2453299,       4925,       0.00,
	DEV_TZ857,	   2453299,       4925,       0.00,
	DEV_TZK08,	   2073600,       3276,       0.00,
	DEV_TZK10,	    483840,       2580,       0.00,
	0,                       0,          0,       0.00
};

/* Getdevinfo() gets info about the "tape" output file, */
/* primarily to determine whether or not it is a diskette. */

static  char
*getdevinfo(medium_file_name)
	char	       *medium_file_name;
{
	int             i,j;  
	struct stat	stb;

#ifdef REMOTE

	/* There should be a strategy to determine the exact medium_flag
	 * for remote accesses.
	 */

	medium_flag = REGULAR_FILE;

#else /* REMOTE */

	/* treat non existing medium as regular file, which is created later */
	if ((pipe_out_flag == TRUE) || (access(medium_file_name, F_OK) < 0))
	{
		medium_flag = REGULAR_FILE;
	}
	else 
	{
		while ( stat(medium_file_name, &stb) < 0 )
		{
			msg(MSGSTR(CNOTST, "Cannot stat %s\n"),medium_file_name);
			if (query(MSGSTR(DYWTR, "Do you want to retry")) == NO)
			{
				abort_dump();

				/* NOT REACHED */
			}
		}
			
		if ( S_ISREG(stb.st_mode) || (strcmp(medium_file_name,"/dev/null") == 0 ) ) 
		{
			medium_flag = REGULAR_FILE;
		}
		else
		{				
			if (ioctl(out_tape_fd, DEVIOCGET, &devinfo) == -1)
			{				
				msg(MSGSTR(IOCE, "Cannot get device information for %s\n"), medium_file_name);
				dump_perror("getdevinfo(): ioctl()");
				abort_dump();
				
				/* NOT REACHED */
			}
				
			if (devinfo.category == DEV_DISK )
			{
				medium_flag = DISKETTE;
			}
			else
			{
				medium_flag = TAPE;
			}
		}
	}

#endif /* REMOTE */


	/* If output device is a diskette, calculate capacity.
	 *
	 * This is done using the DEVGETGEOM ioctl.  This is implicitly
	 * documented in devio.h.  For DEC OSF/1, devio.h is located in
	 * /usr/include/io/common/devio.h.  Beware, it is not an interface
	 * that is resistance to change.
	 *
	 * The ioctl returns the defaulted number of blocks for the 
	 * c-partition.  The code assumes that a block is 512-byte units 
	 * and full_size expects the calculation to be in 1024-byte 
	 * units.  Thus, the returned value is divided by two.  The code
	 * defaults to 1 GB.
	 */

	switch (medium_flag) {

	case DISKETTE:
		by_blocks_flag = TRUE;
		
		if (full_size == 0)
		{
			devgeom.geom_info.dev_size = 0;
			if (ioctl(out_tape_fd, DEVGETGEOM, (char *)&devgeom) < 0)
			{
				msg(MSGSTR(GEOME, "Cannot get geometry information for %s\n"), medium_file_name);
			}
			
			if (devgeom.geom_info.dev_size != 0)
			{
				full_size = devgeom.geom_info.dev_size / 2;
			}
			else
			{
				full_size = 1048576;	/* about 1GB */
			}
		}

		/* truncate to largest multiple of blocks_per_write */
		full_size_blocks = ( full_size / blocks_per_write ) * blocks_per_write;

		break;



	case TAPE:

		if (strstr(tape_file_name,"/dev/nrmt") != 0)
		{
			no_rewind_flag = TRUE;
		}
		
		/* Search for tape name within table and exit with an index
		 * pointing to either an entry within the name of the tape or
		 * NULL entry.
		 */

		for (i = 0; ((tape_info_table[i].device != 0) && (strcmp(devinfo.device,tape_info_table[i].device) != 0)); i++);
		if (tape_info_table[i].device == 0)
		{
			i = -1;
		}

		/* This section is responsible for determining the */
		/* size  of the tape in blocks and the number of */
		/* blocks per write.  The number of blocks was */
		/* calculated above. */

		if (by_blocks_flag == TRUE)
		{
		        if (full_size == 0)
			{
				if (i >= 0)
				{
					full_size = tape_info_table[i].blocks;
				}
				else
				{
					full_size = 1048576;	/* about 1GB */
				}
			}
			full_size_blocks = full_size;
		}
		
                /* Calculate the length of tape and number of inches */
		/* per write. */
		
		else
		{
			if (full_size == 0)
                        {
				if (i >= 0)
				{
					full_size = tape_info_table[i].feet;
				}
				else
				{
					full_size = 2300;	/* 2300 feet */
				}
			}
			full_size_inches = full_size * 12;			

			if (tape_density == 0)
			{
				tape_density = 1600;
				for (j = 0; tape_density_table[j].density != 0; j++)
				{
					if ((devinfo.category_stat & tape_density_table[j].category_stat) != 0)
					{
						tape_density = tape_density_table[j].density;
						break;
					}
				}
			}
			if (i>0)
			{
				tape_record_gap = tape_info_table[i].record_gap;
			}
			else
			{
				tape_record_gap = 0.45;
			}
			inches_per_write = (double) blocks_per_write * TP_BSIZE / tape_density + tape_record_gap;
		}
		break;
	}

#ifdef TDEBUG	
	msg ("\n");
	msg ("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");	
	msg("File name='%s'\n",medium_file_name);
	msg("File descriptor=%d\n",out_tape_fd);
	msg("Device name='%s'\n",devinfo.device);
	if (pipe_out_flag == TRUE)
	{
		msg("pipe_out_flag=TRUE\n");
	}
	else
	{
		msg("pipe_out_flag=FALSE\n");
	}
	switch (medium_flag) {
	case NO_MEDIUM:
		msg("medium_flag=NO_MEDIUM\n");
		break;
	case REGULAR_FILE:
		msg("medium_flag=REGULAR_FILE\n");
		break;
	case TAPE:
		msg("medium_flag=TAPE\n");
		break;
	case DISKETTE:
		msg("medium_flag=DISKETTE\n");
		break;
	}
	msg ("full_size=%d (%d blocks, %6.2f inches)\n",full_size,full_size_blocks,full_size_inches);
	msg ("tape_density=%d, tape_record_gap=%6.2f\n",tape_density,tape_record_gap);
	msg ("blocks_per_write=%d (%6.2f inches)\n",blocks_per_write, inches_per_write);
	if (no_rewind_flag == TRUE)
	{
		msg("no_rewind_flag=TRUE\n");
	}
	else
	{
		msg("no_rewind_flag=FALSE\n");
	}
	msg ("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	msg ("\n");
#endif /* TDEBUG */
	return(devinfo.device);
}
