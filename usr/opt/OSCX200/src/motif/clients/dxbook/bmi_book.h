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
**  CREATION DATE:     18-Oct-1991
**
**  MODIFICATION HISTORY:
**
**--
**/

#ifndef BMI_BOOK_H
#define BMI_BOOK_H
#include "br_prototype.h"

extern lwk_status bmi_check_book_timestamp
    PROTOTYPE((lwk_surrogate surrogate,
               BKR_BOOK_CTX_PTR book));

extern lwk_status bmi_create_book_surrogate
    PROTOTYPE((BKR_BOOK_CTX_PTR book,
               char *object_type,
               char *description,
               lwk_surrogate *surrogate_rtn));

extern lwk_status bmi_book_open_to_target
    PROTOTYPE((char    	     *filename,
               int           object_type,
               lwk_surrogate surrogate,
               lwk_string    operation));
#endif 
