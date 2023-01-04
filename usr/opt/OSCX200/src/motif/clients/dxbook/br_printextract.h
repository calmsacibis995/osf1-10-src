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
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use, 	     **
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


/*========================== br_printextract.h ===============================
File Description: Customer Demand Print Pagerange Header File
----------------

Module: br_printextract.h
-------
History: 02-13-91 f.klum; created
-------
*/

/*------------------------ br_printextract.h-------------------------------*/

#define GLOBAL  globaldef
#define EXTERN  globalref

#define ERROR -1
#define COMENTSIZ 256   /* buffer size to hold structured comment */
#define LINESIZ  80
#define VERBOSE TRUE

/* range types, how user specified */
#define TOPIC   1
#define ENDTOPIC 2
#define FOLIO   3
#define ORDINAL 4
#define PAGE 6
#define DONT_CARE 7
#define CDPINFO	1
#define CDPWRNG 2 
#define CDPERR  3
#define SYSTEMMSG 4

#define SWAPPED  100

/* fseek constants */
#define FROM_BOF 0
#define FROM_CURPOS 1
#define FROM_EOF 2


#define MRM_FETCH(name,parent,widget) \
    status = MrmFetchWidget(bkr_hierarchy_id, \
                             name,                /* index into UIL */ \
                             parent,              /* parent widget */ \
                             widget,              /* &widget being fetched */ \
                             &dummy_class );      /* unused class */ \
    if( status != MrmSUCCESS ) \
      { \
      fprintf( stderr, "Can't fetch %s\n",name ); \
      bkr_cursor_display_wait( OFF ); \
      return; \
      }

#define BKR_ERROR_DIALOG(msg,w) \
    if(error_string = (char *) bkr_fetch_literal(msg,ASCIZ)) \
      { \
      sprintf( errmsg, error_string ); \
      bkr_error_modal( errmsg, XtWindow(w) ); \
      XtFree( error_string ); \
      }

#define READ    i_ch = fgetc(in_fp); \
                if(ferror(in_fp)) \
                  { \
                  perror("   Read"); \
                  return(ERROR); \
                  } \
                if(feof(in_fp) && (i_ch == EOF)) \
                  eof = EOF; \
                ch = (char) i_ch;

#define WRITE   if(output) \
                  { \
                  if(fputc(ch, out_fp) == EOF) \
                    { \
                    perror("   Write"); \
                    return(EOF); \
                    } \
                  }

#ifdef VMS
#define DELETE_FILE(file_name) { \
         char sys_cmd_buf[512]; \
         sprintf(sys_cmd_buf,"DELETE %s",file_name); \
         if(!(strrchr(sys_cmd_buf,';'))) \
           strcat(sys_cmd_buf,";"); \
         if(system(sys_cmd_buf) != SS$_NORMAL) \
           { \
           /* BKR_ERROR_DIALOG(""); */ \
           } \
         }
#else
#define DELETE_FILE(file_name) \
    if(unlink(file_name)) \
      { \
      /* BKR_ERROR_DIALOG(""); */ \
      }
#endif

#define OPEN_ERROR(fname) \
    if(error_string = (char *) bkr_fetch_literal("PE_CANT_OPEN_WRITE",ASCIZ)) \
      { \
      sprintf( errmsg, error_string, fname ); \
      bkr_error_modal( errmsg, XtWindow(parent_w) ); \
      XtFree( error_string ); \
      } \
    return(1);

#define WRITE_ERROR  \
        if(error_string = (char *)bkr_fetch_literal("PE_WRITE_ERROR",ASCIZ)) \
          {  \
          sprintf( errmsg, error_string, out_fname );  \
          bkr_error_modal( errmsg,  XtWindow(parent_w) );  \
          XtFree( error_string );  \
          }  \
        return(1);

#define READ_ERROR \
        if(error_string = (char *) bkr_fetch_literal("PE_READ_ERROR",ASCIZ)) \
          { \
          sprintf( errmsg, error_string, in_fnam ); \
          bkr_error_modal( errmsg,  XtWindow(parent_w)); \
          XtFree( error_string ); \
          } \
        return(1);

#define GET1BYTE \
        ch = fgetc(in_fp); \
        if(ferror(in_fp)) \
          { \
          READ_ERROR \
          }

#define PUT1BYTE \
        fputc(ch, fp); \
        if(ferror(fp)) \
          { \
          WRITE_ERROR \
          }

#define F_PUTS(buf)  \
        fputs(buf,fp);  \
        if(ferror(fp))  \
          {  \
          WRITE_ERROR  \
          }

#define ADD_TO_LINE(workbuf)  \
        len = strlen(workbuf);  \
        psbuflen += len;  \
        if(psbuflen > PSLINE_LENGTH)  \
          {  \
          strcat(psbuf,"\n");  \
          fputs(psbuf,fp);  \
          if(ferror(fp))  \
            {  \
            WRITE_ERROR  \
            }  \
          strcpy(psbuf,workbuf);  \
          psbuflen = len;  \
          }  \
        else  \
          {  \
          strcat(psbuf,workbuf);  \
          }

#define FLUSH_LINE  \
        if(psbuflen)  \
          {  \
          strcat(psbuf,"\n");  \
          fputs(psbuf,fp);  \
          if(ferror(fp))  \
            {  \
            WRITE_ERROR  \
            }  \
          psbuf[0] = '\0';  \
          psbuflen = 0;  \
          }


typedef struct cdp_s /* what type of structured comments in ps file */
  {
  char  *ps_fname; /* name of ps file to demand print from */
  char  *bk_fname; /* name of decw$book file to demand print from */
  char  *cdp_fname;/* name of resulting ps file containing page-range */
  int   n_pages;   /* total number of pages in resulting cdp file */
  int   max_ordinal; /* total number of pages in original ps file */
  unsigned long prolog_offset; /* byte offset where prolog ends */
  unsigned long trail_offset; /* byte offset where trailer starts */
  int   edm;       /* true if edm ps file(index area of ptrs to pages, etc)*/
  int   folio;     /* true if %%Page with folio and ordinal args */
  int   ordinal;   /* true if %%Page with ordinal arg only */
  int   topic;     /* true if %DEC$BKR_TOPIC comments */
  int   sorted;    /* true if user wants to sort the listbox */
  int   n_ranges;  /* number of ranges chosen to print */
  } cdp_t;


typedef struct range_s       /* variety of possible ranges */
  {
  int          type;         /* how specified, TOPIC, FOLIO, or ORDINAL */
  int          ord_start;    /* starting ordinal number */
  int          ord_end;      /* ending ordinal number */
  char         *folio_start; /* starting folio */
  char         *folio_end;   /* ending folio */
  char         *topic_start; /* starting topic symbol */
  char         *topic_end;   /* ending topic symbol */
  unsigned long offset_start; /* file byte offset where range starts */
  unsigned long offset_end;  /* file byte offset where range ends */
  int          n_pages;      /* pages in this range */
  XmString     xstring;      /* X string for display */
  struct range_s *next;      /* ptr to possible next */
  } range_t;

typedef struct pglist_s    /* linked list of comments, ordinals, and offsets */
  {
  char         *folio;       /* ptr to folio string */
  int          ordinal;      /* ordinal page number */
  unsigned long offset;      /* byte offset in file approx. where page starts */
  struct pglist_s *next;     /* ptr to next */
  } pglist_t;

typedef struct topic_s       /* linked list of topic comments */
  {
  char         *string;      /* ptr to topic symbol string */
  pglist_t     *page;        /* ptr to page that this topic is on */
  int          end;          /* FALSE if DEC$BKR_TOPIC,
                                TRUE if DEC$BKR_ENDTOPIC comment */
  struct topic_s *next;      /* ptr to next */
  } topic_t;

typedef struct pagemap_s     /* forms an array indexed by ordinal */
  {
  char         *folio;       /* ptr to folio string */
  unsigned long offset;      /* byte offset in file approx. where page starts */
  } pagemap_t;

