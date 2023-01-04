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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: utils.c,v 1.119 92/10/19 17:19:22 rws Exp $ */
#include "Xos.h"
#include <stdio.h>
#include "misc.h"
#include "X.h"
#include "input.h"
#include "opaque.h"
#include <signal.h>
#ifndef SYSV
#include <sys/resource.h>
#endif
#include <time.h>

#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif

extern char *display;

#if LONG_BIT == 64
extern int defaultScreenSaverTime;	/* for parsing command line */
extern int defaultScreenSaverInterval;
#else /* LONG_BIT == 32 */
extern long defaultScreenSaverTime;	/* for parsing command line */
extern long defaultScreenSaverInterval;
#endif /* LONG_BIT */
extern int defaultScreenSaverBlanking;
extern int defaultBackingStore;
extern Bool disableBackingStore;
extern Bool disableSaveUnders;
extern Bool PartialNetwork;
#ifndef NOLOGOHACK
extern int logoScreenSaver;
#endif
#ifdef RLIMIT_DATA
extern int limitDataSpace;
#endif
#ifdef RLIMIT_STACK
extern int limitStackSpace;
#endif
#ifdef RLIMIT_NOFILE
extern int limitNoFile;
#endif
extern int defaultColorVisualClass;
#if LONG_BIT == 64
extern int ScreenSaverTime;		/* for forcing reset */
#else /* LONG_BIT == 32 */
extern long ScreenSaverTime;		/* for forcing reset */
#endif /* LONG_BIT */
extern Bool permitOldBugs;
extern int monitorResolution;
extern Bool defeatAccessControl;

Bool CoreDump;
Bool noTestExtensions;
int auditTrailLevel = 1;

void ddxUseMsg();

#if !defined(SVR4) && !defined(hpux)
extern char *sbrk();
#endif

#ifdef AIXV3
#define AIXFILE "/tmp/aixfile"
FILE *aixfd;
int SyncOn  = 0;
extern int SelectWaitTime;
#endif

#ifdef ALPHA_DEBUG
unsigned long AlphaDebugLevel = 0L;	/* See levels in server/include/misc.h */
#endif

#ifdef DEBUG
#ifndef SPECIAL_MALLOC
#define MEMBUG
#endif
#endif

#ifdef MEMBUG
#define MEM_FAIL_SCALE 100000
long Memory_fail = 0;

#endif

#ifdef sgi
int userdefinedfontpath = 0;
#endif /* sgi */

Bool Must_have_memory = FALSE;

char *dev_tty_from_init = NULL;		/* since we need to parse it anyway */

/* Force connections to close on SIGHUP from init */

/*ARGSUSED*/
SIGVAL
AutoResetServer (sig)
    int sig;
{
    dispatchException |= DE_RESET;
    isItTimeToYield = TRUE;
#ifdef GPROF
    chdir ("/tmp");
    exit (0);
#endif
#ifdef SYSV
    signal (SIGHUP, AutoResetServer);
#endif
}

/* Force connections to close and then exit on SIGTERM, SIGINT */

/*ARGSUSED*/
SIGVAL
GiveUp(sig)
    int sig;
{
    dispatchException |= DE_TERMINATE;
    isItTimeToYield = TRUE;
}


static void
AbortServer()
{
    extern void AbortDDX();

    AbortDDX();
    fflush(stderr);
    if (CoreDump)
	abort();
    exit (1);
}

void
Error(str)
    char *str;
{
    perror(str);
}

#ifndef DDXTIME
long
GetTimeInMillis()
{
    struct timeval  tp;

    gettimeofday(&tp, 0L);
    return((long)(((tp.tv_sec * 1000) + (tp.tv_usec / 1000)) & 0xffffffff));
}
#endif

AdjustWaitForDelay (waitTime, newdelay)
    pointer	    waitTime;
    unsigned long   newdelay;
{
    static struct timeval   delay_val;
    struct timeval	    **wt = (struct timeval **) waitTime;
    unsigned long	    olddelay;

    if (*wt == NULL)
    {
	delay_val.tv_sec = newdelay / 1000;
	delay_val.tv_usec = 1000 * (newdelay % 1000);
	*wt = &delay_val;
    }
    else
    {
	olddelay = (*wt)->tv_sec * 1000 + (*wt)->tv_usec / 1000;
	if (newdelay < olddelay)
	{
	    (*wt)->tv_sec = newdelay / 1000;
	    (*wt)->tv_usec = 1000 * (newdelay % 1000);
	}
    }
}

static Bool exitOnUseMsg = FALSE;	/* display use messages and exit */
static Bool displayUseMsg = FALSE;	/* display use messages */

#define USE_MSG_PROC_LIST_SIZE 100
typedef void (*VoidProc) ();
static VoidProc useMsgProcList[USE_MSG_PROC_LIST_SIZE];
static int useMsgProcListIndex = 0;

static Bool * unusedArgvs;
static int    privateArgc;

/* Mark for displaying use messages later */
void UseMsg()
{
    displayUseMsg = TRUE;
}

/* Register a proc for displaying use messages */
void RegisterUseMsgProc(proc)
    VoidProc proc;
{
    if ( useMsgProcListIndex < USE_MSG_PROC_LIST_SIZE )
	useMsgProcList[useMsgProcListIndex++] = proc;
    else
	ErrorF("Out of usage message handler space.\n");
}
    
void DisplayUseMsg(argc, argv)
int	argc;
char	*argv[];
{
    register int i, j;
    static Bool Once = FALSE;
    Bool DisplayBadArg = FALSE;

    /* We only want to call this on the original server setup.
     * However, we need to call from the inner server loop since this
     * has to be called after loading screens and extensions. So just
     * do this once, not every time the server resets.
     */
    if ( Once == TRUE ) return;
    Once = FALSE;

    for ( i = 0; i < privateArgc; i++ )
	if ( unusedArgvs[i] == TRUE ) {
	    displayUseMsg = TRUE;
	    exitOnUseMsg = TRUE;
	    DisplayBadArg = TRUE;
	    break;
	}

    if ( !displayUseMsg ) return;

    if ( DisplayBadArg == TRUE ) {
	ErrorF("unrecognized argument(s) or invalid syntax:\n");
	for ( i = 0; i < argc; i++ )
	    ErrorF("%s ",argv[i]);
	ErrorF("\n");
	for ( i = 0; i < argc; i++ ) {
	    if ( unusedArgvs[i] == TRUE ) 
	        for ( j = 0; j < strlen(argv[i]); j++)
	    	    ErrorF("^");
	    else
	        for ( j = 0; j < strlen(argv[i]); j++)
	    	    ErrorF("-");
	    ErrorF("-");
	}
	ErrorF("\n\n");
    }
		
#if !defined(AIXrt) && !defined(AIX386)
    ErrorF("use: X [:<display>] [option]\n");
    ErrorF("-a #                   mouse acceleration (pixels)\n");
    ErrorF("-ac                    disable access control restrictions\n");
#ifdef MEMBUG
    ErrorF("-alloc int             chance alloc should fail\n");
#endif
    ErrorF("-audit int             set audit trail level\n");	
    ErrorF("-auth string           select authorization file\n");	
    ErrorF("bc                     enable bug compatibility\n");
    ErrorF("-bs                    disable any backing store support\n");
    ErrorF("-c                     turns off key-click\n");
    ErrorF("c #                    key-click volume (0-100)\n");
    ErrorF("-cc int                default color visual class\n");
    ErrorF("-co string             color database file\n");
    ErrorF("-core                  generate core dump on fatal error\n");
#if !defined (__osf__)
    ErrorF("-dpi int               screen resolution in dots per inch\n");
#endif
    ErrorF("-f #                   bell base (0-100)\n");
    ErrorF("-fc string             cursor font\n");
    ErrorF("-fn string             default font name\n");
    ErrorF("-fp string             default font path\n");
    ErrorF("-help                  prints message with these options\n");
    ErrorF("-I                     ignore all remaining arguments\n");
#ifdef RLIMIT_DATA
    ErrorF("-ld int                limit data space to N Kb\n");
#endif
#ifdef RLIMIT_NOFILE
    ErrorF("-lf int                limit number of open files to N\n");
#endif
#ifdef RLIMIT_STACK
    ErrorF("-ls int                limit stack space to N Kb\n");
#endif
#ifndef NOLOGOHACK
    ErrorF("-logo                  enable logo in screen saver\n");
    ErrorF("nologo                 disable logo in screen saver\n");
#endif
    ErrorF("-p #                   screen-saver pattern duration (minutes)\n");
    ErrorF("-pn                    accept failure to listen on all ports\n");
#ifdef DNETCONN
    ErrorF("pn                     do not accept failure to listen on all ports\n");
#endif
    ErrorF("-r                     turns off auto-repeat\n");
    ErrorF("r                      turns on auto-repeat \n");
    ErrorF("-s #                   screen-saver timeout (minutes)\n");
    ErrorF("-su                    disable any save under support\n");
    ErrorF("-t #                   mouse threshold (pixels)\n");
    ErrorF("-terminate             terminate at server reset\n");
    ErrorF("-to #                  connection time out\n");
    ErrorF("-tst                   disable testing extensions\n");
    ErrorF("ttyxx                  server started from init on /dev/ttyxx\n");
    ErrorF("v                      video blanking for screen-saver\n");
    ErrorF("-v                     screen-saver without video blanking\n");
    ErrorF("-wm                    WhenMapped default backing-store\n");
    ErrorF("-x string              loads named extension at init time \n");
#ifdef XDMCP
    XdmcpUseMsg();
#endif
#endif /* !AIXrt && ! AIX386 */
    ddxUseMsg();

    for ( i = 0 ; i < useMsgProcListIndex; i++ )
	if ( useMsgProcList[i] )
	    (useMsgProcList[i])();

    if ( exitOnUseMsg == TRUE )
	exit(0);
}

static void MarkArgvUnused (index, count)
    int index;
    int count;
{
    int i;
    if ( index < privateArgc )
	for ( i = index; i < (index+count); i++ )
	    unusedArgvs[i] = TRUE;
}
void MarkArgvUsed (index, count)
    int index;
    int count;
{
    int i;
    if ( index < privateArgc )
	for ( i = index; i < (index+count); i++ )
	    unusedArgvs[i] = FALSE;
}

/*
 * This function parses the command line. Handles device-independent fields
 * and allows ddx to handle additional fields.  It is not allowed to modify
 * argc or any of the strings pointed to by argv.
 */
void
ProcessCommandLine ( argc, argv )
int	argc;
char	*argv[];

{
    int i, skip;

    unusedArgvs = (int *)Xalloc(sizeof(Bool) * argc);
    for ( i = 0; i < argc; i++ )
	unusedArgvs[i] = FALSE;
    privateArgc = argc;

    defaultKeyboardControl.autoRepeat = TRUE;

#ifdef DNETCONN
    /* Default to allow partial network if built to allow DECnet */
    PartialNetwork = TRUE;
#endif

#ifdef AIXV3
    OpenDebug();
#endif
    for ( i = 1; i < argc; i++ )
    {
	/* call ddx first, so it can peek/override if it wants */
        if(skip = ddxProcessArgument(argc, argv, i))
	{
	    i += (skip - 1);
	}
	else if(argv[i][0] ==  ':')  
	{
	    /* initialize display */
	    display = argv[i];
	    display++;
	}
	else if ( strcmp( argv[i], "-a") == 0)
	{
	    if(++i < argc)
	        defaultPointerControl.num = atoi(argv[i]);
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-ac") == 0)
	{
	    defeatAccessControl = TRUE;
	}
#ifdef MEMBUG
	else if ( strcmp( argv[i], "-alloc") == 0)
	{
	    if(++i < argc)
	        Memory_fail = atoi(argv[i]);
	    else
		UseMsg();
	}
#endif
	else if ( strcmp( argv[i], "-audit") == 0)
	{
	    if(++i < argc)
	        auditTrailLevel = atoi(argv[i]);
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-auth") == 0)
	{
	    if(++i < argc)
	        InitAuthorization (argv[i]);
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "bc") == 0)
	    permitOldBugs = TRUE;
	else if ( strcmp( argv[i], "-bs") == 0)
	    disableBackingStore = TRUE;
	else if ( strcmp( argv[i], "c") == 0)
	{
	    if(++i < argc)
	        defaultKeyboardControl.click = atoi(argv[i]);
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-c") == 0)
	{
	    defaultKeyboardControl.click = 0;
	}
	else if ( strcmp( argv[i], "-cc") == 0)
	{
	    if(++i < argc)
	        defaultColorVisualClass = atoi(argv[i]);
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-co") == 0)
	{
	    if(++i < argc)
	        rgbPath = argv[i];
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-core") == 0)
	    CoreDump = TRUE;
#if !defined (__osf__)
	else if ( strcmp( argv[i], "-dpi") == 0)
	{
	    if(++i < argc)
	        monitorResolution = atoi(argv[i]);
	    else
		UseMsg();
	}
#endif
	else if ( strcmp( argv[i], "-f") == 0)
	{
	    if(++i < argc)
	        defaultKeyboardControl.bell = atoi(argv[i]);
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-fc") == 0)
	{
	    if(++i < argc)
	        defaultCursorFont = argv[i];
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-fn") == 0)
	{
	    if(++i < argc)
	        defaultTextFont = argv[i];
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-fp") == 0)
	{
	    if(++i < argc)
	    {
#ifdef sgi
		userdefinedfontpath = 1;
#endif /* sgi */
	        defaultFontPath = argv[i];
	    }
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-help") == 0)
	{
	    UseMsg();
	    exitOnUseMsg = TRUE;
	}
#ifdef RLIMIT_DATA
	else if ( strcmp( argv[i], "-ld") == 0)
	{
	    if(++i < argc)
	    {
	        limitDataSpace = atoi(argv[i]);
		if (limitDataSpace > 0)
		    limitDataSpace *= 1024;
	    }
	    else
		UseMsg();
	}
#endif
#ifdef RLIMIT_NOFILE
	else if ( strcmp( argv[i], "-lf") == 0)
	{
	    if(++i < argc)
	        limitNoFile = atoi(argv[i]);
	    else
		UseMsg();
	}
#endif
#ifdef RLIMIT_STACK
	else if ( strcmp( argv[i], "-ls") == 0)
	{
	    if(++i < argc)
	    {
	        limitStackSpace = atoi(argv[i]);
		if (limitStackSpace > 0)
		    limitStackSpace *= 1024;
	    }
	    else
		UseMsg();
	}
#endif
#ifndef NOLOGOHACK
	else if ( strcmp( argv[i], "-logo") == 0)
	{
	    logoScreenSaver = 1;
	}
	else if ( strcmp( argv[i], "nologo") == 0)
	{
	    logoScreenSaver = 0;
	}
#endif
	else if ( strcmp( argv[i], "-p") == 0)
	{
	    if(++i < argc)
#if LONG_BIT == 64
	        defaultScreenSaverInterval = ((int)atoi(argv[i])) *
					     MILLI_PER_MIN;
#else /* LONG_BIT == 32 */
	        defaultScreenSaverInterval = ((long)atoi(argv[i])) *
					     MILLI_PER_MIN;
#endif /* LONG_BIT */
	    else
		UseMsg();
	}
#ifdef DNETCONN
	else if ( strcmp( argv[i], "pn") == 0)
	    PartialNetwork = FALSE;
#endif
	else if ( strcmp( argv[i], "-pn") == 0)
	    PartialNetwork = TRUE;
	else if ( strcmp( argv[i], "r") == 0)
	    defaultKeyboardControl.autoRepeat = TRUE;
	else if ( strcmp( argv[i], "-r") == 0)
	    defaultKeyboardControl.autoRepeat = FALSE;
	else if ( strcmp( argv[i], "-s") == 0)
	{
	    if(++i < argc)
#if LONG_BIT == 64
	        defaultScreenSaverTime = ((int)atoi(argv[i])) * MILLI_PER_MIN;
#else /* LONG_BIT == 32 */
	        defaultScreenSaverTime = ((long)atoi(argv[i])) * MILLI_PER_MIN;
#endif /* LONG_BIT */
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-su") == 0)
	    disableSaveUnders = TRUE;
	else if ( strcmp( argv[i], "-t") == 0)
	{
	    if(++i < argc)
	        defaultPointerControl.threshold = atoi(argv[i]);
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-terminate") == 0)
	{
	    extern Bool terminateAtReset;
	    
	    terminateAtReset = TRUE;
	}
	else if ( strcmp( argv[i], "-to") == 0)
	{
	    if(++i < argc)
#if LONG_BIT == 64
		TimeOutValue = ((int)atoi(argv[i])) * MILLI_PER_SECOND;
#else /* LONG_BIT == 32 */
		TimeOutValue = ((long)atoi(argv[i])) * MILLI_PER_SECOND;
#endif /* LONG_BIT */
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-tst") == 0)
	{
	    noTestExtensions = TRUE;
	}
	else if ( strcmp( argv[i], "v") == 0)
	    defaultScreenSaverBlanking = PreferBlanking;
	else if ( strcmp( argv[i], "-v") == 0)
	    defaultScreenSaverBlanking = DontPreferBlanking;
	else if ( strcmp( argv[i], "-wm") == 0)
	    defaultBackingStore = WhenMapped;
	else if ( strcmp( argv[i], "-x") == 0)
	{
	    if(++i >= argc)
		UseMsg();
	    /* For U**x, which doesn't support dynamic loading, there's nothing
	     * to do when we see a -x.  Either the extension is linked in or
	     * it isn't */
	}
	else if ( strcmp( argv[i], "-I") == 0)
	{
	    /* ignore all remaining arguments */
	    break;
	}
	else if (strncmp (argv[i], "tty", 3) == 0)
	{
	    /* just in case any body is interested */
	    dev_tty_from_init = argv[i];
	}
#ifdef XDMCP
	else if ((skip = XdmcpOptions(argc, argv, i)) != i)
	{
	    i = skip - 1;
	}
#endif
#ifdef AIXV3
        else if ( strcmp( argv[i], "-timeout") == 0)
        {
            if(++i < argc)
                SelectWaitTime = atoi(argv[i]);
            else
                UseMsg();
        }
        else if ( strcmp( argv[i], "-sync") == 0)
        {
            SyncOn++;
        }
#endif
#ifdef ALPHA_DEBUG
        else if ( strcmp( argv[i], "-debuglevel") == 0 || 
		  strcmp( argv[i], "-debug") == 0 )
        {
            if(++i < argc)
		AlphaDebugLevel = (int)strtol(argv[i], (char *)0L, 0);
            else
                UseMsg();
	}
#endif
 	else
 	{
	    UseMsg();
	    MarkArgvUnused(i, 1);
        }
    }
}

/* XALLOC -- X's internal memory allocator.  Why does it return unsigned
 * int * instead of the more common char *?  Well, if you read K&R you'll
 * see they say that alloc must return a pointer "suitable for conversion"
 * to whatever type you really want.  In a full-blown generic allocator
 * there's no way to solve the alignment problems without potentially
 * wasting lots of space.  But we have a more limited problem. We know
 * we're only ever returning pointers to structures which will have to
 * be long word aligned.  So we are making a stronger guarantee.  It might
 * have made sense to make Xalloc return char * to conform with people's
 * expectations of malloc, but this makes lint happier.
 */

unsigned long * 
Xalloc (amount)
    unsigned long amount;
{
    char		*malloc();
    register pointer  ptr;
    volatile unsigned long tmpamount;
	
    if ((long)amount <= 0)
	return (unsigned long *)NULL;
    /* aligned extra on long word boundary */
#if LONG_BIT == 64
    amount = (amount + (unsigned long)7) & ~(unsigned long)7L;
#else /* LONG_BIT == 32 */
    amount = (amount + 3) & ~3;
#endif /* LONG_BIT */
#ifdef MEMBUG
    if (!Must_have_memory && Memory_fail &&
	((random() % MEM_FAIL_SCALE) < Memory_fail))
	return (unsigned long *)NULL;
#endif
    if (ptr = (pointer)malloc(amount)) 
#if LONG_BIT == 64
    if ( ((unsigned long)ptr & ~0x7UL) != (unsigned long)ptr )
	FatalError("bad alignment in Xalloc");
    else
#endif /* LONG_BIT == 32 */
	return (unsigned long *)ptr;
    if (Must_have_memory)
	FatalError("Out of memory");
    return (unsigned long *)NULL;
}

/*****************
 * Xcalloc
 *****************/

unsigned long *
Xcalloc (amount)
    unsigned long   amount;
{
    unsigned long   *ret;

    /* aligned extra on long word boundary */
#if LONG_BIT == 64
    amount = (amount + (unsigned long)7) & ~(unsigned long)7L;
#else /* LONG_BIT == 32 */
    amount = (amount + 3) & ~3;
#endif /* LONG_BIT */
    ret = Xalloc (amount);
    if (ret)
	bzero ((char *) ret, (int) amount);
#if LONG_BIT == 64
    if ( ((unsigned long)ret & ~0x7UL) != (unsigned long)ret )
	FatalError("bad alignment in Xcalloc");
#endif /* LONG_BIT */
    return ret;
}

/*****************
 * Xrealloc
 *****************/

unsigned long *
Xrealloc (ptr, amount)
    register pointer ptr;
    unsigned long amount;
{
    char *malloc();
    char *realloc();

#ifdef MEMBUG
    if (!Must_have_memory && Memory_fail &&
	((random() % MEM_FAIL_SCALE) < Memory_fail))
	return (unsigned long *)NULL;
#endif
    if ((long)amount <= 0)
    {
	if (ptr && !amount)
	    free(ptr);
	return (unsigned long *)NULL;
    }
#if LONG_BIT == 64
    amount = (amount + 7) & ~7L;
#else /* LONG_BIT == 32 */
    amount = (amount + 3) & ~3;
#endif /* LONG_BIT */
    if (ptr)
        ptr = (pointer)realloc((char *)ptr, amount);
    else
	ptr = (pointer)malloc(amount);
#if LONG_BIT == 64
    if ( ((unsigned long)ptr & ~0x7UL) != (unsigned long)ptr )
	FatalError("bad alignment in Xrealloc");
#endif /* LONG_BIT */
    if (ptr)
        return (unsigned long *)ptr;
    if (Must_have_memory)
	FatalError("Out of memory");
    return (unsigned long *)NULL;
}
                    
/*****************
 *  Xfree
 *    calls free 
 *****************/    

void
Xfree(ptr)
    register pointer ptr;
{
    if (ptr)
	free((char *)ptr); 
}

OsInitAllocator ()
{
#ifdef MEMBUG
    static int	been_here;

    /* Check the memory system after each generation */
    if (been_here)
	CheckMemory ();
    else
	been_here = 1;
#endif
}

/*VARARGS1*/
void
AuditF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9) /* limit of ten args */
    char *f;
    char *s0, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9;
{
#ifdef X_NOT_STDC_ENV
    long tm;
#else
    time_t tm;
#endif
    char *autime, *s;

    if (*f != ' ')
    {
	time(&tm);
	autime = ctime(&tm);
	if (s = index(autime, '\n'))
	    *s = '\0';
	if (s = rindex(argvGlobal[0], '/'))
	    s++;
	else
	    s = argvGlobal[0];
	ErrorF("AUDIT: %s: %d %s: ", autime, getpid(), s);
    }
    ErrorF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
}

/*VARARGS1*/
void
FatalError(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9) /* limit of ten args */
    char *f;
    char *s0, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9;
{
    ErrorF("\nFatal server error:\n");
    ErrorF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
    ErrorF("\n");
    AbortServer();
    /*NOTREACHED*/
}

/*VARARGS1*/
void
ErrorF( f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9) /* limit of ten args */
    char *f;
    char *s0, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9;
{
#ifdef AIXV3
    fprintf(aixfd, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
    fflush (aixfd);

    if (SyncOn)
        sync();
#else
    fprintf( stderr, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
#endif
}

#ifdef AIXV3
OpenDebug()
{
        if((aixfd = fopen(AIXFILE,"w")) == NULL )
        {
                fprintf(stderr,"open aixfile failed\n");
                exit(-1);
        }
        chmod(AIXFILE,00777);
}
#endif
