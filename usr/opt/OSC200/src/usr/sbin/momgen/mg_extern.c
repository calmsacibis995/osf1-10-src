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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: mg_extern.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:54:59 $";
#endif
/*
**+
**  Copyright (c) Digital Equipment Corporation, 1991
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
**++
**  FACILITY:  VMS Framework Services VMS MOM Generator
**
**  MODULE DESCRIPTION:
**
**	This module contains the routines to used to generate extern and 
**	import declarations.
**
**  AUTHORS:
**
**      Gary Allison
**
**	Some routines were extracted from BUILD.C (Rich Bouchard, Jr.) to 
**	form this module.
**
**  CREATION DATE:  23-Sep-1991
**
**  MODIFICATION HISTORY:
**
**	F-17		    Marc Nozell	    	    15-Jun-1992
**	Remove references to module.
**	Update edit history.
**
**  	F-16 	    	    Gary Allison    	    12-JUN-1992
**
**  	F-15 	    	    Gary Allison    	    28-MAY-1992
**  	Merge in ported modules
**
**  	F-14 	    	    Gary Allison    	    19-MAY-1992
**
**  	F-13 	    	    Gary Allison    	    9-APR-1992
**
**  	F-12 	    	    Gary Allison    	    16-MAR-1992
**
**  	F-11 	    	    Gary Allison    	    21-FEB-1992
**
**  	F-10 	    	    Gary Allison    	    14-FEB-1992
**
**  	F-9 	    	    Gary Allison    	    12-FEB-1992 
**  	Fix bug in insert_Module_name
**
**  	F-8 	    	    Gary Allison    	    11-FEB-1992
**  	Add # to generated pragmas
**
**  	F-7	    	    Gary Allison    	    7-FEB-1992
**
**  	F-6 	    	    Gary Allison    	    22-JAN-1992
**  	Update for error handling
**
**  	F-5 	    	    Gary Allison    	    9-JAN-1992
**
**	F-4		    Mike Densmore	    10-Jun-1992
**
**	added support for an snmp specific build_perform_get
**
**	F-3		    Mike Densmore	    21-May-1992
**
**	Modified for use on U**X platforms.
**
**	F-2		    Marc Nozell		    24-May-1991
**                            
**	Support multiple classes.
**
**	25-Sep-1992 Mike Densmore   Modified for ANSI Standard C
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#include "mom_defs.h"
#include <string.h>
#include "file_defs.h"
#include "mg_prototypes.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_external_routines
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on tag insert-code-external-routines.
**
**	This routine generates the "extern" declarations for the
**	various class-specific routines.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
int insert_external_routines( mom,
			      outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    sprintf(buffer,
   	    "    man_status (*perform_%s)();\n",
	    mom->temp);
    fputs(buffer, outfile);

    fputs("    int class_code;\n", outfile);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_define_protos
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-define-protos tag.
**
** 	This routine generates code for function prototypes.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
int insert_define_protos( mom,
			  outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;
    int		    action_num;	
    ACTION_DEF	    *action;
    EVENT_DEF       *event;
    int             event_num;

#ifdef UNIXPLATFORM

    fputs("man_status init_trap PROTOTYPE(());\n\n",outfile);

    /*
     * Generate the prototypes for send_<trap_name>_trap() routines.
     * Note that event_name starts with "EVT_".
     */

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
        event = class->event;

        for (event_num=0; event_num < class->num_events; event_num++)
            {
            fputs("man_status send_", outfile);
            fputs((char *)&event->event_name[strlen("EVT_")], outfile);
            fputs("_trap PROTOTYPE(());\n\n", outfile);

            event = event->next;
            }

        class = class->next;
        }

#endif

    if (mom->multiclass)
	  {
	  fputs("man_status get_instance PROTOTYPE((\n",outfile);
	  fputs("	object_id	    *,\n",outfile);
	  fputs("	avl		    *,\n",outfile);
	  fputs("	scope		     ,\n",outfile);
	  fputs("	void	   	   **,\n",outfile);
	  fputs("	avl                **,\n",outfile);
	  fputs("	uid		   **,\n",outfile);
	  fputs("	int		    *,\n",outfile);
	  fputs("	GET_CONTEXT_STRUCT **,\n",outfile);
	  fputs("	int		     ,\n",outfile);
	  fputs("	int		      ));\n\n",outfile);
	  }
    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
	sprintf(buffer,"man_status %sadd_new_instance PROTOTYPE(( \n", class->prefix_m );
	fputs(buffer,outfile);
	sprintf(buffer,"	%s_DEF      *,\n", class->class_name );
	fputs(buffer,outfile);
	sprintf(buffer,"	%s_DEF      * ));\n\n", class->class_name );
	fputs(buffer,outfile);

	sprintf(buffer,"man_status %sget_next_match PROTOTYPE(( \n", class->prefix_m );
	fputs(buffer,outfile);
	fputs(" avl                 *,\n",outfile);
	fputs(" scope                ,\n",outfile);
	fputs("	GET_CONTEXT_STRUCT  *));\n\n",outfile);

	sprintf(buffer,"man_status %slocate_instance PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs(" avl                 *,\n",outfile);
	fputs("	char		    *,\n",outfile);
	fputs(" int		     ,\n",outfile);
	sprintf(buffer,"	%s_DEF	   ** ));\n\n", class->class_name );
	fputs(buffer,outfile);

	sprintf(buffer,"man_status %sget_attr_oid PROTOTYPE((\n", class->prefix_m);
	fputs(buffer,outfile);
	fputs(" object_id *,\n",outfile);
	fputs(" unsigned int *));\n\n",outfile);

	sprintf(buffer,"man_status %sget_action_oid PROTOTYPE((\n", class->prefix_m);
	fputs(buffer,outfile);
	fputs(" object_id *,\n",outfile);
	fputs(" unsigned int *));\n\n",outfile);

	sprintf(buffer,"man_status %sget_instance PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs("	object_id	    *,\n",outfile);
	fputs("	avl		    *,\n",outfile);
	fputs("	scope		     ,\n",outfile);
	sprintf(buffer,"	%s_DEF	   **,\n", class->class_name );
	fputs(buffer,outfile);
	fputs("	avl                **,\n",outfile);
	fputs("	uid		   **,\n",outfile);
	fputs("	int		    *,\n",outfile);
	fputs("	GET_CONTEXT_STRUCT **,\n",outfile);
	fputs("	int		      ));\n\n",outfile);

	sprintf(buffer,"man_status %scheck_all_attrs PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs("	avl 		    *, \n",outfile);
	fputs("	avl 		    * ));\n\n",outfile);

	sprintf(buffer,"man_status  %sperform_create PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs("	avl		    *,\n",outfile);
	fputs(" octet_string	    *,\n",outfile);
	fputs("	char 		    *,\n",outfile);
	fputs("	int                  ,\n",outfile);
	sprintf(buffer,"	%s_DEF	   **,\n", class->class_name );
	fputs(buffer,outfile);
	fputs("	avl		    *,\n",outfile);
	fputs("	avl		   **));\n\n",outfile);
	
	sprintf(buffer,"man_status  %sperform_delete PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs("	object_id	    *,\n",outfile);
	fputs("	avl		    *,\n",outfile);
	sprintf(buffer,"	%s_DEF	    *,\n", class->class_name );
	fputs(buffer,outfile);
	fputs("	avl		   **,\n",outfile);
	fputs("	reply_type	    *,\n",outfile);
	fputs("	object_id	   ** ));\n\n",outfile);
	
	sprintf(buffer,"man_status  %sperform_get PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs("	object_id           *,\n",outfile);
	fputs("	avl	            *,\n",outfile);
	fputs("	void		    *,\n",outfile);
	fputs("	avl		   **,\n",outfile);
	fputs("	reply_type          *,\n",outfile);
	fputs("	object_id	   ** ));\n\n",outfile);
	
        sprintf(buffer,"man_status  %sperform_getnext PROTOTYPE((\n", class->prefix_m );
        fputs(buffer,outfile);
        fputs(" object_id           *,\n",outfile);
        fputs(" avl                 *,\n",outfile);
        fputs(" avl                 **,\n",outfile);
        fputs(" scope                ,\n",outfile);
        fputs(" avl                **,\n",outfile);
        fputs(" reply_type          *,\n",outfile);
        fputs(" object_id          ** ));\n\n",outfile);

	sprintf(buffer,"man_status  %sperform_init PROTOTYPE(());\n\n", class->prefix_m );
	fputs(buffer,outfile);
	
	sprintf(buffer,"man_status  %sattr_writeable PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs("	object_id	    * ));\n\n",outfile);
	
	sprintf(buffer,"man_status  %scheck_attr_value PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs("	object_id	    *,\n",outfile);
	fputs("	unsigned int    ,\n",outfile);
	fputs("	unsigned int    ,\n",outfile);
	fputs("	octet_string	    *,\n",outfile);
	fputs("	avl                 * ));\n\n",outfile);
	
	sprintf(buffer,"man_status  %sperform_set PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs("	object_id	    *,\n",outfile);
	fputs("	avl		    *,\n",outfile);
	fputs("	void		    *,\n",outfile);
	fputs("	avl		   **,\n",outfile);
	fputs("	reply_type	    *,\n",outfile);
	fputs("	object_id	   ** ));\n\n",outfile);
	
	sprintf(buffer,"man_status  %svalidate_instance PROTOTYPE((\n", class->prefix_m );
	fputs(buffer,outfile);
	fputs("	object_id	    *,\n",outfile);
	fputs("	void		    * ));\n\n",outfile);
	
    	action = class->action;
    	for (action_num=0; action_num < class->num_actions; action_num++)
    	    {
	    /* Skip over CREATE (0) and DELETE (1) */
	    if ( (strcmp(action->action_name,"CREATE") != 0) &&
	         (strcmp(action->action_name,"DELETE") != 0) )
		{
		sprintf(buffer,"man_status  %sperform_%s PROTOTYPE((\n", class->prefix_m,action->action_name );
		fputs(buffer,outfile);
		fputs("	object_id	    *,\n",outfile);
		fputs("	avl		    *,\n",outfile);
		fputs("	void		    *,\n",outfile);
		fputs("	avl		   **,\n",outfile);
		fputs("	reply_type	    *,\n",outfile);
		fputs("	object_id	   ** ));\n\n",outfile);
		}
            action = action->next;
            }

	class = class->next;
        }

    return SS$_NORMAL;
}
