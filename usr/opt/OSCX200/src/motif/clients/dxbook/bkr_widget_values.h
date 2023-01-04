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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_WIDGET_VALUES.H*/
/* *12    5-OCT-1992 11:30:17 KLUM "rename print widget id constants"*/
/* *11   24-SEP-1992 17:07:02 KLUM "integrate CBR results widget into Bookreader"*/
/* *10   10-JUL-1992 17:11:04 ROSE "Renamed Print literals"*/
/* *9    16-JUN-1992 15:09:19 ROSE "Added 2 new literals for Search message widgets"*/
/* *8    11-JUN-1992 15:52:31 ROSE "Added literal to save abort_search button ID"*/
/* *7     5-MAR-1992 14:26:45 PARMENTER "adding simple search"*/
/* *6     3-MAR-1992 17:06:18 KARDON "UCXed"*/
/* *5     6-FEB-1992 09:38:06 KLUM "edit menu"*/
/* *4     6-JAN-1992 16:45:51 PARMENTER "added CBR stuff"*/
/* *3     2-JAN-1992 10:34:02 KLUM "BR Print - new constants"*/
/* *2    13-NOV-1991 14:44:54 KLUM "Green devel work"*/
/* *1    16-SEP-1991 12:47:36 PARMENTER "Widget Indexes, mirrors bkr_widget_values.uil"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_WIDGET_VALUES.H*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_WIDGET_VALUES.H*/
/* *4     7-MAY-1991 16:35:38 ACKERMAN "Added screen button widget"*/
/* *3    25-JAN-1991 16:44:26 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:06:28 FITZELL "V3 IFT Update snapshot"*/
/* *1     8-NOV-1990 11:16:17 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_WIDGET_VALUES.H*/

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
**      Bookreader User Interface (BKR)
**
**  ABSTRACT:
**
**	Literals used for indexing into the array of widgets used in the
**  	user interface.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     1-Nov-1989
**
**  MODIFICATION HISTORY:
**
**--
**/

#ifndef _BKR_WIDGET_VALUES_H
#define _BKR_WIDGET_VALUES_H


/*
 * NOTE: These values MUST match the UIL values exactly
 */

/*
 * The following constants are shared bewteen all 4 types of windows:
 * ie, LIBRARY, SELECTION, and both STANDARD and FORMAL Topic.
 */


#define W_MAIN_WINDOW   	    	1
#define W_MENU_BAR  	    	    	2
#define W_FILE_PULLDOWN_ENTRY   	3
#define W_FILE_MENU  	    	    	4
#define W_EDIT_PULLDOWN_ENTRY   	5
#define W_EDIT_MENU  	    	    	6
#define W_VIEW_PULLDOWN_ENTRY  	    	7
#define W_VIEW_MENU     	    	8

#define W_SEARCH_PULLDOWN_ENTRY  	9
#define W_SEARCH_MENU     	    	10

#define W_OPEN_TOPIC_IN_DEFAULT 	11  /* used by all but LIBRARY window */
#define W_BOOK_POPUP	    	    	11  /* ONLY used by LIBRARY window */

#define W_OPEN_TOPIC_IN_NEW	    	12  /* used by all but LIBRARY window */
#define W_SHELF_POPUP	    	    	12  /* ONLY used by LIBRARY window */

/* SEARCH */
#define W_SEARCH_EVERYTHING_ENTRY       13
#define W_SEARCH_LIBRARY_WINDOW_ENTRY   14
#define W_SEARCH_CONCEPT_LIST_ENTRY     15
#define W_SEARCH_EDIT_CONCEPT_ENTRY     16
#define W_SEARCH_BOOK_ENTRY             17
#define W_SEARCH_SELECTION_WINDOW_ENTRY 18
#define W_SEARCH_NEXT_ENTRY             19
#define W_SEARCH_PREVIOUS_ENTRY         20

#define W_SEARCH_BOX                    21
#define W_SEARCH_BOX_TEXT               22
#define W_SEARCH_BOX_BUTTON_BOX         23
#define W_SEARCH_BOX_OK                 24
#define W_SEARCH_BOX_CANCEL             25
#define W_SEARCH_BOX_HELP               26
#define W_SEARCH_RESULTS_BOX            27
#define W_SEARCH_TOPICS_ENTRY           28
#define W_SEARCH_BOOKS_ENTRY            29
#define W_SEARCH_LIBRARY_ENTRY          30
#define W_SEARCH_BOX_ABORT              31
#define W_SEARCH_MESSAGE_BOX            32
#define W_SEARCH_MESSAGE_LIST           33
#define W_SEARCH_R_QUERYLIST            34
#define W_SEARCH_R_HEADER               35
#define W_SEARCH_R_LIST                 36
#define W_SEARCH_R_GOTO_BUTTON          37
#define W_SEARCH_R_VISIT_BUTTON         38
#define W_SEARCH_R_CANCEL_BUTTON        39
#define W_SEARCH_R_HELP_BUTTON          40
#define W_SEARCH_R_BUTTON_BOX           41
#define W_SEARCH_R_DUMMY1               42
#define W_SEARCH_R_DUMMY2               43

/* end SEARCH */




#define W_PR_PRBOOK_BUTTON              44
#define W_PR_PRTOPIC_BUTTON             45
#define W_PR_PRINTTOPIC_BUTTON          46
#define W_PR_PRINTPOPUP_BUTTON          47
#define W_PR_F_NAME_LABEL               48
#define W_PR_F_NAME_TEXT                49
#define W_PR_LINKPS_TB                  50
#define W_PR_CONVERTPS_TB               51
#define W_PR_TEXT_TB                    52
#define W_PR_PRTS_RC                    53
#define W_PR_SEND_TO_PRINTER_TB         54
#define W_PR_SAVE_TO_FILE_TB            55
#define W_PR_BOTH_TB                    56
#define W_PR_APPENDTOFILE_TB            57
#define W_PR_BOX                        58
#define W_PR_OPTIONS_BOX                59
#define W_PR_0                          60
#define W_PR_1                          61
#define W_PR_2                          62
#define W_PR_3                          63
#define W_PR_4                          64
#define W_PR_5                          65
#define W_PR_6                          66
#define W_PR_7                          67
#define W_PR_8                          68
#define W_PR_9                          69

#define K_MAX_SHARED_WIDGETS    	69



    /*
     *   Widget ids shared by LIBRARY and SELECTION windows.
     */

#define K_START_SVN_SHARED_WIDGETS  	K_MAX_SHARED_WIDGETS

#define W_SVN   	    	    	( K_START_SVN_SHARED_WIDGETS + 1 )
#define W_EXPAND_ENTRY  	    	( K_START_SVN_SHARED_WIDGETS + 2 )
#define W_EXPAND_ALL_ENTRY  	    	( K_START_SVN_SHARED_WIDGETS + 3 )
#define W_COLLAPSE_ENTRY	    	( K_START_SVN_SHARED_WIDGETS + 4 )
#define W_COLLAPSE_ALL_ENTRY        	( K_START_SVN_SHARED_WIDGETS + 5 )
#define K_MAX_SVN_SHARED_WIDGETS    	5


    /*
     *  LIBRARY window widget ids
     *
     * NOTE: Add all NEW widgets specific to the LIBRARY window to the end
     *       of this list.
     */

#define K_START_LIBRARY_WIDGETS	    	( K_MAX_SHARED_WIDGETS + \
    	    	    	    	    	    K_MAX_SVN_SHARED_WIDGETS )

#define W_OPEN_LIBRARY_ENTRY	    	( K_START_LIBRARY_WIDGETS + 1 )
#define W_OPEN_BOOK_IN_DEFAULT	    	( K_START_LIBRARY_WIDGETS + 2 )
#define W_OPEN_BOOK_IN_NEW  	    	( K_START_LIBRARY_WIDGETS + 3 )
#define W_QUIT_ENTRY	    	    	( K_START_LIBRARY_WIDGETS + 4 )
#define W_OPEN_IN_DEFAULT_POPUP_ENTRY	( K_START_LIBRARY_WIDGETS + 5 )
#define W_OPEN_IN_NEW_POPUP_ENTRY   	( K_START_LIBRARY_WIDGETS + 6 )
#define W_OPEN_SHELF_POPUP_ENTRY    	( K_START_LIBRARY_WIDGETS + 7 )
#define W_CLOSE_SHELF_POPUP_ENTRY   	( K_START_LIBRARY_WIDGETS + 8 )
#define W_OPEN_DEFAULT_LIBRARY_ENTRY	( K_START_LIBRARY_WIDGETS + 9 )
#define K_NONSHARED_LIBRARY_WIDGETS 	9
    	/* end of LIBRARY window widgets */

#define K_MAX_LIBRARY_WIDGETS	    	( K_MAX_SHARED_WIDGETS 	     + \
    	    	    	    	    	    K_MAX_SVN_SHARED_WIDGETS + \
    	    	    	    	    	    K_NONSHARED_LIBRARY_WIDGETS )


    /*
     * Widget ids used ONLY in the SELECTION window 
     *
     * NOTE: Add all NEW widgets specific to the SELECTION window to the end
     *       of this list.
     */

#define K_START_SELECTION_WIDGETS   	( K_MAX_SHARED_WIDGETS	+ \
    	    	    	    	    	    K_MAX_SVN_SHARED_WIDGETS )

#define W_CLOSE_BOOK_ENTRY  	    	( K_START_SELECTION_WIDGETS + 1 )
#define K_NONSHARED_SELECTION_WIDGETS	( 1 + 1 )   /* +1 for 1-based array */
    	/* end of SELECTION window widgets */

#define K_MAX_SELECTION_WIDGETS 	( K_MAX_SHARED_WIDGETS	     + \
    	    	    	    	    	    K_MAX_SVN_SHARED_WIDGETS + \
    	    	    	    	    	    K_NONSHARED_SELECTION_WIDGETS )


    /*
     * Widget ids SHARED by the STANDARD and FORMAL Topic windows.
     *
     * NOTE: ONLY add widget to this list which are used by BOTH 
     *	     the STANDARD and FORMAL Topic windows.
     */

#define K_START_TOPIC_SHARED	    	K_MAX_SHARED_WIDGETS

#define W_WORK_AREA     	    	( K_START_TOPIC_SHARED + 1 )
#define W_LABEL     	    	    	( K_START_TOPIC_SHARED + 2 )
#define W_SCROLLED_WINDOW   	    	( K_START_TOPIC_SHARED + 3 )
#define W_WINDOW    	    	    	( K_START_TOPIC_SHARED + 4 )
#define W_VSCROLL_BAR   	    	( K_START_TOPIC_SHARED + 5 )
#define W_HSCROLL_BAR   	    	( K_START_TOPIC_SHARED + 6 )
#define W_CLOSE_TOPIC_ENTRY	    	( K_START_TOPIC_SHARED + 7 )
#define W_HOTSPOTS_ENTRY	    	( K_START_TOPIC_SHARED + 8 )
#define W_EXTENSIONS_ENTRY	    	( K_START_TOPIC_SHARED + 9 )
#define W_OPEN_DEFAULT_DIR_ENTRY 	( K_START_TOPIC_SHARED + 10 )
#define K_MAX_SHARED_TOPIC	    	10


    /*
     * Widget ids used ONLY by the FORMAL Topic window.
     *
     * NOTE: Add all NEW widgets specific to the FORMAL Topic window to 
     *	     the end of this list.
     */

#define K_START_FORMAL_TOPIC        	( K_MAX_SHARED_WIDGETS	+ \
    	    	    	    	    	    K_MAX_SHARED_TOPIC )
    /* NO SPECIFIC WIDGETS */

#define K_NONSHARED_FORMAL_TOPIC    	( 0 + 1 ) /* +1 for 1-based array */
    	/* end of FORMAL Topic window widgets */

#define K_MAX_FORMAL_TOPIC_WIDGETS  	( K_MAX_SHARED_WIDGETS	   + \
    	    	    	    	    	    K_MAX_SHARED_TOPIC 	   + \
    	    	    	    	    	    K_NONSHARED_FORMAL_TOPIC )



    /*
     * Widget ids used ONLY by the STANDARD Topic window.
     *
     * NOTE: Add all NEW widgets specific to the STANDARD Topic window 
     * 	     to the end of this list.
     */

#define K_START_STANDARD_TOPIC	    	( K_MAX_SHARED_WIDGETS 	 + \
    	    	    	    	    	    K_MAX_SHARED_TOPIC )

#define W_PREV_TOPIC_ENTRY  	    	( K_START_STANDARD_TOPIC + 1 )
#define W_NEXT_TOPIC_ENTRY  	    	( K_START_STANDARD_TOPIC + 2 )
#define W_GOBACK_ENTRY	    	    	( K_START_STANDARD_TOPIC + 3 )
#define W_BUTTON_BOX	    	    	( K_START_STANDARD_TOPIC + 4 )
#define W_GOBACK_BUTTON	    	    	( K_START_STANDARD_TOPIC + 5 )
#define W_PREV_TOPIC_BUTTON 	    	( K_START_STANDARD_TOPIC + 6 )
#define W_NEXT_TOPIC_BUTTON 	    	( K_START_STANDARD_TOPIC + 7 )
#define W_PREV_SCREEN_BUTTON		( K_START_STANDARD_TOPIC + 8 )
#define W_NEXT_SCREEN_BUTTON     	( K_START_STANDARD_TOPIC + 9 )
#define K_NONSHARED_STANDARD_TOPIC	( 9 + 1 ) /* +1 for 1-based array */
    	/* end of Standard TOPIC window widgets */

#define K_MAX_STANDARD_TOPIC_WIDGETS	( K_MAX_SHARED_WIDGETS 	   + \
    	    	    	    	    	    K_MAX_SHARED_TOPIC	   + \
    	    	    	    	    	    K_NONSHARED_STANDARD_TOPIC )

/*
 *  Constants used to select how the File Selection dialog box is to be
 *  configured.
 */

#define K_OPEN_BOOK_FILE    	1
#define K_OPEN_LIBRARY_FILE 	2
#define K_OPEN_DEFAULT_LIBRARY	4


/*
 *  Constants used to identify a dialog box in the client message callback.
 */

#define K_LIBRARY_WINDOW    	    	1
#define K_LIBRARY_WINDOW_DIALOG	    	2
#define K_SELECTION_MULTI_HIT_DIALOG  	3


/*
 *  Constants used to index Selection window popup widget ids
 */

#define W_SELECTION_POPUP   	    	1
#define W_TOPIC_POPUP_ENTRY 	    	2
#define W_TOPIC_IN_NEW_POPUP_ENTRY  	3
#define W_TOPIC_MULTI_POPUP_ENTRY   	4
#define W_EXPAND_POPUP_ENTRY	    	5
#define W_COLLAPSE_POPUP_ENTRY	    	6
#define K_MAX_SELECTION_POPUP_WIDGETS	( 6 + 1 ) /* +1 for 1-based array */


/*
 *  Constants used to index Topic window popup widget ids
 */

#define W_TOPIC_POPUP   	    	1
#define W_HOT_SPOT_POPUP_ENTRY	    	2
#define W_HOT_SPOT_IN_NEW_POPUP_ENTRY	3
#define W_PREV_TOPIC_POPUP_ENTRY    	4
#define W_NEXT_TOPIC_POPUP_ENTRY    	5
#define W_GOBACK_POPUP_ENTRY	    	6
#define W_CLOSE_TOPIC_POPUP_ENTRY   	7
#define K_MAX_TOPIC_POPUP_WIDGETS	( 7 + 1 ) /* +1 for 1-based array */


/*
 *  Constants used to index Multi-hit dialog box widget ids.
 */

#define W_MULTI_HIT_DIALOG  	    0
#define W_MULTI_HIT_SCROLLED_LIST   1
#define W_MULTI_HIT_OPEN    	    2
#define W_MULTI_HIT_OPEN_IN_NEW	    3
#define W_MULTI_HIT_CANCEL  	    4
#define W_MULTI_HIT_LABEL   	    5

#define K_MAX_ENTRY_POPUP_WIDGETS   6	/* total # of saved widgets */
    	    	    	    	    	    
#ifdef SEARCH


/*
 *  Search related constants
 */

#define K_SEARCH_LIBRARY 		1
#define K_SEARCH_BOOKS			2
#define K_SEARCH_BOOK			3
#define K_SEARCH_TOPICS			4
#define K_SEARCH_LIBRARY_WINDOW		6
#define K_SEARCH_SELECTION_WINDOW 	7

#define K_SHELF_RESULT			1
#define K_BOOK_RESULT			2
#define K_TOPIC_RESULT			3

#endif   /* SEARCH */


#endif /* _BKR_WIDGET_VALUES_H */

/* DON'T ADD STUFF AFTER THIS #endif */

