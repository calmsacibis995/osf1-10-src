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
static char	*sccsid = "@(#)$RCSfile: mach_o_boot.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:08:34 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from mach_o_boot.c	2.1	(ULTRIX/OSF)	12/3/90";
 */
/*
 * mach_o_boot
 *
 * This module contains routines for loading/bootstraping an executable image
 * into memory and transfering control to its entry point.
 *
 */ 
/*
 *mach_o_boot.c
 */
/*
 * Author: Pete Benoit 6/12/90
 *
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif


/* these files describe the object format */ 

 /* these should be in one include file called standio.h */

 /* define some local constants */
#define DEBUG 0                        /* if 1 print lots of debug info */
#define NULL 0
#define BYTESPERWORD 4
#define STAND_ALONE 1                  /* if 1 stand_alone else user_mode test*/
#define MAX_LOAD_CMD_BUF_SIZE 4*1024   /* create a 4kb heap for load commands */

/* if user mode test then do things slightly differently */
#if STAND_ALONE
#include <mach_o_header.h>
#include <mach_o_format.h>

#else
#include "mach_o_header.h"
#include "mach_o_format.h"
#include <stdio.h>
#include <sys/file.h>
#endif


 /* define inline functions */
#define LAST_LD_CMD LDC_GEN_INFO+1
#define GET_LOAD_CMD(buf,hdr,cmd_off) (buf - hdr->moh_first_cmd_off + cmd_off) 

char *ldCmdTable[LAST_LD_CMD]= {"LDC_UNDEFINED",
			 "LDC_CMD_MAP",
			 "LDC_INTERPRETER",
			 "LDC_STRINGS",
			 "LDC_REGION",
			 "LDC_RELOC",
			 "LDC_PACKAGE",
			 "LDC_SYMBOLS",
			 "LDC_ENTRY",
			 "LDC_FUNC_TABLE",
			 "LDC_GEN_INFO"};
#define GET_LD_CMD_TXT(cmd) (ldCmdTable[cmd])

/* define some variables that will be global */

#if STAND_ALONE
#ifdef mips
#define printf _prom_printf
#define gets _prom_gets
#define stop _prom_restart
#endif mips
#endif STAND_ALONE

#ifdef mmax
extern char *end;                     /* ending address of image */
#endif mmax

/* some variables needed by loadimage */
int ub_argc;                              /* argc for bootee */
char **ub_argv;                           /* argv for bootee */
char **ub_envp;                           /* envp for bootee */

/* define module global variables */
mo_vm_addr_t programEntryPoint;            /* entry point                 */
mo_offset_t  symbolTableOffset;            /* SYMBOL table offset in file */
mo_long_t    symbolTableLength;            /* SYMBOL table length in file */
mo_vm_addr_t regionLow,regionHigh;   /* Start&end addr of all regions*/
unsigned     os_size;                      /* regionHigh - regionLow      */
mo_vm_addr_t loadOffset;                   /* addr to start load          */
char *vers="1.0";

/*process_ldc_symbols
 *
 *function:
 *     This procedure processes the symbols load command struct.
 *     It basically loads the symbol table
 *
 *inputs:
 *     io - input file descriptor
 *     ldcsymbols - a pointer to the symbols load command struct
 * 
 *
 *outputs:
 *     success >= 0
 *
 */

int process_ldc_symbols(io,ldcsymbols)
     int io;
     symbols_command_t *ldcsymbols;
{

#if DEBUG
  printf("kind                : %d 0x%x\n",ldcsymbols->symc_kind,
                                           ldcsymbols->symc_kind);
  printf("nentries            : %d 0x%x\n",ldcsymbols->symc_nentries,
                                           ldcsymbols->symc_nentries);
  printf("pck list            : %d 0x%x\n",ldcsymbols->symc_pkg_list,
                                           ldcsymbols->symc_pkg_list);
  printf("strings section     : %d 0x%x\n",ldcsymbols->symc_strings_section,
                                           ldcsymbols->symc_strings_section);
  printf("relocation addr     : %d 0x%x\n",ldcsymbols->symc_reloc_addr,
                                           ldcsymbols->symc_reloc_addr);
#endif
  
  symbolTableOffset = ldcsymbols->ldc_section_off;
  symbolTableLength = ldcsymbols->ldc_section_len;

return(0);
}

/*process_ldc_region
 *
 *function:
 *     This procedure processes the region load command
 *     It basically loads the executable code, data and bss
 *
 *
 *inputs:
 *     io - input file descriptor
 *     ldcregion - a pointer to the region load command struct
 *     loadOffset - offset to do actual load
 *
 *outputs:
 *     success >= 0
 *     
 */
int process_ldc_region(io,ldcregion,loadOffset)
     int io;
     region_command_t *ldcregion;
     mo_vm_addr_t loadOffset;
{
  mo_vm_addr_t base;

#if DEBUG
  printf("adr lcid            : %d %x(0x0)\n",
     ldcregion->regc_region_name.adr_lcid,ldcregion->regc_region_name.adr_lcid);
  printf("adr section         : %d %x(0x0)\n",
     ldcregion->regc_region_name.adr_sctoff,
     ldcregion->regc_region_name.adr_sctoff);
  printf("rel/vm addr         : %d %x(0x0)\n",
     ldcregion->regc_rel_addr,ldcregion->regc_rel_addr);
  printf("vm size             : %d %x(0x0)\n",
     ldcregion->regc_vm_size,ldcregion->regc_vm_size);
  printf("flags               : %d %x(0x0)\n",
     ldcregion->regc_flags,ldcregion->regc_flags);
  printf("reloc addr          : %d %x(0x0)\n",
     ldcregion->regc_reloc_addr,ldcregion->regc_reloc_addr);
  printf("align               : %d %x(0x0)\n",
     ldcregion->regc_addralign,ldcregion->regc_addralign);
  printf("usage               : %d %x(0x0)\n",
     ldcregion->regc_usage_type,ldcregion->regc_usage_type);
  printf("init protection     : %d %x(0x0)\n",
     ldcregion->regc_initprot,ldcregion->regc_initprot);
#endif

  /* if we have a null region then just ignore it and return */
  if (ldcregion->regc_vm_size == 0){
    return;
  }

  printf("region size         : %d 0x%x\n",ldcregion->regc_vm_size,
	                                   ldcregion->regc_vm_size);

  /* figure out the highest and lowest address's for the regions */
  if(regionHigh == (mo_vm_addr_t)-1){
    regionHigh = ldcregion->regc_vm_addr+ldcregion->regc_vm_size; 
    regionLow  = ldcregion->regc_vm_addr;
  }

  /* save the highest addr */
  if(regionHigh < (ldcregion->regc_vm_addr+ldcregion->regc_vm_size)){
    regionHigh = ldcregion->regc_vm_addr + ldcregion->regc_vm_size;
  }
  /* save the lowest addr */
  if(regionLow > ldcregion->regc_vm_addr){
    regionLow = ldcregion->regc_vm_addr;
  }

  /* load the region */
#if STAND_ALONE
  if(ldcregion->ldc_section_len > 0){
    lseek(io,ldcregion->ldc_section_off,0);
    (unsigned)base = (unsigned)ldcregion->regc_vm_addr + (unsigned)loadOffset;
    if (read(io,base,
	     ldcregion->ldc_section_len) !=
                                   ldcregion->ldc_section_len){
      printf("error reading  region \n");
      return(-1);
    }
  }else{
    /* zero out the bss section */
    (unsigned)base = (unsigned)ldcregion->regc_vm_addr + (unsigned)loadOffset;
    bzero(base, ldcregion->regc_vm_size);
  }
#endif
    
return(0);
}

/*process_ldc_entry
 *
 *function:
 *     This procedure processes the entry load command
 *
 *inputs:
 *      io - input file descriptor
 *      ldcentry - a pointer to the entry load command struct
 *
 *outputs:
 *      ENTRY POINT
 *
 */

int process_ldc_entry(io,ldcentry,cmdOff)
     int io;
     entry_command_t *ldcentry;
{
#if DEBUG
  printf("flags               :%x\nabsaddr             :%x\nentry lcid          :%x\nsec off             :%x\n\n",
	 ldcentry->entc_flags,ldcentry->entc_absaddr,ldcentry->entc_entry_pt.adr_lcid,ldcentry->entc_entry_pt.adr_sctoff);
#endif

  if ((ldcentry->entc_flags & ENT_VALID_ABSADDR_F)) {
    printf("valid entry pt      : %x\n",ldcentry->entc_absaddr);
  }
  else{
    printf("NOT VALID ABSADDR ENTRY %x\n",ldcentry->entc_absaddr);
  }

  programEntryPoint = ldcentry->entc_absaddr;
  return(0);
}

/*process_load_command
 *
 *function:
 *     This procedure processes all valid load commands
 *
 *inputs:
 *     cmdOff - (from BOF) command offset
 *
 *
 *outputs:
 *     NONE
 *
 *
 */

int process_load_command(io,ldCmdBuf,mohdr,cmdOff,loadOffset)
     int io;
     int cmdOff;
     struct mo_header_t *mohdr;
     char *ldCmdBuf;
     mo_vm_addr_t loadOffset;
{
  struct ldc_header_t *ldchdr;

  ldchdr = (ldc_header_t *)GET_LOAD_CMD(ldCmdBuf,mohdr,cmdOff);
  if (ldchdr->ldci_cmd_type > LAST_LD_CMD){
    printf("error unknown command type Please see the creator type = \n\n",
     ldchdr->ldci_cmd_type);
    return(-1);
  }
#if DEBUG
  printf("\nld command type     :%s\n",GET_LD_CMD_TXT(ldchdr->ldci_cmd_type));
  printf("ld command size     :%d %x(0x0)\n",
	 ldchdr->ldci_cmd_size,ldchdr->ldci_cmd_size);
  printf("section offset      :%d %x(0x0)\n",
	 ldchdr->ldci_section_off,ldchdr->ldci_section_off);
  printf("section length      :%d %x(0x0)\n",
	 ldchdr->ldci_section_len,ldchdr->ldci_section_len);
#endif
  switch(ldchdr->ldci_cmd_type) {
       case LDC_UNDEFINED:
	  return(1);
	  break;
	case LDC_CMD_MAP:
	  return(1);
	  break;
	case LDC_INTERPRETER:
	  return(1);
	  break;
	case LDC_STRINGS:
	  return(1);
	  break;
	case LDC_REGION:
	  return(process_ldc_region(io,(region_command_t *)ldchdr,loadOffset));
	  break;
	case LDC_RELOC:
	  return(1);
	  break;
	case LDC_PACKAGE:
	  return(1);
	  break;
	case LDC_SYMBOLS:
	  return(process_ldc_symbols(io,(symbols_command_t *)ldchdr));
	  break;
	case LDC_ENTRY:
	  return(process_ldc_entry(io,(entry_command_t *)ldchdr));
	  break;
	case LDC_FUNC_TABLE:
	  return(1);
	  break;
	case LDC_GEN_INFO:
	  return(1);
	  break;
	}
return(1);
}

 
/*load_image
 *
 *function:
 *    This procedure actually loads a binary image in Mach-o (rose) format
 *    Its analogous to the "exec" procedure getxfile.
 *
 *inputs:
 *    io - file descriptor for the image file to load
 *    
 *outputs:
 *    NONE
 *
 *
 */

int load_image(io)
   int io;
{
  struct mo_header_t  *mohdr;
  char read_buffer[MO_SIZEOF_RAW_HDR],decode_buffer[MO_SIZEOF_RAW_HDR];
  struct ldc_header_t *ldchdr;
  struct load_cmd_map_command_t *ldcmc;
  char ldCmdBufHeap[MAX_LOAD_CMD_BUF_SIZE];
  char *ldCmdBuf;
  int ioStatus,ldcid;
  mo_vm_addr_t loadOffset;

  /* create some storage */
  ldCmdBuf = &ldCmdBufHeap[0];

  /* read in object header information */
  if ((ioStatus = read(io,&read_buffer,MO_SIZEOF_RAW_HDR)) < 1){
    printf("Error reading object header \n \n");
    return(-1);
  }

  /* decode the canonical header */
  (char *)mohdr = &decode_buffer;
  decode_mach_o_hdr(&read_buffer,MO_SIZEOF_RAW_HDR,MOH_HEADER_VERSION,
		    mohdr);
  if (mohdr->moh_magic != MOH_MAGIC){
    printf("error reading magic number from header %x\n\n",mohdr->moh_magic);
    return(-1);
  }

#if DEBUG
  printf(" magic number        %x\n cpu type            %x\n vendor type         %x\n load map cmd offset %x\n first cmd offset    %x\n size of cmd         %d 0x%x\n # of load cmds      %x\n page size           %x\n",
          mohdr->moh_magic,mohdr->moh_cpu_type,mohdr->moh_vendor_type,
	  mohdr->moh_load_map_cmd_off,mohdr->moh_first_cmd_off,
	  mohdr->moh_sizeofcmds,mohdr->moh_sizeofcmds,
	  mohdr->moh_n_load_cmds,mohdr->moh_max_page_size);
#endif


  /* allocate some storage for all commands */

  if (mohdr->moh_sizeofcmds > MAX_LOAD_CMD_BUF_SIZE){
    printf("Load command buffer overflow... Increase size of cmd buffer\n\n");
    return(-1);
  }

  /* now read in the commands  command map  */
  lseek(io,mohdr->moh_first_cmd_off,0);
  if ((ioStatus = read(io,ldCmdBuf,(long)mohdr->moh_sizeofcmds))< 0){
    printf("error reading load commands\n\n");
    return(-1);
  }

  /* get the load command map */
  ldcmc = (load_cmd_map_command_t *)GET_LOAD_CMD(ldCmdBuf,mohdr,
                          mohdr->moh_load_map_cmd_off);

  if (ldcmc->ldc_cmd_type > LAST_LD_CMD){
    printf("error command mismatch not load map command %i\n\n",
	   ldcmc->ldc_cmd_type);
    return(-1);
  }

#if DEBUG
  printf(" cmd type %x %s\n cmd size %x\n section offset %x\n section length %x\n",
           ldcmc->ldc_cmd_type,GET_LD_CMD_TXT(ldcmc->ldc_cmd_type),
           ldcmc->ldc_cmd_size,ldcmc->ldc_section_off,
           ldcmc->ldc_section_len);
  printf(" cmd strings %x\n cmd entries %x\n",ldcmc->lcm_ld_cmd_strings,
                                              ldcmc->lcm_nentries);
  for (ldcid = 0; ldcid < ldcmc->lcm_nentries; ldcid++){
    printf("ldcid = %x %d \n",ldcmc->lcm_map[ldcid],ldcmc->lcm_map[ldcid]);
  }

#endif

#ifdef mmax
  loadOffset = (mo_vm_addr_t)&end;
#else mmax
  loadOffset = 0;
#endif mmax

  /* process each load command */
  for (ldcid = 0; ldcid < ldcmc->lcm_nentries; ldcid++){
    if (process_load_command(io,ldCmdBuf,mohdr,
                             ldcmc->lcm_map[ldcid],loadOffset) < 0){
      return (-1);
    }
  }

  printf("Starting at %d 0x%x\n\n",programEntryPoint,programEntryPoint);

#ifdef LOADSYMBOLS
  lseek(io,symbolTableOffset,0);
  printf("symbol table size   : %d 0x%x\n",symbolTableLength,symbolTableLength);
#if STAND_ALONE
  lseek(io,symbolTableOffset,0);
  if (read(io, regionHigh, symbolTableLength) !=
      symbolTableLength){
    printf("short read on symbol table \n");
    return(-1);
  }
#endif STAND_ALONE
  regionHigh = regionHigh + symbolTableLength;
#endif LOADSYMBOLS

#if DEBUG
  printf("loadOffset           : %d 0x%x\n",loadOffset,loadOffset);
  printf("regionLow            : %d 0x%x\n",regionLow,regionLow);
  printf("regionHigh           : %d 0x%x\n",regionHigh,regionHigh);
  printf("os_size              : %d 0x%x\n",(unsigned)(regionHigh - regionLow),
                                            (unsigned)(regionHigh - regionLow));
#endif DEBUG

#if STAND_ALONE
  /* Start this puppy running */
  /* if this is not a multimax then call as if it were a procedure entry */
#ifndef mmax
  (*((int (*) ())programEntryPoint)) (ub_argc, ub_argv, ub_envp);
#else mmax
  /* its a multimax then we must do a transfer */
  os_size = (unsigned)(regionHigh - regionLow);
  printf("os size               : %x\n",os_size);
  mmax_transfer(regionLow,loadOffset,programEntryPoint,0,os_size);

#endif mmax
#endif STAND_ALONE
  return(-1);
}

/*main
 *
 * function:
 *     The main procedure takes one argument which is the name of the
 *     image to load. If the imagename is not passed as a parameter then
 *     the user is prompted for a file to load. If the file can be opened
 *     then control is passed to loadimage which loads the image into
 *     memory and then transfers control to its entry point
 *
 * inputs:
 *     argc - argc passed from the bootstrap code
 *     argv - argv passed from the bootstrap code
 *     envp - envp passed from the bootstrap code
 *
 * outputs:
 *     NONE
 *
 */

main(argc,argv,envp)
     int argc;
     char **argv, **envp;

{

#define INBUFSIZE 256
  
  char *imageName, askName[INBUFSIZE];
#if STAND_ALONE
  int getsStatus;
#else
  char *getsStatus;
#endif
  register howto, devtype;  /* needed by some impl's (ie vax) */
  int io;
  int loopIfLessThenZero,ioStatus;


  /* save prom arg's for kernel */
  ub_argc = argc;
  ub_argv = argv;
  ub_envp = envp;
  howto = 0;
  regionHigh = (mo_vm_addr_t) -1;
  regionLow  = (mo_vm_addr_t) -1;

  imageName = NULL; 

  /* ok output the song and dance number */
#if STAND_ALONE
  printf("\nMach-o Interlude boot - %s \n\n", vers);
#else
  printf("\nMach-o TEST Interlude boot - TEST \n\n");
#endif

  /* see if the user has passed the imagename as a parameter */
  if (argc > 1){
    imageName = argv[1];
    printf("argv [1] %s imageName %s\n",argv[1],imageName);
  }

  loopIfLessThenZero = -1;
  while (loopIfLessThenZero < 0){

    /* the imagename was not passed as a parameter get input from user */
    if (imageName == NULL){
#ifndef mmax 
      printf("Enter image name to load > ");
      getsStatus = gets(askName);    
#else  mmax
#if STAND_ALONE
      get_string("Enter image name to load > ",askName);
      if (strlen(askName) == 0){
	getsStatus = NULL;
      }else{
	mmax_init(askName);
	getsStatus = 's';
      }
#else STAND_ALONE
      printf("Enter image name to load > ");
      getsStatus = gets(askName);
#endif STAND_ALONE
      
#endif mmax

      /* user typed whitespace tell the boob to enter something */
      if (getsStatus == NULL) 
	continue;
      if (askName[0] == 0)
	continue;
      imageName = askName;
    }
    /* now try and open the file */
    if ((io = open(imageName,0)) < 0){
      printf("Can't open %s\n\n",imageName);
      imageName = NULL; 
      continue;
    }

    /* what are we loading */
    printf("\nLoading image %s...\n\n",imageName);

    /* now try and load the image */
    loopIfLessThenZero = load_image(io);
  
    /* should never return here but if we do close the file */
    close(io);
    /* set loopIfLessThenZero = -1 to try and boot another image */
    imageName = NULL;
  }


}












