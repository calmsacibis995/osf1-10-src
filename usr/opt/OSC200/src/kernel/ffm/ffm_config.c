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
static char *rcsid = "@(#)$RCSfile: ffm_config.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/12 19:27:40 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */

#include <ffm_fs.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>

extern  struct vfsops ffm_vfsops;


/*
 * ROUTINE:	ffm_configure()
 *
 * FUNCTION:	Ffs File System - Configuration entry point
 *
 * ALGORITH:	Branch on valid operations, else immediately return error.
 * 		Configure:
 *		1. Call VFS framework vfssw_add() routine to register us.
 *		   Note: If successful, it will have called our ffm_init()).
 *		   Note: Will fail if ffm is already registered.
 * 		Deconfigure:
 *		1. Call VFS framework vfssw_del() routine to deregister us.
 *		   Note:  Will fail if still referenced by mounts.
 *		   Note: Will fail if ffm is not already registered.
 *		2. Deallocate allocated memory.
 *
 * RETURNS:	0 if operation successful. 
 *		Otherwise, a non-zero error is returned.
 */

int
ffm_configure( op, indata, indatalen, outdata, outdatalen )
	sysconfig_op_t		op;
	char		* 	indata;
	size_t			indatalen;
	char		* 	outdata;
	size_t			outdatalen;
{
        static config_attr_t ffm_attr[] = {
        {SUBSYS_NAME, STRTYPE, (caddr_t)"ffm", CONSTANT, {0, NULL}},
        };
#define NUM_FFS_ATTRS sizeof(ffm_attr)/sizeof(config_attr_t)
        int             size;
        int             ret = 0;
        struct subsystem_info info;
        short           configured;

        strcpy(info.subsystem_name, "ffm");
        configured = (!subsys_reg(SUBSYS_GET_INFO, &info) && info.config_flag);
        if ((configured && (op == SYSCONFIG_CONFIGURE)) ||
            (!configured && (op != SYSCONFIG_CONFIGURE)))
            return(EALREADY);


        switch (op) {
		case SYSCONFIG_CONFIGURE:
                        ret = vfssw_add(MOUNT_FFS, &ffm_vfsops);
			break;

                case SYSCONFIG_QUERYSIZE:
                        if (outdatalen >= sizeof(int)) {
                                *(int *)outdata = do_querysize(ffm_attr,NUM_FFS_ATTRS);
                                ret = 0;
                        } else {
                                ret = ENOMEM;
                        }
                        break;

                case SYSCONFIG_QUERY:
                        size = do_querysize(ffm_attr,NUM_FFS_ATTRS);
                        if (outdatalen < size)
                                ret = ENOMEM;
                        ret = do_query(ffm_attr,NUM_FFS_ATTRS,outdata,outdatalen);
                        break;

                case SYSCONFIG_RECONFIGURE:
                        ret = EINVAL;
                        break;

		case SYSCONFIG_UNCONFIGURE:
                        ret = EINVAL;
			break;
                        
		default:
			ret = EINVAL;
			break;
        }

        return(ret);
}

