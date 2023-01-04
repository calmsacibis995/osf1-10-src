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
static char	*sccsid = "@(#)$RCSfile: sysconfigdb.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:04:59 $";
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

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <locale.h>

#include "cm.h"

#define	TMPEXT			"tmp"
#define	EQ(a,b)			(strcmp(a,b) == 0)

#define	NOSPEC		0000
#define	ADD		0001
#define	DELETE		0002
#define	UPDATE		0004
#define	LIST		0010
#define	LISTALL		0020
#define	AUTO		0040
#define	MULTI		0100

/*
 *      Common Global data
 */
typedef struct {
	AFILE_t 	af_data;
	AFILE_t 	af_file;
	FILE *		fp_temp;
	char *		progname;
	char *		database;
	char *		filename;
	char *		tempname;
	char *		entryname;
	int		vflg;
	int		command;
	int		autoflg;
	int		multiflg;
	int		maxrec;
	int		maxatr;
} sysconfigdb_common_t;
sysconfigdb_common_t	CD;


/*
 *      usage
 */
void
usage()
{
    	fprintf(stderr, cm_msg(MSG_SYSCONFIGDB), CD.progname);
        exit(1);
}


void
options(argc, argv)
        int     argc;
        char ** argv;
{
        register int    c;
        extern char *   optarg;
        extern int      optind;

        CD.progname = strrchr(argv[0],'/') ? strrchr(argv[0],'/') + 1: argv[0];
	CD.command	= NOSPEC;
	CD.vflg 	= FALSE;
	CD.autoflg	= FALSE;
	CD.multiflg	= FALSE;
	CD.database 	= CFGMGR_DATABASE;
	CD.maxrec 	= CFGMGR_MAXRECSIZ;
	CD.maxatr 	= CFGMGR_MAXATRNUM;
	CD.filename 	= NULL;
	CD.tempname 	= NULL;
	CD.fp_temp		= NULL;
	CD.af_data		= NULL;
	CD.af_file		= NULL;
	CD.entryname 	= NULL;

        while ((c = getopt(argc, argv, "vc:f:o:adlpu")) != EOF) {
                switch (c) {
                case 'v':
                        CD.vflg = TRUE;
			break;
                case 'c':
                        CD.database = optarg;
			break;
                case 'f':
                        CD.filename = optarg;
			break;
                case 'o':
			if (!strcmp(optarg,"n")) { 		/* -on */
				CD.command |= AUTO;
				if (CD.command & (DELETE))
					usage();
				CD.autoflg = TRUE;
			} else if (!strcmp(optarg,"ff")) {	/* -off */
				CD.command |= AUTO;
				CD.autoflg = FALSE;
			} else if (!strcmp(optarg,"nm")) { 	/* -onm */
				CD.command |= MULTI;
				if (CD.command & (DELETE))
					usage();
				CD.multiflg = TRUE;
			} else if (!strcmp(optarg,"ffm")) {	/* -offm */
				CD.command |= MULTI;
				CD.multiflg = FALSE;
			} else
				usage();
			break;
                case 'a':
			if (CD.command & (DELETE|UPDATE))
				usage();
			CD.command |= ADD;
			break;
                case 'd':
			if (CD.command & (ADD|UPDATE|LIST))
				usage();
			CD.command |= ( DELETE | AUTO | MULTI );
			CD.autoflg = FALSE;
			break;
                case 'l':
			if (CD.command & (DELETE))
				usage();
			CD.command |= LIST;
			break;
                case 'p':
			CD.command |= LISTALL;
			break;
                case 'u':
			if (CD.command & (ADD|DELETE))
				usage();
			CD.command |= UPDATE;
			break;
                case '?':
                default:
                        usage();
			break;
                }
        }
	if (CD.command == NOSPEC)
		usage();

	if (CD.command & ADD || CD.command & UPDATE ||  CD.command & AUTO
	|| CD.command & MULTI || CD.command & LIST  || CD.command & DELETE) {
		if (optind+1 != argc)
			usage();
		CD.entryname = argv[optind++];
	}

	if (optind != argc)
		usage();

	if (CD.command & LIST && CD.command & LISTALL)
		CD.command &= ~LIST;
}


void
showerrs( AFILE_t af )
{
    if (af->AF_errs & AF_ERRCATR)
		fprintf(stderr, cm_msg(MSG_AF_ERRCATR), CD.progname);
    if (af->AF_errs & AF_ERRCBUF)
		fprintf(stderr, cm_msg(MSG_AF_ERRCBUF), CD.progname);
    if (af->AF_errs & AF_SYNTAX)
		fprintf(stderr, cm_msg(MSG_AF_SYNTAX), CD.progname);
}

void
write_attrval(FILE * fp, ATTR_t attr)
{
        char *  val;
	int	cnt =0;

        while ((val=AFnxtval(attr)) != NULL) {
		if (cnt++)
			fprintf(fp, ",");
		fprintf(fp, " %s", val);
	}
	fprintf(fp, "\n");
}

void
write_attrname(FILE * fp, ATTR_t attr)
{
	fprintf(fp, "\t%s =", AFatrname(attr));
}

void
write_attr( FILE * fp, ATTR_t attr )
{
	write_attrname(fp, attr);
	write_attrval(fp, attr);
}

void
write_entname( FILE * fp, ENT_t entry )
{
	fprintf(fp, "%s:\n", AFentname(entry));
}

void
write_ent( FILE * fp, ENT_t entry )
{
	ATTR_t	attr;

	write_entname(fp, entry);
	while ( (attr=AFnxtatr(entry)) != NULL )
		write_attr(fp, attr);
	fprintf(fp, "\n");
}


/*
 *	Database  file
 */
AFILE_t
afopen_data()
{
	struct stat	statbuf;
	int		fd;

	if (stat(CD.database, &statbuf)) {
		if (CD.command & (ADD|UPDATE) && errno == ENOENT) {
			if ((fd = creat(CD.database, 0640)) < 0) {
				perror(CD.database);
				exit(2);
			}
			close(fd);
		}
	}

	if (CD.database == NULL
	|| (CD.af_data=AFopen(CD.database, CD.maxrec, CD.maxatr)) == NULL) {
		perror(CD.database);
		exit(2);
	}
	AFsetdflt(CD.af_data, NULL);
	return(CD.af_data);
}

void
afclose_data()
{
	if (CD.af_data)
		AFclose(CD.af_data);
}


/*
 *	Input file
 */
AFILE_t
afopen_file()
{
	if (CD.filename == NULL
	|| (CD.af_file=AFopen(CD.filename, CD.maxrec, CD.maxatr)) == NULL) {
		perror(CD.filename);
		exit(2);
	}
	AFsetdflt(CD.af_file, NULL);
	return(CD.af_file);
}

void
afclose_file()
{
	if (CD.af_file)
		AFclose(CD.af_file);
}


/*
 *	Temporary file
 */
FILE *
fopen_temp()
{
	FILE *temp_fd;
	char buf[256];

	if ((CD.tempname = (char *)malloc(strlen(CD.database)
		+strlen(TMPEXT) +1)) == NULL)
		exit(2);
	strcpy(CD.tempname, CD.database);
	strcat(CD.tempname, TMPEXT);
	if (CD.tempname == NULL ||(CD.fp_temp=fopen(CD.tempname,"w")) == NULL) {
		perror(CD.tempname);
		exit(2);
	}

	/*	OSF 1.1 bug fix 
	 * Make sure that the beginning comments (Copyright, revision
	 * history, etc) are copied over into the temp copy of the database
	 * (default is /etc/sysconfigtab).  The temp copy eventually becomes
	 * the new copy.   Although this will move all comments to the
	 * beginning of the database, it appears to be the only way since
	 * AFread() is used to read the database.  AFread() reads the next
	 * Attribute File entry so it skips over all comments.  Chances are
	 * there won't be embedded comments but just let it be known....
	 */

	temp_fd = fopen(CD.database, "r");
	while ((fgets(buf, sizeof(buf), temp_fd) != NULL) && buf[0] == '#')
		fputs(buf, CD.fp_temp);
	fprintf(CD.fp_temp, "\n");
	fclose(temp_fd);
	/* end of OSF 1.1 bug fix */

	return(CD.fp_temp);
}

void
fclose_temp()
{
	if (CD.fp_temp)
		fclose(CD.fp_temp);
}

void
unlink_temp()
{
	fclose_temp();
	unlink(CD.tempname);
}


void
rename_temp()
{
	if (rename(CD.tempname, CD.database) != 0) {
		unlink_temp();
		fprintf(stderr, cm_msg(MSG_AF_RENAME), CD.database,
			CD.tempname, strerror(errno));
		exit(2);
	}
}

int
auto_add( )
{
	ENT_t	entry;
	ATTR_t	attr;
	char *	val;
	int	cnt = 0;
	int	foundent = FALSE;
	int	foundatr = FALSE;
	int	foundval = FALSE;
	int	abort = FALSE;
	FILE *	fp;
	AFILE_t	afd;

	afd = afopen_data();
	/*
	 *	Check for automatic "target" entry
	 */
	if ((entry=AFgetent(afd, CD.entryname)) == NULL) {
		fprintf(stderr, cm_msg(MSG_AF_ENOEXIST), CD.entryname,
			CD.database);
		afclose_data();
		return(1);
	}

	fp = fopen_temp();
	/*
	 *	Check for "automatic" entry
	 *	If not found, output it and continue
	 */
	AFrewind(afd);
	if ((entry=AFgetent(afd, AUTOENTRY)) == NULL) {
		fprintf(fp, "%s:\n\t%s = %s\n\n", AUTOENTRY, 
			AUTO_DYNAMIC, CD.entryname);
		foundent = TRUE;
	}

	AFrewind(afd);
	while (!abort && (entry=AFnxtent(afd)) != NULL) {
		if (foundent || !EQ(AFentname(entry), AUTOENTRY)) {
			write_ent(fp, entry);
			continue;
		}
		foundent = TRUE;
		write_entname(fp, entry);
		while (!abort && (attr=AFnxtatr(entry)) != NULL) {
			if (!EQ(AFatrname(attr), AUTO_DYNAMIC)) {
				write_attr(fp, attr);
				continue;
			}
			foundatr = TRUE;
			write_attrname(fp, attr);
			while ((val=AFnxtval(attr)) != NULL) {
				if (EQ(val, CD.entryname)) {
					foundval = TRUE;
					abort = TRUE;
					break;
				}
				if (EQ(val, KEYWORD_NONE))
					continue;
				if (cnt++)
					fprintf(fp, ",");
				fprintf(fp, " %s", val);
			}
			if (foundval == FALSE) {
				if (cnt++) fprintf(fp, ",");
				fprintf(fp, " %s", CD.entryname);
			}
			fprintf(fp, "\n");
		}
		if (foundatr == FALSE)
			fprintf(fp, "\t%s = %s\n", AUTO_DYNAMIC, CD.entryname);
		fprintf(fp, "\n");
	}
	afclose_data();
	fclose_temp();

	if (foundent == FALSE) {
		fprintf(stderr, cm_msg(MSG_AF_INTERNAL), CD.entryname);
		return(1);
	} 
	if (foundval == TRUE) {
		fprintf(stderr, cm_msg(MSG_AF_AUTOALREADY), CD.entryname);
		return(1);
	} 
	rename_temp();
	return(0);
}

int
auto_del( )
{
	ENT_t	entry;
	ATTR_t	attr;
	char *	val;
	int	cnt = 0;
	int	foundent = FALSE;
	int	foundatr = FALSE;
	int	foundval = FALSE;
	int	abort = FALSE;
	FILE *	fp;
	AFILE_t	afd;

	afd = afopen_data();
	fp = fopen_temp();
	while ((entry=AFnxtent(afd)) != NULL) {
		if (!EQ(AFentname(entry), AUTOENTRY)) {
			write_ent(fp, entry);
			continue;
		}
		foundent = TRUE;
		write_entname(fp, entry);
		while ((attr=AFnxtatr(entry)) != NULL) {
			if (!EQ(AFatrname(attr), AUTO_DYNAMIC)) {
				write_attr(fp, attr);
				continue;
			}
			foundatr = TRUE;
			write_attrname(fp, attr);
			while ((val=AFnxtval(attr)) != NULL) {
				if (EQ(val, CD.entryname)) {
					foundval = TRUE;
					continue;
				}
				if (cnt++)
					fprintf(fp, ",");
				fprintf(fp, " %s", val);
			}
			if (cnt == 0)
				fprintf(fp, " %s", KEYWORD_NONE);
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");
	}
	afclose_data();
	fclose_temp();
	if (foundval == FALSE && ! (CD.command & DELETE)) {
		fprintf(stderr, cm_msg(MSG_AF_AUTONOTALREADY), CD.entryname);
		unlink_temp();
		return(1);
	}
	rename_temp();
	return(0);
}

int
multi_add( )
{
	ENT_t	entry;
	ATTR_t	attr;
	char *	val;
	int	cnt = 0;
	int	foundent = FALSE;
	int	foundatr = FALSE;
	int	foundval = FALSE;
	int	abort = FALSE;
	FILE *	fp;
	AFILE_t	afd;

	afd = afopen_data();
	/*
	 *	Check for automatic "target" entry
	 */
	if ((entry=AFgetent(afd, CD.entryname)) == NULL) {
		fprintf(stderr, cm_msg(MSG_AF_ENOEXIST), CD.entryname,
			CD.database);
		afclose_data();
		return(1);
	}

	fp = fopen_temp();
	/*
	 *	Check for "multiuser" entry
	 *	If not found, output it and continue
	 */
	AFrewind(afd);
	if ((entry=AFgetent(afd, MULTIUSER)) == NULL) {
		fprintf(fp, "%s:\n\t%s = %s\n\n", MULTIUSER, 
			AUTO_DYNAMIC, CD.entryname);
		foundent = TRUE;
	}

	AFrewind(afd);
	while (!abort && (entry=AFnxtent(afd)) != NULL) {
		if (foundent || !EQ(AFentname(entry), MULTIUSER)) {
			write_ent(fp, entry);
			continue;
		}
		foundent = TRUE;
		write_entname(fp, entry);
		while (!abort && (attr=AFnxtatr(entry)) != NULL) {
			if (!EQ(AFatrname(attr), AUTO_DYNAMIC)) {
				write_attr(fp, attr);
				continue;
			}
			foundatr = TRUE;
			write_attrname(fp, attr);
			while ((val=AFnxtval(attr)) != NULL) {
				if (EQ(val, CD.entryname)) {
					foundval = TRUE;
					abort = TRUE;
					break;
				}
				if (EQ(val, KEYWORD_NONE))
					continue;
				if (cnt++)
					fprintf(fp, ",");
				fprintf(fp, " %s", val);
			}
			if (foundval == FALSE) {
				if (cnt++) fprintf(fp, ",");
				fprintf(fp, " %s", CD.entryname);
			}
			fprintf(fp, "\n");
		}
		if (foundatr == FALSE)
			fprintf(fp, "\t%s = %s\n", AUTO_DYNAMIC, CD.entryname);
		fprintf(fp, "\n");
	}
	afclose_data();
	fclose_temp();

	if (foundent == FALSE) {
		fprintf(stderr, cm_msg(MSG_AF_INTERNAL), CD.entryname);
		return(1);
	} 
	if (foundval == TRUE) {
		fprintf(stderr, cm_msg(MSG_AF_MULTIALREADY), CD.entryname);
		return(1);
	} 
	rename_temp();
	return(0);
}

int
multi_del( )
{
	ENT_t	entry;
	ATTR_t	attr;
	char *	val;
	int	cnt = 0;
	int	foundent = FALSE;
	int	foundatr = FALSE;
	int	foundval = FALSE;
	int	abort = FALSE;
	FILE *	fp;
	AFILE_t	afd;

	afd = afopen_data();
	fp = fopen_temp();
	while ((entry=AFnxtent(afd)) != NULL) {
		if (!EQ(AFentname(entry), MULTIUSER)) {
			write_ent(fp, entry);
			continue;
		}
		foundent = TRUE;
		write_entname(fp, entry);
		while ((attr=AFnxtatr(entry)) != NULL) {
			if (!EQ(AFatrname(attr), AUTO_DYNAMIC)) {
				write_attr(fp, attr);
				continue;
			}
			foundatr = TRUE;
			write_attrname(fp, attr);
			while ((val=AFnxtval(attr)) != NULL) {
				if (EQ(val, CD.entryname)) {
					foundval = TRUE;
					continue;
				}
				if (cnt++)
					fprintf(fp, ",");
				fprintf(fp, " %s", val);
			}
			if (cnt == 0)
				fprintf(fp, " %s", KEYWORD_NONE);
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");
	}
	afclose_data();
	fclose_temp();
	if (foundval == FALSE && ! (CD.command & DELETE)) {
		fprintf(stderr, cm_msg(MSG_AF_MULTINOTALREADY), CD.entryname);
		unlink_temp();
		return(1);
	}
	rename_temp();
	return(0);
}


int
update( )
{
	ENT_t	entry;
	ENT_t	newentry;
	int	foundent  = FALSE;
	FILE *	fp;
	AFILE_t	aff;
	AFILE_t	afd;

	aff = afopen_file();
	if ((newentry=AFgetent(aff, CD.entryname)) == NULL) {
		fprintf(stderr, cm_msg(MSG_AF_ENOEXIST), CD.entryname,
			CD.filename);
		afclose_file();
		return(1);
	}
	afd = afopen_data();
	fp = fopen_temp();
	while ((entry=AFnxtent(afd)) != NULL) {
		if (EQ(AFentname(entry), CD.entryname)) {
			foundent = TRUE;
			write_ent(fp, newentry);
		} else
			write_ent(fp, entry);
	}
	if (foundent == FALSE)
		write_ent(fp, newentry);
	afclose_file();
	afclose_data();
	fclose_temp();
	rename_temp();
	return(0);
}

int
add( )
{
	ENT_t	entry;
	ENT_t	newentry;
	int	foundent  = FALSE;
	FILE *	fp;
	AFILE_t	aff;
	AFILE_t	afd;

	aff = afopen_file();
	if ((newentry=AFgetent(aff, CD.entryname)) == NULL) {
		fprintf(stderr, cm_msg(MSG_AF_ENOEXIST), CD.entryname,
			CD.filename);
		afclose_file();
		return(1);
	}

	afd = afopen_data();
	fp = fopen_temp();
	while ((entry=AFnxtent(afd)) != NULL) {
		if (EQ(AFentname(entry), CD.entryname)) {
			foundent = TRUE;
			break;
		}
		write_ent(fp, entry);
	}
	if (foundent == TRUE) {
		    fprintf(stderr, cm_msg(MSG_AF_EEXIST), CD.entryname,
			    CD.database);
		    afclose_file();
		    afclose_data();
		    fclose_temp();
		    unlink_temp();
		    return(1);
	}
	write_ent(fp, newentry);
	afclose_file();
	afclose_data();
	fclose_temp();
	rename_temp();
	return(0);
}


int
del( )
{
	ENT_t	entry;
	int	foundent  = FALSE;
	FILE *	fp;
	AFILE_t	afd;

	afd = afopen_data();
	fp = fopen_temp();

	while ((entry=AFnxtent(afd)) != NULL) {
		if (EQ(AFentname(entry), CD.entryname)) {
			foundent = TRUE;
			continue;
		}
		write_ent(fp, entry);
	}
	if (foundent == FALSE) {
		fprintf(stderr, cm_msg(MSG_AF_ENOEXIST), CD.entryname,
			CD.database);
		afclose_data();
		fclose_temp();
		unlink_temp();
		return(1);
	}
	afclose_data();
	fclose_temp();
	rename_temp();
	return(0);
}


int
list_one( )
{
	ENT_t	entry;
	int	foundent  = FALSE;
	AFILE_t	afd;

	afd = afopen_data();
	while ((entry=AFnxtent(afd)) != NULL) {
		if (EQ(AFentname(entry), CD.entryname)) {
			foundent = TRUE;
			write_ent(stdout, entry);
		}
	}
	afclose_data();
	if (foundent == FALSE) {
		fprintf(stderr, cm_msg(MSG_AF_ENOEXIST), CD.entryname,
			CD.database);
		return(1);
	}
	return(0);
}


int
list_all( )
{
	ENT_t	entry;
	AFILE_t	afd;

	afd = afopen_data();
	while ((entry=AFnxtent(afd)) != NULL) {
		write_ent(stdout, entry);
	}
	afclose_data();
	return(0);
}


/*
 *
 */
main(argc, argv)
	int	argc;
	char *	argv[];
{
	int	rc;

        setlocale(LC_ALL, "");

	options(argc, argv);

	rc = 0;
	if (CD.command & ADD)
		rc = add();

	if (CD.command & AUTO) {
		if (CD.autoflg == TRUE)
			rc = auto_add();
		else 
			rc = auto_del();
	}

	if (CD.command & MULTI) {
		if (CD.multiflg == TRUE)
			rc = multi_add();
		else 
			rc = multi_del();
	}

	if (CD.command & DELETE) 
		rc = del();

	if (CD.command & UPDATE)
		rc = update();

	if (CD.command & LIST)
		list_one();

	if (CD.command & LISTALL)
		list_all();

	exit(rc);
}

