/* #module setup.c "X0.0" */
/*
 *  Title:	setup.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Changed private #include file names to all lower case.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 *  Mark Woodbury	25-May-1990 X3.0-3M
 *	- Motif update
 *
 *  Bob Messenger	30-May-1989	X2.0-13
 *	- Change fprintf to log_message so messages get flushed.
 *
 *  Bob Messenger	14-May-1989	X2.0-10
 *	- Change printf to fprintf on stderr.
 *
 *  Bob Messenger	19-Apr-1989	X2.0-6
 *	- Exit with status code on VMS.
 *	- Call process_exit instead of exit.
 *
 *  <modifier's name>	<date>		<ident of revised code>
 *	<description of change and purpose of change>
 *
 */


#include "decterm.h"
#include "mx.h"

#ifdef VMS_DECTERM

globalvalue DECW$_CANT_FETCH_WIDGET;

#else

#define DECW$_CANT_FETCH_WIDGET 0

#endif

/* DRM */
globalref MrmHierarchy s_MRMHierarchy;    /* DRM database id */
globalref MrmType *dummy_class;           /* and class var */

      
/* SetupInit */
Widget SetupInit(parent, isn)
Widget parent;
ISN isn;
{
    Widget menubar;

    /* fetch application menubar from DRM */
    if (MrmFetchWidget
        (
        s_MRMHierarchy,
        "decterm_mb",
        parent,
        & menubar,
        & dummy_class
        )
      != MrmSUCCESS)
        {
        log_error( "Unable to fetch menubar widget from DRM\n");
        process_exit( DECW$_CANT_FETCH_WIDGET );
        }

    return ( menubar );
}	
