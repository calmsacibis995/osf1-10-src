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
#ifdef CBR
/****************************************************************************/
/*                                                                          */  
/*  Copyright (c) Digital Equipment Corporation, 1991                       */
/*  All Rights Reserved.  Unpublished rights reserved                       */
/*  under the copyright laws of the United States.                          */
/*                                                                          */  
/*  The software contained on this media is proprietary                     */
/*  to and embodies the confidential technology of                          */
/*  Digital Equipment Corporation.  Possession, use,                        */
/*  duplication or dissemination of the software and                        */
/*  media is authorized only pursuant to a valid written                    */
/*  license from Digital Equipment Corporation.                             */
/*                                                                          */  
/*  RESTRICTED RIGHTS LEGEND   Use, duplication, or                         */
/*  disclosure by the U.S. Government is subject to                         */
/*  restrictions as set forth in Subparagraph (c)(1)(ii)                    */
/*  of DFARS 252.227-7013, or in FAR 52.227-19, as                          */
/*  applicable.                                                             */
/*                                                                          */  
/****************************************************************************/
/*.
**. Name:  DXmCbrUtils.c
**.
**. Abstract:   Utility routines for cbr Motif sample application
**.             
**.
**. Table of Contents:
**.
**.
**  Comments:   1.  Use the following definitions to control compilation:
**
**                  a. UNITTEST - build the unit test driver code
**                                (only compiled when unit testing.)
**
**                  b. LIBRARY  - build the library object code.
**
**                  c. DEBUG    - build the debug display code
**
**              2.  Module format:
**
**                  The standard group conventions of argument tagging, and 
**                  tagging for documentation are followed,
**
**                  a.  Function arguments are tagged as r, w, or m.
**                      This corresponds to the VMS convention of read only, 
**                      write only, or modify arguments.
**
**                  b.  Function descriptive text is marked up for automatic
**                      extraction using the convention of:
**
**                      <ASTERISK><ASTERISK><PERIOD> for public documentation,
**                      <ASTERISK><ASTERISK><COMMA>  for internal documentation
**
**  AUTHORS:
**
**      Robert L. Cohen
**      Daniel Leroux
**
**  CREATION DATE:   Oct. 18, 1990   
**
**  Change history:
**
**  Version     Date        Author          Change
**  -------     ----        ------          ------
**. 1.0         25-Apr-1991 R. L. Cohen     Incorporated CBR Motif Widgets
*/

/******************************************************************************/
/*                                                                            */
/*                   INCLUDE FILES and LOCAL DEFINITIONS                      */
/*                                                                            */
/******************************************************************************/

#include <cbr_public.h>
#include <DXmCbr/dxmcbrappldefs.h>
#include "bkr_cbr.h"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unixlib.h>

#ifdef VMS
#include <decw$cursor>
#include <descrip.h>
#include <ssdef.h>
#include <iodef.h>
#include <clidef.h>
#include <decw$include/vendor.h>       
#else
#include <X11/vendor.h>                
#endif

/*===========================*/
/* external references       */
/*===========================*/

extern GLOBAL_CBR_DATA    *cbrdata;

static void MessageAcknowledged( Widget w, int *tag, unsigned long *reason );

MrmRegisterArg  mess_reglist[] = 
{
    { "MessageAcknowledged", (caddr_t)MessageAcknowledged }
};

int mess_reglist_num = ( sizeof( mess_reglist ) / sizeof( mess_reglist[0] ) );


/*===============================================================*/
/* This routine transforms a Compound String to an ascii string. */
/*===============================================================*/

char *CSToAsciz( XmString cstring, char *ascii_string )
{
    XmStringContext         context;
    char                    *ptr;
    int                     status;
    XmStringCharSet         charset;
    XmStringDirection       direction;
    Boolean                 separator;
     
    if( ( status = XmStringInitContext( &context, cstring ) ) != TRUE )
            return( NULL );

    ascii_string[0] = '\0';         /* set end of string at beginning */

    while( ( status = XmStringGetNextSegment( context, 
                                                &ptr,
                                                &charset, 
                                                &direction,
    				                &separator ) ) ) {
        if( status != TRUE ) {
            XmStringFreeContext( &context );
            return( NULL );
        }
 
        strcat( ascii_string, ptr );
        XtFree( ptr );
    }


    XmStringFreeContext( &context );

    return( ascii_string );
}

/*========================================================*/
/* make work in progress cursor                           */
/*========================================================*/

void MakeWorkCursor( cursor_wid, work_cursor )

Widget  cursor_wid;	                /*  Widget to display wait cursor in  */
			                        /*  (Usually application main window) */
Cursor  *work_cursor;	            /*  Cursor to create */
{
   Display  *cur_display;
   int      scr_id;
   Colormap defmap;
   Font     cur_font;
   XColor   background,foreground,exactcolor;
   int      status;

   cur_display = XtDisplay( cursor_wid );
   scr_id      = XDefaultScreen( cur_display );
   defmap      = XDefaultColormap( cur_display, scr_id );

   cur_font = XLoadFont( cur_display, "DECw$Cursor" );

   status   = XAllocNamedColor( cur_display, 
                                defmap,
                                "White", 
                                &background,
                                &exactcolor );

   status   = XAllocNamedColor( cur_display, 
                                defmap,
                                "Black",
                                &foreground,
                                &exactcolor );

   *work_cursor =  XCreateGlyphCursor( cur_display, 
                                       cur_font,
                                       cur_font,
                                       decw$c_wait_cursor, 
                                       decw$c_wait_cursor+1,
                                       &foreground, 
                                       &background);
  return;
}

    
/*========================================================*/
/*                                                        */
/* display_work_cursor                                    */
/*                                                        */
/*  This routine displays a work in progress              */
/*  cursor, and initiates toolkit filtering               */
/*  of input  events                                      */
/*========================================================*/

void DisplayWorkCursor( display_wid, grab_wid, cursor )

Widget  display_wid;	/*  Widget to display wait cursor in
			    (Usually application main window) */
Widget  grab_wid;	/*  Widget to use in call to XtAddGrab.
			    This widget will be the only widget
			    to have all events passed to it.
			    It is easiest to use an unmapped widget.*/
Cursor  cursor;	        /*  Cursor to display */
{

/*
 *  Use XDefineCursor to set the window to the supplied cursor
 */
   XDefineCursor( XtDisplay( display_wid ), 
                    XtWindow( display_wid ), cursor );

/*
 *  Use XtAddGrab to enable toolkit filtering of events
 */

   XtAddGrab( grab_wid, TRUE, FALSE );

   XFlush( XtDisplay( display_wid ) );

}

/*========================================================*/
/*                                                        */
/* display_work_cursor                                    */
/*                                                        */
/*  This routine undisplays a work in progress            */
/*  cursor, and initiates toolkit filtering               */
/*  of input  events                                      */
/*========================================================*/
void RemoveWorkCursor( display_wid, grab_wid )

Widget  display_wid;	/*  Same two widgets used in display_work_cursor() */
Widget  grab_wid;		
{

/*
 *  Use XUndefineCursor to undo the cursor
 */

   XUndefineCursor( XtDisplay( display_wid ), XtWindow( display_wid ) );

/*
 *  Use XtRemoveGrab to disable toolkit filtering of events
 */

   XtRemoveGrab( grab_wid );
}

/*========================================================*/
/* simple error routine                                   */
/*========================================================*/

void SError( char *problem_string )
{
    printf( "%s\n", problem_string );
    exit( 1 );
}

/*========================================================*/
/* This function drives a generic message box which can   */
/* be used to display messages onto the screen.           */
/*========================================================*/
void DoMessage( char *message )
{
    Arg         a[3];
    XmString    label = XmStringLtoRCreate(message , "ISO8859-1");

    if( cbrdata->message_box == NULL ) 
    {
        MrmRegisterNames( mess_reglist, mess_reglist_num );
        if( MrmFetchWidget( cbrdata->CBRMRMHierarchy, 
                            "mess_mb",
                            cbrdata->TopWidget, 
                            &(cbrdata->message_box), 
                            &(cbrdata->DummyClass)) != MrmSUCCESS ) 
        {
            SError( "can't fetch message box\n" );
        }
    }
    XtSetArg( a[0], XmNx, cbrdata->TopWidget->core.x+MESSAGE_OFFSET );
    XtSetArg( a[1], XmNy, cbrdata->TopWidget->core.y+MESSAGE_OFFSET );
    XtSetArg( a[2], XmNmessageString, label );
    XtSetValues( cbrdata->message_box, a, 3 );
    XtManageChild( cbrdata->message_box );

    XtFree( label );
}


/*==========================================================*/
/* Activate callback for acknowledged button of message box */
/*==========================================================*/

static void MessageAcknowledged( Widget w, int *tag, unsigned long *reason )
{
    XtUnmanageChild( cbrdata->message_box );
}

/*========================================================*/
/* free_cs_list - free list of CS strings                 */
/*========================================================*/

int FreeCSList( XmString *array[], int num_items )
{
   int i = num_items;

   for ( i = num_items ; (i) ; i--)
      XtFree(array[(i - 1)]);

   return(FALSE);

}


/*========================================================*/
/* add item to a list box item list                       */
/*========================================================*/
void AddToList( XmString *array[], XmString item, int *num_items )
{
        array[(*num_items)++] = XmStringCopy( item );

        return;
}

/*========================================================*/
/* add item to a list box item list                       */
/*========================================================*/
int AsciiToString( char *ascii[], XmString array[], int num_items )
{
    int i;

    for (i = 0; (i < num_items) ; i++)
        array[i] = XmStringCreate( ascii[i], "");
}

#ifdef VMS

/*--------------------------------------------------*/
/*  run_proc - run cli command in a subprocess      */
/*--------------------------------------------------*/
int  run_proc(device_name, device_len, object, doctype)

char  *device_name;
int    device_len;
char  *object;
char  *doctype;
{
    PROC_SB    *ast_ptr;
    DESCP      devnam,inputfile,outputfile,command;               
    FILE       *cmdfile;
    char       shellfile[256+5], *pshell;
    char       com_exec[256+5];
    char       filename[14];
    struct tm  *time_info;          /* time information */
    time_t     time_buffer;
    int        flag,sts;

    /* assign channel  */

    makedesc(&devnam,device_name,device_len);

   if ((ast_ptr = (PROC_SB *) calloc(1,sizeof(PROC_SB))) == NULL)
      return(-1);

    if ((sts = sys$assign(&devnam,&(ast_ptr->channel),0,0)) != SS$_NORMAL)
       return(sts);

    /* make temporary command file */

    time(&time_buffer);
    time_info = localtime(&time_buffer);
    sprintf(filename,"CBR%d%d%d.COM",time_info->tm_hour,
                                     time_info->tm_min,
                                     time_info->tm_sec);

    if ((cmdfile = fopen(filename,"w")) == NULL)
       return(-3);

    if (!fgetname(cmdfile,ast_ptr->name))
       return(-4);
    
    /* get command file name */

    if (pshell = getenv("CBRSHELL"))
        strcpy(shellfile,pshell);
    else
        strcpy(shellfile,"cbrbin:cbrshell.com");

    fprintf(cmdfile,"@%s \"%s\" %s \n",shellfile, object, doctype);

    fclose(cmdfile);

    sprintf(com_exec,"@%s",ast_ptr->name); 
    makedesc(&command,com_exec,0);
    makedesc(&inputfile,device_name,device_len);
    makedesc(&outputfile,device_name,device_len);

    /* execute subprocess */

    flag = (CLI$M_NOWAIT | CLI$M_NOCLISYM);

    if ((sts = lib$spawn(&command,
                         &inputfile,
                         &outputfile,
                         &flag,
                         0,		     /* process name */
                         0,                  /* process id   */
                         0,                  /* completion status */
                         0,                  /* event flag num    */
                         delete_decterm,     /* ast address       */
                         ast_ptr,            /* ast args          */
                         0,                  /* prompt            */
                         0)) != SS$_NORMAL)  /* cli          */
    {
       return(sts);
    }

    return(FALSE);
}

/*--------------------------------------------------*/
/*  delete_decterm - deassign channel and thus      */
/*                   kill the decterm               */
/*--------------------------------------------------*/
int  delete_decterm(ast_info)

PROC_SB    *ast_info;
{
    int sts;
 
    if ((sts = sys$dassgn(ast_info->channel)) != SS$_NORMAL)
       return(sts);

    remove(ast_info->name);
 
    free(ast_info);

}


/*--------------------------------------------------*/ 
/*  Write to term call                              */ 
/*--------------------------------------------------*/ 
int    write_to_decterm(channel,text)

short channel;
char  *text;
{
	unsigned long sts;
	IOSB local_iosb;
       	
	/*
	** Write out the text to the pseudo terminal so the application 
	** know's what to do
	*/
	sts = sys$qiow (	0,
			channel,
			IO$_WRITEVBLK | IO$M_ENABLMBX,
			&local_iosb,
			0,
			0,
			text,
			strlen(text),
			0, 0, 0, 0);
};


/*--------------------------*/
/* make a string descriptor */
/*--------------------------*/
DESCP *makedesc(desc_ptr,string,len)

DESCP *desc_ptr;      /* address of string descriptor */
char  *string;           /* string to load               */
int   len;               /* length requested, 0 = strlen */
{

     desc_ptr->dsc$w_length   = (len) ? (len) : (strlen(string));
     desc_ptr->dsc$b_dtype    = DSC$K_DTYPE_T;
     desc_ptr->dsc$b_class    = DSC$K_CLASS_S;
     desc_ptr->dsc$a_pointer  = string;

     return(desc_ptr);

}

#endif /* VMS */

#endif /* CBR */
