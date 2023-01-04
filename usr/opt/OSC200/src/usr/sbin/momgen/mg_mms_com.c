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
static char *rcsid = "@(#)$RCSfile: mg_mms_com.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:55:29 $";
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
**	This module contains the routines to used to build the MMS file and
**	the DCL command file used to build the MOM.
**
**  AUTHORS:
**
**      Rich Bouchard, Jr
**
**  CREATION DATE:  23-Sep-1991
**
**  MODIFICATION HISTORY:
**
**	F-11		    Marc Nozell		    15-Jun-1992
**	Remove references to module.
**
**	F-3		    Gary Allison	    23-Sep-1991
**
**	Add new routines, restructure module.  Support instance data.
**	Build command file.
**
**	F-2		    Marc Nozell		    24-May-1991
**                            
**	Support multiple classes.
**
**	25-Sep-1992	Mike Densmore	Modified for ANSI Standard C
**
**	 3-Mar-1993	M. Ashraf	Updated for IPC changes
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#include "mom_defs.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_descrip_cd
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-descrip-cd.
**
**	This routine generates the names of the perform create, delete, get, set,
**	modules in the obj$:mom.exe dependency graph.
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
int insert_descrip_cd( mom,
		       outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    char	buffer[256];
    CLASS_DEF   *class;
    int		class_num;
    momgen_status status;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
	    sprintf(buffer,"        %sfind_instance=obj$:%sfind_instance.obj -\n", class->prefix_m, class->prefix_m);
	    fputs(buffer, outfile);                           
       
	if (mom->support_create)
	    {
	    sprintf(buffer,"        %sperform_create=obj$:%sperform_create.obj -\n", class->prefix_m, class->prefix_m);
	    fputs(buffer, outfile);                           
	    }
	if (mom->support_delete)
	    {
	    sprintf(buffer,"        %sperform_delete=obj$:%sperform_delete.obj -\n", class->prefix_m, class->prefix_m);
	    fputs(buffer, outfile);                           
	    }
	if (mom->support_get)
	    {
	    sprintf(buffer,"        %sperform_get=obj$:%sperform_get.obj  -\n", class->prefix_m, class->prefix_m);
	    fputs(buffer, outfile);                           
	    }
	if (mom->support_set)
	    {
	    sprintf(buffer,"        %sperform_set=obj$:%sperform_set.obj -\n", class->prefix_m, class->prefix_m);
	    fputs(buffer, outfile);                           
	    }
	class = class->next;
	}
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_descrip_cd_dependency
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-descrip-cd-dependency
**
**	This routine generates the names of the perform create, delete, get,
**	and set routines in their dependency graph.
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
int insert_descrip_cd_dependency( mom,
			          outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    char	buffer[256];
    CLASS_DEF   *class;
    int		class_num;
    momgen_status status;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
	    sprintf(buffer,"obj$:%sfind_instance.obj	: src$:%sfind_instance.c src$:common.h src$:mom_prototypes.h\n", 
				class->prefix_m, class->prefix_m); 
	    fputs(buffer, outfile);                           
	if (mom->support_create)
	    {
	    sprintf(buffer,"obj$:%sperform_create.obj	: src$:%sperform_create.c src$:common.h src$:mom_prototypes.h\n", 
				class->prefix_m, class->prefix_m); 
	    fputs(buffer, outfile);                           
	    }
	if (mom->support_delete)
	    {
	    sprintf(buffer,"obj$:%sperform_delete.obj	: src$:%sperform_delete.c src$:common.h src$:mom_prototypes.h\n", 
				class->prefix_m, class->prefix_m);
	    fputs(buffer, outfile);                           
	    }
	if (mom->support_get)
	    {
	    sprintf(buffer,"obj$:%sperform_get.obj	: src$:%sperform_get.c src$:common.h src$:mom_prototypes.h\n", 
				class->prefix_m, class->prefix_m); 
	    fputs(buffer, outfile);                           
	    }
	if (mom->support_set)
	    {
	    sprintf(buffer,"obj$:%sperform_set.obj	: src$:%sperform_set.c src$:common.h src$:mom_prototypes.h\n", 
				class->prefix_m, class->prefix_m);
	    fputs(buffer, outfile);                           
	    }

	class = class->next;
	}
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_descrip_obj_action
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-descrip-obj-action.
**
**	This routine generates the names of the perform action routines
**	in the obj$:mom.exe dependency graph.
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
int insert_descrip_obj_action( mom,
			       outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    char	buffer[256];
    CLASS_DEF   *class;
    int		class_num;
    int		action_num;
    ACTION_DEF	*action;
    momgen_status status;

    if (!mom->support_action)
        return SS$_NORMAL;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
    	action = class->action;
    	for (action_num=0; action_num < class->num_actions; action_num++)
    	    {
	    if ( (strcmp(action->action_name,"CREATE") != 0) &&
	         (strcmp(action->action_name,"DELETE") != 0) )
	        {
		sprintf(buffer,"        %sperform_%s=obj$:%sperform_%s.obj -\n", class->prefix_m,action->action_name,class->prefix_m,action->action_name);
		fputs(buffer, outfile);                           
		}
	    action = action->next;
            }
	class = class->next;
	}
    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_descrip_dependency
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-descrip-dependency
**
**	This routine generates the names of the perform action routines
**	in their dependency graph.
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
int insert_descrip_dependency( mom,
			       outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    char	buffer[256];
    CLASS_DEF   *class;
    int		class_num;
    int		action_num;
    ACTION_DEF	*action;
    momgen_status status;

    if (!mom->support_action)
        return SS$_NORMAL;

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
    	action = class->action;
    	for (action_num=0; action_num < class->num_actions; action_num++)
    	    {
	    if ( (strcmp(action->action_name,"CREATE") != 0) &&
	         (strcmp(action->action_name,"DELETE") != 0) )
	        {
		sprintf(buffer,"obj$:%sperform_%s.obj	: src$:%sperform_%s.c  src$:common.h src$:mom_prototypes.h\n", 
			class->prefix_m,action->action_name,
			class->prefix_m,action->action_name);
		fputs(buffer, outfile);                           
		}
	    action = action->next;
            }
	class = class->next;
	}
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_commands
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-commands tag.
**
**	This routine generates the names of the perform create, delete and action
**	routines in the DCL command file.
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
int insert_commands( mom,
		     outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ACTION_DEF 	    *action;
    int		    action_num;
    int 	    class_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    char	    *buff;
    int		    status;

    buff = (char *) malloc(12);

    switch (mom->oid_type) 
	{
	case DNA_CMIP_OID :
	    buff = DNA_CMIP_OID_STR;
	    break;
	case OSI_CMIP_OID :
	    buff = OSI_CMIP_OID_STR;
 	    break;
	case SNMP_OID :
	    buff = SNMP_OID_STR;
	    break;                 
	case OID :
	    buff = OID_STR;
	    break;
	    }	

    class = mom->class;
    for (class_num=0; class_num < mom->num_classes; class_num++)
        {
	    sprintf(buffer,"$cc/debug/define=(MOMGENDEBUG,%s)/list=lis$:/obj=obj$:/mach/noopt/include=(sys$library,src$) src$:",buff);
	    fputs(buffer, outfile);
	    sprintf(buffer,"%sfind_instance.c\n", class->prefix_m);
	    fputs(buffer, outfile);                           

	    fputs("$library obj$:mom obj$:",outfile);
	    sprintf(buffer,"%sfind_instance\n", class->prefix_m);
	    fputs(buffer, outfile);                           

	if (mom->support_create)
	    {
	    sprintf(buffer,"$cc/debug/define=(MOMGENDEBUG,%s)/list=lis$:/obj=obj$:/mach/noopt/include=(sys$library,src$) src$:",buff);
	    fputs(buffer, outfile);
	    sprintf(buffer,"%sperform_create.c\n", class->prefix_m);
	    fputs(buffer, outfile);                           

	    fputs("$library obj$:mom obj$:",outfile);
	    sprintf(buffer,"%sperform_create\n", class->prefix_m);
	    fputs(buffer, outfile);                           
	    }
	if (mom->support_delete)
	    {
	    sprintf(buffer,"$cc/debug/define=(MOMGENDEBUG,%s)/list=lis$:/obj=obj$:/mach/noopt/include=(sys$library,src$) src$:",buff);
	    fputs(buffer, outfile);
	    sprintf(buffer,"%sperform_delete.c\n", class->prefix_m);
	    fputs(buffer, outfile);                           

	    fputs("$library obj$:mom obj$:",outfile);
	    sprintf(buffer,"%sperform_delete\n", class->prefix_m);
	    fputs(buffer, outfile);                           
	    }

    	action = class->action;
	if (mom->support_action)
    	  for (action_num=0; action_num < class->num_actions; action_num++)
    	    {
	    if ( (strcmp(action->action_name,"CREATE") != 0) &&
	         (strcmp(action->action_name,"DELETE") != 0) )
	        {
    		sprintf(buffer,"$cc/debug/define=(MOMGENDEBUG,%s)/list=lis$:/obj=obj$:/mach/noopt/include=(sys$library,src$) src$:",buff);
	        fputs(buffer, outfile);
		sprintf(buffer,"%sperform_%s.c\n", class->prefix_m,action->action_name);
		fputs(buffer, outfile);                           

	        fputs("$library obj$:mom obj$:",outfile);
		sprintf(buffer,"%sperform_%s\n", class->prefix_m,action->action_name);
	        fputs(buffer, outfile);                           

		}
	    action = action->next;
            }

    	sprintf(buffer,"$cc/debug/define=(MOMGENDEBUG,%s)/list=lis$:/obj=obj$:/mach/noopt/include=(sys$library,src$) src$:",buff);
	fputs(buffer, outfile);
	sprintf(buffer, "%sget_instance.c\n", class->prefix_m );
	fputs(buffer, outfile);

        fputs("$library obj$:mom obj$:",outfile);
	sprintf(buffer, "%sget_instance\n", class->prefix_m );
	fputs(buffer, outfile);                           

	if (mom->support_get)
	    {
    	    sprintf(buffer,"$cc/debug/define=(MOMGENDEBUG,%s)/list=lis$:/obj=obj$:/mach/noopt/include=(sys$library,src$) src$:",buff);
	    fputs(buffer, outfile);
	    sprintf(buffer, "%sperform_get.c\n", class->prefix_m );
	    fputs(buffer, outfile);

	    fputs("$library obj$:mom obj$:",outfile);
	    sprintf(buffer, "%sperform_get\n", class->prefix_m );
	    fputs(buffer, outfile);                           
	    }
	if (mom->support_set)
	    {
    	    sprintf(buffer,"$cc/debug/define=(MOMGENDEBUG,%s)/list=lis$:/obj=obj$:/mach/noopt/include=(sys$library,src$) src$:",buff);
	    fputs(buffer, outfile);
	    sprintf(buffer, "%sperform_set.c\n", class->prefix_m );
	    fputs(buffer, outfile);

	    fputs("$library obj$:mom obj$:",outfile);
	    sprintf(buffer, "%sperform_set\n", class->prefix_m );
	    fputs(buffer, outfile);                           
            }

        sprintf(buffer,"$cc/debug/define=(MOMGENDEBUG,%s)/list=lis$:/obj=obj$:/mach/noopt/include=(sys$library,src$) src$:",buff);
	fputs(buffer, outfile);
	sprintf(buffer, "%sperform_init.c\n", class->prefix_m );
	fputs(buffer, outfile);

	fputs("$library obj$:mom obj$:",outfile);
	sprintf(buffer, "%sperform_init\n", class->prefix_m );
	fputs(buffer, outfile);                           

	class = class->next;
	}

return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_link_dependencies
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-link-dependencies tag.
**
**	This routine generates the list of link dependencies needed by
**	the BUILD_MULTI_DESCRIP.MMS description file.
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
int insert_link_dependencies( mom,
		              outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,
		"        %sget_instance=obj$:%sget_instance.obj -\n",
		class->prefix, class->prefix);
	fputs(buffer, outfile);

	sprintf(buffer,
		"        %sfind_instance=obj$:%sfind_instance.obj -\n",
		class->prefix, class->prefix);
	fputs(buffer, outfile);

	if (mom->support_create)
	    {
    	    sprintf(buffer,
		"        %sperform_create=obj$:%sperform_create.obj -\n",
		class->prefix, class->prefix);
	    fputs(buffer, outfile);
	    }
	if (mom->support_delete)
	    {
	    sprintf(buffer,
		"        %sperform_delete=obj$:%sperform_delete.obj -\n",
		class->prefix, class->prefix);
	    fputs(buffer, outfile);
	    }
	if (mom->support_get)
	    {
	    sprintf(buffer,
		"        %sperform_get=obj$:%sperform_get.obj -\n",
		class->prefix, class->prefix);
	    fputs(buffer, outfile);
	    }
	if (mom->support_set)
	    {
	    sprintf(buffer,
		"        %sperform_set=obj$:%sperform_set.obj -\n",
		class->prefix, class->prefix);
	    fputs(buffer, outfile);
            }
	sprintf(buffer,
		"        %sperform_init=obj$:%sperform_init.obj -\n",
		class->prefix, class->prefix);
	fputs(buffer, outfile);

	class = class->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_compiles
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-compiles tag.
**
**	This routine generates the class-specific compilation dependencies
**	needed for the MMS description file.
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
int insert_compiles( mom,
		     outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;

    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
	sprintf(buffer,
		"obj$:%sget_instance.obj : src$:%sget_instance.c src$:common.h src$:mom_prototypes.h\n",
		class->prefix, class->prefix);
	fputs(buffer, outfile);

	sprintf(buffer,
		"obj$:%sfind_instance.obj : src$:%sfind_instance.c src$:common.h src$:mom_prototypes.h\n",
		class->prefix, class->prefix);
	fputs(buffer, outfile);

	if (mom->support_get)
	    {
	    sprintf(buffer,
		"obj$:%sperform_get.obj : src$:%sperform_get.c src$:common.h src$:mom_prototypes.h\n",
		class->prefix,
		class->prefix);
	    fputs(buffer, outfile);
	    }
	if (mom->support_set)
	    {
	    sprintf(buffer,
		"obj$:%sperform_set.obj : src$:%sperform_set.c src$:common.h src$:mom_prototypes.h\n",
		class->prefix,
		class->prefix);
	    fputs(buffer, outfile);
            }
	sprintf(buffer,
		"obj$:%sperform_init.obj : src$:%sperform_init.c src$:common.h src$:mom_prototypes.h\n",
		class->prefix,
		class->prefix);
	fputs(buffer, outfile);

	class = class->next;
    }

    return SS$_NORMAL;
}
