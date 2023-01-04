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
static char	*sccsid = "@(#)$RCSfile: libld.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:05:39 $";
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
 *	This file provides utilities of general use for accessing an
 *	OSF/ROSE object file.
 *
 */

/* 
Description: 

   Utilities used by the compiler tools to read and write parts
   of a mach-O object file.

Routines exported:

	ldOpen()	- open an object file by name
	ldInit()	- given an open-file, initialize OSF/ROSE data structures
	ldClose()	- close an open object file

	ldAlloc()	- Allocate an object-file descriptor block
	ldFree()	- Free an object-file descriptor AND subsidiary data structures

	ldIsObject()	- Check to see if a file is an object file

	ldReadSyms()	- Read loader symbols
	ldReadStrings()	- Read in string-sections
	ldReadSection() - Read in section info

****************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include <ldfcn.h>

/* static functions */

static int read_load_commands(ldfile *);
static void map_command(ldc_header_t *, ldfile *, mo_long_t);
static int read_header(ldfile *);
static void add_to_region_list(ldfile *,region_command_t *, mo_long_t);
static void * xmalloc( size_t );

/* macros */
#ifdef DEBUG
#	define TRACE(x) x
#else
#	define TRACE(x)
#endif

#define FIRST_CMDS(x) ((x->ldci_cmd_type == LDC_REGION) || \
                      (x->ldci_cmd_type == LDC_SYMBOLS) )



/*
  Routine:  ldfile *ldOpen (char*)
 
  Description: This routine takes the file handle which already has
               the filename as input.
               It checks to make sure this is a Mach-O file and 
	       proceeds to construct a file handle to the Mach-O
	       file.  The file handle is described in ld_macho.h.

  Inputs:   filename - name of Mach-O file to open

  Outputs:  ldfile*  - pointer to the file handle structure (ldfile)

  Side Effects: This routine fills in a file handle (assumed already allocated)
                with a portion of the information filled in.  All the 
                load commands information is filled but the section 
		information is not.  This handle can be used as input to 
		other utility routines to fill in the section information 
		such as symbols.
*/


ldfile* ldOpen( char *filename)
  {
  long ret;
  char *fn;
  ldfile	*ldptr = ldAlloc();

  ldptr->ld_filename = filename;

  ldptr->ld_macho_fd = fopen(ldptr->ld_filename, "r" );

  if (!ldptr->ld_macho_fd) {	/* Error in open, return NULL, errno indicates failure */
      return(NULL); 
    }

  TRACE(fprintf(stderr, ldptr->ld_filename));

  ldptr->ld_is_open_flag = 1; /* entry is open now! */

  ldptr = ldInit(ldptr);

  return(ldptr);
}

/*
 * Routine:	ldfile* ldInit( ldfile* )
 *
 * Description:	This routine takes an already opened ldfile and
 *		attempts to fill in the major object-format fields.
 */
ldfile *
ldInit( ldfile	*lp)
{
  assert( lp->ld_is_open_flag );

  lp->ld_starting_offset = ftell( lp->ld_macho_fd );

  if (read_header(lp) == LD_FAIL) {
    TRACE(fprintf(stderr, "Could not read header.\n"));
    return NULL;
  } else if (!read_load_commands(lp))
    return NULL;

  fseek( lp->ld_macho_fd, lp->ld_starting_offset, SEEK_SET);

  return lp;

}





/* 
 * If the ldfile structure has an open file descriptor associated with
 * it, close the file descriptor.
 */
void
ldClose( ldfile *lp)
{
  if (lp->ld_is_open_flag)
    {
      assert(lp->ld_macho_fd);
      fclose(lp->ld_macho_fd);
      lp->ld_is_open_flag = 0;
      lp->ld_macho_fd = NULL;
    }
}

/*
 * ldAlloc - allocate a new ldfile structure
 */

ldfile *
ldAlloc(void) {
  return (ldfile *) calloc( 1, sizeof (ldfile));
}


/*
 * ldFree - release an ldfile and its subordinate structures
 */

void
ldFree( ldfile * lp )
{
  int	i;

  /*
   * Make sure it's really there AND that the file has been closed
   */
  assert( (lp != NULL) && !lp->ld_is_open_flag );

  free( lp->ld_map_vector );

  for(i=0; i < MAX_USAGE_TYPE; i++) /* For each 'type' of region... */
    {
      ld_region *region = lp->ld_region_list[i].ld_next;
      ld_region *temp;

      while(region) {
	temp = region->ld_next;
	free( region );
	region = temp;
      }
    }

  free(lp->ld_debug);
  free(lp->ld_strings);
  free(lp->ld_symbols);

  free( lp );
}


/*
 * read_load_commands - Initialize the loader map and
 * 			start constructing the loader commands for the Mach-O file.
 *
 *			Returns -1 for errors, 0 for success.  errno gets set to reflect error
 */
static int read_load_commands(p)
ldfile *p;
  {
  int ret, i;
  int ncmds;
  char *tmp;
  mo_long_t type;
  char *ld_cmds;
  ldc_header_t *hdr_ptr;

  load_cmd_map_command_t *ld_cmd_map_cmd;
  mo_offset_t *map_addr;
  char *cmd_addr;

  /* initialization */
  p->ld_region_count = 0;

  
  /* read in the loader commands */
  ld_cmds = (char *) xmalloc(p->ld_header.moh_sizeofcmds);
  
  if( fseek(p->ld_macho_fd, 
	      p->ld_header.moh_first_cmd_off+p->ld_starting_offset, SEEK_SET))
      {
      TRACE( fprintf(stderr, "Couldn't locate first macho loader command with offset %d\n",	\
	     p->ld_header.moh_first_cmd_off));

      return (-1);
      }  

  ret = fread(ld_cmds, 1, p->ld_header.moh_sizeofcmds, p->ld_macho_fd);

  if (ret != p->ld_header.moh_sizeofcmds)
      {
      return (-1);
      }

  /* allocate the translation vector */
  p->ld_map_vector = (char **) xmalloc (p->ld_header.moh_n_load_cmds * 
		 			 sizeof(char*));

  ld_cmd_map_cmd = (load_cmd_map_command_t *) ( ((char *) ld_cmds) +
                    (p->ld_header.moh_load_map_cmd_off -
		     p->ld_header.moh_first_cmd_off));

  cmd_addr = (char *) ld_cmds;

  /* point at the load command map */
  map_addr = (mo_offset_t *) &ld_cmd_map_cmd->lcm_map[0];

  /* build the load_command/ptr translation vector */
  ncmds = p->ld_header.moh_n_load_cmds;
  for(i=0; i<ncmds; i++) {
    p->ld_map_vector[i] = (cmd_addr + 
			   (map_addr[i] - p->ld_header.moh_first_cmd_off));
  }

  /* make a pass over the load commands to make sure we
     allocate the region map before we see any other load commands that
     are tied to the ld_region structure such as relocation. We should
     make sure we read in the symbol load commands before the string load
     commands as well. */

  for(i=0; i<ncmds; i++) {

    /* move to next load command */
    hdr_ptr = (ldc_header_t *) p->ld_map_vector[i];

    if (FIRST_CMDS(hdr_ptr))
      (void) map_command(hdr_ptr, p, i);
  }

  /* now map the other commands to the ldfile structure */
  for(i=0; i<ncmds; i++) {
    /* move to next load command */
    hdr_ptr = (ldc_header_t *) p->ld_map_vector[i];

    if (!(FIRST_CMDS(hdr_ptr)))
      (void) map_command(hdr_ptr, p, i);

  }
}



static void map_command
(
  ldc_header_t *hdr_ptr,
  ldfile *p,
  mo_long_t lcount
)
  {

  long ret, i, reg_id;
  region_command_t *reg_cmd;
  mo_lcid_t owner_reg_id;
  reloc_command_t *rel_cmd;
  symbols_command_t *sym_cmd;
  package_command_t *pkg_cmd;

  ld_region *region;

  switch (hdr_ptr->ldci_cmd_type)
      {
      case 0: 
	/* this is an undefined type */
	TRACE(fprintf(stderr, "ERROR...undefined load command\n"));
	break;

      case LDC_CMD_MAP:
	TRACE(fprintf(stderr, "LDC_CMD_MAP\n"));
       	p->ld_map_cmd = (load_cmd_map_command_t*) hdr_ptr;
        break;

      case LDC_INTERPRETER:
	TRACE(printf("LDC_INTERPRETER\n"));
	break;

      case LDC_STRINGS:
	TRACE(fprintf(stderr, "LDC_STRINGS\n"));

	/* 
	  currently there is only one string section but
	  we need to be able to go from string load command
	  to symbol load command.  
	*/
	p->ld_strings_cmd = (strings_command_t *) hdr_ptr;
        break;

      case LDC_REGION:
	TRACE(fprintf(stderr, "LDC_REGION\n"));

	reg_cmd = (region_command_t*) hdr_ptr;
	(void) add_to_region_list(p, reg_cmd, lcount);
        break;

      case LDC_RELOC:
	TRACE(fprintf(stderr, "LDC_RELOC\n"));

	rel_cmd = (reloc_command_t*)hdr_ptr;
        owner_reg_id = rel_cmd->relc_owner_section;

	/* walk the region list looking for the right region to put
	   the relocation info */
	for (i=1; i <= MAX_USAGE_TYPE; i++)
	    {
	    if (p->ld_region_list[i].ld_region_lcid == owner_reg_id)
	        { 
	        p->ld_region_list[i].ld_reloc_cmd =(reloc_command_t*) hdr_ptr;
		return;
	        }

	    /* search thru the linked list of duplicate usage type regions */
	    region = &p->ld_region_list[i];
	    while (region->ld_next != NULL)
	        {
		region = region->ld_next;
		if (region->ld_region_lcid = owner_reg_id)
		    {
		    p->ld_region_list[i].ld_reloc_cmd = 
		                      (reloc_command_t*) hdr_ptr;
		    return;
		    }
		}
	    }

	fprintf(stderr, "Couldn't find region for relocation command\n");
	assert(0);
        break;
  
      case LDC_PACKAGE:
	pkg_cmd = (package_command_t *) hdr_ptr;

	TRACE(fprintf(stderr, "LDC_PACKAGE\n"));

	switch(pkg_cmd->pkgc_flags)
	    {
	    case PKG_EXPORT_F:
		p->ld_package_export_cmd = (package_command_t *) hdr_ptr;
		break;

	    case PKG_IMPORT_F:
	        p->ld_package_import_cmd = (package_command_t *) hdr_ptr;
		break;

	    default:
	       fprintf(stderr, "Invalid package command flag in %s", 
		       p->ld_filename);
	    }
        break;


      case LDC_SYMBOLS:
	sym_cmd = (symbols_command_t*) hdr_ptr;

	/* switch on the type of symbols load command */
	switch (sym_cmd->symc_kind) 
	  {
	  case SYMC_DEFINED_SYMBOLS:	
	    {
	    p->ld_def_cmd = (symbols_command_t*) hdr_ptr;
	    TRACE(fprintf(stderr, "Defined SYMBOLS\n"));
	    break;
	    }

          case SYMC_IMPORTS:
	    {
	    p->ld_imp_cmd = (symbols_command_t*) hdr_ptr;
	    TRACE(fprintf(stderr, "Import SYMBOLS\n"));
	    break;
	    }

          case SYMC_STABS:
	    {
	    p->ld_debug_cmd = (symbols_command_t*) hdr_ptr;
	    TRACE(fprintf(stderr, "Debug SYMBOLS\n"));
	    break;
	    }

          default:
	    fprintf(stderr, "Invalid symbol load command type\n");
	    break;
	  }

      case LDC_ENTRY:
	TRACE(fprintf(stderr, "LDC_ENTRY\n"));

	p->ld_main_entry = (entry_command_t*) hdr_ptr;
        break;

      case LDC_FUNC_TABLE:
	TRACE(fprintf(stderr, "LDC_TERMINATION\n"));

	p->ld_func_cmd = (func_table_command_t *) hdr_ptr;
        break;

      case LDC_GEN_INFO:
	TRACE(fprintf(stderr, "LDC_GEN_INFO\n"));

	p->ld_gen_info = (gen_info_command_t *) hdr_ptr;
        break;

      default:
	/* We should actually put unknown load commands somewhere. */
	break;
      }

   }



/*

  Routine:  int read_header(ldfile*)
 
  Description: 

  Inputs:   

  Outputs:  

  Side Effects: 

*/

int read_header(ldfile *ldptr)
{
  int ret;
  raw_mo_header_t	header;


  if (fseek(ldptr->ld_macho_fd, ldptr->ld_starting_offset, SEEK_SET))
    return(LD_FAIL);


  ret = fread((char*) &header, 1, sizeof(raw_mo_header_t), ldptr->ld_macho_fd);

  if (ret != sizeof(raw_mo_header_t))
    return (LD_FAIL);

  /* Now make sure that this is a Mach-O file */

  if (! ldIsObject( &header, sizeof(raw_mo_header_t), MOH_HEADER_VERSION, &ldptr->ld_header))
    {
    return(LD_FAIL);
    }

  return(LD_SUCCESS);
  } 


/*
  Routine:  
 
  Description: Check to see if the raw header is a true Mach-O file object header

  Inputs:   

  Outputs:  TRUE if this is a valid object header
  	    FALSE if it's not!

  Side Effects: Copies the raw header into the output_header, possibly converting from
  		network byte order to host-machine order.

*/
/* This is just a stub until we get the real thing */
int ldIsObject( void *raw_header, size_t size, unsigned long version, mo_header_t *output_header)
{
  
  int rslt;

  if (!decode_mach_o_hdr( raw_header, size, version, output_header))
    return TRUE;
  else
    return FALSE;
}


/*
  Routine:   add_to_region_list(region_command_t *)
 
  Description: add a region command to the ld_region_list according to
               its usage type.

  Inputs:   p - ldfile pointer
            reg_cmd - a pointer to the region command
            lcount - loader command index

  Outputs:  

  Side Effects: 

*/

void
add_to_region_list(ldfile *p,
		   region_command_t *reg_cmd,
		   mo_long_t lcount )
{
  int i;
  int usage_type;
  ld_region *region;
  ld_region *new_reg;

  usage_type = reg_cmd->regc_usage_type;

  /* get the ld_region structure for the usage type specified */
  region = (ld_region*) &p->ld_region_list[usage_type];

  /* If the vm_size field is 0 as is the case for object files from
     the assembler, fill it in initially with the section length*/
  if (reg_cmd->regc_vm_size == 0)
    reg_cmd->regc_vm_size = reg_cmd->ldc_section_len;

  /* if the slot is empty we can just add this region command */
  if (region->ld_reg_cmd == 0) 
      {
      p->ld_region_list[usage_type].ld_reg_cmd = reg_cmd;
      p->ld_region_list[usage_type].ld_region_lcid = lcount;

      return;
      }


  /* list is not empty...go to the end...this is a duplicate usage type */
  while (region->ld_next != NULL)
     region = region->ld_next;
  
  new_reg = (ld_region *) xmalloc(sizeof(ld_region));
  region->ld_next = new_reg;
  new_reg->ld_reg_cmd = reg_cmd;

  /* the usage type for this command had better be in the right range */
  assert (usage_type <= MAX_USAGE_TYPE);
  }  


/*
  Routine:   ldReadStrings(ldfile *)
 
  Description:  Read the string table of file ENTRY into core.
                Assume it is already open.
                Also record whether a GDB symbol segment follows the 
		string table. 

  Inputs:   

  Outputs:  

  Side Effects: 

*/

/*
 * This routine must read the entry strings into core. The space for these strings has already
 * been allocated by someone else. The strings fall into (at least) 2 categories:
 *
 *    strings associated with the imports symbols
 *    strings associated with the defined symbols
 *
 * Either of these may be absent from the file, and the routine must deal with this gracefully.
 *
 */

int
ldReadStrings (ldfile *entry)
{
  int ret;
  
  /*for now, let's assume there is only one string section */

  if (!entry->ld_strings_cmd) return 0;		/* no strings load-command */

  /* Read strings */
  if (fseek (entry->ld_macho_fd, 
	     entry->ld_strings_cmd->ldc_section_off+entry->ld_starting_offset,
	     SEEK_SET))
    {
      return -1;				/* SEEK ERROR! */
    };

  if(!entry->ld_strings)			/* No space allocated? */
    entry->ld_strings = xmalloc(STRING_SIZE(entry));

  ret = fread (entry->ld_strings, 
	      1,
	      entry->ld_strings_cmd->ldc_section_len,
	      entry->ld_macho_fd);

  return (ret != entry->ld_strings_cmd->ldc_section_len);

}



/*
  Routine:   ldReadSyms(ldfile *)
 
  Description:  Read the symbols of file ENTRY into core.
                Assume it is already open, on descriptor DESC.

  Inputs:   ldfile pointer

  Side Effects: allocate the symbols and fill in the pointer  
                for the file handle 

*/

int
ldReadSyms (ldfile *entry )
{
  int str_size;
  int ret;

  /* DEFINED symbols */
  if (entry->ld_def_cmd != NULL)
      entry->ld_symsize = entry->ld_def_cmd->ldc_section_len;    
    
  if (entry->ld_imp_cmd != NULL)
      entry->ld_symsize += entry->ld_imp_cmd->ldc_section_len;    

  /* allocate enough for both symbol sections */
  entry->ld_symbols = (symbol_info_t *) xmalloc(entry->ld_symsize);

  /* get the defined symbols first */
  if (entry->ld_def_cmd != NULL)
    {
    if (fseek (entry->ld_macho_fd, 
	       entry->ld_def_cmd->ldc_section_off + 
	           entry->ld_starting_offset, 
	       SEEK_SET))
      return -1;


    if (fread (entry->ld_symbols, 
	       1,
	       entry->ld_def_cmd->ldc_section_len,
	       entry->ld_macho_fd ) != entry->ld_def_cmd->ldc_section_len)
      return -1;
    }


  /* IMPORT symbols */
  if (entry->ld_imp_cmd != NULL)
    {
    entry->ld_imports = entry->ld_symbols + entry->ld_def_cmd->symc_nentries;
    if (fseek (entry->ld_macho_fd, 
	       entry->ld_imp_cmd->ldc_section_off + 
	           entry->ld_starting_offset, 
	       SEEK_SET))
        return -1;

    if (fread (entry->ld_imports, 
	       1,
	       entry->ld_imp_cmd->ldc_section_len,
	       entry->ld_macho_fd) != entry->ld_imp_cmd->ldc_section_len)
      return -1;
    }

  return 0;
}



/*
  Routine:  read_section_info
 
  Description:  Allocate a buffer and read in the section information 
                for the specified load command.  

  Inputs:   entry - ldfile structure 
            ldc   - load_command that contains info to seek to the
	            part of the file containing the section information.

  Side Effects: allocate a buffer and read in the section information 
                from the file designated by entry.

*/

char *
ldReadSection(entry, ldc)
     ldfile *entry;
     ldc_header_t *ldc;
{
  char *retptr;

  if (ldc == NULL)
    {
    TRACE(fprintf (stderr, "NULL load command\n"));
    return NULL;
    };

  if (fseek (entry->ld_macho_fd,
	     ldc->ldci_section_off+entry->ld_starting_offset, 
	     SEEK_SET))
    return NULL;

  retptr =(char *) xmalloc(ldc->ldci_section_len);

  if (ldc->ldci_section_len != 
      fread (retptr, 
	     1,
	     ldc->ldci_section_len,
	     entry->ld_macho_fd))
    {
      free( retptr );
      return NULL;
    }
  
  return retptr;
}



/* Like malloc but get fatal error if memory is exhausted.  */

static void *
xmalloc (size_t size)
{
  void	*result = malloc (size);

  if (!result && size)		/* NULL is okay if we asked for ZERO space */
    {
      fprintf(stderr, "libld: virtual memory exhausted\n");
      exit(1);
    }

  return result;
}

      
