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
static char	*sccsid = "@(#)$RCSfile: msgprep.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:11:20 $";
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
    filename:
        msgprep.c
        
    copyright:
        Copyright (c) 1989  SKM, L.P.
        ALL RIGHTS RESERVED
        
    functions:
        prepare message source file for application
*/

/******** COMMON HEADER INFO *********/
/* Common C include files */
#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#ifdef AUX
#include <sys/dir.h>
#define dirent  direct
#else
#include <dirent.h>
#endif

extern FILE
    *fopen();
    
extern int
    errno,
    strcmp(),
    Parse();
    
extern void
    Clean();

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define TRUE 1
#define FALSE 0
/******** END COMMON HEADER INFO *********/

static char *version = "\nMessage Preparation Utility Program, Version 1.0";
static char *copyright = "\
Copyright (c) 1989 by SKM, L.P.\n\
ALL RIGHTS RESERVED\n";

#define CHECK 19890301L
#define MAX_CLASS 500
#define TAG_LENGTH 49
#define LINE_SIZE  512

typedef struct {
     char           c_tag[TAG_LENGTH+1];    /* help class */
     long           c_pos;                  /* offset in message file */
     unsigned int   c_len;                  /* length of class entry */
     int            c_num;                  /* number of message entries */
} CLASS_REC;

static int ClassCmp();

main(argc, argv)
    int     argc;
    char    **argv;
{
    int     i,
            last_num_items,
            num_items,
            num_class,
            need_bang,
            no_bang,
            rc;
    char    buf[LINE_SIZE],
            *cp,
            *dcp,
            pbuf[LINE_SIZE],
            *scp;
    long    check = CHECK;
    time_t  timecheck;
    FILE    *ifp,
            *ofp;
    struct stat 
            inbuf;
    CLASS_REC 
            class[MAX_CLASS];

    /* Put version and copyright on screen */
    puts(version);
    puts(copyright);
    
    /* Make certain two command line arguments */
    if (argc != 3) {
        puts("Usage: msgprep source_file destination_file");
        exit(1);
    }
   
    ifp = fopen(argv[1], "r");
    if (! ifp) {
        puts("Error: failed to open source file.");
        exit (1);
    }
    stat(argv[1], &inbuf);
    timecheck = inbuf.st_mtime;

    ofp = fopen(argv[2], "w");
    if (! ofp) {
        puts("Error: failed to open destination file.");
        exit(1);
    }

    num_class = -1;
    num_items =  0;
    
    while(fgets(buf,133,ifp)) {
    
        cp = buf;
        i = Parse(buf, pbuf);
        
        if (! stricmp(pbuf, "!class")) {
        
            /* Parse class label from !class line */
            cp += i;
            i = Parse(cp, pbuf);
            if (! i) {
                puts("Error: missing class name after !class.");
                exit(1);
            }
            
            /* Class label small enough for data structure? */
            if (strlen(pbuf) > TAG_LENGTH) {
                printf("Error: class label '%s' too long.", pbuf);
                exit(1);
            }

            /* Tie up loose ends on last class */
            if (num_class >= 0) {
                class[num_class].c_len = ftell(ofp) -
                                             class[num_class].c_pos;
                class[num_class].c_num = num_items;
            }    
 
            /* Room for next class? */
            num_class++;
            if (num_class >= MAX_CLASS) {
                puts("Error: too many !class statements.");
                exit (1);
            }
            
            /* Prep new class entry */
            num_items = 0;
            strcpy(class[num_class].c_tag, pbuf);
            class[num_class].c_pos = ftell(ofp);
            cp += i;
            i = Parse(cp, pbuf);
            if (i && ! strcmp(pbuf, "!")) {
                no_bang = FALSE;
                cp += i;
            }
            else
                no_bang = TRUE;
            if (Parse(cp, pbuf)) {
                Clean(cp);
                printf("\nClass: %s\n*****: %s\n\n", 
                            class[num_class].c_tag, cp);
            }
            else
                printf("\nClass: %s\n\n", class[num_class].c_tag);
            last_num_items = -1;
            continue;
        }
    
        /* Is the next line a comment line? */
        if (! stricmp(pbuf,"!comment")) {
            Clean(cp);
            printf("** %s\n", cp);
            continue;
        }

        /* First line must be a !class line */
        if (num_class < 0) {
            puts("Error: First line in source file must be a '!class' line");
            exit(1);
        }
        
        /* Pick up a text line */
        if (no_bang || strcmp(pbuf,"!")) {
        
            /* Show message text line on screen */
            if (last_num_items != num_items)
                printf("%2d %s", num_items, buf);
            else
                printf("    %s", buf);
            last_num_items = num_items;
            
            /* Scan message line and convert \n \t \v \b \r \f \\ */
            for (scp = dcp = buf; *scp; scp++, dcp++) {
                if (*scp != '\\') {
                    *dcp = *scp;
                    continue;
                }
                scp++;
                switch(*scp) {
                    case 'n':  *dcp = '\n'; break;
                    case 't':  *dcp = '\t'; break;
                    case 'v':  *dcp = '\v'; break;
                    case 'b':  *dcp = '\b'; break;
                    case 'r':  *dcp = '\r'; break;
                    case 'f':  *dcp = '\f'; break;
                    case '\\': *dcp = '\\'; break;
                    default:
                        puts("Error: bad backslash escape sequence in source file.");
                        exit(1);
                }
            }
            *dcp = '\0';
            
            /* Put message in file */
            fputs(buf, ofp);
            if (no_bang) {
                fseek(ofp, -1l, SEEK_CUR);
                putc('\0', ofp);
                num_items++;
                need_bang = FALSE;
            }
            else
                need_bang = TRUE;
            continue;
        }
        
        /* Found terminating ! */
        if (! need_bang) {
            puts("Error: Terminating '!' without a message to terminate.");
            exit(1);
        }
        fseek(ofp, -1l, SEEK_CUR);
        putc('\0', ofp);
        num_items++;
        need_bang = FALSE;                   
    }

    /* Catch no !class entries */
    if (num_class < 0) {
        puts("Error: no message classes found in source file.");
        exit(1);
    }
    
    /* Clean up last entry */
    class[num_class].c_len = ftell(ofp) - class[num_class].c_pos;
    class[num_class].c_num = num_items;

    /* Now can safely up the count of num_class, finished with 0.. index */
    num_class++;
    
    /* Catch missing trailing ! */
    if (need_bang) {
        puts("Error: missing '!' line after last message.");
        exit(1);
    }

    printf("\nMagic number = %ld\n", check);
    printf("\nTimecheck    = %ld\n\n", timecheck);

    /* Sort help entries and write out class data structures */
    qsort(class, num_class, sizeof(CLASS_REC), ClassCmp);
#ifdef MSDOS_or_OS2
    setmode(fileno(ofp), O_BINARY);
    fflush(ofp);
#endif
    fseek(ofp, 0l, SEEK_END);
    fwrite(class, sizeof(CLASS_REC), num_class, ofp);
    fwrite(&num_class, sizeof(int), 1, ofp);
    fwrite(&timecheck, sizeof(time_t), 1, ofp);
    fwrite(&check, sizeof(long), 1, ofp);
    if (ferror(ofp)) {
        puts("Error: failed to write destination file.");
        exit(1);
    }
    
    /* Close files and end */
    fclose(ifp);
    fclose(ofp);
    exit(0);
}

static int ClassCmp(a, b)
    CLASS_REC   *a,
                *b;
{
    return(stricmp(a->c_tag, b->c_tag));
}

/* 
    function:
        provide a parsing function

    Parse(from,to)
    char *from,*to;
    
        from points to a string which parse will search for text
        text may be embedded in either single or double quotes
        embedded quote marks of type equivalent to demarcation quote
            marks must be doubled
        text field will automatically be terminated at the end of a line
        returns: index increment to point to byte after text
        returns: 0 if no text found before EOS
*/

Parse(from, to)
    char    *from;
    char    *to;
{
    int     c,
            qflag;
    char    *start;
    
    start = from;

    /* LOCATE START OF TEXT FIELD */
    while 
    (
          (c = *from++) != '\'' && c != '"'
          &&   c
          && ( c <= ' ' || c == ',' || c == 0x7f ) 
    ) ;

    if (! c) {
        *to = '\0';
        return 0;
    }

    /* SET QUOTE FLAG */
    if ( c == '\'' || c == '"' )
        qflag = c ;
    else {
        qflag = 0;
        *to++ = c;
    }

    if ( qflag ) {
        while ( (c = *from++) && ( c != qflag || *from++ == qflag ) )
            *to++ = c;
        *to = '\0';
        if ( ! c || c == qflag )
            from--;
    }
    else {
        while ( (c = *from++) && c > ' ' && c != ',' && c < 0x7f ) 
            *to++ = c;
        *to = '\0';
        if ( ! c )
            from--;
    }
    return ( from - start );
}

/*
    filename:
        clean.c
        
    function:    
        Remove leading, trailing blanks and trailing newline from
        a character string
        
        Filter high bit characters into 0-127 range
*/

void Clean(string)
register
char    *string;
{
    register
    char    *dp,
            *sp;
            
    for (dp = sp = string; (*sp & 0x7f) == ' ' ; sp++)
        ;
    while( *dp++ = *sp++ & 0x7f ) ;
    dp--; /* point to EOS */
    dp--; /* point to last char */
    if (dp >= string && *dp == '\n')
        dp--;
    while (dp >= string && *dp == ' ')
        dp--;
    *(++dp) = '\0';
}

/* 
*   stricmp()
*       case insensitive compare
*/

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


