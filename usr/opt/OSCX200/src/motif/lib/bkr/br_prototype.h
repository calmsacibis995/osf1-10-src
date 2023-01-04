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
/* DEC/CMS REPLACEMENT HISTORY, Element BR_PROTOTYPE.H*/
/* *5    19-JUN-1992 17:27:24 BALLENGER "Add #ifndef to allow multiple includes."*/
/* *4     8-JUN-1992 19:16:25 BALLENGER "UCX$CONVERT"*/
/* *3     8-JUN-1992 13:33:29 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *2     3-MAR-1992 17:12:27 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:48:40 PARMENTER "function prototype setup"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BR_PROTOTYPE.H*/
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

/*--- just define prototype, this file gets included by *_fp.h
      files doing function prototyping ---*/
#ifndef BR_PROTOTYPE_H
#define BR_PROTOTYPE_H

#if defined (VAXC) || defined(__STDC__) || defined(USE_PROTOS)
#define PROTOTYPE(args) args
#define PARAM_NAMES(names) (
#define PARAM_SEP ,
#define PARAM_END )
#define VOID_PARAM void
#else
#define PROTOTYPE(args) ()
#define PARAM_NAMES(names) names
#define PARAM_SEP ;
#define PARAM_END ;
#define VOID_PARAM 
#endif




#endif /* BR_PROTOTYPE_H */
