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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_SHELF.C*/
/* *13   25-JAN-1993 15:32:46 RAMSHAW "QAR #34 - fix ""Assign channel errors"""*/
/* *12   30-OCT-1992 18:37:13 BALLENGER "handle searchList resource"*/
/* *11   12-AUG-1992 14:03:45 ROSE "Added ISO support for shelves"*/
/* *10   19-JUN-1992 20:16:46 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *9     9-JUN-1992 10:02:38 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *8     2-MAY-1992 10:24:02 GOSSELIN "going back to earlier gen until ISO support can be debugged"*/
/* *7     1-MAY-1992 18:50:48 ROSE "Open shelf code now looks for ISO standard *in addition to* standard extensions"*/
/* *6     1-MAY-1992 16:58:46 ROSE "Done"*/
/* *5    21-APR-1992 23:16:22 ROSE "Support ISO standards for shelf extension; re-write parse code to get rid of continue*/
/*statements"*/
/* *4     3-MAR-1992 17:11:25 KARDON "UCXed"*/
/* *3    21-FEB-1992 16:21:53 GOSSELIN "commented expanded_logical"*/
/* *2    12-FEB-1992 14:15:20 FITZELL "fixes the decw$bookshelf multiple loop problem qar 245"*/
/* *1    16-SEP-1991 12:44:29 PARMENTER "Bookshelf management"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_SHELF.C*/
/*  DEC/CMS REPLACEMENT HISTORY, Element BRI_SHELF.C */
/*  *10   10-FEB-1992 13:39:22 FITZELL "qar 245 fix search list loop when using decw$bookshelf logical" */
/*  *9    15-NOV-1991 10:26:58 GOSSELIN "checked in common sources for VDM 1.1 BL2" */
/*  *8    15-OCT-1991 10:54:47 FITZELL "put setjmp/longjmp back in for alpha" */
/*  *7     8-OCT-1991 09:09:40 FITZELL "forgot to take out a printfd that was put  in for alpha_debug" */
/*  *6    30-SEP-1991 14:07:42 FITZELL "Alpha checkins" */
/*  *5    22-FEB-1991 17:02:57 FITZELL "ift2 bug fixes" */
/*  *4     6-JAN-1991 22:32:07 KRAETSCH "ACCVIO on missing \" */
/*  *3     6-JAN-1991 20:47:56 KRAETSCH "Fix default Libary titles for I18N" */
/*  *2    17-SEP-1990 10:02:57 FERGUSON "put RMS back into BRI and perf. fix for topic invocations" */
/*  *1    21-AUG-1990 17:35:59 FERGUSON "pre-ift (BL6) checkins - MEMEX support" */
/*  DEC/CMS REPLACEMENT HISTORY, Element BRI_SHELF.C */
#ifndef VMS
 /*
#else
# module BRI_SHELF "V03-0002"
#endif
#ifndef VMS
  */
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
**  FACILITY:
**
**   BRI -- Book Reader Interface
**
** ABSTRACT:
**
**   This module implements the bookshelf management routines for BRI
**
** FUNCTIONS:
**
**  	bri_shelf_close
**  	bri_shelf_entry_count
**  	bri_shelf_home_file
**  	bri_shelf_target_file
**  	bri_shelf_file_spec
**  	ReadRmsVarRecord
**  	OpenShelf
**  	bri_shelf_open_file
**  	bri_shelf_open
**  	bri_shelf_entry
**  	bri_shelf_openlib
**
** AUTHORS:
**
**   Joseph M. Kraetsch
**
** CREATION DATE: 02-APR-1989
**
** MODIFICATION HISTORY:
**
**  V03-0002    DLB0006         David L Ballenger           18-Feb-1991
**              Check for and ignore blank lines in shelf files.
**
**  V03-0001    DLB         David L Ballenger           29-Aug-1990
**              Use RMS for file access on VMS.
**
**  V03-0000	JAF0000	    James A. Ferguson	    	16-Aug-1990 
**  	      	Create new module, modify revision history format and
**  	    	update copyright.
**
**  	    	DLB 	    David L Ballenger	    	05-Jul-1990 
**             	Incorporate changes from Jim Ferguson to allow
**             	specification of a home directory when opening a
**             	shelf by file name.
**
**  	    	DLB 	    David L Ballenger	    	30-May-1990 
**             	Cleanup (i.e. remove most contionaliztaion) include
**             	files for new VMS standards.
**
**  	    	DLB 	    David L Ballenger	    	21-Mar-1990 
**             	Set the correct entry type (i.e. BMD_C_SHELF) in the dummy_env
**             	when opening libraries.
**
**  	    	DLB 	    David L Ballenger	    	16-Jan-1990 
**             	Add latent support for shelf titles and comments in bookshelf
**             	files based on code added to Bookreader V2.1 for the VMS
**             	Aetna release.
**
*/


/*  
**  INCLUDE FILES  
*/
#include "bri_private_def.h"
#include "br_malloc.h"

/*
**
**  DECLARATIONS
**
*/

#ifdef vms
#define DEFAULT_DIR "DECW$BOOK:"
#else
#define DEFAULT_DIR "/usr/lib/dxbook"
#endif 
#define MAX_ENTRIES 255
#define LINE_BUFFER_SIZE 513
#define LIBRARY_ENTRY_ALLOC 5

/*extern char *expanded_logical;*/

/*
** FORWARD ROUTINES
*/


/*
**  bri_shelf_close -- closes a shelf.
**
*/
void
bri_shelf_close PARAM_NAMES((shelf_id))
    BMD_SHELF_ID shelf_id PARAM_END
{
    /* Closing a shelf is just deleting the context.
     */
    BriContextDelete((BRI_CONTEXT *)shelf_id);
}

/*
**  bri_shelf_entry_count -- number of entries in a bookshelf
**
*/
BR_UINT_32
bri_shelf_entry_count PARAM_NAMES((shelf_id))
    BMD_SHELF_ID shelf_id PARAM_END

{
    return BriShelfPtr((BRI_CONTEXT *)shelf_id)->n_entries;
}

/*
**  bri_shelf_home_file -- name of the file where the shelf resides.
**
**   Entries can come from more than one file, so we need entry_id.
**/
char *
bri_shelf_home_file PARAM_NAMES((shelf_id,entry_id))
    BMD_SHELF_ID shelf_id PARAM_SEP
    BMD_SHELF_ENTRY_ID entry_id PARAM_END
{
    return BriShelfPtr(shelf_id)->shelf_entries[entry_id-1].home_directory;
}


/*
**  bri_shelf_target_file -- name of the file the shelf entry points to.
**
*/
char *
bri_shelf_target_file PARAM_NAMES((shelf_id,entry_id))
    BMD_SHELF_ID shelf_id PARAM_SEP
    BMD_SHELF_ENTRY_ID entry_id PARAM_END
{

    return BriShelfPtr(shelf_id)->shelf_entries[entry_id-1].target_file;

}

/*
**  bri_shelf_file_spec -- get the file name for a shelf
*/
char * 
bri_shelf_file_spec PARAM_NAMES((shelf_id))
    BMD_SHELF_ID shelf_id PARAM_END
{
    
    return ((BRI_CONTEXT *)shelf_id)->entry.target_file;

};	/*  end of bri_book_title*/


#ifndef vms
static char *
ReadRmsVarRecord(buffer,shelf)
    register char *buffer;
    register BRI_CONTEXT *shelf;
/*
 *
 * Function description:
 *
 *      Routine to read a record from an RMS variable length record file
 *      accessed on ULTRIX via UCX or after being copied to ULTRIX via 
 *      dcp in image mode.  This routine is used only on ULTRIX.
 *
 * Arguments:
 *
 *          buffer - address of the buffer for the record
 *
 *          shelf - shelf context
 *
 * Return value:
 *
 *      Same as fgets(), i.e. the address of the buffer if success or
 *      NULL if EOF or an error occurs.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    BR_UINT_16 record_length;
    int read_length;

    /* Read the two byte record length field at the beginning of the
     * RMS variable length record.
     */
    if (fread(&record_length,2,1,shelf->file) != 1) {
        
        /* The read failed.
         */
        return NULL ;
    }

    /* Make sure the record will fit in the buffer with enough room 
     * for a NULL character at the end.
     */
    if (record_length >= LINE_BUFFER_SIZE) {
        return NULL;
    }

    /* Determine how much data to read.  RMS will pad odd-length records
     * so that the next record will begin on a word boundary. Adjust
     * the read length to read the padding if any.
     */
    read_length = record_length + (record_length & 1);

    /* Get the record.
     */
    if (fread(buffer,read_length,1,shelf->file) == 1) {

        /* Got the record.  Null-terminate the string and return the
         * buffer address.
         */
        buffer[record_length] = (char)0;
        return buffer ;

    } else {

        /* The read failed.  Return NULL.
         */
        return NULL ;
    }

} /* end ReadRmsVarRecord */
#endif 

static char *
read_line(buffer,shelf)
    register char *buffer;
    register BRI_CONTEXT *shelf;
/*
 *
 * Function description:
 *
 *      Routine to read a line from the shelf file.  On VMS routine uses
 *      RMS directlly to read the record from the file, and on ULTRIX
 *      it calls fgets().
 *
 * Arguments:
 *
 *      buffer - buffer in which to store the line of text
 *
 *      shelf - the context for the shelf file.
 *
 * Return value:
 *
 *      Same as fgets(), i.e. the address of the buffer if success or
 *      NULL if EOF or an error occurs.
 *
 * Side effects:
 *
 *      None
 *
 */

{
#ifdef vms
    BR_UINT_32 rms_status;

    /* Put the buffer address and size in the RAB.
     */
    shelf->rab.rab$l_ubf = buffer ;
    shelf->rab.rab$w_usz = LINE_BUFFER_SIZE - 1;

    /* Get the record from the file.
     */
    rms_status = sys$get(&shelf->rab);
    if (rms_status == RMS$_EOF) {

        /* EOF, return NULL.
         */
        return NULL;
    }
    if (rms_status == RMS$_NORMAL) {

        /* The read was successful, NULL terminated the line and
         * return a pointer to the buffer.
         */
        buffer[shelf->rab.rab$w_rsz] = (char)0;
        return buffer;
    } else {

        /* If an error occurred signal it.
         */
        BriLongJmp(shelf,BriErrFieldReadNum);
    }
#else
    /* On ULTRIX just call fgets() and return what it returns.
     */
    return fgets(buffer,LINE_BUFFER_SIZE,shelf->file);
#endif 
} /* end read_line */


static
OpenShelf(shelf)
    BRI_CONTEXT *shelf;
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
    BRI_SHELF_BLOCK *sfb;    /* Shelf block for shelf being opened. */
    char line_buff[LINE_BUFFER_SIZE];
    char *(*get_line)() = read_line;
    int  line_number;
    int  entry_number;
    char *token;

    sfb = BriShelfPtr(shelf);

    /* Open the shelf file.
     */
    BriFileOpen(shelf) ;

#ifndef vms
    /* Hack to determine if this is an RMS variable length record file
     * which has be compied from VMS via dcp with image mode, or is being
     * accessed via UCX.  We check the second byte in the file.  If it
     * is not one of the "legal" second characters in a shelf entry,
     * then we assume it is an RMS variable length record file.  It is
     * that or a corrupt stream (ULTRIX or VMS) file.
     *
     * Note: this hack will go away in some future release when shelf 
     * files are no longer ascii text files.
     */

    /* Skip first byte.
     */
    (void)getc(shelf->file);

    /* Check second byte in the file.
     */
    if (getc(shelf->file) < '\040') {
        
        /* This must be an RMS variable length record file. Use the 
         * ReadRmsVarRecord() routine as the get_line routine.
         */
        get_line = ReadRmsVarRecord;
    }
    
    /* Count entries from the start of the file.
     */
    rewind(shelf->file);
#endif 

    /*  count the entries  */
    sfb->n_entries = 0;

    line_number = 0;
    while ((*get_line)(line_buff,shelf) != NULL) {
        if ((line_buff[0] == 'B') || (line_buff[0] == 'b') ||
            (line_buff[0] == 'S') || (line_buff[0] == 's')
           ) {
            sfb->n_entries++;
        }
        line_number++;
    }

    if (sfb->n_entries == 0) {
        BriLongJmp(shelf,BriErrShelfReadNum,line_number);
    }

    /*  allocate a vector for the shelf entry pointers
     */
    sfb->shelf_entries = BriCalloc(BRI_SHELF_ENTRY_ITEM,sfb->n_entries,shelf);

    /* Now read entries from the start of the file.
     */
#ifdef vms
    sys$rewind(&shelf->rab);
#else
    rewind(shelf->file);
#endif 
    
    line_number = 0;
    entry_number = 0;

    while ((*get_line)(line_buff,shelf) != NULL) {

        BRI_SHELF_ENTRY_ITEM *vse = &sfb->shelf_entries[entry_number];

        line_number++;

        /* Check the entry type.
         */
	token = strtok( line_buff, "\\\n");

        /* Check for blank lines and ignore them.
         */
        if (token == NULL) {
            continue;
        }

        switch (*token) {
        case 'B':
        case 'b':
	    /*  BOOK entry syntax: 'BOOK\filename\Title is rest of line'  
	     */
            vse->entry_type = BMD_C_BOOK ;
            entry_number++;
            break;
        case 'S':
        case 's':
	    /*  SHELF entry syntax: 'SHELF\filename\Title is rest of line'  
	     */
            vse->entry_type = BMD_C_SHELF ;
            entry_number++;
            break;
        case 'T':
        case 't':
	    /*  TITLE syntax: 'Title\symbolic_name\Title is rest of line'  
	     */
            vse->entry_type = BMD_C_TITLE ;
            if (shelf->entry.title == NULL) {
		/*  we don't use shelf symbols yet, so just parse it away,
		 *  then get the title
		 */
		token = strtok( NULL, "\\\n");	/*  symbolic name  */
		token = strtok( NULL, "\n");	/*  title  */
		if (token == NULL)
		    BriLongJmp(shelf,BriErrBadShelfEntryNum,line_number);
                BriStringAlloc( &shelf->entry.title, token, shelf);
            }
            continue ;
        case '!':
        case '#':
	    /*  anything after '!' or "#' is COMMENT text  */
            vse->entry_type = BMD_C_COMMENT ;
            continue ; /* Ignore the comment for now */
        default:
            BriLongJmp(shelf,BriErrBadShelfEntryNum,line_number);
        }

        /* Get the target file name for the shelf entry. */
	token = strtok( NULL, "\\\n");
	if (token == NULL)
	    BriLongJmp(shelf,BriErrBadShelfEntryNum,line_number);
        BriStringAlloc( &vse->target_file, token, shelf);
        vse->home_directory = shelf->entry.home_directory ;

        /* Get the title for the shelf entry.  */
	token = strtok( NULL, "\n");
	if (token == NULL)
	    BriLongJmp(shelf,BriErrBadShelfEntryNum,line_number);
        BriStringAlloc( &vse->title, token, shelf);

        if (vse->target_file == NULL || vse->title == NULL ) {
            BriLongJmp(shelf,BriErrBadShelfEntryNum,line_number);
        }
    }

    if (entry_number < sfb->n_entries) {
        BriLongJmp(shelf,BriErrShelfReadNum,line_number);
    }
#ifdef vms
    sys$disconnect(&shelf->rab);
    sys$close(&shelf->fab);
#else
    fclose(shelf->file);
#endif 
    shelf->file_open = FALSE;

} /* end OpenShelf */


/*
**  bri_shelf_open_file  -- open a shelf by file name
**
**
**  Returns:  shelf id
*/
BMD_SHELF_ID
bri_shelf_open_file PARAM_NAMES((file_name,home_dir,entry_count))
    char *file_name PARAM_SEP  /* Shelf file name */
    char *home_dir PARAM_SEP
    BR_INT_32 *entry_count PARAM_END /* Number of entries in shelf being opened */
{
    BRI_CONTEXT *child;     /* Child context for new shelf being opened. */
    jmp_buf error_return;  /* Jump buffer for errors during open. */

    /* Set up a jump buffer for errors which may occur during the course
     * of opening the child shelf.  Errors will be handled by calling 
     * BriLongJmp() which may take some error specifc action but will
     * finally do a longjmp() to "return" from this setjmp().
     */
    if (setjmp(error_return) != 0) {

        /* An error did occur, so report it by calling BriError() to indicate
         * that an error occurred while opening the shelf.
         */
        BriError(child,BriErrShelfOpenNum);

        /* Delete the context for the child shelf and return NULL to
         * indicate that an error occurred.
         */
        BriContextDelete(child);

        return NULL ;
    }

    /* Create a context for this shelf.
     */
    BriContextNew(&child,file_name,BMD_C_SHELF,error_return);

    /* Use the home directory passed otherwise let it be defaulted.
     */
    if (home_dir != NULL)
        BriStringAlloc(&child->entry.home_directory,home_dir,child);

    /* Open and read the shelf file
     */
    OpenShelf(child);

    if (entry_count)
	*entry_count = child->data.shelf->n_entries;

    return (BMD_SHELF_ID)child;

};	/*  end of bri_shelf_open_file  */

/*
**  bri_shelf_open  -- open a shelf.
**
**
**  Returns:  shelf id
*/
BMD_SHELF_ID
bri_shelf_open PARAM_NAMES((parent,entry_id,entry_count))
    BMD_SHELF_ID parent PARAM_SEP  /* Parent context for shelf being opened. */
    BMD_SHELF_ENTRY_ID entry_id PARAM_SEP /* Entry ID for for shelf being opened. */
    BR_INT_32 *entry_count  PARAM_END     /* Number of entries in shelf
                                     * being opened */
{
    BRI_CONTEXT *child;     /* Child context for new shelf being opened. */
    jmp_buf error_return;  /* Jump buffer for errors during open. */

    /* Set up a jump buffer for errors which may occur during the course
     * of opening the child shelf.  Errors will be handled by calling 
     * BriLongJmp() which may take some error specifc action but will
     * finally do a longjmp() to "return" from this setjmp().
     */
    if (setjmp(error_return) != 0) {

        /* An error did occur, so report it by calling BriError() to indicate
         * that an error occurred while opening the shelf.
         */
        BriError(child,BriErrShelfOpenNum);

        /* Delete the context for the child shelf and return NULL to
         * indicate that an error occurred.
         */
        BriContextDelete(child);

        return NULL ;
    }

    /* The context for the child shelf is inherited from the parent.
     */
    BriContextInherit(&child,(BRI_CONTEXT *)parent,entry_id,BMD_C_SHELF,error_return);

    /* Open and read the shelf file
     */
    OpenShelf(child);

    if (entry_count)
	*entry_count = child->data.shelf->n_entries;

    return (BMD_SHELF_ID)child;

};	/*  end of bri_shelf_open  */


/*
**  bri_shelf_entry -- Get an entry from a shelf
*/
void 
bri_shelf_entry PARAM_NAMES((shelf_id,entry_id,entry_type,file_name,
                             width,height,data_addr,data_len,data_type,
                             title))
    BMD_SHELF_ID shelf_id PARAM_SEP    /*  shelf id (from BRI$$OPEN_SHELF) */
    BMD_SHELF_ENTRY_ID entry_id PARAM_SEP    /*  shelf entry number              */
    BR_UINT_32     *entry_type PARAM_SEP /*  1 = BOOK,  2 = SHELF            */
    char           **file_name PARAM_SEP /*  shelf file name		   */
    BR_UINT_32     *width PARAM_SEP      /*  total width of shelf entry      */
    BR_UINT_32     *height PARAM_SEP     /*  total height of shelf entry     */
    BR_UINT_32     *data_addr PARAM_SEP  /*  addr of display data buffer     */
    BR_UINT_32     *data_len PARAM_SEP   /*  length of display data	   */
    BR_UINT_32     *data_type PARAM_SEP  /*  type of display data            */
    char           **title PARAM_END     /*  title of entry (ascii)      */
{
    BRI_SHELF_ENTRY_ITEM	    *vse ;

    vse = &BriShelfPtr(shelf_id)->shelf_entries[entry_id-1];

    if (entry_type)
	*entry_type = vse->entry_type;
    if (file_name)
	*file_name = vse->target_file;
    if (title)
	*title = vse->title;

    /*  THE FOLLOWING ARGS ARE NOT IMPLEMENTED (USED) YET  */

    if (width)
	*width = 0;
    if (height)
	*height = 0;
    if (data_len)
	*data_len = 0;
    if (data_addr)
	*data_addr = 0;
    if (data_type)
	*data_type = 0;

};	/*  end of bri_shelf_entry  */

/*
**  bri_shelf_openlib -- open the main library shelves.
**
**
**  Returns:  shelf id
*/

BMD_SHELF_ID
bri_shelf_openlib PARAM_NAMES((file_name,entry_count,default_title))
    char *file_name PARAM_SEP     /*  name of shelf file      	*/
    BR_INT_32 *entry_count PARAM_SEP    /*  number of entries	        */
    char *default_title PARAM_END  /*  default title for libraries  */
{
    BRI_CONTEXT *library;
    BRI_CONTEXT *shelf;
    jmp_buf error_return;
    BRI_SHELF_BLOCK *sfb;    /* Shelf block for shelf being opened. */
    int  max_entries,i,flag, extension, last_extension;
    int  count = 0;
    int tmp_name_size;
    char **tmp_name, *period_loc, *bracket_loc;
    Boolean found_match, already_in_list;

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
        BriError(library,BriErrShelfOpenNum);
        BriContextDelete(library);
        return NULL;
    }

    /* Set up the context for the book.  Note that the address of the
     * book context pointer is passed so that it can be set prior to
     * any errors being reported via BriLongJmp().  
     */
    BriContextNew(&library,file_name,BMD_C_SHELF,error_return);

    /* Search for the specified library files
     * use dynamic array for temporary name list
     */
    tmp_name_size = LIBRARY_ENTRY_ALLOC;
    tmp_name = (char *) BKR_MALLOC(tmp_name_size*sizeof(char *));
    tmp_name[0] = NULL;

    /* Get the pointer to the shelf block from the environment */
    sfb = BriShelfPtr(library);

    /* Allocate a vector for the shelf entry pointers */
    max_entries = LIBRARY_ENTRY_ALLOC;
    sfb->n_entries = 0;
    sfb->shelf_entries = BriCalloc(BRI_SHELF_ENTRY_ITEM,max_entries,library);

#ifdef VMS
    /* Get the last period location */       
    period_loc = strrchr (file_name, PERIOD);

    /* Get the last bracket location */
    bracket_loc = strrchr (file_name, RIGHT_SQUIGLLY);
    if (bracket_loc == NULL)
	bracket_loc = strrchr (file_name, RIGHT_BRACKET);

    /* If there are no brackets, or if last bracket is before last period,
       then a specific extension exists */
    if ((period_loc != NULL && bracket_loc == NULL) || bracket_loc < period_loc)
	last_extension = DEFAULT_EXTENSION;
    else
	last_extension = NFS_EXTENSION;
#else
    /* If period is in Ultrix file spec, assume specified extension */
    if (strchr (file_name, PERIOD) != NULL)
	last_extension = DEFAULT_EXTENSION;
    else
	last_extension = NFS_EXTENSION;
#endif

    /* Iterate as many as three times if no extension is present: the first
       time look for decw book extension, the second time look for ISO
       extension, the third time look for NFS */ 
    for (extension = DEFAULT_EXTENSION; extension <= last_extension; extension++) {

	/* Parse the file spec, looking first for decw book extension */
	BriFileParse (library, "", extension, SEARCHING);

	found_match = BriFileSearch (library);

	/* Loop on the search list */
	while (found_match) {

	    BRI_SHELF_ENTRY_ITEM *vse;

	    already_in_list = FALSE;

	    if( tmp_name[0] == NULL) {
		tmp_name[0] = (char *) BKR_MALLOC(strlen(library->found_file_spec)+ 1);
		strcpy(tmp_name[0],library->found_file_spec);
		count++;
	    }
	    else {
		i = 0;
		while ((i < count) && !already_in_list) {
		    if (strcmp(tmp_name[i],library->found_file_spec) == 0)
			already_in_list = TRUE;
		    i++;
		}

		if (!already_in_list) {
		    /* We got here so there is no match put it in the list.
		     * First, expand temporary name array, if necessary.
		     */
		    if (count >= tmp_name_size) {
			tmp_name_size += LIBRARY_ENTRY_ALLOC;
			tmp_name = (char *) BKR_REALLOC(tmp_name,
						tmp_name_size*sizeof(char *));
		    }
		    tmp_name[count] = (char *) BKR_MALLOC(strlen(library->found_file_spec)+ 1);
		    strcpy(tmp_name[count],library->found_file_spec);
		    count++;
		}
	    }

	    /* Open the library files found in the search list, and build
	     * top level libraries containing references to them. */
	    if (!already_in_list) 
		shelf = (BRI_CONTEXT *)bri_shelf_open_file (library->found_file_spec,
							NULL,
							NULL);
	    if ((shelf != NULL) && !already_in_list) {

		/* Make sure we have enough shelf entries */
		sfb->n_entries++;
		if (sfb->n_entries > max_entries) {
		    max_entries += LIBRARY_ENTRY_ALLOC;
		    sfb->shelf_entries = (BRI_SHELF_ENTRY_ITEM *)
                                  BriRealloc(sfb->shelf_entries,
                                             sizeof(BRI_SHELF_ENTRY_ITEM)*max_entries,
                                             library);
		}

		/* Set up the shelf entry for this shelf.  The entry type of
		 * course is shelf, and we get the file name from the shelf
		 * entry.  If the shelf file has a TITLE line in it then that
		 * will be in the shelf entry too.   Otherwise, we will
		 * generate a title string using the entry number */
		vse = &sfb->shelf_entries[sfb->n_entries - 1]; 
		vse->entry_type = BMD_C_SHELF ;
		BriStringAlloc(&vse->target_file,shelf->entry.target_file,library);
		BriStringAlloc(&vse->home_directory,shelf->entry.home_directory,library);
		if (shelf->entry.title != NULL) {
		    BriStringAlloc(&vse->title,shelf->entry.title,library);
		}
		else {
		    char buffer[255];
		    sprintf(buffer,"%s %d", default_title, sfb->n_entries);
		    BriStringAlloc(&vse->title,buffer,library);
		}

		/* Finished with the shelf for now, delete its context */
		BriContextDelete(shelf);
	    }

	    /* Get match for next iteration */
	    found_match = BriFileSearch (library);
	}
    }

    for(i=0;i<count;i++){
	if(tmp_name[i] != NULL) 
	    BKR_FREE(tmp_name[i]); /* bkr_free sets the string to nuLL also */
	}
    /* having freed names now free ptr array
     */
    BKR_FREE(tmp_name);

    if (sfb->n_entries == 0) {
        /* No library files were opened.  Make sure a message is displayed. */
	if (last_extension == DEFAULT_EXTENSION)
	    BriError(library,BriErrLibraryOpenNum);
	else
	    BriError(library,BriErrLibraryOpenAnyNum);

        if (entry_count != NULL) {
            *entry_count = 0;
        BriContextDelete(library);
        return NULL;
        }
    } else {

        /* Return the entry count if necessary.
         */
        if (entry_count != NULL) {
            *entry_count = sfb->n_entries;
        }
    }

    return (BMD_SHELF_ID)library;

};	/*  end of bri_shelf_openlib  */



