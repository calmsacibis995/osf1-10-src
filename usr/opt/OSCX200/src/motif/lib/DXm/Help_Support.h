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
***********************************************************
**                                                        *
**  Copyright (c) Digital Equipment Corporation, 1990  	  *
**  All Rights Reserved.  Unpublished rights reserved	  *
**  under the copyright laws of the United States.	  *
**                                                        *
**  The software contained on this media is proprietary	  *
**  to and embodies the confidential technology of 	  *
**  Digital Equipment Corporation.  Possession, use,	  *
**  duplication or dissemination of the software and	  *
**  media is authorized only pursuant to a valid written  *
**  license from Digital Equipment Corporation.	    	  *
**  							  *
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	  *
**  disclosure by the U.S. Government is subject to	  *
**  restrictions as set forth in Subparagraph (c)(1)(ii)  *
**  of DFARS 252.227-7013, or in FAR 52.227-19, as	  *
**  applicable.	    					  *
**  		                                          *
***********************************************************
**++
**  Subsystem:
**	DXmHelp 
**
**  Version: V1.0
**
**  Abstract:
**	Data structures for the Help Support routines
**
**  Keywords:
**	DXmHelp, widget, DECwindows
**
**  Environment:
**	User mode, executable image
**
**  Author:
**	Andre Pavanello, CASEE Group, MEMEX Project
**
**  Creation Date: 24-Mar-88
**
**  Modification History:
**--
*/
#ifndef _help_support_h_  /* Check if this file has already been included */
#define _help_support_h_
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

/*
**  Macro Definitions
*/
#ifndef TRUE
#define TRUE	(1==1)
#define FALSE	(1==0)
#endif

/*
**  Type Definitions
*/
typedef struct _widget_id_list
	    {
	    Widget  widget_id;
	    int	    mapped;
	    struct _widget_id_list *next;
	    } WIDGET_ID_LIST, *WIDGET_ID_LIST_PTR;

typedef struct _help_context_block
	    {
	    WIDGET_ID_LIST  *help_list;
	    Widget	    parent_id;
	    int		    library_type;
	    XmString   	    application,
			    library,
			    overview,
			    glossary;
	    } HELP_CONTEXT_BLOCK, *HELP_CONTEXT_BLOCK_PTR;
	    
#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif
