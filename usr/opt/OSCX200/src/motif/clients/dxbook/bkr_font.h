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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FONT.H*/
/* *6    31-MAR-1992 15:51:44 ROSE "Added parameter to parse font routine"*/
/* *5    28-MAR-1992 17:24:17 BALLENGER "Add font support for converted postscript"*/
/* *4     8-MAR-1992 19:13:54 BALLENGER " Add topic data and text line support"*/
/* *3     3-MAR-1992 16:59:16 KARDON "UCXed"*/
/* *2     1-NOV-1991 13:05:11 BALLENGER "reintegrate  memex support"*/
/* *1    16-SEP-1991 12:45:44 PARMENTER "Function Prototypes for bkr_font.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FONT.H*/
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
**	Function prototypes for bkr_font.c
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     17-Apr-1990
**
**  MODIFICATION HISTORY:
**
**--
**/
#ifndef BKR_FONT_H
#define BKR_FONT_H

#include "br_prototype.h"


/* Routines defined in bkr_font.c
 */
extern void bkr_font_data_close PROTOTYPE((BKR_BOOK_CTX *book));
extern void bkr_font_data_init PROTOTYPE((BKR_BOOK_CTX *book));

extern BKR_FONT_DATA_PTR
bkr_font_entry_init PROTOTYPE((BKR_BOOK_CTX *book,
                               unsigned short int font_num
                               ));

extern void 
bkr_font_parse_name PROTOTYPE((
    char *font_name,
    char *family,     
    char *style,
    char *weight,
    char *set_width_name,
    long int *size,
    unsigned long *char_set,
    char *ps_font_name
));

#endif 

