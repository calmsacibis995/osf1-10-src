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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: moss_misc.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:32:34 $";
#endif
#ifndef lint
static char *sccsid = "%W%	ULTRIX	%G%" ;
#endif

/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1989, 1990, 1991, 1992.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    The following contains 
 *
 * Routines:
 *
 *    moss_free_time()
 *    moss_get_time()
 *    moss_get_pid()  (internal, currently VMS only)
 *    moss_reply_required()
 *
 * Author:
 *
 *    Oscar Newkerk
 *
 * Date:
 *
 *    October 15, 1990
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, November 12th, 1990.
 *
 *    Change the input parameter to moss_get_time to an address of a pointer.
 *
 *    Kelly C. Green, January 9th, 1991.
 *
 *    Changes to support UTC time format, includes:
 *        Convert MO_TIME structure to universal format.
 *        Incorporate support for the use of DTSS as time provider.
 *        Added routine moss_str_time as a testing aid.
 *
 *    Kelly C. Green, February 27, 1991.
 *
 *    Add support for System V.  Includes:
 *        removal of extended precision macros that choke the System V C compiler.
 *        Use the "time" function.
 *    Add support for input of pointer to previously allocated time buffer
 *
 *    Miriam Amos Nihart, April 10th, 1991.
 *
 *    Changes for OSF/1.
 *
 *    Miriam Amos Nihart, August 13th, 1991.
 *
 *    Move sys_time to routine used in rather than having it span the scope of this
 *    module.
 *
 *    Miriam Amos Nihart, December 3rd, 1991.
 *
 *    Put in macros for malloc and free.
 *
 *    Russ Murray & Steve Pitcher, Jan 7, 1992
 *
 *    Get itmlst.h from resobj$. Get jpidef.h and  itmlst.h only in the VMS
 *    case.
 *    
 *    Integrate the following changes into the 1H Common Agent kit:
 * 
 *   	 Susan Elbeery, September 30, 1991.
 *
 *    	 Add moss_get_pid.
 *
 *       Russ Murray, October 8th, 1991.
 *    
 *       Change get_pid to moss_get_pid.
 *
 *       Russ Murray, October 8th, 1991.
 *
 *       Get itmlst.h from sys$library.
 *
 *    Steve Pitcher, Feb 7, 1992
 *	Module no longer needs NIL.H.
 *
 *    Gary Allison, March 4th, 1992.
 *
 *    Added moss_free_time.
 *
 *    Rich Bouchard, Jr.  June 30, 1992.  RJB1201
 *      Support DTS on VMS.
 *
 *    Rich Bouchard, Jr.  July 24, 1992.  RJB1223
 *      Add moss_reply_required.
 */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - cma_lib_malloc() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace malloc()
 *
 *   - cma_lib_free() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace free()
 *
 *   - gettimeofday() is not defined in <sys/time.h> so define it here
 */

extern int gettimeofday() ;

#ifndef NOIPC
extern void *cma_lib_malloc() ;
extern void *cma_lib_free() ;
#else
#include <stdlib.h>
#endif /* NOIPC */

/*
 *  ANSI C defines __mips and __ultrix. To be compatible with ANSI C,
 *  include <ansi_compat.h>. This file is included by some .h files but
 *  include it here just in case.
 */

#if defined(ultrix) || defined(__ultrix)
#include <ansi_compat.h>
#endif

/*
 *  Support header files
 */  

#include "man_data.h"
#include "man.h"
#ifdef VMS
#include "sme_common"
#include <jpidef.h>
#include "itmlst.h"
#endif /* VMS */

/*
 *  MOSS Specific header files
 */

#include "moss.h" 
#include "moss_private.h" 
#include "moss_utc.h"

/*
 *  External
 */

#include <string.h>

#ifdef VMS

#include <types>
#include <timeb>
#include <starlet.h>
#include <lnmdef.h>
#include <descrip.h>

#include <errno.h>
#include <ctype.h>

#define TRAN_BUFF_SIZE 255

#else

#if defined(__osf__) || defined(sun) || defined(sparc)
#include <sys/time.h>
#endif /* OSF */

#include <time.h>

#endif	    /*  VMS */

/*
 *++
 *  iadd()
 *
 *  Functional Description:
 *        Add 32 bit signed value to 64 bit value,
 *        producing 64-bit signed result.
 *  
 *  Inputs:
 *
 *       Iadd1  address of 64 bit signed addend
 *       I      value of 32 bit signed addend
 *
 *  Implicit Inputs:
 *
 *
 *  Outputs:
 *
 *       sum   address of 64 bit signed result
 *
 *  Implicit Outputs:
 *
 *      
 *  Value Returned:
 *
 *
 *  Side Effects:
 *
 *--
 */
static void iadd (add1, I, sum)
_bits64 *add1;
int I;
_bits64 *sum;
{
    int intermediate = ((add1->lo & 0x80000000) != 0);

    sum->lo = ((unsigned)I) + add1->lo;
    sum->hi = add1->hi;
    if ((intermediate) ^ (I < 0))
    {
        if (I < 0)
            sum->hi-- ;
        if (!(sum->lo & 0x80000000))
	    sum->hi++ ;
    }
}
/* End of routine iadd */



/*
 *++
 *  uemul()
 *
 *  Functional Description:
 *        32-bit unsigned quantity * 32-bit unsigned quantity
 *        producing 64-bit unsigned result. This routine assumes
 *        long's contain at least 32 bits. It makes no assumptions
 *        about byte orderings.
 *  
 *  Inputs:
 *
 *        u, v       Are the numbers to be multiplied passed by value
 *
 *  Implicit Inputs:
 *
 *
 *  Outputs:
 *
 *        prodPtr    is a pointer to the 64-bit result 
 *
 *  Implicit Outputs:
 *
 *      
 *  Value Returned:
 *
 *
 *  Side Effects:
 *
 *
 *  Note:
 *        This algorithm is taken from: "The Art of Computer
 *        Programming", by Donald E. Knuth. Vol 2. Section 4.3.1
 *        Pages: 253-255.
 *--
 */

static void uemul(u, v, prodPtr)

unsigned int     u;
unsigned int     v;
_bits64                *prodPtr;

{
    /* Following the notation in Knuth, Vol. 2 */

    unsigned int   u1, u2, v1, v2, temp;

    u1 = u >> 16;
    u2 = u & 0xffff;
    v1 = v >> 16;
    v2 = v & 0xffff;

    temp = u2 * v2;
    prodPtr->lo = temp & 0xffff;
    temp = u1 * v2 + (temp >> 16);
    prodPtr->hi = temp >> 16;
    temp = u2 * v1 + (temp & 0xffff);
    prodPtr->lo += (temp & 0xffff) << 16;
    prodPtr->hi += u1 * v1 + (temp >> 16);

}
/* End of routine uemul */


man_status 
moss_free_time(
		time
	      )
mo_time *time ;
/*
 *
 * Function Description:
 *
 *    This routine is used to deallocate the memory allocated by
 *    the moss_get_time routine.
 *
 * Parameters:
 *
 *    time		A pointer to time. If this is a
 *			pointer to NULL then space is not deallocated.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS              Success
 *    MAN_C_BAD_PARAMETER        The pointer is NULL
 *
 * Side effects:
 *
 *    None
 *
 */
{
    if (time == NULL)
	return( MAN_C_BAD_PARAMETER ) ;

    free( time ) ;

    return( MAN_C_SUCCESS ) ;
} /* end of moss_free_time() */


man_status
moss_get_time(
              current_time
             )

mo_time     **current_time ;

/*
 *
 * Function Description:
 *
 *    Return the current system time in the formart required by the Protocol Engines.
 *
 * Parameters:
 *
 *    current_time      The address to a pointer to a mo_time structure.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS              Success
 *    MAN_C_BAD_PARAMETER        The pointer is NULL
 *    MAN_C_PROCESSING_FAILURE   Error on get time
 *
 * Side effects:
 *
 *    None
 *
 *
 */

{
_utc *utc_time ;
struct
{
    unsigned int tv_sec ;
    unsigned int tv_usec ;
    unsigned int tz_minuteswest ;
    unsigned int tz_dsttime ;
} sys_time ;    

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     * bugout immediately with error if no output pointer is provided
     */

    if ( current_time == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    /*
     * allocate and initialize a time buffer
     * We assume a non-NULL *current_time is valid and re-use it.
     * Otherwise, allocate a new time buffer.
     * We always clear the buffer.
     */

    utc_time = (_utc *) *current_time;
    if ( NULL == utc_time )
    {
        MOSS_MALLOC( utc_time, _utc, sizeof( mo_time ) )
        if ( utc_time == NULL )
            return( MAN_C_PROCESSING_FAILURE ) ;
    }
    memset( ( void * )utc_time, '\0', sizeof( mo_time ) ) ;

    /*
     * try to get the time from DTSS.
     * if we can't then either
     *   the application is linked against module "utc_dummy" -or-
     *   DTSS is not installed on the host system.
     * the fallback strategy is to manually construct a UTC from the system clock.
     */

    if (0 == utc_gettime(utc_time))
    {
        *current_time = (mo_time *) utc_time ;
        return( MAN_C_SUCCESS ) ;
    }        

    /*
     * The call failed, do it the hard way.
     */
        
#ifdef VMS      /*  VMS  */

    {        
    man_status get_timezone_delta( ) ;
    man_status get_dts_timezone_delta(int *delta_time);
    struct timeb   tval ;
    int tz_min ;
    man_status return_status ;

        ftime( &tval ) ;
        sys_time.tv_sec = (int) tval.time ;
        sys_time.tv_usec = (int) (tval.millitm * 1000) ;

        /*
         * Get the timezone information from the MCC_TDF logical name.
         */

        return_status = get_timezone_delta( &tz_min ) ;
        if ( return_status != MAN_C_SUCCESS )
        {
	    return_status = get_dts_timezone_delta( &tz_min );
	    if (return_status != MAN_C_SUCCESS)
            {
	        if (utc_time != ((_utc *)*current_time))  /* did we allocate the buffer, or were we given one? */
                    MOSS_FREE( utc_time ) ;
                return( return_status );
	    }
        }
        sys_time.tz_minuteswest = tz_min;
        sys_time.tz_dsttime = 0 ;            

        /*
         * convert local VMS time to universal time by adding minuteswest;
         * Note that ULTRIX returns GMT without any further fuss.
         */

        sys_time.tv_sec += tz_min * 60 ;
    }

#elif defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)

    {
    struct timeval tval ;
    struct timezone tzone ;

        if ( gettimeofday( &tval, &tzone ) != 0 )
        {
            if (utc_time != ((_utc *)*current_time))  /* did we allocate the buffer, or were we given one? */
                MOSS_FREE( utc_time ) ;
            return( MAN_C_PROCESSING_FAILURE ) ;
        }
        sys_time.tv_sec = tval.tv_sec ;
        sys_time.tv_usec = tval.tv_usec ;
        sys_time.tz_minuteswest = tzone.tz_minuteswest ;
        sys_time.tz_dsttime = tzone.tz_dsttime ;
    }

#else /* least common denominator, eg System V */

    {
    int gmtval ;
    struct tm *localtm;
        if ( -1 == time( &gmtval ) )
        {
            if (utc_time != ((_utc *) *current_time))  /* did we allocate the buffer, or were we given one? */
                MOSS_FREE( utc_time ) ;
            return( MAN_C_PROCESSING_FAILURE ) ;
        }

        localtm = localtime(&gmtval);     /* computes the externs timezone and daylight */
                                          /* as a side effect - ugly, but true */

        sys_time.tv_sec = gmtval;
        sys_time.tv_usec = 0 ;
        sys_time.tz_minuteswest = timezone/60 ;
        sys_time.tz_dsttime = daylight ;
    }

#endif      /* VMS, ultrix */

    /*
     * convert system time into 100 ns units since the Gregorian reform
     */
    {
    int days_since_reform_to_1970 = 
                 ((1970-1)*365 + ((1970-1)/4) - ((1970-1)/100) + ((1970-1)/400) + 1       ) 
               - ((1582-1)*365 + ((1582-1)/4) - ((1582-1)/100) + ((1582-1)/400) + 273 + 15) ;
    _bits64 time_sec, time_100nsec, tmp_100nsec_lo, tmp_100nsec_hi ;

    uemul(days_since_reform_to_1970, 24*60*60, &time_sec) ;	/* convert 1970 base to seconds */
    iadd(&time_sec, sys_time.tv_sec, &time_sec) ;		/* add elapsed seconds since 1970 */
    uemul(10000000, time_sec.lo, &tmp_100nsec_lo) ;		/* convert low order to 100 ns units */
    uemul(10000000, time_sec.hi, &tmp_100nsec_hi) ;		/* convert high order to 100 ns units */
    tmp_100nsec_lo.hi += tmp_100nsec_hi.lo ;			/* add low 32 of high order to high 32 of low order */
    iadd(&tmp_100nsec_lo, sys_time.tv_usec*10, &time_100nsec) ;	/* add 100 ns part of last second */ 

    /*
     * construct UTC time value
     */

    utc_time->time_lo = time_100nsec.lo ;
    utc_time->time_hi = time_100nsec.hi ;
    utc_time->inacc_lo = 0xffffffff ;
    utc_time->inacc_hi = 0xffff ;
    utc_time->tdf = -sys_time.tz_minuteswest ;  /* "-" converts minuteswest into TDF adder */
    utc_time->vers = UTC_K_VERSION ;

    *current_time = ( mo_time *) utc_time ;
    return( MAN_C_SUCCESS ) ;

    }
} /* end of moss_get_time() */

#ifdef VMS
static man_status get_dts_timezone_delta(int	*delta_time)
/*
 *
 * Function description:
 *
 *   Gets the number of minutes from Universal to Local time.
 *   Obtained by translating the logical name "SYS$TIMEZONE_DIFFERENTIAL",
 *   and parsing the result (number of seconds from GMT) to
 *   an integer number of minutes.
 *    
 *
 * Arguments:
 *
 *    delta_time       <out>    A pointer to the number of minutes
 *                              that must be added to the Local time
 *                              in order to obtain a Universal time.
 *
 * Return value:
 *              
 *    MAN_C_SUCCESS - the translation was successfully obtained
 *    MAN_C_PROCESSING_FAILURE - an error occurred, consult errno for additional information
 *
 * Side effects:   
 *
 *    if the return value is MAN_C_PROCESSING_FAILURE, then errno is set as follows:
 *
 *        EVMSERR - VMS returned a system error code
 *                  the VMS return code is saved in vaxc$errno.
 *
 *        ERANGE - the translation string is malformed and cannot
 *                 be converted to binary minutes.
 */
{
    int translation_attributes = LNM$M_CASE_BLIND ;
    $DESCRIPTOR(table_name, "LNM$SYSTEM_TABLE");
    $DESCRIPTOR(logical_name, "SYS$TIMEZONE_DIFFERENTIAL");
    char result_string[TRAN_BUFF_SIZE + 1] ;
    unsigned short result_length ;
    ITMLST_DECL(item_list, 2);
    int val1 = 0xFFFF;
    int return_status ;

    ITMLST_INIT(item_list, 0, TRAN_BUFF_SIZE, LNM$_STRING, &result_string, &result_length);
    ITMLST_INIT(item_list, 1, 0,              0,           0,              0);

    return_status = sys$trnlnm(&translation_attributes,                     /* attr */
                               &table_name,		                    /* tabname */
                               &logical_name,		                    /* logname */
                               0,                                           /* acmode */
                               &item_list);                                 /* itmlst */

    /*
     * if the least significant bit of the return status is NOT set, 
     * then an error occured.  Return the actual VMS error code in vaxc$errno,
     * and a generic EVMSERR status code in errno.
     */
    if ( 0 == ( return_status & 1 ) )
    {
        vaxc$errno = return_status ;
        errno = EVMSERR ;
        return( MAN_C_PROCESSING_FAILURE ) ;
    }

    /*
     * The name translated!  Convert the string to minutes
     */
    result_string[result_length] = '\000' ;
    sscanf(result_string, "%d", &val1);

    if ((val1 < -46800) || (val1 > 46800))
    {
	errno = ERANGE;
	return MAN_C_PROCESSING_FAILURE;
    }

    *delta_time = -val1 / 60;

    return MAN_C_SUCCESS;
}
#endif /* VMS */

#ifdef VMS

static man_status
get_timezone_delta(
                     delta_time
                  )

int *delta_time ;
/*
 *
 * Function description:
 *
 *   Gets the number of minutes from Universal to Local time.
 *   Obtained by translating the logical name "MCC_TDF",
 *   and parsing the result (of the form "[�]hh:mm") to
 *   an integer number of minutes.
 *    
 *
 * Arguments:
 *
 *    delta_time       <out>    A pointer to the number of minutes
 *                              that must be added to the Local time
 *                              in order to obtain a Universal time.
 *
 * Return value:
 *              
 *    MAN_C_SUCCESS - the translation was successfully obtained
 *    MAN_C_PROCESSING_FAILURE - an error occurred, consult errno for additional inforation
 *
 * Side effects:   
 *
 *    if the return value is MAN_C_PROCESSING_FAILURE, then errno is set as follows:
 *
 *        EVMSERR - VMS returned a system error code
 *                  the VMS return code is saved in vaxc$errno.
 *
 *        ERANGE - the translation string is malformed and cannot
 *                 be converted to binary minutes.
 */
 
{
    char *result_char_p ;
    int result ;
    int val1 ;
    int val2 ;
    char separator[2] ;

    /*
     * the following typedefs and variable declarations are
     * the bletcherous, ugly details needed to invoke the $TRNLNM service
     */

    int translation_attributes = LNM$M_CASE_BLIND ;

    $DESCRIPTOR (table_name, "LNM$SYSTEM_TABLE") ;

    $DESCRIPTOR (logical_name, "MCC_TDF") ;

    char result_string[TRAN_BUFF_SIZE + 1] ;
    unsigned short result_length ;
    ITMLST item_list[2] = {
                                { TRAN_BUFF_SIZE , LNM$_STRING, &result_string, &result_length },
                                { 0, 0, 0, 0}
                               } ;
    int return_status ;

    return_status = sys$trnlnm(&translation_attributes,                     /* attr */
                               &table_name,		                    /* tabname */
                               &logical_name,		                    /* logname */
                               0,                                           /* acmode */
                               &item_list                                   /* itmlst */
                              ) ;
    /*
     * if the least significant bit of the return status is NOT set, 
     * then an error occured.  Return the actual VMS error code in vaxc$errno,
     * and a generic EVMSERR status code in errno.
     */

    if ( 0 == ( return_status & 1 ) )
    {
        vaxc$errno = return_status ;
        errno = EVMSERR ;
        return( MAN_C_PROCESSING_FAILURE ) ;
    }

    /*
     * The name translated!  Convert the string to minutes
     */

    result_string[result_length] = '\000' ;
    
    switch (sscanf(result_string, " %d %2[:] %d",&val1,&separator,&val2))
    {
    case 1:		    /* [�]mm      */
        if (60 < abs(val1))
        {
            errno = ERANGE ;
            return( MAN_C_PROCESSING_FAILURE ) ;
        }
        result = val1 ;
        break ;
    case 2:		    /* [�]hh:     */
        result = val1 * 60 ;
        break ;
    case 3:		    /* [�]hh:mm   */
        if (0 > val2 || 60 < val2)
        {
            errno = ERANGE ;
            return( MAN_C_PROCESSING_FAILURE ) ;
        }
        result = (val1 * 60) + (( 0<=val1 ? 1 : -1) * val2) ;
        break ;
    default:
        errno = ERANGE ;
        return( MAN_C_PROCESSING_FAILURE ) ;
    }

    /*
     * The allowable range is -12:00 to +13:00 hours.
     */

    if (-12*60 > result || 13*60 < result )
        {
            errno = ERANGE ;
            return( MAN_C_PROCESSING_FAILURE ) ;
        }

    *delta_time = -result ;  /* "-" converts TDF to minuteswest of prime meridian */
    return( MAN_C_SUCCESS ) ;

}  /* end of get_delta_time */

#endif      /*  VMS  */

/*
 * Note - this routine is currently VMS-specific.  However, it needs
 * to be updated for Ultrix/OSF to support the MOM Profiler
 */
#ifdef VMS

man_status
moss_get_pid( pid )

process_id *pid;

/*
 *
 * Function Description:
 *
 * This routine returns the pid of the calling process
 *
 * Note that this routine is internal-use-only.  It is currently
 * VMS-specific.
 *
 * Parameters:
 *
 * 	pid process identification of the MOM making the call
 *
 * Return value:
 *
 *
 *
 * Side effects:
 *
 * None.
 *
 */


{
    man_status status;
    ITMLST_DECL( itmlst, 1 ); /* getpid itemlist */

    ITMLST_INIT( itmlst, 0, 4, JPI$_PID, pid, 0 );
    ITMLST_INIT( itmlst, 1, 0, 0, 0, 0 );
    status = sys$getjpiw( 0, 0, 0, itmlst, 0, 0, 0 );

   /*
    * if the least significant bit of the return status is NOT set, 
    * then an error occured. 
    */

    if ( 0 == ( status & 1 ) )
	return( MAN_C_PROCESSING_FAILURE ) ;
      else
	return( MAN_C_SUCCESS ) ;

} /* end of moss_get_pid() */
#endif /* VMS */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine determines is a reply is required for a management
**	request, based on a simple examination of a management handle.
**
**  FORMAL PARAMETERS:
**
**      man_handle	A pointer to the management handle.
**
**  RETURN VALUE:
**
**	MAN_C_FALSE	No reply because management handle does not exist
**	MAN_C_TRUE	Reply returned because management handle exits
**
**--
*/
man_status moss_reply_required(man_handle)

management_handle *man_handle;

{
    if (man_handle != NULL)
	return MAN_C_TRUE;
    else
	return MAN_C_FALSE;
}

/* end of moss_misc.c */
