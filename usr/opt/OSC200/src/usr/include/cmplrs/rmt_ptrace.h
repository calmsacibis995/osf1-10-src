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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header */
#ifndef __rmt_ptrace_h
#define __rmt_ptrace_h

#define PT_DBREAD	14
#define PT_DBWRITE	15
#define PT_DHREAD	16
#define PT_DHWRITE	17
#define PT_QUERY	18

enum ptrace_mode_t {host_debug, sproc_debug, sable_debug, pixie_debug,
	prom_debug};


/* some of the prototypes are commented out because they refer to
   typedef names that are local to dbx */
/* the ptrace prototype below is not as type strong as it could be,
   because "address" is not always an address */

/* ptrace.c functions */
unsigned long ptrace_mux ( int req, int pid, unsigned long addr, unsigned long data );
void set_ptrace_mode( enum ptrace_mode_t pmode );
enum ptrace_mode_t get_ptrace_mode( void );
void set_ptrace_bp_op( unsigned int bp_op );
unsigned get_ptrace_bp_op( void );
void set_debug_ptrace( int on_off );
int is_debug_ptrace( void );
void set_debug_comm( int on_off );
int is_debug_comm( void );
void set_debug_protocol( int on_off );
int is_debug_protocol( void );

/* dbgmon functions */
void prom_init( void );
int prom_ptrace ();
int prom_RdRespone ( int chrequest, int pidrequest, int *data);
void prom_BootRemote ( char *progname, char *args[], unsigned long entry_point,
		       int auto_load, char *dbgmon_pathname, int qualify_name);
void prom_download ( char *progname, char **args,
		     int qualify_name, int just_load);
int prom_InitRemote ( char *pdbxport, int use_sockets, char *service_name,
		      int __sync);
void prom_sigint_handler( void );


/* sable functions */
unsigned long sable_ptrace ();
void sable_BootRemote ( char *progname, char *args[] );
int sable_InitRemote ( int use_sockets );
void sable_sigint_handler( void );


/* pixie functions */
void pixie_init( /* char *pixie_objname, int pixie_textbase,
		 int pixie_textoffset, FILE *pixie_objfile */ );
void set_pixie_base (  long base );
int pixie_map_addr( int addr);
int mipspc_from_pixiepc (int pid, int pcPixie);
int pixie_ptrace();
boolean is_pixie_debug();
void pixie_debug_off();
void pixie_debug_on();
#endif
