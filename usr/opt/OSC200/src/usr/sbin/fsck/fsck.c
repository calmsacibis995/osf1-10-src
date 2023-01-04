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
static char	*sccsid = "@(#)$RCSfile: fsck.c,v $ $Revision: 4.3.2.3 $ (DEC) $Date: 1992/12/18 16:38:42 $";
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

/*
 * fsck
 *
 * This is the front-end to the various fsck programs.  To support a new
 * file system, enter an entry for the file system into the configuration
 * record below.  The entry contains the name of the file system type
 * (e.g. "ufs"), the pathname of the fsck program (e.g. "/sbin/ufs_fsck")
 * and the getopt() string for the fsck program (e.g. "b:cdl:m:NnpYy").
 * There is a default fsck which should be the first entry in the configuration
 * record.  It is also denoted by the #define DEFAULT_FS_TYPE.  The root
 * filesystem should be of the same type as the default fsck.
 *
 * The -T and -t options are special to this front-end.  Do not use
 * them in any fsck program.  This front treats -s and -S special to
 * conform with their use for the System V file system.
 */

#include <standards.h>
#include <stdio.h>
#include <fstab.h>
#include <assert.h>
#include <sys/wait.h>

/*
 * #define	DEBUG	1
 */


#define	DEFAULT_FS_TYPE		"ufs"
#define	OUR_SPECIAL_OPTIONS	"T:t:"
#define	LIST_ITERATOR_MARK	((list_iterator_id_t)0)
#define	TRUE	1
#define	FALSE	0


#ifdef __STDC__
typedef void    *univ_t;
#else /* __STDC__ */
typedef char    *univ_t;
#define const
#endif /* __STDC__ */

typedef univ_t   object_t;
typedef	object_t fs_t;
typedef	object_t list_t;
typedef	univ_t   list_iterator_id_t;
typedef int      boolean_t;

		      
/* Filesystem Configuration */
struct configuration {
	char *type;
	char *fsck;
	char *getopt_string;
} configurations[] = {	/* ADD NEW TYPES OF FILE SYSTEMS HERE !!! */
                        /* NOTE: the default fsck should be the first entry */
	{"ufs",  "/sbin/ufs_fsck", "b:cdl:m:NnopYy"},
	{"s5fs", "/sbin/s5fsck",   "DFfNnqS:s:C:Yy"},
	{(char *)0}
};


/* Global Context */
struct {
	char *getopt_string;
	char *fs_type;
	char *option_type;
	int   file_count;
} context;
char         *initial_getopt_string;
static list_t filesystems;
char         *program;


/* Function Prototypes */
extern int init                __ ((void));
extern int parse               __ ((int, char **));
extern int get_type_from_fstab __ ((char *, char **));
extern int get_fs_by_type      __ ((char *, fs_t *));
extern int put_file_by_type    __ ((char *, char *));
extern int put_option_by_type  __ ((char *, char *));
extern int put_option_all      __ ((char *));
extern int set_use_fstab       __ ((char *));
extern int get_use_fstab       __ ((char *, boolean_t *));
extern int get_getopt_string   __ ((char *, char **));
extern int fsck                __ ((int *));
extern int check_fs            __ ((fs_t, boolean_t *, int *));
extern int run_fsck            __ ((fs_t, int, char **, int *));
extern void child_exit_status  __ ((int, int *));

extern boolean_t default_fsck_error  __ ((char *, int *));

extern boolean_t type_in_fstab __ ((char *));
extern int save_string __ ((char *, char **));
extern struct fstab *get_fs_type __ ((char *));
extern struct fstab *get_fs_file __ ((char *));

extern int       fs_create              __ ((char *, char *, char *, fs_t *));
extern int       fs_put_file            __ ((fs_t, char *));
extern int       fs_put_option          __ ((fs_t, char *));
extern char     *fs_get_type            __ ((fs_t));
extern char     *fs_get_getopt_string   __ ((fs_t));
extern void      fs_set_use_fstab       __ ((fs_t));
extern boolean_t fs_get_use_fstab       __ ((fs_t));
extern char     *fs_get_fsck            __ ((fs_t));
extern int       fs_get_file_count      __ ((fs_t));
extern int       fs_get_option_count    __ ((fs_t));
extern void      fs_get_files           __ ((fs_t, char **));
extern void      fs_get_options         __ ((fs_t, char **));

extern int                list_create  __ ((list_t *));
extern int                list_append  __ ((list_t, univ_t));
extern list_iterator_id_t list_next    __ ((list_t, list_iterator_id_t,
					    univ_t *));
extern int                list_destroy __ ((list_t));

extern int  mem_malloc  __ ((univ_t *, int));
extern int  mem_calloc  __ ((univ_t *, int, int));
extern int  mem_realloc __ ((univ_t *, int));
extern void mem_free    __ ((univ_t));

extern int  optind;
extern char *optarg;

main(argc, argv)
	char **argv;
{
        int exit_status=0;

	program = argv[0];

#ifdef DEBUG
	printf ("Init...\n");
#endif
	if (init()) {
#ifdef DEBUG
		printf ("Init failed\n");
#endif
		fprintf(stderr, "%s: initialization failure\n", program);
		exit(1);
	}
#ifdef DEBUG
	printf ("Parse...\n");
#endif
	if (parse(argc, argv)) {
#ifdef DEBUG
		printf ("Parse failed\n");
#endif
		fprintf(stderr, "%s: command line parse failure\n", program);
		exit(1);
	}
#ifdef DEBUG
	printf ("Fsck...\n");
#endif
	if (fsck(&exit_status)) {
#ifdef DEBUG
		printf ("Fsck failed\n");
#endif
		fprintf(stderr, "%s: fsck failure\n", program);
		exit(1);
	}
	exit(exit_status);
	/*NOTREACHED*/
}

int
init()
{
	struct configuration *cp;
	int len, size, rc;
	char *old_string;
	fs_t fs;

	/* create list of file systems */
	if (rc = list_create(&filesystems))
		return(rc);

	/* get space for initial getopt() string */
	size = 0;
	for (cp = configurations; cp->type; cp++)
		size += strlen(cp->getopt_string);
	len = sizeof(OUR_SPECIAL_OPTIONS);	/* includes NULL */
	if (rc = mem_malloc((univ_t *)&initial_getopt_string, size + len))
		return(rc);
	initial_getopt_string[0] = '\0';

	/*
	 * create initial getopt() string and add our special
	 * options to all getopt() strings
	 */
	for (cp = configurations; cp->type; cp++) {
		old_string = cp->getopt_string;
		(void)strcat(initial_getopt_string, old_string);
		if (rc = mem_malloc((univ_t *)&cp->getopt_string,
		    (strlen(old_string) + len)))
			return(rc);
		(void)strcpy(cp->getopt_string, old_string);
		(void)strcat(cp->getopt_string, OUR_SPECIAL_OPTIONS);
	}
	(void)strcat(initial_getopt_string, OUR_SPECIAL_OPTIONS);

	/* configure all known file systems from the configuration record */
	for (cp = configurations; cp->type; cp++) {
		if (rc = fs_create(cp->type, cp->fsck,
		    cp->getopt_string, &fs))
			return(rc);
		if (rc = list_append(filesystems, (univ_t)fs))
			return(rc);
	}

	/* initialize global context */
	context.getopt_string = initial_getopt_string;
	context.fs_type = (char *)0;
	context.option_type = (char *)0;
	context.file_count = 0;

	/* success */
	return(0);
}


int
parse(argc, argv)
	char **argv;
{
	int i, c, rc, count, prev_optind;
	char *file, *type, *option, *cpp;

loop:

	/* parse options specified */
	prev_optind = optind;
	while (optind < argc) {

		/* handle special options */
		option = argv[optind];
		if ((option[0] == '-')
		    && ((option[1] == 'S')
		      || (option[1] == 's'))
		    && ((!context.option_type)
		      || (!strcmp(context.option_type, "s5fs")))) {
			/*
			 * -S and -s are special because they may have
			 * an optional argument
			 */
			optind++;			
		} else {
			if ((c = getopt(argc, argv, context.getopt_string)) == EOF)
				break;
			switch (c) {

			    case '?':
				return(-1);

			    /*
			     * -t fstype 
			     * means that the following options and device names
			     * are options and devices of the filesystem type
			     * fstype.  
			     *
			     * -T fstype
			     * means that the following options are for the
			     * filesystem type fstype
			     *
			     * We allow multiple -t and -T options on the
			     * command line.  We need to finish the processing
			     * of the previous -t option here.
			     * If no devices follow the previous -t option,
			     * mark that /etc/fstab should be checked for the
			     * fstype of the previous -t option.
			     *
			     * Get new option string for both -t and -T.
			     */
			    case 't':
				if (context.fs_type) {
					if (!context.file_count) {
						if (rc = set_use_fstab(context.fs_type))
							return(rc);
					}
				}
				context.fs_type = optarg;
				context.file_count = 0;
				/* fall through to -T case below */

			    case 'T':
				if (rc = get_getopt_string(optarg, &context.getopt_string))
					return(rc);
				context.option_type = optarg;
				prev_optind = optind;
				continue;
			}
		}

		/* pass other options on to fsck programs */
		count = optind - prev_optind;
		for (i = 0; i < count; i++) {
			option = argv[prev_optind++];
			if (context.option_type) {
			        /*
				 * processing a fstype, options only for the
				 * fstype
				 */
				if (rc = put_option_by_type(context.option_type, option))
					return(rc);
			} else {
			        /* option common to all filesystem types */
				if (rc = put_option_all(option))
					return(rc);
			}
		}

		/* this must be true */
		assert(prev_optind == optind);
	}

	/* parse filesystems specified */
	while (optind < argc) {
		file = argv[optind];
		
		/* 
		 * special processing for options that have optional arguments
		 */
		if (*file == '-')
			goto loop;
		
		context.file_count++;

		if (context.fs_type) {
		        /* 
			 * if we are processing a -t, put file in the list of
			 * the requested fstype 
			 */
			type = context.fs_type;
		} else {
		        /* 
			 * look up the type of the filesystem from /etc/fstab
			 * if not found, put it on the list of the default
			 * filesystem type.
			 */
			if (get_type_from_fstab(file, &type))
				type = DEFAULT_FS_TYPE;
		}

		if (rc = put_file_by_type(type, file))
			return(rc);

		optind++;
	}

	/* final scope close (for the last -t option) */
	if (context.fs_type) {
		if (!context.file_count) {
			if (rc = set_use_fstab(context.fs_type))
				return(rc);
		}
	}

	/* success */
	return(0);
}

/* get the fstype from /etc/fstab given the name of a filesystem */
int
get_type_from_fstab(name, tp)
	char *name;
	char **tp;
{
	struct fstab *fstab;

	if ((fstab = get_fs_file(name)) == (struct fstab *)NULL)
		return(-1);
	return(save_string(fstab->fs_vfstype, tp));
}

/* Is this fstype in /etc/fstab ? */
boolean_t
type_in_fstab(type)
	char *type;
{
	struct fstab *fsp;

	if (get_fs_type(type) == NULL)
		return(FALSE);
	return(TRUE);
}

int
get_fs_by_type(type, fsp)
	char *type;
	fs_t *fsp;
{
	list_iterator_id_t id;
	fs_t               fs;

	id = LIST_ITERATOR_MARK;
	while ((id = list_next(filesystems, id, (univ_t *)&fs))
	    != LIST_ITERATOR_MARK) {
		if (!strcmp(fs_get_type(fs), type)) {
			*fsp = fs;
			return(0);
		}
	}

	/* failure */
	(void)fprintf(stderr, "%s: unknown file system type \"%s\"\n",
		      program, type);
	return(-1);
}

int
put_file_by_type(type, file)
	char *type, *file;
{
	fs_t fs;
	int  rc;

	if (rc = get_fs_by_type(type, &fs)) {
		(void)fprintf(stderr, "%s: default to %s\n", program, 
			      DEFAULT_FS_TYPE);
		get_fs_by_type(DEFAULT_FS_TYPE, &fs);
	}
	if (rc = fs_put_file(fs, file))
		return(rc);

	/* success */
	return(0);
}

int
put_option_by_type(type, option)
	char *type, *option;
{
	fs_t fs;
	int  rc;

	if (rc = get_fs_by_type(type, &fs)) {
		return(rc);
	}

	if (rc = fs_put_option(fs, option))
		return(rc);

	/* success */
	return(0);
}

/*
 * put the option in the option list of all filesystem types (i.e., it is a
 * common option)
 */
int
put_option_all(option)
	char *option;
{
	list_iterator_id_t id;
	fs_t               fs;
	int                rc;

	id = LIST_ITERATOR_MARK;
	while ((id = list_next(filesystems, id, (univ_t *)&fs))
	    != LIST_ITERATOR_MARK) {
		if (rc = fs_put_option(fs, option))
			return(rc);
	}

	/* success */
	return(0);
}

/* for a fs type, indicate that /etc/fstab should be checked for that fstype */
int
set_use_fstab(type)
	char *type;
{
	fs_t fs;
	int  rc;

	if (rc = get_fs_by_type(type, &fs))
		return(rc);
	fs_set_use_fstab(fs);

	/* success */
	return(0);
}

int
get_use_fstab(type, bp)
	char      *type;
	boolean_t *bp;
{
	fs_t fs;
	int  rc;

	if (rc = get_fs_by_type(type, &fs))
		return(rc);
	*bp = fs_get_use_fstab(fs);

	/* success */
	return(0);
}

/* get the option string of a fstype */
int
get_getopt_string(type, string)
	char *type;
	char **string;
{
	fs_t fs;
	int  rc;

	if (rc = get_fs_by_type(type, &fs))
		return(rc);
	*string = fs_get_getopt_string(fs);

	/* success */
	return(0);
}

/*
 * fsck()
 *
 * We are now ready to run the various fsck programs.  If we've not been
 * told explicitly to check any file systems, then we should just run the
 * various fsck programs and let them look in the fstab themselves.  Of
 * course there is no need to run an fsck program if fstab does not
 * contain any file systems of that type.
 */

int
fsck(exit_stat_ptr)
     int *exit_stat_ptr;
{
	list_iterator_id_t id;
	fs_t               fs;
	boolean_t          check_all_in_fstab = TRUE;
	boolean_t          checked_some_fs;
	char              *type;
	int                rc;

#ifdef DEBUG
	printf ("Entering fsck\n");
#endif
	id = LIST_ITERATOR_MARK;
	while ((id = list_next(filesystems, id, (univ_t *)&fs))
	    != LIST_ITERATOR_MARK) {

		if (rc = check_fs(fs, &checked_some_fs, exit_stat_ptr))
			return(rc);

		if (checked_some_fs)
			check_all_in_fstab = FALSE;
	}

	if (check_all_in_fstab == FALSE)
		return(0);

	/* Check everything in /etc/fstab */
	id = LIST_ITERATOR_MARK;
	while ((id = list_next(filesystems, id, (univ_t *)&fs))
	    != LIST_ITERATOR_MARK) {

		type = fs_get_type(fs);

		if (rc = run_fsck(fs, 0, (char **)0, exit_stat_ptr))
			return(rc);
		if (default_fsck_error(type, exit_stat_ptr))
		  /* 
		   * checking everything in /etc/fstab.  If the default fsck
		   * exits with error, exit right away
		   */
		     exit(*exit_stat_ptr);
	}

	return(0);
}


/*
 * Check a file system.  This routine only processes those file systems that
 * are specified by -t or have filesystems specified on the command line.
 */
int
check_fs(fs, checked_some_fs_pointer, exit_stat_ptr)
	fs_t       fs;
	boolean_t *checked_some_fs_pointer;
        int       *exit_stat_ptr;
{
	char    **files;
	char     *type, *type_from_fstab;
	int       i, rc, count;

#ifdef DEBUG
	printf ("Entering check_fs\n");
#endif
	*checked_some_fs_pointer = FALSE;

	if (count = fs_get_file_count(fs)) {
        /* filesystems are specified for this fstype */
		if (rc = mem_calloc((univ_t *)&files, count, sizeof(*files)))
			return(rc);
		fs_get_files(fs, files);
	}

	if (fs_get_use_fstab(fs)) {
	/* use /etc/fstab to check for this fstype */
		if (count) {
	           /* 
		    * filesystems are also specified for this fstype.
		    * check if a specified fs is also in /etc/fstab.  If so,
		    * take it off the specified list of fs to check since it 
		    * will be checked in /etc/fstab anyway.
		    */
			type = fs_get_type(fs);
			i = 0;
			while (i < count) {
				if ((!get_type_from_fstab(files[i], &type_from_fstab))
				    && (!strcmp(type, type_from_fstab))) {
					count--;
					files[i] = files[count];
				} else
					i++;
			}
		}
		if (rc = run_fsck(fs, 0, (char **)0, exit_stat_ptr))
			return(rc);
		*checked_some_fs_pointer = TRUE;
	}
	if (count) {
	/* filesystems are specified explicitly to be tested, so test them */
		if (rc = run_fsck(fs, count, files, exit_stat_ptr))
			return(rc);
		*checked_some_fs_pointer = TRUE;
	}
	return(0);
}

/*
 * Actually run the filesystem dependent fsck program
 *
 * N.B.: Returns non-zero if we never ran fsck, otherwise returns zero!
 */
int
run_fsck(fs, file_count, files, exit_stat_ptr)
	fs_t fs;
	char **files;
        int   *exit_stat_ptr;
{
#ifdef __alpha__
	char    **options, **cpp;
	char	*argv[32];
	int	j;
#else
	char    **options, **argv, **cpp;
#endif
	int       i, rc, argc, option_count;
	char     *fsck;
	pid_t      pid;
	int       status;

#ifdef DEBUG
	printf ("Entering run_fsck\n");
#endif
	if ((!file_count) && (!type_in_fstab(fs_get_type(fs))))
	/* no explicit filesystems listed and the fstype is not in /etc/fstab */
		return(0);

	if (option_count = fs_get_option_count(fs)) {
		if (rc = mem_calloc((univ_t *)&options, option_count, sizeof(*options)))
			return(rc);
		fs_get_options(fs, options);
	}

	argc = option_count + file_count + 2;	/* command name plus terminating NULL */
#ifndef __alpha__
	if (rc = mem_calloc((univ_t *)&argv, argc, sizeof(*argv)))
		return(rc);
#endif

	fsck = fs_get_fsck(fs);

#ifdef DEBUG
	printf ("Did fs_get_fsck \n");
	printf ("Option count = %d  File count = %d\n", option_count,
		file_count);
#endif
	/* create argv for child to exec */
#ifdef __alpha__
	for (i = 0; i < 32; i++)
		argv[i] = (char *)0;
	i = 0;
	argv[i] = (char *)fsck;
	i = 1;		
	for (j = 0; j < option_count; j++) {
		argv[i] = options[j];
		i++;
	}
	for (j = 0; j < file_count; j++) {
		argv[i] = files[j];
		i++;
	}
	argv[i+1] = (char *)0;
#else
	cpp = argv;
	*cpp++ = fsck;
	for (i = 0; i < option_count; i++)
		*cpp++ = options[i]; 	
	for (i = 0; i < file_count; i++)
		*cpp++ = files[i];	
	*cpp++ = (char *)0;
#endif

#ifdef DEBUG
	printf ("argc = %d\n", argc);
	printf ("Printing argv...\n");
#endif
/*#ifdef	DEBUG */
	for (i = 0; i < argc; i++) {
		if (argv[i] == NULL)
			continue;
		if (i) {
			printf(" ");
		}
		printf("%s", argv[i]);
	}
	printf("\n");
/*$$$	return(0); */
/*#else */
#ifdef DEBUG
	printf ("FORK!\n");
#endif
	if ((pid = fork()) == -1) {
		/* fork() error */

		(void)fprintf(stderr, "%s: ", program);
		perror("fork() failed");
		return(-1);
	} else if (pid) {
		/* parent */

		if (wait(&status) == -1) {
			(void)fprintf(stderr, "%s: ", program);
			perror("wait() failed");
			return(-1);
		}
#ifdef DEBUG
		printf("Child done- status %d %d\n", status, *exit_stat_ptr);
#endif
		child_exit_status(status, exit_stat_ptr);
#ifdef DEBUG
		printf("Child status after crunching%d\n", status);
#endif
		return(0);
	} else {
		/* child */

#ifdef DEBUG
		printf ("FSCK %s\n", fsck);
#endif
		execv(fsck, argv);
#ifdef DEBUG
		printf ("execv failed errno = %d\n, errno");
#endif
#ifndef __alpha__
		(void)fprintf(stderr, "%s: execl(\"%s\"", program, fsck);
		for ( ; *argv; argv++)
			(void)fprintf(stderr, ", \"%s\"", *argv);
#endif
		perror(", (char *)0) failed");
		exit(1);
		/*NOTREACHED*/
	}
	/*NOTREACHED*/
/*#endif */
}


/*
 * check the exit status of the child process which actually performs the
 * filesystem dependent fsck.  Update *exit_stat_ptr only if this is the
 * first error.
 */
void
child_exit_status(status, exit_stat_ptr)
        int status;
        int *exit_stat_ptr;
{
        if ((status == 0) || (*exit_stat_ptr != 0))
	      return;
        if (WIFEXITED(status) && WEXITSTATUS(status))
		    *exit_stat_ptr = WEXITSTATUS(status);
	/* 
	 * we are only interested in exit status.  no processing for
	 * signal status.
	 */
}


/*
 * check if the default fsck returns with error.
 */
boolean_t
default_fsck_error(type, exit_stat_ptr)
         char *type;
         int  *exit_stat_ptr;
{
         if ((!strcmp(type, DEFAULT_FS_TYPE)) && (*exit_stat_ptr != 0))
	       return(TRUE);
	 return (FALSE);
}
		      
/********************* File System Abstraction *********************/

typedef struct fs_object {
	list_t    files;
	int       file_count;
	list_t    options;
	int       option_count;
	boolean_t use_fstab;
	char     *fsck;
	char     *type;
	char     *getopt_string;
} fs_object_t;

int
fs_create(type, fsck, getopt_string, return_value_pointer)
	char *type, *fsck, *getopt_string;
	fs_t *return_value_pointer;
{
	fs_object_t *object;
	int rc;

	/* create space for object */
	if (rc = mem_malloc((univ_t)&object, sizeof(*object)))
		return(rc);

	/* initialize object */
	object->files = (list_t)0;
	object->options = (list_t)0;
	object->use_fstab = FALSE;
	object->type = type;
	object->fsck = fsck;
	object->file_count = 0;
	object->option_count = 0;
	object->getopt_string = getopt_string;

	/* return object */
	*return_value_pointer = (fs_t)object;

	/* success */
	return(0);
}


int
fs_put_file(fs, file)
	fs_t  fs;
	char *file;
{
	fs_object_t *object = (fs_object_t *)fs;
	int          rc;

	if (!object->files)
		if (rc = list_create(&object->files))
			return(rc);

	if (rc = list_append(object->files, (univ_t)file))
		return(rc);

	object->file_count++;

	return(0);
}

int
fs_put_option(fs, option)
	fs_t  fs;
	char *option;
{
	fs_object_t *object = (fs_object_t *)fs;
	int          rc;

	if (!object->options)
		if (rc = list_create(&object->options))
			return(rc);

	if (rc = list_append(object->options, (univ_t)option))
		return(rc);

	object->option_count++;

	return(0);
}

char *
fs_get_getopt_string(fs)
	fs_t fs;
{
	fs_object_t *object = (fs_object_t *)fs;

	return(object->getopt_string);
}

void
fs_set_use_fstab(fs)
	fs_t fs;
{
	fs_object_t *object = (fs_object_t *)fs;

	object->use_fstab = TRUE;
}

boolean_t
fs_get_use_fstab(fs)
	fs_t fs;
{
	fs_object_t *object = (fs_object_t *)fs;

	return(object->use_fstab);
}

char *
fs_get_type(fs)
	fs_t fs;
{
	fs_object_t *object = (fs_object_t *)fs;

	return(object->type);
}

char *
fs_get_fsck(fs)
	fs_t fs;
{
	fs_object_t *object = (fs_object_t *)fs;

	return(object->fsck);
}

int
fs_get_file_count(fs)
	fs_t fs;
{
	fs_object_t *object = (fs_object_t *)fs;

	return(object->file_count);
}

int
fs_get_option_count(fs)
	fs_t fs;
{
	fs_object_t *object = (fs_object_t *)fs;

	return(object->option_count);
}

void
fs_get_files(fs, files)
	fs_t   fs;
	char **files;
{
	fs_object_t       *object = (fs_object_t *)fs;
	list_iterator_id_t id;
	char              *file;

	if (!object->file_count)
		return;

	id = LIST_ITERATOR_MARK;
	while ((id = list_next(object->files, id, (univ_t *)&file))
	    != LIST_ITERATOR_MARK) {

		*files++ = file;
	}
}

void
fs_get_options(fs, options)
	fs_t   fs;
	char **options;
{
	fs_object_t       *object = (fs_object_t *)fs;
	list_iterator_id_t id;
	char              *option;

	if (!object->option_count)
		return;

	id = LIST_ITERATOR_MARK;
	while ((id = list_next(object->options, id, (univ_t *)&option))
	    != LIST_ITERATOR_MARK) {

		*options++ = option;
	}
}


/************************* List Abstraction *************************/

typedef struct element {
	struct element *e_next;
	univ_t          e_data;
} element_t;

typedef struct list_object {
	element_t *lo_head;
	element_t *lo_tail;
} list_object_t;

int
list_create(list_ptr)
	list_t *list_ptr;
{
	list_object_t *object;
	int            rc;

	/* allocate a list object */
	if ((rc = mem_malloc((univ_t)&object, sizeof(*object))) < 0)
		return(rc);

	object->lo_head = (element_t *)0;
	object->lo_tail = (element_t *)0;

	*list_ptr = (list_t)object;
	return(0);
}

int
list_append(list, data)
	list_t list;
	univ_t data;
{
	list_object_t  *object = (list_object_t *)list;
	element_t      *e;
	int             rc;

	/* allocate a list element */
	if ((rc = mem_malloc((univ_t)&e, sizeof(*e))) < 0)
		return(rc);

	e->e_next = (element_t *)0;
	e->e_data = data;

	if (object->lo_head) {
		object->lo_tail->e_next = e;
		object->lo_tail = e;
	} else {
		object->lo_head = e;
		object->lo_tail = e;
	}

	return(0);
}

list_iterator_id_t
list_next(list, id, datap)
	list_t             list;
	list_iterator_id_t id;
	univ_t            *datap;
{
	list_object_t *object = (list_object_t *)list;
	element_t     *e;

	if (id == LIST_ITERATOR_MARK) {
		if (e = object->lo_head) {
			*datap = e->e_data;
			return((list_iterator_id_t)e);
		} else
			return(LIST_ITERATOR_MARK);
	}
	e = (element_t *)id;
	if (e = e->e_next) {
		*datap = e->e_data;
		return((list_iterator_id_t)e);
	}
	return(LIST_ITERATOR_MARK);
}

int
list_destroy(list)
	list_t list;
{
	list_object_t  *object = (list_object_t *)list;
	element_t      *e, *next;

	e = object->lo_head;
	while (e) {
		next = e->e_next;
		(void)mem_free((univ_t)e);
		e = next;
	}
	(void)mem_free((univ_t)object);
	return(0);
}

		    
/******************* Memory Allocation Abstraction *******************/

extern char *malloc __ ((unsigned));
extern char *calloc __ ((unsigned, unsigned));
extern char *realloc __ ((char *, unsigned));
extern char *free __ ((char *));

static void mem_error __ ((char *));
static void mem_clear __ ((char *, int));

int
mem_malloc(ptr, size)
	univ_t *ptr;
{
	char *cp;

	if ((cp = malloc((unsigned)size)) == NULL) {
		mem_error("malloc()");
		return(-1);
	}
	mem_clear(cp, size);
	*ptr = (univ_t)cp;
	return(0);
}

int
mem_calloc(ptr, nelem, size)
	univ_t *ptr;
{
	char *cp;

	if ((cp = calloc((unsigned)nelem, (unsigned)size)) == NULL) {
		mem_error("calloc()");
		return(-1);
	}
	mem_clear(cp, (nelem*size));
	*ptr = (univ_t)cp;
	return(0);	
}

int
mem_realloc(ptr, size)
	univ_t *ptr;
{
	char *cp;

	if ((cp = realloc((char *)(*ptr), (unsigned)size)) == NULL) {
		mem_error("realloc()");
		return(-1);
	}
	*ptr = (univ_t)cp;
	return(0);
}

void
mem_free(ptr)
	univ_t ptr;
{
	free((char *)ptr);
}

static void
mem_clear(cp, size)
	register char *cp;
	register int size;
{
	while (size--)
		*cp++ = '\0';
}

static void
mem_error(routine)
	char *routine;
{
	(void)fprintf(stderr, "%s: %s failed\n", program, routine);
#ifdef	notdef
	exit(1);
#endif
}

		    
/******************* General Purpose Subroutines *******************/

int
save_string(s, cpp)
	char *s, **cpp;
{
	char *cp;
	int   rc;

	if (rc = mem_malloc((univ_t *)&cp, strlen(s)+1))
		return(rc);
	(void)strcpy(cp, s);
	*cpp = cp;
	return(0);
}

struct fstab *
get_fs_file(name)
	char *name;
{
	struct fstab *fsp;

	setfsent();
	while ((fsp = getfsent()) != (struct fstab *)NULL)
		if (!strcmp(fsp->fs_spec, name))
			return(fsp);
	return(NULL);
}

struct fstab *
get_fs_type(type)
	char *type;
{
	struct fstab *fsp;

	setfsent();
	while ((fsp = getfsent()) != (struct fstab *)NULL)
		if (!strcmp(fsp->fs_vfstype, type))
			return(fsp);
	return(NULL);
}
