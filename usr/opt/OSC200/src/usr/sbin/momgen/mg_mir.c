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
static char *rcsid = "@(#)$RCSfile: mg_mir.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/07/13 22:53:11 $";
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
**	This module contains the routines used to read the attributes
**	from the Common Agent Management Information Repository (MIR).
**
**  AUTHORS:
**
**      Gary J. Allison
**
**  CREATION DATE:    1-OCT-1991
**
**  MODIFICATION HISTORY:
**
**	Mike Densmore	20-May-1992	Modified for U**X Platforms
**
**      M. Ashraf        8-Dec-1992     Added recursion to search for terminal
**                                      data types.
**
**      M. Ashraf       10-Dec-1992     Uppercase Class/Entity names for CMIPs
**                                      only, not for SNMP.
**      M. Ashraf       16-Dec-1992     class->oid_below_node and class->parent
**                                      made case sensitive for SNMP.
**
**      M. Ashraf        3-Mar-1993     Updated for IPC changes
**--
*/

#ifdef VMS
#include "vaxcshr.h"		/* translate from VAX C to DEC C RTL names */
#endif 
#ifndef UNIXPLATFORM
#define NAME_SIZE 5
#else
#define NAME_SIZE 3
#endif

#define buffer_size 150
#define OID_DEF   /* For duplicate object_id definitions in MIR.H */

#define EXCEPTION_ARG 1
#define RESPONSE_ARG  2
#define EVENT_ARG     3

#include "mom_defs.h"
#include <ctype.h>
#include <string.h>
#include "moss.h"
#include "man.h"
#include "string_defs.h"
#include "mir.h"
#include "moss_dna.h"
#include "mg_prototypes.h"

/*
 *         LOCAL MANDLES          	
 * (For use by functions in this module) 
 */
#define MANDLE_COUNT 21
static rel_pair	pair [] =
   {
    {"MIR_Structured_As",
     NULL},
    {"MIR_DC_Found_In_SMI",
     NULL},
    {"MIR_Text_Name",
     NULL},
    {"MIR_MCC_ID_Code",
     NULL},
    {"MIR_Contained_By",
     NULL},
    {"MIR_DC_ASN1_Tag",
     NULL},
    {"MIR_DC_ASN1_Class",
     NULL},
    {"MIR_DC_SMI_Name",
     NULL},
    {"MIR_DC_SMI_Code",
     NULL},
    {"MIR_DC_SMI_Template",
     NULL},
    {"MIR_List_Entry",
     NULL},
    {"MIR_Description",
     NULL},
    {"MIR_Cont_attribute",
     NULL},
    {"MIR_Cont_request_argument",
     NULL},
    {"MIR_Cont_argument",
     NULL},
    {"MIR_Cont_attrPartition",
     NULL},
    {"MIR_Cont_directive",
     NULL},
    {"MIR_Directive_type",
     NULL},
    {"MIR_Cont_response",
     NULL},
    {"MIR_Cont_exception",
     NULL},
    {"MIR_Cont_event",
     NULL}
    };

enum rel_codes
    {
    REL_MIR_Structured_As,
    REL_MIR_DC_Found_In_SMI,
    REL_MIR_Text_Name,
    REL_MIR_MCC_ID_Code,
    REL_MIR_Contained_By,
    REL_MIR_DC_ASN1_Tag,
    REL_MIR_DC_ASN1_Class,
    REL_MIR_DC_SMI_Name,
    REL_MIR_DC_SMI_Code,
    REL_MIR_DC_SMI_Template,
    REL_MIR_List_Entry,
    REL_MIR_Description,
    REL_MIR_Cont_attribute,
    REL_MIR_Cont_request_argument,
    REL_MIR_Cont_argument,
    REL_MIR_Cont_attrPartition,
    REL_MIR_Cont_directive,
    REL_MIR_Directive_type,
    REL_MIR_Cont_response,
    REL_MIR_Cont_exception,
    REL_MIR_Cont_event,
    max_rel_code };

static mandle *m_mir_rel_name=NULL;
static mandle *m_mir_structured_as=NULL;
static mandle *m_mir_text_name=NULL;
static mandle *m_mir_id_code=NULL;
static mandle *m_mir_contained_by=NULL;
static mandle *m_mir_dc_found_in_smi=NULL;
static mandle *m_mir_dc_asn1_class=NULL;
static mandle *m_mir_dc_asn1_tag=NULL;
static mandle *m_mir_dc_smi_name=NULL;
static mandle *m_mir_dc_smi_code=NULL;
static mandle *m_mir_dc_smi_template=NULL;
static mandle *m_mir_dna_attrib_list=NULL;
static mandle *m_mir_dna_description=NULL;
static mandle *m_mir_dna_directive_type=NULL;
static mandle *m_mir_dna_cont_request_arg=NULL;
static mandle *m_mir_dna_cont_attribute=NULL;
static mandle *m_mir_dna_cont_arg=NULL;
static mandle *m_mir_dna_cont_attrpart=NULL;
static mandle *m_mir_dna_cont_directive=NULL;
static mandle *m_mir_dna_cont_response=NULL;
static mandle *m_mir_dna_cont_exception=NULL;
static mandle *m_mir_dna_cont_event=NULL;

mandle_class    *local_class=NULL;
char 		*current_attribute_group=NULL;

#if defined(sun) || defined(sparc)
mir_status mir_get_group();
#else
mir_status mir_get_group( int, CLASS_DEF *,MOM_BUILD *, DIRECTIVE_DEF *, mandle *, mandle *);
#endif


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	load_local_mandles
**
**	This routine reads all of the relationships searching for certain 
**	matches. Once a match is found (such as DNA_Symbol), the mandle for
**	that OID is saved to be used later.
**
**  FORMAL PARAMETERS:
**
**	None
**
**  RETURN VALUE:
**
**      Any MIR status.
**
**--
*/
static mir_status load_local_mandles(mir_file)
char *mir_file;

{
mandle_class    *mc_a= NULL;
mir_status      status;         /* status from other tier 0 calls        */
int		major_v;	/* Compiler Major Version #		 */
int		minor_v;	/* Compiler Minor Version #              */
int     	*max_oid_size = NULL;  /* Maximum OID arc count          */

/*
 * Check if already loaded (could be multiple classes...)
 */

if (m_mir_structured_as != NULL)
    return(MS_SUCCESS);

/*
 * Load the MIR Database. Use default of []MIR.DAT or logical mir_database_file
 * for location of MIR Database.
 */

status = mir_t0_init(mir_file, 	   	/* Database location - passed by   */
		     NULL,
		     NULL,
		     FALSE,
		     NULL,
		     NULL,
		     NULL,
		     NULL,
		     &max_oid_size);	/* Maximum number of arcs in OID   */

if (status != MS_SUCCESS)
    return MOMGEN_C_MIR_NOT_FOUND;


status = mir_get_rel_mandles( MANDLE_COUNT, pair, &mc_a );
if (status != MS_SUCCESS)
    {
    printf("**** Failure loading mandles\n");
    return MOMGEN_C_MIR_ERROR;
    }

m_mir_structured_as 	= pair[ REL_MIR_Structured_As ].m;
m_mir_dc_found_in_smi 	= pair[ REL_MIR_DC_Found_In_SMI ].m;
m_mir_text_name 	= pair[ REL_MIR_Text_Name ].m;
m_mir_id_code 		= pair[ REL_MIR_MCC_ID_Code ].m;
m_mir_contained_by 	= pair[ REL_MIR_Contained_By ].m;
m_mir_dc_asn1_tag 	= pair[ REL_MIR_DC_ASN1_Tag ].m;
m_mir_dc_asn1_class 	= pair[ REL_MIR_DC_ASN1_Class ].m;
m_mir_dc_smi_name 	= pair[ REL_MIR_DC_SMI_Name ].m;
m_mir_dc_smi_code 	= pair[ REL_MIR_DC_SMI_Code ].m;
m_mir_dc_smi_template 	= pair[ REL_MIR_DC_SMI_Template ].m;
m_mir_dna_attrib_list 	= pair[ REL_MIR_List_Entry ].m;
m_mir_dna_description 	= pair[ REL_MIR_Description ].m;
m_mir_dna_cont_attribute = pair[ REL_MIR_Cont_attribute ].m;
m_mir_dna_cont_request_arg = pair[ REL_MIR_Cont_request_argument ].m;
m_mir_dna_cont_arg 	= pair[ REL_MIR_Cont_argument ].m;
m_mir_dna_cont_attrpart = pair[ REL_MIR_Cont_attrPartition ].m;
m_mir_dna_cont_directive= pair[ REL_MIR_Cont_directive ].m;
m_mir_dna_directive_type= pair[ REL_MIR_Directive_type ].m;
m_mir_dna_cont_response = pair[ REL_MIR_Cont_response ].m;
m_mir_dna_cont_exception= pair[ REL_MIR_Cont_exception ].m;
m_mir_dna_cont_event 	= pair[ REL_MIR_Cont_event ].m;

return (MS_SUCCESS);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	dump_IB
**
**	This routine prints information from an Identification Block such
**	as datatype.
**
**  FORMAL PARAMETERS:
**
**	ib 		Pointer to identification block.
**	mom		Pointer to MOM block
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void dump_IB (ib,mom)

dc_ident        *ib;    /* --> Identification block to dump */
MOM_BUILD 	*mom;
{
if (mom->attr)
    if (ib->b_smi == TRUE) {
      fprintf(mom->log_file,"   Def. in SMI: %s\n",((ib->smi == MIR_SMI_DNA) ? "DNA" : "UNKNOWN"));
      }
    else {
      fprintf(mom->log_file,"   Def. in SMI: <Not Returned>\n");
      }

    if (ib->b_code == TRUE) {
      fprintf(mom->log_file,"   Code       : %d\n",ib->code);
      }
    else {
      fprintf(mom->log_file,"   Code       : <Not Returned>\n");
      }

    if (ib->b_name == TRUE) {
      fprintf(mom->log_file,"   Name       : %s\n",ib->name);
      }
    else {
      fprintf(mom->log_file,"   Name       : <Not Returned>\n");
      }

    if (ib->b_asn1_class == TRUE) {
      fprintf(mom->log_file,"   ASN.1 Class: %d\n",ib->asn1_class);
      }
    else {
      fprintf(mom->log_file,"   ASN.1 Class: <Not Returned>\n");
      }

    if (ib->b_asn1_tag == TRUE) {
      fprintf(mom->log_file,"   ASN.1 Tag  : %d\n",ib->asn1_tag);
      }
    else {
      fprintf(mom->log_file,"   ASN.1 Tag  : <Not Returned>\n");
      }

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	eval_DC_recursive
**
**	This routine prints information from about a builtup object.
**
**  FORMAL PARAMETERS:
**
**	high_status 		Original status (BUILTIN, BUILTUP, etc.)	
**	level			Current number of levels deep
**	ctx			Address of poitner to the context block
**	mom			Pointer to MOM block
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void eval_DC_recursive (high_status, level, ctx, mom)

mir_status      high_status;    /* Status returned at level above */
int             level;          /* Level number of THIS level     */
dc_context     **ctx;           /* Addr of ptr to context block   */
MOM_BUILD	*mom;
/* This function is RECURSIVE */

{
dc_ident        ib;             /* Ident block for loading              */
dc_context      *new_ctx=NULL;  /* New Context block                    */
int             number;         /* Number ret. by eval_builtin_template */
char            *string;        /* String ret. by eval_builtin_template */
mir_status      status;         /* status from tier 1 call              */
mir_status      d_status;       /* status from dissolve call            */
BOOL            restart;        /* "restart" flag from eval_bi_template */

switch (high_status) {

    case MS1_DC_BUILTUP:

        /* Evaluate the next level down */
        status = mir_eval_data_construct(ctx, &ib);

	if (mom->attr)
            fprintf(mom->log_file,"eval_DATA_CONSTRUCT() returns @ lvl %d >>\n",level);

        switch (status) {
            case MS1_DC_BUILTIN:
 	        if (mom->attr)
                    fprintf(mom->log_file,"Value defined as a 'builtin-atomic':\n");

                dump_IB(&ib,mom);
                return;

            case MS1_DC_BUILTUP:
		if (mom->attr)
                    fprintf(mom->log_file,"Value defined as a 'builtup':\n");

                dump_IB(&ib,mom);
		if (mom->attr)
                    fprintf(mom->log_file,". . .which is further qualified. . .\n\n");

                eval_DC_recursive(MS1_DC_BUILTUP, (level + 1), ctx , mom);
                return;

            case MS1_DC_BUILTIN_TMPLT:
		if (mom->attr)
                    fprintf(mom->log_file,"Value defined as a 'builtin-template':\n");

                dump_IB(&ib,mom);
   		if (mom->attr)
                    fprintf(mom->log_file,". . .which is further qualified. . .\n\n");

                eval_DC_recursive(MS1_DC_BUILTIN_TMPLT, (level + 1), ctx, mom);
                return;
                
            default:
		if (mom->attr)
                    fprintf(mom->log_file,"Eval: failure: %s, level %d\n", mir_error(status), level); 
                return;
            }
        break;


    case MS1_DC_BUILTIN_TMPLT:
        /* Evaluate the context block we are being fed for template data */
        do {
            status = mir_eval_builtin_template(ctx, &number, &string, &new_ctx,
                                               &ib, &restart);

	    if (mom->attr)
                fprintf(mom->log_file,"eval_BUILTIN_TEMPLATE() returns @ lvl %d >>",level);

            /* If eval_builtin_template is now restarting the evaluation */
            /* "at the top", bag out.                                    */
            if (restart == TRUE) {
                status = MS0_FIND_NONE;
		if (mom->attr)
                    fprintf(mom->log_file," restart = TRUE\n");

                }

            switch (status) {

                case MS1_TMPLT_NUMBER:
		    if (mom->attr)
                        fprintf(mom->log_file,"Template Number value: %d\n", number);

                    break;

                case MS1_TMPLT_STRING:
		    if (mom->attr)
			fprintf(mom->log_file,"Template String value: '%s'\n", string);

                    break;

                case MS1_DC_BUILTUP:
		    if (mom->attr)
                        fprintf(mom->log_file,"\nValue defined as a 'builtup':\n");

                    dump_IB(&ib,mom);
		    if (mom->attr)
                        fprintf(mom->log_file,". . .which is further qualified. . .\n\n");

                    eval_DC_recursive(MS1_DC_BUILTUP, (level + 1), &new_ctx, mom);
                    if ((d_status = mir_dissolve_context(&new_ctx))
                         != MS_SUCCESS) {
		    if (mom->attr)
                        fprintf(mom->log_file,"MS1_DC_BUILTUP Dissolve at level %d failed: %s\n",level, mir_error(d_status));


                        }
                    break;

                case MS1_DC_BUILTIN:
		    if (mom->attr)
		        {
                        fprintf(mom->log_file,"\nValue defined as a 'builtin':\n");
                        dump_IB(&ib,mom);
			}
                    break;

                case MS0_FIND_NONE:
                    continue;

                default:
		    if (mom->attr)
                        fprintf(mom->log_file,"Eval failure: %s, level %d\n",mir_error(status),level);
                       
                    return;
                }
            } while (status != MS0_FIND_NONE);
	if (mom->attr)
            fprintf(mom->log_file,"<><><> End of Evaluation @ lvl %d<><><>\n\n",level);

        return;

    default:
	if (mom->attr)
            fprintf(mom->log_file,"Eval: failure: %s, level %d\n",mir_error(status),level);
        return;
    }
}

#ifndef UNIXPLATFORM
char *mir_error ( mir_status s )
{ return 1; }
#endif

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	get_non_term
**
**	This routine keeps reading a NON TERMINAL object until there
**	are no more NONTERMs or TERMs.
**
**  FORMAL PARAMETERS:
**
**	non_term	Pointer to mandle of NON TERMINAL object to read.
**
**  RETURN VALUE:
**
**      Any status from mir_search_rel_table 
**
**--
*/
mir_status get_non_term ( non_term, rel_mandle, m_value )

mandle *non_term;
mandle *rel_mandle;
mir_value *m_value;

{
#define REL_NAME_OID_LEN 9

static mandle   *m_junk=NULL;   /* Mandle used to rcv error NON-TERM     */
static mandle   *m_scanner=NULL;/* Mandle used to scan the MIR rels      */
static mandle_class *local_class_mandle=NULL;
mir_status      s_status;       /* status from tier 0 search call        */
mir_status      status;         /* status from other tier 0 calls        */

s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                  non_term,       /* Searched Object Mandle */
                                  rel_mandle,  	  /* Relationship object    */
                                  &local_class_mandle,/* Mandle class           */
                                  &m_junk,        /* Return Mandle          */
                                  m_value         /* Return terminal value  */
                                  );

return( s_status );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_get_name_code 
**
**	This routine gets the name, id code, and symbol from a mandle.
**
**  FORMAL PARAMETERS:
**
**	in_mandle	Pointer to mandle to lookup.
**	str		Address of pointer to name string to return.
**	code		Address of integer to return code.
**	symbol		Address of pointer to return symbol name (optional).
**	cur_size	Current size of name
**
**  RETURN VALUE:
**
**	MS0_FIND_TERM 	if success
**	Any status from get_non_term if failure.
**
**--
*/
mir_status mir_get_name_code( in_mandle, 
			      str, 
			      code,
			      symbol,
			      cur_size,
			      mom )

mandle *in_mandle;
char **str;
int *code;
char **symbol;
int cur_size;
MOM_BUILD *mom;

{
int size;
int max_name_size;
mir_status status;
mir_value m_value;

/* The code always comes first... */

if (mom->attr)
    fprintf(mom->log_file,"Getting... ");

status = get_non_term( in_mandle, m_mir_id_code, &m_value); 
if (status != MS0_FIND_TERM) 
    return( status );
else
    if (IS_MV_SNUMBER((&m_value))) 
    {
	*code = MV_SNUMBER((&m_value));
	if (mom->attr)
  	    fprintf(mom->log_file,"code: %d : ",*code);

    }
status = get_non_term( in_mandle, m_mir_text_name, &m_value); /* Get NAME of definition */
if (status != MS0_FIND_TERM) 
    return( status );
else
    if (IS_MV_STRING((&m_value))) 
    {
      char *bad_char;
      int i,j;

      size = strlen(MV_STRING((&m_value)));
      *str = (char *)calloc(1, size + 1);

      /*
       * Copy the string without the spaces.
       */

      i = 0; j = 0;
      while (j < size)
          if (!isspace(MV_STRING((&m_value))[j]))
	      (*str)[i++] = MV_STRING((&m_value))[j++];
	  else
	      j++;

      /*
       * Check for illegal characters in name.  Print out warning and replace with
       * underscore if found.
       */

       while ((bad_char = (char *) strpbrk( *str, "!@#%^&*()-=+{}[]/\\|?")) != 0)
           {
           fprintf(mom->log_file,"\n*** Warning: Replacing invalid character '%c' found in name %s with '_' ***\n\n", *bad_char, *str );
           *bad_char = '_';
           }

      if (symbol != NULL)
	 {
	 *symbol = (char *)calloc(1, size  + 1);
	 strcpy( *symbol, MV_STRING((&m_value)));
	 }
#ifndef UNIXPLATFORM
      /* 
       * Calculate the largest size allowable for the name. Cur_size is the current
       * total of what's used already (e.g. MP_K_DIR_xxxx) where xxxx cannot be
       * larger than 31 - 9 = 22.
       */

      max_name_size = 31 - cur_size;
      if (size > max_name_size)
         {
	 if (mom->attr)
              fprintf(mom->log_file,"*** name code --- truncating %s to ",*str);

	  (*str)[ max_name_size ] = '\0';
          }
      else
          (*str)[ size ] = '\0';
#else
      (*str)[ size ] = '\0';
#endif
      if ((mom->oid_type == DNA_CMIP_OID) || (mom->oid_type == OSI_CMIP_OID))
        up_case(*str); 

      if (mom->attr)
          fprintf(mom->log_file,"name: %s\n",*str);

    }

return( status );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_get_oid
**
**	This routine sets up an SNMP, OID, or OSI OID.
**
**  FORMAL PARAMETERS:
**
**	oid_mandle	Pointer to mandle to lookup
**	oid_text	Address of pointer to text OID
**	oid_len		Address of OID length
**	oid		Address of OID to return
**	smi		Value of SMI (SNMP,OID,OSI)
**
**  RETURN VALUE:
**
**      Any status from mir_oid_to_mandle.
**
**--
*/
mir_status mir_get_oid( oid_mandle, 
		 	oid_text, 
	         	oid_len, 
		     	oid,
	     		smi )

mandle *oid_mandle;
char **oid_text;
signed_int int *oid_len;
object_id **oid;
mir_oid_smi smi;

{
object_id *temp_oid;
mir_status status;

/* 
 * First, see if this OID type exists, if not, don't return the error unless
 * we got an unexpected error.
 */

status = moss_create_oid( 0, 0, &temp_oid );
status = mir_mandle_to_oid( oid_mandle, temp_oid, &smi );
if (status != MS_SUCCESS)
    {
    moss_free_oid( temp_oid );
    if (status == MS0_FIND_NONE)
	return (SS$_NORMAL);
    else
    	return( status );
    }

/* oid = temp_oid; */

status = moss_get_oid_len( temp_oid, oid_len );
status = moss_oid_to_text( temp_oid, ",", NULL, NULL, oid_text );
moss_free_oid( temp_oid );
return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_check_oid
**
**	This routine checks to make sure at least one of OIDs is specified
**      in the dictionary. If none is specified, an error is returned, unless
**	the logical MOMGEN$DEFAULT_OID logical is specified to override. If
** 	the logical is defined, the MCC Integer Code is copied into the 
**	DNA_CMIP_INT.
**
**	This routine also sets class->multiple_oids if more than one is found.
**
**  FORMAL PARAMETERS:
**
**	mom		- pointer to MOM
**	class		- pointer to current class.
**	code 		- integer containing MCC ID Code.
**	dna_cmip_int 	- address of integer containing DNA_CMIP_INT
**	dna_cmip_int_len- address of integer containing DNA_CMIP_INT_LEN
**	osi_oid_len	- integer containing OSI OID length
**	oid_len		- integer containing OID length
**      snmp_oid_len	- integer containing SNMP OID length
**	
**
**  RETURN VALUE:
**
**      None
**
**--
*/
int mir_check_oid( mom,
	           class,
	           code, 
	           dna_cmip_int,
	           dna_cmip_int_len,
	           osi_oid_len,
	           oid_len,
	           snmp_oid_len )

MOM_BUILD *mom;
CLASS_DEF *class;
int code;
int *dna_cmip_int;
signed_int int *dna_cmip_int_len;
int osi_oid_len;
int oid_len;
int snmp_oid_len;

{
int temp = 0;

/*
 * If none of the OIDs are specified, then use the ID Code for the DNA_CMIP_INT.
 * Or, if the DNA_CMIP_INT is not specified, but one of the others is (e.g. some
 * MSLs had OID=1 specified, then again, use the ID code for the DNA_CMIP_INT.
 */

if (((*dna_cmip_int_len == 0) && (osi_oid_len == 0) && (oid_len == 0) && (snmp_oid_len == 0)) ||
    ((*dna_cmip_int_len == 0) && (mom->oid_type == DNA_CMIP_OID)))
    {
    if (!mom->default_oid)
	{
/*	printf("\n**** Warning -- no default OID type found. Assuming DNA_CMIP OIDs ****\n"); */
/*	return MOMGEN_C_NO_OID_FOUND; */
   	}
    *dna_cmip_int = code;
    *dna_cmip_int_len = 1;
    }

temp = *dna_cmip_int_len + osi_oid_len + oid_len + snmp_oid_len;

if (temp > 1)
    class->multiple_oids = 1;

return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_get_dna_cmip_int
**
**	This routine sets up the DNA_CMIP_INT value
**
**  FORMAL PARAMETERS:
**
**	oid_mandle	Pointer to mandle to lookup
**	dna_cmip_int	Addresss of dna_cmip_int to return
**	oid_len		Address of OID length
**
**  RETURN VALUE:
**
**      Any status from mir_oid_to_mandle.
**
**--
*/
mir_status mir_get_dna_cmip_int( oid_mandle, 
				 dna_cmip_int,
	         		 oid_len )

mandle *oid_mandle;
int *dna_cmip_int;
signed_int int *oid_len;

{
mir_status status;
object_id *temp_oid;
mir_oid_smi smi;

/* 
 * First, see if this OID type exists, if not, don't return the error unless
 * we got an unexpected error.
 */

smi = OID_DNA;
status = moss_create_oid( 0, 0, &temp_oid );
status = mir_mandle_to_oid( oid_mandle, temp_oid, &smi );
if (status != MS_SUCCESS)
    {
    moss_free_oid( temp_oid );
    if (status == MS0_FIND_NONE)
	return SS$_NORMAL;
    else
    	return( status );
    }

status = moss_get_oid_len( temp_oid, oid_len );

*dna_cmip_int = temp_oid->value[*oid_len-1];

/**
 ** The DNA_CMIP_INT length should always be one.
 **/

*oid_len = 1;

moss_free_oid( temp_oid );
return SS$_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	get_class_mandle
**
**	This routine converts an OID into a mandle and returns the mandle.
**
**  FORMAL PARAMETERS:
**
**	oid		Pointer to OID to lookup.
**	ret_mandle	Address of pointer to mandle to return.
**
**  RETURN VALUE:
**
**      Any status from mir_oid_to_mandle.
**
**--
*/
mir_status get_class_mandle( oid, ret_mandle)

object_id *oid;
mandle **ret_mandle;

{
static mandle_class *local_class_mandle=NULL;
mir_status status;

status = mir_oid_to_mandle(GET_EXACT,       /* Exact lookup             */
                           oid,  	    /* OID to lookup 		*/
                           ret_mandle,      /* Mandle of Rel. name      */
                           &local_class_mandle,   /* Mandle Class to use */
			   NULL);

return (status);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	get_mandle
**
**	This routine returns a mandle for an OID and relationship passed in.
**
**  FORMAL PARAMETERS:
**
**	rel		Relationship 
**	oid		Pointer to OID to read.
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      Any status from mir_search_rel_table
**
**--
*/
mir_status get_mandle( search,
		       rel_name, 
		       ret_mandle,
		       non_term,
		       m_value )

int search;
mandle *rel_name;
mandle *ret_mandle;
mandle **non_term;
mir_value *m_value;

{
static mandle   *class_mandle=NULL;/* Mandle used to get rel name      */
static mandle_class *local_class_mandle=NULL;
mir_status      s_status;       /* status from tier 0 search call        */
mir_status      status;         /* status from other tier 0 calls        */
int             i,j;              /* General purpose index                 */

s_status = mir_search_rel_table(  search,
                                  ret_mandle,         /* Searched Object Mandle */
                                  rel_name,           /* Relationship object    */
                                  &local_class_mandle,/* Mandle class           */
                                  non_term,           /* Return NON terminal value          */
                                  m_value             /* Return terminal value  */
                                  );

return( s_status );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_exc
**
**	This routine adds an exception to a directive.
**
**  FORMAL PARAMETERS:
**
**	exc_mandle 	Pointer to current exception mandle.
**	dir		Pointer to current directive.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      MS_SUCCESS or any MIR error status
**
**--
*/
mir_status mir_add_exc(  exc_mandle, 
		         dir, 
			 class, 
			 mom )

mandle *exc_mandle;
DIRECTIVE_DEF *dir;
CLASS_DEF *class;
MOM_BUILD *mom;

{
char *name,*symbol;
int code,size,cur_size;
mir_status status;
EXCEPTION_DEF *exc,*tmp;
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */

if (mom->attr)
    fprintf(mom->log_file,"Exceptions \n");

/* 
 * Cur size = "prefix_" + exc_ + "directive" + _ + _oid 
 */

cur_size = strlen( class->prefix ) + strlen( exc_prefix ) + MIN(6, strlen( dir->directive )) + 1 + 4;
status = mir_get_name_code( exc_mandle, &name, &code, &symbol, cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

if (code >= 15360)
    return MOMGEN_C_NO_DNA_CMIP_INT;

exc = (EXCEPTION_DEF *) calloc (1, sizeof (EXCEPTION_DEF));
exc->next = NULL;
exc->exception_number = code;

tmp = dir->exc;
if (tmp==NULL)
    dir->exc = exc;
else
    {
    while (tmp->next!=NULL)
        tmp=tmp->next;
    tmp->next = exc;
    }

if (!mom->use_symbols) {
    size = strlen( name ) + strlen( exc_prefix ) + MIN(6, strlen( dir->directive )) +
	strlen( underscore );
    exc->exception_name = (char *)calloc(1, size + 1);
    strcpy( exc->exception_name, exc_prefix );
    strncat( exc->exception_name, dir->directive, 6 );
    strcat( exc->exception_name, underscore );
    strcat( exc->exception_name, name );
    }
else
    {
    exc->exception_name = (char *)calloc(1, strlen( symbol ) + 1);
    strcpy( exc->exception_name, symbol );
    }    

exc->name = (char *)calloc(1, strlen( name ) + 1);
strcpy( exc->name, name );

/*
 * See if there is another duplicate. IF so, set the last character to the
 * duplicate character.
 */

if (check_duplicate_exc( class, dir, exc ))
    {
    set_dup_char( exc->exception_name,  &class->current_dup_char_exc );
    exc->original_name = (char *)calloc(1, strlen(symbol) + 1 );
    strcpy( exc->original_name, symbol );
    }
else if (strlen(symbol) > strlen(name))
    {   
    exc->original_name = (char *)calloc(1, strlen(symbol) + 1 );
    strcpy( exc->original_name, symbol );
    }

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( exc_mandle, &exc->snmp_oid_text, &exc->snmp_oid_len, 
			&exc->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( exc_mandle, &exc->osi_oid_text, &exc->osi_oid_len, 
			&exc->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( exc_mandle, &exc->oid_text, &exc->oid_len, 
			&exc->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( exc_mandle, &exc->dna_cmip_int, &exc->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class,   code, &exc->dna_cmip_int, &exc->dna_cmip_int_len, exc->osi_oid_len, 
	   	  exc->oid_len, exc->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

if (mom->attr)
    fprintf(mom->log_file,"Exception %s\n", exc->exception_name );


dir->num_exceptions++;

/*
 * Now, get the exception arguments.
 */

status = mir_get_group( EXCEPTION_ARG,
			class, 
			mom, 
			dir, 
			m_mir_dna_cont_arg, 
			exc_mandle );

return( status );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_resp
**
**	This routine adds a response to a directive.
**
**  FORMAL PARAMETERS:
**
**	resp_mandle 	Pointer to current directive mandle.
**	dir		Pointer to current directive.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      MS_SUCCESS or any MIR error status
**
**--
*/
mir_status mir_add_resp( resp_mandle, 
		         dir, 
			 class, 
			 mom )

mandle *resp_mandle;
DIRECTIVE_DEF *dir;
CLASS_DEF *class;
MOM_BUILD *mom;

{
mir_status status;
char *name,*symbol;
int code,size,cur_size;
RESPONSE_DEF *resp,*tmp;
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */

if (mom->attr)
    fprintf(mom->log_file,"Response \n");

/* 
 * Cur size = "prefix_" + resp_ + "directive" + _ + _oid 
 */

cur_size = strlen( class->prefix ) + strlen( resp_prefix ) + MIN(6, strlen( dir->directive )) + 1 + 4;
status = mir_get_name_code( resp_mandle, &name, &code, &symbol, cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

resp = (RESPONSE_DEF *) calloc (1, sizeof (RESPONSE_DEF));
resp->next = NULL;
resp->response_number = code;

tmp = dir->resp;
if (tmp==NULL)
    dir->resp = resp;
else
    {
    while (tmp->next!=NULL)
        tmp=tmp->next;
    tmp->next = resp;
    }

if (!mom->use_symbols) {
    size = strlen( name ) + strlen( resp_prefix ) + MIN(6, strlen( dir->directive )) +
	strlen( underscore );

    resp->response_name = (char *)calloc(1, size + 1 );
    strcpy( resp->response_name, resp_prefix );
    strncat( resp->response_name, dir->directive, 6 );
    strcat( resp->response_name, underscore );
    strcat( resp->response_name, name );
    }
else
    {
    resp->response_name = (char *)calloc(1, strlen( symbol ) + 1);
    strcpy( resp->response_name, symbol );
    }    

resp->name = (char *)calloc(1, strlen( name )+ 1 );
strcpy( resp->name, name );

/*
 * See if there is another duplicate. IF so, set the last character to the
 * duplicate character.
 */

if (check_duplicate_resp( class, dir, resp ))
    {
    set_dup_char( resp->response_name, &class->current_dup_char_resp );
    resp->original_name = (char *)calloc(1, strlen(symbol) + 1 );
    strcpy( resp->original_name, symbol );
    }

else if (strlen(symbol) > strlen(name))
    {   
    resp->original_name = (char *)calloc(1, strlen(symbol) + 1 );
    strcpy( resp->original_name, symbol );
    }

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( resp_mandle, &resp->snmp_oid_text, &resp->snmp_oid_len, 
			&resp->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( resp_mandle, &resp->osi_oid_text, &resp->osi_oid_len, 
			&resp->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( resp_mandle, &resp->oid_text, &resp->oid_len, 
			&resp->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( resp_mandle, &resp->dna_cmip_int, &resp->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class,   code, &resp->dna_cmip_int, &resp->dna_cmip_int_len, resp->osi_oid_len, 
	   	  resp->oid_len, resp->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

if (mom->attr)
    fprintf(mom->log_file,"Response %s\n", resp->response_name );

/* 
 * Read the RESPONSE arguments.
 */

status = mir_get_group( RESPONSE_ARG,
			class, 
			mom, 
			dir,
			m_mir_dna_cont_arg,
			resp_mandle );

dir->num_responses++;

return( MS_SUCCESS );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_evt
**
**	This routine adds an event to the class structure.
**
**  FORMAL PARAMETERS:
**
**	evt_mandle	Pointer to current event mandle.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      MS_SUCCESS or any MIR error status
**
**--
*/
mir_status mir_add_evt( evt_mandle,
			class,
			mom )

mandle *evt_mandle;
CLASS_DEF *class;
MOM_BUILD *mom;

{
int code,size,cur_size;
char *name,*symbol;
EVENT_DEF *event;
mir_status status,s_status;
mir_value m_value;
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */

cur_size = strlen( class->prefix ) + strlen( arg_prefix ) + strlen( evt_prefix) + 1;
status = mir_get_name_code( evt_mandle, &name, &code, &symbol, cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

/*
 * Allocate the event structure
 */

event = (EVENT_DEF *) calloc (1, sizeof (EVENT_DEF));
event->event_number = code;

insert_event( event, class );

if (!mom->use_symbols) {
    size = strlen( name ) + strlen( evt_prefix );
    event->event_name = (char *)malloc ( size + 1 );
    strcpy( event->event_name, evt_prefix );
    strcat( event->event_name, name );
    }
else
    {
    event->event_name = (char *)calloc(1, strlen( symbol ) + 1);
    strcpy( event->event_name, symbol );
    }    

if (check_duplicate_event( class, event))
    {
    set_dup_char( event->event_name, &class->current_dup_char_evt );
    event->original_name = (char *)calloc(1, strlen(symbol) + 1 );
    strcpy( event->original_name, symbol );
    }
else if (strlen(symbol) > strlen(name))
    {   
    event->original_name = (char *)calloc(1, strlen(symbol) + 1 );
    strcpy( event->original_name, symbol );
    }

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( evt_mandle, &event->snmp_oid_text, &event->snmp_oid_len, 
			&event->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( evt_mandle, &event->osi_oid_text, &event->osi_oid_len, 
			&event->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( evt_mandle, &event->oid_text, &event->oid_len, 
			&event->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( evt_mandle, &event->dna_cmip_int, &event->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class,   code, &event->dna_cmip_int, &event->dna_cmip_int_len, event->osi_oid_len, 
	   	  event->oid_len, event->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

if (mom->attr)
    fprintf(mom->log_file,"Event name %s\n",event->event_name);

/* 
 * Read the EVENT arguments.
 */

status = mir_get_group( EVENT_ARG,
			class, 
			mom, 
			0,
			m_mir_dna_cont_arg,
			evt_mandle );

class->num_events++;

return( status );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	add_mir_action
**
**	This routine is called to allocate an action structure and to determine
**	the type of action. Common action values (as defined by DNA CMIP) as
**	used.
**
**  FORMAL PARAMETERS:
**
**	dir		Pointer to current directive
**	class		Pointer to current class
**
**  RETURN VALUE:
**
**      None
**
**--
*/
add_mir_action( dir, 
                class )

DIRECTIVE_DEF *dir;
CLASS_DEF *class;

{
ACTION_DEF *action,*tmp;
int size;

action = (ACTION_DEF *) malloc (sizeof(ACTION_DEF));
class->num_actions++;
size = strlen( dir->directive );
action->next = 0;
action->directive = dir;
action->action_name = (char *)calloc(1, size+1 );
strcpy( action->action_name, dir->directive );
action->action_number = dir->directive_number;
action->original_action_number = dir->directive_number;

if (strcmp(action->action_name,"CREATE") == 0)
    {
    class->support_create = 1;
/*    action->action_number = 0;  */
    action->action_routine = standard_create_routine;
    }

if (strcmp(action->action_name,"DELETE") == 0)
    {
    class->support_delete = 1;
/*    action->action_number = 1;  */
    action->action_routine = standard_delete_routine;
    }
if (strcmp(action->action_name,"ENABLE") == 0)
    {
    class->support_action = 1;
/*    action->action_number = 2;  */
    action->action_routine = standard_delete_routine;
    }
if (strcmp(action->action_name,"DISABLE") == 0)
    {
    class->support_action = 1;
/*    action->action_number = 3;  */
    action->action_routine = standard_delete_routine;
    }
if (strcmp(action->action_name,"SUSPEND") == 0)
    {
    class->support_action = 1;
/*    action->action_number = 4;  */
    action->action_routine = standard_delete_routine;
    }
if (strcmp(action->action_name,"RESUME") == 0)
    {
    class->support_action = 1;
/*    action->action_number = 5;  */
    action->action_routine = standard_delete_routine;
    }
if (strcmp(action->action_name,"CONNECT") == 0)
    {
    class->support_action = 1;
/*    action->action_number = 6;  */
    action->action_routine = standard_delete_routine;
    }
if (strcmp(action->action_name,"DISCONNECT") == 0)
    {
    class->support_action = 1;
/*    action->action_number = 7;  */
    action->action_routine = standard_delete_routine;
    }
if (strcmp(action->action_name,"TEST") == 0)
    {
    class->support_action = 1;
/*    action->action_number = 8;  */
    action->action_routine = standard_delete_routine;
    }
if (strcmp(action->action_name,"ADD") == 0)
    {
    class->support_set = 1;
    class->support_add = 1;
    }
if (strcmp(action->action_name,"REMOVE") == 0)
    {
    class->support_set = 1;
    class->support_remove = 1;
    }
if (strcmp(action->action_name,"SET") == 0)
    {
    class->support_set = 1;
    }
if (strcmp(action->action_name,"SHOW") == 0)
    {
    class->support_get = 1;
    }

/* 
 *  Add action to class list
 */

tmp = class->action;
if (tmp==NULL)
    class->action = action;
else
    {
    while (tmp->next!=NULL)
        tmp=tmp->next;
    tmp->next = action;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_dir
**
**	This routine adds a directive to the class structure.  It also adds
**	the directive type.
**
**  FORMAL PARAMETERS:
**
**	dir_mandle	Pointer to current directive mandle.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      MS_SUCCESS or any MIR error status
**
**--
*/
mir_status mir_add_dir( dir_mandle,
			class,
			mom )

mandle *dir_mandle;
CLASS_DEF *class;
MOM_BUILD *mom;

{
int cur_size,code,size;
char *name,*symbol;
DIRECTIVE_DEF *dir;
mir_status status,s_status;
mir_value m_value;
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */

/* 
 * Allocate and initialize the directive.
 */

dir = (DIRECTIVE_DEF *) calloc (1, sizeof (DIRECTIVE_DEF));
insert_dir( dir, class );

if (mom->attr)
    fprintf(mom->log_file,"Directive\n ");


/* 
 * Size of prefix + directive prefix + directive name + 4 
 * (for _oid and _arr) (E.G. MGT_ + DIR_ + START + _oid)
 */

cur_size = strlen( class->prefix ) + strlen( dir_prefix ) + 4;
status = mir_get_name_code( dir_mandle, &name, &code, &symbol, cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

dir->directive_number = code;

if (!mom->use_symbols) {
    size = strlen( name ) + strlen( dir_prefix );
    dir->directive_name = (char *)malloc ( size + 1 );
    strcpy( dir->directive_name, dir_prefix );
    strcat( dir->directive_name, name );
    }
else
    {
    dir->directive_name = (char *)calloc(1, strlen( symbol ) + 1);
    strcpy( dir->directive_name, symbol );
    }    

dir->directive = (char *)malloc (strlen( name ) + 1);
strcpy( dir->directive, name );

class->num_directives++;

/*
 * Add the directive type. 
 */

status = get_non_term( dir_mandle, m_mir_dna_directive_type, &m_value); 
if (status != MS0_FIND_TERM) 
    return( status );
else
    if (IS_MV_SNUMBER((&m_value))) 
      {
      code = MV_SNUMBER((&m_value));
      switch( code ) 
	{
        case MCC_K_DIR_EXAMINE:
    	  dir->type = MOMGEN_K_DIR_EXAMINE;
	  break;
        case MCC_K_DIR_MODIFY:
          dir->type = MOMGEN_K_DIR_MODIFY;
          break;
        case MCC_K_DIR_ACTION:
	  dir->type = MOMGEN_K_DIR_ACTION;
	  add_mir_action( dir, class);
          break;
        case MCC_K_DIR_EVENT:
          dir->type = MOMGEN_K_DIR_EVENT;
          break;
        default:
          dir->type = MOMGEN_K_DIR_UNKNOWN;
	  break;
        }
      }
    else
        dir->type = MOMGEN_K_DIR_UNKNOWN;

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( dir_mandle, &dir->snmp_oid_text, &dir->snmp_oid_len, 
			&dir->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( dir_mandle, &dir->osi_oid_text, &dir->osi_oid_len, 
			&dir->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( dir_mandle, &dir->oid_text, &dir->oid_len, 
			&dir->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( dir_mandle, &dir->dna_cmip_int, &dir->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class,   code, &dir->dna_cmip_int, &dir->dna_cmip_int_len, dir->osi_oid_len, 
	   	  dir->oid_len, dir->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

/* 
 * Get all of the responses for this directive
 */

status = mir_get_group( 0, class, mom, dir, m_mir_dna_cont_response, dir_mandle );
if (status != MS_SUCCESS)
    return( status );

/* 
 * Get all of the exceptions for this directive
 */

status = mir_get_group( 0, class, mom, dir, m_mir_dna_cont_exception, dir_mandle );
if (status != MS_SUCCESS)
    return( status );

/* 
 * Get all of the request arguments for this directive. NOTE: MCC stores these
 * under request. The MIR stores them under DIRECTIVE.
 */

status = mir_get_group( 0, class, mom, dir, m_mir_dna_cont_request_arg, dir_mandle );

return( status );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_attr
**
**	This routine adds an attribute to the class structure.
**
**  FORMAL PARAMETERS:
**
**	attr_mandle	Pointer to current attribute mandle.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      MS_SUCCESS or any MIR error status.
**
**--
*/
mir_status mir_add_attr( attr_mandle,
			 class,
			 mom )

mandle *attr_mandle;
CLASS_DEF *class;
MOM_BUILD *mom;

{
dc_ident        ib;             /* Ident block for loading              */
dc_context      *ctx=NULL;      /* Context block pointer                */
object_id       new_oid; 	/* Object id of Structured as    */
char *data_type;
char *name,*symbol;
int code,size,cur_size;
mir_status status,s_status;
mir_value m_value;
ATTRIBUTE_DEF *attr;
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */

if (mom->attr)
    fprintf(mom->log_file,"Attribute\n");


cur_size = strlen( class->prefix ) + strlen( attr_prefix ) + 4;
status = mir_get_name_code( attr_mandle, &name, &code, &symbol, cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

/* 
 * Allocate an attribute definition block
 */

attr = (ATTRIBUTE_DEF *) calloc (1, sizeof (ATTRIBUTE_DEF));
    
attr->attribute_number = code;

/*
 * Build the attribute name "ATTR_" + name 
 */

if (!mom->use_symbols) {
    size = strlen( name ) + strlen( attr_prefix );
    attr->attribute_name = (char *)calloc(1, size + 1 );
    strcpy( attr->attribute_name, attr_prefix );
    strcat( attr->attribute_name, name );        
    if (check_duplicate( class, attr ))
	{                                       
        set_dup_char( attr->attribute_name,  &class->current_dup_char );
	size = strlen(symbol);
	attr->original_name = (char *)calloc(1, size + 1 );
	strcpy( attr->original_name, symbol );
	}
    else if (strlen(symbol) > strlen(name))
	{    
	size = strlen(symbol);
	attr->original_name = (char *)calloc(1, size + 1 );
	strcpy( attr->original_name, symbol );
	}
    }
else
    {
    attr->attribute_name = (char *)calloc(1, strlen( symbol ) + 1);
    strcpy( attr->attribute_name, symbol );
    }    

/* 
 * Insert into attribute linked list
 */

attr->next = 0;
insert_attr( attr, class );
class->num_attributes++;

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( attr_mandle, &attr->snmp_oid_text, &attr->snmp_oid_len, 
			&attr->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( attr_mandle, &attr->osi_oid_text, &attr->osi_oid_len, 
			&attr->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( attr_mandle, &attr->oid_text, &attr->oid_len, 
			&attr->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( attr_mandle, &attr->dna_cmip_int, &attr->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class, code, &attr->dna_cmip_int, &attr->dna_cmip_int_len, attr->osi_oid_len, 
	   	  attr->oid_len, attr->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

oid_smi = OID_ANY;
status = mir_mandle_to_oid( attr_mandle, &new_oid, &oid_smi );
if (status != MS_SUCCESS)
    return( status );

status = mir_lookup_data_construct( &new_oid, &ctx, &ib );
switch (status) {

  case MS1_DC_BUILTIN:
    if (mom->attr)
	{
        fprintf(mom->log_file,"\nBUILTIN MIR Object '");
        print_oid( mom, &new_oid );
        dump_IB(&ib,mom);
	}
    break;
  case MS1_DC_BUILTUP:
    if (mom->attr)
	{
        fprintf(mom->log_file,"\n****** BUILTUP MIR Object '");
        print_oid( mom, &new_oid );
        dump_IB(&ib,mom);
	}
    /* Get next level down... */
    while (status == MS1_DC_BUILTUP) 
      status = mir_eval_data_construct(&ctx, &ib);

    if (mom->attr)
        dump_IB(&ib,mom);

    break;    
  default:
    return( status );
   }
    

mir_get_data_type_length( ib.name, &attr->data_type, &attr->length, attr, class, mom->oid_type );

/*
 * Check to see if the user specified which attribute was the ACL attribute.
 */

if (attr->attribute_number == class->ACL_attribute_num)
    attr->mg_type = MG_C_ACL;

return( MS_SUCCESS );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_ag
**
**	This routine adds an attribute group to the class structure.
**
**  FORMAL PARAMETERS:
**
**	ag_mandle	Pointer to current attribute group mandle.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**	MS_SUCCESS or any MIR error status.
**
**--
*/
mir_status mir_add_part( ag_mandle,
			 class,
			 mom )

mandle *ag_mandle;
CLASS_DEF *class;
MOM_BUILD *mom;

{
mir_status status;
char *name,*symbol;
int size,code,cur_size;

if (mom->attr)
    fprintf(mom->log_file,"Attribute Group\n");

status = mir_get_name_code( ag_mandle, &name, &code, 0, 0, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

/* Allocate the size of the largest string */

if (current_attribute_group == NULL)
    {
    size = strlen("CHARACTERISTICS") + 1;
    current_attribute_group = (char *)calloc(1, size + 1 );
    }
strcpy( current_attribute_group, name);
current_attribute_group[ strlen(name) ] = '\0';
status = mir_get_group( 0, class, mom, 0, m_mir_dna_attrib_list, ag_mandle);

return( status );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_attrib_list
**
**	This routine adds an attribute to the latest attribute group.
**
**  FORMAL PARAMETERS:
**
**	al_mandle	Pointer to current attribute list mandle.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**	MS_SUCCESS or any MIR error status.
**	MS0_FIND_NONE if no attribute was found.
**
**--
*/
mir_status mir_add_attrib_list( al_mandle, 
			        class, 
			        mom )

mandle *al_mandle;
CLASS_DEF *class;
MOM_BUILD *mom;

{
int found = 0;
mir_status status;
char *name,*symbol;
int size,code,cur_size;
ATTRIBUTE_DEF *attribute;

/* 
 * First, find the name of the attribute, then find this attribute and
 * set the attribute group to be what is in the current_attribute_group
 * global.
 */

cur_size = strlen( class->prefix ) + strlen( attr_prefix ) + 4;
status = mir_get_name_code( al_mandle, &name, &code, &symbol,cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

if (get_attr_class( class, code, &attribute))
    found = 1;
/*
attribute = class->attribute;
if (attribute == NULL)
    fprintf(mom->log_file,"Error -- Empty attribute list\n");
else
    while ((attribute != NULL) && !found ) 
	{
	if (mom->use_symbols)	
	    name = symbol;
        if (strcmp(name,(attribute->attribute_name+NAME_SIZE)) == 0)
            found = 1;
	else
            attribute = attribute->next;
	}
*/

up_case(current_attribute_group);
if (found)
    {
    if (strcmp( current_attribute_group,"STATUS") == 0)
    	attribute->group = GROUP_STATUS;

    else if (strcmp( current_attribute_group,"IDENTIFIERS") == 0)
	{
    	attribute->group = GROUP_IDENTIFIERS;

	/*
	 * See if this is the primary identifier attribute. Only set the
	 * name (primary identifier) flag for the first one.
	 */
	if (!class->support_instances)
	    {
	    class->support_instances = 1;
            attribute->name = 1;
	    }
	}

    else if (strcmp( current_attribute_group,"CHARACTERISTICS") == 0)
	attribute->group = GROUP_CHARACTERISTICS;

    else if (strcmp( current_attribute_group,"COUNTERS") == 0)
	attribute->group = GROUP_COUNTERS;

    else if (strcmp( current_attribute_group,"STATISTICS") == 0)
	attribute->group = GROUP_STATISTICS;

    else if (strcmp( current_attribute_group,"REFERENCES") == 0)
	attribute->group = GROUP_REFERENCES;

    else {
	 fprintf(mom->log_file,"Error---No group attribute list found %s\n", current_attribute_group);
	 return( MS0_FIND_NONE );
	 }
    }
else {
     fprintf(mom->log_file,"Error---No attribute found; Looking for %s\n", name );
     return( MS0_FIND_NONE );
     }      

return( MS_SUCCESS );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_resp_arg
**
**	This routine adds response arguments to the latest response.
**
**  FORMAL PARAMETERS:
**
**	arg_mandle	Pointer to current argument mandle.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**	rel		Relationship value (directive, attribute, etc.)
**
**  RETURN VALUE:
**
**	MS_SUCCESS or any MIR error status.
**
**--
*/
mir_status mir_add_resp_arg( arg_mandle, 
			     dir, 
			     class, 
			     mom )

mandle *arg_mandle;
DIRECTIVE_DEF *dir;
CLASS_DEF *class;
MOM_BUILD *mom;

{
mir_status status;
char *name,*symbol;
char arg_char[4];
int size,code,cur_size;
RESPONSE_DEF *resp_tmp;
ARGUMENT_DEF *arg,*tmp,*arg_tmp;
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */

/* 
 * Cur size = "prefix_" + arg_ + "directive" + _ + _oid 
 */

cur_size = strlen( class->prefix ) + strlen( arg_prefix ) + MIN(6, strlen( dir->directive )) + 1 + 4;
status = mir_get_name_code( arg_mandle, &name, &code, &symbol, cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

arg = (ARGUMENT_DEF *) calloc (1, sizeof (ARGUMENT_DEF));
arg->next = NULL;
arg->argument_number = code;

/* Find the last response in the list */

resp_tmp = dir->resp;
if (resp_tmp==NULL)
    fprintf(mom->log_file,"Internal error --- couldn't find respuest for this arg\n");
else
    while (resp_tmp->next!=NULL)
	resp_tmp=resp_tmp->next;

resp_tmp->num_arguments++;
	
/* Add to the end of the argument list */

arg_tmp = resp_tmp->arg;
if (arg_tmp == NULL)
    resp_tmp->arg = arg;
else
    {
    while (arg_tmp->next!=NULL)
        arg_tmp=arg_tmp->next;
    arg_tmp->next = arg;
    }

if (!mom->use_symbols) {
    size = strlen( name ) + strlen( arg_prefix ) + MIN(6, strlen( dir->directive )) +
	strlen( underscore );
    arg->argument_name = (char *)calloc(1, size + 1 );
    strcpy( arg->argument_name, arg_prefix );
    strncat( arg->argument_name, dir->directive, 6 );
    strcat( arg->argument_name, underscore );
    strcat( arg->argument_name, name );
    }
else
    {
    arg->argument_name = (char *)calloc(1, strlen( symbol ) + 1);
    strcpy( arg->argument_name, symbol );
    }    

if (check_duplicate_arg( dir, class, arg, arg_char ))
    {
    set_arg_char( arg->argument_name, arg_char);
    size = strlen(symbol);
    arg->original_name = (char *)calloc(1, size + 1 );
    strcpy( arg->original_name, symbol );
    }
else if (strlen(symbol) > strlen(name))
    {   
    size = strlen( symbol );
    arg->original_name = (char *)calloc(1, size + 1 );
    strcpy( arg->original_name, symbol );
    }

arg->argument_type = resp_type;
arg->argument_length = resp_type_l;

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( arg_mandle, &arg->snmp_oid_text, &arg->snmp_oid_len, 
			&arg->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( arg_mandle, &arg->osi_oid_text, &arg->osi_oid_len, 
			&arg->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( arg_mandle, &arg->oid_text, &arg->oid_len, 
			&arg->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( arg_mandle, &arg->dna_cmip_int, &arg->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class,   code, &arg->dna_cmip_int, &arg->dna_cmip_int_len, arg->osi_oid_len, 
	   	  arg->oid_len, arg->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

if (mom->attr)
    fprintf(mom->log_file,"Argument %s\n", arg->argument_name );

/*
 * Get the datatype of this create argument (using a temporary attribute).
 */

if (strcmp( dir->directive,"CREATE") == 0)
    {
    dc_ident       ib;            /* Ident block for loading       */
    dc_context    *ctx=NULL;      /* Context block pointer         */
    object_id      new_oid; 	  /* Object id of Structured as    */
    ATTRIBUTE_DEF *attr;

    attr = (ATTRIBUTE_DEF *) calloc (1, sizeof (ATTRIBUTE_DEF));

    oid_smi = OID_ANY;
    status = mir_mandle_to_oid( arg_mandle, &new_oid, &oid_smi );
    if (status != MS_SUCCESS)
        return( status );

    status = mir_lookup_data_construct( &new_oid, &ctx, &ib );
    switch (status) {

      case MS1_DC_BUILTIN:
        if (mom->attr)
	    {
            fprintf(mom->log_file,"\nBUILTIN MIR Object '");
            print_oid( mom, &new_oid );
            dump_IB(&ib,mom);
	    }
        break;
      case MS1_DC_BUILTUP:
        if (mom->attr)
	    {
            fprintf(mom->log_file,"\n****** BUILTUP MIR Object '");
            print_oid( mom, &new_oid );
            dump_IB(&ib,mom);
	    }
        /* Get next level down... */

        status = mir_eval_data_construct(&ctx, &ib);

        if (mom->attr)
	    dump_IB(&ib,mom);
        break;    

      default:
        return( status );
       }
    
    mir_get_data_type_length( ib.name, &arg->data_type, &attr->length, attr, class, mom->oid_type );
    arg->mg_type = attr->mg_type;
    arg->sign = attr->sign;
    if (attr->dna_data_type != NULL)
	{
	arg->dna_data_type = (char *)calloc(1,strlen(attr->dna_data_type)+1);   
        strcpy(arg->dna_data_type, attr->dna_data_type);
	}

    if (attr->avl)
	arg->avl = 1;   

    /*
     * Check to see if the user specified which argument was the ACL argument.
     */

    if (arg->argument_number == class->ACL_argument_num)
        arg->mg_type = MG_C_ACL;
    }
return( MS_SUCCESS );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_req_arg
**
**	This routine adds request arguments to the latest request.
**	Create arguments are handled here too.
**
**  FORMAL PARAMETERS:                            
**
**	arg_mandle	Pointer to current argument mandle.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**	rel		Relationship value (directive, attribute, etc.)
**
**  RETURN VALUE:
**
**	MS_SUCCESS or any MIR error status.
**
**--
*/
mir_status mir_add_req_arg( arg_mandle, 
			    dir, 
			    class, 
			    mom )

mandle *arg_mandle;
DIRECTIVE_DEF *dir;
CLASS_DEF *class;
MOM_BUILD *mom;

{
mir_status status;
char *name,*symbol;
char arg_char[4];
int size,code,cur_size;
REQUEST_DEF *req_tmp;
ARGUMENT_DEF *arg,*tmp,*arg_tmp;
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */

/* 
 * Cur size = "prefix_" + argc_ + _oid  for ARGC (create argument)
 * Cur size = "prefix_" + arg_ + "directive" + _ + _oid  for ARG
 */

if (strcmp( dir->directive,"CREATE") == 0)
    cur_size = strlen( class->prefix ) + strlen( argc_prefix ) + 4; 
else
    cur_size = strlen( class->prefix ) + strlen( arg_prefix ) + MIN(6, strlen( dir->directive )) + 1 + 4;

status = mir_get_name_code( arg_mandle, &name, &code, &symbol, cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

arg = (ARGUMENT_DEF *) calloc (1, sizeof (ARGUMENT_DEF));
arg->next = NULL;
arg->argument_number = code;

/*
 * First find out whether this argument is a create argument. If so,
 * add to the create argument list. If not, add the the last request
 * in the list.
 *
 * NOTE: CREATE Arguments have a slightly different prefix to match
 * 	 the attribute prefix (ie. both have 4 chars). This way (along
 *	 with the restriction that the names are the same, both get
 *	 truncated at the same spot.
 */

if (strcmp( dir->directive,"CREATE") == 0)
    {                             
    arg_tmp = class->create_arg;
    if (arg_tmp == NULL)
	class->create_arg = arg;
    else
	{
	while (arg_tmp->next != NULL)
	    arg_tmp = arg_tmp->next;
	arg_tmp->next = arg;
        }
    class->num_create_args++;
    }
else
    {
    /* Find the last request in the list */
    req_tmp = dir->req;
    if (req_tmp==NULL)
        {
        /*
         * The MIR does not save requests (they don't have any names anyway).
         * Allocate a temporary request with no name for a place holder. There 
         * will never be more than one request anyway.
         */
        REQUEST_DEF *req,*tmp;
    
        req = (REQUEST_DEF *) calloc (1, sizeof (REQUEST_DEF));
        dir->req = req;
        dir->num_requests++;
        req_tmp = req;
        }
    else
        while (req_tmp->next!=NULL)
    	req_tmp=req_tmp->next;
    
    req_tmp->num_arguments++;
    	
    /* Add to the end of the argument list */

    arg_tmp = req_tmp->arg;
    if (arg_tmp == NULL)
        req_tmp->arg = arg;
    else
        {
        while (arg_tmp->next!=NULL)
            arg_tmp=arg_tmp->next;
        arg_tmp->next = arg;
        }
    }

if (!mom->use_symbols) 
  if (strcmp( dir->directive,"CREATE") == 0)
    {	/* Build the create argument */
    size = strlen( name ) + strlen( argc_prefix );
    arg->argument_name = (char *)calloc(1, size + 1 );
    strcpy( arg->argument_name, argc_prefix );
    strcat( arg->argument_name, name );
    if (check_duplicate_create_arg( class, arg ))
        {
        set_dup_char( arg->argument_name, &class->current_dup_char_argc );
        size = strlen(symbol);
        arg->original_name = (char *)calloc(1, size + 1 );
        strcpy( arg->original_name, symbol );
	}
    else if (strlen(symbol) > strlen(name))
        {   
        size = strlen( symbol );
        arg->original_name = (char *)calloc(1, size + 1 );
        strcpy( arg->original_name, symbol );
        }
    }
  else
    {		/* Normal request argument (other than create) */
    size = strlen( name ) + strlen( arg_prefix ) + MIN(6, strlen( dir->directive )) +
	strlen( underscore );
    arg->argument_name = (char *)calloc(1, size + 1);
    strcpy( arg->argument_name, arg_prefix );
    strncat( arg->argument_name, dir->directive, 6 );
    strcat( arg->argument_name, underscore );
    strcat( arg->argument_name, name );

    if (check_duplicate_arg( dir, class, arg, arg_char ))
        {
        set_arg_char( arg->argument_name, arg_char);
        size = strlen(symbol);
        arg->original_name = (char *)calloc(1, size + 1 );
        strcpy( arg->original_name, symbol );
	}
    else if (strlen(symbol) > strlen(name))
        {   
        size = strlen( symbol );
        arg->original_name = (char *)calloc(1, size + 1 );
        strcpy( arg->original_name, symbol );
        }
    }
else	/* of use symbols */
    {
    arg->argument_name = (char *)calloc(1, strlen( symbol ) + 1);
    strcpy( arg->argument_name, symbol );
    }    

arg->argument_type = req_type;
arg->argument_length = req_type_l;

/*
 * Get the datatype of this create argument (using a temporary attribute).
 */

if (strcmp( dir->directive,"CREATE") == 0)
    {
    dc_ident       ib;            /* Ident block for loading       */
    dc_context    *ctx=NULL;      /* Context block pointer         */
    object_id      new_oid; 	  /* Object id of Structured as    */
    ATTRIBUTE_DEF *attr;

    attr = (ATTRIBUTE_DEF *) calloc (1, sizeof (ATTRIBUTE_DEF));

    oid_smi = OID_ANY;
    status = mir_mandle_to_oid( arg_mandle, &new_oid, &oid_smi );
    if (status != MS_SUCCESS)
        return( status );

    status = mir_lookup_data_construct( &new_oid, &ctx, &ib );
    switch (status) {

      case MS1_DC_BUILTIN:
        if (mom->attr)
	    {
            fprintf(mom->log_file,"\nBUILTIN MIR Object '");
            print_oid( mom, &new_oid );
            dump_IB(&ib,mom);
	    }
        break;
      case MS1_DC_BUILTUP:
        if (mom->attr)
	    {
            fprintf(mom->log_file,"\n****** BUILTUP MIR Object '");
            print_oid( mom, &new_oid );
            dump_IB(&ib,mom);
	    }
        /* Get next level down... */

        status = mir_eval_data_construct(&ctx, &ib);

        if (mom->attr)
	    dump_IB(&ib,mom);
        break;    

      default:
        return( status );
       }
    
    mir_get_data_type_length( ib.name, &arg->data_type, &attr->length, attr, class, mom->oid_type );
    arg->mg_type = attr->mg_type;
    arg->sign = attr->sign;
    if (attr->dna_data_type != NULL)
	{
	arg->dna_data_type = (char *)calloc(1,strlen(attr->dna_data_type)+1);   
        strcpy(arg->dna_data_type, attr->dna_data_type);
	}

    if (attr->avl)
	arg->avl = 1;   

    /*
     * Check to see if the user specified which argument was the ACL argument.
     */

    if (arg->argument_number == class->ACL_argument_num)
        arg->mg_type = MG_C_ACL;

    free( attr );
    }

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( arg_mandle, &arg->snmp_oid_text, &arg->snmp_oid_len, 
			&arg->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( arg_mandle, &arg->osi_oid_text, &arg->osi_oid_len, 
			&arg->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( arg_mandle, &arg->oid_text, &arg->oid_len, 
			&arg->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( arg_mandle, &arg->dna_cmip_int, &arg->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class,   code, &arg->dna_cmip_int, &arg->dna_cmip_int_len, arg->osi_oid_len, 
	   	  arg->oid_len, arg->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

if (mom->attr)
    fprintf(mom->log_file,"Argument %s\n", arg->argument_name );

return( MS_SUCCESS );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_exc_arg
**
**	This routine adds exception arguments to the latest exception.
**
**  FORMAL PARAMETERS:
**
**	arg_mandle	Pointer to current argument mandle.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**	rel		Relationship value (directive, attribute, etc.)
**
**  RETURN VALUE:
**
**	MS_SUCCESS or any MIR error status.
**
**--
*/
mir_status mir_add_exc_arg( arg_mandle, 
			    dir, 
			    class, 
			    mom )

mandle *arg_mandle;
DIRECTIVE_DEF *dir;
CLASS_DEF *class;
MOM_BUILD *mom;

{
mir_status status;
char *name,*symbol;
char arg_char[4];
int size,code,cur_size;
EXCEPTION_DEF *exc_tmp;
ARGUMENT_DEF *arg,*tmp,*arg_tmp;
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */

/* 
 * Cur size = "prefix_" + arg_ + "directive" + _ + _oid 
 */

cur_size = strlen( class->prefix ) + strlen( arg_prefix ) + MIN(6, strlen( dir->directive )) + 1 + 4;
status = mir_get_name_code( arg_mandle, &name, &code, &symbol, cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

arg = (ARGUMENT_DEF *) calloc (1, sizeof (ARGUMENT_DEF));
arg->next = NULL;
arg->argument_number = code;

/* Find the last exception in the list */

exc_tmp = dir->exc;
if (exc_tmp==NULL)
    fprintf(mom->log_file,"Internal error --- couldn't find exception for this arg\n");
else
    while (exc_tmp->next!=NULL)
	exc_tmp=exc_tmp->next;

exc_tmp->num_arguments++;
	
/* Add to the end of the argument list */

arg_tmp = exc_tmp->arg;
if (arg_tmp == NULL)
    exc_tmp->arg = arg;
else
    {
    while (arg_tmp->next!=NULL)
        arg_tmp=arg_tmp->next;
    arg_tmp->next = arg;
    }

if (!mom->use_symbols) {
    size = strlen( name ) + strlen( arg_prefix ) + MIN(6, strlen( dir->directive )) +
	strlen( underscore );
    arg->argument_name = (char *)calloc(1, size + 1 );
    strcpy( arg->argument_name, arg_prefix );
    strncat( arg->argument_name, dir->directive, 6 );
    strcat( arg->argument_name, underscore );
    strcat( arg->argument_name, name );
    }
else
    {
    arg->argument_name = (char *)calloc(1, strlen( symbol ) + 1);
    strcpy( arg->argument_name, symbol );
    }    

    if (check_duplicate_arg( dir, class, arg, arg_char ))
        {
        set_arg_char( arg->argument_name, arg_char);
        size = strlen(symbol);
        arg->original_name = (char *)calloc(1, size + 1 );
        strcpy( arg->original_name, symbol );
	}
    else if (strlen(symbol) > strlen(name))
        {   
        size = strlen( symbol );
        arg->original_name = (char *)calloc(1, size + 1 );
        strcpy( arg->original_name, symbol );
        }

arg->argument_type = excep_type;
arg->argument_length = excep_type_l;

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( arg_mandle, &arg->snmp_oid_text, &arg->snmp_oid_len, 
			&arg->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( arg_mandle, &arg->osi_oid_text, &arg->osi_oid_len, 
			&arg->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( arg_mandle, &arg->oid_text, &arg->oid_len, 
			&arg->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( arg_mandle, &arg->dna_cmip_int, &arg->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class,   code, &arg->dna_cmip_int, &arg->dna_cmip_int_len, arg->osi_oid_len, 
	   	  arg->oid_len, arg->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

if (mom->attr)
    fprintf(mom->log_file,"Argument %s\n", arg->argument_name );

return( MS_SUCCESS );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_add_evt_arg
**
**	This routine adds event arguments to the latest event.
**
**  FORMAL PARAMETERS:
**
**	arg_mandle	Pointer to current argument mandle.
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**	rel		Relationship value (directive, attribute, etc.)
**
**  RETURN VALUE:
**
**	MS_SUCCESS or any MIR error status.
**
**--
*/
mir_status mir_add_evt_arg( arg_mandle, 
			    class, 
			    mom )

mandle *arg_mandle;
CLASS_DEF *class;
MOM_BUILD *mom;

{
mir_status status;
char *name,*symbol;
char arg_char[4];
int size,code,cur_size;
EVENT_DEF *event_tmp;
ARGUMENT_DEF *arg,*tmp,*arg_tmp;
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */

/* 
 * Cur size = "prefix_" + evt_ + arg_ + _ + _oid 
 */
cur_size = strlen( class->prefix ) + strlen( evt_prefix ) + strlen( arg_prefix ) + 1 + 4;
status = mir_get_name_code( arg_mandle, &name, &code, &symbol, cur_size, mom ); 
if (status != MS0_FIND_TERM) 
    return( status );

arg = (ARGUMENT_DEF *) calloc (1, sizeof (ARGUMENT_DEF));
arg->next = NULL;
arg->argument_number = code;

/* Find the last event in the list */

event_tmp = class->event;
if (event_tmp==NULL)
    fprintf(mom->log_file,"Internal error --- couldn't find event for this arg\n");
else
    while (event_tmp->next!=NULL)
	event_tmp=event_tmp->next;

event_tmp->num_event_args++;
	
/* Add to the end of the argument list */

arg_tmp = event_tmp->event_arg;
if (arg_tmp == NULL)
    event_tmp->event_arg = arg;
else
    {
    while (arg_tmp->next!=NULL)
        arg_tmp=arg_tmp->next;
    arg_tmp->next = arg;
    }

if (!mom->use_symbols) {
    size = strlen( name ) + strlen( arg_prefix ) + strlen( evt_prefix) + 1;
    arg->argument_name = (char *)calloc(1, size + 1 );
    strcpy( arg->argument_name, arg_prefix );
    strcat( arg->argument_name, evt_prefix );
    strcat( arg->argument_name, name );
    }
else
    {
    arg->argument_name = (char *)calloc(1, strlen( symbol ) + 1);
    strcpy( arg->argument_name, symbol );
    }    

    if (check_duplicate_evt_arg( class, arg, arg_char ))
        {
	set_arg_char( arg->argument_name, arg_char );
        size = strlen(symbol);
        arg->original_name = (char *)calloc(1, size + 1 );
        strcpy( arg->original_name, symbol );
	}
    else if (strlen(symbol) > strlen(name))
        {   
        size = strlen( symbol );
        arg->original_name = (char *)calloc(1, size + 1 );
        strcpy( arg->original_name, symbol );
        }

arg->argument_type = event_type;
arg->argument_length = event_type_l;

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( arg_mandle, &arg->snmp_oid_text, &arg->snmp_oid_len, 
			&arg->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( arg_mandle, &arg->osi_oid_text, &arg->osi_oid_len, 
			&arg->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( arg_mandle, &arg->oid_text, &arg->oid_len, 
			&arg->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( arg_mandle, &arg->dna_cmip_int, &arg->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class,   code, &arg->dna_cmip_int, &arg->dna_cmip_int_len, arg->osi_oid_len, 
	   	  arg->oid_len, arg->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

if (mom->attr)
    fprintf(mom->log_file,"Argument %s\n", arg->argument_name );

if (1)
    {
    dc_ident       ib;            /* Ident block for loading       */
    dc_context    *ctx=NULL;      /* Context block pointer         */
    object_id      new_oid; 	  /* Object id of Structured as    */
    ATTRIBUTE_DEF *attr;

    attr = (ATTRIBUTE_DEF *) calloc (1, sizeof (ATTRIBUTE_DEF));

    oid_smi = OID_ANY;
    status = mir_mandle_to_oid( arg_mandle, &new_oid, &oid_smi );
    if (status != MS_SUCCESS)
        return( status );

    status = mir_lookup_data_construct( &new_oid, &ctx, &ib );
    switch (status) {

      case MS1_DC_BUILTIN:
        if (mom->attr)
	    {
            fprintf(mom->log_file,"\nBUILTIN MIR Object '");
            print_oid( mom, &new_oid );
            dump_IB(&ib,mom);
	    }
        break;
      case MS1_DC_BUILTUP:
        if (mom->attr)
	    {
            fprintf(mom->log_file,"\n****** BUILTUP MIR Object '");
            print_oid( mom, &new_oid );
            dump_IB(&ib,mom);
	    }
        /* Get next level down... */

        status = mir_eval_data_construct(&ctx, &ib);

        if (mom->attr)
	    dump_IB(&ib,mom);
        break;    

      default:
        return( status );
       }
    
    mir_get_data_type_length( ib.name, &arg->data_type, &attr->length, attr, class, mom->oid_type );
    arg->mg_type = attr->mg_type;
    arg->sign = attr->sign;
    if (attr->dna_data_type != NULL)
	{
	arg->dna_data_type = (char *)calloc(1,strlen(attr->dna_data_type)+1);   
        strcpy(arg->dna_data_type, attr->dna_data_type);
	}

    if (attr->avl)
	arg->avl = 1;   

    /*
     * Check to see if the user specified which argument was the ACL argument.
     */

    if (arg->argument_number == class->ACL_argument_num)
        arg->mg_type = MG_C_ACL;
    }

return( MS_SUCCESS );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_get_group
**
**	This routine reads all of a certain type of relationship.
**	It then calls the appropriate init routine to add the relationship
**	(dir, attr, etc...) into the class structure.
**
**  FORMAL PARAMETERS:
**
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**	dir		Pointer to current directive.
**	rel		Relationship mandle value (directive, attribute, etc).
**	class_mandle	Pointer to current class mandle.
**
**  RETURN VALUE:
**
**      MS_SUCCESS or any MIR error status
**
**--
*/
mir_status mir_get_group( arg_type,
			  class, 
			  mom, 
			  dir,
			  rel,
			  class_mandle )

int arg_type;
CLASS_DEF *class;
MOM_BUILD *mom;
DIRECTIVE_DEF *dir;
mandle *rel;
mandle *class_mandle;

{
search_style 	search;
int		status;
mir_status 	s_status;
mandle   	*non_term=NULL;  /* Mandle used to rcv NON-TERM     */
mandle   	*type_mandle=NULL;  /* Mandle used to rcv NON-TERM     */
mir_value       m_value;        /* Terminal value structure              */

/*
 * Initialize the type_mandle to loop through.
 */

search = SEARCH_FROMTOP;

s_status = MS0_FIND_NONTERM;
while (s_status == MS0_FIND_NONTERM)
    {
    s_status = get_mandle( search, rel, class_mandle, &type_mandle, &m_value );
 
    if (s_status == MS0_FIND_NONTERM)
      switch (arg_type) {
	case RESPONSE_ARG:
	    status = mir_add_resp_arg( type_mandle, dir, class, mom );
	    if (status != MS_SUCCESS)
            	return( status );
	    break;
	case EXCEPTION_ARG:
	    status = mir_add_exc_arg( type_mandle, dir, class, mom );
   	    if (status != MS_SUCCESS)
                return( status );
	    break;
	case EVENT_ARG:
	    status = mir_add_evt_arg( type_mandle, class, mom );
	    if (status != MS_SUCCESS)
            	return( status );
	    break;
       
        default:
      if (rel == m_mir_dna_cont_attribute) { /* Attributes */ 
	status = mir_add_attr( type_mandle, class, mom );
	if (status != MS_SUCCESS)
            return( status );
	} else
      if (rel == m_mir_dna_cont_attrpart) { /* Attribute partitions */
	status = mir_add_part( type_mandle, class, mom );
	if (status != MS_SUCCESS)
            return( status );
	} else
      if (rel == m_mir_dna_attrib_list)	   { /* Attribute List */
	status = mir_add_attrib_list( type_mandle, class, mom );
	if (status != MS_SUCCESS)
            return( status );
	} else
      if (rel == m_mir_dna_cont_directive) { /* Directives */
	status = mir_add_dir( type_mandle, class, mom );
	if (status != MS_SUCCESS)
            return( status );
	} else
      if (rel == m_mir_dna_cont_response) { /* Responses */
	status = mir_add_resp( type_mandle, dir, class, mom );
	if (status != MS_SUCCESS)
            return( status );
	} else
      if (rel == m_mir_dna_cont_request_arg) { /* Request Arguments */
	status = mir_add_req_arg( type_mandle, dir, class, mom );
	if (status != MS_SUCCESS)
            return( status );
	} else
      if (rel == m_mir_dna_cont_exception) { /* Exceptions */
	status = mir_add_exc( type_mandle, dir, class, mom );
	if ((status != MS_SUCCESS) && (status != MOMGEN_C_NO_DNA_CMIP_INT))
            return( status );
	} else
      if (rel == m_mir_dna_cont_event) { /* Events */
	status = mir_add_evt( type_mandle, class, mom );
	if (status != MS_SUCCESS )
            return( status );
	}
      }
    else if (s_status != MS0_FIND_NONE)
	 return (s_status);

    search = SEARCH_FROMLAST;
    }

return( MS_SUCCESS );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_map_error
**
**	This routine maps a MIR error into a MOMGEN error.
**
**  FORMAL PARAMETERS:
**
**	status		MIR error to map.
**
**  RETURN VALUE:
**
**	Any MOMGEN error status.
**
**--
*/
int mir_map_error( m_status )

mir_status m_status;

{
int status;

switch (m_status) {
    case MS_NO_MEMORY:        /* Insufficient Memory available                 */
	    return MOMGEN_C_INSVIRMEM;
	    break;	
    case MS0_FIND_NONTERM:    /* A match was found and NonTerminal mandle rtned*/
    case MS0_FIND_TERM:       /* A match was found and a Terminal was returned */
    case MS0_FIND_EXACT:      /* Found an exact match                          */
    case MS1_DC_BUILTIN:      /* Built-In Data Construct found and returned    */
    case MS1_DC_BUILTUP:      /* Built-Up Data Construct found and returned    */
    case MS1_DC_BUILTIN_TMPLT:/* Built-In Data Construct Template fnd & ret.   */
    case MS1_TMPLT_NUMBER:    /* Built-In Template "Number Value"              */
    case MS1_TMPLT_STRING:    /* Built-In Template "String Value"              */
    case MS_SUCCESS:          /* The operation succeeded                       */
	    return SS$_NORMAL;
	    break;
    case MS0_FIND_NONE:       /* No object was found                           */
	    return MOMGEN_C_NO_SUCH_CLASS;
	    break;

    case MS_NOT_IMPLEMENTED:  /* Function not implemented yet                  */
    case MS0_DB_NOT_LOADED:  /* Loading of MIR Database File failed           */
    case MS0_INDEX_CORRUPTED: /* MIR Database File Index is corrupted          */
    case MS0_NT_CORRUPTED:    /* MIR Database File Non-Terminals are corrupted */
    case MS0_INVALID_OID:     /* A pointer to a required obj. id was null      */
    case MS0_INVALID_MANDLE:  /* A pointer to a Mandle passed on the call was  */
                             /*   invalid or the mandle was invalid           */
    case MS0_FIND_SHORT:      /* The supplied OID was too short to fully match */
                             /*   the entry found in the MIR                  */
    case MS0_FIND_LONG:       /* The supplied OID was too long to fully match  */
                             /*   the first non-terminal object index entry   */
    case MS1_NOT_A_VALUED_OBJECT:    /* Obj. Found, not defined w/value        */
    case MS1_EXACT_OBJ_NOT_FND:      /* No Object by that exact Obj. ID        */
    case MS1_MISSING_SMI:     /* SMI Relationship was inexplicably missing     */
    case MS1_INIT_FAIL:       /* Initialization of Relationship Mandles failed */
    case MS1_DC_NT_CORRUPT:   /* Non-Terms describing Data-Constructs corrupted*/
    case MS1_INVALID_ARG:     /* Invalid Context or Ident Block pointer arg    */
    default:
	    return MOMGEN_C_MIR_ERROR;
	    break;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	get_MIR_defs
**
**	This routine reads all of the attributes from the MIR of
**	a given class.
**
**  FORMAL PARAMETERS:
**
**	class		Pointer to current class to read.
**      mom		Pointer to the MOM description block.
**
**  RETURN VALUE:
**
**      SS$_NORMAL	Normal successful completion.
**	Any MOMGEN error status.
**
**--
*/
int get_MIR_defs( class, mom )

CLASS_DEF *class;
MOM_BUILD *mom;

{
char		*name,*symbol;
int		code;
search_style	search;
int		rel,size;
mandle   	*parent_mandle=NULL;  /* Mandle used to rcv NON-TERM     */
mandle   	*class_mandle=NULL;  /* Mandle used to rcv NON-TERM     */
mandle   	*non_term=NULL;  /* Mandle used to rcv NON-TERM     */
mir_value       m_value;        /* Terminal value structure              */
mir_status      status;         /* status from other tier 0 calls        */
mir_status      s_status;       /* status from other tier 0 calls        */
mir_oid_smi     oid_smi;        /* SMI indicator for mir_mandle_to_oid() */


if (mom->log)
    fprintf(mom->log_file,"Reading Common Agent MIR...\t\t\t");

/*
 * Load all of the local mandles to be used while reading the MIR.
 */

status = load_local_mandles(mom->mir_file);
if (status != MS_SUCCESS)
    {
    if (status == MOMGEN_C_MIR_NOT_FOUND)
        return status;
    fprintf(stderr,"\n\n**** Error locating class %s **** \n",class->orig_class_string);
    return( MOMGEN_C_NO_SUCH_CLASS );
    }

/* 
 * First get the class mandle to be used to read all of the definitions.
 */

status = get_class_mandle( class->class_oid, &class_mandle );
if (status != MS0_FIND_EXACT)
    {
    fprintf(stderr,"\n\n**** Error locating class %s **** \n",class->orig_class_string);
    return( MOMGEN_C_NO_SUCH_CLASS );
    }

/*
 * Get the name of the class.
 */

status = mir_get_name_code( class_mandle, &class->class_name, &code, &symbol, 0, mom ); 
if (status != MS0_FIND_TERM) 
    {
    fprintf(stderr,"\n\n**** Could not find name code for class %s **** \n",class->orig_class_string);
/*    return( MOMGEN_C_NO_SUCH_CLASS ); */
    }

/*
 * Set up the class_ptr name (used in the CLASS_DEF structure )
 */

class->class_name_ptr = (char *)calloc(1, strlen( class->class_name ) + 5);
strcpy( class->class_name_ptr, class->class_name);
strcat( class->class_name_ptr, "_PTR");

/* 
 * Read the three types of OIDs (OID, SNMP_OID, OSI_CMIP_OID ).
 */

status = mir_get_oid( class_mandle, &class->snmp_oid_text, &class->snmp_oid_len, 
			&class->snmp_oid, OID_SNMP ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( class_mandle, &class->osi_oid_text, &class->osi_oid_len, 
			&class->osi_oid, OID_OSI ); 
if ERROR_CONDITION( status )
    return status; 
status = mir_get_oid( class_mandle, &class->oid_text, &class->oid_len, 
			&class->oid, OID_OID ); 
if ERROR_CONDITION( status )
    return status; 

/* 
 * Read the DNA_CMIP_INT code. 
 */

status = mir_get_dna_cmip_int( class_mandle, &class->dna_cmip_int, &class->dna_cmip_int_len );
if ERROR_CONDITION( status )
    return status;

/*
 * Make sure at least of the OIDs is specified. If not, return an error.
 */

status = mir_check_oid( mom, class, code, &class->dna_cmip_int, &class->dna_cmip_int_len, class->osi_oid_len, 
	   	  class->oid_len, class->snmp_oid_len );
if ERROR_CONDITION( status )
    return status;

if (mom->attr)
    fprintf(mom->log_file,"Class OID %s\n", class->snmp_oid_text);

class->class_number = code;

if (mom->log) 
    {
    fprintf(mom->log_file,"Done!\n");
    fprintf(mom->log_file,"Reading definition data for class %s...\t", class->class_name);
    }

/*
 * Find the parent using the "Contained By" relationship.
 */

status = get_mandle( SEARCH_FROMTOP,
		     m_mir_contained_by, 
		     class_mandle,		
		     &parent_mandle,
		     &m_value );
if (status != MS0_FIND_NONTERM) 
    {
    printf("\n\n**** Error locating parent for class %s **** \n",class->orig_class_string);
    return( MOMGEN_C_NO_SUCH_CLASS );
    }

/* 
 *  Build the class_oid_below_node name -- prefix + "CLASS_" + class_name 
 */ 

size = strlen(class->prefix) + strlen(class->class_name) + 
			strlen( string_class ) + 1;
class->oid_below_node = (char *) calloc( 1, size+1 );
strcpy(class->oid_below_node, class->prefix);
strcat(class->oid_below_node, string_class);
strcat(class->oid_below_node, underscore);
strcat(class->oid_below_node, class->class_name);
class->oid_below_node[size] = 0;
if ((mom->oid_type == DNA_CMIP_OID) || (mom->oid_type == OSI_CMIP_OID))
    up_case( class->oid_below_node );

/* 
 * If supporting instances, set the void name to be [[class]]_DEF otherwise VOID.
 */

if (mom->support_instances)
    {
    class->class_void_name = (char *)calloc(1,strlen(class->class_name) + 5);
    strcpy( class->class_void_name, class->class_name );
    strcat( class->class_void_name, "_DEF");
    }
else
    class->class_void_name = "void";

if (mom->attr)
    printf("class OID %s\n",class->oid_below_node);

/*
 * Get the name of the parent.
 */

status = mir_get_name_code( parent_mandle, &class->parent, &code, &symbol, 0, mom ); 
if (status != MS0_FIND_TERM) 
    {
    fprintf(stderr,"\n\n**** Could not parent name code for class %s **** \n",class->orig_class_string);
    fprintf(stderr,"**** Using NODE as parent ****\n");
    /* No parent was found - USE NODE */
    class->parent = (char *)calloc(1, 5);
    strcpy( class->parent, "NODE" );
/*    return( MOMGEN_C_NO_SUCH_CLASS );  */
    }

    if ((mom->oid_type == DNA_CMIP_OID) || (mom->oid_type == OSI_CMIP_OID))
        up_case( class->parent );
    if (mom->attr) 
        printf("\nParent %s\n",class->parent);  /* attr */

/* 
 * Read the directives.
 */

status = mir_get_group( 0, class, mom, 0, m_mir_dna_cont_directive, class_mandle );
if (status != MS_SUCCESS)
    return( mir_map_error( status ));
/* 
 * Read the attributes.
 */

status = mir_get_group( 0, class, mom, 0, m_mir_dna_cont_attribute, class_mandle );
if (status != MS_SUCCESS)
    return( mir_map_error(  status ));

/* 
 * Read the attribute partitions
 */

status = mir_get_group( 0, class, mom, 0, m_mir_dna_cont_attrpart, class_mandle );
if (status != MS_SUCCESS)
    return( mir_map_error(  status ));

/* 
 * Read the events.
 */

status = mir_get_group( 0, class, mom, 0, m_mir_dna_cont_event, class_mandle );
if (status != MS_SUCCESS)
    return( mir_map_error(  status ));

if (mom->log)
    fprintf(mom->log_file,"Done!\n");

check_identifier_attribute( mom, class );

return( SS$_NORMAL );
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	mir_get_data_type_length
**
**	This routine converts an MIR datatype to ASN1 and DNA datatypes.
**	The conversions were obtained from a routine within the PE that also
**	needs to do this conversion.
**
**  FORMAL PARAMETERS:
**
**	mir_code	MIR datatype string to convert to ASN1 datatype
**	dt		Address of pointer to name string
**	len		Address of pointer to length string
**	attr		Pointer to current attribute
**
**  RETURN VALUE:
**
**      None
**
**--
*/
void	mir_get_data_type_length( mir_code,
		                  dt, 
		                  len, 
		                  attr,
			          class,
                                  oid_type )

char *mir_code;
char **dt;
char **len;
ATTRIBUTE_DEF *attr;
CLASS_DEF *class;
int oid_type;

{
char *dna_data_type = NULL;
char *data_type;
char *length=0;	/* The C compiler breaks since this if/then/else is too long..*/

up_case(mir_code);

      if (strcmp(mir_code, "BITSET" ) == 0) { 
	attr->mg_type = MG_C_BIT;
	dna_data_type = "ASN1_C_BITSTRING"  ; 
	data_type = "BITSTRING";	   
	length = "4"; }
      else if (strcmp(mir_code, "BIT_BITSET" ) == 0) { 
	attr->mg_type = MG_C_BIT;
	dna_data_type = "ASN1_C_BITSTRING"  ; 
	data_type = "BITSTRING";	   
	length = "4"; }
      else if (strcmp(mir_code, "BIDT_BITSET" ) == 0) { 
	attr->mg_type = MG_C_BIT;
	dna_data_type = "ASN1_C_BITSTRING"  ; 
	data_type = "BITSTRING";	   
	length = "4"; }
      else if (strcmp(mir_code, "BITSTRING" ) == 0) { 
	attr->mg_type = MG_C_BIT;
	dna_data_type = "ASN1_C_BITSTRING"; 
	data_type = "BITSTRING";	   
	length = "4";
	}
      else if (strcmp(mir_code, "BOOLEAN" ) == 0) { 
	attr->mg_type = MG_C_BOOLEAN;
	dna_data_type = "ASN1_C_BOOLEAN";  
	data_type  = "BOOLEAN";       
	length = "4"; 
	}
      else if (strcmp(mir_code, "KNOWN" ) == 0) { 
	attr->mg_type = MG_C_KNOWN;
	dna_data_type = "DNA_C_KNOWN" ; 
	data_type = "NULL";	   
	length = "0"; 
	}
      else if (strcmp(mir_code, "NULL" ) == 0) { 
	attr->mg_type = MG_C_NULL;
	dna_data_type = "ASN1_C_NULL"; 
	data_type  = "NULL";	   
	length = "0"; 
	}
      else if (strcmp(mir_code, "OBJECT_IDENTIFIER" ) == 0) { 
	attr->mg_type = MG_C_OID;
	dna_data_type = "ASN1_C_OBJECT_ID"; 
	data_type = "OBJECT_ID"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "OBJECTIDENTIFIER" ) == 0) { 
	attr->mg_type = MG_C_OID;
	dna_data_type = "ASN1_C_OBJECT_ID"; 
	data_type = "OBJECT_ID"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "OBJECT_ID" ) == 0) { 
	attr->mg_type = MG_C_OID;
	dna_data_type = "ASN1_C_OBJECT_ID"; 
	data_type = "OBJECT_ID"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "OBJECTID" ) == 0) { 
	attr->mg_type = MG_C_OID;
	dna_data_type = "ASN1_C_OBJECT_ID"; 
	data_type = "OBJECT_ID"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "VAR_RECORD" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "RECORD" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "BIT_RECORD" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "BIDT_RECORD" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "RANGE" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "DNA_C_RANGE"; 
	data_type = "SEQUENCE"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "SUBRANGE" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "DNA_C_RANGE"; 
	data_type = "SEQUENCE"; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "SEQUENCE_OF" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1; 
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"; length = "4"; 
	}
      else if (strcmp(mir_code, "SEQUENCE" ) == 0) {
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1;  
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"; length = "4"; 
	}
      else if (strcmp(mir_code, "SET_OF" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1; 
	dna_data_type = "ASN1_C_SET"; 
	data_type = "SET"     ; length = "4"; 
	}
      else if (strcmp(mir_code, "SET" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1; 
	dna_data_type = "ASN1_C_SET"; 
	data_type = "SET"     ; length = "4"; 
	}
      else if (strcmp(mir_code, "ATTRIB_LIST" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1; 
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"     ; length = "4"; 
	}
      else if (strcmp(mir_code, "IMPLEMENTATION" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1; 
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"     ; length = "4"; 
	}
      else if (strcmp(mir_code, "ENTITYCLASS" ) == 0) { 
	attr->mg_type = MG_C_OID;
	dna_data_type = "DNA_C_ENTITY_CLASS"; 
	data_type = "OBJECT_ID"     ; length = "4"; 
	}
      else if (strcmp(mir_code, "BIT_SET" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1;  
	dna_data_type = "ASN1_C_SET"; 
	data_type = "SET"     ; length = "4"; 
	}
      else if (strcmp(mir_code, "BIDT_SET" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1;  
	dna_data_type = "ASN1_C_SET"; 
	data_type = "SET"     ; length = "4"; 
	}
      else if (strcmp(mir_code, "BIT_SEQ" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1;  
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"     ; length = "4"; 
	}
      else if (strcmp(mir_code, "BIDT_SEQ" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	attr->avl = class->avl_attribute = 1;  
	dna_data_type = "ASN1_C_SEQUENCE"; 
	data_type = "SEQUENCE"     ; length = "4"; 
	}

      if (length == 0)

      if (strcmp(mir_code, "REAL" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_SIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "8";  
	}
      else if (strcmp(mir_code, "INTEGER8" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_SIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "1";  
	}
      else if (strcmp(mir_code, "INTEGER16" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_SIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "2";  
	}
      else if (strcmp(mir_code, "INTEGER" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_SIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "4";  
	}
      else if (strcmp(mir_code, "INTEGER32" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_SIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "4";  
	}
      else if (strcmp(mir_code, "INTEGER64" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_SIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "8";  
	}
      else if (strcmp(mir_code, "UNSIGNED8" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "2";  
	}
      else if (strcmp(mir_code, "UNSIGNED16" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "3";  
	}
      else if (strcmp(mir_code, "UNSIGNED32" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "5";  
	}
      else if (strcmp(mir_code, "UNSIGNED64" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "9";  
	}
      else if (strcmp(mir_code, "COUNTER16" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "DNA_C_COUNTER16"; 
	data_type = "INTEGER"; 
	length = "3";  
	}
      else if (strcmp(mir_code, "COUNTER32" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "DNA_C_COUNTER32"; 
	data_type = "INTEGER"; 
	length = "5";  
	}
      else if (strcmp(mir_code, "COUNTER64" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "DNA_C_COUNTER64"; 
	data_type = "INTEGER"; 
	length = "9";  
	}
      else if (strcmp(mir_code, "PHASE4ADDRESS" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "DNA_C_PHASE_4_ADDRESS"; 
	data_type = "INTEGER"; 
	length = "4";  
	}
      else if (strcmp(mir_code, "COMPONENT" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "DNA_C_COMPONENT_NAME"; 
	data_type = "SEQUENCE"; 
	length = "4";  
	}
      else if (strcmp(mir_code, "ENUMERATION" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "4";  
	}
      else if (strcmp(mir_code, "BIT_ENUMERATION" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "4";  
	}
      else if (strcmp(mir_code, "BIDT_ENUMERATION" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "ASN1_C_INTEGER"; 
	data_type = "INTEGER"; 
	length = "4";  
	}
      else if (strcmp(mir_code, "UID" ) == 0) { 
	attr->mg_type = MG_C_UID;
	attr->uid = 1; 
	dna_data_type = "DNA_C_UID"; 
	data_type = "OCTET_STRING" ; 
	length = length_uid; 
	}
      else if (strcmp(mir_code, "OCTET_STRING" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_OCTET"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "OCTETSTRING" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "ASN1_C_OCTET_STRING"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "LATIN1STRING" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_LATIN1_STRING"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "SIMPLENAME" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_SIMPLE_NAME"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "SIMPLE_NAME" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_SIMPLE_NAME"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "FULL_NAME" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_FULL_NAME"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "FULLNAME" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_FULL_NAME"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "FULLENTITY" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_FULL_ENTITY_NAME"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "FULL_ENTITY" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_FULL_ENTITY_NAME"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "LOCALENTITY" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "DNA_C_LOCAL_ENTITY_NAME"; 
	data_type = "SEQUENCE" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "LOCAL_ENTITY" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "DNA_C_LOCAL_ENTITY_NAME"; 
	data_type = "SEQUENCE" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "FILE_SPEC" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_FILE_SPEC"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "DIRECTORY_SPEC" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_FILE_SPEC"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "VERSION" ) == 0) { 
	attr->mg_type = MG_C_VERSION;
	dna_data_type = "DNA_C_VERSION"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "ID802" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_ID802"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "ADDRESS_NSAP" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_NSAP_ADDRESS"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "ADDRESS_NET" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_VERSION"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "ADDRESS_AREA" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_AREA_ADDRESS"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "ADDRESS_PREFIX" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_ADDRESS_PREFIX"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "ADDRESS_TSEL" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "ASN1_C_OCTET_STRING"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "ADDRESS_DTE" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_DTE_ADDRESS"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "PHASE4NAME" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_PHASE_4_NAME"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "IPADDRESS" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	data_type = "OCTET_STRING" ; 
	length = "4"; 
        if (oid_type != SNMP_OID)
	  dna_data_type = "ASN1_C_OCTET_STRING";
	else
	  dna_data_type = "INET_C_SMI_IP_ADDRESS"; 
	}
      else if (strcmp(mir_code, "OCTET" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_OCTET"; 
	data_type = "OCTET_STRING" ; 
	length = "1"; 
	}
      else if (strcmp(mir_code, "HEXSTRING" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_HEX_STRING"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "BIN_ABS_TIM" ) == 0) { 
	attr->mg_type = MG_C_TIME;
	dna_data_type = "DNA_C_BIN_ABS_TIME"; 
	data_type = "OCTET_STRING" ; 
	length = "16"; 
	}
      else if (strcmp(mir_code, "BINABSTIM" ) == 0) { 
	attr->mg_type = MG_C_TIME;
	dna_data_type = "DNA_C_BIN_ABS_TIME"; 
	data_type = "OCTET_STRING" ; 
	length = "16"; 
	}
      else if (strcmp(mir_code, "CHAR_ABS_TIM" ) == 0) { 
	attr->mg_type = MG_C_TIME;
	dna_data_type = "DNA_C_CHAR_ABS_TIME"; 
	data_type = "OCTET_STRING" ; 
	length = "16"; 
	}
      else if (strcmp(mir_code, "CHARABSTIM" ) == 0) { 
	attr->mg_type = MG_C_TIME;
	dna_data_type = "DNA_C_CHAR_ABS_TIME"; 
	data_type = "OCTET_STRING" ; 
	length = "16"; 
	}
      else if (strcmp(mir_code, "BIN_REL_TIM" ) == 0) { 
	attr->mg_type = MG_C_TIME;
	dna_data_type = "DNA_C_BIN_REL_TIME"; 
	data_type = "OCTET_STRING" ; 
	length = "16"; 
	}
      else if (strcmp(mir_code, "BINRELTIM" ) == 0) { 
	attr->mg_type = MG_C_TIME;
	dna_data_type = "DNA_C_BIN_REL_TIME"; 
	data_type = "OCTET_STRING" ; 
	length = "16"; 
	}
      else if (strcmp(mir_code, "CHAR_REL_TIM" ) == 0) { 
	attr->mg_type = MG_C_TIME;
	dna_data_type = "DNA_C_CHAR_REL_TIME"; 
	data_type = "OCTET_STRING" ; 
	length = "16"; 
	}
      else if (strcmp(mir_code, "CHARRELTIM" ) == 0) { 
	attr->mg_type = MG_C_TIME;
	dna_data_type = "DNA_C_CHAR_REL_TIME"; 
	data_type = "OCTET_STRING" ; 
	length = "16"; 
	}
      else if (strcmp(mir_code, "TOWERSET" ) == 0) { 
	attr->mg_type = MG_C_AVL;
	dna_data_type = "DNA_C_TOWER_SET"; 
	data_type = "SEQUENCE" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "END_USER_SPEC" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "DNA_C_END_USER_SPEC"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "COUNTER" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "INET_C_SMI_COUNTER"; 
	data_type = "INTEGER" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "GAUGE" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "INET_C_SMI_GAUGE"; 
	data_type = "INTEGER" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "TIMETICKS" ) == 0) { 
	attr->mg_type = MG_C_INTEGER;
	attr->sign = MG_C_UNSIGNED;
	dna_data_type = "INET_C_SMI_TIME_TICKS"; 
	data_type = "INTEGER" ; 
	length = "4"; 
	}
      else if (strcmp(mir_code, "OPAQUE" ) == 0) { 
	attr->mg_type = MG_C_STRING;
	dna_data_type = "INET_C_SMI_OPAQUE"; 
	data_type = "OCTET_STRING" ; 
	length = "4"; 
	}

/*
      case MCC_K_DT_VMS_ERROR     : 
      case MCC_K_DT_MCC_ERROR     : 
      case MCC_K_DT_MCC_REPLY     : 
      case MCC_K_DT_FLOATF        : 
      case MCC_K_DT_ATTRIBUTEIDLIST:
      case MCC_K_DT_INTERNET_NAME :
      case MCC_K_DT_MESSAGE       : 
      case MCC_K_DT_EVENT_REPORT   :
      case MCC_K_DT_EVENTIDLIST    :
        case MCC_K_DT_EXPRESSION:
        case MCC_K_DT_COUNTER48:     
        case MCC_K_DT_LCOUNTER16:
        case MCC_K_DT_LCOUNTER32:
        case MCC_K_DT_ID802_SNAP:
        case MCC_K_DT_ID802_SAP:
        case MCC_K_DT_IDENETV2_TYPE:
        case MCC_K_DT_NSCTS: */

        else { printf("\n**** MIR data type unknown: %s ****\n",mir_code); 
		data_type = "Unknown"; 
		length = "Unknown"; 
		dna_data_type = "Unknown"; 
		attr->mg_type = MG_C_UNKNOWN;
	     }

 *dt = data_type;
 *len = length;

 attr->dna_data_type = (char *)calloc(1, strlen(dna_data_type) + 1);
 memcpy( attr->dna_data_type, dna_data_type, strlen(dna_data_type));
}
