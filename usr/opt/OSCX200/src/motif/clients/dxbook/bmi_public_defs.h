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
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_PUBLIC_DEFS.H*/
/* *3     3-MAR-1992 17:07:44 KARDON "UCXed"*/
/* *2     1-NOV-1991 12:48:16 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:47:54 PARMENTER "Public LinkWorks definitions"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_PUBLIC_DEFS.H*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_PUBLIC_DEFS.H*/
/* *5     1-MAY-1991 00:50:21 BALLENGER "Fix problems with links in the library window."*/
/* *4     2-MAR-1991 19:04:38 BALLENGER "Linkworks name changes and QAR807 fix"*/
/* *3    25-JAN-1991 16:47:08 FITZELL ""*/
/* *2    12-DEC-1990 12:29:10 FITZELL "v3 IFT update"*/
/* *1     8-NOV-1990 11:20:22 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_PUBLIC_DEFS.H*/
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
**      Bookreader Memex Interfaces Interface (bmi*)
**
**  ABSTRACT:
**
**	Private definitions for the Bookreader Memex Interface
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     02-Jul-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0001 DLB0001     David L Ballenger           30-Apr-1991
**           Fix problems with surrogates in the library window.
**
**--
**/


#ifndef BMI_PUBLIC_DEFS_H
#define BMI_PUBLIC_DEFS_H

#include "lwk_def.h"
#include "bmi_surrogate_defs.h"


typedef struct _BMI_SURROGATE_LIST {
    struct _BMI_SURROGATE_LIST *next;
    lwk_surrogate surrogate;
} BMI_SURROGATE_LIST, *BMI_SURROGATE_LIST_PTR;



typedef  void *BMI_MEMEX_HANDLE;

extern void bmi_save_book_menu();
extern void bmi_create_book_context();
extern void bmi_delete_book_context();
extern void bmi_delete_book_ui();

extern void bmi_create_directory_ui();
extern void bmi_update_dir_highlighting();

extern void bmi_create_topic_ui();
extern void bmi_update_chunk_highlighting();

extern void bmi_create_shelf_context();
extern void bmi_create_library_context();
extern void bmi_create_library_ui();
extern void bmi_delete_library_ui();
extern void bmi_save_library_menu();

#endif 
/* DONT ADD STUFF AFTER THIS #endif */

