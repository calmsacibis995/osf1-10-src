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

/* Get/Set Auditmask
    ULTRIX V4.x:    -Dultrix  -laud /sys/`machine`/BINARY/syscalls.o
    DEC OSF/1:      -D__osf__ -laud /sys/BINARY/syscalls.o
    object deselection "#ifdef notyet"'ed pending filesys support
*/

/*
    To add new audstyle flag:
     update audstyle_opt structure
*/

#include <sys/audit.h>
#ifndef	AUDIT_MASK_TYPE
#define	AUDIT_MASK_TYPE	char
#endif
#ifndef	SYSCALL_MASK_SIZE
#define	SYSCALL_MASK_SIZE	(sizeof(AUDIT_MASK_TYPE)*SYSCALL_MASK_LEN)
#endif
#ifndef	TRUSTED_MASK_SIZE
#define	TRUSTED_MASK_SIZE	(sizeof(AUDIT_MASK_TYPE)*TRUSTED_MASK_LEN)
#endif
#ifndef	AUDIT_MASK_LEN
#define	AUDIT_MASK_LEN		(SYSCALL_MASK_LEN+TRUSTED_MASK_LEN)
#endif
#ifndef	AUDIT_MASK_SIZE
#define	AUDIT_MASK_SIZE		(sizeof(AUDIT_MASK_TYPE)*AUDIT_MASK_LEN)
#endif
#ifndef	AUDMASK_SIZE
#define	AUDMASK_SIZE(n)		(sizeof(AUDIT_MASK_TYPE)*AUDMASK_LEN((n)))
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <nlist.h>
#include <sys/syscall.h>
#include <stdio.h>
#ifdef __osf__
#include <sys/systm.h>
#include <sys/habitat.h>
#endif /* __osf__ */


#ifdef ultrix
#define AUID_INVAL      0
#define AUDMASK_LEN(x)  (x > 0 ? ((x)-1)/(NBBY>>1)+1 : 0)
#define FIRST           0
#define LAST            (n_syscall-1)

struct nlist nlst[] = {
    { "_nsysent" },
#define X_NSYSENT      0
    { "_n_sitevents" },
#define X_N_SITEVENTS  1
    { 0 },
};
#endif /* ultrix */


static char *syscall_prefix[] = { "old", "alternate", "obs" };
static char *status[] = { "OFF", "ON" };


/* audstyle flags */
struct {
    char *cmnd;         /* audstyle command line name */
    int value;          /* audstyle constant value    */
} audstyle_opt[] = {
    { "exec_argp", AUD_EXEC_ARGP },
    { "exec_envp", AUD_EXEC_ENVP },
#ifdef __osf__
    { "login_uname", AUD_LOGIN_UNAME },
#endif /* __osf__ */
};


int n_sevents;
int n_syscall;


main ( argc, argv )
int argc;
char *argv[];
{
    AUDIT_MASK_TYPE buf1[AUDIT_MASK_LEN];
    AUDIT_MASK_TYPE *buf2 = &buf1[SYSCALL_MASK_LEN];

    struct stat statbuf;
    int flag_new;                   /* used for audstyle    */
    int flag_old;                   /* used for audstyle    */
    int pid = 0;                    /* used for updevents   */
    int cntl = -1;                  /* used for updevents   */
    uid_t auid = AUID_INVAL;        /* used for updevents   */
    AUDIT_MASK_TYPE *sitemask;      /* site event mask      */
    int verbose = 0;
    int i, j, k;

    /* get # of site events, n_syscall (ULTRIX) */
#ifdef __osf__
    n_sevents = audcntl ( GET_NSITEVENTS, (char *)0, 0, 0, 0, 0 );
#endif /* __osf__ */
#ifdef ultrix
    n_sevents = getkval ( X_N_SITEVENTS );
    if ( (n_syscall = getkval ( X_NSYSENT )) == -1 ) {
        perror ( "n_syscall" );
        exit(1);
    }
#endif /* ultrix */


    /* get auditmasks */
    if ( get_audmask ( buf1, buf2, &sitemask ) == -1 )
        exit(1);

    /* get audit style */
    if ( (flag_old = audcntl ( GET_AUDSTYLE, (char *)0, 0, 0, 0, 0 )) == -1 ) {
        perror ( "audcntl" );
        exit(1);
    }
    flag_new = flag_old;


    /* just display current auditmasks and style flag */
    fstat ( 0, &statbuf );
    if ( !(statbuf.st_mode&S_IFREG) && argc == 1 ) {
        show_audmask ( buf1, buf2, sitemask );
        show_audstyl ( flag_old );
        exit(0);
    }


    /* process command line args */
    for ( i = 1; i < argc; i++ ) {

        if ( argv[i][0] == '-' || argv[i][0] == '+' ) switch ( argv[i][1] ) {


        case 'a':   /* set auid */
            if ( i == argc-1 ) break;
            auid = atoi(argv[++i]);
            if ( (statbuf.st_mode & S_IFREG) == 0 )
                bzero ( buf1, AUDIT_MASK_SIZE );
            /* setting auditmask to null will prevent trying to set a sitemask
               for a process (which would only fail)
            */
            sitemask = (AUDIT_MASK_TYPE *)0;
            break;


        case 'c':   /* set cntl flag - "or", "and", "off", "usr" */
            if ( i == argc-1 ) break;
            if ( strncmp ( argv[++i], "or", 2 ) == 0 ) cntl = AUDIT_OR;
            else if ( strncmp ( argv[i], "and", 3 ) == 0 ) cntl = AUDIT_AND;
            else if ( strncmp ( argv[i], "off", 3 ) == 0 ) cntl = AUDIT_OFF;
            else if ( strncmp ( argv[i], "usr", 3 ) == 0 ) cntl = AUDIT_USR;
            else if ( argv[i][0] >= '0' && argv[i][0] <= '9' ) cntl = atoi(argv[i]);
            else fprintf ( stderr, "bad cntl value: %s\n", argv[i] );
            break;


#ifdef __osf__
#ifdef notyet
        case 'F':   /* set/reset OBJNOAUD bit on specified filelist */
            if ( i == argc-1 ) break;
            objaud_filelist ( argv[i+1], argv[i][0] == '+' ? 1 : 0, verbose );
            i++;
            break;

        case 'f':   /* set/reset OBJNOAUD bit on specified file */
            if ( i == argc-1 ) break;
            k = argv[i][0] == '+' ? 1 : 0;
            if ( (j = audcntl ( GET_OBJAUDBIT, argv[i+1], 0, 0, 0, 0 )) >= 0 && verbose )
                printf ( "%s: deselection %s => %s\n", argv[i+1], status[j], status[k] );
            if ( audcntl ( SET_OBJAUDBIT, argv[++i], 0, k, 0, 0 ) == -1 )
                perror ( argv[i] );
            break;
#endif /* notyet */
#endif /* __osf__ */


        case 'h':   /* show help */
            show_help();
            break;


        case 'n':   /* turn off event auditing */
            no_audit ( buf1, buf2, sitemask );
            break;


        case 'p':   /* updevents */
            if ( i == argc-1 ) break;
            pid = atoi(argv[++i]);
            if ( (statbuf.st_mode & S_IFREG) == 0 )
                bzero ( buf1, AUDIT_MASK_SIZE );
            /* setting auditmask to null will prevent trying to set a sitemask
               for a process (which would only fail)
            */
            sitemask = (AUDIT_MASK_TYPE *)0;
            break;


        case 's':   /* set audstyle flags */
            if ( i == argc-1 ) break;
            for ( i++, j = 0; argv[i][j] && argv[i][j] != ':'; j++ );
            k = ( argv[i][j] == ':' && argv[i][j+1] == '0' ); /* off <-> k==1 */
            argv[i][j] = '\0';
            for ( j = 0; j < sizeof(audstyle_opt)/sizeof(*audstyle_opt); j++ ) {
                if ( strcmp ( argv[i], audstyle_opt[j].cmnd ) == 0 ) {
                    if ( k == 1 ) flag_new &= ~audstyle_opt[j].value;
                    else flag_new |= audstyle_opt[j].value;
                    break;
                }
            }
            if ( j == sizeof(audstyle_opt)/sizeof(*audstyle_opt) )
                fprintf ( stderr, "bad flag value: %s\n", argv[i] );
            break;


        case 'v':   /* verbose */
            verbose++;
            break;


        default:    /* bad option */
            fprintf ( stderr, "auditmask: unknown option: %c ignored\n", argv[i][1] );
            break;
 
        }

        /* pass argument as an event to be incorporated into an auditmask */
        else change_audit_mask ( argv[i], buf1, buf2, sitemask, -1 );

    }


    /* if input redirected, set mask according to stdin */
    if ( statbuf.st_mode & S_IFREG ) input_file_audit ( buf1, buf2, sitemask );


    /* set audstyle */
    if ( flag_new != flag_old )
        if ( audcntl ( SET_AUDSTYLE, (char *)0, 0, flag_new, 0, 0 ) == -1 )
            perror ( "audcntl" );

    /* updevents for specified pid or auid */
    if ( pid || auid != AUID_INVAL ) {
        if ( audcntl ( UPDEVENTS, buf1, AUDIT_MASK_SIZE, cntl, auid, pid ) == -1 )
            perror ( "audcntl" );
    }

    /* set auditmasks */
    else {
        if ( audcntl ( SET_SYS_AMASK, buf1, SYSCALL_MASK_SIZE, 0, 0, 0 ) == -1 )
            perror ( "audcntl" );
        if ( audcntl ( SET_TRUSTED_AMASK, buf2, TRUSTED_MASK_SIZE, 0, 0, 0 ) == -1 )
            perror ( "audcntl" );
        if ( n_sevents > 0 && audcntl ( SET_SITEMASK, sitemask, AUDMASK_SIZE(n_sevents), 0, 0, 0 ) == -1 )
            perror ( "audcntl" );
    }
    exit(0);
}


/* add/remove audit event to/from appropriate mask
   habitat events get passed into audcntl
*/
change_audit_mask ( event, buf1, buf2, sitemask, override_flag )
char const *event;
AUDIT_MASK_TYPE *buf1;
AUDIT_MASK_TYPE *buf2;
AUDIT_MASK_TYPE *sitemask;
short override_flag;    /* cmd-line override of event-alias succeed/fail flag */
{
    char event_l[AUD_MAXEVENT_LEN+1];

    event_l[0] = '\0';	/* get it filled in by the library */

    if (audit_change_mask(event, buf1, buf2, sitemask, override_flag,
				event_l, sizeof event_l - 1)==0)
	return 0;

    /* event not found */
    event_l[sizeof event_l - 1] = '\0'; /* ensure NUL-termination */
    if ( sitemask ) fprintf ( stderr, "Can't find event %s\n", event_l );
    else fprintf ( stderr, "Can't apply event %s to a process-mask\n", event_l );
    return -1;
}


/* get audit masks */
int get_audmask ( sysmask, trustedmask, sitemask )
AUDIT_MASK_TYPE *sysmask;
AUDIT_MASK_TYPE *trustedmask;
AUDIT_MASK_TYPE **sitemask;
{
    int sitemasklen;

    /* get system auditmask */
    if ( audcntl ( GET_SYS_AMASK, sysmask, SYSCALL_MASK_SIZE, 0, 0, 0 ) == -1 ) {
        perror ( "audcntl" );
        return(-1);
    }

    /* get trusted auditmask */
    if ( audcntl ( GET_TRUSTED_AMASK, trustedmask, TRUSTED_MASK_SIZE, 0, 0, 0 ) == -1 ) {
        perror ( "audcntl" );
        return(-1);
    }

    /* get sitemask */
    if ( n_sevents > 0 ) {
        sitemasklen = AUDMASK_SIZE(n_sevents);
        if ( (*sitemask = (AUDIT_MASK_TYPE *)malloc(sitemasklen)) == (char *)0 ) {
            perror ( "sitemask" );
            sitemasklen = 0;
            return(-1);
        }
        if ( audcntl ( GET_SITEMASK, *sitemask, sitemasklen, 0, 0, 0 ) == -1 ) {
            perror ( "audcntl" );
            return(-1);
        }
    }
}


#ifdef ultrix
/* get integer value from kernel */
int getkval ( var )
int var;
{
    static int vm_fd = -1;
    static int km_fd;
    int i = 0;

    if ( vm_fd == -1 ) {
        if ( (vm_fd = open ( "/vmunix", 0 )) == -1 ) return(-1);
        if ( (km_fd = open ( "/dev/kmem", 0 )) == -1 ) return(-1);
        nlist ( "/vmunix", nlst );
        if ( nlst[0].n_type == 0 ) return (-1);
    }

    if ( lseek ( km_fd, nlst[var].n_value, 0 ) == -1 ) return(-1);
    read ( km_fd, &i, sizeof(int) );

    return ( i );
}
#endif /* ultrix */


/* parse input for change_audit_mask */
input_file_audit ( buf1, buf2, sitemask )
AUDIT_MASK_TYPE *buf1;     /* syscall mask       */
AUDIT_MASK_TYPE *buf2;     /* trusted event mask */
AUDIT_MASK_TYPE *sitemask; /* site event mask    */
{
    char inbuf[1024];
    char request[AUD_MAXEVENT_LEN];
    int start;
    int end;
    int i, j, k, l;

    for ( ; i = read ( 0, inbuf, sizeof inbuf ); ) {
        for ( j = 0; j < i; j++ ) {

            /* if don't have complete request, shift buffer and get more input */
            for ( ; inbuf[j] == '\n'; j++ );
            for ( end = j; end < i && inbuf[end] != '\n'; end++ );
            if ( end == i ) {
                for ( start = j; start < end; start++ )
                    inbuf[start-j] = inbuf[start];
                end -= j;
                j = -1;
                i = read ( 0, &inbuf[end], sizeof inbuf - end ) + end;
                continue;
            }

            /* ignore comments */
            if ( inbuf[j] == '!' ) {
                for ( ; inbuf[j] != '\n'; j++ );
                for ( ; inbuf[j] == '\n'; j++ );
                j--;
                continue;
            }


            /* format change_audit_mask request eventname */
            for ( k = 0; inbuf[j] != ' ' && inbuf[j] != '\t' && inbuf[j] != '\n'; j++ )
                request[k++] = inbuf[j];

            /* allow for syscall prefix names, such as "old", "alternate" */
            for ( l = 0; l < sizeof(syscall_prefix)/sizeof(*syscall_prefix); l++ ) {
                if ( strncmp ( request, syscall_prefix[l], k ) == 0 ) {
                    request[k++] = inbuf[j++];
                    for ( ; inbuf[j] != ' ' && inbuf[j] != '\t' && inbuf[j] != '\n'; j++ )
                        request[k++] = inbuf[j];
                    break;
                }
            }

            /* format :succeed:fail flags */
            for ( ; inbuf[j] == ' ' || inbuf[j] == '\t'; j++ );
            request[k++] = ':';
            if ( inbuf[j] != '\n' ) {
                request[k++] = !strncmp ( &inbuf[j], "succeed", 7 ) + '0';
                if ( request[k-1] == '1' )
                    for ( ; inbuf[j] != ' ' && inbuf[j] != '\t' && inbuf[j] != '\n'; j++ );
            }
            else request[k++] = '0';
            for ( ; inbuf[j] == ' ' || inbuf[j] == '\t'; j++ );
            request[k++] = ':';
            if ( inbuf[j] != '\n' ) {
                request[k++] = !strncmp ( &inbuf[j], "fail", 4 ) + '0';
                for ( ; inbuf[j] != '\n'; j++ );
            }
            else request[k++] = '0';
            request[k] = '\0';


            /* pass request to change_audit_mask to adjust mask */
            change_audit_mask ( request, buf1, buf2, sitemask, -1 );
        }
    }
}


/* zero audit masks */
no_audit ( buf1, buf2, buf3 )
AUDIT_MASK_TYPE *buf1;     /* syscall mask       */
AUDIT_MASK_TYPE *buf2;     /* trusted event mask */
AUDIT_MASK_TYPE *buf3;     /* site mask          */
{
#ifdef __osf__
    AUDIT_MASK_TYPE buf[MAXHABSYSCALL_LEN];
    int code;
#endif /* __osf__ */
    int i, j;

    bzero ( buf1, SYSCALL_MASK_SIZE );
    bzero ( buf2, TRUSTED_MASK_SIZE );
    if ( buf3 ) bzero ( buf3, AUDMASK_SIZE(n_sevents) );

#ifdef __osf__
    /* clear audited habitat calls */
    for ( i = 1; i < MAXHABITATS; i++ )
        for ( code = j = 0; j >= 0; code++ ) {
            code |= (i << HABITAT_SHIFT);
            j = audcntl ( GET_HABITAT_EVENT, buf, sizeof(buf), code, 0, 0 );
            if ( j > 0 ) audcntl ( SET_HABITAT_EVENT, buf, 0, 0, 0, 0 );
        }
#endif /* __osf__ */

}


#ifdef __osf__
#ifdef notyet
/* set/reset OBJNOAUD bit of set of files */
objaud_filelist ( fname, flag, verbose )
char *fname;    /* file containing list of files to touch  */
int flag;       /* code to audcntl SET_OBJAUDBIT operation */
int verbose;
{
    char inbuf[MAXPATHLEN];
    char request[MAXPATHLEN];
    int start;
    int end;
    int i, j, k;

    int fd;

    /* list of files */
    if ( (fd = open ( fname, 0 )) == -1 ) {
        fprintf ( stderr, "%s: ", fname );
        perror ( "open" );
        return(-1);
    }

    for ( ; i = read ( fd, inbuf, sizeof inbuf ); ) {
        for ( j = 0; j < i; j++ ) {

            /* if don't have complete request, shift buffer and get more input */
            for ( ; inbuf[j] == '\n'; j++ );
            for ( end = j; end < i && inbuf[end] != '\n'; end++ );
            if ( end == i ) {
                for ( start = j; start < end; start++ )
                    inbuf[start-j] = inbuf[start];
                end -= j;
                j = -1;
                i = read ( fd, &inbuf[end], sizeof inbuf - end ) + end;
                continue;
            }

            /* ignore leading whitespace */
            for ( ; inbuf[j] == ' ' || inbuf[j] == '\t'; j++ );

            /* ignore comments */
            if ( inbuf[j] == '!' ) {
                for ( ; inbuf[j] != '\n'; j++ );
                for ( ; inbuf[j] == '\n'; j++ );
                j--;
                continue;
            }

            /* update object's audbit */
            for ( k = 0; inbuf[j] != ' ' && inbuf[j] != '\t' && inbuf[j] != '\n'; j++ )
                request[k++] = inbuf[j];
            request[k] = '\0';
            if ( (k = audcntl ( GET_OBJAUDBIT, request, 0, 0, 0, 0 )) >= 0 && verbose )
                printf ( "%s: deselection %s => %s\n", request, status[k], status[flag] );
            if ( audcntl ( SET_OBJAUDBIT, request, 0, flag, 0, 0 ) == -1 )
                perror ( request );
        }
    }
}
#endif /* notyet */
#endif /* __osf__ */


/* display auditmask */
show_audmask ( buf1, buf2, sitemask )
AUDIT_MASK_TYPE *buf1;     /* syscall mask       */
AUDIT_MASK_TYPE *buf2;     /* trusted event mask */
AUDIT_MASK_TYPE *sitemask;
{
#ifdef __osf__
    AUDIT_MASK_TYPE buf[MAXHABSYSCALL_LEN];
    int code;
#endif /* __osf__ */
    int sitemasklen = AUDMASK_LEN(n_sevents);
    char eventname[AUD_MAXEVENT_LEN];
    int j, k;

#define	CELL_LIM	(8*sizeof(AUDIT_MASK_TYPE))
#define	CELL_ENT(b,i,e)	(((b)[(i)]>>(CELL_LIM-2-(e)%CELL_LIM)) & 0x3)
#define	CELL_NUM(i,e)	(((i)<<(2*sizeof(AUDIT_MASK_TYPE))) + ((e)>>1))

    /* display syscall mask */
    printf ( "!  Audited system calls:\n" );
    for ( j = 0; j < SYSCALL_MASK_LEN; j++ )
        for ( k = 0; k < CELL_LIM; k+=2 ) {
            if ( CELL_NUM(j,k) > LAST ) break;
            if ( CELL_ENT(buf1,j,k) ) {
                printf ( "%-25s", syscallnames[CELL_NUM(j,k)] );
                if ( CELL_ENT(buf1,j,k) & 0x2 )
                    printf ( "  succeed" );
                else printf ( "         " );
                if ( CELL_ENT(buf1,j,k) & 0x1 )
                    printf ( "  fail\n" );
                else printf ( "      \n" );
            }
        }

#ifdef __osf__
    /* display dynamically added syscalls */
    for ( j = 1; j < MAXHABITATS; j++ )
        for ( code = k = 0; k >= 0; code++ ) {
            code |= (j << HABITAT_SHIFT);
            k = audcntl ( GET_HABITAT_EVENT, buf, sizeof(buf), code, 0, 0 );
            if ( k > 0 ) {
                printf ( "%-25s", buf );
                printf ( "%s", k&0x02 ? "  succeed" : "         " );
                printf ( "%s", k&0x01 ? "  fail\n" : "      \n" );
            }
        }

#endif /* __osf__ */

    /* display trusted event mask */
    printf ( "\n!  Audited trusted events:\n" );
    for ( j = 0; j < TRUSTED_MASK_LEN; j++ )
        for ( k = 0; k < CELL_LIM; k+=2 ) {
            if ( CELL_ENT(buf2,j,k) ) {
                printf ( "%-25s", trustedevent[CELL_NUM(j,k)] );
                if ( CELL_ENT(buf2,j,k) & 0x2 )
                    printf ( "  succeed" );
                else printf ( "         " );
                if ( CELL_ENT(buf2,j,k) & 0x1 )
                    printf ( "  fail\n" );
                else printf ( "      \n" );
            }
        }

    /* display site event mask */
    if ( n_sevents ) {
        for ( j = 0; j < sitemasklen && sitemask[j] == 0; j++ );
        if ( j < sitemasklen ) printf ( "\n!  Site events:\n" );
    }
    for ( j = 0; j < sitemasklen; j++ )
        for ( k = 0; k < CELL_LIM; k+=2 ) {
            if ( CELL_ENT(sitemask,j,k) ) {
                if ( aud_sitevent ( MIN_SITE_EVENT+CELL_NUM(j,k), -1, eventname, '\0' ) == -1 )
                    continue;
                printf ( "%-25s", eventname );
                if ( CELL_ENT(sitemask,j,k) & 0x2 )
                    printf ( "  succeed" );
                else printf ( "         " );
                if ( CELL_ENT(sitemask,j,k) & 0x1 )
                    printf ( "  fail\n" );
                else printf ( "      \n" );
            }
        }
}


/* display audstyle flags */
show_audstyl ( flag )
int flag;
{
    int i;

    if ( flag ) {
        printf ( "\n!  Audstyle flags: " );
        for ( i = 0; i < sizeof(audstyle_opt)/sizeof(*audstyle_opt); i++ )
            if ( flag & audstyle_opt[i].value )
                printf ( "%s ", audstyle_opt[i].cmnd );
        printf ( "\n" );
    }
}


/* print brief help summary */
show_help()
{
    int i;

    fprintf ( stderr, "auditmask usage: [options] [events] [< event_list]\n" );

    fprintf ( stderr, "\nOptions:\n" );
    fprintf ( stderr, "  -a auid          Apply operation to processes owned by specified auid\n" );
    fprintf ( stderr, "  -c audcntl       Apply audcntl flag to target process[es]\n" );
    fprintf ( stderr, "  -n               Clear system auditmask\n" );
    fprintf ( stderr, "  -p pid           Apply operation to process with specified pid\n" );
    fprintf ( stderr, "  -s audstyle      Set system audstyle flag\n" );
    fprintf ( stderr, "                   { " );
    for ( i = 0; i < sizeof(audstyle_opt)/sizeof(*audstyle_opt); i++ )
        fprintf ( stderr, "%s ", audstyle_opt[i].cmnd );
    fprintf ( stderr, "}\n" );

#ifdef __osf__
#ifdef notyet
    fprintf ( stderr, "\nOptions for object deselection:\n" );
    fprintf ( stderr, "  -v               Verbose (must precede 'f'/'F' arguments)\n" );
    fprintf ( stderr, "  {+/-}f filename  Enable/disable deselection for filename\n" );
    fprintf ( stderr, "  {+/-}F filelist  Enable/disable deselection for files in filelist\n" );
#endif /* notyet */
#endif /* __osf__ */
}
