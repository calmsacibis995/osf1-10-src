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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_LICENSE.C*/
/* *7     5-AUG-1992 21:30:28 BALLENGER "Reset alternate product name pointer after searching for NO_PRINT."*/
/* *6    24-JUL-1992 12:27:05 KLUM "repress print if NO_PRINT alt-prod-name"*/
/* *5    19-JUN-1992 20:16:30 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *4     9-JUN-1992 10:02:09 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *3     3-MAR-1992 17:10:56 KARDON "UCXed"*/
/* *2    12-DEC-1991 13:36:19 FITZELL "check producer field before making lmf call"*/
/* *1    16-SEP-1991 12:44:15 PARMENTER "License Management Facility (LMF)"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_LICENSE.C*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_LICENSE.C*/
/* *7    29-MAY-1991 17:29:43 BALLENGER "Fix syntax errorsin Tin build"*/
/* *6    15-MAY-1991 15:06:43 BALLENGER "Compiler/build cleanup for TIN BL1"*/
/* *5    17-APR-1991 19:21:48 FITZELL "eft2 lmf fix change context to 16 byte buffer"*/
/* *4     6-APR-1991 19:33:06 BALLENGER "Use correct lmf calls to allow third party licenses."*/
/* *3    25-JAN-1991 16:49:58 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 11:37:49 FITZELL "V3 ift update snapshot"*/
/* *1     8-NOV-1990 11:26:36 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_LICENSE.C*/
#ifndef VMS
 /*
#else
# module BRI_LICENSE "V03-0002"
#endif
#ifndef VMS
  */
#endif

#ifndef lint
static char *sccsid = "%W%   OZIX    %G%" ;
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
**
** Facility:
**
**   	BRI -- Book Reader Interface
**
** Abstract:
**
**      Contain ULTRIX and VMS versions of the routine to check 
**      book licensing.
**
** Functions:
**
**  	BriBookLmfCheck - Checks to see if a book is licensed for the
**                        system on which the bookreader is running.
**
** Author:
**
**      David L. Ballenger
**
** Date:
**
**      Mon Nov  6 15:16:51 1989
**
** Revision History:
**
**  V03-0002	DLB0002     David L Ballenger           14-May-1991
**  	      	Put in conditional code to deal with current lack of
**              LMF in Tin.
**
**  V03-0001	DLB0001     David L Ballenger           04-Apr-1991
**  	      	Use correct lmf calls to allow third party licenses.
**
**  V03-0000	JAF0000	    James A. Ferguson	    	16-Aug-1990 
**  	      	Create new module, modify revision history format and
**  	    	update copyright.
**
*/


#ifdef vms

#include <descrip.h>
#include <lmfdef.h>
#include <ssdef.h>

#ifndef LICENSE$_NOLICENSE
#define LICENSE$_SYSMGR           177505059
#define LICENSE$_NOAUTH           177507058
#define LICENSE$_NOLICENSE        177507860
#define LICENSE$_EXCEEDED         177507868
#define LICENSE$_NOT_STARTED      177507876
#define LICENSE$_INVALID_DATE     177507884
#define LICENSE$_INVALID_VERSION  177507892
#define LICENSE$_TERMINATED       177507900
#define LICENSE$_INVALID_HW_ID    177507908
#define LICENSE$_BADPARAM         177507916
#define LICENSE$_ILLPRODUCER      177507924
#define LICENSE$_WRONUMARG        177507932
#endif 

#else
#ifndef NO_LMF
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <time.h>
#include <sys/utsname.h>
#include <lmf.h>

#define V3_EXPIRATION_DATE ((time_t)662803199)
#define RELEASE_ALL (0)
#define NANO 10000000
#define DIVISOR (5*5*5*5*5*5*5)		/* NANO / (2 << (32-SHIFT1-SHIFT2)) */
#define SHIFT1 8
#define MASK1 (~((1 << 23) - 1))
#define SHIFT2 17
#define SHIFT3 14
#define SHIFT4 3
#define BASE_HIGH 8164711
#define BASE_LOW 1273708544
#define SIGN 0x80000000

#define ERROR 0
#define OK    1

#endif 
#endif 

#include "bri_private_def.h"

#ifndef vms
#ifndef NO_LMF
static int 
VmsToUltrixDate PARAM_NAMES((vmsdate,tp))
    unsigned int vmsdate[2] PARAM_SEP
    time_t *tp PARAM_END
/* Function to convert VMS format date/time to UNIX format.
 * Returns: 0 - success
 *	    1 - VMS date too far into future
 *	   -1 - VMS date too far in past
 *
 * The UNIX date is assumed to be a signed int, as ULTRIX has it
 * and times before 1970 are processed correctly with this assumption.
 */

{
    register unsigned hi, lo, rem, rem2;
    int sign;
    
    lo = vmsdate[0] - BASE_LOW;
    hi = vmsdate[1] - BASE_HIGH;
    if (vmsdate[0] < BASE_LOW) {
        --hi;
    }
    
    /* Deal with the past.  Since we definitely won't be dealing with products
     * released prior to 1970 and the release date field is currently being 
     * set to VMS zero date in the 1800's, just convert any dates prior to
     * the ULTRIX zero date in 1970 to 0.
     */

    if (hi & SIGN) {
        *tp = (time_t)0;
        return 0;
    } else {
        sign = 1;
    }

    rem = lo % NANO;
    lo = lo / NANO;
    
    /* Now deal with the high 32 bits */
    
    if (hi & MASK1) {
        return sign;
    }
    hi <<= SHIFT1;
    rem2 = hi % DIVISOR;
    hi  = hi / DIVISOR;
    hi <<= SHIFT2;
    lo += hi;
    if (lo < hi) {
        return sign;
    }
    /* Deal with remainder */
    
    hi = rem2 << SHIFT3;
    rem2 = hi % DIVISOR;
    hi  = hi / DIVISOR;
    hi <<= SHIFT4;
    lo += hi;
    if (lo < hi) {
        return sign;
    }
    /* One more time */
    
    hi = rem2 << SHIFT4;
    rem2 = hi % DIVISOR;
    hi  = hi / DIVISOR;
    lo += hi;

    /* Deal with final remainder */
    
    if (lo < hi) {
        return sign; 
    }
    rem += rem2 << 7;
    if (rem >= NANO / 2) {
        ++lo;
        if (!lo) {
            return sign;
        }
        if (rem >=  NANO + NANO / 2) {
            ++lo;
            if (!lo) {
                return sign;
            }
        }
    }
    if (lo & SIGN) {
        return sign;
    }

    *tp =  sign < 0 ? -lo : lo;
    return 0;
}
#endif
#endif 


void
BriBookLmfCheck(book_context)
    register BRI_CONTEXT *book_context;
/*
 *
 * Function description:
 *
 *      Calls the LMF routines to check the licensing for a book.
 *
 * Arguments:
 *
 *      book_context - Context for the book.
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{
    register BODS_BOOK_HEADER *vbh = &book_context->data.book->vbh;
    register int status;
    int initial_error;
    register BRI_STRING_LIST *alt_prod_name = book_context->data.book->alternate_product_names;


#ifdef vms

#define InitDesc(desc,str) { (desc)->dsc$a_pointer = (str) ; \
                             (desc)->dsc$w_length = strlen(str) ; \
                             (desc)->dsc$b_dtype = DSC$K_DTYPE_T ; \
                             (desc)->dsc$b_class = DSC$K_CLASS_S ; \
                         }
                             

    struct dsc$descriptor Product ;
    struct dsc$descriptor Producer ;
    VMS_ITEM_LIST LmfItems[3];
    unsigned flags = LMF$M_RETURN_FAILURES ;
    unsigned context[4] ;


    /*--- check alternate product names to see if book is "un-printable" ---*/
    for(; alt_prod_name; alt_prod_name = alt_prod_name->next ) {
        if(!strcmp(alt_prod_name->string,"NO_PRINT")) {
           book_context->no_print = TRUE;
           break;
        }
    }

    /* Reset the alternate product name pointer before checking the
     * license.
     */
    alt_prod_name = book_context->data.book->alternate_product_names;

    if (strlen(vbh->LMF_producer) == 0) {

        /* The producer field is not filled in so there is no license
         * checking to do.
         */
        return ;
    }

    InitDesc(&Product, vbh->LMF_product_name);
    InitDesc(&Producer, vbh->LMF_producer);

    LmfItems[0].itemcode = LMF$_PROD_DATE ;
    LmfItems[0].pointer  = &vbh->LMF_product_date[0];
    LmfItems[0].length   = LMF$C_DATELEN ;
    LmfItems[1].itemcode = LMF$_PROD_VERSION ;
    LmfItems[1].pointer  = &vbh->LMF_product_version ;
    LmfItems[1].length   = LMF$C_VERSIONLEN ;
    LmfItems[2].itemcode = 0 ;
    LmfItems[2].length   = 0 ;

    /* Check the license using the information in the book
     */

    if(strcmp(vbh->LMF_producer,"DEC") == 0)
	status = sys$lookup_license(&Product,&LmfItems,&Producer,
				    &flags, context);
    else
	status = sys$grant_license(&Product,&Producer,context,&LmfItems);

    switch (status) {
        case SS$_NORMAL: {
            /* We are licensed to read this book.  Release all units.
             */
            sys$release_license(context);
            return ;
        }
        case LICENSE$_EXCEEDED: {
            /* No per user limit on reading books
             */
            return ;
        }
        case LICENSE$_NOLICENSE: {
            initial_error = BriErrLmfNoLicenseNum;
            break;
        }
        case LICENSE$_NOT_STARTED: {
            initial_error = BriErrLmfNotStartedNum;
            break;
        }
        case LICENSE$_INVALID_DATE: {
            initial_error = BriErrLmfInvalidDateNum;
            break;
        }
        case LICENSE$_TERMINATED: {
            initial_error = BriErrLmfTerminatedNum;
            break;
        }
        case LICENSE$_INVALID_VERSION: {
            initial_error = BriErrLmfInvalidVersNum;
            break;
        }
        case LICENSE$_INVALID_HW_ID: {
            initial_error = BriErrLmfInvalidHwIdNum;
            break;
        }
        default:
        case LICENSE$_BADPARAM:
        case LICENSE$_WRONUMARG:
        case SS$_ACCVIO:
        case SS$_BADPARAM: {
            initial_error = BriErrLmfBadParamNum;
            break;
        }
        case SS$_EXENQLM: {
            initial_error = BriErrLmfExEnqlmNum;
            break;
        }
        case SS$_INSFMEM: {
            initial_error = BriErrNoMemoryNum;
            break;
        }
    }

    /* Check the alternate product names, for V1 books BOOKBROWSER has been
     * set up to be the alternate name by BriBookGetHeader().
     */
    while (alt_prod_name != NULL) {

        InitDesc(&Product, alt_prod_name->string);

	if(strcmp(vbh->LMF_producer,"DEC") == 0)
	    status = sys$lookup_license(&Product,&LmfItems,&Producer,
				    &flags, context);
	else
	    status = sys$grant_license(&Product,&Producer,context,&LmfItems);

        if (status & SS$_NORMAL ) {
            /* We are licensed to read this book.  Release all units.
             */
            sys$release_license(context);
            return ;
        } else if (status == LICENSE$_EXCEEDED) {

            /* No per user limit on reading books
             */
            return ;
        }
        alt_prod_name = alt_prod_name->next ;
    }

    BriLongJmp(book_context,initial_error);

#else

#ifndef NO_LMF
    struct utsname uts;
    register int version;
    time_t release_date;
#endif 

    /*--- check alternate product names to see if book is "un-printable" ---*/
    for(; alt_prod_name; alt_prod_name = alt_prod_name->next ) {
        if(!strcmp(alt_prod_name->string,"NO_PRINT")) {
           book_context->no_print = TRUE;
           break;
        }
    }
    
    /* Reset the alternate product name pointer before checking the
     * license.
     */
    alt_prod_name = book_context->data.book->alternate_product_names;

    if (strlen(vbh->LMF_producer) == 0) {

        /* The producer field is not filled in so there is no license
         * checking to do.
         */
        return ;
    }
#ifndef NO_LMF
    /* See what version we are running on.  If it is prior to V4, then
     * there is no LMF support.
     */
    uname(&uts);

    version = atoi(uts.release);

    /* this check only makes sense for ULTRIX... don't do it if running OSF/1 */
#ifndef __osf__

    if (version < 4) {
        if (time(NULL) > V3_EXPIRATION_DATE) {

            /* No longer support reading books with out licensing on
             * V3.1.
             * Signal an error and prevent the book from being opened.
             */
            BriLongJmp(book_context,BriErrLmfNotStartedNum);
        }
#endif /* __osf__*/
#endif 
        /* Check the primary and alternate product names to see if
         * the book allows browsing, i.e. the BROWSER_PRODUCT_NAME
         * is one of the names.  If that is the case then let the
         * user read the book, i.e. just return.  Otherwise, do a
         * BriLongJmp to signal the error.
         */
        if (strcmp(vbh->LMF_product_name,BROWSER_PRODUCT_NAME) == 0 ) {
            return ;
        } 

        while (alt_prod_name != NULL) {
            if (strcmp(alt_prod_name->string,BROWSER_PRODUCT_NAME) == 0) {
                return ;
            }
            alt_prod_name = alt_prod_name->next;
        }

        BriLongJmp(book_context,BriErrLmfNoLicenseNum);

#ifndef NO_LMF
#ifndef __osf__
    }
#endif /* not osf */

    /* Convert the date to ULTRIX format
     */
    if (VmsToUltrixDate(vbh->LMF_product_date,&release_date)) {
        BriLongJmp(book_context,BriErrLmfInvalidDateNum);
    }

    /* Check the license using the information in the book
     */

    status = lmf_probe_license(vbh->LMF_product_name,
                               vbh->LMF_producer,
                               &vbh->LMF_product_version,
                               release_date,
                               0
                               ) ;

    if (status == 0) {
        /* We are licensed to read this book.  Release all units.
         */
        lmf_release_license(vbh->LMF_product_name,
                            vbh->LMF_producer,
                            RELEASE_ALL
                            );
        return ;
    }

    switch (errno) {
        case EDQUOT: {
            /* No per user limit on reading books
             */
            return ;
        }
        case ENOENT: {
            initial_error = BriErrLmfNoLicenseNum;
            break;
        }
        case ERANGE: {
            initial_error = BriErrLmfInvalidLicNum;
            break;
        }
        case ETIMEDOUT: {
            initial_error = BriErrLmfTerminatedNum;
            break;
        }
        case EDOM: {
            initial_error = BriErrLmfNotStartedNum;
            break;
        }
        case EFAULT: {
            initial_error = BriErrLmfBadParamNum;
            break;
        }
    }

    /* Check the alternate product names, for V1 books BOOKBROWSER has been
     * set up to be the alternate name by BriBookGetHeader().
     */
    while (alt_prod_name != NULL) {

        status = lmf_probe_license(alt_prod_name->string,
                                   vbh->LMF_producer,
                                   &vbh->LMF_product_version,
                                   release_date,
                                   0
                                   ) ;
        if (status == 0) {
            /* We are licensed to read this book.  Release all units.
             */
            lmf_release_license(alt_prod_name->string,
                                vbh->LMF_producer,
                                RELEASE_ALL
                                );
            return ;
        }
        if (errno == EDQUOT) {
            
            /* No per user limit on reading books
             */
            return;
        }

        alt_prod_name = alt_prod_name->next ;
    }

    BriLongJmp(book_context,initial_error);
#endif
#endif 
} /* end BriBookLmfCheck */


/* end bri_license.c */




