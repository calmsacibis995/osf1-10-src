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
/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: Xmos.c,v $ $Revision: 1.1.4.6 $ $Date: 1993/12/17 21:19:49 $"
#endif
#endif
/*
*  (c) Copyright 1989, 1992 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <stdio.h>

#ifdef __cplusplus
extern "C" { /* some 'locale.h' do not have prototypes (sun) */
#endif

/*
 * (On AXP Systems)
 * Conditionalize the following, Xmosp.h will include Xlocale.h after stdlib.h
 * if VMS && X_LOCALE are defined.  If Xlocale.h is included before stdlib.h
 * MB_CUR_MAX will be redefined by stdlib.h causing a compilation error.
 */
#if !defined(VMS) || !defined(X_LOCALE)
#include <X11/Xlocale.h>
#endif

#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration */
#endif /* __cplusplus */

#include <X11/Xos.h>

#ifdef VMS
#include <descrip.h>
#include <rmsdef.h>
#include <stsdef.h>
#include <fscndef.h>
#include <time.h>
#include <signal.h>	    /* sleep() */
#include <lib$routines.h>
#else
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <limits.h>
#include <io.h> /* for emulted readdir routine (jtf) */
#endif /* WIN32 */
#endif

#ifdef WIN32
#include <X11\Xlib_NT.h>
#endif

#ifndef WIN32
#include <sys/time.h>  /* For declaration of select(). */
#endif

#if defined(NO_REGCOMP) && !defined(NO_REGEX)
#ifdef __sgi
extern char *regcmp();
#else
#if defined(SVR4) || defined(SYSV)
#include <libgen.h>
#endif /* sysv */
#endif /* __sgi */
#endif /* NO_REGEX */

#ifndef NO_REGCOMP
#include <regex.h>
#endif /* NO_REGCOMP */

#ifdef SYS_DIR
#include <sys/dir.h>
#else
#ifdef NDIR
#include <ndir.h>
#else
#ifdef __apollo
#include <sys/dir.h>
#else
#include <sys/types.h>
#ifndef WIN32
#include <dirent.h>
#endif
#endif
#endif
#endif
#endif	/*  ifdef VMS */

#include <sys/stat.h>
#ifndef VMS
#ifndef MCCABE
#ifndef WIN32
#include <pwd.h>
#endif /* WIN32 */
#endif
#endif	/* ifndef VMS */

#include <Xm/XmosP.h>

#ifdef USE_GETWD
#include <sys/param.h>
#define MAX_DIR_PATH_LEN    MAXPATHLEN
#else
#define MAX_DIR_PATH_LEN    1024
#endif
#define MAX_USER_NAME_LEN   256

#ifndef S_ISDIR
#define S_ISDIR(m) ((m & S_IFMT)==S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(m) ((m & S_IFMT)==S_IFREG)
#endif

#define FILE_LIST_BLOCK 64

#ifdef VMS
#define MAXFILELENGTH	256
#define PARENTDIR "[-]"
#define DIR_EXTENSION ".DIR"
typedef struct _dir
    {
    struct dsc$descriptor_s     filespec$dsc;
    struct dsc$descriptor_s     defspec$dsc;
    struct dsc$descriptor_s     result$dsc;
    unsigned long int		filecontext;
    unsigned long int		status;
    char			buffer[MAXFILELENGTH];
    } DIR;

struct dirent
    {
    char			d_name[MAXFILELENGTH];
    unsigned short int		d_namlen;
    };
#endif /* VMS */

#ifdef WIN32

#define S_IFMT _S_IFMT
#define S_IFREG _S_IFREG

struct dirent {
    char d_name[256];
};

typedef struct _DIR {
    long search_handle;
    struct _finddata_t findinfo;
    char firsttime; /* Boolean */
    struct dirent Dirent;
} DIR;

#endif /* WIN32 */

/********
set defaults for resources that are implementation dependant
and may be modified.
********/ 

#ifdef DEC_MOTIF_EXTENSION
externaldef(_xmsdefault_font) char _XmSDEFAULT_FONT[] = DXmDefaultFont;
#else
#ifdef DEC_MOTIF_BUG_FIX
externaldef(_xmsdefault_font) char _XmSDEFAULT_FONT[] = "fixed";
#else
char _XmSDEFAULT_FONT[] = "fixed";
#endif
#endif

#ifdef DEC_MOTIF_BUG_FIX
externaldef(_xmsdefault_background) char _XmSDEFAULT_BACKGROUND[] = "#729FFF";
#else
char _XmSDEFAULT_BACKGROUND[] = "#729FFF";
#endif
/**************** end of vendor dependant defaults ********/

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static String GetQualifiedDir() ;
#ifndef WIN32 /* No "regular expressions" in NT */
static String GetFixedMatchPattern() ;
#endif

#ifdef VMS
static DIR *		opendir() ;
static void		closedir() ;
static struct dirent *	readdir() ;
static Boolean		IsRemoteFilespec_VMS();
#endif /* VMS */

#ifdef WIN32
static DIR *		opendir();
static void		closedir() ;
static struct dirent *	readdir() ;
#endif /* WIN32 */

#else

static String GetQualifiedDir( 
                        String dirSpec) ;
#ifndef WIN32 /* No "regular expressions" in NT */
static String GetFixedMatchPattern( 
                        String pattern) ;
#endif

#ifdef VMS
static DIR *		opendir(char * dirspec, char * filterspec);
static void		closedir( DIR *dp ) ;
static struct dirent *	readdir( DIR *dp ) ;
static Boolean		IsRemoteFilespec_VMS( char * entryPtr);
#endif /* VMS */

#ifdef WIN32
static DIR *		opendir( char * dirspec );
static void		closedir( DIR *dp );
static struct dirent *	readdir( DIR *dp, char *pattern );
#endif /* WIN32 */

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/****************************************************************/
static String
#ifdef _NO_PROTO
GetQualifiedDir( dirSpec)
            String          dirSpec ;
#else
GetQualifiedDir(
            String          dirSpec)
#endif
/*************GENERAL:
 * dirSpec is a directory name, that can contain relative 
 *   as well as logical reference. This routine resolves all these
 *   references, so that dirSpec is now suitable for open().
 * The routine allocates memory for the result, which is guaranteed to be
 *   of length >= 1.  This memory should eventually be freed using XtFree().
 ****************/
#ifdef VMS
{
    struct dsc$descriptor_s     fs$dsc;
    struct fscndef		fscn$item[2];
    unsigned long int		status;
    char *			outputBuf;
    int				len;
    char                        buffer[MAXFILELENGTH];
    char			endBracket;

    if (strlen(dirSpec) == 0)
    {
	(void)getcwd(buffer, MAXFILELENGTH, 1);
	outputBuf = XtNewString(buffer);
    }
    else
    {
	fs$dsc.dsc$w_length = strlen(dirSpec);
	fs$dsc.dsc$b_dtype = DSC$K_DTYPE_T;
	fs$dsc.dsc$b_class = DSC$K_CLASS_S;
	fs$dsc.dsc$a_pointer = dirSpec;
	fscn$item[0].fscn$w_length = 0;
	fscn$item[0].fscn$w_item_code = FSCN$_NAME;
	fscn$item[0].fscn$l_addr = 0;
	fscn$item[1].fscn$w_length = 0;
	fscn$item[1].fscn$w_item_code = 0;  /* end of item list */
	fscn$item[1].fscn$l_addr = 0;
	status = SYS$FILESCAN(&fs$dsc, &fscn$item, 0);
	if ($VMS_STATUS_SUCCESS(status) && (fscn$item[0].fscn$l_addr != 0))
	{
	    outputBuf = XtMalloc(fs$dsc.dsc$w_length);		/* a little bigger than we really need */
	    len = (char *)fscn$item[0].fscn$l_addr - dirSpec;	/* length is end addr - start addr */
	    strncpy(outputBuf, dirSpec, len);			/* copy up to, but not including, the end bracket */
	    endBracket = dirSpec[len-1];			/* save end bracket char - could be ']' or '>' */
	    outputBuf[len-1] = '.';				/* copy in .dirname */
	    strncpy(&outputBuf[len],
		    (char *)(fscn$item[0].fscn$l_addr),
		    fscn$item[0].fscn$w_length);
	    len += fscn$item[0].fscn$w_length;
	    outputBuf[len++] = endBracket;			/* put the bracket back on the end */
	    outputBuf[len] = '\0';				/* and terminate it */
	}
	else
	{
	    outputBuf = XtNewString(dirSpec);
	}
    }
    return(outputBuf);	    
}
#else
#ifdef WIN32
/************* Windows NT (based on Unix code):
 * Builds directory name showing descriptive path components.  The result
 *   is a directory path beginning at the root directory and terminated
 *   with a '\'.  The path will not contain ".", "..", or "~" components.
 ****************/
{
            int             dirSpecLen ;
            struct passwd * userDir ;
            int             userDirLen ;
            int             userNameLen ;
            char *          outputBuf ;
            char *          destPtr ;
            char *          srcPtr ;
            char *          scanPtr ;
            char            nameBuf[MAX_USER_NAME_LEN] ;
            char            dirbuf[MAX_DIR_PATH_LEN] ;
            int            drive;

    dirSpecLen = strlen( dirSpec) ;
    outputBuf = NULL ;

    /* see if it starts with a drive letter */
    if ((dirSpecLen > 2) && (dirSpec[1] == ':'))
    {
        drive = tolower(*dirSpec) - 'a' + 1;
        dirSpec += 2;
        dirSpecLen -= 2 ;
    }
    else    /* doesn't start with a drive letter - use the current drive */
        drive = _getdrive();

    /* if it's relative to the root dir, just copy it */
    if ( *dirSpec == '\\')
    {
        outputBuf = XtMalloc( dirSpecLen + 4) ;
        outputBuf[0] = 'A' + drive - 1 ;
        outputBuf[1] = ':' ;
        strcpy( outputBuf+2, dirSpec) ;
    }

    /* not relative to root, must be relative to current directory for drive */
    else
    {
        destPtr = (char*)_getdcwd( drive, dirbuf, MAX_DIR_PATH_LEN) ;
        if(    destPtr    )
        {
            userDirLen = strlen( destPtr) ;
            outputBuf = XtMalloc( userDirLen + dirSpecLen + 3) ;
            strcpy (outputBuf, destPtr) ;
            outputBuf[userDirLen++] = '\\';
            strcpy (&outputBuf[userDirLen], dirSpec) ;
        }
    }

    if(    !outputBuf    )
    {   outputBuf = XtMalloc( 4 ) ;
        outputBuf[0] = 'A' + drive - 1 ;
        outputBuf[1] = ':' ;
        outputBuf[2] = '\\' ;
        outputBuf[3] = '\0' ;
    }
    else
    {   userDirLen = strlen( outputBuf) ;
        if(    outputBuf[userDirLen - 1]  !=  '\\'    )
        {   outputBuf[userDirLen] = '\\' ;
            outputBuf[++userDirLen] = '\0' ;
        }
        /* The string in outputBuf is assumed to begin with x:\ (x = drive #) and end with a '\'.
        */
        scanPtr = outputBuf + 2;
        while(    *++scanPtr    )               /* Skip past '\'. */
        {   /* scanPtr now points to non-NULL character following '\'.
            */
            if(    scanPtr[0] == '.'    )
            {
                if(    scanPtr[1] == '\\'    )
                {   /* Have ".\", so just erase (overwrite with shift).
                    */
                    destPtr = scanPtr ;
                    srcPtr = &scanPtr[2] ;
                    while(    *destPtr++ = *srcPtr++    )
                    {   }
                    --scanPtr ;     /* Leave scanPtr at preceding '\'. */
                    continue ;
                }
                else
                {   if(    (scanPtr[1] == '.')  &&  (scanPtr[2] == '\\')    )
                    {   /* Have "..\", so back up one directory.
                        */
                        srcPtr = &scanPtr[2] ;
                        --scanPtr ;      /* Move scanPtr to preceding '\'.*/
                        if(    scanPtr != outputBuf    )
                        {   while(    (*--scanPtr != '\\')    )
                            {   }          /* Now move to previous '\'.*/
                        }
                        destPtr = scanPtr ;
                        while(    *++destPtr = *++srcPtr    )
                            {   }           /* Overwrite "..\" with shift.*/
                        continue ;
                    }
                }
            }
            while(    *++scanPtr != '\\'    )
		{   }
	}
    }
	    return( outputBuf) ;
}

#else
/*************UNIX:
 * Builds directory name showing descriptive path components.  The result
 *   is a directory path beginning at the root directory and terminated
 *   with a '/'.  The path will not contain ".", "..", or "~" components.  
 ****************/
{
            int             dirSpecLen ;
            struct passwd * userDir ;
            int             userDirLen ;
            int             userNameLen ;
            char *          outputBuf ;
            char *          destPtr ;
            char *          srcPtr ;
            char *          scanPtr ;
            char            nameBuf[MAX_USER_NAME_LEN] ;
            char            dirbuf[MAX_DIR_PATH_LEN] ;

    dirSpecLen = strlen( dirSpec) ;
    outputBuf = NULL ;

    switch(    *dirSpec    )
    {   case '~':
        {   if(    !(dirSpec[1])  ||  (dirSpec[1] == '/')    )
	    {
		userDir = (struct passwd *) getpwuid( getuid()) ;
                if(    userDir    )
                {   
    		    userDirLen = strlen( userDir->pw_dir) ;
    		    outputBuf = XtMalloc( userDirLen + dirSpecLen + 2) ;
                    strcpy( outputBuf, userDir->pw_dir) ;
    		    strcpy( &outputBuf[userDirLen], (dirSpec + 1)) ;
                    }
    	        }
	    else
	    {
		destPtr = nameBuf ;
                userNameLen = 0 ;
		srcPtr = dirSpec + 1 ;
		while(    *srcPtr  &&  (*srcPtr != '/')
                       && (++userNameLen < MAX_USER_NAME_LEN)    )
		{   *destPtr++ = *srcPtr++ ;
                    } 
		*destPtr = '\0' ;

                userDir = (struct passwd *)getpwnam( nameBuf) ;
		if(    userDir    )
		{   
		    userDirLen = strlen( userDir->pw_dir) ;
		    dirSpecLen = strlen( srcPtr) ;
		    outputBuf = XtMalloc( userDirLen + dirSpecLen + 2) ;
                    strcpy( outputBuf, userDir->pw_dir) ;
                    strcpy( &outputBuf[userDirLen], srcPtr) ;
                    } 
		}
            break ;
            } 
        case '/':
        {   outputBuf = XtMalloc( dirSpecLen + 2) ;
	    strcpy( outputBuf, dirSpec) ;
            break ;
            } 
        default:
        {  
#ifdef USE_GETWD
            destPtr = (char*)getwd( dirbuf) ;
#else
            destPtr = (char*)getcwd( dirbuf, MAX_DIR_PATH_LEN) ;
#endif
            if(    destPtr    )
            {   userDirLen = strlen( destPtr) ;
	        outputBuf = XtMalloc( userDirLen + dirSpecLen + 3) ;
                strcpy( outputBuf, destPtr) ;
	        outputBuf[userDirLen++] = '/';
                strcpy( &outputBuf[userDirLen], dirSpec) ;
                } 
            break ;
            } 
        } 
    if(    !outputBuf    )
    {   outputBuf = XtMalloc( 2) ;
        outputBuf[0] = '/' ;
        outputBuf[1] = '\0' ;
        } 
    else
    {   userDirLen = strlen( outputBuf) ;
        if(    outputBuf[userDirLen - 1]  !=  '/'    )
        {   outputBuf[userDirLen] = '/' ;
            outputBuf[++userDirLen] = '\0' ;
            } 
        /* The string in outputBuf is assumed to begin and end with a '/'.
        */
        scanPtr = outputBuf ;
        while(    *++scanPtr    )               /* Skip past '/'. */
        {   /* scanPtr now points to non-NULL character following '/'.
            */
            if(    scanPtr[0] == '.'    )
            {   
                if(    scanPtr[1] == '/'    )
                {   /* Have "./", so just erase (overwrite with shift).
                    */
                    destPtr = scanPtr ;
                    srcPtr = &scanPtr[2] ;
                    while(    *destPtr++ = *srcPtr++    )
                    {   } 
                    --scanPtr ;     /* Leave scanPtr at preceding '/'. */
                    continue ;
                    } 
                else
                {   if(    (scanPtr[1] == '.')  &&  (scanPtr[2] == '/')    )
                    {   /* Have "../", so back up one directory.
                        */
                        srcPtr = &scanPtr[2] ;
                        --scanPtr ;      /* Move scanPtr to preceding '/'.*/
                        if(    scanPtr != outputBuf    )
                        {   while(    (*--scanPtr != '/')    )
                            {   }          /* Now move to previous '/'.*/
                            } 
                        destPtr = scanPtr ;
                        while(    *++destPtr = *++srcPtr    )
                        {   }               /* Overwrite "../" with shift.*/
                        continue ;
                        } 
                    } 
                } 
            else
            {   /* Check for embedded "//".  Posix allows a leading double
                *   slash (and Apollos require it).
                */
                if(    *scanPtr == '/'    )
                {   
		    if(    (scanPtr > (outputBuf + 1))
                        || (scanPtr[1] == '/')    )
                    {
                        /* Have embedded "//" (other than root specification),
			 *   so erase with shift and reset scanPtr.
			 */
			srcPtr = scanPtr ;
			--scanPtr ;
			destPtr = scanPtr ;
			while(    *++destPtr = *++srcPtr    )
			    {   } 
		    }
                    continue ;
		}
	    } 
            while(    *++scanPtr != '/'    )
		{   } 
	} 
    } 
	    return( outputBuf) ;
}
#endif  /* ifdef WIN32 */
#endif	/* ifdef VMS */

/****************************************************************/
String
#ifdef _NO_PROTO
_XmOSFindPatternPart(fileSpec)
String   fileSpec ;
#else    
_XmOSFindPatternPart(String  fileSpec)
#endif
/****************GENERAL:
 * fileSpec is made of a directory part and a pattern part.
 * Returns the pointer to the first character of the pattern part
 ****************/
#ifdef VMS
{
    struct dsc$descriptor_s     fs$dsc;
    struct fscndef		fscn$item[3];
    unsigned long int		status;
    char *          maskPtr ;
    char *	    tmpSpec;

    /* First thing to do, is to strip off any leading blanks...		*/
    /* Leading blanks are not ignored by the VMS service routine	*/
    /* therefore causing problems, by returning a NULL string from the	*/
    /* SYS$FILESCAN routine.						*/

    tmpSpec = fileSpec;
    while ((tmpSpec[0] == ' ') &&
	   (tmpSpec != (fileSpec + strlen(fileSpec)))
	   )
	tmpSpec++;

    fs$dsc.dsc$w_length = strlen(tmpSpec);
    fs$dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    fs$dsc.dsc$b_class = DSC$K_CLASS_S;
    fs$dsc.dsc$a_pointer = tmpSpec;

    fscn$item[0].fscn$w_length = 0;
    fscn$item[0].fscn$w_item_code = FSCN$_NAME;
    fscn$item[0].fscn$l_addr = 0;
    fscn$item[1].fscn$w_length = 0;
    fscn$item[1].fscn$w_item_code = FSCN$_TYPE;
    fscn$item[1].fscn$l_addr = 0;
    fscn$item[2].fscn$w_length = 0;
    fscn$item[2].fscn$w_item_code = 0;  /* end of item list */
    fscn$item[2].fscn$l_addr = 0;
    status = SYS$FILESCAN(&fs$dsc, &fscn$item, 0);
    if ($VMS_STATUS_SUCCESS(status))
	return(fscn$item[0].fscn$l_addr);
    else
	return( tmpSpec);
}

#else
#ifdef WIN32
/**************** Windows NT (based on Unix code):
 * Returns the pointer to the character following the '\' of the name segment
 *   which contains a wildcard.
 ****************/
{
    char *          lookAheadPtr = fileSpec ;
    char *          maskPtr ;
    Boolean         hasWildcards ;
    char            prevChar ;
    char            prev2Char  ;

    do {   /* Stop at final name segment or if wildcards were found.*/
	maskPtr = lookAheadPtr ;
        hasWildcards = FALSE ;
        prevChar = '\0' ;
        prev2Char = '\0' ;
        while((*lookAheadPtr != '\\') && !hasWildcards && *lookAheadPtr) {
	    switch (*lookAheadPtr) {
	    /* NT filespecs don't have [set] wildcards, or escape chars */
	    case '*': case '?':
		hasWildcards = TRUE ;
		break ;
	    }
            prev2Char = prevChar ;
            prevChar = *lookAheadPtr ;
            ++lookAheadPtr ;
	}
    } while (!hasWildcards  &&  *lookAheadPtr++) ;

    if(*maskPtr == '\\') ++maskPtr ;

    return(maskPtr) ;
}

#else
/****************UNIX:
 * Returns the pointer to the character following the '/' of the name segment
 *   which contains a wildcard or which is not followed by a '/'.
 ****************/
{
    char *          lookAheadPtr = fileSpec ;
    char *          maskPtr ;
    Boolean         hasWildcards ;
    char            prevChar ;
    char            prev2Char  ;

    do {   /* Stop at final name segment or if wildcards were found.*/
	maskPtr = lookAheadPtr ;
        hasWildcards = FALSE ;
        prevChar = '\0' ;
        prev2Char = '\0' ;
        while((*lookAheadPtr != '/') && !hasWildcards && *lookAheadPtr) {   
	    switch (*lookAheadPtr) {   
	    case '*': case '?': case '[': 
                if((prevChar != '\\')  ||  (prev2Char == '\\')) {   
		    hasWildcards = TRUE ;
		    break ;
		} 
	    }
            prev2Char = prevChar ;
            prevChar = *lookAheadPtr ;
            ++lookAheadPtr ;
	} 
    } while (!hasWildcards  &&  *lookAheadPtr++) ;

    if(*maskPtr == '/') ++maskPtr ;

    return(maskPtr) ;
}
#endif	/* ifdef WIN32 */
#endif	/* ifdef VMS */

/****************************************************************/
void
#ifdef _NO_PROTO
_XmOSQualifyFileSpec( dirSpec, filterSpec, pQualifiedDir, pQualifiedPattern)
            String          dirSpec ;
            String          filterSpec ;
            String *        pQualifiedDir ;     /* Cannot be NULL.*/
            String *        pQualifiedPattern ; /* Cannot be NULL.*/
#else
_XmOSQualifyFileSpec(
            String          dirSpec,
            String          filterSpec,
            String *        pQualifiedDir,      /* Cannot be NULL.*/
            String *        pQualifiedPattern)  /* Cannot be NULL.*/
#endif
/************GENERAL:
 * dirSpec, filterSpec can contain relative or logical reference.
 * dirSpec cannot contain pattern characters.
 * if filterSpec does not specify all for its last segment, a pattern 
 * for 'all' is added.
 * Use GetQualifiedDir() for dirSpec.
 ****************/
/************UNIX:
 * 'all' is '*' and '/' is the delimiter.
 ****************/
{
    int             filterLen ;
    int             dirLen ;
    char *          fSpec ;
    char *          remFSpec ;
    char *          maskPtr ;
    char *          dSpec ;
    char *          dPtr ;

#ifdef VMS
    struct dsc$descriptor_s     fs$dsc;
    struct fscndef		fscn$item[4];
    struct fldflags		flags;
    unsigned long int		status;
#endif

    if(!dirSpec) dirSpec = "" ;
    if(!filterSpec) filterSpec = "" ;
        
    filterLen = strlen(filterSpec) ;

    /* Allocate extra for NULL character and for the appended '*' (as needed).
    */
#ifdef VMS
    fSpec = XtMalloc( filterLen + 4) ;	    /* maybe '*.*' */
    fSpec[filterLen] = '\0';
    fSpec[filterLen+1] = '\0';
    fSpec[filterLen+2] = '\0';
    fSpec[filterLen+3] = '\0';
#else
#ifdef WIN32
    fSpec = XtMalloc( filterLen + 4) ;      /* include drive # */
#else
    fSpec = XtMalloc( filterLen + 2) ;
#endif
    strcpy( fSpec, filterSpec) ;
#endif

#ifdef VMS
    /* If fSpec doesn't contain a filename or extension part, add '*.*'.
    *  If fSpec contains only a filename, add '.*'.  If it contains only
    *  an extension, add '*' before the extension.
    */
    fs$dsc.dsc$w_length = strlen(fSpec);
    fs$dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    fs$dsc.dsc$b_class = DSC$K_CLASS_S;
    fs$dsc.dsc$a_pointer = fSpec;

    fscn$item[0].fscn$w_length = 0;
    fscn$item[0].fscn$w_item_code = FSCN$_NAME;
    fscn$item[0].fscn$l_addr = 0;
    fscn$item[1].fscn$w_length = 0;
    fscn$item[1].fscn$w_item_code = FSCN$_TYPE;
    fscn$item[1].fscn$l_addr = 0;
    fscn$item[2].fscn$w_length = 0;
    fscn$item[2].fscn$w_item_code = FSCN$_VERSION;
    fscn$item[2].fscn$l_addr = 0;
    fscn$item[3].fscn$w_length = 0;
    fscn$item[3].fscn$w_item_code = 0;  /* end of item list */
    fscn$item[3].fscn$l_addr = 0;
    status = SYS$FILESCAN(&fs$dsc, &fscn$item, &flags);
    if ($VMS_STATUS_SUCCESS(status))
    {
	if (flags.fscn$v_name)
	{   /* name present */
	    if (flags.fscn$v_type)
	    {	/* name and type both present - do nothing */
		}
	    else
	    {	/* name present, no type - insert '.*' */
		char *ptr;
		if (flags.fscn$v_version)
		{
		    ptr = fscn$item[2].fscn$l_addr;
		    memmove(ptr, ptr+2, fscn$item[2].fscn$w_length);
		    }
		else
		{
		    ptr = &fSpec[strlen(fSpec)];
		    }		    
		*ptr++ = '.';
		*ptr = '*';
		}
	    }
	else
	{   /* name missing */
	    if (flags.fscn$v_type)
	    {	/* name missing, type present, insert '*' */
		char *ptr;
		ptr = fscn$item[1].fscn$l_addr;
		memmove(ptr, ptr+1, fscn$item[1].fscn$w_length + fscn$item[2].fscn$w_length);
		*ptr = '*';
		}
	    else
	    {	/* name and type both missing */
		char *ptr;
		if (flags.fscn$v_version)
		{
		    ptr = fscn$item[2].fscn$l_addr;
		    memmove(ptr, ptr+3, fscn$item[2].fscn$w_length);
		    }
		else
		{
		    ptr = &(fSpec[strlen(fSpec)]);
		    }
		*ptr++ = '*';
		*ptr++ = '.';
		*ptr = '*';
		}
	    }
	}



    /* *** NYI ***	    NOT DONE YET! This is a HACK! */
    dSpec = XtNewString(dirSpec);

#else	/* not VMS */
    /* If fSpec ends with a '/' or is a null string, add '*' since this is
    *   the interpretation. (Or ends with '\' or is just drive # with colon for NT - jtf)
    */
#ifdef WIN32
    if((filterLen == 2) && (fSpec[1] == ':')){
        fSpec[2] = '\\' ;
        fSpec[3] = '\0' ;
    }

    if(!filterLen  ||  (fSpec[filterLen - 1] == '\\')){
#else
    if(!filterLen  ||  (fSpec[filterLen - 1] == '/')){   
#endif
	fSpec[filterLen] = '*' ;
        fSpec[filterLen + 1] = '\0' ;
    } 

    /* Some parts of fSpec may be copied to dSpec, so allocate "filterLen" 
    *   extra, plus some for added literals.
    */
    dirLen = strlen(dirSpec) ;
    dSpec = XtMalloc(filterLen + dirLen + 4) ;
    strcpy(dSpec, dirSpec) ;
    dPtr = dSpec + dirLen ;

    /* Check for cases when the specified filter overrides anything
    *   in the dirSpec.
    */
    remFSpec = fSpec ;
    switch(*fSpec) {   
#ifdef WIN32
    case '\\':
	dSpec[0] = '\\' ;
#else
    case '/':
	dSpec[0] = '/' ;
#endif
	dSpec[1] = '\0' ;
	dPtr = dSpec + 1 ;
	++remFSpec ;
	break ;
#ifndef WIN32 /* tilde not meaningful in NT filespecs */
    case '~':
        dPtr = dSpec ;
	while((*dPtr = *remFSpec)  &&  (*remFSpec++ != '/')) ++dPtr ;
	*dPtr = '\0' ;
	break ;
#endif
    } 

    /* If directory spec. is not null, then make sure that it has a
    *   trailing '/' (trailing '\' for NT), to be prepared for appending
    *   from filter spec.
    */
#ifdef WIN32
    if(*dSpec  &&  (*(dPtr - 1) != '\\')) {
	*dPtr++ = '\\' ;
#else
    if(*dSpec  &&  (*(dPtr - 1) != '/')) {   
	*dPtr++ = '/' ;
#endif
        *dPtr = '\0' ;
    } 

    maskPtr = _XmOSFindPatternPart(remFSpec) ;

    if(maskPtr != remFSpec) {  
        do {   
	    *dPtr++ = *remFSpec++ ;
	} while(remFSpec != maskPtr) ;
        *dPtr = '\0' ;
    } 

    if(remFSpec != fSpec) {   
	/* Shift remaining filter spec. to the beginning of the buffer. */
        remFSpec = fSpec ;
        while(*remFSpec++ = *maskPtr++ ) ;
    } 
#endif	/* VMS */

    *pQualifiedDir = GetQualifiedDir( dSpec) ;
    *pQualifiedPattern = fSpec ;
    XtFree(dSpec) ;
}

#ifndef WIN32 /* No "regular expressions" in NT */
/****************************************************************/
static String
#ifdef _NO_PROTO
GetFixedMatchPattern( pattern)
            String         pattern ;
#else
GetFixedMatchPattern(
            String         pattern)
#endif
/**********GENERAL:
 * The pattern parameter is converted to the format required of the
 *   the regular expression library routines.
 * Memory is allocated and returned with the result.  This memory
 *   should eventually be freed by a call to XtFree().
 ****************/
/**********UNIX:
 * '/' is used as a delimiter for the pattern.
 ****************/
{
    register char *         bufPtr ;
    char *          outputBuf ;

    outputBuf = XtCalloc( 2, strlen( pattern) + 4) ;

    bufPtr = outputBuf ;
    *bufPtr++ = '^' ;

    while(*pattern  &&  (*pattern != '/')) {   
        switch(*pattern) {   
	case '.':
            *bufPtr++ = '\\' ;
	    *bufPtr++ = '.' ;
	    break ;
	case '?':
            *bufPtr++ = '.' ;
	    break;
	case '*':
            *bufPtr++ = '.' ;
	    *bufPtr++ = '*' ;
	    break ;
	default:
            *bufPtr++ = *pattern ;
	    break ;
	} 
        ++pattern ;
    } 
    *bufPtr++ = '$' ;
    *bufPtr = '\0' ;

    return( outputBuf) ;
}
#endif /* WIN32 */

/****************************************************************/
void
#ifdef _NO_PROTO
_XmOSGetDirEntries(qualifiedDir, matchPattern, fileType, matchDotsLiterally,
	      listWithFullPath, pEntries, pNumEntries, pNumAlloc)
            String          qualifiedDir ;
            String          matchPattern ;
            unsigned char   fileType ;
            Boolean         matchDotsLiterally ;
            Boolean         listWithFullPath ;
            String * *      pEntries ;      /* Cannot be NULL. */
            unsigned int *  pNumEntries ;   /* Cannot be NULL. */
            unsigned int *  pNumAlloc ;     /* Cannot be NULL. */
#else
_XmOSGetDirEntries(
            String          qualifiedDir,
            String          matchPattern,
#if NeedWidePrototypes
	      unsigned int fileType,
	      int matchDotsLiterally,
	      int listWithFullPath,
#else
	      unsigned char fileType,
	      Boolean matchDotsLiterally,
	      Boolean listWithFullPath,
#endif /* NeedWidePrototypes */
            String * *      pEntries,       /* Cannot be NULL. */
            unsigned int *  pNumEntries,    /* Cannot be NULL. */
            unsigned int *  pNumAlloc)      /* Cannot be NULL. */
#endif
/***********GENERAL:
 * This routine opens the specified directory and builds a buffer containing
 * a series of strings containing the full path of each file in the directory 
 * The memory allocated should eventually be freed using XtFree.
 * The 'qualifiedDir' parameter must be a fully qualified directory path 
 * The matchPattern parameter must be in the proper form for a regular 
 * expression parsing.
 * If the location pointed to by pEntries is NULL, this routine allocates
 *   and returns a list to *pEntries, though the list may have no entries.
 *   pEntries, pEndIndex, pNumAlloc are updated as required for memory 
 *   management.
 ****************/
#ifdef VMS
{
    char *          fixedMatchPattern ;
    String          entryPtr ;
    char *	    cwdPtr;
    char *	    parentPtr;
    DIR *           dirStream ;
    struct stat     statBuf ;
    Boolean         entryTypeOK ;
    unsigned int    dirLen = strlen( qualifiedDir) ;
    struct dirent * dirEntry ;

    if(    !*pEntries    )
    {   *pNumEntries = 0 ;
        *pNumAlloc = FILE_LIST_BLOCK ;
        *pEntries = (String *) XtMalloc( FILE_LIST_BLOCK * sizeof( char *)) ;
    } 
    fixedMatchPattern = NULL ;

    dirStream = opendir( qualifiedDir, matchPattern );
    if(    dirStream    )
    {   
	if ( fileType != XmFILE_REGULAR )
	{
	    /* 1st entry must be the current directory */
	    if(    *pNumEntries == *pNumAlloc    )
	    {
		*pNumAlloc += FILE_LIST_BLOCK ;
		*pEntries = (String *) XtRealloc( *pEntries, (*pNumAlloc * sizeof( char *))) ;
	    } 
	    entryPtr = XtNewString( qualifiedDir ) ;
	    (*pEntries)[(*pNumEntries)++] = entryPtr ;

	    /* 2nd entry must be the parent directory */
	    if(    *pNumEntries == *pNumAlloc    )
	    {
		*pNumAlloc += FILE_LIST_BLOCK ;
		*pEntries = (String *) XtRealloc( *pEntries, (*pNumAlloc * sizeof( char *))) ;
	    } 
	    cwdPtr = getcwd(NULL,MAXFILELENGTH);		/* save current */
	    (void)chdir(qualifiedDir);				/* set def to new */
	    (void)chdir(PARENTDIR);				/* set def to parent */
	    parentPtr = getcwd(NULL,MAXFILELENGTH);		/* get parent spec */
	    (void)chdir(cwdPtr);				/* set back to original */
	    free(cwdPtr);					/* was malloc'd by rtl */
	    entryPtr = XtMalloc( strlen(parentPtr) + 1);
	    strcpy( entryPtr, parentPtr );
	    free(parentPtr);					/* was malloc'd by rtl */
	    (*pEntries)[(*pNumEntries)++] = entryPtr ;
	}

	while(dirEntry = readdir( dirStream ))
        {   
            if(    *pNumEntries == *pNumAlloc    )
            {
		*pNumAlloc += FILE_LIST_BLOCK ;
                *pEntries = (String *) XtRealloc( *pEntries, (*pNumAlloc * sizeof( char *))) ;
	    } 
            if(    listWithFullPath    )
	    {
		entryPtr = XtMalloc( dirEntry->d_namlen + 1) ;
                strcpy( entryPtr, dirEntry->d_name) ;
	    } 
	    else
	    /* *** NYI ***
	     * this code isn't implemented yet - should use SYS$FILESCAN
	     * to find the pieces and just copy the name and type.
	     * GetDirEntries is never called with listWithFullPath false
	     * anyway, so don't worry about it.
	     */
	    {
		entryPtr = XtMalloc( dirEntry->d_namlen + 1) ;
                strcpy( entryPtr, dirEntry->d_name) ;
	    } 
	    XtFree(dirEntry);

            /*
	    **	Now screen entry according to type.
            */
	    if (fileType == XmFILE_ANY_TYPE)
		entryTypeOK = TRUE ;
	    else
	    {
		entryTypeOK = FALSE ;

		switch( fileType )
		{
		    case XmFILE_REGULAR:
		    {
			if(strstr(entryPtr, DIR_EXTENSION) == NULL)
			{
			    entryTypeOK = TRUE ;
			}
			else
			{
			    if (!stat(entryPtr, &statBuf))
			    {
				if (S_ISREG(statBuf.st_mode))
				{
				    entryTypeOK = TRUE;
				}
			    }
			}
			break ;
		    }
		    case XmFILE_DIRECTORY:
		    {
			if(strstr(entryPtr, DIR_EXTENSION) != NULL)
			{
			    if (IsRemoteFilespec_VMS(entryPtr))
			    {
				/* can't stat a remote file, so assume it's OK */
				entryTypeOK = TRUE;
			    }
			    else
			    {
				if (!stat(entryPtr, &statBuf))
				{
				    if (S_ISDIR(statBuf.st_mode))
				    {
					entryTypeOK = TRUE;
				    }
				}
			    }
			}
			break ;
		    }
		} /* end switch */
	    } /* end if else */

            if(    entryTypeOK    )
            {
		(*pEntries)[(*pNumEntries)++] = entryPtr ;
	    } 
            else
            {
		XtFree( entryPtr) ;
	    } 
	} /* end while */
        closedir( dirStream) ;
    }

    XtFree( fixedMatchPattern) ;
    return ;
}

#else
/***********UNIX:
 * Fully qualified directory means begins with '/', does not have 
 * embedded "." or "..", but does not need trailing '/'.
 * Regular expression parsing is regcmp or re_comp.
 * Directory entries are also Unix dependent.
 ****************/
/***********Windows NT:
 * '\' instead of '/', no regular expressions, entries are NTFS or FAT (jtf)
 ****************/
{
            char *          fixedMatchPattern ;
            String          entryPtr ;
            DIR *           dirStream ;
            struct stat     statBuf ;
            Boolean         entryTypeOK ;
            unsigned int    dirLen = strlen( qualifiedDir) ;
#ifndef NO_REGCOMP
            regex_t         preg ;
            int             comp_status ;
#else /* NO_REGCOMP */
#ifndef NO_REGEX
            char *          compiledRE = NULL ;
#endif
#endif /* NO_REGCOMP */
#ifdef NDIR 
            struct direct * dirEntry ;
#else 
#ifdef SYS_DIR
            struct direct * dirEntry ;
#else
            struct dirent * dirEntry ;
#endif
#endif
/****************/

    if(    !*pEntries    )
    {   *pNumEntries = 0 ;
        *pNumAlloc = FILE_LIST_BLOCK ;
        *pEntries = (String *) XtMalloc( FILE_LIST_BLOCK * sizeof( char *)) ;
        } 
#ifndef WIN32
    fixedMatchPattern = GetFixedMatchPattern( matchPattern) ;

    if(    fixedMatchPattern    )
    {   
        if(    !*fixedMatchPattern    )
        {   
            XtFree( fixedMatchPattern) ;
            fixedMatchPattern = NULL ;
            } 
        else
        {   
#ifndef NO_REGCOMP
            comp_status = regcomp( &preg, fixedMatchPattern, REG_NOSUB) ;
            if(    comp_status    )
#else /* NO_REGCOMP */
#  ifndef NO_REGEX
            compiledRE = (char *)regcmp( fixedMatchPattern, (char *) NULL) ;
            if(    !compiledRE    )
#  else
            if(    re_comp( fixedMatchPattern)    )
#  endif
#endif /* NO_REGCOMP */
            {   XtFree( fixedMatchPattern) ;
                fixedMatchPattern = NULL ;
                } 
            }
        }
#endif /* WIN32 */

    dirStream = opendir( qualifiedDir) ;

    if(    dirStream    )
    {   
#ifdef WIN32
        while(    dirEntry = readdir( dirStream, matchPattern ) )
        {
#else
        while(    dirEntry = readdir( dirStream)    )
        {   
            if(    fixedMatchPattern    )
            {   
#ifndef NO_REGCOMP
                if(    regexec( &preg, dirEntry->d_name, 0, NULL, 0)    )
#else /* NO_REGCOMP */
#  ifndef NO_REGEX
                if(    !regex( compiledRE, dirEntry->d_name)    )
#  else
                if(    !re_exec( dirEntry->d_name)    )
#  endif
#endif /* NO_REGCOMP */
                {   continue ;
                    } 
                } 
#endif /* WIN32 */
            if(    matchDotsLiterally
                && (dirEntry->d_name[0] == '.')
                && (*matchPattern != '.')    )
            {   continue ;
                } 
            if(    *pNumEntries == *pNumAlloc    )
            {   *pNumAlloc += FILE_LIST_BLOCK ;
                *pEntries = (String *) XtRealloc((char*) *pEntries, 
					(*pNumAlloc* sizeof( char *))) ;
                } 
            if(    listWithFullPath    )
            {   entryPtr = XtMalloc( strlen(dirEntry->d_name) + dirLen + 1) ;
                strcpy( entryPtr, qualifiedDir) ;
                strcpy( &entryPtr[dirLen], dirEntry->d_name) ;
                }
            else
            {   entryPtr = XtMalloc( strlen(dirEntry->d_name) + 1) ;
                strcpy( entryPtr, dirEntry->d_name) ;
                } 
            /* Now screen entry according to type.
            */
            entryTypeOK = FALSE ;
	    if (fileType == XmFILE_ANY_TYPE) {
		entryTypeOK = TRUE ;
	    } else
            if(    !stat( entryPtr, &statBuf)    )
            {   
                switch(    fileType    )
                {   
                    case XmFILE_REGULAR:
                    {   
                        if(    S_ISREG( statBuf.st_mode)    )
                        {   
                            entryTypeOK = TRUE ;
                            } 
                        break ;
                        } 
                    case XmFILE_DIRECTORY:
                    {   
                        if(    S_ISDIR( statBuf.st_mode)    )
                        {   
                            entryTypeOK = TRUE ;
                            } 
                        break ;
                        } 
                    } 
	    }
            if(    entryTypeOK    )
            {   (*pEntries)[(*pNumEntries)++] = entryPtr ;
                } 
            else
            {   XtFree( entryPtr) ;
                } 
            }
        closedir( dirStream) ;
        }
#ifndef WIN32
#ifndef NO_REGCOMP
    if(    !comp_status    )
    {   regfree( &preg) ;
        } 
#else /* NO_REGCOMP */
#  ifndef NO_REGEX
    if(    compiledRE    )
    {   /* Use free instead of XtFree since malloc is inside of regex().
        */
        free( compiledRE) ; 
        } 
#  endif
#endif /* NO_REGCOMP */
    XtFree( fixedMatchPattern) ;
#endif  /* WIN32 */
    return ;
    }
#endif	/* VMS */

/****************************************************************/
void
#ifdef _NO_PROTO
_XmOSBuildFileList(dirPath, pattern, typeMask, pEntries, pNumEntries, pNumAlloc)
            String          dirPath ;
            String          pattern ;
            unsigned char   typeMask ;
            String * *      pEntries ;      /* Cannot be NULL. */
            unsigned int *  pNumEntries ;   /* Cannot be NULL. */
            unsigned int *  pNumAlloc ;     /* Cannot be NULL. */
#else
_XmOSBuildFileList(
	      String          dirPath,
	      String          pattern,
#if NeedWidePrototypes
	      unsigned int typeMask,
#else
	      unsigned char typeMask,
#endif /* NeedWidePrototypes */
	      String * *      pEntries,       /* Cannot be NULL. */
	      unsigned int *  pNumEntries,    /* Cannot be NULL. */
	      unsigned int *  pNumAlloc)      /* Cannot be NULL. */
#endif
/************GENERAL:
 * The 'dirPath' parameter must be a qualified directory path.
 * The 'pattern' parameter must be valid as a suffix to dirPath.
 * typeMask is an Xm constant coming from Xm.h.
 ****************/
/************UNIX:
 * Qualified directory path means no match characters, with '/' at end.
 ****************/
{  
    String          qualifiedDir ;
    String          nextPatternPtr ;
    String *        localEntries ;
    unsigned int    localNumEntries ;
    unsigned int    localNumAlloc ;
    unsigned int    entryIndex ;
/****************/

    qualifiedDir = GetQualifiedDir( dirPath) ;
#ifdef VMS
    _XmOSGetDirEntries ( qualifiedDir, pattern, typeMask, FALSE, TRUE,
					    pEntries, pNumEntries, pNumAlloc) ;
#else
    nextPatternPtr = pattern ;
#ifdef WIN32
    while(*nextPatternPtr  &&  (*nextPatternPtr != '\\')) ++nextPatternPtr ;
#else
    while(*nextPatternPtr  &&  (*nextPatternPtr != '/')) ++nextPatternPtr ;
#endif

    if(!*nextPatternPtr) {   
	/* At lowest level directory, so simply return matching entries.*/
        _XmOSGetDirEntries( qualifiedDir, pattern, typeMask, FALSE, TRUE, 
		      pEntries, pNumEntries, pNumAlloc) ;
    } else {   
	++nextPatternPtr ;               /* Move past '/' character.*/
        localEntries = NULL ;
        _XmOSGetDirEntries( qualifiedDir, pattern, XmFILE_DIRECTORY, TRUE, TRUE, 
		      &localEntries, &localNumEntries, &localNumAlloc) ;
        entryIndex = 0 ;
        while(entryIndex < localNumEntries) {   
	    _XmOSBuildFileList( localEntries[entryIndex], nextPatternPtr, 
			  typeMask, pEntries, pNumEntries, pNumAlloc) ;
            XtFree( localEntries[entryIndex]) ;
            ++entryIndex ;
	} 
        XtFree((char*)localEntries) ;
    }
#endif	/* ifdef VMS */
    XtFree( qualifiedDir) ;
    return ;
}


/****************************************************************/
int
#ifdef _NO_PROTO
_XmOSFileCompare(sp1, sp2)
        XmConst void *sp1 ;
        XmConst void *sp2 ;
#else
_XmOSFileCompare(
        XmConst void *sp1,
        XmConst void *sp2)
#endif
/*********GENERAL:
 * The routine must return an integer less than, equal to, or greater than
 * 0 according as the first argument is to be considered less
 * than, equal to, or greater than the second.
 ****************/
{
    return( strcmp( *((String *) sp1), *((String *) sp2))) ;
}


/************************************<+>*************************************
 *
 *   Path code, used in Mwm and Xm.
 *   Returned pointer should not be freed!
 *
 *************************************<+>************************************/
String
#ifdef _NO_PROTO
_XmOSGetHomeDirName()
#else
_XmOSGetHomeDirName()
#endif
{
    char *ptr = NULL;
    static char empty = '\0';
    static char *homeDir = NULL;

#ifdef VMS
    if (homeDir == NULL) {
        ptr = (char *)getenv("HOME");
	if (ptr != NULL) {
	    homeDir = XtMalloc (strlen(ptr) + 1);
	    strcpy (homeDir, ptr);
	}
	else {
	    homeDir = &empty;
	}
    }
#else
#ifdef WIN32
    if (homeDir == NULL)
    {
        ptr = (char *)getenv("HOME");
	if (ptr != NULL)
        {
	    homeDir = XtMalloc (strlen(ptr) + 1);
	    strcpy (homeDir, ptr);
	}
	else
        {
            char *tmp;

            ptr = getenv("HOMEDRIVE");
            tmp = getenv("HOMEPATH");

            if (ptr && tmp)
            {
                homeDir = XtMalloc(strlen(ptr) + strlen(tmp) + 1);
                sprintf(homeDir, "%s%s", ptr, tmp);
            }
            else
	        homeDir = &empty;
	}
    }
#else
    uid_t uid;
    struct passwd *pw;

    if (homeDir == NULL) {
        if((ptr = (char *)getenv("HOME")) == NULL) {
            if((ptr = (char *)getenv("USER")) != NULL)
                pw = getpwnam(ptr);
            else {
                uid = getuid();
                pw = getpwuid(uid);
            }
            if (pw)
                ptr = pw->pw_dir;
            else
                ptr = NULL;
        }
	if (ptr != NULL) {
	    homeDir = XtMalloc (strlen(ptr) + 1);
	    strcpy (homeDir, ptr);
	}
	else {
	    homeDir = &empty;
	}
    }
#endif  /* ifdef WIN32 */
#endif	/* ifdef VMS */

    return (homeDir);
}


#ifndef VMS
#ifndef WIN32

#ifndef LIBDIR
#define LIBDIR "/usr/lib/X11"
#endif
#ifndef INCDIR
#define INCDIR "/usr/include/X11"
#endif

static char libdir[] = LIBDIR;
static char incdir[] = INCDIR;

static char XAPPLRES_DEFAULT[] = "\
%%P\
%%S:\
%s/%%L/%%T/%%N/%%P\
%%S:\
%s/%%l/%%T/%%N/%%P\
%%S:\
%s/%%T/%%N/%%P\
%%S:\
%s/%%L/%%T/%%P\
%%S:\
%s/%%l/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%P\
%%S:\
%s/%%L/%%T/%%N/%%P\
%%S:\
%s/%%l/%%T/%%N/%%P\
%%S:\
%s/%%T/%%N/%%P\
%%S:\
%s/%%L/%%T/%%P\
%%S:\
%s/%%l/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S";

static char PATH_DEFAULT[] = "\
%%P\
%%S:\
%s/%%L/%%T/%%N/%%P\
%%S:\
%s/%%l/%%T/%%N/%%P\
%%S:\
%s/%%T/%%N/%%P\
%%S:\
%s/%%L/%%T/%%P\
%%S:\
%s/%%l/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%P\
%%S:\
%s/%%L/%%T/%%N/%%P\
%%S:\
%s/%%l/%%T/%%N/%%P\
%%S:\
%s/%%T/%%N/%%P\
%%S:\
%s/%%L/%%T/%%P\
%%S:\
%s/%%l/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S";

static char ABSOLUTE_PATH[] = "\
%P\
%S";

#else   /* WIN32 */

#ifndef LIBDIR
#define LIBDIR "\\usr\\lib\\X11"
#endif
#ifndef INCDIR
#define INCDIR "\\usr\\include\\X11"
#endif

static char *libdir = NULL;
static char *incdir = NULL;

static char XAPPLRES_DEFAULT[] = "\
%%P\
%%S;\
%s\\%%L\\%%T\\%%N\\%%P\
%%S;\
%s\\%%l\\%%T\\%%N\\%%P\
%%S;\
%s\\%%T\\%%N\\%%P\
%%S;\
%s\\%%L\\%%T\\%%P\
%%S;\
%s\\%%l\\%%T\\%%P\
%%S;\
%s\\%%T\\%%P\
%%S;\
%s\\%%T\\%%P\
%%S;\
%s\\%%P\
%%S;\
%s\\%%L\\%%T\\%%N\\%%P\
%%S;\
%s\\%%l\\%%T\\%%N\\%%P\
%%S;\
%s\\%%T\\%%N\\%%P\
%%S;\
%s\\%%L\\%%T\\%%P\
%%S;\
%s\\%%l\\%%T\\%%P\
%%S;\
%s\\%%T\\%%P\
%%S;\
%s\\%%T\\%%P\
%%S";

static char PATH_DEFAULT[] = "\
%%P\
%%S;\
%s\\%%L\\%%T\\%%N\\%%P\
%%S;\
%s\\%%l\\%%T\\%%N\\%%P\
%%S;\
%s\\%%T\\%%N\\%%P\
%%S;\
%s\\%%L\\%%T\\%%P\
%%S;\
%s\\%%l\\%%T\\%%P\
%%S;\
%s\\%%T\\%%P\
%%S;\
%s\\%%P\
%%S;\
%s\\%%L\\%%T\\%%N\\%%P\
%%S;\
%s\\%%l\\%%T\\%%N\\%%P\
%%S;\
%s\\%%T\\%%N\\%%P\
%%S;\
%s\\%%L\\%%T\\%%P\
%%S;\
%s\\%%l\\%%T\\%%P\
%%S;\
%s\\%%T\\%%P\
%%S;\
%s\\%%T\\%%P\
%%S";

static char ABSOLUTE_PATH[] = "\
%P\
%S";

#endif  /* ifndef WIN32 */
#endif	/* ifndef VMS */

String
#ifdef _NO_PROTO
_XmOSInitPath(file_name, env_pathname, user_path)
        String	file_name ;
        String	env_pathname ;
        Boolean * user_path ;
#else
_XmOSInitPath(
        String	file_name,
        String	env_pathname,
	Boolean * user_path)
#endif
{
  String path;
  String old_path;
  char *homedir;
  String local_path;

  *user_path = False ;

#ifdef VMS
  local_path = (char *)getenv (env_pathname);
  if (local_path  == NULL)
    return NULL;
  else
  {
    path = XtMalloc(strlen(local_path) + 1);
    strcpy (path, local_path);
    *user_path = True ;
  }
#else
#ifdef WIN32
  /*
  ** If the libdir and incdir paths have not been set, make
  ** them point to the installed include and lib directories.
  */
  if (libdir == NULL)
  {
      char *path;
      char *libsubdir = "\\lib\\X11";
      char *incsubdir = "\\include\\X11";

      if ((path = _XGetInstalledLocation()) != NULL)
      {
          libdir = XtMalloc(strlen(path) + strlen(libsubdir) + 1);
	  sprintf(libdir, "%s%s", path, libsubdir);
          incdir = XtMalloc(strlen(path) + strlen(libsubdir) + 1);
	  sprintf(incdir, "%s%s", path, incsubdir);

	  XFree(path);
      }
      else
      {
          /*
          ** Could not find the installed location, use default values.
          */
          libdir = XtMalloc(strlen(LIBDIR) + 1);
	  strcpy(libdir, LIBDIR);
	  incdir = XtMalloc(strlen(INCDIR) + 1);
	  strcpy(incdir, INCDIR);
      }
  }

  if (file_name[0] == '\\') {
#else
  if (file_name[0] == '/') {
#endif
      path = XtMalloc(strlen(ABSOLUTE_PATH) + 1);
      strcpy (path, ABSOLUTE_PATH);
  } else {
      local_path = (char *)getenv (env_pathname);
      if (local_path  == NULL)
	{
	  homedir = _XmOSGetHomeDirName();
	  old_path = (char *)getenv ("XAPPLRESDIR");
	  if (old_path == NULL) {
	      path = XtCalloc(1, 7*strlen(homedir) + strlen(PATH_DEFAULT) 
			         + 6*strlen(libdir) + strlen(incdir) + 1);
	      sprintf(path, PATH_DEFAULT, homedir, homedir, homedir,
		      homedir, homedir, homedir, homedir,
		      libdir, libdir, libdir, libdir, libdir, libdir, incdir);
	  } else {
	      path = (String) XtCalloc(1, 6*strlen(old_path) + 2*strlen(homedir) 
			      + strlen(XAPPLRES_DEFAULT) + 6*strlen (libdir)
			      +	strlen(incdir) + 1);
	      sprintf(path, XAPPLRES_DEFAULT, 
		      old_path, old_path, old_path, old_path, old_path, 
		      old_path, homedir, homedir,
		      libdir, libdir, libdir, libdir, libdir, libdir, incdir);
	  }
      } else {
	  path = XtMalloc(strlen(local_path) + 1);
	  strcpy (path, local_path);
	  *user_path = True ;
      }
  }
#endif /* VMS */

  return (path);
}

void
#ifdef _NO_PROTO
_XmSleep( secs )
        unsigned int secs ;
#else
_XmSleep(
        unsigned int secs)
#endif
{   sleep( secs) ;
    }


int
#ifdef _NO_PROTO
_XmMicroSleep( usecs )
        long    usecs ;
#else
_XmMicroSleep(
        long    usecs)
#endif
{
#ifdef VMS
    int delay[2];			/* 64 bit vms time */

    /*
    **	Place the microseconds into the system 64-bit delta time format.  This method assumes that
    **	the number of microseconds does not exceed the number of 100-nanosecond units that can be
    **	expressed in a longword.  This is more than seven minutes so this should not be a problem.
    **	Convert the microseconds into 100-nanosecond units.  Since we want a delta time the values
    **	must be negative.
    */
    delay[0] = - (usecs * 10);		/* 100-nanoseconds = microseconds * (1000/100)  */
    delay[1] = -1;			/* set the high order longword */

    sys$schdwk(0, 0, &delay, 0);	/* schedule a wakeup "delay" time from now */
    sys$hiber();			/* hibernate until wakeup */
    return;
#else
#  ifndef _STRUCT_TIMEVAL
#    define _STRUCT_TIMEVAL
       /* Structure returned by gettimeofday(2) system call and others */
       struct timeval {
       unsigned long  tv_sec;         /* seconds */
       long           tv_usec;        /* and microseconds */
       };
#   endif /* _STRUCT_TIMEVAL */
    struct timeval      timeoutVal;

    timeoutVal.tv_sec = 0;
    timeoutVal.tv_usec = usecs;

#ifdef WIN32
    return (WinSockSelect(0, (int *) 0, (int *) 0, (int *) 0, &timeoutVal));
#else
    return (select(0, (int *) 0, (int *) 0, (int *) 0, &timeoutVal));
#endif
#endif	/* ifdef VMS */
}

/************************************************************************
 *                                                                    *
 *    _XmOSSetLocale   wrapper so vendor can disable call to set       *
 *                    if locale is superset of "C".                   *
 *                                                                    *
 ************************************************************************/

String
#ifdef _NO_PROTO
_XmOSSetLocale(locale)
     String locale;
#else
_XmOSSetLocale(String locale)
#endif
{
  return(setlocale(LC_ALL, locale));
}

/************************************************************************
 *                                                                    *
 *	_XmOSGetLocalizedString	Map an X11 R5 XPCS string in a locale	*
 *				sensitive XmString.			*
 *                                                                    *
 *		reserved	Reserved for future use.		*
 *		widget		The widget id.				*
 *		resource	The resource name.			*
 *		string		The input 8859-1 value.			*
 *                                                                    *
 ************************************************************************/

XmString
#ifdef _NO_PROTO
_XmOSGetLocalizedString( reserved, widget, resource, string)
        char *reserved ;
        Widget widget ;
        char *resource ;
        String string ;
#else
_XmOSGetLocalizedString(
        char *reserved,
        Widget widget,
        char *resource,
        String string)
#endif
{
  return( XmStringCreateLocalized( string)) ;
}


/************************************************************************
 *									*
 *    _XmOSBuildFileName						*
 *									*
 *	Build an absolute file name from a directory and file.		*
 *	Handle case where 'file' is already absolute.
 *	Return value should be freed by XtFree()			*
 *									*
 ************************************************************************/

String
#ifdef _NO_PROTO
_XmOSBuildFileName( path, file)
    String path;
    String file;
#else
_XmOSBuildFileName(
    String path,
    String file)
#endif
{
    String fileName;

#ifdef VMS
    fileName = XtMalloc (strlen(path) + strlen (file) + 1);
    strcpy (fileName, path);
    strcat (fileName, file);
#else
#ifdef WIN32
    if (file[0] == '\\') {
#else
    if (file[0] == '/') {
#endif
	fileName = XtMalloc (strlen (file) + 1);
	strcpy (fileName, file);
    }
    else {
	fileName = XtMalloc (strlen(path) + strlen (file) + 2);
	strcpy (fileName, path);
#ifdef WIN32
	strcat (fileName, "\\");
#else
	strcat (fileName, "/");
#endif
	strcat (fileName, file);
    }
#endif	/* ifdef VMS */

    return (fileName);
}


/************************************************************************
 *									*
 *    _XmOSPutenv							*
 *									*
 *	Provide a standard interface to putenv (BSD) and setenv (SYSV)  *
 *      functions.                                                      *
 *									*
 ************************************************************************/

int
#ifdef _NO_PROTO
_XmOSPutenv( string)
    char *string;
#else
_XmOSPutenv(
    char *string)
#endif
{
#ifdef VMS
    printf("_XmOSPutenv not implemented on VMS\n");
    return -1;
#else
#ifdef WIN32
    return (_putenv(string));
#else
#ifndef NO_PUTENV
    return (putenv(string));

#else
    char *value;

    if ( (value = strchr(string, '=')) != NULL)
      {
	char *name  = XtNewString(string);
	int result;

	name[value-string] = '\0';

	result = setenv(name, value+1, 1);
	XtFree(name);
	return result;
      }
    else
      return -1;
#endif
#endif
#endif	/* ifdef VMS */
}

#ifdef VMS
/*
**  This section contains the VMS specific file search code.
**  These functions emulate the use of the Unix library routines
**  that read directories.
*/

static DIR * opendir(char * dirspec, char * filterspec)
{
    DIR * dp;

    dp = XtMalloc(sizeof(DIR));

    /*
     * set up descriptors and lib$find_file context
     */
    dp->filespec$dsc.dsc$a_pointer  = dirspec;
    dp->filespec$dsc.dsc$w_length   = strlen(dirspec);
    dp->filespec$dsc.dsc$b_dtype    = DSC$K_DTYPE_T;
    dp->filespec$dsc.dsc$b_class    = DSC$K_CLASS_S;

    dp->defspec$dsc.dsc$a_pointer  = filterspec;
    dp->defspec$dsc.dsc$w_length   = strlen(filterspec);
    dp->defspec$dsc.dsc$b_dtype    = DSC$K_DTYPE_T;
    dp->defspec$dsc.dsc$b_class    = DSC$K_CLASS_S;

    dp->result$dsc.dsc$w_length     = MAXFILELENGTH;
    dp->result$dsc.dsc$a_pointer    = dp->buffer;
    dp->result$dsc.dsc$b_dtype      = DSC$K_DTYPE_T;
    dp->result$dsc.dsc$b_class      = DSC$K_CLASS_S;

    dp->filecontext = 0;
    dp->status = 0;

    return(dp);
}

static void closedir(DIR *dp)
{
    (void)lib$find_file_end(&dp->filecontext);
    XtFree(dp);
}

static struct dirent *	readdir(DIR *dp)
{
    struct dirent * de;

    dp->status = lib$find_file(	&dp->filespec$dsc,
				&dp->result$dsc,
				&dp->filecontext,
				&dp->defspec$dsc );
    if ($VMS_STATUS_SUCCESS(dp->status))
    {
	de = XtMalloc(sizeof(struct dirent));
	de->d_namlen = dp->result$dsc.dsc$w_length;
	/* strip trailing spaces */
	while (dp->buffer[de->d_namlen - 1] == ' ') de->d_namlen--;
	strncpy(de->d_name, dp->buffer, de->d_namlen);
	de->d_name[de->d_namlen] = '\0';
    }
    else
    {
	de = NULL;
    }
    return(de);
}

static Boolean IsRemoteFilespec_VMS( char * entryPtr)
{
    struct dsc$descriptor_s     fs$dsc;
    struct fldflags		flags;
    unsigned long int		status;

    fs$dsc.dsc$w_length = strlen(entryPtr);
    fs$dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    fs$dsc.dsc$b_class = DSC$K_CLASS_S;
    fs$dsc.dsc$a_pointer = entryPtr;

    status = SYS$FILESCAN(&fs$dsc, NULL, &flags);
    if ($VMS_STATUS_SUCCESS(status))
    {
	if (flags.fscn$v_node) return TRUE;
    }
    else
	return FALSE;
}
#endif /* VMS */

#ifdef WIN32
/*************************************************************************
 * Emulate directory routines for Windows NT. (jtf)
 *
 * Structures dirent and DIR invented solely for this purpose.
 *************************************************************************/


static struct dirent DirEnt;
static DIR Dir;

static DIR * opendir(char * pDirSpec)
{
    strcpy (Dir.Dirent.d_name, pDirSpec);
    Dir.firsttime = 1;
    Dir.search_handle = (long)NULL;

    return (&Dir);
}

static void closedir(DIR *pDir)
{
    if (pDir->search_handle > 0)
        _findclose (pDir->search_handle);
}

static struct dirent *	readdir(DIR *pDir, char *pattern)
{
    char *filespec;
    char *pd = pDir->Dirent.d_name;
    int len;

    filespec = XtMalloc (strlen(pd) + strlen(pattern) + 2);
    strcpy (filespec, pd);
    len = strlen(filespec);
    if (filespec[len-1] != '\\')
    {
        filespec[len] = '\\';
        filespec[len+1] = '\0';
    }
    strcat (filespec, pattern);

    if (pDir->firsttime)
    {
        pDir->firsttime = 0;
        if ((pDir->search_handle = _findfirst (filespec, &pDir->findinfo)) == -1)
            return (NULL);
    }
    else
    {
        if (_findnext (pDir->search_handle, &pDir->findinfo) != 0)
            return (NULL);
    }

    strcpy (DirEnt.d_name, pDir->findinfo.name);

    return (&DirEnt);
}

#endif /* WIN32 */
