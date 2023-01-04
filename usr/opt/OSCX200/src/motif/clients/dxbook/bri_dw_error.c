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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_DW_ERROR.C*/
/* *7    22-MAR-1993 14:31:35 BALLENGER "Fix crash when directory target is not valid chunk."*/
/* *6    12-AUG-1992 13:53:52 ROSE "Added new error messages for ISO support"*/
/* *5    19-JUN-1992 20:16:18 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *4     3-MAR-1992 17:10:41 KARDON "UCXed"*/
/* *3    18-SEP-1991 20:18:24 BALLENGER "included .c instead of .h"*/
/* *2    17-SEP-1991 21:09:07 BALLENGER "change use of major and minor in version number to avoid conflict with sys/types.h*/
/*definition on ULTRIX"*/
/* *1    16-SEP-1991 12:44:05 PARMENTER "Generic Error Handling"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_DW_ERROR.C*/
#ifndef VMS
 /*
#else
# module BRI_DW_ERROR "V03-0002"
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
**      BRI error handling routine
**
** Functions:
**
**      BriLongJmp - Routine to long jump out of an internal BRI
**                   routine back to the calling public BRI routine.
**                   This routine will get and format the message
**                   for the condition detected by the calling
**                   before calling longjmp().
**
**      BriError - Gets, formats, and displays a message for a BRI
**                 routine when it detects an error.
**
** Author:
**
**      David L. Ballenger
**
** Date:
**
**      Fri Oct 13 19:30:34 1989
**
** Revision History:
**
**  V03-0002    DLB0002     David L Ballenger           04-Apr-1991
**              Adn LMF related error support.
**
**  V03-0001    DLB0001     David L Ballenger           06-Feb-1991
**              Conditionalize system use of system specifc file name
**              access, and add NULL argument to call to bkr_error_modal().
**
**
**  V03-0000   	JAF0000	    James A. Ferguson	    	16-Aug-1990
**  	      	Create new module, modify revision history format and
**  	    	update copyright.
**
**  	    	DLB 	    David L Ballenger	    	30-May-1990 
**             	Cleanup (i.e. remove most contionaliztaion) include
**             	files for new VMS standards.
**
**  	    	DLB 	    David L Ballenger	    	05-Jul-1990 
**             	Incorporate changes from Jim Ferguson to handle unknown
**             	error strings and use new bkr error routines.
 */


#include <errno.h>
#include <string.h>
#include <varargs.h>
#include <X11/Intrinsic.h>
#include <Mrm/MrmPublic.h>
#include <Xm/Xm.h>
#include "br_common_defs.h"	/* Common bookreader defines */
#include "bkr_error.h"		/* Routines for displaying error messages */
#include "bkr_fetch.h"		/* Routines to fetch resoruce literals */
#include "bri_private_def.h"	/* Private bri definitions */

static char	*unknown_string = NULL;
static char	*unknown_string_text = "???" ;

static Boolean	STATIC_all_extensions_tried = FALSE;


void 
BriLongJmp(va_alist)
     va_dcl
/*
 *
 * Function description:
 *
 *      Temporary routine for handling BXI/BRI errors.      
 *
 * Arguments:
 *
 *      Variable list of arguments,  the first two are always:
 *
 *      	o context pointer for a book or shelf
 *
 * 		o error number
 *
 *      The rest depend on the error.
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
    va_list ap;
    register BRI_CONTEXT *env;
    register BR_UINT_32 error;
    register char *msg;
    BR_UINT_32  u_arg1,
                 u_arg2;

    
    /* Names of the UIL literals for the error messages.
     */
    static char *error_name[] = {
        NULL,
        "BriErrBadPageIndex",   /*  1 */
        "BriErrBadVersion",     /*  2 */
        "BriErrRecUnpack",      /*  3 */
        "BriErrBookPos",        /*  4 */
        "BriErrPageRead",       /*  5 */
        "BriErrDirRead",        /*  6 */
        "BriErrInvalidDrid",    /*  7 */
        "BriErrFontRead",       /*  8 */
        "BriErrChkStrRead",     /*  9 */
        "BriErrBadPageId",      /* 10 */
        "BriErrFileSearch",     /* 11 */
        "BriErrNoPool",         /* 12 */
        "BriErrNoMemory",       /* 13 */
        "BriErrInvalidMem",     /* 14 */
        "BriErrShelfRead",      /* 15 */
        "BriErrBadShelfEntry",  /* 16 */
        "BriErrFieldRead",      /* 17 */
        "BriErrBadPgType",      /* 18 */
        NULL,                   /* 19 */
        NULL,                   /* 20 */
        "BriErrLmfNoLicense",   /* 21 */
        "BriErrUnknownFile",    /* 22 */
        "BriErrFileOpen",       /* 23 */
        "BriErrLmfNotStarted",  /* 24 */
        "BriErrLmfInvalidDate", /* 25 */
        "BriErrLmfTerminated",  /* 26 */
        "BriErrLmfInvalidVers", /* 27 */
        "BriErrLmfInvalidHwId", /* 28 */
        "BriErrLmfBadParam",    /* 29 */
        "BriErrLmfExEnqlm",     /* 30 */
        "BriErrLmfInvalidLic",  /* 31 */
        "BriErrFileOpenAny"     /* 32 */
    } ;

    /* Fetch the unknown msg string first */

    if (unknown_string == NULL)
    {
    	unknown_string = bkr_fetch_literal("BriUnknownMsg",ASCIZ);
    	if (unknown_string == NULL)
    	    unknown_string = unknown_string_text;
    }

    /* Get the context and the error.
     */
    va_start(ap);
    env = va_arg(ap,BRI_CONTEXT *);
    error = va_arg(ap, BR_UINT_32);

    /* Do error specific message formmatting.
     */
    switch (error) {
        case BriErrBadPageIndexNum:
        case BriErrRecUnpackNum:
        case BriErrBookPosNum:
        case BriErrDirReadNum:
        case BriErrInvalidDridNum:
        case BriErrFontReadNum:
        case BriErrChkStrReadNum:
        case BriErrBadPageIdNum:
        case BriErrNoMemoryNum:
        case BriErrInvalidMemNum:
        case BriErrFieldReadNum:
        case BriErrBadShelfIdNum:
        case BriErrBadBookIdNum:
	case BriErrLmfNotStartedNum:
	case BriErrLmfBadParamNum:
	case BriErrLmfExEnqlmNum:
        case BriErrUnknownFileNum: {
            msg = bkr_fetch_literal(error_name[error],ASCIZ);
    	    if (msg == NULL)
    	    	msg = unknown_string;
            sprintf(env->reason,msg);
            break ;
        }

        case BriErrLmfNoLicenseNum:
	case BriErrLmfInvalidDateNum:
	case BriErrLmfTerminatedNum:
	case BriErrLmfInvalidVersNum:
	case BriErrLmfInvalidHwIdNum:
	case BriErrLmfInvalidLicNum: {
            msg = bkr_fetch_literal(error_name[error],ASCIZ);
    	    if (msg == NULL) {
    	    	msg = unknown_string;
                sprintf(env->reason,msg);
            } else {
                sprintf(env->reason,
                        msg,
                        env->data.book->vbh.LMF_product_name,
                        env->data.book->vbh.LMF_producer
                        );
            }
            break ;
        }

        case BriErrBadVersionNum:
        {
            BMD_VERSION *version ;
            version = va_arg(ap,BMD_VERSION *);
            msg = bkr_fetch_literal(error_name[error],ASCIZ);
    	    if (msg == NULL)
    	    	msg = unknown_string;
            sprintf(env->reason,msg,version->major_num,version->minor_num);
            break ;
        }

        case BriErrShelfReadNum: 
        case BriErrBadShelfEntryNum:
        case BriErrPageReadNum:
        {
            u_arg1 = va_arg(ap,unsigned int);
            msg = bkr_fetch_literal(error_name[error],ASCIZ);
    	    if (msg == NULL)
    	    	msg = unknown_string;
            sprintf(env->reason,msg,u_arg1);
            break ;
        }

        case BriErrBadPgTypeNum:
        {
            u_arg1 = va_arg(ap,unsigned int);
            u_arg2 = va_arg(ap,unsigned int);
            msg = bkr_fetch_literal(error_name[error],ASCIZ);
    	    if (msg == NULL)
    	    	msg = unknown_string;
            sprintf(env->reason,msg,u_arg1,u_arg2);
            break ;
        }

        case BriErrFileSearchNum:
        case BriErrFileOpenNum: {
            int status;
            
            status = va_arg(ap, int);

#ifdef vms
            msg = strerror(EVMSERR,status);
#else
            msg = strerror(status);
#endif 
            if (msg == NULL) {
                msg = unknown_string;
            }
            strcpy(env->reason,msg);
            break ;
        }

        case BriErrFileOpenAnyNum: {
            int status;
            
            status = va_arg(ap, int);

#ifdef vms
            msg = strerror(EVMSERR,status);
#else
            msg = strerror(status);
#endif 
            if (msg == NULL) {
                msg = unknown_string;
            }
            strcpy(env->reason,msg);

	    STATIC_all_extensions_tried = TRUE;

            break ;
        }

        default:
        {
            msg = bkr_fetch_literal("BriErrUnknown",ASCIZ);
    	    if (msg == NULL)
    	    	msg = unknown_string;
            sprintf(env->reason,msg,error);
            break;
        }
    }

    if (msg)
    	BXI_FREE(msg);
    va_end(ap);


    /* Long jump to the public routine that set up the jump buffer.
     */
    longjmp(env->jump_buffer,error);

} /* end BriLongJmp */


void
BriError(va_alist)
     va_dcl
/*
 *
 * Function description:
 *
 *      Displays an error message using bkr_error_modal().
 *
 * Arguments:
 *
 *      Variable, the first two are always
 *
 *     		o Address of the context block for the book or shelf which
 * 		  had the error.
 *
 * 		o The error number.
 *
 * 	The rest vary based on the error().
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
    va_list ap;
    register BRI_CONTEXT *env;
    register BR_UINT_32 error;
    char buffer[1000], *period_pos;
    register char *literal;
    register char *filename;

    /* Names of the UIL literals for the error message text.
     */
    static char *err_name[] = {
        NULL,
        "BriErrShelfOpen",	/*  1 */
        "BriErrBookOpen",	/*  2 */
        "BriErrDirOpen",	/*  3 */
        "BriErrPageOpen",	/*  4 */
        "BriErrBadChkId",	/*  5 */
        "BriErrChkSym",		/*  6 */
        "BriErrPageClose",	/*  7 */
        "BriErrBookClose",	/*  8 */
        "BriErrNoLmf",		/*  9 */
        NULL,			/* 10 */
        "BriErrLibraryOpen",	/* 11 */
        NULL,
        "BriErrLibraryOpenAny",	/* 13 */
        "BriErrBadDirTarget"    /* 14 */
    } ;

    static char *special_err_name[] = {
        NULL,
        "BriErrShelfOpenAny",	/*  1 */
        "BriErrBookOpenAny"	/*  2 */
    } ;

    /* Fetch the unknown msg string first */

    if (unknown_string == NULL)
    {
    	unknown_string = bkr_fetch_literal("BriUnknownMsg",ASCIZ);
    	if (unknown_string == NULL)
    	    unknown_string = unknown_string_text;
    }

    /* Get the context and error number.
     */
    va_start(ap);
    env = va_arg(ap,BRI_CONTEXT *);
    error = va_arg(ap, unsigned int);

    /* Get the file name from the context.  First try to use the 
     * found_file_spec, i.e. result file spec.  Next try the 
     * parsed_file_spec, i.e. expanded file spec. If that's not present 
     * use the target file, and if that isn't present use the unkown string.
     */
    if (env->found_file_spec[0]) {
        filename = &env->found_file_spec[0];
#ifdef vms
    } else if (env->parsed_file_spec[0]) {
    	filename = &env->parsed_file_spec[0];
#else
    } else if (env->entry.target_file) {
        filename = env->entry.target_file;
#endif 
    } else {
        filename = unknown_string;
    }

    /* Do error specific message formatting.
     */
    switch (error) {
        case BriErrShelfOpenNum:
        case BriErrBookOpenNum: {

	    /* If all extension were tried, put up a different error message */
	    if (STATIC_all_extensions_tried) {
		literal = bkr_fetch_literal(special_err_name[error],ASCIZ);

		/* Don't include the extension with the file name, since we
		   tried all 3 extensions */
		period_pos = strrchr (filename, PERIOD);
		if (period_pos != NULL)
		    *period_pos = NULL_CHAR;

		STATIC_all_extensions_tried = FALSE;
	    }
	    else
		literal = bkr_fetch_literal(err_name[error],ASCIZ);

    	    if (literal == NULL)
    	    	break;

            sprintf(buffer,literal,
    	    	    (env->entry.title)       ? env->entry.title       : unknown_string,
                    filename,
                    (env->reason)   	     ? env->reason  	      : unknown_string
                    ) ;
            break ;
        }

        case BriErrDirOpenNum:
        case BriErrPageOpenNum:
        case BriErrChkSymNum:
        case BriErrPageCloseNum:
        case BriErrBookCloseNum: {

	    literal = bkr_fetch_literal(err_name[error],ASCIZ);

    	    if (literal == NULL)
    	    	break;

            sprintf(buffer,literal,
    	    	    (env->entry.title)       ? env->entry.title       : unknown_string,
                    filename,
                    (env->reason)   	     ? env->reason  	      : unknown_string
                    ) ;
            break ;
        }

        case BriErrBadChkIdNum:
        {
            literal = bkr_fetch_literal(err_name[error],ASCIZ);
    	    if (literal == NULL)
    	    	break;
            sprintf(buffer,literal,
    	    	    (env->entry.title)	     ? env->entry.title       : unknown_string,
                    filename
    	    	    );
            break ;
        }
        case BriErrLibraryOpenNum:
        {
            literal = bkr_fetch_literal(err_name[error],ASCIZ);
    	    if (literal == NULL)
    	    	break;
            sprintf(buffer,literal,filename);
            break ;
        }
        case BriErrNoLmfNum:
        {
            char *sysname;
            char *version;

            sysname = va_arg(ap, char *);
            version = va_arg(ap, char *);

            literal = bkr_fetch_literal(err_name[error],ASCIZ);
    	    if (literal == NULL)
    	    	break;
            sprintf(buffer,
                    literal,
                    (sysname) ? sysname : unknown_string,
                    (version) ? version : unknown_string,
    	    	    (env->entry.title)	     ? env->entry.title       : unknown_string,
                    filename
    	    	    ) ;
            break ;
        }

        case BriErrDirEntryNoText:
	{
	    literal = bkr_fetch_literal("UNTITLED_ENTRY_MSG",ASCIZ);
	    if (literal == NULL)
                break;
	    sprintf(buffer,literal);
	    break;
	}        

        case BriErrLibraryOpenAnyNum:
        {
            literal = bkr_fetch_literal(err_name[error],ASCIZ);
    	    if (literal == NULL)
    	    	break;

	    /* Don't include the extension with the file name, since we
	       tried all 3 extensions */
	    period_pos = strrchr (filename, PERIOD);
	    if (period_pos != NULL)
		*period_pos = NULL_CHAR;

            sprintf(buffer,literal,filename);
            break;
        }
        case BriErrBadDirTargetNum: 
        {
            char *msg = va_arg(ap, char *);

            literal = bkr_fetch_literal(err_name[error],ASCIZ);
    	    if (literal == NULL)
            {
    	    	break;
            }
            sprintf(buffer,
                    literal,
                    msg,
    	    	    env->data.book->vbh.book_name,
                    filename
    	    	    ) ;
            break;
        }
        
        default: {
            literal = bkr_fetch_literal("BriErrUnknown",ASCIZ);
    	    if (literal == NULL)
    	    	break;
            sprintf(buffer,literal,error);
            break;
        }
    }

    if (literal)
    	BXI_FREE(literal);

    /* Display the error.
     */
    bkr_error_modal(buffer,NULL);

    va_end(ap);

} /* end BriError */

/* end bri_dw_error.c */


