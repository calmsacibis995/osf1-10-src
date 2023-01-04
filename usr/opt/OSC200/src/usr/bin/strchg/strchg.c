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
static char *rcsid = "@(#)$RCSfile: strchg.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/10/11 20:00:32 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1991  Mentat Inc.
 ** strchg.c 1.1, last change 4/8/91
 **/

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <stropts.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <locale.h>
#include "strchg_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_STRCHG,n,s)

#include <sys/stream.h>
#define staticf static

	void	cpp_blast(   char **   );
	int	cpp_len(   char **   );
staticf int	get_module_names(   int fd, struct str_list * list   );
staticf	char	** get_module_names_from_file(   char * file_name   );
staticf void	pop_modules(   int fd, int number_to_pop   );

extern	char	* optarg;
extern	int	optind;

staticf char *
errmsg (err)
	int	err;
{
	static	char	buf[40];

	if (err  ||  (err = errno)) {
		if (err > 0  &&  err < sys_nerr  &&  sys_errlist[err])
			return sys_errlist[err];
	}
	if (err)
		(void)sprintf(buf, MSGSTR(STRCHG_ERRNUM,
		    "error number %d"), err);
	else
		(void)sprintf(buf, MSGSTR(STRCHG_UNERR, "unspecified error"));
	return buf;
}

main (argc, argv)
	int	argc;
	char	** argv;
{
	int	c;
	char	* cp;
	int	do_pop;
	char	* file_name;
	int	i1;
	struct str_list	list;
	char	* module_name;
	int	new_index;
	char	** new_modules;
	int	number_of_modules;
	int	old_index;
	int	pop_all;

	(void) setlocale( LC_ALL, "" );
        catd = catopen(MF_STRCHG,NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, MSGSTR(STRCHG_AUTH,
    		    "strchg: need sysadmin authorization\n"));
		exit(1);
	}
	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_OWNER,
#if SEC_MAC
		SEC_ALLOWMACACCESS,
#endif
		-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(STRCHG_PRIV,
		    "strchg: insufficient privileges\n"));
		exit(1);
}
#endif
	do_pop = 0;
	pop_all = 0;
	file_name = 0;
	module_name = 0;
	while ((c = getopt(argc, argv, "aAf:F:h:H:pPu:U:")) != -1) {
		switch (c) {
		case 'a':
		case 'A':
			if (file_name  ||  module_name)
				goto print_all_usage;
			if (!do_pop) {
				printf(MSGSTR(STRCHG_USAGE,
				    "usage: strchg -p [-a | -u module]\n"));
				exit(1);
			}
			pop_all = 1;
			break;
		case 'f':
		case 'F':
			if (module_name ||  do_pop)
				goto print_all_usage;
			file_name = optarg;
			break;
		case 'h':
		case 'H':
			if (file_name ||  do_pop)
				goto print_all_usage;
			module_name = optarg;
			break;
		case 'p':
		case 'P':
			if (file_name  ||  module_name)
				goto print_all_usage;
			do_pop = 1;
			break;
		case 'u':
		case 'U':
			if (file_name  ||  module_name)
				goto print_all_usage;
			module_name = optarg;
			if (!do_pop  ||  !module_name) {
				printf(MSGSTR(STRCHG_USAGE,
				    "usage: strchg -p [-a | -u module]\n"));
				exit(1);
			}
			break;
		default:
			goto print_all_usage;
		}
	}

	/* Handle -p option here */
	if (do_pop) {
		if (pop_all) {
			/* Pop all modules on the stream (-a option). */
			number_of_modules = get_module_names(0, (struct str_list *)0);
			pop_modules(0, number_of_modules-1);
			exit(0);
		}
		if (module_name) {
			/* Pop up to module name given (-u option). */
			number_of_modules = get_module_names(0, &list);
			for (i1 = 0; i1 < number_of_modules; i1++) {
				if (strcmp(module_name, list.sl_modlist[i1].l_name) == 0)
					break;
			}
			free(list.sl_modlist);
			if (i1 == number_of_modules) {
				printf(MSGSTR(STRCHG_NOMOD,
				"module \"%s\" is not present in the stream\n"),
				    module_name);
				exit(1);
			}
			pop_modules(0, i1);
			exit(0);
		}
		/* Pop the topmost module only (-p). */
		pop_modules(0, 1);
		exit(0);
	}

	/* Handle -h option. */
	if (module_name) {
		/* Push modules.  Comma separated in module_name. */
		do {
			if (cp = strchr(module_name, ','))
				*cp = '\0';
			if (ioctl(0, I_PUSH, module_name) == -1) {
				printf(MSGSTR(STRCHG_IPUSHF,
				"I_PUSH ioctl failed for module \"%s\", %s\n"),
				    module_name, errmsg(0));
				exit(1);
			}
			if (!cp)
				break;
			module_name = ++cp;
		} while (*module_name);
		exit(0);
	}

	/*
	 * Handle -f option.
	 * Read module names from file and configure stream as necessary.
	 */
	if (file_name) {
		/* Get the list of modules already on the stream */
		get_module_names(0, &list);
		list.sl_nmods--;	/* Decrement to remove device name from the list */

		/* Get the list of new modules from the file */
		new_modules = get_module_names_from_file(file_name);
		new_index = cpp_len(new_modules);

		/* Now figure out how to fix things... */
		new_index--;
		old_index = list.sl_nmods - 1;
		for (; new_index >= 0 && old_index >= 0; new_index--, old_index--) {
			if (strcmp(new_modules[new_index], list.sl_modlist[old_index].l_name) != 0)
				break;
		}

		/* Pop off the old modules. */
		pop_modules(0, old_index+1);

		/* Push the new ones. */
		while (new_index >= 0) {
			if (ioctl(0, I_PUSH, new_modules[new_index]) == -1) {
				printf(MSGSTR(STRCHG_IPUSHF,
				   "I_PUSH ioctl for module \"%s\" failed, %s"),
				    new_modules[new_index], errmsg(0));
				cpp_blast(new_modules);
				exit(1);
			}
			new_index--;
		}
		cpp_blast(new_modules);
		exit(0);
	}

	/*
	 * The only way to get here is if no command line arguments were given.
	 * That's a no-no.
	 */
print_all_usage:
	printf(MSGSTR(STRCHG_USE1, "usage: strchg -h module1[,module2 ... ]\n"));
	printf(MSGSTR(STRCHG_USE2, "usage: strchg -p [-a | -u module]\n"));
	printf(MSGSTR(STRCHG_USE3, "usage: strchg -f file\n"));
	exit(1);
}

/*
 * This routine returns the number of modules on a stream (including the device)
 * and places the module names in "list" (if non-nil).
 */
staticf int
get_module_names (fd, list)
	int	fd;
	struct str_list	* list;
{
	int	i1;

	/* First get the number of modules on the stream */
	i1 = ioctl(fd, I_LIST, (struct str_list *)0);
	if (i1 == -1) {
		printf("I_LIST ioctl failed, %s\n", errmsg(0));
		exit(1);
	}

	/* If the caller wants the module names as well, place them in list. */
	if (list) {
		/* Allocate space for the module names */
		list->sl_nmods = i1;
		if (!(list->sl_modlist = (struct str_mlist *)malloc(i1 * sizeof(struct str_mlist)))) {
			printf(MSGSTR(STRCHG_NMALL,
			    "Couldn't malloc space for module list"));
			exit(1);
		}

		/* Now get the actual module names */
		i1 = ioctl(fd, I_LIST, list);
		if (i1 == -1) {
			printf("I_LIST ioctl failed, %s\n", errmsg(0));
			exit(1);
		}
	}
	return i1;
}

/*
 * This routine reads the file specified by "file_name" and converts each line in
 * the file to a null-terminated string.  A cpp array is created with one entry
 * pointing to each string.
 */
staticf char **
get_module_names_from_file (file_name)
	char	* file_name;
{
	char	buf[FMNAMESZ*2];	/* Module names should only be FMNAMESZ long, but add a little extra */
	char	* cp;
	char	** cpp;
	char	** cpp_orig;
	FILE	* fp;
	int	i1;
	int	len_needed;

	if ((fp = fopen(file_name, "r")) == NULL) {
		printf(MSGSTR(STRCHG_NOPEN,
		    "Couldn't open \"%s\", %s\n"), file_name, errmsg(0));
		exit(1);
	}

	/* First count the number of lines (modules) in the file */
	i1 = 0;
	while (fgets(buf, sizeof(buf), fp) != NULL)
		i1++;
	fclose(fp);

	if (i1 == 0) {
		printf(MSGSTR(STRCHG_NOMODN,
		    "No module names were given in the file\n"));
		exit(1);
	}

	i1++;			/* Allow for null pointer at end of array */
	if (!(cpp = (char **)malloc(i1 * sizeof(char *)))) {
		printf(MSGSTR(STRCHG_NMALL,
		    "Couldn't allocate memory for module list\n"));
		exit(1);
	}
	cpp_orig = cpp;

	if ((fp = fopen(file_name, "r")) == NULL) {
		printf(MSGSTR(STRCHG_NOPEN,
		    "Couldn't open \"%s\", %s\n"), file_name, errmsg(0));
		exit(1);
	}

	/* Now read the list again and place in the cpp array */
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (cp = strrchr(buf, '\n'))
			*cp = '\0';
		if (!(cp = (char *)malloc(strlen(buf)+1))) {
			printf(MSGSTR(STRCHG_NMNAME,
			    "Couldn't allocate memory for module name\n"));
			exit(1);
		}
		strcpy(cp, buf);
		*cpp++ = cp;
	}
	*cpp = 0;
	fclose(fp);

	return cpp_orig;
}

/* Free all elements in cpp then free cpp itself. */
void
cpp_blast (cpp)
	char	** cpp;
{
	char	** cpp1;

	if (cpp1 = cpp) {
		while (*cpp1)
			free(*cpp1++);
		free(cpp);
	}
}

/* Count the number of entries in cpp */
int
cpp_len (cpp)
	char	** cpp;
{
	int	i1;

	for (i1 = 0; *cpp++; i1++)
		;
	return i1;
}

staticf void
pop_modules (fd, number_to_pop)
	int	fd;
	int	number_to_pop;
{
	while (number_to_pop-- > 0) {
		if (ioctl(fd, I_POP, 0) == -1) {
			printf("I_POP ioctl failed, %s\n", errmsg(0));
			exit(1);
		}
	}
}
