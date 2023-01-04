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
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: %W% %E%";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

/*
 * This is the main program for WML. It declares all global data structures
 * used during a compilation, and during output.
 */

/*
 * WML is a semi-standard Unix application. It reads its input from
 * stdin, which is expected to be a stream containing a WML description
 * of a UIL language. If this stream is successfully parsed and semantically
 * validated, then WML writes a series of standard .h and .dat files into
 * the user directory. The .h files will be used directly to construct
 * the UIL compiler. The .dat files are used by other phases of UIL
 * table generation.
 *
 * The files created by WML are:
 *
 *	.h files:
 *		UilSymGen.h
 *		UilSymArTy.h
 *		UilSymRArg.h
 *		UilDrmClas.h
 *		UilConst.h
 *		UilSymReas.h
 *		UilSymArTa.h
 *		UilSymCtl.h
 *		UilSymNam.h
 *	.dat files
 *		argument.dat
 *		reason.dat
 *		grammar.dat
 *	.mm files
 *		wml-uil.mm
 */

#if defined(DEC_MOTIF_BUG_FIX)
#include <stdlib.h>
#endif

#include "wml.h"

#ifdef SYSV
#include <fcntl.h>
#else
#include <sys/file.h>
#endif



/*
 * Globals used during WML parsing.
 *
 * WML uses globals exclusively to communicate data during parsing. The
 * current object being constructed is held by these globals, and all
 * routines called from the parse assume correct setting of these globals.
 * This simplisitic approach is possible since the WML description language
 * has no recursive constructs requiring a frame stack.
 */

/*
 * Error and other counts
 */
int		wml_err_count = 0;	/* total errors */
int		wml_line_count = 0;	/* lines read from input */

/*
 * Dynamic ordered vector of all objects encountered during parse. This
 * is used to detect name collisions, and is the primary order vector
 * used for all other vectors constructed curing the semantic resolution
 * phase of processing.
 */
DynamicHandleListDef	wml_synobj;
DynamicHandleListDefPtr	wml_synobj_ptr = &wml_synobj;


/*
 * Dynamic vectors of vectors partitioned and ordered
 * as required by the semantic processing and output routines. All
 * point to resolved objects rather than syntactic objects.
 */
DynamicHandleListDef	wml_obj_datatype;	/* datatype objects */
DynamicHandleListDefPtr	wml_obj_datatype_ptr = &wml_obj_datatype;

DynamicHandleListDef	wml_obj_enumval;	/* enumeration value objects */
DynamicHandleListDefPtr	wml_obj_enumval_ptr = &wml_obj_enumval;

DynamicHandleListDef	wml_obj_enumset;	/* enumeration set objects */
DynamicHandleListDefPtr	wml_obj_enumset_ptr = &wml_obj_enumset;

DynamicHandleListDef	wml_obj_reason;	/* reason resource objects */
DynamicHandleListDefPtr	wml_obj_reason_ptr = &wml_obj_reason;

DynamicHandleListDef	wml_obj_arg;	/* argument resource objects */
DynamicHandleListDefPtr	wml_obj_arg_ptr = &wml_obj_arg;

DynamicHandleListDef	wml_obj_allclass;	/* metaclass, widget, gadget */
DynamicHandleListDefPtr	wml_obj_allclass_ptr = &wml_obj_allclass;

DynamicHandleListDef	wml_obj_class;		/* widget & gadget objects */
DynamicHandleListDefPtr	wml_obj_class_ptr = &wml_obj_class;

DynamicHandleListDef	wml_obj_ctrlist;	/* controls list objects */
DynamicHandleListDefPtr	wml_obj_ctrlist_ptr = &wml_obj_ctrlist;

DynamicHandleListDef	wml_obj_charset;	/* charset objects */
DynamicHandleListDefPtr	wml_obj_charset_ptr = &wml_obj_charset;

DynamicHandleListDef	wml_tok_sens;		/* case-sensitive tokens */
DynamicHandleListDefPtr	wml_tok_sens_ptr = &wml_tok_sens;

DynamicHandleListDef	wml_tok_insens;		/* case-insensitive tokens */
DynamicHandleListDefPtr	wml_tok_insens_ptr = &wml_tok_insens;


/*
 * Routines only accessible in this module
 */
void wmlInit ();

/*
 * External variables
 */
extern	int	yyleng;




/*
 * The WML main routine:
 *
 *	1. Initialize global storage
 *	2. Open the input file if there is one
 *	3. Parse the WML description in stdin. Exit on errors
 *	4. Perform semantic validation and resolution. Exit on errors.
 *	5. Output files
 */

int main (argc, argv)
    int		argc;
    char	**argv;

{

int		fd;		/* input file descriptor */

/*
 * Initialize storage
 */
wmlInit ();

/*
 * Assume that anything in argv must be an input file. Open it, and
 * dup it to stdin
 */
if ( argc > 1 )
    {
    if ( (fd=open(argv[1],O_RDONLY)) == -1 )
	printf ("\nCouldn't open file %s", argv[1]);
    else
	dup2 (fd, 0);
    }

/*
 * Process the input
 */
while ( TRUE )
    {
    
    /*
     * Parse the input stream
     */
    yyleng = 0;		/* initialization safety */
    yyparse ();
    if ( wml_err_count > 0 ) break;
    printf ("\nParse of WML input complete");
    
    /*
     * Perform semantic validation, and construct resolved data structures
     */
    wmlResolveDescriptors ();
    if ( wml_err_count > 0 ) break;
    printf ("\nSemantic validation and resolution complete");
    
    /*
     * Output 
     */
    wmlOutput ();
    if ( wml_err_count > 0 ) break;
    printf ("\nWML Uil*.h and wml-uil.mm file creation complete\n");
    
    break;
    }

/*
 * Report inaction on errors
 */
if ( wml_err_count > 0 )
    {
    printf ("\nWML found %d errors, no or incomplete output produced\n",
	    wml_err_count);
#ifdef DEC_MOTIF_BUG_FIX
    exit (1);
    }

exit (0);
#else
    return (1);
    }

return (0);
#endif

}


/*
 * Routine to initialize WML.
 *
 * The main job is to dynamically allocate any dynamic lists to a reasonable
 * initial state.
 */
void wmlInit ()

{

/*
 * Initialize the list of all syntactic objects
 */
wmlInitHList (wml_synobj_ptr, 1000, TRUE);

}

yywrap()
{
return(1);
}
