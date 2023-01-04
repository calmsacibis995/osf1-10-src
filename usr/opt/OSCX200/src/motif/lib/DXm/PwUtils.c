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
/************************************************************************/
/*									*/
/*	Copyright (c) Digital Equipment Corporation, 1990  		*/
/*	All Rights Reserved.  Unpublished rights reserved		*/
/*	under the copyright laws of the United States.			*/
/*									*/
/*	The software contained on this media is proprietary		*/
/*	to and embodies the confidential technology of 			*/
/*	Digital Equipment Corporation.  Possession, use,		*/
/*	duplication or dissemination of the software and		*/
/*	media is authorized only pursuant to a valid written		*/
/*	license from Digital Equipment Corporation.			*/
/*									*/
/*	RESTRICTED RIGHTS LEGEND   Use, duplication, or 		*/
/*	disclosure by the U.S. Government is subject to			*/
/*	restrictions as set forth in Subparagraph (c)(1)(ii)		*/
/*	of DFARS 252.227-7013, or in FAR 52.227-19, as			*/
/*	applicable.							*/
/*									*/
/************************************************************************/
/************************************************************************/
/*									*/
/*   FACILITY:								*/
/*									*/
/*	  Print Widget							*/
/*									*/
/*   ABSTRACT:								*/
/*									*/
/*	Contains utility procedures which deal with			*/
/*									*/
/*		1. file type guessing.					*/
/*									*/
/*		2. system object names (e.g. queues, forms, logicals,	*/
/*		   etc.)   						*/
/*									*/
/*   AUTHORS:								*/
/*									*/
/*	Print Wgt Team							*/
/*									*/
/*   CREATION DATE:							*/
/*									*/
/*	Winter/Spring 1990						*/
/*									*/
/*   MODIFICATION HISTORY:						*/
/*									*/
/*	013	Will Walker		06-Feb-1991			*/
/*		Add "Default" print format.				*/
/*	012	Will Walker		21-Jan-1991			*/
/*		Use saberc to flush out lint errors.			*/
/*	011	Will Walker		20-Sep-1990			*/
/*		Add augment list capability.				*/
/*	010	Will Walker		12-Sep-1990			*/
/*	        Change XmStringCreate calls to DXmCvtOStoCS calls.	*/
/*	009	Will Walker		24-Jul-1990			*/
/*		Remove ULTRIX compilation warnings.			*/
/*	008	Will Walker		24-Jul-1990			*/
/*		Change <DXm/PwVMS.h> to "PwVMS.h"			*/
/*	007	Will Walker		20-Jul-1990			*/
/*		Make pw_xm_string_compare as sub for XmStringCompare.	*/
/*	006	Will Walker		16-Jul-1990			*/
/*		Add additional params for DXmCvt...			*/
/*	005	Will Walker		13-Jul-1990			*/
/*		I18N work.						*/
/*	004	Will Walker		06-Jul-1990			*/
/*		Put DXmCSText in interface instead of XmText.  Make	*/
/*		resources be XmString's instead of char *'s.		*/
/*	003	Will Walker		14-Jun-1990			*/
/*		NULL terminate file extension table.			*/
/*	002	Will Walker		27-Mar-1990			*/
/*		Reorganize.						*/
/*	001	Will Walker		19-Mar-1990			*/
/*		Add modification history.  Change names of resources to	*/
/*		match Xm naming conventions.				*/
/*									*/
/************************************************************************/
#define _PwUtils_c_


/************************************************************************/
/*					     				*/
/* INCLUDE FILES 			     				*/
/*					     				*/
/************************************************************************/
#include <stdio.h>
#ifdef VMS
#   include <ssdef.h>		/* VMS only system definitions */
#   include "PwVMS.h"
#else
#   include <string.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <sys/file.h>
#ifdef __osf__
#   include "filehdr.h"
#   include "aouthdr.h"
#   include "sex.h"
#else
#   include <sys/exec.h>
#endif
#   include <strings.h>
#endif

#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>
#include <Xm/BulletinBP.h>
#include <Xm/DialogSP.h>
#include <DXm/DXmPrintP.h>

/************************************************************************/
/*					     				*/
/* MACRO DEFINITIONS			     				*/
/*					     				*/
/************************************************************************/
#ifdef VMS

    typedef struct _pw_r_assumption_struct
    {
	char	*at_file_extension;
    	int	l_guesser_format_mapping;
    } pw_r_assumption_struct;

    static pw_r_assumption_struct pw_ar_VMS_assumption_table[] = 
    {
	/* VMS file	                       		Resulting default choice       	*/
	/* extension    Assumed data syntax    		in printwgt docprofile menu 	*/
	/* ---------    -------------------    		------------------------------ 	*/

	{ ".",		DXmPRINT_FORMAT_TEXT},		/* default if no match found */

	{ "TEXT",	DXmPRINT_FORMAT_TEXT},		/* Text */
	{ "TXT",	DXmPRINT_FORMAT_TEXT},
	{ "LIS",	DXmPRINT_FORMAT_TEXT},
	{ "RNO",	DXmPRINT_FORMAT_TEXT},
	{ "SDML",	DXmPRINT_FORMAT_TEXT},

	{ "MEM",	DXmPRINT_FORMAT_LINE_PRINTER},	/* Line */

	{ "ANSI2",	DXmPRINT_FORMAT_ANSI2},		/* ANSI_2 */
	{ "SIX",	DXmPRINT_FORMAT_ANSI2},
	{ "IMG",	DXmPRINT_FORMAT_ANSI2},

	{ "LN3",	DXmPRINT_FORMAT_ANSI},		/* ANSI */
	{ "LN03",	DXmPRINT_FORMAT_ANSI},
	{ "ANSI",	DXmPRINT_FORMAT_ANSI},

	{ "PS",		DXmPRINT_FORMAT_POSTSCRIPT},	/* Postscript */
	{ "POST",	DXmPRINT_FORMAT_POSTSCRIPT},

	{ "REG",	DXmPRINT_FORMAT_REGIS},		/* ReGIS */
	{ "REGIS",	DXmPRINT_FORMAT_REGIS},
	{ "PIC",	DXmPRINT_FORMAT_REGIS},
	{ "SLO",	DXmPRINT_FORMAT_REGIS},

	{ "TEK",	DXmPRINT_FORMAT_TEKTRONIX},	/* Tektronix */

	{ "DDIF",	DXmPRINT_FORMAT_DDIF},		/* DDIF */
        { NULL,NULL},				        /* Null terminated */
    };
#else
#   define BUFLEN		512
#endif


/************************************************************************/
/*								     	*/
/* FORWARD DECLARATIONS                      				*/
/*					     				*/
/************************************************************************/
Boolean		pw_convert_cs_to_time();
int		pw_get_cs_list_element_index();
void		pw_free_cs_list_ref_memory();
Boolean		pw_get_variable_value();
int		pw_parse_comma_string();
void		pw_copy_cs_list();
int		pw_xm_string_compare();
void		pw_add_cs_list_element();
void		pw_add_int_list_element();
void		pw_add_opaque_list_element();


#ifdef VMS
    int 		pw_get_print_form_names();
    int 		pw_get_print_queue_names();
    static void		pw_get_filename_extension();
    void		pw_get_guesser_format_mapping();
#else
    static int 		pw_get_prent();
    int 		pw_get_print_form_names();
    int 		pw_get_print_queue_names();
    void		pw_get_guesser_format_mapping();
#endif


/************************************************************************/
/*									*/
/* pw_convert_cs_to_time						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine converts a cs string to time.  This		*/
/*	routine will return FALSE if the time is invalid.		*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	ar_cs		String to be converted.				*/
/*									*/
/*	al_time		Converted Time.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE or FALSE							*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
Boolean pw_convert_cs_to_time(ar_cs,al_time)
    XmString		ar_cs;
    int			al_time[2];
{
#ifdef VMS
    Opaque		r_local_text = NULL;
    unsigned int	al_dsc[2];
    int			l_i;
    long		l_size,l_status;

    /********************************************************************/
    /*									*/
    /* Treat NULL as no time at all.					*/
    /*									*/
    /********************************************************************/
    if (!ar_cs)
    {
	al_time[0] = -1;
	al_time[1] = -1;
	return (TRUE);
    }
    
    /********************************************************************/
    /*									*/
    /* Convert CS to OS.						*/
    /*									*/
    /********************************************************************/
    r_local_text = (Opaque) DXmCvtCStoOS(ar_cs,&l_size,&l_status);
    
    /********************************************************************/
    /*									*/
    /* Treat NULL as no time at all.					*/
    /*									*/
    /********************************************************************/
    if ((!r_local_text) || (!strlen(r_local_text)))
    {
	al_time[0] = -1;
	al_time[1] = -1;
	return (TRUE);
    }

    /********************************************************************/
    /*									*/
    /* Convert to upper case and do a BINTIM.				*/
    /*									*/
    /********************************************************************/
    for (l_i = 0; l_i < strlen(r_local_text); l_i++)
	r_local_text[l_i] = PW_TOUPPER(r_local_text[l_i]);
	
    al_dsc[0] = strlen(r_local_text);
    al_dsc[1] = (unsigned int) r_local_text;
    
    l_status = SYS$BINTIM(al_dsc,al_time);

    XtFree(r_local_text);
       
    if ((l_status & 1) != 1)
	return(FALSE);
    else
	return(TRUE);
#else
    return(TRUE);
#endif
} /* pw_convert_cs_to_time */


/************************************************************************/
/*									*/
/* pw_get_cs_list_element_index						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine finds the index of an element in a cs list.	*/
/*	If the element is not found, a -1 is returned.			*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	cs_string	String to be found.				*/
/*									*/
/*	cs_list		List to be searched.				*/
/*									*/
/*      cs_count	Count of cs strings.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	Element index or (-1).						*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
int pw_get_cs_list_element_index(ar_cs,ar_cs_list,l_cs_count)
    XmString	ar_cs;
    XmString 	ar_cs_list[];
    int		l_cs_count;
{
    Boolean	c_found = FALSE;
    int		l_i;
    
    if (ar_cs_list)
	for (l_i = 0; (l_i < l_cs_count) && !c_found; l_i++)
	    c_found = pw_xm_string_compare(ar_cs_list[l_i],ar_cs);

    if (c_found)
	return(l_i - 1);
    else
	return(-1);

} /* pw_get_cs_list_element_index */


/************************************************************************/
/*									*/
/* pw_free_cs_list_ref_memory						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine frees up the memory pointed to by the entries in 	*/
/*	the array referenced to by the first argument.  It does NOT 	*/
/*	free up the array (assuming this memory is either static or is 	*/
/*	going to be reused.)  					        */
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	cs_list		Array to be freed.				*/
/*									*/
/*      cs_count	Count of cs strings.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
void pw_free_cs_list_ref_memory(ar_cs_list,l_cs_count)
    XmString 	ar_cs_list[];
    int	        l_cs_count;
{
    int	l_i;

    if ( (ar_cs_list != NULL) || l_cs_count )
	for (l_i=0; l_i < l_cs_count; l_i++)
	    if (ar_cs_list[l_i])
		XmStringFree(ar_cs_list[l_i]);

} /* pw_free_cs_list_ref_memory */


/************************************************************************/
/*									*/
/* pw_get_variable_value						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Determines and returns the value of a variable (logical on VMS	*/
/*	or environment variable on Ultrix).  If the variable doesn't	*/
/*	exist, the function returns FALSE.				*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	variable_name	Name of variable to find.			*/
/*									*/
/*      variable_value	ASCIZ representation of found variable.		*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE (variable found) FALSE (variable not found)		*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
Boolean pw_get_variable_value(ar_variable_name,aat_variable_value)
    XmString	ar_variable_name;
    char	**aat_variable_value;
{
    
#ifdef VMS
#define NO_TRANS 0x1bc
    int			l_count = 0;
    char		at_local_buffer[256];		/* local buffer to receive result in    */
    int			l_attribute = LNM$M_CASE_BLIND;	/* specifies case-blind translation	*/
    long		l_ret_status = TRUE;		/* status of routine calls              */
    int			al_lognam_desc[2];		/* descriptor to name to be translated. */
    int			al_tabnam_desc[2];		/* descriptor to table to be searched.  */
    r_itmbuf_struct	ar_itmlst[4];			/* a VMS itemlist structure.            */
    int			l_return_bits = 0;		/* information bits returned            */
    int			l_returned_string_length;	/* length of translated string          */
    char 		*at_variable_name;
    long		l_size,l_status;

    if (!ar_variable_name)
	return FALSE;

    at_variable_name = (char *) DXmCvtCStoOS(ar_variable_name,&l_size,&l_status);

    if (!at_variable_name || !strlen(at_variable_name))
	return FALSE;

    al_lognam_desc[0] = strlen(at_variable_name);
    al_lognam_desc[1] = (int) at_variable_name;

    al_tabnam_desc[0] = strlen("LNM$FILE_DEV");
    al_tabnam_desc[1] = (int) "LNM$FILE_DEV";

    /********************************************************************/
    /*									*/
    /* Ask for bits telling whether a terminal translation took place   */
    /*									*/
    /********************************************************************/
    ar_itmlst[0].at_bufadr    	= &l_return_bits;
    ar_itmlst[0].c_code      	= LNM$_ATTRIBUTES;
    ar_itmlst[0].c_buflen    	= 4L;
    ar_itmlst[0].al_retlenadr	= 0L;

    /********************************************************************/
    /*									*/
    /* Ask for length of resulting string.				*/
    /*									*/
    /********************************************************************/
    ar_itmlst[1].at_bufadr    	= &l_returned_string_length;
    ar_itmlst[1].c_code      	= LNM$_LENGTH;
    ar_itmlst[1].c_buflen    	= 4L;
    ar_itmlst[1].al_retlenadr	= 0L;

    /********************************************************************/
    /*									*/
    /* Specify buffer where returned string is delivered.               */
    /*									*/
    /********************************************************************/
    ar_itmlst[2].at_bufadr    	= &at_local_buffer;
    ar_itmlst[2].c_code      	= LNM$_STRING;
    ar_itmlst[2].c_buflen    	= 256L;
    ar_itmlst[2].al_retlenadr 	= 0L;

    /********************************************************************/
    /*									*/
    /* finally a null entry to mark end of list                         */
    /*									*/
    /********************************************************************/
    ar_itmlst[3].at_bufadr   	= 0;
    ar_itmlst[3].c_code      	= 0;
    ar_itmlst[3].c_buflen    	= 0;
    ar_itmlst[3].al_retlenadr 	= 0;

    /********************************************************************/
    /*									*/
    /* Go out and have a look.			                        */
    /*									*/
    /********************************************************************/
    l_ret_status = sys$trnlnm(&l_attribute,	/* pointer to attribute mask          */
			      &al_tabnam_desc,	/* addr of descriptor of table to sear*/
			      &al_lognam_desc,  /* addr of descriptor of name to trans*/
			      0,		/* no access mode specified           */
			      &ar_itmlst);	/* pointer to itemlist                */

    /********************************************************************/
    /*									*/
    /* If bad status, no translation found.	                        */
    /*									*/
    /********************************************************************/
    if (!(l_ret_status & 1))
    {
	XtFree(at_variable_name);
	return (FALSE);
    }
    
    /********************************************************************/
    /*									*/
    /* If no further translation possible, we're done.                  */
    /*									*/
    /********************************************************************/
    if ((l_return_bits & LNM$M_TERMINAL) != 0L)
    {
	at_local_buffer[l_returned_string_length] = '\0';
	*aat_variable_value = strcpy(XtMalloc(l_returned_string_length + 1),
				     &at_local_buffer);
	l_ret_status = TRUE;
    }
    /********************************************************************/
    /*									*/
    /* Otherwise keep going until the end is reached.   Each time, 	*/
    /* make the newly found string the new logical name to find.	*/
    /*									*/
    /********************************************************************/
    else
    {
	do
	{    
	    l_count += 1;

	    al_lognam_desc[0] = l_returned_string_length;
	    al_lognam_desc[1] = &at_local_buffer;
	
	    l_return_bits = 0;

	    l_ret_status = sys$trnlnm(&l_attribute,	/* pointer to attribute mask          */
				      &al_tabnam_desc,	/* addr of descriptor of table name   */
				      &al_lognam_desc,  /* addr of descriptor of name to trans*/
				      0,		/* no access mode specified           */
				      &ar_itmlst);  	/* pointer to itemlist                */
	}  while ((l_count < 10) & (l_ret_status & 1));


	/****************************************************************/
	/*								*/
	/* Confirm the loop was exited properly.			*/
	/*								*/
	/****************************************************************/
	if ((l_count < 10) && (l_ret_status == NO_TRANS) )
	{
	    at_local_buffer[l_returned_string_length] = '\0';
	    *aat_variable_value = strcpy(XtMalloc(l_returned_string_length + 1),
					 &at_local_buffer);
	    l_ret_status = TRUE;
	}
	else
	    l_ret_status = FALSE;
    }

    XtFree(at_variable_name);

    return (l_ret_status);
#else
    char *at_temp;
    char *at_variable_name;
    long l_size,l_status;

    if (!ar_variable_name)
	return FALSE;

    at_variable_name = (char *) DXmCvtCStoOS(ar_variable_name,&l_size,&l_status);

    if (!at_variable_name || !strlen(at_variable_name))
	return FALSE;
        
    at_temp = (char *) getenv(at_variable_name);

    XtFree(at_variable_name);
    
    if (at_temp)
    {
	*aat_variable_value = strcpy(XtMalloc(strlen(at_temp) + 1),at_temp);
	return(TRUE);
    }
    else
    {
	*aat_variable_value = NULL;
	return(FALSE);
    }    
#endif

} /* pw_get_variable_value */


/************************************************************************/
/*									*/
/* pw_parse_comma_string						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Converts an asciz comma string into a Comp String array.	*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	comma_string	String to be parsed.				*/
/*									*/
/*	cs_list		Compound String array (to be created)		*/
/*									*/
/*      cs_count	Number of elements in array (returned)		*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
int pw_parse_comma_string(at_comma_string,aar_cs_list,al_cs_count)
    char	*at_comma_string;
    XmString	*(aar_cs_list[]);
    int		*al_cs_count;
{
    XmString	*ar_cs_list = *aar_cs_list;
    char    	*at_name = NULL;        /* temporary string holder for strtok routine */
    int     	l_mem_allocation = 0;
    int		l_string_length;	/* length of string obtained.                 */
    long	l_size,l_status;

    /********************************************************************/
    /*									*/
    /* Check for NULL string or string with nothing in it.		*/
    /*									*/
    /********************************************************************/
    *al_cs_count = 0;

    if (at_comma_string == NULL)
	l_string_length = 0;
    else
	l_string_length = strlen(at_comma_string);

    if (l_string_length == 0)
	return TRUE;

    /********************************************************************/
    /*								  	*/
    /* Determine the memory allocation (in longwords) of the vector 	*/
    /* passed in.  (This list's memory will be reused via realloc 	*/
    /* calls.)								*/
    /*									*/
    /********************************************************************/
    l_mem_allocation = 
	((*al_cs_count + K_PW_CHUNK_INCR - 1) / 
	 K_PW_CHUNK_INCR) * 
	K_PW_CHUNK_INCR;

    /********************************************************************/
    /*									*/
    /* Get the first name						*/
    /*									*/
    /********************************************************************/
    at_name = strtok(at_comma_string," ,");

    if (at_name != NULL)		/* check for null before calling strlen */
	if (strlen(at_name) != 0)
	{
	    /************************************************************/
	    /*								*/
	    /* Convert this first q name to compound and add to 	*/
	    /* compound string list.  (Allocate new memory if 		*/
	    /* neccessary.)						*/
	    /*								*/
	    /************************************************************/
	    (*al_cs_count)++;
	    
	    if (*al_cs_count > l_mem_allocation)
	    {
		ar_cs_list = (XmString *) XtRealloc((char *)ar_cs_list,
						    sizeof(XmString) *
						    (l_mem_allocation + K_PW_CHUNK_INCR));
		l_mem_allocation +=  K_PW_CHUNK_INCR;
	    }

	    /************************************************************/
	    /*								*/
	    /* Add the new compound string queue name to the list.	*/
	    /*								*/
	    /************************************************************/
	    ar_cs_list[*al_cs_count - 1] = (XmString) DXmCvtOStoCS(at_name,&l_size,&l_status);

	    /************************************************************/
	    /*								*/
	    /* Loop through the remainder of the list adding the new 	*/
	    /* name to the caller's compound string list (growing that 	*/
	    /* list when necessary.				 	*/
	    /*								*/
	    /************************************************************/
	    while ((at_name = strtok(NULL," ,")) != NULL)
	    {
		/********************************************************/
		/*							*/
		/* Convert this first q name to compound and add to 	*/
		/* compound string list.  (Allocate new memory if 	*/
		/* neccessary.)						*/
		/*							*/
		/********************************************************/
		(*al_cs_count)++;
	    
		if (*al_cs_count > l_mem_allocation)
		{
		    ar_cs_list = (XmString *) XtRealloc((char *)ar_cs_list,
							sizeof(XmString) *
							(l_mem_allocation + K_PW_CHUNK_INCR));
		    l_mem_allocation +=  K_PW_CHUNK_INCR;
		}

		/********************************************************/
		/*							*/
		/* Add the new compound string queue name to the list.	*/
		/*							*/
		/********************************************************/
		ar_cs_list[*al_cs_count - 1] = (XmString) DXmCvtOStoCS(at_name,&l_size,&l_status);

	    } /* while */
	} /* non-null at_name */

    /********************************************************************/
    /*									*/
    /* Now point the passed structure to the malloc'ed memory.		*/
    /*									*/
    /********************************************************************/
    *aar_cs_list = ar_cs_list;

    return TRUE;
} /* pw_parse_comma_string */


/************************************************************************/
/*									*/
/* pw_copy_cs_list							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine copies a compound string list.			*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	cs_from_list	Array to be copied.				*/
/*									*/
/*	cs_to_list	Array to be created.				*/
/*									*/
/*      cs_count	Count of cs strings.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
void pw_copy_cs_list(ar_cs_from_list,aar_cs_to_list,al_cs_count)
    XmString 	ar_cs_from_list[];
    XmString 	*(aar_cs_to_list[]);
    int	        *al_cs_count;
{
    int		l_i;
    XmString	*ar_cs_to_list;
    
    /********************************************************************/
    /*									*/
    /* If there is no list, give them back nothing.			*/
    /*									*/
    /********************************************************************/
    if (!ar_cs_from_list)
    {
	*aar_cs_to_list = NULL;
	*al_cs_count = 0;
	return;
    }
    
    /********************************************************************/
    /*									*/
    /* If the count is less than 0, we will consider this to be a null	*/
    /* terminated list.  Find out how long the list is, then copy it.	*/
    /*									*/
    /********************************************************************/
    if (*al_cs_count < 0)
    {  
	*al_cs_count = 0;
	for (l_i = 0; ar_cs_from_list[l_i]; l_i++)
	    (*al_cs_count)++;
    }
    
    /********************************************************************/
    /*									*/
    /* If there is no list, give them back nothing.			*/
    /*									*/
    /********************************************************************/
    if (!*al_cs_count)
    {
	*aar_cs_to_list = NULL;
	return;
    }
    
    /********************************************************************/
    /*									*/
    /* Copy the list.							*/
    /*									*/
    /********************************************************************/
    ar_cs_to_list = (XmString *) XtMalloc(sizeof(XmString) * (*al_cs_count));
    
    for (l_i = 0; l_i < *al_cs_count; l_i++)
	ar_cs_to_list[l_i] = XmStringCopy(ar_cs_from_list[l_i]);
    
    *aar_cs_to_list = ar_cs_to_list;

} /* pw_copy_cs_list */


/************************************************************************/
/*									*/
/* pw_xm_string_compare							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This routine compares the DXmCvtCStoFC values of two compound	*/
/*	strings.  I returns true if they are the same.			*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	cs_a		String a.					*/
/*									*/
/*	cs_b		String b.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
int pw_xm_string_compare(ar_cs_a,ar_cs_b)
    XmString 	ar_cs_a;
    XmString 	ar_cs_b;
{
    long	l_size,l_status;
    Opaque	r_fc_a = NULL;
    Opaque	r_fc_b = NULL;

    /********************************************************************/
    /*									*/
    /*									*/
    /*									*/
    /********************************************************************/
    r_fc_a = (Opaque) DXmCvtCStoFC(ar_cs_a,&l_size,&l_status);
    r_fc_b = (Opaque) DXmCvtCStoFC(ar_cs_b,&l_size,&l_status);

    /********************************************************************/
    /*									*/
    /*	If the strings exist, perform a strmp.  Note that strcmp returns*/
    /* 	false if the strings are the same (we want to return true since */
    /* 	this is what XmStringCompare returns).  If both strings don't	*/
    /*	exist, return true.  Otherwise, return false.			*/
    /*									*/
    /********************************************************************/
    if (r_fc_a && r_fc_b)
    	l_status = !strcmp(r_fc_a,r_fc_b);
    else if (!r_fc_a &&	!r_fc_b)
	l_status = 1;
    else
	l_status = 0;

    /********************************************************************/
    /*									*/
    /*	Free the memory.						*/
    /*									*/
    /********************************************************************/
    if (r_fc_a)
	XtFree(r_fc_a);
	
    if (r_fc_b)
	XtFree(r_fc_b);

    /********************************************************************/
    /*									*/
    /*	Return the status.						*/
    /*									*/
    /********************************************************************/
    return(l_status);

} /* pw_xm_string_compare */    


/************************************************************************/
/*									*/
/* pw_add_cs_list_element						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Adds an element to a compound string list.			*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	cs_list		cs list.					*/
/*									*/
/*	count		number of elements in cs list.			*/
/*									*/
/*	cs_element	new element.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
void pw_add_cs_list_element(aar_cs_list,l_count,ar_cs_element)
    XmString 	*(aar_cs_list[]);
    int		l_count;
    XmString	ar_cs_element;
{
    int	       	l_i;
    XmString	*ar_cs_from_list = *aar_cs_list;
    XmString	*ar_cs_to_list;

    ar_cs_to_list = (XmString *) XtMalloc(sizeof(XmString) * (l_count + 1));
    
    for (l_i = 0; l_i < l_count; l_i++)
	ar_cs_to_list[l_i] = XmStringCopy(ar_cs_from_list[l_i]);

    ar_cs_to_list[l_count] = XmStringCopy(ar_cs_element);
    
    if (l_count)
    {
	pw_free_cs_list_ref_memory(ar_cs_from_list,l_count);
	XtFree((char *)ar_cs_from_list);
    }
    
    *aar_cs_list = ar_cs_to_list;

} /* pw_add_cs_list_element */


/************************************************************************/
/*									*/
/* pw_add_int_list_element						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Adds an element to a integer list.				*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	int_list       	integer list.					*/
/*									*/
/*	count		number of elements in int list.			*/
/*									*/
/*	int_element	new element.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
void pw_add_int_list_element(aal_int_list,l_count,l_int_element)
    int 	*(aal_int_list[]);
    int		l_count;
    int		l_int_element;
{
    int	       	l_i;
    int		*al_int_from_list = *aal_int_list;
    int		*al_int_to_list;

    al_int_to_list = (int *) XtMalloc(sizeof(int) * (l_count + 1));
    
    for (l_i = 0; l_i < l_count; l_i++)
	al_int_to_list[l_i] = al_int_from_list[l_i];

    al_int_to_list[l_count] = l_int_element;
    
    if (l_count)
	XtFree((char *)al_int_from_list);
    
    *aal_int_list = al_int_to_list;

} /* pw_add_int_list_element */


/************************************************************************/
/*									*/
/* pw_add_opaque_list_element						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Adds an element to an opaque list.				*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	opaque_list	opaque list.					*/
/*									*/
/*	count		number of elements in opaque list.		*/
/*									*/
/*	opaque_element	new element.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
void pw_add_opaque_list_element(aar_opaque_list,l_count,r_opaque_element)
    Opaque 	*(aar_opaque_list[]);
    int		l_count;
    Opaque	r_opaque_element;
{
    int	       	l_i;
    Opaque	*ar_opaque_from_list = *aar_opaque_list;
    Opaque	*ar_opaque_to_list;

    ar_opaque_to_list = (Opaque *) XtMalloc(sizeof(Opaque) * (l_count + 1));
    
    for (l_i = 0; l_i < l_count; l_i++)
	ar_opaque_to_list[l_i] = ar_opaque_from_list[l_i];

    ar_opaque_to_list[l_count] = r_opaque_element;
    
    if (l_count)
	XtFree((char *)ar_opaque_from_list);
    
    *aar_opaque_list = ar_opaque_to_list;

} /* pw_add_opaque_list_element */


#ifdef VMS
/************************************************************************/
/************************************************************************/
/************************************************************************/
/*								  	*/
/*				VMS SPECIFIC				*/
/*									*/
/************************************************************************/
/************************************************************************/
/************************************************************************/


/************************************************************************/
/*									*/
/* pw_get_print_queue_names (VMS)					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Determines and returns the names of the print queues on the 	*/
/*	current system and the number of such names found.              */
/*                                                                      */
/*	Queue names that start with NM$ are omitted from list.          */
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	cs_queues	Array to be created.				*/
/*									*/
/*      queue_count	Count of queues.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	Returns error status codes returned by $GETQUIW system service  */
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
int pw_get_print_queue_names(aar_cs_queues,al_queue_count)
    XmString 	*(aar_cs_queues[]);
    int 	*al_queue_count;
{
    int			l_search_flags = QUI$M_SEARCH_SYMBIONT | QUI$M_SEARCH_WILDCARD;
    r_iosb_struct	r_iosb;    
    int			l_ret_status;
    short		l_ret_length = 0;
    char		at_buffer[K_PW_MAX_QUEUE_NAME_LENGTH+2];
    int			l_mem_allocation = 0;
    int			l_count = 0;
    r_itmbuf_struct 	ar_itmlst[] = 
                        {
			    { 0L, 0L, 0L, 0L },
			    { 0L, 0L, 0L, 0L },
			    { 0L, 0L, 0L, 0L },
			    { 0L, 0L, 0L, 0L }
			};

    XmString		*ar_cs_queues = *aar_cs_queues;

    long	l_size,l_status;

    /********************************************************************/
    /*								  	*/
    /* If this is an 'uninitialized list', the counter will be at -1. 	*/
    /* Bump it to 0 (which it should be if no queues are found) before 	*/
    /* calculating the allocation. 					*/
    /*									*/
    /********************************************************************/
    if (*al_queue_count < 0)
	*al_queue_count = 0;

    /********************************************************************/
    /*								  	*/
    /* Determine the memory allocation (in longwords) of the vector 	*/
    /* passed in.  (This list's memory will be reused via realloc 	*/
    /* calls.)								*/
    /*									*/
    /********************************************************************/
    l_mem_allocation = 
	((*al_queue_count + K_PW_CHUNK_INCR - 1) / 
	 K_PW_CHUNK_INCR) * 
	K_PW_CHUNK_INCR;
    
    /********************************************************************/
    /*									*/
    /* Free up the memory referenced by the pointers in the array (but	*/
    /* not the array itself) and reset the count. 			*/
    /*									*/
    /********************************************************************/
    pw_free_cs_list_ref_memory(ar_cs_queues,*al_queue_count);
    *al_queue_count = 0;
    
    /********************************************************************/
    /*									*/
    /* Dissolve any internal search context block for the process	*/
    /*									*/
    /********************************************************************/
    r_iosb.l_dummy = 0L;
    l_ret_status = sys$getquiw(NULL,
			       QUI$_CANCEL_OPERATION,
			       NULL,
			       &ar_itmlst,
			       &r_iosb,
			       NULL,
			       NULL);

    /********************************************************************/
    /*									*/
    /* Set up itmlst entries to do repeated searches for queue names	*/
    /*									*/
    /********************************************************************/
    ar_itmlst[0].at_bufadr    = "*";	/* Wild-carded name */
    ar_itmlst[0].c_code       = QUI$_SEARCH_NAME;
    ar_itmlst[0].c_buflen     = 1L;
    ar_itmlst[0].al_retlenadr = 0L;

    ar_itmlst[1].at_bufadr    = &l_search_flags;
    ar_itmlst[1].c_code       = QUI$_SEARCH_FLAGS;
    ar_itmlst[1].c_buflen     = 4L;
    ar_itmlst[1].al_retlenadr = 0L;

    ar_itmlst[2].at_bufadr    = &at_buffer;
    ar_itmlst[2].c_code       = QUI$_QUEUE_NAME;
    ar_itmlst[2].c_buflen     = K_PW_MAX_QUEUE_NAME_LENGTH;
    ar_itmlst[2].al_retlenadr = &l_ret_length;

    /********************************************************************/
    /*									*/
    /* Loop through the system queues, adding appropriate queue names 	*/
    /* to the caller's q names compound string list.			*/
    /*									*/
    /********************************************************************/
    r_iosb.l_stat = 1;
    l_ret_status = SS$_NORMAL;

    while ((l_ret_status == SS$_NORMAL) && 
	   ((r_iosb.l_stat & 1) == 1))
    {
	/****************************************************************/
	/*								*/
	/* Try to get a new queue name                                  */
	/*								*/
	/****************************************************************/
	r_iosb.l_dummy = 0L;
	l_ret_status = sys$getquiw(NULL,		/* event flag number */
				   QUI$_DISPLAY_QUEUE,	/* function code */
				   NULL,	        /* not used in this service */
				   &ar_itmlst,		/* item list */
				   &r_iosb,		/* i/o status block */
				   NULL,		/* AST address */
				   NULL);		/* AST parameter*/

	/****************************************************************/
	/*								*/
	/* We want to ignore queue names that start with NM$.  If the 	*/
	/* name just extracted is such a name we simply don't advance 	*/
	/* the ptr.					                */
	/*								*/
	/****************************************************************/
	if ((! ((at_buffer[0] == 'N') &&
		(at_buffer[1] == 'M') &&   
		(at_buffer[2] == '$')))	&&
	    ((l_ret_status == SS$_NORMAL) && ((r_iosb.l_stat & 1) == 1)) )
	{
	    at_buffer[l_ret_length] = 0; /* Null-terminate the queue name */

	    /************************************************************/
	    /*								*/
	    /* Convert the q name to compound and add to compound 	*/
	    /* string queue names list.  (Allocate new memory if 	*/
	    /* necessary.)						*/
	    /*								*/
	    /************************************************************/
	    (*al_queue_count)++;

	    if (*al_queue_count > l_mem_allocation)
	    {
		ar_cs_queues = (XmString *) XtRealloc(ar_cs_queues,
						      sizeof(XmString) *
						      (l_mem_allocation + K_PW_CHUNK_INCR));
		l_mem_allocation +=  K_PW_CHUNK_INCR;
	    }

	    /************************************************************/
	    /*								*/
	    /* Add the new compound string queue name to the list.	*/
	    /*								*/
	    /************************************************************/
	    ar_cs_queues[*al_queue_count - 1] = (XmString) DXmCvtOStoCS(at_buffer,&l_size,&l_status);

	} /* not-an-NM$-and-valid queue */
    } /* while */

    /********************************************************************/
    /*									*/
    /* Now check to see if no valid queues were found.  If there are	*/
    /* none, and the previous list was non-empty, then the vector of 	*/
    /* pointers has not been freed.  Free it now.  (We could also check	*/
    /* to see if the mem_allocation was significantly larger than that	*/
    /* used [meaning that this new list is much shorter than the 	*/
    /* previous one] and do one last realloc to shrink the memory, 	*/
    /* but...) 								*/
    /*									*/
    /********************************************************************/
    if ((*al_queue_count <= 0) && (l_mem_allocation > 0))
    {
	XtFree(ar_cs_queues);
	ar_cs_queues = NULL;
    }

    /********************************************************************/
    /*									*/
    /* Dissolve any internal search context block for the process	*/
    /*									*/
    /********************************************************************/
    ar_itmlst[0].at_bufadr    = 0L;
    ar_itmlst[0].c_code       = 0L;
    ar_itmlst[0].c_buflen     = 0L;
    ar_itmlst[0].al_retlenadr = 0L;

    r_iosb.l_dummy = 0L;
    l_ret_status = sys$getquiw(NULL,
			       QUI$_CANCEL_OPERATION,
			       NULL,
			       &ar_itmlst,
			       &r_iosb,
			       NULL,
			       NULL);

	
    /********************************************************************/
    /*									*/
    /* Now point the passed structure to the malloc'ed memory.		*/
    /*									*/
    /********************************************************************/
    *aar_cs_queues = ar_cs_queues;

    return (SS$_NORMAL);

} /* pw_get_print_queue_names (VMS) */


/************************************************************************/
/*									*/
/* pw_get_print_form_names (VMS)					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Determines and returns the names of the print forms on the 	*/
/*	current system and the number of such names found.              */
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	cs_forms	Array to be created.				*/
/*									*/
/*      form_count	Count of forms.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	Returns error status codes returned by $GETQUIW system service  */
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
int pw_get_print_form_names(aar_cs_forms,al_form_count)
    XmString 	*(aar_cs_forms[]);
    int 	*al_form_count;
{
    int			l_search_flags = QUI$M_SEARCH_WILDCARD;
    r_iosb_struct	r_iosb;    
    int			l_ret_status;
    short		l_ret_length = 0;
    char		at_buffer[K_PW_MAX_FORM_NAME_LENGTH+2];
    int			l_mem_allocation = 0;
    int			l_count = 0;
    r_itmbuf_struct 	ar_itmlst[] = 
                        {
			    { 0L, 0L, 0L, 0L },
			    { 0L, 0L, 0L, 0L },
			    { 0L, 0L, 0L, 0L },
			    { 0L, 0L, 0L, 0L }
			};

    XmString		*ar_cs_forms = *aar_cs_forms;

    long	l_size,l_status;

    /********************************************************************/
    /*								  	*/
    /* If this is an 'uninitialized list', the counter will be at -1. 	*/
    /* Bump it to 0 (which it should be if no forms are found) before 	*/
    /* calculating the allocation. 					*/
    /*									*/
    /********************************************************************/
    if (*al_form_count < 0)
	*al_form_count = 0;

    /********************************************************************/
    /*								  	*/
    /* Determine the memory allocation (in longwords) of the vector 	*/
    /* passed in.  (This list's memory will be reused via realloc 	*/
    /* calls.)								*/
    /*									*/
    /********************************************************************/
    l_mem_allocation = 
	((*al_form_count + K_PW_CHUNK_INCR - 1) / 
	 K_PW_CHUNK_INCR) * 
	K_PW_CHUNK_INCR;
    
    /********************************************************************/
    /*									*/
    /* Free up the memory referenced by the pointers in the array (but	*/
    /* not the array itself) and reset the count. 			*/
    /*									*/
    /********************************************************************/
    pw_free_cs_list_ref_memory(ar_cs_forms,*al_form_count);
    *al_form_count = 0;
    
    /********************************************************************/
    /*									*/
    /* Dissolve any internal search context block for the process	*/
    /*									*/
    /********************************************************************/
    r_iosb.l_dummy = 0L;
    l_ret_status = sys$getquiw(NULL,
			       QUI$_CANCEL_OPERATION,
			       NULL,
			       &ar_itmlst,
			       &r_iosb,
			       NULL,
			       NULL);

    /********************************************************************/
    /*									*/
    /* Set up itmlst entries to do repeated searches for queue names	*/
    /*									*/
    /********************************************************************/
    ar_itmlst[0].at_bufadr    = "*";	/* Wild-carded name */
    ar_itmlst[0].c_code       = QUI$_SEARCH_NAME;
    ar_itmlst[0].c_buflen     = 1L;
    ar_itmlst[0].al_retlenadr = 0L;

    ar_itmlst[1].at_bufadr    = &l_search_flags;
    ar_itmlst[1].c_code       = QUI$_SEARCH_FLAGS;
    ar_itmlst[1].c_buflen     = 4L;
    ar_itmlst[1].al_retlenadr = 0L;

    ar_itmlst[2].at_bufadr    = &at_buffer;
    ar_itmlst[2].c_code       = QUI$_FORM_NAME;
    ar_itmlst[2].c_buflen     = K_PW_MAX_FORM_NAME_LENGTH;
    ar_itmlst[2].al_retlenadr = &l_ret_length;

    /********************************************************************/
    /*									*/
    /* Loop through the system queues, adding appropriate queue names 	*/
    /* to the caller's q names compound string list.			*/
    /*									*/
    /********************************************************************/
    r_iosb.l_stat = 1;
    l_ret_status = SS$_NORMAL;

    while ((l_ret_status == SS$_NORMAL) && 
	   ((r_iosb.l_stat & 1) == 1))
    {
	/****************************************************************/
	/*								*/
	/* Try to get a new form name                                   */
	/*								*/
	/****************************************************************/
	r_iosb.l_dummy = 0L;
	l_ret_status = sys$getquiw(NULL,		/* event flag number */
				   QUI$_DISPLAY_FORM,	/* function code */
				   NULL,	        /* not used in this service */
				   &ar_itmlst,		/* item list */
				   &r_iosb,		/* i/o status block */
				   NULL,		/* AST address */
				   NULL);		/* AST parameter*/

	/****************************************************************/
	/*								*/
	/* If the status is OK, then save the form name.		*/
	/*								*/
	/****************************************************************/
	if ((l_ret_status == SS$_NORMAL) && ((r_iosb.l_stat & 1) == 1))
	{
	    at_buffer[l_ret_length] = 0; /* Null-terminate the form name */

	    /************************************************************/
	    /*								*/
	    /* Convert the form name to compound and add to compound 	*/
	    /* string form names list.  (Allocate new memory if 	*/
	    /* necessary.)						*/
	    /*								*/
	    /************************************************************/
	    (*al_form_count)++;

	    if (*al_form_count > l_mem_allocation)
	    {
		ar_cs_forms = (XmString *) XtRealloc(ar_cs_forms,
						     sizeof(XmString) *
						     (l_mem_allocation + K_PW_CHUNK_INCR));
		l_mem_allocation +=  K_PW_CHUNK_INCR;
	    }

	    /************************************************************/
	    /*								*/
	    /* Add the new compound string form name to the list.	*/
	    /*								*/
	    /************************************************************/
	    ar_cs_forms[*al_form_count - 1] = (XmString) DXmCvtOStoCS(at_buffer,&l_size,&l_status);

	} /* if */
    } /* while */

    /********************************************************************/
    /*									*/
    /* Now check to see if no valid forms were found.  If there are	*/
    /* none, and the previous list was non-empty, then the vector of 	*/
    /* pointers has not been freed.  Free it now.  (We could also check	*/
    /* to see if the mem_allocation was significantly larger than that	*/
    /* used [meaning that this new list is much shorter than the 	*/
    /* previous one] and do one last realloc to shrink the memory, 	*/
    /* but...) 								*/
    /*									*/
    /********************************************************************/
    if ((*al_form_count <= 0) && (l_mem_allocation > 0))
    {
	XtFree(ar_cs_forms);
	ar_cs_forms = NULL;
    }

    /********************************************************************/
    /*									*/
    /* Dissolve any internal search context block for the process	*/
    /*									*/
    /********************************************************************/
    ar_itmlst[0].at_bufadr    = 0L;
    ar_itmlst[0].c_code       = 0L;
    ar_itmlst[0].c_buflen     = 0L;
    ar_itmlst[0].al_retlenadr = 0L;

    r_iosb.l_dummy = 0L;
    l_ret_status = sys$getquiw(NULL,
			       QUI$_CANCEL_OPERATION,
			       NULL,
			       &ar_itmlst,
			       &r_iosb,
			       NULL,
			       NULL);

	
    /********************************************************************/
    /*									*/
    /* Now point the passed structure to the malloc'ed memory.		*/
    /*									*/
    /********************************************************************/
    *aar_cs_forms = ar_cs_forms;

    return (SS$_NORMAL);

} /* pw_get_print_form_names (VMS) */


/************************************************************************/
/*									*/
/* pw_get_filename_extension						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/* 	Get filename extension from a VMS filename.  Return the		*/
/*	extension and its length.					*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	filename	Name of the file				*/
/*									*/
/*	extension	The extension					*/
/*									*/
/* 	length		String length of the extension.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static void pw_get_filename_extension (at_filename,al_extension,al_length)
    char	*at_filename;
    int		*al_extension;
    int		*al_length;
{
    typedef struct {
	unsigned short c_length;		/* file extension length.  This includes the  */
	  					/* preceding "."  Set by @FILESCAN call.      */
	unsigned short c_item_code;		/* requested componenet of file spec.         */
	char           *at_bufadr;		/* pointer to component sought.  Set by       */
	                                	/* $FILESCAN call.                            */
    } r_item_list_2;

    int			al_src_string[2];	/* fixed-length string descriptor of incoming filespec*/
    r_item_list_2	ar_value_list[2];	/* item list for calling $FILESCAN                    */

    /********************************************************************/
    /*									*/
    /* Call the system routine to find the file extension.		*/
    /*									*/
    /********************************************************************/
    al_src_string[0] = strlen(at_filename);
    al_src_string[1] = (int) at_filename;

    ar_value_list[0].c_item_code = FSCN$_TYPE;	/* only need to set itemcode for     */
                                                /* desired component.                */

    ar_value_list[1].c_item_code = 0;		/* to mark end of item list.         */

    SYS$FILESCAN (al_src_string,ar_value_list,NULL);

    /********************************************************************/
    /*									*/
    /* If a non-zero length found.  Decrement length by one and 	*/
    /* incrementaddress by one so that it points to 1st char of text, 	*/
    /* not the preceding "." 	  					*/
    /*									*/
    /********************************************************************/
    if (ar_value_list[0].c_length != 0)
    {
	ar_value_list[0].c_length = ar_value_list[0].c_length - 1;
	ar_value_list[0].at_bufadr = ar_value_list[0].at_bufadr + 1;

	/****************************************************************/
	/*								*/
	/* If reduced length is now zero, we saw only a "."  		*/
	/* Drive the address to zero as well. 	                        */
	/*								*/
	/****************************************************************/
	if (ar_value_list[0].c_length == 0) 
	    ar_value_list[0].at_bufadr = NULL;
    }

    /********************************************************************/
    /*									*/
    /* Deliver the results.						*/
    /*									*/
    /********************************************************************/
    *al_extension = (int) ar_value_list[0].at_bufadr;
    *al_length = ar_value_list[0].c_length;

} /* pw_get_filename_extension */


/************************************************************************/
/*									*/
/* pw_get_guesser_format_mapping (VMS)					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/* 	Accept a filename and return an integer value which matches	*/
/*	one of the DXmPRINT_FORMAT_XXX constants.			*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	filename	Compound String version of filename.		*/
/*									*/
/*	guesser_mapping	Integer which is one of DXmPRINT_FORMAT_XXX.	*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
void pw_get_guesser_format_mapping(ar_filename,al_guesser_mapping)
    XmString 	ar_filename;
    int		*al_guesser_mapping;
{
    char	*at_filename;
    int		al_extension[2];
    int		al_vms_table_extension[2];
    int		l_i;
    long	l_size,l_status;
    
    /********************************************************************/
    /*									*/
    /* Initialize guesser mapping to DXmPRINT_FORMAT_TEXT		*/
    /*									*/
    /********************************************************************/
    *al_guesser_mapping = DXmPRINT_FORMAT_TEXT;
    
    /********************************************************************/
    /*									*/
    /* Get the filename (text version) and its extension.  Free up the	*/
    /* text version of the filename since we no longer need it.		*/
    /*									*/
    /********************************************************************/
    at_filename = (char *) DXmCvtCStoOS(ar_filename,&l_size,&l_status);

    if (!at_filename)
	return;
    
    pw_get_filename_extension(at_filename,
			      &al_extension[1],		/* extension */
			      &al_extension[0]);	/* length */

    XtFree(at_filename);

    /********************************************************************/
    /*									*/
    /* Loop through the VMS assumption table to see if we can find a 	*/
    /* match.								*/
    /*									*/
    /********************************************************************/
    l_i = 0;
    
    do
    {
	al_vms_table_extension[0] = strlen(pw_ar_VMS_assumption_table[l_i].at_file_extension);
	al_vms_table_extension[1] = pw_ar_VMS_assumption_table[l_i].at_file_extension;
	
	l_status = STR$CASE_BLIND_COMPARE(al_vms_table_extension,al_extension);

	/****************************************************************/
	/*								*/
	/* If there's a match, we're done.				*/
	/*								*/
	/****************************************************************/
	if (!l_status)
	{
	    *al_guesser_mapping = pw_ar_VMS_assumption_table[l_i].l_guesser_format_mapping;
	    break;
	}
	/****************************************************************/
	/*								*/
	/* Otherwise, keep going until we get a match or there are no	*/
	/* more extensions in the table.				*/
	/*								*/
	/****************************************************************/
	else
	{
	    l_i++;
	    
	    if (!pw_ar_VMS_assumption_table[l_i].at_file_extension)
		break;
	}
    } while (TRUE);
	    
} /* pw_get_guesser_format_mapping */


#else
/************************************************************************/
/************************************************************************/
/************************************************************************/
/*								  	*/
/*			   ULTRIX SPECIFIC				*/
/*									*/
/************************************************************************/
/************************************************************************/
/************************************************************************/


/************************************************************************/
/*									*/
/* pw_get_prent								*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/* 	Get next entry in /etc/printcap - mallocs a buffer and returns 	*/
/*	a compound string.						*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	ar_cs		Compound String to be created.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/* 	Function returns -1 if printcap cannot be opened		*/
/*                          if printer name too long                    */
/*                   	  1 if no more printer names in database        */
/*                    and 0 if ok                                       */
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static int pw_get_prent(ar_cs)
    XmString 	*ar_cs;
{
    
    static char 	at_table[] = "/etc/printcap";
    static FILE 	*ar_pfp = NULL;
 
    char at_tbuf[K_PW_MAX_QUEUE_NAME_LENGTH];
    char *at_tbp = at_tbuf;

    int l_c, l_skip = 0;
    int l_found = 0, l_esc = 0;
 
    long	l_size,l_status;

    if (ar_pfp == NULL && (ar_pfp = fopen(at_table, "r")) == NULL) 
    {
	fprintf(stderr, "Cannot access printer description file\n");
	return(-1);
    }

    while (!l_found) 
    {
	switch (l_c = getc(ar_pfp)) 
	{
	    case EOF:
	    {		
		fclose(ar_pfp);
		ar_pfp = NULL;
		return(1);
	    }
	    
	    case '\\' :
	    {
		l_esc++;
		break;
	    }
	    
	    case '\n':
	    {
		if (l_esc) 
		    l_esc = 0;
		else 
		{
		    l_skip = 0;              	/* reset skip flag */
		    if (at_tbp != at_tbuf)	/* end loop if name found */
			l_found++;
		}
		break;
	    }
	
	    case '#' :
	    {
		l_skip++;                       /* skip comment lines */
		break;
	    }
	    
	    case '|' :                    	/* end of name */
	    case ':' :
	    {    
		*at_tbp = NULL;
		l_skip++;                       /* skip rest of entry */
		break;
	    }
	    
	    default:
	    {
		if (!l_skip) 
		{
		    if (at_tbp >= at_tbuf + K_PW_MAX_QUEUE_NAME_LENGTH) 
		    {
			fprintf(stderr, "Printer name too long\n");
			*at_tbp = NULL;
			return(-1);
		    }
		    *at_tbp++ = l_c;
		}
	    }
	}
    }
    
    *ar_cs = (XmString) DXmCvtOStoCS(at_tbuf,&l_size,&l_status);
    
    return(0);
}


/************************************************************************/
/*									*/
/* pw_get_print_queue_names (ULTRIX)					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Determines and returns the names of the print queues on the 	*/
/*	current system and the number of such names found.              */
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	cs_queues	Array to be created.				*/
/*									*/
/*      queue_count	Count of queues.				*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	1								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
int pw_get_print_queue_names(aar_cs_queues,al_queue_count)
    XmString 	*(aar_cs_queues[]);
    int 	*al_queue_count;
{
    int		l_mem_allocation = 0;
    XmString 	ar_cs_temp;
    XmString	*ar_cs_queues = *aar_cs_queues;

    /********************************************************************/
    /*								  	*/
    /* If this is an 'uninitialized list', the counter will be at -1. 	*/
    /* Bump it to 0 (which it should be if no queues are found) before 	*/
    /* calculating the allocation. 					*/
    /*									*/
    /********************************************************************/
    if (*al_queue_count < 0)
	*al_queue_count = 0;

    /********************************************************************/
    /*								  	*/
    /* Determine the memory allocation (in longwords) of the vector 	*/
    /* passed in.  (This list's memory will be reused via realloc 	*/
    /* calls.)								*/
    /*									*/
    /********************************************************************/
    l_mem_allocation = 
	((*al_queue_count + K_PW_CHUNK_INCR - 1) / 
	 K_PW_CHUNK_INCR) * 
	K_PW_CHUNK_INCR;
    
    /********************************************************************/
    /*									*/
    /* Free up the memory referenced by the pointers in the array (but	*/
    /* not the array itself) and reset the count. 			*/
    /*									*/
    /********************************************************************/
    pw_free_cs_list_ref_memory(ar_cs_queues,*al_queue_count);
    *al_queue_count = 0;
    
    /********************************************************************/
    /*									*/
    /* Get the print queue entry names one at a time and add them to	*/
    /* the vector of compound strings. 					*/ 
    /*									*/
    /********************************************************************/
    while (pw_get_prent(&ar_cs_temp) == 0) 
    {	
	/****************************************************************/
	/*								*/
	/* Convert the q name to compound and add to compound 		*/
	/* string queue names list.  (Allocate new memory if 		*/
	/* necessary.)							*/
	/*	       							*/
	/****************************************************************/
	(*al_queue_count)++;

	if (*al_queue_count > l_mem_allocation)
	{
	    ar_cs_queues = (XmString *) XtRealloc((char *)ar_cs_queues,
						  sizeof(XmString) *
						  (l_mem_allocation + K_PW_CHUNK_INCR));
	    l_mem_allocation +=  K_PW_CHUNK_INCR;
	}

	/****************************************************************/
	/*								*/
	/* Add the new compound string queue name to the list.		*/
	/*								*/
	/****************************************************************/
	ar_cs_queues[*al_queue_count - 1] = ar_cs_temp;
    } 

    /********************************************************************/
    /*									*/
    /* Now point the passed structure to the malloc'ed memory.		*/
    /*									*/
    /********************************************************************/
    *aar_cs_queues = ar_cs_queues;

    return(1);

} /* pw_get_print_queue_names */


/************************************************************************/
/*									*/
/* pw_get_print_form_names (ULTRIX)					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*      Determines and returns the names of the print forms on the 	*/
/*	current system and the number of such names found.              */
/*                                                                      */
/*      [[[THIS IS CURRENTLY A STUB RETURNING THE CANNED NAME OF 	*/
/*	"Default.]]]							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	cs_forms	Array to be created.				*/
/*									*/
/*      form_count	Count of forms.					*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	1								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
int pw_get_print_form_names(aar_cs_forms,al_form_count)
    XmString 	*(aar_cs_forms[]);
    int 	*al_form_count;
{
    XmString	*ar_cs_forms = *aar_cs_forms;
    long	l_size,l_status;

    /********************************************************************/
    /*								  	*/
    /* If this is an 'uninitialized list', the counter will be at -1. 	*/
    /* Bump it to 0 (which it should be if no queues are found) before 	*/
    /* calculating the allocation. 					*/
    /*									*/
    /********************************************************************/
    if (*al_form_count < 0)
	*al_form_count = 0;
    
    /********************************************************************/
    /*									*/
    /* Free up the memory referenced by the array.			*/
    /*									*/
    /********************************************************************/
    if (ar_cs_forms)
    {
	pw_free_cs_list_ref_memory(ar_cs_forms,*al_form_count);
	*al_form_count = 0;
	XtFree((char *)ar_cs_forms);
    }
    
    ar_cs_forms 	= (XmString *) XtMalloc(sizeof(XmString));
    ar_cs_forms[0] 	= (XmString) DXmCvtOStoCS("Default",&l_size,&l_status);
    *al_form_count = 1;

    /********************************************************************/
    /*									*/
    /* Now point the passed structure to the malloc'ed memory.		*/
    /*									*/
    /********************************************************************/
    *aar_cs_forms = ar_cs_forms;

    return (1);

} /* pw_get_print_form_names */


/************************************************************************/
/*									*/
/* pw_is_postscript							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/* 	Look for postscript mark.					*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	buffer		Buffer to look at.				*/
/*									*/
/*	limit		Limit of characters to look at.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE if this has postscript mark.				*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_is_postscript(at_buffer,l_limit)
    char 	*at_buffer;
    int		l_limit;
{
    static char *at_postscript_mark = "%!PS-Adobe-";

    if (!strncmp(at_buffer,at_postscript_mark,strlen(at_postscript_mark)))
	return TRUE;

    return FALSE;
    
} /* pw_is_postscript */
 

/************************************************************************/
/*									*/
/* pw_is_regis								*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/* 	Look for 'enter ReGIS mode' escape sequence (ESC P [0-3] p)	*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	buffer		Buffer to look at.				*/
/*									*/
/*	limit		Limit of characters to look at.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE if this has REGIS mode escape sequences.			*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_is_regis(at_buffer,l_limit)
    char 	*at_buffer;
    int		l_limit;
{
#define ESC	'\033'
 
    int l_i = 0;

    while (l_i++ < l_limit - 2) 
    {
	if ((at_buffer[l_i - 1] == ESC) && 
	    (at_buffer[l_i] == 'P') &&
	    (((at_buffer[l_i + 1] >= '0') && 
	      (at_buffer[l_i + 1] <= '3') && 
	      (at_buffer[l_i + 2] == 'p')) ||
	     (at_buffer[l_i + 1] == 'p')))
		return TRUE;
    }
    
    return FALSE;

} /* pw_is_regis */
 

/************************************************************************/
/*									*/
/* pw_is_tektronix							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/* 	Look for Tektronix 4010/4014 control and escape sequences.	*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	buffer		Buffer to look at.				*/
/*									*/
/*	limit		Limit of characters to look at.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	TRUE if this has Tektronix control and escape sequences.	*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static Boolean pw_is_tektronix(at_buffer,l_limit)
    char 	*at_buffer;
    int		l_limit;
{
#define ESC	'\033'
#define FF	'\014'		/* set alpha mode */
#define ETB	'\027'		/* page eject */
#define CAN	'\030'		/* set bypass */
#define SUB	'\031'		/* set alpha */
 
#define FS	'\034'		/* enter point plot mode */
#define GS	'\035'		/* enter graph mode */
#define RS	'\036'		/* enter incremental plot mode */
#define US	'\037'		/* reset to alpha mode */
 
#define DEL	'\177'
 
    int 	l_i = 0;
    char	c = 0;
 
    while (l_i++ < l_limit - 2) 
    {
	switch(at_buffer[l_i - 1]) 
	{
	    case ESC:
	    {
		c = at_buffer[l_i];
		if ((c == FF) || (c == ETB) || (c == CAN) || (c == SUB))
		    return TRUE;
		break;
	    }
	    
	    case FS:
	    case GS:
	    case RS:
	    case US:
		return TRUE;
	}
    }

    return FALSE;

} /* pw_is_tektronix */
 

/************************************************************************/
/*									*/
/* pw_get_ansi_level							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/* 	Looks for control characters.  If there is a backspace or	*/	
/*	escape, the ansi level is assumed to be DXmPRINT_FORMAT_ANSI,	*/
/*	otherwise it is assumed to be DXmPRINT_FORMAT_TEXT.		*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	buffer		Buffer to look at.				*/
/*									*/
/*	limit		Limit of characters to look at.			*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	DXmPRINT_FORMAT_ANSI or DXmPRINT_FORMAT_TEXT			*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
static int pw_get_ansi_level(at_buffer,l_limit)
    char 	*at_buffer;
    int		l_limit;
{

#define BS 	'\010'
 
    int l_i;
 
    for (l_i = 0; l_i < l_limit; l_i++)
	if (at_buffer[l_i] == BS || at_buffer[l_i] == ESC)
	    return(DXmPRINT_FORMAT_ANSI);

    return(DXmPRINT_FORMAT_TEXT);

} /* pw_get_ansi_level */

#ifdef __osf__
#define ROSE_MAGIC	0xefbe		/* magic no for Rose format */
static Boolean IsOSF1Executable (ifile)
    int	    ifile;
{
    char buf[BUFSIZ+FILHSZ+AOUTHSZ];
    int in;
    struct	filehdr	*fhdr;		/* pointer to filehdr for this binary */
    struct aouthdr	*ahdr;		/* pointer to aouthdr for this binary */
    int binary=0, swap=0;

    in = read(ifile, buf, sizeof(buf));

    fhdr = ((FILHDR *) buf);

    /* check for mips binary */

    switch (fhdr->f_magic) {

	case ROSE_MAGIC:
	    binary = 1;
	    break;
	case MIPSEBMAGIC:
	    binary = 1;
	    break;
	case MIPSEBMAGIC_2:
	    binary = 1;
	    break;
	case MIPSEBMAGIC_3:
	    binary = 1;
	    break;
	case MIPSELMAGIC:
	    binary = 1;
	    break;
	case MIPSELMAGIC_2:
	    binary = 1;
	    break;
	case MIPSELMAGIC_3:
	    binary = 1;
	    break;
	case SMIPSEBMAGIC:
	    swap = 1;
	    binary = 1;
	    break;
	case SMIPSEBMAGIC_2:
	    swap = 1;
	    binary = 1;
	    break;
	case SMIPSEBMAGIC_3:
	    swap = 1;
	    binary = 1;
	    break;
	case SMIPSELMAGIC:
	    swap = 1;
	    binary = 1;
	    break;
	case SMIPSELMAGIC_2:
	    swap = 1;
	    binary = 1;
	    break;
	case SMIPSELMAGIC_3:
	    swap = 1;
	    binary = 1;
	    break;
	} /* switch */

	if (binary) {
	    if (swap) {
		/* swap_filehdr (fhdr, gethostsex()); */
	    } /* if */

	if (fhdr->f_opthdr) {
		/* get the aouthdr if there is one */
		ahdr = (AOUTHDR *)(buf+FILHSZ);
		if (swap) {
		    /* swap_aouthdr (ahdr, gethostsex()); */
		}
	} 
	else {
		return (FALSE);
	} /* if */

	    /* check the load type */
	    switch (ahdr->magic) {
		case ZMAGIC:
		case NMAGIC:
		case OMAGIC:
		{
		    return (TRUE);
		}
	    } /* switch */
	}   /* if binary */
	    return (FALSE);
}
#endif    


/************************************************************************/
/*									*/
/* pw_get_guesser_format_mapping (ULTRIX)				*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/* 	Accept a filename and return an integer value which matches	*/
/*	one of the DXmPRINT_FORMAT_XXX constants.			*/
/*                                                                      */
/* FORMAL PARAMETERS:							*/
/*									*/
/*	filename	Compound String version of filename.		*/
/*									*/
/*	guesser_mapping	Integer which is one of DXmPRINT_FORMAT_XXX.	*/
/*									*/
/* IMPLICIT INPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* IMPLICIT OUTPUTS:							*/
/*									*/
/*	None								*/
/*									*/
/* FUNCTION VALUE:							*/
/*									*/
/*	None								*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
void pw_get_guesser_format_mapping(ar_filename,al_guesser_mapping)
    XmString 	ar_filename;
    int		*al_guesser_mapping;
{
    char	*at_filename;
#if !defined(__osf__)
    struct exec r_execb;
#endif
    struct stat r_statbuf;

    int		l_fd;
    char 	at_buffer[BUFLEN];
    int 	l_cc;

    long	l_size,l_status;

    /********************************************************************/
    /*									*/
    /* Initialize guesser mapping to DXmPRINT_FORMAT_TEXT		*/
    /*									*/
    /********************************************************************/
    *al_guesser_mapping = DXmPRINT_FORMAT_TEXT;

    /********************************************************************/
    /*									*/
    /* Get the filename (text version).					*/
    /*									*/
    /********************************************************************/
    at_filename = (char *) DXmCvtCStoOS(ar_filename,&l_size,&l_status);

    /********************************************************************/
    /*									*/
    /* If this is a regular file, do some checking...			*/
    /*									*/
    /********************************************************************/
    if (stat(at_filename,&r_statbuf) == 0) 
    {
	if ((r_statbuf.st_mode & S_IFMT) == S_IFREG) 
	{ 
	    if ((l_fd = open(at_filename,O_RDONLY,0)) >= 0) 
	    {
#ifdef __osf__
		if (IsOSF1Executable(l_fd))
		{
		    fprintf(stderr,"%s: guesser: Executable file\n",at_filename);
			    (void) close(l_fd);
		    return;
		}
#else
		if (read(l_fd, &r_execb, sizeof(r_execb)) == sizeof(r_execb)) 
		{
		    switch(r_execb.a_magic) 
		    { /* check if file is an executable */
			case OMAGIC:
			case NMAGIC:
			case ZMAGIC:
			{
			    fprintf(stderr,"%s: guesser: Executable file\n",at_filename);
			    (void) close(l_fd);
			    return;
			}
		    }
		}
		
#endif
		(void) lseek(l_fd,0,L_SET);      /* reset file pointer */
		if ((l_cc = read(l_fd, at_buffer,BUFLEN)) >= 0) 
		{
		    if (pw_is_postscript(at_buffer,l_cc)) 
		    {
			*al_guesser_mapping = DXmPRINT_FORMAT_POSTSCRIPT;
			(void) close(l_fd);
			return;
		    }

		    if (pw_is_regis(at_buffer,l_cc)) 
		    {
			*al_guesser_mapping = DXmPRINT_FORMAT_REGIS;
			(void) close(l_fd);
			return;
		    }

		    if (pw_is_tektronix(at_buffer,l_cc))
		    {
			*al_guesser_mapping = DXmPRINT_FORMAT_TEKTRONIX;
			(void) close(l_fd);
			return;
		    }

		    *al_guesser_mapping = pw_get_ansi_level(at_buffer,l_cc);
		    (void) close(l_fd);
		    return;
		}
		    
		(void) close(l_fd);
	    }
	}
	else 
	{
	    fprintf(stderr,"%s: DXmPrintWgt:  Not regular file\n",at_filename);
	    return;
	}
    }

    /********************************************************************/
    /*									*/
    /* If we get this far, there is an error.				*/
    /*									*/
    /********************************************************************/
    error:
    {
	return;
    }
} /* pw_get_guesser_format_mapping (ULTRIX) */

#endif /* of non-VMS routines */
