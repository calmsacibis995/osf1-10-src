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
/*	This module contains the code necessary to submit a print job.	*/
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
/*      016     AD			3-Feb-1993                      */
/*              convert static warning and error messages to            */
/*		character arrays					*/
/*	015	WDW			03-Apr-1991			*/
/*		Add code to fix up problem with messing up signal	*/
/*		state on ULTRIX.					*/
/*	014	WDW			28-Feb-1991			*/
/*		Modify call to "select" to pass correct arguments.	*/
/*	013	WDW			06-Feb-1991			*/
/*		Add "Default" print format.				*/
/*		Fix problem with page limits on VMS.			*/
/*	012	WDW			29-Jan-1991			*/
/*		Fix up page ranges on ULTRIX.				*/
/*	011	WDW			21-Jan-1991			*/
/*		Make Boolean resources really be Boolean resources.	*/
/*	010	WDW			19-Nov-1990			*/
/*		More Style Guide Compliancy.				*/
/*	009	WDW			20-Sep-1990			*/
/*		Add augment list capability.				*/
/*	008	WDW			18-Sep-1990			*/
/*		Add "Print After" capability to ULTRIX.			*/
/*	007	WDW			24-Jul-1990			*/
/*		Change <DXm/PwVMS.h> to "PwVMS.h"			*/
/*	006	WDW			16-Jul-1990			*/
/*		Add additional params for DXmCvt...			*/
/*	005	WDW			13-Jul-1990			*/
/*		I18N work.						*/
/*	004	WDW			06-Jul-1990			*/
/*		Put DXmCSText in interface instead of XmText.  Make	*/
/*		resources be XmString's instead of char *'s.		*/
/*	003	WDW			25-Jun-1990			*/
/*		Make B4 trasmit as B4, not B3.  (QAR DECWINDOWS-IFT     */
/*		4162.)							*/
/*	002	WDW			27-Mar-1990			*/
/*		Reorganize.						*/
/*	001	WDW			19-Mar-1990			*/
/*		Add modification history.  Change names of resources to	*/
/*		match Xm naming conventions.				*/
/*									*/
/************************************************************************/
#define _PwSendJob_c_

/************************************************************************/
/*									*/
/* Orientation keywords							*/
/*									*/
/************************************************************************/
static char	*rt_orientation_keywords []=
    {
#	ifdef VMS
	{"Default"},	/* not used, but keeps indexing straight  */
	{"PORTRAIT"},
	{"LANDSCAPE"}
#	else
	"Default",
	"portrait",
	"landscape"
#	endif
    };


/************************************************************************/
/*					     				*/
/* INCLUDE FILES 			     				*/
/*					     				*/
/************************************************************************/
#include <stdio.h>
#include <errno.h>

#ifdef VMS
#   include "PwVMS.h"		/* VMS-specific job definitions */
#   include <descrip.h>
#else
#   include <signal.h>
#endif

#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>
#include <Xm/BulletinBP.h>
#include <Xm/DialogSP.h>
#include <DXm/DXmPrintP.h>
#include "DXmMessI.h"
#include "DXmPrivate.h"

/************************************************************************/
/*					     				*/
/* Message definitions for character arrays				*/
/*					     				*/
/************************************************************************/
#define PWGTSNOFILOP	_DXmMsgPWSend_0000
#define PWGTSNOSUBPR	_DXmMsgPWSend_0001
#define PWGTSNOEXECVP	_DXmMsgPWSend_0002


#define PWSMSGNAME0	_DXmMsgPWSendName_0000
#define PWSMSGNAME1	_DXmMsgPWSendName_0001
#define PWSMSGNAME2	_DXmMsgPWSendName_0002

/************************************************************************/
/*					     				*/
/* MACRO DEFINITIONS			     				*/
/*					     				*/
/************************************************************************/
#define K_PW_MAX_NUMBER_ITEM_LIST_ENTRIES 30

#ifndef FD_SET
#    define NFDBITS		(8*sizeof(fd_set))
#    define FD_SETSIZE		NFDBITS
#    define FD_SET(n,p)		((p)->fds_bits[(n)/NFDBITS]  |=  (1 << ((n) % NFDBITS)))
#    define FD_CLR(n,p)		((p)->fds_bits[(n)/NFDBITS]  &= ~(1 << ((n) % NFDBITS)))
#    define FD_ISSET(n,p)	((p)->fds_bits[(n)/NFDBITS]  &   (1 << ((n) % NFDBITS)))
#    define FD_ZERO(p)		bzero((char *)(p), sizeof(*(p)))
#endif  /* FD_SET */

/************************************************************************/
/*								     	*/
/* FORWARD DECLARATIONS                      				*/
/*					     				*/
/************************************************************************/
unsigned int  pw_send_files_to_print_queue ();

#ifdef VMS
    static void		pw_add_parameter();
    static void 	pw_make_parameter_strings ();
    static void 	pw_fill_rest_of_itemlist ();
#else
    FILE 		*pw_fopen_and_check ();
    static int	 	pw_do_command ();
    Boolean		c_childdone;
    extern void         pw_abort();
#endif


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
/* pw_add_parameter							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Adds a parameter to the parameter lists.  If the total count	*/
/*	of parameters added is more than 7 add to 2nd list.   The 	*/
/*	7 was chosen is that the LPS40 can only handle 7 options in	*/
/*	parameter list.							*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	param		Parameter to add				*/
/*									*/
/*      param1string    First string parameter				*/
/*									*/
/*	param2string	Second string parameter				*/
/*									*/
/*	param_count	Number of parameters added so far		*/
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
static void pw_add_parameter(at_param,aat_param1string,aat_param2string,al_param_count)
    char	*at_param;
    char       	**aat_param1string;
    char	**aat_param2string;
    int		*al_param_count;
{
    char	**aat_add_list;
    
    if (*al_param_count >= 7)
	aat_add_list = aat_param2string;
    else
	aat_add_list = aat_param1string;
    
    if (strlen(*aat_add_list))
	strcat(*aat_add_list,",");
    
    strcat(*aat_add_list,at_param);
    
    (*al_param_count)++;
    
} /* pw_add_parameter */


/************************************************************************/
/*									*/
/* pw_make_parameter_strings						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Fabricates the parameter string that is used with the           */
/*	SJC$_PARAMETER_1 item code in calls to $SNDJBC using value	*/
/*	settings present in the print widget.                           */
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      param1string    First string parameter				*/
/*									*/
/*	param2string	Second string parameter				*/
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
static void pw_make_parameter_strings (ar_pw,aat_param1string,aat_param2string)
    DXmPrintWgt	ar_pw;
    char       	**aat_param1string;
    char	**aat_param2string;

{
    char	at_param_string[80];
    char 	at_temp_asciz_num[10];
    Opaque	r_temp_os_string;
    Opaque	r_temp_os_string2;
    long	l_size,l_status;
    int		l_param_count = 0;
    
    /********************************************************************/
    /*									*/
    /* Print format							*/
    /*									*/
    /********************************************************************/
    if (PWl_print_format_choice(ar_pw))
    {
	r_temp_os_string = 
	    DXmCvtCStoOS(PWr_cs_list_res(ar_pw,
					 K_PW_KNOWN_FORMAT_OS_LIST_MAP).ar_cs_list[PWl_print_format_choice(ar_pw)],
			 &l_size,
			 &l_status);

	strcpy(at_param_string,"DATA_TYPE=");
	strcat(at_param_string,r_temp_os_string);
	
	pw_add_parameter(at_param_string,
			 aat_param1string,
			 aat_param2string,
			 &l_param_count);
	
	XtFree(r_temp_os_string);
    }
    
    /********************************************************************/
    /*									*/
    /* Orientation							*/
    /*									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_ORIENTATION_MAP).l_current_value)
    {
	strcpy(at_param_string,"PAGE_ORIENTATION=");
	strcat(at_param_string,rt_orientation_keywords[PWr_int_res(ar_pw,K_PW_ORIENTATION_MAP).l_current_value]);

	pw_add_parameter(at_param_string,
			 aat_param1string,
			 aat_param2string,
			 &l_param_count);
    }

    /********************************************************************/
    /*									*/
    /* Page Size							*/
    /*									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_PAGE_SIZE_MAP).l_current_value)
    {
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_PAGE_SIZE_PULLDOWN_MAP,
						      K_PW_PAGE_SIZE_MAP))
	{	
	    strcpy(at_param_string,"PAGE_SIZE=");
	    strcat(at_param_string,r_temp_os_string);

	    pw_add_parameter(at_param_string,
			     aat_param1string,
			     aat_param2string,
			     &l_param_count);
	}
    }

    /********************************************************************/
    /*									*/
    /* Sheet Size							*/
    /*									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_SHEET_SIZE_MAP).l_current_value)
    {
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_SHEET_SIZE_PULLDOWN_MAP,
						      K_PW_SHEET_SIZE_MAP))
	{
	    strcpy(at_param_string,"SHEET_SIZE=");
	    strcat(at_param_string,r_temp_os_string);

	    pw_add_parameter(at_param_string,
			     aat_param1string,
			     aat_param2string,
			     &l_param_count);
	}
    }

    /********************************************************************/
    /*									*/
    /* Input Tray							*/
    /*									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_INPUT_TRAY_MAP).l_current_value)
    {
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_INPUT_TRAY_PULLDOWN_MAP,
						      K_PW_INPUT_TRAY_MAP))
	{	
	    strcpy(at_param_string,"INPUT_TRAY=");
	    strcat(at_param_string,r_temp_os_string);

	    pw_add_parameter(at_param_string,
			     aat_param1string,
			     aat_param2string,
			     &l_param_count);
	}
    }

    /********************************************************************/
    /*									*/
    /* Output Tray							*/
    /*									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_OUTPUT_TRAY_MAP).l_current_value)
    {
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_OUTPUT_TRAY_PULLDOWN_MAP,
						      K_PW_OUTPUT_TRAY_MAP))
	{
	    strcpy(at_param_string,"OUTPUT_TRAY=");
	    strcat(at_param_string,r_temp_os_string);

	    pw_add_parameter(at_param_string,
			     aat_param1string,
			     aat_param2string,
			     &l_param_count);
	}
    }

    /********************************************************************/
    /*									*/
    /* Message Log							*/
    /*									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_MESSAGE_LOG_MAP).l_current_value)
    {	
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_MESSAGE_LOG_PULLDOWN_MAP,
						      K_PW_MESSAGE_LOG_MAP))
	{
	    strcpy(at_param_string,r_temp_os_string);

	    pw_add_parameter(at_param_string,
			     aat_param1string,
			     aat_param2string,
			     &l_param_count);
	}
    }

    /********************************************************************/
    /*									*/
    /* Sides								*/
    /*									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_SIDES_MAP).l_current_value)
    {
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_SIDES_PULLDOWN_MAP,
						      K_PW_SIDES_MAP))
	{
	    strcpy(at_param_string,"SIDES=");
	    strcat(at_param_string,r_temp_os_string);

	    pw_add_parameter(at_param_string,
			     aat_param1string,
			     aat_param2string,
			     &l_param_count);
	}
    }
    
    /********************************************************************/
    /*									*/
    /* Number Up							*/
    /*									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_NUMBER_UP_MAP).l_current_value)
    {
	sprintf(at_temp_asciz_num,"%d",PWr_int_res(ar_pw,K_PW_NUMBER_UP_MAP).l_current_value);
	
	strcpy(at_param_string,"NUMBER_UP=");
	strcat(at_param_string,at_temp_asciz_num);

	pw_add_parameter(at_param_string,
			 aat_param1string,
			 aat_param2string,
			 &l_param_count);
    }

    /********************************************************************/
    /*									*/
    /* Layup Defintion							*/
    /*									*/
    /********************************************************************/
    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_LAYUP_DEFINITION_MAP).ar_current_cs,
                              		     &l_size,
                              		     &l_status);

    if (r_temp_os_string && strlen(r_temp_os_string))
    {
	strcpy(at_param_string,"LAYUP=");
	strcat(at_param_string,r_temp_os_string);

	pw_add_parameter(at_param_string,
			 aat_param1string,
			 aat_param2string,
			 &l_param_count);
    }

    if (r_temp_os_string)
	XtFree(r_temp_os_string);
    
    /********************************************************************/
    /*									*/
    /* Sheet Count							*/
    /*									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_SHEET_COUNT_MAP).l_current_value)
    {
	sprintf(at_temp_asciz_num,"%d",PWr_int_res(ar_pw,K_PW_SHEET_COUNT_MAP).l_current_value);
	
	strcpy(at_param_string,"SHEET_COUNT=");
	strcat(at_param_string,at_temp_asciz_num);

	pw_add_parameter(at_param_string,
			 aat_param1string,
			 aat_param2string,
			 &l_param_count);
    }

    /********************************************************************/
    /*									*/
    /* Page Ranges							*/
    /* 									*/
    /* Page ranges are pretty special in that one or the other can be	*/
    /* NULL.  If one is null, we use the -Znn format.  If both are non	*/
    /* NULL, we use the -Znn,nn format.					*/
    /* 									*/
    /********************************************************************/
    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_FROM_MAP).ar_current_cs,
                              		     &l_size,
                              		     &l_status);
    r_temp_os_string2 = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_TO_MAP).ar_current_cs,
                              		      &l_size,
                              		      &l_status);
    
    if ((r_temp_os_string && strlen(r_temp_os_string)) ||
	(r_temp_os_string2 && strlen(r_temp_os_string2)))
    {
	strcpy(at_param_string,"PAGE_LIMIT=");

	if (!r_temp_os_string || !strlen(r_temp_os_string))
	{
	    /* Only "to" was specified */
	    strcat (at_param_string,r_temp_os_string2);
	}
	else
	{
	    if (!r_temp_os_string2 || !strlen(r_temp_os_string2))
	    {	    
		/* Only "from" was specified */
		strcat (at_param_string,"(");
		strcat (at_param_string,r_temp_os_string);
		strcat (at_param_string,",)");
	    }
	    else
	    {	    
		/* Both were specified */
		strcat (at_param_string,"(");
		strcat (at_param_string,r_temp_os_string);
		strcat (at_param_string,",");
		strcat (at_param_string,r_temp_os_string2);
		strcat (at_param_string,")");
	    }
	}	

	pw_add_parameter(at_param_string,
			 aat_param1string,
			 aat_param2string,
			 &l_param_count);
    }

    if (r_temp_os_string)
	XtFree(r_temp_os_string);

    if (r_temp_os_string2)
	XtFree(r_temp_os_string2);
    
    if (PWr_debug(ar_pw).l_current_value)
    {
	printf ("param string 1: %s\n",*aat_param1string);
	printf ("param string 2: %s\n",*aat_param2string);
    }

} /* pw_make_parameter_strings */


/************************************************************************/
/*									*/
/* pw_fill_rest_of_itemlist						*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	This procedure is called with an itemlist (itmlst) which has 	*/
/*	been partially filled in.  The next available slot is at i.     */
/*                                                                      */
/*	This procedure fills additional entries in the item list 	*/
/*	reflecting attributes about the job, like number of copies, 	*/
/*	start time, etc.  This information is obtained from fields in 	*/
/*	the print widget itself.  					*/
/*                                                                      */
/*      The index i is advanced as it is used and again points to the 	*/
/*	1st unused slot when this procedure exits.                      */
/*                                                                      */
/*	This routine is called in three (slightly) different situations,*/
/*	depending on whether a single file or a group of files needs to	*/
/*	be printed.					  		*/
/*	                                                                */
/*	If multiple files are to be printed, there are two steps 	*/
/*	involved:    							*/
/*									*/
/*		creation of print job - mode = K_PW_MULTI_JOB_CREATE    */
/*	                                                                */
/*              insertion of individual files                           */
/*              into ongoing job      - mode = K_PW_MULTI_JOB_ADD_FILE  */
/*	                                                                */
/*	If only a single file is being printed, it is done as a 	*/
/*	one-shot operation            - mode = K_PW_SINGLE_JOB_ENTRY    */
/*	                                                                */
/*	Different print attributes are specifiable at different points 	*/
/*	for different modes.  Some attributes are specifiable at either */
/*	K_PW_MULTI_JOB_CREATE or K_PW_MULTI_JOB_ADD_FILE time.  Such 	*/
/*	attributes are done once for job create and not repeated for 	*/
/*	each K_PW_MULTI_JOB_ADD_FILE instance.                  	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*	i		Current position in itmlst			*/
/*									*/
/*	itmlst		The Item List					*/
/*									*/
/* 	mode		Mode (see above)				*/
/*									*/
/*      param1string    First string parameter				*/
/*									*/
/*	param2string	Second string parameter				*/
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
static void pw_fill_rest_of_itemlist (ar_pw,al_i,ar_itmlst,l_mode,aat_param1string,aat_param2string)
    DXmPrintWgt		ar_pw;
    int			*al_i;
    r_itmbuf_struct	*ar_itmlst;
    int			l_mode;
    char		**aat_param1string;
    char		**aat_param2string;
{
    long	l_size,l_status;
    Opaque	r_temp_os_string;
    
    switch (l_mode)
    {
	case K_PW_MULTI_JOB_CREATE:
	{
	    /************************************************************/
	    /*								*/
	    /* File Start Sheet						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_FILE_START_SHEET_MAP).l_current_value &&
		PWr_int_res(ar_pw,K_PW_FILE_START_SHEET_MAP).l_current_value <= DXmFILE_SHEET_ALL)
	    {
		ar_itmlst[*al_i].c_buflen     = 0L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;

		if (PWr_int_res(ar_pw,K_PW_FILE_START_SHEET_MAP).l_current_value ==
		    DXmFILE_SHEET_NONE)
		{
		    ar_itmlst[*al_i].c_code = SJC$_NO_FILE_FLAG;
		}
		else
	        {
		    if (PWr_int_res(ar_pw,K_PW_FILE_START_SHEET_MAP).l_current_value ==
			DXmFILE_SHEET_ALL)
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_FLAG;
		    }
		    else
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_FLAG_ONE;
		    }
	        }
		(*al_i)++;
	    }
	    
	    /************************************************************/
	    /*								*/
	    /* File Burst Sheet						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_FILE_BURST_SHEET_MAP).l_current_value &&
		PWr_int_res(ar_pw,K_PW_FILE_BURST_SHEET_MAP).l_current_value <= DXmFILE_SHEET_ALL)
	    {
		ar_itmlst[*al_i].c_buflen     = 0L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;

		if (PWr_int_res(ar_pw,K_PW_FILE_BURST_SHEET_MAP).l_current_value ==
		    DXmFILE_SHEET_NONE)
		{
		    ar_itmlst[*al_i].c_code = SJC$_NO_FILE_BURST;
		}
		else
	        {
		    if (PWr_int_res(ar_pw,K_PW_FILE_BURST_SHEET_MAP).l_current_value ==
			DXmFILE_SHEET_ALL)
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_BURST;
		    }
		    else
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_BURST_ONE;
		    }
	        }
		(*al_i)++;
	    }
	    
	    /************************************************************/
	    /*								*/
	    /* File End Sheet						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_FILE_END_SHEET_MAP).l_current_value &&
		PWr_int_res(ar_pw,K_PW_FILE_END_SHEET_MAP).l_current_value <= DXmFILE_SHEET_ALL)
	    {
		ar_itmlst[*al_i].c_buflen     = 0L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;

		if (PWr_int_res(ar_pw,K_PW_FILE_END_SHEET_MAP).l_current_value ==
		    DXmFILE_SHEET_NONE)
		{
		    ar_itmlst[*al_i].c_code = SJC$_NO_FILE_TRAILER;
		}
		else
	        {
		    if (PWr_int_res(ar_pw,K_PW_FILE_END_SHEET_MAP).l_current_value ==
			DXmFILE_SHEET_ALL)
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_TRAILER;
		    }
		    else
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_TRAILER_ONE;
		    }
	        }
		(*al_i)++;
	    }

	    /************************************************************/
	    /*								*/
	    /* Notify							*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;
	    if (PWr_boolean_res(ar_pw,K_PW_NOTIFY_MAP).b_current_value)
		ar_itmlst[*al_i].c_code = SJC$_NOTIFY;
	    else
		ar_itmlst[*al_i].c_code = SJC$_NO_NOTIFY;

	    (*al_i)++;
	    
	    /************************************************************/
	    /*								*/
	    /* Job Name							*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_JOB_NAME_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(r_temp_os_string);
		ar_itmlst[*al_i].at_bufadr    = (char *) r_temp_os_string;  /* [[[can't free this memory until after call is made]]] */
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_JOB_NAME;

		(*al_i)++;
	    }

	    /************************************************************/
	    /*								*/
	    /* Print After						*/
	    /*								*/
	    /************************************************************/
	    if ((PWal_binary_start_time(ar_pw)[0] != -1) &&
		(PWal_binary_start_time(ar_pw)[1] != -1))
	    {		
		ar_itmlst[*al_i].c_buflen    = 8L;
		ar_itmlst[*al_i].c_code      = SJC$_AFTER_TIME;
		ar_itmlst[*al_i].at_bufadr   = PWal_binary_start_time(ar_pw);
		ar_itmlst[*al_i].al_retlenadr = 0L;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen    = 4L;
		ar_itmlst[*al_i].c_code      = SJC$_NO_AFTER_TIME;
		ar_itmlst[*al_i].at_bufadr   = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
	    }
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Hold							*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;
	    
	    if (PWr_boolean_res(ar_pw,K_PW_HOLD_JOB_MAP).b_current_value)
		ar_itmlst[*al_i].c_code = SJC$_HOLD;
	    else
		ar_itmlst[*al_i].c_code = SJC$_NO_HOLD;

	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Operator Message						*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_OPERATOR_MESSAGE_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(r_temp_os_string);
		ar_itmlst[*al_i].at_bufadr    = (char *) r_temp_os_string; /* [[[can't free this memory until after call is made]]] */
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_OPERATOR_REQUEST;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NO_OPERATOR_REQUEST;
	    }
	    
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Priority							*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_PRIORITY_MAP).l_current_value)
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].c_code	      = SJC$_PRIORITY;
		ar_itmlst[*al_i].at_bufadr    = &(PWr_int_res(ar_pw,K_PW_PRIORITY_MAP).l_current_value);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		
		(*al_i)++;
	    }

	    /************************************************************/
	    /*								*/
	    /* Start Sheet Comment					*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_START_SHEET_COMMENT_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(r_temp_os_string);
		ar_itmlst[*al_i].at_bufadr    = (char *) r_temp_os_string; /* [[[can't free this memory until after call is made]]] */
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NOTE;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NO_NOTE;
	    }
	    
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Printer Form						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_os_printer_form_choice(ar_pw))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(PWr_os_printer_form_choice(ar_pw));
		ar_itmlst[*al_i].at_bufadr    = PWr_os_printer_form_choice(ar_pw);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_FORM_NAME;

		(*al_i)++;
	    }

	    /************************************************************/
	    /*								*/
	    /* Fabricate the pieces of text we need to build up the 	*/
	    /* parameters that are transmitted via the 			*/
	    /* /PARAMETER=(xxx=yyy) mechanism.       			*/
	    /*								*/
	    /************************************************************/
	    pw_make_parameter_strings (ar_pw,aat_param1string,aat_param2string);

	    if (strlen(*aat_param1string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(*aat_param1string);
		ar_itmlst[*al_i].at_bufadr    = *aat_param1string;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code	  = SJC$_PARAMETER_1;
		(*al_i)++;
	    }
	    
	    if (strlen(*aat_param2string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(*aat_param2string);
		ar_itmlst[*al_i].at_bufadr    = *aat_param2string;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code	  = SJC$_PARAMETER_2;
		(*al_i)++;
	    }
	    
	    break;
	}

	case K_PW_MULTI_JOB_ADD_FILE:
	{
	    /************************************************************/
	    /*								*/
	    /* Automatic Pagination					*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_AUTOMATIC_PAGINATION_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_PAGINATE;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_PAGINATE;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Double Spacing						*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_DOUBLE_SPACING_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_DOUBLE_SPACE;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_DOUBLE_SPACE;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Delete File						*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_DELETE_FILE_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_DELETE_FILE;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_DELETE_FILE;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Pass All							*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_PASS_ALL_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_PASSALL;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_PASSALL;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Number Copies						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_NUMBER_COPIES_MAP).l_current_value)
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = &(PWr_int_res(ar_pw,K_PW_NUMBER_COPIES_MAP).l_current_value);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code	      = SJC$_FILE_COPIES;
		
		(*al_i)++;
	    }
	    
	    /************************************************************/
	    /*								*/
	    /* Header							*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_HEADER_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_PAGE_HEADER;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_PAGE_HEADER;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Setup							*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_SETUP_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(r_temp_os_string);
		ar_itmlst[*al_i].at_bufadr    = (char *) r_temp_os_string; /* [[[can't free this memory until after call is made]]] */
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_FILE_SETUP_MODULES;

		(*al_i)++;
	    }

	    /************************************************************/
	    /*								*/
	    /* Page Range From						*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_FROM_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		PWl_page_range_from(ar_pw) = (int) _DXmCvtCStoI(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_FROM_MAP).ar_current_cs,
								&l_size,
								&l_status);

		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = &PWl_page_range_from(ar_pw);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_FIRST_PAGE;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NO_FIRST_PAGE;
	    }

	    (*al_i)++;

	    if (r_temp_os_string)
		XtFree(r_temp_os_string);
	    
	    /************************************************************/
	    /*								*/
	    /* Page Range To						*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_TO_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		PWl_page_range_to(ar_pw) = (int) _DXmCvtCStoI(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_TO_MAP).ar_current_cs,
							      &l_size,
							      &l_status);
		
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = &PWl_page_range_to(ar_pw);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_LAST_PAGE;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NO_LAST_PAGE;
	    }

	    (*al_i)++;

	    if (r_temp_os_string)
		XtFree(r_temp_os_string);

	    break;
	}

	case K_PW_SINGLE_JOB_ENTRY:
	{    
	    /************************************************************/
	    /*								*/
	    /* File Start Sheet						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_FILE_START_SHEET_MAP).l_current_value &&
		PWr_int_res(ar_pw,K_PW_FILE_START_SHEET_MAP).l_current_value <= DXmFILE_SHEET_ALL)
	    {
		ar_itmlst[*al_i].c_buflen     = 0L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;

		if (PWr_int_res(ar_pw,K_PW_FILE_START_SHEET_MAP).l_current_value ==
		    DXmFILE_SHEET_NONE)
		{
		    ar_itmlst[*al_i].c_code = SJC$_NO_FILE_FLAG;
		}
		else
	        {
		    if (PWr_int_res(ar_pw,K_PW_FILE_START_SHEET_MAP).l_current_value ==
			DXmFILE_SHEET_ALL)
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_FLAG;
		    }
		    else
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_FLAG;
		    }
	        }
		(*al_i)++;
	    }
	    
	    /************************************************************/
	    /*								*/
	    /* File Burst Sheet						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_FILE_BURST_SHEET_MAP).l_current_value &&
		PWr_int_res(ar_pw,K_PW_FILE_BURST_SHEET_MAP).l_current_value <= DXmFILE_SHEET_ALL)
	    {
		ar_itmlst[*al_i].c_buflen     = 0L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;

		if (PWr_int_res(ar_pw,K_PW_FILE_BURST_SHEET_MAP).l_current_value ==
		    DXmFILE_SHEET_NONE)
		{
		    ar_itmlst[*al_i].c_code = SJC$_NO_FILE_BURST;
		}
		else
	        {
		    if (PWr_int_res(ar_pw,K_PW_FILE_BURST_SHEET_MAP).l_current_value ==
			DXmFILE_SHEET_ALL)
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_BURST;
		    }
		    else
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_BURST;
		    }
	        }
		(*al_i)++;
	    }
	    
	    /************************************************************/
	    /*								*/
	    /* File End Sheet						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_FILE_END_SHEET_MAP).l_current_value &&
		PWr_int_res(ar_pw,K_PW_FILE_END_SHEET_MAP).l_current_value <= DXmFILE_SHEET_ALL)
	    {
		ar_itmlst[*al_i].c_buflen     = 0L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;

		if (PWr_int_res(ar_pw,K_PW_FILE_END_SHEET_MAP).l_current_value ==
		    DXmFILE_SHEET_NONE)
		{
		    ar_itmlst[*al_i].c_code = SJC$_NO_FILE_TRAILER;
		}
		else
	        {
		    if (PWr_int_res(ar_pw,K_PW_FILE_END_SHEET_MAP).l_current_value ==
			DXmFILE_SHEET_ALL)
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_TRAILER;
		    }
		    else
		    {
			ar_itmlst[*al_i].c_code = SJC$_FILE_TRAILER;
		    }
	        }
		(*al_i)++;
	    }

	    /************************************************************/
	    /*								*/
	    /* Notify							*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;
	    if (PWr_boolean_res(ar_pw,K_PW_NOTIFY_MAP).b_current_value)
		ar_itmlst[*al_i].c_code = SJC$_NOTIFY;
	    else
		ar_itmlst[*al_i].c_code = SJC$_NO_NOTIFY;

	    (*al_i)++;
	    
	    /************************************************************/
	    /*								*/
	    /* Delete File						*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_DELETE_FILE_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_DELETE_FILE;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_DELETE_FILE;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Number Copies						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_NUMBER_COPIES_MAP).l_current_value)
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = &(PWr_int_res(ar_pw,K_PW_NUMBER_COPIES_MAP).l_current_value);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code	      = SJC$_FILE_COPIES;
		
		(*al_i)++;
	    }
	    
	    /************************************************************/
	    /*								*/
	    /* Job Name							*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_JOB_NAME_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(r_temp_os_string);
		ar_itmlst[*al_i].at_bufadr    = (char *) r_temp_os_string; /* [[[can't free this memory until after call is made]]] */
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_JOB_NAME;

		(*al_i)++;
	    }
	
	    /************************************************************/
	    /*								*/
	    /* Setup							*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_SETUP_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(r_temp_os_string);
		ar_itmlst[*al_i].at_bufadr    = (char *) r_temp_os_string; /* [[[can't free this memory until after call is made]]] */
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_FILE_SETUP_MODULES;

		(*al_i)++;
	    }

	    /************************************************************/
	    /*								*/
	    /* Print After						*/
	    /*								*/
	    /************************************************************/
	    if ((PWal_binary_start_time(ar_pw)[0] != -1) &&
		(PWal_binary_start_time(ar_pw)[1] != -1))
	    {		
		ar_itmlst[*al_i].c_buflen    = 8L;
		ar_itmlst[*al_i].c_code      = SJC$_AFTER_TIME;
		ar_itmlst[*al_i].at_bufadr   = PWal_binary_start_time(ar_pw);
		ar_itmlst[*al_i].al_retlenadr = 0L;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen    = 4L;
		ar_itmlst[*al_i].c_code      = SJC$_NO_AFTER_TIME;
		ar_itmlst[*al_i].at_bufadr   = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
	    }
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Hold							*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;
	    
	    if (PWr_boolean_res(ar_pw,K_PW_HOLD_JOB_MAP).b_current_value)
		ar_itmlst[*al_i].c_code = SJC$_HOLD;
	    else
		ar_itmlst[*al_i].c_code = SJC$_NO_HOLD;

	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Header							*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_HEADER_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_PAGE_HEADER;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_PAGE_HEADER;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Operator Message						*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_OPERATOR_MESSAGE_MAP).ar_current_cs,
                              		     	     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(r_temp_os_string);
		ar_itmlst[*al_i].at_bufadr    = (char *) r_temp_os_string; /* [[[can't free this memory until after call is made]]] */
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_OPERATOR_REQUEST;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NO_OPERATOR_REQUEST;
	    }
	    
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Priority							*/
	    /*								*/
	    /************************************************************/
	    if (PWr_int_res(ar_pw,K_PW_PRIORITY_MAP).l_current_value)
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].c_code	      = SJC$_PRIORITY;
		ar_itmlst[*al_i].at_bufadr    = &(PWr_int_res(ar_pw,K_PW_PRIORITY_MAP).l_current_value);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		
		(*al_i)++;
	    }

	    /************************************************************/
	    /*								*/
	    /* Start Sheet Comment					*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_START_SHEET_COMMENT_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(r_temp_os_string);
		ar_itmlst[*al_i].at_bufadr    = (char *) r_temp_os_string; /* [[[can't free this memory until after call is made]]] */
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NOTE;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NO_NOTE;
	    }
	    
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Page Range From						*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_FROM_MAP).ar_current_cs,
                              			     &l_size,
                              			     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		PWl_page_range_from(ar_pw) = (int) _DXmCvtCStoI(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_FROM_MAP).ar_current_cs,
								&l_size,
								&l_status);
		
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = &PWl_page_range_from(ar_pw);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_FIRST_PAGE;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NO_FIRST_PAGE;
	    }

	    (*al_i)++;

	    if (r_temp_os_string)
		XtFree(r_temp_os_string);
	    
	    /************************************************************/
	    /*								*/
	    /* Page Range To						*/
	    /*								*/
	    /************************************************************/
	    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_TO_MAP).ar_current_cs,
                                                     &l_size,
                                                     &l_status);
	    
	    if (r_temp_os_string && strlen(r_temp_os_string))
	    {
		PWl_page_range_to(ar_pw) = (int) _DXmCvtCStoI(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_TO_MAP).ar_current_cs,
							      &l_size,
							      &l_status);
		
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = &PWl_page_range_to(ar_pw);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_LAST_PAGE;
	    }
	    else
	    {
		ar_itmlst[*al_i].c_buflen     = 4L;
		ar_itmlst[*al_i].at_bufadr    = 0L;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_NO_LAST_PAGE;
	    }

	    (*al_i)++;

	    if (r_temp_os_string)
		XtFree(r_temp_os_string);
	    
	    /************************************************************/
	    /*								*/
	    /* Printer Form						*/
	    /*								*/
	    /************************************************************/
	    if (PWr_os_printer_form_choice(ar_pw))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(PWr_os_printer_form_choice(ar_pw));
		ar_itmlst[*al_i].at_bufadr    = PWr_os_printer_form_choice(ar_pw);
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code       = SJC$_FORM_NAME;

		(*al_i)++;
	    }

	    /************************************************************/
	    /*								*/
	    /* Automatic Pagination					*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_AUTOMATIC_PAGINATION_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_PAGINATE;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_PAGINATE;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Double Spacing						*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_DOUBLE_SPACING_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_DOUBLE_SPACE;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_DOUBLE_SPACE;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Pass All							*/
	    /*								*/
	    /************************************************************/
	    ar_itmlst[*al_i].c_buflen     = 4L;
	    ar_itmlst[*al_i].at_bufadr    = 0L;
	    ar_itmlst[*al_i].al_retlenadr = 0L;

	    if (PWr_boolean_res(ar_pw,K_PW_PASS_ALL_MAP).b_current_value)
		ar_itmlst[*al_i].c_code	      = SJC$_PASSALL;
	    else
		ar_itmlst[*al_i].c_code	      = SJC$_NO_PASSALL;
		
	    (*al_i)++;

	    /************************************************************/
	    /*								*/
	    /* Fabricate the pieces of text we need to build up the 	*/
	    /* parameters that are transmitted via the 			*/
	    /* /PARAMETER=(xxx=yyy) mechanism.       			*/
	    /*								*/
	    /************************************************************/
	    pw_make_parameter_strings (ar_pw,aat_param1string,aat_param2string);

	    if (strlen(*aat_param1string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(*aat_param1string);
		ar_itmlst[*al_i].at_bufadr    = *aat_param1string;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code	  = SJC$_PARAMETER_1;
		(*al_i)++;
	    }
	    
	    if (strlen(*aat_param2string))
	    {
		ar_itmlst[*al_i].c_buflen     = strlen(*aat_param2string);
		ar_itmlst[*al_i].at_bufadr    = *aat_param2string;
		ar_itmlst[*al_i].al_retlenadr = 0L;
		ar_itmlst[*al_i].c_code	  = SJC$_PARAMETER_2;
		(*al_i)++;
	    }
	    
	    break;
	}
    }

    /********************************************************************/
    /*									*/
    /* Finally, zero out last (first unused) entry                      */
    /*									*/
    /********************************************************************/
    ar_itmlst[*al_i].c_buflen     = 0L;
    ar_itmlst[*al_i].c_code       = 0L;
    ar_itmlst[*al_i].at_bufadr    = 0L;
    ar_itmlst[*al_i].al_retlenadr = 0L;

    (*al_i)++;
} /* pw_fill_rest_of_itemlist */


/************************************************************************/
/*									*/
/* pw_send_files_to_print_queue	(VMS)					*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Sends given files to print queue.  The print widget is used	*/
/*	to determine the attributes of the job.				*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      filenames	Files to be printed				*/
/*									*/
/*	filename_count	Number of files to be printed			*/
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
/*	The result of the $SNDJBCW system service.			*/
/*									*/
/* SIDE EFFECTS:							*/
/*									*/
/*	None								*/
/*									*/
/************************************************************************/
unsigned int pw_send_files_to_print_queue (ar_pw,ar_filenames,l_filename_count)
    DXmPrintWgt	ar_pw;
    Opaque	ar_filenames[];
    int 	l_filename_count;
{
    /********************************************************************/
    /*									*/
    /* The variable below contains the pointer to a dynamically		*/
    /* constructed text string which is needed to transmit information	*/
    /* via the SJB$_PARAMETER_n item codes to  $SNDJBC.                 */
    /*                                                                  */
    /* The second variable contains the actual string that has been     */
    /* constructed and is pointed to by the 1st.                        */
    /*									*/
    /********************************************************************/
    char		at_param1buffer[255];		
    char		*at_param1string = at_param1buffer;
    char		at_param2buffer[255];
    char		*at_param2string = at_param2buffer;

    r_iosb_struct	r_iosb;				/* I/O status block */
    unsigned long int	l_ret_status;			/* status of routine calls */
    unsigned long int 	l_i;				/* loop index */

    /********************************************************************/
    /*									*/
    /* A VMS itemlist structure.  Some entries are filled in this	*/
    /* routine, the remainder are added by the procedure 		*/
    /* fill_rest_of_itemlist.                     			*/
    /*									*/
    /********************************************************************/
    r_itmbuf_struct	ar_itmlst[K_PW_MAX_NUMBER_ITEM_LIST_ENTRIES ]; 

    /********************************************************************/
    /*									*/
    /* Initialize parameter strings to NULL.				*/
    /*									*/
    /********************************************************************/
    at_param1buffer[0] = '\0';
    at_param2buffer[0] = '\0';

    /********************************************************************/
    /*									*/
    /* Check for no file names.						*/
    /*									*/
    /********************************************************************/
    if ((!l_filename_count) || (!ar_filenames))
	return (FALSE);

    /********************************************************************/
    /*									*/
    /* Set up itmlst entry zero for queue name.  This is the initial 	*/
    /* entry whether we are submitting one or several files.            */
    /*                                                                  */
    /********************************************************************/
    l_i = 0;
    ar_itmlst[l_i].at_bufadr     	= PWr_os_printer_choice(ar_pw);
    ar_itmlst[l_i].c_code       	= SJC$_QUEUE;
    ar_itmlst[l_i].c_buflen     	= strlen(PWr_os_printer_choice(ar_pw));
    ar_itmlst[l_i].al_retlenadr 	= 0L;

    l_i++;

    /********************************************************************/
    /*									*/
    /* If only one file name is involved we can use the ENTER_FILE 	*/
    /* function, otherwise we must do a START_JOB , add each of the 	*/
    /* file names individually with a ADD_FILE function, then CLOSE_JOB */
    /*									*/
    /********************************************************************/
    if (l_filename_count == 1)
    {
	/****************************************************************/
	/*								*/
	/* Single File Involved			                        */
	/*                                                              */
	/****************************************************************/

	/****************************************************************/
	/*								*/
	/* Set up itmlst entry to identify the file                     */
	/*                                                              */
	/****************************************************************/
	ar_itmlst[l_i].at_bufadr	= (char *) ar_filenames[0];
	ar_itmlst[l_i].c_code      	= SJC$_FILE_SPECIFICATION;
	ar_itmlst[l_i].c_buflen    	= strlen(ar_filenames[0]);
	ar_itmlst[l_i].al_retlenadr 	= 0L;

	l_i++;

	/****************************************************************/
	/*								*/
	/* Build rest of item list to describe attributes like number	*/
	/* of copies, etc.  Last item will be one of all zeroes.        */
	/*                                                              */
	/****************************************************************/
	pw_fill_rest_of_itemlist(ar_pw,
				 &l_i, 	
				 ar_itmlst, 
				 K_PW_SINGLE_JOB_ENTRY,
				 &at_param1string, 
				 &at_param2string);

	/****************************************************************/
	/*								*/
	/* Make call to submit file.					*/
	/*                                                              */
	/****************************************************************/
	r_iosb.l_dummy = 0L;
	l_ret_status = sys$sndjbcw(NULL,
				   SJC$_ENTER_FILE,
				   NULL,
				   &ar_itmlst,
				   &r_iosb,
				   NULL,
				   NULL);
	l_ret_status = r_iosb.l_stat;
    }
    else
    {
	/****************************************************************/
	/*								*/
	/* Multiple Files Involved		                        */
	/*                                                              */
	/****************************************************************/

	/****************************************************************/
	/*                                                              */
	/* Build rest of item list to describe attributes like number 	*/
	/* of copies, etc.   Last item will be one of all zeroes.       */
	/*                                                              */
	/****************************************************************/
	int l_j;

	pw_fill_rest_of_itemlist (ar_pw, 
				  &l_i, 
				  ar_itmlst, 
				  K_PW_MULTI_JOB_CREATE,
				  &at_param1string, 
				  &at_param2string);

	/****************************************************************/
	/* 								*/
	/* Make call to start a job					*/
	/*								*/
	/****************************************************************/
	r_iosb.l_dummy = 0L;
	l_ret_status = sys$sndjbcw(NULL,
				   SJC$_CREATE_JOB,
				   NULL,
				   &ar_itmlst,
				   &r_iosb,
				   NULL,
				   NULL);
	l_ret_status = r_iosb.l_stat;

	for (l_j = 0; l_j < l_filename_count ; l_j++)
	{
	    /************************************************************/
	    /*								*/
	    /* Set up itmlst entry [0] to identify file to be added 	*/
	    /* to job.							*/
	    /*								*/
	    /************************************************************/
	    l_i = 0;
	    
	    ar_itmlst[l_i].at_bufadr  	= ar_filenames[l_j];
	    ar_itmlst[l_i].c_code      	= SJC$_FILE_SPECIFICATION;
	    ar_itmlst[l_i].c_buflen    	= strlen(ar_filenames[l_j]);
	    ar_itmlst[l_i].al_retlenadr	= 0L;

	    l_i++;

	    pw_fill_rest_of_itemlist (ar_pw, 
				      &l_i, 
				      ar_itmlst, 
				      K_PW_MULTI_JOB_ADD_FILE,
				      &at_param1string, 
				      &at_param2string);

	    /************************************************************/
	    /*								*/
	    /* Make call to add a file to ongoing job                   */
	    /*								*/	
	    /************************************************************/
	    r_iosb.l_dummy = 0L;
	    l_ret_status = sys$sndjbcw(NULL,
				       SJC$_ADD_FILE,
				       NULL,
				       &ar_itmlst,
				       &r_iosb,
				       NULL,
				       NULL);
	    l_ret_status = r_iosb.l_stat;
	} /* for each file */

	/****************************************************************/
	/*								*/
	/* Finally, set up itmlst entry [0] and make call to close the	*/
	/* job.							        */
	/*								*/
	/****************************************************************/
	ar_itmlst[0].at_bufadr	 = 0L;
	ar_itmlst[0].c_code      = 0L;
	ar_itmlst[0].c_buflen    = 0L;
	ar_itmlst[0].al_retlenadr= 0L;

	r_iosb.l_dummy = 0L;
	l_ret_status = sys$sndjbcw (NULL,
				    SJC$_CLOSE_JOB,
				    NULL,
				    &ar_itmlst,
				    &r_iosb,
				    NULL,
				    NULL);

	return (r_iosb.l_stat);
    }
}  /* pw_send_files_to_print_queue (VMS) */


#else
/************************************************************************/
/************************************************************************/
/************************************************************************/
/*									*/
/*				ULTRIX SPECIFIC				*/
/*									*/
/************************************************************************/
/************************************************************************/
/************************************************************************/

/************************************************************************/
/*									*/
/* pw_fopen_and_check							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Open a file and aborts if it isn't possible.			*/
/*									*/
/*	NB: This routine should only be called from a child process	*/
/*	    since we don't want to abort the entire application.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	filename	Filename to open.				*/
/*									*/
/*	mode		Filemode.					*/
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
/*	The Print Widget ABORTS! Actually _exit's...we don't need	*/
/*	a core dump just because it can't find a file!	== kbm ==	*/
/*									*/
/************************************************************************/
FILE *pw_fopen_and_check (ar_context, at_name, at_mode)
    XtAppContext ar_context;
    char *at_name;
    char *at_mode;
{
    FILE *ar_result;

    ar_result = fopen(at_name, at_mode);

    if (ar_result == NULL) 
    {
        String params = at_name;
	Cardinal num_params = 1;

	/*
	 *  The assumption is this is the child process stream, so we 
	 *  need to warn the user and kill the child.  We aren't using 
	 *  the fatal error-handling routine because we just want to 
	 *  abort the child, not the whole application.  
	 */
	DXMAPPWARNING(ar_context,
		      PWSMSGNAME0,
		      PWGTSNOFILOP,
		      &params,&num_params);
	_exit();
    }

    return ar_result;

} /* pw_fopen_and_check */


#ifdef _NO_PROTO
typedef void (*signalfunction) ();
#else
typedef void (*signalfunction) (int);
#endif

/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*	This procedure ...                                                    */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */
/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       None                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*	 None								      */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       None                                                                 */
/*                                                                            */
/******************************************************************************/
static int pw_do_command (argv,inputfile,outputfile,theDisplay)
    char	**argv;
    char	*inputfile;
    char	*outputfile;
    Display 	*theDisplay;
{
    FILE	*fid;
    int		pid;
    fd_set	readfds;
    fd_set	fds;
    int		pw_set_child_done();
    void	(*signal_routine)();
    long	l_ret_status = 1;
    XtAppContext ar_context = XtDisplayToApplicationContext(theDisplay);
    
    FD_ZERO( &fds );
    FD_SET(ConnectionNumber(theDisplay), &fds);

    c_childdone = FALSE;

    signal_routine = signal(SIGCHLD, (signalfunction) pw_set_child_done);
    pid = vfork();

    if (pid == -1) 
    {
	l_ret_status = errno;
	DXMAPPWARNING(ar_context,
		      PWSMSGNAME1,
		      PWGTSNOSUBPR,
		      NULL,NULL);
	return(l_ret_status);
    }

    if (!pid)
    {				/* We're the child process. */
	if (inputfile)
	{
	    fid = pw_fopen_and_check (ar_context, inputfile, "r");
	    (void) dup2(fileno(fid), fileno(stdin));
	}
	if (outputfile)
	{
	    fid = pw_fopen_and_check (ar_context, outputfile, "w");
	    (void) dup2(fileno(fid), fileno(stdout));
	}
	{
	    fid = pw_fopen_and_check (ar_context, "/dev/null", "w");
	    (void) dup2(fileno(fid), fileno(stderr));
	    if (!outputfile)
	    (void) dup2(fileno(fid), fileno(stderr));
	}
	(void)execvp (argv[0], argv);

	/*
	 *  Child should not get here.  If it does, warn the user and
	 *  kill the child.  We aren't using the fatal error-handling
	 *  routine because we just want to abort the child, not the 
	 *  whole application.  
	 */
	{
	    String params[2];
	    Cardinal num_params = 2;
            params[0] = argv[0];
            params[1] = strerror(errno);

	    DXMAPPWARNING(ar_context, 
		          PWSMSGNAME2,
		          PWGTSNOEXECVP,
		          params,&num_params);
            _exit();
        }
    }
    return(l_ret_status);
}


/************************************************************************/
/*									*/
/* pw_set_child_done							*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/* 	This procedure is called when the child subprocess is finished. */
/*	It merely turns on the Boolean "childdone" which allows the 	*/
/*	parent process to fall out of the "while" loop it is in.	*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	None								*/
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
int pw_set_child_done()
{
	c_childdone = TRUE;
}


/************************************************************************/
/*									*/
/* pw_send_files_to_print_queue	(ULTRIX)				*/
/*									*/
/* FUNCTIONAL DESCRIPTION:		                              	*/
/*									*/
/*	Sends given files to print queue.  The print widget is used	*/
/*	to determine the attributes of the job.				*/
/*									*/
/* FORMAL PARAMETERS:							*/
/*									*/
/*	pw		Print Widget					*/
/*									*/
/*      filenames	Files to be printed				*/
/*									*/
/*	filename_count	Number of files to be printed			*/
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
unsigned int pw_send_files_to_print_queue (ar_pw,ar_filenames,l_filename_count)
    DXmPrintWgt	ar_pw;
    Opaque	ar_filenames[];
    int 	l_filename_count;
{
    char		*aat_big_vector[30];    /* a vector of pointers to string.		*/
    char		**aat_argv;
    char		*at_bigbuf;

    Opaque		r_temp_os_string;
    Opaque		r_temp_os_string2;

    long		l_size,l_status;
    
    unsigned int	l_ret_status;		/* status of routine calls 			*/
    int			l_i;			/* loop index 					*/
    int			l_ac = 0;		/* count of argv entries used.                  */
    char 		*at_str1;		/* temporary string pointer.                    */
    Display 		*ar_theDisplay;		/* poniter to the display the printwgt is on.   */
    char 		*at_filename = NULL;

    aat_argv = aat_big_vector;

    at_bigbuf = (char *) XtMalloc(1000);

    /********************************************************************/
    /*									*/
    /* The lpr command							*/
    /*									*/
    /********************************************************************/
    aat_argv[l_ac] = "lpr";	l_ac++;

    at_str1 = at_bigbuf;

    /********************************************************************/
    /*									*/
    /* Printer Name							*/
    /* 									*/
    /********************************************************************/
    sprintf (at_str1, "-P%s", PWr_os_printer_choice(ar_pw));
    aat_argv[l_ac] = at_str1;	

    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);

    l_ac++;

    /********************************************************************/
    /*									*/
    /* File Burst Sheet							*/
    /* 									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_FILE_BURST_SHEET_MAP).l_current_value == DXmFILE_SHEET_NONE)
    {  
	aat_argv[l_ac] =  "-h";
	l_ac++;
    }

    /********************************************************************/
    /*									*/
    /* Header								*/
    /* 									*/
    /********************************************************************/
    if (PWr_boolean_res(ar_pw,K_PW_HEADER_MAP).b_current_value)
    {  
	aat_argv[l_ac] =  "-p";
	l_ac++;
    }

    /********************************************************************/
    /*									*/
    /* Notify								*/
    /* 									*/
    /********************************************************************/
    if (PWr_boolean_res(ar_pw,K_PW_NOTIFY_MAP).b_current_value)
    { 
	aat_argv[l_ac] = "-m";
        l_ac++;
    }

    /********************************************************************/
    /*									*/
    /* Delete file							*/
    /* 									*/
    /********************************************************************/
    if (PWr_boolean_res(ar_pw,K_PW_DELETE_FILE_MAP).b_current_value)
    {
	aat_argv[l_ac] = "-r";
	l_ac++;
    };

    /********************************************************************/
    /*									*/
    /* Pass All								*/
    /* 									*/
    /********************************************************************/
    if (PWr_boolean_res(ar_pw,K_PW_PASS_ALL_MAP).b_current_value)
    {
	aat_argv[l_ac] = "-x";
	l_ac++;
    }

    /********************************************************************/
    /*									*/
    /* Number Copies							*/
    /* 									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_NUMBER_COPIES_MAP).l_current_value)
    {
	sprintf (at_str1, "-#%d",
		 PWr_int_res(ar_pw,K_PW_NUMBER_COPIES_MAP).l_current_value);

	aat_argv[l_ac] = at_str1;
	at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	l_ac++;
    }

    /********************************************************************/
    /*									*/
    /* Job Name								*/
    /* 									*/
    /********************************************************************/
    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_JOB_NAME_MAP).ar_current_cs,
                                             &l_size,
                                             &l_status);
    
    if (r_temp_os_string && strlen(r_temp_os_string))
    {
	sprintf (at_str1,"-J%s",r_temp_os_string);
	
	aat_argv[l_ac] = at_str1;
	at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	l_ac++;
    }
    
    if (r_temp_os_string)
	XtFree(r_temp_os_string);
    
    /********************************************************************/
    /*									*/
    /* Start Sheet Comment						*/
    /* 									*/
    /********************************************************************/
    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_START_SHEET_COMMENT_MAP).ar_current_cs,
                                             &l_size,
                                             &l_status);
    
    if (r_temp_os_string && strlen(r_temp_os_string))
    {
	sprintf (at_str1,"-C%s",r_temp_os_string);
	
	aat_argv[l_ac] = at_str1;
	at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	l_ac++;
    }

    if (r_temp_os_string)
	XtFree(r_temp_os_string);
    
    /********************************************************************/
    /*									*/
    /* Print Format							*/
    /* 									*/
    /********************************************************************/
#if !defined(__osf__)

    /* The -D lpr option is  not yet implemented on OSF/1 */

    if (PWl_print_format_choice(ar_pw))
    {
	r_temp_os_string = (Opaque)
	    DXmCvtCStoOS(PWr_cs_list_res(ar_pw,
					 K_PW_KNOWN_FORMAT_OS_LIST_MAP).ar_cs_list[PWl_print_format_choice(ar_pw)],
			 &l_size,
			 &l_status);

	sprintf (at_str1,"-D%s",r_temp_os_string);
	
	aat_argv[l_ac] = at_str1;
	at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	l_ac++;
    
	XtFree(r_temp_os_string);
    }

#endif
    
    /********************************************************************/
    /*									*/
    /* Input Tray							*/
    /* 									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_INPUT_TRAY_MAP).l_current_value)
    {	
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_INPUT_TRAY_PULLDOWN_MAP,
						      K_PW_INPUT_TRAY_MAP))
	{	
	    sprintf (at_str1,
		     "-I%s",
		     r_temp_os_string);
	
	    aat_argv[l_ac] = at_str1;
	    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	    l_ac++;
	}
    }

    /********************************************************************/
    /*									*/
    /* Output Tray							*/
    /* 									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_OUTPUT_TRAY_MAP).l_current_value)
    {	
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_OUTPUT_TRAY_PULLDOWN_MAP,
						      K_PW_OUTPUT_TRAY_MAP))
	{	
	    sprintf (at_str1,
		     "-o%s",
		     r_temp_os_string);
	
	    aat_argv[l_ac] = at_str1;
	    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	    l_ac++;
	}
    }

    /********************************************************************/
    /*									*/
    /* Orientation							*/
    /* 									*/
    /********************************************************************/
    if (PWr_int_res(ar_pw,K_PW_ORIENTATION_MAP).l_current_value)
    {	
	sprintf (at_str1,
		 "-O%s",
		 rt_orientation_keywords[PWr_int_res(ar_pw,K_PW_ORIENTATION_MAP).l_current_value]);
	
	aat_argv[l_ac] = at_str1;
	at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	l_ac++;
    }

    /********************************************************************/
    /*									*/
    /* Page Size							*/
    /* 									*/
    /********************************************************************/    
    if (PWr_int_res(ar_pw,K_PW_PAGE_SIZE_MAP).l_current_value)
    {	
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_PAGE_SIZE_PULLDOWN_MAP,
						      K_PW_PAGE_SIZE_MAP))
	{	
	    sprintf (at_str1,
		     "-F%s",
		     r_temp_os_string);
	
	    aat_argv[l_ac] = at_str1;
	    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	    l_ac++;
	}
    }

    /********************************************************************/
    /*									*/
    /* Sheet Count							*/
    /* 									*/
    /********************************************************************/    
    if (PWr_int_res(ar_pw,K_PW_SHEET_COUNT_MAP).l_current_value)
    {
	sprintf (at_str1, "-X%d",
		 PWr_int_res(ar_pw,K_PW_SHEET_COUNT_MAP).l_current_value);

	aat_argv[l_ac] = at_str1;
	at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	l_ac++;
    }

    /********************************************************************/
    /*									*/
    /* Page Ranges							*/
    /* 									*/
    /* Page ranges are pretty special in that one or the other can be	*/
    /* NULL.  If one is null, we use the -Znn format.  If both are non	*/
    /* NULL, we use the -Znn,nn format.					*/
    /* 									*/
    /********************************************************************/
    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_FROM_MAP).ar_current_cs,
                              		     &l_size,
                              		     &l_status);
    r_temp_os_string2 = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_PAGE_RANGE_TO_MAP).ar_current_cs,
                                              &l_size,
                                              &l_status);
    
    if ((r_temp_os_string && strlen(r_temp_os_string)) ||
	(r_temp_os_string2 && strlen(r_temp_os_string2)))
    {
	if (!r_temp_os_string || !strlen(r_temp_os_string))
	{	    
	    /* Only "to" was specified */
	    sprintf (at_str1,"-Z,%s",r_temp_os_string2);
	
	    aat_argv[l_ac] = at_str1;
	    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	    l_ac++;
	}
	else
	{
	    if (!r_temp_os_string2 || !strlen(r_temp_os_string2))
	    {	    
		/* Only "from" was specified */
		sprintf (at_str1,"-Z%s",r_temp_os_string);
		
		aat_argv[l_ac] = at_str1;
		at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
		l_ac++;
	    }
	    else
	    {	    
		/* Both were specified */
		sprintf (at_str1,"-Z%s,%s",r_temp_os_string,r_temp_os_string2);
	
		aat_argv[l_ac] = at_str1;
		at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
		l_ac++;
	    }
	}	
    }

    if (r_temp_os_string)
	XtFree(r_temp_os_string);

    if (r_temp_os_string2)
	XtFree(r_temp_os_string2);
    
    /********************************************************************/
    /*									*/
    /* Sheet Size							*/
    /* 									*/
    /********************************************************************/    
    if (PWr_int_res(ar_pw,K_PW_SHEET_SIZE_MAP).l_current_value)
    {	
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_SHEET_SIZE_PULLDOWN_MAP,
						      K_PW_SHEET_SIZE_MAP))
	{	
	    sprintf (at_str1,
		     "-S%s",
		     r_temp_os_string);
	
	    aat_argv[l_ac] = at_str1;
	    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	    l_ac++;
	}
    }

    /********************************************************************/
    /*									*/
    /* Message Log							*/
    /* 									*/
    /********************************************************************/    
    if (PWr_int_res(ar_pw,K_PW_MESSAGE_LOG_MAP).l_current_value)
    {	
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_MESSAGE_LOG_PULLDOWN_MAP,
						      K_PW_MESSAGE_LOG_MAP))
	{
	    sprintf (at_str1,
		     "-M%s",
		     r_temp_os_string);
	
	    aat_argv[l_ac] = at_str1;
	    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	    l_ac++;
	}
    }

    /********************************************************************/
    /*									*/
    /* Number Up							*/
    /* 									*/
    /********************************************************************/    
    if (PWr_int_res(ar_pw,K_PW_NUMBER_UP_MAP).l_current_value)
    {	
	sprintf (at_str1,
		 "-N%d",
		 PWr_int_res(ar_pw,K_PW_NUMBER_UP_MAP).l_current_value);
	
	aat_argv[l_ac] = at_str1;
	at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	l_ac++;
    }

    /********************************************************************/
    /*									*/
    /* Layup Definition							*/
    /* 									*/
    /********************************************************************/
    r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_LAYUP_DEFINITION_MAP).ar_current_cs,
                              		     &l_size,
                              		     &l_status);

    if (r_temp_os_string && strlen(r_temp_os_string))
    {
	sprintf (at_str1,"-L%s",r_temp_os_string);
	
	aat_argv[l_ac] = at_str1;
	at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	l_ac++;
    }

    if (r_temp_os_string)
	XtFree(r_temp_os_string);
    
    /********************************************************************/
    /*									*/
    /* Sides								*/
    /* 									*/
    /********************************************************************/    
    if (PWr_int_res(ar_pw,K_PW_SIDES_MAP).l_current_value)
    {	
	if (r_temp_os_string = PW_GET_OS_TABLE_STRING(ar_pw,
						      K_PW_SIDES_PULLDOWN_MAP,
						      K_PW_SIDES_MAP))
	{	
	    sprintf (at_str1,
		     "-K%s",
		     r_temp_os_string);
	
	    aat_argv[l_ac] = at_str1;
	    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	    l_ac++;
	}
    }

    /********************************************************************/
    /*									*/
    /* Filenames							*/
    /* 									*/
    /********************************************************************/    
    for (l_i=0; l_i < l_filename_count; l_i++)
    {
	sprintf (at_str1, "%s", ar_filenames[l_i]);
	aat_argv[l_ac] = at_str1;
	at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	l_ac++;
    }

    /********************************************************************/
    /*									*/
    /* Print After							*/
    /*									*/
    /*	If the user has selected a time for print after, we have to	*/
    /*	submit an "at" command rather than an "lpr" command.  Since	*/
    /*	"at" takes a shell script, it is necessary to write the lpr	*/
    /*	command out to a file.						*/
    /*									*/
    /********************************************************************/
    if (!pw_xm_string_compare(PWr_cs_res(ar_pw,K_PW_PRINT_AFTER_MAP).ar_current_cs,
			      PWar_now_string(ar_pw)))
    {		
	r_temp_os_string = (Opaque) DXmCvtCStoOS(PWr_cs_res(ar_pw,K_PW_PRINT_AFTER_MAP).ar_current_cs,
						 &l_size,
						 &l_status);

	if (r_temp_os_string && strlen(r_temp_os_string))
	{
	    FILE	*ar_file;

	    /************************************************************/
	    /*								*/
	    /* Open a temporary file and dump in the lpr command.	*/
	    /*								*/
	    /************************************************************/
	    at_filename = tempnam(NULL,"print");
	    ar_file = fopen(at_filename,"w");
	    if (ar_file == NULL)
	    {
		String params = at_filename;
		Cardinal num_params = 1;

		l_ret_status = errno;
		DXMAPPWARNING(XtWidgetToApplicationContext((Widget)ar_pw),
			      PWSMSGNAME0,
			      PWGTSNOFILOP,
			      &params,&num_params);
		return(l_ret_status);
	    }
	
	    for (l_i = 0; l_i < l_ac; l_i++)
		fprintf(ar_file,"%s ",aat_argv[l_i]);
	    fprintf(ar_file,"\n");

	    /************************************************************/
	    /*								*/
	    /* Reset the arg list to be the "at" command.		*/
	    /*								*/
	    /************************************************************/
	    l_ac = 0;
	    at_str1 = at_bigbuf;
	
	    aat_argv[l_ac] = "at";
	    l_ac++;
	
	    sprintf (at_str1, "%s", r_temp_os_string);
	    aat_argv[l_ac] = at_str1;
	    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	    l_ac++;
	 
	    sprintf (at_str1, "%s", at_filename);
	    aat_argv[l_ac] = at_str1;
	    at_str1 = at_str1 + 1 + strlen(aat_argv[l_ac]);
	    l_ac++;
	 
	    /************************************************************/
	    /*								*/
	    /* Close the file.						*/
	    /*								*/
	    /************************************************************/
	    fclose(ar_file);
	}
    
	if (r_temp_os_string)
	    XtFree(r_temp_os_string);
    }
    
    /********************************************************************/
    /*									*/
    /* Finally submit the job.						*/
    /*									*/
    /********************************************************************/
    aat_argv[l_ac] = NULL;		/* force last entry to be NULL        */

    ar_theDisplay = XtDisplay(ar_pw);
    l_ret_status = pw_do_command (aat_argv, NULL, NULL, ar_theDisplay);

    /********************************************************************/
    /*									*/
    /* Free up some memory and get rid of the temp file if we made one	*/
    /*									*/
    /********************************************************************/
    XtFree(at_bigbuf);

    if (at_filename)
    {
	remove(at_filename);
	free(at_filename);
    }
    
    return(l_ret_status);

}  /* pw_send_files_to_print_queue (ULTRIX) */
#endif

#ifdef VMS
unsigned int pw_send_files_to_kprint ( w,
				      a_file_names,
				      count_of_file_names)
/******************************************************************************/
/*                                                                            */
/*   FUNCTIONAL DESCRIPTION:                                                  */
/*                                                                            */
/*	Accepts a list of one or more file names to be KPRINTed for printing. */
/*      The name of the queue on which to submit, as well as all other        */
/*      attributes of the print job -- no. of copies, etc. is derived from    */
/*      fields in the print widget whose id is supplied.                      */
/*                                                                            */
/*   FORMAL PARAMETERS:                                                       */
/*                                                                            */
DXmPrintWgt	w;	             /* Widget id of the print widget         */

Opaque		a_file_names[];     /*array of file names to be submitted    */

int 		count_of_file_names; /*count of the number of file names in   */
				     /*array                                  */

/*                                                                            */
/*   IMPLICIT INPUTS:                                                         */
/*                                                                            */
/*       None                                                                 */
/*                                                                            */
/*   IMPLICIT OUTPUTS:                                                        */
/*                                                                            */
/*	Returns status codes returned by KPRINT DCL command                   */
/*                                                                            */
/*   SIDE EFFECTS:                                                            */
/*                                                                            */
/*       None                                                                 */
/*                                                                            */
/******************************************************************************/

{
    register int i, file_name_offset, status = 1;
    char command_string[512];
    unsigned long spawn_flag = 0, process_id, completion_status;
    $DESCRIPTOR(command_string_dsc, command_string);
    $DESCRIPTOR(null_device_dsc, "NL:");

    if (count_of_file_names == 0) {
	printf("printwidget: no file names supplied for submission to print system\n");
	return (FALSE);
    }

    strcpy(command_string, "KPRINT/LN80/QUEUE=");
    strcat(command_string, PWr_os_printer_choice(w));

  {
    char *print_format;
    long len, status;

    print_format = (char *)DXmCvtCStoFC(
                      PWr_cs_res(w,
			         K_PW_PRINT_FORMAT_CHOICE_MAP).ar_current_cs,
                      &len, &status);
    /* BOGUS
     * These strings shouldn't be hardcoded.
     */
    if (strcmp(print_format, "LN80 (Sixel)") == 0)
	strcat(command_string, "/SIXEL");

    if (strcmp(print_format, "LN80 (Text)") == 0 &&
	PWr_int_res(w,K_PW_ORIENTATION_MAP).l_current_value ==
	  DXmORIENTATION_LANDSCAPE)
	strcat(command_string, "/LANDSCAPE");

    XmStringFree(print_format);
  }

    if (PWr_boolean_res(w,K_PW_DELETE_FILE_MAP).b_current_value)
	strcat(command_string, "/DELETE");

    if (PWr_int_res(w,K_PW_NUMBER_COPIES_MAP).l_current_value > 1) {
	char temp[30];

	sprintf(temp, "/COPIES=%d",
		PWr_int_res(w,K_PW_NUMBER_COPIES_MAP).l_current_value);
	strcat(command_string, temp);
    }

    {
	char *low_str, *high_str;
	long l_size, l_status;
	Boolean high_exist, low_exist;

	low_str = DXmCvtCStoOS(
		   PWr_cs_res(w,K_PW_PAGE_RANGE_FROM_MAP).ar_current_cs,
		   &l_size,
		   &l_status);
	high_str = DXmCvtCStoOS(
		    PWr_cs_res(w,K_PW_PAGE_RANGE_TO_MAP).ar_current_cs,
                    &l_size,
                    &l_status);
	low_exist = low_str && strlen(low_str);
	high_exist = high_str && strlen(high_str);

	if (low_exist || high_exist) {
	    char temp[40];

	    strcpy(temp, "/PRINT=(");
	    if (low_exist) {
		strcat(temp, "START=");
		strcat(temp, low_str);
	    }
	    if (low_exist && high_exist)
		strcat(temp, ",");
	    if (high_exist) {
		strcat(temp, "END=");
		strcat(temp, high_str);
	    }
	    strcat(temp, ")");
	    strcat(command_string, temp);
	}
    }

    /*** All qualifiers should be placed here. ***/

    strcat(command_string, " ");
    file_name_offset = strlen(command_string);

#ifdef DEC_MOTIF_MULTIBYTE_DEBUG
    printf("command :  %s\n", command_string);
#endif
    for (i = 0; i < count_of_file_names; i++) {
	strcpy(&command_string[file_name_offset], a_file_names[i]);
	command_string_dsc.dsc$w_length = strlen(command_string);
	completion_status = 1;
	status = LIB$SPAWN(&command_string_dsc,    /* command line string */
			   &null_device_dsc,       /* input device */
			   0,                      /* output device */
			   &spawn_flag,
			   0,
			   &process_id,
			   &completion_status,
			   0,
			   0,
			   0,
			   0,
			   0
	);
	if ((status & 1) != 1)
	    return status;
	else if ((completion_status & 1) != 1)
	    return completion_status;
    }
    return completion_status;
}
#endif /* VMS */
