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
static char *rcsid = "@(#)$RCSfile: mg_oid.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/07/13 22:30:13 $";
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
**	This module contains the routines to used to generate code to 
**	compare OIDs (Object IDs).
**
**  AUTHORS:
**
**      Gary J. Allison
**
**  CREATION DATE:  23-Sep-1991
**
**  MODIFICATION HISTORY:
**
**  	Marc Nozell 	15-Jun-1992 	Remove reference to module.
**	Mike Densmore	21-May-1992	Modified for use on U**X platforms.
**
**	Mike Densmore	25-Sep-1992	Modified for ANSI Standard C
**
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#include "mom_defs.h"
#include <string.h>
#include "mg_prototypes.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_init_instance
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-init-instance.
**
**	This routine generates code that will create an object instance for
**	a given class.
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
int insert_init_instance( mom,
		    	  outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int		    count = 0;
    ATTRIBUTE_DEF   *attr = 0;
    int		    attribute_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

class = mom->current_class; 

get_identifier_attr( class, &attr );

if (attr == 0)
    return;

fputs("\f\n",outfile);
fputs("#ifdef VMS\n",outfile);
fputs("/*\n",outfile);
fputs("**++\n",outfile);
fputs("**  FUNCTIONAL DESCRIPTION:\n",outfile);
fputs("**\n",outfile);
sprintf(buffer,"**      %sinit_instance\n",class->prefix_m);
fputs(buffer,outfile);
fputs("**\n",outfile);
fputs("**	This routine is used as a starter routine for building object instances.\n",outfile);
fputs("**	It currently supports only the first class level under node. This routine \n",outfile);
fputs("**	requires modifications if the class is more than one level under node.\n",outfile);
fputs("**	Another moss_avl_add call must be made for each level.\n",outfile);
fputs("**	The identifier attribute of the parent class and an instance name\n",outfile);
fputs("**	must be specified for each of those levels.\n",outfile);
fputs("**	\n",outfile);
fputs("**	If used, this routine should get called from the class specific perform init routine.\n",outfile);
fputs("**\n",outfile);
fputs("**  FORMAL PARAMETERS:\n",outfile);
fputs("**\n",outfile);
fputs("**	name_avl          -- address of pointer to avl containing object instance to create.\n",outfile);
fputs("**	instance_name_str -- pointer to string containing instance name (Note this datatype \n",outfile);
fputs("**			     may be changed to match the identifier datatype if it is not a string).\n",outfile);
fputs("**\n",outfile);
fputs("**  RETURN VALUE:\n",outfile);
fputs("**\n",outfile);
fputs("**	MAN_C_SUCCESS			Normal success\n",outfile);
fputs("**	Any moss error \n",outfile);
fputs("**--\n",outfile);
fputs("*/\n",outfile);
sprintf(buffer,"static man_status %sinit_instance_avl( avl **name_avl, \n", class->prefix_m);
fputs(buffer,outfile);
fputs("		  char *instance_name_str)\n", outfile);
fputs("{\n", outfile);
fputs("    int		instance_length;\n", outfile);
fputs("    uid		*temp_uid;\n", outfile);
sprintf(buffer,"    %s_DEF    *new_%s_instance;\n",
	class->class_name,class->class_name);
fputs(buffer, outfile);
fputs("    char		sys_name[18];\n", outfile);
fputs("    unsigned short	sys_name_l = 0;		    /* length of name */\n", outfile);

fputs("    struct	getsyi_item\n", outfile);
fputs("    {\n", outfile);
fputs("	unsigned short	buff_length;\n", outfile);
fputs("	unsigned short  code;\n", outfile);
fputs("	unsigned int	buff_address;\n", outfile);
fputs("	unsigned int    ret_l_address;\n", outfile);
fputs("	unsigned int    end_list;\n", outfile);
fputs("    }	    sysname = {15,SYI$_NODENAME,&sys_name,&sys_name_l,0};\n", outfile);

fputs("    struct	io_stat_blk\n", outfile);
fputs("    {\n", outfile);
fputs("	unsigned short	iosb_cond_value;\n", outfile);
fputs("	unsigned short	iosb_count;\n", outfile);
fputs("	unsigned int	iosb_info;\n", outfile);
fputs("    }	    iosblock = {0,0,0};\n", outfile);
fputs("    man_status status;\n", outfile);
fputs("    int stat;\n", outfile);
fputs("    octet_string octet;\n", outfile);
fputs("    char abFullName[512];\n", outfile);
fputs("    short wFullNameLen;\n", outfile);
fputs("    short wNodeNameLen;\n", outfile);

fputs("    /*\n", outfile);
fputs("     *  initialize caller's AVL\n", outfile);
fputs("     */\n", outfile);
fputs("    status = moss_avl_init (name_avl);\n", outfile);
fputs("    /*\n", outfile);
fputs("     *  for full object name, AVL must use constructor\n", outfile);
fputs("     */\n", outfile);
fputs("    moss_avl_start_construct (*name_avl,\n", outfile);
fputs("                             NULL, 0, ASN1_C_SEQUENCE, NULL);\n", outfile);


    
fputs("    stat = SYS$GETSYI(0,0,0,&sysname,&iosblock,0,0);    /* sys serv call to get node name */\n", outfile);
fputs("    if (stat != SS$_NORMAL)\n", outfile);
fputs("        return MAN_C_PROCESSING_FAILURE;	   \n", outfile);
	/* Change this to use 0:: if failure */

fputs("    /*\n", outfile);
fputs("     *  convert the node name must be a full name\n", outfile);
fputs("     */\n", outfile);
fputs("    stat = moss_dns_opaque_to_fullname(\n", outfile);
fputs("                    sys_name,\n", outfile);
fputs("                    &sys_name_l,\n", outfile);
fputs("                    abFullName,\n", outfile);
fputs("                    &wFullNameLen);\n", outfile);
fputs("    /*\n", outfile);
fputs("     *  Add the node name into the AVL construct\n", outfile);
fputs("     *  with the FULL_NAME datatype.\n", outfile);
fputs("     */\n", outfile);
fputs("    octet.string = sys_name;\n", outfile);
fputs("    octet.length = sys_name_l;\n", outfile);
fputs("    octet.data_type = ASN1_C_OCTET_STRING;\n", outfile);
fputs("    status = moss_avl_add (\n", outfile);
fputs("                        *name_avl,\n", outfile);
fputs("                        &NODE_ATTR_NAME_DNA,\n", outfile);
fputs("                        MAN_C_SUCCESS,\n", outfile);
fputs("                        DNA_C_FULL_NAME,\n", outfile);
fputs("                        &octet);\n", outfile);

fputs("    if (status != MAN_C_SUCCESS)\n", outfile);
fputs("        {\n", outfile);
fputs("        moss_avl_free(name_avl, TRUE);\n", outfile);
fputs("        return status;\n", outfile);
fputs("        }\n", outfile);
fputs("    /**\n", outfile);
fputs("     **  Add the instance name for each parent into the AVL construct\n", outfile);
fputs("     **  If the class is more than one level under node, instance names must be\n",outfile);
fputs("     **  added for each level.\n", outfile);
fputs("     **/\n", outfile);

if (attr->mg_type == MG_C_STRING)
    {
    if (strcmp(attr->dna_data_type,"DNA_C_SIMPLE_NAME") == 0)
	{
	fputs("    status = char_to_simple_name( instance_name_str, strlen( instance_name_str), &octet );\n", outfile);
	fputs("    instance_length = octet.length;\n", outfile);
	}
    else
	{
        fputs("  octet.string = instance_name_str;\n",outfile);
	fputs("  octet.length = strlen( instance_name_str );\n",outfile);
	fputs("  instance_length = octet.length;\n", outfile);
	}
    fputs("    octet.data_type = ASN1_C_OCTET_STRING;\n", outfile);
    }
else if (attr->mg_type == MG_C_INTEGER)
    {
    if (attr->sign == MG_C_UNSIGNED)
	fputs("    status = copy_unsigned_int_to_octet( &octet, &instance_name_str );\n", outfile);
    else
	fputs("    status = copy_signed_int_to_octet( &octet, &instance_name_str  );\n", outfile);

    fputs("    octet.data_type = ASN1_C_INTEGER;\n", outfile);
    fputs("    instance_length = octet.length;\n", outfile);
    }
else fputs("/** Unknown identifier attribute datatype. Set up octet from datatype **/\n", outfile);
    
fputs("    status = moss_avl_add (\n", outfile);
fputs("                        *name_avl,\n", outfile);
sprintf(buffer,"                        &%s%s_DNA,\n", class->prefix_m, attr->attribute_name);
fputs(buffer, outfile);	    
fputs("                        MAN_C_SUCCESS,\n", outfile);
sprintf(buffer,"                    %s,\n", attr->dna_data_type );
fputs(buffer, outfile);	    
fputs("                        &octet);\n", outfile);

fputs("    if (status != MAN_C_SUCCESS)\n", outfile);
fputs("        {\n",outfile);
fputs("        moss_avl_free(name_avl, TRUE);\n", outfile);
fputs("        return status;\n",outfile);
fputs("        }\n",outfile);
fputs("\n", outfile);
fputs("    status = moss_avl_end_construct (*name_avl);\n", outfile);
fputs("\n", outfile);
fputs("    dbg_print_avl( *name_avl );\n", outfile);

sprintf(buffer,"    new_%s_instance = (%s_DEF *) malloc( sizeof( %s_DEF ));\n", 
	class->class_name,class->class_name,class->class_name);
fputs(buffer, outfile);	    
sprintf(buffer,"    if (new_%s_instance == NULL)\n", 
	class->class_name);
fputs(buffer, outfile);	    
fputs("        return MAN_C_INSUFFICIENT_RESOURCES;\n", outfile);
sprintf(buffer,"    memset( (void *)new_%s_instance, \'\\0\', sizeof( %s_DEF ));\n", 
	class->class_name,class->class_name,class->class_name);
fputs(buffer, outfile);	    

sprintf(buffer,"    temp_uid = &new_%s_instance->instance_uid;\n", 
	class->class_name);
fputs(buffer, outfile);	    
fputs("    status = (man_status) moss_get_uid( &temp_uid );\n", outfile);
fputs("    if (ERROR_CONDITION(status))\n", outfile);
fputs("	{\n", outfile);
sprintf(buffer,"	free( new_%s_instance );\n", 
	class->class_name);
fputs(buffer, outfile);	    
sprintf(buffer,"	free( new_%s_instance->instance_name );\n", 
	class->class_name);
fputs(buffer, outfile);	    
fputs("        return status;\n", outfile);
fputs("	}\n\n", outfile);
    
fputs("   /* \n", outfile);
fputs("    * Copy the instance name to the momdef structure. Use the correct datatype  format.\n", outfile);
fputs("    */\n", outfile);
sprintf(buffer,"    new_%s_instance->instance_name = (char *) malloc( instance_length + sizeof( char ));\n", 
	class->class_name);
fputs(buffer, outfile);	    
sprintf(buffer,"    if (new_%s_instance->instance_name == NULL)       \n", 
	class->class_name);
fputs(buffer, outfile);	    
fputs("        return MAN_C_INSUFFICIENT_RESOURCES;\n\n", outfile);
sprintf(buffer,"    strncpy( new_%s_instance->instance_name, octet.string, instance_length );\n", 
	class->class_name);
fputs(buffer, outfile);	               
sprintf(buffer,"    new_%s_instance->instance_name[ instance_length ] = (char) NULL;\n", 
	class->class_name);
fputs(buffer, outfile);	    
sprintf(buffer,"    new_%s_instance->instance_name_length  = instance_length;\n", 
	class->class_name);
fputs(buffer, outfile);	    
fputs("\n", outfile);
if (class->support_create)
    {
    sprintf(buffer,"    %sadd_new_instance( new_%s_header, new_%s_instance );\n", 
	class->prefix_m,class->class_name,class->class_name);
    fputs(buffer, outfile);	    
    }
fputs("    return MAN_C_SUCCESS;\n", outfile);
fputs("}\n",outfile);
fputs("#endif /* VMS */\n",outfile);

return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_map_oid
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-map-oid tag.
**
** 	This routine generates code to compare the OID against the 
**	perform action OID.  It also generates the IMPORT for the
**	perform action OID.
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
int insert_map_oid( mom,
		    outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    int		    count = 0;
    ARGUMENT_DEF    *argument;
    int		    argument_num;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class; 

    sprintf(buffer,"static man_status %smap_oid_to_arg( struct object_id *oid, unsigned int *argument)\n{\n", class->prefix_m);
    fputs(buffer, outfile);

    /* Get the request block from the create directive. If not found, don't
     * generate anything. 
     */

    count = 0;
    class = mom->current_class;					      
    argument = class->create_arg;

    for (argument_num=0; argument_num < class->num_create_args; argument_num++)
    {
      if (argument->dna_cmip_int_len)
	{
        if (argument_num)
            	fputs("  else ",outfile);      
	sprintf(buffer,
		"  if (moss_compare_oid( oid,		&%s%s_DNA) == MAN_C_EQUAL)\n",
		class->prefix_m,argument->argument_name);
	fputs(buffer, outfile);

	sprintf(buffer,
		"    *argument = %s%s;\n", 
		class->prefix_k,  argument->argument_name);
	fputs(buffer, outfile);
        count = 1;
	}
      if (argument->osi_oid_len)
	{
        if (count)
            	fputs("  else ",outfile);      
	sprintf(buffer,
		"  if (moss_compare_oid( oid,		&%s%s_OSI) == MAN_C_EQUAL)\n",
		class->prefix_m,argument->argument_name);
	fputs(buffer, outfile);

	sprintf(buffer,
		"    *argument = %s%s;\n", 
		class->prefix_k,  argument->argument_name);
	fputs(buffer, outfile);
        count = 1;
	}
      if (argument->snmp_oid_len)
	{
        if (count)
            	fputs("  else ",outfile);      
	sprintf(buffer,
		"  if (moss_compare_oid( oid,		&%s%s_SNP) == MAN_C_EQUAL)\n",
		class->prefix_m,argument->argument_name);
	fputs(buffer, outfile);

	sprintf(buffer,
		"    *argument = %s%s;\n", 
		class->prefix_k,  argument->argument_name);
	fputs(buffer, outfile);
	count=1;	
	}
      if (argument->oid_len)
	{
        if (count)
            	fputs("  else ",outfile);      
	sprintf(buffer,
		"  if (moss_compare_oid( oid,		&%s%s_OID) == MAN_C_EQUAL)\n",
		class->prefix_m,argument->argument_name);
	fputs(buffer, outfile);

	sprintf(buffer,
		"    *argument = %s%s;\n", 
		class->prefix_k,  argument->argument_name);
	fputs(buffer, outfile);
	count=1;	
	}

        argument = argument->next;
    }

    if (count)
	fputs(  "  else \n    return MAN_C_NO_SUCH_ATTRIBUTE_ID;\n",outfile);

    fputs(  "\n  return MAN_C_SUCCESS;\n}\n",outfile);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_class_compare
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-class-compare tag.
**
**	This routine generates code based to return the class ID based on
**	comparing the object_class with the class OID.
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
int insert_class_compare( mom,
			  outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num,one;
    char	    buffer[256];

    class = mom->class;
    one = 0;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    	{
  
        if (class->dna_cmip_int_len)
	    {
      	    if (class_num)
            	fputs("  else ",outfile);      
	    sprintf(buffer,"  if ((status = moss_compare_oid( object_class, &%s_DNA)) == MAN_C_EQUAL)\n",class->class_name);
   	    fputs(buffer, outfile);
	    sprintf(buffer,"     *class_code = %s%s_ID;\n",class->prefix_k,class->class_name);
	    fputs(buffer, outfile);
	    one = 1;
	    }

	if (class->oid_len)
	    {
	    if (one)
		fputs("   else ", outfile);
	    sprintf(buffer,"  if ((status = moss_compare_oid( object_class, &%s_OID)) == MAN_C_EQUAL)\n",class->class_name);
   	    fputs(buffer, outfile);
	    sprintf(buffer,"     *class_code = %s%s_ID;\n",class->prefix_k,class->class_name);
	    fputs(buffer, outfile);
	    one = 1;
            }

	if (class->snmp_oid_len)
	    {	
	    if (one)
		fputs("   else ", outfile);
	    sprintf(buffer,"  if ((status = moss_compare_oid( object_class, &%s_SNP)) == MAN_C_EQUAL)\n",class->class_name);
	    fputs(buffer, outfile);
	    sprintf(buffer,"     *class_code = %s%s_ID;\n",class->prefix_k,class->class_name);
	    fputs(buffer, outfile);
	    one = 1;
	    }

	if (class->osi_oid_len)
	    {	
	    if (one)
		fputs("   else ", outfile);
	    sprintf(buffer,"  if ((status = moss_compare_oid( object_class, &%s_OSI)) == MAN_C_EQUAL)\n",class->class_name);
	    fputs(buffer, outfile);
	    sprintf(buffer,"     *class_code = %s%s_ID;\n",class->prefix_k,class->class_name);
	    fputs(buffer, outfile);
	    one = 1;
	    }
	
    	class = class->next;
    	}
    fputs(		"  else\n     return MAN_C_ENTITY_CLASS_NOT_SUPPORTED; 	/* unknown OID */\n",outfile);

return SS$_NORMAL;
}
