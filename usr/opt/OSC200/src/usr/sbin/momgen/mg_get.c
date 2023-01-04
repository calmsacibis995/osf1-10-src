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
static char *rcsid = "@(#)$RCSfile: mg_get.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:55:03 $";
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
**	This module contains the routines to used to support the get entry point.
**
**  AUTHORS:
**
**      Rich Bouchard, Jr
**
**  CREATION DATE:  23-Sep-1991
**
**  MODIFICATION HISTORY:
**
**	F-22		    Marc Nozell		    15-Jun-1992
**	Remove references to module.
**
**	F-4		    Mike Densmore	    20-May-1992
**
**	Modified to work on U**X platforms
**
**	F-3		    Gary Allison	    23-Sep-1991
**
**	Add new routines, restructure module.  Support instance data.
**
**	F-2		    Marc Nozell		    24-May-1991
**                            
**	Support multiple classes.
**
**	25-Sep-1992	Mike Densmore	Modified for ANSI Standard C
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#define NAME_SIZE 5

#include "mom_defs.h"
#include <string.h>
#include "mg_prototypes.h"

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_get_attr_oid
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-get-attr-oid tag.
**
**	This routine generates code to compare attribute OIDs and returns
**	the unique attribute number.
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
int insert_get_attr_oid( mom,
		         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int 	    argument_num;
    ARGUMENT_DEF    *argument;
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num,one=0;
    int		    total=0;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
        if (attribute->dna_cmip_int_len)
	    {
      	    if (one)
            	fputs(     "   else ",outfile);      

	    if (get_create_arg( class, attribute, &argument ))
		{
	    	sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_DNA)) == MAN_C_EQUAL) \n",
			class->prefix_m, attribute->attribute_name);
   	    	fputs(buffer, outfile);
	        sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,attribute->attribute_name);
	        fputs(buffer, outfile);
	        total++;
        	if (argument->dna_cmip_int_len)
	    	    {
		    sprintf(buffer,"  else if ((status = moss_compare_oid( attr_oid, &%s%s_DNA)) == MAN_C_EQUAL)\n",
			class->prefix_m, argument->argument_name);
   	    	    fputs(buffer, outfile);
     	    	    sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,argument->argument_name);
	    	    fputs(buffer, outfile);
	            total++;
		    }
		}
	    else
		{
	    	sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_DNA)) == MAN_C_EQUAL)\n",
			class->prefix_m, attribute->attribute_name);
   	    	fputs(buffer, outfile);
	        sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,attribute->attribute_name);
	        fputs(buffer, outfile);
	        total++;
		}
	    one = 1;
	    }

	if (attribute->oid_len)
	    {
	    if (one)
		fputs("   else ", outfile);

	    if (get_create_arg( class, attribute, &argument ))
		{
	    	sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_OID)) == MAN_C_EQUAL) \n",
			class->prefix_m, attribute->attribute_name);
   	    	fputs(buffer, outfile);
	        sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,attribute->attribute_name);
	        fputs(buffer, outfile);
	        total++;

                if (argument->oid_len)
                    {
	    	    sprintf(buffer,"  else if ((status = moss_compare_oid( attr_oid, &%s%s_OID)) == MAN_C_EQUAL)\n",
			class->prefix_m, argument->argument_name);
   	    	    fputs(buffer, outfile);
     	    	    sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,argument->argument_name);
	    	    fputs(buffer, outfile);
	            total++;
		    }
		}
	    else
		{
	    	sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_OID)) == MAN_C_EQUAL)\n",
			class->prefix_m, attribute->attribute_name);
   	    	fputs(buffer, outfile);
	        sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,attribute->attribute_name);
	        fputs(buffer, outfile);
	        total++;
		}

	    one = 1;
            }

	if (attribute->snmp_oid_len)
	    {	
	    if (one)
		fputs("   else ", outfile);

	    if (get_create_arg( class, attribute, &argument ))
		{
	    	sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_SNP)) == MAN_C_EQUAL) \n",
			class->prefix_m, attribute->attribute_name);
   	    	fputs(buffer, outfile);
	        sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,attribute->attribute_name);
	        fputs(buffer, outfile);
	        total++;

                if (argument->snmp_oid_len)
                    {
  	    	    sprintf(buffer,"  else if ((status = moss_compare_oid( attr_oid, &%s%s_SNP)) == MAN_C_EQUAL)\n",
			    class->prefix_m, argument->argument_name);
   	    	    fputs(buffer, outfile);
     	    	    sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,argument->argument_name);
	    	    fputs(buffer, outfile);
	            total++;
		    }
		}
	    else
		{
	    	sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_SNP)) == MAN_C_EQUAL)\n",
			class->prefix_m, attribute->attribute_name);
   	    	fputs(buffer, outfile);
	        sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,attribute->attribute_name);
	        fputs(buffer, outfile);
	        total++;
		}

	    one = 1;
	    }

	if (attribute->osi_oid_len)
	    {	
	    if (one)
		fputs("   else ", outfile);

	    if (get_create_arg( class, attribute, &argument ))
		{
	    	sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_OSI)) == MAN_C_EQUAL)\n",
			class->prefix_m, attribute->attribute_name);
   	    	fputs(buffer, outfile);
	        sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,attribute->attribute_name);
	        fputs(buffer, outfile);
	        total++;

                if (argument->osi_oid_len)
                    {
		    sprintf(buffer,"  else if ((status = moss_compare_oid( attr_oid, &%s%s_OSI)) == MAN_C_EQUAL)\n",
			    class->prefix_m, argument->argument_name);
   	    	    fputs(buffer, outfile);
     	    	    sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,argument->argument_name);
	    	    fputs(buffer, outfile);
	    	    total++;
		    }
		}
	    else
		{
	    	sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_OSI)) == MAN_C_EQUAL)\n",
			class->prefix_m, attribute->attribute_name);
   	    	fputs(buffer, outfile);
	        sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,attribute->attribute_name);
	        fputs(buffer, outfile);
	        total++;
		}
	    one = 1;
	    }

	attribute = attribute->next;
	
        /* 
	 * C has a restriction of about 80 nested if/then/elses. 
	 * Start a new nested if/then/else around 70.
	 */
	
       if (total > 70)
	    {
	    fputs("\n    if (*attr != 0)\n        return MAN_C_SUCCESS;\n\n",outfile);
	    one = 0;
	    total-=70;
	    }
    }

    argument = class->create_arg;
    for (argument_num=0; argument_num < class->num_create_args; argument_num++)
     {
      if (create_arg_not_attribute( class, argument ))
      {
        if (argument->dna_cmip_int_len)
	    {
      	    if (one)
            	fputs(     "   else ",outfile);      

	    sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_DNA)) == MAN_C_EQUAL)\n",
			class->prefix_m, argument->argument_name);
   	    fputs(buffer, outfile);
	    sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,argument->argument_name);
	    fputs(buffer, outfile);
	    one = 1;
	    total++;
	    }

	if (argument->oid_len)
	    {
	    if (one)
		fputs("   else ", outfile);

	    sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_OID)) == MAN_C_EQUAL)\n",
			class->prefix_m, argument->argument_name);
   	    fputs(buffer, outfile);
	    sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,argument->argument_name);
	    fputs(buffer, outfile);
	    total++;
	    one = 1;
            }

	if (argument->snmp_oid_len)
	    {	
	    if (one)
		fputs("   else ", outfile);

	    sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_SNP)) == MAN_C_EQUAL)\n",
			class->prefix_m, argument->argument_name);
   	    fputs(buffer, outfile);
	    sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,argument->argument_name);
	    fputs(buffer, outfile);
	    total++;
	    one = 1;
	    }

	if (argument->osi_oid_len)
	    {	
	    if (one)
		fputs("   else ", outfile);

	    sprintf(buffer,"  if ((status = moss_compare_oid( attr_oid, &%s%s_OSI)) == MAN_C_EQUAL)\n",
			class->prefix_m, argument->argument_name);
   	    fputs(buffer, outfile);
	    sprintf(buffer,"     *attr = %s%s;\n",class->prefix_k,argument->argument_name);
	    fputs(buffer, outfile);
	    total++;
	    one = 1;
	    }
        }
     argument = argument->next;

       if (total > 70)
	    {
	    fputs("\n    if (*attr != 0)\n        return MAN_C_SUCCESS;\n\n",outfile);
	    one = 0;
	    total-=70;
	    }
     }

    if (one || total)
	{
	fputs("  else \n", outfile);
	fputs("      {\n", outfile);
	fputs("#ifdef MOMGENDEBUG\n", outfile);
	fputs("      printf(\"*** Attribute OID not found ***\\n\");\n",outfile);  
	fputs("#endif /* MOMGENDEBUG */\n", outfile);
	fputs("      return MAN_C_PROCESSING_FAILURE;\n",outfile);
	fputs("      }\n", outfile);
	}

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_get_items
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-get-items tag.
**
**	This routine generates the "switch" statement to perform
**	the "get" function on each individual attribute.
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
int insert_get_items( mom,
		      outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	sprintf(buffer,  "        case %s%s:\n", class->prefix_k, attribute->attribute_name);
	fputs(buffer, outfile);

	/* This switch generates the code to get the data and length into the octet */

	switch (attribute->mg_type)
            {
            case  MG_C_STRING :

	    /* Check DNA data types to see if any specific action needs to be done */

            if (attribute->uid)
		{
		fputs("            octet.string = (char *) instance->instance_uid;\n", outfile);
	        fputs("            octet.length = 16;\n", outfile);
		}
            else if (attribute->name)
		{
		fputs("            octet.string = instance->instance_name;\n", outfile);
	        fputs("            octet.length = instance->instance_name_length;\n",outfile);
		}
	    else
		{
	        sprintf(buffer,"            octet.string = instance->%s;\n", attribute->attribute_name+NAME_SIZE);
	    	fputs(buffer, outfile);
	        sprintf(buffer,"            octet.length = instance->%s_len;\n",
		        attribute->attribute_name+NAME_SIZE);
	    	fputs(buffer, outfile);
		}

	   /**
	    ** The following are hacks to prevent MCC from ACCVIOing, etc. MCC
	    ** cannot deal with an empty OCTET or HEXSTRING, so don't return an
	    ** empty one.
	    **/
            if (strncmp( attribute->dna_data_type, "DNA_C_OCTET", 11) == 0)
		{
		fputs("            /** Skip adding an empty OCTET **/\n",outfile);
	        fputs("            if (octet.length == 0)\n",outfile);
		fputs("                return MAN_C_SUCCESS; \n",outfile);
		}
            if (strncmp( attribute->dna_data_type, "DNA_C_HEX_STRING", 16) == 0)
		{
		fputs("            /** Skip adding an empty HEXSTRING **/\n",outfile);
	        fputs("            if (octet.length == 0)\n",outfile);
		fputs("                return MAN_C_SUCCESS; \n",outfile);
		}

	    break;

	case MG_C_BOOLEAN :
	    sprintf(buffer,"            octet.string = (char *)&instance->%s;\n",
		    attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("            octet.length = 4;\n",outfile);
	    break;

	case MG_C_INTEGER :
	    if (attribute->sign == MG_C_UNSIGNED)
	    	sprintf(buffer,"            status = copy_unsigned_int_to_octet( &octet, &instance->%s );\n", 
		    attribute->attribute_name+NAME_SIZE);
	    else
	    	sprintf(buffer,"            status = copy_signed_int_to_octet( &octet, &instance->%s  );\n", 
		    attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    break;

	case MG_C_UID :
	    sprintf(buffer,"            if (memcmp( &instance->%s, nil_uid_p, sizeof (uid)) == 0)\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs(         "                octet.string = (char *) &instance->instance_uid;\n",outfile);
	    fputs(         "            else\n",outfile);
	    sprintf(buffer,"                octet.string = (char *) &instance->%s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("            octet.length = 16;\n", outfile );
	    break;
	
	case MG_C_OID :
	    sprintf(buffer,"            if (instance->%s != NULL)\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,"                status = moss_oid_to_octet( instance->%s, &octet );\n",
		    attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs("            else {\n",outfile);
	    fputs("                octet.length = 0;\n",outfile);
            fputs("                octet.string = \"\";\n",outfile);
	    fputs("            }\n",outfile);
	    break;

	case MG_C_TIME :
	    sprintf(buffer,"            if (memcmp( &instance->%s, nil_time_p, sizeof( mo_time )) == 0)\n",  attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs(         "                {\n",outfile);
	    sprintf(buffer,"                temp_time = &instance->%s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs(         "                status = moss_get_time( &temp_time );\n",outfile);
	    fputs(         "                }\n",outfile);
	    fputs(buffer, outfile);
	    sprintf(buffer,"            octet.string = (char *) &instance->%s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs(         "            octet.length = 16;\n", outfile );
	    break;


	case MG_C_VERSION :
	    sprintf(buffer,"            if (instance->%s == NULL)\n",  attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,"                instance->%s = %s%s_version;\n",
			attribute->attribute_name+NAME_SIZE, class->prefix_k, class->class_name );
	    fputs(buffer, outfile);
	    sprintf(buffer,"            octet.string = (char *) instance->%s;\n", attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs(         "            octet.length = 4;\n", outfile );
	    break;

	/* AVLs get copied using moss_avl_copy */

	case MG_C_ACL :
	case MG_C_AVL :	
	    break;

	default : 
	    sprintf(buffer,"            octet.string = (char *)&instance->%s; /** VERIFY data type **/\n",
		    attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
            fputs(	        "            octet.length = 4; /** VERIFY length **/\n", outfile);
	    }

	/* This section generates the MOSS_AVL_ADD with the correct OID */

	if ((attribute->mg_type == MG_C_AVL) || 
	    (attribute->mg_type == MG_C_ACL))
	    {
	    sprintf(buffer,"            if (instance->%s != NULL)\n                {\n",attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    sprintf(buffer,"                status = moss_avl_reset( instance->%s );\n",attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs(		"                status = moss_avl_copy(\n                    reply_attribute_list,\n",outfile);
	    sprintf(buffer,"                    instance->%s,\n",attribute->attribute_name+NAME_SIZE);
	    fputs(buffer, outfile);
	    fputs(		"                    FALSE,\n",outfile);
	    fputs(		"                    NULL,\n",outfile);
	    fputs(		"                    NULL,\n",outfile);
	    fputs(		"                    &last);\n",outfile);
	    fputs(		"                }\n            break;\n",outfile);
	    }
        else {
	    if (attribute->mg_type == MG_C_UNKNOWN)
                fputs(	    "            octet.data_type = 0; /** Unknown data type **/\n", outfile);
	    else
		{
	        sprintf(buffer,"            octet.data_type = ASN1_C_%s;\n", attribute->data_type);
	        fputs(buffer, outfile);
		}
	    fputs("            status = moss_avl_add(reply_attribute_list,\n", outfile);

	    if (attribute->dna_cmip_int_len)
	        {
	        if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID\n",outfile);

    	        sprintf(buffer,"                                  &%s%s_DNA,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);

	        if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_OID */\n",outfile);
		}
	    if (attribute->osi_oid_len)
	        {
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
    	        sprintf(buffer,"                                  &%s%s_OSI,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);
		}
	    if (attribute->snmp_oid_len)
	        {
		fputs("#ifdef SNMP_OID\n",outfile);
    	        sprintf(buffer,"                                  &%s%s_SNP,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* SNMP_OID */\n",outfile);
		}
	    if (attribute->oid_len)
	        {
		fputs("#ifdef OID\n",outfile);
    	        sprintf(buffer,"                                  &%s%s_OID,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* OID */\n",outfile);
		}

	    fputs("                                  (unsigned int) MAN_C_SUCCESS,\n", outfile);

	    if (attribute->mg_type == MG_C_UNKNOWN)
	        fputs(    	    "                                  0, /** Unknown data type **/\n",outfile);
	    else
	        {
	        sprintf(buffer,"                                  %s,\n", attribute->dna_data_type);
	        fputs(buffer, outfile);
	        }
	    fputs("                                  &octet);\n", outfile);
	    fputs("            break;\n", outfile);
	    }

	attribute = attribute->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_get_all_attr
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-get-all-attr tag.
**
**	This routine generates the code to retrieve all attributes
**	for the given class.
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
int insert_get_all_attr( mom,
		         outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class; 
    attribute = class->attribute;
    

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	sprintf(buffer,
		"    status = %sget_list_item(%s%-24s, instance, reply_attribute_list);\n",
		class->prefix_m, class->prefix_k, attribute->attribute_name);
	fputs(buffer, outfile);
	fputs(  "    if ERROR_CONDITION( status )\n", outfile);
	fputs(  "        return status;\n\n", outfile );

	attribute = attribute->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_switch_get_candidate
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-switch-get-candidate tag.
**
**	This routine generates the code in "get_candidate_instance" which
**	dispatches to the class-specific get_candidate function.
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
int insert_switch_get_candidate( mom,
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
		"        case %s%s_ID:\n", class->prefix_k, class->class_name);
	fputs(buffer, outfile);

	sprintf(buffer,
		"            status = %sget_instance(object_class,\n",
		class->prefix);
	fputs(buffer, outfile);

	fputs(  "                                        object_instance,\n", outfile);
	fputs(  "                                        iso_scope,\n", outfile);
	sprintf(buffer,"                                 (%s_DEF **) specific_instance,\n", class->class_name);
	fputs(buffer, outfile);
	fputs(	"                                        specific_object_instance,\n", outfile);
	fputs(  "                                        object_uid,\n", outfile);
	fputs(  "                                        more_instances,\n", outfile);
	fputs(  "                                        return_context,\n", outfile);
	fputs(  "                                        build_avl);\n", outfile);
	fputs(  "            break;\n", outfile);

	class = class->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	one_group
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file.
**
**	This routine generates the code to retrieve all of the attributes
**	in the specific attribute group called from insert_get_groups routine.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	class		Pointer to the data structure describing the current class
**	group		The group to retrieve attributes for
**	outfile		File handle of the current output file.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
*/
int one_group( mom,
	       class,
	       group,
	       outfile)

MOM_BUILD *mom;
CLASS_DEF *class;
int group;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    char	    buffer[256];
    int		    status;

    attribute = class->attribute;

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	if (attribute->group == group)
	{
	    sprintf(buffer,
		    "                status = %sget_list_item(%s%-25s, instance, reply_attribute_list);\n",
		    class->prefix_m, class->prefix_k, attribute->attribute_name);
	    fputs(buffer, outfile);
	    fputs(  "    	     if ERROR_CONDITION( status )\n", outfile);
	    fputs(  "        	         return status;\n\n", outfile );
        }
	attribute = attribute->next;
    }

    fputs("                break;\n", outfile);

    return SS$_NORMAL;
}	    

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_get_group
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-get-groups tag.
**
**	This routine generates the code to retrieve all of the attributes
**	for each attribute group.
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
int insert_get_groups( mom,
		       outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    fputs("            case GROUP_IDENTIFIERS:\n", outfile);
    status = one_group(mom, class, GROUP_IDENTIFIERS, outfile);
    if (ERROR_CONDITION(status))
	return status;
    
    fputs("            case GROUP_CHARACTERISTICS:\n", outfile);
    status = one_group(mom, class, GROUP_CHARACTERISTICS, outfile);
    if (ERROR_CONDITION(status))
	return status;
    
    fputs("            case GROUP_STATUS:\n", outfile);
    status = one_group(mom, class, GROUP_STATUS, outfile);
    if (ERROR_CONDITION(status))
	return status;
    
    fputs("            case GROUP_COUNTERS:\n", outfile);
    status = one_group(mom, class, GROUP_COUNTERS, outfile);
    if (ERROR_CONDITION(status))
	return status;
    
    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_list_attr
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-list-attr tag.
**
**	This routine generates a "switch" statement with an entry
**	for every attribute.
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
int insert_list_attr( mom,
		      outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    if (class->num_attributes == 0) {	/* some SNMP classes have no attributes */
	fputs("                case 0:\n", outfile);
	return SS$_NORMAL;
    }

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
	sprintf(buffer,
		"                case %s%s:\n",
		class->prefix_k, attribute->attribute_name);
	fputs(buffer, outfile);

	attribute = attribute->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_list_status
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-list-status tag.
**
**	This routine generates a "switch" statement with an entry
**	for every status attribute.
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
int insert_list_status( mom,
		        outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    num_status = 0;
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
        if (attribute->group == GROUP_STATUS)
        {
        if (!num_status)
            fputs("                /*\n                 * Return Status attributes\n                */\n",outfile);
	sprintf(buffer,"                status = %sget_list_item( %s%s	, instance, reply_attribute_list );\n",
		class->prefix_m, class->prefix_k, attribute->attribute_name);
	fputs(buffer, outfile);
	fputs("    	                    if ERROR_CONDITION( status )\n", outfile);
	fputs("        	                return status;\n\n", outfile );
        num_status++;
	}
	attribute = attribute->next;
    }

    if (!num_status)
	{
	fputs("                /*\n                 * No status attributes\n                 */\n",outfile);
        fputs("		return MAN_C_SUCCESS;\n",outfile);
	}

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_list_counters
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-list-counters tag.
**
**	This routine generates a "switch" statement with an entry
**	for every counter attribute.
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
int insert_list_counters( mom,
		     	  outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    counters = 0;    
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
        if (attribute->group == GROUP_COUNTERS)
        {
        if (!counters)
            fputs("                /*\n                 * Return Counter attributes\n                */\n",outfile);
	sprintf(buffer,
		"                status = %sget_list_item( %s%s	, instance, reply_attribute_list );\n",
		class->prefix_m, class->prefix_k, attribute->attribute_name);
	fputs(buffer, outfile);
	fputs("                if ERROR_CONDITION( status )\n", outfile);
	fputs("                    return status;\n\n", outfile );
        counters++;
	}
	attribute = attribute->next;
    }

    if (!counters)
	{
        fputs("                /*\n                 * No counter attributes\n                 */\n",outfile);
        fputs("		return MAN_C_SUCCESS;\n",outfile);
	}

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_list_id
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-list-id tag.
**
**	This routine generates a "switch" statement with an entry
**	for every identifier attribute.
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
int insert_list_id( mom,
		    outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    id = 0;
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
        if (attribute->group == GROUP_IDENTIFIERS)
        {
        if (!id)
            fputs("                /*\n                 * Return identifier attributes\n                 */\n",outfile);
	sprintf(buffer,
		"                status = %sget_list_item( %s%s	, instance, reply_attribute_list );\n",
		class->prefix_m, class->prefix_k, attribute->attribute_name);
	fputs(buffer, outfile);
	fputs("                if ERROR_CONDITION( status )\n", outfile);
	fputs("                    return status;\n\n", outfile );
        id++;
	}
	attribute = attribute->next;
    }

    if (!id)
	{
	fputs("          octet_string      octet;\n", outfile);
        fputs("                /*\n                 * No identifier attributes - return NULL identifier OID.\n                 */\n",outfile);
	fputs("          octet.string = (char *) NULL;\n", outfile);
	fputs("          octet.length = 0;\n", outfile);
	fputs("          octet.data_type = ASN1_C_NULL;\n", outfile);
	fputs("          status = moss_avl_add(reply_attribute_list,\n", outfile);
	fputs("                                &NULL_ID_ATTRIBUTE_DNA,\n", outfile);
	fputs("                                (unsigned int)MAN_C_SUCCESS,\n", outfile);
	fputs("                                ASN1_C_NULL,\n", outfile);
	fputs("                                &octet);\n\n", outfile);
	}

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_list_char
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-list-char tag.
**
**	This routine generates a "switch" statement with an entry
**	for every characteristic attribute.
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
int insert_list_char( mom,
		      outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    chars = 0;
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
        if (attribute->group == GROUP_CHARACTERISTICS)
        {
        if (!chars)
            fputs("                /*\n                 * Return chacteristic attributes\n                 */\n",outfile);
	sprintf(buffer,
		"                status = %sget_list_item( %s%s	, instance, reply_attribute_list );\n",
		class->prefix_m, class->prefix_k, attribute->attribute_name);
	fputs(buffer, outfile);
	fputs("                if ERROR_CONDITION( status )\n", outfile);
	fputs("                    return status;\n\n", outfile );
        chars++;
	}
	attribute = attribute->next;
    }

    if (!chars)
	{
        fputs("                /*\n                 * No characteristic attributes\n                 */\n",outfile);
        fputs("		return MAN_C_SUCCESS;\n",outfile);
	}

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_build_avl_type
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-build-avl-type tag.
**
**	This routine generates code based on the data type of the identifier
**	attribute for the build_instance_avl routine.
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
int insert_build_avl_type( mom,
		           outfile)

MOM_BUILD *mom;
FILE *outfile;

{
ATTRIBUTE_DEF       *attribute;
int		    attribute_num;
CLASS_DEF	    *class;
char	    	    buffer[256];
int		    chars = 0;
int		    found_one = 0;
int		    status;

class = mom->current_class;					      
attribute = class->attribute;

for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
    if (attribute->group == GROUP_IDENTIFIERS)
      {
      if (found_one)
           {
	   fputs("/** Multiple identifier OIDs found. Verify the correct OID **/\n",outfile);
	   return SS$_NORMAL;
	   }

      if (attribute->mg_type == MG_C_STRING)
	{      
	sprintf(buffer,"  octet.data_type = ASN1_C_%s;\n",attribute->data_type);
	fputs(buffer, outfile);
        fputs("  octet.string = specific_instance->instance_name;\n",outfile);
	fputs("  octet.length = specific_instance->instance_name_length;\n",outfile);
	fputs("  \n",outfile);
        fputs("  status =(man_status) moss_avl_add(*specific_object_instance,		/* avl_handle */\n",outfile);
	    if (attribute->dna_cmip_int_len)
	        {
	        if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID\n",outfile);
    	        sprintf(buffer,"                             &%s%s_DNA,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
	        if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_OID */\n",outfile);
		}
	    if (attribute->osi_oid_len)
	        {
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
    	        sprintf(buffer,"                             &%s%s_OSI,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);
		}
	    if (attribute->snmp_oid_len)
	        {
		fputs("#ifdef SNMP_OID\n",outfile);
    	        sprintf(buffer,"                             &%s%s_SNP,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* SNMP_OID */\n",outfile);
		}
	    if (attribute->oid_len)
	        {
		fputs("#ifdef OID\n",outfile);
    	        sprintf(buffer,"                             &%s%s_OID,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* OID */\n",outfile);
		}

        fputs("				    (unsigned int) MAN_C_SUCCESS,			/* modifier */\n",outfile);
	sprintf(buffer,
	      "                             (unsigned int) %s,\n",
		attribute->dna_data_type);
	fputs(buffer, outfile);
        fputs("				    &octet);				/* octet */\n",outfile);
	}        
      else if (attribute->uid)
	{
        fputs("  octet.data_type = ASN1_C_OCTET_STRING;\n",outfile);
        fputs("  octet.string = (char *) &specific_instance->instance_name;\n",outfile);
        fputs("  octet.length = 16;\n",outfile);
        fputs("  \n",outfile);
        fputs("  status =(man_status) moss_avl_add(*specific_object_instance,		/* avl_handle */\n",outfile);
	    if (attribute->dna_cmip_int_len)
	        {
	        if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID\n",outfile);
    	        sprintf(buffer,"                                  &%s%s_DNA,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* DNA_CMIP_OID */\n",outfile);
		}
	    if (attribute->osi_oid_len)
	        {
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
    	        sprintf(buffer,"                                  &%s%s_OSI,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);
		}
	    if (attribute->snmp_oid_len)
	        {
		fputs("#ifdef SNMP_OID\n",outfile);
    	        sprintf(buffer,"                                  &%s%s_SNP,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* SNMP_OID */\n",outfile);
		}
	    if (attribute->oid_len)
	        {
		fputs("#ifdef OID\n",outfile);
    	        sprintf(buffer,"                                  &%s%s_OID,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
	        if (class->multiple_oids)
		    fputs("#endif /* OID */\n",outfile);
		}
        fputs("				    (unsigned int) MAN_C_SUCCESS,			/* modifier */\n",outfile);
        fputs("				    (unsigned int) DNA_C_UID, 			/* tag */\n",outfile);
        fputs("				    &octet);				/* octet */\n",outfile);
	}        
      else 
	{
	sprintf(buffer,"  octet.data_type = ASN1_C_%s; /** Verify datatype **/\n", attribute->data_type);
	fputs(buffer, outfile);
        fputs("  octet.string = (char *) specific_instance->instance_name;\n",outfile);
	fputs("  octet.length = specific_instance->instance_name_length;\n",outfile);
	fputs("  \n",outfile);
        fputs("  status =(man_status) moss_avl_add(*specific_object_instance,		/* avl_handle */\n",outfile);
	    if (attribute->dna_cmip_int_len)
	        {
	        if (class->multiple_oids)
		    fputs("#ifdef DNA_CMIP_OID\n",outfile);
    	        sprintf(buffer,"                             &%s%s_DNA,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
	        if (class->multiple_oids)
		    fputs("#endif /* DNA_CMIP_OID */\n",outfile);
		}
	    if (attribute->osi_oid_len)
	        {
		fputs("#ifdef OSI_CMIP_OID\n",outfile);
    	        sprintf(buffer,"                             &%s%s_OSI,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* OSI_CMIP_OID */\n",outfile);
		}
	    if (attribute->snmp_oid_len)
	        {
		fputs("#ifdef SNMP_OID\n",outfile);
    	        sprintf(buffer,"                             &%s%s_SNP,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* SNMP_OID */\n",outfile);
		}
	    if (attribute->oid_len)
	        {
		fputs("#ifdef OID\n",outfile);
    	        sprintf(buffer,"                             &%s%s_OID,\n", 
			class->prefix_m, attribute->attribute_name);
	        fputs(buffer, outfile);
		fputs("#endif /* OID */\n",outfile);
		}

        fputs("				    (unsigned int) MAN_C_SUCCESS,			/* modifier */\n",outfile);
	sprintf(buffer,
	      "                             (unsigned int) %s, /** Verify data type **/ \n",
		attribute->dna_data_type);
	fputs(buffer, outfile);
        fputs("				    &octet);				/* octet */\n",outfile);
	}        
      found_one = 1;
      }
    attribute = attribute->next;
    }
return 1;
}
