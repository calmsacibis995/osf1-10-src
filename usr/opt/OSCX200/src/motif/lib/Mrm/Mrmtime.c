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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: Mrmtime.c,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 13:48:15 $"
#endif
#endif

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
 *++
 *  FACILITY:
 *
 *      UIL Resource Manager (URM):
 *
 *  ABSTRACT:
 *
 *	System-dependent routines dealing with time and dates.
 *
 *--
 */


/*
 *
 *  INCLUDE FILES
 *
 */

#ifdef VMS
#include <descrip.h>
#endif

#include <Mrm/MrmAppl.h>
#include <Mrm/Mrm.h>
#include <time.h>

/*
 *
 *  TABLE OF CONTENTS
 *
 *	Urm__UT_Time		- Get current time as a string
 *
 */


/*
 *
 *  DEFINE and MACRO DEFINITIONS
 *
 */



/*
 *
 *  EXTERNAL VARIABLE DECLARATIONS
 *
 */


/*
 *
 *  GLOBAL VARIABLE DECLARATIONS
 *
 */

/*
 *
 *  OWN VARIABLE DECLARATIONS
 *
 */




void Urm__UT_Time (time_stg)
    char			*time_stg ;

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	This routine writes the current date/time string into a buffer
 *
 *  FORMAL PARAMETERS:
 *
 *	time_stg	location into which to copy time string. The
 *			length of this buffer should be at least
 *			URMhsDate1
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  FUNCTION VALUE:
 *
 *  SIDE EFFECTS:
 *
 *--
 */

{

/*
 *  External Functions
 */

/*
 *  Local variables
 */

#ifdef VMS
MrmCount			timelen ;
double				date_time ;
long				cvtflag ;
struct dsc$descriptor_s		time_desc ;

time_desc.dsc$a_pointer = time_stg ;
time_desc.dsc$w_length = URMhsDate ;
time_desc.dsc$b_class = DSC$K_CLASS_S ;
time_desc.dsc$b_dtype = DSC$K_DTYPE_T ;

date_time = 0.0 ;
cvtflag = 0 ;

sys$asctim (&timelen, &time_desc, NULL, cvtflag) ;

time_stg[timelen] = 0 ;

#endif

#ifndef VMS
#if __STDC__ 
time_t		timeval;
#else
long		timeval;
#endif /* __STDC__ */

time (&timeval);
strcpy (time_stg, ctime(&timeval));
#endif

}

