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
static char *rcsid = "@(#)$RCSfile: mg_make.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:55:17 $";
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
**  FACILITY:  Ultrix MOM Generator
**
**  MODULE DESCRIPTION:
**
**	This module contains the routines to used to build the makefile and
**	the c shell script file used to build the MOM.
**
**  AUTHORS:
**
**      Mike Densmore
**
**  CREATION DATE:  27-May-1992
**
**  MODIFICATION HISTORY:
**
**      19-Aug-1992   Adam Peller   Added code to support Ultrix makefiles.
**                                  Substitutes makefile macros for code
**                                  markers in the template file.
**
**	25-Sep-1992   Mike Densmore Added mg_prototypes.h include
**					ANSI C
**
**      16-Dec-1992   M. Ashraf     Made Makefile <class>_SRC/OBJ case-
**                                    sensitive for SNMP
**
**       3-Mar-1993   M. Ashraf     Updated for IPC changes
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#include "mom_defs.h"
#include <ctype.h>
#include <string.h>
#include "mg_prototypes.h"


char *string_toupper( s )

char *s;

{
    int i;
    char *u;

    u = (char *) malloc( strlen(s) + 1 );
    for (i=0; i<strlen(s); i++)
      u[i] = toupper(s[i]);
    u[i] = '\0';

    return u;
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_makefile_macro_file_list
**
**          Insert macro in ULTRIX Makefile
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
int insert_makefile_macro_file_list( mom,
		                     outfile)

MOM_BUILD *mom;
FILE *outfile;

{
    CLASS_DEF	    *class;
    int		    class_num;
    char	    buffer[256];
    int		    status;
    char            *upper;
    
    class = mom->class;

    for (class_num=0; class_num < mom->num_classes; class_num++)
    {
      if ((mom->oid_type == DNA_CMIP_OID) || (mom->oid_type == OSI_CMIP_OID))
      {
	upper = string_toupper(class->prefix);
	sprintf(buffer, "%sSRC = \\\n", upper);
      }
      else
	sprintf(buffer, "%sSRC = \\\n", class->prefix);

	fputs(buffer, outfile);

	sprintf(buffer,
		"\t%sperform_init.c \\\n",
		class->prefix);
	fputs(buffer, outfile);

	if (mom->support_get)
	    {
	    sprintf(buffer,
		"\t%sperform_get.c \\\n",
		class->prefix);
	    fputs(buffer, outfile);
	    }
	if (mom->support_set)
	    {
	    sprintf(buffer,
		"\t%sperform_set.c \\\n",
		class->prefix);
	    fputs(buffer, outfile);
            }
	if (mom->support_getnext)
	    {
	    sprintf(buffer,
		"\t%sperform_getnext.c \\\n",
		class->prefix);
	    fputs(buffer, outfile);
            }
	if (mom->support_create)
	    {
	    sprintf(buffer,
		"\t%sperform_create.c \\\n",
		class->prefix);
	    fputs(buffer, outfile);
            }
	if (mom->support_delete)
	    {
	    sprintf(buffer,
		"\t%sperform_delete.c \\\n",
		class->prefix);
	    fputs(buffer, outfile);
            }

	sprintf(buffer,
		"\t%sget_instance.c \\\n",
		class->prefix);
	fputs(buffer, outfile);

	sprintf(buffer,
		"\t%sfind_instance.c \n",
		class->prefix);
	fputs(buffer, outfile);

      if ((mom->oid_type == DNA_CMIP_OID) || (mom->oid_type == OSI_CMIP_OID))
	sprintf(buffer,
		"\n%sOBJ = $(%sSRC:.c=.o)\n\n",
		upper, upper); 
      else
	sprintf(buffer,
		"\n%sOBJ = $(%sSRC:.c=.o)\n\n",
		class->prefix, class->prefix);

	fputs(buffer, outfile);

      if ((mom->oid_type == DNA_CMIP_OID) || (mom->oid_type == OSI_CMIP_OID))
	free (upper);

	class = class->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_makefile_macro_src
**
**          Insert macro in ULTRIX Makefile
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
int insert_makefile_macro_src( mom,
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
	/* Print all upper case if CMIP, else case-sensitive! */

      if ((mom->oid_type == DNA_CMIP_OID) || (mom->oid_type == OSI_CMIP_OID))
	sprintf(buffer, "\t$(%sSRC) \\\n",
		string_toupper(class->prefix)); 
      else
	sprintf(buffer, "\t$(%sSRC) \\\n",
		class->prefix);

	fputs(buffer, outfile);

	class = class->next;
    }

    return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      insert_makefile_macro_obj
**
**          Insert macro in ULTRIX Makefile
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
int insert_makefile_macro_obj( mom,
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
	/* Print all upper case if CMIP, else case-sensitive! */

      if ((mom->oid_type == DNA_CMIP_OID) || (mom->oid_type == OSI_CMIP_OID))
	sprintf(buffer,
		"\t$(%sOBJ) \\\n",
		string_toupper(class->prefix)); 
      else
	sprintf(buffer,
		"\t$(%sOBJ) \\\n",
		class->prefix);

	fputs(buffer, outfile);

	class = class->next;
    }

    return SS$_NORMAL;
}
