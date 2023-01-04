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
static char *rcsid = "@(#)$RCSfile: kern_sysfs.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/03 22:59:25 $";
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/fstyp.h>
#include <sys/fsid.h>
#include <sys/mount.h>


static char *fs_names[MOUNT_MAXTYPE+1];
static boolean_t fsname_setup = FALSE;


/*
 * Function Name:
 *	sysv_sysfs()
 *
 * Description:
 *	Implements SVR4 sysfs() system call.
 *
 * Inputs:
 *	current proc pointer
 *	system call args
 *	pointer to system call return value array
 *
 * Outputs:
 *	None
 *
 * Return value:
 *
 * Called Functions:
 *	minor kernel functions
 *
 * Called by:
 *	syscall()
 *
 * Side effects:
 *	None
 *	
 * Notes:
 *
 * Dependencies:
 *
 */

sysfs(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
#ifdef	__alpha
		long opcode;
#else	/* __alpha */
		int opcode;
#endif	/* __alpha */
		char *fsname;
		char *buf;
	} *uap = (struct args *) args;
#define	uap_fs_index	(int)(uap->fsname)

	int i, error;
	char fsstring[FSTYPSZ];


	/*
	 * Note that no name should be longer than 16 chars long
	 * if the SVR4 sysfs() system call is to work correctly.
	 * This value is defined in svr4/fstyp.h
	 *
	 * Setting up the array of file system names is performed
	 * at run time to avoid being out of sync with the values
	 * for MOUNT_UFS, etc., which are defined in sys/mount.h
	 * Really, it might be better to declare an array of names
	 * in mount.h, but we're trying not to touch OSF/1 code 
	 * unnecessarily.  We could also have just hard coded mappings 
	 * between strings and indexes, but this isolates the mappings
	 * to one place, so it's easy to find where to add new file
	 * system types.
	 *
	 * If dynamic addition of file system types is allowed, then
	 * perhaps there should be a way to dynamically added file system
	 * type strings to some global array of such strings...
	 */
	if( fsname_setup == FALSE ) {
		for(i=0; i<= MOUNT_MAXTYPE; i++ ) {
			fs_names[i] = NULL;
		}
		if(MOUNT_NONE <= MOUNT_MAXTYPE) fs_names[MOUNT_NONE] = NULL;
		if(MOUNT_UFS <= MOUNT_MAXTYPE)  fs_names[MOUNT_UFS] = UFS_STRING;
		if(MOUNT_NFS <= MOUNT_MAXTYPE)  fs_names[MOUNT_NFS] = NFS;
		if(MOUNT_MFS <= MOUNT_MAXTYPE)  fs_names[MOUNT_MFS] = MFS;
		if(MOUNT_PC <= MOUNT_MAXTYPE)   fs_names[MOUNT_PC] = PCFS;
		if(MOUNT_S5FS <= MOUNT_MAXTYPE) fs_names[MOUNT_S5FS] = S5FS;
		if(MOUNT_CDFS <= MOUNT_MAXTYPE) fs_names[MOUNT_CDFS] = CDFS;
		if(MOUNT_FDFS <= MOUNT_MAXTYPE) fs_names[MOUNT_FDFS] = FDFS;
		if(MOUNT_PROCFS <= MOUNT_MAXTYPE)  
						fs_names[MOUNT_PROCFS] = PROCFS;
		if(MOUNT_DFS <= MOUNT_MAXTYPE)  fs_names[MOUNT_DFS] = DFS;
		if(MOUNT_EFS <= MOUNT_MAXTYPE)  fs_names[MOUNT_EFS] = EFS;
		if(MOUNT_MSFS <= MOUNT_MAXTYPE) fs_names[MOUNT_MSFS] = MSFS;
		fsname_setup = TRUE;
	};

	switch(uap->opcode) {

	case GETFSIND:
		/* translate fs string to fs index */

		if( error = copyin((caddr_t)uap->fsname, fsstring, 
		  sizeof(fsstring)) ) {
			return(error);
		}

		/* make sure copied in name ends with a null */
		for(i = 0; i < sizeof(fsstring); i++) { 
			if( fsstring[i] == 0 )
				break;
		}
		/* name too long or name too short */
		if( i == sizeof(fsstring) || i == 0 ) {
			return(EINVAL);
		}

		/*
		 * get index for this name
		 *
		 * Note that the fs_name for file system type MOUNT_NONE 
		 * is set to NULL, along with any other new file system 
		 * types that weren't been added to the fs_names table 
		 * when they were added to the system.
		 *
		 * Any file system index where the name is set to NULL can
		 * never be matched, so it's not possible to get the file
		 * system type index until this string is added to the
		 * fs_names table.  MOUNT_NONE, of course, should always
		 * remain NULL, and should never be matched.
		 */
		for(i = 0; i<=MOUNT_MAXTYPE; i++) {
			if ( fs_names[i] && !strcmp(fsstring,fs_names[i]) ) {
				break;
			}
		}
		if ( i > MOUNT_MAXTYPE ||
		    vfssw[i] == (struct vfsops *)0) {
			return(EINVAL);
		}

		/* only get here if found valid matching index */
		*retval = i;
		break;


	case GETFSTYP:
		/* translate fs index to fs string */

		if (uap_fs_index > MOUNT_MAXTYPE || uap_fs_index < 0 ||
		    vfssw[uap_fs_index] == (struct vfsops *)0) {
			return(EINVAL);
		}

		if( fs_names[uap_fs_index] == NULL ) {
			return ENOSYS;
		}

		if( (i=strlen(fs_names[uap_fs_index])) >= FSTYPSZ) {
			return(ENOSYS);
		}
		
		if( error = copyout( fs_names[uap_fs_index], uap->buf, i+1 ) )
			return( error );

		*retval = 0;
		break;


	case GETNFSTYP:
		/* return valid number of file system types */

		/* if new file system types can be added dynamically, this
		 * should really be some dynamic number that holds the
		 * count of file system types.
		 */
		*retval = MOUNT_MAXTYPE;
		break;


	default:
		return(EINVAL);
		break;
	}


	return(KERN_SUCCESS);
}
