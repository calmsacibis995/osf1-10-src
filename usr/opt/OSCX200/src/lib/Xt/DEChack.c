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
 * This file contains DEC_EXTENSIONS that can compile separatly from
 * the parts that they enhance or use. This is in an effort to isolate 
 * changes made by DEC for easier migration to the next release.
 */
#include "IntrinsicI.h"

#ifdef DEC_EXTENSION
/* This routine is provided to give Xm a way to register destroy callbacks
 * that are invoked when a display is closed.  These callbacks are needed
 * so that Xm can clean up various gc and pixmap caches ad other per display
 * items.
 */
void _XtAddPDDestroyCallback (display, widget, cbproc, closure)
    Display *display;
    Widget widget;
    XtCallbackProc cbproc;
    XtPointer closure;
{
    XtPerDisplay pd = _XtGetPerDisplay(display);

    _XtAddCallback (&pd->destroy_callbacks,cbproc, closure);
}
#endif


#ifdef DEC_EXTENSION

#include "I18N.h"

/*  Global declarations
*/

I18nContext  _I18nGlobalContextBlock = NULL;

#ifdef VMS
#include <lnmdef.h>
#include <ssdef.h>
#include <psldef.h>

/* Provide the context for the VMS logical names that will be redefined.
**
** NOTE: do NOT add SYS$SHARE to this list.  As of VMS V5.4 at least, this
** will cause any images which are dynamically activated after this to use
** a different form of the image file specification than the one which is
** in the system's INSTALL'd list.  This causes activation of any protected
** (privileged) images to fail (such as nmail and debug).
**	Jim V. (who did this, then spent 2 days trying to figure out
**		how he'd broken it!)
*/
#define num_VMS_paths 7
static char	*VMS_paths[num_VMS_paths+1] = { 
					"SYS$LIBRARY",
					"SYS$MESSAGE",
					"SYS$HELP",
					"SYS$EXAMPLES",
					"DECW$SYSTEM_DEFAULTS",
					"CDA$LIBRARY",
					"VUE$LIBRARY"
					};
#define VMS_lnmtable "LNM$PROCESS"
#ifdef XNL_DEBUG
#define VMS_acmode PSL$C_SUPER
#else
#define VMS_acmode PSL$C_USER
#endif /* XNL_DEBUG */

int xnl_createpath();
int xnl_trnlnm();
int xnl_crelnm();

#endif  /* VMS */


/* This is a global place used to remember the last language that was set.
** It is the common pointer shared by the set and get routines.
*/
static char *last_language = NULL;


/*
 *  xnl_setlanguage sets user-mode process logical names for the 
 *  i18n search lists.
 */
void xnl_setlanguage(language)
	String language;
{
    int language_len;

    if (language != NULL) {
#ifdef VMS
        int status, i;
	for (i=0; i<num_VMS_paths; i++) {
	    status = xnl_createpath(VMS_paths[i], language);
	}
#endif  /* VMS */
	if (last_language != NULL) free(last_language);
	language_len = strlen(language) + 1;
	last_language = (char*) malloc((unsigned) language_len);
	bcopy(language, last_language, language_len);
    }
    /*
     *  Create an I18N context block on the first call to this routine
     */
    if (_I18nGlobalContextBlock == NULL)
    {
        _I18nGlobalContextBlock = (I18nContext)XtMalloc 
(sizeof(I18nContextRec));
        _I18nGlobalContextBlock->locale = XtNewString(language);
        _I18nGlobalContextBlock->use_mrm_hierarchy = True;
        _I18nGlobalContextBlock->mrm_hierarchy_id = NULL;
        _I18nGlobalContextBlock->widget_class = NULL;
    }
}

 
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      xnl_getlanguage - returns the last saved language specification.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**      The global pointer "last_language"
**
**  FUNCTION VALUE:
**
**      The language that was set is returned.  This may be null if
**	no language has been successfully set.
**
**  SIDE EFFECTS:
**
**	None
**
**--
**/

char *xnl_getlanguage()
{
	return(last_language);
}


#ifdef VMS
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	xnl_createpath - create a language specific variant of a
**	    logical name defined search path.
**
**  FORMAL PARAMETERS:
**
**	path - the logical name to create a language specific
**	    variant of.
**	language - the variant to create.
**
**  IMPLICIT INPUTS:
**
**	Existing search list elements from the logical name are 
**	propagated to the new logical name.
**
**  FUNCTION VALUE:
**
**	1 is returned if success, 0 otherwise.
**
**  SIDE EFFECTS:
**
**	The new logical name is created.
**
**--
**/

int xnl_createpath(path, language)
    char *path;
    char *language;
{
	int status, i, trnsl_num;
	char *new_path;
	char *trnsl_table[129];
	int path_len, language_len;


	/* Allocate enough memory to build a new logical name specification 
	** and then build the logical name string.
	*/
	path_len = strlen(path);
	language_len = strlen(language);
	new_path = XtMalloc(path_len + language_len + 3);

	if (new_path == 0) {
#ifdef XNL_DEBUG
	    printf("Error: could not allocate memory\n");
#endif /* XNL_DEBUG */
	    return(0);
	}

	bcopy( path, new_path, path_len);
	new_path[path_len] =  '_';
	bcopy( language, &new_path[path_len+1], language_len);
	new_path[path_len+1+language_len] =  ':';
	new_path[path_len+1+language_len+1] =  '\0';
	trnsl_table[0] = new_path;


	/* Get the old values of the general purpose search list.  If this 
	** fails, then something is really wrong since these logical names 
	** should always be defined.
	*/
	status = xnl_trnlnm(path,
		          "LNM$FILE_DEV",
		          &trnsl_num,
		          &trnsl_table[1],
		          PSL$C_USER);

	if (status != 1) {
	    XtFree(new_path);
#ifdef XNL_DEBUG
	    printf("Error: logical name %s does not exist\n",path);
#endif /* XNL_DEBUG */
	    return(0);
	}


	/* Everything looks good.  Define the logical.
	*/
	trnsl_num = trnsl_num + 1;

	status = xnl_crelnm(path,
			     VMS_lnmtable,
			     trnsl_table,
			     trnsl_num,
			     VMS_acmode);


	/* We're done.  Free up our memory and get out.
	*/
	for (i=0; i<trnsl_num; i++) XtFree(trnsl_table[i]);
	return(status);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      xnl_trnlnm - translates a logical name and returns its values.
**
**  FORMAL PARAMETERS:
**
**	lognam - name to translate 
**	tabnam - logical name table to look in
**	trnsl_num (returned) - num of translations found 
**	trnsl_table (returned) - table of pointers to translations found.
**		These pointers need to be deallocated by the caller.
**	access_mode - access mode to look in
**
**  IMPLICIT INPUTS:
**
**      The existing logical name.
**
**  FUNCTION VALUE:
**
**	If successful, 1.  Otherwise, 0.
**
**  SIDE EFFECTS:
**
**	None.
**
**--
**/

int xnl_trnlnm(lognam, tabnam, trnsl_num, trnsl_table, access_mode)
    char *lognam;
    char *tabnam;
    int	*trnsl_num;
    char *trnsl_table[];
    unsigned char access_mode;
{
	typedef struct {
	    unsigned short buf_len;
	    unsigned short item_code;
	    unsigned char *buf_add;
	    unsigned long ret_add;
	} item_desc;

	item_desc item_list[3];
	unsigned long attr, index;
	unsigned short ret_len;
	struct dsc$descriptor_s	lognam_dsc, tabnam_dsc;
	char temp_buf[255];
	int status;


	/* Set up the inputs for the translation.  We can do this once 
	** because everything that changes is passed by ref.
	*/
	lognam_dsc.dsc$a_pointer = lognam;
	lognam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	lognam_dsc.dsc$w_length  = strlen(lognam);
	lognam_dsc.dsc$b_class   = DSC$K_CLASS_S;

	tabnam_dsc.dsc$a_pointer = tabnam;
	tabnam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	tabnam_dsc.dsc$w_length  = strlen(tabnam);
	tabnam_dsc.dsc$b_class   = DSC$K_CLASS_S;

	attr = LNM$M_CASE_BLIND;
	index = 0;

	item_list[0].buf_len   = 4;
	item_list[0].buf_add   = &index;
	item_list[0].item_code = LNM$_INDEX;
	item_list[0].ret_add   = &ret_len;

	item_list[1].buf_len   = 255;
	item_list[1].buf_add   = temp_buf;
	item_list[1].item_code = LNM$_STRING;
	item_list[1].ret_add   = &ret_len;

	item_list[2].buf_len   = 0;
	item_list[2].buf_add   = 0;
	item_list[2].item_code = 0;
	item_list[2].ret_add   = 0;


	/* Do as many translations as are required to get all the existing
	** values for the logical name.  For each value, allocate enough 
	** memory to save a copy of it and then put a pointer to it into 
	** our return table.
	*/
	do {
	    status = SYS$TRNLNM(&attr,
			    &tabnam_dsc,
			    &lognam_dsc,
			    &access_mode,
			    &item_list);

	    if ( ((status & SS$_NORMAL) == SS$_NORMAL) && (ret_len != 0) ) {
		trnsl_table[index] = XtMalloc( ret_len + 1);
		bcopy(item_list[1].buf_add, trnsl_table[index], ret_len);
		strcpy(trnsl_table[index] + ret_len, "");
		index++;
	    }
	}
	while ( ((status & SS$_NORMAL) == SS$_NORMAL) && (ret_len != 0) );


	/* Return the number of translations found and a normalized status.
	*/
	*trnsl_num = index;
	if ((status & SS$_NORMAL) == SS$_NORMAL) {
	    return (1);
	}
	else return (0);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      xnl_crelnm - creates a logical name 
**	
**  FORMAL PARAMETERS:
**
**	lognam - name to create
**	tabnam - logical name table to create it in
**	trnsl_num - num of translations to be provided
**	trnsl_table - table of translations to provide
**	access_mode - access mode to look in
**
**  IMPLICIT INPUTS:
**
**      The existing logical name.
**
**  FUNCTION VALUE:
**
**	If successful, 1.  Otherwise, 0.
**
**  SIDE EFFECTS:
**
**	The logical name is created.
**
**--
**/

int xnl_crelnm(lognam, tabnam, trnsl_table, trnsl_num, access_mode)
    char *lognam;
    char *tabnam;
    char *trnsl_table[];
    int trnsl_num;
    unsigned char access_mode;
{
	typedef struct {
	    unsigned short buf_len;
	    unsigned short item_code;
	    unsigned long buf_add;
	    unsigned long ret_add;
	} item_desc;

	struct dsc$descriptor_s	lognam_dsc, tabnam_dsc;
	item_desc item_list[129];
	unsigned long attr, index;
	int status;


	/* Set up the inputs for the creation.  
	*/
	attr = 0;

	lognam_dsc.dsc$a_pointer = lognam;
	lognam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	lognam_dsc.dsc$w_length  = strlen(lognam);
	lognam_dsc.dsc$b_class   = DSC$K_CLASS_S;

	tabnam_dsc.dsc$a_pointer = tabnam;
	tabnam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	tabnam_dsc.dsc$w_length  = strlen(tabnam);
	tabnam_dsc.dsc$b_class   = DSC$K_CLASS_S;

	for (index=0; index < trnsl_num; index++) {
	    item_list[index].buf_add   = trnsl_table[index];
	    item_list[index].buf_len   = strlen(trnsl_table[index]);
	    item_list[index].item_code = LNM$_STRING;
	}

	item_list[trnsl_num].buf_add   = 0;
	item_list[trnsl_num].buf_len   = 0;
	item_list[trnsl_num].item_code = 0;


	/* Create the logical name
	*/
	if (access_mode == PSL$C_SUPER) {
	    status = LIB$SET_LOGICAL(&lognam_dsc,
			   0,
		           &tabnam_dsc,
		           0,
		           &item_list);
	}
	else {
	    status = SYS$CRELNM(&attr,
		           &tabnam_dsc,
		           &lognam_dsc,
		           &access_mode,
		           &item_list);
	}


	/* Return a normalized status.
	*/
	if ((status != SS$_NORMAL) && (status != SS$_SUPERSEDE)) {
#ifdef XNL_DEBUG
	    printf("Error: creation of %s in %s failed\n", lognam, tabnam);
#endif /* XNL_DEBUG */
	    return (0);
	}
	else return (1);
}
#endif /* VMS */
#endif /* DEC_EXTENSION */
