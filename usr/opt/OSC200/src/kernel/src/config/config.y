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
 * @(#)$RCSfile: config.y,v $ $Revision: 4.4.18.4 $ (DEC) $Date: 1993/10/29 21:11:55 $
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*	Change History							*
 *									*
 * 27-Oct-91	Fred Canter						*
 *		Added maxssiz, dflssiz. dfldsiz tokens for configuring	*
 *		stack and data size limits.				*
 *		Make System V IPC definitions configurable.		*
 *									*
 * 6-June-1991	Brian Stevens						*
 *		Added new config file options maxuprc, bufcache,	*
 *		maxcallouts, and maxthreads (per task).			*
 *									*
 * 4-1-91	robin-							*
 *		Made a change to fix the problem where 'slot' would not *
 *		work for a BUS type.					*
 *									*
 * 3-20-91	robin-							*
 *		Made changes to support new device data structures	*
 *									*
 */


%union {
	char	*str;
	long	val;
	struct	file_list *file;
	struct	idlst *lst;
	double	fval;
}

%token	ADDRMOD
%token	AND
%token	ANONKLSHIFT
%token	ANONKLPAGES
%token	ANY
%token	ARGS
%token	AT
%token	BIN
%token	BUFCACHE
%token	BUS
%token  CALLOUT
%token	CLUSTERMAP
%token	CLUSTERSIZE
%token	COMMA
%token	CONFIG
%token	CONTROLLER
%token	COWFAULTS
%token	CPU
%token	CSR
%token	CSUBMAPSIZE
%token	DEVICE
%token	DFLDSIZ
%token	DFLSSIZ
%token	DISK
%token	DRIVE
%token	DST
%token	DUMPS
%token	DYNAMIC
%token	EQUALS
%token	FLAGS
%token	HEAPPERCENT
%token	HZ
%token	IDENT
%token  KENTRY_ZONE_SIZE
%token	MACHINE
%token	MAJOR
%token	MAPENTRIES
%token	MASTER
%token	MAXCALLOUTS
%token	MAXDSIZ
%token	MAXSSIZ
%token	MAXTHREADS
%token	THREADMAX
%token	TASKMAX
%token	MAXUPRC
%token	MAXUSERS
%token  NCLIST
%token	MAXPROC
%token	SYS_V_MODE
%token	MAXVAS
%token	MAXWIRE
%token	MBA
%token	MBII
%token	MINOR
%token	MINUS
%token	MSCP
%token	MSGMAX
%token	MSGMNB
%token	MSGMNI
%token	MSGTQL
%token	NEXUS
%token  NOT
%token	ON
%token	OPTIONS
%token	MAKEOPTIONS
%token  PORT
%token	PRIORITY
%token	PROCESSORS
%token	PSEUDO_DEVICE
%token	READIO_KLUSTER
%token  READONLY
%token  RELEASE
%token	REMOTE_CONTROLLER
%token	ROOT
%token  SCSID
%token  SEGMENTATION
%token	SEMICOLON
%token	SEMMNI
%token	SEMMNS
%token	SEMMSL
%token	SEMOPM
%token	SEMUME
%token	SEMVMX
%token	SEMAEM
%token	SHMMIN
%token	SHMMAX
%token	SHMMNI
%token	SHMSEG
%token	SIZE
%token	SLAVE
%token  SLOT
%token	SWAP
%token	SWAPBUFFERS
%token	SYSWIREDPERCENT
%token	TAPE
%token	TIMEZONE
%token	TRACE
%token	UBA
%token	UBCMINPERCENT
%token	UBCMAXPERCENT
%token  UNIT
%token  VAXBI
%token  VBA
%token	VECTOR
%token  VERSION
%token	VME
%token  VME16D16
%token  VME24D16
%token  VME32D16
%token  VME16D32
%token  VME24D32
%token  VME32D32
%token	VPAGEMAX
%token	UBCBUFFERS
%token	UBCDIRTYPERCENT
%token	UBCPAGESTEAL
%token	UBCSEQSTARTPERCENT
%token	UBCSEQPERCENT
%token  WILD
%token	WRITEIO_KLUSTER
%token  WRITEONLY
%token  ZONE_SIZE

%token  AT_START
%token  AT_EXIT
%token	AT_SUCCESS
%token  BEFORE_H
%token	AFTER_H
%token	BEFORE_MAKEFILE
%token	AFTER_MAKEFILE
%token	BEFORE_C
%token	AFTER_C
%token	BEFORE_CONF
%token	AFTER_CONF

/* following 3 are unique to CMU */
%token	LUN

%token	<str>	ID
%token	<val>	NUMBER
%token	<fval>	FPNUMBER

%type	<str>	Save_id
%type   <str>	Unix_command
%type	<str>	Opt_value
%type	<str>	Dev
%type	<lst>	Id_list
%type	<val>	optional_size
%type	<str>	device_name
%type	<val>	major_minor
%type	<val>	arg_device_spec
%type	<val>	root_device_spec
%type	<val>	dump_device_spec
%type	<file>	swap_device_spec
%type	<val>	Value
%type   <val>   Csr_Number

%{

#include "config.h"
#include <ctype.h>
#include <stdio.h>
#include <io/common/devdriver.h>

struct cpequiv {
	char *cp_alias;
	char *cp_equiv;
} cpequiv[] = {
	{ "DS2100",  "DS3100"  },
	{ NULL, NULL }
	};

struct	device_entry cur;
struct	device_entry *curp = 0;
char	*temp_id;
char	*val_id;
char	*malloc();
char 	command_str[256];


struct orphen_dev *orph_hdp;
struct orphen_dev *orph_p;

int first_mscp = 1;
int NOTset = 0;

struct device_entry psuedo_controller;

%}
%%
Configuration:
	Many_specs
		= {
		   verifysystemspecs();
		  };

Many_specs:
	Many_specs Spec
		|
	/* lambda */
		;

Spec:
	WRITEONLY Device_spec SEMICOLON
	      = { NOTset = ALV_WONLY;
		  newdev(&cur);
	        } |
	READONLY Device_spec SEMICOLON
	      = { NOTset = ALV_RONLY;
		  newdev(&cur);
	        } |
	NOT Device_spec SEMICOLON
	      = { NOTset = ALV_NOCNFG;
		  newdev(&cur);
	        } |
	Device_spec SEMICOLON
	      = { NOTset = ALV_FREE;
		  newdev(&cur); 
                } |
	Config_spec SEMICOLON
		|
	TRACE SEMICOLON
	      = { do_trace = !do_trace; } |
	SEMICOLON
		|
	error SEMICOLON
		;

Config_spec:
	MACHINE Save_id
	    = {
		if (!strcmp($2, "VAX")) {
			machine = MACHINE_VAX;
			machinename = "VAX";
		} else if (!strcmp($2, "PMAX")) {
			machine = MACHINE_DEC_RISC;
			machinename = "mips";
		} else if (!strcmp($2, "mips")) {
			machine = MACHINE_DEC_RISC;
			machinename = "mips";
		} else if (!strcmp($2, "DEC_RISC")) {
			machine = MACHINE_DEC_RISC;
			machinename = "mips";
		} else if (!strcmp($2, "alpha")) {
			machine = MACHINE_ALPHA;
			machinename = "alpha";
		} else
			yyerror("Unknown machine type");
	      } |
	CPU Save_id
	      = {
		struct cpequiv *cpe;
		struct cputype *cp =
		    (struct cputype *)malloc(sizeof (struct cputype));
		cp->cpu_name = ns($2);
		cp->cpu_next = cputype;
		cputype = cp;
		free(temp_id);
		init_dev(&cur);
		cur.d_name=ns($2);
		cur.d_type=CPU;
		for (cpe = cpequiv; cpe->cp_alias != NULL; cpe++) {
			if (strcmp(cp->cpu_name, cpe->cp_alias) == 0) {
				cp = (struct cputype *)malloc(sizeof (struct cputype));
				cp->cpu_name = cpe->cp_equiv;
				cp->cpu_next = cputype;
				cputype = cp;
				init_dev(&cur);
				newdev(&cur);
				cur.d_name = cpe->cp_equiv;
				cur.d_type = CPU;
				break;
			}
		}
		newdev(&cur);
	      } |
	OPTIONS Opt_list
		|
	CALLOUT Callout_list
		|
	MAKEOPTIONS Mkopt_list
		|
	IDENT ID
	      = { ident = ns($2); } |
	RELEASE FPNUMBER
	      = { release = $2; } |
	VERSION NUMBER
	      = { version = $2; } |
	PROCESSORS NUMBER
	      = { processors = $2; } |
	System_spec
		|
	HZ NUMBER
	      = { yyerror("HZ specification obsolete; delete"); } |
	SCSID NUMBER NUMBER
	      = { scs_system_id.hos = $2; scs_system_id.lol = $3;} |
	SCSID NUMBER 
	      = { scs_system_id.hos = 0; scs_system_id.lol = $2;} |
	TIMEZONE NUMBER
	      = { timezone = 60 * $2; check_tz(); } |
	TIMEZONE NUMBER DST NUMBER
	      = { timezone = 60 * $2; dst = $4; check_tz(); } |
	TIMEZONE NUMBER DST
	      = { timezone = 60 * $2; dst = 1; check_tz(); } |
	TIMEZONE FPNUMBER
	      = { timezone = (int) (60 * $2 + 0.5); check_tz(); } |
	TIMEZONE FPNUMBER DST NUMBER
	      = { timezone = (int) (60 * $2 + 0.5); dst = $4; 
		  check_tz(); } |
	TIMEZONE FPNUMBER DST
	      = { timezone = (int) (60 * $2 + 0.5); dst = 1; 
		  check_tz(); } |
	TIMEZONE MINUS NUMBER
	      = { timezone = -60 * $3; check_tz(); } |
	TIMEZONE MINUS NUMBER DST NUMBER
	      = { timezone = -60 * $3; dst = $5; check_tz(); } |
	TIMEZONE MINUS NUMBER DST
	      = { timezone = -60 * $3; dst = 1; check_tz(); } |
	TIMEZONE MINUS FPNUMBER
	      = { timezone = -((int) (60 * $3 + 0.5)); check_tz(); } |
	TIMEZONE MINUS FPNUMBER DST NUMBER
	      = { timezone = -((int) (60 * $3 + 0.5)); dst = $5; 
		  check_tz(); } |
	TIMEZONE MINUS FPNUMBER DST
	      = { timezone = -((int) (60 * $3 + 0.5)); dst = 1; 
		  check_tz(); } |
	BUFCACHE NUMBER
	      = { bufcache = $2; }; |
	UBCMINPERCENT NUMBER
	      = { ubcminpercent = $2; }; |
	UBCMAXPERCENT NUMBER
	      = { ubcmaxpercent = $2; }; |
	COWFAULTS NUMBER
	      = { cowfaults = $2; }; |
	MAPENTRIES NUMBER
	      = { mapentries = $2; }; |
	MAXVAS NUMBER
	      = { maxvas = $2; }; |
	MAXWIRE NUMBER
	      = { maxwire = $2; }; |
	HEAPPERCENT NUMBER
	      = { heappercent = $2; }; |
	ANONKLSHIFT NUMBER
	      = { anonklshift = $2; }; |
	ANONKLPAGES NUMBER
	      = { anonklpages = $2; }; |
	VPAGEMAX NUMBER
	      = { vpagemax = $2; }; |
	SEGMENTATION NUMBER
	      = { segmentation = $2 + 1; }; |
	UBCPAGESTEAL NUMBER
	      = { ubcpagesteal = $2; }; |
	UBCDIRTYPERCENT NUMBER
	      = { ubcdirtypercent = $2; }; |
	UBCSEQSTARTPERCENT NUMBER
	      = { ubcseqstartpercent = $2; }; |
	UBCSEQPERCENT NUMBER
	      = { ubcseqpercent = $2; }; |
	CSUBMAPSIZE NUMBER
	      = { csubmapsize = $2; }; |
	UBCBUFFERS NUMBER
	      = { ubcbuffers = $2; }; |
	SWAPBUFFERS NUMBER
	      = { swapbuffers = $2; }; |
	CLUSTERMAP NUMBER
	      = { clustermap = $2; }; |
	CLUSTERSIZE NUMBER
	      = { clustersize = $2; }; |
        ZONE_SIZE NUMBER
              = { zone_size = $2; }; |
        KENTRY_ZONE_SIZE NUMBER
              = { kentry_zone_size = $2; }; |
	SYSWIREDPERCENT NUMBER
	      = { syswiredpercent = $2; }; |	
	READIO_KLUSTER NUMBER
	      = { readio_kluster = $2; }; |
	WRITEIO_KLUSTER NUMBER
	      = { writeio_kluster = $2; }; |
	MAXCALLOUTS NUMBER
	      = { maxcallouts = $2; }; |
	MAXTHREADS NUMBER
	      = { maxthreads = $2; }; |
	THREADMAX NUMBER
	      = { threadmax = $2; }; |
	TASKMAX NUMBER
	      = { taskmax = $2; }; |
	MAXUPRC NUMBER
	      = { maxuprc = $2; }; |
	MAXUSERS NUMBER
	      = { maxusers = $2; }; |
	NCLIST NUMBER
	      = { nclist = $2; }; |
	MAXPROC NUMBER
	      = { maxproc = $2; }; |
	SYS_V_MODE NUMBER
	      = { sys_v_mode = $2; }; |
	MAXDSIZ NUMBER
	      = { maxdsiz = $2; }; |
	MAXSSIZ NUMBER
	      = { maxssiz = $2; }; |
	DFLDSIZ NUMBER
	      = { dfldsiz = $2; }; |
	DFLSSIZ NUMBER
	      = { dflssiz = $2; }; |
	MSGMAX NUMBER
	      = { msgmax = $2; }; |
	MSGMNB NUMBER
	      = { msgmnb = $2; }; |
	MSGMNI NUMBER
	      = { msgmni = $2; }; |
	MSGTQL NUMBER
	      = { msgtql = $2; }; |
	SEMMNI NUMBER
	      = { semmni = $2; }; |
	SEMMNS NUMBER
	      = { semmns = $2; }; |
	SEMMSL NUMBER
	      = { semmsl = $2; }; |
	SEMOPM NUMBER
	      = { semopm = $2; }; |
	SEMUME NUMBER
	      = { semume = $2; }; |
	SEMVMX NUMBER
	      = { semvmx = $2; }; |
	SEMAEM NUMBER
	      = { semaem = $2; }; |
	SHMMIN NUMBER
	      = { shmmin = $2; }; |
	SHMMAX NUMBER
	      = { shmmax = $2; }; |
	SHMMNI NUMBER
	      = { shmmni = $2; }; |
	SHMSEG NUMBER
	      = { shmseg = $2; };

System_spec:
	  System_id System_parameter_list
		= { checksystemspec(*confp); }
	;

System_id:
	  CONFIG Save_id
		= { mkconf($2); }
	;

System_parameter_list:
	  System_parameter_list System_parameter
	| System_parameter
	;

System_parameter:
	  swap_spec
	| root_spec
	| dump_spec
	| arg_spec
	;

swap_spec:
	  SWAP optional_on swap_device_list
	;

swap_device_list:
	  swap_device_list AND swap_device
	| swap_device
	;

swap_device:
	  swap_device_spec optional_size
	      = { mkswap(*confp, $1, $2); }
	;

swap_device_spec:
	  device_name
		= {
			struct file_list *fl = newswap();

			if (eq($1, "generic") || eq($1, "boot") || eq($1, "none"))
				fl->f_fn = $1;
			else {
				fl->f_swapdev = nametodev($1, 0, 'b');
				fl->f_fn = devtoname(fl->f_swapdev);
			}
			$$ = fl;
		}
	| major_minor
		= {
			struct file_list *fl = newswap();

			fl->f_swapdev = $1;
			fl->f_fn = devtoname($1);
			$$ = fl;
		}
	;

root_spec:
	  ROOT optional_on root_device_spec
		= {
			struct file_list *fl = *confp;

			if (fl && fl->f_rootdev != NODEV)
				yyerror("extraneous root device specification");
			else
				fl->f_rootdev = $3;
		}
	;

root_device_spec:
	  device_name
		= { $$ = nametodev($1, 0, 'a'); }
	| major_minor
	;

dump_spec:
	  DUMPS optional_on dump_device_spec
		= {
			struct file_list *fl = *confp;

			if (fl && fl->f_dumpdev != NODEV)
				yyerror("extraneous dump device specification");
			else
				fl->f_dumpdev = $3;
		}

	;

dump_device_spec:
	  device_name
		= { $$ = nametodev($1, 0, 'b'); }
	| major_minor
	;

arg_spec:
	  ARGS optional_on arg_device_spec
		= {
			struct file_list *fl = *confp;

			if (fl && fl->f_argdev != NODEV)
				yyerror("extraneous arg device specification");
			else
				fl->f_argdev = $3;
		}
	;

arg_device_spec:
	  device_name
		= { $$ = nametodev($1, 0, 'b'); }
	| major_minor
	;

major_minor:
	  MAJOR NUMBER MINOR NUMBER
		= { $$ = makedev($2, $4); }
	;

optional_on:
	  ON
	| /* empty */
	;

optional_size:
	  SIZE NUMBER
	      = { $$ = $2; }
	| /* empty */
	      = { $$ = 0; }
	;

device_name:
	  Save_id
		= { $$ = $1; }
	| Save_id NUMBER
		= {
			char buf[80];

			(void) sprintf(buf, "%s%d", $1, $2);
			$$ = ns(buf); free($1);
		}
	| Save_id NUMBER ID
		= {
			char buf[80];

			(void) sprintf(buf, "%s%d%s", $1, $2, $3);
			$$ = ns(buf); free($1);
		}
	;

Opt_list:
	Opt_list COMMA Option
		|
	Option
		;

Option:
	Basic_option
		|
	Basic_option DYNAMIC Save_id
	      = { opt_tail->op_dynamic = $3; }
		;

Basic_option:
	Save_id
	      = {
		struct opt *op = (struct opt *)malloc(sizeof (struct opt));
		op->op_name = ns($1);
		op->op_next = (struct opt *) 0;
		op->op_value = 0;
		op->op_dynamic = 0;
		if (opt == (struct opt *) 0)
			opt = op;
		else
			opt_tail->op_next = op;
		opt_tail = op;
		free(temp_id);
	      } |
	Save_id EQUALS Opt_value
	      = {
		struct opt *op = (struct opt *)malloc(sizeof (struct opt));
		op->op_name = ns($1);
		op->op_next = (struct opt *) 0;
		op->op_value = ns($3);
		op->op_dynamic = 0;
		if (opt == (struct opt *) 0)
			opt = op;
		else
			opt_tail->op_next = op;
		opt_tail = op;
		free(temp_id);
		if (val_id)
			free(val_id);
	      } ;

Opt_value:
	ID
	      = { $$ = val_id = ns($1); } |
	NUMBER
	      = { char nb[16];sprintf(nb, "%u", $1); $$ = val_id = ns(nb); } |
	/* lambda from MIPS -- WHY */
	      = { $$ = val_id = ns(""); }
	      ;


Save_id:
	ID
	      = { $$ = temp_id = ns($1); }
	;

Callout_list:
	AT_START Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL0, val_id);
	      }|
	AT_EXIT Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL1, val_id);
	       }|
	AT_SUCCESS Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL2, val_id);
	       }|
	BEFORE_H Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL3, val_id);
	       }|
	AFTER_H Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL4, val_id);
	       }|
	BEFORE_MAKEFILE Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL5, val_id);
	       }|
	AFTER_MAKEFILE Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL6, val_id);
	       }|
	BEFORE_C Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL7, val_id);
	       }|
	AFTER_C Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL8, val_id);
	       }|
	BEFORE_CONF Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL9, val_id);
	       }|
	AFTER_CONF Unix_command
	      = {
		      Exec_unix_command(EVENT_LEVEL10, val_id);
	       };

Unix_command:
	ID
	      = { $$ = val_id = ns($1); }
	      ;

Mkopt_list:
	Mkopt_list COMMA Mkoption
		|
	Mkoption
		;

Mkoption:
	Save_id EQUALS Opt_value
	      = {
		struct opt *op = (struct opt *)malloc(sizeof (struct opt));
		op->op_name = ns($1);
		op->op_next =  (struct opt *) 0;
		op->op_value = ns($3);
		if (mkopt == (struct opt *) 0)
			mkopt = op;
		else
			mkopt_tail->op_next = op;
		mkopt_tail = op;
		free(temp_id);
		if (val_id)
			free(val_id);
	      } ;

Dev:
	UBA
	      = { $$ = ns("uba"); } |
	VAXBI
	      = { $$ = ns("vaxbi"); } |
	VBA
	      = { $$ = ns("vba"); } |
	MBA
	      = { $$ = ns("mba"); } |
	VME
	      = {
		if ( machine != MACHINE_DEC_RISC )
			yyerror("wrong machine type for vme");
			$$ = ns("vme");
		} |
	MBII
	      = {
		if ( machine != MACHINE_DEC_RISC )
			yyerror("wrong machine type for mbii");
			$$ = ns("mbii");
		} |
	ID
	      = { $$ = ns($1); }
	;

Device_spec:
	Basic_device_spec
		|
	Basic_device_spec DYNAMIC Save_id
		= { cur.d_dynamic = $3; }
	;

Basic_device_spec:
	MASTER Dev_name Dev_info Int_spec
	      = { 
		      cur.d_type = MASTER;
	        } |
	DISK Dev_name Dev_info
	      = { 
		  cur.d_type_string = "disk";
		  cur.d_dk = 1; 
		  cur.d_type = DEVICE; 
	        } |
	TAPE Dev_name Dev_info
	      = { 
		      cur.d_type_string = "tape";
		      cur.d_type = DEVICE; 
	        } |
	DEVICE Init_dev Dev NUMBER Dev_info
	      = {
		      cur.d_name = $3;
		      cur.d_unit = $4;
		      cur.d_type_string = $3;
		      cur.d_type = DEVICE; 
	         } |
	DEVICE DISK Dev_name Dev_info
	      = { 
		  cur.d_type_string = "disk";
		  cur.d_dk = 1; 
		  cur.d_type = DEVICE; 
	        } |
	DEVICE TAPE Dev_name Dev_info
	      = { 
		      cur.d_type_string = "tape";
		      cur.d_type = DEVICE; 
	        } |
	CONTROLLER Dev_name Dev_info Int_spec
	      = { 
		      cur.d_type = CONTROLLER;
	        } |
	PSEUDO_DEVICE Init_dev Dev
	      = {
		cur.d_name = $3;
		cur.d_type = PSEUDO_DEVICE;
		} |
	PSEUDO_DEVICE Init_dev Dev NUMBER
	      = {
		cur.d_name = $3;
		cur.d_type = PSEUDO_DEVICE;
		cur.d_slave = $4;
		} |
	BUS Dev_name Dev_info Int_spec
	      = { 
		      cur.d_type = BUS;
	        };

Dev_name:
	Init_dev Dev NUMBER
	      = {
		cur.d_name = $2;
		if (eq($2, "mba"))
			seen_mba = 1;
		else if (eq($2, "uba")){
			seen_uba = 1;
			if($3 > highuba)
				highuba = $3; 
		     }
		else if (eq($2, "mbii"))
			seen_mbii = 1;
		else if (eq($2, "vme"))
			seen_vme = 1;
		cur.d_unit = $3;
		};

Init_dev:
	/* lambda */
	      = { init_dev(&cur); };

Dev_info:
	Con_info Info_list
		|
	/* lambda */
		;

Con_info:
	AT '*'
	      = {
		   cur.d_wildcard = "*";	/* Mark it as a WILD CARD entry */
		   cur.d_conn = (struct device_entry *)QUES;/* Wildcard the connection 	*/
                } |
	AT Dev NUMBER
	      = {
		   cur.d_conn = connect($2, $3);
		   if( (cur.d_conn != 0)) {
			cur.d_adaptor = cur.d_conn ->d_adaptor;
			cur.d_nexus = cur.d_conn ->d_nexus;
			cur.d_extranum = cur.d_conn->d_extranum;

			if(needs_csr(cur.d_conn->d_name)) {
			    cur.d_addr = cur.d_conn->d_addr;
			    cur.d_conn->d_addr = 0;
			}
			if(needs_vector(cur.d_conn->d_name)){
			    cur.d_vec = cur.d_conn->d_vec;
			    cur.d_conn->d_vec = 0;
			}
		   } else
			   save_orphen(&cur,$2,$3, cur.d_type);
		   if(needs_pseudo_uba(cur.d_name)) {
		       cur.d_extranum = extrauba++;
		   }
		} |
	AT MSCP = {
			/* create a mscp pseudo if it doesn't exist */
			/* and link this one to it */
		    if (first_mscp) {
			struct device_entry tempdev;

		    	init_dev(&tempdev);
	       		tempdev.d_type = CONTROLLER;
			tempdev.d_name = ns("mscp");
			tempdev.d_unit = 0;
			psuedo_controller.d_name = ns("mscp");
		        tempdev.d_conn = &psuedo_controller;
		    	newdev(&tempdev);
			first_mscp = 0;

		    }
		    cur.d_conn = connect("mscp", 0);
		    if(cur.d_conn == NULL)
			   save_orphen(&cur,"MSCP",0, CONTROLLER);
		    cur.d_drive = QUES;
	    	} |
	AT NEXUS
	      = {
		cur.d_conn = TO_NEXUS;
		cur.d_adaptor = QUES;
		cur.d_nexus = QUES;
		} |
	AT NEXUS NUMBER
	      = { 
	        check_nexus(&cur, $3);
		cur.d_conn = TO_NEXUS;
		cur.d_nexus = $3;
		cur.d_adaptor = cur.d_unit;
		} |
	AT Dev NUMBER PORT Dev
	     =	{
		cur.d_conn = connect($2,$3);
		if(cur.d_conn == NULL)
			save_orphen(&cur,$2,$3,cur.d_type);

		cur.d_adaptor = $3;
		cur.d_port_name = $5;
	      	} |

	AT Dev NUMBER REMOTE_CONTROLLER NUMBER
	     =	{
		cur.d_conn = connect($2,$3);
		if(cur.d_conn == NULL)
			save_orphen(&cur,$2,$3,cur.d_type);

		cur.d_adaptor = $3;
		cur.d_rcntl = $5;
	      	};

Info_list:
	Info_list Info
		|
	/* lambda */
		;
Info:
	Csr_Number
	      = {
		cur.d_addr = $1;
		} |
	Csr_Number NUMBER
	      = { 
		cur.d_addr2 = $2;
	        } |
	DRIVE NUMBER
	      = {
		cur.d_drive = $2;
		} |
	SLOT NUMBER
	      = {
		cur.d_slot = $2;
		} |
	UNIT NUMBER
	      = {
			cur.d_drive = $2;
		} |
	SLAVE NUMBER
	      = {
		if (cur.d_conn != 0 && cur.d_conn != TO_NEXUS &&
		     cur.d_conn->d_type == MASTER)
			cur.d_slave = $2;
		else
			yyerror("can't specify slave--not to master");
		} |
	ADDRMOD NUMBER
	      = { cur.d_addrmod = $2; } |
	LUN NUMBER
	      = {
		if ((cur.d_conn != 0) && (cur.d_conn != TO_SLOT) &&
			(cur.d_conn->d_type == CONTROLLER)) {
			cur.d_addr = $2; 
		}
		else {
			yyerror("device requires controller card");
		    }
		} |
	FLAGS NUMBER
	      = {
		cur.d_flags = $2;
	      } |
	BIN NUMBER
	      = { 
		 if ($2 < 1 || $2 > 7)  
			yyerror("bogus bin number");
		 else {
			cur.d_bin = $2;
			dev_param(&cur, "bin", $2);
		}
	       } |
	Dev Value
	      = {
		dev_param(&cur, $1, $2);
		};
Csr_Number:
	CSR NUMBER
	      = { $$ = $2; } ;

Value:
	NUMBER
	      |
	MINUS NUMBER
	      = { $$ = -($2); }
	;

Int_spec:
	Vec_spec
	      = { cur.d_pri = 0; } |
	PRIORITY NUMBER
	      = { cur.d_pri = $2; } |
        PRIORITY NUMBER Vec_spec
	      = { cur.d_pri = $2; } |
        Vec_spec PRIORITY NUMBER
	      = { cur.d_pri = $3; } |
	/* lambda */
		;

Vec_spec:
        VECTOR Id_list
	      = { cur.d_vec = $2; } ;

Id_list:
	Save_id
	      = {
		struct idlst *a = (struct idlst *)malloc(sizeof(struct idlst));
		a->id = $1; a->id_next = 0; $$ = a;
		a->id_vec = 0;
		} |
	Save_id Id_list =
		{
		struct idlst *a = (struct idlst *)malloc(sizeof(struct idlst));
	        a->id = $1; a->id_next = $2; $$ = a;
		a->id_vec = 0;
		} |
        Save_id NUMBER
	      = {
		struct idlst *a = (struct idlst *)malloc(sizeof(struct idlst));
		a->id_next = 0; a->id = $1; $$ = a;
		cur.d_ivnum = a->id_vec = $2;
		} |
        Save_id NUMBER Id_list
	      = {
		struct idlst *a = (struct idlst *)malloc(sizeof(struct idlst));
		a->id_next = $3; a->id = $1; $$ = a;
		cur.d_ivnum = a->id_vec = $2;
		};

%%

yyerror(s)
	char *s;
{

	fprintf(stderr, "config: line %d: %s\n", yyline, s);
}

/*
 * return the passed string in a new space
 */
char *
ns(str)
	register char *str;
{
	register char *cp;

	cp = malloc((unsigned)(strlen(str)+1));
	(void) strcpy(cp, str);
	return (cp);
}

/*
 * add a device to the list of devices
 */
newdev(dp)
	register struct device_entry *dp;
{
	register struct device_entry *np;

	np = (struct device_entry *) malloc(sizeof *np);
	*np = *dp;
	if (curp == 0){
		dtab = np;
        }else
		curp->d_next = np;
	curp = np;
	curp->d_next = 0;
	curp->d_disable = NOTset;
}

/*
 * note that a configuration should be made
 */
mkconf(sysname)
	char *sysname;
{
	register struct file_list *fl, **flp;
	int i;

	fl = (struct file_list *) malloc(sizeof *fl);
	fl->f_type = SYSTEMSPEC;
	for (i = 1; i < NNEEDS; i++) {
		fl->f_needs[i] = 0;
	}
	fl->f_needs[0] = sysname;
	fl->f_rootdev = NODEV;
	fl->f_argdev = NODEV;
	fl->f_dumpdev = NODEV;
	fl->f_fn = 0;
	fl->f_next = 0;
	for (flp = confp; *flp; flp = &(*flp)->f_next)
		;
	*flp = fl;
	confp = flp;
}

struct file_list *
newswap()
{
	struct file_list *fl = (struct file_list *)malloc(sizeof (*fl));
	int i;

	fl->f_type = SWAPSPEC;
	fl->f_next = 0;
	fl->f_swapdev = NODEV;
	fl->f_swapsize = 0;
	for (i = 0; i < NNEEDS; i++) {
		fl->f_needs[i] = 0;
	}
	fl->f_fn = 0;
	return (fl);
}

/*
 * Add a swap device to the system's configuration
 */
mkswap(system, fl, size)
	struct file_list *system, *fl;
	int size;
{
	register struct file_list **flp;
	char *cp, name[80];

	if (system == 0 || system->f_type != SYSTEMSPEC) {
		yyerror("\"swap\" spec precedes \"config\" specification");
		return;
	}
	if (size < 0) {
		yyerror("illegal swap partition size");
		return;
	}
	/*
	 * Append swap description to the end of the list.
	 */
	flp = &system->f_next;
	for (; *flp && (*flp)->f_type == SWAPSPEC; flp = &(*flp)->f_next)
		;
	fl->f_next = *flp;
	*flp = fl;
	fl->f_swapsize = size;
	/*
	 * If first swap device for this system,
	 * set up f_fn field to insure swap
	 * files are created with unique names.
	 */
	if (system->f_fn)
		return;
	if (eq(fl->f_fn, "generic") || eq(fl->f_fn, "boot") || eq(fl->f_fn, "none"))
		system->f_fn = ns(fl->f_fn);
	else
		system->f_fn = ns(system->f_needs[0]);
}

/*
 * find the pointer to connect to the given device and number.
 * returns 0 if no such device and prints an error message
 */
struct device_entry *
connect(dev, num)
	register char *dev;
	register int num;
{
	register struct device_entry *dp;
	struct device_entry *huhcon();
	if (num == QUES){
		return (huhcon(dev));
	}
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if ((num != dp->d_unit) || !eq(dev, dp->d_name))
			continue;
		if (dp->d_type != CONTROLLER && dp->d_type != MASTER && dp->d_type != BUS) {
			sprintf(errbuf, "%s is not connected to a bus or a controller", dev);
			yyerror(errbuf);
			return (0);
		}
		return (dp);
	}
	return (0);
}

/*
 * connect to an unspecific thing.  On success return a pointer to a device entry
 * that all ready exists as a wild card by this name or create an entry (wildcard)
 * and return it.  At least one entry with this name must already exist or we fail.
 * That is we expect any device spec entry to create a "newdev()" call and to get
 * here without a device_entry should be impossible.
 */
struct device_entry *
huhcon(dev)
	register char *dev;
{
	register struct device_entry *dp, *dcp;
	struct device_entry rdev;
	int oldtype;
	/*
	 * First make certain that there are some of these to wildcard on
	 * This is done by walking the entire device list looking for a match
	 * on the name passed in and the same name in the device structure
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next)
		if (eq(dp->d_name, dev))
			break;
	/* The only way dp can == 0 is if the search for a name match
	 * in the above for loop never found a match and the end of the
	 list was reached.
	 */
	if (dp == 0) {
		sprintf(errbuf, "no %s's to wildcard", dev);
		yyerror(errbuf);
		return (0);
	}
	oldtype = dp->d_type;
	dcp = dp->d_conn;
	/*
	 * Now see if there is already a wildcard entry for this device
	 * (e.g. Search for a "uba ?")
	 */
	for (; dp != 0; dp = dp->d_next)
		if (eq(dev, dp->d_name) && dp->d_unit == -1)
			break;
	/*
	 * If there isn't, make one because everything needs to be connected
	 * to something.
	 */
	if (dp == 0) {
		dp = &rdev;
		init_dev(dp);
		dp->d_unit = QUES;
		dp->d_adaptor = dp->d_nexus = QUES;
		dp->d_name = ns(dev);
		dp->d_type = oldtype;
		dp->d_next = 0;
		newdev(dp);
		dp = curp;
		/*
		 * Connect it to the same thing that other similar things are
		 * connected to, but make sure it is a wildcard unit
		 * (e.g. up connected to sc ?, here we make connect sc? to a
		 * uba?).  If other things like this are on the NEXUS or
		 * if they aren't connected to anything, then make the same
		 * connection, else call ourself to connect to another
		 * unspecific device.
		 */
		if (dcp == TO_NEXUS || dcp == 0)
			dp->d_conn = dcp;
		else
			dp->d_conn = connect(dcp->d_name, QUES);
		if(dp->d_conn == NULL)
			save_orphen(dp,dcp->d_name, QUES, dp->d_type);
	}
	return (dp);
}

init_dev(dp)
	register struct device_entry *dp;
{

	dp->d_name = "OHNO!!!";
	dp->d_type = DEVICE;
	dp->d_type_string = "";
	dp->d_conn = 0;
	dp->d_port_name = "";
	dp->d_vec = 0;
	dp->d_addr = dp->d_addr2 = dp->d_ivnum = 0;
        dp->d_pri = dp->d_flags = dp->d_dk = 0;
	dp->d_rcntl = 0;
	dp->d_slot = -1;
	dp->d_slave = dp->d_drive = dp->d_unit = UNKNOWN;
	dp->d_adaptor = dp->d_nexus = dp->d_extranum = UNKNOWN;
	dp->d_addrmod = 0;
	dp->d_wildcard = "";
	dp->d_dynamic = 0;
}

/*
 * Check that the NEXUS entry is OK
 */
check_nexus(dev, num)
	register struct device_entry *dev;
	int num;
{

	if (num != QUES)
		yyerror("can't give specific nexus numbers");
}

/*
 * Check the timezone to make certain it is sensible
 */

check_tz()
{
	if (abs(timezone) > 12 * 60)
		yyerror("timezone is unreasonable");
	else
		hadtz = 1;
}

/*
 * Check system specification and apply defaulting
 * rules on root, argument, dump, and swap devices.
 */
checksystemspec(fl)
	register struct file_list *fl;
{
	char buf[BUFSIZ];
	register struct file_list *swap;
	int generic;
	register minor_t min_no;

	if (fl == 0 || fl->f_type != SYSTEMSPEC) {
		yyerror("internal error, bad system specification");
		exit(1);
	}
	swap = fl->f_next;
	generic = swap && swap->f_type == SWAPSPEC &&
			(eq(swap->f_fn, "generic") || eq(swap->f_fn, "boot"));
	if (fl->f_rootdev == NODEV && !generic) {
		yyerror("no root device specified");
		exit(1);
	}
	/*
	 * Default swap area to be in 'b' partition of root's
	 * device.  If root specified to be other than on 'a'
	 * partition, give warning, something probably amiss.
	 */
	if (swap == 0 || swap->f_type != SWAPSPEC) {
/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY 
 * on OSF systems.
 */
		uint_t dev;

		swap = newswap();
		dev = fl->f_rootdev;
/*		if (minor(dev) & DEV_MASK) {
			sprintf(buf,
"Warning, swap defaulted to 'b' partition with root on '%c' partition",
				(minor(dev) & DEV_MASK) + 'a');
			yyerror(buf);
		}
*/
		/*
		 * We differentiate between SCSI and DSA because each has
		 * a different underlying minor number format.
		 */
		switch (major(dev)) {
			case SCSI_MAJ:
			    min_no = MAKECAMMINOR(GETCAMUNIT(dev), ('b'-'a'));
			    break;
			case MSCP_MAJ:
			    min_no = MAKEMINOR(GETUNIT(dev), ('b'-'a'));
			    break;
			default:
			    min_no = MAKEMINOR(GETUNIT(dev), ('b'-'a'));
		}

		swap->f_swapdev = makedev(major(dev), min_no);
		swap->f_fn = devtoname(swap->f_swapdev);
		mkswap(fl, swap, 0);
	}
	/*
	 * Make sure a generic swap isn't specified, along with
	 * other stuff (user must really be confused).
	 */
	if (generic) {
		if (fl->f_rootdev != NODEV)
			yyerror("root device specified with generic swap");
		if (fl->f_argdev != NODEV)
			yyerror("arg device specified with generic swap");
		if (fl->f_dumpdev != NODEV)
			yyerror("dump device specified with generic swap");
		return;
	}
	/*
	 * Default argument device and check for oddball arrangements.
	 */
	if (fl->f_argdev == NODEV)
		fl->f_argdev = swap->f_swapdev;
	if (fl->f_argdev != swap->f_swapdev)
		yyerror("Warning, arg device different than primary swap");
	/*
	 * Default dump device and warn if place is not a
	 * swap area or the argument device partition.
	 */
	if (fl->f_dumpdev == NODEV)
		fl->f_dumpdev = swap->f_swapdev;
	if (fl->f_dumpdev != swap->f_swapdev && fl->f_dumpdev != fl->f_argdev) {
		struct file_list *p = swap->f_next;

		for (; p && p->f_type == SWAPSPEC; p = p->f_next)
			if (fl->f_dumpdev == p->f_swapdev)
				return;
/*		sprintf(buf, "Warning, orphaned dump device, %s",
			"do you know what you're doing");
		yyerror(buf); */
	}
}

/*
 * Verify all devices specified in the system specification
 * are present in the device specifications.
 */
verifysystemspecs()
{
	register struct file_list *fl;
/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY 
 * on OSF systems.
 */
	uint_t checked[50], *verifyswap();
	register uint_t *pchecked = checked;

	for (fl = conf_list; fl; fl = fl->f_next) {
		if (fl->f_type != SYSTEMSPEC)
			continue;
		if (!finddev(fl->f_rootdev))
			deverror(fl->f_needs[0], "root");
		*pchecked++ = fl->f_rootdev;
		pchecked = verifyswap(fl->f_next, checked, pchecked);
#define	samedev(dev1, dev2) \
	((GETUNIT(dev1)) != (GETUNIT(dev2)))
		if (!alreadychecked(fl->f_dumpdev, checked, pchecked)) {
			if (!finddev(fl->f_dumpdev))
				deverror(fl->f_needs[0], "dump");
			*pchecked++ = fl->f_dumpdev;
		}
		if (!alreadychecked(fl->f_argdev, checked, pchecked)) {
			if (!finddev(fl->f_argdev))
				deverror(fl->f_needs[0], "arg");
			*pchecked++ = fl->f_argdev;
		}
	}
}

/*
 * Do as above, but for swap devices.
 */
/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY 
 * on OSF systems.
 */
uint_t *
verifyswap(fl, checked, pchecked)
	register struct file_list *fl;
/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY 
 * on OSF systems.
 */
	uint_t checked[];
	register uint_t *pchecked;
{

	for (;fl && fl->f_type == SWAPSPEC; fl = fl->f_next) {
		if (eq(fl->f_fn, "generic") || eq(fl->f_fn, "boot")||
			eq(fl->f_fn, "none"))
			continue;
		if (alreadychecked(fl->f_swapdev, checked, pchecked))
			continue;
		if (!finddev(fl->f_swapdev))
			fprintf(stderr,
			   "config: swap device %s not configured", fl->f_fn);
		*pchecked++ = fl->f_swapdev;
	}
	return (pchecked);
}

/*
 * Has a device already been checked
 * for it's existence in the configuration?
 */
alreadychecked(dev, list, last)
/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY 
 * on OSF systems.
 */
	uint_t dev, list[];
	register uint_t *last;
{
/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY 
 * on OSF systems.
 */
	register uint_t *p;

	for (p = list; p < last; p++)
		if (samedev(*p, dev))
			return (1);
	return (0);
}

deverror(systemname, devtype)
	char *systemname, *devtype;
{

	fprintf(stderr, "config: %s: %s device not configured\n",
		systemname, devtype);
}

/*
 * Look for the device in the list of
 * configured hardware devices.  Must
 * take into account stuff wildcarded.
 */
finddev(dev)
/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY 
 * on OSF systems.
 */
	uint_t dev;
{

	/* punt on this right now */
	return (1);
}

/*
 * bi_info gives the magic number used to construct the token for
 * the autoconf code.  bi_max is the maximum value (across all
 * machine types for a given architecture) that a given "bus 
 * type" can legally have.
 */

struct bi_bus_info {
	char    *bi_name;
	u_short bi_info;
	u_int   bi_max;
};

struct bi_bus_info sun2_info[] = {
	{ "virtual",    0x0001, (1<<24)-1 },
	{ "obmem",      0x0002, (1<<23)-1 },
	{ "obio",       0x0004, (1<<23)-1 },
	{ "mbmem",      0x0010, (1<<20)-1 },
	{ "mbio",       0x0020, (1<<16)-1 },
	{ "vme16d16",   0x0100, (1<<16)-1 },
	{ "vme24d16",   0x0200, (1<<24)-(1<<16)-1 },
	{ (char *)0,    0,      0 }
};

struct bi_bus_info sun3_info[] = {
	{ "virtual",    0x0001, (1<<32)-1 },
	{ "obmem",      0x0002, (1<<32)-1 },
	{ "obio",       0x0004, (1<<21)-1 },
	{ "vme16d16",   0x0100, (1<<16)-1 },
	{ "vme24d16",   0x0200, (1<<24)-(1<<16)-1 },
	{ "vme32d16",   0x0400, (1<<32)-(1<<24)-1 },
	{ "vme16d32",   0x1000, (1<<16) },
	{ "vme24d32",   0x2000, (1<<24)-(1<<16)-1 },
	{ "vme32d32",   0x4000, (1<<32)-(1<<24)-1 },
	{ (char *)0,    0,      0 }
};

struct bi_bus_info sun4_info[] = {
	{ "virtual",    0x0001, 0xffffffff },
	{ "obmem",      0x0002, 0xffffffff },
	{ "obio",       0x0004, 0xffffffff },
	{ "vme16d16",   0x0100, (1<<16)-1 },
	{ "vme24d16",   0x0200, (1<<24)-(1<<16)-1 },
	{ "vme32d16",   0x0400, 0xfeffffff },
	{ "vme16d32",   0x1000, (1<<16) },
	{ "vme24d32",   0x2000, (1<<24)-(1<<16)-1 },
	{ "vme32d32",   0x4000, 0xfeffffff },
	{ (char *)0,    0,      0 }
};

/*
 * See if the device canbe connected to a nexus
 */
can_nexus(dp)
register struct device_entry *dp;
{
	if(dp->d_type == BUS)
		return(1);
	else
		return(0);
}


char *tbl_pseudo_uba[] = { "kdb", "kdm", "klesib", 0 } ;
 
/*
 * look up this device in a table to see if it needs a pseudo_uba.
 */
needs_pseudo_uba(str)
register	char	*str;
{
	register	char	**ptr = tbl_pseudo_uba;

	while(*ptr) if(!strcmp(str,*ptr++)) return(1);
	
	return(0);
}

Print_dev(dev)
struct device_entry *dev;
{
char Str[256];
printf("\n ENTRY +++++++++++++++\n");
switch (dev->d_type) {
      case CPU:
	printf("\t\td_type %s\n","CPU");
	break;
      case BUS:
	printf("\t\td_type %s\n","BUS");
	break;
      case CONTROLLER:
	printf("\t\td_type %s\n","CONTROLLER");
        break;
      case DEVICE:
        printf("\t\td_type %s\n","DEVICE");
        break;
      case PSEUDO_DEVICE:
        printf("\t\td_type %s\n","PSEUDO DEVICE");
        break;
      default:
        printf("\t\td_type %d\n",dev->d_type);
      }
       
printf("\t\t*d_conn 0x%x\n \t\t*d_name %s\n \t\t*d_dynamic %s\n \t\t*d_vec 0x%x\n \t\td_pri 0x%x\n \t\td_addr 0x%x\n \t\td_addr2 0x%x\n \t\td_ivnum 0x%x\n \t\td_unit %d\n \t\td_drive %d\n \t\td_slave %d\n \t\td_rcntl 0x%x\n \t\td_dk 0x%x\n \t\td_flags 0x%x\n \t\td_adaptor 0x%x\n \t\td_nexus 0x%x\n \t\td_extranum 0x%x\n \t\td_counted 0x%x\n \t\t*d_next 0x%x\n \t\td_mach 0x%x\n \t\td_bus 0x%x\n \t\td_fields[NFIELDS] 0x%x\n \t\td_bin 0x%x\n \t\td_addrmod 0x%x\n ",
       dev->d_conn, 
       dev->d_name, 
       dev->d_dynamic, 
       dev->d_vec, 
       dev->d_pri, 
       dev->d_addr, 
       dev->d_addr2, 
       dev->d_ivnum, 
       dev->d_unit, 
       dev->d_drive, 
       dev->d_slave, 
       dev->d_rcntl, 
       dev->d_dk, 
       dev->d_flags, 
       dev->d_adaptor, 
       dev->d_nexus, 
       dev->d_extranum, 
       dev->d_counted, 
       dev->d_next, 
       dev->d_mach, 
       dev->d_bus, 
       dev->d_fields[0], 
       dev->d_bin, 
       dev->d_addrmod
       ); 
}

Exec_unix_command(when, command_str)
int when;
char *command_str;
{
		struct callout_data *callout_p = 
			(struct callout_data *)malloc(sizeof (struct callout_data));
		      /* Save all the callout entries in a link list
		       * for use in several places in config processing.
		       * It contains a event class to control when a unix 
		       * command will be run.
		       */
		callout_p->event_class = when;
		callout_p->unix_command = ns(command_str);
		callout_p->next =  (struct callout_data *) 0;
		if (callout_hd == (struct callout_data *) 0)
		{
		        callout_hd = callout_p;
		        callout_lst = callout_p;
			callout_p->last = (struct callout_data *) 0;
		} else {
			callout_p->last =  callout_lst;
		        callout_lst->next =  callout_p;
			callout_lst = callout_p;
		}
		if(( when < 0) || ( when > MAX_EVENT_LEVEL))
			yyerror("Callout function timing out of range\n\tlegal values range from 1 - %d\n",MAX_EVENT_LEVEL -1 );
		

		if(val_id)
			free(val_id);
}

save_orphen(dp,device,num,type)
struct device_entry *dp;
char *device;
int num;
int type;
{

	/* create an entry at end of the orphen list for 
	 * this device.
	 */

	orph_p = (struct orphen_dev *)malloc(sizeof (struct orphen_dev));
	orph_p->dev = dp->d_name;
	orph_p->num = dp->d_unit;
	orph_p->dev_to = device;
	orph_p->num_to = num;
	orph_p->type = DEVICE;
	orph_p->next = orph_hdp;
	orph_hdp = orph_p;

}



