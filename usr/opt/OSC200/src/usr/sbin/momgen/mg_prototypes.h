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
/*
 * @(#)$RCSfile: mg_prototypes.h,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/07/13 22:19:19 $
 */
/*
**++
**  FACILITY:	MOMGEN
**
**  MODULE DESCRIPTION:
**
**	 This header file for the MOMGEN builder containing function
**	 prototypes and common include files.
**
**  AUTHORS:
**
**	Mike Densmore
**
**  CREATION DATE:  23-Sep-1992
**
**  MODIFICATION HISTORY:
**
**
**--
*/


#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
# define PROTOTYPE(args) args
#else
# define PROTOTYPE(args) ()
#endif

#include <stdlib.h>

void	check_identifier_attribute  PROTOTYPE((
	    MOM_BUILD   *,
	    CLASS_DEF   * ));

void	up_case	PROTOTYPE((
	    char	* ));

int	get_attr_class	PROTOTYPE((
	    CLASS_DEF   *,
	    int,
	    ATTRIBUTE_DEF   ** ));

void	insert_attr PROTOTYPE((
	    ATTRIBUTE_DEF   *,
	    CLASS_DEF	    * ));

void	insert_dir  PROTOTYPE((
	    DIRECTIVE_DEF   *,
	    CLASS_DEF	    * ));

void    insert_event	PROTOTYPE((
	    EVENT_DEF   *,
	    CLASS_DEF   * ));

void	print_class PROTOTYPE((
	    MOM_BUILD   *,
	    CLASS_DEF   * ));

void	print_oid   PROTOTYPE((
	    MOM_BUILD   *,
	    object_id   * ));

int	check_duplicate_arg PROTOTYPE((
	    DIRECTIVE_DEF   *,
	    CLASS_DEF	    *,
	    ARGUMENT_DEF    *,
	    char	    * ));

int     check_duplicate_evt_arg	PROTOTYPE((
	    CLASS_DEF	    *,
	    ARGUMENT_DEF    *,
	    char	    * ));

int	check_duplicate_exc PROTOTYPE((
	    CLASS_DEF	    *,
	    DIRECTIVE_DEF   *,
	    EXCEPTION_DEF   * ));

int	check_duplicate_req PROTOTYPE((
	    CLASS_DEF       *,
            DIRECTIVE_DEF   *,
            REQUEST_DEF	    * ));

int     check_duplicate_resp	PROTOTYPE((
            CLASS_DEF       *,
            DIRECTIVE_DEF   *,
            RESPONSE_DEF    * ));

int	check_duplicate_event	PROTOTYPE((
	    CLASS_DEF	*,
	    EVENT_DEF	* ));

int	check_duplicate_create_arg  PROTOTYPE((
	    CLASS_DEF	    *,
	    ARGUMENT_DEF    * ));

int	check_duplicate	PROTOTYPE((
	    CLASS_DEF	    *,
	    ATTRIBUTE_DEF   * ));

void	set_dup_char	PROTOTYPE((
	    char    *,
	    int	    * ));

int	add_class   PROTOTYPE((
	    char	*,
	    MOM_BUILD	* ));

int	add_oid	PROTOTYPE((
	    CLASS_DEF	*,
	    MOM_BUILD   * ));

int	read_info_file	PROTOTYPE((
	    MOM_BUILD   * ));

int	add_aes PROTOTYPE((
	    CLASS_DEF   *,
            MOM_BUILD   * ));

void	set_arg_char	PROTOTYPE((
	    char    *,
	    char    * ));

void	mir_get_data_type_length    PROTOTYPE((
	    char	    *,
	    char	    **,
	    char            **,
	    ATTRIBUTE_DEF   *,
	    CLASS_DEF	    *,
            int ));

#ifdef UNIXPLATFORM
int	get_qual    PROTOTYPE((
	    int,
	    char	**,
	    MOM_BUILD	* ));
#endif

int	get_MIR_defs	PROTOTYPE((
	    CLASS_DEF	*,
	    MOM_BUILD   * ));

int	get_MCC_defs	PROTOTYPE((
	    CLASS_DEF	*,
	    MOM_BUILD   * ));

momgen_status	build_mom   PROTOTYPE((
		    MOM_BUILD   * ));

int	ca_check_developer_license PROTOTYPE(( ));

momgen_status move_file_to_output   PROTOTYPE((
		    MOM_BUILD     *,
                    char          *,
                    FILE          * ));

momgen_status create_output_file    PROTOTYPE((
		    MOM_BUILD      *,
                    char           *,
                    char           * ));

momgen_status create_output_per_class	PROTOTYPE((
		    MOM_BUILD *,
                    char      *,
                    char      * ));

momgen_status create_output_file_build	PROTOTYPE((
		    MOM_BUILD	*mom,
                    char        *,
                    char        * ));

momgen_status move_file_to_output   PROTOTYPE((
		    MOM_BUILD	*,
                    char        *,
                    FILE        * ));


int	get_create_arg	PROTOTYPE((
	    CLASS_DEF	    *,
            ATTRIBUTE_DEF   *,
            ARGUMENT_DEF    ** ));

int	create_arg_not_attribute    PROTOTYPE((
	    CLASS_DEF	    *,
            ARGUMENT_DEF    * ));

int	arg_create_not_attribute    PROTOTYPE((
	    CLASS_DEF	    *,
            ARGUMENT_DEF    *,
            ATTRIBUTE_DEF   ** ));

void    get_identifier_attr PROTOTYPE((
	    CLASS_DEF	    *,
            ATTRIBUTE_DEF   ** ));

