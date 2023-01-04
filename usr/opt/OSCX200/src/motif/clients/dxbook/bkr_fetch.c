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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FETCH.C*/
/* *5    17-JUN-1992 20:41:39 BALLENGER "Include br_common_defs.h"*/
/* *4     8-JUN-1992 19:09:24 BALLENGER "UCX$CONVERT"*/
/* *3     8-JUN-1992 12:55:26 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *2     3-MAR-1992 16:58:44 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:39:17 PARMENTER "Fetch literals and icons from the UID file"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FETCH.C*/
#ifndef VMS
 /*
#else
# module BKR_FETCH "V03-0000"
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
**	Routines for fetching literals and icons from the UID file.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     20-Apr-1990
**
**  MODIFICATION HISTORY:
**
**--
**/


/*
 * INCLUDE FILES
 */


#include    <X11/Intrinsic.h>
#include    <Mrm/MrmPublic.h>
#include "br_common_defs.h"
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_fetch.h"       /* function prototypes for .c module */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_fetch_window_icon
**
** 	Fetchs a window icon literal from the UID file given its name.
**
**  FORMAL PARAMETERS:
**
**	index_string - name of the literal to fetch.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    pointer to literal fetched or
**  	    	    0 if icon literal can not be fetched
**
**  SIDE EFFECTS:
**
**	Virtual memory is allocated.
**
**--
**/
Pixmap
bkr_fetch_window_icon PARAM_NAMES((index_string))
    String index_string PARAM_END
{
    Screen  	*screen;
    Pixmap  	pixmap_rtn;
    Dimension   width, height;	

    screen = XDefaultScreenOfDisplay( bkr_display );

    if ( MrmFetchBitmapLiteral( 
    	    	bkr_hierarchy_id,
    	    	index_string,	    /* name of icon literal */
    	    	screen,	    	    /* screen pointer 	    */
    	    	bkr_display,
    	    	&pixmap_rtn,
    	    	&width, 		
    	    	&height ) !=  MrmSUCCESS )
    	return  (Pixmap) 0;

    return (Pixmap) pixmap_rtn;

};  /* end of bkr_fetch_window_icon */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_fetch_icon_literal
**
** 	Fetchs an icon literal from the UID file given its name.
**
**  FORMAL PARAMETERS:
**
**	index_string - name of the literal to fetch.
**  	foreground   - foreground pixel color for returned pixmap.
**  	background   - background pixel color for returned pixmap.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    pointer to literal fetched or
**  	    	    0 if icon literal can not be fetched
**
**  SIDE EFFECTS:
**
**	Virtual memory is allocated.
**
**--
**/
Pixmap
bkr_fetch_icon_literal PARAM_NAMES((index_string,foreground,background))
    String  	index_string PARAM_SEP
    Pixel   	foreground PARAM_SEP
    Pixel   	background PARAM_END

{
    int	    	status;
    Screen  	*screen;
    Pixmap  	pixmap_rtn;

    screen = XDefaultScreenOfDisplay( bkr_display );
    status = MrmFetchIconLiteral( 
    	    	bkr_hierarchy_id,
    	    	index_string,	    /* name of icon literal */
    	    	screen,	    	    /* screen pointer 	    */
    	    	bkr_display,
    	    	foreground, 	    /* pixmap foreground    */
    	    	background, 	    /* pixmap background    */
    	    	&pixmap_rtn );

    if ( status != MrmSUCCESS )
    	return  (Pixmap) 0;

    return (Pixmap) pixmap_rtn;

};  /* end of bkr_fetch_icon_literal */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_fetch_literal
**
** 	Fetchs a literal from the UID file given its name.
**
**  FORMAL PARAMETERS:
**
**	index_string	- name of the literal to fetch from the UID file.
**  	data_type   	- data type of literal to fetch.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    pointer to literal fetched or
**  	    	    NULL if literal can not be fetched
**
**  SIDE EFFECTS:
**
**	Virtual memory is allocated.
**
**--
**/
caddr_t
bkr_fetch_literal PARAM_NAMES((index_string, data_type))
    String index_string PARAM_SEP
    MrmCode 	data_type PARAM_END
{
    int	    	status;
    caddr_t 	value;
    MrmCode 	value_type;

    status = MrmFetchLiteral( 
    	    	bkr_hierarchy_id,
    	    	index_string,	    /* name of literal to fetch    */
    	    	bkr_display,
    	    	&value,	    	    /* value of the literal 	   */
    	    	&value_type );	    /* returned literals data type */
    if ( status != MrmSUCCESS )
    	return  (caddr_t) NULL;

    if ( value_type != data_type )
    	return  (caddr_t) NULL;

    return (caddr_t) value;

};  /* end of bkr_fetch_literal */

