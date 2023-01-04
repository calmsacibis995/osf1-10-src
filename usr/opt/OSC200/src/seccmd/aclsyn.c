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
static char	*sccsid = "@(#)$RCSfile: aclsyn.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 15:23:23 $";
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
/*  Copyright (c) 1988-90 SecureWare, Inc.
 *    All rights reserved
 *
 *  Synonym compiler for Access Control Lists
 *
 *  Usage Summary:
 *	aclsyn [-d database] [synonym] ...	# print synonyms
 *	aclsyn [-d database] -r [file] ...	# recompile the database
 *	aclsyn [-d database] -a [file] ...	# add to the database
 *	aclsyn [-d database] -u synonym ...	# undefine synonyms
 */



/*
 * Based on:

 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "aclsyn_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ACLSYN,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif
#if SEC_ACL_SWARE

#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <acl.h>

#define	PRINT		'p'
#define	REPLACE		'r'
#define	ADD		'a'
#define	UNDEFINE	'u'

#ifndef FALSE
#define	FALSE		0
#endif
#ifndef TRUE
#define	TRUE		1
#endif


/*
 * Global variables
 */
FILE		*inputfp;	/* current input file descriptor */
char		*inputfile;	/* current input file name */
char		*dbfile;	/* synonym database file name */
int		inputline;	/* current input line number */
int		syn_error;	/* cumulative error counter */
int		mem_error;

/*
 * Forward declarations of non-int functions
 */
char		*tempdbname();

/*
 * External variables and functions
 */
extern aclsyn_t	acl_syns;	/* head of in-core synonym list */
extern int	optind;
extern char	*optarg;
extern char	*strrchr(), *malloc(), *realloc(), *getenv();

/*
 * Main program for ACL synonym compiler
 */
main(argc, argv)
	int	argc;
	char	*argv[];
{
	int	c, mode = PRINT;

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_ACLSYN,NL_CAT_LOCALE);
#endif

	/* Scan arguments for switches */
	while ((c = getopt(argc, argv, "d:rau")) != EOF)
		switch (c) {
		case 'd':	/* Specify the database file */
			dbfile = optarg;
			break;
		case 'r':	/* Replace an existing database */
			if (mode != PRINT)
				usage();
			mode = REPLACE;
			break;
		case 'a':	/* Add to an existing database */
			if (mode != PRINT)
				usage();
			mode = ADD;
			break;
		case 'u':	/* Undefine synonyms */
			if (mode != PRINT)
				usage();
			mode = UNDEFINE;
			break;
		default:
			usage();
			break;
		}

	/* Use default database file if none specified */
	if (dbfile == NULL) {
		dbfile = getenv(ACL_DBENV);
		if (dbfile == NULL)
			dbfile = ACL_DBFILE;
	}
	
	/* Execute requested operation */
	switch (mode) {
	case ADD:
		/* preload existing database */
		acl_load_syns(dbfile);
		/* fall through */

	case REPLACE:
		if (optind >= argc) {	/* no file args, read stdin */
			inputfile = "<stdin>";
			inputfp = stdin;
			inputline = 0;
			yyparse();
		} else
			for (; optind < argc; ++optind) {
				inputfile = argv[optind];
				inputfp = fopen(inputfile, "r");
				if (inputfp == NULL) {
					perror(inputfile);
					continue;
				}
				inputline = 0;
				yyparse();
			}
		if (syn_error > 0 || acl_store_syns(dbfile) == ACL_ERR)
			exit(1);
		break;

	case UNDEFINE:
		if (optind >= argc)	/* need at least 1 more arg */
			usage();
		if (acl_load_syns(dbfile) == ACL_ERR) {
			fprintf(stderr, MSGSTR(ACLSYN_1, "Can't load synonyms from %s\n"),
				dbfile);
			exit(1);
		}
		for (; optind < argc; ++optind)
			acl_delete_syn(argv[optind]);
		if (acl_store_syns(dbfile) == ACL_ERR) {
			fprintf(stderr, MSGSTR(ACLSYN_2, "Can't rewrite synonym file %s\n"),
				dbfile);
			exit(1);
		}
		break;

	case PRINT:
		if (acl_load_syns(dbfile) == ACL_ERR) {
			fprintf(stderr, MSGSTR(ACLSYN_1, "Can't load synonyms from %s\n"),
				dbfile);
			exit(1);
		}
		if (optind >= argc)	/* no synonym args, print them all */
			print_all_syns();
		else
			for (; optind < argc; ++optind)
				print_syn(argv[optind]);
		break;
	}

	exit(0);
}

/*
 * Print usage message and exit
 */
usage()
{
	fprintf(stderr, MSGSTR(ACLSYN_3, "usage:	aclsyn [-d database] [synonym] ...\n"));
	fprintf(stderr, MSGSTR(ACLSYN_4, "	aclsyn [-d database] -u synonym ...\n"));
	fprintf(stderr, MSGSTR(ACLSYN_5, "	aclsyn [-d database] -a [file] ...\n"));
	fprintf(stderr, MSGSTR(ACLSYN_6, "	aclsyn [-d database] -r [file] ...\n"));
	exit(1);
}

/*
 * Print a synonym by name
 */
print_syn(name)
	char	*name;
{
	aclsyn_t	*sp;

	sp = acl_lookup_syn(name);
	if (sp == NULL)
		fprintf(stderr, MSGSTR(ACLSYN_7, "%s: undefined synonym\n"), name);
	else
		print_one_syn(sp);
}

/*
 * Print all defined synonyms
 */
print_all_syns()
{
	register aclsyn_t	*sp;

	for (sp = acl_syns.syn_next; sp; sp = sp->syn_next)
		print_one_syn(sp);
}

/*
 * Print a synonym given a pointer to its definition
 */
print_one_syn(sp)
	aclsyn_t	*sp;
{
	register acle_t	*ep;
	register int	count, cc;
	char		buf[24];

	cc = printf("%s =", sp->syn_name);

	ep = sp->syn_ents;
	for (count = sp->syn_count; count > 0; ++ep, --count) {

		if (cc >= 56) {
			printf("\n\t");
			cc = 8;
		}
		acl_1ir_to_er(ep, buf);
		cc += printf(" %s", buf);
	}
	putchar('\n');
}

/*
 * Copy entries from one synonym into another
 */
copy_entries(src, dst)
	register aclsyn_t	*src, *dst;
{
	if (src->syn_count)
		if (acl_expand_syn(dst, src->syn_count) == ACL_OK) {
			memcpy((char *)(dst->syn_ents + dst->syn_count),
				(char *)src->syn_ents,
				src->syn_count * sizeof(acle_t));
			dst->syn_count += src->syn_count;
		} else if (mem_error++ == 0)
			yyerror(MSGSTR(ACLSYN_8, "out of memory"));
}

/*
 * Add a new ACL entry to a synonym
 */
add_entry(sp, uid, gid, perm)
	register aclsyn_t	*sp;
	int			uid, gid, perm;
{
	register acle_t	*ep;

	if (acl_expand_syn(sp, 1) == ACL_OK) {
		ep = &sp->syn_ents[sp->syn_count];
		ep->acl_uid = uid;
		ep->acl_gid = gid;
		ep->acl_perm = perm;
		sp->syn_count += 1;
	} else if (mem_error++ == 0)
		yyerror(MSGSTR(ACLSYN_8, "out of memory"));
}

#endif
