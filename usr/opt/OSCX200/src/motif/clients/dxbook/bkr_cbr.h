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
**++
**  FACILITY
**
**      DXmCbrSampleDefs.h
**
**  ABSTRACT:
**
**      
**
**  AUTHORS:
**
**      Daniel Leroux
**      Robert L. Cohen
**
**
**  CREATION DATE:   Oct. 10, 1990   
**
**  MODIFICATION HISTORY:
**
**. 1.0         25-Apr-1991 R. L. Cohen     Incorporated CBR Motif Widgets
**--
*/

#ifdef VMS
#include <decw$include/MrmAppl.h>
#include <decw$include/MrmWidget.h>
#else
#include <X11/MrmAppl.h>
#endif

#define MAX_DOC_BOXES       10

                                        /* Keeps track of doc boxes.    */
typedef struct 
{
    int                 num;
    Widget              pdb;
    Widget              stext;
    char                *file;
    char                *doctype;
    int                 highlighting;
    Boolean             CurrentCNetValid;
    /* lwk_ui              dwui;        */
    /* lwk_composite_net   CurrentCNet; */
    /* SurrogateList       *surrogates; */

} DOCBox;

                                        /* Global window Variables      */

typedef struct global_window
{
    Widget          TopWidget;
    XtAppContext    AppContext;
    Display        *FcDisplay;
    MrmHierarchy    FcDRMHierarchy;
    MrmType        *DummyClass;
    Widget          SVNWidget;
    Cursor          WaitCursor;
    Pixmap          LargeIconPixmap;
    Pixmap          cabinet_pixmap;
    Pixmap          top_pixmap;
    Pixmap          drawer_pixmap;
    Widget          text_box;           /* SText Widget in query_box    */
    Widget          list_box;           /* List Widget in query_box     */
    Widget          main_window;
    Widget          link_menu_entry;
    int             text_box_num;          /* number of text widgets open  */
    DOCBox          DocBoxs[MAX_DOC_BOXES];
    char            document_data[512+1];
    
} GLOBAL_WINDOW_DATA;

void GetOpenCollectionBox();
void GetCloseCollectionBox();

void  ToggleNode ( Widget svnw, int node_number, unsigned int entry_tag );
void  CloseNode ( Widget svnw, int node_number, unsigned int entry_tag );
void  OpenNode ( Widget svnw, int node_number, unsigned int entry_tag );
void  RecCloseNode ( int node_tag, int *total_children );
void  RecOpenNode ( Widget svnw, int  node_tag, int *count );

Boolean SourceIsNodeParent ();
void SourceToggleNode ();
void SourceOpenNode   ();
void SourceCloseNode  ();

void AddWordToTextBox( char *word ) ;
void AddToList( XmString *array[], XmString item, int *num_items );
int FreeCSList( XmString *array[], int num_items );

int ShowTextDoc( char *file, void *user_data);
int EditTextDoc( char *file, void *user_data);

