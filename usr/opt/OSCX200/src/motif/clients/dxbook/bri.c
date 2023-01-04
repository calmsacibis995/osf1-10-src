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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI.C*/
/* *24   22-MAR-1993 14:31:24 BALLENGER "Fix crash when directory target is not valid chunk."*/
/* *23    5-JAN-1993 16:38:00 GOSSELIN "fixed HyperHelp/VMS V5.5-2 problem by NULLing appropriate fields"*/
/* *22   24-SEP-1992 17:07:39 KLUM "fixup ditroff filtering for index level adjust"*/
/* *21   17-SEP-1992 19:06:53 BALLENGER "Don't free pme->buffer. Its allocated as part of the pme."*/
/* *20   17-SEP-1992 11:48:01 GOSSELIN "fixed memory leak"*/
/* *19   16-SEP-1992 14:59:27 GOSSELIN "commented debug stuff"*/
/* *18   15-SEP-1992 16:39:37 KLUM "adjust index dre->header_levels based on ftext x offsets"*/
/* *17    4-AUG-1992 09:24:37 GOSSELIN "removed XtFree of Asian lang var"*/
/* *16   29-JUL-1992 15:30:48 GOSSELIN "fixed Index resolution in new books"*/
/* *15   20-JUL-1992 09:39:53 FITZELL " typo"*/
/* *14   13-JUL-1992 15:08:13 FITZELL "mswindows condtionals for pc port"*/
/* *13   22-JUN-1992 16:04:26 BALLENGER "use off_t typedef for file positions"*/
/* *12   19-JUN-1992 20:15:58 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *11    9-JUN-1992 10:01:29 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *10   23-APR-1992 00:15:12 GOSSELIN "finished making I18N changes"*/
/* *9     8-APR-1992 14:49:54 GOSSELIN "added ALPHA fixes"*/
/* *8     3-MAR-1992 17:09:50 KARDON "UCXed"*/
/* *7     4-FEB-1992 14:22:51 FITZELL "alpha alignment"*/
/* *6     2-JAN-1992 10:13:46 FITZELL "Xbook references in OAF level B using the extension header"*/
/* *5    12-DEC-1991 14:46:51 BALLENGER " Fix SVN/directory entry problems"*/
/* *4     1-NOV-1991 12:51:08 BALLENGER "Reintegrate memex support"*/
/* *3    20-SEP-1991 17:02:55 BALLENGER "Fix problems with multiple calls to bri_directory_open()."*/
/* *2    17-SEP-1991 21:08:53 BALLENGER "change use of major and minor in version number to avoid conflict with sys/types.h*/
/*definition on ULTRIX"*/
/* *1    16-SEP-1991 12:43:50 PARMENTER "Low-level private access routines"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI.C*/
#ifndef VMS
 /*
#else
# module BRI "V03-0010"
#endif
#ifndef VMS
  */
#endif

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxbook/bri.c,v 1.1.4.2 1993/08/24 18:24:57 Ellen_Johansen Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

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
**
** Facility:
**
**   BRI -- Book Reader Interface
**
** Abstract:
**
**      This module implements the Book-Reader Interface private routines.
**
** Functions:
**
**  	read_record
**  	read_page
**  	BriClosePage
**  	BriBookGetPageIndexAndMap
**  	BriGetV3DirHeaders
**  	BriBookGetHeader
**  	BriBookGetDataPage
**  	BriReadDirectoryPage
**  	BriBookGetFontDefinitions
**  	ReadChunkStrings
**  	BriBookGetChunkTitles
**  	BriBookGetSymbolTable
**  	BriBookGetSymbolExrefTable
**  	BriBookGetStaticExrefTable
**  	BriBookGetChunkIndex
**	replace_blank_entry
**      bri_get_extension_header
**
** Author:
**
**      David L. Ballenger
**
** Date:
**
**      Tue May 23 13:12:36 1989
**
** Revision History:
**  V31-0001     MJF0001         Michael J Fitzell       14-nov-1991
**              Added routine to read in data from the extension
**              header. This will allow the use of Xbook referencing
**              without a change to the file format.
**
**  V03-0010    DLB0006	    David L Ballenger          29-Apr-1991
**              Use ISO-LATIN-1 compatible streql() routine instead of
**              strcasecmp() and strncasecmp().
**
**  V03-0009    DLB0005	    David L Ballenger          29-Apr-1991
**              Improve handling of TOC entries in V2 books produced by
**              DECwrite.  Fixes problems described in QAR 01444 from 
**              DECW-V3-INTERNAL database.
**
**  V03-0008	MJF0001 	Michael J Fitzell	1-Apr-1991
**		Added replace_blank_entry() to put translatable text
**		"UNTITLED" in directory entries that are blank.
**
**  V03-0007    DLB0004	    David L Ballenger          27-Feb-1991
**              Improve NULL title string handling in GetDirEntry().
**              This is fix for DECW_V3_INTERNAL QAR 00872.
**
**  V03-0006    DLB0003     David L Ballenger          16-Feb-1991
**              Modify SVN cleanup algorithm for TOC entries in pre-V3
**              books to better handle Appendices and other unnumbered
**              headers follwoing numbered headers.
**
**  V03-0005    DLB0002     David L Ballenger           7-Feb-1991
**              Add fixes to provide better support for use of SVN
**              in the navigation window with pre-V3 books.
**
**  V03-0004	JAF0003	    James A. Ferguson	    	2-Dec-1990
**  	    	Comment out forcing header_level's to 1 for V1 and V2.
**
**  V03-0003    JMK 	    Joe M. Kraetsch 	    	2-Dec-1990
**  	    	Add new BriBookGetSymbolExrefTable for static external 
**  	    	references and modify code which reads in symbols table
**  	    	for V1 and V2 books in BriBookGetSymbolTable to
**  	    	ignore symbols with prefix of _DECW, make symbols with
**  	    	leading underscores valid.
**
**  V03-0002	JAF0002	    James A. Ferguson	    	7-Sep-1990
**  	    	Move check for book version just after the book header
**  	    	is parsed, and add check for major version of 3 and minor 
**  	    	version of 0 in BriBookGetHeader.
**
**  V03-0001    DLB         David L Ballenger           29-Aug-1990
**              Use RMS for file access on VMS.
**
**  V03-0000	JAF0001	    James A. Ferguson	    	16-Aug-1990 
**  	      	Create new module, modify revision history format and
**  	    	update copyright.
** 
**  	    	JAF 	    James A. Ferguson	    	15-Aug-1990 
**  	      	Change drb queue to an array of pointers to drb's
**  	      	in BriBookGetHeader.
**
**  	    	JAF 	    James A. Ferguson	    	30-Jul-1990 
**  	      	Modify BriBookGetHeader to store the directory object 
**  	      	id as each DRB is initialized.
**
**  	    	JAF 	    James A Ferguson	    	23-Jul-1990 
**  	      	Modify BriReadDirectoryPage to set the field 
**  	      	"header_level" in the dre to 1 for version 1 and 2 books.
**  	      	We do this so the contents of a directory will be
**  	      	fully expanded.  This is a V3 change for compatibility
**  	      	with version 1 and 2 books.
**
**  	    	DLB         David L Ballenger	    	30-May-1990 
**             	Make changes to improve performance. On VMS access the
**             	book as a variable length record file as opposed to 
**             	stream.  For general performance improvements change
**             	macros for unpacking records.  Also, fix routine to
**             	read chunk symbols.
**
*/


/* Include files */
#ifndef MSWINDOWS
#include    <X11/Intrinsic.h>
#include    <Mrm/MrmPublic.h>
#include    <Xm/Xm.h>
#include "br_common_defs.h"
#include "bkr_error.h"
#include "bkr_fetch.h"
#include "bri_private_def.h"
#else
#include "bmtcdefs.h"
#include "bmtipdef.h"
#include "bmterr.h"
#include "bmtfetch.h"
#include "bmtalloc.h"
#include "bmtibook.h"
#include  <sys\stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#define UNTILED "NO TEXT FOUND"
#endif

#ifdef VMS
#include "stat.h"	/* Definition of off_t on VMS */
#endif /* VMS */

#include <ctype.h>

/*
** EXTERNAL ROPUTINES
*/
extern char * FtextToAscii();


/* Defines */
#define RMS_BLOCK_SIZE 512

#define VBH_C_BLOCK_NUMBER 1
#define VBH_C_BYTE_OFFSET 0
#define VBH_C_RECORD_LENGTH 1022
#define VBH_EXT_C_RECORD_LENGTH 510


#define _GetField(env,pme,field,size) \
    {  \
        register BR_UINT_32 field_end; \
        field_end = (pme)->index + size; \
        if (field_end > (pme)->tag.rec_length) { \
            BriLongJmp(env,BriErrRecUnpackNum); \
        } \
        memcpy(field,&(pme)->buffer[(pme)->index],size); \
        (pme)->index = field_end; \
    }
#define BRI_GET_FIELD(env,pme,field) _GetField(env,pme,&(field),sizeof(field))
#define BRI_GET_ARRAY(env,pme,array) _GetField(env,pme,array,sizeof(array))
#define BRI_GET_VAR_ARRAY(env,pme,array,size) _GetField(env,pme,array,size)

#define BRI_GET_BYTE(env,pme,dst) \
    {   \
        register BR_UINT_8 *src = (BR_UINT_8 *)&(pme)->buffer[(pme)->index]; \
        (pme)->index++; \
        if ((pme)->index > (pme)->tag.rec_length) {  \
            BriLongJmp(env,BriErrRecUnpackNum); \
        } \
        *(BR_UINT_8 *)dst = *src; \
    }

#if defined(mips) || defined(ALPHA) || defined(MSWINDOWS)
/*BRI_GET_WORD(BRI_CONTEXT * env, BRI_PAGE_MAP_ENTRY * pme, BR_UINT_8 * addr)
    {
        register BR_UINT_8 *src = 
	    (BR_UINT_8 *)&(pme)->buffer[(pme)->index];
        register BR_UINT_8 *dst = (BR_UINT_8 *)addr;
        (pme)->index += 2;
        if ((pme)->index > (pme)->tag.rec_length) {
            BriLongJmp(env,BriErrRecUnpackNum);
        }
        *dst++ = *src++;
        *dst = *src;
    } */
#define BRI_GET_WORD(env,pme,addr) \
    {   \
        register BR_UINT_8 *src = (BR_UINT_8 *)&(pme)->buffer[(pme)->index]; \
        register BR_UINT_8 *dst = (BR_UINT_8 *)addr;\
        (pme)->index += 2; \
        if ((pme)->index > (pme)->tag.rec_length) {  \
            BriLongJmp(env,BriErrRecUnpackNum); \
        } \
        *dst++ = *src++; \
        *dst = *src; \
    }
#else
#define BRI_GET_WORD(env,pme,dst) \
    {   \
        register BR_UINT_16 *src = (BR_UINT_16 *)&(pme)->buffer[(pme)->index]; \
        (pme)->index += 2; \
        if ((pme)->index > (pme)->tag.rec_length) {  \
            BriLongJmp(env,BriErrRecUnpackNum); \
        } \
        *(BR_UINT_16 *)dst = *src; \
    }
#endif


#if defined(mips) || defined(ALPHA) || defined(MSWINDOWS)
#define BRI_GET_LONG(env,pme,addr) \
    {   \
        register BR_UINT_8 *src = (BR_UINT_8 *)&(pme)->buffer[(pme)->index]; \
        register BR_UINT_8 *dst = (BR_UINT_8 *)addr;\
        (pme)->index += 4; \
        if ((pme)->index > (pme)->tag.rec_length) {  \
            BriLongJmp(env,BriErrRecUnpackNum); \
        } \
        *dst++ = *src++; \
        *dst++ = *src++; \
        *dst++ = *src++; \
        *dst = *src; \
    }
#else
#define BRI_GET_LONG(env,pme,dst) \
    {   \
        register BR_UINT_32 *src = (BR_UINT_32 *)&(pme)->buffer[(pme)->index]; \
        (pme)->index += 4; \
        if ((pme)->index > (pme)->tag.rec_length) {  \
            BriLongJmp(env,BriErrRecUnpackNum); \
        } \
        *(BR_UINT_32 *)dst = *src; \
    }
#endif 

#ifdef vms
#define BRI_GET_STRUCT(env,pme,addr,type_name) \
    { \
        register type_name *src = (type_name *)&(pme)->buffer[(pme)->index]; \
        (pme)->index += sizeof(type_name); \
        if ((pme)->index > (pme)->tag.rec_length) {  \
            BriLongJmp(env,BriErrRecUnpackNum); \
        } \
        *(addr) = *src; \
    }
#endif 

#if defined (vms) && !defined(ALPHA) 
#define BRI_GET_TAG(env,pme,tag) \
    pme->record_start = pme->index; \
    BRI_GET_STRUCT(env,pme,tag,BODS_TAG)
#else
#define BRI_GET_TAG(env,pme,tag) \
    pme->record_start = pme->index; \
    BRI_GET_WORD(env,pme,&(tag)->rec_type); \
    BRI_GET_LONG(env,pme,&(tag)->rec_length)
#endif

#if defined(vms) && !defined(ALPHA)
#define BRI_GET_PAGE_INDEX_ENTRY(env,pme,pie) \
    BRI_GET_STRUCT(env,pme,pie,BODS_PAGE_INDEX_ENTRY)
#else
#define BRI_GET_PAGE_INDEX_ENTRY(env,pme,pie) \
    BRI_GET_LONG(env,pme,&(pie)->rfa.block_number); \
    BRI_GET_WORD(env,pme,&(pie)->rfa.byte_offset); \
    BRI_GET_LONG(env,pme,&(pie)->record_length) ; 
#endif

#define BRI_BUFFER_END(pme) ((pme)->index >= (pme)->tag.rec_length)
#define BRI_DATA_ADDRESS(pme) \
               (BRI_BUFFER_END(pme) ? NULL : \
                                      ((BMD_GENERIC_PTR)&(pme)->buffer[(pme)->index]))
#define BRI_START_RECORD(pme) ((pme)->record_start = (pme)->index)
#define BRI_NEXT_RECORD(pme,tag) ((pme)->index = (pme)->record_start \
                                                + (tag)->rec_length) 


#ifdef vms
#define CONVERT_RFA(rfa) ( (((rfa)->block_number - 1) * RMS_BLOCK_SIZE)  \
                           + (rfa)->byte_offset \
                         )
#else
#define CONVERT_RFA(rfa) ( (((rfa)->block_number - 1) * RMS_BLOCK_SIZE)  \
                           + (rfa)->byte_offset \
                           + sizeof(rms_t_record_length_field) \
                         )
#endif 


/* Type definitions */

typedef BR_UINT_16 rms_t_record_length_field;

typedef struct _DrbQueue {
    	BXI_QUEUE queue;
    	BRI_DIRECTORY_BLOCK *drb;
  } DrbQueue, *DrbQueuePtr;

/* Directory context information used by the routines that read and process
 * directory pages.
 */
typedef struct _DirContext {
    BRI_CONTEXT *env;   
    BRI_DIRECTORY_BLOCK *drb;   
    BRI_PAGE_MAP_ENTRY *pme;    
    BODS_DIRECTORY_ENTRY *dre;
    BR_UINT_32 entries_used;
    BR_UINT_32 max_entries;
} DirContext ;

   

/* Local data */
static
BXI_QUEUE bri_booklist = { (BMD_GENERIC_PTR)&bri_booklist, 
                             (BMD_GENERIC_PTR)&bri_booklist } ;
    
static 
BRI_STRING_LIST v1_alternate_product_name = { BROWSER_PRODUCT_NAME , NULL } ;

BR_INT_32 
streq(s1,s2)
    BR_UINT_8 	*s1;		/* string to be searched */
    BR_UINT_8	*s2;		/* substring to look for with s1 */
/*
 *
 * Function description:
 *
 *      Compares two strings for equality using a case insensitive comparison.
 *
 * Arguments:
 *
 *      s1 - first string
 *      s2 - second string
 *
 * Return value:
 *
 *      TRUE if the strings are equal, FALSE otherwise.
 *
 * Side effects:
 *
 *      None
 *
 */
{
    while(*s1 && *s2)
	{
	if(_br_tolower(*s1) != _br_tolower(*s2))  /* compare one letter */
	    return(FALSE);			
	s1++;
	s2++;
	}
    return(!(*s1 || *s2));		/* both must be null to return TRUE */

} /* end streq */


BR_INT_32 
substreq(s1,s2)
    BR_UINT_8 	*s1;		/* string to be searched */
    BR_UINT_8	*s2;		/* substring to look for with s1 */
/*
 *
 * Function description:
 *
 *      Compares string 1 with initial substring of string 2 for equality 
 *      using a case insensitive comparison.
 *
 * Arguments:
 *
 *      s1 - first string
 *      s2 - second string
 *
 * Return value:
 *
 *      TRUE if the inital part of string 2 equals string 1, FALSE otherwise.
 *
 * Side effects:
 *
 *      None
 *
 */
{
    while(*s1 && *s2) {
	if (_br_tolower(*s1) != _br_tolower(*s2)) {  /* compare one letter */
            return(FALSE);			
        }
	s1++;
	s2++;
    }
    if ((*s1 == '\0') && ((*s2 == '\0') || isspace(*s2))) {
        return (TRUE);
    } else {
        return (FALSE);
    }
} /* end substreq */


static void
read_record(env,pme,pie,tag)
    BRI_CONTEXT *env;
    BRI_PAGE_MAP_ENTRY *pme;
    BODS_PAGE_INDEX_ENTRY_PTR pie;
    BODS_TAG *tag;
/*
 *
 * Function description:
 *
 *      Reads a record from an "RMS" variable length record from a book 
 *      file.  On VMS this is done using RMS directly, on ULTRIX this is
 *      done using read()
 *
 * Arguments:
 *
 *      env - the environment context for the book
 *
 *      pme - the page map entry for the the book "page" that this 
 *            record is part of. This provides the buffer and the
 *            index into the buffer for the record
 *
 *      pie - the page index entry for the record.  This defines the 
 *            location and size of the record.
 *
 *      tag - pointer to a tag structure to receive the TLV at the
 *            beginning of the record.
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
#ifdef vms

    /* Use RMS
     */

    BR_INT_32 rms_status;
    BR_INT_32 i;

    /* Put the record file address (rfa) from the page index entry into 
     * rfa in the RAB.
     */

    BR_UINT_8    *dst_rfa =  (BR_UINT_8 *)&env->rab.rab$w_rfa;
    BR_UINT_8    *src_rfa = (BR_UINT_8 *)&pie->rfa;

    for(i = 0;i < (sizeof(RmsRecordFileAddress));i++)
	*dst_rfa++ = *src_rfa++;

/* RmsRecordFileAddress is an unaligned data structure that will */
/* not work well on alpha*/       
/*    *((RmsRecordFileAddress *)&env->rab.rab$w_rfa) = pie->rfa;*/

    /* Set up the buffer address and size in the rab.  Note that we
     * index into the buffer in the page map entry, since we may be
     * reading a page from multiple records.
     */
    env->rab.rab$l_ubf = &pme->buffer[pme->index] ;
    env->rab.rab$w_usz = pie->record_length;
    
    /* Get the record
     */
    rms_status = sys$get(&env->rab);
    if (rms_status != RMS$_NORMAL) {
        BriLongJmp(env,BriErrFieldReadNum);
    } else {
        if (env->rab.rab$w_rsz != pie->record_length) {
            BriLongJmp(env,BriErrFieldReadNum);
        }
    }
#else
    off_t filepos;

    /* On ULTRIX we convert the rfa to a file position and then do an lseek
     * to position the file.
     */
    filepos = CONVERT_RFA(&pie->rfa);
    if (lseek(fileno(env->file),filepos,0) == -1) {
        BriLongJmp(env,BriErrBookPosNum);
    }

    /* Now read the record into the specified place in the page map entry
     * buffer.
     */
    if (read(fileno(env->file),&pme->buffer[pme->index],pie->record_length) 
        != pie->record_length
        ) {
        BriLongJmp(env,BriErrFieldReadNum);
    }
#endif

    /* Now get the tag from the buffer
     */
    BRI_GET_TAG(env,pme,tag);
}


static BRI_PAGE_MAP_ENTRY *
read_page(env,page_id,page_type)
    register BRI_CONTEXT *env;
    register BMD_OBJECT_ID page_id;
    BR_UINT_32 page_type ;
/*
 *
 * Function description:
 *
 *      Reads a page (logical record) from the file.  Since pages may span
 *      RMS records on VMS, the routine will read in and reassemble the
 *      page from multiple RMS records.  
 *
 *      A buffer will be created for the page.  The page map entry for the
 *      page will be set up for the buffer, and the page will be read and
 *      assembled in it.  The page record type and length will be taken
 *      from the assembled page and placed in the page map.
 *      
 *      The record type will be checked against the specified page type.
 *
 * Arguments:
 *
 *      book -- Pointer to the book block for the book.
 *
 *      page_id  -- Page ID of the page to be read.
 *
 *      page_type -- Page type for the page to be read.
 *
 * Return value:
 *
 *      Pointer to the page map entry if the page was successfully read 
 *      and assembled and is of the correct type, NULL otherwise.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    register BODS_PAGE_INDEX_ENTRY *pie;
    register BRI_PAGE_MAP_ENTRY *pme; 
    BODS_TAG tag;

    pie = &env->data.book->page_index[page_id];
    pme = env->data.book->page_map[page_id];

    /* Make sure we haven't already read this page.
     */
    if (pme == NULL) {
        /* Attempt to allocate a buffer for the page.
         */
        pme = (BRI_PAGE_MAP_ENTRY *)
        BriMalloc(sizeof(BRI_PAGE_MAP_ENTRY)+pie->record_length,
                          env);
    }

    /* We got the buffer now set up the buffer field information for getting
     * fields from the records.
     */
    pme->n_users = 0;
    pme->index = 0;
    pme->flags.all_flags = (char)0;
    pme->data_page = NULL;


    /* Read and assemble the page.
     */
    if (pie->record_length <= BODS_C_FRAGMENT_SIZE) {

        /* This is an easy one, the page fits in one RMS record, so just
         * read it into the buffer and the get the tag fields from it and
         * put them in the pme.
         */
        pme->tag.rec_length = pie->record_length;
        read_record(env,pme,pie,&tag);

    } else {

        /* This page will have to be reassembled from fragments
         * spread across multiple RMS records.
         */
        register BR_UINT_32 index; 
        BODS_PAGE_INDEX_ENTRY  fragment;

        /* Define a page index entry for the first fragment.  The rfa comes
         * from the rfa for the page.  The first fragment is a
         * predefined size and includes room for the pie at the tail
         * of the record.
         */
        fragment.rfa = pie->rfa;
        fragment.record_length = BODS_C_FIRST_FRAGMENT 
                                    + BODS_C_FRAGMENT_TAIL;
        pme->tag.rec_length = fragment.record_length;

        /* Read the first fragment, and set the index into the buffer
         * to account for it.  Note that the index is set to the end
         * of the data in the record, i.e. beginning of the page index
         * entry for the next fragment at the end of record.
         */
        read_record(env,pme,&fragment,&tag);
        pme->index = BODS_C_FIRST_FRAGMENT ;

        /* Get the rest of the fragments.
         */
        do {

            BR_UINT_8 save_buf[BODS_C_TAG_PACKED_SIZE];
            BR_UINT_8 *buf_ptr;

            /* Get the page index entry, at the end of the current record,
             * for the next fragment record for the page.
             */
            BRI_GET_PAGE_INDEX_ENTRY(env,pme,&fragment);

            /* Backup the index so that next fragment can be concatenated
             * with the previous records in the buffer.  The tag field in
             * the next fragment will overwrite the end of the previous
             * record, so we have to save that part of the previous record
             * and restore it after we get the tag for the next fragment.
             */
            pme->index -= (BODS_C_TAG_PACKED_SIZE + BODS_C_FRAGMENT_TAIL);
            buf_ptr = &pme->buffer[pme->index] ;
            memcpy(save_buf,buf_ptr,BODS_C_TAG_PACKED_SIZE);

            /* Read the record and get the tag from the beginning of the next
             * fragment, and decrement the fragment length by the
             * size of these fields.  Note the tag does not stay in 
             * the buffer.
             */
            pme->tag.rec_length += fragment.record_length;
            read_record(env,pme,&fragment,&tag);
            fragment.record_length -= BODS_C_TAG_PACKED_SIZE;
            pme->tag.rec_length  -= BODS_C_TAG_PACKED_SIZE;

            memcpy(buf_ptr,save_buf,BODS_C_TAG_PACKED_SIZE);


            /* Verify the record type for the fragment.
             */
            if (tag.rec_type == BODS_C_PAGE_FRAGMENT) {

                /* This isn't the last, fragment so decrease the amount
                 * of the to be read into the buffer by the RFA at the
                 * tail.
                 */
                fragment.record_length -=  BODS_C_FRAGMENT_TAIL ;
                pme->tag.rec_length  -= BODS_C_FRAGMENT_TAIL ;

                /* Read the next fragment into the appropriate place in the
                 * buffer and increase the index accordingly.
                 */
                pme->index += fragment.record_length ;

            } else if (tag.rec_type != BODS_C_LAST_FRAGMENT) {

                /* This isn't the last or any other fragment.  Print an
                 * error and return.
                 */
                BriFree(pme,env);
                BriLongJmp(env,BriErrPageReadNum,page_id);
            }
        } while (tag.rec_type != BODS_C_LAST_FRAGMENT) ;

        /* Get the tag from the beginning of the page, and adjust the 
         * record length to match that of the page index entry.
         */
        pme->index = 0;
        BRI_GET_TAG(env,pme,&tag);
        tag.rec_length = pie->record_length ;
    }

    if (tag.rec_type != page_type) {
        BriFree(pme,env);
        BriLongJmp(env,BriErrBadPgTypeNum,tag.rec_type,page_type);
    }

    pme->tag = tag;
    env->data.book->page_map[page_id] = pme;
    return pme ;

} /* end read_page */


void
BriClosePage(env,pgid)
    BRI_CONTEXT *env;
    BMD_OBJECT_ID pgid;
/*
 *
 * Function description:
 *
 *      Close a page  in a book.  This maily consists of freeing the buffers
 * 	associated with the page.
 *
 * Arguments:
 *
 *      env - context for the book
 *
 * 	pgid - id of page to be closed
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
    BRI_BOOK_BLOCK *book = env->data.book;
    BRI_PAGE_MAP_ENTRY *pme;
    BR_INT_32 index;

    /* Make sure it is valid page.
     */
    if (pgid > book->vbh.n_pages ) {
        BriLongJmp(env,BriErrBadPageIdNum);
    }

    /* If it is not open, i.e. doesn't have a page map entry
     * allocated, then it is not open, just return.
     */
    pme = book->page_map[pgid];
    if (pme == NULL) {
        return ;
    }

    /* Free all of the buffers associated with the page. 
     */
    if (pme->data_page != NULL) {
        BODS_CHUNK_HEADER *chunk = pme->data_page->chunks;
        if (chunk != NULL) {

            BR_INT_32 n_chunks = pme->data_page->n_chunks;
            for (index = 0; index < n_chunks ; index++ , chunk++) {
                if ((chunk->data != NULL) 
                    && ((chunk->data < &pme->buffer[0])
                        || (chunk->data >= &pme->buffer[pme->tag.rec_length])
                       )
                   ) {
                    BriFree(chunk->data,env);
                }
            }
            BriFree(pme->data_page->chunks,env);
        }
        BriFree(pme->data_page,env);
        pme->data_page = NULL;
    }

    /* Now free the page map entry itself.  Note that the buffer is
     * allocated at the end of the pme so this frees it too.
     */
    BriFree(pme,env);
    book->page_map[pgid] = NULL;

} /* end BriClosePage */

void
BriBookGetPageIndexAndMap(env)
    register BRI_CONTEXT *env;
/*
 *
 * Function description:
 *
 *      Get the page index and page map from the book file.
 *
 * Arguments:
 *
 *      env - context for the book
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Does a BriLongJmp() if an error occurs.
 *
 */

{
    register BRI_BOOK_BLOCK *book = env->data.book;
    register BR_UINT_32 index;
    register BODS_PAGE_INDEX_ENTRY_PTR pie;
    register BRI_PAGE_MAP_ENTRY *pme;
    register BMD_OBJECT_ID pgid;
    

    pgid = book->vbh.n_pages ;

    /* Allocate buffers for the page index and the page_map.
     */
    book->page_index = BriCalloc(BODS_PAGE_INDEX_ENTRY,(pgid + 1),env);
    book->page_map = BriCalloc(BRI_PAGE_MAP_ENTRY *,(pgid + 1),env);

    pie = &book->page_index[pgid];
    pie->rfa = book->vbh.page_index_rfa;
    pie->record_length = book->vbh.page_index_length;
    book->page_map[pgid] = NULL;

    pme = read_page(env,pgid,BODS_C_PAGE_INDEX);

    /* Check the record length in the tag for the page index page.  If
     * not equal to the length specified in the vbh, fix it and print
     * an error message if debugging is turned on.
     */
    if (pme->tag.rec_length != book->vbh.page_index_length) {
        pme->tag.rec_length = book->vbh.page_index_length ;
#ifdef DEBUG
        BriLongJmp(env,BriErrBadPageIndexNum);
#endif
    }


    for (index = 0 , pie = book->page_index ; index < pgid ; index++ , pie++) {
        
        book->page_map[index] = NULL;

        /* Get the on-disk RMS record file address and convert it
         * to a file position (byte offset into the file).
         */
        BRI_GET_PAGE_INDEX_ENTRY(env,pme,pie);
    }

} /* end BriBookGetPageIndexAndMap */


void bri_get_extension_header(env)
    register BRI_CONTEXT *env;
/*
 *
 * Function description:
 *
 *      Get data out of the extension header
 *
 * Arguments:
 *
 *      env - context for the book
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Does a BriLongJmp() if an error occurs.
 *
 */

{
    register BRI_BOOK_BLOCK *book = env->data.book;
    register BR_UINT_32 index;
    register BODS_PAGE_INDEX_ENTRY_PTR pie;
    register BRI_PAGE_MAP_ENTRY *ext_pme;
    register BMD_OBJECT_ID pgid;
    BR_UINT_32    fill_len;
    BODS_TAG tag;

    pgid = book->vbh.vbh_extension;

    ext_pme = read_page(env,pgid,BODS_C_BOOK_HEADER_EXTENSION);
  
    BRI_GET_LONG(env,ext_pme,&book->vbh.filler_offset);
    BRI_GET_LONG(env,ext_pme,&book->vbh.copyright_topic);
    BRI_GET_WORD(env,ext_pme,&book->vbh.oaf_version.major_num);
    BRI_GET_WORD(env,ext_pme,&book->vbh.oaf_version.minor_num);
    BRI_GET_LONG(env,ext_pme,&book->vbh.symbol_exref_index_page);
    BRI_GET_LONG(env,ext_pme,&book->vbh.static_exref_index_page);
    BRI_GET_LONG(env,ext_pme,&book->vbh.n_symbols);
    BRI_GET_LONG(env,ext_pme,&book->vbh.n_symbol_exrefs);
    BRI_GET_LONG(env,ext_pme,&book->vbh.n_sys_directories);
    BRI_GET_WORD(env,ext_pme,&book->vbh.bwi_version.major_num);
    BRI_GET_WORD(env,ext_pme,&book->vbh.bwi_version.minor_num);
    ext_pme->tag.rec_length = VBH_EXT_C_RECORD_LENGTH ;
    index = ext_pme->index;

        /*  NOTE:  for forward compatibility, we need to allow for new
        *           fields to be added here.  The filler_offset field
        *           tells us where the variable part begins.  We first
        *           read in all of the rest of the bytes
        */
    fill_len = VBH_EXT_C_RECORD_LENGTH - index - sizeof(book->vbh.checksum);
    BRI_GET_VAR_ARRAY(env,ext_pme,book->vbh.vbh_filler, fill_len);
    BRI_GET_LONG(env,ext_pme,&book->vbh.checksum);
    ext_pme->index = book->vbh.filler_offset;

        while (! BRI_BUFFER_END(ext_pme) )
        {
            BRI_GET_TAG(env,ext_pme,&tag) ;
            if (tag.rec_length == 0)
                break ;

            switch (tag.rec_type)
            {

                case BODS_C_PARTNUM :
                    book->partnum = (char *)BRI_DATA_ADDRESS(ext_pme);
                    break;
                case BODS_C_TOOLSID :
                    book->toolsid = (char *)BRI_DATA_ADDRESS(ext_pme);
                    break;
                case BODS_C_COPYRIGHT_CORP :
                    book->copyright_corp = (char *)BRI_DATA_ADDRESS(ext_pme);
                    break;
                case BODS_C_COPYRIGHT_DATE :
                    book->copyright_date = (char *)BRI_DATA_ADDRESS(ext_pme);
                    break;
                case BODS_C_COPYRIGHT_TEXT :
                    book->copyright_text = (char *)BRI_DATA_ADDRESS(ext_pme);
                    break;
                case BODS_C_LANGUAGE :
                    book->language = (char *)BRI_DATA_ADDRESS(ext_pme);
                    break;
                case BODS_C_BOOKNAME :
                    book->bookname = (char *)BRI_DATA_ADDRESS(ext_pme);
                    break;

                default:
                      break;
                }
            BRI_NEXT_RECORD(ext_pme,&tag);
        }       /* while not end of buffer */


} /* end bri_get_extension_header */


void
BriGetV3DirHeaders (env)
    BRI_CONTEXT *env;
/*
 *
 * Function description:
 *
 *      Reads V3 directory headers from a separate page on disk.
 *
 * Arguments:
 *
 *      env - context for the book
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *
 */

{
    register BRI_BOOK_BLOCK *book = env->data.book;
    BRI_PAGE_MAP_ENTRY *vdh_pme;
    register BR_UINT_32 index;
    BODS_TAG tag;
    BRI_DIRECTORY_BLOCK **drb_list;
    BXI_QUEUE drb_queue;
    BR_UINT_32 dir_cnt;
    BR_UINT_32 i;

    	/*  Object ids are used as the index into the DRB list.
    	 *  Id 0 is reserved for the chunk list, 1 for the topic list,
	 *  and 2 for the ext. ref. list.
    	 *
    	 *  NOTE: We first store the DRB's in a queue because we don't know
    	 *  how many directories are actually in the book.  We can't depend
    	 *  upon the field "n_directories" in the book header for books
	 *  prior to V3.
	 *  Starting with V3.0, this is filled in, as is the directory_id
    	 */

        dir_cnt = 0;

        BXI_QINIT(&drb_queue);

	vdh_pme = read_page(env, book->vbh.dir_headr_page,
			    BODS_C_DIRECTORY_HEADER_PAGE);
        while (! BRI_BUFFER_END(vdh_pme) ) {

            BRI_GET_TAG(env,vdh_pme,&tag) ;

            if (tag.rec_length == 0) {
                break ;
            }

            switch (tag.rec_type) {
                case BODS_C_DIRECTORY_HEADER: {
            
    	    	    DrbQueuePtr drb_entry;
                    BRI_DIRECTORY_BLOCK *drb;

    	    	    drb_entry = (DrbQueuePtr) BXI_MALLOC(sizeof(DrbQueue));
                    drb = BriCalloc(BRI_DIRECTORY_BLOCK,1,env);

                    BXI_INSQT(&drb_queue,drb_entry);
    	    	    drb_entry->drb = drb;

                    BRI_GET_LONG(env,vdh_pme,&drb->vdh.directory_id);
                    BRI_GET_BYTE(env,vdh_pme,&drb->vdh.flags.all_flags);
                    BRI_GET_LONG(env,vdh_pme,&drb->vdh.n_entries);
                    BRI_GET_LONG(env,vdh_pme,&drb->vdh.dir_entries_page);
                    BRI_GET_BYTE(env,vdh_pme,&drb->vdh.name_len);
                    BRI_GET_VAR_ARRAY(env,vdh_pme,drb->vdh.directory_name,drb->vdh.name_len);
                    drb->book = (BMD_BOOK_ID *)book;
                    drb->entry_list = NULL;
		    drb->directory_object_id = drb->vdh.directory_id;
                    dir_cnt++;
                }
                

            }
            BRI_NEXT_RECORD(vdh_pme,&tag);
        }   	/* while not end of buffer */

        if (dir_cnt != book->vbh.n_directories) {
            /* Adjust the directory count in the book header to be the
             * number of directories actually found.
             */
            book->vbh.n_directories = dir_cnt;
        }

    	/*  Allocate the drb_list and copy the drb pointers from the queue.
    	 */
    	drb_list = BriCalloc(BRI_DIRECTORY_BLOCK *, 
	    book->vbh.n_directories,env);
    	book->drb_list = drb_list;
    	i = 0;
    	while ( !BXI_QEMPTY(&drb_queue) && (i < book->vbh.n_directories)) {
    	    DrbQueuePtr entry;

    	    entry = (DrbQueuePtr) drb_queue.flink;
	    drb_list[i] = entry->drb;
    	    i++;
	    BXI_REMQ(entry);
    	    BXI_FREE(entry);
    	}
        

} /* end BriGetV3DirHeaders  */

void
BriBookGetHeader(env)
    BRI_CONTEXT *env;
/*
 *
 * Function description:
 *
 *      Get the book header form the book file.
 *
 * Arguments:
 *
 *      env - context for the book
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Does a BriLongJmp() if an error occurs.
 *
 */

{
    register BRI_BOOK_BLOCK	*book = env->data.book;
    BRI_PAGE_MAP_ENTRY		*vbh_pme;
    register BRI_STRING_LIST	**alt_prod_names;
    BR_UINT_32			fill_len;
    static BODS_PAGE_INDEX_ENTRY vbh_pie = { { VBH_C_BLOCK_NUMBER,
                                              VBH_C_BYTE_OFFSET
                                            } ,
                                            VBH_C_RECORD_LENGTH 
                                          } ;

    /* Initialize the dummy page map entry for the book header then
     * call read_page to read it.
     */
    book->page_map = &vbh_pme;
    vbh_pme = NULL;
    book->page_index = &vbh_pie;
    vbh_pme = read_page(env,0,BODS_C_BOOK_HEADER);

    /* Get the individual fields from the book header into VBH in the
     * book block.
     */

    BRI_GET_WORD(env,vbh_pme,&book->vbh.version.major_num);
    BRI_GET_WORD(env,vbh_pme,&book->vbh.version.minor_num);
    BRI_GET_ARRAY(env,vbh_pme,book->vbh.LMF_producer);
    BRI_GET_ARRAY(env,vbh_pme,book->vbh.LMF_product_name);
    BRI_GET_ARRAY(env,vbh_pme,book->vbh.LMF_product_date);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.LMF_product_version);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.n_pages);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.n_chunks);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.n_directories);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.n_fonts);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.max_font_id);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.page_index_rfa.block_number);
    BRI_GET_WORD(env,vbh_pme,&book->vbh.page_index_rfa.byte_offset);
    BRI_GET_WORD(env,vbh_pme,&book->vbh.page_index_length);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.vbh_extension);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.chunk_index_page);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.chunk_titles_page);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.symbol_table_page);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.font_definitions_page);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.first_data_page);
    BRI_GET_LONG(env,vbh_pme,&book->vbh.copyright_chunk);
    BRI_GET_BYTE(env,vbh_pme,&book->vbh.name_length);
    BRI_GET_ARRAY(env,vbh_pme,book->vbh.book_name);

    /*  Check version level of book file 
     *	Valid versions so far are 1.0, 2.0, and 3.0, but for forward
     *	compatibility from 3.0 to 3.*, we will not check the minor 
     *  version for Version 3.
     */

    if ( book->vbh.version.major_num == 0 
	||  book->vbh.version.major_num < 3 && book->vbh.version.minor_num != 0
	||  book->vbh.version.major_num > 3 ) 
    {
        BriLongJmp(env,BriErrBadVersionNum,&book->vbh.version) ;
    }

    switch (book->vbh.version.major_num) {
        case 1: { 
            book->alternate_product_names = &v1_alternate_product_name;
            break ;
        }
        case 2: {
            register BR_UINT_32 index;
            
            BRI_GET_ARRAY(env,vbh_pme,book->vbh.book_build_date);
            BRI_GET_ARRAY(env,vbh_pme,book->vbh.symbolic_name);
            BRI_GET_BYTE(env,vbh_pme,&book->vbh.LMF_alternate_product_count);
            vbh_pme->tag.rec_length = VBH_C_RECORD_LENGTH ;
            index = vbh_pme->index;
	    /* vbh.filler_offset is where the V2 filler started  */
            BRI_GET_VAR_ARRAY(env,vbh_pme,&book->vbh.filler_offset,722);
            BRI_GET_LONG(env,vbh_pme,&book->vbh.checksum);
            vbh_pme->index = index;

            book->vbh.oaf_version.major_num = 0;
            book->vbh.oaf_version.minor_num = 0;	    
            book->vbh.copyright_topic = NULL;
            book->vbh.dir_headr_page = NULL;
            book->vbh.symbol_exref_index_page = NULL;
            book->vbh.static_exref_index_page = NULL;
            book->vbh.n_symbols = NULL;
            book->vbh.n_symbol_exrefs = NULL;
            book->vbh.n_sys_directories = NULL;
            book->vbh.bwi_version.major_num = 0;
            book->vbh.bwi_version.minor_num = 0;	    

            alt_prod_names = &book->alternate_product_names;
            book->alternate_product_names = NULL ;
            break ;
        }
        case 3: {
            register BR_UINT_32 index;
            BRI_GET_ARRAY(env,vbh_pme,book->vbh.book_build_date);
            BRI_GET_ARRAY(env,vbh_pme,book->vbh.symbolic_name);
            BRI_GET_BYTE(env,vbh_pme,&book->vbh.LMF_alternate_product_count);
            BRI_GET_WORD(env,vbh_pme,&book->vbh.filler_offset);
            BRI_GET_LONG(env,vbh_pme,&book->vbh.copyright_topic);
            BRI_GET_LONG(env,vbh_pme,&book->vbh.dir_headr_page);
            BRI_GET_LONG(env,vbh_pme,&book->vbh.symbol_exref_index_page);
            BRI_GET_LONG(env,vbh_pme,&book->vbh.static_exref_index_page);
            BRI_GET_LONG(env,vbh_pme,&book->vbh.n_symbols);
            BRI_GET_LONG(env,vbh_pme,&book->vbh.n_symbol_exrefs);
            BRI_GET_LONG(env,vbh_pme,&book->vbh.n_sys_directories);
            BRI_GET_WORD(env,vbh_pme,&book->vbh.bwi_version.major_num);
            BRI_GET_WORD(env,vbh_pme,&book->vbh.bwi_version.minor_num);
            vbh_pme->tag.rec_length = VBH_C_RECORD_LENGTH ;
            index = vbh_pme->index;

	    /*  NOTE:  for forward compatibility, we need to allow for new 
	     *	    fields to be added here.  The filler_offset field
	     *	    tells us where the variable part begins.  We first
	     *	    read in all of the rest of the bytes 
	     */
            fill_len = VBH_C_RECORD_LENGTH - index - sizeof(book->vbh.checksum);
            BRI_GET_VAR_ARRAY(env,vbh_pme,book->vbh.vbh_filler, fill_len);
            BRI_GET_LONG(env,vbh_pme,&book->vbh.checksum);
            vbh_pme->index = book->vbh.filler_offset;

            alt_prod_names = &book->alternate_product_names;
            book->alternate_product_names = NULL ;
            break ;
        }
        default: {
            BriLongJmp(env,BriErrBadVersionNum,&book->vbh.version) ;
        }
    }

    /* Get any directory headers or alternate product names from the header
     * filler.  
     */
    { 
    	BRI_DIRECTORY_BLOCK **drb_list;
    	BXI_QUEUE drb_queue;
    	BR_UINT_32 directory_num;
        BR_UINT_32 dir_cnt;
    	BR_UINT_32 i;
        BODS_TAG tag;

    	/*  Object ids are used to compute the index into the DRB list.
    	 *  Id 0 is reserved for the chunk list, 1 for the topic list,
	 *  and 2 for the ext. ref. list.
	 *  We first store the DRB's in a local queue because we don't
	 *  know the number of directories in the book for V1 and V2 books
	 *  (the book header "n_directories" field was not filled in)
	 *  Starting with V3.0, this is filled in, as is the directory_id
    	 */

    	directory_num = BODS_C_FIRST_BOOK_DIR_NUM;
        dir_cnt = 0;
        BXI_QINIT(&drb_queue);
        while (! BRI_BUFFER_END(vbh_pme) ) 
	{
            BRI_GET_TAG(env,vbh_pme,&tag) ;
            if (tag.rec_length == 0) 
                break ;

            switch (tag.rec_type) 
	    {
                case BODS_C_DIRECTORY_HEADER: 
		{
    	    	    DrbQueuePtr drb_entry;
                    BRI_DIRECTORY_BLOCK *drb;

    	    	    drb_entry = (DrbQueuePtr) BXI_MALLOC(sizeof(DrbQueue));
                    drb = BriCalloc(BRI_DIRECTORY_BLOCK,1,env);
                    BXI_INSQT(&drb_queue,drb_entry);
    	    	    drb_entry->drb = drb;

                    BRI_GET_LONG(env,vbh_pme,&drb->vdh.directory_id);
                    BRI_GET_BYTE(env,vbh_pme,&drb->vdh.flags.all_flags);
                    BRI_GET_LONG(env,vbh_pme,&drb->vdh.n_entries);
                    BRI_GET_LONG(env,vbh_pme,&drb->vdh.dir_entries_page);
                    BRI_GET_BYTE(env,vbh_pme,&drb->vdh.name_len);
                    BRI_GET_VAR_ARRAY(env,vbh_pme,drb->vdh.directory_name,drb->vdh.name_len);

                    drb->book = (BMD_BOOK_ID *)book;
                    drb->entry_list = NULL;
                    if (book->vbh.version.major_num < 3)
			BriStoreDirObjectId(drb,directory_num);
    	    	    directory_num++;
                    dir_cnt++;
                    break;
                }

                case BODS_C_PARTNUM :
                    book->partnum = (char *)BRI_DATA_ADDRESS(vbh_pme);
                    break;
                case BODS_C_TOOLSID :
                    book->toolsid = (char *)BRI_DATA_ADDRESS(vbh_pme);
                    break;
                case BODS_C_COPYRIGHT_CORP :
                    book->copyright_corp = (char *)BRI_DATA_ADDRESS(vbh_pme);
                    break;
                case BODS_C_COPYRIGHT_DATE :
                    book->copyright_date = (char *)BRI_DATA_ADDRESS(vbh_pme);
                    break;
                case BODS_C_COPYRIGHT_TEXT :
                    book->copyright_text = (char *)BRI_DATA_ADDRESS(vbh_pme);
                    break;
                case BODS_C_LANGUAGE :
                    book->language = (char *)BRI_DATA_ADDRESS(vbh_pme);
                    break;
                case BODS_C_BOOKNAME :
                    book->bookname = (char *)BRI_DATA_ADDRESS(vbh_pme);
                    break;
                
                case BODS_C_ALTERNATE_PRODUCT: {
                    
                    if (book->vbh.version.major_num < 2) {
                        break ;
                    }
                    *alt_prod_names = BriCalloc(BRI_STRING_LIST,1,env);
                    (*alt_prod_names)->string = (char *)BRI_DATA_ADDRESS(vbh_pme);
                    (*alt_prod_names)->next = NULL ;
                    alt_prod_names = &(*alt_prod_names)->next ;
                }
                default:
                    /*  ignore unknown tags  */
                    break;
            }
            BRI_NEXT_RECORD(vbh_pme,&tag);
        }   	/* while not end of buffer */

        if( book->vbh.version.minor_num < 2) 
	{
            if (dir_cnt != book->vbh.n_directories) 
	    {
                /* Adjust the directory count in the book header to be the
                * number of directories actually found.
                */
                book->vbh.n_directories = dir_cnt;
            }

            /*  Allocate the drb_list and copy the drb pointers from the queue*/
            drb_list = BriCalloc(BRI_DIRECTORY_BLOCK *,
		book->vbh.n_directories, env);
	    book->drb_list = drb_list;
            i = 0;
            while ( !BXI_QEMPTY(&drb_queue) && (i < book->vbh.n_directories)) 
	    {
                DrbQueuePtr entry;

                entry = (DrbQueuePtr) drb_queue.flink;
                drb_list[i] = entry->drb;
                i++;
                BXI_REMQ(entry);
                BXI_FREE(entry);
            }
        } /* end if( book->vbh.version.major_num < 3) */
     }
        
} /* end bri_open_book */

void
BriBookGetDataPage(env,pgid)
    BRI_CONTEXT *env;
    BMD_OBJECT_ID pgid;
/*
 *
 * Function description:
 *
 *      Get a data page from a book.
 *
 * Arguments:
 *
 *      env - context of the book
 *
 * 	pgid - id of the data page
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      BriLongJmp() if an error occurs.
 *
 */
{
    register BRI_PAGE_MAP_ENTRY *pme;
    register BODS_PAGE_HEADER *page;
    register BR_UINT_32 index ;

    /* Read the page from the file.
     */
    pme = read_page(env,pgid,BODS_C_PAGE);

    /* Allocate the page header buffer and get the header info from
     * the page.
     */
    page = BriCalloc(BODS_PAGE_HEADER,1,env);
    pme->data_page = page ;

    BRI_GET_LONG(env,pme,&page->page_id);
    BRI_GET_LONG(env,pme,&page->page_type);
    BRI_GET_LONG(env,pme,&page->n_chunks);
    BRI_GET_LONG(env,pme,&page->prev_page_id);
    BRI_GET_LONG(env,pme,&page->next_page_id);

    /* Allocate a buffer for the chunk headers and then get them from the
     * page.  Note that the chunk data is align if necessary (mips only).
     */
    page->chunks = BriCalloc(BODS_CHUNK_HEADER,page->n_chunks,env);

    for ( index = 0 ; index < page->n_chunks; index ++) {
         register BODS_CHUNK_HEADER *ck = &page->chunks[index];

         BRI_GET_TAG(env,pme,&ck->tag);
         BRI_GET_LONG(env,pme,&ck->data_type);
         BRI_GET_LONG(env,pme,&ck->chunk_id);
         BRI_GET_LONG(env,pme,&ck->parent_chunk);
         BRI_GET_LONG(env,pme,&ck->x);
         BRI_GET_LONG(env,pme,&ck->y);
         BRI_GET_LONG(env,pme,&ck->width);
         BRI_GET_LONG(env,pme,&ck->height);
         BRI_GET_LONG(env,pme,&ck->target);
         BRI_GET_LONG(env,pme,&ck->data_length);
         ck->data = BriAlign(BR_UINT_8,BRI_DATA_ADDRESS(pme),ck->data_length,env);
         BRI_NEXT_RECORD(pme,&ck->tag);
    }
} /* end BriBookGetDataPage */

char *replace_blank_entry( env)
BRI_CONTEXT *env;
/*
 *
 * Function description:
 *    Copy the string "UNTITLED" from the UIL file 
 *    BKR_UI_TEST_STRINGS into the text string passed in.
 *
 *
 * Arguments:
 *
 *      char   *text pointer where to place text
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
#ifndef MSWINDOWS
    char    *error_string;
    char    *text;

        /* Fetch a translatable string to fill in an
         * entry with no text; also pop a message box
         * telling what we've done, We only do this once per
	 * book to prevent a plethora of message boxes that 
	 * will have to be acknowledged 
	 */
	error_string = (char *) bkr_fetch_literal("s_untitled_entry", 
							ASCIZ);
	if ( error_string != NULL )
	{
	    text = (char *)BriMalloc((strlen(error_string) + 1),env );
	    strcpy(text,error_string);
	    XtFree( error_string );

	    if(env->data.book->blank_dir_entry == 0) {
		bkr_error_set_parent_shell( NULL );
		BriError(env, BriErrDirEntryNoText);
	    }

	    env->data.book->blank_dir_entry++;

	    return(text);
	 }	   
#else
    return( UNTITLED )
#endif
} /* replace_blank_entry */

static BODS_DIRECTORY_ENTRY *
GetDirEntry(dir_ctx,index)
/*
 *
 * Function description:
 *
 *      
 *
 * Arguments:
 *
 *      None
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
    register DirContext *dir_ctx;
    BR_UINT_32 index;
{
    register char *data;
    register BODS_DIRECTORY_ENTRY *dre;
    char    *error_string;
    BR_INT_32	    i;

    dir_ctx->entries_used++;
    dre = &dir_ctx->drb->entry_list[dir_ctx->entries_used];

    BRI_GET_TAG(dir_ctx->env,dir_ctx->pme,&dre->tag);
    if (dre->tag.rec_type != BODS_C_DIR_ENTRY) {
        BriLongJmp(dir_ctx->env,BriErrDirReadNum,index);
    } 
    BRI_GET_BYTE(dir_ctx->env,dir_ctx->pme,&dre->header_level);
    BRI_GET_BYTE(dir_ctx->env,dir_ctx->pme,&dre->data_type);
    BRI_GET_LONG(dir_ctx->env,dir_ctx->pme,&dre->width);
    BRI_GET_LONG(dir_ctx->env,dir_ctx->pme,&dre->height);
    BRI_GET_BYTE(dir_ctx->env,dir_ctx->pme,&dre->title_length);
    BRI_GET_BYTE(dir_ctx->env,dir_ctx->pme,&dre->n_targets);
    BRI_GET_WORD(dir_ctx->env,dir_ctx->pme,&dre->data_length);
    data = (char *)BRI_DATA_ADDRESS(dir_ctx->pme);
    dre->data = BriAlign(BR_UINT_8,data,dre->data_length,dir_ctx->env);
    data = &data[dre->data_length];
    /* if the for loop isn't broken no printable characters where found */
    for(i = 0; i < dre->title_length; i++) {
	if(isgraph(data[i]))
	    break;
    }    
    if ((dre->title_length == 0) || (*(char *)data == '\0') || (i == dre->title_length)) {
	dre->title = FtextToAscii(dir_ctx->env,dre->data,dre->data_length);
	if ( dre->title ){
	    /* Replace any newlines (\n) with spaces.
	    */
	    for (i = 0; dre->title[i] != '\0'; i++) {
		if (dre->title[i] == '\n') {
		    dre->title[i] = ' ';
		}
	    }
        } else {

            /* Use an "Untitled" string and produce an error message if this is
             * the first call.
             */
            dre->title = replace_blank_entry(dir_ctx->env);
        }

    }
    else {
        dre->title = BriAlign(char,data,dre->title_length,dir_ctx->env);
    }
    data = &data[dre->title_length];
    dre->target_list = BriAlign(BR_UINT_32,data,dre->n_targets,dir_ctx->env);
    BRI_NEXT_RECORD(dir_ctx->pme,&dre->tag);
#ifdef DEBUG_DIR
fprintf(stderr,"%4d: Header level = %2d ",index,dre->header_level);
#endif 
    if (dre->header_level > (dre[-1].header_level + 1)) {
        dre->header_level = dre[-1].header_level + 1;
    }
    dre->title_length = strlen(dre->title) + 1;
    return dre;

} /* end GetDirEntry */


static void
SetTOCHeaderLevel(dre)
/*
 *
 * Function description:
 *
 *      Sets the header level for a Table Of Contents directory entry, based
 *      on the number of dots in the initial nonblank substring.  Used 
 *      primarily for numbered headers and Appendices.
 *
 * Arguments:
 *
 *      dre - the directory entry
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
    register BODS_DIRECTORY_ENTRY *dre;

{
    BR_INT_32 n_dots = 0;
    char *ptr = &dre->title[1];
    
    /* Scan the entry title string.
     */
    while (*ptr) {

        if (isspace(*ptr)) {
            
            /* This is the end of the initial nonblank substring, stop.
             */
            break;
        } 

        /* Count the dots.
         */
        if (*ptr == '.') {
            n_dots++;
        }
        ptr++;
    }

    /* Set the header level to 1 + the number of dots, unless the
     * number of dots is already greater than the header level for the
     * previous entry.  In that case this header level can only be one 
     * greater than the previous header level.
     */
    if (n_dots <= (dre[-1].header_level)) {
        dre->header_level = n_dots + 1;
    } else {
        dre->header_level = dre[-1].header_level + 1;
    }
} /* end SetTOCHeaderLevel */


static void
InsertTargetEntries(dir_ctx,n_targets)
/*
 *
 * Function description:
 *
 *      Inserts extra (sub)entries in an index directory for index entries
 *      that have multiple targes.
 *
 * Arguments:
 *
 *      dir_ctx - the context for the directory
 *
 *      n_targets - the number of target entries to be inserted
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Reallocates the directory entry array to make room for the new
 *      entries.
 */
    DirContext *dir_ctx;
    register BR_UINT_32 n_targets;
{
    register BODS_DIRECTORY_ENTRY *dre;
    register BR_UINT_32 target;
    register BR_UINT_32 target_level;
    register char **chunk_titles;
    BR_INT_32 offset;
    BR_UINT_32 new_size;
    int msg_len = 0;
    char msg_buffer[1000];

    static char *bad_target = NULL;

    /* Reallocate a larger buffer for the directory entries
     */
    dir_ctx->max_entries += n_targets;
    new_size = sizeof(BODS_DIRECTORY_ENTRY) * dir_ctx->max_entries;
    dre = (BODS_DIRECTORY_ENTRY *)
          BriRealloc(dir_ctx->drb->entry_list,new_size,dir_ctx->env);
    dir_ctx->drb->entry_list = dre;
    
    /* Get a pointer to the parent entry for the target subentries.  The
     * parent entry will be marked as no longer having targets, only
     * subentries.
     */
    dre = &dre[dir_ctx->entries_used];
    dre->n_targets = 0;

    /* The header level for the target entries will be one greater
     * than the parent entry.  The entry titles will come from the chunk
     * titles for the target chunks.
     */
    target_level = dre->header_level + 1;
    chunk_titles = dir_ctx->env->data.book->chunk_titles;
#ifdef DEBUG_DIR
    fprintf(stderr,"Inserting %d targets\n",n_targets);
#endif 

    /* Set up the target subentries.
     */
    offset = 0;
    for (target = 1; target <= n_targets; target++) 
    {
        BR_UINT_32 target_chunk_id = dre->target_list[target-1];

        if (target_chunk_id > dir_ctx->env->data.book->vbh.n_chunks) 
        {
            offset++;
            if (bad_target == NULL) {
                bad_target = bkr_fetch_literal("BriBadTargetMsg",ASCIZ);
                if (bad_target == NULL) {
                    bad_target = "Bad target id = %d, for subentry %u of %u of entry \"%s\"\n";
                }
            }
            sprintf(&msg_buffer[msg_len],
                    bad_target,
                    target_chunk_id,
                    target,
                    n_targets,
                    dre->title);
            msg_len = strlen(msg_buffer);
        }
        else 
        {
            BODS_DIRECTORY_ENTRY *target_dre = &dre[target-offset];
            
            target_dre->header_level = target_level;
            target_dre->data_type = 0;
            target_dre->width = 0;
            target_dre->height = 0;
            target_dre->n_targets = 1;
            target_dre->data_length = 0;
            target_dre->data = NULL;
            target_dre->title = chunk_titles[target_chunk_id];
            if ((target_dre->title == NULL) || (*target_dre->title == '\0')) 
            {
                target_dre->title = dre->title ;
                target_dre->title_length =  dre->title_length;
            } 
            else 
            {
                target_dre->title_length = strlen(target_dre->title)+1;
            }
            target_dre->target_list = &dre->target_list[target-1];
        }
    }
    /* Adjust the entries used for this directory by the number of target
     * subentries.
     */
    dir_ctx->entries_used += (n_targets - offset);
    if (offset > 0) 
    {
        BriError(dir_ctx->env,
                 BriErrBadDirTargetNum,
                 msg_buffer);
    }

} /* end InsertTargetEntries */
 
static void
ReadPreV3TOC(dir_ctx)
/*
 *
 * Function description:
 *
 *      Read the TOC for a pre-V3 book and clean up the header levels for
 *      the entries.
 *
 * Arguments:
 *
 *      dir_ctx - the directory context for the TOC
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
    register DirContext *dir_ctx;
{
    register BODS_DIRECTORY_ENTRY *dre;
    register BR_UINT_32 index;
    enum     { UnNumberedHead, NumberedHead, Appendix } header_type;
    Boolean  subdirectory = FALSE;

    /* Assume we'll start with unnumbered headers, which will essentially
     * let the headers pass through unchanged.
     */
    header_type = UnNumberedHead;

    for (index = 2 ; index <= dir_ctx->drb->vdh.n_entries ; index++) {

        /* Get the entry, if it has no targets then we're at the end
         * of the regular TOC entries, and starting the included
         * entries from the Examples, Figures, or Tables directories.
         * Since those have are in separate directories that follow
         * the TOC in the window, they don't need to be replicated
         * here.
         */
        dre = GetDirEntry(dir_ctx,index);
        if (dre->n_targets == 0) {
            
            /* Entries with no targets are handled specially in the TOC.
             */
            if (dir_ctx->entries_used == 1) {

                /* This is dummy entry from a DECwrite book which was used for
                 * formatting a TOC with the V2 Bookreader.  Just get rid of
                 * the entry.
                 */
                dir_ctx->entries_used--;
            } else {

                /* This is probaly the start of a section like "Figures",
                 * "Examples", etc.  If there is a directory with the same
                 * title then we can quit processing the TOC.
                 */

                BRI_DIRECTORY_BLOCK *drb;	/* Directory block */
                BR_UINT_32 dir_cnt;

                /* Search the directory list for a matching directory
                 */
                dir_cnt = 0;
                while (dir_cnt < dir_ctx->env->data.book->vbh.n_directories) {

                    drb = dir_ctx->env->data.book->drb_list[dir_cnt];
                    if (streq(drb->vdh.directory_name,dre->title)) {
                        dir_ctx->entries_used--;
                        return ;
                    }
                    dir_cnt++;
                }

                /* There are no directories with the same name.  So leave this and
                 * its subentries in.  Make sure this entry is at level one, and all
                 * of it's subdirectory entries are under it at level 2.
                 */
                dre->header_level = 1;
                subdirectory = TRUE;
            }
        } else if (subdirectory) {

            /* Entries for all "Figures", "Examples", etc. subdirectories must be at
             * level 2.
             */
            dre->header_level = 2;

#ifdef I18N_MULTIBYTE
        } else if (!(*dre->title & 0x80) && isdigit(*dre->title)) {
#else
        } else if (isdigit(*dre->title)) {
#endif

            /* This is a numbered head, so base the header level on the
             * number of dots in the header, i.e. header level = 1 + the
             * number of dots.
             */
            SetTOCHeaderLevel(dre);
            header_type = NumberedHead;

        } else if (header_type == Appendix) {

            /* Were processing an appendix, so count dots to set the
             * header level, just like numbered heads.
             */
            SetTOCHeaderLevel(dre);

        } else if (dre->title[1] == '.') {

            /* We've started an appendix, since the first character is
             * not a digit, but the second character is dot.  So remember
             * that and set the header level.  Were assuming that the
             * previous header was a level one.  In any case the
             * SetTOCHeaderLevel() routine will make sure that this is
             * no more than one greater than the previous level.
             */
            header_type = Appendix;
            SetTOCHeaderLevel(dre);

        } else if (header_type == NumberedHead) {

            /* Special processing if we've been processing numbered
             * headers, but this entry doesn't begin with a digit.
             */
            if (dre->header_level == 1) {

                /* This is probably the start of an appendex section or
                 * some other unnumbered section like a glossary after
                 * the numbered heads.  Of course, it could be a single
                 * unnumbered head use to group sections of numbered
                 * heads.  Is so, well switch back next iteration.
                 */
                header_type = UnNumberedHead;

            } else if (dre->header_level <= dre[-1].header_level) {

                /* We'll ssume that these are unnumbered heads which
                 * are subordinate to a numbered head.  So we'll
                 * shift them up a level to make them subordinate.
                 * This is only done if the previous entry was a numbered
                 * header and the header level of this entry is
                 * greater than 1.
                 */
                dre->header_level++;
            }
        }
#ifdef DEBUG_DIR
        fprintf(stderr,"-> %2d, Title = \"%s\"\n", dre->header_level,dre->title);
#endif 
    }
} /* end ReadPreV3TOC */

static void
RdPreV3Index(dir_ctx)
/*
 *
 * Function description:
 *
 *      Read the Index for a pre-V3 book and clean up the header levels for
 *      the entries.
 *
 * Arguments:
 *
 *      dir_ctx - the directory context for the Index
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

  /*---
  text_ptr = (BMD_TEXT_PKT *)dre->data;
  first_x = text_ptr->x;
  ---*/



  register DirContext *dir_ctx;
  {
  register BODS_DIRECTORY_ENTRY *dre;
  register BR_UINT_32 index;


#ifdef I18N_MULTIBYTE
  char *lang = (char *)xnl_getlanguage();
#define IsJapanese(s) (((s) != NULL) && ((s)[0] == 'j') && ((s)[1] == 'a'))
#endif

  /*--- setup to get first dre - directory entry struct, just return if the
        number of entries is less than 3 ---*/
  if(dir_ctx->entries_used == 0)
    {
    if(dir_ctx->drb->vdh.n_entries > 2)
      {
      index = 3;
      dre = GetDirEntry(dir_ctx,index);
      }
    else
      {
      return;
      }
    }
  else
    {
    dre = &dir_ctx->drb->entry_list[dir_ctx->entries_used];
    index = 2;
    }

#ifdef DEBUG_DIR
    fprintf(stderr,"-> %2d, Title = \"%s\"\n",dre->header_level,dre->title);
#endif 

  /*--- first entry should be a guide-heading - for VAX DOC and DECWrite
        books this is a single char, ditroff books have some extra chars
        here. Thus, this is the way that we tell what authoring tool
        we have ---*/

  /*--- test to determine authoring tool as described ---*/
#ifdef I18N_MULTIBYTE
  if((dre->title_length <= 2) ||
      (IsJapanese(lang) && (dre->title_length == 3) &&
      ((*dre->title & 0xfe) == 0xa4)))
#else
  if(dre->title_length <= 2)
#endif  /*------*/
    {

    /*---------------------------------*/
    /*--- VAX Document and DECWrite ---*/
    /*---------------------------------*/

    char *lastGuideLetter = dre->title;

    /*--- loop on index entries ---*/
    while(index <= dir_ctx->drb->vdh.n_entries)
      {
      index++;
      dre = GetDirEntry(dir_ctx,index);

      if(dre->header_level == 1)
        {  /*--- header level is 1 ---*/

        /*--- if not guide-heading ---*/
#ifdef I18N_MULTIBYTE
        if ((dre->title_length > 2) &&
           (!IsJapanese(lang) || (dre->title_length != 3) || ((*dre->title & 0xfe) != 0xa4)))
#else
        if (dre->title_length > 2)
#endif  /*------*/

          {
          /* if prev header_level is 1 or first letter of this
             title is the same as *lastGuideLetter, set level to 2 */

          if ((dre[-1].header_level == 1) ||
              (*dre->title == *lastGuideLetter))
            {
            dre->header_level = 2;
            }
          /* else if first char of title is alpha, set level to 3 */
#ifdef I18N_MULTIBYTE
          else if (!(*dre->title & 0x80) && isalpha(*dre->title))
#else
          else if (isalpha(*dre->title))
#endif
            {
            dre->header_level = 3;
            }

          /* else if second char of title == *lastGuideLetter, set to 2 */
          else if (dre->title[1] == *lastGuideLetter)
            {
            dre->header_level = 2;
            }
#ifdef I18N_MULTIBYTE
          else if (IsJapanese(lang) && ((*lastGuideLetter & 0xfe) == 0xa4))
            {
            dre->header_level = 2;
            }
#endif
          else
            {
            dre->header_level = 3;
            }
          }

        else  /*--- guide heading ---*/
          {
          lastGuideLetter = dre->title;
          }
        }    /* end if(dre->header_level == 1) */


      else  /*--- header level != 1 ---*/
      /* if current header level <= previous, increment currrent  */
      if (dre->header_level <= dre[-1].header_level)
        {
        dre->header_level++;
        }

      /* else (level != 1) && (current > previous), has to be more by just 1 */
      else
        {
        dre->header_level = dre[-1].header_level + 1;
        }

#ifdef DEBUG_DIR
      fprintf(stderr,"-> %2d, Title = \"%s\"\n",dre->header_level,dre->title);
#endif
      if (dre->n_targets > 1)
        {
        InsertTargetEntries(dir_ctx,dre->n_targets);
        }
      }
    }

  else
    {
    /*---------------*/
    /*--- ditroff ---*/
    /*---------------*/

    char *lastLevel2 = NULL;
    char *lastLevel3 = NULL;
    char *lastGuideLetter = dre->title;
    char *ptr;

    while (index <= dir_ctx->drb->vdh.n_entries)
      {   /*--- loop through index entries ---*/

      index++;

      /* get the directory entry struct */
      dre = GetDirEntry(dir_ctx,index);

      /* backup and filter stuff at end of string */
      ptr = &dre->title[dre->title_length - 2];
      if ((*ptr) == 't' && isspace(ptr[-1]))
        {
        ptr--;
        }
      while (isspace(*ptr))
        {
        ptr--;
        }

      dre->title_length = (ptr - dre->title) + 2;

      /* if entry is empty, don't bother */
      if((dre->title_length < 1) && (dre->n_targets == 0))
        {
        dir_ctx->entries_used--;
        continue;
        }

      /* entry has some substance */
      ptr[1] = (char)0;

      if(dre->header_level == 1)
        {
        if(dre->title_length > 2)
          {  /* level 1, non guide-heading, change to level 2 */
          dre->header_level = 2;
          lastLevel2 = dre->title;
          }
        else
          {  /* guide-heading, leave as level 1 */
          lastGuideLetter = dre->title;
          lastLevel2 = NULL;
          }
        lastLevel3 = NULL;
        }




      else if (lastLevel3 && substreq(lastLevel3,dre->title))
        {
        if((dre->n_targets == 0) && streq(lastLevel3,dre->title))
          {
          dir_ctx->entries_used--;
          }
        else
          {
          dre->header_level = 4 ;
          }
        }
      else if (lastLevel2)
        {
        if(substreq(lastLevel2,dre->title))
          {
          if((dre->n_targets == 0) && streq(lastLevel2,dre->title))
            {
            dir_ctx->entries_used--;
            }
          else
            {
            lastLevel3 = dre->title;
            if((dre[-1].header_level == 3) && (dre[-1].n_targets == 0)) 
              {
              dre->header_level = 4;
              }
            else
              {
              dre->header_level = 3 ;
              }
            }
          }
        else if(_br_tolower(*lastGuideLetter) != _br_tolower(*dre->title))
          {
          lastLevel3 = dre->title;
          dre->header_level = 3 ;
          }
        else
          {
          dre->header_level = 2;
          lastLevel2 = dre->title;
          lastLevel3 = NULL;
          }
        }
      else
        {
        dre->header_level = 2;
        lastLevel2 = dre->title;
        lastLevel3 = NULL;
        }

#ifdef DEBUG_DIR
                fprintf(stderr,"-> %2d, Title = \"%s\"\n",dre->header_level,dre->title);
#endif 
      if(dre->n_targets > 1)
        {
        InsertTargetEntries(dir_ctx,dre->n_targets);
        }
      }
    }

  return;
  }





int
get_dir_x_offset PARAM_NAMES((dre))
    BODS_DIRECTORY_ENTRY *dre PARAM_END
{
    BMD_FTEXT_PKT *ftext_pkt;
    BMD_TEXT_PKT  *text_pkt;

    ftext_pkt = (BMD_FTEXT_PKT *)dre->data;
    text_pkt = (BMD_TEXT_PKT  *)&ftext_pkt->value[0];

    return ((int)text_pkt->x);
}


#define MAX_LEVELS 8

static void
ReadPreV3Index(dir_ctx)
/*
 *
 * Function description:
 *
 *      Read the Index for a pre-V3 book and clean up the header levels for
 *      the entries.
 *
 * Arguments:
 *
 *      dir_ctx - the directory context for the Index
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
  register DirContext *dir_ctx;
  {
  register BODS_DIRECTORY_ENTRY *dre;
  BMD_TEXT_PKT        *text_ptr;
  int                 current_x, prev_x;
  int                 level, prev_level;
  int                 x_value[MAX_LEVELS+2];
  int                 index;
  /* register BR_UINT_32 index; */
  int                 i;
  Boolean             ditroff;
  char                *ptr;
/*  FILE                *fp;  */

#ifdef I18N_MULTIBYTE
  char *lang = (char *)xnl_getlanguage();
#define IsJapanese(s) (((s) != NULL) && ((s)[0] == 'j') && ((s)[1] == 'a'))
#endif

/* DEBUG stuff */

/*  fp = fopen("x_levels.txt","w"); */

  /* init x_values to be impossible values */
  for(i=0; i <= MAX_LEVELS; ++i)
    x_value[i] = -1;

  /*--- setup to get first dre - directory entry struct, just return if the
        number of entries is less than 3 ---*/
  if(dir_ctx->entries_used == 0)
    {
    if(dir_ctx->drb->vdh.n_entries > 2)
      {
      index = 3;
      dre = GetDirEntry(dir_ctx,index);
      }
    else
      {
      return;
      }
    }
  else
    {
    dre = &dir_ctx->drb->entry_list[dir_ctx->entries_used];
    index = 2;
    }

  /* assume first entry is guide-heading, level 1 */

  /*--- first entry should be a guide-heading - for VAX DOC and DECWrite
        books this is a single char, ditroff books have some extra chars
        here. Thus, this is the way that we tell what authoring tool
        we have ---*/

  /*--- test to determine authoring tool as described ---*/
#ifdef I18N_MULTIBYTE
  ditroff = ((dre->title_length <= 2) ||
      (IsJapanese(lang) && (dre->title_length == 3) &&
      ((*dre->title & 0xfe) == 0xa4)))? FALSE: TRUE;
#else
  ditroff = (dre->title_length <= 2)? FALSE: TRUE;
#endif  /*------*/

  text_ptr = (BMD_TEXT_PKT *)dre->data;
  prev_x = x_value[1] = (int)get_dir_x_offset(dre);
  prev_level = 1;

#ifdef DEBUG_DIR
    fprintf(stderr,"-> %2d, Title = \"%s\"\n",dre->header_level,dre->title);
#endif 

  /*--- loop on index entries ---*/
  while(index <= dir_ctx->drb->vdh.n_entries)
    {
    index++;
    dre = GetDirEntry(dir_ctx,index);

    current_x = (int)get_dir_x_offset(dre);

    if(ditroff)
      {
      /* backup and filter stuff at end of string */
      ptr = &dre->title[dre->title_length - 2];
      if ((*ptr) == 't' && isspace(ptr[-1]))
        {
        ptr--;
        }
      while (isspace(*ptr))
        {
        ptr--;
        }

      dre->title_length = (ptr - dre->title) + 2;

      /* if entry is empty, don't bother */
      if((dre->title_length < 1) && (dre->n_targets == 0))
        {
        dir_ctx->entries_used--;
        continue;
        }
      }

  /* fprintf(fp,"\n title = \"%s\" level = %d, x = %d",dre->title,
              dre->header_level,current_x); */


    /*--- if guide-heading ---*/
#ifdef I18N_MULTIBYTE
    if(!((dre->title_length > 2) &&
     (!IsJapanese(lang) || (dre->title_length != 3) || ((*dre->title & 0xfe) != 0xa4))))
#else
    if(dre->title_length <= 2)
#endif  /*------*/
      {   /* guide heading, set level to 1 */
      level = 1;
      }
    else if(prev_level == 1)
      {  /* previous was guide-heading, this must be level 2 */
      level = 2;
      }
    else if(current_x > prev_x)
      { /* if x increased, level increased, can only be by 1 to be valid */
      level = prev_level +1;
      if(level > MAX_LEVELS)
        level = MAX_LEVELS;    /* safety */
      else /* only set x_values for non-stray levels */
        {
        if(x_value[level] < 0)
          x_value[level] = current_x;  /* save x value for this new level */
        }
      }
    else if(current_x == prev_x)
      {  /* x is same as prev level, use the prev level for this level */
      ;
      }
#if FALSE
    else
      { /* current_x is less than prev_x, find which level it goes with */
      while(level > 2)
        {
        if(current_x > x_value[--level])
          {
          x_value[++level] = current_x;
          break;
          }
        }
      x_value[level] = current_x;
      }
#endif
    else
      { /* current_x is less than prev_x, find which level it goes with */
      while(level > 2)
        {
        if(current_x >= x_value[--level])
          {
          break;
          }
        }
      x_value[level] = current_x;
      }

    prev_x = current_x;
    prev_level = level;

    dre->header_level = level;
    if(dre->n_targets > 1)
      {
      InsertTargetEntries(dir_ctx,dre->n_targets);
      }

    }

/*  fclose(fp); */

  return;
  }

static void
ReadPreV3Directory(dir_ctx)
    register DirContext *dir_ctx;
/*
 *
 * Function description:
 *
 *      Read the TOC for a pre-V3 book and clean up the header levels for
 *      the entries.
 *
 * Arguments:
 *
 *      dir_ctx - the directory context for the directory
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
    register BR_UINT_32 index;

    /* Get the directory entries out of the page.
     */
    for (index = 2 ; index <= dir_ctx->drb->vdh.n_entries ; index++) {

        register BODS_DIRECTORY_ENTRY *dre;

        dre = GetDirEntry(dir_ctx,index);

#ifdef DEBUG_DIR
fprintf(stderr,"-> %2d, Title = \"%s\"\n",dre->header_level,dre->title);
#endif 
        if (dre->n_targets == 0) {
            break ;
        }
    }
}


void
BriReadDirectoryPage(env,drb)
    register BRI_CONTEXT *env;
    BRI_DIRECTORY_BLOCK *drb;
/*
 *
 * Function description:
 *
 *      Read a directory page from the book.
 *
 * Arguments:
 *
 *      env - context for the book
 *
 * 	drb - directory block for the directory
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      BriLongJmp() on error.
 *
 */

{

    register BODS_DIRECTORY_ENTRY *dre;
    register BR_UINT_32 index;
    BRI_BOOK_BLOCK *book = env->data.book;
    DirContext dir_ctx;

    dir_ctx.env = env;
    dir_ctx.drb = drb;

    /* Read the page from the book.
     */
    dir_ctx.pme = read_page(dir_ctx.env,
                            dir_ctx.drb->vdh.dir_entries_page,
                            BODS_C_DIRECTORY_PAGE);

    /* Allocate a buffer for the directory entries
     */
    dre = BriCalloc(BODS_DIRECTORY_ENTRY,(drb->vdh.n_entries+1),dir_ctx.env);

    dir_ctx.drb->entry_list = dre;
    dre->header_level = 0;
    dre->title = NULL ;

    dir_ctx.entries_used = 0;
    dir_ctx.max_entries = drb->vdh.n_entries ;

    if ((book->vbh.version.major_num < 3) && 
	(!(dir_ctx.drb->vdh.flags.bits.minor_version))) {

        /* First get rid of the first entry that replicates the directory name.
         */
        dre = GetDirEntry(&dir_ctx,1);
        if (dre->n_targets == 0) {
            dir_ctx.entries_used = 0;
        }

        /* Now do directory specific processing for pre-V3 books,
         */
        if (dir_ctx.drb->vdh.flags.bits.index) {
            ReadPreV3Index(&dir_ctx);
        } else if (dir_ctx.drb->vdh.flags.bits.contents) {
            ReadPreV3TOC(&dir_ctx);
        } else {
            ReadPreV3Directory(&dir_ctx);           
        }
    } else {
        
	/* First get rid of the first entry that replicates 
	 * the directory name.
	 */
	dre = GetDirEntry(&dir_ctx,1);
	if (dre->n_targets == 0) {
	    dir_ctx.entries_used = 0;
	}

        /* Get the directory entries out of the page. Note that the data
         * is aligned on mips.
         */
        for (index = 2; index <= dir_ctx.drb->vdh.n_entries; index++) {
            dre = GetDirEntry(&dir_ctx,index);
            if(dre->n_targets > 1) {
                InsertTargetEntries(&dir_ctx,dre->n_targets);
            }
        }
    }
    drb->vdh.n_entries = dir_ctx.entries_used;

#ifdef mips
    BriClosePage(env,drb->vdh.dir_entries_page);
#endif
} /* end BriReadDirectoryPage */


void
BriBookGetFontDefinitions(env)
    BRI_CONTEXT *env;
/*
 *
 * Function description:
 *
 *      Get the font definitions for the book
 *
 * Arguments:
 *
 *      env - context for the book
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      BriLongJmp() on error.
 *
 */

{
    register BRI_BOOK_BLOCK *book = env->data.book;
    register BRI_PAGE_MAP_ENTRY *pme;
    register BR_UINT_32 index ;
    BODS_FONT_DEFINITION fd;

    pme = read_page(env,
                    book->vbh.font_definitions_page,
                    BODS_C_FONT_DEFINITIONS
                    );

    book->font_list = BriCalloc(char *,(book->vbh.max_font_id + 1),env);

    for (index = 0 ; index <= book->vbh.max_font_id ; index++ ) {
        book->font_list[index] = NULL;
    }

    while (!BRI_BUFFER_END(pme)) {
        BRI_GET_TAG(env,pme,&fd.tag);
        if (fd.tag.rec_type != BODS_C_FONT) {
            BriLongJmp(env,BriErrFontReadNum);
        }
        BRI_GET_WORD(env,pme,&fd.font_id);
        book->font_list[fd.font_id] = (char *)BRI_DATA_ADDRESS(pme);
        BRI_NEXT_RECORD(pme,&fd.tag);
    }
} /* end format_font_definitions */

static char **
ReadChunkStrings(env,pgid,page_type,data_type)
    BRI_CONTEXT *env;
    BMD_OBJECT_ID pgid;
    BR_INT_32 page_type;
    BR_INT_32 data_type;
/*
 *
 * Function description:
 *
 *      Utility routine to get different types of chunk strings from a book.
 *
 * Arguments:
 *
 *      env - context for the book
 *
 *      pgid - id of the page containing the strings
 *
 *      page_type - tag value for the page
 *
 *      data_type - type of string
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      BriLongJmp on error.
 *
 */

{
    register BRI_BOOK_BLOCK *book = env->data.book;
    register BRI_PAGE_MAP_ENTRY *pme;
    register BR_UINT_32 index;
    register char **strings;

    /* Read the page containing the strings.
     */
    pme = read_page(env,pgid,page_type); 

    /* Allocate a buffer for the string pointers
     */
    strings = BriCalloc(char *,(book->vbh.n_chunks + 2),env);

    strings[0] = NULL;

    /* Point to the strings in the page.
     */
    for (index = 1 ; index <= book->vbh.n_chunks ; index++ ) {
        BODS_TAG tag;

        if (!BRI_BUFFER_END(pme)) {
            BRI_GET_TAG(env,pme,&tag);
            if (tag.rec_type != data_type) {
                BriLongJmp(env,BriErrChkStrReadNum);
            }
            strings[index] = (char *)BRI_DATA_ADDRESS(pme);
            BRI_NEXT_RECORD(pme,&tag);
        } else {
            BriLongJmp(env,BriErrChkStrReadNum);
        }
    }
    strings[book->vbh.n_chunks + 1] = NULL;
    return strings;
} /* end ReadChunkStrings */

void
BriBookGetChunkTitles(env)
    register BRI_CONTEXT *env;
/*
 *
 * Function description:
 *
 *      Get the Chunk tittles for a book.
 *
 * Arguments:
 *
 *      env - context for the book
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      BriLongJmp() on error
 *
 */

{
    /* Just call ReadChunkStrings to get the titles
     */
    env->data.book->chunk_titles 
                    = ReadChunkStrings(env,
                                       env->data.book->vbh.chunk_titles_page,
                                       BODS_C_CHUNK_TITLES,
                                       BODS_C_TITLE
                                       );
} /* end BriBookGetChunkTitles  */

void
BriBookGetSymbolTable(env)
    register BRI_CONTEXT *env;
/*
 *
 * Function description:
 *
 *      Get the symbols for a book.
 *
 * Arguments:
 *
 *      env - context for the book
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
    register BRI_BOOK_BLOCK *book = env->data.book;
    register BRI_PAGE_MAP_ENTRY *pme;
    register BR_UINT_32 index;
    register BRI_SYMBOL_ENTRY  *bri_symbol;
    register BODS_SYMBOL_ENTRY	*bods_symbol;
    char	*string;
    BR_UINT_32	hash_value;

    if (book->symbol_table != NULL)
	return;		/*  already read in  */

    /*  Read the page containing the symbol table.  */
    pme = read_page(env,book->vbh.symbol_table_page,BODS_C_SYMBOL); 

    /*  Allocate a buffer for the hash table  */
    book->symbol_table = 
	BriCalloc(BRI_SYMBOL_ENTRY *,(BRI_SYMBOL_HASH_SIZE),env);
    memset (book->symbol_table, 0, 
	BRI_SYMBOL_HASH_SIZE * sizeof (BRI_SYMBOL_ENTRY *));

    if (book->vbh.oaf_version.minor_num < 2)
    {
	/*  read old chunk symbol format  */
	string = (char *)BRI_DATA_ADDRESS(pme);
	for (index = 1 ; index <= book->vbh.n_chunks ; index++ ) 
	{
	    BR_UINT_8   valid_symbol = FALSE;
	    BR_INT_32		    ch;

            /*  ignore generated symbols -- V2 authoring tools generated
             *  phoney numeric symbols (chunk ids, page numbers, ...)
             *  treat any purely numeric symbol as phoney.
    	     *  Also treat symbols with "_DECW" as phoney--these were
	     *  generated by VAX Document.
    	     */
	    for (ch = 0; ch < 32; ch++)
	    {
		if ( string[ch] == '\0' )
		    break;
		if ( ( isalpha( string[ch] ) ) || ( string[ch] == '_') )
		{
		    valid_symbol = TRUE;
		    break;
		}
	    }
	    if ( valid_symbol )	    /*  contains alpha or '_'  */
    	    {
    	    	/*  Symbol beginning with _DECW are phoney  */
    	    	if ( strstr( string, "_DECW" ) == string )
    	    	    valid_symbol = FALSE;
    	    }

	    if ( valid_symbol )	    /*  ignore generated symbols  */
	    {
		bri_symbol = (BRI_SYMBOL_ENTRY *)BriMalloc (sizeof(BRI_SYMBOL_ENTRY), env);
		hash_value = BriHash(index);
		bri_symbol->next = book->symbol_table[hash_value];
		bri_symbol->id = index;
		bri_symbol->name = string;
		book->symbol_table[hash_value] = bri_symbol;
	    }
	    string += 32;  /*  next string  */
	}
	return;
    }
	
    /*  Allocate a buffer for the symbol entries  */
    bri_symbol = BriCalloc(BRI_SYMBOL_ENTRY,(book->vbh.n_symbols),env);

    for (index = 0; index < book->vbh.n_symbols; index++)
    {
	bods_symbol = (BODS_SYMBOL_ENTRY *)BRI_DATA_ADDRESS(pme);
	if (bods_symbol == NULL || bods_symbol->len == 0)
	    break;

	hash_value = BriHash(bods_symbol->id);
	bri_symbol->next = book->symbol_table[hash_value];
	bri_symbol->id = bods_symbol->id;
	bri_symbol->name = bods_symbol->name;
	book->symbol_table[hash_value] = bri_symbol;

	/*  prepare for next symbol  */
	bri_symbol++;   /*  next symbol slot  */
	pme->index += bods_symbol->len;
    }
} /* end BriBookGetSymbolTable  */


void
BriBookGetSymbolExrefTable(env)
    register BRI_CONTEXT *env;
/*
 *
 * Function description:
 *
 *      Get the symbolic external references for a book.
 *
 * Arguments:
 *
 *      env - context for the book
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
    register BRI_BOOK_BLOCK *book = env->data.book;
    register BRI_PAGE_MAP_ENTRY *pme;
    register BR_UINT_32 index;
    register BRI_SYMBOL_EXREF_ENTRY *exref;
    register BR_UINT_8   *lenptr;

    if (book->vbh.oaf_version.minor_num < 2 ||
					book->vbh.symbol_exref_index_page == 0)
	return;

    if (book->symbol_exref_table != NULL)
	return;		/*  already read in  */

    /*  Read the page containing the symbolic exref table.  */
    pme = read_page( env, book->vbh.symbol_exref_index_page,
	BODS_C_SYMBOL_EXREF_PAGE ); 

    /*  Allocate a buffer for the symbolic XREF table  */
    exref = BriCalloc( BRI_SYMBOL_EXREF_ENTRY, (book->vbh.n_symbol_exrefs + 1),
	env);
    book->symbol_exref_table = exref;
    exref[0].book_name = NULL;
    exref[0].object_name = NULL;
    lenptr = (BR_UINT_8 *)BRI_DATA_ADDRESS(pme);

    for (index = 1 ; index <= book->vbh.n_symbol_exrefs; index++ ) 
    {
	if (lenptr == NULL || *lenptr == 0)
	    break;
	exref[index].book_name = (char *)&lenptr[1];
	exref[index].object_name = exref[index].book_name 
	    + strlen(exref[index].book_name) + 1;

	lenptr += *lenptr;  /*  next exref  */
	if (exref[index].object_name >= (char *)lenptr 
	    || exref[index].object_name[0] == '\0')
	    exref[index].object_name = NULL;
    }
} /* end BriBookGetSymbolExrefTable  */


void
BriBookGetStaticExrefTable(env)
    register BRI_CONTEXT *env;
/*
 *
 * Function description:
 *
 *      Get the static external references for a book.
 *
 * Arguments:
 *
 *      env - context for the book
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
    register BRI_BOOK_BLOCK *book = env->data.book;
    register BRI_PAGE_MAP_ENTRY *pme;
    register BR_UINT_32 index;
    register BR_UINT_8 *lenptr;
    BODS_STATIC_EXREF_BOOK **exref_booklist;
    BODS_STATIC_EXREF_DIR_HEADER    *exref_hdr;
    BR_UINT_8    *pme_buffer;

    if (book->vbh.oaf_version.minor_num < 2 || 
					book->vbh.static_exref_index_page == 0)
	return;
    if (book->static_exref_table != NULL)
	return;		/*  already read in  */

    /*  Read the page containing the exref table.  */
    pme = read_page( env, book->vbh.static_exref_index_page,
			BODS_C_STATIC_EXREF_PAGE); 
    pme_buffer = (pme)->buffer;
    exref_hdr = (BODS_STATIC_EXREF_DIR_HEADER *) BRI_DATA_ADDRESS(pme);

    /*  Allocate a buffer for the book list  */
    exref_booklist = BriCalloc( BODS_STATIC_EXREF_BOOK *, 
	(exref_hdr->num_books + 1), env);
    book->static_exref_booklist = exref_booklist;
    book->num_static_exref_books = exref_hdr->num_books;
    book->num_static_exrefs = exref_hdr->num_targets;
    book->static_exref_table = (BODS_STATIC_EXREF_TARGET *)
	&pme_buffer[exref_hdr->target_list_offset];
    exref_booklist[0] = NULL;

    lenptr = (BR_UINT_8 *)&pme_buffer[exref_hdr->book_list_offset];
    for (index = 1 ; index <= book->num_static_exref_books; index++ ) 
    {
	if (lenptr == NULL || *lenptr == 0)
	    break;
	exref_booklist[index] = (BODS_STATIC_EXREF_BOOK *) lenptr;
	lenptr += *lenptr;  /*  next book  */
    }

    return;
} /* end BriBookGetStaticExrefTable  */


void
BriBookGetChunkIndex(env)
    register BRI_CONTEXT *env;

/*
 *
 * Function description:
 *
 *      Get the Chunk index for a book
 *
 * Arguments:
 *
 *      env - context for the book.
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      BriLongJmp() on error
 *
 */

{
    register BRI_BOOK_BLOCK *book = env->data.book;
    register BRI_PAGE_MAP_ENTRY *pme;
    register BR_UINT_32 size;

    pme = read_page(env,book->vbh.chunk_index_page,BODS_C_CHUNK_INDEX);

    size = pme->tag.rec_length - pme->index;
    book->chunk_index = (BMD_OBJECT_ID *)BriMalloc(size,env);

    BRI_GET_VAR_ARRAY(env,pme,book->chunk_index,size);

} /* end BriBookGetChunkIndex */

/* end bri.c */


