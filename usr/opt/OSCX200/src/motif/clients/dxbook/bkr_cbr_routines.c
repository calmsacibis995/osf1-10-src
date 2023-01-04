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
/*  Copyright (c) Digital Equipment Corporation, 1992                       */
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef VMS
#include <decw$cursor>
#include <descrip.h>
#include <ssdef.h>
#include <iodef.h>
#include <clidef.h>
#include <unixlib.h>
#else
#endif

char *get_token(char *line,char *buffer,char *delimiters, int *line_pos);

/*==================================================*/
/*  GLOBAL VARIABLES                                */
/*==================================================*/

#define cNL     (15)
#define cMTQ    (100)

/*
**.
**. The following are the search query operator token definitions:
**.
**.     mTOKEN_AND          AND         conjunction operator
**.     mTOKEN_OR           OR          disjunction operator
**.     mTOKEN_NOT          NOT         negation operator
**.     mTOKEN_LPAREN       (           left parenthesis scoping operator
**.     mTOKEN_RPAREN       )           right parenthesis scoping operator
**.     mTOKEN_SENTENCE     SENTENCE    sentence proximity operator
**.     mTOKEN_PARAGRAPH    PARAGRAPH   paragraph proximity operator
**.     mTOKEN_PHRASE       PHRASE      phrase proximity operator
**.
*/
#define mTOKEN_AND          ("and")
#define mTOKEN_OR           ("or")
#define mTOKEN_NOT          ("not")
#define mTOKEN_LPAREN       ("(")
#define mTOKEN_RPAREN       (")")
#define mTOKEN_SENTENCE     ("sentence")
#define mTOKEN_PARAGRAPH    ("paragraph")
#define mTOKEN_PHRASE       ("phrase")
#define mTOKEN_CONCEPT      ("concept")


/*========================================================*/
/* This function creates the session.                     */
/*========================================================*/

void InitSession( CBR_VOID *pUCB, CBR_CHAR *path, CBR_VOID **phSSN ) 
{
    CBR_INT      status;
    CBR_INT      ni;  
    CBR_ITEM     pi[1];

    pUCB = (CBR_VOID *)NULL;                                          /*1*/
    ni = 0;
    if (path)
    {
        pi[0].type = CBR_C_IN_ITEM;
        pi[0].code = CBR_I_SSN_PATHNAME;
        pi[0].buffer = path;
        pi[0].length = strlen( path);
        pi[0].status = CBR_S_SUCCESS;
        ni = 1;
    }

    status = cbr_create( CBR_O_SSN, (CBR_VOID *)NULL, ni, pi, pUCB, phSSN );
    if( status != CBR_S_SUCCESS )
    {
        demo_error_display( "Session", status, CBR_O_SSN, *phSSN );
        exit( -1 );
    }

    return ;                                                            /*2*/
}

/*========================================================*/
/* This function gets all the names of the profiles in    */
/* a session.                                             */
/*========================================================*/

void GetProfileNames( CBR_VOID *hSSN, int *np, char *names[], CBR_VOID *pUCB)
{
    CBR_INT     status;
    CBR_INT     ni;
    CBR_ITEM    pi[2];
    int         c;

    pi[0].type = CBR_C_OUT_ITEM;
    pi[0].code = CBR_I_PRFL_TOTAL;
    pi[0].buffer = np;
    pi[0].length = sizeof( *np );
    pi[0].status = CBR_S_SUCCESS;
    ni = 1;

    status = cbr_getinfo( CBR_O_SSN, hSSN, ni, pi, pUCB );
    if( status != CBR_S_SUCCESS ) {
        demo_error_display( "Profile", status, CBR_O_SSN, hSSN );
        exit( -1 );
    }

    pi[0].type = CBR_C_IN_ITEM;
    pi[0].code = CBR_I_PRFL_NUMBER;
    pi[0].buffer = &c;
    pi[0].length = sizeof( c );
    pi[0].status = CBR_S_SUCCESS;
    ni = 2;

    for( c=1; c <= *np; c++ ) 
    {
        names[c-1] = malloc( MAX_BUFF * sizeof( char ) );
        pi[1].type = CBR_C_OUT_ITEM;
        pi[1].code = CBR_I_PRFL_NAME;
        pi[1].buffer = names[c-1];
        pi[1].length = MAX_BUFF;
        pi[1].status = CBR_S_SUCCESS;
        
        status = cbr_getinfo( CBR_O_SSN, hSSN, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS ) {
            demo_error_display( "Profile", status, CBR_O_SSN, hSSN );
            exit( -1 );
        }
    }
}


/*========================================================*/
/* This function opens a profile.                         */
/*========================================================*/

void InitProfile( CBR_VOID *pUCB, CBR_VOID *hSSN, 
                    CBR_INT entry, CBR_VOID **phPRFL )
{
    CBR_INT      status;
    CBR_INT      ni;
    CBR_ITEM     pi[2];
                                                                        /*1*/
    pi[0].type   = CBR_C_IN_ITEM;                   
    pi[0].code   = CBR_I_PRFL_NUMBER;          /* input profile number */
    pi[0].buffer = &entry;
    pi[0].length = sizeof( entry );
    pi[0].status = CBR_S_SUCCESS;

    pi[1].type   = CBR_C_OUT_ITEM;                  
    pi[1].code   = CBR_I_PRFL_HANDLE;          /* output profile handle */
    pi[1].buffer = phPRFL;
    pi[1].length = 0;
    pi[1].status = CBR_S_SUCCESS;
    ni = 2;

    status = cbr_getinfo( CBR_O_SSN, hSSN, ni, pi, pUCB );               /*2*/
    if( status != CBR_S_SUCCESS )
    {
        demo_error_display( "Profile", status, CBR_O_SSN, hSSN );
        exit( -1 );
    }

    ni = 0;                                                            /*3*/
    status = cbr_open( CBR_O_PRFL, ni, pi, pUCB, *phPRFL );
    if( status != CBR_S_SUCCESS )
    {
        demo_error_display( "Profile", status, CBR_O_SSN, hSSN );
        exit( -1 );
    }

    return;                                                            /*4*/
}


/*========================================================*/
/* This function asks CBR for the names of all the        */
/* collections in a particular profile.                   */
/*========================================================*/

void GetCollectionNames(CBR_VOID *hPRFL, int *nc,char *names[], CBR_VOID *pUCB)
{
    CBR_INT     status;
    CBR_INT     ni;
    CBR_ITEM    pi[2];
    int c;

    pi[0].type = CBR_C_OUT_ITEM;
    pi[0].code = CBR_I_CLCN_TOTAL;
    pi[0].buffer = nc;
    pi[0].length = sizeof( *nc );
    pi[0].status = CBR_S_SUCCESS;
    ni = 1;

    status = cbr_getinfo( CBR_O_PRFL, hPRFL, ni, pi, pUCB );
    if( status != CBR_S_SUCCESS ) {
        demo_error_display( "Profile", status, CBR_O_PRFL, hPRFL );
        exit( -1 );
    }

    pi[0].type = CBR_C_IN_ITEM;
    pi[0].code = CBR_I_CLCN_NUMBER;
    pi[0].buffer = &c;
    pi[0].length = sizeof( c );
    pi[0].status = CBR_S_SUCCESS;
    ni = 2;

    for( c=1; c <= *nc; c++ ) {
        names[c-1] = malloc( MAX_BUFF * sizeof( char ) );
        pi[1].type = CBR_C_OUT_ITEM;
        pi[1].code = CBR_I_CLCN_NAME;
        pi[1].buffer = names[c-1];
        pi[1].length = MAX_BUFF;
        pi[1].status = CBR_S_SUCCESS;
        
        status = cbr_getinfo( CBR_O_PRFL, hPRFL, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS ) {
            demo_error_display( "Profile", status, CBR_O_PRFL, hPRFL );
            exit( -1 );
        }
    }
}

/*========================================================*/
/* This function opens a collection.                      */
/*========================================================*/

void InitCollection(CBR_VOID *hPRFL, int entry, 
                    CBR_VOID **phCLCN, CBR_VOID *pUCB)
{
    CBR_INT     status;
    CBR_INT     ni;
    CBR_ITEM    pi[3];
    CBR_CHAR    cname[MAX_BUFF];

    pi[0].type = CBR_C_IN_ITEM;
    pi[0].code = CBR_I_CLCN_NUMBER;
    pi[0].buffer = &entry;
    pi[0].length = sizeof( entry );
    pi[0].status = CBR_S_SUCCESS;

    pi[1].type = CBR_C_OUT_ITEM;
    pi[1].code = CBR_I_CLCN_HANDLE;
    pi[1].buffer = phCLCN;
    pi[1].length = 0;
    pi[1].status = CBR_S_SUCCESS;

    pi[2].type = CBR_C_OUT_ITEM;
    pi[2].code = CBR_I_CLCN_NAME;
    pi[2].buffer = cname;
    pi[2].length = MAX_BUFF;
    pi[2].status = CBR_S_SUCCESS;
    ni=3;
    
    status = cbr_getinfo( CBR_O_PRFL, hPRFL, ni, pi, pUCB );
    if( status != CBR_S_SUCCESS ) {
        demo_error_display( "Profile", status, CBR_O_PRFL, hPRFL );
        exit( -1 );
    }

#ifdef DEBUG
    printf( "Opening Collection %s\n", cname );
#endif

    ni = 0;
    status = cbr_open( CBR_O_CLCN, ni, pi, pUCB, *phCLCN );
    if( status != CBR_S_SUCCESS ) {
        demo_error_display( "Collection", status, CBR_O_PRFL, hPRFL );
        exit( -1 );
    }
#ifdef DEBUG
    printf( "Collection Opened\n" );
#endif
}
    

/*========================================================*/
/* This function closes a collection.                     */
/*========================================================*/

void CloseCollection( CBR_VOID **phCLCN, CBR_VOID *pUCB)

{
    CBR_INT     status;
    CBR_INT     ni;
    CBR_ITEM    pi[1];
    CBR_INT     update = FALSE;

    ni = 0;
    status = cbr_close( CBR_O_CLCN, update, ni, pi, pUCB, phCLCN );
    if( status != CBR_S_SUCCESS ) 
    {
        demo_error_display("CLCN information", status, CBR_O_CLCN, *phCLCN);
        exit( -1 );
    }
#ifdef DEBUG
    printf( "Collection Closed\n" );
#endif
}

/*---------------------------------------------------------*/
/*  process search commands                                */
/*---------------------------------------------------------*/

int MakeQuery(char *query_txt, 
              CBR_VOID  *hSSN, 
              CBR_VOID  *pUCB,
              CBR_VOID  *hCSET, 

              CBR_QUERY q_items[], 
              CBR_CHAR  words[cMTQ][MAX_BUFF+1],
              CBR_VOID **cbr_query)
{
    int           status      ;   /*  generic status return value     */
    int           ni          ;   /*  number of paramters             */
    CBR_ITEM      pi[5]       ;   /*  parameters                      */
    int           error       ;   /*  query error number              */
    char         *ptr;
    int           count;
    CBR_QUERY    qilist [2]   ;   /*   CBR query items                */
    char         *query = NULL;

    if (!query_txt)
       return(TRUE);

    if (!strlen(query_txt))
       return(TRUE);

    /* clear previous query handle */

    if (*cbr_query != (void *) NULL)
    {
        CBR_M_CLR(pi, ni);
        status = cbr_delete(CBR_O_SQRY, ni, pi, pUCB, cbr_query) ;
        if (status)
        {
            demo_error_display( "Search query delete",status,CBR_O_SQRY, cbr_query);
            exit(-1);
        }
    }

    if (words == NULL)
    {
        query = query_txt;
    }    
    else
    {
        while (ptr = strchr(query_txt,'\n'))
           *ptr = ' ';

       Tokenize(&query, q_items, query_txt, words, &count, hCSET);
    }

    CBR_M_CLR(pi, ni);
    CBR_M_INP(pi, ni, CBR_I_SQRY_QUERY, query, strlen(query) );

   status = cbr_create (CBR_O_SQRY, hSSN, ni, pi, pUCB, cbr_query) ;
   if ( status != CBR_S_SUCCESS )
   {
      /* See if CBR returns error information */

      if (status == CBR_S_BAD_QUERY)
      {
         ni           = 0;
         status = cbr_getinfo(CBR_O_SQRY, *cbr_query, ni, pi, pUCB);
         if ( status != CBR_S_SUCCESS )
            demo_error_display("Search query", status, CBR_O_SQRY, *cbr_query);

         return(FALSE);
       }
       else
       {
        demo_error_display("Search query", status, CBR_O_SQRY, *cbr_query);
       }

       return(TRUE);
    }

    if (words != NULL)
        free(query);

    return(FALSE);

} 

/*---------------------------------------------*/
/* Takes the user input, uses an "application" */
/* specific syntax and constructs a CBRS query */
/*---------------------------------------------*/
int  Tokenize(search_text, q_items, topic_text, words, count, hCSET)

char     **search_text;
CBR_QUERY *q_items;
char      *topic_text;
char       words[cMTQ][MAX_BUFF+1];
int       *count;
void      * hCSET           ;   /*r  handle of concept set              */
{
   char  token[MAX_BUFF+1];
   int    cur_size;
   int    size    ;   
   char  *txt = NULL;
   char  *ptr;
   int   last_cnt;
   int   cnt;
   int   position;
   int   len,i;
   int   concept_pending ;   /*   a concept indicated by keyword is supplied  */
   int   check_for_concept;  /*   check for a potential concept      */
   int   expr_pending;
   int   phrase_cnt;         
   int   status = CBR_S_SUCCESS;
   CBR_VOID *cbr_concept;
   void *pUCB = NULL;
    CBR_INT     ni;
    CBR_ITEM    pi[1];

   /**/

   check_for_concept = concept_pending = FALSE;

   position = 0;
   phrase_cnt = last_cnt = cnt = 0;

   while (get_token(topic_text, token,"()",&position))
   {
      /* lower case and save word */

      len = strlen(token);
      for (i = 0 ; i < len ; i++)
         token[i] = tolower(token[i]);

      strcpy(words[cnt],token);
      
      /* build CBR_QUERY stack */

      q_items[cnt].maxlength = MAX_BUFF;
      q_items[cnt].attribute = NULL;
      q_items[cnt].cset      = NULL;
      q_items[cnt].length = strlen(token) ;
      q_items[cnt].query  = &words[cnt][0] ;
      
      check_for_concept = FALSE;

      if (strcmp(token,"and") == 0)
      {
          q_items[cnt].type = CBR_I_OP_AND;
             
          if (phrase_cnt == 1)
            check_for_concept = TRUE;

          phrase_cnt = 0;
      }
      else if (strcmp(token,"or") == 0)
      {
          q_items[cnt].type = CBR_I_OP_OR;

          if (phrase_cnt == 1)
            check_for_concept = TRUE;

          phrase_cnt = 0;
      }
      else if (strcmp(token,"not") == 0)
      {
          q_items[cnt].type = CBR_I_OP_NOT;
      }
      else if (strcmp(token,"(") == 0)
      {
          q_items[cnt].type = CBR_I_OP_LPAREN;
      }
      else if (strcmp(token,")") == 0)
      {
          q_items[cnt].type = CBR_I_OP_RPAREN;
      }
      else if (strcmp(token,"sentence") == 0)
      {
          q_items[cnt].type = CBR_I_OP_SENTENCE;
      }
      else if (strcmp(token,"paragraph") == 0)
      {
          q_items[cnt].type = CBR_I_OP_PARAGRAPH;
      }
      else if (strcmp(token,"phrase") == 0)
      {
          q_items[cnt].type = CBR_I_OP_PHRASE;
      }
      else if (strcmp (token, "concept") == 0)
      {
         concept_pending  = TRUE ;      
      }
      else
      {  
         if (concept_pending)
         {
            q_items[cnt].type = CBR_I_OP_CONCEPT;
            concept_pending   = FALSE;
         }
         else
         {
            q_items[cnt].type   = CBR_I_OP_WORD ;
            
            last_cnt = cnt;      /* may check for concept with next token */
   
            phrase_cnt++;
         }
      } 

      if (!concept_pending)        
         cnt++ ;                                                     

      if (check_for_concept) 
      {
        if (get_a_concept(pUCB,q_items[last_cnt].query, hCSET, &cbr_concept))
        {
           q_items[last_cnt].type = CBR_I_OP_CONCEPT;

           /* close now unneeded concept handle */

           CBR_M_CLR(pi, ni);
           status = cbr_close (CBR_O_CNCPT, FALSE, ni, pi, pUCB, &cbr_concept) ;
           if ( status != CBR_S_SUCCESS )
              demo_error_display( "concept", status, CBR_O_CNCPT, cbr_concept);
        }
      }

   }

   if (phrase_cnt == 1)
   {
      if (get_a_concept(pUCB, q_items[last_cnt].query, hCSET, &cbr_concept))
      {
         q_items[last_cnt].type = CBR_I_OP_CONCEPT;

         /* close now unneeded concept handle */

           CBR_M_CLR(pi, ni);
           status = cbr_close (CBR_O_CNCPT, FALSE, ni, pi, pUCB, &cbr_concept) ;
           if ( status != CBR_S_SUCCESS )
              demo_error_display( "concept", status, CBR_O_CNCPT, cbr_concept);
      }
   }

   *count = cnt;
     
   /* make search query */

   expr_pending = FALSE;
   cur_size = size = 0;
   for (i = 0; (i < cnt) ; i++)
   {
        switch( q_items[i].type )
        {
        case CBR_I_OP_WORD:                                             

            if( !expr_pending)                                        
                if ( ((i+1)< cnt) && (q_items[(i+1)].type == CBR_I_OP_WORD) )
                {
                    expr_pending = TRUE;
                    strcpy(token,"PHRASE(");
                    len = strlen(token);
                    status = txt__expand_token(&txt, token, len, 
                                                    &cur_size, &size);
                    if (status)
                        break;
                }

            ptr = strpbrk( q_items[i].query, "*%" );                   
            if(!ptr)                                    
            {
                sprintf(token,"'%s'",q_items[i].query);
            }
            else
            {
                sprintf(token,"\"%s\"",q_items[i].query);
            }

            if ( ((i+1)< cnt) && (q_items[(i+1)].type == CBR_I_OP_WORD) )
                strcat(token,",");

            strcat(token," ");
            len = strlen(token);
            status = txt__expand_token(&txt, token, len, &cur_size, &size);
            if (status)
                break;

            break;

        case CBR_I_OP_CONCEPT:

            sprintf(token,"{%s} ",q_items[i].query);
            len = strlen(token);

            status = txt__expand_token(&txt, token, len, &cur_size, &size);
            if (status)
                break;

            break;

        case CBR_I_OP_PHRASE:                                           
        case CBR_I_OP_SENTENCE:                                         
        case CBR_I_OP_PARAGRAPH:                                        

            expr_pending = TRUE;

            sprintf(token,"%s(",q_items[i].query);
            len = strlen(token);
            status = txt__expand_token(&txt, token, len, &cur_size, &size);
            if (status)
                break;

            break;

        case CBR_I_OP_OR:   
        case CBR_I_OP_AND:  
        case CBR_I_OP_NOT:  

            if( expr_pending )
            {
                expr_pending = FALSE;
                strcpy(token,") ");
                len = strlen(token);
                status = txt__expand_token(&txt, token, len, &cur_size, &size);
                if (status)
                    break;
            }

            sprintf(token,"%s ",q_items[i].query);
            len = strlen(token);
            status = txt__expand_token(&txt, token, len, &cur_size, &size);
            if (status)
                break;

            break;

        case CBR_I_OP_RPAREN:                                           
        case CBR_I_OP_LPAREN:                                           

            sprintf(token,"%s ",q_items[i].query);
            len = strlen(token);
            status = txt__expand_token(&txt, token, len, &cur_size, &size);
            if (status)
                break;

            break;

        default:

            status = CBR_S_SUCCESS;
            break;
        }

        if (status != CBR_S_SUCCESS)
            return( status ); 
    }

    if( expr_pending )                                                     
    {
        expr_pending = FALSE;
        strcpy(token,") ");
        len = strlen(token);
        status = txt__expand_token(&txt, token, len, &cur_size, &size);
    }

    *search_text = txt;

    return(status);

}

/*========================================================*/
/* This function executes the search                      */
/*========================================================*/
int DoSearch( CBR_VOID *hSQRY, CBR_VOID *hCLCN, 
              CBR_VOID *pUCB, CBR_VOID **phRSLT )
{
    CBR_INT      status; 
    CBR_INT      ni;
    CBR_ITEM     pi[2];
    int          state;
    CBR_INT      i = 0;
    CBR_INT      total;
    CBR_INT      avail;
    CBR_INT      old_total;

    /**/                                

    CBR_M_CLR(pi, ni);
    CBR_M_INP(pi, ni, CBR_I_CLCN_HANDLE, hCLCN, 0 );

    total = old_total = 0;
    while (status = cbr_search (CBR_O_CLCN, hSQRY, ni, pi, pUCB, phRSLT))
    {
        switch(status)
        {
        case CBR_S_IN_PROGRESS:

        ++i;

        if (total != old_total)
        {
            printf("%d) Found %d of %d documents\n",i,total,avail);
            old_total = total;
        }

        break;

        case CBR_S_BAD_QUERY:

            return (FALSE) ;
            break;

        default:

            demo_error_display( "Collection", status, CBR_O_CLCN, hCLCN );
            exit (EXIT_FAILURE) ;
            break;
        }
    }

#ifdef DEBUG
    printf( "Final results:\n");
#endif
    return( TRUE );

}

/*========================================================*/
/* This function fetches and displays the document        */
/* selected by the user                                   */
/*========================================================*/
void OperateOnResults( CBR_VOID *pUCB, CBR_VOID *hRSLT, int selection, 
                       CBR_INT  (* fcn_adr)(), CBR_VOID *document_data,
                       CBR_VOID *user_data)
{
    char          object[MAX_BUFF];
    char          doctype[MAX_BUFF];
    int           status;
    int           ni;
    CBR_ITEM      pi[5];

                                        /* retrieve selected item */
    ni = 3;
    pi[0].type   = CBR_C_IN_ITEM;
    pi[0].code   = CBR_I_DOC_NUMBER;  
    pi[0].buffer = &selection;
    pi[0].status = FALSE;
    
    pi[1].type   = CBR_C_OUT_ITEM;
    pi[1].code   = CBR_I_DOC_IDENTIFIER;
    pi[1].length = MAX_BUFF;
    pi[1].buffer = object;
    pi[1].status = FALSE;
    
    pi[2].type   = CBR_C_OUT_ITEM;
    pi[2].code   = CBR_I_DOC_TYPE;
    pi[2].length = MAX_BUFF;
    pi[2].buffer = doctype;
    pi[2].status = FALSE;
    
    status = cbr_getinfo(CBR_O_RSLT, hRSLT, ni, pi, pUCB);
    if ( status != CBR_S_SUCCESS )
        demo_error_display( "Result", status, CBR_O_RSLT, hRSLT );

   /* create word hits list */

   status = tpu_display(hRSLT, 
                        selection,
                        pUCB);

    if (fcn_adr)
    {
        strcpy(document_data, object);
        (*fcn_adr)(document_data, user_data);
    }

}


/*========================================================*/
/* This function fetches the concept handle to a concept  */
/* if the concept exists, returns false if not found      */
/*========================================================*/
int get_a_concept(pUCB, name, hCSET, phCNCPT)

    CBR_VOID   * pUCB               ;   /*m: opaque user control block      */
    CBR_CHAR   * name               ;   /*r: concept name                   */
    CBR_VOID   * hCSET              ;   /*r: concept set handle             */
    CBR_VOID  **phCNCPT             ;   /*w: concept handle                 */
{
    CBR_INT      status             ;   /*   generic status return value    */
    CBR_INT      ni                 ;   /*   number of paramters            */
    CBR_ITEM     pi [3]             ;   /*   parameters                     */
    CBR_INT      found = FALSE;

    /**/

    if (hCSET == (void *) NULL)                                 /* 1 */
       return(FALSE);

    pi[0].type   = CBR_C_IN_ITEM ;                              /* 2 */
    pi[0].code   = CBR_I_CNCPT_SYSID ;          /* input concept name */
    pi[0].buffer = name ;
    pi[0].length = strlen (name) ;
    pi[0].status = CBR_S_SUCCESS ;

    pi[1].type   = CBR_C_OUT_ITEM ;             /* output concept handle */  
    pi[1].code   = CBR_I_CNCPT_HANDLE ;
    pi[1].buffer = phCNCPT ;
    pi[1].length = sizeof(phCNCPT) ; 
    pi[1].status = CBR_S_SUCCESS ;

    ni = 2 ;

    status = cbr_getinfo(CBR_O_CSET, hCSET, ni, pi, pUCB) ;     /* 3 */ 
    if ( status == CBR_S_SUCCESS )                  
    {
       found = TRUE;                                            /* 4 */
    }
    else 
    {
       found = FALSE;
    }

    return(found);
}


/*========================================================*/
/* This function tokenizes the user input into tokens     */
/*========================================================*/
char * get_token (line, buffer, delimiters, line_pos)

    char *  line        ;   /*r: line to tokenize                       */
    char *  buffer      ;   /*m: place to put the token                 */
    char *  delimiters  ;   /*r: left and right parenthetic operators   */
    int  *  line_pos    ;   /*r: current line position                  */
{
    int     i           ;   /*   generic index                          */
    int     begin       ;   /*   beginning tokenization position        */
    int     len         ;   /*   length of string to tokenize           */
    char    ch          ;   /*   current character                      */

    /**/

    begin = *line_pos ;                                                 /*1*/

    if (line == NULL)
        return ( (char *) NULL) ;

    if ((len = strlen(line)) == 0)
        return ((char *) NULL) ;

    if (begin > len)
        return ((char *) NULL) ;
                                                                      /*2*/

    for (i = begin,ch = *(line + i) ; ((ch != '\0') && (!isgraph(ch))) ; ) 
        ch = *(line + ++i) ;
    if (ch == '\0')
       return ((char *) NULL) ;

    begin = i ;

    if ( (delimiters != (char *) NULL)  &&                            /*3*/
         (strchr(delimiters, (int) ch) != (char *) NULL) )
    {
        buffer[0] = ch ;
        buffer[1] = '\0' ;
        i++ ;
    }
    else
    {
        ch = *(line + i) ; 
        while ( (ch != '\0') && isgraph(ch) )
        {
           if ( (delimiters != (char *) NULL) && 
                (strchr(delimiters, (int) ch) != (char *) NULL) )
           {
              break ;
           }

           ch = *(line + ++i) ;
        }
        
        strncpy( buffer, (line + begin), (i - begin) ) ;
        buffer[(i - begin)] = '\0' ;
    }

    *line_pos = i ;                                                     /*4*/
     
    return (buffer) ;
}

/*========================================================*/
/* This function get the names of all the concept sets    */
/* for a particular collection.                           */
/*========================================================*/

void GetConceptSetNames(CBR_VOID *pUCB,CBR_VOID *hCLCN, int *nc, char *names[])
{
    CBR_INT     status;
    CBR_INT     ni;
    CBR_ITEM    pi[2];
    int c;

    pi[0].type = CBR_C_OUT_ITEM;
    pi[0].code = CBR_I_CSET_TOTAL;
    pi[0].buffer = nc;
    pi[0].length = FALSE;
    pi[0].status = FALSE;
    ni = 1;

    status = cbr_getinfo( CBR_O_CLCN, hCLCN, ni, pi, pUCB );
    if( status != CBR_S_SUCCESS ) {
        demo_error_display( "collection", status, CBR_O_CLCN, hCLCN );
        exit( -1 );
    }

    pi[0].type = CBR_C_IN_ITEM;
    pi[0].code = CBR_I_CSET_NUMBER;
    pi[0].buffer = &c;
    pi[0].length = sizeof( c );
    pi[0].status = FALSE;
    ni = 2;

    for( c=1; c <= *nc; c++ ) {
        names[c-1] = malloc( MAX_BUFF * sizeof( char ) );
        pi[1].type = CBR_C_OUT_ITEM;
        pi[1].code = CBR_I_CSET_NAME;
        pi[1].buffer = names[c-1];
        pi[1].length = MAX_BUFF;
        pi[1].status = CBR_S_SUCCESS;
        status = cbr_getinfo( CBR_O_CLCN, hCLCN, ni, pi, pUCB );
        if( status != CBR_S_SUCCESS ) {
            demo_error_display( "collection", status, CBR_O_CLCN, hCLCN );
            exit( -1 );
        }
    }
}


/*========================================================*/
/* This function initializes a concept set.               */
/*========================================================*/

void InitConceptSet(CBR_VOID *hCLCN, int entry, int *total,
                    CBR_VOID **phCSET, CBR_VOID *pUCB )
{
    CBR_VOID  * hUCSET  = NULL      ;   /*   user concept set handle        */
    CBR_VOID  * hSCSET  = NULL      ;   /*   system concept set handle      */
    CBR_VOID  * hCSET  = NULL       ;   /*   concept set handle             */
    CBR_VOID  * hCNCPT = NULL       ;   /*   concept handle                 */
    CBR_INT     status              ;   /*   generic status return value    */
    CBR_INT     ni                  ;   /*   number of paramters            */
    CBR_ITEM    pi [3]              ;   /*   parameters                     */
    CBR_CHAR    cname [BUFSIZ+1]    ;   /*   collection name                */
    CBR_CHAR    line [BUFSIZ+1]     ;   /*   line buffer                    */
    CBR_INT     num                 ;   /*   number of concepts retrieved   */
    CBR_INT     found = 0           ;   /*   flag indicating found concept  */
    CBR_INT     line_cnt            ;   /*   count of lines displayed       */
    CBR_INT     type                ;   /*   concept set type               */
    CBR_INT     cset_total          ;   /*   total number of concepts       */
    CBR_INT     i                   ;   

    /**/
                                                                        
    CBR_M_CLR(pi, ni);
    CBR_M_OUT(pi, ni, CBR_I_CSET_TOTAL, &cset_total, sizeof(cset_total) );
    status = cbr_getinfo(CBR_O_CLCN, hCLCN, ni, pi, pUCB) ;             
    if ( status != CBR_S_SUCCESS )
    {
        demo_error_display("CSET information", status, CBR_O_CLCN, hCLCN);
        exit (EXIT_FAILURE) ;
    }

    for (i = 1 ; (i <= cset_total) ; i++)
    {
        CBR_M_CLR(pi, ni);
        CBR_M_INP(pi, ni, CBR_I_CSET_NUMBER, &i, sizeof( i ) );
        CBR_M_OUT(pi, ni, CBR_I_CSET_HANDLE, &hCSET, 0 );
        CBR_M_OUT(pi, ni, CBR_I_CSET_TYPE, &type, 0 );
        status = cbr_getinfo(CBR_O_CLCN, hCLCN, ni, pi, pUCB) ;             
        if ( status != CBR_S_SUCCESS )
        {
            demo_error_display("CSET information", status, CBR_O_CLCN, hCLCN);
            exit (EXIT_FAILURE) ;
        }

        if (type == CBR_C_SYSTEM)
            hSCSET = hCSET;
        else if (type == CBR_C_USER)
            hUCSET = hCSET;
    }

    if (hUCSET)
        *phCSET = hUCSET;
    else
        *phCSET = hSCSET;

    
    *total = 0;
    if (*phCSET)
    {
        /* get total number of concepts availible */

        pi[0].type   = CBR_C_OUT_ITEM ;              
        pi[0].code   = CBR_I_CNCPT_TOTAL;
        pi[0].buffer = total;
        pi[0].length = sizeof (*total) ;
        pi[0].status = CBR_S_SUCCESS ;

        ni = 1;
        status = cbr_getinfo(CBR_O_CSET, *phCSET, ni, pi, pUCB) ;             
        if ( status != CBR_S_SUCCESS )
        {
            demo_error_display( "concept set", status, CBR_O_CSET, *phCSET );
            exit( -1 );
        }
    }
}
    
/*----------------------------------------*/
/* fetch list of concepts                 */
/*----------------------------------------*/
int FetchCbrConcepts(pUCB, hCSET, total, num, names, nc, nret)

CBR_VOID   * pUCB               ;   /*m: opaque user control block      */
CBR_VOID   * hCSET              ;   /*w: concept set handle             */
CBR_INT      total              ; 
CBR_INT      num                ;   /*   number of concepts retrieved   */
CBR_CHAR   * names[]            ;   /*w: name buffer for concepts       */
CBR_INT      nc                 ;   /*w: number of concept buffers      */
CBR_INT    * nret               ;   /*w: number of concept buffers      */
{
    CBR_VOID  * hCNCPT = NULL       ;   /*   concept handle                 */
    CBR_INT     status              ;   /*   generic status return value    */
    CBR_INT     ni                  ;   /*   number of paramters            */
    CBR_ITEM    pi [3]              ;   /*   parameters                     */
    CBR_INT     total_concepts;
    CBR_INT     cnt;
    CBR_INT     ci;
    CBR_INT     num_nc;
    CBR_CHAR    cname[255+1];

    /**/

    pi[0].type   = CBR_C_IN_ITEM ;              
    pi[0].code   = CBR_I_CNCPT_NUMBER ;          /* input concept number */
    pi[0].buffer = &cnt;
    pi[0].length = sizeof (cnt) ;
    pi[0].status = CBR_S_SUCCESS ;

    pi[1].type   = CBR_C_OUT_ITEM ;             /* output concept name */  
    pi[1].code   = CBR_I_CNCPT_SYSID ;
    pi[1].buffer = cname ;
    pi[1].length = MAX_BUFF;
    pi[1].status = CBR_S_SUCCESS ;
    ni = 2 ;

    for (ci = 0, cnt = num; (cnt <= total) && (ci < nc) ; cnt++, ci++)
    {
       status = cbr_getinfo(CBR_O_CSET, hCSET, ni, pi, pUCB) ;            
       if ( status != CBR_S_SUCCESS )
       {
            demo_error_display( "concept set", status, CBR_O_CSET, hCSET );
           exit (EXIT_FAILURE) ;
       }

       strcpy(names[ci], cname);

    } /* while concepts exist */

    *nret = ci;

    return ;                                                           
}

    
/*----------------------------------------*/
/* fetch list of results                  */
/*----------------------------------------*/
int FetchCbrResults(pUCB, hRSLT, total, num, results, rankings, nc, nret)

CBR_VOID   * pUCB               ;   /*m: opaque user control block      */
CBR_VOID   * hRSLT              ;   /*w: result handle                  */
CBR_INT      total              ; 
CBR_INT      num                ;   /*   number of results retrieved    */
CBR_CHAR   * results[]          ;   /*w: name buffer for results        */
CBR_INT      rankings[]		    ;   /*w: int buffer for rankings 	*/
CBR_INT      nc                 ;   /*w: number of results buffers      */
CBR_INT    * nret               ;   /*w: number of results buffers      */
{
    CBR_RLE     cbr_results[MAX_TITLE];
    CBR_CHAR    titles[MAX_TITLE][MAX_BUFF+1];
    CBR_INT     status              ;   /*   generic status return value    */
    CBR_INT     ni                  ;   /*   number of paramters            */
    CBR_ITEM    pi [3]              ;   /*   parameters                     */
    CBR_INT     cnt, max;
    CBR_INT     i, k, limit;
    CBR_CHAR    buffer[255+1];

    /**/

    max = (nc > MAX_TITLE) ? (MAX_TITLE) : (nc);
    for (i = 0; i < max ; i++) 
    {
       cbr_results[i].visible_text.word      = &titles[i][0]; /* text buffer */
       cbr_results[i].visible_text.maxlength = MAX_BUFF;
       cbr_results[i].visible_text.location  = 0;
       cbr_results[i].visible_text.length    = 0;
       cbr_results[i].visible_text.attribute = NULL;
       cbr_results[i].visible_text.begin_end = 0;
    }

                                        /* display results until done */
    ni    = 2 ;
    pi[0].type   = CBR_C_IN_ITEM;
    pi[0].code   = CBR_I_DOC_NUMBER;
    pi[0].buffer = &num;
    pi[0].length = FALSE;
    pi[0].status = FALSE;
    
    pi[1].type   = CBR_C_OUT_ITEM;
    pi[1].code   = CBR_I_RSLT_ARRAY;
    pi[1].buffer = cbr_results;
    pi[1].length = max;
    pi[1].status = FALSE;
        

    for (i = 0; (i < nc) && (num <= total) ; num += i )
    {
    	status = cbr_getinfo( CBR_O_RSLT, hRSLT, ni, pi, pUCB);
    	if( status != CBR_S_SUCCESS )
	    demo_error_display( "results", status, CBR_O_RSLT, hRSLT );
	
        for (k = 0; (i < nc) && (k < pi[1].length); k++, i++)
        {
    	    strcpy( results[i], cbr_results[k].visible_text.word );
    	    rankings[i] = cbr_results[k].entry_rank;
        }
    }

    *nret = i;

    return ;                                                           
}

/*.
**.
**. Function:
**.
**.     void GetConceptDef()
**.     
**. Description:
**. 
**.     This routine is responsible for retrieving the concept definition 
**.     in otl format from the concept set.  Returns false if the concept
**.     does not exist
**. 
**. Algorithm:
**. 
**. Return values:
**.
**.     none
**.
**  Version     Date        Author          Change
**  -------     ----        ------          ------
**  1.0         21-Aug-1990 R. L. Cohen     Created
*/

int GetConceptDef(pUCB, entry, name, hCSET, definition, found)

    CBR_VOID   * pUCB               ;   /*m: opaque user control block      */
    CBR_INT      entry              ;   /*r: entry number in object         */
    CBR_CHAR   * name               ;   /*r: concept name                   */
    CBR_VOID   * hCSET              ;   /*r: concept set handle             */
    CBR_CHAR   **definition         ;   /*w: ptr to concept definition      */
    CBR_INT    *found               ;   /*w: found concept flag             */
{
    CBR_INT      status             ;   /*   generic status return value    */
    CBR_INT      ni                 ;   /*   number of paramters            */
    CBR_ITEM     pi [3]             ;   /*   parameters                     */
    CBR_CHAR     cname [BUFSIZ+1]   ;   /*   concept name                   */
    CBR_INT      num,i              ;   /*   generic counters               */
    CBR_VOID   * hCNCPT             ;   /*   concept handle                 */
    CBR_INT      size               ;

    if (name == NULL)
    {    
       pi[0].type   = CBR_C_IN_ITEM ;              
       pi[0].code   = CBR_I_CNCPT_NUMBER ;          /* input concept number */
       pi[0].buffer = &entry ;
       pi[0].length = sizeof (entry) ;
       pi[0].status = CBR_S_SUCCESS ;

       pi[1].type   = CBR_C_OUT_ITEM ;             /* output concept name */  
       pi[1].code   = CBR_I_CNCPT_SYSID ;
       pi[1].buffer = cname; 
       pi[1].length = BUFSIZ;
       pi[1].status = CBR_S_SUCCESS ;

       pi[2].type   = CBR_C_OUT_ITEM ;             /* output concept name */  
       pi[2].code   = CBR_I_CNCPT_HANDLE ;
       pi[2].buffer = &hCNCPT ;
       pi[2].length = sizeof(hCNCPT) ; 
       pi[2].status = CBR_S_SUCCESS ;

       ni = 3 ;
    }
    else
    {
       pi[0].type   = CBR_C_IN_ITEM ;              
       pi[0].code   = CBR_I_CNCPT_SYSID ;          /* input concept name */
       pi[0].buffer = name ;
       pi[0].length = strlen (name) ;
       pi[0].status = CBR_S_SUCCESS ;

       pi[1].type   = CBR_C_OUT_ITEM ;             /* output concept name */  
       pi[1].code   = CBR_I_CNCPT_HANDLE ;
       pi[1].buffer = &hCNCPT ;
       pi[1].length = sizeof(hCNCPT) ; 
       pi[1].status = CBR_S_SUCCESS ;

       ni = 2 ;
    }

    status = cbr_getinfo(CBR_O_CSET, hCSET, ni, pi, pUCB) ;            
    if ( status == CBR_S_SUCCESS )
    {
       pi[0].type   = CBR_C_OUT_ITEM ;              
       pi[0].code   = CBR_I_CNCPT_DEF_SIZE ;        
       pi[0].buffer = &size ;
       pi[0].length = sizeof(size);
       pi[0].status = CBR_S_SUCCESS ;

       ni = 1 ;

       status = cbr_getinfo(CBR_O_CNCPT, hCNCPT, ni, pi, pUCB) ;            
       if ( status != CBR_S_SUCCESS )
       {
            demo_error_display( "concept", status, CBR_O_CNCPT, hCNCPT);
       }

       *definition = calloc(1, size + 1);

       pi[0].type   = CBR_C_OUT_ITEM ;              
       pi[0].code   = CBR_I_CNCPT_DEF ;        
       pi[0].buffer = *definition ;
       pi[0].length = size;
       pi[0].status = CBR_S_SUCCESS ;

       ni = 1 ;

       status = cbr_getinfo(CBR_O_CNCPT, hCNCPT, ni, pi, pUCB) ;            
       if ( status != CBR_S_SUCCESS )
       {
            demo_error_display( "concept", status, CBR_O_CNCPT, hCNCPT );
       }

       *found = TRUE;
    }
    else 
    {
       *found = FALSE;
       if (name)
          printf("%s not found\n",name);
       else
          printf("Entry %d not found\n",entry);
    }

    return(*found);
}

/*-------------------------------------------------*/
/* Uses the default function of CBRS               */
/*-------------------------------------------------*/
int  make_concept(query, concept_def, textbody, words, name, hCSET, language)

char     **query;
char     **concept_def;
char      *textbody;
char       words[cMTQ][MAX_BUFF+1];
char       name[MAX_BUFF+1];
void      *hCSET           ;        /*r  handle of concept set              */
char      *language        ;
{
    CBR_INT      status             ;   /*   generic status return value    */
    CBR_INT      ni                 ;   /*   number of input paramters      */
    CBR_ITEM     pi [5]             ;   /*   input parameters               */
    CBR_VOID    *hCNCPT;    
    CBR_VOID    *pUCB = NULL;
    CBR_INT      def_size           ;   /*   size of concept def            */
    CBR_INT      exp_size           ;   /*   size of expanded concept       */
    CBR_INT      update = 0         ;
    FILE       * outfile            ;
    
    /**/                                
    
    CBR_M_CLR(pi, ni);
    CBR_M_INP(pi, ni, CBR_I_CNCPT_LANGUAGE, language, strlen(language));
    CBR_M_INP(pi, ni, CBR_I_WORD_DAT, textbody, strlen(textbody) );

    status = cbr_create(CBR_O_CNCPT, hCSET, ni, pi, pUCB, &hCNCPT);
    if (status)
    {
        demo_error_display( "Collection", status, CBR_O_CNCPT, hCNCPT);
        exit(1);
    }

    def_size = 0;
    CBR_M_CLR(pi, ni);
    CBR_M_OUT(pi, ni, CBR_I_CNCPT_DEF_SIZE, &def_size,  sizeof(def_size) );

    status = cbr_getinfo(CBR_O_CNCPT, hCNCPT, ni, pi, pUCB) ;            
    if ( status != CBR_S_SUCCESS )
    {
        demo_error_display("1 cset - name", status, CBR_O_CNCPT, hCNCPT );
        exit (EXIT_FAILURE) ;
    }

    if (def_size)
        *concept_def = calloc(1,def_size);

    CBR_M_CLR(pi, ni);
    CBR_M_OUT(pi, ni, CBR_I_CNCPT_DEF, *concept_def, def_size);
    CBR_M_OUT(pi, ni, CBR_I_CNCPT_SYSID, name,  MAX_BUFF );
    status = cbr_getinfo(CBR_O_CNCPT, hCNCPT, ni, pi, pUCB) ;            
    if ( status != CBR_S_SUCCESS )
    {
        demo_error_display("2 cset - name", status, CBR_O_CNCPT, hCNCPT );
        exit (EXIT_FAILURE) ;
    }

    /* convert concept into a search form */

    if (query)
    {
        CBR_M_CLR(pi, ni);                                          
        status = cbr_parse(CBR_O_CNCPT, ni, pi, pUCB, hCNCPT);
        if ( status != CBR_S_SUCCESS )
        {
            demo_error_display("parse hCNCPT", status, CBR_O_CNCPT, hCNCPT);
            exit (EXIT_FAILURE);
        }
        
        CBR_M_CLR(pi, ni);                                          
        CBR_M_OUT(pi, ni, CBR_I_CNCPT_EXP_SIZE, &exp_size, sizeof(exp_size));
        status = cbr_getinfo(CBR_O_CNCPT, hCNCPT, ni, pi, pUCB);
        if ( status != CBR_S_SUCCESS )
        {
            demo_error_display("hCNCPT size", status, CBR_O_CNCPT, hCNCPT);
            exit (EXIT_FAILURE); 
        }
        *query = malloc( exp_size + 1);
        if( *query == NULL )
        {
            printf( "Memory allocation failed\n" );
            exit( EXIT_FAILURE );        
        }
        
        CBR_M_CLR(pi, ni);                                          
        CBR_M_OUT(pi, ni, CBR_I_CNCPT_EXPANDED, *query, exp_size );
        status = cbr_getinfo(CBR_O_CNCPT, hCNCPT, ni, pi, pUCB);
        if ( status != CBR_S_SUCCESS )
        {
            demo_error_display("hCNCPT defn", status, CBR_O_CNCPT, hCNCPT);
            exit (EXIT_FAILURE); 
        }

    }

    CBR_M_CLR(pi, ni);
    status = cbr_close (CBR_O_CNCPT, update, ni, pi, pUCB, &hCNCPT) ;
    if (status)
    {
        demo_error_display( "concept delete",status,CBR_O_CNCPT, hCNCPT);
    }
    return(FALSE);    
}


/*-------------------------------------------------*/
/* Uses the default function of CBRS               */
/*-------------------------------------------------*/
int  make_new_concept(query, concept_def, name, hCSET, language)

char     **query;
char      *concept_def;
char       name[MAX_BUFF+1];
void      *hCSET           ;        /*r  handle of concept set              */
char      *language        ;
{
    CBR_INT      status             ;   /*   generic status return value    */
    CBR_INT      ni                 ;   /*   number of input paramters      */
    CBR_ITEM     pi [5]             ;   /*   input parameters               */
    CBR_VOID    *hCNCPT;    
    CBR_VOID    *pUCB = NULL;
    CBR_INT      def_size           ;   /*   size of concept def            */
    CBR_INT      exp_size           ;   /*   size of expanded concept       */
    CBR_INT      update = 0         ;
    FILE       * outfile            ;
    
    /**/                                
    
    CBR_M_CLR(pi, ni);
    CBR_M_INP(pi, ni, CBR_I_CNCPT_LANGUAGE, language, strlen(language));
    CBR_M_INP(pi, ni, CBR_I_CNCPT_SYSID, name, strlen(name) );
    CBR_M_INP(pi, ni, CBR_I_CNCPT_DEF, concept_def, strlen(concept_def) );

    status = cbr_create(CBR_O_CNCPT, hCSET, ni, pi, pUCB, &hCNCPT);
    if (status)
    {
        demo_error_display( "Collection", status, CBR_O_CNCPT, hCNCPT);
        exit(1);
    }

    /* convert concept into a search form */

    if (query)
    {
        CBR_M_CLR(pi, ni);                                          
        status = cbr_parse(CBR_O_CNCPT, ni, pi, pUCB, hCNCPT);
        if ( status != CBR_S_SUCCESS )
        {
            demo_error_display("parse hCNCPT", status, CBR_O_CNCPT, hCNCPT);
            exit (EXIT_FAILURE);
        }
        
        CBR_M_CLR(pi, ni);                                          
        CBR_M_OUT(pi, ni, CBR_I_CNCPT_EXP_SIZE, &exp_size, sizeof(exp_size));
        status = cbr_getinfo(CBR_O_CNCPT, hCNCPT, ni, pi, pUCB);
        if ( status != CBR_S_SUCCESS )
        {
            demo_error_display("hCNCPT size", status, CBR_O_CNCPT, hCNCPT);
            exit (EXIT_FAILURE); 
        }
        *query = malloc( exp_size + 1);
        if( *query == NULL )
        {
            printf( "Memory allocation failed\n" );
            exit( EXIT_FAILURE );        
        }
        
        CBR_M_CLR(pi, ni);                                          
        CBR_M_OUT(pi, ni, CBR_I_CNCPT_EXPANDED, *query, exp_size );
        status = cbr_getinfo(CBR_O_CNCPT, hCNCPT, ni, pi, pUCB);
        if ( status != CBR_S_SUCCESS )
        {
            demo_error_display("hCNCPT defn", status, CBR_O_CNCPT, hCNCPT);
            exit (EXIT_FAILURE); 
        }

    }

    CBR_M_CLR(pi, ni);
    status = cbr_close (CBR_O_CNCPT, update, ni, pi, pUCB, &hCNCPT) ;
    if (status)
    {
        demo_error_display( "concept delete",status,CBR_O_CNCPT, hCNCPT);
    }
    return(FALSE);    
}

/*.
**.
**. Function:
**.
**.     int tpu_display ( hRSLT, choice, pUCB)
**.     
**.         CBR_VOID  * hRSLT   ;   (*r: result handle                  *)
**.         CBR_INT     choice  ;   (*r: document chosen                *)
**.         CBR_VOID  * pUCB    ;   (*r: opaque user control block      *)
**.
**. Description:
**. 
**.     This routine is responsible for fetching the word locators, and writing
**.     them out to a transfer file for subsequent TPU viewing.
**.     
**. Algorithm:
**.     
**.     The word locators are decoded into the line_id and the word offset 
**.     within the line_id, and written out to a transfer file, tpu_xfer_fname,
**.     "cbr_tpu_pipe" (note that "cbr_tpu_pipe" may refer to a VMS logical 
**.     name.)  A TPU procedure then reads this transfer file for these word
**.     locations.
**. 
**. Return values:
**.
**.     none
**.
**  Version     Date        Author          Change
**  -------     ----        ------          ------
**  1.10        22-OCT-1991 A. R. Hagel     boundary condition fix
**  1.0         14-FEB-1991 C.Chan          Created
*/
int tpu_display( hRSLT, choice, pUCB )

    CBR_VOID  * hRSLT               ;   /*r: result handle                  */
    CBR_INT     choice              ;   /*r: document chosen                */
    CBR_VOID  * pUCB                ;   /*r: opaque user control block      */
{
    int         i                   ;   /*   generic index                  */
    int         num_items           ;   /*   number of token items          */
    CBR_INT     status              ;   /*   generic status return value    */
    CBR_INT     ni                  ;   /*   number of input paramters      */
    CBR_ITEM    pi [16]             ;   /*   input parameters               */
    CBR_INT     tot_loc             ;   /*   total word locations           */
    CBR_INT     start               ;   /*   word location start            */
    CBR_INT     num_fet             ;   /*   num of word locations fetched  */

    cbr_locator locations[16]       ;   /*   word locations buffer          */
    FILE      * fp                  ;   /*   tpu locations transfer file fp */
    CBR_INT     line_id             ;   /*   line id                        */
    CBR_INT     line_off            ;   /*   word offset within line        */

    static char *tpu_xfer_fname = "cbr_tpu_pipe";

    /**/

    fp = fopen ( tpu_xfer_fname , "w" );
    start = 0;
    do 
    {
        CBR_M_CLR(pi, ni);
        CBR_M_INP(pi, ni, CBR_I_DOC_NUMBER,           &choice, sizeof(choice));
        CBR_M_INP(pi, ni, CBR_I_RSLT_LOCATION_NUMBER, &start,  sizeof(start));
        CBR_M_OUT(pi, ni, CBR_I_RSLT_LOCATIONS,       &locations[0], 16);

        status = cbr_getinfo (CBR_O_RSLT, hRSLT, ni, pi, pUCB) ;
        if ( status != CBR_S_SUCCESS )
        {
            fflush(fp);
            fclose(fp);
            return( start == 0 );
        }

        num_fet = pi[2].length;
        start += num_fet;

        for (i = 0 ; i < num_fet ; i++ )
        {
            line_id  = locations[i].id;
            line_off = locations[i].offset;
            fprintf(fp, "%6d|%3d|%3d|\n", line_id, line_off, 1 );
        }

    } while (num_fet == 16);

    fflush(fp);
    fclose(fp);

    return( start == 0 );
}

/*-------------------------------------------------*/
/* An "example" of qbe.  This function takes       */
/* the user input and constructs a concept from it */
/* While this type of processing will always be    */
/* availible to the user, a "default" QBE function */
/* will be supplied by CBRS                        */
/*-------------------------------------------------*/
int  make_concept2(concept_def, textbody, words, name, hCSET, language)

char     **concept_def;
char      *textbody;
char       words[cMTQ][MAX_BUFF+1];
char      * name;
void      * hCSET           ;        /*r  handle of concept set              */
char      * language        ;
{

   char  token[MAX_BUFF+1];
   char  head[MAX_BUFF+1];
   int    cur_size;
   int    size    ;   
   char  *txt = NULL;
   char  *ptr;
   int   last_cnt;
   int   cnt;
   int   position;
   int   len,i;
   int   status = CBR_S_SUCCESS;
   CBR_VOID *cbr_concept;
   void *pUCB = NULL;

   /**/

   position = 0;
   cnt = 0;

    /* parse text */

    while (get_token(textbody, token,"",&position))
    {
        /* lower case and save word */

        len = strlen(token);
        for (i = 0 ; i < len ; i++)
            token[i] = tolower(token[i]);

        strcpy(words[cnt++],token);
      
    }

    if (cnt == 0)
        return;

    /* make concept definition */

    cur_size = size = 0;

    /* enter required header information */

    status = txt__expand_token(&txt, 
                               "$control:1\n\n", 
                               strlen("$control:1\n\n"),
                               &cur_size, 
                               &size);

    if (status)
        return(status);

    strcpy(head,words[0]);

    /* root concept */

    sprintf(token,"%s-tmpconcpt accrue\n",head);

    /* supply name */

    sprintf(name,"%s-tmpconcept",head);

    len = strlen(token);
    if (status = txt__expand_token(&txt, token, len, &cur_size, &size))
        return(status);

    /* phrase operator */

    strcpy(token,"* 0.95 '");

    len = strlen(token);
        if (status = txt__expand_token(&txt, token, len, &cur_size, &size))
        return(status);

    for (i = 0; (i < cnt) ; i++)
    {
        if ((i + 1) == cnt)
            sprintf(token,"%s'\n",words[i]);
        else
            sprintf(token,"%s ",words[i]);

        len = strlen(token);
        if (status = txt__expand_token(&txt, token, len, &cur_size, &size))
            return(status);
    }

    /* accrue operator */

    if (cnt > 1)
    {
        /* accrue operator */

        sprintf(token,"* 0.90 %s-tmp-accrue accrue\n", head);

        len = strlen(token);
        if (status = txt__expand_token(&txt, token, len, &cur_size, &size))
            return(status);

        for (i = 0; (i < cnt) ; i++)
        {
            sprintf(token,"** 0.50 '%s'\n",words[i]);
            len = strlen(token);
            if (status = txt__expand_token(&txt, token, len, &cur_size, &size))
                return(status);
        }
    }

    /* sentence operator */

    if (cnt > 1)
    {
        /* sentence */

        sprintf(token,"* 0.80 %s-tmp-sentence sentence\n", head);

        len = strlen(token);
        if (status = txt__expand_token(&txt, token, len, &cur_size, &size))
            return(status);

        for (i = 0; (i < cnt) ; i++)
        {
            sprintf(token,"** 1.00 '%s'\n",words[i]);
            len = strlen(token);
            if (status = txt__expand_token(&txt, token, len, &cur_size, &size))
                return(status);
        }
    }

    /* paragraph operator */

    if (cnt > 1)
    {
        /* accrue operator */

        sprintf(token,"* 0.65 %s-tmp-paragraph paragraph\n", head);

        len = strlen(token);
        if (status = txt__expand_token(&txt, token, len, &cur_size, &size))
            return(status);

        for (i = 0; (i < cnt) ; i++)
        {
            sprintf(token,"** 1.00 '%s'\n",words[i]);
            len = strlen(token);
            if (status = txt__expand_token(&txt, token, len, &cur_size, &size))
                return(status);
        }
    }

    *concept_def = txt;

    return(status);

}

int SaveConcept(concept_def, size, hCSET, pUCB)

    CBR_VOID   * concept_def        ;   /*w: concept handle                 */
    CBR_INT    * size               ;   /*w: concept handle                 */
    CBR_VOID   * hCSET              ;   /*w: concept set handle             */
    CBR_VOID   * pUCB               ;   /*m: opaque user control block      */
{
    FILE       * outfile            ;
    CBR_CHAR   * otl_file[255];
    CBR_INT      ni                 ;   /*   number of paramters            */
    CBR_ITEM     pi [1]             ;   /*   parameters                     */
    CBR_INT      status = CBR_S_SUCCESS;

    /**/

    strcpy(otl_file,"tmp.dat");
    outfile = fopen(otl_file,"w");
    fwrite( concept_def, size, 1, outfile);
    fclose(outfile);
    
    CBR_M_CLR(pi, ni);
    CBR_M_INP(pi, ni, CBR_I_CSET_LOADFILE, otl_file, strlen(otl_file));
    status = cbr_modify(CBR_O_CSET, ni, pi, pUCB, hCSET) ;
    if ( status != CBR_S_SUCCESS )
    {
        demo_error_display( "CSET modify", status, CBR_O_CSET, hCSET );
        exit (EXIT_FAILURE) ;
    }

    remove( otl_file );
    
    return(CBR_S_SUCCESS) ;                                                   
}

/*-------------------------------------------*/
/*  add token to search                      */
/*-------------------------------------------*/
int txt__expand_token( query, string, len, count, size)

CBR_CHAR       **query  ;   /* query text             */
CBR_CHAR        *string ;   /* string to add          */
CBR_INT          len    ;   /* length of string       */
CBR_INT         *count  ;   /* current bytes used     */
CBR_INT         *size   ;   /* current size of buffer */
{
    CBR_CHAR        *ptr    ;
    CBR_CHAR        *pnew   ;

    /**/
 
    if( (*count + len) >= *size )                                      
    {
        while( (*count + len) >= *size )                                
            *size += 256;
        pnew = (CBR_CHAR *) calloc( 1, (*size + 1));                    
        if( !pnew )                            
            return( -1 );
        
        if (*query != NULL)
        {
            memcpy( pnew, *query, *count );                             
            free(*query);                                               
        }

        *query = pnew;                                                  
    }

    memcpy( (*query + *count), string, len ); 
                                        
    *count += len;                                                      

    ptr = (*query + *count);
    *ptr = '\0';

    return( CBR_S_SUCCESS );                                            
}

#endif /* CBR */
