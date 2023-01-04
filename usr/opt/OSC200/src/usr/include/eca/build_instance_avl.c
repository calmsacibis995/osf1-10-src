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
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]build_instance_avl 
**
**	This routine builds an avl containing the entire object instance
**      name given the short hand instance name as a zero terminated string. 
**
**	Example: If object_instance contains "HELENA <null> **", and
**		 specific_instance pointed to the "test" instance. the
**		 specific_object_instance returned would be "HELENA <null> test" 
**		 
**  FORMAL PARAMETERS:
**
**	object_instance - AVL containing the object instance. The assumption is
**			  that the last octet of the object_instance avl is a 
**			  wild card and can be removed and replaced with the
**			  instance name forming the full instance name.
**	 
**	specific_object_instance - Address of pointer to the returned object_instance avl. 
**	 
**	specific_instance - The pointer to the instance data.
**
**  RETURN VALUE:
**
**      MAN_C_SUCCESS 		 - if success
**	Any status returned from:
**	    moss_avl_init
**	    moss_avl_reset
**	    moss_avl_find_item
**	    moss_avl_backup
**	    moss_avl_remove
**--
*/
static man_status [[class_prefix]]build_instance_avl( object_instance, 
						      specific_object_instance,
			        		      specific_instance )

avl *object_instance;
avl **specific_object_instance;
[[class_void_name]] *specific_instance;

{
  unsigned int  	modifier;
  unsigned int  	tag;
  octet_string 		*junk_octet;
  octet_string 		octet;
  man_status		status;
  int			last = FALSE;

  status = moss_avl_init(specific_object_instance);
  if ERROR_CONDITION( status )
    return status;
  
  status = moss_avl_reset(object_instance);
  if ERROR_CONDITION( status )
    return status;
  
  /* 
   *  Copy the entire object_instance avl.
   */ 
  do {
    status = moss_avl_copy(*specific_object_instance,	/* avl_handle_dest */
			   object_instance,		/* avl_handle_source */
			   FALSE,			/* reset_dest */
			   NULL,			/* dest_oid */
			   NULL,			/* dest_modifier */
			   &last);			/* source_last_one */
    
  } while ( (status == MAN_C_SUCCESS) && (last == FALSE));
    
  /*
   * Move down the avl to the [[class_name]]'s name, backup one octet,
   * remove it and replace it with the instance name. Note, we store the
   * instance name as a simple zero-terminated string, but we must return it as
   * a SimpleName which as two extra bytes.
   *
   * NOTE: DNA CMIP requires the number of the identifier attribute ID to be 1.
   */
  status = moss_avl_find_item( *specific_object_instance,
			       /*-insert-code-name-oid-*/
			       &modifier,
			       &tag,
			       &junk_octet);
  if ERROR_CONDITION( status )
    {
#ifdef MOMGENDEBUG
    printf("\n*** Identifier attribute not found in object instance. For DNA_CMIP, identifier attr must be 1. *** \n");
#endif /* MOMGENDEBUG */
    return status;
    }  

  status = moss_avl_backup(*specific_object_instance);
  if ERROR_CONDITION( status )
    return status;
  
  status = moss_avl_remove(*specific_object_instance);
  if ERROR_CONDITION( status )
    return status;

  /*-insert-code-build-avl-type-*/

  if ERROR_CONDITION( status )
    return status;
  
#ifdef MOMGENDEBUG
  printf("------- Object instance from BUILD_INSTANCE_AVL --------- \n\n");
  dbg_print_avl (*specific_object_instance);
  printf("------- end of instance from BUILD_INSTANCE_AVL --------- \n\n");
#endif /* MOMGENDEBUG */

return MAN_C_SUCCESS;
} /* End of [[class_prefix]]build_instance_avl() */

/* End of [[class_prefix]]build_get_instance.c */
