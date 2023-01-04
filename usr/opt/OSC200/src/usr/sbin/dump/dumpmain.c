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
static char   *sccsid = "@(#)$RCSfile: dumpmain.c,v $ $Revision: 4.2.12.7 $ (DEC) $Date: 1993/12/16 15:22:01 $";
#endif 

/* 
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 Release 1.0
 */
/* 
 * dumpmain.c
 * 
 *    Modification History:
 * 
 * 01-Apr-91	Fred Canter
 * 	MIPS C 2.20+, changes for -std
 * 
 * 21-Jul-91	Sam Lambert
 * 	Added support for "knowledege" of DEC cartridge tape devices.
 * 	Module getdevinfo() now issues a DEVIOCGET ioctl, determines
 * 	device density, and sets variables 'tape_density' and 'full_size'
 * 	of tape (via an internal table which must be updated when new 
 * 	devices come along).  This allows the capacity calculations to
 * 	be accurate for each device we support.
 * 
 * 25-Jul-91	Sam Lambert
 * 	If user specifies "nrmt" in the output device string then
 * 	force the "no_rewind_flag" (-N) to be set (for use in 
 * 	rewind_tape(), module dumptape.c).
 * 	Also do check if (argc < 2) to avoid segfault error from 
 * 	dereferencing a null pointer if *argv == NULL in main() 
 * 	(eg, check for an arglist).  Comments in the code indicate
 * 	some default action was expected to be taken (9u), but there
 * 	is no supporting code for this default, and no input device
 * 	indicated as the default.   ???
 * 
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

/* 
 * 	Possible enhancements:
 * 
 * 	1) To reduce the number of processes (currently one per output
 * 	volume plus one, which could get quite numerous when using
 * 	floppies for output), combine all the global variables into one
 * 	structure and for each tape, don't fork, rather recurse, with
 * 	the first task being to malloc a new global variable structure,
 * 	copy the old values into it, and proceed from there.
 * 
 * 	2) To reduce disk accesses (inodes in particular), buffer all the
 * 	inodes.  This should be more practical now that large address spaces
 * 	are available.  To keep the space required from getting out of
 * 	hand when forks are done, save them in a shared memory segment.
 */

#include	"dump.h"

static int	bsd_style();
static void     usage();

/* 
 * We catch these signals
 */

static void		sighup();
static void		sigtrap();
static void		sigfpe();
static void		sigbus();
static void		sigalrm();
static void		sigsegv();
static void		sigterm();
static void		sigintr();

static int		bmapest();

static void		handle_signal();
char	*tc_label	=  NULL;  /* Added for inclusion of user label */

void
main(argc, argv)
	int			argc;
	char		       *argv[];
{
	char			local_host_name[256];
	int			i;
	char		       *str;
	int			update_flag = TRUE; /* update dump history */
	double			d_est_tot_tapes;
	register struct fstab  *disk_fstab_entry;
	register struct fstab  *new_disk_fstab_entry; /* Used to fix qar 12022*/
	int			optchar;
	int			num_mounted_fs;
	char			mount_info[8192];
	struct vmount	       *mount_tab_entry;
	char		       *mount_device;
	char		       *remote_host_name;
	time_t			end_time;


	catd = catopen(MF_DUMP, NL_CAT_LOCALE);

	/* initialize NLS catalog and yes and no strings */
	setlocale(LC_ALL, "");

	if ((str = getenv("YESSTR")) != NULL)
	{
		yes_flag = TRUE;
		yes_str = str;
	}
	if ((str = getenv("NOSTR")) != NULL)
	{
		no_flag = TRUE;
		no_str = str;
	}

	bzero((char *)&u_spcl, sizeof(u_spcl));

	/* get our host name */

	if (gethostname(local_host_name, sizeof(local_host_name)) < 0)
	{
		msg(MSGSTR(CNGLHN, "Cannot get local host name\n"));
		dump_perror("main(): gethostname()");
		abort_dump();
	}

	/* get start time */

	(void) time(&spcl.c_date);

	/* if no args are given, then default arg is 9u, otherwise	*/
	/* u option must be explicitly specified			*/

	if (argc > 1)
	{
		update_flag = FALSE;
	}

	/* process command line arguments */
	/* BSD style uses first argument string to determine the 
	 * meaning of the following arguments. In this case, the
	 * value doesn't follow immediately the option.
	 */

	if ( (argc > 1) && (bsd_style(argc, argv)) ) {
	   char *arg;

	   if(argc > 1) {
		argv++;
		argc--;
		arg = *argv;
		if (*arg == '-')
			argc++;
	   }
	   while(*arg)
		switch (*arg++) {
		case 'w':
			lastdump('w');		/* tell us only what has to be done */
			Exit(X_FINOK);

			/* NOT REACHED */

		case 'W':			/* what to do */
			lastdump('W');		/* tell us the current state of what has been done */
			Exit(X_FINOK);		/* do nothing else */

			/* NOT REACHED */

		case 'B':		/* block mode device */
			by_blocks_flag = TRUE;
			break;

		case 'E':		/* print estimate information, only */
			estimate_only_flag = TRUE;
			break;

		case 'N':		/* do not rewind tape when done */
			no_rewind_flag = TRUE;
			break;

		case 'S':		/* full tape size, blocks or feet */
			if(argc > 1) {
				argv++;
				argc--;
				full_size = atol(*argv);
			}
			break;

		case 'T':		/* tape number */
			if(argc > 1) {
				argv++;
				argc--;
				curr_tape_num = atol(*argv);
			}
			break;

		case 'f':			/* output file */
			if(argc > 1) {
				argv++;
				argc--;
				tape_file_name = *argv;
			}
			break;
	
		case 'd':			/* density, in bytes per inch */
			if (argc > 1) {
				argv++;
				argc--;
				tape_density = atoi(*argv);
			}
			break;
	
		case 's':			/* tape size, feet */
			if(argc > 1) {
				argv++;
				argc--;
				full_size = atol(*argv);
			}
			break;
	
		case 'b':			/* blocks per tape write */
			if(argc > 1) {
				argv++;
				argc--;
				blocks_per_write = atol(*argv);
			}
			break;
	
		case 'c':			/* Tape is cart. not 9-track */
			/* medium_flag = CARTRIDGE; */
			break;
/* ========================================================================= */
/*      Begin modifications for qar 17232  12/6/93                           */
/*         William_Soreth                                                    */
/* ========================================================================= */ 
		case 'L':
			if(argc > 1) {
                                argv++;
                                argc--;
                                tc_label = *argv;
				if(strlen(tc_label)>16) {
		(void)	msg(MSGSTR(LABLONG, "Label must not be more than 16 characters.\n"));
				abort_dump();
				}
				if(strlen(tc_label)==0){
				tc_label = NULL;
				}
			break;	
			}
/* ========================================================================= */ 
/*      End modifications for qar 17232  12/6/93                             */ 
/*         William_Soreth                                                    */ 
/* ========================================================================= */	
		case '0':			/* dump level */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			incr_num = arg[-1];
			break;
	
		case 'u':			/* update /etc/dumpdates */
			update_flag = TRUE;
			break;
	
		case 'n':			/* notify operators */
			notify_flag = TRUE;
			break;
	
		default:
			(void) msg(MSGSTR(BADKEY, "illegal option -- %c\n"), arg[-1]);
			(void) usage();

			/* NOT REACHED */
		}
		if(argc > 1) {
			argv++;
			argc--;
			disk_file_name = *argv;
		}
	
	} else {
	   while ((optchar = getopt(argc, argv, "0123456789WwBENS:T:b:cd:f:L:ns:u")) != EOF)
	   {
		switch (optchar)
		{
		case '-':
			break;

		case '0':		/* dump level */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			incr_num = optchar;
			break;

		case 'W':		/* tell us the current state of what
					 * has been done */
		case 'w':		/* tell us only what has to be done */
			lastdump(optchar);
			Exit(X_FINOK);

			/* NOT REACHED */

		case 'B':		/* block mode device */
			by_blocks_flag = TRUE;
			break;

		case 'E':		/* print estimate information, only */
			estimate_only_flag = TRUE;
			break;

		case 'N':		/* do not rewind tape when done */
			no_rewind_flag = TRUE;
			break;

		case 'S':		/* full tape size, blocks or feet */
			full_size = atol(optarg);
			break;

		case 'T':		/* tape number */
			curr_tape_num = atol(optarg);
			break;

		case 'b':		/* blocks per tape write */
			blocks_per_write = atol(optarg);
			break;

		case 'c':		/* tape is cart, not 9-track */
			/* medium_flag = CARTRIDGE; */
			break;

		case 'd':		/* density, in bytes per inch */
			tape_density = atoi(optarg);
			break;

		case 'f':		/* output file */
			tape_file_name = optarg;
			break;

/* ========================================================================= */ 
/*      Begin modifications for qar 17232  12/6/93                           */ 
/*         William_Soreth                                                    */ 
/* ========================================================================= */
		case 'L':
			tc_label = optarg;
			if(strlen(tc_label)>16) {
                (void)  msg(MSGSTR(LABLONG, "Label must not be more than 16 characters.\n"));
                                abort_dump();
                                }
                                if(strlen(tc_label)==0){
                                tc_label = NULL;
                                }
			break;
/* ========================================================================= */ 
/*      End modifications for qar 17232  12/6/93                             */ 
/*         William_Soreth                                                    */ 
/* ========================================================================= */

		case 'n':		/* notify operators */
			notify_flag = TRUE;
			break;

		case 's':		/* first tape size, blocks or feet */
			full_size = atol(optarg);
			break;

		case 'u':		/* update /etc/dumpdates */
			update_flag = TRUE;
			break;

		case '?':
		default:
			(void) usage();

			/* NOT REACHED */
		}
	   }

	/* remaining arg is name of disk file system file to dump */

	   if (argc > optind)
	   {
		disk_file_name = argv[optind];
	   }
	} /* end or argument parsing */

#ifdef REMOTE

	/* decode old tape_file_name in format host:tape into */
	/* remote_host_name and tape_file_name components */

	remote_host_name = tape_file_name;
	tape_file_name = (char *) index(remote_host_name, ':');
	if (tape_file_name == NULL || tape_file_name == remote_host_name)
	{
		msg(MSGSTR(NEEDKEY, "Need option 'f' followed by remote device \"host:tape\"\n"));
		Exit(X_FINBAD);

		/* NOT REACHED */
	}
	*tape_file_name = '\0';
	++tape_file_name;

	/* establish connection to remote host */

	rmthost(&remote_host_name);

	(void) setuid(getuid());	/* rmthost() is the only reason to be setuid */

#else /* REMOTE */

	/* test whether output is to stdout, otherwise get output device info */

	if (strcmp(tape_file_name, "-") == 0)
	{
		pipe_out_flag = TRUE;
		tape_file_name = MSGSTR(STDOUT, "standard output");
		medium_flag = REGULAR_FILE;
	}

#endif /* REMOTE */

	/* arrange to catch the following signals, provided they */
	/* were not previously ignored, in which case, continue */
	/* to ignore them */

	if (signal(SIGHUP, sighup) == SIG_IGN)
	{
		(void) signal(SIGHUP, SIG_IGN);
	}
	if (signal(SIGTRAP, sigtrap) == SIG_IGN)
	{
		(void) signal(SIGTRAP, SIG_IGN);
	}
	if (signal(SIGFPE, sigfpe) == SIG_IGN)
	{
		(void) signal(SIGFPE, SIG_IGN);
	}
	if (signal(SIGBUS, sigbus) == SIG_IGN)
	{
		(void) signal(SIGBUS, SIG_IGN);
	}
	if (signal(SIGALRM, sigalrm) == SIG_IGN)
	{
		(void) signal(SIGALRM, SIG_IGN);
	}
	if (signal(SIGSEGV, sigsegv) == SIG_IGN)
	{
		(void) signal(SIGSEGV, SIG_IGN);
	}
	if (signal(SIGTERM, sigterm) == SIG_IGN)
	{
		(void) signal(SIGTERM, SIG_IGN);
	}
	if (signal(SIGINT, sigintr) == SIG_IGN)
	{
		(void) signal(SIGINT, SIG_IGN);
	}

	/* get list of members of operator group from /etc/group */

	if (notify_flag == TRUE)
	{
		if (set_operators() < 0)
		{
			notify_flag = FALSE;
		}
	}

	/* 
	 * disk can be either the full special file name, the suffix of the
	 * special file name, the special name missing the leading '/', the
	 * file system name with or without the leading '/'.
	 */

	getfstab();

	disk_fstab_entry = fstabsearch(disk_file_name);
	/* Now test if tape name is that of an existing file system - 
	- could be disaterous! */ 
	new_disk_fstab_entry = fstabsearch(tape_file_name); 
	if (new_disk_fstab_entry &&  (estimate_only_flag == FALSE)) {
	    if (query(MSGSTR(DYRWTOFS, "Do you really wish to overwrite a file system ")) == NO)
	    {
		abort_dump();

		/* NOT REACHED */
	    }
        }

	if (disk_fstab_entry != NULL)
	{
		disk_file_name = rawname(disk_fstab_entry->fs_spec);
		strncpy(spcl.c_dev, disk_fstab_entry->fs_spec, NAMELEN);
		strncpy(spcl.c_filesys, disk_fstab_entry->fs_file, NAMELEN);
	}
	else
	{
		strncpy(spcl.c_dev, disk_file_name, NAMELEN);
		strncpy(spcl.c_filesys, MSGSTR(AULFS, "An unlisted file system"), NAMELEN);
	}
/* ========================================================================= */ 
/*      Begin modifications for qar 17232  12/6/93                           */ 
/*         William_Soreth                                                    */ 
/* ========================================================================= */
	if (tc_label == NULL){
	strcpy(spcl.c_label, MSGSTR(NOLBL, "No label"));
	}
	else{
	strcpy(spcl.c_label,tc_label);
	}
/* ========================================================================= */ 
/*      End modifications for qar 17232  12/6/93                             */ 
/*         William_Soreth                                                    */ 
/* ========================================================================= */
	strncpy(spcl.c_host, local_host_name, NAMELEN);
	spcl.c_level = incr_num - '0';
	spcl.c_type = TS_TAPE;

	/* get dates and times of previous dumps from dumpdates file */

	if ( (update_flag == TRUE) || ((update_flag == FALSE) && (incr_num != '0')) )
	{
		getitime();
	}

	if (estimate_only_flag == FALSE)
	{
		msg(MSGSTR(DUMPH, "Dumping from host %s\n"), local_host_name);
		msg(MSGSTR(DUMPD, "Date of this level %c dump: %s\n"), incr_num, prdate(spcl.c_date));
		msg(MSGSTR(DUMPDL, "Date of last level %c dump: %s\n"), last_incr_num, prdate(spcl.c_ddate));
		msg(MSGSTR(DUMPING, "Dumping %s "), disk_file_name);

		if (disk_fstab_entry != NULL)
		{
			msgtail("(%s) ", disk_fstab_entry->fs_file);
		}

#if	EDUMP

		msgtail(MSGSTR(TOEHST, "to %s on %s\n"), tape_file_name, remote_host_name);

#else	! EDUMP

#if	REMOTE

		msgtail(MSGSTR(TOHOST, "to %s on host %s\n"), tape_file_name, remote_host_name);

#else	! REMOTE

		msgtail(MSGSTR(TOLOCLA, "to %s\n"), tape_file_name);

#endif	! REMOTE

#endif	! EDUMP
	}
	
#if	AIX

	/* check to see whether disk file system is currently mounted, */
	/* and if so, issue warning message */
	
	if (disk_fstab_entry != NULL)
	{
		mount_device = disk_fstab_entry->fs_spec;
	}
	else
	{
		mount_device = disk_file_name;
	}
	
	num_mounted_fs = mntctl(MCTL_QUERY, sizeof(mount_info), mount_info);
	if (num_mounted_fs <= 0)
	{
		msg(MSGSTR(MNTCTLE, "Cannot get status of mounted file systems\n"));
		dump_perror("main(): mntctl()");
		abort_dump();
		
		/* NOT REACHED */
	}

	mount_tab_entry = (struct vmount *) mount_info;
	for (i = 0; i < num_mounted_fs; ++i)
	{
		if (strcmp(vmt2dataptr(mount_tab_entry, VMT_OBJECT), mount_device) == 0)
		{
			msg(MSGSTR(WARNING1, "File system still mounted -- inconsistent dump possible\n"));
			break;
		}
		mount_tab_entry = (struct vmount *) ((char *) mount_tab_entry + mount_tab_entry->vmt_length))
	}

#endif	AIX

	/* open raw device file of disk file system */

	in_disk_fd = open(disk_file_name, O_RDONLY);
	if (in_disk_fd < 0)
	{
		msg ("\n");
		msg(MSGSTR(CANTODISK, "Cannot open file-system file %s\n"), disk_file_name);
		msg (MSGSTR(BADFS1, "Bad file system specification or bad file system.  The raw device must\n"));
		msg (MSGSTR(BADFS2, "be entered when the file system's pathname has not been entered in the\n"));
		msg (MSGSTR(BADFS3, "fstab file.  A bad file system is reported when the the magic number\n"));
		msg (MSGSTR(BADFS4, "is not found in the super block.\n"));
		dump_perror("main(): open()");
		invalid_file_system=TRUE; /* Stop abort_dump from killing
                                         a process group that doesn't
                                         exist */

		abort_dump();

		/* NOT REACHED */
	}

	/* get super block of disk file system */

	sync();

#ifdef SBOFF

	dev_bsize = 1;
	bread(SBOFF, (char *) super_block, SBSIZE);

#else /* SBOFF */

	/* tahoe system is too new for us */

	dev_bsize = DEV_BSIZE;
	bread(SBLOCK, (char *) super_block, SBSIZE);

#endif /* SBOFF */

        if (super_block->fs_magic != FS_MAGIC)
	{
		msg ("\n");
		msg (MSGSTR(BADFS1, "Bad file system specification or bad file system.  The raw device must\n"));
		msg (MSGSTR(BADFS2, "be entered when the file system's pathname has not been entered in the\n"));
		msg (MSGSTR(BADFS3, "fstab file.  A bad file system is reported when the the magic number\n"));
		msg (MSGSTR(BADFS4, "is not found in the super block.\n"));
		invalid_file_system=TRUE; /* Stop abort_dump from killing
   					 a process group that doesn't
   					 exist */

		abort_dump();

		/* NOT REACHED */
	}

        dev_bsize = super_block->fs_fsize / fsbtodb(super_block, 1);

	if (TP_BSIZE % dev_bsize != 0)
	{
		msg(MSGSTR(TPBSIZE, "TP_BSIZE must be a multiple of DEV_BSIZE\n"));
		abort_dump();

		/* NOT REACHED */
	}

	if ((TP_BSIZE * TP_NINDIR) % super_block->fs_bsize != 0)
	{
		msg(MSGSTR(TPBSNIN, "TP_BSIZE * TP_NINDIR must be a multiple of the dumped file system's block size\n"));
		abort_dump();

		/* NOT REACHED */
	}

	/* alloc inode bit map arrays */
	/* these arrays must be able to accomodate one bit per inode in */
	/* file system */

	imap_size = roundup(howmany(super_block->fs_ipg * super_block->fs_ncg, NBBY), TP_BSIZE);

	not_clear_map = (char *) calloc((unsigned) imap_size, sizeof(char));
	directory_map = (char *) calloc((unsigned) imap_size, sizeof(char));
	to_dump_map = (char *) calloc((unsigned) imap_size, sizeof(char));

	/* start the real work!! */

	/* This first pass completes the inode maps of clear inodes */
	/* and directory inodes.  It also marks inodes changed since */
	/* the last dump (of this increment level) as to-be-dumped */
	/* in the to-be-dumped inode map.  In addition, if any direcories */
	/* are skipped, because they have not changed since the last */
	/* increment, this fact is noted. */

	if (estimate_only_flag == FALSE)
	{
		msg(MSGSTR(MAP1, "Mapping (Pass I) [regular files]\n"));
	}

	pass(mark, NULL);

	/* If any unchanged directories were skipped the in the first */
	/* pass, check up on them now, in the second pass. */

	if (estimate_only_flag == FALSE)
	{
		msg(MSGSTR(MAP2, "Mapping (Pass II) [directories]\n"));
	}

	if (dir_skipped_flag == TRUE)
	{
		dir_added_flag = FALSE;

		pass(add, directory_map);

		while (dir_added_flag == TRUE)
		{
			if (estimate_only_flag == FALSE)
			{
				msg(MSGSTR(MAP2C, "Mapping (Pass II) [directories] (continued)\n"));
			}

			dir_added_flag = FALSE;

			pass(add, directory_map);
		};
	}

	/* update the estimated total block count to account for the sizes */
	/* of the clear and to-dump bit maps */

	est_tot_blocks += bmapest(not_clear_map);
	est_tot_blocks += bmapest(to_dump_map);

        /* Call open_at_start() to begin output processing */

	open_at_start();

	/* only the child process ever makes it past open_at_start() to here */

	/* Estimate the number of tapes which will be required for */
	/* the entire dump. */

	if ((pipe_out_flag == TRUE) || (medium_flag == REGULAR_FILE))
	{
		d_est_tot_tapes = 0.0;
	}
	else
	{
		if (by_blocks_flag == TRUE)
		{
			d_est_tot_tapes = (double)est_tot_blocks / (double)full_size_blocks;
		}
		else
		{
			double		est_writes;

			est_writes = (double)est_tot_blocks / blocks_per_write;
			est_tot_inches = est_writes * inches_per_write;
			d_est_tot_tapes = est_tot_inches / full_size_inches;
		}
	}

	/* convert (double) estimate of number of tapes to an */
	/* (int), rounding up */

	est_tot_tapes = (int) (d_est_tot_tapes + 1);

	/* increase the blocks estimate to account for the to-dump map */
	/* on each tape after the first tape */


	for (i = 1; i < est_tot_tapes; ++i)
	{
		est_tot_blocks += bmapest(to_dump_map);

	}

	/* increase the total blocks estimate to account for headers */
	/* on each tape and some trailer blocks on the last tape */

	est_tot_blocks += est_tot_tapes + blocks_per_write;

        if (estimate_only_flag == FALSE)
        {
		msg(MSGSTR(EST1, "Estimate: %ld tape blocks on %3.2f volume(s)\n"), est_tot_blocks, d_est_tot_tapes);
	}
        else
        {
		fprintf (stderr, MSGSTR(EST2,"%ld blocks, %3.2f volumes\n"), est_tot_blocks, d_est_tot_tapes);
		abort_dump();

		/* NOT REACHED */
	}

	/* put the map of (not) clear inodes on the first tape now */

	bitmap(not_clear_map, TS_CLRI);

	/* now write out all the direcories */

	msg(MSGSTR(MAP3, "Dumping (Pass III) [directories]\n"));

	pass(dirdump, directory_map);

	/* follow the directories with the regular files */

	msg(MSGSTR(MAP4, "Dumping (Pass IV) [regular files]\n"));

	pass(dump, to_dump_map);

	job_trailer();

	msg(MSGSTR(DUMPBLKS, "Actual: %ld tape blocks on %d volume(s)\n"), spcl.c_tapea, curr_volume_num);

	if (pipe_out_flag == FALSE && medium_flag != REGULAR_FILE)
	{
		if (by_blocks_flag == TRUE)
		{
			smsg(MSGSTR(BLKREM, "Blocks remaining on volume: %d"), full_size_blocks - blocks_written);
		}
		else
		{
			smsg(MSGSTR(FTREM, "Feet remaining on tape: %d"), (int) (((full_size_inches - inches_written) / 12) + 0.5));
		}
		smsg(MSGSTR(VOLUSD, "Volumes used: %d"), spcl.c_volume);
	}

	/* close up output tape processing */

	close_at_end();

	/* update the dump dates file */

	if (update_flag == TRUE)
	{
		putitime();
	}

	/* get end time */

	(void) time(&end_time);

	msg(MSGSTR(DONE, "Dump completed at %s\n"), prdate(end_time));

	if (notify_flag == TRUE)
	{
		broadcast(MSGSTR(DONE2, "DUMP IS DONE!\7\7\n"));
	}

#if	EDUMP

	/* say the dump is done before exiting */

	smsg(MSGSTR(DMPOK, "Dump completed ok"));

#endif	EDUMP

	/* good bye */

	Exit(X_FINOK);

	/* NOT REACHED */
}

/* These routines are where the program goes when any of the */
/* corresponding signals (which are not being ignored) are */
/* received */

static void
sighup()
{
	msg(MSGSTR(TRYREW, "SIGHUP received -- Try rewriting\n"));
	handle_signal();
}

static void
sigtrap()
{
	msg(MSGSTR(TRYREW1, "SIGTRAP received -- Try rewriting\n"));
	handle_signal();
}

static void
sigfpe()
{
	msg(MSGSTR(TRYREW2, "SIGFPE received -- Try rewriting\n"));
	handle_signal();
}

static void
sigbus()
{
	msg(MSGSTR(TRYREW3, "SIGBUS received -- Try rewriting\n"));
	handle_signal();
}

static void
sigalrm()
{
	msg(MSGSTR(TRYREW4, "SIGALRM received -- Try rewriting\n"));
	handle_signal();
}

static void
sigsegv()
{
	msg(MSGSTR(ABORTING, "SIGSEGV received -- ABORTING!\n"));
	abort();
}

static void
sigterm()
{
	msg(MSGSTR(TRYREW5, "SIGTERM received -- Try rewriting\n"));
	handle_signal();
}

static void
sigintr()
{
	void sigintr();

	msg(MSGSTR(INTR, "Interrupt received.\n"));

#if	EDUMP

	abort_dump();

	/* NOT REACHED */

#else	! EDUMP

	if (query(MSGSTR(ABORTDUM, "Do you want to abort dump")) == YES)
	{
		abort_dump();

		/* NOT REACHED */
	}
	(void) signal(SIGINT, sigintr);

#endif	! EDUMP

}

static void
handle_signal()
{
	if (medium_flag == DISKETTE || medium_flag == TAPE)
	{
		msg(MSGSTR(REWRITE6, "Unexpected signal -- attempting to rewrite from last checkpoint\n"));
		
		/* call rewrite_tape to close up tape and kill any child */
		/* processes */
		
		rewrite_tape();
		
		/* NOTREACHED */
	}
	else
	{
		msg(MSGSTR(UNKSIG, "Unexpected signal -- cannot recover\n"));
		abort_dump();
		
		/* NOT REACHED */
	}
}

/* 	rawname takes a non-raw device name and returns its	*/
/* 	corresponding raw device name by placing an 'r' in the	*/
/* 	front of its basename					*/
/* 	e.g.:  rawname("/dev/disk") == "/dev/rdisk"		*/

char	       *
rawname(cp)
	char	       *cp;
{
	static char	rawbuf[32];
	char	       *lastslash;
	
	if ((lastslash = (char *) rindex(cp, '/')) == NULL)
		if ((lastslash = (char *) rindex(cp, '#')) != NULL)
			{
			strncpy(rawbuf, cp, sizeof(rawbuf));
			return(rawbuf);
			}
		else
			return(NULL);
	*lastslash = '\0';
	(void) sprintf(rawbuf, "%s/r%s", cp, lastslash + 1);
	*lastslash = '/';
	return(rawbuf);
}


/* Bmapest() returns the number of tape blocks which will be needed to */
/* dump the specified bit map, plus one for the TS_CLRI/TS_BITS record */

static int
bmapest(map_to_size)
	register char	       *map_to_size;
{
	register int		i;

	/* starting with the last byte in the bit map array, look for */
	/* the last non-zero byte */

	for (i = imap_size - 1; i >= 0; --i)
	{
		if (map_to_size[i] != (char) 0)
		{
			break;
		}
	}

	/* if the bit map was entirely zeroes, return zero */

	if (i < 0)
	{
		return(0);
	}

	return(1 + howmany((i + 1) * sizeof(map_to_size[0]), TP_BSIZE));
}


static int
bsd_style(argn, argp)
int argn;
char *argp[];
{
	int i;

	for( i = 1; i < argn; i++ ) {
		/* look for -option and avoid "-" as default output file */
		if ( argp[i][0] == '-' && argp[i][1] != '\0' )
			return(0);
	}
	return(1);
}

static void
usage()
{

#ifndef REMOTE

        msg(MSGSTR(USAGE0, "Usage:\n%s%s%s"),
            MSGSTR(USAGE1, "\tdump [-#nuwENW] [-f dumpfile] [-L dump_label] [-b #] [filesystem]\n"),
            MSGSTR(USAGE2, "\tdump [-#cnuwENW] [-f dumpfile] [-b #] [-d #] [-s #] [-T #] [filesystem]\n"),
            MSGSTR(USAGE3, "\tdump [-#cnuwBENW] [-f dumpfile] [-b #] [-S #] [-T #] [filesystem]\n"));

#else /* REMOTE */

        msg(MSGSTR(USAGE10, "Usage:\n%s%s%s\n%s"),
            MSGSTR(USAGE11, "\trdump [-#nuwENW] -f dumpfile [-L dump_label] [-b #] [filesystem]\n"),
            MSGSTR(USAGE12, "\trdump [-#cnuwENW] -f dumpfile [-b #] [-d #] [-s #] [-T #] [filesystem]\n"),
            MSGSTR(USAGE13, "\trdump [-#cnuwBENW] -f dumpfile [-b #] [-S #] [-T #] [filesystem]\n"),
            MSGSTR(USAGE14, "\twhere the dumpfile is specified as machine\:device.\n"));

#endif /* REMOTE */

	Exit(X_FINBAD);
}
