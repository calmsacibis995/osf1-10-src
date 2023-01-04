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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: id.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 16:59:16 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *  id.c   1.16  com/cmd/s/auth/id.c, cmdsauth, bos320, 9128320 6/24/91 16:39:48
 */


#include <stdio.h>
#include <pwd.h>
#include <locale.h>
#include <grp.h>
#include "id_msg.h"
#define MSGSTR(num,str) catgets(catd,MS_id,num,str)


/*  Function prototyping */
void	usage();
int	idusername(char *);
void	idfastpath();
int	idfastusername();
int	idinvoker();

int	Gflag = 0;
int	gflag = 0;
int	uflag = 0;
int	nflag = 0;
int	rflag = 0;
char	*user = NULL;
nl_catd	catd;


/*
 * NAME:        id
 *
 * SYNTAX:	id [user]
 *		id -G [-n] [user]
 *		id -g [-nr] [user]
 *		id -u [-nr] [user]
 *
 * FUNCTION:    id - display system identity of user
 *
 * NOTES:       Id displays the user and group ids (and corresponding
 *              names) of the invoking process.  If effective user and
 *              group ids do not match their corresponding ids, they
 *              are printed also.
 *
 * RETURN VALUE DESCRIPTION:    0 upon success, 1 on failure
 */

int
main(int argc, char **argv)
{
	int		c;
	extern int	optind;
	extern int	opterr;

        (void) setlocale (LC_ALL, "");  /* set up message catalog functions */
        catd = catopen(MF_ID, NL_CAT_LOCALE);

	if (argc == 1) {	     	/* run id command as before */
		idfastpath();
		return(0);
	}
		
	opterr = 0;			/* suppress getopt error messages */

	/*
	 * check all command line flags
	 * for valid/invalid combinations
	 */

	while ((c = getopt(argc, argv, "Ggunr")) != EOF)
		switch (c)
		{
		    case 'G':
			Gflag++;
			break;
		    case 'g':
			gflag++;
			break;
		    case 'u':
			uflag++;
			break;
		    case 'n':
			nflag++;
			break;
		    case 'r':
			rflag++;
			break;
		    default:
			usage();
		}

	switch (argc - optind)  	/*  check for a [user] argument  */
	{
	    case 1:
		user = argv[optind];	/*  set [user] to last argument */ 
		break;
	    case 0:			/*  if no user, go on through   */
		break;
	    default:
		usage();
	}

	/*
	 *  Check all of the flags for invalid combinations
	 */

	if ((Gflag + gflag + uflag) > 1) 
		usage();

	if (!Gflag && !gflag && !uflag && (rflag || nflag))
		usage();

	if (Gflag && rflag)
		usage();

	if (user)
		return (idusername(user));	/*  run id for [user]	*/

	idinvoker();		/* run id for invoking process */
	catclose(catd);		/* close message catalogs */
	return(0);
}

/*
 * NAME:        usage
 *
 * FUNCTION:    print the usage of the id command
 *
 * RETURN VALUE DESCRIPTION:    none
 */

void
usage()
{
	fprintf(stderr,MSGSTR(M_USE1, "usage: id [user]\n"));
	fprintf(stderr,MSGSTR(M_USE2, "       id  -G [-n] [user]\n"));
	fprintf(stderr,MSGSTR(M_USE3, "       id  -g [-nr] [user]\n"));
	fprintf(stderr,MSGSTR(M_USE4, "       id  -u [-nr] [user]\n"));
	exit(1);
}

/*
 * NAME:        idusername
 *
 * FUNCTION:    print the id information of a specified user
 *
 * RETURN VALUE DESCRIPTION:    0
 */

int
idusername(char *user)
{
	register struct passwd *pw;
	register struct group *gr;
	char *grattr;
	char *sep = "";
	char **cp;


	/*
	 *  Check for no flags, just [user] name
	 */

	if ((Gflag + gflag + uflag) == 0)
		return(idfastusername(user));

	/*
	 *  Make sure this is a valid user, and put info into
	 *  	pw structure.  If valid user, print the user's 
	 *	infomation according to the flags given
	 */

	if ((pw = getpwnam(user)) == NULL) {
		fprintf(stderr,MSGSTR(M_BADUSER,
			"User not found in /etc/passwd file\n"));
		return(1);
	}

	if (uflag) {
		if (nflag) 
			printf("%s\n",pw->pw_name);
		else
			printf("%u\n",pw->pw_uid);
		return(0);
	}
	if (gflag) {

		/*
		 *  Get user's gid information into gr structure
		 *  	and print the user's group information
		 *	according to the flags given
		 */

		if ((gr = getgrgid(pw->pw_gid)) == NULL) {
			fprintf(stderr,MSGSTR(M_NOGROUP,
				"Could not get \"group\" information\n"));
			return(1);
		}
		if (nflag) 
			printf("%s\n",gr->gr_name);
		else
			printf("%u\n",pw->pw_gid);
		return(0);
	}
	if (Gflag) {

		/*
		 * open group database and search for user's groups
		 */
		
		if ((gr = getgrent()) == NULL) {
			fprintf(stderr,MSGSTR(M_NOGROUP,
			     "Could not get \"group\" information\n"));
			return(1);
		}
		setgrent();
		while (gr = getgrent()) {
			if (pw->pw_gid == gr->gr_gid) {
				if (nflag)
					printf("%s%s", sep, gr->gr_name);
				else
					printf("%s%d", sep, gr->gr_gid);
				sep = " ";
				continue;
			}	
			for (cp = gr->gr_mem; cp && *cp; cp++) {
				if (strcmp(*cp, user) == 0) {
					if (nflag)
					    printf("%s%s", sep, gr->gr_name);
					else
					    printf("%s%d", sep, gr->gr_gid);
					sep = " ";
					break;
				} 
			}
		}
		endgrent();	/*  close group database  */
	printf("\n");
	return(0);
	}
}
		
/*
 * NAME:        idinvoker
 *
 * FUNCTION:    print the id information of the invoker
 *
 * RETURN VALUE DESCRIPTION:    0
 */

int
idinvoker()
{
	register struct passwd *pw;
	register struct group *gr;
	int ngroups, i;
	gid_t	groups[NGROUPS_MAX];
	char *sep = "";

	/*
	 *  Get the invoker's REAL and EFFECTIVE id's according to the
	 *  	flags given.  Get the user information into the pw
	 *	structure and the group information into the gr 
	 *	structure and output the information according to the
	 *	flags given
	 */

	if (uflag) {
		uid_t  uid;

		if (rflag)
			uid = getuid();
		else
			uid = geteuid();
		if (nflag) {
			if ((pw = getpwuid(uid)) == NULL) {
				fprintf(stderr,MSGSTR(M_NOUSER,
				     "Could not get \"user\" information\n"));
				return(1);
			}
			printf("%s\n",pw->pw_name);
			return(0);
		}
		printf("%u\n",uid);
		return(0);
	}
	if (gflag) {
		gid_t  gid;

		if (rflag) 
			gid = getgid();
		else
			gid = getegid();
		if (nflag) {
			if ((gr = getgrgid(gid)) == NULL) {
				fprintf(stderr,MSGSTR(M_NOGROUP,
				     "Could not get \"group\" information\n"));
				return(1);
			}
			printf("%s\n",gr->gr_name);
			return(0);
		}
		else {
			printf("%u\n",gid);
			return(0);
		}
	}
	if (Gflag) {

		int yflag = 0;
		gid_t rgid,egid;

					/*  Determine if the process has   */
		rgid = getgid();	/*  an effective id different than */
		egid = getegid();	/*  the the real.                  */
		if (rgid != egid)
			yflag++;

		ngroups = getgroups(NGROUPS_MAX, groups);
		for (i = 0; i < ngroups; i++) {
			if ((gr = getgrgid(groups[i])) == NULL) {
				printf("%s%u", sep, groups[i]);
				sep = " ";
			} else if (nflag) {
				printf("%s%s", sep, gr->gr_name);
				sep = " ";
			} else {
			    	printf("%s%u", sep, gr->gr_gid);
				sep = " ";
			}
		}
		if (yflag) {
			if (nflag) {
				if ((gr = getgrgid(egid)) != NULL)
					printf("%s%s", sep, gr->gr_name);
				else
					printf("%s%u", sep, egid);
			}
			else
				printf("%s%u", sep, egid);
		}
		printf("\n");
		return(0);
	}
}
		
/*
 * NAME:        idfastpath
 *
 * FUNCTION:    run the id command with no other arguments
 *
 * RETURN VALUE DESCRIPTION:    none
 */

void
idfastpath()
{
	uid_t 	ruid,euid;	
	gid_t 	rgid,egid;	
	gid_t	groups[NGROUPS_MAX];
	int 	ngroups, i;
	char	*sep = MSGSTR(M_GROUPS, " groups=");
	register struct passwd *pw;
	register struct group *gr;
	
	/*
	 *  Run the id command just as before, with the user just
	 *  entering the normal command "id" <ENTER>
	 */
	
	ruid = getuid();
	euid = geteuid();
	rgid = getgid();
	egid = getegid();
	printf(MSGSTR(M_SUID, "uid=%u"), ruid);
	if ((pw = getpwuid(ruid)) != NULL)
		printf ("(%s)", pw->pw_name);
	printf(MSGSTR(M_SGID, " gid=%u"), rgid);
	if ((gr = getgrgid(rgid)) != NULL)
		printf ("(%s)", gr->gr_name);
	if (ruid != euid) {
		printf(MSGSTR(M_SEUID, " euid=%u"), euid);
		if ((pw = getpwuid(euid)) != NULL) {
			printf ("(%s)", pw->pw_name);
			pw = getpwuid(ruid); 	   /*  set pw back to real  */
		}
	}
	if (rgid != egid) {
		printf(MSGSTR(M_SEGID, " egid=%u"), egid);
		if ((gr = getgrgid(egid)) != NULL)
			printf ("(%s)", gr->gr_name);
	}
	if ((ngroups = getgroups(NGROUPS_MAX, groups)) != 1) {
		for (i = 0; i < ngroups; i++) {
			if ((gr = getgrgid(groups[i])) != NULL) {
				if (pw->pw_gid != gr->gr_gid) {
					printf("%s%u(%s)", sep, groups[i], gr->gr_name);
					sep = ",";
				}
			}
		}
	}
	putchar('\n');
}

/*
 * NAME:        idfastusername
 *
 * FUNCTION:    print the id information of a user with no other arguments
 *
 * RETURN VALUE DESCRIPTION:    0 on success, 1 on failure
 */

int
idfastusername(char *user)
{
	register struct passwd *pw;
	register struct group *gr;
	char	*sep = MSGSTR(M_GROUPS, " groups=");
	char	**cp;
	
	
	/*
	 *  Make sure this is a valid user, and put info into
	 *  	pw structure.  If valid user, print the user's 
	 *	infomation according to the flags given
	 */

	if ((pw = getpwnam(user)) == NULL) {
		fprintf(stderr,MSGSTR(M_BADUSER,
			"User not found in /etc/passwd file\n"));
		return(1);
	}

	/*
	 *  Run the id command with one argument such as
	 *	"id [username]" <ENTER>
	 */

	gr = getgrgid(pw->pw_gid);
	printf(MSGSTR(M_SUID, "uid=%u"), pw->pw_uid);
	printf ("(%s)", pw->pw_name);
	printf(MSGSTR(M_SGID, " gid=%u"), pw->pw_gid);
	if (gr != NULL)
		printf ("(%s)", gr->gr_name);
	setgrent();
	while (gr = getgrent()) {    /* search group file for user */
		if (pw->pw_gid == gr->gr_gid) {
			continue;
		}	
		for (cp = gr->gr_mem; cp && *cp; cp++) {
			if (strcmp(*cp, user) == 0) {
			       printf("%s%u(%s)", sep, gr->gr_gid, gr->gr_name);
			       sep = ",";
			       break;
			} 
		}
	}
	endgrent();
	putchar('\n');
	return(0);
}

