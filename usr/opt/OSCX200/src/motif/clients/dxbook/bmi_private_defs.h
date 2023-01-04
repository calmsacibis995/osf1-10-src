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
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_PRIVATE_DEFS.H*/
/* *3     3-MAR-1992 17:07:36 KARDON "UCXed"*/
/* *2     1-NOV-1991 12:48:11 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:47:48 PARMENTER "Private LinkWorks definitions"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_PRIVATE_DEFS.H*/
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
*/

/*
**++
**  FACILITY:
**
**      Bookreader Memex Interfaces Interface (bmi*)
**
**  ABSTRACT:
**
**	Private definitions for the Bookreader Memex Interface
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     02-Jul-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0003 DLB0003     David L Ballenger           30-Apr-1991
**           Fix problems with surrogates in the library window.
**
**  V03-0002 DLB0002     David L Ballenger           09-Apr-1991
**           Add support of select destination.
**
**  V03-0001 DLB0001     David L Ballenger           01-Mar-1991
**           Fix problems with surrogate highlighting after the composite
**           network changes for QAR 807.  LinkWorks naming convention 
**           changes.
**
**--
**/

#ifndef BMI_PRIVATE_DEFS_H
#define BMI_PRIVATE_DEFS_H

#include <stdlib.h>
#include "br_prototype.h"
#include "br_common_defs.h"
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"
#include "br_globals.h"      /* BR external variables declared here */


/*
 * Macro Definitions
 */

#ifdef  MEMEX_DEBUG
#define TRACE(routine_name) fprintf(stderr,"Entering routine %s\n",routine_name);
#define PRINT_STATUS(msg,status) \
        { \
            lwk_string status_string; \
            lwk_status_to_string(status,&status_string); \
            fprintf(stderr,"%s: status = %s\n",msg,status_string); \
        }
#else
#define PRINT_STATUS(msg,status)
#define TRACE(routine_name)
#endif


#define RETURN_ON_ERROR(expression) \
        { \
            lwk_status return_status = (expression); \
            if (return_status != lwk_s_success) { \
                PRINT_STATUS("Returning from routine",return_status); \
                return return_status; \
            } \
        } 

#define DELETE_AND_RETURN_ON_ERROR(expression,object) \
        { \
            lwk_status return_status = (expression); \
            if (return_status != lwk_s_success) {  \
                PRINT_STATUS("Returning from routine",return_status); \
                (void)lwk_delete(&object); \
                return return_status; \
            } \
        } 

#define HIGHLIGHT_STATE(ui,list) \
        ((list) ? bmi_check_highlighting(ui,list) : FALSE)

#define HIGHLIGHTING_ON(wc) (wc->highlighting & lwk_c_hl_on)
#define HIGHLIGHT(wc,flags) \
        (HIGHLIGHTING_ON(wc) && (wc->highlighting & (flags)))

/* Define macros for getting properties from lwk_objects
 */
#define GET_INTEGER(obj,prop,addr) \
        lwk_get_value(obj,prop,lwk_c_domain_integer,addr)
#define GET_STRING(obj,prop,addr) \
        lwk_get_value(obj,prop,lwk_c_domain_string,addr)
#define GET_HIGHLIGHTING(w) \
        GET_INTEGER((w)->memex_ui,lwk_c_p_appl_highlight,&(w)->highlighting)
#define GET_NETWORK(obj) \
        lwk_get_value(obj,lwk_c_p_recording_linknet,lwk_c_domain_linknet,\
                      &bmi_context.current_network)
#define GET_COMPOSITE_NET(obj) \
        lwk_get_value(obj,lwk_c_p_active_comp_linknet,lwk_c_domain_comp_linknet,\
                      &bmi_context.current_composite_net)


/* Define macros for settting properties
 */
#define SET_INTEGER(obj,prop,addr) \
        lwk_set_value(obj,prop,lwk_c_domain_integer,addr,lwk_c_set_property)
#define SET_STRING(obj,prop,addr) \
        lwk_set_value(obj,prop,lwk_c_domain_string,addr,lwk_c_set_property)
#define SET_CSTRING(obj,prop,addr) \
        lwk_set_value(obj,prop,lwk_c_domain_ddif_string,addr,lwk_c_set_property)


typedef struct _BMI_GLOBAL_CONTEXT {
    lwk_boolean          initialized;
    lwk_boolean          hyperinvoked;
    lwk_string           operation;
    lwk_surrogate        surrogate;
    Pixmap               highlight_icon;
    int                  highlight_icon_width;
    int                  highlight_icon_height;
    Boolean              composite_net_valid;
    Boolean              select_destination;
    lwk_composite_linknet    current_composite_net;
    BMI_NETWORK_LIST_PTR active_networks;
} BMI_GLOBAL_CONTEXT ;

extern BMI_GLOBAL_CONTEXT bmi_context;
#endif 



