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
 * Copyright (c) 1988-1991 SecureWare, Inc.  All rights reserved.
 *
 *	@(#)lsacl.c	1.5 14:39:33 10/2/91 SecureWare
 *
 * This is unpublished proprietary source code of SecureWare, Inc. and
 * should be treated as confidential.
 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "lsacl_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LSACL,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_ACL_POSIX

#include <stdio.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <acl.h>

static int printacl ();
static int get_name ();
static int get_perm ();
extern void pacl_printbuf();

char	*acl_er;

/*
 *
 * lsacl command, prints ACLs of files.
 *
 *	Usage:  lsacl [ -d ] file . . .
 *		-d print default acl of	the directories listed.
 *
 */

main(argc,argv)
int	argc;
char	*argv[];
{
	int		c;
	extern	int 	optind, opterr;
	extern	char 	*optarg;
	acl_t		acl;
	acl_type_t	type = ACL_TYPE_ACCESS;
	int		errflag = 0;
	int		nument;


#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_LSACL,NL_CAT_LOCALE);
#endif

	opterr = 0;
	while ((c = getopt (argc, argv, "d")) != EOF)  
		switch (c) {

		case 'd':
			type = ACL_TYPE_DEFAULT;
			break;

		case '?':
			errflag++ ;
			break;

		}

	if (errflag || optind >= argc) {
		(void) fprintf (stderr, MSGSTR(LSUSAGE,
			"Usage: lsacl [ -d ] filenames.\n"));
		exit (1);
	}

	/*
	 * Allocate initial space for ACL and internal rep.
	 */
	if (acl_alloc (&acl)) {
		(void) fprintf (stderr, MSGSTR(MALLOCERR,
			"%s: Memory allocation error.\n"), argv[0]);
		exit (1);
	}

	acl_er = (char *)malloc(8192);
	if (!acl_er) {
		(void) fprintf (stderr, MSGSTR(MALLOCERR,
			"%s: Memory allocation error.\n"), argv[0]);
		exit (1);
	}

	/*
	 * For each argument in argument list,
	 * read the acl of the file and
	 * print it.
	 */
	for ( ; optind < argc; optind++) {

		nument = acl_read (argv[optind], type, acl);
		if (nument == 0) {
			/* Zero returned only on default type */
			(void) fprintf (stderr, MSGSTR(DEFREADERR,
				"%s: \"%s\" does not have a default acl.\n")
				,argv[0],argv[optind]);
			continue;
		}
				
		else if (nument == -1) {
			errflag++;
			(void) fprintf (stderr, MSGSTR(READERR,
				"%s: Error reading acl of \"%s\""),
					argv[0],argv[optind]);
			perror (" ");
			continue;
		}

		if (printacl (argv[0],argv[optind],acl))
			errflag++;
	}

	exit (errflag);
}


/*
 * FUNCTION:
 *	printacl()
 *
 * ARGUMENTS:
 *	char	*prog;		- the name of this process.
 *	char	*file;		- the file name.
 *	acl_t	acl;		- the file's acl entries.
 *
 * DESCRIPTION:
 *	The printacl() function prints a file's ACL, a directory's ACL,
 * 	or the default ACL associated with a directory.
 *	The output will be suitable for input to the chacl command
 *	and will have the following format:
 *
 *		# file: /tcb/files/test
 *		# owner: root
 *		# group: sware
 *		user::rwx
 *		mask::r-x
 *		user:adm:rwx	#effective:r-x
 *		user:tomg:rwx	#effective:r-x
 *		
 *		group::r--
 *		group:daemon:r-x
 *		
 *		other::r--
 *
 *	To indicate that an ACL is restricted by
 * 		the mask entry, it shall be displayed as:
 *
 *		user:tomg:rw-	#effective:r-x
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR - (-1).
 */

static int 
printacl (prog,file,acl)
char	*prog;	/* name of program, for messages */
char	*file;
acl_t	acl;
{
	struct  stat    st;

	if (acl->acl_num * ACL_DATA_SIZE > 8192) {
		(void) fprintf (stderr, MSGSTR(MALLOCERR,
			"%s: Memory allocation error.\n"), prog);
		exit (1);
	}

	if (acl_pack (acl, acl_er, 8192, ACL_TEXT_PACKAGE) < 0) {

		(void) fprintf (stderr, MSGSTR(READERR,
			"%s: Error reading acl of \"%s\""), prog, file);
		perror (" ");
		return -1;
	}

        /* get owner and group */

        if (stat (file,&st) < 0) {
		(void) fprintf (stderr, MSGSTR(STATERR,
			"%s: Failed stat of \"%s\".\n"), prog,file);
                return -1;
	}

	pacl_printbuf (file, acl_er, st.st_uid, st.st_gid);

	return 0;
}

#endif
