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
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_SURROGATE_DEFS.H*/
/* *3     3-MAR-1992 17:08:20 KARDON "UCXed"*/
/* *2     1-NOV-1991 12:48:29 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:48:01 PARMENTER "Surrogate definitions"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_SURROGATE_DEFS.H*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_SURROGATE_DEFS.H*/
/* *6     1-MAY-1991 00:50:39 BALLENGER "Fix problems with links in the library window."*/
/* *5     2-MAR-1991 19:04:54 BALLENGER "Linkworks name changes and QAR807 fix"*/
/* *4    18-FEB-1991 14:26:07 BALLENGER "IFT2 Fixes, portability fixes, and Hyperhelp fixes"*/
/* *3    25-JAN-1991 16:47:19 FITZELL ""*/
/* *2    12-DEC-1990 12:29:20 FITZELL "v3 IFT update"*/
/* *1     8-NOV-1990 11:20:29 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_SURROGATE_DEFS.H*/
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
**	This include file contains the defintions bookreader surrogate
**      objects and properties.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     02-Jul-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0002  DLB0002     David L Ballenger           30-Apr-1991
**            Fix problems with surrogates in the library window.
**
**  V03-0001  DLB0001     David L Ballenger           30-Jan-1991
**            Add support for new HyperHelp related operations:
**            "View In New Window", "View in Default Window", and
**            "Close".
**
**--
**/



#ifndef BMI_SURROGATE_DEFS_H
#define BMI_SURROGATE_DEFS_H

#define BMI_SURROGATE_SUBTYPE      "Bookreader"
#define BMI_OPERATION_VIEW         "View"
#define BMI_OPERATION_VIEW_NEW     "View In New Window"
#define BMI_OPERATION_VIEW_DEFAULT "View In Default Window"
#define BMI_OPERATION_CLOSE        "Close"
#define BMI_OPERATION_DEFAULT BMI_OPERATION_VIEW

/* Define Surrogate Object Types
 */
#define BMI_MIN_OBJ       1
#define BMI_OBJ_SHELF     1
#define BMI_OBJ_BOOK      2
#define BMI_OBJ_DIRECTORY 3
#define BMI_OBJ_CHUNK     4
#define BMI_MAX_OBJ       4


#define BMI_OBJ_NAME_SHELF     "Shelf"
#define BMI_OBJ_NAME_BOOK      "Book"
#define BMI_OBJ_NAME_DIRECTORY "Directory"
#define BMI_OBJ_NAME_CHUNK     "Chunk"


/* Define property names
 */
#define BMI_PROP_CONTAINER   "%Container"    /* lwk_c_domain_string  */
#define BMI_PROP_OBJECT_TYPE "%Type"         /* lwk_c_domain_string  */

#define BMI_PROP_VERSION     "Version"       /* lwk_c_domain_integer */
#define BMI_PROP_TIMESTAMP0  "TimeStamp0"    /* lwk_c_comain_integer */
#define BMI_PROP_TIMESTAMP1  "TimeStamp1"    /* lwk_c_comain_integer */

#define BMI_PROP_SHELF_LIBRARY     "ShelfLibrary"      /* lwk_c_domain_string */
#define BMI_PROP_SHELF_PATH        "ShelfPath"         /* lwk_c_domain_list */
#define BMI_PROP_SHELF_ENTRY_ID    "ShelfEntryId"      /* lwk_c_domain_integer */
#define BMI_PROP_SHELF_ENTRY_TYPE  "ShelfEntryType"    /* lwk_c_domain_integer */
#define BMI_PROP_SHELF_ENTRY_TITLE "ShelfEntryTitle"   /* lwk_c_domain_string */

#define BMI_PROP_DIR_NAME    "%SubContainer" /* lwk_c_domain_string  */
#define BMI_PROP_DIR_ID      "%Id"           /* lwk_c_domain_integer */
#define BMI_PROP_DIR_ENTRY   "DirEntry"      /* lwk_c_domain_integer */

#define BMI_PROP_TOPIC_NAME  "%SubContainer" /*lwk_c_domain_string  */
#define BMI_PROP_TOPIC_ID    "%Id"           /* lwk_c_domain_integer */

#define BMI_PROP_CHUNK_NAME  "%SubContainer" /* lwk_c_domain_string  */
#define BMI_PROP_CHUNK_ID    "%Id"           /* lwk_c_domain_integer */

#endif 


