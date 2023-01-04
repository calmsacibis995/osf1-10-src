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

/******************************************************************************/
/*                                                                            */
/*                   INCLUDE FILES and LOCAL DEFINITIONS                      */
/*                                                                            */
/******************************************************************************/

#include <cbr_public.h>
#include <DXmCbr/dxmcbrappldefs.h>
#include <DXmCbr/dxmcbrwidget.h>
#include <DXmCbr/dxmcbrconcept.h>
#include <DXmCbr/dxmcbredit.h>
#include <DXmCbr/dxmcbrquery.h>
#include <DXmCbr/dxmcbrresults.h>
#include "bkr_cbr.h"

#include <string.h>
#include <stdio.h>

#ifdef VMS
#include <decw$cursor>
#include <descrip.h>
#include <ssdef.h>
#include <iodef.h>
#include <clidef.h>
#else
#endif


#define CABINET_width 16
#define CABINET_height 16
static char CABINET_bits[] = {
   0x00, 0x00, 0xfe, 0x7f, 0x06, 0x60, 0xc6, 0x63, 0x06, 0x60, 0xfe, 0x7f,
   0x06, 0x60, 0xc6, 0x63, 0x06, 0x60, 0xfe, 0x7f, 0x06, 0x60, 0xc6, 0x63,
   0x06, 0x60, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00};

#define SET_ARG( arg, new_value )    \
    { XtSetArg( arglist[argcnt], (arg), (new_value) ); argcnt++; }

/*===========================*/
/* global references         */
/*===========================*/

GLOBAL_CBR_DATA    *cbrdata;

static Pixmap make_temp_pixmap ();
static void ResultsOkAct();
static void ResultsCancelAct();

static void ConceptSearchAct();
static void ConceptReviewAct();
static void ConceptFilterAct();
static void ConceptCancelAct();

static void EditSearchAct();
static void EditMoreAct();
static void EditSaveAct();
static void EditCancelAct();

static void QuerySearchAct();
static void QueryReviewAct();
static void QueryCancelAct();

/*==================================================*/
/* Register callbacks for Results Widget            */
/*==================================================*/

MrmRegisterArg  results_reglist[] = {
    { "ResultsOkAct",       (caddr_t)ResultsOkAct },
    { "ResultsCancelAct",   (caddr_t)ResultsCancelAct }
    };

int results_reglist_num = ( sizeof( results_reglist ) / 
                            sizeof( results_reglist[0] ) );


/*==================================================*/
/* Register callbacks for concept display Widget    */
/*==================================================*/

MrmRegisterArg  concept_reglist[] = 
{
    { "ConceptSearchAct",   (caddr_t)ConceptSearchAct },
    { "ConceptReviewAct",   (caddr_t)ConceptReviewAct },
    { "ConceptFilterAct",   (caddr_t)ConceptFilterAct },
    { "ConceptCancelAct",   (caddr_t)ConceptCancelAct },
};

int concept_reglist_num = (sizeof(concept_reglist) / 
                           sizeof(concept_reglist[0]));

/*==================================================*/
/* Register callbacks for concept editor Widget     */
/*==================================================*/

MrmRegisterArg  edit_reglist[] = 
{
    { "EditSearchAct",      (caddr_t)EditSearchAct },
    { "EditMoreAct",        (caddr_t)EditMoreAct },
    { "EditSaveAct",        (caddr_t)EditSaveAct },
    { "EditCancelAct",      (caddr_t)EditCancelAct }
};

int edit_reglist_num = ( sizeof( edit_reglist ) / 
                         sizeof( edit_reglist[0] ));


/*==================================================*/
/* Register callbacks for concept builder widget    */
/*==================================================*/

MrmRegisterArg  query_reglist[] = 
{
    { "QuerySearchAct",     (caddr_t)QuerySearchAct },
    { "QueryReviewAct",     (caddr_t)QueryReviewAct },
    { "QueryCancelAct",     (caddr_t)QueryCancelAct }
};

int query_reglist_num = ( sizeof( query_reglist ) / sizeof( query_reglist[0] ));

static char *cbr_filename_vec[] = { "bkr_cbr_widgets.uid" };

static int cbr_filename_num = 
        ( sizeof( cbr_filename_vec ) / sizeof( cbr_filename_vec[0] ) );

/*==================================================*/
/* Initialize the CBR DW stuff.                     */
/*==================================================*/

int 
StartupCBR( toplevelwidget, appcontext, fcdisplay, session_path,
            doc_function, document_data, context, user_data )
    Widget              toplevelwidget;
    XtAppContext        appcontext;
    Display             *fcdisplay;
    char                *session_path;
    int                 (* doc_function)();   
    void                *document_data;
    void                **context;
    void                *user_data;
{
    NodePtr             node;
    int                 i, p, nc;
    char                *names[MAX_COLLECTIONS];

    InitializeCBR( toplevelwidget,
                   appcontext,
                   fcdisplay,
                   session_path,
                   doc_function,
                   document_data,
                   context,
                   user_data );

    /* get 1st profile entry */

    node = cbrdata->Session.children;

    InitProfile( cbrdata->UCB, 
                 cbrdata->Session.handle, 
                 node->entry, 
                 &node->handle ); 

    p = (node->entry - 1);
    cbrdata->Prof[p]->opened = TRUE;

    GetCollectionNames( cbrdata->Prof[p]->handle, 
                        &nc, 
                        names, 
                        cbrdata->UCB );

    cbrdata->Prof[p]->number = nc;
    cbrdata->Prof[p]->level = PROFILE;
    
    for( i=0; i < nc; i++ ) 
            {
            cbrdata->Coll[p][i]             = malloc( sizeof( _Node ) );
            cbrdata->Coll[p][i]->text       = names[i];
            cbrdata->Coll[p][i]->cstext     = 
                    XmStringLtoRCreate(names[i], "ISO8859-1");
            cbrdata->Coll[p][i]->entry      = i+1;
            cbrdata->Coll[p][i]->expanded   = FALSE;
            cbrdata->Coll[p][i]->par_entry  = node->entry;
            cbrdata->Coll[p][i]->level      = COLLECTION;
            cbrdata->Coll[p][i]->opened     = FALSE;
            }

    for( i=1; i < nc; i++ ) 
        cbrdata->Coll[p][i-1]->sibling = cbrdata->Coll[p][i];
    
    /* get 1st collection entry */

    node = cbrdata->Prof[p]->children = cbrdata->Coll[p][0];

    InitCollection( 
            cbrdata->Prof[node->par_entry-1]->handle, 
            node->entry, 
            &node->handle, 
            cbrdata->UCB );

    node->opened = TRUE;
    node->level  = COLLECTION;
    AddCollToOpenList( 
            node->text, 
            node->entry, 
            node->par_entry );
    
    GetConceptSetNames( 
            cbrdata->UCB, 
            node->handle, 
            &nc,
            names );

    if( nc > 0 ) 
    {
            InitConceptSet( node->handle, 
                            1, 
                            &(cbrdata->total_concepts),
                            &(cbrdata->hCSET), 
                            cbrdata->UCB );
    }                   

    return(FALSE);
}



/*==================================================*/
/* Initialize the CBR DW stuff.                     */
/*==================================================*/

int 
InitializeCBR( toplevelwidget, appcontext, fcdisplay, session_path,
               doc_function, document_data, context, user_data)

    Widget          toplevelwidget;
    XtAppContext    appcontext;
    Display         *fcdisplay;
    char            *session_path;
    int             (* doc_function)();
    void            *document_data;
    void            **context;
    void            *user_data;
{
    int             np, i;
    char            *names[MAX_PROFILES];

    if ((cbrdata = (GLOBAL_CBR_DATA *) 
                  calloc (1, sizeof(GLOBAL_CBR_DATA))) == NULL)
    {    
        exit(-1);
    }

    /* get required window data */

    cbrdata->TopWidget  = toplevelwidget;
    cbrdata->AppContext = appcontext;
    cbrdata->FcDisplay  = fcdisplay;
    cbrdata->user_data  = user_data;

    cbrdata->doc_function   = doc_function;
    cbrdata->document_data  = document_data;
    cbrdata->context        = context;

    if( MrmOpenHierarchy( cbr_filename_num, 
                          cbr_filename_vec, 
                          NULL,
                          &(cbrdata->CBRMRMHierarchy)) != MrmSUCCESS ) 
    {
        SError( "Can't open hierarchy\n" );
    }

    MakeWorkCursor( cbrdata->TopWidget, &(cbrdata->WaitCursor) );

    DXmCbrInitializeForMrm();

    InitSession( cbrdata->UCB, 
                 session_path, 
                 &(cbrdata->Session.handle) );

    GetProfileNames( cbrdata->Session.handle, 
                     &np, 
                     names, 
                     cbrdata->UCB );

    cbrdata->Session.number     = np;
    cbrdata->Session.level      = SESSION;
    cbrdata->Session.text       = malloc( 7 * sizeof( char ) );

    strcpy( cbrdata->Session.text, "Session" );

    cbrdata->Session.cstext     = XmStringLtoRCreate( "Session" , "ISO8859-1");
    cbrdata->Session.sibling    = NULL;
    cbrdata->Session.expanded   = FALSE;
    cbrdata->Session.entry      = 1;


    /* Allocate the profiles        */

    for( i=0; i < np; i++ ) 
    {
        cbrdata->Prof[i]            = malloc( sizeof( _Node ) );
        cbrdata->Prof[i]->text      = names[i];
        cbrdata->Prof[i]->cstext    = XmStringLtoRCreate( names[i], "ISO8859-1");

        cbrdata->Prof[i]->entry     = i+1;
        cbrdata->Prof[i]->par_entry = 1;
        cbrdata->Prof[i]->expanded  = FALSE;
        cbrdata->Prof[i]->level     = PROFILE;
        cbrdata->Prof[i]->opened    = FALSE;
    }

    for( i=1; i < np; i++ ) 
        cbrdata->Prof[i-1]->sibling = cbrdata->Prof[i];

    cbrdata->Session.children = cbrdata->Prof[0];

    return(FALSE);
}


/*==================================================*/
/* The DoQuery function fetches and manages QueryBox*/
/*==================================================*/

void DoQuery()
{
    Arg                 arglist[10];
    int                 argcnt = 0;
    XmString            title;

    title = XmStringCreate("Query","");
    
    argcnt = 0;
    SET_ARG( XmNdialogTitle, title );

    if( cbrdata->query_box == NULL )
    {
            MrmRegisterNames( query_reglist, query_reglist_num );
            if( MrmFetchWidgetOverride( 
                            cbrdata->CBRMRMHierarchy, 
                            "query_pdb",
                            cbrdata->TopWidget, 
                            NULL,
                            arglist,
                            argcnt,
                            &(cbrdata->query_box), 
                            &(cbrdata->DummyClass)) != MrmSUCCESS ) 
                {
                SError( "can't fetch query box\n" );
                }
            }

    if ( !(XtIsManaged( cbrdata->query_box)))
        XtManageChild( cbrdata->query_box );

}   /* end DoQuery */


/*====================================================*/
/* Search callback for QUERY (concept builder) widget */
/*====================================================*/

static void 
QuerySearchAct( 
    Widget                      w, 
    int                         *tag, 
    DXmCbrCallbackStruct        *data )
{
    CBR_LONG              len;              
    CBR_VOID              *query;          /* otl definition as input     */
    oc_struct             *ocptr;
    Arg                   arglist[10];       
    int                   argcnt;
    char                  *concept_def = NULL;
    int                   i;
    NodePtr               col_node;
    static CBR_VOID       *hSQRY;
    CBR_VOID              *hRSLT = NULL;
    int                   valid_query;
    int                   result_box_num;
    char                  name[MAX_BUFF+1];
    XmString              name_cs;


    /* test for obvious errors */

    if( data->value == 0 )
        return;
      
    if( data->value[0] == '\0')
        return;
      
    /* turn on work cursor */

    DisplayWorkCursor( w, w, cbrdata->WaitCursor );

    /*
     * get the right collection
     */

    ocptr = &(cbrdata->OpenColl[0]);
    col_node = cbrdata->Coll[ ocptr->par_entry - 1][ ocptr->entry - 1];

    /* 
     * make a concept definition from the users input
     */

    if( data->quick_query )
    {
        /* generate concept and extract concept in search form */

        make_concept( 
                &query,
                &concept_def, 
                data->value, 
                cbrdata->words, 
                name, 
                cbrdata->hCSET,
                "en_US" );

        if (!concept_def)
            SError( "can't generate concept\n" );

        MakeQuery( 
                query,
                cbrdata->Session.handle, 
                cbrdata->UCB, 
                cbrdata->hCSET, 
                cbrdata->q_items,
                NULL,                  /* this makes it do a QuickQuery */
                &hSQRY );
    }
    else
    {
        /* do a structured query */

        MakeQuery( 
                data->value,
                cbrdata->Session.handle, 
                cbrdata->UCB, 
                cbrdata->hCSET, 
                cbrdata->q_items,
                cbrdata->words,        /* this makes it do a structured Query */
                &hSQRY );
    
        strncpy(name, data->value, MAX_BUFF);
    }

    valid_query = DoSearch( 
                  hSQRY, 
                  col_node->handle, 
                  cbrdata->UCB, 
                  &hRSLT );

    name_cs = XmStringCreate(name,"");

    if( valid_query ) 
    {
        if ((result_box_num = DoResults( hRSLT, name_cs)) != -1)
        {
            ocptr->result_w[ ocptr->num_res_boxes] = result_box_num;
            ocptr->result_handle[ ocptr->num_res_boxes] = hRSLT;
            ocptr->num_res_boxes++;
        }
    } 
    else 
    {
        DoMessage( "Invalid Query" );
    };
    
    XtFree(name_cs);

    if( data->quick_query )
        free(query);
                                        /* Stop watch cursor */
    RemoveWorkCursor( w, w);
}


/*==================================================*/
/* Cancel callback for QUERY widget                   */
/*==================================================*/

static void 
QueryCancelAct( 
    Widget                  w, 
    int                     *tag, 
    unsigned long           *reason )
{
}







/*==================================================*/
/* Review callback for QUERY widget                   */
/*==================================================*/

static void 
QueryReviewAct(
    Widget                      w,
    int                         *tag,
    DXmCbrCallbackStruct        *data )
{
    Arg             arglist[10];       
    int             argcnt;
    char            *concept_def = NULL;
    char            name[MAX_BUFF+1];
    int             i;

    /* test for obvious errors */

    if( data->value == 0 )
        return;
      
    if( data->value[0] == '\0')
        return;
      
    /* display work cursor */

    DisplayWorkCursor( w, w, cbrdata->WaitCursor );

    /* make a concept from users input */

    make_concept( 
            NULL,
            &concept_def, 
            data->value, 
            cbrdata->words, 
            name, 
            cbrdata->hCSET,
            "en_US" );

    if (!concept_def)
    {
        /* turn off work cursor */
        RemoveWorkCursor( w, w );                    
        return;
    }

    argcnt = 0;
    SET_ARG( DXmNeditConceptDefinition, concept_def );

    /* create a new concept editor widget */

    for( i=0; i < MAX_EDIT_BOXES; i++ ) 
    {
        if( cbrdata->edit_box[i] == NULL ) 
        {
            cbrdata->edit_box_num = i;
            if( i == 0 ) 
            {
                MrmRegisterNames( edit_reglist, edit_reglist_num );
            }

            if (MrmFetchWidgetOverride(cbrdata->CBRMRMHierarchy, 
                               "edit_pdb",
                               cbrdata->TopWidget, 
                               NULL,
                               arglist,
                               argcnt,
                               &(cbrdata->edit_box[i]),
                               &cbrdata->DummyClass) != MrmSUCCESS) 
            {
                SError( "can't fetch results box\n" );
            }
            break;
        } 
        else if( !(XtIsManaged( cbrdata->edit_box[i]))) 
        {
           cbrdata->edit_box_num = i;
           XtSetValues( cbrdata->edit_box[i], arglist, argcnt );

           break;
        }
    }
    if( i == MAX_EDIT_BOXES ) 
    {
        SError( "Unexpected error in concept select\n" );
    }
    
    free(concept_def);

    XtManageChild( cbrdata->edit_box[i] );

    /* turn off work cursor */

    RemoveWorkCursor( w, w );                    

    return;
}


/*==================================================*/
/* Fetch and manage the concept editor              */
/*==================================================*/

void DoEdit()
{
    Arg         arglist[10];       
    int         argcnt = 0;
    char        *concept_def = NULL;
    char        name[MAX_BUFF+1];
    int         i;


    /* create a new concept editor widget */

    for( i=0; i < MAX_EDIT_BOXES; i++ ) 
    {
        if( cbrdata->edit_box[i] == NULL ) 
        {
            cbrdata->edit_box_num = i;
            if( i == 0 ) 
            {
                MrmRegisterNames( edit_reglist, edit_reglist_num );
            }

            if (MrmFetchWidgetOverride(cbrdata->CBRMRMHierarchy, 
                               "edit_pdb",
                               cbrdata->TopWidget, 
                               NULL,
                               arglist,
                               argcnt,
                               &(cbrdata->edit_box[i]),
                               &cbrdata->DummyClass) != MrmSUCCESS) 
            {
                SError( "can't fetch results box\n" );
            }
            break;
        } 
        else if( !(XtIsManaged( cbrdata->edit_box[i]))) 
        {
           cbrdata->edit_box_num = i;
           XtSetValues( cbrdata->edit_box[i], arglist, argcnt );

           break;
        }
    }
    if( i == MAX_EDIT_BOXES ) 
    {
        SError( "Unexpected error in concept select\n" );
    }

    XtManageChild( cbrdata->edit_box[i] );

    return;

}   /* end DoEdit */









/*==================================================*/
/* activate callback for concept edit search        */
/*==================================================*/

static void 
EditSearchAct( 
    Widget                  w, 
    int                     *tag, 
    DXmCbrCallbackStruct    *data )
{
    CBR_LONG                len;              
    CBR_CHAR                *query;          /* otl definition as input */
    oc_struct               *ocptr;
    NodePtr                 col_node;
    static CBR_VOID         *hSQRY;
    CBR_VOID                *hRSLT = NULL;
    int                     valid_query;
    int                     result_box_num;
    char                    name[MAX_BUFF+1];

    /* display work cursor */

    DisplayWorkCursor( w, w, cbrdata->WaitCursor );

    ocptr = &(cbrdata->OpenColl[0]);
    col_node = cbrdata->Coll[ ocptr->par_entry - 1][ ocptr->entry - 1];

    /* make a concept from concept defintion */

    CSToAsciz( data->name, &name[0] );

    make_new_concept( 
            &query,
            data->value, 
            name, 
            cbrdata->hCSET,
            "en_US" );

    if (!query)
    {
        /* turn off work cursor */
        RemoveWorkCursor( w, w );                    
        return;
    }

    MakeQuery( 
            query,
            cbrdata->Session.handle, 
            cbrdata->UCB, 
            cbrdata->hCSET, 
            cbrdata->q_items,
            NULL,           /* if no words supplied, dont tokenize */
            &hSQRY );

    valid_query = DoSearch( 
                        hSQRY, 
                        col_node->handle, 
                        cbrdata->UCB, 
                        &hRSLT );

    if( valid_query ) 
    {
        if ((result_box_num = DoResults( hRSLT, data->name)) != -1)
        {
            ocptr->result_w[ ocptr->num_res_boxes] = result_box_num;
            ocptr->result_handle[ ocptr->num_res_boxes] = hRSLT;
            ocptr->num_res_boxes++;
        }
    } 
    else 
    {
        DoMessage( "Invalid Query" );
    };
    
    free(query);

    /* Stop watch cursor */

    RemoveWorkCursor( w, w);
}






/*==================================================*/
/* Edit Cancel callback                             */
/*==================================================*/

static void 
EditCancelAct( 
    Widget              w, 
    int                 *tag, 
    unsigned long       *reason )
{
}





/*==================================================*/
/* Save edited concept to user concept set          */
/*==================================================*/

static void 
EditSaveAct(
    Widget                      w,
    int                         *tag,
    DXmCbrCallbackStruct        *data )
{
#ifdef BOOKREADER
    FILE			*otl_fp;
    char			otl_name[255];
    char			otl_definiton[1000];
#endif

    /* display work cursor */

    DisplayWorkCursor( w, w, cbrdata->WaitCursor );

    /* store the concept */

    SaveConcept( 
            data->value,
            strlen( data->value ),
            cbrdata->hCSET, 
            cbrdata->UCB );

#ifdef BOOKREADER
    CSToAsciz( data->name, &otl_name[0] );
    strcat( otl_name, ".otl" );
    otl_fp = fopen( otl_name, "w" );
    fputs( data->value, otl_fp );
    fclose( otl_fp );
#endif

    /* turn off work cursor */

    RemoveWorkCursor( w, w );                    
}






/*==================================================*/
/* Fetch concept for edit (more callback)           */
/*==================================================*/

static void 
EditMoreAct(
    Widget                      w,
    int                         *tag,
    DXmCbrCallbackStruct        *data )
{

    char    name[MAX_BUFF+1];
    char    *definition;
    int     found;
    int     type = 0;

    /* test for obvious errors */

    if( data->value == 0 )
        return;
      
    if( data->value[0] == '\0')
        return;
      
    definition = NULL;
    GetConceptDef( 
                cbrdata->UCB, 
                1, 
                data->value,
                cbrdata->hCSET, 
                &definition, 
                &found );

    if (found)
    {
        DXmCbrEditAddConcept(w, definition, type);
    }

    if (definition)
        free(definition);

#ifdef DEBUG
    printf("reason %d name '%s'\n",data->reason, name);
#endif

}


/*================================================================*/
/* The Doconcepts function fetches and manages concept display box */
/*================================================================*/

void 
DoConcepts()
{
    Arg     arglist[5];
    int     nc, nreturned, i, argcnt;
    char    *concepts[MAX_TITLE];

    if( cbrdata->concept_box == NULL ) 
    {
        MrmRegisterNames( concept_reglist,concept_reglist_num );
        if( MrmFetchWidget( cbrdata->CBRMRMHierarchy, 
                            "concept_pdb",
                            cbrdata->TopWidget, 
                            &(cbrdata->concept_box), 
                            &(cbrdata->DummyClass)) != MrmSUCCESS ) 
        {
            SError( "can't fetch concept box\n" );
        }

        cbrdata->new_concepts = TRUE;
    }

    if (cbrdata->new_concepts)
    {
        cbrdata->new_concepts = FALSE;

        for (i = 0; i < MAX_TITLE; i++)
            concepts[i] = (char *) calloc( MAX_BUFF, sizeof( char ) );

        for ( i = 1; i <= cbrdata->total_concepts; i += nreturned )
        {
            FetchCbrConcepts( 
                        cbrdata->UCB, 
                        cbrdata->hCSET, 
                        cbrdata->total_concepts, 
                        i,                          /* start here */
                        concepts,
                        MAX_TITLE,                  /* get this many */
                        &nreturned );

            DXmCbrConceptAddItems( 
                        cbrdata->concept_box,       /* widget */
                        concepts,                   /* names  */
                        nreturned,                  /* number */
                        DXmCBR_ADD_AT_BOTTOM );     /* add at bottom  */
            }

        for (i = 0; i < MAX_TITLE; i++)
            cfree( concepts[i] );
    }

    if ( !(XtIsManaged( cbrdata->concept_box)))
        XtManageChild( cbrdata->concept_box );
}



/*==================================================*/
/* Concept Search callback                          */
/*==================================================*/
static void 
ConceptSearchAct( 
    Widget                      w, 
    int                         *tag, 
    DXmCbrCallbackStruct        *data)
{
    static CBR_VOID     *hSQRY;
    oc_struct           *ocptr;
    NodePtr             col_node;
    int                 result_box_num;
    int                 found, nreturned, valid_query;
    XmString            name_cs;
    int                 local = FALSE;
    char                *definition = NULL;
    CBR_VOID            *query      = NULL;          
    CBR_VOID            *hRSLT      = NULL;


    /* test for obvious errors */

    if( data->value == 0 )
        return;
      
    if( data->value[0] == '\0')
        return;

    /* display work cursor */

    DisplayWorkCursor( w, w, cbrdata->WaitCursor );

    MakeQuery( 
              data->value,
              cbrdata->Session.handle, 
              cbrdata->UCB, 
              cbrdata->hCSET, 
              cbrdata->q_items,
              cbrdata->words,             /* do a structured Query */
              &hSQRY );

    ocptr = &(cbrdata->OpenColl[0]);
    col_node = cbrdata->Coll[ ocptr->par_entry - 1][ ocptr->entry - 1];

    valid_query = DoSearch( 
                        hSQRY, 
                        col_node->handle, 
                        cbrdata->UCB, 
                        &hRSLT );

    name_cs = XmStringCreate(data->value, "");

    if( valid_query ) 
    {
            if ((result_box_num = DoResults( hRSLT, name_cs)) != -1)
            {
            ocptr->result_w[ ocptr->num_res_boxes] = result_box_num;
            ocptr->result_handle[ ocptr->num_res_boxes] = hRSLT;
            ocptr->num_res_boxes++;
            }
    } 
    else 
    {
        DoMessage( "Invalid Query" );
    };
    
    XtFree(name_cs);
                            
    /* turn off work cursor */

    RemoveWorkCursor( w, w );                    
}


/*==================================================*/
/* Concept display Cancel callback                  */
/*==================================================*/

static void 
ConceptCancelAct( 
    Widget                  w, 
    int                     *tag,         
    unsigned long           *reason )
{
    XtUnmanageChild( cbrdata->concept_box );
}



/*==================================================*/
/* Concept filter select callback                   */
/*==================================================*/

static void 
ConceptFilterAct( 
    Widget                  w, 
    int                     *tag, 
    DXmCbrCallbackStruct    *data )
{
}



/*==================================================*/
/* Concept display select callback                  */
/*==================================================*/

static void 
ConceptReviewAct( 
    Widget                      w, 
    int                         *tag, 
    DXmCbrCallbackStruct        *data )
{
    char            *definition = NULL;
    char            title[50];          /* Title of result box.         */
    XmString        title_cs;           /* Title of result box.         */
    XmString        name_cs;            /* name of concept              */
    int             nreturned;
    Arg             arglist[10];       
    int             argcnt,
                    found,
                    i;

    /* test for obvious errors */

    if( data->value == 0 )
        return;
      
    if( data->value[0] == '\0')
        return;
      
    GetConceptDef( 
                cbrdata->UCB, 
                1, 
                data->value,
                cbrdata->hCSET, 
                &definition, 
                &found );

    name_cs = XmStringCreate(data->value , "ISO8859-1");

    argcnt = 0;
    SET_ARG( DXmNeditConceptName, name_cs ); 
    
    if (found)
    {
        SET_ARG( DXmNeditConceptDefinition, definition); 
    }
    else
    {
        SET_ARG( DXmNeditConceptDefinition, NULL); 
    }

    /* create a new concept editor widget */

    for( i=0; i < MAX_EDIT_BOXES; i++ ) 
    {
        if( cbrdata->edit_box[i] == NULL ) 
        {
            cbrdata->edit_box_num = i;
            if( i == 0 ) 
            {
                MrmRegisterNames( edit_reglist, edit_reglist_num );
            }

            if (MrmFetchWidgetOverride(cbrdata->CBRMRMHierarchy, 
                               "edit_pdb",
                               cbrdata->TopWidget, 
                               NULL,
                               arglist,
                               argcnt,
                               &(cbrdata->edit_box[i]),
                               &cbrdata->DummyClass) != MrmSUCCESS) 
            {
                SError( "can't fetch results box\n" );
                break;
            }
            break;
        } 
        else if( !(XtIsManaged( cbrdata->edit_box[i]))) 
        {
           cbrdata->edit_box_num = i;
           XtSetValues( cbrdata->edit_box[i], arglist, argcnt );

           break;
        }
    }

    if( i == MAX_EDIT_BOXES ) 
    {
        SError( "Unexpected error in concept select\n" );
    }
    
    if (found)
        free(definition);

    XtManageChild( cbrdata->edit_box[i] );
    XmStringFree(name_cs);

    return;

}




/*========================================================*/
/* This function adds a collections name to the open      */
/* collection list                                        */
/*========================================================*/
void AddCollToOpenList( char *name, int entry, int par_entry )
{
    int             i;

    cbrdata->OpenColl[cbrdata->NumOpenColl].name            = name;
    cbrdata->OpenColl[cbrdata->NumOpenColl].entry           = entry;
    cbrdata->OpenColl[cbrdata->NumOpenColl].par_entry       = par_entry;
    cbrdata->OpenColl[cbrdata->NumOpenColl].num_res_boxes   = 0;
    cbrdata->NumOpenColl++;
}







/*========================================================*/
/* This function displays the results of a query in the   */
/* result box.                                            */
/*========================================================*/

int 
DoResults( 
    void            *result_handle, 
    XmString        name )
{
    int                     total_docs;
    int                     total_results;
    int                     nreturned;
    XmString                item;
    Arg                     arglist[10];       /* List arguments for results box */
    int                     argcnt;
    int                     cur_result;
    int                     num, num_items;
    char                    buffer[MAX_BUFF+1];       
    int                     size, pos;
    int                     status;
    int                     ni;
    CBR_ITEM                pi[5];
    int                     entry, i;
    char                    query[250];          /* Query of result box.         */
    XmString                query_cs;            /* Query of result box.         */
    XmString                final_query_cs;      /* Query of result box.         */
    char                    *res_titles[MAX_TITLE];
    int                     rankings[MAX_TITLE];
    Pixmap                  pixmaps[MAX_TITLE];
    static Pixmap           temp_pixmap = 0;

    for( i=0; i < MAX_RESULTS_BOXES; i++ ) 
    {
        if( cbrdata->results_box[i] == NULL ) 
        {
            cur_result = i;
            if( i == 0 ) 
            {
                MrmRegisterNames( results_reglist, results_reglist_num );
            }
            if( MrmFetchWidget( cbrdata->CBRMRMHierarchy, 
                                "results_pdb",
                                cbrdata->TopWidget, 
                                &(cbrdata->results_box[i]), 
                                &(cbrdata->DummyClass)) != MrmSUCCESS ) 
            {
                SError( "can't fetch results box\n" );
            }
            break;
        } 
        else if( !(XtIsManaged( cbrdata->results_box[i]))) 
        {
           cur_result = i;
           break;
        }
    }
    if( i == MAX_RESULTS_BOXES ) 
    {
        SError( "Unexpected error in DoResults\n" );
    }

                                        /* allocate result list */
    ni    = 1;
    pi[0].type   = CBR_C_OUT_ITEM;
    pi[0].code   = CBR_I_DOC_TOTAL;
    pi[0].buffer = &total_docs;
    pi[0].length = FALSE;
    pi[0].status = FALSE;

    status = cbr_getinfo( CBR_O_RSLT, result_handle, ni, pi, cbrdata->UCB );
    if( status != CBR_S_SUCCESS ) 
    {
        demo_error_display( "result", status, CBR_O_RSLT, result_handle );
    }
    if( total_docs == 0 ) 
    {
        DoMessage( "No documents retrieved");
        return( -1 );
    }

    /* hack the pixmaps for now */

    if (temp_pixmap == 0)
        temp_pixmap = make_temp_pixmap( cbrdata->TopWidget );

    for (i = 0; i < MAX_TITLE; i++)
        pixmaps[i] = temp_pixmap;


    /* results titles must be dynamic */

    for (i = 0; i < MAX_TITLE; i++)
        res_titles[i] = (char *) calloc( MAX_BUFF, sizeof( char ) );

    /* get the results.  CBR is 1-based */

    total_results = (total_docs > 100) ? (100) : (total_docs);
    for (i = 1; i <= total_results; i += nreturned )
    {
            FetchCbrResults(
                cbrdata->UCB, 
                result_handle, 
                total_docs,                 
                i,                          /* start here */
                res_titles,                 /* titles go here */
                rankings,                   /* rankings go here */
                MAX_TITLE,                  /* get this manby */
                &nreturned );

            DXmCbrResultsAddItems( 
                cbrdata->results_box[ cur_result ],
                pixmaps,                
                rankings,                
                res_titles,                
                nreturned,
                DXmCBR_ADD_AT_BOTTOM );     /* add at bottom */
    }

    /* pass in additional results information */

    sprintf( query, "%d:  Found %d documents - ",  cur_result + 1, total_docs );
    query_cs = XmStringCreate(query , "ISO8859-1");

    /* set result widget descriptive Text */

    if (name)
    {
        final_query_cs = XmStringConcat(query_cs, name);
        DXmCbrResultSetQuery(cbrdata->results_box[cur_result], final_query_cs);
        XmStringFree(final_query_cs);
    }

    XtManageChild( cbrdata->results_box[ cur_result ] );

    /* clean up memory */

    for (i = 0; i < MAX_TITLE; i++)
        free( res_titles[i] );
    
    XtFree( query_cs ); 

    return( cur_result );
}



/*========================================================*/
/* result Activate callback for the Ok button.            */
/*========================================================*/

static void 
ResultsOkAct( 
    Widget                      w, 
    int                         *tag, 
    DXmCbrCallbackStruct        *data )
{
    int             l_box, clcn, r_box;
    int             num_open;
    int             num_res;

    for( l_box = 0; l_box < MAX_RESULTS_BOXES; l_box++ )
        {
        if( w == cbrdata->results_box[l_box])
            {
            break;
            }
        }
    if( l_box == MAX_RESULTS_BOXES ) 
        {
        SError( "Unexpected error in ResultsOkAct\n" );
        }

    num_open = cbrdata->NumOpenColl;
    for( clcn = 0; clcn < num_open; clcn++ ) 
        {
        num_res = cbrdata->OpenColl[clcn].num_res_boxes; 
        for( r_box = 0; r_box < num_res; r_box++ ) 
            {
            if( cbrdata->OpenColl[clcn].result_w[r_box] == l_box ) 
                {
                /* Start watch cursor */

                DisplayWorkCursor( cbrdata->results_box[l_box], 
                                   cbrdata->results_box[l_box], 
                                   cbrdata->WaitCursor );

                OperateOnResults( cbrdata->UCB,
                                cbrdata->OpenColl[clcn].result_handle[r_box],
                                data->position,
                                cbrdata->doc_function,
                                cbrdata->document_data,
                                cbrdata->user_data);

                 /* Stop  watch cursor */

                RemoveWorkCursor( cbrdata->results_box[l_box], 
                                  cbrdata->results_box[l_box] );

                return;
                }
            }
        }

    if( clcn == num_open) 
        {
        SError( "Unexpected error in ResultsOkAct\n" );
        }

    return;
}


/*========================================================*/
/* results Cancel button.                                 */
/*========================================================*/

static void 
ResultsCancelAct( 
    Widget              w, 
    int                 *tag, 
    unsigned long       *reason )
{
    int                 ni;         /*  number of paramters             */
    CBR_ITEM            pi[5];      /*  parameters                      */
    int                 i, 
                        l_box, 
                        clcn, 
                        status, 
                        r_box,
                        num_open, 
                        num_res,
                        found;

    /* Find out which collection and */
    /* result box we're dealing with */

    for( found = FALSE, l_box = 0; l_box < MAX_RESULTS_BOXES; l_box++ ) 
    {
        if(w  == cbrdata->results_box[l_box] )
        {
            found = TRUE;
            break;
        }
    }

    if(!found)
        SError( "Unexpected error in ResultsCancelAct\n" );

    num_open = cbrdata->NumOpenColl;
    for( found = FALSE, clcn = 0; clcn < num_open; clcn++ ) 
    {
        num_res = cbrdata->OpenColl[clcn].num_res_boxes; 
        for( r_box = 0; r_box < num_res; r_box++ ) 
        {
            if( cbrdata->OpenColl[clcn].result_w[r_box] == l_box ) 
            {
                found = TRUE;
                break;
            }
        }

        if (found)
            break;
    }

    if(!found)
        SError( "Unexpected error in ResultsCancelAct\n" );

    /* remove previous results from list */

    DXmCbrResultsDeleteAllItems(cbrdata->results_box[l_box]);

    /* Delete the result handle.    */

    CBR_M_CLR(pi, ni);
    status = cbr_delete(
                    CBR_O_RSLT, 
                    ni, 
                    pi, 
                    cbrdata->UCB, 
                    &(cbrdata->OpenColl[clcn].result_handle[r_box]));

    if( status != CBR_S_SUCCESS ) 
        SError( "Result handle not deleted\n" );

    /* Clean up the OpenColl struct */

    for( i=r_box; i < num_res; i++ ) 
    {
        cbrdata->OpenColl[clcn].result_handle[i] = 
                                cbrdata->OpenColl[clcn].result_handle[i+1];
        cbrdata->OpenColl[clcn].result_w[i] = 
                                cbrdata->OpenColl[clcn].result_w[i+1];
    }

    cbrdata->OpenColl[clcn].num_res_boxes--;

    XtUnmanageChild( XtParent( w ) );
}




/*========================================================*/
/* Determine which results box we are dealing with        */
/*========================================================*/

void DetermineLBoxColl( Widget w, int *clcn, int *r_box )
{
    int i, j, k;
    int num_open;
    int num_result;

    for( i=0; i < MAX_RESULTS_BOXES; i++ ) 
    {
        if( cbrdata->results_box[i] == w ) 
        {
            break;
        }
    }
    if( i == MAX_RESULTS_BOXES ) 
    {
        SError( "Unexpected error1 in DetermineLBox" );
    }

    num_open = cbrdata->NumOpenColl;
    for( j=0; j < num_open; j++ ) 
    {
        num_result = cbrdata->OpenColl[j].num_res_boxes;
        for( k = 0; k < num_result; k++ ) {
            if( cbrdata->OpenColl[j].result_w[k] == i ) 
            {
                *clcn  = j;
                *r_box = k;
                return;
            }
        }
    }

    if( j == cbrdata->NumOpenColl ) 
    {
        SError( "Unexpected error in DetermineLBox" );
    }

    return;
}

/*========================================================*/
/* close up collection - remove all outstanding results   */
/* and edit boxs before closing collection                */
/*========================================================*/

void 
CloseUpCollection ( )
{
    Widget              w;
    int                 ni;         /*  number of paramters             */
    CBR_ITEM            pi[5];      /*  parameters                      */
    int                 i, 
                        clcn, 
                        status, 
                        num_res, 
                        r_box;

    /* Assume for now just one collection open */

    clcn = 0;
    num_res = cbrdata->OpenColl[clcn].num_res_boxes; 
    for( r_box = 0; r_box < num_res; r_box++ ) 
    {
        /* Delete the result handle.    */

        CBR_M_CLR(pi, ni);
        status = cbr_delete( CBR_O_RSLT, 
                             ni, 
                             pi, 
                             cbrdata->UCB, 
                             &(cbrdata->OpenColl[clcn].result_handle[r_box]));

        cbrdata->OpenColl[clcn].result_handle[r_box] = NULL;
    }

    for( i=0; i < MAX_RESULTS_BOXES; i++ ) 
    {
        if( cbrdata->results_box[i]) 
        {
            if (XtIsManaged(cbrdata->results_box[i]))
            {
                DXmCbrResultsDeleteAllItems(cbrdata->results_box[i]);
                XtUnmanageChild( cbrdata->results_box[i] );
            }
        }
    }

    for( i=0; i < MAX_EDIT_BOXES; i++ ) 
    {
        if(cbrdata->edit_box[i]) 
        {
            if (XtIsManaged( cbrdata->edit_box[i]) )
                XtUnmanageChild( cbrdata->edit_box[i]);
        }
    }

    if (cbrdata->concept_box)
    {
        if (XtIsManaged( cbrdata->concept_box) )
        {
            DXmCbrConceptDeleteAllItems( cbrdata->concept_box );
            XtUnmanageChild( cbrdata->concept_box );
        }
        cbrdata->new_concepts = TRUE;
    }

    if (XtIsManaged( cbrdata->query_box))
        XtUnmanageChild( cbrdata->query_box);

    cbrdata->edit_box_num = 0;
    cbrdata->OpenColl[clcn].num_res_boxes = 0;
    cbrdata->NumOpenColl = 0;
    cbrdata->total_concepts = 0;
}


/*------------------------------------------------------------*/
/* setup pixmaps                                              */
/*------------------------------------------------------------*/
Pixmap
make_temp_pixmap (Widget w )
{
    Screen      *screen  = XtScreen(w);
    Display     *display = DisplayOfScreen (screen);
    Pixel       background_pixel;
    Pixel       foreground_pixel;
    Arg         args [2];

    XtSetArg( args[0], XmNforeground, &foreground_pixel);
    XtSetArg( args[1], XmNbackground, &background_pixel);
    XtGetValues( w, args, 2);

    return (
        XCreatePixmapFromBitmapData (
                display,                            /* (IN) display         */
                XDefaultRootWindow(display),        /* (IN) drawable        */
                CABINET_bits,                       /* (IN) bitmap data     */
                CABINET_width,                      /* (IN) width           */
                CABINET_height,                     /* (IN) height          */
                foreground_pixel,                   /* (IN) foreground pixel */
                background_pixel,                   /* (IN) background pixel */
                DefaultDepthOfScreen (screen)));    /* (IN) pixmap depth    */
}
#endif /* CBR */
