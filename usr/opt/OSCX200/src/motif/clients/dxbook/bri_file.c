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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_FILE.C*/
/* *16   28-JAN-1993 16:04:10 GOSSELIN "fixed NO_CONCEAL bug"*/
/* *15   25-JAN-1993 15:26:58 RAMSHAW "QAR #34 - fix ""Assign channel errors"""*/
/* *14   30-OCT-1992 18:37:03 BALLENGER "handle searchList resource"*/
/* *13   17-SEP-1992 19:08:13 BALLENGER "Look for upper case extensions on ISO disks."*/
/* *12   13-AUG-1992 13:34:27 ROSE "Code added to handle ISO message on Ultrix, too"*/
/* *11   12-AUG-1992 13:58:51 ROSE "Added ISO support"*/
/* *10   19-JUN-1992 20:16:24 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *9     9-JUN-1992 10:02:00 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *8     2-MAY-1992 10:23:32 GOSSELIN "going back to earlier gen until ISO support can be debugged"*/
/* *7     1-MAY-1992 16:59:53 ROSE "Repress all error messages until last iteration of file is opened"*/
/* *6    22-APR-1992 16:27:34 ROSE "Fix bug where book in current directory fails to open when specified on commmand line"*/
/* *5    21-APR-1992 23:15:37 ROSE "Support ISO standards for file extensions"*/
/* *4     3-MAR-1992 17:10:48 KARDON "UCXed"*/
/* *3    13-FEB-1992 11:21:44 PARMENTER "fixing qar #1760.  library file not reopening correctly"*/
/* *2    13-NOV-1991 14:50:57 GOSSELIN "alpha checkins"*/
/* *1    16-SEP-1991 12:44:09 PARMENTER "Low-level private access file routines"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_FILE.C*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_FILE.C*/
/* *14   30-OCT-1992 18:37:03 BALLENGER "handle searchList resource"*/
/* *13   17-SEP-1992 19:08:13 BALLENGER "Look for upper case extensions on ISO disks."*/
/* *12   13-AUG-1992 13:34:27 ROSE "Code added to handle ISO message on Ultrix, too"*/
/* *11   12-AUG-1992 13:58:51 ROSE "Added ISO support"*/
/* *10   19-JUN-1992 20:16:24 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *9     9-JUN-1992 10:02:00 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *8     2-MAY-1992 10:23:32 GOSSELIN "going back to earlier gen until ISO support can be debugged"*/
/* *7     1-MAY-1992 16:59:53 ROSE "Repress all error messages until last iteration of file is opened"*/
/* *6    22-APR-1992 16:27:34 ROSE "Fix bug where book in current directory fails to open when specified on commmand line"*/
/* *5    21-APR-1992 23:15:37 ROSE "Support ISO standards for file extensions"*/
/* *4     3-MAR-1992 17:10:48 KARDON "UCXed"*/
/* *3    13-FEB-1992 11:21:44 PARMENTER "fixing qar #1760.  library file not reopening correctly"*/
/* *2    13-NOV-1991 14:50:57 GOSSELIN "alpha checkins"*/
/* *1    16-SEP-1991 12:44:09 PARMENTER "Low-level private access file routines"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_FILE.C*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_FILE.C*/
/* *7    30-MAY-1991 17:44:02 BALLENGER "Set network timeout for remote file access on VMS"*/
/* *6    20-MAR-1991 08:32:36 ACKERMAN "Fixed error message with extra ;;book characters at end"*/
/* *5    22-FEB-1991 17:59:37 FITZELL "fix for missing error message characters"*/
/* *4    18-FEB-1991 14:27:34 BALLENGER "IFT2 Fixes, portability fixes, and Hyperhelp fixes"*/
/* *3    25-JAN-1991 16:49:53 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 11:37:44 FITZELL "V3 ift update snapshot"*/
/* *1     8-NOV-1990 11:26:33 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_FILE.C*/
#ifndef VMS
 /*
#else
# module BRI_FILE "V03-0004"
#endif
#ifndef VMS
  */
#endif

#ifndef lint
static char *sccsid = "%W%   OZIX    %G%" ;
#endif

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
**
** Facility:
**
**   	BRI -- Book Reader Interface
**
** Abstract:
**
**      File handling routines
**
** Functions:
**
**      BriFileParse
**  	check_file
**  	BriFileSearch
**  	BriFileOpen
**  	bri_file_type
**
** Author:
**
**      David L. Ballenger
**
** Date:
**
**      Fri Sep  1 11:11:55 1989
**
** Revision History:
**
**  V03-0005    DLB0002     David L Ballenger           30-May-1991
**              Set timeout value for network fiel access on VMS to
**              one minute.
**
**  V03-0004    DLB0001     David L Ballenger           06-Feb-1991
**              Conditionalize inclusion of <xabitmdef.h>.
**
**  V03-0003	JAF0003	    James A. Ferguson	    	10-Sep-1990
**  	    	Conditionalize BUFFER_SIZE on VMS because RMS's 2 byte
**  	    	overhead doesn't appear in the header, and reparse the file
**  	    	spec on VMS if it contains a node name so logical names
**  	    	get translated correctly in BriFileOpen.
**
**  V03-0002	JAF0002	    James A. Ferguson	    	7-Sep-1990
**  	      	Fix XAB initialization code in BriFileParse, conditionalize
**  	    	current directory specification in BriFileOpen and set
**  	    	"file_open" to TRUE in open_file, if successfully opened.
**
**  V03-0001    DLB         David L Ballenger           29-Aug-1990
**              Use RMS for file access on VMS.
**
**  V03-0000	JAF0001	    James A. Ferguson	    	16-Aug-1990 
**  	      	Create new module, modify revision history format and
**  	    	update copyright.
** 
**  	    	JAF 	    James A. Ferguson	    	3-Aug-1990  
**  	       	Modify BRI_FILE_TYPE to exclude the rms_length field
**  	       	when checking the file for a BMD_C_BOOK on VMS.
**
**  	    	DLB 	    David L Ballenger	    	30-May-1990 
**             Make changes to improve performance. On VMS access the
**             book as a variable length record file as opposed to 
**             stream.
**
**  	    	DLB 	    David L Ballenger	    	21-Mar-1990 
**             	Fix file extension logic in BriFileParse().
**
*/


#ifdef vms
#define FOPEN_ARGS "r","rfm=var"
#define RMS_NODENAME_BITSET 	(1L << NAM$V_NODE)
#define BUFFER_SIZE 1022  /* RMS's 2 bytes are not part of the header on VMS */
#else
#define FOPEN_ARGS "r"
#define BUFFER_SIZE 1024
#endif 

#define DOUBLE_QUOTE_CHAR   '\"'

#include "bri_private_def.h"
#ifdef vms
#include <xabitmdef.h>
#endif 
#include <errno.h>
#include <ctype.h>


static Boolean	STATIC_all_extensions_tried = FALSE;


void
BriFileParse(context, home_directory, file_extension, search_mode)
    register BRI_CONTEXT	*context; /* Pointer to book or shelf context block */
    register char		*home_directory;
    unsigned			file_extension;
    int				search_mode;
/*
 *
 * Function description:
 *
 *      Parses a file specifcation, substituting default file information.
 *      On VMS this is done with the RMS PARSE routine.  On ULTRIX
 *      this is done on our own since there is no PARSE routine on ULTRIX.
 *
 * Arguments:
 *
 *      context - pointer to the context for the book or shelf to which 
 *                the file spec belongs.
 *	search_mode - either SEARCHING or NOT_SEARCHING - used to correctly 
 *			set nam$b_nop
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Calls BriLongJmp() if an error occurs.
 *
 * Note:
 *	Normally sys$parse assigns system resources (including an I/O
 *	channel) this is done in preparation for sys$search, the
 *	resources are de-allocated when sys$search fails to find a match.
 *	So, when using sys$parse, prior to sys$open, NAM$M_SYNCHK must
 *	be set in nam$b_nop in order to inhibit this resource allocation.
 */

{

#ifdef  vms	/* VMS declarations */

    /* Define an item list structure.  There doesn't seem to be one in
     * any of the include files.
     */
    struct ITEM_LIST { 
        unsigned short ITM$W_BUFSIZ;
        unsigned short ITM$W_ITMCOD;
        unsigned char  *ITM$L_BUFADR;
        unsigned long  ITM$L_RETLEN;
    };

    /* Define the network buffer size,
     */
    static unsigned long network_buffer_size = 32767 ;
    static unsigned long timeout_seconds = 60 ;

    /* Define  item list and xab item for setting the network buffer size
     * and the timeout value in seconds to use when accessing files
     * across the network.  Item lists MUST BE zero terminated.
     */
    static struct ITEM_LIST net_item_list[3] = {
    	{ sizeof( network_buffer_size ),
          XAB$_NET_BLOCK_COUNT,
          (unsigned char *)&network_buffer_size,
          0 },
        { sizeof(timeout_seconds), 
          XAB$_NET_LINK_TIMEOUT, 
          (unsigned char *)&timeout_seconds,
          0 },
    	{ 0, 0, 0, 0 }
    } ;
    static struct XABITM net_item_xab = {
        XAB$C_ITM,  	    	    	/* cod 	     */
        XAB$C_ITMLEN,	    	    	/* bln 	     */
    	0,  	    	    	    	/* spare     */
        NULL,	    	    	    	/* nxt xab   */
        NULL,				/* item list */
        XAB$K_SETMODE,	    	    	/* mode      */
    	0,  	    	    	    	/* itm fill1 */
    	0   	    	    	    	/* itm fill2 */
    } ;

    /* Use RMS
     */
    int rms_status;

    context->found_file_spec[0] = '\000';

    net_item_xab.xab$l_itemlist = (char *)&net_item_list;

    if (*home_directory) {
        strcpy(context->default_file_spec,home_directory);
    } else {
        strcpy(context->default_file_spec,DEFAULT_DIRECTORY);
    }

    if (context->entry.entry_type == BMD_C_SHELF) {
	if (file_extension == DEFAULT_EXTENSION)
	    strcat (context->default_file_spec, DECW_DEFAULT_SHELF_EXT);
	else if (file_extension == ISO_EXTENSION)
	    strcat (context->default_file_spec, ISO9960_SHELF_EXT);
	else
	    strcat (context->default_file_spec, NFS_SHELF_EXT);
    } 
    else {
	if (file_extension == DEFAULT_EXTENSION)
	    strcat (context->default_file_spec, DECW_DEFAULT_BOOK_EXT);
	else if (file_extension == ISO_EXTENSION)
	    strcat (context->default_file_spec, ISO9960_BOOK_EXT);
	else
	    strcat (context->default_file_spec, NFS_BOOK_EXT);
    }

    /* Initialize the fab and nam blocks, and an xab for setting 
     * network buffering.
     */
    context->fab=cc$rms_fab;		/* FAB data structure */
    context->nam=cc$rms_nam;		/* NAM data structure */
    context->fab.fab$l_xab = (char *)&net_item_xab;
    context->fab.fab$l_dna = &context->default_file_spec[0];
    context->fab.fab$b_dns = strlen(context->fab.fab$l_dna);
    context->fab.fab$l_fna = context->entry.target_file;
    context->fab.fab$b_fns = strlen(context->entry.target_file);
    context->fab.fab$w_ifi = 0;
    context->fab.fab$l_nam = &context->nam;
    context->nam.nam$l_esa = &context->parsed_file_spec[0];
    context->nam.nam$b_ess = NAM$C_MAXRSS ;
    context->nam.nam$l_rsa = &context->found_file_spec[0];
    context->nam.nam$b_rss = NAM$C_MAXRSS ;

    if (search_mode == NOT_SEARCHING)
	context->nam.nam$b_nop |= NAM$M_SYNCHK ;
    
    /* Get the parsed file specfication -- use default_file_spec
     * for missing values
     */
    
    context->parsed_file_spec[0] = '\0';
    rms_status = sys$parse(&context->fab);
    if (rms_status != RMS$_NORMAL) {
        BriLongJmp(context,BriErrFileSearchNum,rms_status);
    }
    context->parsed_file_spec[context->nam.nam$b_esl] = '\0';
    
#else /* end of VMS-specific code, start of ULTRIX-specific code */

    register char *last_dot;
    register char *last_slash;

    /* Get the target file from the shelf_entry.
     */
    context->target_file = context->entry.target_file;
    context->found_file_spec[0] = '\000';
    
    last_slash = strrchr(context->entry.target_file,'/');
    if ( last_slash != NULL) {
        
        /* The target file has a directory (absolute or relative)
         * specified.  So we will look for it explicitly and not look
         * in the shelf home directory or in the directory search list.
         */
        context->search_state = absolute;
        context->search_list = "";
        
    } else {
        
        /* No directory was specified in the target file, so set the
         * home directory to be home directory where the bookshelf was
         * found.  If the search list environment variable is defined
         * then the directories specified by it will be searched, else
         * the default directory will be searched.
         */
        context->search_state = relative ;

        if (*home_directory) {
            context->search_list = home_directory;
        } else {

            register char *search_list_logical = ULTRIX_SEARCH_LIST_NAME ;

            context->search_list = bri_logical(search_list_logical);
            if (context->search_list == search_list_logical) {
                context->search_list = DEFAULT_DIRECTORY;
            }
        }
    }

    last_dot = strrchr(context->entry.target_file,'.');
    if ( (last_dot == NULL) ||
         ((last_slash != NULL) && (last_slash > last_dot))
       ) {
        
        /* Use the appropriate file extension.
         */
        if (context->entry.entry_type == BMD_C_SHELF) {
	    if (file_extension == DEFAULT_EXTENSION)
		context->file_extension = DECW_DEFAULT_SHELF_EXT;
	    else if (file_extension == ISO_EXTENSION)
		context->file_extension = ISO9960_SHELF_EXT;
	    else
		context->file_extension = NFS_SHELF_EXT;
        }
	else {
	    if (file_extension == DEFAULT_EXTENSION)
		context->file_extension = DECW_DEFAULT_BOOK_EXT;
	    else if (file_extension == ISO_EXTENSION)
		context->file_extension = ISO9960_BOOK_EXT;
	    else
		context->file_extension = NFS_BOOK_EXT;
        }
    } 
    else {

        /* The target file already has a file extension.
         */
        context->file_extension = NULL ;
    }

# endif /* end of ULTRIX-specific code */

} /* end BriFileParse */

#ifndef vms
static
check_file(context,directory_list)
    register BRI_CONTEXT *context;
    char **directory_list;
/*
 *
 * Function description:
 *
 *      ULTRIX only routine to splice together the parts of a file spec
 *      and then check if it is accessible.
 *
 * Arguments:
 *
 *      context            - pointer to the file spec block
 *      directory_list - pointer to a list of file directory strings separated
 *                       by spaces.
 *
 * Return value:
 *
 *      True if the specified file is accessible, false if not.
 *
 * Side effects:
 *
 *      Updates the directory string pointer to point to the next directory,
 *      if any in the string.
 *
 */

{
    struct stat buffer;
    register char *src = *directory_list;
    register char *dst = &context->found_file_spec[0];
    char *start_of_name;


    /* Skip any initial spaces in the directory string.
     */
    while (*src && isspace(*src)) {
        src++;
    }
    
    /* Copy the directory string, up to any terminating space, into the
     * found_file_spec.
     */
    while (*src && !isspace(*src)) {
        *dst++ = *src++ ;
    }

    /* Skip any trailing spaces in the directory string.
     */
    while (*src && isspace(*src)) {
        src++;
    }
    
    /* Return the updated directory_list.
     */
    *directory_list = src;

    /* Make sure the directory string in the found_file_spec ends with
     * a '/'.
     */
    if (dst != &context->found_file_spec[0] 
        && directory_list != &context->target_file) {
        if (dst[-1] != '/') {
            *dst++ = '/' ;
        }
    }
    start_of_name = dst;

    /* Copy the target file name into the found file spec and append
     * the file extension.
     */
    strcpy(dst,context->target_file);
    if (context->file_extension) {
        strcat(dst,context->file_extension);
    }

    /* See if the specified file is accessible.
     */
    if (stat(context->found_file_spec,&buffer) != -1) {
        return TRUE ;
    }
    else 
    {
        /* It wasn't so try substituting a '$' for the '_' in the file extension
         * (i.e. .decw_book -> .decw$book) in case we are accessing the
         * file via UCX on a VMS system.
         */
        dst = strrchr(start_of_name,'.');
        while (*dst) {
            if (*dst == '_') {
                *dst = '$';
                break ;
            }
            dst++;
        }

        /* Try again.
         */
        if (stat(context->found_file_spec,&buffer) != -1) {
            return TRUE ;
        }
        else 
        {
            /* Try uppercasing the filename and extension
             */
            register char *to, *from;
            
            to = start_of_name;
            from = start_of_name;
            
            while (*from) {
                *to++ = toupper(*from++);
            }
            *to = *from;
            
            if (stat(context->found_file_spec,&buffer) != -1) {
                return TRUE ;
            }
        }
    }
    return FALSE ;

} /* end check_file */
#endif 

BriFileSearch(context)
    register BRI_CONTEXT *context;
/*
 *
 * Function description:
 *
 *      Search for a file in a search list.  On VMS this is done with the
 *      RMS $SEARCH routine.  On ULTRIX we roll our own.  This hides the
 *      gory details from the callers.
 *
 *      If and accessible file is found its full file spec is left in the
 *      found_file_spec field of the file spec block.
 *
 * Arguments:
 *
 *      context - pointer to the file spec block
 *
 * Return value:
 *
 *      True if an accessible file was found in the search list, false
 *      if not.
 *
 * Side effects:
 *
 *      Updates the file spec block.
 *
 */

{
#ifdef vms
    
    /* Just call the RMS Search routine.
     */
    if (sys$search(&context->fab) == RMS$_NORMAL) {
	char *cp;
	
	/* first null terminated the f_f_s[] string */

	context->found_file_spec[context->nam.nam$b_rsl] = '\000' ;

	/* 
	 * now look for a trailing ';'.  We have to strip off the version 
	 * number, so that we can reopen the library file correctly 
	 */
	
	cp = strrchr( context->found_file_spec, ';' );
	if ( cp != 0 )
	    *cp = '\000';

        return TRUE ;
    } else {
        context->found_file_spec[0] = '\000' ;
        return FALSE ;
    }
#else

    struct stat buffer;

    if (context->search_state == absolute) {
        char *dummy_directory = "";
        
        /* This will complete the search for this file spec block.
         */
        context->search_state = complete ;

        return check_file(context,&dummy_directory);

    } else {
        if (context->search_state == relative) {
    
            while (*context->search_list) {

                /* We have a home directory to look in, so try it.
                 */
                if (check_file(context,&context->search_list)) {
                    return TRUE;
                }
            }
            
            context->search_state = complete;
        }
    }
    return FALSE ;
#endif 
} /* end BriFileSearch */


static 
open_file(context,report_file_not_found)
    register BRI_CONTEXT *context;
    int report_file_not_found;
/*
 *
 * Function description:
 *
 *      Opens a file using RMS directly on VMS and stdio on ULTRIX.
 *
 * Arguments:
 *
 *      context - the context for the file
 *
 * Return value:
 *
 *      TRUE if the file was opened, FALSE if not.
 *
 * Side effects:
 *
 *      None
 *
 */

{
#ifdef vms
    /* Use RMS
     */
    int rms_status;


    /* Open the file
     */
    rms_status = sys$open(&context->fab);
    context->parsed_file_spec[context->nam.nam$b_esl] = '\0';
    if (rms_status == RMS$_NORMAL) {
        
        /* The open succeeded, go ahead and set up the RAB and connected
         * it.  The access mode will be set to RFA if the file is a book,
         * and sequential if the file is a shelf or we don't know yet.
         */
        context->rab = cc$rms_rab;
        context->rab.rab$l_fab = &context->fab;
        if (context->entry.entry_type == BMD_C_BOOK) {
            context->rab.rab$b_rac = RAB$C_RFA;
        } else {
            context->rab.rab$b_rac = RAB$C_SEQ;
        }
        rms_status = sys$connect(&context->rab);
    	context->file_open = TRUE;
        return TRUE;

    } else {
        
        if ((rms_status != RMS$_FNF) || report_file_not_found) {

	    /* Tried every extension, so put up a different error message
	       letting the user know all were tried */
	    if (STATIC_all_extensions_tried) {
		BriLongJmp (context,BriErrFileOpenAnyNum,rms_status);
		STATIC_all_extensions_tried = FALSE;
	    }
	    else
		/* Couldn't open the file.  Tell the user which file could not
		   be opened and why. */
		BriLongJmp(context,BriErrFileOpenNum,rms_status);
        }
    }
    context->file_open = FALSE;
    return FALSE;
#else
    while ( BriFileSearch(context)) {

        /* Found a file spec for an existing file, so try to open it.
         */
        context->file = fopen(context->found_file_spec,FOPEN_ARGS);
        if (context->file == NULL) {

            /* Couldn't open the file.  Tell the user which
             * file could not be opened and why.
             */
            BriLongJmp(context,BriErrFileOpenNum,errno);

        } else {
            /* Actually found and opened a file.
             */

            /* If this is a book, then it will be read directly rather
             * than through the stdio buffer so turn off buffering.
             */
            if (context->entry.entry_type == BMD_C_BOOK) {
                setvbuf(context->file,NULL,_IONBF,0);
            }

            /* Now get out of the loop and return TRUE.
             */
    	    context->file_open = TRUE;
            return TRUE;
        }
    }
    if (report_file_not_found) {
	/* Tried every extension, so put up a different error message
	   letting the user know all were tried */
	if (STATIC_all_extensions_tried) {
	    BriLongJmp (context,BriErrFileOpenAnyNum,ENOENT);
	    STATIC_all_extensions_tried = FALSE;
	}
	else
	    /* Couldn't open the file.  Tell the user which file could not
	       be opened and why. */
	    BriLongJmp(context,BriErrFileOpenNum,ENOENT);
	    /*BriLongJmp(context,BriErrFileSearchNum,ENOENT);*/
    }
    context->file_open = FALSE;
    return FALSE;
#endif 

} /* end open_file */


void
BriFileOpen(context)
    register BRI_CONTEXT *context;
/*
 *
 * Function description:
 *
 *      Open a file
 *
 * Arguments:
 *
 *      context - pointer to the context block for the book or shelf file
 *                being opened.
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Calls BriLongJmp() if the file can not be opened.
 *
 */

{
    register char	*directory_end;
    char		*open_args, *period_loc, *bracket_loc;
    Boolean		status = FALSE, report_error = FALSE;
    short		file_extension = DEFAULT_EXTENSION, last_extension;

#ifdef VMS
    /* Get the last period location */       
    period_loc = strrchr (context->entry.target_file, PERIOD);

    /* Get the last bracket location */
    bracket_loc = strrchr (context->entry.target_file, RIGHT_SQUIGLLY);
    if (bracket_loc == NULL)
	bracket_loc = strrchr (context->entry.target_file, RIGHT_BRACKET);

    /* If there are no brackets, or if last bracket is before last period,
       then a specific extension exists */
    if ((period_loc != NULL && bracket_loc == NULL) || bracket_loc < period_loc) {
	last_extension = DEFAULT_EXTENSION;
	STATIC_all_extensions_tried = FALSE;
    }
    else {
	last_extension = NFS_EXTENSION;
	STATIC_all_extensions_tried = TRUE;
    }
#else
    /* If period is in Ultrix file spec, assume specified extension */
    if (strchr (context->entry.target_file, PERIOD) != NULL) {
	last_extension = DEFAULT_EXTENSION;
	STATIC_all_extensions_tried = FALSE;
    }
    else {
	last_extension = NFS_EXTENSION;
	STATIC_all_extensions_tried = TRUE;
    }
#endif

    while (file_extension <= last_extension && !status) {

	/* Parse the file spec in the entry for the book or shelf environment
	   using the default bookreader extension */
	BriFileParse (context, context->entry.home_directory,
			file_extension, NOT_SEARCHING);

#ifdef vms
	/* If the parsed file spec contains a node name, and NOT a quoted
	 * string (ie, username/password), and the home_directory used in the
	 * previous parse doesn't contain a node name, then reparse so
	 * logical names get translated correctly. */
	if ((context->nam.nam$l_fnb & RMS_NODENAME_BITSET)
    	    && (context->nam.nam$b_node > 0)) {
	    char	quoted_str_not_found = TRUE;
	    int 	index = 0;
	    char	*ch_ptr = context->nam.nam$l_node;

	    while (index < context->nam.nam$b_node) {
		if (*ch_ptr == DOUBLE_QUOTE_CHAR) {
		    quoted_str_not_found = FALSE;
		    break;
		}
		index++;
		ch_ptr++;
	    }
	    if (( !strstr(context->entry.home_directory,"::"))
    	    	   && (quoted_str_not_found))
		BriFileParse(context, "", file_extension, NOT_SEARCHING);
	}
#endif

	/* Try to open a file relative to the book/shelf environment */
	status = open_file (context, report_error);
        
	/* If it failed, try again using the search list */
	if (!status) {
	    BriFileParse (context, "", file_extension, NOT_SEARCHING);

	    if (file_extension == last_extension)
		report_error = TRUE;

	    status = open_file (context, report_error);
	}

	file_extension++;
    }

    /* If the file spec included a directory, then that becomes the
     * home directory for the shelf.  This is important for entries
     * in the shelf since we want to look for books and shelves it
     * specifes relative to it before looking in the search list.
     */
#ifdef vms
    directory_end = context->nam.nam$l_name;
#else
    directory_end = strrchr(context->found_file_spec,'/');
#endif 
    if (directory_end != NULL) {
        char save_char = *directory_end;
        *directory_end = (char)0;
        BriStringAlloc(&context->entry.home_directory,
                         context->found_file_spec,
                         context
                         );
        *directory_end = save_char;
    } else {
        
        /* No directory was specfied, so look relative to the current
         * directory.
         */
        context->entry.home_directory = CURRENT_DIRECTORY ;
    }


} /* end BriFileOpen */


BMD_ENTRY_TYPE
bri_file_type PARAM_NAMES((file_name))
    char *file_name PARAM_END
/*
 *
 * Function description:
 *
 *      
 *
 * Arguments:
 *
 *      None
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{
    BRI_CONTEXT *context;
    int bytes_read;
#ifdef vms
    int rms_status;
#endif 
    union {
        struct {
#ifndef VMS 	    /* RMS's 2 bytes don't appear in the buffer read on VMS */
            BR_UINT_16 rms_length;
#endif
            BR_UINT_16 tag;
            BR_UINT_32 length;
        } desc ;
        char data[BUFFER_SIZE];
    } buffer ;
    BMD_ENTRY_TYPE file_type = BMD_C_UNKNOWN;
    jmp_buf error_return;

    /* Set up the jump_buffer for error unwinding prior to calling any
     * routines.  From this point on any errors that occur while
     * opening the book will result in a call to BriLongJmp which after
     * processing the error will do a longjmp to "return" from the setjmp().
     */
    if (setjmp(error_return) != 0) {

        /* Whoops!!! An error was detected while trying to open the book.
         * The reason should be in the book context.  Report the error,
         * delete the book context, and return NULL to indicate to the
         * calling routine that an error occurred.
         */
        BriContextDelete(context);
        return BMD_C_UNKNOWN;
    }

    /* Set up the context for the book.  Note that the address of the
     * book context pointer is passed so that it can be set prior to
     * any errors being reported via BriLongJmp().  
     */
    BriContextNew(&context,file_name,BMD_C_UNKNOWN,error_return);

    /* Open the file and then try to figure out what it is.
     */
    BriFileOpen(context);

    /* Read in the first 1K block of the file and do some test to see
     * if it is a book or shelf.
     */
#ifdef vms
    context->rab.rab$l_ubf = &buffer ;
    context->rab.rab$w_usz = sizeof(buffer);
    
    rms_status = sys$get(&context->rab);
    if (rms_status != RMS$_NORMAL) {
        BriLongJmp(context,BriErrFieldReadNum);
    } else {
        bytes_read = context->rab.rab$w_rsz;
    }
#else 
    bytes_read = fread(&buffer,1,sizeof(buffer),context->file);
#endif 
    if (bytes_read == 0) {
        BriLongJmp(context,BriErrFieldReadNum) ;
    }

    if ((bytes_read == BUFFER_SIZE) &&
        (buffer.desc.length <= BUFFER_SIZE ) &&
#ifndef VMS
        (buffer.desc.rms_length == (BUFFER_SIZE - sizeof(buffer.desc.rms_length))) &&
#endif
        (buffer.desc.tag == BODS_C_BOOK_HEADER)
        ) {
        /* Looks like a book.
         */
        file_type = BMD_C_BOOK ;
    } else {

        /* See if it is a shelf.
         */
        char *ptr = &buffer.data[1];

        if ( *ptr < '\040') {
            /* Presumably a RMS variable length record file, skip the
             * RMS length field.
             */
            ptr++;
        } else {
            /* A stream file, start with the first byte.
             */
            ptr--;
        }

        /* See if if begins with any of the valid characters for a
         * shelf.
         */
        switch (*ptr) {
            case '#':
            case '!':
            case 'T':
            case 't':
            case 'B':
            case 'b':
            case 'S':
            case 's': {
                file_type = BMD_C_SHELF;
                break;
            }
            default: {
                BriLongJmp(context,BriErrUnknownFileNum);
                break;
            }
        }
    }
    
    /* Delete the context and return the file type.
     */
    BriContextDelete(context);

    return file_type;

} /* end BRI_FILE_TYPE */

/* end bri_file.c */

