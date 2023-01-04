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
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_DIR.C*/
/* *8    19-JUN-1992 20:16:13 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *7     9-JUN-1992 10:01:52 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *6     3-MAR-1992 17:10:23 KARDON "UCXed"*/
/* *5     5-FEB-1992 10:37:16 FITZELL "fixes accvio when no directories found"*/
/* *4     1-NOV-1991 12:51:17 BALLENGER "Reintegrate memex support"*/
/* *3    20-SEP-1991 17:03:04 BALLENGER "Fix problems with multiple calls to bri_directory_open()."*/
/* *2    17-SEP-1991 21:09:03 BALLENGER "change use of major and minor in version number to avoid conflict with sys/types.h*/
/*definition on ULTRIX"*/
/* *1    16-SEP-1991 12:43:59 PARMENTER "Public directory management routines"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_DIR.C*/
#ifndef VMS
 /*
#else
# module BRI_DIR "V03-0002"
#endif
#ifndef VMS
  */
#endif

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
**  FACILITY:
**
**   BRI -- Book Reader Interface
**
** ABSTRACT:
**
**   This module implements the public directory management routines.
**
** FUNCTIONS:
**
**  	bri_directory_open
**  	bri_directory_find
**  	bri_directory_entry
**  	bri_directory_name
**  	bri_directory_next
**  	bri_get_object_type
**
** AUTHORS:
**
**   Joseph M. Kraetsch
**
** CREATION DATE: 30-SEP-1987
**
** MODIFICATION HISTORY:
**
**  V03-0002    DLB0001     David L Ballenger           14-Feb-1991
**              Handle null directory entry titles when reading the directory
**              entries from the file.  This is now done in GetDirEntry in bri.c
**
**  V03-0001    DLB0001     David L Ballenger           06-Feb-1991
**              Add external declaration of FtextToAscii() and add SVN
**              fixes.
**
**  V03-0000	JAF0000	    James A. Ferguson	    	16-Aug-1990 
**  	      	Create new module, modify revision history format and
**  	    	update copyright.
**
**  	    	DLB 	    David L Ballenger	    	14-Sep-1989 
**             	Integrate Bookreader V2 changes.
**
**  	    	DLB 	    David L Ballenger	    	30-May-1990 
**             Cleanup (i.e. remove most contionaliztaion) include
**             files for new VMS standards.
**
*/

/*  
**  INCLUDE FILES  
*/
#include "bri_private_def.h"
#include <ctype.h>

/*
**  EXTERNAL ROUTINES
*/

/*
**  bri_directory_open  -- open a directory.
**
**	The book must be open.
**
**  Returns:  Directory id
*/
BMD_OBJECT_ID 
bri_directory_open PARAM_NAMES((bkid,drid,flags,entry_count))
    BMD_BOOK_ID bkid PARAM_SEP	       /*  Book id (from bri_book_open) */
    BMD_OBJECT_ID drid PARAM_SEP       /*  Directory name */
    BR_UINT_32 *flags PARAM_SEP	       /*  Directory flags  */
    BR_UINT_32  *entry_count PARAM_END   /*  number of entries */

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BRI_DIRECTORY_BLOCK *drb;	/* Directory block */
    BR_UINT_32 drb_index;

    if (setjmp(env->jump_buffer) != 0) {
        BriError(env,BriErrDirOpenNum);
	BriFree(drb->entry_list,env);
	drb->entry_list = NULL;
	return 0 ;
    }

    /* Get the drb index from the object id and validate it
     */
    drb_index = (drid & 0xFF000000) >> 24;
    if (env->data.book->vbh.version.major_num < 3)
	drb_index -= BODS_C_FIRST_BOOK_DIR_NUM;
    else
	drb_index -= env->data.book->vbh.n_sys_directories;
    BriValidateDrbIndex(env,drb_index);

    drb = (BRI_DIRECTORY_BLOCK *) env->data.book->drb_list[drb_index];

    /* fill in return parameters if requested 
     */
    if (flags)
	*flags = (BR_UINT_32) drb->vdh.flags.all_flags;

    if (drb->entry_list == NULL) {
        BriReadDirectoryPage(env,drb);
    }

    if (entry_count)
	*entry_count = drb->vdh.n_entries;

    return (BMD_OBJECT_ID)drb->directory_object_id;

};	/*  end of bri_directory_open  */


/*
**  bri_directory_find  -- Finds the id of a directory given its name.
**
**	The book must be open.
**
**  Returns:  Directory id
*/
BMD_OBJECT_ID 
bri_directory_find PARAM_NAMES((bkid,dir_name))
    BMD_BOOK_ID bkid PARAM_SEP	    /*  Book id (from bri_book_open) */
    char *dir_name PARAM_END	    /*  Directory name */

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BRI_DIRECTORY_BLOCK *drb;	/* Directory block */
    BR_UINT_32 dir_cnt;

    if (setjmp(env->jump_buffer) != 0) {
        BriError(env,BriErrDirOpenNum);
        return 0 ;
    }

    /* Search the directory list for the named directory
     */
    dir_cnt = 0;
    while (dir_cnt < env->data.book->vbh.n_directories) {

    	drb = env->data.book->drb_list[dir_cnt];
	if (strcmp(drb->vdh.directory_name,dir_name) == 0) {
    	    return (BMD_OBJECT_ID)drb->directory_object_id;
    	}
    	dir_cnt++;
    }

    /*  Directory name not found!
     */
    BriLongJmp(env,BriErrInvalidDridNum);

};	/*  end of bri_directory_open  */


/*
**  bri_directory_entry -- Get an entry from a directory
*/
BMD_OBJECT_ID
bri_directory_entry PARAM_NAMES((bkid,drid,entry_num,num_targets,
                                 target,level,width,height,
                                 data_addr,data_len,data_type,title 
                                 ))

    BMD_BOOK_ID bkid PARAM_SEP
    BMD_OBJECT_ID drid PARAM_SEP         /*  Directory id              */
    BR_UINT_32 entry_num PARAM_SEP	 /*  directory entry number	  */
    BR_UINT_32 *num_targets PARAM_SEP      /*  number of targets         */
    BMD_OBJECT_ID *target PARAM_SEP      /*  list of target chunks	  */
    BR_UINT_32 *level PARAM_SEP	         /*  Header level (top level = 1)  */
    BR_UINT_32 *width PARAM_SEP	         /*  total width of directory entry  */
    BR_UINT_32 *height PARAM_SEP           /*  total height of entry  */
    BMD_GENERIC_PTR *data_addr PARAM_SEP
    BR_UINT_32    *data_len PARAM_SEP      /*  length of display data	   */
    BR_UINT_32    *data_type PARAM_SEP     /*  type of display data	   */
    char	    **title PARAM_END    /*  title of entry (ascii)	   */

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BRI_DIRECTORY_BLOCK   *drb; 	/*  Directory control block */
    BODS_DIRECTORY_ENTRY *dre;   /*  dir. entry block  */
    BR_UINT_32 drb_index;

    if (setjmp(env->jump_buffer) != 0) {
        BriError(env,BriErrDirOpenNum);
        return 0 ;
    }

    /* Get the drb index from the object id and validate it
     */
    drb_index = (drid & 0xFF000000) >> 24;
    if (env->data.book->vbh.version.major_num < 3)
	drb_index -= BODS_C_FIRST_BOOK_DIR_NUM;
    else
	drb_index -= env->data.book->vbh.n_sys_directories;
    BriValidateDrbIndex(env,drb_index);

    drb = (BRI_DIRECTORY_BLOCK *) env->data.book->drb_list[drb_index];

    dre = &drb->entry_list[entry_num];
    if (level)
	*level = dre->header_level;
    if (width)
	*width = dre->width;
    if (height)
	*height = dre->height;
    if (num_targets)
	*num_targets = dre->n_targets;
    if (target) 
	*target = ((dre->n_targets > 0) ? ((BMD_OBJECT_ID)dre->target_list[0])
                                        : ((BMD_OBJECT_ID)NULL) );
    if (data_len)
	*data_len = dre->data_length;
    if (data_addr)
	*data_addr = (BMD_GENERIC_PTR)dre->data;
    if (data_type)
	*data_type = 2;	/*** ==CHUNK_FTEXT  should be dre->DRE$W_DATA_TYPE */
    if (title)
        *title = dre->title;

    return BriBuildDirectoryEntryId(drb,entry_num);

};	/*  end of bri_directory_entry  */


char *
bri_directory_name PARAM_NAMES((bkid,drid))
    BMD_BOOK_ID bkid PARAM_SEP
    BMD_OBJECT_ID drid PARAM_END
/*
 *
 * Function description:
 *
 *      Return a pointer to the name of the specified directory.
 *
 * Arguments:
 *
 *      bkid - id of the open book.
 *
 *      drid - Directory id specifying the directory.
 *
 * Return value:
 *
 *      Pointer to the character string containing the name of the
 *      directory
 *
 * Side effects:
 *
 *      None
 *
 */
{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BRI_DIRECTORY_BLOCK *drb;	    	    /*  Directory block */
    BR_UINT_32 dir_cnt;
    char *uilstring;
    BR_INT_32  i;

    if (setjmp(env->jump_buffer) != 0) {
        BriError(env,BriErrDirOpenNum);
        return 0 ;
    }

    /* Search the directory list for the object id
     */
    dir_cnt = 0;
    while (dir_cnt < env->data.book->vbh.n_directories) {

    	drb = env->data.book->drb_list[dir_cnt];
	if (drb->directory_object_id == drid) {
	    /* if the for loop isn't broken no printable characters where found */
	    for(i = 0; i < drb->vdh.name_len; i++) {
		if(isgraph(drb->vdh.directory_name[i]))
		    break;
	    }    
	    if((drb->vdh.directory_name == NULL) 
		|| (drb->vdh.directory_name[0] ==  0)
		|| (i == drb->vdh.name_len)) 
		{
		    uilstring = (char *)replace_blank_entry( env);
		    strcpy( drb->vdh.directory_name, uilstring);
		    BriFree( uilstring, env);
	    }
    	    return drb->vdh.directory_name;
    	}
    	dir_cnt++;
    }

    /*  Directory not found!
     */
    BriLongJmp(env,BriErrInvalidDridNum);

} /* end bri_directory_name */


BMD_OBJECT_ID
bri_directory_next PARAM_NAMES((bkid,drid))
    BMD_BOOK_ID bkid PARAM_SEP
    BMD_OBJECT_ID drid PARAM_END

/*
 *
 * Function description:
 *
 *      Get the id of the next directory in a book.
 *
 * Arguments:
 *
 *      bkid - specifies the book
 *
 *      drid - specifies the current directory
 *
 * Return value:
 *
 *      Directory id of the next directory.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BRI_DIRECTORY_BLOCK *drb;
    BR_UINT_32 drb_index;

    /* If the directory ID is 0 return the id of the first
     * directory in the directory list for the book or 0 if there
     * are no directories in the list.
     */

    if (setjmp(env->jump_buffer) != 0) {
        BriError(env,BriErrDirOpenNum);
        return 0 ;
    }


    if (env->data.book->drb_list == NULL)
    	return 0;
    drb = env->data.book->drb_list[0];
    if (drid == 0) {
        if (drb == NULL) {
            return 0 ;
        } else {
            return (BMD_OBJECT_ID)drb->directory_object_id;
        }
    }

    /* Get the index from the object id passed and validate it
     */
    drb_index = (drid & 0xFF000000) >> 24;
    if (env->data.book->vbh.version.major_num < 3)
	drb_index -= BODS_C_FIRST_BOOK_DIR_NUM;
    else
	drb_index -= env->data.book->vbh.n_sys_directories;
    BriValidateDrbIndex(env,drb_index);

    /* see if the next index value is valid.  (can't use
     * BriValidateDrbIndex because it generates errors for invalid ids)
     */
    drb_index++;
    if (drb_index >= env->data.book->vbh.n_directories) 
    	return 0 ;	/*  no more directories  */

    /* Return the id of next directory
     */
    return env->data.book->drb_list[drb_index]->directory_object_id;

} /* end bri_directory_next */


BMD_OBJECT_TYPE
bri_get_object_type PARAM_NAMES((bkid,object_id))
    BMD_BOOK_ID bkid PARAM_SEP
    BMD_OBJECT_ID object_id PARAM_END

/*
 *
 * Function description:
 *
 *      Determines the type of the object_id passed in.
 *
 * Arguments:
 *
 *      bkid - specifies the book
 *
 *      object_id - id of the object to check.
 *
 * Return value:
 *
 *      type of the object passed.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    BRI_CONTEXT *env = (BRI_CONTEXT *)bkid;
    BMD_OBJECT_ID   dir_id;
    BR_UINT_32	    entry_num;
    BR_UINT_32	    dir_num;
    BR_UINT_32	    num_sys_dirs;
    	
    /* Valid directory id 
     */
    dir_id = BRI_DIRECTORY_ID(object_id);
    entry_num = BRI_DIRECTORY_ENTRY_NUMBER(object_id);
    dir_num = dir_id >> 24;

    /*  is it a system directory?  */
    num_sys_dirs = env->data.book->vbh.version.major_num < 3 ?
	BODS_C_FIRST_BOOK_DIR_NUM : env->data.book->vbh.n_sys_directories ;
    if ( dir_num < num_sys_dirs )
    {
	if (entry_num == 0)
	    /*  system directories are not valid objects  */
	    return BMD_C_NO_OBJECT_TYPE;

	switch (dir_num)
	{
	    case BODS_C_CHUNK_DIR_NUM:
		return BMD_C_CHUNK_OBJECT;
	    case BODS_C_TOPIC_DIR_NUM:
		return BMD_C_TOPIC_OBJECT;
	    case BODS_C_SYMBOL_EXREF_DIR_NUM:
		return BMD_C_SYMBOL_EXREF_OBJECT;
	    case BODS_C_STATIC_EXREF_DIR_NUM:
		return BMD_C_STATIC_EXREF_OBJECT;
	    default:
		/*  if we get here, it's a system directory that
		 *  the current format level does not know about
		 */
		return BMD_C_NO_OBJECT_TYPE;
	}
    }

    /*  regular book directory  */
    return entry_num != 0 ? BMD_C_DIRECTORY_ENTRY_OBJECT : 
	BMD_C_DIRECTORY_OBJECT;

}   /* end bri_get_object_type */


