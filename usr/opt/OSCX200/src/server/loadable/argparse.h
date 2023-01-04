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
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1992 BY            			    *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/

#ifndef ARGPARSE_H
#define ARGPARSE_H

/* This is similar to the xrm stuff, but a bit different and specialized
 * for us. We're not interested in resource matching, just parsing the
 * command line options.
 */
typedef enum {
    apChar,
    apShort,
    apInt,
    apLong,
    apPointer
} apValueType;

typedef enum { 
    apOptionNoArg,	/* value is specified
    apOptionNoArg,	/* Value is specified in OptionDescRec.value	    */
    apOptionIsArg,     	/* Value is the option string itself		    */
    apOptionStickyArg, 	/* Value is characters immediately following option */
    apOptionSepArg,    	/* Value is next argument in argv		    */
    apOptionSkipArg,   	/* Ignore this option and the next argument in argv */
    apOptionSkipLine,  	/* Ignore this option and the rest of argv	    */
    apOptionSkipNArgs	/* Ignore this option and the next 
			   OptionDescRes.value arguments in argv */
} apOptionKind;;

typedef struct {
    char	    	* option;   /* Option abbreviation in argv	    */
    apOptionKind   	argKind;    /* Which style of option it is	    */
    apValueType		type;       /* type of storate in value 	    */
    void		* defValue; /* Value to provide if XrmoptionNoArg   */
    void		* value;    /* return value			    */
} apOptionDescRec, *apOptionDescList;

#endif /* ARGPARSE_H */
