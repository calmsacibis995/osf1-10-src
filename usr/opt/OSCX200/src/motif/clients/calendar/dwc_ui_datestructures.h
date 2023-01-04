#ifndef _datestructures_h_
#define _datestructures_h_
/* $Header$ */
/* #module DWC$UI_DATESTRUCTURES.H "V1-001"				    */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Marios Cleovoulou, November-1987
**
**  ABSTRACT:
**
**	This include file contains the date data structures used in calendar.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	V1-001  Marios Cleovoulou				27-Nov-1987
**		Initial version.
**--
**/


#include    "dwc_compat.h"


/* Date Time block */

typedef struct {

    int	    year ;	/* Includes century  */
    int	    month ;	/* 1-12		     */
    int	    day ;	/* 1-31		     */
    int	    hour ;	/* 0-23		     */
    int	    minute ;	/* 0-59		     */
    int	    second ;	/* 0-59		     */
    int	    hundredth ;	/* 0-99		     */
    int	    weekday ;	/* 1-7, 1 is a Sunday, 0 means not calculated yet */

} dtb ;


#endif /* _datestructures_h_ */
