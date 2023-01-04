/* #module <module name> "X0.0" */
/*
 *  Title:	<module name>
 *
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
 *  Alfred von Campe    08-Oct-1992     Ag/BL10
 *      - Small #include change to satisfy Alpha compiler.
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Made small changes to make OSF compiler/linker happy.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Mark Woodbury	25-May-1990 X3.0-3M
 * - Motif update
 *
 * Bob Messenger	22-Apr-1989	X2.0-6
 *	- Avoid compiler warning by defining dummy routine on non-VMS
 *	  systems.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Make static data readonly on VMS, so this module can be part
 *	  of the shareable library.
 *	- Move find_top_level_widget to menu_stubs.c and dwt_get_optimal_size
 *	  to ptysub.c (where it can be commented out, since the only place
 *	  it's called has been deleted).
 *
 * Tom Porcher		 7-Apr-1988	X0.4-7
 *	- Made dwt_get_optimal_size() use XtQueryGeometry() rather than
 *	  optimal_size_proc().
 *
 */

#if defined (VMS_DECTERM) || defined (VXT_DECTERM)
#include "MrmAppl.h"
#else
#include <Mrm/MrmAppl.h>
#endif

/*
   NOTE: this declaration was stolen from the XUI dwtwidget.h file.  It is
   nolonger used in the Motif version.  In the interest of time this allows
   the code here to remain the same.  When time allows this shouyld be taken
   out, and the pointers fixed up so the two callback structures can be 
   compared.

 * The DECtoolkit internal callback structure which maps the
 * external representation to the intrinsics internal representation
 */
typedef struct
{
    XtCallbackList	ecallback;	/* Xtoolkit callback list */
}
    DwtCallbackStruct, *DwtCallbackStructPtr;

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
readonly globaldef
#endif
int	dpy_event_flag_cluster = 1,
	dpy_table_mask = ~0;

void  DecTermUpdateCallback (r, rstruct, s, sstruct, argname)
    Widget r;                                   /* the real widget*/
    DwtCallbackStructPtr rstruct;               /* the real callback list */
    Widget s;                                   /* the scratch widget*/
    DwtCallbackStructPtr sstruct;               /* the scratch callback list */
    char           *argname;
{

    XtCallbackList list;

    /*
     * if a new callback has been specified in the scratch widget,
     * remove and deallocate old callback and init new
     */
    if (rstruct->ecallback != sstruct->ecallback)
    {
        XtRemoveAllCallbacks(r, argname);
        list = (XtCallbackList)sstruct->ecallback;
        sstruct->ecallback = NULL;
        XtAddCallbacks(s, argname, list);
    }
}
