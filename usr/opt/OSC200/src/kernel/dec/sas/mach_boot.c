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
static char	*sccsid = "@(#)$RCSfile: mach_boot.c,v $ $Revision: 4.3.10.3 $ (DEC) $Date: 1993/07/30 18:30:41 $";
#endif 
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
 * derived from mach_boot.c	3.1	(ULTRIX/OSF)	2/28/91";
 */
/*
 *	Modification History
 *
 * 31-May-91 -- Don Dutile
 *	Added support for new boot switch -debug, to load dbgmon
 *	in addition to requested kernel
 *
 * 22-Feb-91 -- Don Dutile
 *	Merged osf/1.0 to v4.2.  note: vax-based ifdef's added back
 *	in but untested (not compile or runtime checked).
 *
 * 06-Nov-90 -- Joe Szczypek
 *	Added support for VAX6000-5x0 Diagnostic Supervisor (emsaa).
 */

#include <sys/param.h>
#include <sys/reboot.h>

#include <machine/cpuconf.h>
#include <machine/entrypt.h>   /* Contains TURBOchannel defines */
#ifdef __alpha
#include "alpha/load_image.h"
#endif

strncmp(s1, s2, n)
    char *s1, *s2;
    int n;
{
    while (--n >= 0 && *s1 == *s2++)
	if (*s1++ == '\0')
	    return(0);
    return(n<0 ? 0 : *s1 - *--s2);
}

char    vmunix_name[] = "vmunix";
char	hvmunix_name[]= "hvmunix";
unsigned systype;		/* Store the  "systype" from the PROM */
unsigned cpu;			/* Store the  cpu from the sid */

extern int devpart;		/* defined in io.c */

#define getenv prom_getenv

#define MAXARG 15
#define INBUFSZ 256

char    ask_name[INBUFSZ];

char   *imagename;

/*
 * Functional Discription:
 *	main of `osf_boot' program ... 
 *
 * Inputs:
 *	none although R10 and R11 are preserved on VAX
 *	arg list is preserved on MIPS
 *
 * Outputs:
 *	none
 *
 */

int ub_argc;
char **ub_argv;
char **ub_envp;
char **ub_vector;
int gub_argc;
char **gub_argv;

main (argc,argv,envp,vector)
int argc;
char **argv, **envp, **vector;
{
    register howto, devtype;		/* howto=r11, devtype=r10 */
    register char *arg, *cp;
    char    tmp_name[INBUFSZ];
    char    *default_name = vmunix_name;
    int	i,j;
    int	io;
    char	fs;
    char	*boot;
    extern char *version;
    extern char *strchr();
    
    char line[INBUFSZ];
    char *local_argv[MAXARG];
    char booted_osflags[128];
    char booted_file[128];
    
#if LABELS
    printf("\nOSF boot - %s\n\n", version);
#else
    printf("\nMach boot - %s\n\n", version);
#endif /* LABELS */
    
    /*
     * Get "cpu" then get and open "booted_dev" for alpha. 
     */
    cpu = getcputype();
    imagename = (char *)prom_getenv("booted_dev");
    if (devopen(imagename, 0) < 0 ) {
	printf("devopen(imagename) failed: ");
	printf("can't open channel to boot driver\n");
	stop();
    }
    
    argc = 0;
    argv = local_argv;

    /* Read the booted_osflags and the booted_file console environment */
    strcpy(booted_osflags, getenv("booted_osflags"));
    strcpy(booted_file, getenv("booted_file"));
    
    /* NOTE: booted_osflags may contain other flags used by alpha_init */
    
    /*
     * Attempt to open the 'hupdate' file.
     * If the open succeeds, then load the image and call it
     * to determine if the default vmunix should be changed 
     * to "hvmunix".  If the 'hupdate' file is not there
     * then continue to use existing default.
     */
    if ((io = open ("hupdate", 0)) >= 0) {
	long hupdate;
	struct image_return *image;

	printf("Loading hupdate ...\n");
	image = (struct image_return *)load_bootstrap_image(io, 0);
	printf("Starting hupdate at 0x%lx\n", image->entry);
	hupdate = (*((long (*) ()) image->entry))();
	if (hupdate) {
	    default_name = hvmunix_name;
	}
	close(io);
    }
    
    /* If interactive mode is selected, prompt user */
    if (strchr(booted_osflags, 'I') || strchr(booted_osflags, 'i')) {
	/*
	 * get the boot command line - argc and argv get passed on to
	 * the kernel.
	 */
	printf("Enter [kernel_name] [option_1 ... option_n]: ");
	
	gets(line);
	
	/* if null line entered by user, select defaults */
	if (line[0] == 0) {
	    argc = 1;
	    argv[0] = default_name;
	}
	else {
	    /* change spaces to terminate argv components */
	    for (i = 0; i < INBUFSZ; i++)
		if (line[i] == ' ') line[i] = 0;
	    
	    /* point argv[i] to the command line components */
	    for (i = 0; i < INBUFSZ; i++) {
		if (line[i] == 0)
		    continue;
		argv[argc] = &line[i];
		i += strlen(argv[argc]);
		argc++;
	    }
	}
    }
    else { /* boot non-interactively */
	argc = 1;
	/* if booted_file is set, attempt to boot that. */
	if ((strlen(booted_file) > 0) && 
	    (io = open (booted_file, 0)) >= 0) {
	    argv[0] = booted_file;
	    close(io);
	}
	else {  /* no valid booted_file so boot the default */
	    argv[0] = default_name;
	}
    }
    
    ub_argc = argc;		/* save prom args for kernel */
    ub_argv = argv;		/* save prom args for kernel */
    ub_envp = envp;		/* save prom args for kernel */
    ub_vector = vector;     /* save prom args for kernel */
    gub_argc = argc;	/* global save of arg */
    gub_argv = argv;	/* global save of arg */
    howto = 0;
    imagename = argv[0];
    
#ifdef DEBUG
    printf("imagename being opened for boot is %s \n", imagename);
#endif
    
    io = -1;				/* set to start loop */
    while (io < 0) {
	/* 
	 * If RB_ASKNAME is not set, then assume the default 
	 * 'vmunix' name, otherwise ask for a unix file name to
	 * load.
	 */
	if (howto & RB_ASKNAME) {
	    printf("Enter [kernel_name] [option_1 ... option_n]: ");
	    argc = 0;
	    argv = local_argv;
	    gets (ask_name);
	    /* if null line entered by user, select defaults */
	    if (ask_name[0] == 0) {
		argc = 1;
		argv[0] = default_name;
	    }
	    else {
		/* change spaces to terminate argv components */
		for (i = 0; i < INBUFSZ; i++)
		    if (ask_name[i] == ' ') ask_name[i] = 0;
		
		/* point argv[i] to the command line components */
		for (i = 0; i < INBUFSZ; i++) {
		    if (ask_name[i] == 0)
			continue;
		    argv[argc] = &ask_name[i];
		    i += strlen(argv[argc]);
		    argc++;
		}
	    }
	    ub_argc = argc;		/* save prom args for kernel */
	    ub_argv = argv;		/* save prom args for kernel */
	    ub_envp = envp;		/* save prom args for kernel */
	    ub_vector = vector;     /* save prom args for kernel */
	    gub_argc = argc;	/* global save of arg */
	    gub_argv = argv;	/* global save of arg */
	    howto = 0;
	    imagename = argv[0];
	}
	if ((io = open (imagename, 0)) < 0) {
	    printf ("can't open %s\n", imagename);
	    howto |= RB_ASKNAME;
	    continue;
	}
	printf ("Loading %s ...\n", imagename);
	
	load_image (io, 1);
    }
    howto |= RB_ASKNAME;
    close (io);
    io = -1;
}
