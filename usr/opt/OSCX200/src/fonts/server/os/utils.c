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
/* $XConsortium: utils.c,v 1.11 92/11/18 21:30:57 gildea Exp $ */
/*
 * misc os utilities
 */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include	<stdio.h>
#include	<X11/Xos.h>
#include	"misc.h"
#include	"globals.h"
#include	<signal.h>
#ifdef MEMBUG
#include	<util/memleak/memleak.h>
#endif

#ifndef X_NOT_POSIX
#ifdef _POSIX_SOURCE
#include <limits.h>
#else
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif
#endif /* X_NOT_POSIX */
#ifndef PATH_MAX
#include <sys/param.h>
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#endif /* PATH_MAX */

#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif

extern char *configfilename;
char       *progname;
Bool        CloneSelf;
int         ListenSock = -1;
extern int  ListenPort;

/* ARGSUSED */
SIGVAL
AutoResetServer(n)
    int n;
{

#ifdef DEBUG
    fprintf(stderr, "got a reset signal\n");
#endif

    dispatchException |= DE_RESET;
    isItTimeToYield = TRUE;

#ifdef SYSV
    signal(SIGHUP, AutoResetServer);
#endif
}

#ifdef ALPHA_DEBUG
#include <ctype.h>
unsigned long AlphaDebugLevel = 0;	/* See levels in server/include/misc.h */
#endif

SIGVAL
GiveUp()
{

#ifdef DEBUG
    fprintf(stderr, "got a TERM signal\n");
#endif

    dispatchException |= DE_TERMINATE;
    isItTimeToYield = TRUE;
}

/* ARGSUSED */
SIGVAL
ServerReconfig(n)
    int n;
{

#ifdef DEBUG
    fprintf(stderr, "got a re-config signal\n");
#endif

    dispatchException |= DE_RECONFIG;
    isItTimeToYield = TRUE;

#ifdef SYSV
    signal(SIGUSR1, ServerReconfig);
#endif
}

/* ARGSUSED */
SIGVAL
ServerCacheFlush(n)
    int n;
{

#ifdef DEBUG
    fprintf(stderr, "got a flush signal\n");
#endif

    dispatchException |= DE_FLUSH;
    isItTimeToYield = TRUE;

#ifdef SYSV
    signal(SIGUSR2, ServerCacheFlush);
#endif
}

long
GetTimeInMillis()
{
    struct timeval tp;

    gettimeofday(&tp, 0);
    return ((tp.tv_sec * 1000) + (tp.tv_usec / 1000));
}

static void
usage()
{
    fprintf(stderr, "%s: [-cf config-file] [-p tcp_port] [-s server_number]\n", progname);
    exit(-1);
}

OsInitAllocator ()
{
#ifdef MEMBUG
    CheckMemory ();
#endif
}

/* ARGSUSED */
void
ProcessCmdLine(argc, argv)
    int         argc;
    char      **argv;
{
    int         i;

    progname = argv[0];
    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-port")) {
	    if (argv[i + 1])
		ListenPort = atoi(argv[++i]);
	    else
		usage();
	} else if (!strcmp(argv[i], "-ls")) {
	    if (argv[i + 1])
		ListenSock = atoi(argv[++i]);
	    else
		usage();
	} else if (!strcmp(argv[i], "-cf") || !strcmp(argv[i], "-config")) {
	    if (argv[i + 1])
		configfilename = argv[++i];
	    else
		usage();
	}
#ifdef MEMBUG
	else if ( strcmp( argv[i], "-alloc") == 0)
	{
	    extern unsigned long    MemoryFail;
	    if(++i < argc)
	        MemoryFail = atoi(argv[i]);
	    else
		usage ();
	}
#endif
#ifdef ALPHA_DEBUG
        else if ( strcmp( argv[i], "-debuglevel") == 0)
        {
            if(++i < argc) {
                AlphaDebugLevel = (int)strtol(argv[i], (char *)0L, 0);
	    }
        }
#endif
	else
	    usage();
    }
}


#ifndef SPECIAL_MALLOC

unsigned long	Must_have_memory;

#ifdef MEMBUG
#define MEM_FAIL_SCALE	100000
unsigned long	MemoryFail;

#endif

/* FSalloc -- FS's internal memory allocator.  Why does it return unsigned
 * int * instead of the more common char *?  Well, if you read K&R you'll
 * see they say that alloc must return a pointer "suitable for conversion"
 * to whatever type you really want.  In a full-blown generic allocator
 * there's no way to solve the alignment problems without potentially
 * wasting lots of space.  But we have a more limited problem. We know
 * we're only ever returning pointers to structures which will have to
 * be long word aligned.  So we are making a stronger guarantee.  It might
 * have made sense to make FSalloc return char * to conform with people's
 * expectations of malloc, but this makes lint happier.
 */

unsigned long * 
FSalloc (amount)
    unsigned long amount;
{
    char		*malloc();
    register pointer  ptr;
	
    if ((long)amount < 0)
	return (unsigned long *)NULL;
    if (amount == 0)
	amount++;
    /* aligned extra on long word boundary */
    amount = (amount + 3) & ~3;
#ifdef MEMBUG
    if (!Must_have_memory && MemoryFail &&
	((random() % MEM_FAIL_SCALE) < MemoryFail))
	return (unsigned long *)NULL;
    if (ptr = (pointer)fmalloc(amount))
	return (unsigned long *) ptr;
#else
    if (ptr = (pointer)malloc(amount))
	return (unsigned long *)ptr;
#endif
    if (Must_have_memory)
	FatalError("Out of memory");
    return (unsigned long *)NULL;
}

/*****************
 * FScalloc
 *****************/

unsigned long *
FScalloc (amount)
    unsigned long   amount;
{
    unsigned long   *ret;

    ret = FSalloc (amount);
    if (ret)
	bzero ((char *) ret, (int) amount);
    return ret;
}

/*****************
 * FSrealloc
 *****************/

unsigned long *
FSrealloc (ptr, amount)
    register pointer ptr;
    unsigned long amount;
{
    char *malloc();
    char *realloc();

#ifdef MEMBUG
    if (!Must_have_memory && MemoryFail &&
	((random() % MEM_FAIL_SCALE) < MemoryFail))
	return (unsigned long *)NULL;
    ptr = (pointer)frealloc((char *) ptr, amount);
    if (ptr)
	return (unsigned long *) ptr;
#else
    if ((long)amount <= 0)
    {
	if (ptr && !amount)
	    free(ptr);
	return (unsigned long *)NULL;
    }
    amount = (amount + 3) & ~3;
    if (ptr)
        ptr = (pointer)realloc((char *)ptr, amount);
    else
	ptr = (pointer)malloc(amount);
    if (ptr)
        return (unsigned long *)ptr;
#endif
    if (Must_have_memory)
	FatalError("Out of memory");
    return (unsigned long *)NULL;
}
                    
/*****************
 *  FSfree
 *    calls free 
 *****************/    

void
FSfree(ptr)
    register pointer ptr;
{
#ifdef MEMBUG
    if (ptr)
	ffree((char *)ptr); 
#else
    if (ptr)
	free((char *)ptr); 
#endif
}

#ifdef ALPHA_DEBUG
/*VARARGS1*/
void
dprintf(level,f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9) /* limit of ten args */
    int level;
    char *f;
    char *s0, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9;
{
    if ( AlphaDebugLevel & level ) {
        fprintf( stderr, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
	fflush(stderr);
    }
}

#endif

#endif /* SPECIAL_MALLOC */
