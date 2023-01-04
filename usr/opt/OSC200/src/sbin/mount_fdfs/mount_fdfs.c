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
#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
/*
 * main() of mount.c calls mountfs() to the actual mount.  If the
 * mount type passed in is not recognized or not specifically
 * mentioned in the switch() list, it falls through to the default:
 * action.  Here, a new command line is formed to call an independent
 * mount command.  The first argument is formed from mntname, set
 * when  getmnttype() was called to find if the name was one of the
 * standard types. If there were any flags specified the next two
 * arguments are "-F" and a decimal representation of the flags,
 * respectively.  The executable name is formed from  _PATH_EXECDIR,
 * as specified in pathnames.h ("/sbin") and mntname ("fdfs") to form
 * "/sbin/mount_fdfs".
 */

long	options;
char	*directory;

int
main(int argc, char **argv, char **envp)
{
	char	ch;
	int	error;
	extern char	*optarg;
	extern int	optind;

	error = 0;

	if (strcmp(argv[0], "fdfs") != 0) {
		fprintf(stderr, "(mount_fdfs) file system fdfs expected\n");
		exit (1);
	}

	while ((ch = getopt(argc, argv, "F:")) != EOF) {
		switch ((char)ch) {
		case 'F':
			 {
				char	*remaining;

				options = strtol(optarg, &remaining, 10);
				break;
			}
		default:
			 {
				fprintf(stderr,
				    "(mount_fdfs) unknown flag %c\n",
				    (char)ch);
				exit (1);
			}
		}
	}

	/*
   * The next argument should be the device specification.
   * This MUST be '/dev/fd'.
   */
	if ((optind >= argc) || 
	    (strcmp(argv[optind++], "/dev/fd") != 0)) {
		fprintf(stderr,
		    "(mount_fdfs) special device /dev/fd expected\n");
		exit (1);
	}

	/*
   * The next argument should be then directory to be covered.
   */
	if (optind >= argc) {
		fprintf(stderr,
		    "(mount_fdfs) file name to cover expected\n");
		exit (1);
	}
	directory = argv[optind++];

	while ((optind < argc) && 
	    (argv[optind] == (char *)0)) {
		optind++;
	}

	/*
   * There should be no additional parameters.
   */
	if (optind < argc) {
		fprintf(stderr,
		    "(mount_fdfs) unknown parameter: %s\n",
		    argv[optind]);
		exit (1);
	}

	if (mount(MOUNT_FDFS,
	    directory,
	    (int) options,
	    (void *)0)
	     == -1) {
		fprintf(stderr, "fdfs on %s: ", directory);
		switch (errno) {
		case EMFILE:
			 {
				fprintf(stderr, "Mount table full\n");
				break;
			}

		case EINVAL:
			 {
				fprintf(stderr,
				    "(mount_fdfs) invalid parameter\n");
				break;
			}

		case EOPNOTSUPP:
			 {

				fprintf(stderr,
				    "Operation not supported\n");
				break;
			}

		default:
			 {
				perror((char *)NULL);
				break;
			}
		}

		return(1);
	} else
	 {
		return (0);
	}
}
