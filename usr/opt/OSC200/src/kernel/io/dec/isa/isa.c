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
static char *rcsid = "@(#)$RCSfile: isa.c,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/06/24 22:40:44 $";
#endif

/* routines for general isa bus support
 * taken from io.c for beta rom
 *
 *
 *   Modifications:
 *
 *    05-Oct-92	  Tim Burke - changed usage of "printf" to "printf".
 *		  Fixed up include syntax for isa.h.
 *
 *    24-Sep-92   Paul Grist
 *	Split BETA/Jensen code, for now ifdef on systye in the
 *	bus code for platform-specific needs. Jensen/Theta2 
 *	addressing macros now in ka_jensen.c, until further cleanup.
 *
 *    28-Aug-91   Joe Notarangelo
 *
 *       started this mess
 *
 * L. Stewart 12-19-91, newest versions from roms, with ldqp and stqp
 *
 */

#include <sys/types.h>
#include <io/common/handler.h>
#include <io/common/devdriver.h>
#include <io/dec/isa/isa.h>
#include <data/isa_data.c>
#include <hal/cpuconf.h>


extern struct isa_config_struct isa_conf[];


int isa_config_controller();
int eisa_config_controller();

/* Probe the ISA bus devices. */

void
isa_probe(struct bus *bus)
{
	struct apc_isa_struct *isap = apc_isa_drivers;
	int no_isa = NO_ISA;
	int probed;
	int unit;
	int isal;        /* isa interrupt level */
	ulong scb_vector;     /* pc irq channel */
	int i;
	int j;
	int ahaintr(), lnintr();


	for (i=0; i < no_isa; i++, isap++ ) {
		/* once through for each possible unit */
		for (unit = 0; unit < isap->max_units; unit++ ) {

		  if (strcmp(isap->device_name, "aha") == 0 || strcmp(isap->device_name, "ln") == 0)
		    {
		      for (j = 0; j < 7; j++)
			{
/* aha doesn't care what gets passed as an address to its probe function
 * The LANCE needs to know which slot is being probed
 * need a better idea of what the strategy is here...
 */
			  probed = eisa_config_controller(j, j, isap->device_name, bus);
			  if (probed)
			    break;
			}
		    }
		  else
		      probed = isa_config_controller(0, isap->device_name,bus);

			if (probed) {   /* probe successful */

				/* get isa level for device/unit */
				isal = isa_level( isap->device_name, unit );
				if (!isal) {
					printf("Bad ISA level for %s\n",
					       isap->device_name);
					continue;
				}

			/* get irqchan for this isa/eisa interrupt level */

				scb_vector = irq_to_scb_vector( isal );

				/* allocate vector in scb for interrupts */
				intrsetvec(scb_vector, isap->intr_handler,
						 isap->intr_parameter );
				
				i82357_maskon( isal );

		  } /* probed */
		} /* max_units */
	} /* no_isa */
}

/*	isaconfl1()
 *
 *	ISA bus level 1 configuration routine:
 *
 *	This procedure calls the probe routine for each ISA device.
 *
 */

isaconfl1(local_bus, eisa_bus)
struct bus *local_bus, *eisa_bus;
{
    int found;
    register struct controller *ctlr;
    register int i, j;
    int savebus;
    char *savebusname;

    /*
     * Modify based on cobra lbusconfl1 routine
     * This is now a connected bus, not system bus
     */

    eisa_bus->bus_type = BUS_ISA;
    eisa_bus->alive |= ALV_ALIVE;
    printf("%s%d at ibus0\n",
	   eisa_bus->bus_name, eisa_bus->bus_num);
    conn_bus(local_bus, eisa_bus);

    isa_probe(eisa_bus);

    return(1);
}

/* ISA buslevel 2 configuration routine:
 *
 *	Nothing to do, I think.  wtfix
 */
isaconfl2(bustype, binfo, bus)
int bustype;
caddr_t binfo;
struct bus *bus;
{
	return(1);
}

/* isa_level returns the isa interrupt level specified for a device/controller
 *       in the config file, if none was specified in the config file then
 *       zero will be returned
 *
 *      name - pointer to name of device
 *      unit - unit/controller number to look up
 */
int
isa_level( char *name, int unit )
{
   struct isa_config_struct *isac = isa_conf;

   while( *(isac->isa_name) ){   /* null signifies end of isa conf structure */
     if( (strcmp( isac->isa_name, name ) == 0 ) && (unit == isac->isa_unit) ) {
       /* found a match */
       return( isac->isa_level );
     }

     isac++;

   }
   return( 0 );
}


/* isa_dmachan returns the isa dma channel 
 * number specified for a device/controller
 *       in the config file, if none was specified in the config file then
 *       zero will be returned
 *
 *      name - pointer to name of device
 *      unit - unit/controller number to look up
 */
int
isa_dmachan( char *name, int unit )
{
   struct isa_config_struct *isac = isa_conf;

   while( *(isac->isa_name) ){   /* null signifies end of isa conf structure */

     if( (strcmp( isac->isa_name, name ) == 0 ) && (unit == isac->isa_unit) )
       /* found a match */
       return( isac->isa_channel );

     isac++;

   }

   return( 0 );
}


/* isa_csr returns the isa csr address  specified for a device/controller
 *       in the config file, if none was specified in the config file then
 *       zero will be returned
 *
 *      name - pointer to name of device
 *      unit - unit/controller number to look up
 */
int
isa_csr( char *name, int unit )
{
	struct isa_config_struct *isac;

	printf("isa CSR for %s unit %d\n", name, unit);
	isac = isa_conf;
	while( *(isac->isa_name) ){ /* null signifies end of isa_conf struct.*/
		printf("isa_csr: name %s unit %d\n",isac->isa_name,
			isac->isa_unit);
		if( (strcmp( isac->isa_name, name ) == 0 )
		   && (unit == isac->isa_unit) ) {
			/* found a match */
			printf("found match %d!\n", isac->isa_csr);
			return( isac->isa_csr );
		}
		isac++;
	}
	printf("isa csr -- no match\n");
	return( 0 );
}
