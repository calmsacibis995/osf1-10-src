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
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**	Application title and version information
**
**  AUTHORS:
**	Tom Rose 
**
**  CREATION DATE:     07-Apr-1992
**
**  MODIFICATION HISTORY:
**
**--
**/

#define BKR_APP_NAME		"Bookreader"
#define BKR_MAJOR_VERSION	4
#define BKR_MINOR_VERSION	1
#define BKR_VERSION_STRING	"V4.1"

/* Definitions used to check the BOOKREADER-CC license.  Note that the
 * version number in the PAK for BOOKREADER-CC support can be, and 
 * probably is, different from the version number for Bookreader itself.
 */
#define BKR_CC_PRODUCT_NAME     "BOOKREADER-CC"
#define BKR_CC_PRODUCER         "DEC"
#define BKR_CC_MAJOR_VERSION	1
#define BKR_CC_MINOR_VERSION	0
#define BKR_CC_PRODUCT_DATE	"01-OCT-1992"
#define BKR_CC_PRODUCT_BIN_TIME	718009199	/* To calculate, use command 
                                                 * /usr/etc/lmfck -d 011092 
                                                 */

