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
 * @(#)$RCSfile: build_prototypes.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:48:11 $
 */
 /*
 **++
 **  FACILITY:  [[facility]]
 **
 **  Copyright (c) [[copyright_date]]  [[copyright_owner]]
 **
 **  MODULE DESCRIPTION:
 **
 **	 MOM_PROTOTYPES.H
 **
 **	 This header file defines the function prototypes for the [[mom_name]].
 **
 **  AUTHORS:
 **
 **	[[author]]
 **
 **     This code was initially created with the 
 **	[[system]] MOM Generator - version [[version]]
 **
 **  CREATION DATE:  [[creation_date]]
 **
 **  MODIFICATION HISTORY:
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

man_status check_access PROTOTYPE((
	object_id   *,
	avl	    *,
	avl	    *,
	avl	    *,
	avl	    * ));
                                     

man_status check_access_instance PROTOTYPE((
	int	     ,
	void	    *,
	avl	    * ));

man_status moi_directive PROTOTYPE((
	man_binding_handle ,
	object_id   *,
	avl 	    *,
	scope 	     ,
	avl 	    *,
	avl 	    *,
	object_id   *,
	avl 	    *,
	man_status  (*)(),
	man_status  (*)(),
	int     ,
	management_handle *,
	object_id   * ));

man_status moi_invoke_action PROTOTYPE((   
	man_binding_handle  ,
	object_id   *,
	avl	    *,
	scope	     ,
	avl	    *,
	avl	    *,
	int	     ,
	object_id   *,
	avl	    *,
	int     ,
	management_handle   * ));

man_status  create_instance  PROTOTYPE((
	man_binding_handle  ,
	object_id	    *,
	avl		    *,
	avl		    *,
	avl		    *,
	avl		    *,
	avl		    *,
	int	     ,
	management_handle   *,
	object_id	    *,
	int	     ));

man_status moi_create_instance PROTOTYPE((
	man_binding_handle   ,
	object_id	    *,
	avl		    *,
	avl		    *,
	avl		    *,
	avl		    *,
	avl		    *,
	int	     ,
	management_handle   * ));

man_status delete_instance PROTOTYPE(( 
	man_binding_handle   ,
	object_id	    *,
	avl		    *,
	scope	    	     ,
	avl		    *,
	avl		    *,
	int		     ,
	int	     ,
	management_handle   *,
	object_id	    *,
	avl		    *,
	int		     ));

man_status moi_delete_instance PROTOTYPE(( 
	man_binding_handle   ,
	object_id	    *,
	avl		    *,
	scope		     ,
	avl		    *,
	avl		    *,
	int		     ,
	int	     ,
	management_handle   * ));

man_status moi_get_attributes PROTOTYPE((
	man_binding_handle   ,
	object_id	    *,
	avl		    *,
	scope		     ,
	avl		    *,
	avl		    *,
	int		     ,
	avl		    *,
	int	     ,
	management_handle   * ));

man_status moi_set_attributes PROTOTYPE((
	man_binding_handle   ,
	object_id	    *,
	avl		    *,
	scope		     ,
	avl		    *,
	avl		    *,
	int		     ,
	avl		    *,
	int	     ,
	management_handle   * ));

void map_create_to_action PROTOTYPE((
        int                 * ));

man_status _get_class_code PROTOTYPE((
	object_id 	    *,
	int 		    * ));

man_status add_avl_attribute PROTOTYPE((
	object_id 	    *,
	avl		    *,
	avl 	 	   ** ));

man_status char_to_simple_name  PROTOTYPE((
	char 		    *, 
	int 		     ,
	octet_string 	    * ));

man_status  find_instance_avl  PROTOTYPE((
                        avl                 *,
                        object_id           *,
                        unsigned int   *,
                        unsigned int   *,
                        octet_string        **
                        ));

man_status copy_avl_attribute PROTOTYPE((
	object_id 	    *, 
	avl 		    *,
	avl 		   ** ));

man_status copy_octet_to_unsigned_int PROTOTYPE((
	unsigned int   *,
	octet_string 	    * ));

man_status copy_octet_to_signed_int PROTOTYPE((
	int   	    *,
	octet_string 	    * ));

man_status copy_unsigned_int_to_octet PROTOTYPE((
	octet_string 	    *,
	unsigned int   * ));

man_status copy_signed_int_to_octet PROTOTYPE((
	octet_string 	    *,
	int   	    * ));

man_status remove_avl_attribute PROTOTYPE((
	object_id 	    *,
	avl 		    *,
	avl 		   ** ));

man_status setup_reply PROTOTYPE((
	man_status 	     ,
	reply_type 	    *, 
	object_id  	   **,
	avl	 	    *,
	int	     ,
	int 	     ,
	object_id           * ));

man_status simple_name_to_char  PROTOTYPE((
	octet_string 	     *, 
	char 		    **,
	int 		     * ));

man_status send_error_reply PROTOTYPE((
	management_handle   *,
	man_status  	(*)(),
	int	     ,
	man_status           ,
	object_id	    *,
	avl	    	    *,
	uid	    	    *,
	avl	            *,
	object_id	    * ));

man_status validate_acl PROTOTYPE((
        avl		    *,
	object_id	    *,
	avl 		   **,
	object_id	    * ));
	
man_status moss_free_time PROTOTYPE ((mo_time	    *));
man_status moss_get_uid PROTOTYPE((uid **uid_value));
man_status moss_free_uid PROTOTYPE((uid **uid_value));
man_status  _moss_avl_get_last_name PROTOTYPE ((avl			*,
				    object_id		**,
				    unsigned int   *,
				    unsigned int   *,
				    octet_string	**,
				    int			*));

man_status _reply_required PROTOTYPE((management_handle   *));
man_status _list_error PROTOTYPE((object_id		*,
		      avl		*,
		      unsigned int	*,
		      reply_type	,
		      reply_type	));

int moss_dns_simplename_to_opaque PROTOTYPE (( char *, short int *, char *, short int *));
int moss_dns_opaque_to_simplename PROTOTYPE (( char *, short int *, char *, short int *));

man_status moss_print_dna_avl PROTOTYPE((avl *avl_handle));

man_status _send_error PROTOTYPE((man_binding_handle ,
		      man_status    (*)(),
		      int	    ,
		      man_status    ,
		      object_id	    *,
		      avl	    *,
		      uid	    *,
		      avl	    *,
		      object_id	    *));

man_status _error_exit PROTOTYPE((
		    char       *,
		    man_status  ));

/*-insert-code-define-protos-*/

