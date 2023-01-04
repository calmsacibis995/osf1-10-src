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
static char *rcsid = "@(#)$RCSfile: hyperhelp.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/24 19:22:56 $";
#endif
/*
*****************************************************************************
**                                                                          *
**                     COPYRIGHT (c) 1990, 1991 BY                          *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
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
*****************************************************************************
*/

/*
**
**  FACILITY:  DECwindows Motif Help System (HyperHelp)
**
**  DESCRIPTION:
**
**      This module contains routines which enable applications
**	to act as Bookreader (HyperHelp) clients. 
**
**  FUNCTIONS:
**
**      DXmHelpSystemOpen
**      DXmHelpSystemDisplay
**      DXmHelpSystemClose
**
**
**  AUTHOR:  Dan Gosselin, OSAG
**
**  CREATION DATE:  1-Mar-1991
**
**  MODIFICATION HISTORY:
**
**	3-Nov-1991	Modified old HyperHelp (DXmHelpSystem) routines
**			to call new callable Bookreader routines.
**	12-Nov-1991	Integrated routines into DXm layer.  Dynamically
**			activate the bookreader shareable, so we don't have
**			to link against it.
**			
*/

/* Include files */

#include <X11/Intrinsic.h>

/* Constants */

#define BkrNfilename            "Bkr_Filename"
#define BkrNfilePath		"Bkr_FilePath"
#define BkrNfileType		"Bkr_FileType"
#define BkrNfileSuffix		"Bkr_FileSuffix"
#define BkrNobject              "Bkr_Object"
#define BkrNobjectName          "Bkr_ObjectName"
#define BkrNwindowUsage         "Bkr_Window_Usage"
#define BkrNxPosition           "Bkr_X_Position"
#define BkrNyPosition           "Bkr_Y_Position"

#define Bkr_Success		    0
#define Bkr_Busy		    1
#define Bkr_Send_Event_Failure	    2
#define Bkr_Startup_Failure	    3
#define Bkr_Create_Client_Failure   4
#define Bkr_Invalid_Object          5
#define Bkr_Get_Data_Failure        6
#define Bkr_Bad_Filename            7

#define Bkr_Topic		    0
#define Bkr_Directory		    1
#define Bkr_Book		    2
#define Bkr_Widget		    3
  
#define Bkr_Default_Window          0
#define Bkr_New_Window              1


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

/* Bkr API declarations */

extern int BkrOpen PROTOTYPE((Opaque *, Widget, ArgList, int));
extern int BkrDisplay PROTOTYPE((Opaque, ArgList, int));
extern int BkrClose PROTOTYPE((Opaque, ArgList, int));

#ifdef VMS
#include "descrip.h"
#include "ssdef.h"
#include "libdef.h"
#include "rmsdef.h"
#endif


#ifdef VMS
typedef (*funcptr) ();

static funcptr Bkr_Open = NULL;
static funcptr Bkr_Display = NULL;
static funcptr Bkr_Close = NULL;

static Boolean bkr_found = False;
static Boolean bkr_looked_for = False;
#endif /* VMS */

#ifdef VMS
static int
exception_handler (sigargs, mchargs)
        unsigned long sigargs[];
        unsigned long mchargs[5];
{
    /*BOGUS should check to see if file-not-found or key-not-found before
       returning SS$_CONTINUE
     */
    return ( SS$_CONTINUE );

}

static funcptr locate_entry_point(func_addr, symbol_name)
    funcptr func_addr;
    char *symbol_name;
{

    /* case when func_addr is assigned already
     */
    if ( bkr_looked_for && func_addr != NULL )
    {
        return (func_addr);

    } else if ( !bkr_looked_for || ((func_addr == NULL ) && (bkr_found))) {

        long cond_value;
        struct dsc$descriptor_s func_symbol;

        $DESCRIPTOR(file_name,   "DECW$BKRSHR");

        func_symbol.dsc$w_length  = strlen(symbol_name);
        func_symbol.dsc$b_dtype   = DSC$K_DTYPE_T;
        func_symbol.dsc$b_class   = DSC$K_CLASS_S;
        func_symbol.dsc$a_pointer = symbol_name;

        /* not tried yet, or func_addr == NULL, try it now
         */
        if ( (!bkr_looked_for) || ((bkr_looked_for) && (bkr_found)) )
        {

             if (!bkr_looked_for)
             {
                VAXC$ESTABLISH ( exception_handler );
             }
             bkr_looked_for = True;

/*  Notice that this is missing the optional image_name parameter.  This is
    because it should not be used when finding a symbol, since it causes a
    disk load. */
             cond_value = LIB$FIND_IMAGE_SYMBOL ( &file_name,
                                                  &func_symbol,
                                                  &func_addr);

             /* symbol exists, return
              */
             if ( cond_value == SS$_NORMAL )
             {
                 bkr_found = True;
                 return (func_addr);

             } else {
                 LIB$REVERT();  /* no image found, reset the exception handler
                                 */
                 bkr_found = False;
                 return (NULL);
             }

        } else {     /* image not exist, return */

               bkr_found = False;
               return (NULL);
        }

    } else {

        return (NULL);
    }
}
#endif /* VMS */

static void evaluate_status ( status, routine, tag )
int      status;
void     ( (*routine) () );
char	*tag;
{    
    switch ( status )
    {
        case Bkr_Success:               break;
        case Bkr_Busy:                  break;
        case Bkr_Send_Event_Failure:    { routine( tag, status ); break; }
        case Bkr_Startup_Failure:       { routine( tag, status ); break; }
        case Bkr_Create_Client_Failure: { routine( tag, status ); break; }
        case Bkr_Invalid_Object:        { routine( tag, status ); break; }
        case Bkr_Bad_Filename:          { routine( tag, status ); break; }
        case Bkr_Get_Data_Failure:      routine( tag, status );
    }
}


void DXmHelpSystemOpen( help_context, parent, help_file, routine, tag )
Opaque	*help_context;
Widget   parent; 
char    *help_file;
void     ( (*routine) () );
char    *tag;
{
    int status;

    Arg BkrArgs[1];

    XtSetArg( BkrArgs[0], BkrNfilename, help_file );

#ifdef VMS
    if (!bkr_found)
    {
        Bkr_Open = locate_entry_point(Bkr_Open, "BkrOpen");
        Bkr_Display = locate_entry_point(Bkr_Display,"BkrDisplay");
        Bkr_Close = locate_entry_point(Bkr_Close, "BkrClose");
    }

    if (!Bkr_Open)
       return;

    status = (*Bkr_Open)( help_context, parent, BkrArgs, XtNumber(BkrArgs) );
#else /* VMS */
    status = BkrOpen( help_context, parent, BkrArgs, XtNumber(BkrArgs) );
#endif /* VMS */
    if ( routine != NULL )
	evaluate_status( status, routine, tag );
}

void DXmHelpSystemDisplay( help_context, help_file, keyword, name, routine, tag)
Opaque	 help_context;
char    *help_file;
char    *keyword;
char    *name;
void     ( (*routine) () );
char    *tag;
{
    int	    status;
    char    keyword2[8];
    char    *cp;
    int     argc = 2;

    Arg BkrArgs[3];

    XtSetArg( BkrArgs[0], BkrNobject, NULL );
    XtSetArg( BkrArgs[1], BkrNobjectName, name );
    if (help_file && (*help_file != '\000')) {
        XtSetArg( BkrArgs[2], BkrNfilename, help_file );
        argc++;
    }

#ifdef VMS
    if (!Bkr_Display)
       return;

#endif /* VMS */
    strncpy( keyword2, keyword, 8 );

    /* set keyword to lower case for comparison */

    for ( cp = keyword2; *cp; cp++ )
        *cp = tolower( *cp );

    /* if ( keyword == "topic" ) */

    if ( strncmp( keyword2, "topic", 8 ) == 0 )

	BkrArgs[0].value = (XtArgVal)Bkr_Topic;

    else  /* keyword == "dir" */

	BkrArgs[0].value = (XtArgVal)Bkr_Directory;    

#ifndef VMS
    status = BkrDisplay( help_context, BkrArgs, argc);
#else   /* VMS */
    status = (*Bkr_Display)( help_context, BkrArgs, argc );
#endif  /* VMS */

    if ( routine != NULL )
	evaluate_status( status, routine, tag );
}


void DXmHelpSystemClose( help_context, routine, tag )
Opaque	 help_context;
void     ( (*routine) () );
char    *tag;
{
    int	status;

#ifndef VMS
    status = BkrClose( help_context, NULL, 0 );
#else   /* VMS */
    if (!Bkr_Close)
       return;

    status = (*Bkr_Close)( help_context, NULL, 0 );
#endif /* VMS */

    if ( routine != NULL )
	evaluate_status( status, routine, tag );
}

