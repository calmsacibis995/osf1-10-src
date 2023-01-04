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
static char	*sccsid = "@(#)$RCSfile: kloadcall.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:45:49 $";
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

#include <sys/kloadcall.h>

#define	TRACE	1

char usage_string[] = "\
usage: kloadcall vm_allocate       <address> <size> <anywhere>\n\
       kloadcall vm_deallocate     <address> <size>\n\
       kloadcall vm_read           <address> <size>\n\
       kloadcall vm_write          <address> <data_count>\n\
       kloadcall vm_protect        <address> <size> <set_max> <new_prot>\n\
       kloadcall vm_allocate_wired <address> <size> <prot> <anywhere>\n\
       kloadcall call_function     <address> <arg1> <arg2> <arg3>\n\
";

extern char *sys_errlist[];
extern int errno;

void getarg(char *, int *, char *, char *);
int  number(char *, int *);

main(argc, argv)
	char *argv[];
{
	char *operation;

	argv++; argc--;

	if (argc < 2)
		usage();

	operation = *argv++; argc--;

	if (!strcmp("vm_allocate", operation))
		klc_vm_allocate(argc, argv);
	else if (!strcmp("vm_deallocate", operation))
		klc_vm_deallocate(argc, argv);
	else if (!strcmp("vm_read", operation))
		klc_vm_read(argc, argv);
	else if (!strcmp("vm_write", operation))
		klc_vm_write(argc, argv);
	else if (!strcmp("vm_protect", operation))
		klc_vm_protect(argc, argv);
	else if (!strcmp("vm_allocate_wired", operation))
		klc_vm_allocate_wired(argc, argv);
	else if (!strcmp("call_function", operation))
		klc_call_function(argc, argv);
	else
		usage();
	exit(0);
}

klc_vm_allocate(argc, argv)
	char *argv[];
{
	vm_address_t	address;
	vm_size_t	size;
	boolean_t	anywhere;
	int		rc;

	if (argc < 3)
		usage();

	getarg(argv[0], (int *)&address, "kloadcall", "address");
	getarg(argv[1], (int *)&size, "kloadcall", "size");
	getarg(argv[2], (int *)&anywhere, "kloadcall", "anywhere");

#ifdef	TRACE
	printf("kloadcall: kloadcall(KLC_VM_ALLOCATE, %#x, %#x, %d)",
		address, size, anywhere);
	rc = kloadcall(KLC_VM_ALLOCATE, &address, size, anywhere);
	print_rc(rc);
	printf("kloadcall: address=%#x\n", address);
#else
	(void)kloadcall(KLC_VM_ALLOCATE, &address, size, anywhere);
#endif
}

klc_vm_deallocate(argc, argv)
	char *argv[];
{
	vm_address_t	address;
	vm_size_t	size;
	boolean_t	anywhere;
	int		rc;

	if (argc < 2)
		usage();

	getarg(argv[0], (int *)&address, "kloadcall", "address");
	getarg(argv[1], (int *)&size, "kloadcall", "size");

#ifdef	TRACE
	printf("kloadcall: kloadcall(KLC_VM_DEALLOCATE, %#x, %#x)",
		address, size);
	rc = kloadcall(KLC_VM_DEALLOCATE, address, size);
	print_rc(rc);
#else
	(void)kloadcall(KLC_VM_DEALLOCATE, address, size);
#endif
}

klc_vm_read(argc, argv)
	char *argv[];
{
	vm_address_t	address;
	vm_size_t	size;
	pointer_t	data;
	int		data_count;
	int		*array, i, rc;

	if (argc < 2)
		usage();

	getarg(argv[0], (int *)&address, "kloadcall", "address");
	getarg(argv[1], (int *)&size, "kloadcall", "size");

#ifdef	TRACE
	printf("kloadcall: kloadcall(KLC_VM_READ, %#x, %#x, &data, &data_count)",
		address, size);
	rc = kloadcall(KLC_VM_READ, address, size, &data, &data_count);
	print_rc(rc);
	printf("kloadcall: data=%#x, data_cnt=%#x\n", data, data_count);
#else
	(void)kloadcall(KLC_VM_READ, address, size, &data, &data_count);
#endif

#define	PER_LINE	6
	array = (int *)data;
	size /= 4;
	for (i = 0; i < size; i++)
		if ((i % PER_LINE) == (PER_LINE - 1))
			printf("%#08x\n", array[i]);
		else
			printf("%#08x ", array[i]);
	if (!(i % PER_LINE))
		putchar('\n');
}

klc_vm_write(argc, argv)
	char *argv[];
{
	vm_address_t	address;
	vm_size_t	size;
	pointer_t	data;
	int		data_count;
	unsigned int	*array;
	unsigned int	acnt, aval;
	int		i, rc;

	if (argc < 2)
		usage();

	getarg(argv[0], (int *)&address, "kloadcall", "address");
	getarg(argv[1], &data_count, "kloadcall", "data_count");

	size = (vm_size_t)data_count;
	if ((rc = vm_allocate(task_self(), (vm_address_t *)&array, size,
	    TRUE)) != KERN_SUCCESS) {
		fprintf(stderr, "kloadcall: vm_allocate(task_self(), (vm_address_t *)&array, %#x, TRUE) failed: %s\n",
			argv[0], size, mach_error_string(rc));
		exit(1);
	}

	acnt = size / 4;
	aval = (unsigned int)address;
	for (i = 0; i < acnt; i++) {
		array[i] = aval;
		aval += 4;
	}

	data = (pointer_t)array;
#ifdef	TRACE
	printf("kloadcall: kloadcall(KLC_VM_WRITE, %#x, %#x, %#x)",
		address, data, data_count);
	rc = kloadcall(KLC_VM_WRITE, address, data, data_count);
	print_rc(rc);
#else
	(void)kloadcall(KLC_VM_WRITE, address, data, data_count);
#endif

#ifdef	notdef
	printf("looping ...");
	fflush(stdout);
	while (1)
		;
#endif
}

klc_vm_protect(argc, argv)
	char *argv[];
{
	vm_address_t	address;
	vm_size_t	size;
	boolean_t	set_max;
	vm_prot_t	new_prot;
	int		rc;

	if (argc < 4)
		usage();

	getarg(argv[0], (int *)&address, "kloadcall", "address");
	getarg(argv[1], (int *)&size, "kloadcall", "size");
	getarg(argv[2], (int *)&set_max, "kloadcall", "set_max");
	getarg(argv[3], (int *)&new_prot, "kloadcall", "new_prot");

#ifdef	TRACE
	printf("kloadcall: kloadcall(KLC_VM_PROTECT, %#x, %#x, %d, %d)",
		address, size, set_max, new_prot);
	rc = kloadcall(KLC_VM_PROTECT, address, size, set_max, new_prot);
	print_rc(rc);
#else
	(void)kloadcall(KLC_VM_PROTECT, address, size, set_max, new_prot);
#endif
}

klc_vm_allocate_wired(argc, argv)
	char *argv[];
{
	vm_address_t	address;
	vm_size_t	size;
	vm_prot_t	prot;
	boolean_t	anywhere;
	int		rc;

	if (argc < 4)
		usage();

	getarg(argv[0], (int *)&address, "kloadcall", "address");
	getarg(argv[1], (int *)&size, "kloadcall", "size");
	getarg(argv[2], (int *)&prot, "kloadcall", "prot");
	getarg(argv[3], (int *)&anywhere, "kloadcall", "anywhere");

#ifdef	TRACE
	printf("kloadcall: kloadcall(KLC_VM_ALLOCATE_WIRED, %#x, %#x, %d, %d)",
		address, size, prot, anywhere);
	rc = kloadcall(KLC_VM_ALLOCATE_WIRED, &address, size, prot, anywhere);
	print_rc(rc);
	printf("kloadcall: address=%#x\n", address);
#else
	(void)kloadcall(KLC_VM_ALLOCATE_WIRED, &address, size, prot, anywhere);
#endif
}

klc_call_function(argc, argv)
	char *argv[];
{
	vm_address_t	address;
	int		arg1, arg2, arg3;
	int		rc;

	if (argc < 4)
		usage();

	getarg(argv[0], (int *)&address, "kloadcall", "address");
	getarg(argv[1], &arg1, "kloadcall", "arg1");
	getarg(argv[2], &arg2, "kloadcall", "arg2");
	getarg(argv[3], &arg3, "kloadcall", "arg3");

#ifdef	TRACE
	printf("kloadcall: kloadcall(KLC_CALL_FUNCTION, %#x, %#x, %#x, %#x)",
		address, arg1, arg2, arg3);
	rc = kloadcall(KLC_CALL_FUNCTION, address, arg1, arg2, arg3);
	printf(" = %#x\n", rc);
#else
	(void)kloadcall(KLC_CALL_FUNCTION, address, arg1, arg2, arg3);
#endif
}

#ifdef	TRACE
print_rc(rc)
{
	if (rc == -1)
		printf(" = %s\n", sys_errlist[errno]);
	else
		printf(" = %s\n", mach_error_string(rc));
}
#endif

usage()
{
	fprintf(stderr, "%s", usage_string);
	exit(1);
}

void
getarg(cp, ip, program, name)
	char *cp, *program, *name;
	int *ip;
{
	if (number(cp, ip)) {
		fprintf(stderr, "%s: bad %s\n", program, name);
		exit(1);
	}
}

/*
 * number() - Ascii (Hex, Octal, or Decimal) to Number
 */
int
number(cp, ip)
	char *cp;
	int *ip;
{
	int c, i;

	i = 0;
	if ((cp[0] == '0') && ((cp[1] == 'x') || (cp[1] == 'X')))
		for (cp = &cp[2]; c = *cp; cp++)
			if (('0' <= c) && (c <= '9'))
				i  = (i*16) + c - '0';
			else if (('a' <= c) && (c <= 'f'))
				i = (i*16) + c - 'a' + 10;
			else if (('A' <= c) && (c <= 'F'))
				i = (i*16) + c - 'A' + 10;
			else
				return(-1);
	else if (cp[0] == '0')
		for (cp = &cp[1]; c = *cp; cp++)
			if (('0' <= c) && (c <= '7'))
				i  = (i*8) + c - '0';
			else
				return(-1);
	else
		for (; c = *cp; cp++)
			if (('0' <= c) && (c <= '9'))
				i  = (i*10) + c - '0';
			else
				return(-1);
	*ip = i;
	return(0);
}
