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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SHELL.C*/
/* *9    17-JUN-1992 20:33:47 BALLENGER "Include br_common_defs.h"*/
/* *8     9-JUN-1992 09:58:56 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *7    10-MAY-1992 23:55:26 BALLENGER "fix bug with default_shell and all_shells list"*/
/* *6    15-APR-1992 17:01:02 ROSE "Make sure bkr_all_shells is non-NULL when a shell is re-used"*/
/* *5    13-APR-1992 14:11:43 BALLENGER " Fix problem with hang when following link opens book in default window."*/
/* *4    13-APR-1992 14:07:05 BALLENGER "Fix problem with hang when following link opens book in default window."*/
/* *3     3-MAR-1992 17:04:33 KARDON "UCXed"*/
/* *2    18-SEP-1991 18:07:52 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:40:39 PARMENTER "Shell manipulation"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SHELL.C*/
#ifndef VMS
 /*
#else
#module BKR_SHELL "V03-0001"
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
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Shell manipulation routines.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     01-Jul-1991
**
**  MODIFICATION HISTORY:
**
**--
**/


/*
 * INCLUDE FILES
 */
#include "br_common_defs.h"
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_shell.h"       /* function prototypes for .c module */
#include "bkr_book.h"        /* Book access routines */
#include "bkr_close.h"      

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_shell_get
**
** 	Routine to get a shell for use with a particular book and client.
**
**  FORMAL PARAMETERS:
**
**	book             - the book context that that will own the shell.
**      client           - the client context that will own the shell.
**      create_new_shell - if FALSE reuse and existing shell for this
**                         book/client, otherwise create a new shell
**
**  IMPLICIT INPUTS:
**
**	None
**
**  IMPLICIT OUTPUTS:
**
**	None
**
**  COMPLETION CODES:
**
**	Returns a pointer to the shell or NULL if not successful.
**
**  SIDE EFFECTS:
**
**	Memory allocation
**
**--
**/
BKR_SHELL *
bkr_shell_get PARAM_NAMES((book,client,create_new_shell))
    BKR_BOOK_CTX *book PARAM_SEP
    BKR_CLIENT *client PARAM_SEP
    Boolean create_new_shell PARAM_END
{
    BKR_SHELL *shell = NULL;

    if (create_new_shell == FALSE) {

        /* Try to reuse an existing shell.
         */
        if (client) {

            shell = client->shells;
            while (shell && (shell->book != book)) {
                shell = shell->client_shells;
            }
        } else {

            /* No client was specified so this is just being used by
             * Bookreader itself use the default shell.
             */
            shell = bkr_default_shell;

            /* Put it back on the list of all shells if it has been
             * freed.
             */
            if (shell)
            {
                if (shell->n_active_windows <= 0)
                {
                    /* Now put it back on the list of all shells.
                     */
                    shell->all_shells = bkr_all_shells;
                    bkr_all_shells = shell;
                }
                else if (shell->book != book) 
                {
                    /* If the default shell is in use and it's not the
                     * same book, then close it.
                     */
                    bkr_close_shell(shell,TRUE);
                }
            }
        }
    }
    if (shell == NULL) {
        shell = (BKR_SHELL *)BKR_MALLOC(sizeof(BKR_SHELL));
        if (shell) {
            memset(shell,0,sizeof(BKR_SHELL));
            if (client) {
                shell->client = client;
                shell->client_shells = client->shells;
                client->shells = shell;
            } else if (bkr_default_shell == NULL) {
                bkr_default_shell = shell;
            }
            shell->all_shells = bkr_all_shells;
            bkr_all_shells = shell;
        } else {
            return NULL;
        }
    }

    if (shell->book == NULL) {
        shell->book = book;
        shell->book_shells = book->shells;
        book->shells = shell;
    }

    /* Make sure the global points at the only shell in existence if
       it is not pointing at anything */
    if (bkr_all_shells == NULL)
	bkr_all_shells = shell;

    return shell;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_shell_free
**
** 	Frees a BKR_SHELL structure if it is not the default shell or
**      in use, i.e. has windows.
**
**  FORMAL PARAMETERS:
**
**	shell - pointer to the shell to free.
**
**  IMPLICIT INPUTS:
**
**	bkr_default_shell
**
**      bkr_client_context
**
**  IMPLICIT OUTPUTS:
**
**	bkr_client_context
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	May free memory.
**
**--
**/
void
bkr_shell_free PARAM_NAMES((shell,release_client))
    BKR_SHELL *shell PARAM_SEP
    Boolean release_client PARAM_END
{
    BKR_SHELL_PTR *ptr_to_ptr;
    BKR_SHELL_PTR ptr;

    /* Preliminary test to see if this shell can be freed. Unless it is
     * the default shell, it can't have any windows and even then then
     * the default shells windows can't be active.
     */
    if ((shell == NULL) 
        || ((shell != bkr_default_shell)
            && (shell->selection
                || shell->default_topic
                || shell->other_topics
                )
            )
        || (shell->n_active_windows > 0)
        ) {
        return;
    }

    /* If this shell belongs to a client, then we can only free the
     * shell if we can release it's client connection.
     */
    if (shell->client) {

        if (release_client) {
            /* Remove from the list of all shells for the client
             */
            ptr_to_ptr = &shell->client->shells;
            while (*ptr_to_ptr) {
                ptr = *ptr_to_ptr;
                if (ptr == shell) {
                    *ptr_to_ptr = shell->client_shells;
                    break;
                }
                ptr_to_ptr = &ptr->client_shells;
            }
            shell->client == NULL;

        } else {
            /* We can't do anything else if this shell still belongs
             * to a client.
             */
            return;
        }
    }
        
    /* Remove from the list of all shells.
     */
    ptr_to_ptr = &bkr_all_shells;
    while (*ptr_to_ptr) {
        ptr = *ptr_to_ptr;
        if (ptr == shell) {
            *ptr_to_ptr = shell->all_shells;
            break;
        }
        ptr_to_ptr = &ptr->all_shells;
    }
    
    if (shell->book) {
        /* Remove from the list of all shells for the book
         */
        ptr_to_ptr = &shell->book->shells;
        while (*ptr_to_ptr) {
            ptr = *ptr_to_ptr;
            if (ptr == shell) {
                *ptr_to_ptr = shell->book_shells;
                break;
            }
            ptr_to_ptr = &ptr->book_shells;
        }
        bkr_book_free(shell->book);
    }
    
    if (shell->library_node) {
        /* Remove from the list of all shells for the library_node
         */
        ptr_to_ptr = &shell->library_node->u.book.shells;
        while (*ptr_to_ptr) {
            ptr = *ptr_to_ptr;
            if (ptr == shell) {
                *ptr_to_ptr = shell->library_shells;
                break;
            }
            ptr_to_ptr = &ptr->library_shells;
        }
    }
    
    if (shell == bkr_default_shell) {
        
        /* Make sure the default shell doesn't point to any of the
         * lists.
         */
        shell->all_shells = NULL;
        shell->book = NULL;
        shell->library_node = NULL;
    } else {
        
        BKR_FREE(shell);
    }
}


