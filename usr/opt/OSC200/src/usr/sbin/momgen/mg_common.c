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
static char *rcsid = "@(#)$RCSfile: mg_common.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:54:50 $";
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
**	This module contains common routines used when reading the MIR or
**	the MCC dictionary.  
**
**  AUTHORS:
**
**      Gary J. Allison
**
**  CREATION DATE:  10-OCT-1991
**
**  MODIFICATION HISTORY:
**
**	Marc Nozell	15-Jun-1992	    Improve portability
**  	    	    - removed references to module. 
**
**	Mike Densmore	    19-May-1992	    Modified to port to Ultrix
**		    - define module ident
**		    - ifdef out unused VMS include files
**		    - fixed declarations for "tmp" in 3 routines:
**		      insert_attr, insert_dir, insert_event
**		    - cast the malloc in set_dup_char (causes warning in mips)
**
**	Mike Densmore	    3-Jun-1992	    Moved common routines from MG_CLI
**					    and MG_PARSE to this module:
**		    - add_class()
**		    - add_oid()
**		    - read_info_file()
**		    - remove_quotes()
**					    Fixed various casting and mips
**					    compiler warnings (declaration
**					    of "entity" has to be static)
**
**	Mike Densmore	    23-Sep-1992	    Added include of prototypes
**					    file and made various fixes
**					    for ANSI C compatibility
**	M. Ashraf	     3-Mar-1993	    Updated for IPC changes
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#define NAME_SIZE 5

#define buffer_size 256
#define TRUE 1
#define FALSE 0

#include <ctype.h>
#include "mom_defs.h"
#include "iso_defs.h"
#include "moss.h"
#include "mg_prototypes.h"

#ifndef UNIXPLATFORM
#include <descrip.h>
#include <lnmdef.h>
#endif

#include <string.h>

remove_quotes( str )

char *str;

{
int size,i;
size = strlen( str );

for (i=0; i<size; i++)	/* Replace the .'s with ,'s */
    {
    if ((str[i] == '\'') || (str[i] == '\"'))
   	str[i] = ' ';
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	check_identifier_attribute
**
**	This routine checks to see if there is an identifier attribute of
**	1 for DNA_CMIP.
**
**  FORMAL PARAMETERS:
**
**	mom		Pointer to the MOM description block.
**      class		Pointer to the class
**
**  RETURN VALUE:
**
**      none
**
**--
*/
void check_identifier_attribute( mom, 
				 class)

MOM_BUILD *mom;
CLASS_DEF *class;

{
int	       dna_cmip = 0;
ATTRIBUTE_DEF *attribute;
int	       attribute_num;

/*
 * Return if there are no identifier attributes
 */

if (!class->support_instances)
    return;

attribute = class->attribute;
for (attribute_num=0; attribute_num < class->num_attributes; attribute_num++)
    {
    if (attribute->dna_cmip_int_len != 0)
	{
        dna_cmip = 1;
    	if ((attribute->group == GROUP_IDENTIFIERS) && (attribute->attribute_number == 1))
            return;
	}
    attribute = attribute->next;
    }

return;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	up_case
**
**	This routine uppercases a string.
**
**  FORMAL PARAMETERS:
**
**	s 		pointer to string to upcase.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	up_case (s)
char *s;
{
int ii,size;
size = strlen(s);
for (ii=0; ii<size; ii++)
 s[ii] = toupper( s[ii] );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	get_attr_class
**
**	This routine returns an attribute block based on the attribute number.
**
**  FORMAL PARAMETERS:
**
**	class		Pointer to current class
**	num		Number of attribute to retrieve
**	attr		Return address of pointer to attribute block
**
**  RETURN VALUE:
**
**      None
**
**--
*/
int get_attr_class( class, 
		    num, 
		    attr )

CLASS_DEF *class;
int num;
ATTRIBUTE_DEF **attr;

{
ATTRIBUTE_DEF *tmp;

tmp = class->attribute;

while (tmp->attribute_number!=num)
    { if (tmp->next == NULL)
          return (0);
      tmp=tmp->next;
    }
*attr = tmp;
return (1);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_attr
**
**	This routine inserts an attribute into the class attribute list.
**
**  FORMAL PARAMETERS:
**
**	attr		Pointer of attribute to insert
**	class		Pointer of current class
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_attr( attr, 
	             class )

ATTRIBUTE_DEF *attr;
CLASS_DEF *class;

{
ATTRIBUTE_DEF	*tmp;

tmp = class->attribute;

if (tmp==NULL)
    class->attribute = attr;
else
    {
    while (tmp->next!=NULL)
        tmp=tmp->next;
    tmp->next = attr;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_dir
**
**	This routine inserts a directive into the class directive list.
**
**  FORMAL PARAMETERS:
**
**	dir		Pointer of directive to insert
**	class		Pointer of current class
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_dir( dir, 
	            class )

DIRECTIVE_DEF *dir;
CLASS_DEF *class;

{
DIRECTIVE_DEF	*tmp;

tmp = class->directive;

if (tmp==NULL)
    class->directive = dir;
else
    {
    while (tmp->next!=NULL)
        tmp=tmp->next;
    tmp->next = dir;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	insert_event
**
**	This routine inserts an event into the class event list.
**
**  FORMAL PARAMETERS:
**
**	event 		Pointer of event to insert
**	class		Pointer of current class
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	insert_event( event, 
	              class )

EVENT_DEF *event;
CLASS_DEF *class;

{
EVENT_DEF   *tmp;

tmp = class->event;

if (tmp==NULL)
    class->event = event;
else
    {
    while (tmp->next!=NULL)
        tmp=tmp->next;
    tmp->next = event;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	print_class
**
**	This routine prints out the attribute information for a class
**
**  FORMAL PARAMETERS:
**
**	class		Pointer of current class
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	print_class( mom, class )

MOM_BUILD *mom;
CLASS_DEF *class;

{
ATTRIBUTE_DEF *attr;
int i;

if (!mom->log)
    return;

attr = class->attribute;

for (i=0;i<class->num_attributes;i++)
    {
    fprintf(mom->log_file,"Attribute name  : %s\n",attr->attribute_name );
    fprintf(mom->log_file,"Attribute type  : %s\n",attr->data_type);
    fprintf(mom->log_file,"Attribute length: %s\n",attr->length);
    fprintf(mom->log_file,"Attribute group : %d\n\n",attr->group);

    attr = attr->next;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      print_oid
**
**	This routine reads an OID to standard output.
**
**  FORMAL PARAMETERS:
**
**	oid		Pointer to OID to print.
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	print_oid( mom, oid )

MOM_BUILD *mom;
object_id *oid;

{
int i;

fprintf(mom->log_file,"Found OID: ");
for (i=0; i < oid->count; i++) 
    fprintf(mom->log_file, ((i==0) ? "%d" : ".%d"),oid->value[i]);

fprintf(mom->log_file,"\n");
}

int check_duplicate_arg( dir,
			 class,
		         arg,
			 arg_char)

DIRECTIVE_DEF *dir;
CLASS_DEF *class;
ARGUMENT_DEF *arg;
char *arg_char;

{
int found=0;
EVENT_DEF *event_tmp;
REQUEST_DEF *req_tmp;
EXCEPTION_DEF *exc_tmp;
RESPONSE_DEF *resp_tmp;
ARGUMENT_DEF *argument = NULL;

/*
 * First check to see if there are any duplicate arguments under responses.
 */

resp_tmp = dir->resp;
while ((resp_tmp != NULL) && !found)
    {
    argument = resp_tmp->arg;

    while (argument != NULL)
        {
        if (arg != argument) 	 /* Skip the one we're looking for */
            if (strcmp((arg->argument_name+NAME_SIZE),(argument->argument_name+NAME_SIZE)) == 0)
                {
                found = 1;
		break;
		}
        argument = argument->next;
        }
    resp_tmp=resp_tmp->next;
    }

/*
 * Now check for duplicates under requests.
 */

req_tmp = dir->req;
while ((req_tmp != NULL) && !found )
    {
    argument = req_tmp->arg;

    while (argument != NULL)
        {
        if (arg != argument) /* Skip the one we're looking for */
            if (strcmp((arg->argument_name+NAME_SIZE),(argument->argument_name+NAME_SIZE)) == 0)
                {
                found = 1;
		break;
		}
        argument = argument->next;
        }
    req_tmp=req_tmp->next;
    }

/*
 * Now, check for duplicates under exceptions. 
 */

exc_tmp = dir->exc;
while ((exc_tmp != NULL) && !found)
    {
    argument = exc_tmp->arg;

    while (argument != NULL)
        {
        if (arg != argument) /* Skip the one we're looking for */
            if (strcmp((arg->argument_name+NAME_SIZE),(argument->argument_name+NAME_SIZE)) == 0)
                {
                found = 1;
		break;
		}
        argument = argument->next;
        }
    exc_tmp=exc_tmp->next;
    }

/*
 * Now, check for duplicates under events
 */

event_tmp = class->event;
while ((event_tmp != NULL) && !found)
    {
    argument = event_tmp->event_arg;

    while (argument != NULL)
        {
        if (arg != argument) /* Skip the one we're looking for */
            if (strcmp((arg->argument_name+NAME_SIZE),(argument->argument_name+NAME_SIZE)) == 0)
                {
                found = 1;
		break;
		}
        argument = argument->next;
        }
    event_tmp=event_tmp->next;
    }

if (found)
    {
    dir->current_dup_char++;
    if (dir->current_dup_char < 10)
	{
    	sprintf( arg_char, "%1d", dir->current_dup_char);
	arg_char[1] = '\0';
	}
    else if (dir->current_dup_char < 100)
	{
    	sprintf( arg_char, "%2d", dir->current_dup_char);
	arg_char[2] = '\0';
	}
    else 
	{
    	sprintf( arg_char, "%3d", dir->current_dup_char);
	arg_char[3] = '\0';
	}
    return 1;
    }

return 0;
}

int check_duplicate_evt_arg( class,
		             arg,
			     arg_char)

CLASS_DEF *class;
ARGUMENT_DEF *arg;
char *arg_char;

{
int found=0;
EVENT_DEF *event_tmp;
ARGUMENT_DEF *argument = NULL;

/*
 * Check for duplicates under events
 */

event_tmp = class->event;
while ((event_tmp != NULL) && !found)
    {
    argument = event_tmp->event_arg;

    while (argument != NULL)
        {
        if (arg != argument) /* Skip the one we're looking for */
            if (strcmp((arg->argument_name+NAME_SIZE),(argument->argument_name+NAME_SIZE)) == 0)
                {
                found = 1;
		break;
		}
        argument = argument->next;
        }
    event_tmp=event_tmp->next;
    }

if (found)
    {
    class->current_dup_char_evt_arg++;
    if (class->current_dup_char_evt_arg < 10)
	{
    	sprintf( arg_char, "%1d", class->current_dup_char_evt_arg);
	arg_char[1] = '\0';
	}
    else if (class->current_dup_char_evt_arg < 100)
	{
    	sprintf( arg_char, "%2d", class->current_dup_char_evt_arg);
	arg_char[2] = '\0';
	}
    else 
	{
    	sprintf( arg_char, "%3d", class->current_dup_char_evt_arg);
	arg_char[3] = '\0';
	}
    return 1;
    }

return 0;
}

int check_duplicate_exc( class,
			 dir,
		         exc )

CLASS_DEF *class;
DIRECTIVE_DEF *dir;
EXCEPTION_DEF *exc;

{
EXCEPTION_DEF *exception;

exception = dir->exc;
while (exception != NULL)
   {
   if (exc != exception) /* Skip the one we're looking for */
       if (strcmp((exc->exception_name+NAME_SIZE),(exception->exception_name+NAME_SIZE)) == 0)
           return 1;

   exception = exception->next;
   }

return 0;
}

int check_duplicate_req( class,
			 dir,
		         req )

CLASS_DEF *class;
DIRECTIVE_DEF *dir;
REQUEST_DEF *req;

{
REQUEST_DEF *request;

request = dir->req;
while (request != NULL)
   {
   if (req != request) /* Skip the one we're looking for */
       if (strcmp((req->request_name+NAME_SIZE),(request->request_name+NAME_SIZE)) == 0)
           return 1;

   request = request->next;
   }

return 0;
}

int check_duplicate_resp( class,
			  dir,
		          resp )

CLASS_DEF *class;
DIRECTIVE_DEF *dir;
RESPONSE_DEF *resp;

{
RESPONSE_DEF *response;

response = dir->resp;
while (response != NULL)
   {
   if (resp != response) /* Skip the one we're looking for */
       if (strcmp((resp->response_name+NAME_SIZE),(response->response_name+NAME_SIZE)) == 0)
           return 1;

   response = response->next;
   }

return 0;
}

int check_duplicate_event( class,
		           evt )

CLASS_DEF *class;
EVENT_DEF *evt;

{
EVENT_DEF *event;

event = class->event;
while (event != NULL)
   {
   if (evt != event) /* Skip the one we're looking for */
       if (strcmp((evt->event_name+NAME_SIZE),(event->event_name+NAME_SIZE)) == 0)
           return 1;

   event = event->next;
   }

return 0;
}

int check_duplicate_create_arg( class,
		                arg )

CLASS_DEF *class;
ARGUMENT_DEF *arg;

{
ARGUMENT_DEF *argument;

argument = class->create_arg;
while (argument != NULL)
   {
   if (arg != argument) /* Skip the one we're looking for */
       if (strcmp((arg->argument_name+NAME_SIZE),(argument->argument_name+NAME_SIZE)) == 0)
           return 1;

   argument = argument->next;
   }

return 0;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	check_duplicate
**	
**	This routine checks to see if there are duplicate attributes.
**
**  FORMAL PARAMETERS:
**
**	class		Pointer to current class
**	attr		Pointer to attribute to check
**
**  RETURN VALUE:
**
**      1 - if there are duplicates
**	0 - if there are not
**
**--
*/
int check_duplicate( class, 
		     attr )

CLASS_DEF *class;
ATTRIBUTE_DEF *attr;

{
ATTRIBUTE_DEF *attribute;

attribute = class->attribute;
while (attribute != NULL)
   {
   if (strcmp((attr->attribute_name+NAME_SIZE),(attribute->attribute_name+NAME_SIZE)) == 0)
       return 1;
   else
       attribute = attribute->next;
   }

return 0;
}
void set_arg_char( name, arg_char )

char *name;
char *arg_char;

{
int len,size;

len = strlen( name );
size = strlen( arg_char );

if (size == 1)
    name[ len - 1 ] = arg_char[ 0 ];

if (size == 2)
    {
    name[ len - 2 ] = arg_char[ 0 ];
    name[ len - 1 ] = arg_char[ 1 ];
    }

if (size == 3)
    {
    name[ len - 3 ] = arg_char[ 0 ];
    name[ len - 2 ] = arg_char[ 1 ];
    name[ len - 1 ] = arg_char[ 2 ];
    }

}

void set_dup_char( name, cur_dup_num )

char *name;
int *cur_dup_num;

{
int len;
char *arg_char;
char achar[3];

len = strlen( name );

arg_char = (char *)malloc(3);

(*cur_dup_num)++;

if (*cur_dup_num < 10)
    {
    sprintf( arg_char, "%1d", *cur_dup_num);
    name[ len - 1 ] = arg_char[ 0 ];
    }
else if (*cur_dup_num < 100)
    {
    sprintf( arg_char, "%1d", *cur_dup_num);
    name[ len - 2 ] = arg_char[ 0 ];
    name[ len - 1 ] = arg_char[ 1 ];
    }
else 
    {
    sprintf( arg_char, "%1d", *cur_dup_num);
    name[ len - 3 ] = arg_char[ 0 ];
    name[ len - 2 ] = arg_char[ 1 ];
    name[ len - 1 ] = arg_char[ 2 ];
    }

free( arg_char );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	add_class
**
**	This routine allocates a class and sets up the class string.
**
**  FORMAL PARAMETERS:
**
**      buf		String containing class string from CLI.
**	mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**	MOMGEN_C_INSVIRMEM 	- Insufficient Virtual Memory
**
**--
*/
int add_class( buf, mom )

char *buf;
MOM_BUILD *mom;

{
static	unsigned int entity[ ENTITIES_LENGTH ] = { ENTITIES_SEQ };
int status,class_number;
CLASS_DEF *newclass,*tmp;
char *x, *str;
int len,i,size;
int steps_below_node = 0;

/* Allocate class and add to MOM structure */

newclass = (CLASS_DEF *) calloc (1, sizeof (CLASS_DEF));
if (newclass == NULL)
    return MOMGEN_C_INSVIRMEM;

newclass->ACL_attribute_num = -1;
newclass->ACL_argument_num = -1;

tmp = mom->class;
if (tmp==NULL)
    mom->class = newclass;
else
    {
    while (tmp->next!=NULL)
        tmp=tmp->next;
    tmp->next = newclass;
    }

len = strlen( buf );
i = 0;
while( i < len )
{
    x = &buf[i];
    while( isspace(buf[i]) || isdigit(buf[i]) && ( i < len ))
        i++;
    if (i==len) /* clear larger old buffer */
        buf[i]=0;
    class_number = atoi(x);

    i++; /* Skip past period */

    steps_below_node++;
}
newclass->steps_below_node = --steps_below_node; /* Remove the top level node */

newclass->class_string = (char *)malloc( len + 1);
if (newclass->class_string == NULL)
    return MOMGEN_C_INSVIRMEM;
strcpy( newclass->class_string, buf);

newclass->orig_class_string = (char *)malloc( len + 1);
if (newclass->orig_class_string == NULL)
    return MOMGEN_C_INSVIRMEM;
strcpy( newclass->orig_class_string, buf);

size = strlen(newclass->class_string);
str = (char *) newclass->class_string;
for (i=0; i<size; i++)	/* Replace the .'s with ,'s */
    {
    if (str[i] == '.')	
   	str[i] = ',';
    }

return 1;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	add_oid
**
**	This routine allocates a class and an OID describing the 
**	class/instance pair from the input descriptor. 
**
**  FORMAL PARAMETERS:
**
**      newclass 	Pointer to class.
**	mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**	MOMGEN_C_INSVIRMEM 	- Insufficient Virtual Memory
**
**--
*/
int add_oid( newclass, mom )

CLASS_DEF *newclass;
MOM_BUILD *mom;

{
static	unsigned int entity[ ENTITIES_LENGTH ] = { ENTITIES_SEQ };
int status,class_number;
CLASS_DEF *tmp;
char *buf;
char *x, *str;
int parent_number,len,i,size;
int steps_below_node = 0;

/*
 * First, set up the appropriate object id.  The user specified 1.99 (starting
 * from global node). Start with the common prefix - ENTITIES_SEQ currently, 
 * (1.3.12.2.1011.2.1) and then add the user supplied class string.
 */

if (mom->OID_EMA)
    {
    status = moss_create_oid ( ENTITIES_LENGTH, entity, &newclass->class_oid );
    if (ERROR_CONDITION( status ))
        return status;
    parent_number = 1;
    }
else
    {
    status = moss_create_oid ( 0, entity, &newclass->class_oid );
    if (ERROR_CONDITION( status ))
        return status;
    }

buf = newclass->orig_class_string;
len = strlen( buf );
i = 0;
while( i < len )
{
    x = &buf[i];
    while( isspace(buf[i]) || isdigit(buf[i]) && ( i < len ))
        i++;
    if (i==len) /* clear larger old buffer */
        buf[i]=0;
    parent_number = class_number;
    class_number = atoi(x);

    i++; /* Skip past period */

    status = moss_oid_append( newclass->class_oid, class_number, &newclass->class_oid );    
    if (ERROR_CONDITION( status ))
    	return status;

}

if (mom->OID_EMA)
    {
    newclass->dna_cmip_int = class_number;
    newclass->parent_dna_cmip_int = parent_number;
    newclass->parent_dna_cmip_int_len = 1;
    newclass->dna_cmip_int_len = 1;
    }

return 1;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	read_info_file
**
**	This routine reads the information file specified in the INFO_FILE
**	parameter and allocates a string in the MOM for the class(es), 
**	symbol prefix(es), MOM Name, copyright_owner, author, 
**	organization, facility, include_file, msl_file.
**
**	If /OUTPUT is specified, the info file is echoed to sys$output.
**
**  FORMAL PARAMETERS:
**
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL		- Normal successful completion.
**	MOMGEN_C_INSVIRMEM 	- Insufficient Virtual Memory
**	MOMGEN_C_ABSENT 	- Qualifier absent
**	MOMGEN_C_VALUE_REQUIRED - Qualifier value required
**	MOMGEN_C_NO_CLASSES     - NO classes
**	MOMGEN_C_INV_NUM_PARENT_CLASSES - Invalid number of parent classes
**	MOMGEN_C_NO_AUTHOR	- No author specified
**	MOMGEN_C_NO_FACILITY	- No facility specified
**	MOMGEN_C_NO_ORGANIZATION - No organization specified
**	MOMGEN_C_NO_PARENT_CLASS - No parent class was specified.
**	MOMGEN_C_NO_CLASS_PREFIX - No Class prefix was specified.
**	MOMGEN_C_INV_NUM_PREFIXES - Invalid number of class prefixes.
**	MOMGEN_C_NO_MOM_NAME	- No MOM Name was specified.
**--
*/
int read_info_file( mom )
MOM_BUILD *mom;
{
#define MAX_PREFIX 13

int	size,class_num;
CLASS_DEF *class;
char    buffer[buffer_size];
char 	*keyword,*x;
FILE    *infile;
int	parent_num=0,num=0,len,i,status,stat,ppnum;
int	ACL_argument_num=0,ACL_attribute_num=0;
object_id *temp_oid;
char	zero[2];	/* temporary fix for handling root oids (parent is root) */

    zero[0] = '0';
    zero[1] = '\0';

/*
 * Open the info file.
 */

infile = fopen(mom->info_file, "r" );
if (infile == NULL) 
    {
    fprintf(stderr, "Can't read parameter file: %s\n", mom->info_file);
    return MOMGEN_C_OPENREADERROR;
    }


while ((fgets( buffer, buffer_size, infile)) != '\0')
    {
    i = 0;
    buffer [strlen(buffer)-1] = 0;  /* Remove EOL */
    len = strlen(buffer);

    while (isspace(buffer[i]))
	i++;
    keyword = &buffer[i];

    /*
     * Find out the KEYWORD.
     */

    while ((i < len) && (buffer[i] != '='))
	i++;

    if (i < len)
	{
	buffer[i++] = 0;
	
	up_case(keyword);
	if (strncmp(keyword,"CLASS",5) == 0)
	    {	

	    /*
             * Check for CLASSES =This parameter is required.
             */

    	    while (isspace(buffer[i]))
	        i++;

	    remove_quotes( &buffer[i] );

	    mom->num_classes++;

            stat = add_class( &buffer[i], mom );
	    if (ERROR_CONDITION(stat))
    	        return stat;
            
            if (mom->num_classes > 1)
                mom->multiclass = TRUE;

            class = mom->class;
	    for (class_num = 0; class_num < mom->num_classes-1; class_num++)
    		class = class->next;
    	    if (mom->log)
    	    	fprintf(mom->log_file,"Class:           %s\n",class->orig_class_string);
	    }   

	else if (strncmp(keyword,"PREFIX",6) == 0)
            {	
	    /*
	     * Check for /PREFIX
	     * class->prefix_k = class prefix + "K_" (e.g., MP_K_)
	     * class->prefix_dna = class prefix + "D_"
	     * class->prefix_mcc = class prefix + "M_"
	     * class->prefix_m = class prefix (eg. MP_)
	     */
    	    while (isspace(buffer[i]))
	        i++;

	    remove_quotes( &buffer[i] );

	    x = &buffer[i];
	
	    /* Get the right class... ALways use last class in list. */
	    num++;
	    if (mom->num_classes == 0)
		return MOMGEN_C_NO_CLASS_PREFIX;

	    if (num > mom->num_classes)
	    	return MOMGEN_C_INV_NUM_PREFIXES;

            class = mom->class;
	    for (class_num = 0; class_num < num - 1; class_num++)
    		class = class->next;

            class->prefix = (char *)malloc (strlen(x) + 1);
            if (class->prefix == NULL)
                return MOMGEN_C_INSVIRMEM;

	    strcpy(class->prefix,x);

/*
**  Replace with algorithm to produce "unique short prefix"
**	    if (strlen( class->prefix)  > MAX_PREFIX)
**	        return MOMGEN_C_INV_INFO_FILE;
*/
    	    if (mom->log)
    	    	fprintf(mom->log_file,"Prefix:          %s\n",class->prefix);

    	    class->prefix_k = (char *) malloc( strlen(class->prefix) + 
				       strlen(mom->prefix) + 1 );
	    if (class->prefix_k == NULL)
		return MOMGEN_C_INSVIRMEM;
	
	    strcpy( class->prefix_k, class->prefix );
	    strcat( class->prefix_k, mom->prefix );

    	    class->prefix_dna = (char *) malloc( strlen(class->prefix) + 
				       strlen(dna_prefix) + 1 );
	    if (class->prefix_dna == NULL)
		return MOMGEN_C_INSVIRMEM;
	
	    strcpy( class->prefix_dna, class->prefix );
	    strcat( class->prefix_dna, dna_prefix );

    	    class->prefix_mcc = (char *) malloc( strlen(class->prefix) + 
				       strlen(mcc_prefix) + 1 );
	    if (class->prefix_mcc == NULL)
		return MOMGEN_C_INSVIRMEM;
	
	    strcpy( class->prefix_mcc, class->prefix );
	    strcat( class->prefix_mcc, mcc_prefix );
	
	    class->prefix_m = (char *) malloc( strlen(class->prefix) + 2 );
	    if (class->prefix_m == NULL)
	        return MOMGEN_C_INSVIRMEM;
	
	    strcpy( class->prefix_m, class->prefix );
		
	    class = class->next;
	    }

	else if (strncmp(keyword,"PARENT_PREFIX",13) == 0)
            {	
	    /*
	     * Check for /PARENT_PREFIX
	     * used for SNMP getnext only
	     */
    	    while (isspace(buffer[i]))
	        i++;

	    remove_quotes( &buffer[i] );

	    x = &buffer[i];
	
	    /* Get the right class... ALways use last class in list. */
	    if (mom->num_classes == 0)
		return MOMGEN_C_NO_CLASS_PREFIX;

	    ppnum = mom->num_classes;
            class = mom->class;
	    for (class_num = 0; class_num < ppnum - 1; class_num++)
    		class = class->next;

            class->parent_prefix = (char *)malloc (strlen(x) + 1);
            if (class->parent_prefix == NULL)
                return MOMGEN_C_INSVIRMEM;

	    strcpy(class->parent_prefix,x);

    	    if (mom->log)
    	    	fprintf(mom->log_file,"Parent_Prefix:          %s\n",class->parent_prefix);

	    }

	else if (strncmp(keyword,"PARENT",6) == 0)
            {	
	    char *str;
	    /*
	     * Check for PARENT
	     */
    	    while (isspace(buffer[i]))
	        i++;

	    remove_quotes( &buffer[i] );
	
	    x = &buffer[i];

	    /* Get the right class... ALways use last class in list. */
	    parent_num++;

	    if (mom->num_classes == 0)
		return MOMGEN_C_NO_CLASS_PARENT;

	    if (parent_num > mom->num_classes)
	    	return MOMGEN_C_INV_NUM_PARENT_CLASSES;

            class = mom->class;
	    for (class_num = 0; class_num < parent_num - 1; class_num++)
    		class = class->next;

            class->orig_parent_class_string = (char *)malloc (strlen(x) + 1);
            if (class->orig_parent_class_string == NULL)
                return MOMGEN_C_INSVIRMEM;

	    strcpy(class->orig_parent_class_string,x);

    	    if (mom->log)
    	    	fprintf(mom->log_file,"Parent class:    %s\n",class->orig_parent_class_string);
	
            class->parent_class_string = (char *)malloc (strlen(class->orig_parent_class_string) + 1);
	    strcpy(class->parent_class_string,class->orig_parent_class_string);

	    str = (char *) class->parent_class_string;
	    size = strlen( str );
	    for (i=0; i<size; i++)	/* Replace the .'s with ,'s */
    		{
    		if (str[i] == '.')	
   		    str[i] = ',';
    		}

	    /* Get length of parent OID */  /* if clause is temporary hack, else is permanent */

	    if (strcmp(class->parent_class_string,zero) == 0 )
		class->parent_class_string_len = 0;
	    else {
	
		status = moss_text_to_oid( class->parent_class_string, &temp_oid );
		status = moss_get_oid_len( temp_oid, &class->parent_class_string_len );	   
		}
	    }

	else if (strncmp(keyword,"MOM_NAME",8) == 0)
            {	
	    /*
	     * Check for MOM Name
	     */
    	    while (isspace(buffer[i]))
	        i++;

	    remove_quotes( &buffer[i] );

            size = strlen(&buffer[i]);
	    mom->mom_name = (char *)malloc (size + 1);
    	    if (mom->mom_name == NULL)
        	return MOMGEN_C_INSVIRMEM;
    	    strcpy( mom->mom_name, &buffer[i] );
	    mom->mom_name[ size ] = '\0';
    	    if (mom->log)
        	fprintf(mom->log_file,"MOM Name:        %s\n",mom->mom_name);
	    }

	else if (strncmp(keyword,"COPYRIGHT_OWNER",15) == 0)
            {	
	    /*
	     * Check for COPYRIGHT_OWNER
	     */
    	    while (isspace(buffer[i]))
	        i++;

	    remove_quotes( &buffer[i] );

            size = strlen(&buffer[i]);
	    mom->copyright_owner = (char *)malloc (size + 1);
    	    if (mom->copyright_owner == NULL)
        	return MOMGEN_C_INSVIRMEM;
	    strcpy( mom->copyright_owner, &buffer[i] );
	    mom->copyright_owner[ size ] = '\0';
	    if (mom->log)
	    	fprintf(mom->log_file,"Copyright Owner: %s\n",mom->copyright_owner);
	    }

	else if (strncmp(keyword,"AUTHOR",6) == 0)
            {	
	    /*
 	     * Check for AUTHOR
	     */
    	    while (isspace(buffer[i]))
	        i++;

	    remove_quotes( &buffer[i] );

            size = strlen(&buffer[i]);
	    mom->author = (char *)malloc (size + 1);
	    if (mom->author == NULL)
	        return MOMGEN_C_INSVIRMEM;
	    strcpy( mom->author, &buffer[i] );
	    mom->author[ size ] = '\0';
	    if (mom->log)
		fprintf(mom->log_file,"Author:          %s\n",mom->author);
	    }

	else if (strncmp(keyword,"ORGANIZATION",12) == 0)
            {	
	    /*
 	     * Check for ORGANIZATION
	     */
    	    while (isspace(buffer[i]))
	        i++;

	    remove_quotes( &buffer[i] );

            size = strlen(&buffer[i]);
	    mom->organization = (char *)malloc (size + 1);
	    if (mom->organization == NULL)
	        return MOMGEN_C_INSVIRMEM;
	    strcpy( mom->organization, &buffer[i] );
	    mom->organization[ size ] = '\0';
 	    if (mom->log)
	       fprintf(mom->log_file,"Organization:    %s\n",mom->organization);
	    }

	else if (strncmp(keyword,"FACILITY",8) == 0)
            {	
	    /*
	     * Check for FACILITY
	     */
    	    while (isspace(buffer[i]))
	        i++;
	    remove_quotes( &buffer[i] );

            size = strlen(&buffer[i]);
	    mom->facility = (char *)malloc (size + 1);
	    if (mom->facility == NULL)
	        return MOMGEN_C_INSVIRMEM;
	    strcpy( mom->facility, &buffer[i] );
	    mom->facility[ size ] = '\0';
	    if (mom->log)
	    	fprintf(mom->log_file,"Facility:        %s\n",mom->facility);
	    }
	else if (strncmp(keyword,"SYSTEM",6) == 0)
            {	
	    /*
	     * Check for SYSTEM (OPTIONAL)
	     */
    	    while (isspace(buffer[i]))
	        i++;
	    remove_quotes( &buffer[i] );
            size = strlen(&buffer[i]);
	    mom->system = (char *)malloc (size + 1);
	    if (mom->system == NULL)
	        return MOMGEN_C_INSVIRMEM;
	    strcpy( mom->system, &buffer[i] );
	    mom->system[ size ] = '\0';
	    if (mom->log)
	    	fprintf(mom->log_file,"System:          %s\n",mom->system);
	    }

	else if (strncmp(keyword,"NUM_THREADS",11) == 0)
            {	
	    int x;
	    /*
	     * Check for NUM_THREADS (OPTIONAL)
	     */
    	    while (isspace(buffer[i]))
	        i++;
	    x = atoi(&buffer[i]);
	    if ((x > 0) && (x < MAX_THREADS))
	        mom->num_threads = x;
	    if (mom->log)
	    	fprintf(mom->log_file,"Threads:         %d\n",mom->num_threads);
	    }

	else if (strncmp(keyword,"ACL_ATTRIBUTE",13) == 0)
            {	
	    int x;
	    /*
	     * Check for ACL_ATTRIBUTE (OPTIONAL)
	     */
    	    while (isspace(buffer[i]))
	        i++;
	    x = atoi(&buffer[i]);
	
	    ACL_attribute_num++;

	    if (mom->num_classes == 0)
		return MOMGEN_C_NO_CLASS_ACL;

	    if (ACL_attribute_num > mom->num_classes)
	    	return MOMGEN_C_INV_NUM_ACL_CLASSES;

            class = mom->class;
	    for (class_num = 0; class_num < ACL_attribute_num - 1; class_num++)
    		class = class->next;

	    class->ACL_attribute_num = x;

	    if (mom->log)
	    	fprintf(mom->log_file,"ACL_ATTRIBUTE:   %d\n",class->ACL_attribute_num);
	    }

	else if (strncmp(keyword,"ACL_ARGUMENT",12) == 0)
            {	
	    int x;
	    /*
	     * Check for ACL_ARGUMENT (OPTIONAL)
	     */
    	    while (isspace(buffer[i]))
	        i++;
	    x = atoi(&buffer[i]);
	
	    ACL_argument_num++;

	    if (mom->num_classes == 0)
		return MOMGEN_C_NO_CLASS_ACL;

	    if (ACL_argument_num > mom->num_classes)
	    	return MOMGEN_C_INV_NUM_ACL_CLASSES;

            class = mom->class;
	    for (class_num = 0; class_num < ACL_argument_num - 1; class_num++)
    		class = class->next;

	    class->ACL_argument_num = x;

	    if (mom->log)
	    	fprintf(mom->log_file,"ACL_ARGUMENT:    %d\n",class->ACL_argument_num);
	    }
	else if (strncmp(keyword,"OID_TYPE", 8) == 0)
            {	
	    /*
	     * Check for OID_TYPE (OPTIONAL)
	     * Defaults to DNA_CMIP if NONE is specified.
	     */
    	    while (isspace(buffer[i]))
	        i++;
	    remove_quotes( &buffer[i] );
            size = strlen(&buffer[i]);

	    up_case(&buffer[i]);
	
	    if (strncmp(&buffer[i], "DNA_CMIP", 8 ) == 0)
		{
		mom->OID_EMA = 1;
		mom->oid_type = DNA_CMIP_OID;
		}
	    else if (strncmp(&buffer[i], "OSI_CMIP", 8 ) == 0)
		mom->oid_type = OSI_CMIP_OID;
	    else if (strncmp(&buffer[i], "SNMP", 4 ) == 0)
		{
		mom->oid_type = SNMP_OID;
		mom->OID_SNMP = 1;
		}
	    else if (strncmp(&buffer[i], "OID", 3 ) == 0)
		{
		mom->oid_type = OID;				
		}
	    else
		{
		printf("**** Unknown OID type -- %s\n", &buffer[i]);
    		return MOMGEN_C_UNKNOWN_OID_TYPE;
		}

	    if (mom->log)
		switch (mom->oid_type) 
	    	{
	    	case DNA_CMIP_OID :
	    	    fprintf(mom->log_file,"OID Type:        %s\n",DNA_CMIP_OID_STR);
		    break;
	    	case OSI_CMIP_OID :
	    	    fprintf(mom->log_file,"OID Type:        %s\n",OSI_CMIP_OID_STR);
		    break;
	    	case SNMP_OID :
	    	    fprintf(mom->log_file,"OID Type:        %s\n",SNMP_OID_STR);
		    break;                 
	    	case OID :
	    	    fprintf(mom->log_file,"OID Type:        %s\n",OID_STR);
		    break;
	    	}	
	
	    }
        }
    }

if (mom->num_classes == 0)
    return MOMGEN_C_NO_CLASSES;

if (num == 0)
    return MOMGEN_C_NO_CLASS_PREFIX;

if (parent_num == 0)
    return MOMGEN_C_NO_PARENT_CLASS;

if (num < mom->num_classes)
    return MOMGEN_C_INV_NUM_PREFIXES;

if (parent_num < mom->num_classes)
    return MOMGEN_C_INV_NUM_PARENT_CLASSES;

if (mom->mom_name == NULL)
    return MOMGEN_C_NO_MOM_NAME;

if (mom->organization == NULL)
    return MOMGEN_C_NO_ORGANIZATION;

if (mom->facility == NULL)
    return MOMGEN_C_NO_FACILITY;

if (mom->author == NULL)
    return MOMGEN_C_NO_AUTHOR;

if (mom->oid_type == 0)
    {
    mom->oid_type = DNA_CMIP_OID;
    if (mom->log)
	fprintf(mom->log_file,"OID Type:        DNA_CMIP (default)\n");
    mom->OID_EMA = 1;
    }

class = mom->class;
for (class_num = 0; class_num < mom->num_classes; class_num++)
    {
    class->primary_id = 1;
    if (mom->MCC_dictionary)
	stat = add_aes( class, mom );
    else
	stat = add_oid( class, mom );

    class = class->next;
    }

if (mom->log)
    fprintf(mom->log_file,"\n");
    
if (infile != NULL)
    fclose( infile );

return 1;
}
