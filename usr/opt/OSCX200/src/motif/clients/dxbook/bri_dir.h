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
**
**  FACILITY:
**
**   BRI -- Book Reader Interface
**
** ABSTRACT:
**
**   Prototypes for the public directory management routines.
**
** AUTHORS:
**
**   David L Ballenger
**
** CREATION DATE: 21-Oct-1991
**
** MODIFICATION HISTORY:
**
*/

#ifndef BRI_DIR_H
#define BRI_DIR_H
extern BMD_OBJECT_ID  	    bri_directory_entry
        PROTOTYPE((BMD_BOOK_ID bkid,
                   BMD_OBJECT_ID drid,
                   BR_UINT_32 entry_num,
                   BR_UINT_32 *num_targets,
                   BMD_OBJECT_ID *target_list,
                   BR_UINT_32 *level,
                   BR_UINT_32 *width,
                   BR_UINT_32 *height,
                   BMD_GENERIC_PTR *data_addr,
                   BR_UINT_32    *data_len,
                   BR_UINT_32    *data_type,
                   char	    **title));
extern char *               bri_directory_name
        PROTOTYPE((BMD_BOOK_ID bkid, 
                   BMD_OBJECT_ID drid));
extern BMD_OBJECT_ID 	    bri_directory_open
        PROTOTYPE((BMD_BOOK_ID bkid,
                   BMD_OBJECT_ID drid,
                   BR_UINT_32 *flags,
                   BR_UINT_32	*entry_count
                   ));
extern BMD_OBJECT_ID 	    bri_directory_next
        PROTOTYPE((BMD_BOOK_ID bkid, 
                   BMD_OBJECT_ID drid));
extern BMD_OBJECT_ID 	    bri_directory_find
        PROTOTYPE((BMD_BOOK_ID bkid,
                   char *dir_name));
extern BMD_OBJECT_TYPE	    bri_get_object_type
        PROTOTYPE((BMD_BOOK_ID bkid,
                   BMD_OBJECT_ID object_id));
#endif 










