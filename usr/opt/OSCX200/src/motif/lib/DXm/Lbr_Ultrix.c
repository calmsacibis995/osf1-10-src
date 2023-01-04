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
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
*/

/*
**++
**  Subsystem:
**	DECwindows Help Widget
**
**  Version: V1.0
**
**  Abstract:
**	This module contains Ultrix-compatable simulation of those VMS LBR$
**	routines that the Help Widget needs to access VMS Help libraries.
**
**  Environment:
**	Ultrix
**
**  Author:
**	Doug Rayner
**
**  Creation Date: 26-Jan-1988
**
**  Modification History:
**
**	    Rich Reichert				       29-Oct-90
**	In LBR_GET_RECORD, change handling of lines that consist of only
**	'\n' -- they are causing a double-space in the help text.
**
**	    Rich Reichert				       23-Jul-90
**	Remove DXm/ from private include file includes
**
**	    Rich Reichert				       11-Jul-90
**	Fix the call to XtResolvePathname
**
**	    Rich Reichert				        8-Jun-90
**	More Ultrix compilation cleanups.
**
**	    Rich Reichert				        8-Jun-90
**      Clean up compilation errors on Ultrix.
**
**	    Rich Reichert					6-Jun-90
**	Make LBR_OPEN use XtResolvePathname
**
**  BL3-0-1 Andre Pavanello					1-Mar-88
**	Remove \n's from the record returned by LBR$GET_RECORD
**--
*/


#ifndef VMS   /* not to compile on VMS */

/*
**  Include Files
*/
#include <DXm/DXmHelpBP.h>
#include "Help_Descrip.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>        /* defines MAXPATHLEN */
#include <sys/stat.h>	      /* define status codes like R_OK */
#include <unistd.h>	      /* define status codes like R_OK */
#if defined(UNIX_SYSV) || defined(__osf__)
#include <dirent.h>
#else
#include <sys/dir.h>
#endif

/*
**  Macro Definitions
*/
#define BUFFER_SIZE 1024

#define LBR__NORMAL TRUE
#define LBR__LIBNOTOPN FALSE
#define LBR__TOOMNYLIB FALSE
#define LBR__INVRFA FALSE
#define LBR__NULIDX FALSE
#define RMS__EOF FALSE
#define RMS__FNF FALSE

/*
**  Type Definitions
*/
typedef struct dsc_descriptor DESCRIPTOR;

typedef struct dsc_descriptor_s DESCRIPTOR_S;

typedef struct INDEX {
    DIR  *dirp;
    FILE *fd;
    int  file_open;
    char *library;
    char *buffer;
    } INDEX;

/*
**  Table of Contents
*/
int LBR_INI_CONTROL();
int LBR_OPEN();
int LBR_CLOSE();
int LBR_GET_INDEX();
int LBR_SET_LOCATE();
int LBR_FIND();
int LBR_GET_RECORD();
Boolean TestFileAndDir();

/*
**  External Routines
*/
char *XtMalloc();
char *XtResolvePathname();


int LBR_INI_CONTROL(index, funct, type, namblk)
    INDEX **index;
    int *funct;
    int *type;
    int *namblk;
/*
**++
**  Functional Description:
**	Simulates the VMS LBR$INI_CONTROL routine.  Allocates a structure to
**	store library state.  Initializes the fields in that structure.
**
**  Arguments:
**	index  -- library index as defined
**	funct  -- library function as defined -- ignored
**	type   -- library type as defined -- ignored
**	namblk -- RMS name block as defined -- ignored
**
**  Result:
**	Status code as defined
**--
*/
    {
    *index = (INDEX *) XtMalloc(sizeof(INDEX));

    if (*index == NULL)
        return (LBR__TOOMNYLIB);

    (*index)->dirp = NULL;
    (*index)->fd = NULL;
    (*index)->file_open = FALSE;
    (*index)->library = NULL;
    (*index)->buffer = NULL;

    return (LBR__NORMAL);
    }


int LBR_OPEN(index, fns, create_options, dns, rlfna, rns, rnslen,help_widget_id)
    INDEX **index;
    DESCRIPTOR *fns;
    int *create_options;
    DESCRIPTOR *dns;
    int *rlfna;
    DESCRIPTOR *rns;
    int *rnslen;
    DXmHelpWidget help_widget_id;
/*
**++
**  Functional Description:
**	Simulates the VMS LBR$OPEN routine.  Copies the library file spec and
**	stores it in the index structure.  Opens that directory.  Allocates a
**	buffer for reading the records from the library and stores the pointer
**	to it in the index structure.
**
**  Arguments:
**	index -- library index as defined
**	fns   -- file spec for the help library as defined
**	create_options -- options for creating library -- ignored
**	dns   -- default file spec for opening library -- ignored
**	rlfna -- related file spec for opening library -- ignored
**	rns   -- resultant file spec from opening library -- ignored
**	rnslen -- length of resultant file spec -- ignored
**	help_widget_id -- help widget id
**
**  Result:
**	Status code as defined
**--
*/
    {
#define LANG_STUFF    

    (*index)->library = XtMalloc(fns->dsc_w_length + 1);

    strncpy((*index)->library, fns->dsc_a_pointer, fns->dsc_w_length);
    (*index)->library[fns->dsc_w_length] = '\0';

#ifdef LANG_STUFF    
    /*printf("LBR_OPEN--Orig. filespec '%s%'\n",(*index)->library);*/
    if ((*index)->library[0] != '/')
    {
	char	*resolvedpathname;
	resolvedpathname = 0;
	resolvedpathname =  XtResolvePathname(
				XtDisplay(help_widget_id), /* Display         */
				"help",			   /* Type            */
				(*index)->library,	   /* filename        */
				NULL,			   /* suffix          */
				NULL,	                   /* path            */
				NULL,			   /* user def'd subs */
			        0,			   /* no. of subs.    */
				TestFileAndDir);	   /* predicate routin*/

	/*printf("LBR_OPEN--Resolved pathname '%s%'\n",resolvedpathname);*/

	if (resolvedpathname !=0) {
	    XtFree ((*index)->library);	
	    (*index)->library = XtMalloc(strlen(resolvedpathname) + 1);
            strcpy((*index)->library, resolvedpathname);
	    XtFree (resolvedpathname);
	}
    }
#endif

    (*index)->dirp = opendir((*index)->library);

    if ((*index)->dirp == NULL) {
	XtFree((*index)->library);
	(*index)->library = NULL;
	return (RMS__FNF);
    };

    (*index)->buffer = XtMalloc(BUFFER_SIZE);

    return (LBR__NORMAL);
    }


int LBR_CLOSE(index)
    INDEX **index;
/*
**++
**  Functional Description:
**	Simulates the VMS LBR$CLOSE routine.  Closes any open files, closes the
**	library directory, and frees allocated storage.
**
**  Arguments:
**	index -- library index as defined
**
**  Result:
**	Status code as defined
**--
*/
    {
    if ((*index)->file_open && (*index)->fd != NULL)
	fclose((*index)->fd);

    if ((*index)->dirp != NULL)
	closedir((*index)->dirp);

    if ((*index)->library != NULL)
	XtFree((*index)->library);

    if ((*index)->buffer != NULL)
	XtFree((*index)->buffer);

    XtFree((char *)*index);

    return (LBR__NORMAL);
    }


int LBR_GET_INDEX(index, number, routine, match)
    INDEX **index;
    int *number;
    int (*routine)();
    DESCRIPTOR *match;
/*
**++
**  Functional Description:
**	Simulates the VMS LBR$GET_INDEX routine.  Calls specified routine for
**	each file in the directory, passing as arguments the name of the file
**	and the location of that file in the directory (simulates RFA's).
**
**  Arguments:
**	index -- library index as defined
**	number -- library number -- ignored
**	routine -- routine that is called for each index entry
**	match -- match descriptor use to select index entries -- ignored
**
**  Result:
**	Status code as defined
**--
*/
    {
    int cnt;
#if defined(UNIX_SYSV) || defined(__osf__)
    struct dirent *dp;
#else
    struct direct *dp;
#endif
    DESCRIPTOR_S key_name;

    cnt = 0;

    if ((*index)->dirp == NULL)
	return (LBR__LIBNOTOPN);

    key_name.dsc_b_dtype = DSC_K_DTYPE_T;
    key_name.dsc_b_class = DSC_K_CLASS_S;

    rewinddir((*index)->dirp);

    while (TRUE) {
	int status,
	    loc;

	loc = telldir((*index)->dirp);
	dp = readdir((*index)->dirp);

	if (dp == NULL)
	    break;

	if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
	    continue;

#if defined(UNIX_SYSV) || defined(__osf__)
	key_name.dsc_w_length = strlen(dp->d_name);
#else
	key_name.dsc_w_length = dp->d_namlen;
#endif
	key_name.dsc_a_pointer = dp->d_name;

	status = (*routine)(&key_name, &loc);

	if ((status & 1) == 0)
	    break;

	cnt++;
    }

    
    if (cnt > 0)
	return (LBR__NORMAL);
    else
	return (LBR__NULIDX);
    }


int LBR_SET_LOCATE(index)
    INDEX **index;
/*
**++
**  Functional Description:
**	Simulates the VMS LBR$SET_LOCATE routine.  Basically, a no-op.
**
**  Arguments:
**	index -- library index as specified -- ignored
**
**  Result:
**	Status code as specified
**--
*/
    {
    return (LBR__NORMAL);
    }


int LBR_FIND(index, loc)
    INDEX **index;
    int *loc;
/*
**++
**  Functional Description:
**	Simulates the VMS LBR$FIND routine.  Uses the "RFA" to reposition the
**	open library directory to the reqeusted file, gets the name of that
**	file, and prepends the name of the library directory, then opens that
**	file.
**
**  Arguments:
**	index -- library index as defined
**	loc   -- location (RFA) of the module as defined
**
**  Result:
**	Status code as defined
**--
*/
    {
    char *fs;
#if defined(UNIX_SYSV) || defined(__osf__)
    struct dirent *dp;
#else
    struct direct *dp;
#endif

    if ((*index)->dirp == NULL)
	return (LBR__LIBNOTOPN);

    seekdir((*index)->dirp, *loc);

    dp = readdir((*index)->dirp);
    
    if (dp == NULL)
	return (LBR__INVRFA);

#if defined(UNIX_SYSV) || defined(__osf__)
    fs = XtMalloc(strlen((*index)->library) + strlen(dp->d_name) + 2);
#else
    fs = XtMalloc(strlen((*index)->library) + dp->d_namlen + 2);
#endif

    strcpy(fs, (*index)->library);
    strcat(fs, "/");
    strcat(fs, dp->d_name);

    if ((*index)->file_open) {
        fclose((*index)->fd);
	(*index)->file_open = FALSE;
    };

    (*index)->fd = fopen(fs, "r");

    XtFree(fs);

    if ((*index)->fd == NULL)
	return (LBR__INVRFA);

    (*index)->file_open = TRUE;

    return (LBR__NORMAL);
    }    


int LBR_GET_RECORD(index, inbufdes, outbufdes)
    INDEX **index;
    DESCRIPTOR *inbufdes;
    DESCRIPTOR *outbufdes;
/*
**++
**  Functional Description:
**	Simulates the VMS LBR$GET_RECORD routine.  Reads the next record from
**	the library topic file into the read buffer and then returns a
**	descriptor to the buffer.
**
**  Arguments:
**	index -- library index as defined
**	inbufdes -- input buffer descriptor as defined -- ignored
**	outbufdes -- output buffer descriptor as defined
**
**  Result:
**	Status code as defined
**--
*/
    {
    if ((*index)->fd == NULL || (*index)->buffer == NULL)
	return (LBR__LIBNOTOPN);

    if (fgets((*index)->buffer, BUFFER_SIZE, (*index)->fd) == NULL)
	return (RMS__EOF);

    outbufdes->dsc_w_length = strlen((*index)->buffer);
    outbufdes->dsc_a_pointer = (*index)->buffer;

    /*  strip trailing '\n', even if record consists ONLY of '\n'.            */
    /*                                                                        */
    if (outbufdes->dsc_w_length > 0) {
	if (outbufdes->dsc_a_pointer[outbufdes->dsc_w_length - 1] == '\n') {
	    outbufdes->dsc_w_length--;
	}
    }

    return (LBR__NORMAL);
    }


Boolean TestFileAndDir(path)
    String path;

/* Check if path is a valid file or directory spec.                       */

/* This logic is lifted from the routine Boolean TestFile in Intrinsic.c. */
/* It differs from that logic in that it deems a directory as a success,  */
/* instead of just a file name.                                           */
{
    struct stat status;

    /*printf("Inspecting path '%s'\n",path);*/
    return (access(path, R_OK) == 0 &&		/* exists and is readable */
	    stat(path, &status) == 0 );	

/* return of TRUE = resolved string fit, FALSE = didn't fit.  Not
   null-terminated and not collapsed if it didn't fit */
}


#endif  /* end if VMS def */

