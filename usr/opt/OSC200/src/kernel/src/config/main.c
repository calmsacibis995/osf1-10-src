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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.3.12.3 $ (DEC) $Date: 1993/09/29 21:39:29 $";
#endif /* lint */
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */


/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*	Change History							*
 *									*
 * 3-20-91	robin-							*
 *		Made changes to support new device data structures	*
 * 4-10-91	robin-							*
 *		Added BINARY build and make depend functionality	*
 *									*
 * 18-Oct-91	jestabro
 *		Add ALPHA support.
 *									*
 * 19-Nov-91	paul hansen
 *		ifdef'd swapgeneric_dir to allow for alpha generic 
 *		configs.
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "y.tab.h"
#include "config.h"

extern char *realloc();
char *search_path;
char TARGET_dir[256];
char *source_confdir = "conf";
#ifdef __alpha
char *swapgeneric_dir = "arch/alpha/";
#else
char *swapgeneric_dir = "arch/mips/";
#endif
int query_file = FALSE;
char *CONF_name;

int dupSTDerr;
int dupSTDout;
int dupSTDin;

/*
 * Config builds a set of files for building a UNIX
 * system given a description of the desired system.
 */


main(argc, argv)
	int argc;
	char **argv;
{

	register struct device_entry *dp;
	extern char *optarg;
	extern int optind;
	extern int errno;
	extern struct orphen_dev *orph_hdp;
	extern int yyline;

	int optc,errflg = FALSE;
	char buf[256];
	FILE *fp;
	int i, config_flag = 0;
	int c;
	orph_hdp =  NULL;

	object_directory = "..";
	config_directory = (char *) ".";
	search_path = (char *) 0;

	/* Save off the normal file descriptors */
	dupSTDin = dup((int)0);
	dupSTDout = dup((int)1);
	dupSTDerr = dup((int)2);
/* Parse the input line
 */
	while ((optc=getopt(argc, argv, "kpsqI:c:o:")) != EOF) {
		switch(optc) {

		case 'k':
			kdebug = TRUE;
			break;
		case 'p':
			profiling = TRUE;
			break;
		case 's':			/* Flag used to build all files - source build */
			source = TRUE;
			break;
                case 'q':			/* will 'list' all parts of a config file */
			query_file = TRUE;
			break;
		case 'o':
			object_directory = optarg;
			break;
		case 'c':
			config_directory = optarg;
			config_flag = 1;
			break;
		case 'I': 
			if (search_path == 0) {
				i = strlen(optarg) + 1;
				search_path = malloc(i);
				strcpy(search_path, optarg);
			} else {
				i = strlen(optarg) + strlen(search_path) + 2;
				search_path = realloc(search_path, i);
				strcat(search_path, ":");
				strcat(search_path, optarg);
			}
			break;
		case '?':
			errflg = TRUE;
			break;
		}
	}
	/* No 'c' flag was seen and its a build based on '*.o' files so set
	 * the path up to find the tools when building on a non-source
	 * environment.
	 */
	if(source && !config_flag)
		object_directory = source_confdir;
	if(((argc-optind) != 1) || errflg) {	/* no error and 1 arg left */
		fprintf(stderr, "usage: config [ -kpqs ] sysname\n");

		exit(2);
	}
	PREFIX = argv[optind];
	if (freopen(argv[optind], "r", stdin) == NULL) {
		perror(argv[optind]);
		exit(2);
	}

	CONF_name=argv[optind];
	if (search_path == (char *) 0)
		search_path = ".";

	dtab = NULL;
	highuba = -1;
	extrauba = 0;
	emulation_instr = 0;
	confp = &conf_list;
	opt = 0;
	/* Parse the config file via YACC
	 */
	yyline = 1;
	if(query_file){
		while((c = getchar()) != EOF)
			putchar(c);
	} else
	    if (yyparse())
		exit(3);
	    {
		/* If we find any 3ed party layered products registered
		 * that have config file fragments then read them into
		 * the language.
		 */
		char *List, *Config;
		char *entry, *rtn, *copy, *index;
		FILE *list_fp;

		List = malloc(strlen(PREFIX)+10);
		Config = malloc((sizeof(char) * 1024));

		(void) sprintf(List, "./%s.list", PREFIX);
		list_fp = VPATHopen(List, "r");
		if (list_fp != 0)
		    {
			for ( entry = malloc(sizeof(char) * 1024),
			     copy = malloc(sizeof(char) * 1024),
			     rtn = fgets(entry,1024,list_fp);
			     rtn != (char *) 0;
			     rtn = fgets(entry,1024,list_fp)) 
			{
			     /* OK now we have a line from the PREFIX.list 
			      * file which should point us to an
			      * area that MAY contain a files file and config file 
			      * entries for use when building THIS kernel.
			      */
			     (void) strcpy(copy,entry);

			     /* test for blank lines and comment lines */
			     if (((strlen(copy)-1) != (strspn (copy, " \t")))
				 && (copy[0] != '#')) {
			     
				/* get first entry from colon separated string.
				 * remember to account for the \n at the end
				 */
				index = strtok(copy,":\n");
			     
				/* calculate the path and filename for
				 * the config.file.  opent it and either
				 * spit it out to stdout or parse it for
				 * correctness
				 */

				(void) strncpy(Config, index, strlen(index));
				(void) strcat(Config,"/config.file");

				if(freopen(Config, "r", stdin) != NULL){
				    if(query_file){
					while((c = getchar()) != EOF)
					    putchar(c);
				    } else if(yyparse())
					exit(4);
				}
			     }
			     bzero(Config,strlen(Config));
			     bzero(entry,(sizeof(char) * 1024));
			     bzero(copy,(sizeof(char) * 1024));
			}
		    }
	    }
	
	if(query_file)	/* Just print the config file to stdout and do no more */
		return;

	/* If there are and orphen entries we now need to get them into the 
	 * data structures connected up to what ever they need connected to.
	 * and if we have any left on the orphen list when we are do we will 
	 * need to generate an error message.
	 */
	{
		struct device_entry *connect_orphen();
		extern struct orphen_dev *orph_hdp;
		extern struct orphen_dev *orph_p;
		struct device_entry *dpi;
		for (dpi = dtab; dpi != NULL; dpi = dpi->d_next) {
			if(dpi->d_conn != NULL)
				continue;

			/* Connect all the controllers up first so the devices have
			 * something to connect to.
			 */
			for (orph_p = orph_hdp; 
			     orph_p != NULL; 
			     orph_p = orph_p->next)
			{
				if(orph_p->type != CONTROLLER)
					continue;
				if(orph_p->dev == dpi->d_name && orph_p->num == dpi->d_unit)
					break;
			}
			if(orph_p == NULL)
				continue;
			dpi->d_conn = 
				connect_orphen(orph_p->dev_to,orph_p->num_to);
			if( dpi->d_conn != NULL)
				orph_p->type = NULL;
		}

		/* Now go a head and connect the devices up to the controllers
		 */
		for (dpi = dtab; dpi != 0; dpi = dpi->d_next) {
			if(dpi->d_conn != NULL)
				continue;

			/* Connect all the controllers up first so the devices have
			 * something to connect to.
			 */
			for (orph_p = orph_hdp; 
			     orph_p != NULL; 
			     orph_p = orph_p->next)
			{
				if(orph_p->type != DEVICE)
					continue;
				if(orph_p->dev == dpi->d_name && orph_p->num == dpi->d_unit)
					break;
			}
			if( orph_p == NULL)
				continue;
			dpi->d_conn = 
				connect_orphen(orph_p->dev_to,orph_p->num_to);
			if( dpi->d_conn != NULL)
				orph_p->type = NULL;
		}

		/* The orphen list should be empty by now
		 */
		for (orph_p = orph_hdp; 
		     orph_p != NULL; 
		     orph_p = orph_p->next)
			if(orph_p->type != NULL)
				printf("%s %d not connected to anything\n");
	}

	/* Make a callout for level 0 which are callouts that take place
	 * before config does any work at all.
	 */
	 callout_exec(EVENT_LEVEL0);
	 callout_exec(EVENT_LEVEL7);


	switch (machine) {

	case MACHINE_DEC_RISC:
		sprintf(TARGET_dir,"../%s",PREFIX);

		/* Print ioconf.c */
		dec_ioconf();	

		if (is_cpu_declared("DS3100") || is_cpu_declared("DS5000"))
                    if(!isconfigured("dc")) {
                       printf("config file error: must specify device dc\n");
                       Exit(1);
                     }

		/* Create scb_vec.c */
		ubglue();	
		break;
	case MACHINE_ALPHA:
		sprintf(TARGET_dir,"../%s",PREFIX);

		/* Print ioconf.c */
		dec_ioconf();
		break;

	default:
		if( !query_file )
		printf("Specify machine type, e.g. ``machine vax, mips or alpha''\n");
		Exit(1);
	}

	callout_exec(EVENT_LEVEL8);

	/* build HOSTNAME/conf.c file */
	callout_exec (EVENT_LEVEL9);
	mklocalconf();
	callout_exec (EVENT_LEVEL10);

	/* build Makefile */
	callout_exec(EVENT_LEVEL5);
	makefile();
	callout_exec(EVENT_LEVEL6);

	/* make a lot of .h files */
	callout_exec(EVENT_LEVEL3);
	headers();	
	callout_exec(EVENT_LEVEL4);

	/* swap config files */
	swapconf();

	/* If this is a build from BINARY's (not source) a file vers.config
	 * needs to be made in the target directory that contains PREFIX's
	 * value 
	 */
	if ( !source ){
		sprintf(buf,"%s/%s",TARGET_dir,"vers.config");
		fp=fopen(buf,"w");
		if(fp){
			fprintf(fp,"%s\n",PREFIX);
			fclose (fp); }
		else
			perror(buf);
	}
	callout_exec(EVENT_LEVEL2);
}

/*
 * get_word
 *	returns EOF on end of file
 *	NULL on end of line
 *	pointer to the word otherwise
 */
char *
get_word(fp)
	register FILE *fp;
{
	static char line[80];
	register int ch;
	register char *cp;

	while ((ch = getc(fp)) != EOF)
		if (ch != ' ' && ch != '\t')
			break;

	if (ch == EOF)
		return ((char *)EOF);

	if (ch == '\n')
		return (NULL);

	if (ch == '|')
		return( "|");

	cp = line;
	*cp++ = ch;
	while ((ch = getc(fp)) != EOF) {
		if (isspace(ch))
			break;
		*cp++ = ch;
	}
	*cp = 0;
	if (ch == EOF)
		return ((char *)EOF);

	(void) ungetc(ch, fp);
	return (line);
}

/*
 * get_rest
 *	returns EOF on end of file
 *	NULL on end of line
 *	pointer to the word otherwise
 */
char *
get_rest(fp)
	register FILE *fp;
{
	static char line[80];
	register int ch;
	register char *cp;

	cp = line;
	while ((ch = getc(fp)) != EOF) {
		if (ch == '\n')
			break;
		*cp++ = ch;
	}
	*cp = 0;
	if (ch == EOF)
		return ((char *)EOF);
	return (line);
}

/*
 * prepend the path to a filename
 *	PREFIX and object_directory are extablished during the command line
 *	parsing.
 */

#define SLOP 50
char *
path(file)
	char *file;
{
	register char *cp;
	register struct stat tmpbuf;
	char buf[1024];
	
	cp = malloc((unsigned)(strlen(file)+1+
			       strlen(PREFIX)+1+
			       strlen(object_directory)+1+SLOP));
	/*
	if( source ){
		sprintf(cp, "%s/%s/%s", object_directory, PREFIX, file);
		return (cp);
	} else {
	*/

		(void) strcpy(cp, "..");

		/* Stick on the machine name part here */
		(void) strcat(cp, "/");
		(void) strcat(cp, PREFIX);
		/* Make the target directory if its not there */
		if(stat(cp, &tmpbuf)) {
			(void) sprintf(buf,"/bin/mkdir %s",cp);
			(void) system(buf);
		}

		/* complete the path name by adding the file name */
		(void) strcat(cp, "/");
		(void) strcat(cp, file);
		return (cp);
	/*
	}
	*/
}

/* Called to look through a list of callouts to run.  If the entry in the
 * list has the specified class the /bin/sh process runs it.
 */
int
callout_exec(class)
int	class;
{
	struct	callout_data *ptr;
	int	rtn, pid;

        if(callout_hd == 0)
		return(-1);				 /* no callouts to process */
	/* Fork to re-establish the correct stdin, stdout & stderr without
	 * screwing up the current settings.
	 */
	if((pid = fork()) == 0) {
		/* Close what ever is open because we don't know what it is */
		dup2(dupSTDin,0);
		dup2(dupSTDout,1);
		dup2(dupSTDerr,2);

		for ( ptr = callout_hd; 
		     ptr != (struct callout_data *) 0;
		     ptr = ptr->next )
		{
			 if(ptr->event_class == class)
			 {
				 if( setenv("CONFIG_NAME",CONF_name,1) == -1 ) {
				 	fprintf(stderr,"config: callout setenv CONFIG_NAME=%s failed\n",CONF_name);
					break;
				 }
				 rtn = system(ptr->unix_command);
				 if( rtn != 0 ) {
					perror(ptr->unix_command);
					fprintf(stderr,"config: callout `%s' failed with exit status %d (errno %d)\n",
						ptr->unix_command, rtn, errno);
				 }
			 }
		}
	      	_exit(0);	/* The child dies */
	}
	if (pid > 0)			/* Wait on the child */
		while (wait((union wait *)0) != pid)
			;
	else
	        if(pid == -1){
		        perror("config");
		        fprintf(stderr,"config: Could not fork for callout(s)");
	        }

}

Exit( number)
int number;
{
	callout_exec(EVENT_LEVEL2);
	exit(number);
}


/*
 * find the pointer to connect to the given device and number.
 * returns 0 if no such device and prints an error message otherwise
 * return the dp to connect to.
 */
struct device_entry *
connect_orphen(dev, num)
	register char *dev;
	register int num;
{
	struct device_entry *dp;
	extern struct device_entry *huhcon(); 
	extern struct device_entry *dtab;

	if (num == QUES){
		return ( huhcon(dev)); 
	}
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if ((num != dp->d_unit) || !eq(dev, dp->d_name))
			continue;
		if (dp->d_type != CONTROLLER && dp->d_type != MASTER && dp->d_type != BUS) {
			printf("%s is not connected to a bus or a controller\n", dev);
			return (NULL);
		}
		return (dp);
	}

	printf("%s %d not defined\n",dev,num);
	return (NULL);
}

