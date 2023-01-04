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
 * loadimage.c
 */

#if !defined(lint) && defined(SECONDARY)
static char	*sccsid = "@(#)$RCSfile: loadimage.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:38:22 $";
#endif lint

#if !defined(COFF) && !defined(MACHO)
#error Must define at least one object file format (COFF or MACHO)
#endif

#include "../../../sys/param.h"
#include "../../cpu.h"
#include "../../entrypt.h"

#define printf _prom_printf
extern int prom_io;
extern int ub_argc;
extern char **ub_argv;
extern char **ub_envp;

/**************************************************************************
 *STARTCOFF- start of COFF specific information                           *
 *                                                                        *
 **************************************************************************/
#if defined(COFF)
#include <a.out.h>

/*
 * format of bootable a.out file headers
 */
struct execinfo {
  struct filehdr fh;
  AOUTHDR ah;
};


/*
 * getxfile -- load binary image
 */

/*load_coff (load_image)
 *
 *function:
 *    Load a coff format image
 *
 *inputs:
 *    io - input file descriptor
 *
 *outputs:
 *    NONE
 *
 *returns:
 *    NONE
 *
 */
#if defined (COFF) && !defined(MACHO)
#define load_coff load_image
#endif
int
load_coff(io)
int io;
{
	struct execinfo  ei;
	HDRR		*si;
	unsigned int	 i, j;

	if (read(io, &ei, sizeof(ei)) != sizeof(ei)) {
		printf("bad a.out format\n");
		goto bad;
	}
	if (N_BADMAG(ei.ah)) {
		printf("bad magic number\n");
		goto bad;
	}

	lseek(io, N_TXTOFF(ei.fh, ei.ah), 0);
#ifdef SECONDARY
	printf ("\nSizes:\ntext = %d\n", ei.ah.tsize);
#endif /* SECONDARY */
	if (read(io, ei.ah.text_start, ei.ah.tsize)
	    != ei.ah.tsize) {
		printf("short read\n");
		goto bad;
	}

#ifdef SECONDARY
	printf ("data = %d\n", ei.ah.dsize);
#endif /* SECONDARY */
	if (read(io, ei.ah.data_start, ei.ah.dsize)
	    != ei.ah.dsize) {
		printf("short read\n");
		goto bad;
	}

#ifdef SECONDARY
	printf ("bss  = %d\n", ei.ah.bsize);
#endif /* SECONDARY */
	bzero(ei.ah.bss_start, ei.ah.bsize);

	/*
	 * Read in symbols, if there are any and if they fit 
	 */
	bzero( ei.ah.bss_start + ei.ah.bsize, sizeof ei);

	if (ei.fh.f_symptr > 0) {
#ifdef	SECONDARY
		printf("symtab = ");
#endif	/* SECONDARY */

		/*
		 * Pass along all the information we have and are about
		 * to lose, e.g. the exec struct and symtab header
		 */
		si = (HDRR*) (ei.ah.bss_start + ei.ah.bsize + sizeof ei);

		lseek(io, ei.fh.f_symptr, 0);
		if ((i = read(io, si, ei.fh.f_nsyms)) != ei.fh.f_nsyms) {
			printf("short read\n");
			goto nosyms;
		}

		/* how much stuff is there */
		i = si->cbExtOffset + si->iextMax * cbEXTR;
		j = i - (ei.fh.f_symptr +  ei.fh.f_nsyms);

		if (read(io, (char*)si + ei.fh.f_nsyms, j) != j) {
			printf("Inconsistent symtable\n");
			goto nosyms;
		}

		bcopy( &ei, ei.ah.bss_start + ei.ah.bsize, sizeof ei);

#ifdef	SECONDARY
		printf("%d\n", i - ei.fh.f_symptr);
#endif	/* SECONDARY */
	}
nosyms:

	/* All done */
	_prom_close(prom_io);

#ifdef SECONDARY
	printf ("Starting at 0x%x\n\n", ei.ah.entry);
#endif /* SECONDARY */
	(*((int (*) ()) ei.ah.entry)) (ub_argc,ub_argv,ub_envp);
bad:
	return(-1);
}

#endif /* COFF */


/*************************************************************************
 *ENDCOFF - End of COFF specific code                                    *
 *                                                                       *
 *************************************************************************/


/*************************************************************************
 *STARTMACHO - Start of MACHO specific code                                   *
 *                                                                       *
 *************************************************************************/

#if defined(MACHO)

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
#define printf _prom_printf
#define gets _prom_gets
#define stop _prom_restart
#endif /* STAND_ALONE */


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
  printf("max prot            : %d %x(0x0)\n",
     ldcregion->regc_maxprot,ldcregion->regc_maxprot);
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

 
/*load_macho (load_image)
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
#if !defined (COFF) && defined(MACHO)
#define load_macho load_image
#endif

int load_macho(io)
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

  loadOffset = 0;

  /* process each load command */
  for (ldcid = 0; ldcid < ldcmc->lcm_nentries; ldcid++){
    if (process_load_command(io,ldCmdBuf,mohdr,
                             ldcmc->lcm_map[ldcid],loadOffset) < 0){
      return (-1);
    }
  }

  printf("Starting at 0x%x\n\n",programEntryPoint);
#ifdef LOADSYMBOLS
  lseek(io,symbolTableOffset,0);
  printf("symbol table size   : %d 0x%x\n",symbolTableLength,symbolTableLength);
  if (read(io, regionHigh, symbolTableLength) !=
      symbolTableLength){
    printf("short read on symbol table \n");
    return(-1);
  }
  regionHigh = regionHigh + symbolTableLength;
#endif /* LOADSYMBOLS */


#if STAND_ALONE
  /* Start this puppy running */
  /* if this is not a multimax then call as if it were a procedure entry */

  (*((int (*) ())programEntryPoint)) (ub_argc, ub_argv, ub_envp);

#endif /* STAND_ALONE */
  return(-1);
}
#endif /*MACHO */


/*************************************************************************
 *ENDMACHO  - end of MACHO specific information                          *
 *                                                                       *
 *************************************************************************/

#if defined(COFF) && defined(MACHO)

#define COFF_FORMAT 1
#define MACHO_FORMAT 2
typedef struct execinfo coff_obj_t;
typedef struct mo_header_t mach_obj_t;
union objtype {
 coff_obj_t coff;
 mach_obj_t mach;
};

/* define a macro for checking magic number */
#define MACHO_BADMAG(x) ((x).moh_magic != MOH_MAGIC)

/*chkObjFormat
 *
 *function:
 *    routine to determine the correct object file format
 *inputs:
 *    io - input file descriptor
 *
 *outputs:
 *    NONE
 *
 *returns:
 *    object file format
 *
 */

int chkObjFormat(io)
     int io;


{
  union objtype *objhdr;
  char read_buffer[MO_SIZEOF_RAW_HDR];
  char decode_buffer[MO_SIZEOF_RAW_HDR];
  int ioStatus;
  
  if ((ioStatus = read(io,&read_buffer,MO_SIZEOF_RAW_HDR)) < 1){
    printf("Error reading object header \n \n");
    return(-1);
  }

  /* go back to the head of the file */
  lseek(io,0,0);
  /* turn the canonical header into machine specific format */
  decode_mach_o_hdr(&read_buffer,MO_SIZEOF_RAW_HDR,MOH_HEADER_VERSION,
		    (mo_header_t *)&decode_buffer);
 
 (char *)objhdr = &decode_buffer;
 if (!(MACHO_BADMAG(objhdr->mach))){
    return(MACHO_FORMAT);
  }
  else {
    /* the header has been converted.. use the original if its coff */
    (char *)objhdr = &read_buffer;
    if (!(N_BADMAG(objhdr->coff.ah))){
      return(COFF_FORMAT);
    }
  }

return(-1);
}


#endif /*SECONDARY*/
/*load_image
 *
 *function:
 *    load image reads the object file header information, process's it
 *    and loads the image. Depending on how it is built loadimage either
 *    is built with knowledge of the object file format, or determines
 *    the format at runtime.
 *
 *inputs:
 *    io - input file descriptor 

 *outputs:
 *    NONE
 *returns:
 *    NONE
 *
 */
#if defined (COFF) && defined(MACHO)
void
load_image(io)
int io;
{
	/*
	 * if this is a secondary boot then room is not a problem.
	 * Check for the object file format at runtime and load
	 * the image accordingly.
	 */
	switch (chkObjFormat(io)){
	case COFF_FORMAT:
		load_coff(io);
		break;
	case MACHO_FORMAT:
		load_macho(io);
		break;
	default:
		printf("error unknown object file format\n\n");
	}
}
#endif
