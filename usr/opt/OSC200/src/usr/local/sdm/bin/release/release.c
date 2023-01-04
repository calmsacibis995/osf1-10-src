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
static char	*sccsid = "@(#)$RCSfile: release.c,v $ $Revision: 4.3.7.3 $ (DEC) $Date: 1993/04/19 16:28:35 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 **********************************************************************
 */
/*
 * release.c
 *
 *	Modification History:
 *
 * 20-Jul-91	Fred Canter
 *	Changes to allow release to strip all executables in the
 *	DIGITAL version of the OSF/1 build. The changes are conditional
 *	on _BLD, so the release command will behave as before outside
 *	the OSF/1 build environment.
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
/*
 *
 *   Release a program either from the build area (-fromfile) or move
 *   through release stages (-fromstage).
 *
 *   One of either -fromfile or -fromstage must given and they are mutually
 *   exclusive.  Each of these switches can be used to name 'modes' of
 *   operation.  In '-fromfile' mode, the source is taken to be the name
 *   given after the switch and can be anywhere.  For all of the other
 *   switches e.g. '-m', the appropriate actions are performed.
 *
 *   In '-fromstage' mode, the assumption is that the file(s) are to be
 *   released from one stage to the next (e.g. alpha to beta).  Some of
 *   the switches have no meaning (mainly various 'stripping' switches)
 *   in this mode, so are ignored.  If the target file(s) is/are created
 *   successfully, then the source is automatically removed, unless the
 *   '-norm' switch is specified.
 *
 *   The meaning of the switches:
 *
 *	-fromfile <file>   -- The file to be copied into the release area;
 *			      to be used when a 'program' is first built.
 *	-fromstage <stage> -- The stage directory prefix in the release area
 *			      to be concatenated with a file name in the
 *			      local file system to form the name for the source
 *			      file; to be used in moving a program through
 *			      various releases.
 *	-tostage   <stage> -- The stage directory prefix in the release are to
 *			      be concatenated with a file name in the local
 *			      file system to form the name of the destination
 *			      file; this is used with either of the above 2
 *			      switches and is required.
 *	-m	<mode>     -- The file protection of the target.
 *	-o	<owner>    -- The owner who should own the target.
 *	-g	<group>    -- The group who should own the target.
 *	-nostrip	   -- The target is not a binary, so 'strip' should not
 *			      be run on it.
 *	-nopost		   -- The log message is gathered up for inclusion in
 *			      the log but not posted to the bulletin board.
 *	-norm		   -- The source files are not removed after being
 *			      copied with -fromstage.
 *	-logfile <file>    -- The file containing the log message to use.
 *	-uselog <target>   -- The target to be used when searching for a log
 *			      message instead of the target being installed.
 *			      This log message is extracted from the -tostage
 *			      directory under the assumption that the current
 *			      target should receive the same log message as
 *			      this previously installed related target.
 *	-fast		   -- "fast" mode.  Totally non-interactive, any
 *			      errors abort.
 *	-q		   -- "Quick" mode.  No prompting to continue when
 *			      a non-fatal problem occurs, but the script
 *			      continues.
 *
 *   The remaining arguments are taken to be names in the local file system
 *   for the target.  The first name is taken to be the canonical name.
 *   Any other names are taken to be (hard) links to the canonical name,
 *   so the links will be made in the release area.
 */
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
extern int errno;
#include <stdio.h>
#include <a.out.h>
#if	defined(COFF) || defined(MACHO)
#ifdef	multimax
#include <sgs.h>	/* For ISMAGIC */
#endif
#endif
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#ifndef _BLD
#include <sdm/parse_rc_file.h>
#include <sdm/parse_cmd_line.h>
#include "release_db.h"
#endif

#ifdef	lint
/*VARARGS2*//*ARGSUSED*/
char *concat(buf, len) char *buf; int len; { return buf; };
#endif	lint

#ifndef	FALSE
#define FALSE	0
#endif
#ifndef	TRUE
#define TRUE	1
#endif
#ifndef	NAME_MAX
#define NAME_MAX	255
#endif

extern char *rindex();
extern char *salloc();
#ifndef _BLD
extern char *nxtarg();
#else
char *nxtarg();
#endif
#ifdef __STDC__ && defined(_BLD)
char *concat(char *, int, ...);
#else
extern char *concat();
#endif
extern char *index();
extern char *getenv();
extern char *ttyname();
extern char *getlogin();
extern char *errmsg();

void
#ifdef __STDC__
diag(char *fmt, ...);
#else
diag();
#endif

void
#ifdef __STDC__
ewarn(char *fmt, ...);
#else
ewarn();
#endif

void
#ifdef __STDC__
efatal(char *fmt, ...);
#else
efatal();
#endif

void
#ifdef __STDC__
warn(char *fmt, ...);
#else
warn();
#endif

void
#ifdef __STDC__
fatal(char *fmt, ...);
#else
fatal();
#endif

char *fixuppath();
unsigned int atoo();

#define	O_WRNEW	O_WRONLY|O_CREAT|O_EXCL

#ifndef _BLD
struct stage {
    struct stage *next;
    char **names;
    char **roots;
    int flags;
#define	STAGE_NORM	1
#define	STAGE_NOPOST	2
};

char sandbox[BUFSIZ];
char sb_rcfile[BUFSIZ];
char usr_rcfile[BUFSIZ];

struct rcfile rcfile;

char *rcfile_release_base;
char *rcfile_release_lock;
struct stage *rcfile_stages;
char *rcfile_release_database;
#endif	/* _BLD */
char *rcfile_release_idlist;

char *prog;

int havecmdlineinfo;
char *user;
char *owner;
char *group;
int uid;
int gid;
u_short mode;
int replaceok;
int debug;
int fast;
int quick;
char *fromfile;
char *tostage;
#ifndef _BLD
char *fromstage;
struct stage *fromstage_p;
struct stage *tostage_p;
char *uselog;
char *logfile;
char logfilebuf[MAXPATHLEN];
int post;
int rmflag;
int check;
#endif
int strip;
char *target;
char **links;
int nlinks;
int targetlen;
int lock_fd;

struct timeval source_tv[2];

char frombase[MAXPATHLEN];
char tobase[MAXPATHLEN];
char fullsource[MAXPATHLEN];
char fulltarget[MAXPATHLEN];
#ifndef _BLD
char fromlogdb[MAXPATHLEN];
char fromwhodb[MAXPATHLEN];
char tologdb[MAXPATHLEN];
char towhodb[MAXPATHLEN];

#define HL_FILE	1
#define HL_FROM	2
#define HL_TO	4

int havelog;

enum option { EDIT, TYPE, LOG, POST, SUBJECT, NOTIFY, NOCOMMAND };

char *posttab[] = {
    "edit		edit notice",
    "type		print notice for inspection",
    "notify		change \"To\" line",
    "subject		change \"Subject\" line",
    "log		log notice as planned, no post",
    "post		post notice as planned",
    0
};

enum option postopt[] = {
    EDIT,
    TYPE,
    NOTIFY,
    SUBJECT,
    LOG,
    POST
};

char *logtab[] = {
    "edit		edit notice",
    "type		print notice for inspection",
    "log		log notice as planned, no post",
    0
};

enum option logopt[] = {
    EDIT,
    TYPE,
    LOG
};

char *arc_table[] = {
    "abort",
#define ABORT	0
    "retry",
#define RETRY	1
    "continue",
#define CONTIN	2
    NULL
};

char *aur_table[] = {
    "abort",
    "unlock",
#define UNLOCK	1
    "replace",
#define REPLACE	2
    NULL
};

char *ar_table[] = {
    "abort",
    "retry",
    NULL
};
#endif	/* _BLD */

getarc()
{
    int key;

#ifdef _BLD
    return(-1);
#else
    if (quick || fast)
	return(check ? -1 : 1);
    if (check)
	key = getstab("Abort or retry?", ar_table, "retry");
    else
	key = getstab("Abort, retry or continue?", arc_table, "retry");
    if (key == ABORT)
	return(-1);
    if (key == CONTIN)
	return(1);
    return(0);
#endif	/* _BLD */
}

exists(path)
char *path;
{
    struct stat statb;

    return(stat(path, &statb));
}

main(argc, argv)
int argc;
char **argv;
{
    char basedir[BUFSIZ];
#ifndef _BLD
    struct stage *stage_p;
#endif

    /*
     * parse command line
     */
    parse_command_line(argc, argv);

#ifndef _BLD
    /*
     * read sandbox description
     */
    if (get_current_sb_basedir(sandbox, basedir, sb_rcfile, usr_rcfile) != 0)
	fatal("unable to parse home sandbox rc");
    if (parse_rc_file(sb_rcfile, &rcfile) != 0)
	fatal("unable to parse sandbox");

    /*
     * check sandbox description
     */
    check_sandbox(&rcfile);
#endif

    /*
     * get component information
     */
    get_component_info();

#ifdef _BLD
    /*
     * nothing fancy
     */
    do_simple_release();
#else	/* _BLD */
    /*
     * check and lock all release stages
     */
    if (check_release_stages() != 0)
	exit(1);

    /*
     * get default message for log/post
     */
    if (get_log_message() != 0)
	exit(1);

    /*
     * release from object directory
     */
    if (fromstage_p == NULL) {
	do_stage_release((struct stage *)NULL, rcfile_stages);
	fromstage_p = rcfile_stages;
    }
    strip = FALSE;
    fromfile = NULL;

    /*
     * release through all remaining stages
     */
    for (stage_p = fromstage_p; stage_p != tostage_p; stage_p = stage_p->next)
	do_stage_release(stage_p, stage_p->next);

    /*
     * make post
     */
    make_post(tostage_p);

    /*
     * all done
     */
    (void) unlink(logfile);
#endif	/* _BLD */

    exit(0);
}

parse_command_line(argc, argv)
int argc;
char **argv;
{
    int i;

    if (argc > 0) {
	if ((prog = rindex(argv[0], '/')) != NULL)
	    prog++;
	else
	    prog = argv[0];
	argc--; argv++;
    } else
	prog = "release";

    havecmdlineinfo = FALSE;
    owner = NULL;
    group = NULL;
    mode = (u_short)-1;
    fromfile = NULL;
    tostage = NULL;
#ifndef _BLD
    fromstage = NULL;
    rmflag = TRUE;
    post = TRUE;
    logfile = NULL;
    uselog = NULL;
#endif
    strip = TRUE;
    replaceok = FALSE;
    debug = FALSE;
    fast = FALSE;
    quick = FALSE;

    while (argc > 0) {
	if (argv[0][0] != '-')
	    break;
	switch (argv[0][1]) {
	case 'f':
	    if (strcmp(argv[0], "-fast") == 0) {
		fast = TRUE;
	    } else if (strcmp(argv[0], "-fromfile") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		fromfile = fixuppath(argv[0]);
#ifndef _BLD
	    } else if (strcmp(argv[0], "-fromstage") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		fromstage = argv[0];
#endif
	    } else
		usage(argv[0]);
	    break;
	case 't':
	    if (strcmp(argv[0], "-tostage") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		tostage = argv[0];
	    } else
		usage(argv[0]);
	    break;
	case 'm':
	    if (argv[0][2] == '\0') {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		if ((i = atoo(argv[1])) < 0)
		    fatal("invalid argument to %s: %s", argv[0], argv[1]);
		argc--; argv++;
		mode = (u_short)i;
		havecmdlineinfo = TRUE;
	    } else
		usage(argv[0]);
	    break;
	case 'o':
	    if (argv[0][2] == '\0') {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		owner = salloc(argv[0]);
		havecmdlineinfo = TRUE;
	    } else
		usage(argv[0]);
	    break;
	case 'g':
	    if (argv[0][2] == '\0') {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		group = salloc(argv[0]);
		havecmdlineinfo = TRUE;
	    } else
		usage(argv[0]);
	    break;
	case 'n':
	    if (strcmp(argv[0], "-nostrip") == 0) {
		strip = FALSE;
#ifndef _BLD
	    } else if (strcmp(argv[0], "-norm") == 0) {
		rmflag = FALSE;
	    } else if (strcmp(argv[0], "-nopost") == 0) {
		post = FALSE;
#endif
	    } else
		usage(argv[0]);
	    break;
#ifndef _BLD
	case 'l':
	    if (strcmp(argv[0], "-logfile") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		logfile = argv[0];
	    } else
		usage(argv[0]);
	    break;
	case 'u':
	    if (strcmp(argv[0], "-uselog") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		uselog = argv[0];
	    } else
		usage(argv[0]);
	    break;
#endif
	case 'r':
	    if (strcmp(argv[0], "-replaceok") == 0) {
		replaceok = TRUE;
#ifndef _BLD
	    } else if (strcmp(argv[0], "-rc") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		(void) strcpy(usr_rcfile, argv[0]);
#endif
	    } else
		usage(argv[0]);
	    break;
#ifndef _BLD
	case 's':
	    if (strcmp(argv[0], "-sb") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		(void) strcpy(sandbox, argv[0]);
	    } else if (strcmp(argv[0], "-sb_rc") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		(void) strcpy(sb_rcfile, argv[0]);
	    } else
		usage(argv[0]);
	    break;
#endif
	case 'd':
	    if (strcmp(argv[0], "-debug") == 0) {
		debug = TRUE;
	    } else
		usage(argv[0]);
	    break;
#ifdef _BLD
	case 'i':
	    if (strcmp(argv[0], "-idfile") == 0) {
		if (argc == 1)
		    fatal("missing argument to %s", argv[0]);
		argc--; argv++;
		rcfile_release_idlist = argv[0];
	    } else
		usage(argv[0]);
	    break;
#endif
	case 'q':
	    if (argv[0][2] == '\0') {
		quick = TRUE;
	    } else
		usage(argv[0]);
	    break;
	default:
	    usage(argv[0]);
	}
	argc--; argv++;
    }

    /*
     * remaining arguments are target and optional hard links
     */
    if (argc == 0) {
	warn("must specify at least one target file");
	usage((char *) NULL);
    }
    if (argc > 1)
	havecmdlineinfo = TRUE;
    nlinks = argc;
    links = argv;
    for (i = 0; i < nlinks; i++) {
	if (*links[i] != '/')
	    warn("target \"%s\" not absolute", links[i]);
	links[i] = fixuppath(links[i]);
    }
    target = links[0];
    targetlen = strlen(target);
/*    if (target[targetlen-1] == 'o' && target[targetlen-2] == '.')
	strip = FALSE; */
#ifdef _BLD
    if (fromfile == NULL) {
	warn("must specify -fromfile");
	usage((char *) NULL);
    }
#else
    if (fromstage != NULL) {
	if (fromfile != NULL) {
	    warn("-fromfile and -fromstage are mutually exclusive");
	    usage((char *) NULL);
	}
    } else {
	if (fromstage == NULL && fromfile == NULL) {
	    warn("must specify -fromfile or -fromstage");
	    usage((char *) NULL);
	}
    }
#endif
    if (tostage == NULL) {
	warn("must specify -tostage");
	usage((char *) NULL);
    }
}

usage(opt)
char *opt;
{
    if (opt != NULL)
	warn("unknown switch %s", opt);
    fprintf(stderr, "usage: %s %s %s %s target [ links ... ]\n", prog,
	 "[ -fromfile <file> | -fromstage <stage> ]",
	 "[ -tostage <stage> ] [ -idfile <file> ]",
	 "[ -m <mode> -o <owner> -g <group> -norm -nostrip -nopost -q ]");
    exit(1);
}

char *fixuppath(path)
char *path;
{
    char buf[MAXPATHLEN];
    register char *p = buf;

    *p = *path;
    while (*path != '\0') {
	path++;
	while (*p == '/' && *path == '/')
	    path++;
	*++p = *path;
    }
    return(salloc(buf));
}

#ifndef _BLD
check_sandbox(rcfile_p)
struct rcfile *rcfile_p;
{
    char temp[MAXPATHLEN];
    struct stage *stage_p;
    struct field *field_p;
    struct arg_list *args_p;

    if (debug)
	printf("[ checking sandbox ]\n");
    if (rc_file_field(rcfile_p, "release_base", &field_p) != 0)
	fatal("release_base not defined");
    args_p = field_p->args;
    if (args_p->ntokens != 1)
	fatal("improper release_base");
    rcfile_release_base = args_p->tokens[0];
    if (*rcfile_release_base != '/')
	fatal("release_base is not an absolute path");
    if (exists(rcfile_release_base) < 0)
	fatal("release_base %s not found", rcfile_release_base);
    if (rc_file_field(rcfile_p, "release_lock", &field_p) != 0)
	rcfile_release_lock = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    fatal("improper release_lock");
	rcfile_release_lock = args_p->tokens[0];
	if (*rcfile_release_lock != '/')
	    fatal("release_lock is not an absolute path");
    }
    if (rc_file_field(rcfile_p, "stage", &field_p) != 0)
	fatal("release stages not defined");
    parse_stages(field_p->args);
    if (find_stage(tostage, &tostage_p))
	fatal("release stage %s not defined", tostage);
    if (fromstage != NULL) {
	if (find_stage(fromstage, &fromstage_p))
	    fatal("release stage %s not defined", fromstage);
	for (stage_p = rcfile_stages; stage_p != NULL; stage_p = stage_p->next) {
	    if (stage_p == fromstage_p) {
		if (stage_p == tostage_p)
		    fatal("release stages %s and %s are the same",
			  fromstage, tostage);
		break;
	    } else if (stage_p == tostage_p)
		fatal("unable to release to previous stage %s", tostage);
	}
    }
    if (rc_file_field(rcfile_p, "release_database", &field_p) != 0)
	rcfile_release_database = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    fatal("improper release_database");
	rcfile_release_database = args_p->tokens[0];
	if (*rcfile_release_database != '/') {
	    (void) concat(temp, sizeof(temp),
			  rcfile_release_base, "/", rcfile_release_database, NULL);
	    if ((rcfile_release_database = salloc(temp)) == NULL)
		fatal("release_database salloc failure");
	}
    }
    if (rc_file_field(rcfile_p, "release_idlist", &field_p) != 0)
	rcfile_release_idlist = NULL;
    else {
	args_p = field_p->args;
	if (args_p->ntokens != 1)
	    fatal("improper release_idlist");
	rcfile_release_idlist = args_p->tokens[0];
	if (*rcfile_release_idlist != '/') {
	    (void) concat(temp, sizeof(temp),
			  rcfile_release_base, "/", rcfile_release_idlist, NULL);
	    if ((rcfile_release_idlist = salloc(temp)) == NULL)
		fatal("release_idlist salloc failure");
	}
    }
}

char **
alloc_ptrs(name)
char *name;
{
    char *ptr, *p, **pp;
    int i;

    if ((ptr = salloc(name)) == NULL)
	fatal("unable to allocate ptrs");
    i = 1;
    for (p = ptr; *name != '\0'; p++, name++) {
	*p = *name;
	if (*p == ',')
	    i++;
    }
    if ((pp = (char **)calloc((unsigned)i+1, sizeof(char *))) == NULL)
	fatal("unable to allocate ptrs");
    i = 0;
    p = ptr;
    do {
	pp[i++] = nxtarg(&p, ",");
    } while (*p != '\0');
    pp[i] = NULL;
    return(pp);
}

parse_stages(args_p)
struct arg_list *args_p;
{
    struct stage **stage_pp;
    int i;

    stage_pp = &rcfile_stages;
    do {
	*stage_pp = (struct stage *)calloc(1, sizeof(struct stage));
	if (*stage_pp == (struct stage *)NULL)
	    fatal("unable to allocate stage");
	if (args_p->ntokens < 2)
	    fatal("bad stage in sandbox description");
	(*stage_pp)->names = alloc_ptrs(args_p->tokens[0]);
	(*stage_pp)->roots = alloc_ptrs(args_p->tokens[1]);
	for (i = 2; i < args_p->ntokens; i++) {
	    if (strcmp(args_p->tokens[i], "norm") == 0)
		(*stage_pp)->flags |= STAGE_NORM;
	    else if (strcmp(args_p->tokens[i], "nopost") == 0)
		(*stage_pp)->flags |= STAGE_NOPOST;
	    else
		fatal("unknown stage token %s", args_p->tokens[i]);
	}
	stage_pp = &((*stage_pp)->next);
	args_p = args_p->next;
    } while (args_p != NULL);
}

find_stage(stage, stage_pp)
char *stage;
struct stage **stage_pp;
{
    struct stage *stage_p;
    char **pp;

    for (stage_p = rcfile_stages; stage_p != NULL; stage_p = stage_p->next)
	for (pp = stage_p->names; *pp != NULL; pp++)
	    if (strcmp(stage, *pp) == 0) {
		*stage_pp = stage_p;
		return(0);
	    }
    return(1);
}
#endif	/* _BLD */

get_release_id(type, value)
char *type, *value;
{
    FILE *fp;
    char buffer[MAXPATHLEN];
    char *ptr, *p;
    int neg;
    int retval;

    if ((fp = fopen(rcfile_release_idlist, "r")) == NULL)
	efatal("fopen %s", rcfile_release_idlist);
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
	if ((p = index(buffer, '\n')) != NULL)
	    *p = '\0';
	p = buffer;
	if (index("#", *p) != NULL)
	    continue;
	ptr = nxtarg(&p, " \t");
	if (*ptr != *type || strcmp(ptr, type) != 0)
	    continue;
	ptr = nxtarg(&p, " \t");
	if (*ptr != *value || strcmp(ptr, value) != 0)
	    continue;
	ptr = nxtarg(&p, " \t");
	if (neg = (*ptr == '-'))
	    ptr++;
	if (*ptr < '0' || *ptr > '9')
	    fatal("bad format %s for %s", type, value);
	if ((retval = atoi(ptr)) == -1)
	    fatal("bad format %s for %s", type, value);
	return(neg ? -retval : retval);
    }
    fatal("%s for %s not found in release id list", type, value);
}

get_default_info()
{
    struct passwd *pw;
    struct group *gr;

    if (mode == (u_short)-1)
	mode = 0755;
    if (owner == NULL)
	fatal("must specify -o <owner> for target");
    if (rcfile_release_idlist == NULL) {
	if ((pw = getpwnam(owner)) == NULL)
	    fatal("user %s not found in passwd database", owner);
	uid = pw->pw_uid;
    } else
	uid = get_release_id("uid", owner);
    if (group == NULL)
	fatal("must specify -g <group> for target");
    if (rcfile_release_idlist == NULL) {
	if ((gr = getgrnam(group)) == NULL)
	    fatal("user %s not found in group database", group);
	gid = gr->gr_gid;
    } else
	gid = get_release_id("gid", group);
}

#ifndef _BLD
get_backup_database()
{
    char buf[MAXPATHLEN];
    int status;

    if (fast || !getbool("Restore from backup copy?", FALSE)) {
	unlock_database();
	exit(1);
    }
    initialize_database();
    (void) concat(buf, sizeof(buf),
		  rcfile_release_database, ".backup", NULL);
    if ((status = restore_database(buf)) != 0) {
	if (status > 0)
	    warn("Encountered %d errors reading backup database", status);
	warn("Unable to restore database from backup copy");
	if (!getbool("Rebuild database from description?", FALSE)) {
	    unlock_database();
	    exit(1);
	}
	initialize_database();
	(void) concat(buf, sizeof(buf),
		      rcfile_release_database, ".description", NULL);
	if ((status = read_database_description(buf)) != 0) {
	    if (status > 0)
		warn("Encountered %d errors reading database", status);
	    unlock_database();
	    fatal("Unable to rebuild release database, cannot continue");
	}
    }
}

compare_component_info(file_ptr, file_info_ptr)
struct file *file_ptr;
struct file_info *file_info_ptr;
{
    char buf[MAXPATHLEN];
    char *ptr;
    int i;
    int temp;
    struct file *link_file_ptr;
    int makechanges;

    makechanges = FALSE;
    if (owner != NULL &&
	strcmp(owner, file_info_ptr->owner->data) != 0) {
	diag("owner difference: database %s vs. command line %s",
	     file_info_ptr->owner->data, owner);
	if (!fast && getbool("Replace database information?", FALSE))
	    makechanges = TRUE;
	else {
	    (void) free(owner);
	    owner = salloc(file_info_ptr->owner->data);
	}
    }
    if (group != NULL &&
	strcmp(group, file_info_ptr->group->data) != 0) {
	diag("group difference: database %s vs. command line %s",
	     file_info_ptr->group->data, group);
	if (!fast && getbool("Replace database information?", FALSE))
	    makechanges = TRUE;
	else {
	    (void) free(group);
	    group = salloc(file_info_ptr->group->data);
	}
    }
    if (mode != (u_short)-1 && mode != file_info_ptr->mode) {
	diag("mode difference: database %#o vs. command line %#o",
	     file_info_ptr->mode, mode);
	if (!fast && getbool("Replace database information?", FALSE))
	    makechanges = TRUE;
	else
	    mode = file_info_ptr->mode;
    }
    temp = 1;
    if (file_ptr->links != NULL) {
	link_file_ptr = file_ptr->links;
	while (link_file_ptr != file_ptr) {
	    temp++;
	    link_file_ptr = link_file_ptr->links;
	}
    }
    if (nlinks == temp) {
	if (nlinks == 1)
	    return(makechanges);
	link_file_ptr = file_ptr->links;
	while (link_file_ptr != file_ptr) {
	    component_path(link_file_ptr, buf);
	    for (i = 1; i < nlinks; i++)
		if (strcmp(buf, links[i]) == 0)
		    break;
	    if (i == nlinks)
		break;
	    link_file_ptr = link_file_ptr->links;
	}
	if (link_file_ptr == file_ptr)
	    return(makechanges);
    }
    diag("link difference:");
    diag("database:");
    link_file_ptr = file_ptr->links;
    while (link_file_ptr != file_ptr) {
	component_path(link_file_ptr, buf);
	diag("  %s", buf);
	link_file_ptr = link_file_ptr->links;
    }
    diag("command line:");
    for (i = 1; i < nlinks; i++)
	diag("  %s", links[i]);
    if (!fast && getbool("Replace database information?", FALSE))
	return(TRUE);
    if (temp > nlinks) {
	links = (char **) calloc((unsigned)temp, sizeof(char *));
	if (links == NULL) {
	    unlock_database();
	    fatal("Unable to allocate new link table");
	}
    }
    links[0] = target;
    nlinks = temp;
    if (nlinks == 1)
	return(makechanges);
    link_file_ptr = file_ptr->links;
    for (i = 1; i < nlinks; i++) {
	component_path(link_file_ptr, buf);
	links[i] = fixuppath(buf);
	link_file_ptr = link_file_ptr->links;
    }
    if (link_file_ptr != file_ptr) {
	unlock_database();
	fatal("Inconsistant component links");
    }
    return(makechanges);
}
#endif	/* _BLD */

get_component_info()
{
    char buf[MAXPATHLEN];
    char *p, *ptr;
    int i;
    struct file *file_ptr;
    struct file_info *file_info_ptr;
    int status;
    int temp;
    struct string *comp_owner, *comp_group;
    u_short comp_mode;
    int makechanges;
    int save_database_changes;
    struct passwd *pw;
    struct group *gr;

    if ((user = getenv("USER")) == NULL)
	fatal("USER environment variable not set");
#ifndef _BLD
    if (rcfile_release_database == NULL) {
#endif
	get_default_info();
	return;
#ifndef _BLD
    }
    initialize_database();
    if (lock_database(rcfile_release_database) != 0)
	fatal("Unable to lock release database");
    save_database_changes = FALSE;
    if ((status = restore_database(rcfile_release_database)) != 0) {
	if (status > 0)
	    warn("Encountered %d errors restoring release database", status);
	warn("Unable to restore release database");
	get_backup_database();
	save_database_changes = TRUE;
    }
    makechanges = FALSE;
    if (lookup_component(target, &file_ptr, &file_info_ptr) != 0) {
	warn("Unable to find component %s in release database", target);
	if (fast || !getbool("Do you wish to continue?", FALSE)) {
	    unlock_database();
	    exit(1);
	}
	if (create_component(target, &file_ptr, &file_info_ptr) != 0) {
	    unlock_database();
	    fatal("Unable to create component");
	}
	makechanges = TRUE;
    } else {
	/*
	 * target may have been a link name, so we follow the list of links
	 * until we are at the main component name.
	 */
	while (file_ptr->links != NULL && file_ptr->srcdir == NULL)
	    file_ptr = file_ptr->links;
	if (havecmdlineinfo)
	    makechanges = compare_component_info(file_ptr, file_info_ptr);
    }
    if (makechanges) {
	diag("[ target %s ]", target);
	if (owner == NULL)
	    owner = file_info_ptr->owner->data;
	(void) getstr("Owner", owner, buf);
	if (create_string(buf, &comp_owner) != 0) {
	    unlock_database();
	    fatal("Unable to create owner string");
	}
	if (group == NULL)
	    group = file_info_ptr->group->data;
	(void) getstr("Group", group, buf);
	if (create_string(buf, &comp_group) != 0) {
	    unlock_database();
	    fatal("Unable to create group string");
	}
	if (mode == (u_short)-1)
	    mode = file_info_ptr->mode;
	(void) sprintf(buf, "%#o", mode);
	(void) getstr("Mode", buf, buf);
	if ((temp = atoo(buf)) < 0) {
	    unlock_database();
	    fatal("Invalid mode %s", buf);
	}
	comp_mode = (u_short) temp;
	if (nlinks == 1)
	    diag("No other links to %s", target);
	else {
	    diag("Links to %s:", target);
	    for (i = 1; i < nlinks; i++)
		diag("  %s", links[i]);
	}
	if (!fast && !getbool("Is this link information correct?", TRUE)) {
	    (void) getstr("How many links are there including the target?",
			  "1", buf);
	    if ((temp = atoi(buf)) < 0) {
		unlock_database();
		fatal("Invalid link count");
	    }
	    if (temp > nlinks) {
		links = (char **) calloc((unsigned)temp, sizeof(char *));
		if (links == NULL) {
		    unlock_database();
		    fatal("Unable to allocate new link table");
		}
		links[0] = target;
	    }
	    nlinks = temp;
	    for (i = 1; i < nlinks; i++) {
		(void) getstr("Link path", "", buf);
		if (*buf != '/')
		    warn("target \"%s\" not absolute", buf);
		links[i] = fixuppath(buf);
	    }
	}
	for (i = 1; i < nlinks; i++)
	    diag("  [ linked to %s ]", links[i]);
	if (fast || !getbool("Use this information?", FALSE)) {
	    unlock_database();
	    exit(1);
	}
	if (set_component_info(file_ptr, &file_info_ptr,
			       nlinks, links,
			       comp_owner, comp_group, comp_mode) != 0) {
	    unlock_database();
	    fatal("Unable to set component information");
	}
	save_database_changes = TRUE;
    }
    if (save_database_changes && !fast &&
	getbool("Update changes to database?", FALSE)) {
	(void) concat(buf, sizeof(buf),
		      rcfile_release_database, ".new", NULL);
	(void) unlink(buf);
	if ((status = save_database(buf)) != 0) {
	    if (status > 0)
		warn("Encountered %d errors saving database", status);
	    unlock_database();
	    fatal("Unable to update database changes");
	}
	(void) concat(buf, sizeof(buf),
		      rcfile_release_database, ".newdesc", NULL);
	(void) unlink(buf);
	if ((status = write_database_description(buf)) != 0) {
	    if (status > 0)
		warn("Encountered %d errors writing database", status);
	    unlock_database();
	    fatal("Unable to update database changes");
	}
    }
    unlock_database();

    owner = file_info_ptr->owner->data;
    if ((pw = getpwnam(owner)) == NULL)
	fatal("user %s not found in passwd database", owner);
    uid = pw->pw_uid;
    group = file_info_ptr->group->data;
    if ((gr = getgrnam(group)) == NULL)
	fatal("user %s not found in group database", group);
    gid = gr->gr_gid;
    mode = file_info_ptr->mode;
#endif	/* _BLD */
}

#ifndef	_BLD
check_release_stages()
{
    int status;
    struct stage *stage_p;
    struct stat statb;
    int fd;

    for (;;) {
	/*
	 * lock release base directory
	 */
	if (lock_release() != 0)
	    return(1);

	/*
	 * check release from object directory
	 */
	if ((stage_p = fromstage_p) == NULL) {
	    while (lstat(fromfile, &statb) < 0) {
		ewarn("stat %s", fromfile);
		if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
		    return(1);
	    }
	    if (((statb.st_mode&S_IFMT) != S_IFREG) &&
                ((statb.st_mode&S_IFMT) != S_IFLNK))
		fatal("%s is not a regular file", fromfile);
	    while ((fd = open(fromfile, O_RDONLY, 0)) < 0) {
		ewarn("open %s", fromfile);
		if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
		    return(1);
	    }
	    (void) close(fd);
	    stage_p = rcfile_stages;
	}

	/*
	 * check release through intermediate stages
	 */
	for (;;) {
	    if ((status = check_release_stage(stage_p)) < 0)
		return(1);
	    if (status > 0)
		break;
	    if (stage_p == tostage_p)
		return(0);
	    stage_p = stage_p->next;
	}

	/*
	 * unlock and wait
	 */
	(void) close(lock_fd);
	if (fast || !getbool("Ready to re-lock?", TRUE))
	    return(1);
    }
}

lock_release()
{
    char buf[MAXPATHLEN], datebuf[MAXPATHLEN];
    char *ptr;
    struct tm *tm;
    time_t now;
    char *tty;

    if (rcfile_release_lock == NULL) {
	diag("[ release not locked for exclusive access ]");
	return(0);
    }
    for (;;) {
	if ((lock_fd = open(rcfile_release_lock, O_RDWR|O_CREAT, 0600)) < 0)
	    ewarn("open %s", rcfile_release_lock);
	else {
	    if (flock(lock_fd, LOCK_EX|LOCK_NB) == 0)
		break;
	    if (errno != EWOULDBLOCK)
		ewarn("flock");
	    else {
		diag("release lock busy:");
		(void) putc('\t', stderr);
		(void) fflush(stderr);
		(void) filecopy(lock_fd, fileno(stderr));
		if (fast || !getbool("Do you want to wait for the lock?", TRUE))
		    return(1);
		diag("[ waiting for exclusive access lock ]");
		if (flock(lock_fd, LOCK_EX) == 0)
		    break;
		ewarn("flock");
	    }
	    (void) close(lock_fd);
	}
	if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
    if ((tty = ttyname(0)) == NULL)
	tty = "tty??";
    now = time((time_t *) 0);
    tm = localtime(&now);
    fdate(datebuf, "%3Month %02day %year %02hour:%02min:%02sec", tm);
    ptr = concat(buf, sizeof(buf),
		 user, " on ", tty, " at ", datebuf, "\n", NULL);
    errno = 0;
    if (lseek(lock_fd, (long)0, L_SET) == -1 && errno != 0)
	ewarn("lseek");
    if (write(lock_fd, buf, ptr-buf) != ptr-buf)
	ewarn("write");
    if (ftruncate(lock_fd, (off_t)(ptr-buf)) < 0)
	ewarn("ftruncate");
    diag("[ release locked for exclusive access ]");
    return(0);
}

check_release_stage(stage_p)
struct stage *stage_p;
{
    char base[MAXPATHLEN];
    struct stat statb;
    char *ptr;

    ptr = stage_p->roots[0];
    if (*ptr == '/')
	(void) strcpy(base, ptr);
    else
	(void) concat(base, sizeof(base), rcfile_release_base, "/", ptr, NULL);
    if (stat(base, &statb) < 0)
	efatal("stat %s", base);
    if ((statb.st_mode&S_IFMT) != S_IFDIR)
	fatal("%s: not a directory", base);

    if (check_release(base) != 0)
	return(-1);

    if (stage_p == fromstage_p || stage_p->next == NULL)
	return(0);

    return(check_log(base));
}
#endif	/* _BLD */

check_release(base)
char *base;
{
    char temp[MAXPATHLEN];
    int havetarget;
    int i;
    int fd;
    struct stat statb;

    (void) concat(fulltarget, sizeof(fulltarget), base, target, NULL);
    if (lstat(fulltarget, &statb) < 0) {
#ifdef	_BLD
	if (ckmakepath(fulltarget, base, target) != 0) {
	    warn("makepath failed for %s", fulltarget);
	    return(1);
	}
#else
	while (ckmakepath(fulltarget, base, target) != 0) {
	    warn("makepath failed for %s", fulltarget);
	    if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
		return(1);
	}
	havetarget = FALSE;
#endif
    } else {
	havetarget = TRUE;
	if (((statb.st_mode&S_IFMT) != S_IFREG) &&
                ((statb.st_mode&S_IFMT) != S_IFLNK))
	    fatal("%s is not a regular file", fulltarget);
    }
    for (i = 1; i < nlinks; i++) {
	(void) concat(temp, sizeof(temp), base, links[i], NULL);
	if (exists(temp) < 0) {
	    if (ckmakepath(temp, base, links[i]) != 0) {
		warn("makepath failed for %s", temp);
#ifndef _BLD
		if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
#endif
		    return(1);
	    }
	}
    }
#ifdef _BLD
    if (havetarget && checkstate(fulltarget, &statb) > 0)
	return(1);
#else
    if (havetarget && checkstate(fulltarget, &statb) > 0) {
	if (fast || !getbool("Do you wish to continue?", 0))
	    return(1);
	diag("");
    }
#endif

#ifndef	_BLD
    (void) concat(tologdb, sizeof(tologdb), base, "/.post.log", NULL);
    (void) concat(towhodb, sizeof(towhodb), base, "/.installers.log", NULL);

    /*
     * check that all log files exist and are readable
     */
    if (exists(tologdb) < 0) {
	while ((fd = open(tologdb, O_WRONLY|O_CREAT, 0644)) < 0) {
	    ewarn("open %s", tologdb);
	    if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
		return(1);
	}
	(void) close(fd);
    }
    if (exists(towhodb) < 0) {
	while ((fd = open(towhodb, O_WRONLY|O_CREAT, 0644)) < 0) {
	    ewarn("open %s", towhodb);
	    if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
		return(1);
	}
	(void) close(fd);
    }
    while ((fd = open(tologdb, O_RDONLY, 0)) < 0) {
	ewarn("open %s", tologdb);
	if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
    (void) close(fd);
    while ((fd = open(towhodb, O_RDONLY, 0)) < 0) {
	ewarn("open %s", towhodb);
	if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
    (void) close(fd);
#endif
    (void) concat(temp, sizeof(temp), base, "/.release.chk", NULL);
#ifdef _BLD
    if ((fd = open(temp, O_WRONLY|O_CREAT, 0644)) < 0) {
	ewarn("open %s", temp);
	return(1);
    }
#else
    while ((fd = open(temp, O_WRONLY|O_CREAT, 0644)) < 0) {
	ewarn("open %s", temp);
	if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
#endif
    (void) close(fd);
    (void) unlink(temp);
    (void) concat(temp, sizeof(temp), fulltarget, ".chk", NULL);
#ifdef _BLD
    if ((fd = open(temp, O_WRONLY|O_CREAT, 0644)) < 0) {
	ewarn("open %s", temp);
	return(1);
    }
#else
    while ((fd = open(temp, O_WRONLY|O_CREAT, 0644)) < 0) {
	ewarn("open %s", temp);
	if (fast || getstab("Abort or retry?", ar_table, "retry") == ABORT)
	    return(1);
    }
#endif
    (void) close(fd);
    (void) unlink(temp);
    return(0);
}

checkstate(file, statp)
char *file;
struct stat *statp;
{
    int errs = 0;

    /*
     * make from target checks
     */
    if (statp->st_uid != uid) {
	diag("\n*** WARNING ***");
	diag("The owner of '%s' is not consistent with", file);
	diag("the owner given on the command line.\n");
	diag("The uid of '%s' is %d.", file, statp->st_uid);
	diag("The uid given is %d.\n", uid);
	errs++;
    }
    if (statp->st_gid != gid) {
	diag("\n*** WARNING ***");
	diag("The group of '%s' is not consistent with", file);
	diag("the group given on the command line.\n");
	diag("The gid of '%s' is %d.", file, statp->st_gid);
	diag("The gid given is %d.\n", gid);
	errs++;
    }
    if ((statp->st_mode&07777) != (mode&07777)) {
	diag("\n*** WARNING ***");
	diag("The mode of '%s' is not consistent with", file);
	diag("the mode given on the command line.\n");
	diag("The mode of '%s' is %#o.", file, statp->st_mode&07777);
	diag("The mode given is %#o.\n", mode&07777);
	errs++;
    }
    if (statp->st_nlink != nlinks) {
	diag("\n*** WARNING ***");
	diag("The number of links to '%s' is not consistent with", file);
	diag("the number of files given on the command line.\n");
	diag("The number of links to '%s' is %d.", file, statp->st_nlink);
	diag("The number of files given is %d.\n", nlinks);
	errs++;
    }
    return(errs);
}

#ifndef	_BLD
check_log(base)
char *base;
{
    char temp[MAXPATHLEN];
    char buf[MAXPATHLEN];
    int keptwho, keptlog;
    int fd;
    int query;
    int len;
    int key;
    FILE *fp;

    (void) strcpy(temp, "/tmp/release.XXXXXX");
    if ((fd = mkstemp(temp)) < 0) {
	ewarn("mkstemp %s", temp);
	return(-1);
    }
    (void) concat(tologdb, sizeof(tologdb), base, "/.post.log", NULL);
    (void) concat(towhodb, sizeof(towhodb), base, "/.installers.log", NULL);
    if (keep_lines(towhodb, fd, &keptwho) != 0) {
	(void) close(fd);
	(void) unlink(temp);
	return(-1);
    }
    if (keep_lines(tologdb, fd, &keptlog) != 0) {
	(void) close(fd);
	(void) unlink(temp);
	return(-1);
    }
    (void) close(fd);
    if (!keptwho && !keptlog) {
	(void) unlink(temp);
	return(0);
    }
    diag("\n*** WARNING ***");
    diag("There is already a verion of %s installed.\n", fulltarget);
    (void) fflush(stderr);
    if ((fp = fopen(temp, "r")) == NULL) {
	ewarn("fopen %s", temp);
	(void) unlink(temp);
	return(-1);
    }
    query = !replaceok;
    if (keptwho <= 0) {
	diag("%s\t???\tunknown");
	query = TRUE;
    } else {
	len = strlen(user);
	while (keptwho-- > 0) {
	    if (fgets(buf, sizeof(buf), fp) == NULL) {
		ewarn("fgets %s", temp);
		(void) fclose(fp);
		(void) unlink(temp);
		return(-1);
	    }
	    (void) fputs(buf, stdout);
	    if (strncmp(buf, user, len) != 0 ||
		(buf[len] != ' ' && buf[len] != '\t')) {
		query = TRUE;
		break;
	    }
	}
    }
    (void) ffilecopy(fp, stdout);
    (void) fclose(fp);
    (void) unlink(temp);
    diag("");
    if (!query)
	return(0);
    if (quick || fast)
	return(-1);
    key = getstab("Abort, unlock or replace?", aur_table, "");
    if (key == ABORT)
	return(-1);
    diag("");
    if (key == REPLACE)
	return(0);
    return(1);
}

get_old_log(stage_p, fd_p, temp)
struct stage *stage_p;
int *fd_p;
char *temp;
{
    char buf[MAXPATHLEN];
    char ifile[MAXPATHLEN];
    char wbuf[BUFSIZ];
    char *ptr;
    int logfd;
    int kept;
    int keptwho;
    char *who;
    char *when;

    if (stage_p->next == NULL)
	return(0);
    if (stage_p != tostage_p)
	if (get_old_log(stage_p->next, fd_p, temp) != 0)
	    return(1);
    ptr = stage_p->roots[0];
    if (*ptr == '/') {
	(void) concat(buf, sizeof(buf), ptr, "/.post.log", NULL);
	(void) concat(ifile, sizeof(ifile), ptr, "/.installers.log", NULL);
    } else {
	(void) concat(buf, sizeof(buf),
		      rcfile_release_base, "/", ptr, "/.post.log", NULL);
	(void) concat(ifile, sizeof(ifile),
		      rcfile_release_base, "/", ptr, "/.installers.log", NULL);
    }
    if (keep_lines(ifile, *fd_p, &keptwho) != 0)
	return(1);
    (void) close(*fd_p);
    who = NULL;
    when = NULL;
    if (keptwho) {
	FILE *fp;

	fp = fopen(temp, "r");
	if (fp == NULL) {
	    ewarn("fopen %s", temp);
	    return(1);
	}
	if (fgets(wbuf, sizeof(wbuf)-1, fp) != NULL) {
	    char *p = wbuf;

	    who = nxtarg(&p, " \t\n");
	    when = nxtarg(&p, "\n");
	}
	(void) fclose(fp);
    }
    (void) unlink(temp);
    if ((*fd_p = open(temp, O_WRNEW, 0600)) < 0) {
	ewarn("open %s", temp);
	return(1);
    }
    if (who) {
	char sbuf[BUFSIZ];
	int i,l;

	(void) sprintf(sbuf, "[ Previous changes by %s (to %s on %s) ]\n",
		 who, stage_p->names[0], when);
	l = strlen(sbuf);
	i = write(*fd_p, sbuf, l);
	if (i != l) {
	    ewarn("write error (%d, not %d)", i, l);
	    return(1);
	}
    }
    if (keep_lines(buf, *fd_p, &kept) != 0)
	return(1);
    if (!kept)
	return(0);
    if (havelog&(HL_TO|HL_FROM)) {
	if ((logfd = open(logfilebuf, O_RDONLY, 0)) < 0) {
	    ewarn("open %s", logfilebuf);
	    return(1);
	}
	if (write(*fd_p, "\n", 1) != 1) {
	    ewarn("write");
	    return(1);
	}
	if (filecopy(logfd, *fd_p) < 0) {
	    ewarn("filecopy");
	    return(1);
	}
	(void) close(logfd);
    }
    (void) close(*fd_p);
    if (rename(temp, logfilebuf) < 0) {
	ewarn("rename");
	return(1);
    }
    if ((*fd_p = open(temp, O_WRNEW, 0600)) < 0) {
	ewarn("open %s", temp);
	return(1);
    }
    if (stage_p == fromstage_p)
	havelog |= HL_FROM;
    else
	havelog |= HL_TO;
    return(0);
}

get_log_message()
{
    char temp[MAXPATHLEN];
    char buf[MAXPATHLEN];
    char *otarget;
    int otargetlen;
    int fd;
    int logfd;
    char *ptr;
    int kept;
    enum option command;
    char *defans;
    FILE *fp;
    struct stage *stage_p;
    struct stat statb;

    (void) strcpy(logfilebuf, "/tmp/release-log.XXXXXX");
    if ((logfd = mkstemp(logfilebuf)) < 0)
	efatal("mkstemp %s", logfilebuf);
    if (uselog != NULL) {
	otarget = target;
	otargetlen = targetlen;
	target = uselog;
	targetlen = strlen(uselog);
	ptr = tostage_p->roots[0];
	if (*ptr == '/')
	    (void) concat(buf, sizeof(buf), ptr, "/.post.log", NULL);
	else
	    (void) concat(buf, sizeof(buf),
			  rcfile_release_base, "/", ptr, "/.post.log", NULL);
	if (keep_lines(buf, logfd, &kept) != 0)
	    return(1);
	if (!kept)
	    fatal("log message not found for %s", uselog);
	target = otarget;
	targetlen = otargetlen;
	havelog = HL_FILE;
	if (close(logfd) < 0) {
	    ewarn("close");
	    return(1);
	}
    } else if (logfile != NULL) {
	if ((fd = open(logfile, O_RDONLY, 0)) < 0)
	    efatal("open %s", logfile);
	if (filecopy(fd, logfd) < 0)
	    efatal("filecopy %s", logfile);
	if (close(fd) < 0)
	    efatal("close");
	havelog = HL_FILE;
	if (close(logfd) < 0) {
	    ewarn("close");
	    return(1);
	}
    } else {
	if (close(logfd) < 0) {
	    ewarn("close");
	    return(1);
	}
	havelog = 0;
	(void) strcpy(temp, "/tmp/release.XXXXXX");
	if ((fd = mkstemp(temp)) < 0)
	    efatal("mkstemp %s", temp);
	if ((stage_p = fromstage_p) == NULL)
	    stage_p = rcfile_stages;
	if (get_old_log(stage_p, &fd, temp) != 0) {
	    (void) close(fd);
	    (void) unlink(temp);
	    return(1);
	}
	(void) close(fd);
	(void) unlink(temp);
    }
    logfile = logfilebuf;

    /*
     * if not posting, no conflicts and have log, just return
     */
    if ((havelog&HL_TO) == 0 && !post && (havelog&(HL_FILE|HL_FROM)))
	return(0);

    /*
     * If no conflicts and have log, type it, otherwise edit
     */
    if (havelog != 0) {
	command = TYPE;
	diag("\nPlease update the following message, if necessary.");
    } else {
	command = EDIT;
	diag("\nPlease compose a log message for %s.", fulltarget);
    }

    /*
     * parse commands
     */
    for (;;) {
	switch (command) {
	case EDIT:
	    (void) editor(logfile, "Entering editor.");
	    defans = "type";
	    break;
	case TYPE:
	    if ((fp = fopen(logfile, "r")) == NULL)
		break;
	    printf("\n");
	    (void) ffilecopy(fp, stdout);
	    (void) fclose(fp);
	    defans = "log";
	    break;
	case LOG:
	    defans = NULL;
	    break;
	}
	if (defans == NULL) {
	    if (stat(logfile, &statb) == 0 && statb.st_size > 0)
		break;
	    diag("You are not allowed to log an empty message");
	    command = EDIT;
	} else {
	    if (fast)
		return(1);
	    command = logopt[getstab("Edit, type, log", logtab, defans)];
	    clearerr(stdin);
	}
    }
    return(0);
}
#endif	/* _BLD */

#ifdef _BLD

do_simple_release()
{
    struct stat statb;
    char *ptr;

    /*
     * check source
     */
    (void) strcpy(fullsource, fromfile);

    /*
     * check target
     */
    if (*tostage != '/')
	fatal("%s must be an absolute path", tostage);
    (void) strcpy(tobase, tostage);
    if (stat(tobase, &statb) < 0 || (statb.st_mode&S_IFMT) != S_IFDIR)
	fatal("%s is not a directory", tobase);
    (void) concat(fulltarget, sizeof(fulltarget), tobase, target, NULL);

    /*
     * release to main target directory
     */
    do_release();
}
#else	/* _BLD */
do_stage_release(fstage_p, tstage_p)
struct stage *fstage_p, *tstage_p;
{
    struct stat statb;
    char *ptr;
    int saverm = rmflag, savestrip = strip;
    int i;

    /*
     * check source
     */
    if (fstage_p != NULL) {
	ptr = fstage_p->roots[0];
	if (*ptr == '/')
	    (void) strcpy(frombase, ptr);
	else
	    (void) concat(frombase, sizeof(frombase),
			  rcfile_release_base, "/", ptr, NULL);
	if (stat(frombase, &statb) < 0 || (statb.st_mode&S_IFMT) != S_IFDIR)
	    efatal("%s", frombase);
	(void) concat(fullsource, sizeof(fullsource),
		      frombase, target, NULL);
    } else
	(void) strcpy(fullsource, fromfile);

    /*
     * check target
     */
    ptr = tstage_p->roots[0];
    if (*ptr == '/')
	(void) strcpy(tobase, ptr);
    else
	(void) concat(tobase, sizeof(tobase),
		      rcfile_release_base, "/", ptr, NULL);
    if (stat(tobase, &statb) < 0 || (statb.st_mode&S_IFMT) != S_IFDIR)
	fatal("%s is not a directory", tobase);
    (void) concat(fulltarget, sizeof(fulltarget), tobase, target, NULL);

    printf("[ %s -> %s ]\n",
	   fstage_p == NULL ? "BUILD" : fstage_p->names[0],
	   tstage_p->names[0]);
    /*
     * set flags based on target (all false on backups)
     */
    check = TRUE;
    if (rmflag && (tstage_p->flags&STAGE_NORM))
	rmflag = FALSE;

    /*
     * release to main target directory
     */
    do_release();

    /*
     * release to backup target directories
     */
    if (tstage_p->roots[1] != NULL) {
	fromfile = NULL;
	(void) strcpy(fullsource, fulltarget);
	(void) strcpy(frombase, tobase);
	check = FALSE;
	rmflag = FALSE;
	strip = FALSE;
	for (i = 1; tstage_p->roots[i] != NULL; i++) {
	    ptr = tstage_p->roots[i];
	    if (*ptr == '/')
		(void) strcpy(tobase, ptr);
	    else
		(void) concat(tobase, sizeof(tobase),
			      rcfile_release_base, "/", ptr, NULL);
	    if (stat(tobase, &statb) < 0 || (statb.st_mode&S_IFMT) != S_IFDIR)
		warn("%s is not a directory", tobase);
	    else {
		(void) concat(fulltarget, sizeof(fulltarget),
			      tobase, target, NULL);
		do_release();
	    }
	}
    }

    /*
     * restore flags
     */
    rmflag = saverm;
    strip = savestrip;
}
#endif	/* _BLD */

do_release()
{
    char targettmp[MAXPATHLEN];
#ifndef _BLD
    char fromlogtmp[MAXPATHLEN];
    char fromwhotmp[MAXPATHLEN];
    char tologtmp[MAXPATHLEN];
    char towhotmp[MAXPATHLEN];
#endif
    int status = 0;

    /*
     * generate any files that we will need later
     */
#ifndef _BLD
    if (fromfile == NULL) {
	(void) concat(fromlogdb, sizeof(fromlogdb),
		      frombase, "/.post.log", NULL);
	if (check) {
	    (void) concat(fromwhodb, sizeof(fromwhodb),
			  frombase, "/.installers.log", NULL);
	    (void) concat(fromlogtmp, sizeof(fromlogtmp),
			  fromlogdb, ".tmp", NULL);
	    (void) concat(fromwhotmp, sizeof(fromwhotmp),
			  fromwhodb, ".tmp", NULL);
	}
    }
    (void) concat(tologdb, sizeof(tologdb), tobase, "/.post.log", NULL);
    (void) concat(towhodb, sizeof(towhodb), tobase, "/.installers.log", NULL);
    (void) concat(tologtmp, sizeof(tologtmp), tologdb, ".tmp", NULL);
    (void) concat(towhotmp, sizeof(towhotmp), towhodb, ".tmp", NULL);
#endif
    (void) concat(targettmp, sizeof(targettmp), fulltarget, ".tmp", NULL);

    /*
     * check out source for release
     */
    status = check_source_and_targets();

    /*
     * check any files that we will need later
     */
    if (status == 0)
	status = check_files();

    /*
     * bail out here if any problems so far
     */

    if (status < 0)
	exit(1);
    if (status > 0)
	return;

    /*
     * We are committed at this point, the only errors possible should
     * be file errors.  Other errors should have been checked already.
     * First, do as much as possible to temporary files.
     */

#ifndef _BLD
    /*
     * make a new post log
     */
    if (status == 0 && check && fromfile == NULL)
	status = create_old_log_db(fromlogtmp);
    if (status == 0)
	status = create_new_log_db(tologtmp);

    /*
     * make a new installer log
     */
    if (status == 0 && check && fromfile == NULL)
	status = create_old_who_db(fromwhotmp);
    if (status == 0)
	status = create_new_who_db(towhotmp);
#endif	/* _BLD */

    /*
     * make a new target
     */
    if (status == 0)
	status = create_new_target(targettmp);

    /*
     * done as much as we can before making targets and links so
     * install new targets
     */
    if (status == 0)
	diag("%s: installing as %s", target, fulltarget);
#ifndef _BLD
    if (status == 0 && check && fromfile == NULL)
	status = install_old_dbs(fromlogtmp, fromwhotmp);
    if (status == 0)
	status = install_new_dbs(tologtmp, towhotmp);
    else {
	(void) unlink(fromlogtmp);
	(void) unlink(fromwhotmp);
    }
#endif
    if (status == 0)
	status = install_targets(targettmp);
#ifndef _BLD
    else {
	(void) unlink(tologtmp);
	(void) unlink(towhotmp);
    }
#endif
    if (status != 0)
	(void) unlink(targettmp);

    if (status < 0)
	exit(1);

#ifndef _BLD
    /*
     * clean up old stuff
     */
    if (status == 0 && rmflag)
	remove_old_sources();
#endif
}

check_source_and_targets()
{
    int i;
    int fd;
    struct stat tostatb;
    struct stat fromstatb;
    char temp[MAXPATHLEN];
    char *refbase = (fromfile != NULL) ? NULL : frombase;
    int status;

    while (lstat(fullsource, &fromstatb) < 0) {
	ewarn("stat %s", fullsource);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
    }

    if (((fromstatb.st_mode&S_IFMT) != S_IFREG) &&
                ((fromstatb.st_mode&S_IFMT) != S_IFLNK))
	fatal("%s is not a regular file", fullsource);
    source_tv[0].tv_sec = fromstatb.st_atime;
    source_tv[1].tv_sec = fromstatb.st_mtime;
    source_tv[0].tv_usec = source_tv[1].tv_usec = 0;
    while ((fd = open(fullsource, O_RDONLY, 0)) < 0) {
	ewarn("open %s", fullsource);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
    }
    (void) close(fd);

    /*
     * check out targets for release
     */
    if (lstat(fulltarget, &tostatb) < 0) {
	while (ckmakepath(fulltarget, refbase, target) != 0) {
	    warn("makepath failed for %s", fulltarget);
	    if ((status = getarc()) != 0) {
		diag("%s: aborting installation", fulltarget);
		return(status);
	    }
	}
    } else {
	if (((tostatb.st_mode&S_IFMT) != S_IFREG) &&
                ((tostatb.st_mode&S_IFMT) != S_IFLNK))
	    fatal("%s is not a regular file", fulltarget);
	if (fromstatb.st_dev == tostatb.st_dev &&
	    fromstatb.st_ino == tostatb.st_ino)
	    fatal("%s and %s are identical", fullsource, fulltarget);
    }
    for (i = 1; i < nlinks; i++) {
	(void) concat(temp, sizeof(temp), tobase, links[i], NULL);
	if (exists(temp) < 0) {
	    if (ckmakepath(temp, refbase, links[i]) != 0) {
		warn("makepath failed for %s", temp);
		if ((status = getarc()) != 0) {
		    diag("%s: aborting installation", fulltarget);
		    return(status);
		}
	    }
	}
    }

    return(0);
}

ckmakepath(pathname, refbase, refpath)
char *pathname, *refbase, *refpath;
{
    char dirbuf[MAXPATHLEN], filbuf[MAXPATHLEN];
    int status;

    path(pathname, dirbuf, filbuf);
    if (exists(dirbuf) == 0)
	return(0);
    if (refbase != NULL) {
	(void) concat(filbuf, sizeof(filbuf), refbase, refpath, NULL);
	refpath = filbuf;
    }
    if (exists(refpath) < 0)
	refpath = NULL;
/*
 * Changed refpath to NULL, so that the the running system
 * is not checked.
 *                   -------------
 *                               |
 *                               V
 */
    status = makepath(pathname, NULL, TRUE, TRUE);
    if (status != 0)
	return(status);
    if (exists(dirbuf) < 0)
	return(-1);
    return(0);
}

try_to_open(file, how, mode)
char *file;
int how, mode;
{
    int fd;
    int status;

    while ((fd = open(file, how, mode)) < 0) {
	ewarn("open %s", file);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
    }
    (void) close(fd);
    return(0);
}

check_files()
{
    char temp[MAXPATHLEN];
    int status;

#ifndef _BLD
    /*
     * check that all log files exist and are readable
     */
    if (fromfile == NULL) {
	if (exists(fromlogdb) < 0 &&
	    (status = try_to_open(fromlogdb,O_WRONLY|O_CREAT,0644)) != 0)
	    return(status);
	if (check && exists(fromwhodb) < 0 &&
	    (status = try_to_open(fromwhodb, O_WRONLY|O_CREAT, 0644)) != 0)
	    return(status);
	if ((status = try_to_open(fromlogdb, O_RDONLY, 0)) != 0)
	    return(status);
	if (check && (status = try_to_open(fromwhodb, O_RDONLY, 0)) != 0)
	    return(status);
    }
    if (exists(tologdb) < 0 &&
	(status = try_to_open(tologdb, O_WRONLY|O_CREAT, 0644)) != 0)
	return(status);
    if (exists(towhodb) < 0 &&
	(status = try_to_open(towhodb, O_WRONLY|O_CREAT, 0644)) != 0)
	return(status);
    if ((status = try_to_open(tologdb, O_RDONLY, 0)) != 0)
	return(status);
    if ((status = try_to_open(towhodb, O_RDONLY, 0)) != 0)
	return(status);
#endif	/* _BLD */

    /*
     * check that we can create files in target directories
     */
#ifndef _BLD
    if (check && fromfile == NULL) {
	(void) concat(temp, sizeof(temp), frombase, "/.release.chk", NULL);
	if ((status = try_to_open(temp, O_WRONLY|O_CREAT, 0644)) != 0)
	    return(status);
	(void) unlink(temp);
    }
#endif
    (void) concat(temp, sizeof(temp), tobase, "/.release.chk", NULL);
    if ((status = try_to_open(temp, O_WRONLY|O_CREAT, 0644)) != 0)
	return(status);
    (void) unlink(temp);
    (void) concat(temp, sizeof(temp), fulltarget, ".chk", NULL);
    if ((status = try_to_open(temp, O_WRONLY|O_CREAT, 0644)) != 0)
	return(status);
    (void) unlink(temp);
    return(0);
}

#ifndef	_BLD
keep_lines(filename, fd, kept)
char *filename;
int fd;
int *kept;
{
    FILE *fp;
    char buf[BUFSIZ];
    int status;
    int len;
    char *ptr;

    if ((fp = fopen(filename, "r")) == NULL) {
	ewarn("reopen %s", filename);
	return(1);
    }
    ptr = buf + targetlen;
    *kept = FALSE;
    while (fgets(buf, sizeof(buf)-1, fp) != NULL) {
	if (*buf != *target || bcmp(buf, target, targetlen) != 0 ||
	    (*ptr != ' ' && *ptr != '\t'))
	    continue;
	len = strlen(ptr+1);
	if ((status = write(fd, ptr+1, len)) != len) {
	    ewarn("write error (%d, not %d)", status, len);
	    (void) fclose(fp);
	    return(1);
	}
	*kept = TRUE;
    }
    if (ferror(fp) || fclose(fp) == EOF) {
	ewarn("error reading %s", filename);
	(void) fclose(fp);
	return(1);
    }
    return(0);
}

flush_lines(filename, fd)
char *filename;
int fd;
{
    FILE *fp;
    char buf[BUFSIZ];
    int status;
    int len;
    char *ptr;

    if ((fp = fopen(filename, "r")) == NULL) {
	ewarn("reopen %s", filename);
	return(1);
    }
    ptr = buf + targetlen;
    while (fgets(buf, sizeof(buf)-1, fp) != NULL) {
	if (*buf == *target && bcmp(buf, target, targetlen) == 0 &&
	    (*ptr == ' ' || *ptr == '\t')) {
	    continue;
	}
	len = strlen(buf);
	if ((status = write(fd, buf, len)) != len) {
	    ewarn("write error (%d, not %d)", status, len);
	    (void) fclose(fp);
	    return(1);
	}
    }
    if (ferror(fp) || fclose(fp) == EOF) {
	ewarn("error reading %s", filename);
	(void) fclose(fp);
	return(1);
    }
    return(0);
}

create_old_log_db(fromlogtmp)
char *fromlogtmp;
{
    int fd;
    int status;

    for (;;) {
	if ((fd = open(fromlogtmp, O_WRNEW, 0644)) < 0)
	    ewarn("open %s", fromlogtmp);
	else if (flush_lines(fromlogdb, fd) == 0) {
	    if (close(fd) < 0)
		ewarn("close %s", fromlogtmp);
	    else
		return(0);
	}
	(void) close(fd);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
	(void) unlink(fromlogtmp);
    }
}

create_new_log_db(tologtmp)
char *tologtmp;
{
    FILE *fp;
    char buf[BUFSIZ];
    int status;
    int fd;
    int len;
    int bufsiz;
    char *ptr;
    int errs;

    for (;;) {
	fp = NULL;
	if ((fd = open(tologtmp, O_WRNEW, 0644)) < 0)
	    ewarn("open %s", tologtmp);
	else if (flush_lines(tologdb, fd) == 0) {
	    if ((fp = fopen(logfile, "r")) == NULL)
		ewarn("reopen %s", logfile);
	    else {
		(void) strcpy(buf, target);
		ptr = buf + targetlen;
		*ptr++ = ' ';
		bufsiz = sizeof(buf) - targetlen - 2;
		errs = FALSE;
		while (fgets(ptr, bufsiz, fp) != NULL) {
		    len = strlen(ptr) + targetlen + 1;
		    if ((status = write(fd, buf, len)) != len) {
			ewarn("write error (%d, not %d)", status, len);
			errs = TRUE;
			break;
		    }
		}
		if (ferror(fp) || fclose(fp) == EOF)
		    ewarn("error reading %s", logfile);
		else if (close(fd) < 0)
		    ewarn("close %s", tologtmp);
		else if (!errs)
		    return(0);
	    }
	}
	if (fp != NULL)
	    (void) fclose(fp);
	(void) close(fd);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
	(void) unlink(tologtmp);
    }
}

create_old_who_db(fromwhotmp)
char *fromwhotmp;
{
    int fd;
    int status;

    for (;;) {
	if ((fd = open(fromwhotmp, O_WRNEW, 0644)) < 0)
	    ewarn("open %s", fromwhotmp);
	else if (flush_lines(fromwhodb, fd) == 0) {
	    if (close(fd) < 0)
		ewarn("close %s", fromwhotmp);
	    else
		return(0);
	}
	(void) close(fd);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
	(void) unlink(fromwhotmp);
    }
}

create_new_who_db(towhotmp)
char *towhotmp;
{
    struct tm *tm;
    time_t now;
    char datebuf[64];
    char *ptr, buf[BUFSIZ];
    int status;
    int fd;

    for (;;) {
	if ((fd = open(towhotmp, O_WRNEW, 0644)) < 0)
	    ewarn("open %s", towhotmp);
	else if (flush_lines(towhodb, fd) == 0) {
	    now = time((time_t *)0);
	    tm = localtime(&now);
	    fdate(datebuf, "%3Month %02day %02hour:%02min:%02sec %year", tm);
	    ptr = concat(buf, sizeof(buf),
			  target, "\t", user, "\t", datebuf, "\n", NULL);
	    if ((status = write(fd, buf, ptr-buf)) != ptr-buf)
		ewarn("write error (%d, not %d)", status, ptr-buf);
	    else if (close(fd) < 0)
		ewarn("close %s", towhotmp);
	    else
		return(0);
	}
	(void) close(fd);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
	(void) unlink(towhotmp);
    }
}
#endif	/* _BLD */

copy_link(src, dst, siz)
char *src, *dst;
off_t siz;
{
    int cc, status;
    char temp[MAXPATHLEN+1];

    for (;;) {
	if (siz >= sizeof(temp))
	    fatal("symlink %s too long", src);
	if ((cc = readlink(src, temp, sizeof(temp)-1)) < 0)
	    ewarn("readlink %s", src);
	else {
	    temp[cc] = '\0';
	    if (symlink(temp, dst) == 0)
		break;
	    ewarn("symlink %s", dst);
	}
	if ((status = getarc()) != 0)
	    return(status);
	(void) unlink(dst);
    }
    return(0);
}

copy_file(src, dst, fname, fullfname)
char *src, *dst;
char *fname, *fullfname;
{
    int sfd, tfd, status;

    for (;;) {
	for (;;) {
	    tfd = -1;
	    if ((sfd = open(src, O_RDONLY, 0)) < 0)
		ewarn("reopen %s", src);
	    else if ((tfd = open(dst, O_RDWR|O_CREAT|O_EXCL, 0600)) < 0)
		ewarn("reopen %s", dst);
	    else if (filecopy(sfd, tfd) < 0)
		ewarn("filecopy %s to %s failed", src, dst);
	    else if (close(sfd) < 0)
		ewarn("close %s", src);
	    else
		break;
	    if (sfd >= 0)
		(void) close(sfd);
	    if (tfd >= 0)
		(void) close(tfd);
	    if ((status = getarc()) != 0)
		return(status);
	    (void) unlink(dst);
	}
/* We want to call stripfile(). Use -nostrip to disable stripping if desired. */
/*#ifndef _BLD*/
	while (stripfile(target, tfd, dst) != 0) {
	    if ((status = getarc()) < 0) {
		(void) close(tfd);
		return(status);
	    }
	    if (status != 0)
		break;
	}
/*#endif*/
	diag("%s: owner %s, group %s, mode %#o", fname, owner, group, mode);
#ifdef _BLD
	while (fchown(tfd, uid, gid) < 0) {
#else
	while (fchown(tfd, uid, gid) < 0 && check) {
#endif
	    ewarn("chown %s", fullfname);
	    if ((status = getarc()) != 0) {
		(void) close(tfd);
		return(status);
	    }
	}
	while (fchmod(tfd, (int) mode) < 0) {
	    ewarn("chmod %s", fullfname);
	    if ((status = getarc()) < 0) {
		(void) close(tfd);
		return(status);
	    }
	    if (status != 0)
		break;
	}
	while (utimes(dst, source_tv) < 0) {
	    ewarn("utimes %s", fullfname);
	    if ((status = getarc()) < 0) {
		(void) close(tfd);
		return(status);
	    }
	    if (status != 0)
		break;
	}
	if (close(tfd) == 0)
	    break;
	ewarn("close %s", fullfname);
	if ((status = getarc()) != 0) {
	    (void) close(tfd);
	    return(status);
	}
    }
    return(0);
}

create_new_target(targettmp)
char *targettmp;
{
    struct stat statb;
    int status;

    while (lstat(fullsource, &statb) < 0) {
	ewarn("lstat %s", fullsource);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
    }
    if ((statb.st_mode&S_IFMT) == S_IFLNK)
	status = copy_link(fullsource, targettmp, statb.st_size);
    else
	status = copy_file(fullsource, targettmp, target, fulltarget);
    if (status != 0)
	diag("%s: aborting installation", fulltarget);
    return(status);
}


stripfile(file, fd, fname)
char *file;
register int fd;
char *fname;
{
#if	defined(COFF) || defined(MACHO)
    int pid, status, w;
    struct {
	struct filehdr fhdr;
	struct aouthdr ahdr;
    } hdrs;
    char scmd[MAXPATHLEN];	/* Not perfect, but plenty big enough. */

    if (!strip)
	return(0);
    (void) lseek(fd, (long)0, L_SET);
    if (read(fd, (char *)&hdrs, sizeof(hdrs)) != sizeof(hdrs))
	return(0);
#if defined(mips) || defined(alpha)
    if (! ISCOFF(hdrs.fhdr.f_magic))
#endif
#ifdef multimax
    if (! ISMAGIC(hdrs.fhdr.f_magic))
#endif
	return(0);
    if ((hdrs.fhdr.f_flags & F_RELFLG) == 0)
	return 0;	/* Relocation information present in file */
#ifdef multimax
    if (hdrs.ahdr.magic != AOUT1MAGIC && hdrs.ahdr.magic != AOUT2MAGIC &&
	hdrs.ahdr.magic != PAGEMAGIC)
	return 0;
#endif
    if (hdrs.fhdr.f_symptr == 0) {
	diag("%s: already stripped", file);
	return(0);
    }
#ifdef	_BLD
	/*
	 * We use system in stead of fork/exec, because system calls the
	 * shell, which obeys the PATH environment variable, which allows
	 * us to use tools/pmax/mcc/strip instead of /bin/strip.
	 */
	sprintf(scmd, "strip %s", fname);
	status = system(scmd);
	if (status == -1) {
		ewarn("system strip failed");
		return(1);
	}
#else
    if ((pid = vfork()) == -1) {
	ewarn("strip vfork failed");
	return(1);
    }
    if (pid == 0) {
	execl("/bin/strip", "strip", fname, 0);
	_exit(127);
    }
    do {
	w = wait(&status);
    } while (w != pid && w != -1);
    if (w != pid)
	return(1);
#endif	/* _BLD */
    return(0);
#else	/* defined(COFF) || defined(MACHO) */
    long size;
    struct exec head;

    if (!strip)
	return(0);
    (void) lseek(fd, (long)0, L_SET);
    if (read(fd, (char *)&head, sizeof(head)) != sizeof(head))
	return(0);
    if (N_BADMAG(head))
	return(0);
    if ((head.a_syms == 0) && (head.a_trsize == 0) && (head.a_drsize ==0)) {
	diag("%s: already stripped", file);
	return(0);
    }
    head.a_syms = head.a_trsize = head.a_drsize = 0;
    size = N_SYMOFF(head);
    diag("%s: stripping", file);
    if (ftruncate(fd, size) < 0) {
	ewarn("ftruncate %s", file);
	return(1);
    }
    (void) lseek(fd, (long)0, L_SET);
    (void) write(fd, (char *)&head, sizeof(head));
    return(0);
#endif	/* defined(COFF) || defined(MACHO) */
}

#ifndef _BLD
install_old_dbs(fromlogtmp, fromwhotmp)
char *fromlogtmp, *fromwhotmp;
{
    int status;

    while (rename(fromlogtmp, fromlogdb) < 0) {
	ewarn("rename %s to %s failed", fromlogtmp, fromlogdb);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    if (status < 0)
		return(status);
	    break;
	}
    }
    while (rename(fromwhotmp, fromwhodb) < 0) {
	ewarn("rename %s to %s failed", fromwhotmp, fromwhodb);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    if (status < 0)
		return(status);
	    break;
	}
    }
    return(0);
}

install_new_dbs(tologtmp, towhotmp)
char *tologtmp, *towhotmp;
{
    int status;

    while (rename(tologtmp, tologdb) < 0) {
	ewarn("rename %s to %s failed", tologtmp, tologdb);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    if (status < 0)
		return(status);
	    break;
	}
    }
    while (rename(towhotmp, towhodb) < 0) {
	ewarn("rename %s to %s failed", towhotmp, towhodb);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    if (status < 0)
		return(status);
	    break;
	}
    }
    return(0);
}
#endif	/* _BLD */

install_targets(targettmp)
char *targettmp;
{
    int i;
    int status;
    char temp[MAXPATHLEN];

    while (rename(targettmp, fulltarget) < 0) {
	ewarn("rename %s to %s failed", targettmp, fulltarget);
	if ((status = getarc()) != 0) {
	    diag("%s: aborting installation", fulltarget);
	    return(status);
	}
    }
    for (i = 1; i < nlinks; i++) {
	(void) concat(temp, sizeof(temp), tobase, links[i], NULL);
	for (;;) {
	    diag("%s: linking to %s", fulltarget, temp);
	    (void) unlink(temp);
	    if (link(fulltarget, temp) == 0)
		break;
	    ewarn("link %s to %s failed", fulltarget, temp);
	    if ((status = getarc()) != 0) {
		diag("%s: incomplete installation", fulltarget);
		if (status < 0)
		    return(status);
		break;
	    }
	}
    }
    return(0);
}

#ifndef	_BLD
remove_old_sources()
{
    int i;
    char temp[MAXPATHLEN];

    /* remove source release targets */
    if (unlink(fullsource) == 0)
	diag("%s: being removed", fullsource);
    if (fromfile != NULL)
	return;
    for (i = 1; i < nlinks; i++) {
	(void) concat(temp, sizeof(temp), frombase, links[i], NULL);
	if (unlink(temp) == 0)
	    diag("%s: being removed", temp);
    }
}

make_post(stage_p)
struct stage *stage_p;
{
    char *ptr;
    char *av[16];
    struct stat statb;
    char subject[BUFSIZ];
    int i;
    int fd;
    FILE *fp;
    enum option command;
    char *defans;
    char notify[BUFSIZ];
    int child;
    int pid, omask;
    union wait w;

    if (!post || (stage_p->flags&STAGE_NOPOST) != 0)
	return;

    diag("\nPlease update the following message, if necessary.");
    diag("Your name and a default subject will be added automatically.");

    /*
     * default subject is tail of tobase and target
     */
    ptr = rindex(stage_p->roots[0], '/');
    if (ptr != NULL)
	ptr++;
    else
	ptr = stage_p->roots[0];
    (void) concat(subject, sizeof(subject), ptr, target, NULL);

    /*
     * default address for notification comes from the environment
     */
    if ((ptr = getenv("NOTIFICATION")) == NULL)
	*notify = '\0';
    else
	(void) concat(notify, sizeof(notify), ptr, NULL);

    /*
     * copy log file to post template and stdout
     */
    command = TYPE;

    /*
     * parse commands
     */
    for (;;) {
	switch (command) {
	case EDIT:
	    (void) editor(logfile, "Entering editor.");
	    defans = "type";
	    break;
	case TYPE:
	    if ((fp = fopen(logfile, "r")) == NULL)
		break;
	    if (*notify != '\0')
		printf("\nTo: %s", notify);
	    printf("\nSubject: %s\n", subject);
	    (void) ffilecopy(fp, stdout);
	    (void) fclose(fp);
	    defans = "post";
	    break;
	case NOTIFY:
	    (void) getstr("To", notify, notify);
	    defans = "type";
	    break;
	case SUBJECT:
	    (void) getstr("Subject", subject, subject);
	    defans = "type";
	    break;
	case POST:
	    defans = NULL;
	    break;
	case LOG:
	    return;
	}
	if (defans == NULL) {
	    if (stat(logfile, &statb) == 0 && statb.st_size > 0) {
		if (*notify != '\0')
		    break;
		diag("You have not specified anyone to notify");
		command = NOTIFY;
	    } else {
		diag("You are not allowed to post an empty message");
		command = EDIT;
	    }
	} else {
	    if (fast)
		return;
	    command = postopt[getstab("Edit, type, notify, subject, post, log",
				      posttab, defans)];
	    clearerr(stdin);
	}
    }

    if (addheaders(subject, notify, logfile) < 0)
	ewarn("trouble adding mail headers");
    if ((fd = open(logfile, O_RDONLY, 0)) < 0) {
	ewarn("open %s", logfile);
	diag("Unable to make post");
	return;
    }
    i = 0;
    av[i++] = "sendmail";
    av[i++] = "-t";
    av[i++] = NULL;
    if ((child = vfork()) == -1) {
	ewarn("post vfork failed");
	diag("Unable to make post");
	return;
    }
    if (child == 0) {
	(void) setgid(getgid());
	(void) setuid(getuid());
	if (fd >= 0 && fd != 0) {
	    (void) dup2(fd, 0);
	    (void) close(fd);
	}
	execv("/usr/lib/sendmail", av);
	exit(0377);
    }
    (void) close(fd);
    omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
    do {
	pid = wait3(&w, WUNTRACED, (struct rusage *)NULL);
	if (WIFSTOPPED(w)) {
	    (void) kill(0, SIGTSTP);
	    pid = 0;
	}
    } while (pid != child && pid != -1);
    (void) sigsetmask(omask);
    if (pid == -1) {
	ewarn("wait");
	diag("Unable to make post");
    } else if (WIFSIGNALED(w) || w.w_retcode == 0377) {
	warn("post killed");
	diag("Unable to make post");
    } else if (w.w_retcode != 0) {
	warn("post exited with status %d", w.w_retcode);
	diag("Unable to make post");
    }
    return;
}

/*
 * build a user's fullname from the pw_gecos field
 */
char *
buildfname(p, login)
register char *p;
char *login;
{
    static char buf[64];
    register char *bp = buf+1;

    if (*p == '*')
	p++;
    while (*p != '\0' && *p != ',' && *p != ';' && *p != '%') {
	if (*p == '&') {
	    (void) strcpy(bp, login);
	    if (islower(*bp))
		*bp = toupper(*bp);
	    while (*bp != '\0')
		bp++;
	    p++;
	} else
	    *bp++ = *p++;
    }
    *bp = '\0';
    if (index(buf+1, '.') == NULL)
	return buf+1;
    *bp++ = *buf = '"';
    *bp = '\0';
    return buf;
}


/*
 * add headers to the beginning of logfile for sendmail
 */
addheaders(subject, recipients, logfile)
char *subject, *recipients, *logfile;
{
    int fd, uid;
    char *up;
    struct passwd *pw;
    FILE *fp, *lp;
    char progname[NAME_MAX+1];
    char hostname[MAXHOSTNAMELEN+1];
    char temp[MAXPATHLEN+1];

    (void) strcpy(progname, prog);
    if (islower(*progname))
	*progname = toupper(*progname);
    pw = (((up = getenv("USER")) && *up)
	|| ((up = getlogin()) && *up))
	    ? getpwnam(up) : NULL;
    if (pw == NULL && (pw = getpwuid(uid = getuid())) == NULL)
	fatal("uid %d not found in passwd database", uid);
    hostname[MAXHOSTNAMELEN] = '\0';
    if (gethostname(hostname, MAXHOSTNAMELEN) < 0)
	efatal("gethostname");
    (void) strcpy(temp, "/tmp/release.XXXXXX");
    if ((fd = mkstemp(temp)) < 0)
	return(-1);
    if ((fp = fdopen(fd, "w")) == NULL) {
	(void) close(fd);
	return(-1);
    }
    if ((lp = fopen(logfile, "r")) == NULL) {
	(void) fclose(fp);
	return(-1);
    }
    fprintf(fp, "From: %s <%s@%s>\n",
		buildfname(pw->pw_gecos, pw->pw_name),
		pw->pw_name, hostname);
    if ((pw = getpwuid(geteuid())) == NULL)
	pw->pw_name = "root";
    fprintf(fp, "Sender: %s <%s@%s>\n", progname, pw->pw_name, hostname);
    fprintf(fp, "Subject: %s\n", subject);
    fprintf(fp, "To: %s\n\n", recipients);
    (void) ffilecopy(lp, fp);
    (void) fclose(lp);
    (void) fclose(fp);
    if (rename(temp, logfile) < 0) {
	(void) unlink(temp);
	return(-1);
    }
    return(0);
}
#endif	/* _BLD */

static
_gettemp(as, doopen)
char *as;
register int *doopen;
{
    extern int errno;
    register char *start, *trv;
    struct stat sbuf;
    u_int pid;

    pid = getpid();

    /* extra X's get set to 0's */
    for (trv = as; *trv; ++trv);
    while (*--trv == 'X') {
	*trv = (pid % 10) + '0';
	pid /= 10;
    }

    /*
     * check for write permission on target directory; if you have
     * six X's and you can't write the directory, this will run for
     * a *very* long time.
     */
    for (start = ++trv; trv > as && *trv != '/'; --trv);
    if (*trv == '/') {
	*trv = '\0';
	if (stat(as, &sbuf) || !(sbuf.st_mode & S_IFDIR))
	    return(FALSE);
	*trv = '/';
    }
    else if (stat(".", &sbuf) == -1)
	return(FALSE);

    for (;;) {
	if (doopen) {
	    if ((*doopen = open(as, O_CREAT|O_EXCL|O_RDWR, 0600)) >= 0)
		return(TRUE);
	    if (errno != EEXIST)
		return(FALSE);
	}
	else if (stat(as, &sbuf))
	    return(errno == ENOENT ? TRUE : FALSE);

	/* tricky little algorithm for backward compatibility */
	for (trv = start;;) {
	    if (!*trv)
		return(FALSE);
	    if (*trv == 'z')
		*trv++ = 'a';
	    else {
		if (isdigit(*trv))
		    *trv = 'a';
		else
		    ++*trv;
		break;
	    }
	}
    }
    /*NOTREACHED*/
}

mkstemp(as)
char *as;
{
    int fd;

    return (_gettemp(as, &fd) ? fd : -1);
}

char *
mktemp(as)
char *as;
{
    return(_gettemp(as, (int *)NULL) ? as : (char *)NULL);
}

/*
 * error routines
 */

/*VARARGS1*/
void
#ifdef __STDC__
diag(char *fmt, ...)
#else
diag(fmt, va_alist)
char *fmt;
va_dcl
#endif
{
    va_list vargs;

    (void) fflush(stdout);
#ifdef __STDC__
    va_start(vargs, fmt);
#else
    va_start(vargs);
#endif
#ifdef _BLD
    (void) _doprnt(fmt, vargs, stderr);
#else
    (void) vfprintf(stderr, fmt, vargs);
#endif
    va_end(vargs);
    (void) putc('\n', stderr);
}

/*VARARGS1*/
void
#ifdef __STDC__
ewarn(char *fmt, ...)
#else
ewarn(fmt, va_alist)
char *fmt;
va_dcl
#endif
{
    va_list vargs;
    int serrno = errno;

    (void) fflush(stdout);
    (void) fputs(prog, stderr);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
#ifdef __STDC__
    va_start(vargs, fmt);
#else
    va_start(vargs);
#endif
#ifdef _BLD
    (void) _doprnt(fmt, vargs, stderr);
#else
    (void) vfprintf(stderr, fmt, vargs);
#endif
    va_end(vargs);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
    (void) fputs(errmsg(serrno), stderr);
    (void) putc('\n', stderr);
}

/*VARARGS1*/
void
#ifdef __STDC__
efatal(char *fmt, ...)
#else
efatal(fmt, va_alist)
char *fmt;
va_dcl
#endif
{
    va_list vargs;
    int serrno = errno;

    (void) fflush(stdout);
    (void) fputs(prog, stderr);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
#ifdef __STDC__
    va_start(vargs, fmt);
#else
    va_start(vargs);
#endif
#ifdef _BLD
    (void) _doprnt(fmt, vargs, stderr);
#else
    (void) vfprintf(stderr, fmt, vargs);
#endif
    va_end(vargs);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
    (void) fputs(errmsg(serrno), stderr);
    (void) putc('\n', stderr);
    exit(1);
}

/*VARARGS1*/
void
#ifdef __STDC__
warn(char *fmt, ...)
#else
warn(fmt, va_alist)
char *fmt;
va_dcl
#endif
{
    va_list vargs;

    (void) fflush(stdout);
    (void) fputs(prog, stderr);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
#ifdef __STDC__
    va_start(vargs, fmt);
#else
    va_start(vargs);
#endif
#ifdef _BLD
    (void) _doprnt(fmt, vargs, stderr);
#else
    (void) vfprintf(stderr, fmt, vargs);
#endif
    va_end(vargs);
    (void) putc('\n', stderr);
}

/*VARARGS1*/
void
#ifdef __STDC__
fatal(char *fmt, ...)
#else
fatal(fmt, va_alist)
char *fmt;
va_dcl
#endif
{
    va_list vargs;

    (void) fflush(stdout);
    (void) fputs(prog, stderr);
    (void) putc(':', stderr);
    (void) putc(' ', stderr);
#ifdef __STDC__
    va_start(vargs, fmt);
#else
    va_start(vargs);
#endif
#ifdef _BLD
    (void) _doprnt(fmt, vargs, stderr);
#else
    (void) vfprintf(stderr, fmt, vargs);
#endif
    va_end(vargs);
    (void) putc('\n', stderr);
    exit(1);
}

unsigned int atoo(ap)
char *ap;
{
	register unsigned int n;
	register char *p;

	p = ap;
	n = 0;
	while(*p == ' ' || *p == '	')
		p++;
	while(*p >= '0' && *p <= '7')
		n = n * 8 + *p++ - '0';
	return(n);
}

#ifdef _BLD
char *malloc();

char *salloc(p)
char *p;
{
	register char *q;
	register int l;

	q = malloc(l = strlen(p) + 1);
	if (q != 0)
		bcopy(p, q, l);
	return(q);
}

char *
vconcat(buf, buflen, ap)
char *buf;
int buflen;
va_list ap;
{
    register char *arg, *ptr, *ep;

    if (buf == NULL)
	return(NULL);
    if (buflen <= 0)
	return(NULL);
    ptr = buf;
    *ptr = '\0';
    ep = buf + buflen;
    while (ptr != NULL && (arg = va_arg(ap, char *)) != NULL)
	while (*ptr = *arg++)
	    if (++ptr == ep) {
		ptr = NULL;
		break;
	    }
    return(ptr);
}

/*VARARGS2*/
char *
#ifdef __STDC__
concat(char *buf, int buflen, ...)
#else
concat(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
    char *buf;
    int buflen;
#endif
    va_list ap;
    char *ptr;

#ifdef __STDC__
    va_start(ap, buflen);
#else
    va_start(ap);
    buf = va_arg(ap, char *);
    buflen = va_arg(ap, int);
#endif
    ptr = vconcat(buf, buflen, ap);
    va_end(ap);
    return(ptr);
}

path (original,direc,file)
char *original,*direc,*file;
{
	register char *y;
	/* x is direc */
	register char *p;

	/* copy and note the end */
	p = original;
	y = direc;
	while (*y++ = *p++) ;		/* copy string */
	/* y now points to first char after null */
	--y;	/* y now points to null */
	--y;	/* y now points to last char of string before null */

	/* chop off trailing / except as first character */
	while (y>direc && *y == '/') --y;	/* backpedal past / */
	/* y now points to char before first trailing / or null */
	*(++y) = 0;				/* chop off end of string */
	/* y now points to null */

	/* find last /, if any.  If found, change to null and bump y */
	while (y>direc && *y != '/') --y;
	/* y now points to / or direc.  Note *direc may be / */
	if (*y == '/') {
		*y++ = 0;
	}

	/* find file name part */
	if (*y)  strcpy (file,y);
	else     strcpy (file,".");

	/* find directory part */
	if (direc == y)        strcpy (direc,".");
	else if (*direc == 0)  strcpy (direc,"/");
	/* else direc already has proper value */
}

static
char *fixpath(path, buf)
register char *path;
char *buf;
{
    register char *ls = NULL;
    register char *p = buf;

    *p = *path;
    while (*path != '\0') {
	path++;
	while (*p == '/' && *path == '/')
	    path++;
	*++p = *path;
	if (*p == '/')
	    ls = p;
    }
    return(ls);
}

makepath(path, refpath, trace, showerrs)
char *path, *refpath;
int trace, showerrs;
{
    char pathbuf[MAXPATHLEN], refpathbuf[MAXPATHLEN];
    char linkbuf[MAXPATHLEN], linkbuf2[MAXPATHLEN];
    char *base, *refbase;
    char *slash, *refslash;
    struct stat statb, refstatb;
    int ch, cc, cc2;
    extern uid_t getuid();
    uid_t uid = getuid();

    if (path == NULL) {
	if (showerrs)
	    fprintf(stderr, "makepath: NULL path argument\n");
	return(1);
    }

    base = fixpath(path, pathbuf);

    if (refpath == NULL) {
	if (base == NULL || base == pathbuf) {
	    if (showerrs)
		fprintf(stderr,
			"makepath: %s must have an imbedded '/' character\n",
			pathbuf);
	    return(1);
	}
	*base = '\0';
	base = pathbuf;
	if (*base == '/')
	    base++;
	if (*base == '\0') {
	    if (showerrs)
		fprintf(stderr, "makepath: illegal pathname %s\n", pathbuf);
	    return(1);
	}
	for (;;) {
	    /* find end of this component */
	    while (*base != '\0' && *base != '/')
		base++;

	    /* create path so far, if necessary */
	    ch = *base; *base = '\0';
	    if (stat(pathbuf, &statb) < 0) {
		if (mkdir(pathbuf, 0777) < 0) {
		    if (showerrs)
			fprintf(stderr,
				"makepath: unable to create directory %s: %s\n",
				pathbuf, errmsg(-1));
		    return(1);
		}
		if (stat(pathbuf, &statb) < 0) {
		    if (showerrs)
			fprintf(stderr,
				"%s: unable to stat directory %s: %s\n",
				pathbuf, errmsg(-1));
		    return(1);
		}
		if (trace)
		    fprintf(stderr, "%s: created directory\n", pathbuf);
	    } else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
		if (showerrs)
		    fprintf(stderr,
			    "makepath: %s is not a directory (mode %#o)\n",
			    pathbuf, (statb.st_mode&S_IFMT));
		return(1);
	    }
	    if (ch == '\0')
		break;
	    *base++ = ch;
	}
	return(0);
    }

    refbase = fixpath(refpath, refpathbuf);

    /* if not a directory path, strip final component */
    if (stat(refpathbuf, &refstatb) < 0 ||
	(refstatb.st_mode&S_IFMT) != S_IFDIR) {
	if (base == NULL || base == pathbuf) {
	    if (showerrs)
		fprintf(stderr,
			"makepath: %s must have an imbedded '/' character\n",
			pathbuf);
	    return(1);
	}
	if (refbase == NULL || refbase == refpathbuf) {
	    if (showerrs)
		fprintf(stderr,
			"makepath: %s must have an imbedded '/' character\n",
			refpathbuf);
	    return(1);
	}
	if (strcmp(base, refbase) != 0) {
	    if (showerrs)
		fprintf(stderr,
			"makepath: %s and %s do not have common trailing components\n",
			base, refbase);
	    return(1);
	}
	*base = *refbase = '\0';
	if (stat(refpathbuf, &refstatb) < 0) {
	    if (showerrs)
		fprintf(stderr,
			"%s: unable to stat reference directory %s: %s\n",
			refpathbuf, errmsg(-1));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) != S_IFDIR) {
	    if (showerrs)
		fprintf(stderr, "makepath: %s is not a directory (mode %#o)\n",
			refpathbuf, (refstatb.st_mode&S_IFMT));
	    return(1);
	}
    }

    /* find beginning of common part of path names */
    base = pathbuf + strlen(pathbuf);
    refbase = refpathbuf + strlen(refpathbuf);
    while (*base == *refbase) {
	if (base == pathbuf || refbase == refpathbuf)
	    break;
	base--; refbase--;
    }
    if (*base == *refbase && *base != '/') {
	if (base == pathbuf && *(refbase-1) == '/') {
	    base = pathbuf + strlen(pathbuf) + 2;
	    do {
		*base = *(base - 2);
	    } while (base-- > pathbuf + 2);
	    *(base-1) = '.';
	    *base = '/';
	    refbase--;
	}
	if (refbase == refpathbuf && *(base-1) == '/') {
	    refbase = refpathbuf + strlen(refpathbuf) + 2;
	    do {
		*refbase = *(refbase - 2);
	    } while (refbase-- > refpathbuf + 2);
	    *(refbase-1) = '.';
	    *refbase = '/';
	    base--;
	}
    }
    while (*base != '\0' && *base != '/') {
	base++; refbase++;
    }
    slash = base++;
    refslash = refbase++;
    ch = *slash; *slash = '\0';
    if (stat(pathbuf, &statb) < 0) {
	if (showerrs)
	    fprintf(stderr,
		    "makepath: unable to stat target directory %s: %s\n",
		    pathbuf, errmsg(-1));
	return(1);
    }
    if ((statb.st_mode&S_IFMT) != S_IFDIR) {
	if (showerrs)
	    fprintf(stderr, "makepath: %s: invalid mode %#o\n",
		    pathbuf, (statb.st_mode&S_IFMT));
	return(1);
    }
    *slash = ch;

    /* check each component along common path and make them the same */
    while (ch != '\0') {
	/* find end of this component */
	while (*base != '\0' && *base != '/') {
	    base++; refbase++;
	}

	/* get stat information for source path */
	ch = *refbase; *refbase = '\0'; *base = '\0';
	if (stat(refpathbuf, &refstatb) < 0) {
	    if (showerrs)
		fprintf(stderr, "makepath: stat %s: %s\n",
			refpathbuf, errmsg(-1));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) != S_IFDIR) {
	    if (showerrs)
		fprintf(stderr, "makepath: %s: invalid mode %#o\n",
			refpathbuf, (refstatb.st_mode&S_IFMT));
	    return(1);
	}
	if (lstat(refpathbuf, &refstatb) < 0) {
	    if (showerrs)
		fprintf(stderr, "makepath: lstat %s: %s\n",
			refpathbuf, errmsg(-1));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) == S_IFLNK) {
	    if ((cc = readlink(refpathbuf, linkbuf, sizeof(linkbuf)-1)) < 0) {
		if (showerrs)
		    fprintf(stderr, "makepath: readlink %s: %s\n",
			    refpathbuf, errmsg(-1));
		return(1);
	    }
	    if (cc > 0 && *linkbuf != '/') {
		*refbase = ch;
		linkbuf[cc] = '\0';
		if (lstat(pathbuf, &statb) < 0) {
		    if (symlink(linkbuf, pathbuf) < 0) {
			if (showerrs)
			    fprintf(stderr, "makepath: symlink %s: %s\n",
				    pathbuf, errmsg(-1));
			return(1);
		    }
		    if (trace)
			fprintf(stderr, "%s: created symlink to %s\n",
				pathbuf, linkbuf);
		} else {
		    if ((statb.st_mode&S_IFMT) != S_IFLNK) {
			if (showerrs)
			    fprintf(stderr, "makepath: %s: invalid mode %#o\n",
				    pathbuf, (statb.st_mode&S_IFMT));
			return(1);
		    }
		    cc2 = readlink(pathbuf, linkbuf2, sizeof(linkbuf2)-1);
		    if (cc2 < 0) {
			if (showerrs)
			    fprintf(stderr, "makepath: readlink %s: %s\n",
				    pathbuf, errmsg(-1));
			return(1);
		    }
		    if (cc != cc2 || bcmp(linkbuf, linkbuf2, cc) != 0) {
			if (showerrs)
			    fprintf(stderr,
				    "makepath: symlinks %s and %s differ\n",
				    refpathbuf, pathbuf, errmsg(-1));
			return(1);
		    }
		}
		(void) strcpy(linkbuf+cc, refbase);
		(void) strcpy(refslash+1, linkbuf);
		(void) strcpy(slash+1, linkbuf);
		(void) fixpath(refslash, refslash);
		(void) fixpath(slash, slash);
		refbase = refslash+1;
		base = slash+1;
		ch = *refslash;
		continue;
	    }
	}

	/* create path so far, if necessary */
	if (lstat(pathbuf, &statb) < 0) {
	    if (mkdir(pathbuf, (int)(refstatb.st_mode&07777)) < 0) {
		if (showerrs)
		    fprintf(stderr, "makepath: mkdir %s: %s\n",
			    pathbuf, errmsg(-1));
		return(1);
	    }
	    if (stat(pathbuf, &statb) < 0) {
		if (showerrs)
		    fprintf(stderr, "makepath: stat %s: %s\n",
			    pathbuf, errmsg(-1));
		return(1);
	    }
	    if (trace)
		fprintf(stderr, "%s: created directory\n", pathbuf);
	} else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
	    if (showerrs)
		fprintf(stderr, "makepath: %s: invalid mode %#o\n",
			pathbuf, (statb.st_mode&S_IFMT));
	    return(1);
	}

	if (uid == 0 && (statb.st_uid != refstatb.st_uid ||
			 statb.st_gid != refstatb.st_gid)) {
	    (void) chown(pathbuf, (int)refstatb.st_uid, (int)refstatb.st_gid);
	    if (trace)
		fprintf(stderr, "%s: owner %d, group %d\n", pathbuf,
			refstatb.st_uid, refstatb.st_gid);
	}
	if ((statb.st_mode&07777) != (refstatb.st_mode&07777)) {
	    (void) chmod(pathbuf, (int)(refstatb.st_mode&07777));
	    if (trace)
		fprintf(stderr, "%s: mode %#o\n", pathbuf,
			refstatb.st_mode&07777);
	}

	refslash = refbase;
	*refbase++ = ch;
	slash = base;
	*base++ = ch;
    }
    return(0);
}

#define BUFFERSIZE 10240

int filecopy (here,there)
int here,there;
{
	register int kount;
	char buffer[BUFFERSIZE];
	kount = 0;
	while (kount == 0 && (kount=read(here,buffer,BUFFERSIZE)) > 0)
		kount -= write (there,buffer,kount);
	return (kount ? -1 : 0);
}

extern int	errno;
extern int	sys_nerr;
extern char	*sys_errlist[];

static char *itoa(p,n)
char *p;
unsigned n;
{
    if (n >= 10)
	p =itoa(p,n/10);
    *p++ = (n%10)+'0';
    return(p);
}

char *errmsg(cod)
int cod;
{
	static char unkmsg[] = "Unknown error ";
	static char unk[sizeof(unkmsg)+11];		/* trust us */

	if (cod < 0) cod = errno;

	if((cod >= 0) && (cod < sys_nerr))
	    return(sys_errlist[cod]);

	strcpy(unk,unkmsg);
	*itoa(&unk[sizeof(unkmsg)-1],cod) = '\0';

	return(unk);
}


char _argbreak;
char *skipto();

char *nxtarg (q,brk)
char **q,*brk;
{
	register char *front,*back;
	front = *q;			/* start of string */
	/* leading blanks and tabs */
	while (*front && (*front == ' ' || *front == '\t')) front++;
	/* find break character at end */
	if (brk == 0)  brk = " ";
	back = skipto (front,brk);
	_argbreak = *back;
	*q = (*back ? back+1 : back);	/* next arg start loc */
	/* elim trailing blanks and tabs */
	back -= 1;
	while ((back >= front) && (*back == ' ' || *back == '\t')) back--;
	back++;
	if (*back)  *back = '\0';
	return (front);
}


static unsigned char tab[256] = {
	0};

char *skipto (string,charset)
unsigned char *string, *charset;
{
	register unsigned char *setp,*strp;

	tab[0] = 1;		/* Stop on a null, too. */
	for (setp=charset;  *setp;  setp++) tab[*setp]=1;
	for (strp=string;  tab[*strp]==0;  strp++)  ;
	for (setp=charset;  *setp;  setp++) tab[*setp]=0;
	return ((char *)strp);
}

char *skipover (string,charset)
unsigned char *string, *charset;
{
	register unsigned char *setp,*strp;

	tab[0] = 0;		/* Do not skip over nulls. */
	for (setp=charset;  *setp;  setp++) tab[*setp]=1;
	for (strp=string;  tab[*strp];  strp++)  ;
	for (setp=charset;  *setp;  setp++) tab[*setp]=0;
	return ((char *)strp);
}

#endif /* _BLD */
