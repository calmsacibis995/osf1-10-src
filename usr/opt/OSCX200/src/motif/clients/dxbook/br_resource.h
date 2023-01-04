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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_RESOURCE.H*/
/* *6    30-OCT-1992 18:33:28 BALLENGER "Add CC specific resources, and resources for DECW*BOOK*"*/
/* *5     3-AUG-1992 18:43:15 BALLENGER "Add support for character cell specific resources."*/
/* *4    28-JUL-1992 14:35:06 BALLENGER "Character cell specific resources."*/
/* *3    19-JUN-1992 20:14:02 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *2     3-MAR-1992 17:12:34 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:48:44 PARMENTER "Xrm Resource definitions"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_RESOURCE.H*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_RESOURCE.H*/
/* *3    25-JAN-1991 16:42:52 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:05:01 FITZELL "V3 IFT Update snapshot"*/
/* *1     8-NOV-1990 11:15:24 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_RESOURCE.H*/

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
**	Header file for Bookreader Xrm customizable resource names.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     16-Apr-1990
**
**  MODIFICATION HISTORY:
**
**--
**/

#ifndef _BKR_RESOURCE_H
#define _BKR_RESOURCE_H


/*
 *  Resource definitions
 */

#define	    bkrNconfirm_close	    	    "confirm_close"
#define	    bkrCConfirm_Close	    	    "Confirm_Close"

#define	    bkrNfirst_directory_to_open	    "first_directory_to_open"
#define	    bkrCFirst_Directory_To_Open	    "First_Directory_To_Open"
#define	    bkrRFirst_Directory_To_Open	    "First_Directory_To_Open"

#define	    bkrNmax_default_topic_width	    "max_default_topic_width"
#define	    bkrCMax_Default_Topic_Width	    "Max_Default_Topic_Width"

#define	    bkrNmax_default_topic_height    "max_default_topic_height"
#define	    bkrCMax_Default_Topic_Height    "Max_Default_Topic_Height"

#define	    bkrNshelf_to_open_on_startup    "shelf_to_open_on_startup"
#define	    bkrCShelf_To_Open_On_Startup    "Shelf_To_Open_On_Startup"
#define	    bkrRShelf_To_Open_On_Startup    "Shelf_To_Open_On_Startup"

#define	    bkrNshow_hot_spots	    	    "show_hot_spots"
#define	    bkrCShow_Hot_Spots	    	    "Show_Hot_Spots"

#define	    bkrNcshow_hot_spots	   	    "cshow_hot_spots"
#define	    bkrCCShow_Hot_Spots	            "CShow_Hot_Spots"

#define	    bkrNshow_extensions	    	    "show_extensions"
#define	    bkrCShow_Extensions	    	    "Show_Extensions"

#define	    bkrNmm_width    	    	    "mm_width"
#define	    bkrCMM_Width    	    	    "MM_Width"

#define	    bkrNmm_height    	    	    "mm_height"
#define	    bkrCMM_Height    	    	    "MM_Height"

#define	    bkrNx_offset    	    	    "x_offset"
#define	    bkrCX_Offset    	    	    "X_Offset"

#define	    bkrNy_offset    	    	    "y_offset"
#define	    bkrCY_Offset    	    	    "Y_Offset"

#define	    bkrNcx_offset    	    	    "cx_offset"
#define	    bkrCCX_Offset    	    	    "CX_Offset"

#define	    bkrNcy_offset    	    	    "cy_offset"
#define	    bkrCCY_Offset    	    	    "CY_Offset"

#define	    bkrNcinitialState	    	    "cinitialState"
#define	    bkrCCInitialState	    	    "CInitialState"

#define	    bkrNsearchList	    	    "searchList"
#define	    bkrCSearchList	    	    "SearchList"

#define	    bkrNdefaultLibraryName    	    "defaultLibraryName"
#define	    bkrCDefaultLibraryName  	    "DefaultLibraryName"

/*
 *  first_directory_to_open literals
 */

#define	    TOC_DIR 	    1
#define	    INDEX_DIR	    2
#define	    DEFAULT_DIR	    3
#define	    NO_DIRECTORY    4

/*
 *  shelf_to_open_on_startup literals
 */

#define	    NO_SHELF	    1
#define	    FIRST_SHELF	    2
#define	    ALL_SHELVES	    3



typedef struct 
    {
    	Dimension   	    	width;
    	Dimension   	    	height;
    	Dimension   	    	mm_width;
    	Dimension   	    	mm_height;
    	unsigned char	    	shelf_to_open;
    	Boolean	    	    	confirm_close;
        int                     initial_state;
        String                  searchList;
        String                  defaultLibraryName;


    }   BKR_LIBRARY_RESOURCES, *BKR_LIBRARY_RESOURCES_PTR;

typedef struct 
    {
    	Dimension   	    	width;
    	Dimension   	    	height;
    	Dimension   	    	mm_width;
    	Dimension   	    	mm_height;
    	Position    	    	x_offset;
    	Position    	    	y_offset;
    	unsigned char	    	first_directory_to_open;
    }   BKR_RESOURCES, *BKR_RESOURCES_PTR;

typedef struct 
    {
    	Dimension   	    	max_default_topic_width;
    	Dimension   	    	max_default_topic_height;
    	Dimension   	    	width;
    	Dimension   	    	height;
    	Dimension   	    	mm_width;
    	Dimension   	    	mm_height;
    	Position    	    	x_offset;
    	Position    	    	y_offset;
    	Boolean	    	    	show_hot_spots;
    	Boolean	    	    	show_extensions;

    }   BKR_TOPIC_RESOURCES, *BKR_TOPIC_RESOURCES_PTR;

#endif /* _BKR_RESOURCE_H */

/* DON'T ADD STUFF AFTER THIS #endif */
