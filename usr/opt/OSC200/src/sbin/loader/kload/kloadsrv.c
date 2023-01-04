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
static char	*sccsid = "@(#)$RCSfile: kloadsrv.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:38:16 $";
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
 * file
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */

/*
 * kloadsrv - the kernel load server
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <loader.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#include <loader/kloadsrv.h>

#include "ldr_errno.h"
#include "ldr_types.h"
#include "ldr_lock.h"
#include "kernel_loader.h"
#include "kls_message.h"
#include "kls_ipc.h"

#define	SIG_INIT_SUCCESS	SIGUSR1

static char usage_string[] =
"usage: %s [-f] [-d <debug-level>] [-p <package-name>] [<kernel-object-filename>]\n\
\n\
       -f  remain in foreground\n\
       -d  specify debug level\n\
       -p  specify default kernel package name\n";

char *kernel =			 "/vmunix";
char *kls_default_package_name = "kernel";
int   kls_debug_level;
int   foreground;

void main_initialize_success(int);
void main_terminate(int);
void main_usage(void);

struct sigaction init_sigaction = {
	main_initialize_success,
	(sigset_t)0,
	0
};

struct sigaction term_sigaction = {
	main_terminate,
	(sigset_t)0,
	0
};

struct sigaction ignore_sigaction = {
	SIG_IGN,
	(sigset_t)0,
	0
};

char *argv0;

#ifdef LDR_STATIC_LINK

/*
 * NOTE: following six declarations are only needed while kloadsrv is
 * bound with loader.  Once loader becomes dynamic, these will be
 * unnecessary.
 */

void *ldr_process_context;
const char *ldr_global_data_file = "/etc/ldr_global.dat";
const char *ldr_dyn_database = "/tmp/ldr_dyn_mgr.conf";
/* structure to save locking functions used by loader */
lib_lock_functions_t	ldr_lock_funcs;
/* This is the only loader lock for this process */
ldr_lock_t ldr_global_lock;

extern int ldr_bootstrap(const char *, void **);
#endif /* LDR_STATIC_LINK */

extern void kls_server_initialize(void);
extern void kls_server_terminate(void);
extern void kls_server_process_request(kls_request_header_t *);

extern void kls_server_unknown_request(kls_request_header_t *);
extern void kls_server_load(kls_request_header_t *);
extern void kls_server_unload(kls_request_header_t *);
extern void kls_server_entry(kls_request_header_t *);
extern void kls_server_lookup(kls_request_header_t *);
extern void kls_server_lookup_package(kls_request_header_t *);
extern void kls_server_next_module(kls_request_header_t *);
extern void kls_server_inq_module(kls_request_header_t *);
extern void kls_server_inq_region(kls_request_header_t *);

extern char *optarg;
extern int   optind;

main(argc, argv)
	char *argv[];
{
	kls_request_header_t *p;
	int c;

	argv0 = argv[0];

	while ((c = getopt(argc, argv, "fd:p:")) != EOF) {
		switch (c) {

		case 'f':
			foreground++;
			break;

		case 'd':
			kls_debug_level = atoi(optarg);
			break;

		case 'p':
			kls_default_package_name = optarg;
			break;

		case '?':
			main_usage();
			/*NOTREACHED*/
		}
	}

	if (optind < argc)
		kernel = argv[optind];

	/* initialize server */
	kls_server_initialize();

	/* server loop */
	while (TRUE) {
		if (kls_server_ipc_receive_request(&p) < 0)
			continue;
		(void)kls_server_process_request(p);		
	}
}

void
main_usage()
{
	(void)fprintf(stderr, usage_string, KLS_SERVER_NAME);
	exit(1);
}
			
void
main_initialize_success(sig)
{
	exit(0);
}

void
main_terminate(sig)
{
	kls_server_terminate();
}

void
kls_server_initialize()
{
	int  rc, status;
	long pid;

#ifdef	notdef
	/* make sure we are running as root */
	if (getuid() && geteuid()) {
		fprintf(stderr, "%s: %s must be run as root\n",
			KLS_SERVER_NAME, KLS_SERVER_NAME);
		exit(1);
	}
#endif

	/* put ourself into the background */
	if (!foreground) {
		if ((pid = fork()) == -1) {
			(void)fprintf(stderr, "%s: fork() to put server in background failed: %s\n",
				KLS_SERVER_NAME, strerror(errno));
			exit(1);
		} else if (pid != 0) {
			/*
			 * parent
			 *
			 * The parent waits in case the child fails, in
			 * which case the parent propogates the exit status.
			 * If the child succeeds, it will send the parent
			 * a SIG_INIT_SUCCESS signal and the parent will
			 * simply exit(0).  See the signal handler in
			 * init_sigaction.
			 */

#ifdef LDR_STATIC_LINK
			/* this call is only needed while running bound with loader */
			if ((rc = ldr_bootstrap(argv0, &ldr_process_context)) != LDR_SUCCESS) {
				fprintf(stderr, "%s: loader_bootstrap(\"%s\") failed, return code %d: %s\n",
					KLS_SERVER_NAME, argv0, rc, strerror(errno));
				exit(1);
			}
#endif /* LDR_STATIC_LINK */

			(void)sigaction(SIG_INIT_SUCCESS, &init_sigaction,
			    (struct sigaction *)0);
			(void)wait(&status);
			if (!WIFEXITED(status))
				exit(0);
			if (WIFSIGNALED(status))
				exit(1);
			exit(WEXITSTATUS(status));
		}	
	}

	/* make sure server not already running */
	if (pid_file_open() < 0)
		exit(1);
	if (pid_file_lock() < 0) {
		(void)fprintf(stderr, "%s: kernel load server already running\n",
			KLS_SERVER_NAME);
		exit(1);
	}

#ifdef LDR_STATIC_LINK
	/* this call is only needed while running bound with loader */
	if ((rc = ldr_bootstrap(argv0, &ldr_process_context)) != LDR_SUCCESS) {
		fprintf(stderr, "%s: loader_bootstrap(\"%s\") failed, return code %d: %s\n",
			KLS_SERVER_NAME, argv0, rc, strerror(errno));
		exit(1);
	}
#endif /* LDR_STATIC_LINK */

	/* initialize the kernel context */
	if ((rc = ldr_kernel_bootstrap(kernel)) != LDR_SUCCESS) {
		fprintf(stderr, "%s: ldr_kernel_bootstrap(\"%s\") failed, return code %d: %s\n",
			KLS_SERVER_NAME, kernel, rc, strerror(errno));
		exit(1);
	}

	/* intialize the server */
	if ((rc = kls_server_ipc_initialize()) < 0) {
		(void)fprintf(stderr, "%s: kls_server_ipc_initialize() failed, return code %d: %s\n",
			KLS_SERVER_NAME, rc, strerror(errno));
		exit(1);
	}

	/* log our PID */
	if (pid_file_write() < 0)
		exit(1);
	(void)pid_file_lock();	/* file may now be longer */

	/* set up signal handlers */
	(void)sigaction(SIGTERM, &term_sigaction, (struct sigaction *)0);
	if (foreground) {
		(void)sigaction(SIGINT, &term_sigaction,
		    (struct sigaction *)0);
		(void)sigaction(SIGQUIT, &term_sigaction,
		    (struct sigaction *)0);
	} else {
		(void)sigaction(SIGINT, &ignore_sigaction,
		    (struct sigaction *)0);
		(void)sigaction(SIGQUIT, &ignore_sigaction,
		    (struct sigaction *)0);
	}

	/* new session and tell parent initialization was successful */
	if (!foreground) {
		(void)setsid();
		(void)kill(getppid(), SIG_INIT_SUCCESS);
	}
}

void
kls_server_terminate()
{
	kls_server_ipc_terminate();
	(void)pid_file_close();
	(void)pid_file_unlink();
	exit(0);
}

void
kls_server_process_request(request)
	kls_request_header_t *request;
{
	switch (request->klsi_msg_type) {

	default:
		kls_server_unknown_request(request);
		break;

	case KLS_LOAD_REQUEST:
		kls_server_load(request);
		break;

	case KLS_UNLOAD_REQUEST:
		kls_server_unload(request);
		break;

	case KLS_ENTRY_REQUEST:
		kls_server_entry(request);
		break;

	case KLS_LOOKUP_REQUEST:
		kls_server_lookup(request);
		break;

	case KLS_LOOKUP_PACKAGE_REQUEST:
		kls_server_lookup_package(request);
		break;

	case KLS_NEXT_MODULE_REQUEST:
		kls_server_next_module(request);
		break;

	case KLS_INQ_MODULE_REQUEST:
		kls_server_inq_module(request);
		break;

	case KLS_INQ_REGION_REQUEST:
		kls_server_inq_region(request);
		break;
	}
}

void
kls_server_unknown_request(h)
	kls_request_header_t *h;
{
	kls_reply_header_t *reply;
	int rc;

	(void)fprintf(stderr, "%s: received message of unknown type, %d\n",
		KLS_SERVER_NAME, h->klsi_msg_type);
	rc = LDR_EINVAL;
	if ((rc = kls_message_create_unknown_reply(rc, &reply)) < 0)
		return;
	kls_server_ipc_send_reply(h, reply);
}

void
kls_server_load(h)
	kls_request_header_t *h;
{
	kls_load_request_t *request;
	kls_reply_header_t *reply;
	ldr_module_t module;
	int rc;

	request = (kls_load_request_t *)h;
	rc = kernel_load(request->kls_file_pathname, request->kls_load_flags,
		&module);
	if ((rc = kls_message_create_load_reply(rc, module, &reply)) < 0)
		return;
	kls_server_ipc_send_reply(h, reply);
}

void
kls_server_unload(h)
	kls_request_header_t *h;
{
	kls_unload_request_t *request;
	kls_reply_header_t *reply;
	int rc;

	request = (kls_unload_request_t *)h;
	rc = kernel_unload(request->kls_module);
	if ((rc = kls_message_create_unload_reply(rc, &reply)) < 0)
		return;
	kls_server_ipc_send_reply(h, reply);
}

void
kls_server_entry(h)
	kls_request_header_t *h;
{
	kls_entry_request_t *request;
	kls_reply_header_t *reply;
	ldr_entry_pt_t entry;
	int rc;

	request = (kls_entry_request_t *)h;
	rc = kernel_entry(request->kls_module, &entry);
	if ((rc = kls_message_create_entry_reply(rc, entry, &reply)) < 0)
		return;
	kls_server_ipc_send_reply(h, reply);
}

void
kls_server_lookup(h)
	kls_request_header_t *h;
{
	kls_lookup_request_t *request;
	kls_reply_header_t *reply;
	void *symbol_addr;
	int rc;

	request = (kls_lookup_request_t *)h;
	rc = kernel_lookup(request->kls_module, request->kls_symbol_name, &symbol_addr);
	if ((rc = kls_message_create_lookup_reply(rc, symbol_addr, &reply)) < 0)
		return;
	kls_server_ipc_send_reply(h, reply);
}

void
kls_server_lookup_package(h)
	kls_request_header_t *h;
{
	kls_lookup_package_request_t *request;
	kls_reply_header_t *reply;
	char *package_name;
	char *symbol_name;
	void *symbol_addr;
	int rc;

	request = (kls_lookup_package_request_t *)h;
	package_name = request->kls_strings + request->kls_package_name_offset;
	symbol_name = request->kls_strings + request->kls_symbol_name_offset;
	rc = kernel_lookup_package(package_name, symbol_name, &symbol_addr);
	if ((rc = kls_message_create_lookup_package_reply(rc, symbol_addr, &reply)) < 0)
		return;
	kls_server_ipc_send_reply(h, reply);
}

void
kls_server_next_module(h)
	kls_request_header_t *h;
{
	kls_next_module_request_t *request;
	kls_reply_header_t *reply;
	ldr_module_t module;
	int rc;

	request = (kls_next_module_request_t *)h;
	module = request->kls_module;
	rc = kernel_next_module(&module);
	if ((rc = kls_message_create_next_module_reply(rc, module, &reply)) < 0)
		return;
	kls_server_ipc_send_reply(h, reply);
}

void
kls_server_inq_module(h)
	kls_request_header_t *h;
{
	kls_inq_module_request_t *request;
	kls_reply_header_t *reply;
	ldr_module_info_t info;
	size_t ret_size;
	int rc;

	request = (kls_inq_module_request_t *)h;
	rc = kernel_inq_module(request->kls_module, &info, sizeof(info), &ret_size);
	if ((rc = kls_message_create_inq_module_reply(rc, &info, &reply)) < 0)
		return;
	kls_server_ipc_send_reply(h, reply);
}

void
kls_server_inq_region(h)
	kls_request_header_t *h;
{
	kls_inq_region_request_t *request;
	kls_reply_header_t *reply;
	ldr_region_info_t info;
	size_t ret_size;
	int rc;

	request = (kls_inq_region_request_t *)h;
	rc = kernel_inq_region(request->kls_module, request->kls_region, &info, sizeof(info), &ret_size);
	if ((rc = kls_message_create_inq_region_reply(rc, &info, &reply)) < 0)
		return;
	kls_server_ipc_send_reply(h, reply);
}


/*********************** PID File Abstraction ***********************/
static int pid_file_fd = -1;

int
pid_file_open()
{
	if ((pid_file_fd = open(KLS_SERVER_PID_PATHNAME, (O_CREAT | O_WRONLY),
	    0644)) == -1) {
		fprintf(stderr, "%s: cannot open PID file \"%s\": %s\n",
			KLS_SERVER_NAME, KLS_SERVER_PID_PATHNAME,
			strerror(errno));
		return(-errno);
	}
	return(0);
}

int
pid_file_lock()
{
	struct flock flock;

	flock.l_type = F_WRLCK;
	flock.l_whence = SEEK_SET;
	flock.l_start = 0;
	flock.l_len = 0;

	if (fcntl(pid_file_fd, F_SETLK, &flock) == -1)
		return(-errno);
	return(0);
}

int
pid_file_write()
{
	char pid[64];
	int  rc;

	(void)ftruncate(pid_file_fd, 0);
	(void)sprintf(pid, "%d\n", getpid());
	if (write(pid_file_fd, pid, strlen(pid)) == -1) {
		rc = -errno;
		fprintf(stderr, "%s: cannot write PID to PID file \"%s\": %s\n",
			KLS_SERVER_NAME, KLS_SERVER_PID_PATHNAME,
			strerror(errno));
		(void)pid_file_close();
		return(rc);
	}
	return(0);
}

int
pid_file_close()
{
	if (close(pid_file_fd) == -1)
		return(-errno);
	return(0);
}

int
pid_file_unlink()
{
	if (unlink(KLS_SERVER_PID_PATHNAME) == -1)
		return(-errno);
	return(0);
}
