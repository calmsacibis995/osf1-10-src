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
 * COMPONENT_NAME: CMDUUCP uupoll.c
 * 
 * FUNCTIONS: Muupoll, cleanup 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef BSD4_2
#undef BSD4_2
#endif

#include	"uucp.h"

nl_catd catd;
#define	UUSCHED		"/usr/lbin/uucp/uusched"

main(argc,argv)
register int	argc;
register char	**argv;
{
	int	nulljob = 0, mode, i;
	char	grade, lockfile[256], file[256];
	FILE	*fd;

	if (argc < 2) {
		fprintf(stderr, MSGSTR(MSG_UUPOLL_1, 
			"Usage: uupoll [-ggrade] [-n] system\n"));
		exit(1);
	}
	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);

	myName(Myname);
	chdir(SPOOL);

	for (--argc, ++argv; argc > 0; --argc, ++argv) {
		if (strcmp(argv[0], Myname) == SAME) {
			fprintf(stderr, MSGSTR(MSG_UUPOLL_2,
				"This *is* %s!\n"), Myname);
			continue;
		}
		if (strncmp(argv[0],"-g",2) == SAME) {
			grade = argv[0][2];	/* just one byte */
			continue;
		}
		if (strncmp(argv[0],"-n") == SAME) {
			nulljob++;
			continue;
		}
	
		if (versys(argv[0])) {
			fprintf(stderr, MSGSTR(MSG_UUPOLL_3,
				"%s: unknown system\n"), argv[0]);
			continue;
		}

			/*
			** In order to convince UUCICO to ignore any
			** status (/usr/spool/uucp/.Status/<sys_name>
			** files that might prevent it from calling,
			** we'll just take the bull by the horns and
			** blow blow away the status file.  If the
			** sysadmin is really needing that info, he
			** can use "uulog"
			*/

		sprintf(lockfile, "%s/%s", STATDIR, argv[0]);
		unlink(lockfile);

			/*
			** Now we have to trick UUCICO into thinking there
			** is some work to do.  So we generate a phony job
			** (after we check to ensure we're not going to
			** accidently erase a valid one!).  We need to create
			** a system_name directory under /usr/spool/uucp.
			** If it already exists, no problem . . . just go
			** ahead and continue.  If it isn't created for
			** some reason, report the problem and close up 
			** shop.
			*/

			sprintf(file, "%s/%s", SPOOL, argv[0]);
			mode =  S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
			 	S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH |
			 	S_IXOTH;
			if (mkdir(file, mode)) {
				if (errno != EEXIST) {
					fprintf(stderr, MSGSTR(MSG_UUPOLL_4,
					  "could not create directory '%s'\n"),
						file);
					cleanup(1);
				}
			}
					/*
					** we have to find an unused filename
					** inside of /usr/spool/uucp/argv[0]
					*/

			for (i = 0; i < 9999; i++) {
				sprintf(file, "%s/%s/C.%4sa%.4d", SPOOL,
					argv[0], argv[0], i);

				if (( fd = fopen(file, "r")) != NULL) {
					fclose(fd);
					continue;
				}
				break;
			}

					/*
					** create the phony job
					*/

			if ((fd = fopen(file, "w")) == NULL) {
			fprintf(stderr, MSGSTR(MSG_UUPOLL_5,
				"Can't open '%s' for writing!\n"), file);
				cleanup(1);
			}
			fclose(fd);		/*
						** file now exists, length = 0
						*/

			if (nulljob)
				exit(0);

			if (execl(UUSCHED, "uusched", 0)) {
					/*
					** what are we still doing here?
					** the execl() failed.
					*/
			  fprintf(stderr, MSGSTR(MSG_UUPOLL_6,
				"could not exec uusched!\n"));
				cleanup(1);
			}
	} 				/* end of for() loop */
}


/*
**	cleanup(value)
**	int	value;
**
**	Exit the program with the error level passed in "value".
**
**	Return code:  We don't return.
*/

cleanup(value)
int	value;
{

	catclose(catd);

	exit(value);
}
