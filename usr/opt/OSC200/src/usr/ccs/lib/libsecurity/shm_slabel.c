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
static char	*sccsid = "@(#)$RCSfile: shm_slabel.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:18:32 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988 SecureWare, Inc.
 * All rights reserved.
 */




#include <sys/secdefines.h>

#if SEC_MAC

#include <sys/types.h>
#include <sys/errno.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/secpolicy.h>
#include <mandatory.h>

extern int errno;
extern int sec_errno;

shm_statslabel (shmid, slabel)
	int	shmid;
	mand_ir_t *slabel;
{
	obj_t	obj;
	attr_t	attr;
	int	ret;

	/* Make sure the mand_config structure is initialized */
	if (mand_init () != 0) {
		errno = EINVAL;
		sec_errno = ESEC_MAC_CONFIG_FAILURE;
		return -1;
	}

	attr.ir = (char *) slabel;
	attr.ir_length = mand_bytes();

	obj.o_shmid = shmid;

	ret = getlabel(mand_config.policy, MAND_OBJ_SL_TAG, &attr,
			OT_SHARED_MEMORY, &obj);

	if (ret == 0 && attr.code == SEC_WILDCARD_TAG) {
		ret = -1;
		errno = EINVAL;
		sec_errno = ESEC_WILDCARD_TAG;
	}

	return ret;
}


shm_chslabel (shmid, slabel)
	int	shmid;
	mand_ir_t *slabel;
{
	obj_t	obj;
	attr_t	attr;

	/* Make sure the mand_config structure is initialized */
	if (mand_init () != 0) {
		errno = EINVAL;
		sec_errno = ESEC_MAC_CONFIG_FAILURE;
		return -1;
	}

	if (slabel == (mand_ir_t *) 0) {
		attr.code = SEC_WILDCARD_TAG;
		attr.ir = (char *) 0;
		attr.ir_length = 0;
	} else {
		attr.code = SEC_ACTUAL_TAG;
		attr.ir = (char *) slabel;
		attr.ir_length = mand_bytes();
	}

	obj.o_shmid = shmid;

	return setlabel(mand_config.policy, MAND_OBJ_SL_TAG, &attr,
			OT_SHARED_MEMORY, &obj);
}
#endif
