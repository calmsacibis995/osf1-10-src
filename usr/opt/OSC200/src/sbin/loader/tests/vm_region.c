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
static char	*sccsid = "@(#)$RCSfile: vm_region.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:46:42 $";
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

#include <stdio.h>
#include <mach.h>

main(argc, argv)
	char *argv[];
{
	task_t		task;
	vm_address_t	address, end;
	vm_size_t	size;
	vm_prot_t	protection, max_protection;
	vm_inherit_t	inheritance;
	boolean_t	shared;
	port_t		object_name;
	vm_offset_t	offset;
	int		rc, first = 1;
	int		pid;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <process-id>\n", argv[0]);
		exit(1);
	}

	pid = atoi(argv[1]);

	if ((rc = task_by_unix_pid(task_self(), pid, &task)) != KERN_SUCCESS) {
		fprintf(stderr, "%s: task_by_unix_pid(task_self(), %d, &task) failed: %s\n",
			argv[0], pid, mach_error_string(rc));
		exit(1);
	}

	for (address = (vm_address_t)0; 
	  vm_region(task, &address, &size, &protection, &max_protection,
	    &inheritance, &shared, &object_name, &offset) == KERN_SUCCESS;
	  address += size) {

		if (first)
			first = 0;
		else
			if (address != end)
				printf("*                     %#10x\n", address-end);
		end = address+size;
		printf("%#10x %#10x %#10x ", address, end, size);
		if (protection == (vm_prot_t)0)
			printf("none");
		else {
			if (protection & VM_PROT_READ)
				putchar('r');
			if (protection & VM_PROT_WRITE)
				putchar('w');
			if (protection & VM_PROT_EXECUTE)
				putchar('x');
		}
#ifdef	INHERIT
		putchar(' ');
		if (inheritance == VM_INHERIT_SHARE)
			printf("share");
		else if (inheritance == VM_INHERIT_COPY)
			printf("copy");
		else if (inheritance == VM_INHERIT_NONE)
			printf("none");
#endif
		putchar('\n');
	}
	exit(0);
}
