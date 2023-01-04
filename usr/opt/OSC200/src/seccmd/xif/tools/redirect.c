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
static char	*sccsid = "@(#)$RCSfile: redirect.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:11:24 $";
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



#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#ifdef AUX
#include <string.h>
#else
#include <strings.h>
#endif
#include <signal.h>

extern char
    *calloc(),
    *malloc(),
    *realloc(),
    *strdup(),
    *strrchr();
    
static int
	stricmp ();

main(argc, argv)
    int     argc;
    char    **argv;
{
    int     fd,
            i,
            n;
    char    *cp,
            **exargv,
            *executable,
            *executable_name,
            *pstderr,
            *pstdin,
            *pstdout;
    char    buf[50];
    int     pid,
            ret,
            wait_stat;
            
    /* Should only be called with at least 3 command line args */
    if (argc < 3) {
        printf("\nInsufficient command line arguments\n");
        goto error_out;
    }
    
    /* Build array of pointers to args */
    exargv = (char **) malloc(sizeof(char *) * argc);
    if (! exargv) {
        printf("Insufficient memory for arg list\n");
        goto error_out;
    }
    
    /* Flag no stdin, stdout, or stderr set */
    pstdin = NULL;
    pstdout = NULL;
    pstderr =  NULL;
    
    /* Pick up executable and name */
    executable = argv[1];
    cp = strrchr(executable, '/');
    if (! cp)
        cp = executable;
    else
        cp++;
    executable_name = cp;
    
    /* Start argument list for spawned creature */
    n = 0;
    exargv[n++] = executable_name;
    
    /* Scan through args and redirect where asked to */
    for (i = 2; i < argc; i++) {
        if (! stricmp(argv[i], "stdin")) {
            if (++i >= argc) {
                printf("\nStdin missing argument\n");
                sleep(10);
                exit(1);
            }
            pstdin = argv[i];
        } 
        else if (! stricmp(argv[i], "stdout")) {
            if (++i >= argc) {
                printf("Stdout missing argument\n");
                goto error_out;
            }
            pstdout = argv[i];
        }
        else if (! stricmp(argv[i], "stderr")) {
            if (++i >= argc) {
                printf("\nStderr missing argument\n");
                goto error_out;
            }
            pstderr = argv[i];
        }
        else
            exargv[n++] = argv[i];
    }
    
    exargv[n] = NULL;
    
    /* Fork and run the backup program */
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    switch (pid = fork())  {
        case    -1: /* error - can't fork sub-process */
            signal(SIGHUP, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            printf("\nFailed to fork\n");
            goto error_out;
            
        case    0:  /* child */
            /* Reset signals */
            signal(SIGHUP, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            setgid(getgid());
            if (pstdin) {
                fd = dup(fileno(stdin));
                close(fileno(stdin));
                if (open(pstdin,O_RDONLY) < 0) {
                    dup(fd);
                    close(fd);
                    printf("\nFailed to open stdin %s\n", pstdin);
                    sleep(10);
                    exit(0x7f);
                }
            }
            if (pstdout) {
                fd = dup(fileno(stdout));
                close(fileno(stdout));
                if (open(pstdout, O_WRONLY) < 0 
                &&  creat(pstdout, S_IWUSR) < 0
                ) {
                    dup(fd);
                    close(fd);
                    printf("\nFailed to open stdout %s\n", pstdout);
                    sleep(10);
                    exit(0x7f);
                }
            }
            if (pstderr) {
                fd = dup(fileno(stderr));
                close(fileno(stderr));
                if (open(pstderr, O_WRONLY) < 0
                &&  creat(pstderr, S_IWUSR) < 0) {
                    dup(fd);
                    close(fd);
                    printf("\nFailed to open stderr %s\n", pstderr);
                    sleep(10);
                    exit(0x7f);
                }
            }
            /* Run xterm program */
            execv(executable, exargv);
            /* Should never get here */
            exit(0x7f);
            
        default:
            wait(&wait_stat);
            /* Put signals back */
            signal(SIGHUP, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            if (! (wait_stat & 0xFF)) {
                /* terminated due to exit() */
                ret = (wait_stat >> 8) & 0xFF; /* exit status */
                if (ret == 0x7f)
                    ret = -1;
            }
            else 
                /* terminated by signal */
                ret = -1;
            break;
    }

error_out:
    puts("\nPress [Enter] to continue...\n");
    fgets(buf, 40, stdin);
    exit(0);
}    

static int
stricmp(a, b)
	register char *a;
	register char *b;
{
	for( ; *a && *b && (tolower(*a) == tolower(*b)); a++, b++ )
		;
	if (! *a && ! *b)
		return 0;
	if (! *a)
		return -1;
	if (! *b)
		return 1;
	return (tolower(*a) - tolower(*b));
}
