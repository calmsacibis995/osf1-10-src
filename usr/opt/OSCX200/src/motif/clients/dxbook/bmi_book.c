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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BMI_BOOK.C*/
/* *7     4-MAR-1993 17:02:14 BALLENGER "Fix problem with not being able to close books through LinkWorks."*/
/* *6     9-JUN-1992 10:00:10 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *5     6-MAY-1992 13:55:37 BALLENGER "Fix type mismatches."*/
/* *4    13-APR-1992 14:11:48 BALLENGER " Fix problem with hang when following link opens book in default window."*/
/* *3     3-MAR-1992 17:06:49 KARDON "UCXed"*/
/* *2     1-NOV-1991 12:47:51 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:41:08 PARMENTER "Linkworks Interfaces"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BMI_BOOK.C*/
#ifndef VMS
 /*
#else
#module BMI_BOOK "V03-0005"
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
*/


/*
**++
**  FACILITY:
**
**      Bookreader Memex Interfaces (bmi*)
**
**  ABSTRACT:
**
**	Interfaces to the hyperinformation services for dealing
**      with book contexts.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     ??-Jul-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0005    DLB0003     David L Ballenger           30-Apr-1991
**              Fix problems with opening directories.
**
**  V03-0004    DLB0003     David L Ballenger           01-Mar-1991
**              Fix problems with surrogate highlighting after the composite
**              network changes for QAR 807.  LinkWorks naming
**              convention changes.
**
**  V03-0003	DLB0002		David L Ballenger	18-Feb-1991
**		When operation is "Close", close book even if the selection
**		shell is ACTIVE, i.e. the Navigation window is active.
**
**      DLB0001     David L Ballenger           30-Jan-1991
**                  Add support for new HyperHelp related operations:
**                  "View In New Window", "View in Default Window", and
**                  "Close".
**
**	JAF0001	    James A. Ferguson   	26-Jul-1990
**  	    	    Update bmi_book_open_to_target to support new
**  	    	    SVN Selection window in BL5.
**
**--
**/


#include "bmi_private_defs.h"
#include "br_common_defs.h"
#include "bods_private_def.h"
#include "br_typedefs.h"
#include "br_globals.h"
#include "bkr_book.h"
#include "bkr_shell.h"
#include "bkr_selection.h"
#include "bmi_surrogate_defs.h"
#include "bmi_book.h"
#include "bmi_surrogate.h"
#include "bmi_user_interface.h"

lwk_status
bmi_check_book_timestamp PARAM_NAMES((surrogate,book))
    lwk_surrogate surrogate PARAM_SEP
    BKR_BOOK_CTX_PTR book PARAM_END

/*
 *
 * Function description:
 *
 *     Routine to see if the timestamp in some species of book surrogate
 *     object matches that of the book in our book context.
 *
 * Arguments:
 *
 *      surrogate - the surrogate object
 *
 *      book - the BMI book context tocheck
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
    lwk_status status;
    unsigned timestamp[2];

    status = GET_INTEGER(surrogate,BMI_PROP_TIMESTAMP0,&timestamp[0]);
    RETURN_ON_ERROR(status);

    status = GET_INTEGER(surrogate,BMI_PROP_TIMESTAMP1,&timestamp[1]);
    RETURN_ON_ERROR(status);

    if ((timestamp[0] != book->timestamp[0]) 
        || (timestamp[1] != book->timestamp[1])
        ) {
        return lwk_s_failure;
    }
    return lwk_s_success;

} /* end bmi_check_book_timestamp () */



lwk_status
bmi_create_book_surrogate PARAM_NAMES((book,object_type,description,surrogate_rtn))
    BKR_BOOK_CTX_PTR book PARAM_SEP
    char *object_type PARAM_SEP
    char *description PARAM_SEP
    lwk_surrogate *surrogate_rtn PARAM_END

/*
 *
 * Function description:
 *
 *     Create a basic book surrogate of the specified type. 
 *
 * Arguments:
 *
 *      book - the book context where theroutine gets some of the info
 *
 *      object_type - name of the object type
 *
 *      description - value for the description property
 *
 *      surrogate_rtn - return parameter for the created surrogate.
 *
 * Return value:
 *
 *      Returns lwk_s_success if the surrogate is successfull created.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    lwk_status status;
    lwk_surrogate surrogate;

    /* Create a basic bookreader surrogate object.
     */
    status = bmi_create_surrogate(book->filename,
                                  object_type,
                                  description,
                                  &surrogate
                                  );
    RETURN_ON_ERROR(status);

    /* Now add properties common to all book (i.e. directory, chunk)
     * objects.
     */
    status = SET_INTEGER(surrogate,BMI_PROP_VERSION,&book->version);
    DELETE_AND_RETURN_ON_ERROR(status,surrogate);

    /* Don't add a timestamp property for pre V2 books, 
     * they didn't have them.
     */
    if (book->version.major_num >= 2) {
        status = SET_INTEGER(surrogate,BMI_PROP_TIMESTAMP0,&book->timestamp[0]);
        DELETE_AND_RETURN_ON_ERROR(status,surrogate);
        
        status = SET_INTEGER(surrogate,BMI_PROP_TIMESTAMP1,&book->timestamp[1]);
        DELETE_AND_RETURN_ON_ERROR(status,surrogate);

    }
    /* Return the created surrogate.
     */
    *surrogate_rtn = surrogate;
    return lwk_s_success;

} /* end bmi_create_book_surrogate () */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bmi_book_open_to_target
**
** 	Opens a book in a Selection window by filename, and opens the
**  	Topic window to the given chunk if non-zero otherwise opens to
**  	copyright chunk or first page in book.
**
**  FORMAL PARAMETERS:
**
**  	filename    	 - string name of book to open.
**  	object_type      - the type of the object to open
**      surrogate        - the surrogate describing the object
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    status of the open book operation.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
lwk_status
bmi_book_open_to_target PARAM_NAMES((filename,object_type,surrogate,operation))
    char    	  *filename PARAM_SEP
    int           object_type PARAM_SEP
    lwk_surrogate surrogate PARAM_SEP
    lwk_string    operation PARAM_END

{
    BKR_SHELL_PTR	    shell;
    BKR_BOOK_CTX_PTR        book;
    Boolean                 open_in_new;
    lwk_status      	    status;

    TRACE("bmi_open_to_target");
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Operation = %s, book = %s\n",operation,filename);
#endif 

    book = bkr_book_get(filename,NULL,0);

    if (book == NULL) 
    {
        return lwk_s_failure;
    }    

    
    if (strcmp(operation,BMI_OPERATION_CLOSE) == 0) 
    {
        BKR_SHELL_PTR next_shell;

        shell = book->shells;
        while (shell) 
        {
            next_shell = shell->book_shells;
            if (shell->client == NULL) 
            {
                bkr_close_shell(shell,FALSE);
            }
            shell = next_shell;
        }
        return lwk_s_success;
    }
    
    open_in_new = (strcmp(operation,BMI_OPERATION_VIEW_DEFAULT) != 0);
    
    shell = bkr_shell_get(book,NULL,open_in_new);
    if (shell == NULL) {
        return lwk_s_failure;
    }

    if ((object_type == BMI_OBJ_BOOK) || (object_type == BMI_OBJ_DIRECTORY)) 
    {
        
        lwk_string  	    dir_name;
        lwk_integer 	    dir_entry_num = 0;
        BMD_OBJECT_ID       dir_id = (BMD_OBJECT_ID)0;
        BKR_DIR_ENTRY       *dir_entry = NULL;
        
        /* See if the surrogate describes a directory and may specify the
         * entry. We're allowing both book and directory objects to specify
         * directory name and directory entry properties
         */

        status = GET_STRING(surrogate,BMI_PROP_DIR_NAME,&dir_name);
        if (status == lwk_s_success)
        {
            /*  Open the directory by name
             */
            dir_id = bri_directory_find(book->book_id,dir_name);
            if (dir_id) 
            {
                /* Get the specific directory entry number.
                 */
                status = GET_INTEGER(surrogate,BMI_PROP_DIR_ENTRY,&dir_entry_num);
                if (status == lwk_s_no_such_property) 
                {
                    dir_entry_num = 0;
                    status = lwk_s_success;
                }
            } 
            else 
            {
                dir_id = book->default_directory->object_id;
            }
        } else if (status == lwk_s_no_such_property) {
            dir_id = book->default_directory->object_id;
            status = lwk_s_success;
        }

        /* From here on treat book and directory objects the same.
         */
        if (status == lwk_s_success) {
            BMD_OBJECT_ID  object_id = (dir_id | dir_entry_num);
            bkr_object_id_dispatch(shell,NULL,object_id);
            dir_entry = bkr_selection_find_entry_by_id(book->directories,object_id);
            if (dir_entry && bmi_context.select_destination) 
            {
                /* Select the directory entry if "select destination" is turned
                 * on in the LinkWorks environment.
                 */
                bmi_select_svn_entry(shell->selection,(unsigned)dir_entry);
            }
        }
    } else if (object_type == BMI_OBJ_CHUNK) {

    	BMD_OBJECT_ID object_id = 0;
        unsigned target_id = 0;
        char *target_symbol;
        BKR_WINDOW *window_to_use = (open_in_new) ? NULL : shell->default_topic;

        /* If the timestamp matches then use the chunk id property as
         * the target chunk.
         */
        status = bmi_check_book_timestamp(surrogate,book);
        if (status == lwk_s_success) {
            status = GET_INTEGER(surrogate,BMI_PROP_CHUNK_ID,&target_id);
            if (status != lwk_s_success) {
                target_id = 0;
            }
        }

        if (target_id == 0) 
        {
            /* Haven't found a target chunk yet.  Use the chunk symbol
             * if it's there and in the symbol table.  
             */
            status = GET_STRING(surrogate,BMI_PROP_CHUNK_NAME,&target_symbol);
            if (status == lwk_s_success) 
            {
                object_id = bri_symbol_to_chunk(book->book_id,target_symbol);
            }
        } 
        else 
        {
            object_id = target_id;
        }

    	if (object_id == 0) 
        {
            /* Have not found an object_id just open to the first page.
             */
            object_id = BODS_TOPIC_DIR_ID;
        }
        bkr_object_id_dispatch(shell,window_to_use,object_id);

    } /* end if object_type */

    return lwk_s_success;

};  /* end of bmi_book_open_to_target */



