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
static char *rcsid = "@(#)$RCSfile: authck.c,v $ $Revision: 4.2.10.4 $ (DEC) $Date: 1993/10/07 14:27:40 $";
#endif
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 */

/* #ident "@(#)authck.c	6.4 09:22:23 2/27/91 SecureWare" */

/*
 * Based on:
 *   "@(#)authck.c	10.1.1.2 14:57:25 8/23/90 SecureWare, Inc."
 */

/*
 * This program validates the infrastructure of the Authentication database
 * by performing a set of tests.
 */

#include <sys/secdefines.h>
#include <locale.h>
#include "authck_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_AUTHCK,n,s)

#if SEC_BASE /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>

#ifdef SEC_NET_TTY
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#include <errno.h>
#include <prot.h>

#define AUTH_NAME	"auth"
#define	HELP_DIR	"help"
#define	SUBSYS_MODE	0640
#define	PROT_DB		"/tcb/files/auth"
#define PRPW_DB		"Protected Password Database"

#include <string.h>

#define USER_DB		"/etc/auth/subsystems/users"

struct cmdauth {
	char	*auth_name;
	priv_t	*auth_impl;
};

extern struct cmdauth	*cmdauths;		/* command authorizations */
extern priv_t		*defauths;		/* system default auths */

static int		bad_sys_default = 0;

struct rmnames {
	char	*name;
	struct rmnames	*next;
} *names_ptr = (struct rmnames *) 0;

static struct rmnames	*save_names_ptr;

static char	*dflt_auths[] = {
	"default"
};

#define CMDAUTH_DB	"Command Authorization Database"
#define WIDTH 80

struct pr_names  {
	char *name;
	int seen;
};

struct pr_passwd *pr;

/* Data structure to store the command authorizations in the
 * protected password database.
 */

struct cmd_auth {
	char	name[sizeof(pr->ufld.fd_name)];
	char	prpw_default;
	char	cmd_default;
	char	wrong;
	mask_t	prpw_mask[sizeof(pr->ufld.fd_cprivs)/sizeof(mask_t)];
	mask_t	cmdauth_mask[sizeof(pr->ufld.fd_cprivs)/sizeof(mask_t)];
};

static struct cmd_auth *cmd_auth = (struct cmd_auth *) 0;
static int cmd_auth_size = 0;
static int n_cmd_auth = 0;

/* the number of users appearing in the command authorization database
 * without passwd file entries
 */
static int extra_names = 0;

static void passwd_to_pr();
static void check_terminal();
static void check_cmdauths();
static void prpwd_cmdauth_entries();
static void extra_subsys_entries();
static struct pr_names *build_pr_passwd_names();
static struct pr_names *add_pr_name();
static void check_pr();
static void check_passwd_and_pr();
static void check_passwd_and_pr_fields();
static void check_pr_consistency();
static void check_admin_seed();
static void fix_cmdauth();
static void remove_cmdauth();
static void check_format();
static int comp();
static int check_db_entries();
static void check_fcdb();
static char *qstrtok();

int verbose = 0;
int total_errors = 0;
int dontask = 0;



int
main(argc, argv)
	int argc;
	char *argv[];
{
	register int option;
	int password = 0;
	int term = 0;
	int cmdauths = 0;
	int fcdb = 0;
	int error = 0;

        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_AUTHCK,NL_CAT_LOCALE);

	set_auth_parameters(argc, argv);
	initprivs();
	if (!authorized_user("auth")) {
		fprintf(stderr,
			MSGSTR(AUTHCK_112, "%s: need auth authorization\n"),
			command_name);
		exit(1);
	}

	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_OWNER, SEC_CHOWN,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, "%s: insufficient privileges\n", command_name);
		exit(1);
	}

	opterr = 0;
	while ((option = getopt(argc, argv, "vnaptcf")) != EOF)
		switch (option) {
			/*
			 * Tell about all actions.
			 */
			case 'v':
				verbose++;
				break;

			/*
			 * Process full Authentication database checking.
			 */
			case 'a':
				password++;
				term++;
				cmdauths++;
				fcdb++;
				break;

			/*
			 * Protected Password Database checking.
			 */
			case 'p':
				password++;
				break;

			/*
			 * Terminal Control Database checking.
			 */
			case 't':
				term++;
				break;

			/*
			 * Command Authorization Database checking.
			 */
			case 'c':
				cmdauths++;
				break;

			/*
			 * File Control Database checking.
			 */
			case 'f':
				fcdb++;
				break;

			/*
			 * don't ask to fix Command Authorization database
			 */
			case 'n':
				dontask++;
				break;

			case '?':
				error = 1;
				break;
			}

	if (error || (optind < argc)) {
		fprintf(stderr,
			MSGSTR(AUTHCK_2, "usage:  %s [-v] [-a] [-p] [-t] [-c] [-f] [-n]\n"),
			command_name);
		exit(2);
	}

	if (!password && !term && !cmdauths && !fcdb)  {
		fprintf(stderr,
			MSGSTR(AUTHCK_3, "%s: Need one of -a -p -t -f -c\n"), command_name);
		exit(2);
	}


	/*
	 * Perform the tests that were requested.
	 */
	if (password)
		passwd_to_pr();
	if (term)
		check_terminal();
	if (cmdauths)
		check_cmdauths();
	if (fcdb)
		check_fcdb();

	return total_errors;
}


/*
 * Make sure that the names from /etc/passwd and the Protected Passwd
 * files match one-to-one.  Also, check for internal consistency in
 * the Protected Password database.
 */
static void
passwd_to_pr()
{
	struct pr_names *pr_names;
	int pr_size;

	pr_size = 0;
	pr_names = build_pr_passwd_names(&pr_size);
	if (pr_names != (struct pr_names *) 0)  {
		check_format(OT_PRPWD);
		check_pr(pr_names, pr_size);
		check_passwd_and_pr(pr_names, pr_size);
		check_passwd_and_pr_fields(pr_names, pr_size);
		check_pr_consistency();
	}
}


/*
 * Make sure that the fields in the Terminal Control Database are sane.
 */
static void
check_terminal()
{
	register struct pr_term *pr;
	register struct passwd *p;
	register time_t now;
	register entry_no = 0;
#ifdef SEC_NET_TTY
        struct hostent *hp;
        struct in_addr addr;
#endif
	check_format(OT_TERM_CNTL);

	setprtcent();

	if (verbose)
		printf(MSGSTR(AUTHCK_4, "Checking all entries in the Terminal Control database\n"));
	now = time((long *) 0);

	if (chdir("/dev") < 0)
		perror("/dev");

	while ((pr = getprtcent()) != (struct pr_term *) 0)  {
		entry_no++;
		if ((pr->ufld.fd_devname[0] == ':' ) ||
		    (pr->ufld.fd_devname[0] == '*' )) {
			continue ;
		}
		if (!pr->uflg.fg_devname)  {
			total_errors++;
			printf( MSGSTR(AUTHCK_6, "Entry %d in Terminal Control database does not have a name.\n"), entry_no);
		}
		if (access(pr->ufld.fd_devname, 0) != 0)  {
#ifdef SEC_NET_TTY
			sscanf(pr->ufld.fd_devname, "%8x", &addr);
                	hp = gethostbyaddr(&addr.s_addr, 4, AF_INET);
			if (hp == (struct hostent *) 0) {
				total_errors++;
				printf( MSGSTR(AUTHCK_7, "%s in Terminal/Host Control database refers\n"), pr->ufld.fd_devname);
				printf( MSGSTR(AUTHCK_8, " to a non-existent terminal or host.\n"));
			}
#else
				total_errors++;
				printf( MSGSTR(AUTHCK_9, "%s in Terminal Control database refers to a non-existent terminal.\n"), pr->ufld.fd_devname);
#endif
		}

		if (pr->uflg.fg_slogin && (pr->ufld.fd_slogin > now))  {
			total_errors++;
			printf(MSGSTR(AUTHCK_11, "%s has a login time in the future\n"),
				pr->ufld.fd_devname);
		}
		if (!pr->uflg.fg_slogin && pr->uflg.fg_uid)  {
			total_errors++;
			printf(MSGSTR(AUTHCK_13, "%s has a last login user, with no accompanying time\n"),
				pr->ufld.fd_devname);
		}
		if (pr->uflg.fg_ulogin && (pr->ufld.fd_ulogin > now))  {
			total_errors++;
			printf(MSGSTR(AUTHCK_15, "%s has an unsuccessful login time in the future\n"),
				pr->ufld.fd_devname);
		}
		if (!pr->uflg.fg_ulogin && pr->uflg.fg_uuid)  {
			total_errors++;
			printf(MSGSTR(AUTHCK_17, "%s has a last unsuccessful login user, with no accompanying time\n"),
				pr->ufld.fd_devname);
		}
		if (pr->uflg.fg_louttime && (pr->ufld.fd_louttime > now))  {
			total_errors++;
			printf(MSGSTR(AUTHCK_19, "%s has a previous logout time in the future\n"),
				pr->ufld.fd_devname);
		}
		if (!pr->uflg.fg_louttime && pr->uflg.fg_loutuid)  {
			total_errors++;
			printf(MSGSTR(AUTHCK_21, "%s has a last logout user, with no accompanying time\n"),
				pr->ufld.fd_devname);
		}
		if (pr->uflg.fg_uid)  {
			p = getpwuid(pr->ufld.fd_uid);
			if (p == (struct passwd *) 0)  {
				total_errors++;
			printf(MSGSTR(AUTHCK_23, "The last user of %s, UID %d, is not in the Authentication database\n"),
				pr->ufld.fd_devname, pr->ufld.fd_uid);
			}
		}
		if (pr->uflg.fg_uuid)  {
			p = getpwuid(pr->ufld.fd_uuid);
			if (p == (struct passwd *) 0)  {
				total_errors++;
			printf(MSGSTR(AUTHCK_25, "The last unsuccessful user of %s, UID %d, is not in the Authentication database\n"),
				pr->ufld.fd_devname, pr->ufld.fd_uuid);
			}
		}
		if (pr->uflg.fg_loutuid)  {
			p = getpwuid(pr->ufld.fd_loutuid);
			if (p == (struct passwd *) 0)  {
				total_errors++;
				printf(
MSGSTR(AUTHCK_27, "The last user of %s to logout, UID %d,\n"),
				  pr->ufld.fd_devname, pr->ufld.fd_loutuid);
				printf(
MSGSTR(AUTHCK_28, "  is not in the Authentication database\n"));
			}
		}
	}

	endprtcent();
}

/* comparison function for sorting */
static int
comp (sa1, sa2)
struct cmd_auth *sa1, *sa2;
{
	return (strcmp (sa1->name, sa2->name));
}

#define CMD_AUTH_CHUNK	25

/* For each user entry in the protected password database,
 * store off the command authorization mask
 */

static void
prpwd_cmdauth_entries()
{
	struct cmd_auth *sa;

	cmd_auth = (struct cmd_auth *) calloc (CMD_AUTH_CHUNK,
	  sizeof (*cmd_auth));
	if (cmd_auth == (struct cmd_auth *) 0) {
		fprintf(stderr, MSGSTR(AUTHCK_29, "Unable to allocate cmd_auth\n"));
		exit (1);
	}
	cmd_auth_size = CMD_AUTH_CHUNK;

	setprpwent();
	while ((pr = getprpwent()) != (struct pr_passwd *) 0) {
                /* clear all memory in new structure space */
                int prev_size;
                struct cmd_auth *tmp_ptr;
                prev_size = cmd_auth_size;
		if (n_cmd_auth == cmd_auth_size) {
			cmd_auth_size += CMD_AUTH_CHUNK;
			cmd_auth = (struct cmd_auth *)
			  realloc (cmd_auth,
			  cmd_auth_size * sizeof (*cmd_auth));
			if (cmd_auth == (struct cmd_auth *) 0) {
				fprintf(stderr, MSGSTR(AUTHCK_30, "Can not allocate memory\n"));
				exit (1);
			}
                        /* clear all memory in new structure space */
	                tmp_ptr = cmd_auth+prev_size;
	                memset(tmp_ptr, '\0', 
                                (CMD_AUTH_CHUNK*sizeof(*cmd_auth)));
		}
		sa = &cmd_auth[n_cmd_auth++];
		strncpy (sa->name, pr->ufld.fd_name, sizeof (sa->name));
		if (pr->uflg.fg_cprivs) {
			sa->prpw_default = 0;
			memcpy (sa->prpw_mask, pr->ufld.fd_cprivs,
			  sizeof(pr->ufld.fd_cprivs));
		} else
			sa->prpw_default = 1;
	}

	/* sort the table according to name */

	qsort ((char *) cmd_auth, n_cmd_auth, sizeof (*sa), comp);
	return;
}


static void
check_cmdauths()

{
	struct pr_default	*pr_default;
	int 			count = total_auths();
	int			index;
	char			buffer[10];
	int			wauth;
	char			**auth_table;
	char			*cp;
	int			filled;

	check_format(OT_DFLT_CNTL);

	if (verbose)
		printf(MSGSTR(AUTHCK_31, "Checking all entries in the subsystem data base.\n"));

	pr_default = getprdfnam("default");
	if (pr_default == (struct pr_default *) 0) {
		fprintf(stderr, MSGSTR(AUTHCK_32, "Unable to get default data base entry\n"));
		return;
	}

	if (pr_default->prg.fg_cprivs) {
		if (memcmp((char *) pr_default->prd.fd_cprivs,
			   (char *) defauths, (count/sizeof(priv_t)) != 0)) {
			bad_sys_default = 1;

			if (verbose) {
				for (index = 0; index < count ; index++) {
					if (((ISBITSET(defauths, index)) &&
					     (ISBITSET(pr_default->prd.fd_cprivs, index))) ||
					    ((!ISBITSET(defauths, index)) &&
					     (!ISBITSET(pr_default->prd.fd_cprivs, index))))
						continue;

					if (ISBITSET(defauths, index)) {
						printf(MSGSTR(AUTHCK_33,
"The authorization \"%s\") appears in the subsystem authorization file,\n"),
 						  cmdauths[index].auth_name);
						printf(MSGSTR(AUTHCK_34,
		      "\tbut not in the system default data base\n\n"));
					} else {
						printf(MSGSTR(AUTHCK_33,
"The authorization \"%s\") appears in the system default data base,\n"),
						  cmdauths[index].auth_name);
						printf(MSGSTR(AUTHCK_35,
"\tbut not in the subsystem authorization file\n\n"));
					}
				}
			}
		}
	}
	else {
		for (index = 0; index < count ; index++) {
			if (ISBITSET(defauths, index)) {
				bad_sys_default = 1;
				if (verbose) {
					printf(MSGSTR(AUTHCK_33,
"The authorization \"%s\") appears in the subsystem authorization file,\n"),
					  cmdauths[index].auth_name);
					printf(MSGSTR(AUTHCK_34,
"\tbut not in the system default data base\n\n"));
				}
			}
		}
	}

	prpwd_cmdauth_entries();

	if(cmdauth_entries()) {
		fprintf(stderr, MSGSTR(AUTHCK_36, "Unable to read %s\n"), USER_DB); 
		endprdfent();
		free((char *) cmd_auth);
		return;
	}

	if (dontask == 0 && (compare_cmdauth() > 0 || extra_names > 0 ||
						      bad_sys_default > 0)) {

		printf(MSGSTR(AUTHCK_37, "There are discrepancies between the databases.\n"));
		printf(MSGSTR(AUTHCK_38, "Fix them (Y or N)? "));
		fflush(stdout);
		gets(buffer);
		if (buffer[0] == 'Y' || buffer[0] == 'y')
			fix_cmdauth();


		if (bad_sys_default) {
			printf(MSGSTR(AUTHCK_39, "Fixing default command authorizations\n"));
			wauth = widest_auth();

			auth_table = (char **) malloc (count * sizeof (char *) +
							count * wauth);
			if (auth_table == (char **) 0)
				return;

			cp = ((char *) auth_table) + count * sizeof(char *);
			memset (cp, '\0', count * wauth);
			for (index = 0; index < count; index++) {
				auth_table[index] = cp;
				cp += wauth;
			}

			for (index = 0; index < count; index++)
				auth_table[index][0] = '\0';

			filled = 0;
			for (index = 0; index < count; index++) {
				if (ISBITSET(pr_default->prd.fd_cprivs,
				  cmd_priv[index].value))
					strcpy (auth_table[filled++],
						cmd_priv[index].name);
			}
			write_authorizations((char *) 0, auth_table, filled);
		}
	}
        else if (compare_cmdauth() || extra_names || bad_sys_default) {
		printf(MSGSTR(AUTHCK_113, "There are discrepancies between the databases.\n"));
        }

	endprdfent();
	free((char *) cmd_auth);
}

int
cmdauth_entries() 

{
	FILE		*file_ptr;
	char		*buffer_ptr, *end_ptr, *work_ptr;
	char		*nl_ptr, *comment_ptr;
	struct stat	stat_buffer;
	int		index, priv_index;
	struct cmd_auth	sa_buf, *sa_ptr;
	int		count;
	int		new_line;
	struct rmnames	*ex_name;

	file_ptr = fopen(USER_DB, "r");
	if (file_ptr == (FILE *) 0) {
		printf(MSGSTR(AUTHCK_52,"cannot open %s.\n"),USER_DB);
		return(1);
	}

	fstat(fileno(file_ptr), &stat_buffer);

	buffer_ptr = (char *) malloc(stat_buffer.st_size + 1);
	if (buffer_ptr == (char *) 0) {
		fclose(file_ptr);
		return(1);
	}

	if (fread(buffer_ptr, stat_buffer.st_size, 1, file_ptr) != 1) {
		fclose(file_ptr);
		free(buffer_ptr);
		return(1);
	}

	fclose(file_ptr);

	count = total_auths();

	end_ptr = &buffer_ptr[stat_buffer.st_size];
	*end_ptr = '\0';
	new_line = 1;

	work_ptr = buffer_ptr;
	while(work_ptr < end_ptr) {
		nl_ptr = strchr(work_ptr, '\n');
		if (nl_ptr == (char *) 0)
			break;
		*nl_ptr = '\0';

		comment_ptr = strchr(work_ptr, '#');
		if (comment_ptr != (char *) 0)
			*comment_ptr = '\0';

		while(*work_ptr == ' ' || *work_ptr == '\t')
			work_ptr++;
		if (*work_ptr == ':') {
			work_ptr = ++nl_ptr;
			continue;
		}

		work_ptr = qstrtok(work_ptr, ":, \t");
		while(work_ptr != (char *) 0) {

			if (new_line) {
				strncpy(sa_buf.name, work_ptr,
					sizeof(sa_buf.name));
				sa_ptr = (struct cmd_auth *) bsearch(
						(char *) &sa_buf,
						(char *) cmd_auth, n_cmd_auth,
						sizeof(sa_buf), comp);
				if (sa_ptr == (struct cmd_auth *) 0) {
					extra_names = 1;
					if (verbose) {
						fprintf(stderr,
MSGSTR(AUTHCK_41, "The user \"%s\") appears in the  file \"%s\",\n"),
						  work_ptr, USER_DB);
						fprintf(stderr,
MSGSTR(AUTHCK_42, "\tbut does not have a protected password entry.\n"));
					}
					ex_name = (struct rmnames *) calloc(1,
							sizeof(struct rmnames));
					if (ex_name == (struct rmnames *) 0)
						break;
					ex_name->name = (char *) strdup(work_ptr);
					if (names_ptr == (struct rmnames *) 0)
						names_ptr = ex_name;
					else
						save_names_ptr->next = ex_name;
					save_names_ptr = ex_name;
					break;
				}
				new_line = 0;
			}
			else {
				for (index = 0; index < count ; index++) {
					if (strcmp(work_ptr,
					    cmdauths[index].auth_name) == 0) {
						ADDBIT(sa_ptr->cmdauth_mask,
						  index);
						break;
					}
				}
			}
			work_ptr = qstrtok((char *) 0, ":, \t");
		}
		work_ptr = ++nl_ptr;
		new_line = 1;
	}

	return(0);
}


/* Compare the data structures built up by searching the files */

compare_cmdauth ()
{
	int i, j;
	int nwrong = 0;
	struct cmd_auth *sa;
	char buffer[16];
	int first = 1;
	int column;
	int len;
	int spaces;

	sa = cmd_auth;
	for (i = 0; i < n_cmd_auth; i++, sa++) {
		if (memcmp ((char *)sa->prpw_mask,
			    (char *)sa->cmdauth_mask,
			    sizeof (sa->prpw_mask)) != 0)
			sa->wrong = 1;
		else
			sa->wrong = 0;

		if (sa->wrong) {
			nwrong++;
			strcpy (buffer, sa->name);
			len = strlen(buffer);
			if (first) {
				printf (
	MSGSTR(AUTHCK_43, "The following users have %s entries\n"), PRPW_DB);
				printf (
		MSGSTR(AUTHCK_44, " that do not match their %s entries:\n"), CMDAUTH_DB);
				first = 0;
				column = 0;
			}
			if (column + sizeof(cmd_auth->name) > WIDTH) {
				printf("\n");
				column = 0;
			}
			if (column == 0) {
				printf ("   ");
				column = 3;
			} else {
				spaces = sizeof(buffer) -
				  (column % sizeof(buffer));
				for (j = 0; j < spaces; j++)
					printf(" ");
				column += spaces;
			}
			printf (buffer);
			column += len;
		}
	}
	if (!first)
		printf("\n");
	return (nwrong);
}

/* fix the command authorization database based on the contents of the
 * protected password database
 */

static void
fix_cmdauth ()
{
	struct cmd_auth *sa;
	int i, j;
	int k, flag;
	char **auth_table;
	int tauths, wauth;
	int filled;
	char *cp;

	/* allocate a table of pointers to authorization strings */
	tauths = total_auths();
	wauth = widest_auth() + 1;
	auth_table = (char **) malloc (tauths * sizeof (char *) +
					tauths * wauth);
	if (auth_table == (char **) 0)
		return;
	cp = ((char *) auth_table) + tauths * sizeof(char *);
	memset (cp, '\0', tauths * wauth);
	for (i = 0; i < tauths; i++) {
		auth_table[i] = cp;
		cp += wauth;
	}
	sa = cmd_auth;
	for (i = 0; i < n_cmd_auth; i++, sa++) {
		if (sa->wrong == 0)
			continue;
		for (j = 0; j < tauths; j++)
			auth_table[j][0] = '\0';
		filled = 0;
		for (j = 0; j < tauths; j++) {
			if (ISBITSET(sa->prpw_mask, cmd_priv[j].value))
				strcpy (auth_table[filled++],
					cmd_priv[j].name);
		}
		if (verbose) {
			printf(MSGSTR(AUTHCK_45, "FIXING user: %s, authorizations: "), sa->name);
			for (k = 0; k < tauths; k++)
				if (auth_table[k][0] != '\0')
					printf (" %s", auth_table[k]);
			printf("\n");
		}
		
		flag = 1;
		for (k = 0; k < tauths; k++)
		  if (auth_table[k][0] != '\0')
		      flag = 0;
		if (flag)
		  write_authorizations(sa->name, dflt_auths, 0);
		else
		  write_authorizations (sa->name, auth_table, filled);
	}
	free ((char *) auth_table);

/*
 * remove user entries in files for users who do not have protected password
 * entries in the database.
 */

	if (extra_names > 0)
		remove_cmdauth();
	return;
}

static void
remove_cmdauth()

{
	struct rmnames		*ex_ptr;

	ex_ptr = names_ptr;
	while(ex_ptr != (struct rmnames *) 0) {
		if (verbose)
			printf(MSGSTR(AUTHCK_46,
			"Removing \"%s\") from \"%s\"\n"),
				ex_ptr->name, USER_DB);
		write_authorizations(ex_ptr->name, dflt_auths, 1);
		ex_ptr = ex_ptr->next;
	}
}

/*
 * "Manually" go through the Protected Password database file hierarchy
 * and construct a list of user (file) names.  This is later checked against
 * the names produced by the getprpw* routines.
 */
static struct pr_names *
build_pr_passwd_names(pr_size)
	int *pr_size;
{
	register struct pr_names *pr_names;
	register DIR *par_dir;
	register DIR *dir;
	struct dirent *dir_buf;
	struct dirent *file_buf;
	struct stat dir_stat;
	char dirname[sizeof(PROT_DB) + 255 + 2];
	char entry_name[256];

	if (verbose)
		printf(
MSGSTR(AUTHCK_47, "finding all entries in the Protected Password database, in %s\n"), PROT_DB);

	/*
	 * Open the top of the hierarchy.
	 */
	par_dir = opendir(PROT_DB);
	if (par_dir == (DIR *) 0)  {
		total_errors++;
		fprintf(stderr,
			MSGSTR(AUTHCK_48, "%s: cannot open Protected Password hierarchy.\n"),
			command_name);
		exit(1);
	}

	pr_names = (struct pr_names *) 0;

	/*
	 * Go through each directory in the hierarchy.  Open each in
	 * turn and collect the names contained therein.  Each of those
	 * names should correspond to a name in the Protected Password
	 * database.
	 */
	while ((dir_buf = readdir(par_dir)) != (struct dirent *) 0) {

		if (dir_buf->d_fileno == 0 ||
		    strcmp(dir_buf->d_name, ".") == 0 ||
		    strcmp(dir_buf->d_name, "..") == 0 ||
		    strcmp(dir_buf->d_name, HELP_DIR) == 0)
			continue;

		strcpy(entry_name, dir_buf->d_name);
		strcpy(dirname, PROT_DB);
		strcat(dirname, "/");
		strcat(dirname, entry_name);

		if (stat(dirname, &dir_stat) == -1)  {
			if (verbose)  {
				total_errors++;
				printf(MSGSTR(AUTHCK_49, "cannot access file in Protected Password hierarchy"));
				perror(dirname);
			}
			continue;
		}

		if ((dir_stat.st_mode & S_IFMT) != S_IFDIR ||
		    strlen(entry_name) != 1) {
			if (verbose)  {
				total_errors++;
				fprintf(stderr,
					MSGSTR(AUTHCK_51, "%s is an extraneous entry in %s\n"),
					entry_name, PROT_DB);
			}
			continue;
		}

		/* Found a directory.  The files in it will be remembered. */

		dir = opendir(dirname);
		if (dir == (DIR *) 0)  {
			total_errors++;
			fprintf(stderr, MSGSTR(AUTHCK_52, "cannot open %s.\n"), dirname);
			continue;
		}

		while ((file_buf = readdir(dir)) != (struct dirent *) 0) {

			if (file_buf->d_fileno == 0 ||
			    strcmp(file_buf->d_name, ".") == 0 ||
			    strcmp(file_buf->d_name, "..") == 0)
				continue;

			if (verbose && *file_buf->d_name != *dir_buf->d_name) {
				total_errors++;
				printf(MSGSTR(AUTHCK_53, "%s does not belong in %s\n"),
					file_buf->d_name, dirname);
			} else {
				/* Actually store the name found. */
				pr_names = add_pr_name(pr_names, pr_size,
						file_buf->d_name);
			}
		}

		(void) closedir(dir);
	}

	(void) closedir(par_dir);

	return pr_names;
}


/*
 * Add the name to the list of names so far and return the new list.
 * Also, update the new list size.
 */
static struct pr_names *
add_pr_name(pr_names, pr_size, new)
	struct pr_names *pr_names;
	int *pr_size;
	char *new;
{
	char *save_name;

	(*pr_size)++;
	if (pr_names == (struct pr_names *) 0)
		pr_names = (struct pr_names *) malloc(sizeof(struct pr_names));
	else
		pr_names = (struct pr_names *) realloc((char *) pr_names,
				*pr_size * sizeof(struct pr_names));

	if (pr_names == (struct pr_names *) 0)  {
		total_errors++;
		fprintf(stderr, MSGSTR(AUTHCK_54, "%s: ran out of space allocating name %s\n"),
			command_name, new);
		exit(1);
	}

	pr_names[*pr_size - 1].name = malloc(strlen(new) + 1);
	if (pr_names[*pr_size - 1].name == (char *) 0)  {  {
		total_errors++;
		fprintf(stderr, MSGSTR(AUTHCK_54, "%s: ran out of space allocating name %s\n"),
			command_name, new);
		}
		exit(1);
	}

	(void) strcpy(pr_names[*pr_size - 1].name, new);
	pr_names[*pr_size - 1].seen = 0;

	return pr_names;
}


/*
 * Make sure each entry in the Protected Password database has a name.
 * Also, make sure the scan used with successive calls to getprpwent()
 * is complete and catures all the files we obtained manually.
 */
static void
check_pr(pr_names, pr_size)
	struct pr_names *pr_names;
	int pr_size;
{
	register struct pr_passwd *pr;
	register int scan;
	register int found;

	if (verbose)
		printf(MSGSTR(AUTHCK_55, "Checking Protected Password against getprpwent()\n"));

	for (scan = 0; scan < pr_size; scan++)
		pr_names[scan].seen = 0;

	/*
	 * Make sure the getprpwent() routine captures all the true
	 * entries in the Protected Password database.  Also, make
	 * sure that getprpwent() only gets entries once.
	 */
	setprpwent();
	while ((pr = getprpwent()) != (struct pr_passwd *) 0)  {
		found = 0;
		for (scan = 0; !found && (scan < pr_size); scan++)  {
			if (!pr->uflg.fg_name || !pr->uflg.fg_uid)  {
				total_errors++;
				printf(
MSGSTR(AUTHCK_57, "Malformed entry in Protected Password database\n"));
			}
			else if (strcmp(pr_names[scan].name,
					pr->ufld.fd_name) == 0)  {
				if (pr_names[scan].seen)  {
					total_errors++;
				   printf(MSGSTR(AUTHCK_59, "Duplicate userid allocated for %s\n"),
						pr->ufld.fd_name);
				}
				pr_names[scan].seen++;
				found = 1;
			}
		}

		if (!found)  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_61, "%s appears in Protected Password database but cannot be retrieved\n"),
 			  pr->ufld.fd_name);
			printf(MSGSTR(AUTHCK_62, "\twith getprpwent()\n"));
		}
	}

	endprpwent();

	/*
	 * Now rescan the entries we picked up in the Protected Password
	 * database and see if any are not available with getprpwent().
	 */
	for (scan = 0; !found && (scan < pr_size); scan++)
		if (!pr_names[scan].seen)  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_61, "%s appears in Protected Password database but cannot be retrieved\n"),
			  pr->ufld.fd_name);
			printf(MSGSTR(AUTHCK_62, "\twith getprpwent()\n"));
		}
}


/*
 * Make sure the scans done by getpwent() and getprpwent() cover each
 * other.  Also, make sure that a name is listed only once in /etc/passwd.
 */
static void
check_passwd_and_pr(pr_names, pr_size)
	struct pr_names *pr_names;
	int pr_size;
{
	register struct passwd *p;
	register int scan;
	register int found;

	if (verbose)
		printf(MSGSTR(AUTHCK_63, "Checking Protected Password against /etc/passwd\n"));

	for (scan = 0; scan < pr_size; scan++)
		pr_names[scan].seen = 0;

	/*
	 * Make sure all entries in /etc/passwd have a counterpart in
	 * the Protected Password database in PROT_DB.
	 */
	setpwent();
	while((p = getpwent()) != (struct passwd *) 0)  {
		found = 0;
		for (scan = 0; !found && (scan < pr_size); scan++)  {
			if (strcmp(pr_names[scan].name, p->pw_name) == 0)  {
				if (pr_names[scan].seen)  {
					total_errors++;
				   printf(MSGSTR(AUTHCK_65, "%s multiply listed in /etc/passwd\n"),
					p->pw_name);
				}
				pr_names[scan].seen++;
				found = 1;
			}
		}

		if (!found)  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_67, "%s appears in /etc/passwd but not in Protected Password database\n"),
			  p->pw_name);
		}
	}

	/*
	 * Now rescan the entries we picked up in the Protected Password
	 * database and see if any are not listed in /etc/passwd.
	 */
	for (scan = 0; scan < pr_size; scan++)
		if (!pr_names[scan].seen)  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_68, "%s not listed in /etc/passwd, but is in the Protected Password database.\n"),
				pr_names[scan].name);
		}
}


/*
 * Check that the common fields in /etc/passwd and the Protected Password
 * file are in sync.
 */
static void
check_passwd_and_pr_fields(pr_names, pr_size)
	struct pr_names *pr_names;
	int pr_size;
{
	register struct pr_passwd *pr;
	register int scan;
	register int found;
	char *user_name;
	int user_id;

	if (verbose)
		printf(
MSGSTR(AUTHCK_69, "Checking Protected Password fields against those in /etc/passwd\n"));

	for (scan = 0; scan < pr_size; scan++)  {
	    pr = getprpwnam(pr_names[scan].name);
	    user_name = pr_names[scan].name;
	    user_id = pw_nametoid (user_name);
	    if ((pr != (struct pr_passwd *) 0) && (user_id >= 0))  {
		if (strcmp(user_name, pr->ufld.fd_name) != 0)  {
			total_errors++;
		   printf(MSGSTR(AUTHCK_71, "%s stored as %s in Protected Password\n"),
			user_name, pr->ufld.fd_name);
		}
		if (!pr->uflg.fg_name)  {
			total_errors++;
		   printf(MSGSTR(AUTHCK_73, "%s lacks a name in Protected Password\n"),
			user_name);
		}
		if (!pr->uflg.fg_uid)  {
			total_errors++;
		   printf(MSGSTR(AUTHCK_75, "%s lacks a uid in Protected Password\n"),
			user_name, pr->ufld.fd_uid);
		}
		if (pr->ufld.fd_uid != user_id)  {
			total_errors++;
		   printf(MSGSTR(AUTHCK_77, "%s has a uid inconsistency (it's %d in the Protected Password and %d in /etc/passwd)\n"),
			user_name, pr->ufld.fd_uid, user_id);
		}
	    }
	}
}


/*
 * Check that the fields in the Protected Password are valid.
 */
static void
check_pr_consistency()
{
	register struct pr_passwd *pr;
	register int scan;
	register int scan_privs;
	register int found;
	register time_t now;
	char *user_name;
	int user_id;

	if (verbose)
		printf(
MSGSTR(AUTHCK_78, "Checking internal consistency of Protected Password fields\n"));

	now = time((long *) 0);

	setprpwent();
	while ((pr = getprpwent()) != (struct pr_passwd *) 0)  {
		if (pr->uflg.fg_min && pr->uflg.fg_expire &&
		    (pr->ufld.fd_expire > 0) &&
		    (pr->ufld.fd_min > pr->ufld.fd_expire))  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_80, "%s has a shorter password expiration period than minimum change period\n"),
			pr->ufld.fd_name);
		}
		if (pr->uflg.fg_lifetime && pr->uflg.fg_expire &&
		    (pr->ufld.fd_lifetime < pr->ufld.fd_expire))  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_82, "%s has a shorter password lifetime than an expiration period\n"),
			pr->ufld.fd_name);
		}
		if ((!pr->uflg.fg_encrypt ||
		    (pr->ufld.fd_encrypt[0] == '\0')) &&
		    !(pr->uflg.fg_nullpw && pr->ufld.fd_nullpw) &&
		    !(pr->sflg.fg_nullpw && pr->sfld.fd_nullpw)) {
			total_errors++;
			printf(MSGSTR(AUTHCK_84, "%s has no password\n"), pr->ufld.fd_name);
		}
		if (pr->uflg.fg_owner)  {
			user_id = pw_nametoid(pr->ufld.fd_owner);
			if (user_id == -1)  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_86, "%s has an owner of '%s', who is not in the Authentication database\n"),
				pr->ufld.fd_name, pr->ufld.fd_owner);
			}
		}
/*
		if (!pr->uflg.fg_auditcntl && pr->uflg.fg_auditdisp)  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_88, "%s has an audit disposition mask but has no audit control mask\n"),
				pr->ufld.fd_name);
		}
		if (pr->uflg.fg_auditcntl && pr->uflg.fg_auditdisp)
		    for (scan_privs = 0; scan_privs < AUTH_AUDITMASKVEC_SIZE;
		         scan_privs++)  {
			if ((~pr->ufld.fd_auditcntl[scan_privs] &
			    pr->ufld.fd_auditdisp[scan_privs]) != 0)  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_90, "%s has an audit disposition mask having more entries than the control mask\n"),
				pr->ufld.fd_name);
			}
		    }
*/
		if (pr->uflg.fg_schange && (pr->ufld.fd_schange > now))  {
			total_errors++;
			printf(MSGSTR(AUTHCK_92, "%s has a password change time in the future\n"),
				pr->ufld.fd_name);
		}
		if (pr->uflg.fg_uchange && (pr->ufld.fd_uchange > now))  {
			total_errors++;
			printf(
MSGSTR(AUTHCK_94, "%s has an unsuccessful password change time in the future\n"),
				pr->ufld.fd_name);
		}

		if ((!pr->uflg.fg_gen_pwd || !pr->ufld.fd_gen_pwd) &&
		    (!pr->sflg.fg_gen_pwd || !pr->sfld.fd_gen_pwd) &&
		    (!pr->uflg.fg_gen_chars || !pr->ufld.fd_gen_chars) &&
		    (!pr->sflg.fg_gen_chars || !pr->sfld.fd_gen_chars) &&
		    (!pr->uflg.fg_gen_letters || !pr->ufld.fd_gen_letters) &&
		    (!pr->sflg.fg_gen_letters || !pr->sfld.fd_gen_letters) &&
		    (!pr->uflg.fg_pick_pwd || !pr->ufld.fd_pick_pwd) &&
		    (!pr->sflg.fg_pick_pwd || !pr->sfld.fd_pick_pwd))  {
			total_errors++;
			printf(MSGSTR(AUTHCK_96, "%s cannot have a password set on the account\n"),
				pr->ufld.fd_name);
		}
		if (pr->uflg.fg_slogin && (pr->ufld.fd_slogin > now))  {
				total_errors++;
			printf(MSGSTR(AUTHCK_11, "%s has a login time in the future\n"),
				pr->ufld.fd_name);
		}
		if (pr->uflg.fg_ulogin && (pr->ufld.fd_ulogin > now))  {
				total_errors++;
				printf(
MSGSTR(AUTHCK_15, "%s has an unsuccessful login time in the future\n"), pr->ufld.fd_name);
		}
	}
	endprpwent();
}



static void
check_format(database)
int database;
{
	char *pathname;
	struct pr_names *pr_entries;
        int pr_num, scan, errors;

        switch(database) {
                case OT_TERM_CNTL:
                        pathname = find_auth_file((char *) 0, OT_TERM_CNTL); 
			if (verbose)
				printf(
MSGSTR(AUTHCK_99, "Checking format of Terminal Control database %s\n"),pathname);
                        if ((check_db_entries(pathname) == 0) && (verbose))
                                printf(MSGSTR(AUTHCK_100, "Format of Terminal Control database OK\n"));
                        break;

                case OT_DEV_ASG:
                        pathname = find_auth_file((char *) 0, OT_DEV_ASG);
			if (verbose)
				printf(
MSGSTR(AUTHCK_101, "Checking format of Device Assignment database %s\n"), pathname);
                        if ((check_db_entries(pathname) == 0) && (verbose))
                                printf(
MSGSTR(AUTHCK_102, "Format of Device Assignment database OK\n"));
                        break;

		case OT_PRPWD:
			if (verbose)
				printf(
MSGSTR(AUTHCK_103, "Checking format of files in Protected Password database %s\n"), PROT_DB);
                        errors = 0;
			pr_num = 0;
			pr_entries = build_pr_passwd_names(&pr_num);
                        for (scan = 0; scan < pr_num; scan++) {
                                pathname = find_auth_file(pr_entries[scan].name,
                                                                OT_PRPWD);
                                if (check_db_entries(pathname) != 0)
                                        errors++;
                        }
                        if ((!errors) && (verbose))
                                printf(
MSGSTR(AUTHCK_104, "Format of all Protected Password entries OK\n"));
                        break;

		case OT_DFLT_CNTL:
			pathname = find_auth_file((char *) 0, OT_DFLT_CNTL);
			if (verbose)
				printf(
MSGSTR(AUTHCK_105, "Checking format of Defaults database %s\n"), pathname);
			if ((check_db_entries(pathname) == 0) && (verbose))
				printf(MSGSTR(AUTHCK_106, "Format of Defaults database OK\n"));
			break;

                default:
                        printf(
MSGSTR(AUTHCK_107, "Incorrect database specified for format check\n"));
        }
}	

static int
check_db_entries(pathname)
char *pathname;
{
        FILE *fp;
        char buffer[512];
        char * chkent_ptr;
        int chkent_found;
        int continuation = 0;
	int errors = 0;

        fp = fopen (pathname, "r");
	if (fp == 0) {
		fprintf(stderr, MSGSTR(AUTHCK_114, "%s: Can't open %s - "), command_name, pathname);
		perror("");
		return 1;
	}
        while (fgets(buffer, sizeof(buffer), fp) != (char *) 0) {
		if (buffer[0] == '#' ) 
			continue ;
		if ((buffer[0] == '\t') ||
		    (buffer[0] == ' ')  ||
		    (buffer[0] == ':')) {
                	printf(MSGSTR(AUTHCK_108, "%s has TAB, space or colon at start of entry\n"),pathname);
			errors++;
                }
                do {
                        if (buffer[0] == '\n') {
				printf(MSGSTR(AUTHCK_109, "blank line in %s\n"), pathname);
				errors++;
                        }
                        continuation = (buffer[strlen(buffer) - 2] == '\\');
                        if (continuation) {
                                fgets(buffer, sizeof(buffer), fp);
                           if ((buffer[0] != '\t') || (buffer[1] != ':')) {
                                printf( MSGSTR(AUTHCK_110, "continuation lines must begin with TAB followed by a colon\n"));
				errors++;
                           }
                        }
                } while (continuation);
                chkent_found = 0;
                chkent_ptr = (char *) qstrtok(buffer, ":");
                while ((!chkent_found) && (chkent_ptr != (char *) 0)) {
                        chkent_found = (strncmp(chkent_ptr, "chkent",
                                                strlen("chkent")) == 0);
                        chkent_ptr = (char *) qstrtok((char *) 0, ":");
		}
                if (!chkent_found) {
                        printf(
MSGSTR(AUTHCK_111, "%s has an entry without the 'chkent' terminator\n"), pathname);
			errors++;
                }
        }
	fclose(fp);
        return(errors);
}

/*
 * Check the file control database entries for consistency
 */

#define F_OWNER         1
#define F_GROUP         2
#define F_MODE          3
#define F_TYPE          4
#if SEC_ACL
#define F_ACL		5
#endif
#if SEC_MAC
#define F_SLEVEL        6
#endif
#if SEC_PRIV
#define F_PPRIVS        7
#define F_GPRIVS        8
#endif
#define F_CHKENT        9

#define FCDB_BUFMAX     MAXPATHLEN+1024

static void
check_fcdb()
{
        FILE *fp;
        char buffer[FCDB_BUFMAX];
        char *pathname;
	char *entry, *token;
	char *newline;
        int chkent_found;
	int syntax_error;
	int tokencnt;
	int linecnt = 0;
        int continuation = 0;
	int errors = 0;
	int pegnum;
#if SEC_ACL
	int acl_count;
#endif /* SEC_ACL */
#if SEC_PRIV
	privvec_t privs;
#endif


	/* Get the pathname for the file control database */

	pathname = find_auth_file((char *) 0, OT_FILE_CNTL);

	if (!pathname) {
	    printf(MSGSTR(AUTHCK_117, "Unable to locate the file control database\n"));
	    errors++;
	    return;
	}

	/* Open the file control database and parse entries */

        fp = fopen (pathname, "r");

	if (!fp) {
	    fprintf(stderr, MSGSTR(AUTHCK_114, "%s: Can't open %s - "), command_name, pathname);
	    perror("");
	    errors++;
	    return;
	}

	if (verbose)
		printf(MSGSTR(AUTHCK_115, "\nChecking all entries in the File Control database\n"));

        while (fgets(buffer, sizeof(buffer), fp) != (char *) 0) {
	    char *cptr, *rest_ptr;
	    int entry_size, skip;

	    linecnt++;
	    syntax_error = 0;
	    chkent_found = 0;

	    if ((buffer[0] == '\t') || (buffer[0] == ' ')  ||
	        (buffer[0] == ':')) {

		printf(MSGSTR(AUTHCK_118, "line # %d in %s has TAB, space or colon at start of entry\n"),
			linecnt, pathname);
		syntax_error++;
	    }

	   if (newline = strchr(buffer, '\n'))
		*newline = '\0';

	    /* Process continuation lines concatenating them in the buffer */

	    do {

		if (buffer[0] == '\n') {

                        printf(MSGSTR(AUTHCK_119, "line # %d in %s is a blank line\n"), linecnt, pathname);
			syntax_error++;
		}

		entry_size = strlen(buffer);

		continuation = (buffer[entry_size - 1] == '\\');

		if (continuation) {

		    entry_size--;
		    cptr = (char *) (buffer + entry_size);

		    fgets(cptr, sizeof(buffer) - entry_size, fp);

		    linecnt++;

		    if (cptr[0] == '\n') {


			printf(MSGSTR(AUTHCK_120, "continuation line # %d in %s is a blank line\n"),
				linecnt, pathname);
			syntax_error++;
		    }

		    /*
		     * Continuation lines either 
		     *  1) begin : (possibly preceded by whitespace) or 
		     *  2) the preceding line ended ,\
		     *     and this is a continuation of that field
		     */

		    skip = strspn(cptr, "\t ");
		    if (cptr[skip] == ':')
			    skip++;
		    else if (buffer[entry_size - 1] != ',')
			    {
				    
				    printf(
                                           MSGSTR(AUTHCK_122, "continuation line # %d in %s must either \n\ begin with [whitespace and] ':' or the preceding line must end with ',\\' \n"),
					   linecnt, pathname);
				    syntax_error++;
			    }
		    
		    rest_ptr = cptr + skip;
			    
		    /* Compress the continuation characters from line */

		    strncpy(cptr, rest_ptr, sizeof(buffer) - entry_size - skip);

		    /* Remove the line feed */

		    if (newline = strchr(buffer, '\n'))
			*newline = '\0';
		}
	    } while (continuation);

	    if (buffer[0] == '#')
		continue;

	    /* Process next entry if a syntax error was detected */

	    if (syntax_error)
		continue;

	    /* Parse the fields contained in the FCDB entry */

            entry = token = qstrtok(buffer, ":");

            if (!token || ((token = qstrtok((char *) 0, ":")) == (char *) 0)) {
		printf("line # %d in %s has a missing entry or ':' separator\n",
			linecnt, pathname);
		syntax_error++;
		goto synerr;
	    }

	    tokencnt = 1;

	    while (token) {
		int field;

		/* Classify the token into an index */

		field = 0;

		if(strncmp(token, AUTH_F_OWNER, strlen(AUTH_F_OWNER)) == 0)
			field = F_OWNER;
		else if(strncmp(token, AUTH_F_GROUP, strlen(AUTH_F_GROUP)) == 0)
			field = F_GROUP;
		else if(strncmp(token, AUTH_F_MODE, strlen(AUTH_F_MODE)) == 0)
			field = F_MODE;
		else if(strncmp(token, AUTH_F_TYPE, strlen(AUTH_F_TYPE)) == 0)
			field = F_TYPE;
#if SEC_ACL
		else if(strncmp(token, AUTH_F_ACL, strlen(AUTH_F_ACL)) == 0)
			field = F_ACL;
#endif
#if SEC_MAC
		else if(strncmp(token, AUTH_F_SLEVEL,
						strlen(AUTH_F_SLEVEL)) == 0)
			field = F_SLEVEL;
#endif
#if SEC_PRIV
		else if(strncmp(token, AUTH_F_PPRIVS,
						strlen(AUTH_F_PPRIVS)) == 0)
			field = F_PPRIVS;
		else if(strncmp(token, AUTH_F_GPRIVS,
						strlen(AUTH_F_GPRIVS)) == 0)
			field = F_GPRIVS;
#endif
		else if(strncmp(token, "chkent", strlen("chkent")) == 0)
			field = F_CHKENT;

		/* Perform a token specific syntax check */

		switch (field) {
		    char *fieldptr;

		    case F_OWNER:

			fieldptr = token + strlen(AUTH_F_OWNER);

			if (*fieldptr != '=') {
			    printf(MSGSTR(AUTHCK_124, "entry %s in %s has a syntax error in the %s field\n"),
					entry, pathname, AUTH_F_OWNER);
			    syntax_error++;
			    goto synerr;
			}

			fieldptr++;

			if (pw_nametoid(fieldptr) == (uid_t)-1) {
			    printf(MSGSTR(AUTHCK_125, "entry %s in %s has an invalid owner name '%s'\n"),
					entry, pathname, fieldptr);
			    syntax_error++;
			}
			break;

		    case F_GROUP:

			fieldptr = token + strlen(AUTH_F_GROUP);

			if (*fieldptr != '=') {
			    printf(MSGSTR(AUTHCK_124, "entry %s in %s has a syntax error in the %s field\n"),
					entry, pathname, AUTH_F_GROUP);
			    syntax_error++;
			    goto synerr;
			}

			fieldptr++;

			if (gr_nametoid(fieldptr) == (uid_t)-1) {
			    printf(MSGSTR(AUTHCK_126, "entry %s in %s has an invalid group name '%s'\n"),
					entry, pathname, fieldptr);
			    syntax_error++;
			}
			break;

		    case F_MODE:

			fieldptr = token + strlen(AUTH_F_MODE);

			if (*fieldptr != '#') {
			    printf(MSGSTR(AUTHCK_124, "entry %s in %s has a syntax error in the %s field\n"),
					entry, pathname, AUTH_F_MODE);
			    syntax_error++;
			    goto synerr;
			}

			fieldptr++;

			/* Validate that the modeword starts with leading 0 */

			if (fieldptr[0] != '0' ) {
			    printf(MSGSTR(AUTHCK_127, "entry %s in %s specifies a modeword without a leading '0'\n"),
					entry, pathname);
			    syntax_error++;
			    goto synerr;
			}

			fieldptr++;

			/* Validate that the modeword is all numeric */

			while(*fieldptr) {

				if (!isdigit(*fieldptr) || fieldptr[0] == '8' || fieldptr[0] == '9') {
				    printf(MSGSTR(AUTHCK_128, "entry %s in %s has an invalid modeword character\n"),
						entry, pathname);
				    syntax_error++;
				    goto synerr;
				}
				fieldptr++;
			}

			break;

		    case F_TYPE:

			fieldptr = token + strlen(AUTH_F_TYPE);
			if (*fieldptr != '=') {
			    printf(MSGSTR(AUTHCK_124, "entry %s in %s has a syntax error in the %s field\n"),
					entry, pathname, AUTH_F_TYPE);
			    syntax_error++;
			    goto synerr;
			}

			fieldptr++;

			if (fieldptr[1] != '\0' ||
			   (fieldptr[0] != 'r' &&
			    fieldptr[0] != 'd' &&
			    fieldptr[0] != 'b' &&
			    fieldptr[0] != 'c' &&
#if SEC_MAC
			    fieldptr[0] != 'm' &&
#endif
			    fieldptr[0] != 'f' &&
			    fieldptr[0] != 's')
			   ) {

			    printf(MSGSTR(AUTHCK_129, "entry %s in %s has invalid f_type specification '%s'\n"),
					entry, pathname, fieldptr);
			    syntax_error++;
			    goto synerr;
			}

			break;

#if SEC_ACL
		    case F_ACL:

			acl_init();

			fieldptr = token + strlen(AUTH_F_ACL);
			if (*fieldptr != '=') {
			    printf(MSGSTR(AUTHCK_124, "entry %s in %s has a syntax error in the %s field\n"),
					entry, pathname, AUTH_F_ACL);
			    syntax_error++;
			    goto synerr;
			}

			fieldptr++;

			if ((strcmp(fieldptr, AUTH_F_WILD) != 0) &&
			    (!acl_er_to_ir(fieldptr, &acl_count))) {
			    printf(MSGSTR(AUTHCK_130, "entry %s in %s contains an invalid ACL specification\n"),
					entry, pathname);
			    syntax_error++;
			    goto synerr;
			}

			break;
#endif
#if SEC_MAC
		    case F_SLEVEL:

			fieldptr = token + strlen(AUTH_F_SLEVEL);
			if (*fieldptr != '=') {
			    printf(MSGSTR(AUTHCK_124, "entry %s in %s has a syntax error in the %s field\n"),
					entry, pathname, AUTH_F_SLEVEL);
			    syntax_error++;
			    goto synerr;
			}

			fieldptr++;

			/* Must be one of the well-defined strings */

			if ((strcmp(fieldptr, AUTH_F_SYSLO) != 0) &&
			    (strcmp(fieldptr, AUTH_F_SYSHI) != 0) &&
			    (strcmp(fieldptr, AUTH_F_WILD) != 0)) {
			    printf(MSGSTR(AUTHCK_131, "entry %s in %s contains invalid sensitivity level specification\n"),
					entry, pathname);

			    syntax_error++;
			}

			break;
#endif
#if SEC_PRIV
		    case F_PPRIVS:
		    case F_GPRIVS:
			fieldptr = token + (field == F_PPRIVS ?
				strlen(AUTH_F_PPRIVS) : strlen(AUTH_F_GPRIVS));

			if (*fieldptr != '=') {
			    printf(MSGSTR(AUTHCK_132, "entry %s in %s is missing the '=' separator for the %s privileges field\n"),
					entry, pathname,
					field == F_PPRIVS ? "potential" :
							    "granted");
			    syntax_error++;
			    goto synerr;
			}

			fieldptr++;

			if (strtoprivs(fieldptr, (char *) 0, privs) != -1) {
			    printf(MSGSTR(AUTHCK_133, "entry %s in %s contains invalid %s privilege name(s)\n"),
					entry, pathname,
					field == F_PPRIVS ? "potential" :
							    "granted");
			    syntax_error++;
			}
			break;
#endif

		    case F_CHKENT:

			/* If the line contains only chkent, then error */

			if (tokencnt == 1) {
			    printf(MSGSTR(AUTHCK_134, "line # %d in %s contains misplaced 'chkent'\n"),
					linecnt, pathname);
			    syntax_error++;
			    goto synerr;
			}

			chkent_found = 1;
			break;

		    default:

			printf(MSGSTR(AUTHCK_135, "entry %s in %s contains invalid field '%s'\n"),
				entry, pathname, token);
			syntax_error++;
			goto synerr;
		}


		/* Bump the pointer to the next token */

                token = qstrtok((char *) 0, ":");
		tokencnt++;
	    }

	    /* Make sure the current entry is terminated by a chkent */

	    if (!chkent_found) {
		printf(MSGSTR(AUTHCK_136, "entry %s in %s is missing 'chkent' terminator\n"),
			entry, pathname);

		syntax_error++;
	    }

synerr:
	    if (syntax_error)
		errors++;
        }

	fclose(fp);
	
	if ((!errors) && (verbose))
		printf(MSGSTR(AUTHCK_116, "Format of File Control database OK\n"));
	if (errors)
		total_errors++;

        return;
}

/*
 * Get next token from string s (NULL on 2nd, 3rd, etc. calls),
 * where tokens are nonempty strings separated by runs of
 * chars from delim.  Writes NULs into s to end tokens.  delim need not
 * remain constant from call to call.
 * This version has been hacked to understand \-quoting.
 * Originally from Henry Spencer's public-domain posting to mod.sources of
 * 8 July 1986.
 */

char *				/* NULL if no token left */
qstrtok(s, delim)
char *s;
register const char *delim;
{
	register char *scan;
	char *tok;
	register const char *dscan;

	static char *scanpoint;

	if (s == NULL && scanpoint == NULL)
		return(NULL);
	if (s != NULL)
		scan = s;
	else
		scan = scanpoint;

	/*
	 * Scan leading delimiters.
	 */
	for (; *scan != '\0'; scan++) {
		for (dscan = delim; *dscan != '\0'; dscan++)
			if (*scan == *dscan)
				break;
		if (*dscan == '\0')
			break;
	}
	if (*scan == '\0') {
		scanpoint = NULL;
		return(NULL);
	}

	tok = scan;

	/*
	 * Scan token.
	 */
	for (; *scan != '\0'; scan++) {
		if (*scan == '\\' && scan[1]) {
			scan++;
			continue;
		}
		for (dscan = delim; *dscan != '\0';)	/* ++ moved down. */
			if (*scan == *dscan++) {
				scanpoint = scan+1;
				*scan = '\0';
				return(tok);
			}
	}

	/*
	 * Reached end of string.
	 */
	scanpoint = NULL;
	return(tok);
}

#endif /*} SEC_BASE */
