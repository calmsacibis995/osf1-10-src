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
**      Bookreader Memex Interface (bmi)
**
**  ABSTRACT:
**
**	Function prototypes for bmi_surrogate.c
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     18-Oct-1991
**
**  MODIFICATION HISTORY:
**
**--
**/
#ifndef BMI_SURROGATE_H
#define BMI_SURROGATE_H

#include "br_prototype.h"


/*
** Routines defined in bmi_surrogate.c
*/
extern void bmi_print_surrogate
    PROTOTYPE((lwk_surrogate surrogate));

extern lwk_status bmi_save_surrogate_in_list
    PROTOTYPE((BMI_SURROGATE_LIST_PTR *list_head,
               lwk_surrogate surrogate));

extern lwk_status bmi_save_surrogate_in_tree
    PROTOTYPE((BMI_SURROGATE_TREE_PTR *tree,
               lwk_surrogate surrogate,
               unsigned key));

extern BMI_SURROGATE_LIST_PTR bmi_find_surrogate_in_tree
    PROTOTYPE((BMI_SURROGATE_TREE_PTR tree,
               unsigned key));

extern void bmi_clear_surrogate_list
    PROTOTYPE((BMI_SURROGATE_LIST_PTR *list_array,
               unsigned n_elements));

extern void bmi_delete_surrogate_tree
    PROTOTYPE((BMI_SURROGATE_TREE_PTR *tree));

extern lwk_status bmi_create_surrogate
    PROTOTYPE((char *container,
               char *object_type,
               char *description,
               lwk_surrogate *surrogate_rtn));

extern lwk_surrogate bmi_find_existing_surrogate
    PROTOTYPE((BKR_WINDOW_PTR winctx,
               BMI_SURROGATE_LIST_PTR surrogate_list,
               lwk_status             *status_rtn));

extern lwk_status bmi_get_surrogates
    PROTOTYPE((BMI_SURROGATE_LIST *list,
               lwk_object *return_object));

#endif 



