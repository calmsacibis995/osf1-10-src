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
static char *rcsid = "@(#)$RCSfile: mg_getnext.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/07/13 22:28:17 $";
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
**  FACILITY:  MOM Generator
**
**  MODULE DESCRIPTION:
**
**	This module contains the routines to used to support the getnext entry point.
**
**  AUTHORS:
**
**      Mike Densmore
**
**  CREATION DATE:  07-Jul-1992
**
**  MODIFICATION HISTORY:
**
**	25-Sep-1992 Mike Densmore   Modified for ANSI Standard C
**
**	16-Mar-1993 M. Ashraf       Modified for class with no attribute, 
**                                  and no instance
**
**	 5-May-1993 M. Ashraf       insert_first_oid() - for nontable,
**                                  add line "in_oid = ret_oid;"
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#include "mom_defs.h"
#include <string.h>
#include "man.h"
#include "moss.h"
#include "mg_prototypes.h"

#define MOMGEN_OID_COMPARE( x, y ) \
        ( ((x) > (y)) ? MAN_C_GREATER : \
                ((x) < (y)) ? MAN_C_LESS : MAN_C_EQUAL )



man_status
momgen_compare_oid(
                    oid1,
                    oid2
                  )

object_id *oid1 ;
object_id *oid2 ;

/*
 *    Function Description:
 *
 *       This routine compares the first oid to the second oid. If the first
 *       oid is greater than the second oid, MAN_C_GREATER is returned. If
 *       the first oid is less than the second oid, MAN_C_LESS is returned.
 *       If they are equal, MAN_C_EQUAL is returned.
 *
 *    Arguments:
 *
 *       oid1		Address of the first oid
 *       oid2		Address of the second oid
 *
 *    Return Value:
 *
 *       MAN_C_EQUAL		The two oids are equal
 *       MAN_C_GREATER		The first oid is greater than the second oid
 *       MAN_C_LESS		The first oid is less than the second oid
 *       MAN_C_BAD_PARAMETER	Invalid parameter
 *
 *    Side Effects:
 *
 *       None
 */

{
    int longer_depth ;
    int i ;
    int ret_value ;
    unsigned int *first ;
    unsigned int *last ;

    /*
     * Invalid address of oid, return error.
     */

    if ( ( oid1 == NULL ) || ( oid2 == NULL ) )
	return( MAN_C_BAD_PARAMETER ) ;

    /*
     * Invalid count, return error.
     */

    if ( ( oid1->count < 1 ) || ( oid2->count < 1 ) )
	return( MAN_C_BAD_PARAMETER ) ;

    /*
     * Invalid pointer, return error.
     */

    if ( ( oid1->value == NULL ) || ( oid2->value == NULL ) )
	return( MAN_C_BAD_PARAMETER ) ;

    first = oid1->value ;
    last = oid2->value ;

    /*
     * Get the longer length between the two oids.
     */

    if ( oid1->count > oid2->count )
	longer_depth = oid1->count ;
    else
	longer_depth = oid2->count ;

    /*
     * Compare each element.
     */

    for ( i = 0 ; i < longer_depth ; i++ )
    {
	/*
	 * If the two oids are not of the same length, then one will
	 * be greater or less than the other based on which is shorter.
	 */

	if ( i >= oid1->count )
	    return( MAN_C_LESS ) ;

	if ( i >= oid2->count )
	    return( MAN_C_GREATER ) ;

	ret_value = MOMGEN_OID_COMPARE( *first, *last ) ;

	first++ ;
	last++ ;

	if ( ret_value != MAN_C_EQUAL )
	    return( ret_value ) ;
    }

    return( ret_value ) ;

}   /* end of momgen_compare_oid() */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	find_lowest_oid
**
**      Finds the lowest attribute id using lexicographic ordering.
**	There must be at least one attribute in the current class structure.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	attribute	Pointer to attribute structure in current class.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
**/

int find_lowest_oid( mom,
		     attribute)

MOM_BUILD *mom;
ATTRIBUTE_DEF **attribute;

{

ATTRIBUTE_DEF   *next_attribute;
CLASS_DEF	*class;
man_status	status;
object_id	*oid1 = (object_id *)NULL;
object_id       *oid2 = (object_id *)NULL;

    class = mom->current_class;
    next_attribute = class->attribute;
    *attribute = next_attribute;
    next_attribute = next_attribute->next;

    while (next_attribute != NULL) {
	status = moss_text_to_oid((*attribute)->snmp_oid_text,&oid1);
	status = moss_text_to_oid(next_attribute->snmp_oid_text,&oid2);
	status = momgen_compare_oid(oid1,oid2);
	if (status == MAN_C_GREATER)
	    *attribute = next_attribute;
	next_attribute = next_attribute->next;
    }
    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	find_higher_oid
**
**      Finds the next highest attribute id using lexicographic ordering.
**	There must be at least one attribute in the current class structure.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**	attribute	Pointer to attribute structure in current class.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**
**--
**/

int find_higher_oid( mom,
		     attribute)

MOM_BUILD *mom;
ATTRIBUTE_DEF **attribute;

{

ATTRIBUTE_DEF   *next_attribute;
ATTRIBUTE_DEF   *candidate_attribute = (ATTRIBUTE_DEF *)NULL;
CLASS_DEF	*class;
man_status	status;
object_id       *oid1 = (object_id *)NULL;
object_id       *oid2 = (object_id *)NULL;
object_id       *oid3 = (object_id *)NULL;

    class = mom->current_class;
    next_attribute = class->attribute;

    while (next_attribute != NULL) {
	status = moss_text_to_oid((*attribute)->snmp_oid_text,&oid1);
	status = moss_text_to_oid(next_attribute->snmp_oid_text,&oid2);
	status = momgen_compare_oid(oid1,oid2);
	if (status == MAN_C_LESS) {
	    if (candidate_attribute != NULL) {
		status = moss_text_to_oid(candidate_attribute->snmp_oid_text,&oid3);
		status = momgen_compare_oid(oid3,oid2);
		if (status == MAN_C_GREATER) {
		    candidate_attribute = next_attribute;
		}
	    }
	    else {
		candidate_attribute = next_attribute;
	    }
	}
	next_attribute = next_attribute->next;
    }
    *attribute = candidate_attribute;
    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_next_oid
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-getnext-attribute-oid-code tag.
**
**	This routine generates code to take an attribute oid and return the
**	oid of the next lexicographic attribute.  Note: the routine does not
**	check the SMI being used (eg; is the object using SNMP OIDS?).
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
int insert_next_oid( mom,
		     outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    CLASS_DEF	    *class;
    char	    buffer[256];
    char	    *oid;

    oid = "_SNP";

    class = mom->current_class;					      
    attribute = class->attribute;

    if (attribute == NULL)  {	/* does this class have attributes? */
	fputs(" /* this class has no attributes */\n\n",outfile);
	fputs("        status = MAN_C_NO_SUCH_ATTRIBUTE_ID;\n",outfile);
	return SS$_NORMAL;
    }

    find_lowest_oid( mom, &attribute );

    fputs("        status = MAN_C_SUCCESS;\n\n",outfile);
    sprintf(buffer,"    if ((status = moss_compare_oid( attr_oid, &%s%s%s)) == MAN_C_EQUAL) \n",
					class->prefix_m, attribute->attribute_name, oid);
    fputs(buffer, outfile);

    do {
	find_higher_oid( mom, &attribute );
	if (attribute != NULL) {
	    sprintf(buffer,"        *ret_oid = &%s%s%s;\n",class->prefix_m, attribute->attribute_name, oid);
	    fputs(buffer, outfile);
	    sprintf(buffer,"    else if ((status = moss_compare_oid( attr_oid, &%s%s%s)) == MAN_C_EQUAL) \n",
					class->prefix_m, attribute->attribute_name, oid);
	    fputs(buffer, outfile);
	}
	else
	    fputs("        status = MAN_C_NO_SUCH_ATTRIBUTE_ID;\n",outfile);
    }
    while (attribute != NULL);
    fputs("    else status = MAN_C_NO_SUCH_ATTRIBUTE_ID;\n\n",outfile);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_first_oid
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-getnext-first-attribute-code tag.
**
**	This routine generates code to set a pointer to the oid of the first
**	lexicographic attribute for the current class.  Note: the routine does not
**	check the SMI being used (eg; is the object using SNMP OIDS?).
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
int insert_first_oid( mom,
		      outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    ATTRIBUTE_DEF   *attribute;
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      
    attribute = class->attribute;

    if (attribute == NULL) {	/* does this class have attributes? */
	fputs("       /* this class has no attributes */\n\n",outfile);
	fputs("       return MAN_C_NO_SUCH_ATTRIBUTE_ID;\n",outfile);
    }
    else {
	find_lowest_oid( mom, &attribute );
	sprintf(buffer,"       ret_oid = &%s%s_SNP;\n\n",class->prefix_m, attribute->attribute_name);
        fputs(buffer, outfile);
	if (!class->support_instances)
	  fputs("       in_oid = ret_oid;   /* make sure in_oid points to something */\n",outfile);
    }
    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_class_code
**
**      Generates a class code symbol for perform_get routine call in getnext
**
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
int insert_class_code( mom,
		       outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      

        sprintf(buffer,
                "					%s%s_ID);\n", class->prefix_k, class->class_name);
        fputs(buffer, outfile);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_instance_construct
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-instance_construct-code tag.
**
**	This routine generates code to construct an instance for the current
**	class.
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
int insert_instance_construct( mom,
		               outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;					      

    if (class->parent_prefix != NULL) {
	if ((strcmp(class->parent_prefix,"SNMP_"))) {
	    fputs("    status = moss_avl_add( instance_name,\n", outfile);
	    sprintf(buffer,"                            &%sNULL_SNP,\n",class->parent_prefix);
	    fputs(buffer, outfile);
	    fputs("                            (int)MAN_C_SUCCESS,\n", outfile);
	    fputs("                            ASN1_C_NULL,\n", outfile);
	    fputs("                            &null_instance_octet);\n", outfile);
	    fputs("    if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;\n\n", outfile);
	}
    }
    fputs("    status = moss_avl_add( instance_name,\n", outfile);
    sprintf(buffer,"                            &%sNULL_SNP,\n",class->prefix);
    fputs(buffer, outfile);
    fputs("                            (int)MAN_C_SUCCESS,\n", outfile);
    fputs("                            ASN1_C_NULL,\n", outfile);
    fputs("                            &null_instance_octet);\n", outfile);
    fputs("    if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;\n", outfile);

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_par_inst_construct
**
**      Generates a code section for inclusion in the current MOM Generator
**	output file based on the insert-code-instance_construct-code tag.
**
**	This routine generates code to construct an instance for the current
**	class.
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
int insert_par_inst_construct( mom,
		               outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    char	    buffer[256];
    int		    status;

    class = mom->current_class;

    if (class->parent_prefix != NULL) {
	if ((strcmp(class->parent_prefix,"SNMP_"))) {
	    fputs("    status = moss_avl_add( instance_name,\n", outfile);
	    sprintf(buffer,"                            &%sNULL_SNP,\n",class->parent_prefix);
	    fputs(buffer, outfile);
	    fputs("                            (int)MAN_C_SUCCESS,\n", outfile);
	    fputs("                            ASN1_C_NULL,\n", outfile);
	    fputs("                            &null_instance_octet);\n", outfile);
	    fputs("    if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;\n\n", outfile);
	}
    }

    return SS$_NORMAL;

}
