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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getconf.c,v $ $Revision: 1.1.3.4 $ (OSF) $Date: 1993/10/11 20:04:42 $";
#endif
/*
 * COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
 *
 * FUNCTIONS: getconf
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.3  com/cmd/posix/getconf.c, cmdposix, bos320, 9125320 6/5/91 09:16:59
 */

#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/limits.h>

#include "getconf_msg.h"

nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_GETCONF,num,str) 

/*
 *
 *  NOTE:  This table must remain sorted 
 *
 */
struct values{
	char *name;
	int kind;
#define CONSTANT	1
#define SYSCONF		2
#define PATHCONF	3
#define CONFSTR		4
#define UNDEFINE	5
	long numb;
} value[] = 
	{
	"ARG_MAX",		SYSCONF,	_SC_ARG_MAX,
	"ATEXIT_MAX",		SYSCONF,	_SC_ATEXIT_MAX,
	"BC_BASE_MAX",		SYSCONF,	_SC_BC_BASE_MAX,
	"BC_DIM_MAX",		SYSCONF,	_SC_BC_DIM_MAX,
	"BC_SCALE_MAX",		SYSCONF,	_SC_BC_SCALE_MAX,
	"BC_STRING_MAX",	SYSCONF,	_SC_BC_STRING_MAX,
	"CHARCLASS_NAME_MAX",	CONSTANT,	CHARCLASS_NAME_MAX,
	"CHAR_BIT",		CONSTANT,	CHAR_BIT,
	"CHAR_MAX",		CONSTANT,	CHAR_MAX,
	"CHAR_MIN",		CONSTANT,	CHAR_MIN,
	"CHILD_MAX",		SYSCONF,	_SC_CHILD_MAX,
	"CLK_TCK",		SYSCONF,	_SC_CLK_TCK,
	"COLL_WEIGHTS_MAX",	SYSCONF,	_SC_COLL_WEIGHTS_MAX,
	"CS_PATH",		CONFSTR,	_CS_PATH,
#ifdef	_SC_DATAKEYS_MAX
	"DATAKEYS_MAX",		SYSCONF,	_SC_DATAKEYS_MAX,
#else
	"DATAKEYS_MAX",		UNDEFINE,	0,
#endif	/* _SC_DATAKEYS_MAX */
	"EXPR_NEST_MAX",	SYSCONF,	_SC_EXPR_NEST_MAX,
	"INT_MAX",		CONSTANT,	INT_MAX,
	"INT_MIN",		CONSTANT,	INT_MIN,
	"LINE_MAX",		SYSCONF,	_SC_LINE_MAX,
	"LINK_MAX",		PATHCONF,	_PC_LINK_MAX,
	"LONG_BIT",		CONSTANT,	LONG_BIT,
	"LONG_MAX",		CONSTANT,	LONG_MAX,
	"LONG_MIN",		CONSTANT,	LONG_MIN,
	"MAX_CANON",		PATHCONF,	_PC_MAX_CANON,
	"MAX_INPUT",		PATHCONF,	_PC_MAX_INPUT,
	"MB_LEN_MAX",		CONSTANT,	MB_LEN_MAX,
	"NAME_MAX",		PATHCONF,	_PC_NAME_MAX,
	"NGROUPS_MAX",		SYSCONF,	_SC_NGROUPS_MAX,
	"NL_ARGMAX",		CONSTANT,	NL_ARGMAX,
	"NL_LANGMAX",		CONSTANT,	NL_LANGMAX,
	"NL_MSGMAX",		CONSTANT,	NL_MSGMAX,
	"NL_NMAX",		CONSTANT,	NL_NMAX,
	"NL_SETMAX",		CONSTANT,	NL_SETMAX,
	"NL_TEXTMAX",		CONSTANT,	NL_TEXTMAX,
	"NZERO",		CONSTANT,	NZERO,
	"OPEN_MAX",		SYSCONF,	_SC_OPEN_MAX,
	"PAGE_SIZE",		SYSCONF,	_SC_PAGE_SIZE,
	"PASS_MAX",		SYSCONF,	_SC_PASS_MAX,
	"PATH",			CONFSTR,	_CS_PATH,
	"PATH_MAX",		PATHCONF,	_PC_PATH_MAX,
	"PIPE_BUF",		PATHCONF,	_PC_PIPE_BUF,
	"POSIX2_BC_BASE_MAX",	CONSTANT,	_POSIX2_BC_BASE_MAX,
	"POSIX2_BC_DIM_MAX",	CONSTANT,	_POSIX2_BC_DIM_MAX,
	"POSIX2_BC_SCALE_MAX",	CONSTANT,	_POSIX2_BC_SCALE_MAX,
	"POSIX2_BC_STRING_MAX",	CONSTANT,	_POSIX2_BC_STRING_MAX,
	"POSIX2_CHAR_TERM",	SYSCONF,	_SC_2_CHAR_TERM,
	"POSIX2_COLL_WEIGHTS_MAX",CONSTANT,	_POSIX2_COLL_WEIGHTS_MAX,
	"POSIX2_C_BIND",	SYSCONF,	_SC_2_C_BIND,
	"POSIX2_C_DEV",		SYSCONF,	_SC_2_C_DEV,
	"POSIX2_C_VERSION",	CONSTANT,	_SC_2_C_VERSION,
	"POSIX2_EXPR_NEST_MAX",	CONSTANT,	_POSIX2_EXPR_NEST_MAX,
	"POSIX2_FORT_DEV",	SYSCONF,	_SC_2_FORT_DEV,
	"POSIX2_FORT_RUN",	SYSCONF,	_SC_2_FORT_RUN,
	"POSIX2_LINE_MAX",	CONSTANT,	_POSIX2_LINE_MAX,
	"POSIX2_LOCALEDEF",	SYSCONF,	_SC_2_LOCALEDEF,
	"POSIX2_RE_DUP_MAX",	CONSTANT,	_POSIX2_RE_DUP_MAX,
	"POSIX2_SW_DEV",	SYSCONF,	_SC_2_SW_DEV,
	"POSIX2_UPE",		SYSCONF,	_SC_2_UPE,
	"POSIX2_VERSION",	SYSCONF,	_SC_2_VERSION,
	"RE_DUP_MAX",		SYSCONF,	_SC_RE_DUP_MAX,
	"SCHAR_MAX",		CONSTANT,	SCHAR_MAX,
	"SCHAR_MIN",		CONSTANT,	SCHAR_MIN,
	"SHRT_MAX",		CONSTANT,	SHRT_MAX,
	"SHRT_MIN",		CONSTANT,	SHRT_MIN,
	"SSIZE_MAX",		CONSTANT,	_POSIX_SSIZE_MAX,
	"STREAM_MAX",		SYSCONF,	_SC_STREAM_MAX,
	"TMP_MAX",		CONSTANT,	TMP_MAX,
	"TZNAME_MAX",		SYSCONF,	_SC_TZNAME_MAX,
	"UCHAR_MAX",		CONSTANT,	UCHAR_MAX,
	"UINT_MAX",		CONSTANT,	UINT_MAX,
	"ULONG_MAX",		CONSTANT,	ULONG_MAX,
	"USHRT_MAX",		CONSTANT,	USHRT_MAX,
	"WORD_BIT",		CONSTANT,	WORD_BIT,
	"_AES_OS_VERSION",	SYSCONF,	_SC_AES_OS_VERSION,
	"_POSIX_ARG_MAX",	CONSTANT,	_POSIX_ARG_MAX,
	"_POSIX_CHILD_MAX",	CONSTANT,	_POSIX_CHILD_MAX,
	"_POSIX_CHOWN_RESTRICTED",PATHCONF,	_PC_CHOWN_RESTRICTED,
	"_POSIX_JOB_CONTROL",	SYSCONF,	_SC_JOB_CONTROL,
	"_POSIX_LINK_MAX",	CONSTANT,	_POSIX_LINK_MAX,
#ifdef _POSIX_LOCALEDEF
	"_POSIX_LOCALEDEF",	CONSTANT,	_POSIX_LOCALEDEF,
#else
	"_POSIX_LOCALEDEF",	UNDEFINE,	0,
#endif /* _POSIX_LOCALEDEF */
	"_POSIX_MAX_CANON",	CONSTANT,	_POSIX_MAX_CANON,
	"_POSIX_MAX_INPUT",	CONSTANT,	_POSIX_MAX_INPUT,
	"_POSIX_NAME_MAX",	CONSTANT,	_POSIX_NAME_MAX,
	"_POSIX_NGROUPS_MAX",	CONSTANT,	_POSIX_NGROUPS_MAX,
	"_POSIX_NO_TRUNC",	PATHCONF,	_PC_NO_TRUNC,
	"_POSIX_OPEN_MAX",	CONSTANT,	_POSIX_OPEN_MAX,
	"_POSIX_PATH_MAX",	CONSTANT,	_POSIX_PATH_MAX,
	"_POSIX_PIPE_BUF",	CONSTANT,	_POSIX_PIPE_BUF,
#ifdef	_SC_REENTRANT_FUNCTIONS
	"_POSIX_REENTRANT_FUNCTIONS",
				SYSCONF,	_SC_REENTRANT_FUNCTIONS,
#else
	"_POSIX_REENTRANT_FUNCTIONS",
				UNDEFINE,	0,
#endif	/* _SC_REENTRANT_FUNCTIONS */
	"_POSIX_SAVED_IDS",	SYSCONF,	_SC_SAVED_IDS,
	"_POSIX_SSIZE_MAX",	CONSTANT,	_POSIX_SSIZE_MAX,
	"_POSIX_STREAM_MAX",	CONSTANT,	_POSIX_STREAM_MAX,
#ifdef	_SC_THREADS
	"_POSIX_THREADS",	SYSCONF,	_SC_THREADS,
#else
	"_POSIX_THREADS",	UNDEFINE,	0,
#endif	/* _SC_THREADS */
#ifdef	_SC_THREADS_PRIO_CEILING
	"_POSIX_THREADS_PRIO_CEILING",
				SYSCONF,	_SC_THREADS_PRIO_CEILING,
#else
	"_POSIX_THREADS_PRIO_CEILING",
				UNDEFINE,	0,
#endif	/* _SC_THREADS_PRIO_CEILING */
#ifdef	_SC_THREADS_PRIO_INHERIT
	"_POSIX_THREADS_PRIO_INHERIT",
				SYSCONF,	_SC_THREADS_PRIO_INHERIT,
#else
	"_POSIX_THREADS_PRIO_INHERIT",
				UNDEFINE,	0,
#endif	/* _SC_THREADS_PRIO_INHERIT */
#ifdef	_SC_THREAD_ATTR_STACKSIZE
	"_POSIX_THREAD_ATTR_STACKSIZE",
				SYSCONF,	_SC_THREAD_ATTR_STACKSIZE,
#else
	"_POSIX_THREAD_ATTR_STACKSIZE",
				UNDEFINE,	0,
#endif	/* _SC_THREAD_ATTR_STACKSIZE */
#ifdef	_SC_THREAD_PRIORITY_SCHEDULING
	"_POSIX_THREAD_PRIORITY_SCHEDULING",
				SYSCONF,	_SC_THREAD_PRIORITY_SCHEDULING,
#else
	"_POSIX_THREAD_PRIORITY_SCHEDULING",
				UNDEFINE,	0,
#endif	/* _SC_THREAD_PRIORITY_SCHEDULING */
	"_POSIX_TZNAME_MAX",	CONSTANT,	_POSIX_TZNAME_MAX,
	"_POSIX_VDISABLE",	PATHCONF,	_PC_VDISABLE,
	"_POSIX_VERSION",	SYSCONF,	_SC_VERSION,
	"_XOPEN_CRYPT",		SYSCONF,	_SC_XOPEN_CRYPT,
	"_XOPEN_ENH_I18N",	SYSCONF,	_SC_XOPEN_ENH_I18N,
	"_XOPEN_SHM",		SYSCONF,	_SC_XOPEN_SHM,
	"_XOPEN_VERSION",	SYSCONF,	_SC_XOPEN_VERSION,
	"_XOPEN_XCU_VERSION",	SYSCONF,	_SC_XOPEN_XCU_VERSION,
#ifdef _XOPEN_XPG2
	"_XOPEN_XPG2",		CONSTANT,	_XOPEN_XPG2,
#else
	"_XOPEN_XPG2",		UNDEFINE,	0,
#endif /* _XOPEN_XPG2 */

/* With _XOPEN_XPG4 defined, the test suite vsx4 runs successfully in 
   XPG3 and XPG4 mode, thus _XOPEN_XPG4 implies XPG3 and XPG4 branding 
   support.
*/
#ifdef _XOPEN_XPG3
	"_XOPEN_XPG3",		CONSTANT,	_XOPEN_XPG3,
#else
#ifdef _XOPEN_XPG4
	"_XOPEN_XPG3",		CONSTANT,	3,
#else
	"_XOPEN_XPG3",		UNDEFINE,	0,
#endif /* _XOPEN_XPG4 */
#endif /* _XOPEN_XPG3 */

	"_XOPEN_XPG4",		CONSTANT,	_XOPEN_XPG4,
	(char *)0,		  0,
	};

main(int argc, char **argv)
{
	int i;		/* index */
	size_t size;    /* size of buffer necessary for confstr */
	char *buf;      /* buffer necessary for confstr */
	int ret;	/* return value */

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_GETCONF, NL_CAT_LOCALE);

	errno = 0;

       /*  
	*  Getconf only expects the following two formats:
	*
	*      getconf system_var
	*      getconf path_var pathname
	*
	*  Give usage and exit if not correct format.
	*/

	if ((argc < 2) || (argc > 3))
		usage();


	while((i = getopt(argc, argv, "")) != EOF)
		switch(i) {

		case '?':
			usage();
			break;
		}

       /*
	* The first argument is either the system or path variable
	* to be queried.  Convert to the correct index via the table.
	*/

    	for (i = 0; i < sizeof(value)/sizeof(value[0]) - 1; i++) {
		ret = strcmp(argv[1], value[i].name);

	       /*
		*  If match is found, break the loop and call correct
		*  interpreting routine.
		*/
		if (ret == 0)
			break;

	       /*
		*  If a string bigger than the argument is found in the
		*  table (which is sorted), then we know that it is an
		*  invalid argument.  Exit with error status
		*/
		if (ret < 0)   {
			fprintf(stderr, MSGSTR(NOTVAL, "getconf: specified variable is not valid on this system\n"));
			exit(2);
		}
	}

	/* If did not find argument in table, exit with error status */
	if (value[i].name == (char *)0) {
		fprintf(stderr, MSGSTR(NOTVAL, "getconf: specified variable is not valid on this system\n"));
		exit(2);
	}

       /*
	*  When there is only one argument to getconf, it is assumed
	*  to be a system variable.  If the variable is valid and is
	*  defined on the system, print the associated value.  If it is 
	*  valid, but not defined, print 'undefined'. Otherwise exit
	*  with >0.
	*/

	if (argc == 2) { 
		switch (value[i].kind) {
			case CONSTANT:
				/* use different format code for ULONG_MAX */
				if (strcmp(value[i].name,"ULONG_MAX") == 0)
					printf("%lu\n", ULONG_MAX);
				else
					printf("%ld\n", value[i].numb);
				exit(0);
				break;

			case SYSCONF:
				ret = sysconf(value[i].numb);
				if (ret != -1) {
					printf("%ld\n", ret);
					exit(0);
				}
				else if (errno == 0) {
					printf(MSGSTR(NOTDEF, "undefined\n"));
					exit(0);
				}
				else 
					exit(1);
				break;

			case CONFSTR:
				size = confstr(value[i].numb, (char *)0, 0);
				if (size == 0) {  /* Argument is not defined */
					if (errno == 0) {
						printf(MSGSTR(NOTDEF, 
								"undefined\n"));
						exit(0);
					}
					else
						exit(1);
				}
				if ((buf = malloc(size)) == NULL) 
					perror("getconf");
				confstr(value[i].numb, buf, size);
				printf("%s\n", buf);
				exit(0);
				break;

			case UNDEFINE:
				printf(MSGSTR(NOTDEF, "undefined\n"));
				exit(0);
				break;

			default: 
				fprintf(stderr, MSGSTR(NOPATH, "getconf: must specify pathname with path_var\n"));
			exit(2);
		} /* switch */
	}

       /*
	*  When there are two argument to getconf, it is assumed
	*  to be a path variable.  If the variable is valid and is
	*  defined on the system, print the associated value.  If it is 
	*  valid, but not defined, print 'undefined', else exit w/ value 1
	*/

	else if (value[i].kind == PATHCONF) {
		ret = pathconf(argv[2], value[i].numb);
		if (ret != -1) {
			printf("%d\n", ret);
			exit(0);
		}
		else if (errno == 0) {
			printf(MSGSTR(NOTDEF, "undefined\n"));
			exit(0);
                }
		else
			exit(1);
	} else {
		fprintf(stderr, MSGSTR(BADPATH, "getconf: must not specify pathname with system_var\n"));
		exit(2);
	}
	
}
	
usage(void)
{
	fprintf(stderr, MSGSTR(USAGE1, "Usage: getconf system_var\n"));
	fprintf(stderr, MSGSTR(USAGE2, "       getconf path_var pathname\n"));
	exit(2);
}

